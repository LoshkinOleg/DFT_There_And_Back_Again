#include "MyDFT.h"

#include <iostream>

#include "MyUtils.h"

void MyDFT::DFT(MyUtils::ComplexSignal& out, const MyUtils::RealSignal& x, const MyUtils::uint K, const bool printProgress)
{
	const auto PrintProgress = [](const MyUtils::uint k, const MyUtils::uint K)->void
	{
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing DFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent > 99) percent = 0;
		}
	};

	const MyUtils::uint N = (MyUtils::uint)x.size();
	MyUtils::ComplexSignal& y = out;

	std::fill(y.begin(), y.end(), MyUtils::Complex(0.0f, 0.0f));

	for (MyUtils::uint k = 0; k < K; ++k)
	{
		if (printProgress) PrintProgress(k, K);

		for (MyUtils::uint n = 0; n < N; ++n)
		{
			y[k] += x[n] * MyUtils::InverseEulersFormula(2.0f * MyUtils::PI * k * n / N);
		}
	}

	if (printProgress) std::cout << "DFT done." << std::endl;
}

MyUtils::ComplexSignal MyDFT::DFT(const MyUtils::RealSignal& x, const MyUtils::uint K, const bool printProgress)
{
	const auto PrintProgress = [](const MyUtils::uint k, const MyUtils::uint K)->void
	{
		if (k % (K / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing DFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent > 99) percent = 0;
		}
	};

	const MyUtils::uint N = (MyUtils::uint)x.size();

	MyUtils::ComplexSignal y(K, MyUtils::Complex(0.0f, 0.0f));

	for (MyUtils::uint k = 0; k < K; ++k)
	{
		if (printProgress) PrintProgress(k, K);

		for (MyUtils::uint n = 0; n < N; ++n)
		{
			y[k] += x[n] * MyUtils::InverseEulersFormula(2.0f * MyUtils::PI * k * n / N);
		}
	}

	if (printProgress) std::cout << "DFT done." << std::endl;

	return y;
}

void MyDFT::IDFT(MyUtils::RealSignal& out, const MyUtils::ComplexSignal& y, const MyUtils::uint N, const bool printProgress)
{
	const auto PrintProgress = [](const MyUtils::uint n, const MyUtils::uint N)->void
	{
		if (n % (N / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing IDFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent >= 100) percent = 0;
		}
	};

	const MyUtils::uint K = (MyUtils::uint)y.size();
	MyUtils::RealSignal& x = out;

	std::fill(x.begin(), x.end(), 0.0f);

	for (MyUtils::uint n = 0; n < N; ++n)
	{
		if (printProgress) PrintProgress(n, N);

		for (MyUtils::uint k = 0; k < K; ++k)
		{
			x[n] += (y[k] * MyUtils::EulersFormula(2.0f * MyUtils::PI * k * n / N)).real();
		}
		x[n] /= N;
	}

	if (printProgress) std::cout << "IDFT done." << std::endl;
}

MyUtils::RealSignal MyDFT::IDFT(const MyUtils::ComplexSignal& y, const MyUtils::uint N, const bool printProgress)
{
	const auto PrintProgress = [](const MyUtils::uint n, const MyUtils::uint N)->void
	{
		if (n % (N / 100) == 0)
		{
			static unsigned int percent = 0;
			std::cout << "Computing IDFT: " << std::to_string(percent++) << "% done." << std::endl;
			if (percent >= 100) percent = 0;
		}
	};

	const MyUtils::uint K = (MyUtils::uint)y.size();

	MyUtils::RealSignal x(N, 0.0f);

	for (MyUtils::uint n = 0; n < N; ++n)
	{
		if (printProgress) PrintProgress(n, N);

		for (MyUtils::uint k = 0; k < K; ++k)
		{
			x[n] += (y[k] * MyUtils::EulersFormula(2.0f * MyUtils::PI * k * n / N)).real();
		}
		x[n] /= N;
	}

	if (printProgress) std::cout << "IDFT done." << std::endl;

	return x;
}
