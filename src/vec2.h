#ifndef VEC_MATH_VEC2_H
#define VEC_MATH_VEC2_H

#include <ostream>

#include <cmath>

//===========================================================================================
template <typename T>
class vec2Templated {
	public:
		union {
				struct {
						T x, y;
				};

				T data[2];
		};

		vec2Templated() {
			x = y = 0;
		}

		vec2Templated(T f) {
			x = y = f;
		}

		vec2Templated(T xx, T yy) {
			x = xx;
			y = yy;
		}

		vec2Templated(const vec2Templated<T>& rhs) {
			x = rhs.x;
			y = rhs.y;
		}

		const vec2Templated<T>& operator =(const vec2Templated<T>& v2) {
			x = v2.x;
			y = v2.y;

			return *this;
		}

		//do
		void set(T xx, T yy) {
			x = xx;
			y = yy;
		}

		void normalize() {
			*this /= length();
		}

		void rotate(const vec2Templated<T>& center, T theta) {
			T sinAng = sin(theta);
			T cosAng = cos(theta);

			T a = x - center.x;
			T b = y - center.y;

			x = center.x + a * cosAng - b * sinAng;
			y = center.y + a * sinAng + b * cosAng;
		}

		void operator +=(const vec2Templated<T>& v2) {
			x += v2.x;
			y += v2.y;
		}
		void operator -=(const vec2Templated<T>& v2) {
			x -= v2.x;
			y -= v2.y;
		}

		void operator *=(T f) {
			x *= f;
			y *= f;
		}

		void operator /=(T f) {
			x /= f;
			y /= f;
		}

		//get
		T length() const {
			return sqrt(x * x + y * y);
		}
		T sqrLength() const {
			return x * x + y * y;
		}
		T distanceTo(const vec2Templated<T>& v2) const {
			return sqrt((x - v2.x) * (x - v2.x) + (y - v2.y) * (y - v2.y));
		}
		vec2Templated<T> operator -() const {
			return vec2Templated<T>(-x, -y);
		}
		vec2Templated<T> unit() const {
			return *this / length();
		}
		vec2Templated<T> abs() const {
			return vec2Templated<T>(fabs(x), fabs(y));
		}
		T operator *(const vec2Templated<T>& v2) const {
			return (x * v2.x + y * v2.y);
		}

		vec2Templated<T> operator +(const vec2Templated<T>& v2) const {
			return vec2Templated<T>(x + v2.x, y + v2.y);
		}
		vec2Templated<T> operator -(const vec2Templated<T>& v2) const {
			return vec2Templated<T>(x - v2.x, y - v2.y);
		}
		vec2Templated<T> operator *(T f) const {
			return vec2Templated<T>(x * f, y * f);
		}
		vec2Templated<T> operator /(T f) const {
			return vec2Templated<T>(x / f, y / f);
		}

		//all
		bool operator ==(const vec2Templated<T>& v2) {
			return x == v2.x and y == v2.y;
		}
		bool operator !=(const vec2Templated<T>& v2) {
			return !(*this == v2); //strange this wasn't filled in by the compiler
		}
		bool operator <(const vec2Templated<T>& v2) {
			return x < v2.x and y < v2.y;
		}
		bool operator <=(const vec2Templated<T>& v2) {
			return x <= v2.x and y <= v2.y;
		}
		bool operator >(const vec2Templated<T>& v2) {
			return x > v2.x and y > v2.y;
		}
		bool operator >=(const vec2Templated<T>& v2) {
			return x >= v2.x and y >= v2.y;
		}
};

template <typename T>
inline vec2Templated<T> operator*(T f, const vec2Templated<T>& v) {
	return v * f;
}

template <typename T>
inline vec2Templated<T> minVec(const vec2Templated<T>& v1, const vec2Templated<T>& v2) {
	vec2Templated<T> v(v1);
	if (v2.x < v1.x) v.x = v2.x;
	if (v2.y < v1.y) v.y = v2.y;

	return v;
}
		
template <typename T>
inline vec2Templated<T> maxVec(const vec2Templated<T>& v1, const vec2Templated<T>& v2) {
	vec2Templated<T> v(v1);
	if (v2.x > v1.x) v.x = v2.x;
	if (v2.y > v1.y) v.y = v2.y;

	return v;
}


typedef vec2Templated<float> vec2;
typedef vec2Templated<float> vec2f;
typedef vec2Templated<int> vec2i;

namespace std {
	template <typename T> struct hash<vec2Templated<T>>{
		#ifndef ROTATE_LEFT
		#define ROTATE_LEFT(x, amount) ((x << amount) | (x >> (sizeof(x) * 8 - amount)))
		#endif

		std::size_t operator()(const vec2Templated<T>& k) const{
			//x = 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
			//y = 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17
			using std::hash;
			return hash<T>()((k.x) ^ ROTATE_LEFT(k.y, 15));
		}
	};
}

#endif

