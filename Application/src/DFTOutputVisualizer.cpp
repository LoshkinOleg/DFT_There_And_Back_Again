#include "DFTOutputVisualizer.h"

#include <cassert>

#include "FDN.h"

DFTVisualizer::AudioEngine::AudioEngine(const Action action, const char* path, const float audioDataDurationInSec, const size_t sampleRate, const size_t bufferSize): sampleRate(sampleRate), bufferSize(bufferSize)
{
	// Initialize PortAudio.
	auto err = Pa_Initialize();
	assert(err == paNoError && "Failed to initialize PortAudio!");

	const auto device = Pa_GetDefaultOutputDevice();
	PaStreamParameters outputParameters =
	{
		device,
		2,
		paFloat32,
		Pa_GetDeviceInfo(device)->defaultLowOutputLatency,
		NULL
	};
	assert(outputParameters.device != paNoDevice && "Failed to retrieve a default playback device!");

	err = Pa_OpenStream(
		&pStream_,
		NULL, /* no input */
		&outputParameters,
		(double)sampleRate,
		(unsigned long)bufferSize,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		AudioEngine::ServiceAudio_,
		this);
	assert(err == paNoError && "Failed to open stream to device.");

	err = Pa_StartStream(pStream_);
	assert(err == paNoError && "Failed to start stream.");

	// Write or playback DFT result.
	switch (action)
	{
		case Action::ArgandPlane:
		{
			clip_ = LoadWavFile(path, 1, (uint32_t)sampleRate);
		}break;

		case Action::RectangularWindowLinear:
		{
			clip_ = LoadWavFile(path, 1, (uint32_t)sampleRate);
		}break;

		default:
		break;
	}
}

DFTVisualizer::AudioEngine::~AudioEngine()
{
	auto err = Pa_StopStream(pStream_);
	assert(err == paNoError && "Failed to stop stream.");
	err = Pa_CloseStream(pStream_);
	assert(err == paNoError && "Failed to close stream.");
	Pa_Terminate();
}

const std::vector<std::complex<float>>& DFTVisualizer::AudioEngine::GetCurrentBufferDFT() const
{
	return currentBufferDFT_;
}

int DFTVisualizer::AudioEngine::ServiceAudio_(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	AudioEngine* renderer = (AudioEngine*)userData;
	float* out = (float*)outputBuffer;
	static std::vector<float> outputBuff(renderer->bufferSize, 0.0f);
	auto& begin = renderer->clip_.currentBegin;
	auto& end = renderer->clip_.currentEnd;
	const auto& audioData = renderer->clip_.audioData;
	const auto pcmSize = renderer->clip_.audioData.size();

	if (!renderer->clip_.audioData.size())
	{
		for (size_t i = 0; i < renderer->bufferSize; i++)
		{
			out[2 * i] = 0.0f;
			out[2 * i + 1] = 0.0f;
		}
		return paContinue;
	}

	// Read pcm subset.
	if (begin > end) // Wrapping around pcm renderer.
	{
		for (size_t i = begin; i < pcmSize; i++) // Finish reading end of the pcm.
		{
			outputBuff[i - begin] = audioData[i];
		}
		for (size_t i = 0; i < end + 1; i++) // Read the start of the pcm into the remaining not yet updated part of the soundDataSubset_.
		{
			outputBuff[pcmSize - begin + i] = audioData[i];
		}
	}
	else // Not wrapping around pcm renderer, just copy soundData into subset continuously.
	{
		for (size_t i = begin; i < end + 1; i++)
		{
			outputBuff[i - begin] = audioData[i];
		}
	}

	renderer->currentBufferDFT_ = MyFDN::SimpleFFT_FFT(outputBuff);

	for (size_t i = 0; i < renderer->bufferSize; i++)
	{
		out[2 * i] = outputBuff[i];
		out[2 * i + 1] = outputBuff[i];
	}

	// Update current indices.
	assert(end + 1 <= pcmSize && "end somehow incremented past wavSize! Last iteration must have had end = wavSize, which shouldn't be possible. Check your code.");
	if (end + 1 == pcmSize) // If pcmSize % bufferSize = 0, this can happen, wrap back to 0.
	{
		begin = 0;
	}
	else // Just advance to next subset with no issues.
	{
		begin = end + 1;
	}

	// Update currentEnd_
	if (begin + renderer->bufferSize - 1 >= pcmSize) // If overruning pcm renderer, wrap around.
	{
		end = renderer->bufferSize - 1 - (pcmSize - begin);
	}
	else // Not overruning pcm renderer, just update currentEnd_ with no issues.
	{
		end = begin + renderer->bufferSize - 1;
	}

	return paContinue;
}

