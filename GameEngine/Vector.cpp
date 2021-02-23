#include "Vector.h"

Vector::Vector(float* arr, int length_)
{
	vector = arr;
	length = length_;
}
Vector::~Vector()
{
	if (vector != nullptr) {
		delete vector;
	}
}

float Vector::qInvSqrt(float value)
{
	return qInvSqrt(value, 2);
}

float Vector::qInvSqrt(float value, int passes)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = value * 0.5F;
	y = value;
	i = *(long*)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float*)&i;

	if (passes > 0) {
		for (int i = 0; i < passes; i++) {
			y = y * (threehalfs - (x2 * y * y));
		}
	}

	return y;
}