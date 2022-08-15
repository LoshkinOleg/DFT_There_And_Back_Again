#pragma once

#include <array>

#include "AssetManager.h"
#include "SdlManager.h"
#include "AudioEngine.h"

#include "MyMath.h"

namespace MyApp
{
	class Application
	{
	public:

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

		Application() = delete;
		Application(const unsigned int displaySize, const unsigned int sampleRate, const unsigned int bufferSize);

		// User implemented methods.
		void OnStart();
		void OnUpdate();
		void OnShutdown();

		void Run(const std::vector<float>& generatedTimeDomain, const std::vector<float>& generatedTimeDomainFromDFT, const std::vector<float>& synthesizedTimeDomainFromDFT,
			const std::vector<std::complex<float>>& generatedFreqDomain, const std::vector<std::complex<float>>& synthesizedFreqDomain);

		SoundToPlay toPlay = SoundToPlay::None;
		std::vector<Waveform> toDisplay = {};
	private:
		/**
		* Rotates the waveform.
		*
		* @param relx Screenspace x movement of the mouse.
		* @param rely Screenspace y movement of the mouse.
		*/
		void Callback_ProcessLMB_(const float relx, const float rely);

		/**
		* Scales the waveform by spacing out the samples on the Z axis.
		*
		* @param relx Screenspace x movement of the mouse.
		* @param rely Screenspace y movement of the mouse. Unused.
		*/
		void Callback_ProcessRMB_(const float relx, const float rely);

		/**
		* Scrolls through the signal by applying a Z offset on the waveform's model.
		*
		* @param relx Horizontal scrolling, if any. Most mice don't have a horizontal scroll wheel. Unused.
		* @param relx Vertical scrolling.
		*/
		void Callback_ProcessScrollWheel_(const float relx, const float rely);

		/**
		* Resets all model and camera transformations. Useful when you've lost view of the signal.
		*/
		void Callback_ResetTransformations_();

		/**
		* Draws the signal as a series of lines in 3D space where X = 1, Y = i and Z = frequency.
		*
		* @param signal The complex-valued signal to be drawn.
		* @param sdl The SdlManager to be used to draw the lines.
		* @param color The color of the lines. Useful for distinguishing between multiple signals on screen.
		* @param offset The global offset on the Z axis of the lines. Useful for distinguishing between multiple signals on screen.
		*/
		void Callback_RenderFrequencyDomainSignal_(const std::vector<std::complex<float>>& signal, const MyApp::ColorBytes color, const float& offset);

		/**
		* Draws the signal as a series of lines in 3D space where X = 1, Y = i and Z = time.
		*
		* @param signal The real-valued signal to be drawn.
		* @param sdl The SdlManager to be used to draw the lines.
		* @param color The color of the lines. Useful for distinguishing between multiple signals on screen.
		* @param offset The global offset on the Z axis of the lines. Useful for distinguishing between multiple signals on screen.
		*/
		void Callback_RenderTimeDomainSignal_(const std::vector<float>& signal, const MyApp::ColorBytes color, const float offset);

		/**
		* Displays an ImGui window to interact with the application. Shows the controls, allows to switch between signals played back and allows to show/hide the visualizations of different signals.
		*/
		void Callback_RenderImgui_();

		/**
		* Updates which waveforms to display and which sound to play.
		*/
		void UpdateToDisplayAndToPlay_(const std::vector<float>& generatedTimeDomain, const std::vector<float>& generatedTimeDomainFromDFT, const std::vector<float>& synthesizedTimeDomainFromDFT,
			const std::vector<std::complex<float>>& generatedFreqDomain, const std::vector<std::complex<float>>& synthesizedFreqDomain);

	private:
		SdlManager sdl_;
		AudioEngine audioEngine_;
		AssetManager assetManager_{};

		static constexpr const MyMath::Box BOUNDS{ -2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f }; // Bounds of the space in which objects are rendered.
		static constexpr const MyMath::Mat4x4 ORTHO_PROJ_MAT = MyMath::OrthogonalProjectionMatrix(BOUNDS.back, BOUNDS.front, BOUNDS.right, BOUNDS.left, BOUNDS.bottom, BOUNDS.top); // Projection matrix used.
		const std::array<float, 5> offsets_ = { 0.0f, 0.01f, 0.02f, 0.03f, 0.04f }; // Small offsets to be able to distinguish between overlapping signals. Has to be static const, not a constexpr const because the values are passed to lambdas.
		const std::array<MyApp::ColorBytes, 5> colors_ = { { {255, 0,0,255}, {0,255,0,255}, {0,0,255,255}, {255,255,0,255}, {255,0,255,255} } }; // Color differences to differenciate between signals. Idem.

		// Graphical rendering structures.
		MyMath::Mat4x4 modelRotation_ = MyMath::MAT4_IDENTITY; // Holds rotation of the model. Used to rotate waveforms around local Z.
		MyMath::Mat4x4 modelTranslation_ = MyMath::MAT4_IDENTITY; // Holds position of the model. Used to offset the waveforms on local Z.
		MyMath::Mat4x4 viewMatrix_ = MyMath::MAT4_IDENTITY; // Holds camera rotation. Used to rotate waveforms around world's Y.
		float accumulatedYaw_ = 0.0f; // Controlled with left mouse button. Allows you to rotate the signal around.
		float accumulatedPitch_ = 0.0f; // Controlled with left mouse button. Allows you to pitch the signal towards and away from camera.
		float samplesSpacing_ = 1.0f; // Controlled with right mouse button. Allows you to zoom into the signal.
		float accumulatedZoffset_ = 0.0f; // Controlled with scroll wheel. Allows you to scroll through the signal.
	};
}