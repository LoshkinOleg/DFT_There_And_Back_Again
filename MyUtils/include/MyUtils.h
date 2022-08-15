#pragma once

#include <vector>
#include <complex>

namespace MyUtils
{
	/**
	* Sums two signals in-place. Signals must be of same length.
	* 
	* @param out First signal to sum. The second signal gets added to this one.
	* @param other Second signal to sum. Remains unchanged.
	*/
	void SumSignals(std::vector<float>& out, const std::vector<float>& other);

	/**
	* Sums two signals out-of-place. Signals must be of same length.
	*
	* @param first First signal to sum.
	* @param second Second signal to sum.
	*/
	std::vector<float> SumSignals(const std::vector<float>& first, const std::vector<float>& second);

	/**
	* Interleaves two monophonic signals into one interleaved stereophonic signal. See this blogpost for an illustration of an interleaved stereo signal: https://dylanmeeus.github.io/posts/audio-from-scratch-pt4/
	* Make sure that out.size() is (first.size() + second.size()) and that first.size() is equal to second.size().
	* 
	* @param out The output signal.
	* @param first First (usually the left channel) signal to interleave.
	* @param second Second (usually the right channel) signal to interleave.
	*/
	void InterleaveSignals(std::vector<float>& out, const std::vector<float>& first, const std::vector<float>& second);

	/**
	* Generates a real-valued, uniform distributed, white noise signal. Out-of-place, not for performance sensitive code, use a signal table if you need that. Uses a static instance of the standard's mt19937 pseudo-random number generator with a uniform distribution.
	* 
	* @param N Length of the signal to generate. Generated values are in range [-1.0f;1.0f] .
	* @param seed Seed for the mt19937 generator.
	* @return Dynamically allocated buffer of size N with the white noise signal.
	*/
	std::vector<float> WhiteNoise(const unsigned int N, const size_t seed);

	/**
	* Generates a real-valued, normal distributed, white noise signal. Out-of-place, not for performance sensitive code, use a signal table if you need that. Uses a static instance of the standard's mt19937 pseudo-random number generator with a normal distribution.
	*
	* @param N Length of the signal to generate. Generated values are in range [-1.0f;1.0f] .
	* @param seed Seed for the mt19937 generator.
	* @return Dynamically allocated buffer of size N with the white noise signal.
	*/
	std::vector<float> GaussianWhiteNoise(const unsigned int N, const size_t seed);

	/**
	* Pads a real-valued signal with 0's up to the next power of two, in-place.
	* 
	* @param buffer Signal to be padded.
	*/
	void PadToNextPowerOfTwo(std::vector<float>& buffer);

	/**
	* Pads a complex-valued signal with 0's up to the next power of two, in-place.
	*
	* @param buffer Signal to be padded.
	*/
	void PadToNextPowerOfTwo(std::vector<std::complex<float>>& buffer);

	/**
	* Checks whether a number is a power of two.
	* 
	* @param x Number to check.
	* @return True means x is a power of two. False means x is not a power of two.
	*/
	bool IsPowerOfTwo(const unsigned int x);
}

