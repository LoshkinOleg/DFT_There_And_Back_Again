#include "AudioEngine.h"

#include <cassert>
#include <iostream>
#include <thread>

#include <easy/profiler.h>

#include "AssetManager.h"
#include "MyUtils.h"

void MyApp::Sound::Play()
{
	currentBegin_ = 0;
	currentEnd_ = bufferSize - 1;
}
void MyApp::Sound::Stop()
{
	currentBegin_ = (unsigned int)-1;
	currentEnd_ = (unsigned int)-1;
}

void MyApp::Sound::AddEffect(const std::function<void(std::vector<float>&)>& effect)
{
	fx_.push_back(effect);
}
std::vector<std::function<void(std::vector<float>&)>> MyApp::Sound::GetEffectsCopy() const
{
	return fx_;
}
void MyApp::Sound::RemoveAllEffects()
{
	fx_.clear();
}

MyApp::Sound::Sound(const unsigned int bufferSize): bufferSize(bufferSize) {}

void MyApp::Sound::Process_(std::vector<float>& outLeft, std::vector<float>& outRight)
{
	assert(outLeft.size() == outRight.size() && outRight.size() == bufferSize && "Invalid buffer sizes.");

	const bool playing = IsPlaying();

	if (paused || !playing) return;

	const unsigned int dataSize = (unsigned int)data.size();

	std::fill(outLeft.begin(), outLeft.end(), 0.0f);
	std::fill(outRight.begin(), outRight.end(), 0.0f);

	if (currentBegin_ > currentEnd_) // Wrapping around wav data.
	{
		for (unsigned int i = currentBegin_; i < dataSize; ++i) // Finish reading end of the wav.
		{
			outLeft[i - currentBegin_] = data[i];
		}
		if (looping)
		{
			for (unsigned int i = 0; i < currentEnd_ + 1; ++i) // Read the start of the data into the remaining not yet updated part of the soundDataSubset_.
			{
				outLeft[dataSize - currentBegin_ + i] = data[i];
			}
		}
	}
	else // Not wrapping around wav data, just copy soundData into subset continuously.
	{
		for (unsigned int i = currentBegin_; i < currentEnd_ + 1; ++i)
		{
			outLeft[i - currentBegin_] = data[i];
		}
	}

	// Apply sound effects.
	for (size_t i = 0; i < fx_.size(); ++i)
	{
		fx_[i](outLeft);
	}

	// Just copy left buffer to right buffer.
	std::copy(outLeft.begin(), outLeft.end(), outRight.begin());

	// Update indices.
	if (looping) // currentBegin_ can never reach data.size().
	{
		// Update currentBegin_
		if (currentEnd_ + 1 == dataSize) // If dataSize % bufferSize = 0, this can happen, wrap back to 0.
		{
			currentBegin_ = 0;
		}
		else // Just advance to next subset.
		{
			currentBegin_ = currentEnd_ + 1;
		}

		// Update currentEnd_
		if (currentBegin_ + bufferSize - 1 >= dataSize) // If overruning wav data, wrap around.
		{
			currentEnd_ = bufferSize - 1 - (dataSize - currentBegin_);
		}
		else // Not overruning wav data, just update currentEnd_.
		{
			currentEnd_ = currentBegin_ + bufferSize - 1;
		}
	}
	else // currentBegin_ can reach wavSize.
	{
		if (currentEnd_ + 1 == dataSize) // Happens when clip reached the last window of the wav data, stop playing.
		{
			currentBegin_ = (unsigned int)-1;
			currentEnd_ = (unsigned int)-1;
		}
		else
		{
			currentBegin_ = currentEnd_ + 1;

			if (currentBegin_ + bufferSize - 1 >= dataSize) // Reached end of the wav.
			{
				currentEnd_ = dataSize - 1;
			}
			else
			{
				currentEnd_ = currentBegin_ + bufferSize - 1;
			}
		}
	}
}

