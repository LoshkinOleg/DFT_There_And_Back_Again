#pragma once

#include <vector>
#include <functional>
#include <mutex>

#include <portaudio.h>

namespace MyApp
{
	class AssetManager;

	class Sound
	{
	public:

		Sound() = delete;
		Sound(const unsigned int bufferSize);

		void Play();
		void Stop();

		void AddEffect(const std::function<void(std::vector<float>&)>& effect);
		std::vector<std::function<void(std::vector<float>&)>> GetEffectsCopy() const;
		void RemoveAllEffects();

		inline bool IsPlaying() const
		{
			return currentBegin_ < data.size();
		}

		bool looping = true;
		bool paused = false;
		std::vector<float> data;

		const unsigned int bufferSize;

	private:
		friend class AudioEngine;

		void Process_(std::vector<float>& outLeft, std::vector<float>& outRight);

		unsigned int currentBegin_ = (unsigned int)-1;
		unsigned int currentEnd_ = (unsigned int)-1;

		std::vector<std::function<void(std::vector<float>&)>> fx_;
	};

	class AudioEngine
	{
	public:

		AudioEngine() = delete;
		AudioEngine(const unsigned int sampleRate, const unsigned int bufferSize);
		~AudioEngine();

		Sound& CreateSound(const char* path, AssetManager& assetManager);
		Sound& CreateSound(const std::vector<float>& data);
		Sound& DuplicateSound(const Sound& other);

		void StopAll();

		void ServiceAudio();
		void ProcessAudioAsync();

		const unsigned int sampleRate;
		const unsigned int bufferSize;

	private:
		std::vector<float> frontBuffer_ = std::vector<float>(2 * (size_t)bufferSize, 0.0f);
		std::vector<float> backBuffer_ = std::vector<float>(2 * (size_t)bufferSize, 0.0f);
		std::vector<std::function<void(std::vector<float>&)>> postProcessFx_;

		PaStream* stream_ = nullptr;
		std::vector<Sound> sounds_;

		std::mutex m_;

		double defaultPlaybackDelay_ = 0.0;
	};
}