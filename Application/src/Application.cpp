#include "Application.h"

#include <easy/profiler.h>

MyApp::Application::Application(const unsigned int displaySize, const unsigned int sampleRate, const unsigned int bufferSize): sdl_(SdlManager(displaySize)), audioEngine_(AudioEngine(sampleRate, bufferSize)) {}

void MyApp::Application::Run()
{
	EASY_PROFILER_ENABLE;

	OnStart();

	bool shutdown = false;
	while (!shutdown)
	{
		EASY_BLOCK("Application's update");
		shutdown = sdl_.Update();
		audioEngine_.ProcessAudio();
		OnUpdate();
	}

	OnShutdown();

#if BUILD_WITH_EASY_PROFILER
	const auto success = profiler::dumpBlocksToFile((std::string(APPLICATION_PROFILER_OUTPUTS_DIR) + "session.prof").c_str());
	if (!success) throw std::runtime_error(std::string("Failed to write easy profiler session to disk."));
#endif
}