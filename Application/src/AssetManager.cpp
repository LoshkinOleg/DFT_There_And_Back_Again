#include "AssetManager.h"

#include <fstream>

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

MyApp::AudioAsset::AudioAsset(const std::vector<float>& data): data(data) {}

std::vector<float> MyApp::AssetManager::LoadWav(const char* path, unsigned int& nrOfChannels, unsigned int& sampleRate)
{
	drwav_uint64 totalPCMFrameCount;
	float* pSampleData = drwav_open_file_and_read_pcm_frames_f32(path, &nrOfChannels, &sampleRate, &totalPCMFrameCount, NULL);
	if (!pSampleData) throw std::runtime_error(std::string("Failed to load wav file ") + path);

	std::vector<float> buff(totalPCMFrameCount * nrOfChannels);
	memcpy(&*buff.begin(), pSampleData, totalPCMFrameCount * nrOfChannels * sizeof(float));
	drwav_free(pSampleData, NULL);

	return buff;
}

bool MyApp::AssetManager::WriteWav(const std::vector<float>& data, const char* path, const unsigned int nrOfChannels, const unsigned int sampleRate)
{
	drwav wav;

	drwav_data_format format;
	format.container = drwav_container_riff;
	format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
	format.channels = nrOfChannels;
	format.sampleRate = sampleRate;
	format.bitsPerSample = 32;

	auto success = drwav_init_file_write(&wav, path, &format, NULL);
	if (!success)
	{
		drwav_uninit(&wav);
		return false;
	}

	const drwav_uint64 framesWritten = drwav_write_pcm_frames(&wav, data.size() / format.channels, data.data());
	if (framesWritten != data.size() / nrOfChannels)
	{
		drwav_uninit(&wav);
		return false;
	}

	return true;
}

bool MyApp::AssetManager::WriteCarr(const std::vector<float>& data, const char* path)
{
	std::ofstream file(path, std::ofstream::out);
	if (!file.is_open()) return false;

	file << "float arr[" << data.size() << "] = {\n";
	for (size_t i = 0; i < data.size(); ++i)
	{
		file << data[i] << ",\n";
	}
	file << "};";

	file.close();
	return true;
}

bool MyApp::AssetManager::WriteCarr(const std::vector<std::complex<float>>& data, const char* path)
{
	std::ofstream file(path, std::ofstream::out);
	if (!file.is_open()) return false;

	file << "std::complex<float> arr[" << data.size() << "] = {\n";
	for (size_t i = 0; i < data.size(); ++i)
	{
		file << "{" << data[i].real() << "," << data[i].imag() << "},\n";
	}
	file << "};";

	file.close();
	return true;
}