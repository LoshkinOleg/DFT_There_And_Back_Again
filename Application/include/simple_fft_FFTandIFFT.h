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

#include "Common.h"
#include "FDN.h"

int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	float* out = (float*)outputBuffer;
	std::vector<float> outputBuff = std::vector<float>(MYFDN_BUFFER_SIZE, 0.0f);
	MyUserData* const data = (MyUserData*)userData;
	auto& begin = data->currentBegin;
	auto& end = data->currentEnd;
	const auto& audioData = data->audioData;
	const auto wavSize = data->audioData.size();

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

	MyFDN::SimpleFDN(outputBuff, outputBuff, MYFDN_BUFFER_SIZE, 0.8f);

	for (size_t i = 0; i < MYFDN_BUFFER_SIZE; i++)
	{
		*out++ = outputBuff[i];
		*out++ = outputBuff[i];
	}

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

	return paContinue;
}

void PrintComplex(const std::complex<float> c)
{
    std::cout << "(" << std::to_string(c.real()) << ";" << std::to_string(c.imag()) << ")" << std::endl;
}

std::complex<float> ComplexFromString(const std::string& str)
{
	const std::string realStr = std::string(str.begin() + str.find_first_of("(") + 1, str.begin() + str.find_first_of(";"));
	const std::string imagStr = std::string(str.begin() + str.find_first_of(";") + 1, str.begin() + str.find_first_of(")") - 1);

	return std::complex<float>(std::stof(realStr), std::stof(imagStr));
}

std::vector<std::complex<float>> ParseDFTOutput(const char* path)
{
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
		returnVal[i] = ComplexFromString(lines[i]);
	}

	return returnVal;
}

int Run()
{
	/*
		Uncomment this part to write the output of the DFT to standard output.
	*/
    const auto data = LoadWavFile("../resources/audioSamples/sine440_8000Hz_32f_1sec.wav", 1, MYFDN_SAMPLE_RATE);
    const auto dftResult = MyFDN::SimpleFFT_FFT(data.audioData);
	
    for (size_t i = 0; i < dftResult.size(); i++)
    {
        PrintComplex(dftResult[i]);
    }

	/*
		Uncomment this part to load, process and play the result of the IDFT that takes an input the DFT's output.
	*/
	// MyUserData data;
	// data.audioData = MyFDN::IDFT(ParseDFTOutput("../results/440InputForIDFT.txt"));
	// auto err = Pa_Initialize();
	// assert(err == paNoError && "Failed to initialize PortAudio!");
	// 
	// PaStreamParameters outputParameters =
	// {
	// 	Pa_GetDefaultOutputDevice(),
	// 	2,
	// 	paFloat32,
	// 	Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice())->defaultLowOutputLatency,
	// 	NULL
	// };
	// assert(outputParameters.device != paNoDevice && "Failed to retrieve a default playback device!");
	// 
	// PaStream* stream;
	// err = Pa_OpenStream(
	// 	&stream,
	// 	NULL, /* no input */
	// 	&outputParameters,
	// 	(double)MYFDN_SAMPLE_RATE,
	// 	(double)MYFDN_BUFFER_SIZE,
	// 	paClipOff,      /* we won't output out of range samples so don't bother clipping them */
	// 	ServiceAudio_,
	// 	&data);
	// assert(err == paNoError && "Failed to open stream to device.");
	// 
	// err = Pa_StartStream(stream);
	// assert(err == paNoError && "Failed to start stream.");
	// 
	// std::cin.ignore();
	// 
	// err = Pa_StopStream(stream);
	// assert(err == paNoError && "Failed to stop stream.");
	// 
	// err = Pa_CloseStream(stream);
	// assert(err == paNoError && "Failed to close stream.");
	// 
	// Pa_Terminate();

    return 0;
}
