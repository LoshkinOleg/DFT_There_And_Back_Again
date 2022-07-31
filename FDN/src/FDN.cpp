#include "FDN.h"

#include <cassert>
#include <random>
#include <cmath>
#include <iostream>
#include <array>

void MyFDN::SumSignals(std::vector<float>& out, const std::vector<float> other)
{
	assert(out.size() == other.size() && "Mismatching buffer sizes.");

	for (size_t i = 0; i < other.size(); i++)
	{
		out[i] += other[i];
	}
}

void MyFDN::FeedbackCombFilter(std::vector<float>& output, const std::vector<float>& input, const size_t bufferSize, const float attenuationFactor)
{
	// https://www.dsprelated.com/freebooks/pasp/Feedback_Comb_Filters.html

	static std::vector<float> lastResult = std::vector<float>(bufferSize, 0.0f);

	assert(output.size() == input.size() && input.size() == bufferSize && "Mismatching buffer sizes.");

	for (size_t i = 0; i < input.size(); i++)
	{
		lastResult[i] = input[i] + lastResult[i] * attenuationFactor;
		output[i] = lastResult[i];
	}
}

void MyFDN::FourthOrderSISO_FDN(std::vector<float>& output, const std::vector<float>& input)
{
	// https://www.dsprelated.com/freebooks/pasp/Feedback_Delay_Networks_FDN.html
	// https://www.dsprelated.com/freebooks/pasp/Choice_Lossless_Feedback_Matrix.html

	assert(output.size() == input.size() && "Mismatching buffer sizes.");

	using vector = std::array<float, 4>;
	using matrix = std::array<vector, 4>;

	constexpr const float b0 = 1.0f / 4.0f;
	constexpr const float b1 = 1.0f / 4.0f;
	constexpr const float b2 = 1.0f / 4.0f;
	constexpr const float b3 = 1.0f / 4.0f;

	constexpr const float c0 = 1.0f;
	constexpr const float c1 = 1.0f;
	constexpr const float c2 = 1.0f;
	constexpr const float c3 = 1.0f;

	constexpr const float g0 = 0.8f;
	constexpr const float g1 = 0.8f;
	constexpr const float g2 = 0.8f;
	constexpr const float g3 = 0.8f;

	// TODO: check that this is the right matrix orientation.
	constexpr const matrix HADAMARD_MATRIX =
	{ {
		{ 0.5f,  0.5f,  0.5f, 0.5f},
		{-0.5f,  0.5f, -0.5f, 0.5f},
		{-0.5f, -0.5f,  0.5f, 0.5f},
		{ 0.5f, -0.5f, -0.5f, 0.5f}
	} };

	static std::vector<float> delayed0(input.size(), 0.0f);
	static std::vector<float> delayed1(input.size(), 0.0f);
	static std::vector<float> delayed2(input.size(), 0.0f);
	static std::vector<float> delayed3(input.size(), 0.0f);

	static std::vector<float> input0(input.size(), 0.0f);
	static std::vector<float> input1(input.size(), 0.0f);
	static std::vector<float> input2(input.size(), 0.0f);
	static std::vector<float> input3(input.size(), 0.0f);

	std::copy(input.begin(), input.end(), input0.begin());
	std::copy(input.begin(), input.end(), input1.begin());
	std::copy(input.begin(), input.end(), input2.begin());
	std::copy(input.begin(), input.end(), input3.begin());

	for (size_t i = 0; i < input.size(); i++)
	{
		input0[i] *= b0;
		input1[i] *= b1;
		input2[i] *= b2;
		input3[i] *= b3;
	}

	for (size_t i = 0; i < input.size(); i++)
	{
		input0[i] += delayed0[i];
		input1[i] += delayed1[i];
		input2[i] += delayed2[i];
		input3[i] += delayed3[i];
	}

	std::copy(input0.begin(), input0.end(), delayed0.begin());
	std::copy(input1.begin(), input1.end(), delayed1.begin());
	std::copy(input2.begin(), input2.end(), delayed2.begin());
	std::copy(input3.begin(), input3.end(), delayed3.begin());

	for (size_t i = 0; i < input.size(); i++)
	{
		input0[i] *= c0;
		input1[i] *= c1;
		input2[i] *= c2;
		input3[i] *= c3;
	}

	for (size_t i = 0; i < input.size(); i++)
	{
		output[i] = input0[i] + input1[i] + input2[i] + input3[i];
	}

	// TODO: check that this is the right matrix orientation.
	const auto VectorMatrixMultiplication = [](const vector& v, const matrix& m)->vector
	{
		return
		{
			m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2] + m[0][3] * v[3],
			m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2] + m[1][3] * v[3],
			m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2] + m[2][3] * v[3],
			m[3][0] * v[0] + m[3][1] * v[1] + m[3][2] * v[2] + m[3][3] * v[3]
		};
	};

	for (size_t i = 0; i < input.size(); i++)
	{
		const auto mixed = VectorMatrixMultiplication
		(
			{delayed0[i], delayed1[i], delayed2[i], delayed3[i]},
			HADAMARD_MATRIX
		);
		delayed0[i] = mixed[0];
		delayed1[i] = mixed[1];
		delayed2[i] = mixed[2];
		delayed3[i] = mixed[3];
	}

	for (size_t i = 0; i < input.size(); i++)
	{
		delayed0[i] *= g0;
		delayed1[i] *= g1;
		delayed2[i] *= g2;
		delayed3[i] *= g3;
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

std::vector<float> MyFDN::IDFT(const std::vector<std::complex<float>>& y, const float N)
{
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

std::vector<std::complex<float>> MyFDN::FFT(const std::vector<std::complex<float>>& x)
{
	// Taken from: https://www.youtube.com/watch?v=h7apO7q16V0

	using complex = std::complex<float>;
	using complex_vector = std::vector<complex>;
	constexpr const float PI = 3.14159265;

	const size_t N = x.size();

	if (N == 1) return x;

	assert(IsPowerOfTwo(x.size()) && "Please pass an input with length of power of two.");

	complex_vector even(N / 2, 0.0f);
	complex_vector odd(N / 2, 0.0f);
	for (size_t i = 0; i < N / 2; i++)
	{
		even[i] = x[2 * i];
		odd[i] = x[2 * i + 1];
	}

	const auto evenTransform = FFT(even);
	const auto oddTransform = FFT(odd);

	complex_vector y(N, complex(0.0f, 0.0f));

	for (size_t i = 0; i < N / 2; i++)
	{
		const auto w = EulersFormula(2.0f * PI * i * i / N);

		y[i] = even[i] + w * odd[i];
		y[i + N / 2] = even[i] - w * odd[i];
	}

	return y;
}
