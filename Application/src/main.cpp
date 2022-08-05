#include <Application.h>

#include <imgui.h>

void MyApp::Application::OnStart()
{
	sdl_.RegisterImguiCallback([]()
	{
		ImGui::Begin("Hello, world!");

		ImGui::Text("This is some useful text.");

		ImGui::End();
	});
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