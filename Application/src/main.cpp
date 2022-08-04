#include <Application.h>

#include "MyDFT.h"

void MyApp::Application::Init()
{
	auto& sound = audioEngine_.CreateSound("../resources/olegSpeech_8000Hz_32f_4_274sec.wav", assetManager_);

	const size_t lowCutoffFrequency = 0;
	const size_t highCutoffFrequency = 2000;
	auto fourierTransform = MyDFT::DFT(sound.data, 2 * audioEngine_.sampleRate);
	for (size_t i = lowCutoffFrequency; i <= highCutoffFrequency; i++)
	{
		// fourierTransform[i] = std::complex<float>(0.0f, 0.0f);
	}
	MyDFT::IDFT(sound.data, fourierTransform, sound.data.size());

	sound.Play();

	// sound.looping = false;
	// sdl_.RegisterCallback(Input::SPACE, [&sound]() { sound.Play(); });
}

int main()
{
	MyApp::Application app = MyApp::Application(720, 8000, 2 * 512);

	app.Run();

	return 0;
}