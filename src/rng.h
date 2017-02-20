#pragma once

#include <random>

namespace rng
{
	int rand_int(int min, int max)
	{
		//static std::random_device rd;
		static std::mt19937 rng(0);

		std::uniform_int_distribution<int> uni(min, max);

		return uni(rng);
	}

	float rand_float(float min, float max)
	{
		float i = rand_int(int(min * 100), int(max * 100)) / 100.f;
		return i;
	}
}