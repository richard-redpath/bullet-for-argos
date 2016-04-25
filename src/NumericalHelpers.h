//
// Created by rar500 on 13/11/15.
//

#ifndef ARGOS3_BULLET_NUMERICALHELPERS_H
#define ARGOS3_BULLET_NUMERICALHELPERS_H

#include <limits>
#include <cmath>
#include <argos3/core/utility/math/quaternion.h>
#include <argos3/core/utility/math/vector3.h>

/*
 * Some float definitions
 * Positive and negative infinity and NaN
 */
#define pInf (std::numeric_limits<float>::infinity())
#define nInf (-pInf)
#define NaN  (std::numeric_limits<float>::signaling_NaN())

/**
 * Restrict a value to a specified range
 */
template<class T>
void clamp(T &input, T min, T max)
{
	if(input < min)
		input = min;

	if(input > max)
		input = max;
}

/**
 * Return the negative magnitude of a provided value
 */
template<class T>
T makeNegative(T input)
{
	if(input > 0)
		input *= -1;

	return input;
}

/**
 * Convenience method to get an ARGoS quaternion from roll, pitch and yaw
 */
inline argos::CQuaternion quaternionFromRadians(float roll, float pitch, float yaw)
{
	using namespace argos;
	CQuaternion ret;
	CRadians r{roll};
	CRadians p{pitch};
	CRadians y{yaw};
	ret.FromEulerAngles(y, p, r);
	return ret;
}

/**
 * Return a rotated ARGoS vector
 */
static inline argos::CVector3 rotateARGoSVector(const argos::CVector3& vec, const argos::CQuaternion& rot)
{
	argos::CVector3 t = vec;
	t.Rotate(rot);
	return t;
}

/**
 * Combine 2 quaternions
 */
static inline argos::CQuaternion combineARGoSQuaternions(const argos::CQuaternion& first, const argos::CQuaternion& second)
{
	return first * second;
}

#endif //ARGOS3_BULLET_NUMERICALHELPERS_H
