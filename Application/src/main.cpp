#include <Application.h>

#include <string>
#include <cassert>
#include <iostream>
#include <array>

#include "MyDFT.h"
#include "MyMath.h"

inline float GenerateSine(const float n, const float sampleRate, const float frequency)
{
	return std::sinf((2.0f * MyMath::PI * n / sampleRate) * frequency);
}

inline void ProcessLMB(const float relx, const float rely, float& accumulatedYaw, float& accumulatedPitch, MyMath::Mat4x4& rotation, MyMath::Mat4x4& viewMatrix)
{
	constexpr const float RIGHT_ANGLE = MyMath::PI * 0.5f;

	accumulatedYaw += relx;
	accumulatedPitch -= rely;
	rotation = MatrixMultiplication(MyMath::MAT4_IDENTITY, MyMath::RotationMatrix(MyMath::VEC3_UP, RIGHT_ANGLE * accumulatedYaw));
	viewMatrix = MatrixMultiplication(MyMath::MAT4_IDENTITY, MyMath::RotationMatrix(MyMath::VEC3_LEFT, RIGHT_ANGLE * accumulatedPitch));
}

inline void ProcessRMB(const float relx, const float rely, float& samplesSpacing)
{
	samplesSpacing += relx;
	if (samplesSpacing <= 0.0f) samplesSpacing = 0.01f;
}

inline void ProcessScrollWheel(const float relx, const float rely, MyMath::Mat4x4& translation)
{
	translation.m23 -= rely;
}

inline void ResetTransformations(float& accumulatedYaw, float& accumulatedPitch, float& samplesSpacing, MyMath::Mat4x4& rotation, MyMath::Mat4x4& translation, MyMath::Mat4x4& viewMatrix)
{
	accumulatedYaw = 0.0f;
	accumulatedPitch = 0.0f;
	samplesSpacing = 1.0f;
	rotation = {};
	translation = {};
	viewMatrix = {};
}

