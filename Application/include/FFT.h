#pragma once

#include <cassert>
#include <iostream>
#include <algorithm>

#include <portaudio.h>

#include "Common.h"
#include "FDN.h"
#include "FIRfilterCoef.h"

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

	err = Pa_OpenStream(
		&pStream,
		NULL, /* no input */
		&outputParameters,
		(double)MYFDN_SAMPLE_RATE,
		(double)MYFDN_BUFFER_SIZE,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		patestCallback,
		&data);
	assert(err == paNoError && "Failed to open stream to device.");

	err = Pa_StartStream(pStream);
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

std::vector<std::complex<float>> Difference(const std::vector<std::complex<float>>& a, const std::vector<std::complex<float>>& b)
{
	assert(a.size() == b.size() && "a and b have mismatching sizes.");
	std::vector<std::complex<float>> returnVal(a.size(), 0.0f);
	for (size_t i = 0; i < a.size(); i++)
	{
		returnVal[i] = b[i] - a[i];
	}
	return returnVal;
}

std::vector<std::complex<float>> RealToComplex(const std::vector<float>& input)
{
	std::vector<std::complex<float>> returnVal(input.size(), 0.0f);
	for (size_t i = 0; i < input.size(); i++)
	{
		returnVal[i] = std::complex<float>(input[i], 0.0f);
	}
	return returnVal;
}

float ComplexMagnitude(const std::complex<float> c)
{
	return std::sqrtf(c.real() * c.real() + c.imag() * c.imag());
}

int Run()
{
	Clip data = LoadWavFile("../resources/audioSamples/sine512Hz_8000Hz_32f_8192samples.wav", 1, MYFDN_SAMPLE_RATE);

	const auto fft = MyFDN::FFT(RealToComplex(data.audioData));
	// const auto dft = MyFDN::DFT(data.audioData, MYFDN_BUFFER_SIZE);

	// return 0;

	// const auto dft = MyFDN::DFT(data.audioData, MYFDN_SAMPLE_RATE);

	// const auto diff = Difference(dft, fft);

	// constexpr const float errThreshold = 0.001f;
	// for (size_t i = 0; i < diff.size(); i++)
	// {
	// 	if (ComplexMagnitude(diff[i]) > errThreshold)
	// 	{
	// 		std::cout << "Difference for frequency " << std::to_string(i) << "Hz: " << std::to_string(ComplexMagnitude(diff[i])) << std::endl;
	// 	}
	// }

	data.audioData = MyFDN::IDFT(fft, MYFDN_SAMPLE_RATE * 1.024f);

	ToWavFile(data.audioData, "output.wav", MYFDN_SAMPLE_RATE, 1);

	InitPortaudio(data);
	std::cin.ignore();
	ShutdownPortAudio();

	return 0;
}