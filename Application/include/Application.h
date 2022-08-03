#pragma once

#include "SdlManager.h"

#include <map>
#include <vector>
#include <portaudio.h>
#include "MyUtilsTypedefs.h"

namespace MyApp
{

	class IoManager
	{

	};

	typedef size_t SoundId; // Preferred way to refer to sounds.
	typedef size_t AssetId; // Preferred way to refer to assets.
	constexpr const SoundId INVALID_SOUND_INDEX = (size_t)-1;
	constexpr const AssetId INVALID_ASSET_ID = (size_t)-1;
	typedef float SampleFormat; // Defines the sample format used for audio processing.
	typedef std::vector<SampleFormat> AudioBuffer;

	class AudioEngine
	{
	public:

		AudioEngine() = delete; // AudioEngine references the IoManager, so no empty constructors.
		/**
		* Constructs an instance of the AudioEngine. The instance references an IoManager for loading audio assets.
		* Initializes PortAudio and open a stream to the default playback device.
		*/
		AudioEngine(const IoManager& ioManager, const MyUtils::uint sampleRate, const MyUtils::uint bufferSize);

		/**
		* Destroys the AudioEngine instance. Unloads all sound assets.
		* Shuts down PortAudio and closes stream to the default playback device.
		*/
		~AudioEngine();


		/**
		* Creates an instance of a sound asset and returns a handle to it.
		* 
		* @param path Relative path to the sound asset with extension.
		* @return SoundId for the newly created instance. Returns INVALID_SOUND_INDEX if the sound could not be created.
		*/
		SoundId CreateSound(const char* path);
		/**
		* Duplicates an existing instance of a sound asset and returns a handle to it.
		* Newly created sounds start unpaused but stopped.
		* 
		* @param id Sound instance to duplicate.
		* @return SoundId for the newly created instance. Returns INVALID_SOUND_INDEX if the sound could not be created.
		*/
		SoundId DuplicateSound(const SoundId id);
		/**
		* Destroys the instance of a sound asset specified by the SoundId.
		* Newly created sounds start unpaused but stopped.
		* 
		* @param id SoundId of the sound to be destroyed.
		* @return Returns true if the destruction was successful. False otherwise.
		*/
		bool DestroySound(const SoundId id);

		/**
		* Retrieves a constant reference to the audio buffer referenced by a sound.
		* 
		* @param id SoundId of the sound instance whose audio data you wish to retireve.
		* @return Constant audio buffer reference that is used by the sound.
		*/
		const AudioBuffer& GetSoundBuffer(const SoundId id) const;

		/**
		* Starts playing a sound. Note that you won't hear it if the sound has previously been paused.
		*/
		bool StartSound(const SoundId id) const;
		/**
		* Stops playing a sound. Does nothing is sound is already stopped. Does not modify a sound's paused state.
		*/
		bool StopSound(const SoundId id) const;
		/**
		* Returns whether a sound is playing or stopped.
		*/
		bool IsPlaying(const SoundId id) const;

		/**
		* Returns whether a sound is paused or not.
		*/
		bool IsPaused(const SoundId id) const;
		/**
		* Unpauses a sound. Has no effect on whether or not the sound is playing.
		*/
		bool UnpauseSound(const SoundId id) const;
		/**
		* Pauses a sound, meaning it is not processed upon servicing and it's audio buffer window isn't advanced. Has no effect on whether or not the sound is playing.
		*/
		bool PauseSound(const SoundId id) const;

		/**
		* Stops all sounds. Has no effect of the sounds' paused state.
		*/
		void StopAll() const;
		/**
		* Returns whether any sound is playing. Does not account for sound's paused states.
		*/
		bool AnyPlaying() const;

		/**
		* Swaps front and back buffers and pushes the front buffer into the playback device's stream. Blocks the thread until the audio processing thread has finished processing the backbuffer and the individual sounds.
		*/
		void ServiceAudio();

		/**
		* Asynchronously processes the sound effects on each sound and adds them to the backbuffer. Then applies post-process effects to the backbuffer.
		*/
		void ProcessAudio();

