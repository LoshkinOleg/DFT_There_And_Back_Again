#include "FDN.h"

#include <cassert>
#include <random>
#include <cmath>
#include <iostream>

#include "simple_fft/fft_settings.h"
#include "simple_fft/fft.h"

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

std::vector<std::complex<float>> MyFDN::DFT(const std::vector<float>& input)
{
	// Taken from: https://www.youtube.com/watch?v=ITnPS8HGqLo

	using complex = std::complex<float>;
	constexpr const float PI = 3.14159265;
	const size_t N = input.size();
	const size_t K = N;
	std::vector<complex> returnVal;
	returnVal.reserve(K);
	
	std::vector<complex> X(input.size(), 0);
	for (size_t i = 0; i < N; i++)
	{
		X[i] = complex(input[i], 0.0f);
	}
	
	for (size_t k = 0; k < K; k++)
	{
		auto intSum = complex(0.0f, 0.0f); // Int stands for integral, not integer right?
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
	
			std::cout << "Running DFT: " << std::to_string(percent++) << std::endl;
		}
	
		for (size_t n = 0; n < N; n++)
		{
			const float real = std::cosf(((2.0f * PI) / N) * k * n);
			const float imaginary = -std::sinf(((2.0f * PI) / N) * k * n);
			const complex w(real, imaginary);
			intSum += X[n] * w;
		}
		returnVal.push_back(intSum);
	}
	
	return returnVal;
}

std::vector<std::complex<float>> MyFDN::SimpleFFT_FFT(const std::vector<float>& input)
{
	// TODO: slice input into vectors of power of two, process, stitch back together and return.

	typedef std::vector<real_type> RealArray1D;
	typedef std::vector<complex_type> ComplexArray1D;

	ComplexArray1D out(input.size());
	const char* errMsg = nullptr;

	if (!simple_fft::FFT<RealArray1D, ComplexArray1D>(input, out, input.size(), errMsg))
	{
		std::cerr << "Failed to compute FFT: " << errMsg << std::endl;
		throw;
	}

	return out;
}

std::vector<float> MyFDN::IDFT(const std::vector<std::complex<float>>& input)
{
	// Adapted from: https://www.geeksforgeeks.org/discrete-fourier-transform-and-its-inverse-using-c/

	constexpr const float PI = 3.14159265;
	const size_t len = input.size();
	const size_t N = len;
	std::vector<float> returnVal(len, 0.0f);

	for (size_t n = 0; n < N; n++)
	{
		if (n % (N / 100) == 0)
		{
			static unsigned int percent = 0;

			std::cout << "Running IDFT: " << std::to_string(percent++) << std::endl;
		}

		for (size_t k = 0; k < N; k++)
		{
			const float theta = (2.0f * PI * k * n) / N;
			returnVal[n] += input[k].real() * std::cosf(theta) + input[k].imag() * std::sinf(theta);
		}
		returnVal[n] /= N;
	}

	return returnVal;
}

std::vector<float> MyFDN::SimpleFFT_IFFT(const std::vector<std::complex<float>>& input)
{
	// TODO: slice input into vectors of power of two, process, stitch back together and return.

	typedef std::vector<real_type> RealArray1D;
	typedef std::vector<complex_type> ComplexArray1D;

	ComplexArray1D out(input.size());
	const char* errMsg = nullptr;

	if (!simple_fft::IFFT<ComplexArray1D>(input, out, input.size(), errMsg))
	{
		std::cerr << "Failed to compute FFT: " << errMsg << std::endl;
		throw;
	}

	std::vector<float> returnVal(input.size());
	for (size_t i = 0; i < input.size(); i++)
	{
		returnVal[i] = out[i].real();
	}

	return returnVal;
}
