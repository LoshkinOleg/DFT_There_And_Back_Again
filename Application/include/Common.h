#pragma once

#include <vector>
#include <complex>

#define MYFDN_SAMPLE_RATE 44100
#define MYFDN_BUFFER_SIZE 2048
#define MYFDN_SEED 0x1337

struct Clip
{
    std::vector<float> audioData;
    size_t currentBegin = 0;
    size_t currentEnd = 0;
};

Clip LoadWavFile(const char* filePath, const std::uint32_t channels, const std::uint32_t sampleRate);

std::vector<std::complex<float>> ParseDFTOutput(const char* path);

void PrintDFT(const std::vector<std::complex<float>>& dftData);