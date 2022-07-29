#pragma once

#include <vector>
#include <complex>

#define MYFDN_SAMPLE_RATE 8000
#define MYFDN_BUFFER_SIZE 1024
#define MYFDN_SEED 0x1337

struct Clip
{
    std::vector<float> audioData;
    size_t currentBegin = 0;
    size_t currentEnd = 0;
};

Clip LoadWavFile(const char* filePath, const std::uint32_t channels, const std::uint32_t sampleRate);

void ToWavFile(const std::vector<float>& signal, const char* path, const size_t sampleRate, const size_t nrOfChannels);

std::vector<std::complex<float>> ParseDFTOutput(const char* path);

void PrintDFT(const std::vector<std::complex<float>>& dftData);