#ifndef VEC_MATH_VEC3_H
#define VEC_MATH_VEC3_H

#include <cmath>
#include <sstream>
#include <cstdio>

class vec2;
class quat;

//TODO: is there a way to heap allocate within alignment with malloc? does it waste space? is it worth it?
class vec3 {
	public:
		union {
			struct {
				float x, y, z;
			};

			float data[3];
		};

		vec3() {
			x = y = z = 0.0f;

		}
		vec3(float f) {
			x = y = z = f;
		}

		vec3(float xx, float yy, float zz) {
			x = xx;
			y = yy;
			z = zz;
		}

		/*vec3(const vec2& v, float zz) {
			x = v.x;
			y = v.y;
			z = zz;
		}*/

		vec3(const vec3& v) {
			x = v.x;
			y = v.y;
			z = v.z;
		}

		vec3(const char* commaSeparated) {
			x = y = z = 0.0f;
			sscanf(commaSeparated, "%f,%f,%f", &x, &y, &z);
		}

		const vec3& operator =(const vec3& v2) {
			x = v2.x;
			y = v2.y;
			z = v2.z;

			return *this;
		}

		//do
		void set(float xx, float yy, float zz) {
			x = xx;
			y = yy;
			z = zz;
		}

		void normalize() {
			*this /= length();
		}

		/*void rotate(const vec3& center, float theta, axis_enum a) {
			float sinAng = sin(theta);
			float cosAng = cos(theta);

			vec3 v = *this - center;

			switch (a) {
			case axisX:
				y = center.y + v.y * cosAng - v.z * sinAng;
				z = center.z + v.y * sinAng + v.z * cosAng;
				break;

			case axisY:
				x = center.x + v.z * sinAng + v.x * cosAng;
				z = center.z + v.z * cosAng - v.x * sinAng;
				break;

			case axisZ:
				x = center.x + v.x * cosAng - v.y * sinAng;
				y = center.y + v.x * sinAng + v.y * cosAng;
				break;
			}
		}*/

		void operator +=(const vec3& v2) {
			x += v2.x;
			y += v2.y;
			z += v2.z;
		}
		void operator -=(const vec3& v2) {
			x -= v2.x;
			y -= v2.y;
			z -= v2.z;
		}
		void operator *=(float f) {
			x *= f;
			y *= f;
			z *= f;
		}
		void operator /=(float f) {
			x /= f;
			y /= f;
			z /= f;
		}

		//get
		float length() {
			return sqrt(sqrLength());
		}

		float sqrLength() const {
			return x * x + y * y + z * z;
		}
		float distanceTo(const vec3& rhs) const {
			vec3 diff = *this - rhs;
			return sqrt(diff * diff);
		}
		float angleWith(vec3& v2) {
			return acos((*this * v2) / (length() * v2.length()));
		}
		vec3 operator -() const {
			return vec3(-x, -y, -z);
		}
		vec3 normal() const {
			vec3 a(*this);
			a.normalize();
			return a;
		}
		vec3 abs(const vec3& v) {
			return vec3(fabs(v.x), fabs(v.y), fabs(v.z));
		}

		vec3 operator +(const vec3& v2) const {
			return vec3(x + v2.x, y + v2.y, z + v2.z);
		}
		vec3 operator -(const vec3& v2) const {
			return vec3(x - v2.x, y - v2.y, z - v2.z);
		}
		vec3 operator /(const vec3& v2) const {
			return vec3(x / v2.x, y / v2.y, z / v2.z);
		}
		vec3 operator *(float f) const {
			return vec3(x * f, y * f, z * f);
		}
		vec3 operator /(float f) const {
			return vec3(x / f, y / f, z / f);
		}

		float operator *(const vec3& v2) const {
			return (x * v2.x + y * v2.y + z * v2.z);
		}

		vec3 operator ^(const vec3& v2) const {
			return vec3(y * v2.z - z * v2.y, //
			/*        */z * v2.x - x * v2.z, //
			/*        */x * v2.y - y * v2.x);
		}

		quat operator *(const quat& q) const;

		//all
		bool operator ==(const vec3& v2) const {
			return (x == v2.x and y == v2.y and z == v2.z);
		}
		bool operator <(const vec3& v2) const {
			return (x < v2.x and y < v2.y and z < v2.z);
		}
		bool operator <=(const vec3& v2) const {
			return (x <= v2.x and y <= v2.y and z <= v2.z);
		}
		bool operator >(const vec3& v2) const {
			return (x > v2.x and y > v2.y and z > v2.z);
		}
		bool operator >=(const vec3& v2) const {
			return (x >= v2.x and y >= v2.y and z >= v2.z);
		}

		float minComponent() const {
			return fmin(fmin(x, y), z);
		}
		float maxComponent() const {
			return fmax(fmax(x, y), z);
		}

		friend vec3 minVec(const vec3& v1, const vec3& v2) {
			vec3 v(v1);
			if (v2.x < v1.x) v.x = v2.x;
			if (v2.y < v1.y) v.y = v2.y;
			if (v2.z < v1.z) v.z = v2.z;

			return v;
		}

		friend vec3 maxVec(const vec3& v1, const vec3& v2) {
			vec3 v(v1);
			if (v2.x > v1.x) v.x = v2.x;
			if (v2.y > v1.y) v.y = v2.y;
			if (v2.z > v1.z) v.z = v2.z;

			return v;
		}

		friend vec3 computeNormal(const vec3& a, const vec3& b, const vec3& c) {
			vec3 A = a - b;
			vec3 B = a - c;

			return (A ^ B).normal();
		}
};

inline vec3 operator*(float f, const vec3& v) {
	return v * f;
}

#endif /* VEC_MATH_VEC3_H */
