#include <Application.h>

#include <iostream>
#include <algorithm>

// TODO: use mathematical convention for cartesian coordinates.

// Literals to configure the demo.
static constexpr const size_t SINE_SAMPLE_RATE = 8000; // Length of the signals. Corresponds to N (for time-domain signals) and K (for frequency-domain signals).
static constexpr const size_t SINE_FREQ = 440; // ISO standard "La" or "A".
static constexpr const float ANGLE_IN_RADS = -0.5f * MyMath::PI; // Clockwise 90° rotation.
static constexpr const float SYNTHESIZED_COMPLEX_MAGNITUDE = SINE_SAMPLE_RATE * 0.5f; // Magnitude that a frequency bin should have to rebuild the generated 440 Hz pure tone. The 1/2 factor is here because the spectrum should contain a peak at 440 Hz and 8000-440 Hz.

// The signals to render.
static std::vector<float> generatedTimeDomain(SINE_SAMPLE_RATE, 0.0f); // Pure 440 Hz tone generated via code.
static std::vector<std::complex<float>> generatedFreqDomain(SINE_SAMPLE_RATE, 0.0f); // Fourier transform of the same tone.
static std::vector<float> generatedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f); // The pure 440 Hz tone reconstructed from the fourier transform of the pure 440 Hz tone.
static std::vector<float> synthesizedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f); // Manually constructed fourier transform. We'll be synthesizing the 440 Hz pure tone with it.
static std::vector<std::complex<float>> synthesizedFreqDomain(SINE_SAMPLE_RATE, 0.0f); // Tone reconstructed from the manually constructed fourier transform.

// Defines which waveforms to render.
enum class Waveform : int
{
	Generated = 0,
	GeneratedFreqDomain,
	GeneratedFromDFT,

	SynthesizedFreqDomain,
	SynthesizedFromDFT
};

enum class SoundToPlay : int
{
	None = 0,

	Generated,
	GeneratedFromDFT,

	Synthesized
};
SoundToPlay toPlay = SoundToPlay::None;
std::vector<Waveform> toDisplay = {};

/**
* Implementation of euler's identity e^(i*x) = cos(x) + i*sin(x)
*/
std::complex<float> EulersFormula(const float x)
{
	return std::complex<float>(std::cosf(x), std::sinf(x));
}

/**
* Discrete Fourier Transform. Computes the frequency-domain representation of a time-domain signal. For more information about the DFT, see https://www.youtube.com/watch?v=ITnPS8HGqLo and https://www.youtube.com/watch?v=spUNpyF58BY
*
* @param out Output of the function, the frequency bins resulting from the DFT. Ensure out.size() is K before calling this function.
* @param x Input real-valued signal. x.size() defines N.
* @param K Number of frequency bins that constitute the out signal. Any value K < N results in spectral loss so for lossless transfomation use K = x.size().
*/
void DFT(std::vector<std::complex<float>>& out, const std::vector<float>& x, const unsigned int K, const bool printProgress = true)
{
	// Just a lambda to print the progress of the computation since it can be quite lengthy.
	const auto PrintProgress = [](const unsigned int k, const unsigned int K)->void
	{
		if(k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing DFT: " << std::to_string(percent++) << "% done." << std::endl;
			if(percent > 99) percent = 0;
		}
	};

	const auto N = (unsigned int)x.size(); // Just an alias for the number of samples in the time-domain signal.
	std::vector<std::complex<float>>& y = out; // Just an alias for the output frequency-domain signal.

	std::fill(y.begin(), y.end(), std::complex<float>(0.0f, 0.0f)); // Set output signal to 0.

	for(unsigned int k = 0; k < K; ++k) // Frequency bins loop.
	{
		if(printProgress) PrintProgress(k, K);

		for(unsigned int n = 0; n < N; ++n) // Samples loop.
		{
			// Apply the euler's formula to compute e^i*(-2*PI*k*n/N), which is the operation required to compute a single frequency bin. We're using a negative value to wind the signal in the counter-clockwise fashion.
			y[k] += x[n] * EulersFormula(-2.0f * MyMath::PI * k * n / N); // Accumulate current sample's contribution to the current frequency bin.
		}
	}

	if(printProgress) std::cout << "DFT done." << std::endl;
}

