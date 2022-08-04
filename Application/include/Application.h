#pragma once

#include <easy/profiler.h>

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
			EASY_PROFILER_ENABLE;

			Init();

			bool shutdown = false;
			while (!shutdown)
			{
				EASY_BLOCK("Application's update");
				shutdown = sdl_.ProcessInputs();
				audioEngine_.ProcessAudio();
			}

#if BUILD_WITH_EASY_PROFILER
			const auto success = profiler::dumpBlocksToFile((std::string(APPLICATION_PROFILER_OUTPUTS_DIR) + "session.prof").c_str());
			if (!success) throw std::runtime_error(std::string("Failed to write easy profiler session to disk."));
#endif
		}

	private:
		SdlManager sdl_;
		AudioEngine audioEngine_;
		AssetManager assetManager_{};
	};
}