#include "AudioEngine.h"

#include <cassert>
#include <thread>
#include <iostream>

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
	static float theta = 0.0f;
	for (size_t i = 0; i < bufferSize; i++)
	{
		outLeft[i] = sinf(theta);
		outRight[i] = outLeft[i];
		theta += 0.01f;
	}
	return;

	assert(outLeft.size() == outRight.size() && outRight.size() == bufferSize && "Invalid buffer sizes.");

	if (paused || !IsPlaying()) return;

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
		NULL, // Implementing blocking IO, no callback.
		NULL
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

void MyApp::AudioEngine::ServiceAudio()
{
	std::lock_guard<std::mutex> l(m_);
	std::swap(frontBuffer_, backBuffer_);
	
	auto err = Pa_WriteStream(stream_, frontBuffer_.data(), bufferSize);
	if (err != paNoError) throw std::runtime_error(std::string("Failed to service the audio: ") + Pa_GetErrorText(err));
	
	std::thread audioProcessThread([&]() { ProcessAudioAsync(); }); // Omnissiah forgive me.
	audioProcessThread.detach();
}
void MyApp::AudioEngine::ProcessAudioAsync()
{
	// static float theta = 0.0f;
	// for (size_t i = 0; i < bufferSize; i++)
	// {
	// 	backBuffer_[2 * i] = sinf(theta);
	// 	backBuffer_[2 * i + 1] = backBuffer_[2 * i];
	// 	theta += 0.01f;
	// }
	// return;

	std::lock_guard<std::mutex> l(m_);
	static std::vector<float> left(bufferSize);
	static std::vector<float> right(bufferSize);
	static std::vector<float> stereoSignal(2 * (size_t)bufferSize);

	for (size_t i = 0; i < sounds_.size(); ++i)
	{
		sounds_[i].Process_(left, right);
		MyUtils::InterleaveSignals(stereoSignal, left, right);
		MyUtils::SumSignals(backBuffer_, stereoSignal);
	}

	for (size_t i = 0; i < postProcessFx_.size(); ++i)
	{
		// postProcessFx_[i](backBuffer_);
	}
}