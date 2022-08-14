#include <Application.h>

#include <string>
#include <cassert>
#include <iostream>
#include <array>
#include <algorithm>

// #include "MyDFT.h"
#include "MyMath.h"

// https://www.youtube.com/watch?v=ITnPS8HGqLo
// https://www.youtube.com/watch?v=spUNpyF58BY

// TODO: use mathematical convention for cartesian coordinates.

enum class Waveform: int
{
	Generated = 0,
	GeneratedFreqDomain,
	GeneratedFromDFT,

	SynthesizedFreqDomain,
	SynthesizedFromDFT
};

enum class SoundToPlay: int
{
	None = 0,

	Generated,
	GeneratedFromDFT,

	Synthesized
};

static constexpr const MyMath::Box BOUNDS{-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f}; // Bounds of the space in which objects are rendered.
static constexpr const MyMath::Mat4x4 ORTHO_PROJ_MAT = MyMath::OrthogonalProjectionMatrix(BOUNDS.back, BOUNDS.front, BOUNDS.right, BOUNDS.left, BOUNDS.bottom, BOUNDS.top); // Projection matrix used.
static constexpr const size_t SINE_SAMPLE_RATE = 8000; // Length of the signals. Corresponds to N (for time-domain signals) and K (for frequency-domain signals).
static constexpr const size_t SINE_FREQ = 440; // ISO standard "La" or "A".
static constexpr const float ANGLE_IN_RADS = -0.5f * MyMath::PI; // Clockwise 90° rotation.
static constexpr const float SYNTHESIZED_COMPLEX_MAGNITUDE = SINE_SAMPLE_RATE * 0.5f; // Magnitude that a frequency bin should have to rebuild the generated 440 Hz pure tone. The 1/2 factor is here because the spectrum should contain a peak at 440 Hz and 8000-440 Hz.
static const std::array<float, 5> offsets = {0.0f, 0.01f, 0.02f, 0.03f, 0.04f}; // Small offsets to be able to distinguish between overlapping signals. Has to be static const, not a constexpr const because the values are passed to lambdas.
static const std::array<MyApp::ColorBytes, 5> colors = {{ {255, 0,0,255}, {0,255,0,255}, {0,0,255,255}, {255,255,0,255}, {255,0,255,255} }}; // Color differences to differenciate between signals. Idem.

// Graphical rendering structures.
static MyMath::Mat4x4 objectRotation = MyMath::MAT4_IDENTITY; // Holds rotation of the model. Used to rotate waveforms around local Z.
static MyMath::Mat4x4 objectTranslation = MyMath::MAT4_IDENTITY; // Holds position of the model. Used to offset the waveforms on local Z.
static MyMath::Mat4x4 viewMatrix = MyMath::MAT4_IDENTITY; // Holds camera rotation. Used to rotate waveforms around world's Y.
static float accumulatedYaw = 0.0f; // Controlled with left mouse button. Allows you to rotate the signal around.
static float accumulatedPitch = 0.0f; // Controlled with left mouse button. Allows you to pitch the signal towards and away from camera.
static float samplesSpacing = 1.0f; // Controlled with right mouse button. Allows you to zoom into the signal.
static float accumulatedZoffset = 0.0f; // Controlled with scroll wheel. Allows you to scroll through the signal.

// Defines which waveforms to render.
static SoundToPlay toPlay = SoundToPlay::None;
static std::vector<Waveform> toDisplay = {};

// The signals to render.
static std::vector<float> generatedTimeDomain(SINE_SAMPLE_RATE, 0.0f); // Pure 440 Hz tone generated via code.
static std::vector<std::complex<float>> generatedFreqDomain(SINE_SAMPLE_RATE, 0.0f); // Fourier transform of the same tone.
static std::vector<float> generatedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f); // The pure 440 Hz tone reconstructed from the fourier transform of the pure 440 Hz tone.
static std::vector<float> synthesizedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f); // Manually constructed fourier transform. We'll be synthesizing the 440 Hz pure tone with it.
static std::vector<std::complex<float>> synthesizedFreqDomain(SINE_SAMPLE_RATE, 0.0f); // Tone reconstructed from the manually constructed fourier transform.

/**
* Implementation of euler's identity e^(i*x) = cos(x) + i*sin(x)
*/
std::complex<float> EulersFormula(const float x)
{
	return std::complex<float>(std::cosf(x), std::sinf(x));
}

