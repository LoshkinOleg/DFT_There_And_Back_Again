#pragma once

#include <vector>
#include <complex>

#include "MyUtilsTypedefs.h"
namespace MyUtils
{
	constexpr const float PI = 3.14159265f;

	/**
	* Sums two signals in-place. Signals must be of same length.
	* 
	* @param out First signal to sum. The second signal gets added to this one.
	* @param other Second signal to sum. Remains unchanged.
	*/
	void SumSignals(RealSignal& out, const RealSignal& other);

	/**
	* TODO: comment this
	*/
	void InterleaveSignals(RealSignal& out, const RealSignal& first, const RealSignal& second);

	/**
	* Sums two signals out-of-place. Signals must be of same length.
	*
	* @param first First signal to sum.
	* @param second Second signal to sum.
	*/
	RealSignal SumSignals(const RealSignal& first, const RealSignal& second);

	/**
	* Generates a real-valued, uniform distributed, white noise signal. Out-of-place, not for performance sensitive code, use a signal table if you need that. Uses a static instance of the standard's mt19937 pseudo-random number generator with a uniform distribution.
	* 
	* @param N Length of the signal to generate. Generated values are in range [-1.0f;1.0f] .
	* @param seed Seed for the mt19937 generator.
	* @return Dynamically allocated buffer of size N with the white noise signal.
	*/
	RealSignal WhiteNoise(const uint N, const size_t seed);

	/**
	* Generates a real-valued, normal distributed, white noise signal. Out-of-place, not for performance sensitive code, use a signal table if you need that. Uses a static instance of the standard's mt19937 pseudo-random number generator with a normal distribution.
	*
	* @param N Length of the signal to generate. Generated values are in range [-1.0f;1.0f] .
	* @param seed Seed for the mt19937 generator.
	* @return Dynamically allocated buffer of size N with the white noise signal.
	*/
	RealSignal GaussianWhiteNoise(const uint N, const size_t seed);

	/**
	* Padds a real-valued signal with 0's up to the next power of two, in-place.
	* 
	* @param buffer Signal to be padded.
	*/
	void PadToNextPowerOfTwo(RealSignal& buffer);

	/**
	* Padds a complex-valued signal with 0's up to the next power of two, in-place.
	*
	* @param buffer Signal to be padded.
	*/
	void PadToNextPowerOfTwo(ComplexSignal& buffer);

	/**
	* Checks whether a number is a power of two.
	* 
	* @param x Number to check.
	* @return True means x is a power of two. False means x is not a power of two.
	*/
	bool IsPowerOfTwo(const uint x);
}

