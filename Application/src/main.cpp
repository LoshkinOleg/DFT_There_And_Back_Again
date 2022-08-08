#include <Application.h>

#include "SpinningCubeDemo.h"

int main()
{
	MyApp::Application app = MyApp::Application(720, 8000, 2 * 512);

	app.Run();

	return 0;
}