#pragma once

#include <vector>
#include <complex>

namespace MyFDN
{
	void SumSignals(std::vector<float>& out, const std::vector<float> other);

	void SimpleFDN(std::vector<float>& output, const std::vector<float>& input, const size_t bufferSize, const float attenuationFactor);

	void WhiteNoise(std::vector<float>& out, const size_t seed);

	void GaussianWhiteNoise(std::vector<float>& out, const size_t seed);

	std::vector<std::complex<float>> DFT(const std::vector<float>& input);

	std::vector<std::complex<float>> SimpleFFT_FFT(const std::vector<float>& input);

	std::vector<float> IDFT(const std::vector<std::complex<float>>& input);

	std::vector<float> SimpleFFT_IFFT(const std::vector<std::complex<float>>& input);
}