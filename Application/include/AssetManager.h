#pragma once

#include <vector>
#include <complex>

namespace MyApp
{
	class AudioAsset
	{
	public:
		AudioAsset() = delete;
		AudioAsset(const std::vector<float>& data);

		const std::vector<float> data;
	};

	class AssetManager
	{
	public:
		static std::vector<float> LoadWav(const char* path, unsigned int& nrOfChannels, unsigned int& sampleRate);
		static bool WriteWav(const std::vector<float>& data, const char* path, const unsigned int nrOfChannels, const unsigned int sampleRate);
		static bool WriteCarr(const std::vector<float>& data, const char* path);
		static bool WriteCarr(const std::vector<std::complex<float>>& data, const char* path);
		static bool ReadCarr(std::vector<float>& out, const char* path);
		static bool ReadCarr(std::vector<std::complex<float>>& out, const char* path);

	private:
		std::vector<AudioAsset> audioAssets_;
	};
}