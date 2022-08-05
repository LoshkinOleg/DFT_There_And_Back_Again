#pragma once

#include <vector>
#include <complex>

#include "MyUtilsTypedefs.h"
namespace MyDFT
{
	/**
	* Discrete Fourier Transform. Computes the frequency-domain representation of a time-domain signal. In-place version, meaning a constant memory footprint regardless of input. Suitable for performance-sensitive code.
	* 
	* @param out Output of the function, the frequency bins resulting from the DFT. Ensure out.size() is K before calling this function.
	* @param x Input real-valued signal. x.size() defines N.
	* @param K Number of frequency bins that constitute the out signal. Any value K < N results in spectral loss so for lossless transfomation use K = x.size().
	*/
	void DFT(MyUtils::ComplexSignal& out, const MyUtils::RealSignal& x, const MyUtils::uint K, const bool printProgress = true);

	/**
	* Discrete Fourier Transform. Computes the frequency-domain representation of a time-domain signal. Out-of-place version, meaning there is a dynamic memory allocation inside the function. Suitable for testing purposes.
	*
	* @param x Input real-valued signal. x.size() defines N.
	* @param K Number of frequency bins that constitute the out signal. Any value K < N results in spectral loss so for lossless transfomation use K = x.size(). 
	* @param printProgress Whether to putput % progression of the operation to standard output. Just a piece of mind since simple DFT takes a long time to compute.
	* @return Output of the function, the frequency bins resulting from the DFT. ComplexSignal of size K.
	*/
	MyUtils::ComplexSignal DFT(const MyUtils::RealSignal& x, const MyUtils::uint K, const bool printProgress = true);

	/**
	* Inverse Discrete Fourier Transform. Computes the time-domain representation of a frequency-domain signal. In-place version, meaning a constant memory footprint regardless of input. Suitable for performance-sensitive code.
	* 
	* @param out Output of the function, the real-valued time-domain signal. Ensure out.size() is N before calling this function.
	* @param y Input frequency bins, the frequency-domain representation of a signal. y.size() defines K.
	* @param N Number of samples in the output real-valued signal out. Compute this as samplingFrequency * durationOfRealValuedSignal (as floats!). Setting this incorrectly results in a change in pitch.
	*/
	void IDFT(MyUtils::RealSignal& out, const MyUtils::ComplexSignal& y, const MyUtils::uint N, const bool printProgress = true);

	/**
	* Inverse Discrete Fourier Transform. Computes the time-domain representation of a frequency-domain signal.
	*
	* @param y Input frequency bins, the frequency-domain representation of a signal. y.size() defines K.
	* @param N Number of samples in the output real-valued signal out. Compute this as samplingFrequency * durationOfRealValuedSignal (as floats!). Setting this incorrectly results in a change in pitch.
	* @param printProgress Whether to putput % progression of the operation to standard output. Just a piece of mind since simple IDFT takes a long time to compute.
	* @return Output of the function, the real-valued time-domain signal. RealSignal of size N.
	*/
	MyUtils::RealSignal IDFT(const MyUtils::ComplexSignal& y, const MyUtils::uint N, const bool printProgress = true);
}