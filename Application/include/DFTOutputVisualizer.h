#include <vector>
#include <math.h>
#include <cassert>
#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <fstream>

#include "portaudio.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#include <SDL.h>
#undef main

#include "Common.h"
#include "FDN.h"

namespace DFTVisualizer
{
	class AudioEngine
	{
	public:
		enum class Action
		{
			ComputeAndPrintDFT = 0,
			PlaySignalFromDFToutput,
			PlayWav
		};

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
					PrintDFT_(dftResult);
				}break;

				case Action::PlaySignalFromDFToutput:
				{
					clip_.audioData = MyFDN::IDFT(ParseDFTOutput_(path), audioDataDurationInSec);
				}break;

				case Action::PlayWav:
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

		static std::vector<std::complex<float>> ParseDFTOutput_(const char* path)
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

			std::vector<std::complex<float>> returnVal(lines.size());
			for (size_t i = 0; i < lines.size(); i++)
			{
				returnVal[i] = complexFromString(lines[i]);
			}

			return returnVal;
		}

		static void PrintDFT_(const std::vector<std::complex<float>>& dftData)
		{
			for (size_t i = 0; i < dftData.size(); i++)
			{
				std::cout << "(" << std::to_string(dftData[i].real()) << ";" << std::to_string(dftData[i].imag()) << ")" << std::endl;
			}
		}

	public:
		const size_t sampleRate, bufferSize;
	private:
		MyUserData clip_{};
		PaStream* pStream_ = nullptr;
	};

	class SdlManager
	{
	public:
		SdlManager()
		{
			auto err = SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS);
			assert(err == 0 && "Failed to initialize sdl.");
		}
	
	private:
		SDL_Window* pWindow_ = nullptr;
	};

	class Application
	{
	public:
		Application() = delete;
		Application(const AudioEngine::Action action, const char* pathToRelevantFile, const float clipDuration, const size_t sampleRate, const size_t bufferSize): renderer_(AudioEngine(action, pathToRelevantFile, clipDuration, sampleRate, bufferSize))
		{

		}

		int Run()
		{
			Pa_Sleep(1000 * 5);

			return 0;
		}

	private:
		AudioEngine renderer_;
		SdlManager sdl_;
	};
}
