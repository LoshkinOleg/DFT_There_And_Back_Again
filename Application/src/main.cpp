#include "DFTOutputVisualizer.h"

int main()
{
	DFTVisualizer::Application* app = new DFTVisualizer::Application(DFTVisualizer::AudioEngine::Action::PlayWav, "../resources/audioSamples/sine441_44100Hz_32f_1sec.wav", 1.0f, 44100, 2048);
	app->Run();
	delete app;

	return 0;
}