#include "Application.h"

#include <easy/profiler.h>

MyApp::Application::Application(const unsigned int displaySize, const unsigned int sampleRate, const unsigned int bufferSize): sdl_(SdlManager(displaySize)), audioEngine_(AudioEngine(sampleRate, bufferSize)) {}

void MyApp::Application::Run(const std::vector<float>& generatedTimeDomain, const std::vector<float>& generatedTimeDomainFromDFT, const std::vector<float>& synthesizedTimeDomainFromDFT,
	const std::vector<std::complex<float>>& generatedFreqDomain, const std::vector<std::complex<float>>& synthesizedFreqDomain)
{
	EASY_PROFILER_ENABLE;

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

	OnStart();

	// Write time-domain signals to disk. Useful for checking if the audio artifacts come from the signal itself or from the hardware's processing.
	if (!assetManager_.WriteWav(generatedTimeDomain, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "generated.wav").c_str(), 1, generatedTimeDomain.size()))
	{
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	}
	if (!assetManager_.WriteWav(generatedTimeDomainFromDFT, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "generatedFromDFT.wav").c_str(), 1, generatedTimeDomainFromDFT.size()))
	{
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	};
	if (!assetManager_.WriteWav(synthesizedTimeDomainFromDFT, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "synthesizedFromDFT.wav").c_str(), 1, synthesizedTimeDomainFromDFT.size()))
	{
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	};

	bool shutdown = false;
	while (!shutdown)
	{
		EASY_BLOCK("Application's update");
		shutdown = sdl_.Update();
		audioEngine_.ProcessAudio();
		OnUpdate();
		UpdateToDisplayAndToPlay_(generatedTimeDomain, generatedTimeDomainFromDFT, synthesizedTimeDomainFromDFT, generatedFreqDomain, synthesizedFreqDomain);
	}

	OnShutdown();

#if BUILD_WITH_EASY_PROFILER
	const auto success = profiler::dumpBlocksToFile((std::string(APPLICATION_PROFILER_OUTPUTS_DIR) + "session.prof").c_str());
	if (!success) throw std::runtime_error(std::string("Failed to write easy profiler session to disk."));
#endif
}

void MyApp::Application::Callback_ProcessLMB_(const float relx, const float rely)
{
	constexpr const float RIGHT_ANGLE = MyMath::PI * 0.5f;
	accumulatedYaw_ += relx;
	accumulatedPitch_ -= rely;
	// Rotate signal model about the Z axis.
	modelRotation_ = MatrixMultiplication(MyMath::MAT4_IDENTITY, MyMath::RotationMatrix(MyMath::VEC3_UP, RIGHT_ANGLE * accumulatedYaw_));
	// Pitch the camera up / down. Pitching the camera rather than the model ensures that the percieved pitching of the model is always done towards / away from the camera.
	viewMatrix_ = MatrixMultiplication(MyMath::MAT4_IDENTITY, MyMath::RotationMatrix(MyMath::VEC3_RIGHT, RIGHT_ANGLE * accumulatedPitch_));
}

void MyApp::Application::Callback_ProcessRMB_(const float relx, const float rely)
{
	(void)rely; // rely is unused.
	samplesSpacing_ += relx;
	if (samplesSpacing_ <= 0.0f) samplesSpacing_ = 0.01f; // To prevent reversing the signal, which looks weird.
}

void MyApp::Application::Callback_ProcessScrollWheel_(const float relx, const float rely)
{
	(void)relx; // relx is unused.
	modelTranslation_.m23 -= rely;
}

void MyApp::Application::Callback_ResetTransformations_()
{
	accumulatedYaw_ = 0.0f;
	accumulatedPitch_ = 0.0f;
	samplesSpacing_ = 1.0f;
	modelRotation_ = {};
	modelTranslation_ = {};
	viewMatrix_ = {};
}

