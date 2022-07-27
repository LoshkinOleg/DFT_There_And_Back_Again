#include <vector>
#include <math.h>
#include <cassert>
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>

#include "portaudio.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <SDL.h>
#undef main
#define BUILD_WITH_EASY_PROFILER
#include "easy/profiler.h"

#include "Common.h"
#include "FDN.h"

namespace DFTVisualizer
{
	enum class Action
	{
		ComputeAndPrintDFT = 0,
		PlaySignalFromDFToutput,
		PlayWav,
		VisualizeDFToutput,
		VisualizeCurrentBufferDFTArgand,
		VisualizeCurrentBufferDFTrectangularWindow
	};

	class AudioEngine
	{
	public:
		AudioEngine() = delete;
		AudioEngine(const Action action, const char* path, const float audioDataDurationInSec, const size_t sampleRate, const size_t bufferSize): sampleRate(sampleRate), bufferSize(bufferSize)
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
				(double)bufferSize,
				paClipOff,      /* we won't output out of range samples so don't bother clipping them */
				AudioEngine::ServiceAudio_,
				this);
			assert(err == paNoError && "Failed to open stream to device.");

			err = Pa_StartStream(pStream_);
			assert(err == paNoError && "Failed to start stream.");

			// Write or playback DFT result.
			switch (action)
			{
				case Action::ComputeAndPrintDFT:
				{
					const auto data = LoadWavFile(path, 1, sampleRate);
					const auto dftResult = MyFDN::DFT(data.audioData, sampleRate);
					PrintDFT(dftResult);
				}break;

				case Action::PlaySignalFromDFToutput:
				{
					const auto dftData = ParseDFTOutput(path);
					for (size_t i = 0; i < dftData.size(); i++)
					{
						if (std::abs(dftData[i].imag()) > 1.0f || std::abs(dftData[i].real()) > 1.0f)
						{
							std::cout << std::to_string(i) << std::endl;
						}
					}
					clip_.audioData = MyFDN::IDFT(dftData, audioDataDurationInSec);
				}break;

				case Action::PlayWav:
				{
					clip_ = LoadWavFile(path, 1, sampleRate);
				}break;

				case Action::VisualizeCurrentBufferDFTArgand:
				{
					clip_ = LoadWavFile(path, 1, sampleRate);
				}break;

				case Action::VisualizeCurrentBufferDFTrectangularWindow:
				{
					clip_ = LoadWavFile(path, 1, sampleRate);
				}break;

				default:
				break;
			}
		}

		~AudioEngine()
		{
			auto err = Pa_StopStream(pStream_);
			assert(err == paNoError && "Failed to stop stream.");
			err = Pa_CloseStream(pStream_);
			assert(err == paNoError && "Failed to close stream.");
			Pa_Terminate();
		}

		static std::vector<std::complex<float>> ParseDFTOutput(const char* path)
		{
			const auto complexFromString = [](const std::string& str)->std::complex<float>
			{
				const std::string realStr = std::string(str.begin() + str.find_first_of("(") + 1, str.begin() + str.find_first_of(";"));
				const std::string imagStr = std::string(str.begin() + str.find_first_of(";") + 1, str.begin() + str.find_first_of(")") - 1);

				return std::complex<float>(std::stof(realStr), std::stof(imagStr));
			};

			std::vector<std::string> lines;
			std::string currentLine;

			std::ifstream ifs(path, std::ifstream::in);
			assert(ifs.is_open() && !ifs.bad() && "Failed to open file.");

			while (std::getline(ifs, currentLine))
			{
				lines.push_back(currentLine);
			}

			ifs.close();

			std::vector<std::complex<float>> returnVal(lines.size() / 2);
			for (size_t i = 0; i < returnVal.size(); i++)
			{
				returnVal[i] = complexFromString(lines[i]);
			}

			return returnVal;
		}

		const std::vector<std::complex<float>>& GetCurrentBufferDFT()
		{
			return currentBufferDFT_;
		}

		static void PrintDFT(const std::vector<std::complex<float>>& dftData)
		{
			for (size_t i = 0; i < dftData.size(); i++)
			{
				std::cout << "(" << std::to_string(dftData[i].real()) << ";" << std::to_string(dftData[i].imag()) << ")" << std::endl;
			}
		}

	private:
		static int ServiceAudio_(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
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

	public:
		const size_t sampleRate, bufferSize;
	private:
		MyUserData clip_{};
		PaStream* pStream_ = nullptr;
		std::vector<std::complex<float>> currentBufferDFT_;
	};

	class SdlManager
	{
	public:
		SdlManager() = delete;
		SdlManager(const size_t displaySideLen, const Action action): displaySideLen(displaySideLen), action(action)
		{
			auto err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
			assert(err == 0 && "Failed to initialize sdl.");

			err = SDL_CreateWindowAndRenderer(displaySideLen, displaySideLen, SDL_WINDOW_SHOWN, &pWindow_, &pRenderer_);
			assert(err == 0 && "Failed to create sdl window and renderer!");
		}
		~SdlManager()
		{
			if (pRenderer_)SDL_DestroyRenderer(pRenderer_);
			if (pWindow_)SDL_DestroyWindow(pWindow_);
			SDL_Quit();
		}

		void ArgandPlaneVisualization(const std::vector<std::complex<float>>& dftData)
		{
			SDL_SetRenderDrawColor(pRenderer_, 255, 255, 255, 255);

			const int displayCenter = displaySideLen / 2;
			for (size_t i = 0; i < dftData.size(); i++)
			{
				constexpr const float SCALING_FACTOR = 5.0f;
				const int x = dftData[i].real() * SCALING_FACTOR;
				const int y = dftData[i].imag() * SCALING_FACTOR;

				SDL_RenderDrawLine(pRenderer_, displayCenter, displayCenter, displayCenter + x, displayCenter + y);
			}
		}

		void RectangularWindowVisualization(const std::vector<std::complex<float>>& dftData)
		{
			if (!dftData.size()) return;

			const auto magnitude = [](const std::complex<float>& c)->float
			{
				return std::sqrtf(c.real() * c.real() + c.imag() * c.imag());
			};

			SDL_SetRenderDrawColor(pRenderer_, 255, 255, 255, 255);

			float currentX = 0.0f;
			const float step = displaySideLen / dftData.size();
			for (size_t i = 0; i < dftData.size(); i++)
			{
				constexpr const float SCALING_FACTOR = 15.0f;
				SDL_RenderDrawLineF(pRenderer_, currentX, displaySideLen, currentX, displaySideLen - magnitude(dftData[i]) * SCALING_FACTOR);
				currentX += step;
			}
		}

		void Draw(const std::vector<std::complex<float>>& dftData)
		{
			SDL_SetRenderDrawColor(pRenderer_, 0, 0, 0, 255);
			SDL_RenderClear(pRenderer_);

			switch (action)
			{
				case Action::VisualizeCurrentBufferDFTArgand:
				{
					ArgandPlaneVisualization(dftData);
				}break;

				case Action::VisualizeCurrentBufferDFTrectangularWindow:
				{
					RectangularWindowVisualization(dftData);
				}break;

				default:
				break;
			}

			SDL_RenderPresent(pRenderer_);
		}

		bool ProcessInputs(AudioEngine& engine)
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
							AudioEngine::PrintDFT(engine.GetCurrentBufferDFT());
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
	
	public:
		const size_t displaySideLen;
		const Action action;
	private:
		SDL_Event event_{};
		SDL_Window* pWindow_ = nullptr;
		SDL_Renderer* pRenderer_ = nullptr;
	};

	class Application
	{
	public:
		Application() = delete;
		Application(const Action action, const char* pathToRelevantFile, const float clipDuration, const size_t sampleRate, const size_t bufferSize, const size_t displaySideLen):
			renderer_(AudioEngine(action, pathToRelevantFile, clipDuration, sampleRate, bufferSize)),
			sdl_(SdlManager(displaySideLen, action)),
			action_(action)
		{
			switch (action)
			{
				case Action::VisualizeDFToutput:
				{
					dftData = renderer_.ParseDFTOutput(pathToRelevantFile);
				}break;

				default:
				break;
			}

			EASY_PROFILER_ENABLE;
		}
		~Application()
		{
			if (!profiler::dumpBlocksToFile("profilingData/session0.prof"))
			{
				assert(true, "Couldn't write profiling session to file!");
			}
		}

		int Run()
		{
			bool shutdown = false;
			while (!shutdown)
			{
				switch (action_)
				{
					case Action::VisualizeCurrentBufferDFTArgand:
					{
						dftData = renderer_.GetCurrentBufferDFT();
					}break;

					case Action::VisualizeCurrentBufferDFTrectangularWindow:
					{
						dftData = renderer_.GetCurrentBufferDFT();
					}break;

					default:
					break;
				}

				shutdown = sdl_.ProcessInputs(renderer_);
				sdl_.Draw(dftData);
				Pa_Sleep(32); // ~30 fps
			}

			return 0;
		}

	private:
		AudioEngine renderer_;
		SdlManager sdl_;
		Action action_;

		std::vector<std::complex<float>> dftData;
	};
}
