#pragma once

#include <vector>
#include <functional>
#include <mutex>

#include <portaudio.h>

namespace MyApp
{
	class AssetManager;

	// Class representing a single instance of a sound. It's lifetime is managed by the AudioEngine.
	class Sound
	{
	public:

		Sound() = delete;
		/**
		* Constructs a Sound instance.
		* 
		* @param bufferSize Size of the audio buffer used to service the audio (not the size of data!).
		*/
		Sound(const unsigned int bufferSize);

		/**
		* Plays the sound by setting currentBegin_ to 0 and currentEnd_ to bufferSize.
		*/
		void Play();
		/**
		* Stops the sound by setting currentBegin_ and currentEnd_ to a value > data.size().
		*/
		void Stop();

		/**
		* Adds a post-processing effect to this Sound that will be applied before servicing the audio.
		* 
		* @param effect The callback that will apply the effect. Use a lambda. Signature should be: void fx(std::vector<float>& outBuffer).
		*/
		void AddEffect(std::function<void(std::vector<float>&)> effect);

		/**
		* Returns a copy of fx_.
		* 
		* @return Copy of fx_.
		*/
		std::vector<std::function<void(std::vector<float>&)>> GetEffectsCopy() const;

		/**
		* Clears the fx_ vector.
		*/
		void RemoveAllEffects();

		inline bool IsPlaying() const
		{
			return currentBegin_ < data.size();
		}

		inline unsigned int GetCurrentBegin() const
		{
			return currentBegin_;
		}
		inline unsigned int GetCurrentEnd() const
		{
			return currentEnd_;
		}

		bool looping = true; // When set to true, the sound will start playing over once it has reached the end of data.
		bool paused = false; // When set to true, suspends the update of currentBegin_ and currentEnd_ and prevents the Sound instance from servicing the audio.
		std::vector<float> data; // Buffer containing a monophonic signal to play back. It's lifetime is managed by Sound.

		const unsigned int bufferSize; // Size of the audio buffer used to service the audio (not the size of data).

	private:
		friend class AudioEngine;

		/**
		* Called at every AudioEngine update. Loads the next subsection of data into the provided buffers with fx_ applied.
		* 
		* @param outLeft Output left channel.
		* @param outRight Output right channel.
		*/
		void Process_(std::vector<float>& outLeft, std::vector<float>& outRight);

		unsigned int currentBegin_ = (unsigned int)-1; // Start of the subsection of data currently being played back.
		unsigned int currentEnd_ = (unsigned int)-1; // End of the subsection of data currently being played back.

		std::vector<std::function<void(std::vector<float>&)>> fx_; // List of callbacks used to add arbitrary effects to the current subsection of data before being returned in Process_().
	};

	// Class responsible for servicing the audio.
	class AudioEngine
	{
	public:

		AudioEngine() = delete;
		/**
		* Constructs an instance of the AudioEngine.
		* 
		* @param sampleRate The sampling rate at which the audio data should be processed.
		* @param bufferSize Size of the monophonic audio buffer used to service the audio.
		*/
		AudioEngine(const unsigned int sampleRate, const unsigned int bufferSize);
		~AudioEngine();

		/**
		* Creates an instance of a Sound and returns a pointer to it. The instance of the AudioEngine on which this method is called is responsible for this Sound's lifetime.
		* 
		* @param path Path to a .wav file containing the audio data to be played by the new Sound.
		* @param assetManager Reference to the AssetManager that should be responsible for the lifetime of the loaded wav data.
		* @return Pointer to the newly created Sound.
		*/
		Sound* CreateSound(const char* path, AssetManager& assetManager);

		/**
		* Creates an instance of a Sound and returns a pointer to it. The instance of the AudioEngine on which this method is called is responsible for this Sound's lifetime.
		*
		* @param data Reference to the data of a monophonic audio signal that should be copied and played back by the new Sound.
		* @return Pointer to the newly created Sound.
		*/
		Sound* CreateSound(const std::vector<float>& data);

		/**
		* Creates an instance of a Sound from an existing Sound and returns a pointer to it. The instance of the AudioEngine on which this method is called is responsible for this Sound's lifetime.
		*
		* @param other Reference to another existing Sound that should be copied from.
		* @return Pointer to the newly created Sound.
		*/
		Sound* DuplicateSound(const Sound& other);

		/**
		* Calls Stop() on all Sounds in sounds_.
		*/
		void StopAll();

		/**
		* Destroys all sounds_ and clears the sounds_ vector.
		*/
		void DestroyAll();

		/**
		* To be called by the user at a rate of roughly samplingRate / bufferSize times per second. Processes all sounds to the backBuffer_.
		*/
		void ProcessAudio();

		const unsigned int sampleRate; // Sampling rate at which the audio should be serviced.
		const unsigned int bufferSize; // The size of the audio buffer used to service the audio.

	private:
		/**
		* Method called asynchronously by PortAudio at roughly samplingRate / bufferSize times per second. Swaps the frontBuffer and backBuffer and copies the contents of frontBuffer to the audiobuffer to be sent to the playback device.
		* 
		* @param input Unused. Pointer to an input device's audio buffer.
		* @param output Buffer to be sent for playback by the playback device.
		* @param frameCount Unused. PortAudio's stuff.
		* @param timeInfo Unused. PortAudio's stuff.
		* @param statusFlags Unused. PortAudio's stuff.
		* @param userData Pointer to the AudioEngine used to service the audio (this).
		* @return PortAudio's status code. Returns 0 if all went well, an error code otherwise.
		*/
		static int ServiceAudio_(const void* input, void* output,
								 unsigned long frameCount,
								 const PaStreamCallbackTimeInfo* timeInfo,
								 PaStreamCallbackFlags statusFlags,
								 void* userData);

		std::vector<float> frontBuffer_ = std::vector<float>(2 * (size_t)bufferSize, 0.0f); // Stereo buffer containing the processed audio data for the next playback device servicing.
		std::vector<float> backBuffer_ = std::vector<float>(2 * (size_t)bufferSize, 0.0f); // Stereo buffer containing the not-yet-processed audio data being worked upon currently.
		std::vector<std::function<void(std::vector<float>&)>> postProcessFx_; // List of global sound effects to apply to the frontBuffer before sending it over for playback. Currently unsused.

		PaStream* stream_ = nullptr; // PortAudio's stream to playback device.
		std::vector<Sound> sounds_; // List of Sounds managed by this AudioEngine.

		std::mutex m_; // Mutex used to synchronize the PortAudio's servicing thread to the SoundEngine's rendering thread. TODO: there should be two mutexes. TODO: make this class into an actual 2 layers deep pipeline rather than being a linear process with extra steps.
		bool processNextBuffer_ = true; // Mutex protected boolean used to tell whether the AudioEngine should be processing the next set of audio data.
	};
}