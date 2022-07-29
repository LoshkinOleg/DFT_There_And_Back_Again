#pragma once

#include <cassert>
#include <iostream>
#include <algorithm>

#include <portaudio.h>

#include "Common.h"
#include "FDN.h"
#include "FIRfilterCoef.h"

class FIRfilter
{
public:
	void Apply(const std::vector<float>& input, std::vector<float>& output) const
	{
		// TODO: find origin of popping

		// Taken from: https://www.youtube.com/watch?v=6sxsrUkaTGU

		// assert(input.size() == output.size() && "Input and output vectors must have the same length.");
		assert(&*input.begin() != &*output.begin() && "Do not pass the same vector as input and output.");

		// Math aliases.
		const auto& x = input;
		const auto& c = ir_;
		auto& y = output;
		const auto K = output.size();
		const auto N = FILTER_TAP_NUM;

		std::fill(y.begin(), y.end(), 0.0f); // Set output to zero.
		for (size_t k = 0; k < K; k++)
		{
			for (size_t n = 0; n < N; n++)
			{
				if (k + n < x.size())
				{
					y[k] = y[k] + x[k + n] * c[n];
				}
			}
		}
	}
private:
	const float* ir_ = filter_taps;
};

void ComputeNextIndices(Clip& clip)
{
	auto& begin = clip.currentBegin;
	auto& end = clip.currentEnd;
	const auto& audioData = clip.audioData;
	const auto wavSize = clip.audioData.size();

	// Update begin
	assert(end + 1 <= wavSize && "end somehow incremented past wavSize! Last iteration must have had end = wavSize, which shouldn't be possible. Check your code.");
	if (end + 1 == wavSize) // If wavSize % bufferSize = 0, this can happen, wrap back to 0.
	{
		begin = 0;
	}
	else // Just advance to next subset with no issues.
	{
		begin = end + 1;
	}

	// Update currentEnd_
	if (begin + MYFDN_BUFFER_SIZE - 1 >= wavSize) // If overruning wav data, wrap around.
	{
		end = MYFDN_BUFFER_SIZE - 1 - (wavSize - begin);
	}
	else // Not overruning wav data, just update currentEnd_ with no issues.
	{
		end = begin + MYFDN_BUFFER_SIZE - 1;
	}
}

void LoadNextAudioData(Clip& clip, std::vector<float>& outputBuff)
{
	auto& begin = clip.currentBegin;
	auto& end = clip.currentEnd;
	const auto& audioData = clip.audioData;
	const auto wavSize = clip.audioData.size();

	// Read wav subset.
	if (begin > end) // Wrapping around wav data.
	{
		for (size_t i = begin; i < wavSize; i++) // Finish reading end of the wav.
		{
			outputBuff[i - begin] = audioData[i];
		}
		for (size_t i = 0; i < end + 1; i++) // Read the start of the wav into the remaining not yet updated part of the soundDataSubset_.
		{
			outputBuff[wavSize - begin + i] = audioData[i];
		}
	}
	else // Not wrapping around wav data, just copy soundData into subset continuously.
	{
		for (size_t i = begin; i < end + 1; i++)
		{
			outputBuff[i - begin] = audioData[i];
		}
	}
}

void ApplyFilter(std::vector<float>& outputBuff)
{
	static FIRfilter f;
	static std::vector<float> filterInput(MYFDN_BUFFER_SIZE, 0.0f);
	std::copy(outputBuff.begin(), outputBuff.end(), filterInput.begin());
	f.Apply(filterInput, outputBuff);
}

int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	float* out = (float*)outputBuffer;
	std::vector<float> outputBuff = std::vector<float>(MYFDN_BUFFER_SIZE, 0.0f);
	Clip* const data = (Clip*)userData;
	auto& begin = data->currentBegin;
	auto& end = data->currentEnd;
	const auto& audioData = data->audioData;
	const auto wavSize = data->audioData.size();

	LoadNextAudioData(*data, outputBuff);
	// ApplyFilter(outputBuff);

	for (size_t i = 0; i < MYFDN_BUFFER_SIZE; i++)
	{
		*out++ = outputBuff[i];
		*out++ = outputBuff[i];
	}

	ComputeNextIndices(*data);

	return paContinue;
}

PaStream* pStream = nullptr;
void InitPortaudio(Clip& data)
{
	auto err = Pa_Initialize();
	assert(err == paNoError && "Failed to initialize PortAudio!");

	PaStreamParameters outputParameters =
	{
		Pa_GetDefaultOutputDevice(),
		2,
		paFloat32,
		Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->defaultLowOutputLatency,
		NULL
	};
	assert(outputParameters.device != paNoDevice && "Failed to retrieve a default playback device!");

	PaStream* stream;
	err = Pa_OpenStream(
		&stream,
		NULL, /* no input */
		&outputParameters,
		(double)MYFDN_SAMPLE_RATE,
		(double)MYFDN_BUFFER_SIZE,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		patestCallback,
		&data);
	assert(err == paNoError && "Failed to open stream to device.");

	err = Pa_StartStream(stream);
	assert(err == paNoError && "Failed to start stream.");
}

void ShutdownPortAudio()
{
	auto err = Pa_StopStream(pStream);
	assert(err == paNoError && "Failed to stop stream.");

	err = Pa_CloseStream(pStream);
	assert(err == paNoError && "Failed to close stream.");

	Pa_Terminate();
}

int Run()
{
	Clip data = LoadWavFile("../resources/audioSamples/olegSpeech_44100Hz_32f.wav", 1, MYFDN_SAMPLE_RATE);
	
	InitPortaudio(data);

	std::cin.ignore();

	ShutdownPortAudio();

	return 0;
}

void WriteFilteredWav(const char* wavPath, const size_t clipDuration)
{
	Clip data = LoadWavFile("../resources/audioSamples/olegSpeech_8000Hz_32f.wav", 1, MYFDN_SAMPLE_RATE);
	
	// MyFDN::PadToNearestPowerOfTwo(data.audioData);

	const auto fourierTransform = MyFDN::DFT(data.audioData, MYFDN_SAMPLE_RATE);
	
	// std::vector<std::complex<float>> idealFilterComplexIR(MYFDN_SAMPLE_RATE / 2, std::complex<float>(0.0f, 0.0f));
	// constexpr const size_t cutoffFrequency = 1500;
	// for (size_t i = 0; i < cutoffFrequency; i++)
	// {
	// 	idealFilterComplexIR[i] = std::complex<float>(1.0f, 0.0f);
	// }
	// 
	// const auto timeDomainIR = MyFDN::IDFT(idealFilterComplexIR, clipDuration);
	data.audioData = MyFDN::IDFT(fourierTransform, clipDuration);

	ToWavFile(data.audioData, wavPath, MYFDN_SAMPLE_RATE, 1);

	std::cout << "Done." << std::endl;

	InitPortaudio(data);

	std::cin.ignore();

	ShutdownPortAudio();
}