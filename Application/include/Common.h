#pragma once

#include <cassert>

#include "dr_wav.h"

#define MYFDN_SAMPLE_RATE 8000
#define MYFDN_BUFFER_SIZE 2048
#define MYFDN_SEED 0x1337

struct MyUserData
{
    std::vector<float> audioData;
    size_t currentBegin = 0;
    size_t currentEnd = 0;
};

MyUserData LoadWavFile(const char* filePath, const std::uint32_t channels, const std::uint32_t sampleRate)
{
    drwav_uint64 totalPCMFrameCount;
    unsigned int chan, smplRte;

    float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(filePath, &chan, &smplRte, &totalPCMFrameCount, NULL);
    assert(pSampleData != nullptr && "Failed to load wav file.");

    assert(chan == channels && smplRte == sampleRate && "Sound data retrieved is in a different format than specified!");

    MyUserData returnVal;
    returnVal.audioData.resize(totalPCMFrameCount);
    memcpy(&*returnVal.audioData.begin(), pSampleData, totalPCMFrameCount * sizeof(float));
    drwav_free(pSampleData, NULL);

    return returnVal;
}