#pragma once

/*
Exercice for you to try and implement yourself a sine generator, a DFT and an IDFT.
Areas for you to write the code in have been marked with the comment "TODO".
*/

#include <iostream>
#include <algorithm>

#include "Application.h"

// Literals to configure the demo.
static constexpr const size_t SINE_SAMPLE_RATE = 8000; // Length of the signals. Corresponds to N (for time-domain signals) and K (for frequency-domain signals).
static constexpr const size_t SINE_FREQ = 440; // ISO standard "La" or "A".
static constexpr const float ANGLE_IN_RADS = -0.5f * MyMath::PI; // Clockwise 90° rotation.
static constexpr const float SYNTHESIZED_COMPLEX_MAGNITUDE = SINE_SAMPLE_RATE * 0.5f; // Magnitude that a frequency bin should have to rebuild the generated 440 Hz pure tone. The 1/2 factor is here because the spectrum should contain a peak at 440 Hz and 8000-440 Hz.

// The signals to render.
static std::vector<float> generatedTimeDomain(SINE_SAMPLE_RATE, 0.0f); // Pure 440 Hz tone generated via code.
static std::vector<std::complex<float>> generatedFreqDomain(SINE_SAMPLE_RATE, 0.0f); // Fourier transform of the same tone.
static std::vector<float> generatedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f); // The pure 440 Hz tone reconstructed from the fourier transform of the pure 440 Hz tone.
static std::vector<float> synthesizedTimeDomainFromDFT(SINE_SAMPLE_RATE, 0.0f); // Manually constructed fourier transform. We'll be synthesizing the 440 Hz pure tone with it.
static std::vector<std::complex<float>> synthesizedFreqDomain(SINE_SAMPLE_RATE, 0.0f); // Tone reconstructed from the manually constructed fourier transform.

/**
* Generates a single sample of a pure tone.
*
* @param n Index of the sample.
* @param sampleRate Number of samples to complete a single period with a frequency of 1 Hz.
* @param frequency The desired tone, in Hertz.
* @return Value of the signal at sample index n.
*/
float GenerateSine(const float n, const float sampleRate, const float frequency)
{
	// TODO: implement a function that returns the normalized value of a sine at the "n"'th index of a signal sampled at "sampleRate". The sine should have a frequency of "frequency".
	return 0.0f;
}

/**
* Implementation of euler's identity e^(i*x) = cos(x) + i*sin(x)
*/
std::complex<float> EulersFormula(const float x)
{
	// TODO: implement euler's identity.
	return std::complex<float>(0.0f, 0.0f);
}

/**
* Discrete Fourier Transform. Computes the frequency-domain representation of a time-domain signal. For more information about the DFT, see https://www.youtube.com/watch?v=ITnPS8HGqLo and https://www.youtube.com/watch?v=spUNpyF58BY
*
* @param out Output of the function, the frequency bins resulting from the DFT. Ensure out.size() is K before calling this function.
* @param x Input real-valued signal. x.size() defines N.
* @param K Number of frequency bins that constitute the out signal. Any value K < N results in spectral loss so for lossless transfomation use K = x.size().
*/
void DFT(std::vector<std::complex<float>>& out, const std::vector<float>& x, const unsigned int K, const bool printProgress = true)
{
	// Just a lambda to print the progress of the computation since it can be quite lengthy.
	const auto PrintProgress = [](const unsigned int k, const unsigned int K)->void
	{
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing DFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent > 99) percent = 0;
		}
	};

	const auto N = (unsigned int)x.size(); // Just an alias for the number of samples in the time-domain signal.
	std::vector<std::complex<float>>& y = out; // Just an alias for the output frequency-domain signal.

	std::fill(y.begin(), y.end(), std::complex<float>(0.0f, 0.0f)); // Set output signal to 0.

	// TODO: implement the DFT.

	if (printProgress) std::cout << "DFT done." << std::endl;
}

/**
* Inverse Discrete Fourier Transform. Computes the time-domain representation of a frequency-domain signal.
*
* @param out Output of the function, the real-valued time-domain signal. Ensure out.size() is N before calling this function.
* @param y Input frequency bins, the frequency-domain representation of a signal. y.size() defines K.
* @param N Number of samples in the output real-valued signal out. Compute this as samplingFrequency * durationOfRealValuedSignal (as floats!). Setting this incorrectly results in a change in pitch (look up "pitch scaling").
*/
void IDFT(std::vector<float>& out, const std::vector<std::complex<float>>& y, const unsigned int N, const bool printProgress = true)
{
	// Just a lambda to print the progress of the computation since it can be quite lengthy.
	const auto PrintProgress = [](const unsigned int n, const unsigned int N)->void
	{
		if (n % (N / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing IDFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent >= 100) percent = 0;
		}
	};

	const auto K = (unsigned int)y.size(); // Just an alias for the number of samples in the frequency-domain signal.
	std::vector<float>& x = out; // Just an alias for the output time-domain signal.

	std::fill(x.begin(), x.end(), 0.0f); // Set output signal to 0.

	// TODO: implement the IDFT.

	if (printProgress) std::cout << "IDFT done." << std::endl;
}

void GenerateSignals()
{
	// TODO: Generate a sine monophonic signal we'll be visualizing / hearing. generatedTimeDomain is the vector containing this signal.

	// TODO: Define a frequency-domain signal whose IDFT will yield the same signal as the one defined by generatedTimeDomain. synthesizedFreqDomain is the vector containing this signal.

	// Compute the DFT's and IDFT's of the generated / synthesized signals.
	DFT(generatedFreqDomain, generatedTimeDomain, SINE_SAMPLE_RATE);
	IDFT(generatedTimeDomainFromDFT, generatedFreqDomain, SINE_SAMPLE_RATE);
	IDFT(synthesizedTimeDomainFromDFT, synthesizedFreqDomain, SINE_SAMPLE_RATE);
}