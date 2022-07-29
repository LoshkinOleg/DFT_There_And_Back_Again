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

std::vector<std::complex<float>> MyFDN::DFT(const std::vector<float>& input, const size_t sampleRate)
{
	// Taken from: https://www.youtube.com/watch?v=ITnPS8HGqLo
	// Adjusted with: https://www.youtube.com/watch?v=mkGsMWi_j4Q&list=WL&index=1

	/*
		Noted e^jx x engineering formulas.
	*/
	const auto eulersFormula = [](const float x)->std::complex<float>
	{
		return std::complex<float>(std::cosf(x), std::sinf(x));
	};
	const auto inverseEulerFormula = [](const float x)->std::complex<float>
	{
		return std::complex<float>(std::cosf(x), -std::sinf(x));
	};

	using complex = std::complex<float>;
	constexpr const float PI = 3.14159265;
	const size_t nrOfSamples = input.size(); // Noted N x math formulas.
	
	std::vector<float> frequencyBinsReal(sampleRate, 0.0f);
	std::vector<float> frequencyBinsImag(sampleRate, 0.0f);
	
	for (size_t k = 0; k < sampleRate; k++)
	{
		for (size_t n = 0; n < nrOfSamples; n++)
		{
			frequencyBinsReal[k] += input[n] * cosf(2.0f * PI * k * n / nrOfSamples);
			frequencyBinsImag[k] += -input[n] * sinf(2.0f * PI * k * n / nrOfSamples);
		}
	}

	std::vector<complex> output(nrOfSamples, std::complex<float>(0.0f, 0.0f));
	for (size_t i = 0; i < output.size(); i++)
	{
		output[i] = std::complex<float>(frequencyBinsReal[i], frequencyBinsImag[i]);
	}

	return output;
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

	return ComplexArray1D(out.begin(), out.begin() + out.size());
}

std::vector<float> MyFDN::IDFT(const std::vector<std::complex<float>>& input, const float duration)
{
	// Adapted from: https://www.geeksforgeeks.org/discrete-fourier-transform-and-its-inverse-using-c/

	/*
		Taken from: https://math.libretexts.org/Bookshelves/Analysis/Complex_Variables_with_Applications_(Orloff)/01%3A_Complex_Algebra_and_the_Complex_Plane/1.12%3A_Inverse_Euler_formula
	*/
	const auto inverseEulerFormula = [](const float x)->std::complex<float>
	{
		return std::complex<float>(std::cosf(x), -std::sinf(x));
	};

	constexpr const float PI = 3.14159265;
	const size_t sampleRate = input.size();
	const size_t nrOfSamples = sampleRate * (size_t)(duration);
	std::vector<float> signal(nrOfSamples, 0.0f);

	for (size_t n = 0; n < nrOfSamples; n++)
	{
		if (n % (nrOfSamples / 100) == 0)
		{
			static unsigned int percent = 0;

			std::cout << "Running IDFT: " << std::to_string(percent++) << std::endl;
		}

		float realSample = 0.0f;
		for (size_t k = 0; k < sampleRate; k++)
		{
			const auto bn = 2.0f * PI * k * n / nrOfSamples;
			// const auto xn = input[k] * inverseEulerFormula(bn) / (float)nrOfSamples;
			realSample += input[k].real() * std::cosf(bn) + input[k].imag() * std::sinf(bn);
		}
		signal[n] = realSample / nrOfSamples;
	}

	return signal;
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
