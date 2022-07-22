#include "FDN.h"

#include <cassert>

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
