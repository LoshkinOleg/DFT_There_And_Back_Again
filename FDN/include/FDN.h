#pragma once

#include <vector>

namespace MyFDN
{
	void SumSignals(std::vector<float>& out, const std::vector<float> other);

	void SimpleFDN(std::vector<float>& output, const std::vector<float>& input, const size_t bufferSize, const float attenuationFactor);

	void WhiteNoise(std::vector<float>& out, const size_t seed);

	void GaussianWhiteNoise(std::vector<float>& out, const size_t seed);

	void Radix2DITCooleyTukeyAlgoritm(std::vector<float>& output, const std::vector<float>& input, const size_t stride, const size_t N);
}