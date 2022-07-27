#include "Common.h"

#include <cassert>
#include <fstream>
#include <iostream>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

Clip LoadWavFile(const char* filePath, const std::uint32_t channels, const std::uint32_t sampleRate)
{
	drwav_uint64 totalPCMFrameCount;
	unsigned int chan, smplRte;

	float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(filePath, &chan, &smplRte, &totalPCMFrameCount, NULL);
	assert(pSampleData != nullptr && "Failed to load wav file.");

	assert(chan == channels && smplRte == sampleRate && "Sound data retrieved is in a different format than specified!");

	Clip returnVal;
	returnVal.audioData.resize(totalPCMFrameCount);
	memcpy(&*returnVal.audioData.begin(), pSampleData, totalPCMFrameCount * sizeof(float));
	drwav_free(pSampleData, NULL);

	return returnVal;
}

std::vector<std::complex<float>> ParseDFTOutput(const char* path)
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

void PrintDFT(const std::vector<std::complex<float>>& dftData)
{
	for (size_t i = 0; i < dftData.size(); i++)
	{
		std::cout << "(" << std::to_string(dftData[i].real()) << ";" << std::to_string(dftData[i].imag()) << ")" << std::endl;
	}
}