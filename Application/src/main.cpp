#include <Application.h>

void MyApp::Application::Init()
{
	auto sound = audioEngine_.CreateSound("../resources/olegSpeech_44100Hz_32f_4.274sec.wav", assetManager_);
	sound.Play();
}

int main()
{
	MyApp::Application app = MyApp::Application(720, 44100, 1024);

	app.Run();

	return 0;
}