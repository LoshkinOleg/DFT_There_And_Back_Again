#pragma once

#include <vector>
#include <complex>

#include <portaudio.h>
#include <SDL.h>
#undef main

#include "Common.h"

namespace DFTVisualizer
{
	enum class Action
	{
		ArgandPlane = 0,
		RectangularWindowLinear
	};

	class AudioEngine
	{
	public:
		AudioEngine() = delete;
		AudioEngine(const Action action, const char* path, const float audioDataDurationInSec, const size_t sampleRate, const size_t bufferSize);
		~AudioEngine();

		const std::vector<std::complex<float>>& GetCurrentBufferDFT() const;

		const size_t sampleRate, bufferSize;
	private:
		static int ServiceAudio_(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

		Clip clip_{};
		PaStream* pStream_ = nullptr;
		std::vector<std::complex<float>> currentBufferDFT_;
	};

	class SdlManager
	{
	public:
		SdlManager() = delete;
		SdlManager(const size_t displaySideLen, const Action action);
		~SdlManager();

		bool ProcessInputs(AudioEngine& engine);
		void Draw(const std::vector<std::complex<float>>& dftData);

		const size_t displaySideLen;
		const Action action;
	private:
		void ArgandPlaneVisualization_(const std::vector<std::complex<float>>& dftData);
		void RectangularWindowVisualization_(const std::vector<std::complex<float>>& dftData);

		SDL_Event event_{};
		SDL_Window* pWindow_ = nullptr;
		SDL_Renderer* pRenderer_ = nullptr;
	};

	class Application
	{
	public:
		Application() = delete;
		Application(const Action action, const char* pathToRelevantFile, const float clipDuration, const size_t sampleRate, const size_t bufferSize, const size_t displaySideLen);

		void Run();

		const Action action;
	private:
		AudioEngine renderer_;
		SdlManager sdl_;

		std::vector<std::complex<float>> dftData_;
	};
}
