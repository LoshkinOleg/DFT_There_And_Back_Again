#include "DFTOutputVisualizer.h"

int main()
{
	DFTVisualizer::Application* app = new DFTVisualizer::Application(DFTVisualizer::Action::RectangularWindowLinear, "../resources/audioSamples/olegSpeech_8000Hz_32f.wav", 4.272f, 8000, 512, 720);
	app->Run();
	delete app;

	return 0;
}