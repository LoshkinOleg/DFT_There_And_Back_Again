#pragma once

#include <vector>
#include <complex>

namespace MyApp
{
	/**
	* Represents a (TODO)unique audio clip loaded from disk or created by a user.
	*/
	class AudioAsset
	{
	public:
		AudioAsset() = delete;
		/**
		* Constructs an AudioAsset.
		* 
		* @param data Monophonic signal whose data is to be copied.
		*/
		AudioAsset(const std::vector<float>& data);

		const std::vector<float> data; // Holds a monophonic signal.
	};

	/**
	* Responsible for loading and unloading assets. For now, only handles wav files.
	* TODO: add hashing to avoid loading the same file multiple times.
	*/
	class AssetManager
	{
	public:
		/**
		* Loads and returns the wav data of a wav file.
		* 
		* @param path Relative path of the wav file.
		* @param nrOfChannels Output for reading of how many channels is composed the wav file.
		* @param sampleRate Output for reading at what sample rate the wav file has been encoded at.
		* @return A new vector containing the audio data.
		*/
		static std::vector<float> LoadWav(const char* path, unsigned int& nrOfChannels, unsigned int& sampleRate);

		/**
		* Writes a wav file to disk.
		* 
		* @param data The time-domain signal to write.
		* @param path The relative path the the file where the array should be written.
		* @param nrOfChannels The number of channels composing the data signal. Note that only interleaved stereo is supported.
		* @param sampleRate The sampling rate of the signal.
		* @return Whether the writing of the data has been successful.
		*/
		static bool WriteWav(const std::vector<float>& data, const char* path, const unsigned int nrOfChannels, const unsigned int sampleRate);

		/**
		* Writes a C array of floats to a text file on disk.
		*
		* @param data The time-domain signal to write.
		* @param path The relative path the the file where the array should be written.
		* @return Whether the writing of the data has been successful.
		*/
		static bool WriteCarr(const std::vector<float>& data, const char* path);

		/**
		* Writes a C array of std::complex<float> to a text file on disk.
		*
		* @param data The frequency-domain signal to write.
		* @param path The relative path the the file where the array should be written.
		* @return Whether the writing of the data has been successful.
		*/
		static bool WriteCarr(const std::vector<std::complex<float>>& data, const char* path);

		/**
		* Reads a C array of floats from a text file on disk.
		*
		* @param out Reference to the buffer that will store the data. Is resized by the method.
		* @param path The relative path the the file where the array should be written.
		* @return Whether the reading of the data has been successful.
		*/
		static bool ReadCarr(std::vector<float>& out, const char* path);

		/**
		* Reads a C array of std::complex<float> from a text file on disk.
		*
		* @param out Reference to the buffer that will store the data. Is resized by the method.
		* @param path The relative path the the file where the array should be written.
		* @return Whether the reading of the data has been successful.
		*/
		static bool ReadCarr(std::vector<std::complex<float>>& out, const char* path);

	private:
		std::vector<AudioAsset> audioAssets_; // All audio clips currently loaded.
	};
}