DFTVisualizer::SdlManager::SdlManager(const size_t displaySideLen, const DFTVisualizer::Action action): displaySideLen(displaySideLen), action(action)
{
	auto err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
	assert(err == 0 && "Failed to initialize sdl.");

	err = SDL_CreateWindowAndRenderer((int)displaySideLen, (int)displaySideLen, SDL_WINDOW_SHOWN, &pWindow_, &pRenderer_);
	assert(err == 0 && "Failed to create sdl window and renderer!");
}

DFTVisualizer::SdlManager::~SdlManager()
{
	if (pRenderer_)SDL_DestroyRenderer(pRenderer_);
	if (pWindow_)SDL_DestroyWindow(pWindow_);
	SDL_Quit();
}

void DFTVisualizer::SdlManager::ArgandPlaneVisualization_(const std::vector<std::complex<float>>& dftData)
{
	SDL_SetRenderDrawColor(pRenderer_, 255, 255, 255, 255);

	const int displayCenter = (int)(displaySideLen / 2);
	for (size_t i = 0; i < dftData.size(); i++)
	{
		constexpr const float SCALING_FACTOR = 5.0f;
		const int x = (int)(dftData[i].real() * SCALING_FACTOR);
		const int y = (int)(dftData[i].imag() * SCALING_FACTOR);

		SDL_RenderDrawLine(pRenderer_, displayCenter, displayCenter, displayCenter + x, displayCenter + y);
	}
}

void DFTVisualizer::SdlManager::RectangularWindowVisualization_(const std::vector<std::complex<float>>& dftData)
{
	// TODO: figure out why the signal doesn't seem to have any high frequencies according to the visualization (right 1/3 part of the screen has no columns, check it).

	if (!dftData.size()) return;

	const auto magnitude = [](const std::complex<float>& c)->float
	{
		return std::sqrtf(c.real() * c.real() + c.imag() * c.imag());
	};

	SDL_SetRenderDrawColor(pRenderer_, 255, 255, 255, 255);

	float currentX = 0.0f;
	const float step = (float)(displaySideLen / dftData.size());
	for (size_t i = 0; i < dftData.size(); i++)
	{
		constexpr const float SCALING_FACTOR = 15.0f;
		SDL_RenderDrawLineF(pRenderer_, currentX, (float)displaySideLen, currentX, displaySideLen - magnitude(dftData[i]) * SCALING_FACTOR);
		currentX += step;
	}
}

void DFTVisualizer::SdlManager::Draw(const std::vector<std::complex<float>>& dftData)
{
	SDL_SetRenderDrawColor(pRenderer_, 0, 0, 0, 255);
	SDL_RenderClear(pRenderer_);

	switch (action)
	{
		case Action::ArgandPlane:
		{
			ArgandPlaneVisualization_(dftData);
		}break;

		case Action::RectangularWindowLinear:
		{
			RectangularWindowVisualization_(dftData);
		}break;

		default:
		break;
	}

	SDL_RenderPresent(pRenderer_);
}

bool DFTVisualizer::SdlManager::ProcessInputs(AudioEngine& engine)
{
	while (SDL_PollEvent(&event_))
	{
		switch (event_.type)
		{
			case SDL_KEYDOWN:
			{
				if (event_.key.keysym.scancode == SDL_SCANCODE_ESCAPE) // Key reserved for exiting the program.
				{
					return true;
				}
				else if (event_.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					PrintDFT(engine.GetCurrentBufferDFT());
				}
			}
			break;

			case SDL_QUIT:
			{
				return true;
			}
			break;

			default:
			break;
		}
	}

	return false;
}

DFTVisualizer::Application::Application(const DFTVisualizer::Action action, const char* pathToRelevantFile, const float clipDuration, const size_t sampleRate, const size_t bufferSize, const size_t displaySideLen):
	renderer_(AudioEngine(action, pathToRelevantFile, clipDuration, sampleRate, bufferSize)),
	sdl_(SdlManager(displaySideLen, action)),
	action(action){}

void DFTVisualizer::Application::Run()
{
	constexpr const long MILLISECONDS_BETWEEN_FRAMES = 33; // ~30 fps

	bool shutdown = false;
	while (!shutdown)
	{
		switch (action)
		{
			case Action::ArgandPlane:
			{
				dftData_ = renderer_.GetCurrentBufferDFT();
			}break;

			case Action::RectangularWindowLinear:
			{
				dftData_ = renderer_.GetCurrentBufferDFT();
			}break;

			default:
			break;
		}

		shutdown = sdl_.ProcessInputs(renderer_);
		sdl_.Draw(dftData_);
		Pa_Sleep(MILLISECONDS_BETWEEN_FRAMES);
	}
}