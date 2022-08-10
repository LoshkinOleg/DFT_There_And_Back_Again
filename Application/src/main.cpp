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
	samplesSpacing += rely;
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

inline void RenderFrequencyDomainSignal(const std::vector<std::complex<float>>& freqDomSignal, const float samplesSpacing,
										const MyMath::Mat4x4& rotation, const MyMath::Mat4x4& translation,
										const MyMath::Mat4x4& view, const MyMath::Box& bounds, const MyMath::Mat4x4& proj,
										MyApp::SdlManager& sdl)
{
	// Draw each frequency bin as a line.
	for (size_t n = 0; n < freqDomSignal.size(); n++)
	{
		MyMath::Vec4 pt0, pt1; // World position.
		// Single vertices. No model transformation since it's assumed to be an identity matrix.
		const float zPos = float(n) / float(freqDomSignal.size());
		pt0 = { 0.0f,							0.0f,							samplesSpacing * zPos, 1.0f };
		pt1 = { freqDomSignal[n].real(), freqDomSignal[n].imag(), samplesSpacing * zPos, 1.0f };

		// Rotate rotate around Z.
		pt0 = MatrixVectorMultiplication(rotation, pt0);
		pt1 = MatrixVectorMultiplication(rotation, pt1);
		// Offset on Z.
		pt0 = MatrixVectorMultiplication(translation, pt0);
		pt1 = MatrixVectorMultiplication(translation, pt1);

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
		sdl.RenderLine(windowPt0.x, windowPt0.y, windowPt1.x, windowPt1.y);
	}
}

void VisualizeSineInFreqencyDomain(MyApp::AudioEngine& audioEngine, MyApp::SdlManager& sdl)
{
	// Generate a sine to be visualized.
	constexpr const size_t SINE_SAMPLE_RATE = 8000;
	constexpr const size_t SINE_FREQ = 441;
	std::vector<float> sine(SINE_SAMPLE_RATE, 0.0f);
	for (size_t n = 0; n < sine.size(); n++)
	{
		sine[n] = GenerateSine(n, SINE_SAMPLE_RATE, SINE_FREQ);
	}

	// Create a sound to hear the sine signal.
	auto& sound = audioEngine.CreateSound(sine);
	sound.Play();

	// Compute the full discrete fourier transform of the sine.
	static std::vector<std::complex<float>> sineFourierTransform(SINE_SAMPLE_RATE, 0.0f);
	MyDFT::DFT(sineFourierTransform, sine, SINE_SAMPLE_RATE);

	// Static data for 3D rendering.
	static MyMath::Mat4x4 objectRotation = MyMath::MAT4_IDENTITY;
	static MyMath::Mat4x4 objectTranslation = MyMath::MAT4_IDENTITY;
	static MyMath::Mat4x4 viewMatrix = MyMath::MAT4_IDENTITY;
	static float accumulatedYaw = 0.0f; // Controlled with left mouse button. Allows you to rotate the signal around.
	static float accumulatedPitch = 0.0f; // Controlled with left mouse button. Allows you to pitch the signal towards and away from camera.
	static float samplesSpacing = 1.0f; // Controlled with right mouse button. Allows you to zoom into the signal.
	static float accumulatedZoffset = 0.0f; // Controlled with scroll wheel. Allows you to scroll through the signal.

	// Register user input callbacks.
	sdl.RegisterMouseInputCallback(MyApp::Input::LEFT_MOUSE_BUTTON, [&](const float x, const float y)
	{
		constexpr const float MOUSE_SENSITIVITY = 0.001f;
		ProcessLMB(x * MOUSE_SENSITIVITY, y * MOUSE_SENSITIVITY, accumulatedYaw, accumulatedPitch, objectRotation, viewMatrix);
	});
	sdl.RegisterMouseInputCallback(MyApp::Input::RIGHT_MOUSE_BUTTON, [&](const float x, const float y)
	{
		constexpr const float WHEEL_SENSITIVITY = 1.0f;
		ProcessRMB(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY, samplesSpacing);
	});
	sdl.RegisterMouseInputCallback(MyApp::Input::SCROLL_WHEEL, [&](const float x, const float y)
	{
		constexpr const float WHEEL_SENSITIVITY = 0.1f;
		ProcessScrollWheel(x * WHEEL_SENSITIVITY, y * WHEEL_SENSITIVITY, objectTranslation);
	});
	sdl.RegisterInputCallback(MyApp::Input::R, [&]()
	{
		ResetTransformations(accumulatedYaw, accumulatedPitch, samplesSpacing, objectRotation, objectTranslation, viewMatrix);
	});

	// Register rending callback.
	sdl.RegisterRenderCallback([&]()
	{
		constexpr const MyMath::Box BOUNDS{ -2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f };
		constexpr const MyMath::Mat4x4 ORTHO_PROJ_MAT = MyMath::OrthogonalProjectionMatrix(BOUNDS.back, BOUNDS.front, BOUNDS.right, BOUNDS.left, BOUNDS.bottom, BOUNDS.top);
		RenderFrequencyDomainSignal(sineFourierTransform, samplesSpacing, objectRotation, objectTranslation, viewMatrix, BOUNDS, ORTHO_PROJ_MAT, sdl);
	});
}

void MyApp::Application::OnStart()
{
	VisualizeSineInFreqencyDomain(audioEngine_, sdl_);
}

void MyApp::Application::OnUpdate()
{

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