void MyApp::Application::Callback_RenderFrequencyDomainSignal_(const std::vector<std::complex<float>>& signal, const MyApp::ColorBytes color, const float& offset)
{
	// Compute scaling factor so that the element with the biggest value fits on screen.
	float biggestComponent = 0.0001f;
	for (const auto& complex : signal)
	{
		if (std::fabs(complex.real()) > biggestComponent) biggestComponent = std::fabs(complex.real());
		if (std::fabs(complex.imag()) > biggestComponent) biggestComponent = std::fabs(complex.imag());
	}
	const float amplitudeScale = 1.0f / biggestComponent;

	// Offset the samples on the Z axis.
	MyMath::Mat4x4 translationWithOffset = modelTranslation_;
	translationWithOffset.m23 += offset;

	const float halfDisplaySize = sdl_.displaySize * 0.5f;
	// Draw each frequency bin as a line.
	for (size_t n = 0; n < signal.size(); n++)
	{
		// Single vertices. No model transformation since it's assumed to be an identity matrix.
		MyMath::Vec4 pt0, pt1; // World / model position.
		const float zPos = float(n) / float(signal.size());
		pt0 = { 0.0f,								0.0f,								samplesSpacing_ * zPos, 1.0f }; // XY center of the signal.
		pt1 = { signal[n].real() * amplitudeScale,	signal[n].imag() * amplitudeScale,	samplesSpacing_ * zPos, 1.0f }; // Position of the sample.

		// World / model space transformations.
		// Rotate rotate around Z.
		pt0 = MatrixVectorMultiplication(modelRotation_, pt0);
		pt1 = MatrixVectorMultiplication(modelRotation_, pt1);
		// Offset on Z.
		pt0 = MatrixVectorMultiplication(translationWithOffset, pt0);
		pt1 = MatrixVectorMultiplication(translationWithOffset, pt1);

		// To view space.
		pt0 = MatrixVectorMultiplication(viewMatrix_, pt0);
		pt1 = MatrixVectorMultiplication(viewMatrix_, pt1);

		// Cull points outside the viewing volume.
		if (pt0.x < BOUNDS.back || pt0.x > BOUNDS.front ||
			pt0.y < BOUNDS.right || pt0.y > BOUNDS.left ||
			pt0.z < BOUNDS.bottom || pt0.z > BOUNDS.top)	continue;
		if (pt1.x < BOUNDS.back || pt1.x > BOUNDS.front ||
			pt1.y < BOUNDS.right || pt1.y > BOUNDS.left ||
			pt1.z < BOUNDS.bottom || pt1.z > BOUNDS.top)	continue;

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
								 halfDisplaySize * pt0.x + halfDisplaySize,
			sdl_.displaySize - (halfDisplaySize * pt0.z + halfDisplaySize)
			// Depth (x component) is discarded, we don't need it. No perspective divide since we're using orthogonal projection.
		};
		const MyMath::Vec2 windowPt1 =
		{
								 halfDisplaySize * pt1.x + halfDisplaySize,
			sdl_.displaySize - (halfDisplaySize * pt1.z + halfDisplaySize)
		};
		assert(windowPt0.x >= 0.0f && windowPt0.x <= sdl_.displaySize &&
			windowPt0.y >= 0.0f && windowPt0.y <= sdl_.displaySize && "Window point lies outside the screen's bounds.");
		assert(windowPt1.x >= 0.0f && windowPt1.x <= sdl_.displaySize &&
			windowPt1.y >= 0.0f && windowPt1.y <= sdl_.displaySize && "Window point lies outside the screen's bounds.");

		// Draw the line representing this sample.
		sdl_.RenderLine(windowPt0.x, windowPt0.y, windowPt1.x, windowPt1.y, color);
	}
}

