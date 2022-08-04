#include <Application.h>

void MyApp::Application::Init()
{
	auto& sound = audioEngine_.CreateSound("../resources/pluck_44100Hz_32f_0_44sec.wav", assetManager_);
	sound.looping = false;
	sdl_.RegisterCallback(Input::SPACE, [&sound]() { sound.Play(); });
}

int main()
{
	MyApp::Application app = MyApp::Application(720, 44100, 2 * 512);

	app.Run();

	return 0;
}