#include <vector>
#include <math.h>
#include <cassert>
#include <algorithm>
#include <array>
#include <iostream>

#include "portaudio.h"
#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "Common.h"
#include "FDN.h"

int patestCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo * timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
 {
    float* out = (float*)outputBuffer;
	std::vector<float> outputBuff = std::vector<float>(MYFDN_BUFFER_SIZE, 0.0f);

    // MyFDN::WhiteNoise(outputBuff, MYFDN_SEED);
    MyFDN::GaussianWhiteNoise(outputBuff, MYFDN_SEED);

    for (size_t i = 0; i < MYFDN_BUFFER_SIZE; i++)
    {
        *out++ = outputBuff[i];
        *out++ = outputBuff[i];
    }

    return paContinue;
}

int Run()
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
        NULL);
    assert(err == paNoError && "Failed to open stream to device.");

    err = Pa_StartStream(stream);
    assert(err == paNoError && "Failed to start stream.");

    std::cin.ignore();

    err = Pa_StopStream(stream);
    assert(err == paNoError && "Failed to stop stream.");

    err = Pa_CloseStream(stream);
    assert(err == paNoError && "Failed to close stream.");

    Pa_Terminate();

    return 0;
}