/**
* Discrete Fourier Transform. Computes the frequency-domain representation of a time-domain signal.
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
inline float GenerateSine(const float n, const float sampleRate, const float frequency)
{
	// 2*PI is a full rotation. n/sampleRate ensures that the full rotation happens at 1 Hz. frequency then changes the pitch of this "unit pitch".
	return std::sinf((2.0f * MyMath::PI * n / sampleRate) * frequency);
}

/**
* Rotates the waveform.
* 
* @param relx Screenspace x movement of the mouse.
* @param rely Screenspace y movement of the mouse.
*/
inline void ProcessLMB(const float relx, const float rely)
{
	constexpr const float RIGHT_ANGLE = MyMath::PI * 0.5f;
	accumulatedYaw += relx;
	accumulatedPitch -= rely;
	// Rotate signal model about the Z axis.
	objectRotation = MatrixMultiplication(MyMath::MAT4_IDENTITY, MyMath::RotationMatrix(MyMath::VEC3_UP, RIGHT_ANGLE * accumulatedYaw));
	// Pitch the camera up / down. Pitching the camera rather than the model ensures that the percieved pitching of the model is always done towards / away from the camera.
	viewMatrix = MatrixMultiplication(MyMath::MAT4_IDENTITY, MyMath::RotationMatrix(MyMath::VEC3_LEFT, RIGHT_ANGLE * accumulatedPitch));
}

/**
* Scales the waveform by spacing out the samples on the Z axis.
* 
* @param relx Screenspace x movement of the mouse.
* @param rely Screenspace y movement of the mouse. Unused.
*/
inline void ProcessRMB(const float relx, const float rely)
{
	(void)rely; // rely is unused.
	samplesSpacing += relx;
	if(samplesSpacing <= 0.0f) samplesSpacing = 0.01f; // To prevent reversing the signal, which looks weird.
}

/**
* Scrolls through the signal by applying a Z offset on the waveform's model.
* 
* @param relx Horizontal scrolling, if any. Most mice don't have a horizontal scroll wheel. Unused.
* @param relx Vertical scrolling.
*/
inline void ProcessScrollWheel(const float relx, const float rely)
{
	(void)relx; // relx is unused.
	objectTranslation.m23 -= rely;
}

/**
* Resets all model and camera transformations. Useful when you've lost view of the signal.
*/
inline void ResetTransformations()
{
	accumulatedYaw = 0.0f;
	accumulatedPitch = 0.0f;
	samplesSpacing = 1.0f;
	objectRotation = {};
	objectTranslation = {};
	viewMatrix = {};
}

