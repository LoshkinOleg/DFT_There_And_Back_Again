#include "FDN.h"

#include <cassert>
#include <random>
#include <cmath>


void MyFDN::SumSignals(std::vector<float>& out, const std::vector<float> other)
{
	assert(out.size() == other.size() && "Mismatching buffer sizes.");

	for (size_t i = 0; i < other.size(); i++)
	{
		out[i] += other[i];
	}
}

void MyFDN::SimpleFDN(std::vector<float>& output, const std::vector<float>& input, const size_t bufferSize, const float attenuationFactor)
{
	static std::vector<float> lastResult = std::vector<float>(bufferSize, 0.0f);

	assert(output.size() == input.size() && input.size() == bufferSize && "Mismatching buffer sizes.");

	for (size_t i = 0; i < input.size(); i++)
	{
		lastResult[i] = input[i] + lastResult[i] * attenuationFactor;
		output[i] = lastResult[i];
	}
}

void MyFDN::WhiteNoise(std::vector<float>& out, const size_t seed)
{
	static auto e = std::default_random_engine((unsigned int)seed);
	static auto d = std::uniform_real_distribution<float>(-1.0f, 1.0f);

	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = d(e);
	}
}

void MyFDN::GaussianWhiteNoise(std::vector<float>& out, const size_t seed)
{
	static auto e = std::default_random_engine((unsigned int)seed);
	static auto d = std::normal_distribution<float>(0.0f, 1.0f);

	for (size_t i = 0; i < out.size(); i++)
	{
		out[i] = d(e);
	}
}

void MyFDN::Radix2DITCooleyTukeyAlgoritm(std::vector<float>& output, const std::vector<float>& input, const size_t stride, const size_t N)
{
	/*
	Taken from: https://en.wikipedia.org/wiki/Cooley%E2%80%93Tukey_FFT_algorithm
	
	X0,...,N−1 ← ditfft2(x, N, s):						DFT of (x0, xs, x2s, ..., x(N-1)s):
    if N = 1 then
        X0 ← x0											trivial size-1 DFT base case
    else
        X0,...,N/2−1 ← ditfft2(x, N/2, 2s)				DFT of (x0, x2s, x4s, ..., x(N-2)s)
        XN/2,...,N−1 ← ditfft2(x+s, N/2, 2s)			DFT of (xs, xs+2s, xs+4s, ..., x(N-1)s)
        for k = 0 to N/2−1 do							combine DFTs of two halves into full DFT:
            p ← Xk
            q ← exp(−2πi/N k) Xk+N/2
            Xk ← p + q 
            Xk+N/2 ← p − q
        end for
    end if
	*/

	assert(std::ceilf(std::log2f(N)) == std::floorf(std::log2f(N)) && "N is not a power of two.");

	if (input.size() == 1)
	{
		output = input;
		return;
	}
	else
	{
		std::vector<float> radix0(input.size() / 2);
		std::vector<float> radix1(input.size() / 2);
		Radix2DITCooleyTukeyAlgoritm(radix0, input, 2 * stride, N / 2);
		Radix2DITCooleyTukeyAlgoritm(radix1, std::vector<float>(input.begin() + stride, input.end()), 2 * stride, N / 2);
		for (size_t k = 0; k < (N / 2) - 1; k++)
		{

		}
	}
}