void MyApp::Application::Callback_RenderTimeDomainSignal_(const std::vector<float>& signal, const MyApp::ColorBytes color, const float offset)
{
	// Compute scaling factor so that the element with the biggest value fits on screen.
	float biggestComponent = 0.0001f;
	for (const auto& amplitude : signal)
	{
		if (std::fabs(amplitude) > biggestComponent) biggestComponent = std::fabs(amplitude);
	}
	const float amplitudeScale = 1.0f / biggestComponent;

	// Offset the samples on the Z axis.
	MyMath::Mat4x4 translationWithOffset = modelTranslation_;
	translationWithOffset.m23 += offset;

	// Draw each sample as a line.
	const float halfDisplaySize = sdl_.displaySize * 0.5f;
	for (size_t n = 0; n < signal.size(); n++)
	{
		// Single vertices. No model transformation since it's assumed to be an identity matrix.
		MyMath::Vec4 pt0, pt1; // World / model position.
		const float zPos = float(n) / float(signal.size());
		pt0 = { 0.0f,						0.0f,	samplesSpacing_ * zPos, 1.0f }; // XY center of the signal.
		pt1 = { amplitudeScale * signal[n],	0.0f,	samplesSpacing_ * zPos, 1.0f }; // Position of the sample.

		// World / model space transformations.
		// // Rotate rotate around Z.
		pt0 = MatrixVectorMultiplication(modelRotation_, pt0);
		pt1 = MatrixVectorMultiplication(modelRotation_, pt1);
		// // Offset on Z.
		pt0 = MatrixVectorMultiplication(translationWithOffset, pt0);
		pt1 = MatrixVectorMultiplication(translationWithOffset, pt1);

		// To view space.
		pt0 = MatrixVectorMultiplication(viewMatrix_, pt0);
		pt1 = MatrixVectorMultiplication(viewMatrix_, pt1);

		// Cull points outside the viewing volume.
		if (pt0.x < BOUNDS.back || pt0.x > BOUNDS.front ||
			pt0.y < BOUNDS.right || pt0.y > BOUNDS.left ||
			pt0.z < BOUNDS.bottom || pt0.z > BOUNDS.top)	continue;
		if (pt1.x < BOUNDS.back || pt1.x > BOUNDS.front ||
			pt1.y < BOUNDS.right || pt1.y > BOUNDS.left ||
			pt1.z < BOUNDS.bottom || pt1.z > BOUNDS.top)	continue;

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
								 halfDisplaySize * pt0.x + halfDisplaySize,
			sdl_.displaySize - (halfDisplaySize * pt0.z + halfDisplaySize)
			// Depth (x component) is discarded, we don't need it. No perspective divide since we're using orthogonal projection.
		};
		const MyMath::Vec2 windowPt1 =
		{
								 halfDisplaySize * pt1.x + halfDisplaySize,
			sdl_.displaySize - (halfDisplaySize * pt1.z + halfDisplaySize)
		};
		assert(windowPt0.x >= 0.0f && windowPt0.x <= sdl_.displaySize &&
			windowPt0.y >= 0.0f && windowPt0.y <= sdl_.displaySize && "Window point lies outside the screen's bounds.");
		assert(windowPt1.x >= 0.0f && windowPt1.x <= sdl_.displaySize &&
			windowPt1.y >= 0.0f && windowPt1.y <= sdl_.displaySize && "Window point lies outside the screen's bounds.");

		// Draw the line representing this sample.
		sdl_.RenderLine(windowPt0.x, windowPt0.y, windowPt1.x, windowPt1.y, color);
	}
}

void MyApp::Application::Callback_RenderImgui_()
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

void MyApp::Application::UpdateToDisplayAndToPlay_(const std::vector<float>& generatedTimeDomain, const std::vector<float>& generatedTimeDomainFromDFT, const std::vector<float>& synthesizedTimeDomainFromDFT,
	const std::vector<std::complex<float>>& generatedFreqDomain, const std::vector<std::complex<float>>& synthesizedFreqDomain)
{
	// Update rendering callbacks if needed.
	static auto lastUpdateSounds = SoundToPlay::None;
	if (toPlay != lastUpdateSounds)
	{
		audioEngine_.DestroyAll();

		switch (toPlay)
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
	if (toDisplay != lastUpdateWaveforms)
	{
		sdl_.ClearRenderCallbacks();
		sdl_.RegisterRenderCallback([&]()
			{
				for (size_t i = 0; i < toDisplay.size(); i++)
				{
					switch (toDisplay[i])
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
