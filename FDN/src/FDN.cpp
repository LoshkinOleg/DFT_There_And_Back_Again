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

std::vector<std::complex<float>> MyFDN::DFT(const std::vector<float>& x, const size_t K)
{
	// DFT -> IDFT results in a signal that has only 1/4 of the specter of the input signal. Look into it.

	const auto PrintProgress = [](const size_t k, const size_t K)->void
	{
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing DFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent >= 100) percent = 0;
		}
	};

	using complex = std::complex<float>;
	constexpr const float PI = 3.14159265;
	const size_t N = x.size();
	
	std::vector<complex> y(2 * K, complex(0.0f, 0.0f)); // 2 * K To prevent spectral loss.
	
	for (size_t k = 0; k < 2 * K; k++)
	{
		PrintProgress(k, 2 * K);

		for (size_t n = 0; n < N; n++)
		{
			y[k] += x[n] * InverseEulersFormula(2.0f * PI * k * n / N);
		}
		y[k] *= 2.0f; // Account for the -6db volume loss due to 2*K.
	}

	return y;
}

std::vector<std::complex<float>> MyFDN::SimpleFFT_FFT(std::vector<float>& input)
{
	if (!IsPowerOfTwo(input.size())) PadToNearestPowerOfTwo(input);

	typedef std::vector<real_type> RealArray1D;
	typedef std::vector<complex_type> ComplexArray1D;

	ComplexArray1D out(input.size());
	const char* errMsg = nullptr;

	if (!simple_fft::FFT<RealArray1D, ComplexArray1D>(input, out, input.size(), errMsg))
	{
		std::cerr << "Failed to compute FFT: " << errMsg << std::endl;
		throw;
	}

	return ComplexArray1D(out.begin(), out.begin() + out.size());
}

std::vector<float> MyFDN::IDFT(const std::vector<std::complex<float>>& y, const float N)
{
	// DFT -> IDFT results in a signal that has only 1/4 of the specter of the input signal. Look into it.

	const auto PrintProgress = [](const size_t k, const size_t K)->void
	{
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing IDFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent >= 100) percent = 0;
		}
	};

	using complex = std::complex<float>;
	constexpr const float PI = 3.14159265;
	const size_t K = y.size();

	std::vector<float> x(N, 0.0f);

	for (size_t n = 0; n < N; n++)
	{
		PrintProgress(n, N);

		for (size_t k = 0; k < K; k++)
		{
			x[n] += (y[k] * EulersFormula(2.0f * PI * k * n / N)).real();
		}
		x[n] /= N;
	}

	return x;
}

std::vector<float> MyFDN::SimpleFFT_IFFT(std::vector<std::complex<float>>& input)
{
	if (!IsPowerOfTwo(input.size())) PadToNearestPowerOfTwo(input);

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

void MyFDN::PadToNearestPowerOfTwo(std::vector<std::complex<float>>& buffer)
{
	const size_t currentSize = buffer.size();

	// Taken from https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	size_t newSize = currentSize;
	newSize--;
	newSize |= newSize >> 1;
	newSize |= newSize >> 2;
	newSize |= newSize >> 4;
	newSize |= newSize >> 8;
	newSize |= newSize >> 16;
	newSize |= newSize >> 32;
	newSize++;

	const size_t nrOfNewElements = newSize - currentSize;

	buffer.insert(buffer.end(), nrOfNewElements, std::complex<float>(0.0f, 0.0f));
}

void MyFDN::PadToNearestPowerOfTwo(std::vector<float>& buffer)
{
	const size_t currentSize = buffer.size();

	// Taken from https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	size_t newSize = currentSize;
	newSize--;
	newSize |= newSize >> 1;
	newSize |= newSize >> 2;
	newSize |= newSize >> 4;
	newSize |= newSize >> 8;
	newSize |= newSize >> 16;
	newSize |= newSize >> 32;
	newSize++;

	const size_t nrOfNewElements = newSize - currentSize;

	buffer.insert(buffer.end(), nrOfNewElements, 0.0f);
}

std::complex<float> MyFDN::EulersFormula(const float theta)
{
	// e^ix = cos(x) + i sin(x)
	return std::complex<float>(std::cosf(theta), std::sinf(theta)); // Clockwise rotation.
}

std::complex<float> MyFDN::InverseEulersFormula(const float theta)
{
	// e^-ix = cos(x) - i sin(x)
	return std::complex<float>(std::cosf(theta), -std::sinf(theta)); // Counterclockwise rotation.
}

bool MyFDN::IsPowerOfTwo(const size_t x)
{
	// Taken from: https://stackoverflow.com/questions/108318/how-can-i-test-whether-a-number-is-a-power-of-2
	return (x & (x - 1)) == 0;
}