/**
* Inverse Discrete Fourier Transform. Computes the time-domain representation of a frequency-domain signal.
*
* @param out Output of the function, the real-valued time-domain signal. Ensure out.size() is N before calling this function.
* @param y Input frequency bins, the frequency-domain representation of a signal. y.size() defines K.
* @param N Number of samples in the output real-valued signal out. Compute this as samplingFrequency * durationOfRealValuedSignal (as floats!). Setting this incorrectly results in a change in pitch (look up "pitch scaling").
*/
void IDFT(std::vector<float>& out, const std::vector<std::complex<float>>& y, const unsigned int N, const bool printProgress = true)
{
	// Just a lambda to print the progress of the computation since it can be quite lengthy.
	const auto PrintProgress = [](const unsigned int n, const unsigned int N)->void
	{
		if(n % (N / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing IDFT: " << std::to_string(percent++) << "% done." << std::endl;
			if(percent >= 100) percent = 0;
		}
	};

	const auto K = (unsigned int)y.size(); // Just an alias for the number of samples in the frequency-domain signal.
	std::vector<float>& x = out; // Just an alias for the output time-domain signal.

	std::fill(x.begin(), x.end(), 0.0f); // Set output signal to 0.

	for(unsigned int n = 0; n < N; ++n) // Samples loop.
	{
		if(printProgress) PrintProgress(n, N);

		for(unsigned int k = 0; k < K; ++k) // Frequency bins loop.
		{
			// Apply the euler's formula to compute e^i*(2*PI*k*n/N), which is the operation required to compute the influence of a single frequency bin on the time-domain signal. We're using a positive value to undo the initial winding done by the DFT.
			x[n] += (y[k] * EulersFormula(2.0f * MyMath::PI * k * n / N)).real(); // Accumulate current frequency bin's contribution to the current sample.
		}
		x[n] /= N; // Divide by N to obtain the normalized sample. See the following post for an explanation: https://math.stackexchange.com/questions/765085/why-divide-by-n-length-of-input-sequence-during-idft
		x[n] = std::clamp(x[n], -1.0f, 1.0f); // Out-of-bounds samples can be generated even with correct N due to float imprecision which results in crackling when played back by a device that expects a normalized PCM.
	}

	if(printProgress) std::cout << "IDFT done." << std::endl;
}

/**
* Generates a single sample of a pure tone.
* 
* @param n Index of the sample.
* @param sampleRate Number of samples to complete a single period with a frequency of 1 Hz.
* @param frequency The desired tone, in Hertz.
* @return Value of the signal at sample index n.
*/
float GenerateSine(const float n, const float sampleRate, const float frequency)
{
	// 2*PI is a full rotation. n/sampleRate ensures that the full rotation happens at 1 Hz. frequency then changes the pitch of this "unit pitch".
	return std::sinf((2.0f * MyMath::PI * n / sampleRate) * frequency);
}

/**
* Displays an ImGui window to interact with the application. Shows the controls, allows to switch between signals played back and allows to show/hide the visualizations of different signals.
*/
void Callback_RenderImgui_()
{
	constexpr const char* soundNames[4] = { "NONE", "Generated sine", "Generated sine reconstructed from it's DFT", "Sine synthesized from constructed DFT" };
	static std::array<bool, 5> whetherToDisplay({ false, false, false, false, false });

	// Draw the UI.
	ImGui::Begin("Controls", 0, ImGuiWindowFlags_AlwaysAutoResize);

	ImGui::Text("Left mouse button: rotate, right mouse button: scale,\nmouse wheel: scroll through the signal, R: reset the view.\n");

	ImGui::ListBox("Sound to play", (int*)&toPlay, soundNames, 4);

	bool updateDisplayedWaveform = false;
	updateDisplayedWaveform = ImGui::Checkbox("Show Generated in time-domain: ", &(whetherToDisplay[0])) ? true : updateDisplayedWaveform; // Set updateDisplayedWaveform to true only if there was a change.
	updateDisplayedWaveform = ImGui::Checkbox("Show Generated in frequency-domain: ", &(whetherToDisplay[1])) ? true : updateDisplayedWaveform;
	updateDisplayedWaveform = ImGui::Checkbox("Show reconstructed Generated in frequency-domain: ", &(whetherToDisplay[2])) ? true : updateDisplayedWaveform;
	updateDisplayedWaveform = ImGui::Checkbox("Show Synthesized in frequency-domain: ", &(whetherToDisplay[3])) ? true : updateDisplayedWaveform;
	updateDisplayedWaveform = ImGui::Checkbox("Show Synthesized in time-domain: ", &(whetherToDisplay[4])) ? true : updateDisplayedWaveform;

	ImGui::End();

	// Update container that defines what signals to draw.
	if (updateDisplayedWaveform)
	{
		toDisplay.resize(0);
		for (int i = 0; i < (int)whetherToDisplay.size(); i++)
		{
			if (whetherToDisplay[i]) toDisplay.push_back((Waveform)i);
		}
	}
}

/*
* Application's method ran once upon Application's startup, before the update loop.
*/
void MyApp::Application::OnStart()
{
	// Compute the signals we'll be visualizing / hearing.
	for(size_t n = 0; n < generatedTimeDomain.size(); n++)
	{
		// Generate a 440 Hz sine signal of length 8'000 samples.
		generatedTimeDomain[n] = GenerateSine(n, SINE_SAMPLE_RATE, SINE_FREQ);
	}
	// Synthesize a frequency-domain signal of what we hope is the same 440 Hz signal. Note that there are two non-zero entries due to DFT's mirrored nature. More info here: https://dsp.stackexchange.com/questions/4825/why-is-the-fft-mirrored
	// Note that we could have just generated a fourier transform with only one 2*SYNTHESIZED_COMPLEX_MAGNITUDE peak and still obtained the same signal. TODO: verify claim
	synthesizedFreqDomain[SINE_FREQ] = std::complex<float>(std::cosf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE,
														   std::sinf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE);
	synthesizedFreqDomain[SINE_SAMPLE_RATE - SINE_FREQ] = std::complex<float>(-std::cosf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE,
																			  -std::sinf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE);
	// Compute the DFT's and IDFT's of the generated / synthesized signals.
	DFT(generatedFreqDomain, generatedTimeDomain, SINE_SAMPLE_RATE);
	IDFT(generatedTimeDomainFromDFT, generatedFreqDomain, SINE_SAMPLE_RATE);
	IDFT(synthesizedTimeDomainFromDFT, synthesizedFreqDomain, SINE_SAMPLE_RATE);

	// Write time-domain signals to disk. Useful for checking if the audio artifacts come from the signal itself or from the hardware's processing.
	if(!assetManager_.WriteWav(generatedTimeDomain, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "generated.wav").c_str(), 1, SINE_SAMPLE_RATE))
	{
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	}
	if(!assetManager_.WriteWav(generatedTimeDomainFromDFT, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "generatedFromDFT.wav").c_str(), 1, SINE_SAMPLE_RATE))
	{
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	};
	if(!assetManager_.WriteWav(synthesizedTimeDomainFromDFT, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "synthesizedFromDFT.wav").c_str(), 1, SINE_SAMPLE_RATE))
	{
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	};

	// Register callbacks.
	sdl_.RegisterImguiCallback([&]()
	{
		Callback_RenderImgui_();
	});
	sdl_.RegisterMouseInputCallback(MyApp::Input::LEFT_MOUSE_BUTTON, [&](const float x, const float y)
	{
		constexpr const float MOUSE_SENSITIVITY = 0.001f;
		Callback_ProcessLMB_(x * MOUSE_SENSITIVITY, y * MOUSE_SENSITIVITY);
	});
	sdl_.RegisterMouseInputCallback(MyApp::Input::RIGHT_MOUSE_BUTTON, [&](const float x, const float y)
	{
		constexpr const float WHEEL_SENSITIVITY = 1.0f;
		Callback_ProcessRMB_(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY);
	});
	sdl_.RegisterMouseInputCallback(MyApp::Input::SCROLL_WHEEL, [&](const float x, const float y)
	{
		constexpr const float WHEEL_SENSITIVITY = 0.1f;
		Callback_ProcessScrollWheel_(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY);
	});
	sdl_.RegisterInputCallback(MyApp::Input::R, [&]()
	{
		Callback_ResetTransformations_();
	});
}

