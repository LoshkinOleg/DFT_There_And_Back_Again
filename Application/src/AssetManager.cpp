#include "AssetManager.h"

#include <fstream>
#include <cstdlib>

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

bool StringToInt(const char* str, size_t& out)
{
	// Taken from: https://stackoverflow.com/questions/194465/how-to-parse-a-string-to-an-int-in-c
	char* end;
	int l;
	errno = 0;
	l = l = strtol(str, &end, 10);
	if (((errno == ERANGE && l == LONG_MAX) || l > INT_MAX) ||
		((errno == ERANGE && l == LONG_MIN) || l < INT_MIN) ||
		(*str == '\0' || *end != '\0'))
	{
		return false;
	}
	out = l;
	return true;
}

bool StringToFloat(const char* str, float& out)
{
	// Taken from: https://codereview.stackexchange.com/questions/199775/convert-string-to-double-and-check-for-errors

	char* endptr;

	const int errno_original = errno;
	errno = 0;

	float f = strtof(str, &endptr);
	int errno_my_strtof = errno;
	if (errno == 0) {
		errno = errno_original;
	}

	if (str == endptr) {
		return false;
	}

	while (isspace((unsigned char)*endptr)) {
		endptr++;
	}
	if (*endptr) {
		return false;
	}

	if (errno_my_strtof == ERANGE && fabs(f) == HUGE_VALF) {
		return false;
	}

	if (errno_my_strtof == ERANGE && fabs(f) <= FLT_MIN) {
		return false;
	}

	errno = errno_original;

	out = f;
	return true;
}

bool MyApp::AssetManager::ReadCarr(std::vector<float>& out, const char* path)
{
	std::string line;

	// Open file.
	std::ifstream file(path, std::ifstream::in);
	if (!file.is_open()) return false;

	// Read first line and ensure it's the right format.
	std::getline(file, line);
	assert(!line.empty() && "Failed to read first line.");
	assert(line.substr(0, 10) == "float arr[" && "Reading a file that does not begin with float arr[");

	// Find out the size of the c array.
	const size_t sizeIdxBegin = line.find('[') + 1;
	const size_t sizeIdxEnd = line.find(']') - 1;
	assert(sizeIdxBegin - 1 != std::string::npos && "Couldn't find string begin index of the size of the c array.");
	assert(sizeIdxEnd + 1 != std::string::npos && "Couldn't find string end index of the size of the c array.");
	
	size_t len;
	{
		const auto success = StringToInt(line.substr(sizeIdxBegin, sizeIdxEnd - sizeIdxBegin + 1).c_str(), len);
		assert(success && "Failed to c array size string as an integer.");
	}
	
	// Parse c array text.
	out.resize(len);
	bool success;
	for (size_t i = 0; i < len; i++)
	{
		std::getline(file, line);
		assert(!line.empty() && "Retireved an empty line from c array.");

		const auto realStr = line.substr(0, line.end() - line.begin() - 1);

		success = StringToFloat(realStr.c_str(), out[i]);
		assert(success && "Failed to parse float string.");
	}

	file.close();

	return true;
}

bool MyApp::AssetManager::ReadCarr(std::vector<std::complex<float>>& out, const char* path)
{
	std::string line;

	// Open file.
	std::ifstream file(path, std::ifstream::in);
	if (!file.is_open()) return false;

	// Read first line and ensure it's the right format.
	std::getline(file, line);
	assert(!line.empty() && "Failed to read first line.");
	assert(line.substr(0, 24) == "std::complex<float> arr[" && "Reading a file that does not begin with std::complex<float> arr[");

	// Find out the size of the c array.
	const size_t sizeIdxBegin = line.find('[') + 1;
	const size_t sizeIdxEnd = line.find(']') - 1;
	assert(sizeIdxBegin - 1 != std::string::npos && "Couldn't find string begin index of the size of the c array.");
	assert(sizeIdxEnd + 1 != std::string::npos && "Couldn't find string end index of the size of the c array.");

	size_t len;
	{
		const auto success = StringToInt(line.substr(sizeIdxBegin, sizeIdxEnd - sizeIdxBegin + 1).c_str(), len);
		assert(success && "Failed to c array size string as an integer.");
	}

	// Parse c array text.
	out.resize(len);
	size_t sepIdx;
	float real, imag;
	bool success;
	for (size_t i = 0; i < len; i++)
	{
		std::getline(file, line);
		assert(!line.empty() && "Retireved an empty line from c array.");

		sepIdx = line.find(',');
		assert(sepIdx != std::string::npos && "Couldn't find a separator in c array entry of complex numbers.");

		const auto realStr = line.substr(1, sepIdx - 1);
		const auto imagStr = line.substr(sepIdx + 1, line.size() - sepIdx - 3);

		success = StringToFloat(realStr.c_str(), real);
		assert(success && "Failed to parse complex number string.");

		success = StringToFloat(imagStr.c_str(), imag);
		assert(success && "Failed to parse complex number string.");

		out[i] = std::complex<float>(real, imag);
	}

	file.close();

	return true;
}