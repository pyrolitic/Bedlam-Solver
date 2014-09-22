#ifndef VEC_MATH_QUAT_H
#define VEC_MATH_QUAT_H

#include <cmath>

#include "vec.h"

template <typename T>
struct quatT {
	T x, y, z, w;
	//1+ i+ j+ k

	quatT() {
		x = T(0);
		y = T(0);
		z = T(0);
		w = T(1);
	}

	template <typename S>
	quatT(S xx, S yy, S zz, S ww) {
		x = xx;
		y = yy;
		z = zz;
		w = ww;
	}

	template <typename S>
	const quatT<T>& operator =(const quatT<S>& q){
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
	}

	//get
	template <typename S>
	void extractFlightAngles(S& heading, S& attitude, S& bank) const {
		T a = w * w + x * x - y * y - z * z;
		T b = T(2) * (x * y + w * z);
		T c = T(2) * (x * z - w * y);
		T d = T(2) * (y * z + w * x);
		T e = w * w - x * x - y * y + z * z;

		T fe = fabs(e);
		if (fe >= T(1)) {
			T f = T(2) * (x * y - w * z);
			T g = T(2) * (w * z + w * y);

			attitude = T(0);
			heading = -M_PI_2 * c / fe;
			bank = atan2(-f, -c * g);
		}

		else {
			attitude = atan2(d, e);
			heading = asin(-c);
			bank = atan2(b, a);
		}
	}

	//void extractMatrix(const mat4& m) const; //TODO

	T magnitude() const {
		return sqrt(x * x + y * y + z * z + w * w);
	}

	quatT<T> normal() const {
		return *this / magnitude();
	}

	template <typename S>
	vec3T<T> rotate(const vec3T<S>& v) const{
		quatT<T> t = ((*this) * quatT(v.x, v.y, v.z, T(0))) * ~(*this);
		return vec3T<T>(t.x, t.y, t.z);
	}

	quatT operator *(T f) const {
		return quatT(x * f, y * f, z * f, w * f);
	}

	quatT operator /(T f) const {
		return quatT(x / f, y / f, z / f, w / f);
	}

	quatT operator *(const quatT& q) const {
		quatT r;
		r.w = w * q.w - x * q.x - y * q.y - z * q.x;
		r.x = w * q.x + x * q.w + y * q.z - z * q.y;
		r.y = w * q.y + y * q.w + z * q.x - x * q.z;
		r.z = w * q.z + z * q.w + x * q.y - y * q.x;
		return r;
	}

	quatT operator +(const quatT& q) const {
		return quatT(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	quatT operator -(const quatT& q) const {
		return quatT(x - q.x, y - q.y, z - q.z, w - q.w);
	}

	//conjugate
	quatT operator ~() const {
		return quatT(-x, -y, -z, w);
	}



	void operator *=(T f) {
		w *= f;
		x *= f;
		y *= f;
		z *= f;
	}

	void operator /=(T f) {
		w /= f;
		x /= f;
		y /= f;
		z /= f;
	}

	void operator *=(const quatT& q) {
		*this = *this * q;
	}

	void operator +=(const quatT& q) {
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;
	}

	void normalize() {
		*this /= magnitude();
	}


	template <typename S, typename P>
	static quatT<T> chainRotation(const quatT<S>& q1, const quatT<P>& q2) {
		return q1 * q2 * (~q1);
	}

	template <typename S, typename P>
	static quatT<T> axisRotation(vec3T<S> axis, P angle){
		quatT<T> q;

		q.x = axis.x * sin(angle / 2);
		q.y = axis.y * sin(angle / 2);
		q.z = axis.z * sin(angle / 2);
		q.w = cos(angle / 2);

		return q;
	}

	template <typename S>
	static quatT<T> flightRotation(S heading, S attitude, S bank) {
		attitude /= T(2);
		heading /= T(2);
		bank /= T(2);

		T cx = cos(attitude);
		T cy = cos(heading);
		T cz = cos(bank);

		T sx = sin(attitude);
		T sy = sin(heading);
		T sz = sin(bank);

		quatT<T> q;
		q.x = sx * cy * cz - cx * sy * sz;
		q.y = cx * sy * cz + sx * cy * sz;
		q.z = cx * cy * sx - sx * sy * cx;
		q.w = cx * cy * cz + sx * sy * sz;

		return q;
	}

	template <typename S, typename P>
	static quatT<T> lerpRot(const quatT<S>& a, const quatT<P>& b, T x){
		//http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/index.htm

		//cos((angle between $a and $b) / 2)
		T cht = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;

		if (fabs(cht) >= T(1)){
			return a;
		}

		T ht = acos(cht);
		T sht = sqrt(T(1) - cht * cht);

		if (fabs(sht) < T(0.001)){
			return (a / 2) + (b / 2);
		}

		else{
			T ra = sin((T(1) - x) * ht) / sht;
			T rb = sin(x * ht) / sht;

			return a * ra + b * rb;
		}
	}
};

typedef quatT<float> quat;
typedef quatT<double> dquat;

#endif