inline void RenderFrequencyDomainSignal(const std::vector<std::complex<float>>& signal, const float samplesSpacing,
										const MyMath::Mat4x4& rotation, const MyMath::Mat4x4& translation,
										const MyMath::Mat4x4& view, const MyMath::Box& bounds, const MyMath::Mat4x4& proj,
										MyApp::SdlManager& sdl, const MyApp::ColorBytes color, const float& zOffset)
{
	float biggestComponent = -1.0f;
	for (const auto& complex : signal)
	{
		if (std::fabs(complex.real()) > biggestComponent) biggestComponent = std::fabs(complex.real());
		if (std::fabs(complex.imag()) > biggestComponent) biggestComponent = std::fabs(complex.imag());
	}
	const float amplitudeScale = 1.0f / biggestComponent;
	
	MyMath::Mat4x4 translationWithOffset = translation;
	translationWithOffset.m23 += zOffset;

	// Draw each frequency bin as a line.
	for (size_t n = 0; n < signal.size(); n++)
	{
		MyMath::Vec4 pt0, pt1; // World position.
		// Single vertices. No model transformation since it's assumed to be an identity matrix.
		const float zPos = float(n) / float(signal.size());
		pt0 = { 0.0f,							0.0f,							samplesSpacing * zPos, 1.0f };
		pt1 = { signal[n].real() * amplitudeScale, signal[n].imag() * amplitudeScale, samplesSpacing * zPos, 1.0f };

		// Rotate rotate around Z.
		pt0 = MatrixVectorMultiplication(rotation, pt0);
		pt1 = MatrixVectorMultiplication(rotation, pt1);
		// Offset on Z.
		pt0 = MatrixVectorMultiplication(translationWithOffset, pt0);
		pt1 = MatrixVectorMultiplication(translationWithOffset, pt1);

		// To view space.
		pt0 = MatrixVectorMultiplication(view, pt0);
		pt1 = MatrixVectorMultiplication(view, pt1);

		// Cull points outside the viewing volume.
		if (pt0.x < bounds.back || pt0.x > bounds.front ||
			pt0.y < bounds.right || pt0.y > bounds.left ||
			pt0.z < bounds.bottom || pt0.z > bounds.top) continue;
		if (pt1.x < bounds.back || pt1.x > bounds.front ||
			pt1.y < bounds.right || pt1.y > bounds.left ||
			pt1.z < bounds.bottom || pt1.z > bounds.top) continue;

		// To clip space.
		pt0 = MatrixVectorMultiplication(proj, pt0);
		pt1 = MatrixVectorMultiplication(proj, pt1);

		assert(pt0.x >= -1.0f && pt0.x <= 1.0f &&
			   pt0.y >= -1.0f && pt0.y <= 1.0f &&
			   pt0.z >= -1.0f && pt0.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");
		assert(pt1.x >= -1.0f && pt1.x <= 1.0f &&
			   pt1.y >= -1.0f && pt1.y <= 1.0f &&
			   pt1.z >= -1.0f && pt1.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");

		// Clip viewport space following "Viewport transform" section of: https://www.khronos.org/opengl/wiki/Viewport_Transform note: adjusted to fit SDL's coordinate convention.
		// Depth (x component) is discarded, we don't need it. No perspective divide since we're using orthogonal projection.
		const MyMath::Vec2 windowPt0 =
		{
			sdl.displaySize * 0.5f * -pt0.y + 0 + sdl.displaySize * 0.5f,
			sdl.displaySize - (sdl.displaySize * 0.5f * pt0.z + 0 + sdl.displaySize * 0.5f)
		};
		const MyMath::Vec2 windowPt1 =
		{
			sdl.displaySize * 0.5f * -pt1.y + 0 + sdl.displaySize * 0.5f,
			sdl.displaySize - (sdl.displaySize * 0.5f * pt1.z + 0 + sdl.displaySize * 0.5f)
		};
		assert(windowPt0.x >= 0.0f && windowPt0.x <= sdl.displaySize &&
			   windowPt0.y >= 0.0f && windowPt0.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");
		assert(windowPt1.x >= 0.0f && windowPt1.x <= sdl.displaySize &&
			   windowPt1.y >= 0.0f && windowPt1.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");

		// Draw vertices.
		sdl.RenderLine(windowPt0.x, windowPt0.y, windowPt1.x, windowPt1.y, color);
	}
}

inline void RenderTimeDomainSignal(const std::vector<float>& signal, const float samplesSpacing,
										const MyMath::Mat4x4& rotation, const MyMath::Mat4x4& translation,
										const MyMath::Mat4x4& view, const MyMath::Box& bounds, const MyMath::Mat4x4& proj,
										MyApp::SdlManager& sdl, const MyApp::ColorBytes color, const float zOffset)
{
	float biggestComponent = -1.0f;
	for (const auto& amplitude : signal)
	{
		if (std::fabs(amplitude) > biggestComponent) biggestComponent = std::fabs(amplitude);
	}
	const float amplitudeScale = 1.0f / biggestComponent;

	MyMath::Mat4x4 translationWithOffset = translation;
	translationWithOffset.m23 += zOffset;

	// Draw each frequency bin as a line.
	for (size_t n = 0; n < signal.size(); n++)
	{
		MyMath::Vec4 pt0, pt1; // World position.
		// Single vertices. No model transformation since it's assumed to be an identity matrix.
		const float zPos = float(n) / float(signal.size());
		pt0 = { 0.0f, 0.0f, samplesSpacing * zPos, 1.0f };
		pt1 = { amplitudeScale * signal[n], 0.0f, samplesSpacing * zPos, 1.0f };

		// Rotate rotate around Z.
		pt0 = MatrixVectorMultiplication(rotation, pt0);
		pt1 = MatrixVectorMultiplication(rotation, pt1);
		// Offset on Z.
		pt0 = MatrixVectorMultiplication(translationWithOffset, pt0);
		pt1 = MatrixVectorMultiplication(translationWithOffset, pt1);

		// To view space.
		pt0 = MatrixVectorMultiplication(view, pt0);
		pt1 = MatrixVectorMultiplication(view, pt1);

		// Cull points outside the viewing volume.
		if (pt0.x < bounds.back || pt0.x > bounds.front ||
			pt0.y < bounds.right || pt0.y > bounds.left ||
			pt0.z < bounds.bottom || pt0.z > bounds.top) continue;
		if (pt1.x < bounds.back || pt1.x > bounds.front ||
			pt1.y < bounds.right || pt1.y > bounds.left ||
			pt1.z < bounds.bottom || pt1.z > bounds.top) continue;

		// To clip space.
		pt0 = MatrixVectorMultiplication(proj, pt0);
		pt1 = MatrixVectorMultiplication(proj, pt1);

		assert(pt0.x >= -1.0f && pt0.x <= 1.0f &&
			   pt0.y >= -1.0f && pt0.y <= 1.0f &&
			   pt0.z >= -1.0f && pt0.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");
		assert(pt1.x >= -1.0f && pt1.x <= 1.0f &&
			   pt1.y >= -1.0f && pt1.y <= 1.0f &&
			   pt1.z >= -1.0f && pt1.z <= 1.0f && "Normalized device coordinate lies outside the normal range.");

		// Clip viewport space following "Viewport transform" section of: https://www.khronos.org/opengl/wiki/Viewport_Transform note: adjusted to fit SDL's coordinate convention.
		// Depth (x component) is discarded, we don't need it. No perspective divide since we're using orthogonal projection.
		const MyMath::Vec2 windowPt0 =
		{
			sdl.displaySize * 0.5f * -pt0.y + 0 + sdl.displaySize * 0.5f,
			sdl.displaySize - (sdl.displaySize * 0.5f * pt0.z + 0 + sdl.displaySize * 0.5f)
		};
		const MyMath::Vec2 windowPt1 =
		{
			sdl.displaySize * 0.5f * -pt1.y + 0 + sdl.displaySize * 0.5f,
			sdl.displaySize - (sdl.displaySize * 0.5f * pt1.z + 0 + sdl.displaySize * 0.5f)
		};
		assert(windowPt0.x >= 0.0f && windowPt0.x <= sdl.displaySize &&
			   windowPt0.y >= 0.0f && windowPt0.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");
		assert(windowPt1.x >= 0.0f && windowPt1.x <= sdl.displaySize &&
			   windowPt1.y >= 0.0f && windowPt1.y <= sdl.displaySize && "Window point lies outside the screen's bounds.");

		// Draw vertices.
		sdl.RenderLine(windowPt0.x, windowPt0.y, windowPt1.x, windowPt1.y, color);
	}
}

constexpr const MyMath::Box BOUNDS{ -2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f };
constexpr const MyMath::Mat4x4 ORTHO_PROJ_MAT = MyMath::OrthogonalProjectionMatrix(BOUNDS.back, BOUNDS.front, BOUNDS.right, BOUNDS.left, BOUNDS.bottom, BOUNDS.top);
// Static data for 3D rendering.
MyMath::Mat4x4 objectRotation = MyMath::MAT4_IDENTITY;
MyMath::Mat4x4 objectTranslation = MyMath::MAT4_IDENTITY;
MyMath::Mat4x4 viewMatrix = MyMath::MAT4_IDENTITY;
float accumulatedYaw = 0.0f; // Controlled with left mouse button. Allows you to rotate the signal around.
float accumulatedPitch = 0.0f; // Controlled with left mouse button. Allows you to pitch the signal towards and away from camera.
float samplesSpacing = 1.0f; // Controlled with right mouse button. Allows you to zoom into the signal.
float accumulatedZoffset = 0.0f; // Controlled with scroll wheel. Allows you to scroll through the signal.

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

constexpr const size_t SINE_SAMPLE_RATE = 8000;
constexpr const size_t SINE_FREQ = 441;
constexpr const float ANGLE_IN_RADS = -0.5f * MyMath::PI;
constexpr const float SYNTHESIZED_COMPLEX_MAGNITUDE = 1.0f * SINE_SAMPLE_RATE * 0.5f;
const std::array<float, 5> offsets = { 0.0f, 0.01f, 0.02f, 0.03f, 0.04f }; // Has to be static const, not a constexpr const because the values are passed to lambdas.
const std::array<MyApp::ColorBytes, 5> colors = { { {255, 0,0,255}, {0,255,0,255}, {0,0,255,255}, {255,255,0,255}, {255,0,255,255} } }; // Idem.

SoundToPlay toPlay = SoundToPlay::None;
std::vector<Waveform> toDisplay = {};
std::vector<float> generatedTimeDomain(SINE_SAMPLE_RATE, 0.0f);
std::vector<std::complex<float>> generatedFreqDomain(SINE_SAMPLE_RATE, 0.0f);
std::vector<float> generatedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f);
std::vector<float> synthesizedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f);
std::vector<std::complex<float>> synthesizedFreqDomain(SINE_SAMPLE_RATE, 0.0f);

void MyApp::Application::OnStart()
{
	// Compute signals for the generated sine.
	for (size_t n = 0; n < generatedTimeDomain.size(); n++)
	{
		generatedTimeDomain[n] = GenerateSine(n, SINE_SAMPLE_RATE, SINE_FREQ);
	}
	MyDFT::DFT(generatedFreqDomain, generatedTimeDomain, SINE_SAMPLE_RATE);
	MyDFT::IDFT(generatedTimeDomainFromDFT, generatedFreqDomain, SINE_SAMPLE_RATE);

	// Compute signals for the synthesized sine.
	synthesizedFreqDomain[SINE_FREQ] = std::complex<float>(std::cosf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE, std::sinf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE);
	synthesizedFreqDomain[SINE_SAMPLE_RATE - SINE_FREQ] = std::complex<float>(-std::cosf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE, -std::sinf(ANGLE_IN_RADS) * SYNTHESIZED_COMPLEX_MAGNITUDE);
	MyDFT::IDFT(synthesizedTimeDomainFromDFT, synthesizedFreqDomain, SINE_SAMPLE_RATE);

	if (!assetManager_.WriteWav(generatedTimeDomain, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "generated.wav").c_str(), 1, SINE_SAMPLE_RATE)) {
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	}
	if (!assetManager_.WriteWav(generatedTimeDomainFromDFT, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "generatedFromDFT.wav").c_str(), 1, SINE_SAMPLE_RATE)) {
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	};
	if (!assetManager_.WriteWav(synthesizedTimeDomainFromDFT, (std::string(APPLICATION_WAV_OUTPUTS_DIR) + "synthesizedFromDFT.wav").c_str(), 1, SINE_SAMPLE_RATE)) {
		throw std::runtime_error(std::string("Couldn't write wav to file."));
	};

	sdl_.RegisterImguiCallback([&]()
		{
			constexpr const char* soundNames[4] = { "NONE", "Generated sine", "Generated sine reconstructed from it's DFT", "Sine synthesized from constructed DFT" };
			static std::array<bool, 5> whetherToDisplay({ false, false, false, false, false });

			ImGui::Begin("Visualizer");

			ImGui::Text("Controls\nLeft mouse button: rotate, right mouse button: scale, mouse wheel: scroll through the signal, R: reset the view.\n");

			ImGui::ListBox("Sound to play", (int*)&toPlay, soundNames, 4);

			bool updateDisplayedWaveform = false;
			updateDisplayedWaveform = ImGui::Checkbox("Show Generated in time-domain: ", &(whetherToDisplay[0])) ? true : updateDisplayedWaveform;
			updateDisplayedWaveform = ImGui::Checkbox("Show Generated in frequency-domain: ", &(whetherToDisplay[1])) ? true : updateDisplayedWaveform;
			updateDisplayedWaveform = ImGui::Checkbox("Show reconstructed Generated in frequency-domain: ", &(whetherToDisplay[2])) ? true : updateDisplayedWaveform;
			updateDisplayedWaveform = ImGui::Checkbox("Show Synthesized in frequency-domain: ", &(whetherToDisplay[3])) ? true : updateDisplayedWaveform;
			updateDisplayedWaveform = ImGui::Checkbox("Show Synthesized in time-domain: ", &(whetherToDisplay[4])) ? true : updateDisplayedWaveform;

			ImGui::End();

			if (updateDisplayedWaveform)
			{
				toDisplay.resize(0);
				for (int i = 0; i < (int)whetherToDisplay.size(); i++)
				{
					if (whetherToDisplay[i]) toDisplay.push_back((Waveform)i);
				}
			}
		});

	// Register user input callbacks.
	sdl_.RegisterMouseInputCallback(MyApp::Input::LEFT_MOUSE_BUTTON, [&](const float x, const float y)
		{
			constexpr const float MOUSE_SENSITIVITY = 0.001f;
			ProcessLMB(x * MOUSE_SENSITIVITY, y * MOUSE_SENSITIVITY, accumulatedYaw, accumulatedPitch, objectRotation, viewMatrix);
		});
	sdl_.RegisterMouseInputCallback(MyApp::Input::RIGHT_MOUSE_BUTTON, [&](const float x, const float y)
		{
			constexpr const float WHEEL_SENSITIVITY = 1.0f;
			ProcessRMB(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY, samplesSpacing);
		});
	sdl_.RegisterMouseInputCallback(MyApp::Input::SCROLL_WHEEL, [&](const float x, const float y)
		{
			constexpr const float WHEEL_SENSITIVITY = 0.1f;
			ProcessScrollWheel(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY, objectTranslation);
		});
	sdl_.RegisterInputCallback(MyApp::Input::R, [&]()
		{
			ResetTransformations(accumulatedYaw, accumulatedPitch, samplesSpacing, objectRotation, objectTranslation, viewMatrix);
		});
}

void MyApp::Application::OnUpdate()
{
	static auto lastUpdateSounds = SoundToPlay::None;
	if (toPlay != lastUpdateSounds)
	{
		audioEngine_.DestroyAll();

		switch (toPlay)
		{
		case SoundToPlay::None:
			break;
		case SoundToPlay::Generated: {
			MyApp::Sound* generatedSound = audioEngine_.CreateSound(generatedTimeDomain);
			generatedSound->Play();
		}
			break;
		case SoundToPlay::GeneratedFromDFT: {
			MyApp::Sound* generatedSoundFromDFT = audioEngine_.CreateSound(generatedTimeDomainFromDFT);
			generatedSoundFromDFT->Play();
		}
			break;
		case SoundToPlay::Synthesized: {
			MyApp::Sound* synthesizedSoundFromDFT = audioEngine_.CreateSound(synthesizedTimeDomainFromDFT);
			synthesizedSoundFromDFT->Play();
		}
			break;
		default:
			break;
		}
	}
	lastUpdateSounds = toPlay;

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
						RenderTimeDomainSignal(generatedTimeDomain, samplesSpacing, objectRotation, objectTranslation, viewMatrix, BOUNDS, ORTHO_PROJ_MAT, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::GeneratedFreqDomain:
					{
						RenderFrequencyDomainSignal(generatedFreqDomain, samplesSpacing, objectRotation, objectTranslation, viewMatrix, BOUNDS, ORTHO_PROJ_MAT, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::GeneratedFromDFT:
					{
						RenderTimeDomainSignal(generatedTimeDomainFromDFT, samplesSpacing, objectRotation, objectTranslation, viewMatrix, BOUNDS, ORTHO_PROJ_MAT, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::SynthesizedFreqDomain:
					{
						RenderFrequencyDomainSignal(synthesizedFreqDomain, samplesSpacing, objectRotation, objectTranslation, viewMatrix, BOUNDS, ORTHO_PROJ_MAT, sdl_, colors[i], offsets[i]);
					}break;

					case Waveform::SynthesizedFromDFT:
					{
						RenderTimeDomainSignal(synthesizedTimeDomainFromDFT, samplesSpacing, objectRotation, objectTranslation, viewMatrix, BOUNDS, ORTHO_PROJ_MAT, sdl_, colors[i], offsets[i]);
					}break;

					default:
						break;
					}
				}
			});
	}
	lastUpdateWaveforms = toDisplay;
}

void MyApp::Application::OnShutdown()
{

}

int main()
{
	MyApp::Application app = MyApp::Application(720, 8000, 2 * 512);

	app.Run();

	return 0;
}