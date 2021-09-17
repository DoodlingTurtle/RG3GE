#pragma once

#include "./Macros.h"

template <typename T> class Vec2;

namespace RG3GE {

	struct Angle {
		double angle;
		Vec2<double> direction;

		Angle(double ang = 0);
		Angle(double dirx, double diry);

		Angle& operator += (double a);
		Angle& operator -= (double a);
	};

	struct Transform {
		Vec2<float> position;
		Vec2<float> origin;
		Vec2<float> scale;

		Angle rotation;
	};

}