/*
* Application's method ran every time at the end of Application's update.
*/
void MyApp::Application::OnUpdate()
{
	// Update rendering callbacks if needed.
	static auto lastUpdateSounds = SoundToPlay::None;
	if(toPlay != lastUpdateSounds)
	{
		audioEngine_.DestroyAll();

		switch(toPlay)
		{
			case SoundToPlay::None:
			break;
			case SoundToPlay::Generated:
			{
				MyApp::Sound* generatedSound = audioEngine_.CreateSound(generatedTimeDomain);
				generatedSound->Play();
			}
			break;
			case SoundToPlay::GeneratedFromDFT:
			{
				MyApp::Sound* generatedSoundFromDFT = audioEngine_.CreateSound(generatedTimeDomainFromDFT);
				generatedSoundFromDFT->Play();
			}
			break;
			case SoundToPlay::Synthesized:
			{
				MyApp::Sound* synthesizedSoundFromDFT = audioEngine_.CreateSound(synthesizedTimeDomainFromDFT);
				synthesizedSoundFromDFT->Play();
			}
			break;
			default:
			break;
		}
	}
	lastUpdateSounds = toPlay;

	// Update played sound if needed.
	static auto lastUpdateWaveforms = std::vector<Waveform>();
	if(toDisplay != lastUpdateWaveforms)
	{
		sdl_.ClearRenderCallbacks();
		sdl_.RegisterRenderCallback([&]()
		{
			for(size_t i = 0; i < toDisplay.size(); i++)
			{
				switch(toDisplay[i])
				{
					case Waveform::Generated:
					{
						Callback_RenderTimeDomainSignal_(generatedTimeDomain, colors_[i], offsets_[i]);
					}break;

					case Waveform::GeneratedFreqDomain:
					{
						Callback_RenderFrequencyDomainSignal_(generatedFreqDomain, colors_[i], offsets_[i]);
					}break;

					case Waveform::GeneratedFromDFT:
					{
						Callback_RenderTimeDomainSignal_(generatedTimeDomainFromDFT, colors_[i], offsets_[i]);
					}break;

					case Waveform::SynthesizedFreqDomain:
					{
						Callback_RenderFrequencyDomainSignal_(synthesizedFreqDomain, colors_[i], offsets_[i]);
					}break;

					case Waveform::SynthesizedFromDFT:
					{
						Callback_RenderTimeDomainSignal_(synthesizedTimeDomainFromDFT, colors_[i], offsets_[i]);
					}break;

					default:
					break;
				}
			}
		});
	}
	lastUpdateWaveforms = toDisplay;
}

/*
* Application's method ran once upon Application's shutdown, before the shutdown of internal systems. Unused.
*/
void MyApp::Application::OnShutdown()
{

}

int main()
{
	MyApp::Application app = MyApp::Application(720, 8000, 2 * 512);

	app.Run();

	return 0;
}