/**
* Draws the signal as a series of lines in 3D space where X = 1, Y = i and Z = frequency.
* 
* @param signal The complex-valued signal to be drawn.
* @param sdl The SdlManager to be used to draw the lines.
* @param color The color of the lines. Useful for distinguishing between multiple signals on screen.
* @param offset The global offset on the Z axis of the lines. Useful for distinguishing between multiple signals on screen.
*/
inline void RenderFrequencyDomainSignal(const std::vector<std::complex<float>>& signal, MyApp::SdlManager& sdl, const MyApp::ColorBytes color, const float& offset)
{
	// Compute scaling factor so that the element with the biggest value fits on screen.
	float biggestComponent = -1.0f;
	for(const auto& complex : signal)
	{
		if(std::fabs(complex.real()) > biggestComponent) biggestComponent = std::fabs(complex.real());
		if(std::fabs(complex.imag()) > biggestComponent) biggestComponent = std::fabs(complex.imag());
	}
	const float amplitudeScale = 1.0f / biggestComponent;

	// Offset the samples on the Z axis.
	MyMath::Mat4x4 translationWithOffset = objectTranslation;
	translationWithOffset.m23 += offset;

	const float halfDisplaySize = sdl.displaySize * 0.5f;
	// Draw each frequency bin as a line.
	for(size_t n = 0; n < signal.size(); n++)
	{
		// Single vertices. No model transformation since it's assumed to be an identity matrix.
		MyMath::Vec4 pt0, pt1; // World / model position.
		const float zPos = float(n) / float(signal.size());
		pt0 = {0.0f,								0.0f,								samplesSpacing * zPos, 1.0f}; // XY center of the signal.
		pt1 = {signal[n].real() * amplitudeScale,	signal[n].imag() * amplitudeScale,	samplesSpacing * zPos, 1.0f}; // Position of the sample.

		// World / model space transformations.
		// Rotate rotate around Z.
		pt0 = MatrixVectorMultiplication(objectRotation, pt0);
		pt1 = MatrixVectorMultiplication(objectRotation, pt1);
		// Offset on Z.
		pt0 = MatrixVectorMultiplication(translationWithOffset, pt0);
		pt1 = MatrixVectorMultiplication(translationWithOffset, pt1);

		// To view space.
		pt0 = MatrixVectorMultiplication(viewMatrix, pt0);
		pt1 = MatrixVectorMultiplication(viewMatrix, pt1);

		// Cull points outside the viewing volume.
		if(pt0.x < BOUNDS.back		|| pt0.x > BOUNDS.front	||
		   pt0.y < BOUNDS.right		|| pt0.y > BOUNDS.left	||
		   pt0.z < BOUNDS.bottom	|| pt0.z > BOUNDS.top)	continue;
		if(pt1.x < BOUNDS.back		|| pt1.x > BOUNDS.front	||
		   pt1.y < BOUNDS.right		|| pt1.y > BOUNDS.left	||
		   pt1.z < BOUNDS.bottom	|| pt1.z > BOUNDS.top)	continue;

		// To clip space.
		pt0 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt0);
		pt1 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt1);

		assert(pt0.x >= -1.0f && pt0.x <= 1.0f &&
			   pt0.y >= -1.0f && pt0.y <= 1.0f &&
			   pt0.z >= -1.0f && pt0.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");
		assert(pt1.x >= -1.0f && pt1.x <= 1.0f &&
			   pt1.y >= -1.0f && pt1.y <= 1.0f &&
			   pt1.z >= -1.0f && pt1.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");

		// To screen space following "Viewport transform" section of: https://www.khronos.org/opengl/wiki/Viewport_Transform note: adjusted to fit SDL's coordinate convention.
		const MyMath::Vec2 windowPt0 =
		{
								 halfDisplaySize * -pt0.y + halfDisplaySize,
			sdl.displaySize -	(halfDisplaySize *  pt0.z + halfDisplaySize)
			// Depth (x component) is discarded, we don't need it. No perspective divide since we're using orthogonal projection.
		};
		const MyMath::Vec2 windowPt1 =
		{
								 halfDisplaySize * -pt1.y + halfDisplaySize,
			sdl.displaySize -	(halfDisplaySize *  pt1.z + halfDisplaySize)
		};
		assert(windowPt0.x >= 0.0f && windowPt0.x <= sdl.displaySize &&
			   windowPt0.y >= 0.0f && windowPt0.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");
		assert(windowPt1.x >= 0.0f && windowPt1.x <= sdl.displaySize &&
			   windowPt1.y >= 0.0f && windowPt1.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");

		// Draw the line representing this sample.
		sdl.RenderLine(windowPt0.x, windowPt0.y, windowPt1.x, windowPt1.y, color);
	}
}

/**
* Draws the signal as a series of lines in 3D space where X = 1, Y = i and Z = time.
*
* @param signal The real-valued signal to be drawn.
* @param sdl The SdlManager to be used to draw the lines.
* @param color The color of the lines. Useful for distinguishing between multiple signals on screen.
* @param offset The global offset on the Z axis of the lines. Useful for distinguishing between multiple signals on screen.
*/
inline void RenderTimeDomainSignal(const std::vector<float>& signal, MyApp::SdlManager& sdl, const MyApp::ColorBytes color, const float offset)
{
	// Compute scaling factor so that the element with the biggest value fits on screen.
	float biggestComponent = -1.0f;
	for(const auto& amplitude : signal)
	{
		if(std::fabs(amplitude) > biggestComponent) biggestComponent = std::fabs(amplitude);
	}
	const float amplitudeScale = 1.0f / biggestComponent;

	// Offset the samples on the Z axis.
	MyMath::Mat4x4 translationWithOffset = objectTranslation;
	translationWithOffset.m23 += offset;

	// Draw each sample as a line.
	const float halfDisplaySize = sdl.displaySize * 0.5f;
	for(size_t n = 0; n < signal.size(); n++)
	{
		// Single vertices. No model transformation since it's assumed to be an identity matrix.
		MyMath::Vec4 pt0, pt1; // World / model position.
		const float zPos = float(n) / float(signal.size());
		pt0 = {0.0f,						0.0f,	samplesSpacing * zPos, 1.0f}; // XY center of the signal.
		pt1 = {amplitudeScale * signal[n],	0.0f,	samplesSpacing * zPos, 1.0f}; // Position of the sample.

		// World / model space transformations.
		// Rotate rotate around Z.
		pt0 = MatrixVectorMultiplication(objectRotation, pt0);
		pt1 = MatrixVectorMultiplication(objectRotation, pt1);
		// Offset on Z.
		pt0 = MatrixVectorMultiplication(translationWithOffset, pt0);
		pt1 = MatrixVectorMultiplication(translationWithOffset, pt1);

		// To view space.
		pt0 = MatrixVectorMultiplication(viewMatrix, pt0);
		pt1 = MatrixVectorMultiplication(viewMatrix, pt1);

		// Cull points outside the viewing volume.
		if(pt0.x < BOUNDS.back		|| pt0.x > BOUNDS.front	||
		   pt0.y < BOUNDS.right		|| pt0.y > BOUNDS.left	||
		   pt0.z < BOUNDS.bottom	|| pt0.z > BOUNDS.top)	continue;
		if(pt1.x < BOUNDS.back		|| pt1.x > BOUNDS.front ||
		   pt1.y < BOUNDS.right		|| pt1.y > BOUNDS.left	||
		   pt1.z < BOUNDS.bottom	|| pt1.z > BOUNDS.top)	continue;

		// To clip space.
		pt0 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt0);
		pt1 = MatrixVectorMultiplication(ORTHO_PROJ_MAT, pt1);

		assert(pt0.x >= -1.0f && pt0.x <= 1.0f &&
			   pt0.y >= -1.0f && pt0.y <= 1.0f &&
			   pt0.z >= -1.0f && pt0.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");
		assert(pt1.x >= -1.0f && pt1.x <= 1.0f &&
			   pt1.y >= -1.0f && pt1.y <= 1.0f &&
			   pt1.z >= -1.0f && pt1.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");

		// To screen space following "Viewport transform" section of: https://www.khronos.org/opengl/wiki/Viewport_Transform note: adjusted to fit SDL's coordinate convention.
		const MyMath::Vec2 windowPt0 =
		{
								 halfDisplaySize * -pt0.y + halfDisplaySize,
			sdl.displaySize -	(halfDisplaySize *  pt0.z + halfDisplaySize)
			// Depth (x component) is discarded, we don't need it. No perspective divide since we're using orthogonal projection.
		};
		const MyMath::Vec2 windowPt1 =
		{
								 halfDisplaySize * -pt1.y + halfDisplaySize,
			sdl.displaySize -	(halfDisplaySize *  pt1.z + halfDisplaySize)
		};
		assert(windowPt0.x >= 0.0f && windowPt0.x <= sdl.displaySize &&
			   windowPt0.y >= 0.0f && windowPt0.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");
		assert(windowPt1.x >= 0.0f && windowPt1.x <= sdl.displaySize &&
			   windowPt1.y >= 0.0f && windowPt1.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");

		// Draw the line representing this sample.
		sdl.RenderLine(windowPt0.x, windowPt0.y, windowPt1.x, windowPt1.y, color);
	}
}

/**
* Displays an ImGui window to interact with the application. Shows the controls, allows to switch between signals played back and allows to show/hide the visualizations of different signals.
*/
inline void RenderImgui()
{
	constexpr const char* soundNames[4] = {"NONE", "Generated sine", "Generated sine reconstructed from it's DFT", "Sine synthesized from constructed DFT"};
	static std::array<bool, 5> whetherToDisplay({false, false, false, false, false});

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
	if(updateDisplayedWaveform)
	{
		toDisplay.resize(0);
		for(int i = 0; i < (int)whetherToDisplay.size(); i++)
		{
			if(whetherToDisplay[i]) toDisplay.push_back((Waveform)i);
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
		RenderImgui();
	});
	sdl_.RegisterMouseInputCallback(MyApp::Input::LEFT_MOUSE_BUTTON, [&](const float x, const float y)
	{
		constexpr const float MOUSE_SENSITIVITY = 0.001f;
		ProcessLMB(x * MOUSE_SENSITIVITY, y * MOUSE_SENSITIVITY);
	});
	sdl_.RegisterMouseInputCallback(MyApp::Input::RIGHT_MOUSE_BUTTON, [&](const float x, const float y)
	{
		constexpr const float WHEEL_SENSITIVITY = 1.0f;
		ProcessRMB(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY);
	});
	sdl_.RegisterMouseInputCallback(MyApp::Input::SCROLL_WHEEL, [&](const float x, const float y)
	{
		constexpr const float WHEEL_SENSITIVITY = 0.1f;
		ProcessScrollWheel(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY);
	});
	sdl_.RegisterInputCallback(MyApp::Input::R, [&]()
	{
		ResetTransformations();
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
						RenderTimeDomainSignal(generatedTimeDomain, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::GeneratedFreqDomain:
					{
						RenderFrequencyDomainSignal(generatedFreqDomain, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::GeneratedFromDFT:
					{
						RenderTimeDomainSignal(generatedTimeDomainFromDFT, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::SynthesizedFreqDomain:
					{
						RenderFrequencyDomainSignal(synthesizedFreqDomain, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::SynthesizedFromDFT:
					{
						RenderTimeDomainSignal(synthesizedTimeDomainFromDFT, sdl_, colors[i], offsets[i]);
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