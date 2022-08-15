#include <Application.h>

// Two implementations of the signal processing functions. FunctionalVisualization.h implementation for a working DFT and IDFT. Replace it with ExerciseVisualization.h to implement your own functions as an exercise.
#include "FunctionalVisualization.h"
// #include "ExerciseVisualization.h"

/*
* Application's method ran once upon Application's startup, before the update loop.
*/
void MyApp::Application::OnStart()
{
	GenerateSignals();
}

/*
* Application's method ran every time at the end of Application's update.
*/
void MyApp::Application::OnUpdate()
{

}

/*
* Application's method ran once upon Application's shutdown, before the shutdown of internal systems.
*/
void MyApp::Application::OnShutdown()
{

}

int main()
{
	MyApp::Application app = MyApp::Application(720, 8000, 2 * 512);

	app.Run(generatedTimeDomain, generatedTimeDomainFromDFT, synthesizedTimeDomainFromDFT, generatedFreqDomain, synthesizedFreqDomain);

	return 0;
}