		const MyUtils::uint sampleRate; // Sampling rate used by the engine.
		const MyUtils::uint bufferSize; // Buffer size used by the engine.

	private:

		/**
		* A sound is an abstract class that allows subclasses to play their audio data.
		*/
		class ASound_
		{
		public:
			/**
			* A sound is considered not to be playing when currentBegin_ >= data.size().
			*/
			virtual bool IsPlaying() const = 0;

			/**
			* Copies contents of data into the passed buffer and applies stored effects to it.
			*/
			virtual void ProcessFx(AudioBuffer& out) = 0;

			void AddFx(const std::function<void(AudioBuffer&)>& fx)
			{
				fx_.push_back(fx);
			}

			bool looping = false; // Buffer window' indices don't wrap back to start unless looping is set to true.
			bool paused = false; // Sound isn't rendered and buffer window isn't advanced if it's paused.

		protected:
			MyUtils::uint currentBegin_ = MyUtils::uint(-1); // Start index of the buffer window being played.
			MyUtils::uint currentEnd_ = MyUtils::uint(-1); // End index of the buffer window being played.

			std::vector<std::function<void(AudioBuffer& out)>> fx_; // Vector of signal processing callbacks to apply in sequence before adding it to the common output.
		};

		/**
		* Represents a single instance of a read-only sound loaded from disk, referred to as a clip. Reads audio data from a common audio data buffer.
		*/
		class ImmutableSound_ final : public ASound_
		{
		public:
			ImmutableSound_() = delete; // A clip should have a reference to an audio asset, so no empty constructors.

			/**
			* Creates a new instance of a clip. If the clip asset hasn't been loaded yet, IoManager loads it. It is it loaded already, uses the exising asset instead.
			* Increments asset usage counter.
			* 
			* @param ioManager Reference to the asset manager.
			* @param path Relative path to audio asset, including the extension.
			*/
			ImmutableSound_(IoManager& ioManager, const char* path);

			/**
			* Destroy instance of the clip.
			* Decrements asset usage counter, IoManager unloads the asset if it isn't used by any other clip.
			*/
			~ImmutableSound_();

			/**
			* A clip is considered not to be playing when currentBegin_ >= data.size().
			*/
			bool IsPlaying() const override;

			void ProcessFx(AudioBuffer& out) override;

			const AudioBuffer& data; // Constant reference to an audio asset.

		private:
			std::function<void(void)> onDestroy_; // Callback to decrement audio asset usage counter upon destruction.
		};

		/**
		* A sound that can be modified, unlike a clip.
		*/
		class MutableSound_ final: public ASound_
		{
		public:
			/**
			* A sound is considered not to be playing when currentBegin_ >= data.size().
			*/
			bool IsPlaying() const override;

			void ProcessFx(AudioBuffer& out) override;

			AudioBuffer data; // An audio data buffer that can be modified.
		};

		const IoManager& io_; // Constant reference to an asset manager.

		AudioBuffer frontBuffer_ = AudioBuffer(bufferSize, 0.0f); // Processed signal to be sent over the stream on next audio servicing.
		AudioBuffer backBuffer_ = AudioBuffer(bufferSize, 0.0f); // Signal currently processing. Is swapped with frontBuffer_ in ServiceAudio().
		std::vector<std::function<void(AudioBuffer& out)>> postProcessFx_; // Vector of signal processing callbacks to apply in sequence before presenting the rendered signal.

		PaStream* stream_ = nullptr; // Stream to playback device. AudioEngine is responsible for the stream's lifetime.
		std::vector<ImmutableSound_> clips_; // Vector of clips instances. AudioEngine is responsible for the instances' lifetime.
		std::vector<MutableSound_> sounds_; // Vector of mutable sound instances. AudioEngine is responsible for the sounds' lifetime.
	};

	class Application
	{
		Application();
	private:
		SdlManager sdl_;
		IoManager io_;
		AudioEngine audio_;
	};
}