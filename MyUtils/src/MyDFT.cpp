﻿#include "MyDFT.h"

#include <iostream>
#include <algorithm>

#include "MyMath.h"

static std::complex<float> EulersFormula(const float x)
{
	return std::complex<float>(std::cosf(x), std::sinf(x));
}

void MyDFT::DFT(std::vector<std::complex<float>>& out, const std::vector<float>& x, const unsigned int K, const bool printProgress)
{
	const auto PrintProgress = [](const unsigned int k, const unsigned int K)->void
	{
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing DFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent > 99) percent = 0;
		}
	};

	const unsigned int N = (unsigned int)x.size();
	std::vector<std::complex<float>>& y = out;

	std::fill(y.begin(), y.end(), std::complex<float>(0.0f, 0.0f));

	for (unsigned int k = 0; k < K; ++k)
	{
		if (printProgress) PrintProgress(k, K);

		for (unsigned int n = 0; n < N; ++n)
		{
			y[k] += x[n] * EulersFormula(-2.0f * MyMath::PI * k * n / N);
		}
	}

	if (printProgress) std::cout << "DFT done." << std::endl;
}

std::vector<std::complex<float>> MyDFT::DFT(const std::vector<float>& x, const unsigned int K, const bool printProgress)
{
	const auto PrintProgress = [](const unsigned int k, const unsigned int K)->void
	{
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing DFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent > 99) percent = 0;
		}
	};

	const unsigned int N = (unsigned int)x.size();

	std::vector<std::complex<float>> y(K, std::complex<float>(0.0f, 0.0f));

	for (unsigned int k = 0; k < K; ++k)
	{
		if (printProgress) PrintProgress(k, K);

		for (unsigned int n = 0; n < N; ++n)
		{
			y[k] += x[n] * EulersFormula(-2.0f * MyMath::PI * k * n / N);
		}
	}

	if (printProgress) std::cout << "DFT done." << std::endl;

	return y;
}

void MyDFT::IDFT(std::vector<float>& out, const std::vector<std::complex<float>>& y, const unsigned int N, const bool printProgress)
{
	const auto PrintProgress = [](const unsigned int n, const unsigned int N)->void
	{
		if (n % (N / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing IDFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent >= 100) percent = 0;
		}
	};

	const unsigned int K = (unsigned int)y.size();
	std::vector<float>& x = out;

	std::fill(x.begin(), x.end(), 0.0f);

	for (unsigned int n = 0; n < N; ++n)
	{
		if (printProgress) PrintProgress(n, N);

		for (unsigned int k = 0; k < K; ++k)
		{
			x[n] += (y[k] * EulersFormula(2.0f * MyMath::PI * k * n / N)).real();
		}
		x[n] /= N;
		x[n] = std::clamp(x[n], -1.0f, 1.0f); // Out-of-bounds samples can be generated even with correct N due to float imprecision which results in crackling when played back by a device that expects a normalized PCM.
	}

	if (printProgress) std::cout << "IDFT done." << std::endl;
}

std::vector<float> MyDFT::IDFT(const std::vector<std::complex<float>>& y, const unsigned int N, const bool printProgress)
{
	const auto PrintProgress = [](const unsigned int n, const unsigned int N)->void
	{
		if (n % (N / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing IDFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent >= 100) percent = 0;
		}
	};

	const unsigned int K = (unsigned int)y.size();

	std::vector<float> x(N, 0.0f);

	for (unsigned int n = 0; n < N; ++n)
	{
		if (printProgress) PrintProgress(n, N);

		for (unsigned int k = 0; k < K; ++k)
		{
			x[n] += (y[k] * EulersFormula(2.0f * MyMath::PI * k * n / N)).real();
		}
		x[n] /= N;
	}

	if (printProgress) std::cout << "IDFT done." << std::endl;

	return x;
}
