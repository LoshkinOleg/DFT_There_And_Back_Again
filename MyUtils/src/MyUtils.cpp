#include "MyUtils.h"

#include <cassert>
#include <random>

void MyUtils::SumSignals(MyUtils::RealSignal& out, const MyUtils::RealSignal& other)
{
	assert(out.size() == other.size() && "Mismatching buffer sizes.");

	const size_t len = other.size();
	for (size_t i = 0; i < len; ++i)
	{
		out[i] += other[i];
	}
}

void MyUtils::InterleaveSignals(RealSignal& out, const RealSignal& first, const RealSignal& second)
{
	assert(2 * out.size() == first.size() + second.size() && first.size() == second.size() && "Mismatching buffer sizes.");

	const size_t len = first.size();
	for (size_t i = 0; i < len; ++i)
	{
		out[2 * i] = first[i];
		out[2 * i + 1] = second[i];
	}
}

std::vector<float> MyUtils::SumSignals(const MyUtils::RealSignal& first, const MyUtils::RealSignal& second)
{
	assert(first.size() == second.size() && "Mismatching buffer sizes.");

	const size_t len = first.size();
	std::vector<float> returnVal(len);
	for (size_t i = 0; i < len; ++i)
	{
		returnVal[i] = first[i] + second[i];
	}
	return returnVal;
}

std::vector<float> MyUtils::WhiteNoise(const MyUtils::uint N, const size_t seed)
{
	static auto e = std::default_random_engine((unsigned int)seed);
	static auto d = std::uniform_real_distribution<float>(-1.0f, 1.0f);

	std::vector<float> returnVal(N);
	for (size_t i = 0; i < N; ++i)
	{
		returnVal[i] = d(e);
	}
	return returnVal;
}

std::vector<float> MyUtils::GaussianWhiteNoise(const MyUtils::uint N, const size_t seed)
{
	static auto e = std::default_random_engine((unsigned int)seed);
	static auto d = std::normal_distribution<float>(0.0f, 1.0f);

	std::vector<float> returnVal(N);
	for (size_t i = 0; i < N; ++i)
	{
		returnVal[i] = d(e);
	}
	return returnVal;
}

void MyUtils::PadToNextPowerOfTwo(MyUtils::RealSignal& buffer)
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

void MyUtils::PadToNextPowerOfTwo(MyUtils::ComplexSignal& buffer)
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

MyUtils::Complex MyUtils::EulersFormula(const float x)
{
	return std::complex<float>(std::cosf(x), std::sinf(x));
}

MyUtils::Complex MyUtils::InverseEulersFormula(const float x)
{
	return std::complex<float>(std::cosf(x), -std::sinf(x));
}

bool MyUtils::IsPowerOfTwo(const MyUtils::uint x)
{
	// Taken from: https://stackoverflow.com/questions/108318/how-can-i-test-whether-a-number-is-a-power-of-2
	return (x & (x - 1)) == 0;
}
