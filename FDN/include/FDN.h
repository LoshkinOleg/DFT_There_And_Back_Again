#pragma once

#include <vector>
#include <complex>

namespace MyFDN
{
	void SumSignals(std::vector<float>& out, const std::vector<float> other);

	void FeedbackCombFilter(std::vector<float>& output, const std::vector<float>& input, const size_t bufferSize, const float attenuationFactor);

	void FourthOrderSISO_FDN(std::vector<float>& output, const std::vector<float>& input);

	void WhiteNoise(std::vector<float>& out, const size_t seed);

	void GaussianWhiteNoise(std::vector<float>& out, const size_t seed);

	std::vector<std::complex<float>> DFT(const std::vector<float>& x, const size_t K);

	std::vector<float> IDFT(const std::vector<std::complex<float>>& y, const float N);

	void PadToNearestPowerOfTwo(std::vector<std::complex<float>>& buffer);

	void PadToNearestPowerOfTwo(std::vector<float>& buffer);

	std::complex<float> EulersFormula(const float theta);
	
	std::complex<float> InverseEulersFormula(const float theta);
	
	bool IsPowerOfTwo(const size_t x);

	std::vector<std::complex<float>> FFT(const std::vector<std::complex<float>>& x);
}