MyApp::AudioEngine::AudioEngine(const unsigned int sampleRate, const unsigned int bufferSize): sampleRate(sampleRate), bufferSize(bufferSize)
{
	// Init portaudio.
	auto err = Pa_Initialize();
	if (err != paNoError) throw std::runtime_error(std::string("Failed to initialize PortAudio: ") + Pa_GetErrorText(err));

	const PaDeviceIndex selectedDevice = Pa_GetDefaultOutputDevice();
	if (selectedDevice == paNoDevice) throw std::runtime_error(std::string("PortAudio failed to retireve a default playback device."));

	defaultPlaybackDelay_ = Pa_GetDeviceInfo(selectedDevice)->defaultLowInputLatency;

	PaStreamParameters outputParams{
		selectedDevice,
		2, // Engine only supports headphones. 2 channels.
		paFloat32,
		defaultPlaybackDelay_,
		NULL
	};

	err = Pa_OpenStream(
		&stream_,
		NULL, // Not handling audio input.
		&outputParams,
		(double)sampleRate,
		(unsigned long)bufferSize,
		paClipOff,
		&ServiceAudio_,
		this
	);
	if (err != paNoError) throw std::runtime_error(std::string("Failed to open a stream to default playback device: ") + Pa_GetErrorText(err));

	err = Pa_StartStream(stream_);
	if (err != paNoError) throw std::runtime_error(std::string("Failed to start stream to default playback device: ") + Pa_GetErrorText(err));
}

MyApp::AudioEngine::~AudioEngine()
{
	auto err = Pa_StopStream(stream_);
	if (err != paNoError) std::cerr << std::string("Error stopping stream to playback device: ") + Pa_GetErrorText(err) << std::endl;
	err = Pa_CloseStream(stream_);
	if (err != paNoError) std::cerr << std::string("Error closing stream to playback device: ") + Pa_GetErrorText(err);
	err = Pa_Terminate();
	if (err != paNoError) std::cerr << std::string("Error shutting down PortAudio: ") + Pa_GetErrorText(err);
}

MyApp::Sound& MyApp::AudioEngine::CreateSound(const char* path, AssetManager& assetManager)
{
	unsigned int nrOfChannels, sampleRate;
	auto wavData = assetManager.LoadWav(path, nrOfChannels, sampleRate);

	sounds_.push_back(Sound(bufferSize));
	sounds_.back().data = wavData;

	return sounds_.back();
}
MyApp::Sound& MyApp::AudioEngine::CreateSound(const std::vector<float>& data)
{
	sounds_.push_back(Sound(bufferSize));
	sounds_.back().data = data;

	return sounds_.back();
}
MyApp::Sound& MyApp::AudioEngine::DuplicateSound(const Sound& other)
{
	sounds_.push_back(Sound(bufferSize));
	sounds_.back().data = other.data;
	return sounds_.back();
}

void MyApp::AudioEngine::StopAll()
{
	for (size_t i = 0; i < sounds_.size(); ++i)
	{
		sounds_[i].Stop();
	}
}

int MyApp::AudioEngine::ServiceAudio_(const void* input, void* output,
									   unsigned long frameCount,
									   const PaStreamCallbackTimeInfo* timeInfo,
									   PaStreamCallbackFlags statusFlags,
									   void* userData)
{
	EASY_BLOCK("ServiceAudio_()");

	auto self = (MyApp::AudioEngine*)userData;
	std::lock_guard<std::mutex> l(self->m_);

	std::swap(self->frontBuffer_, self->backBuffer_);
	std::memcpy(output, self->frontBuffer_.data(), sizeof(float) * self->frontBuffer_.size());
	self->processNextBuffer_ = true;

	return paContinue;
}
void MyApp::AudioEngine::ProcessAudio()
{
	EASY_BLOCK("ProcessAudio()");

	// Don't process unless the audio needs servicing. Acquire lock to check boolean. Note: I'm sure there's a better way to do this?
	{
		EASY_BLOCK("ProcessAudio(): checking bool");
		std::lock_guard<std::mutex> l(m_);
		if (!processNextBuffer_) return;
	}

	static std::vector<float> left(bufferSize);
	static std::vector<float> right(bufferSize);
	static std::vector<float> stereoSignal(2 * (size_t)bufferSize);
	static std::vector<float> processedBackbuffer(2 * (size_t)bufferSize);

	std::fill(processedBackbuffer.begin(), processedBackbuffer.end(), 0.0f);
	for (size_t i = 0; i < sounds_.size(); ++i)
	{
		sounds_[i].Process_(left, right);
		MyUtils::InterleaveSignals(stereoSignal, left, right); // Note: pretty sure you can rearrange things to move this method out of the for loop.
		MyUtils::SumSignals(processedBackbuffer, stereoSignal);
	}

	for (size_t i = 0; i < postProcessFx_.size(); ++i)
	{
		postProcessFx_[i](backBuffer_);
	}

	// Acquire lock and write to backbuffer.
	{
		EASY_BLOCK("ProcessAudio(): writing to backBuffer");
		std::lock_guard<std::mutex> l(m_);
		std::copy(processedBackbuffer.begin(), processedBackbuffer.end(), backBuffer_.begin());
		processNextBuffer_ = false;
	}
}