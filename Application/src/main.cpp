#include <Application.h>

#include <cassert>

void MyApp::Application::OnStart()
{
	auto& sound = audioEngine_.CreateSound("../resources/sine441_8000Hz_32f_1sec.wav", assetManager_);

	sound.Play();
}

void MyApp::Application::OnUpdate()
{

}

void MyApp::Application::OnShutdown()
{

}

int main()
{
	MyApp::Application app = MyApp::Application(720, 8000, 2 * 512);

	app.Run();

	return 0;
}