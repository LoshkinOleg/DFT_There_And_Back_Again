#pragma once

#include <Windows.h>

#include "AssetManager.h"
#include "SdlManager.h"
#include "AudioEngine.h"

namespace MyApp
{
	class Application
	{
	public:

		Application() = delete;
		Application(const unsigned int displaySize, const unsigned int sampleRate, const unsigned int bufferSize): sdl_(SdlManager(displaySize)), audioEngine_(AudioEngine(sampleRate, bufferSize)) {}

		void Init();

		void Run()
		{
			bool shutdown = false;
			while (!shutdown)
			{
				shutdown = sdl_.ProcessInputs();
				audioEngine_.ServiceAudio();
				Sleep(33);
			}
		}

	private:
		SdlManager sdl_;
		AudioEngine audioEngine_;
		AssetManager assetManager_{};
	};
}