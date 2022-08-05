#pragma once

#include "AssetManager.h"
#include "SdlManager.h"
#include "AudioEngine.h"

namespace MyApp
{
	class Application
	{
	public:

		Application() = delete;
		Application(const unsigned int displaySize, const unsigned int sampleRate, const unsigned int bufferSize);

		// User implemented methods.
		void OnStart();
		void OnUpdate();
		void OnShutdown();

		void Run();

	private:
		SdlManager sdl_;
		AudioEngine audioEngine_;
		AssetManager assetManager_{};
	};
}