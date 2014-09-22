#ifndef VEC_MATH_BASE_MAT2_H
#define VEC_MATH_BASE_MAT2_H

#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>

#include "vec.h"

template <typename T>
struct mat2T {
	union {
		struct {
			//use these for referencing elements in the matrix
			T _11, _21;
			T _12, _22;
		};

		T data[4]; //vertical layout
	};

	mat2T() {
		memset(data, 0, 4 * sizeof(float));
	}

	template <typename S>
	mat2T(const mat2T<S>& m) {
		for(int i = 0; i < 4; i++)
			data[i] = m.data[i];
	}

	template <typename S>
	void operator =(const mat2T& m) {
		for(int i = 0; i < 4; i++)
			data[i] = m.data[i];
	}

	/*std::string represent() {
		char buf[4][64];
		int lenghts[4];

		for (int i = 0; i < 4; i++) {
			snprintf(buf[i], 64, "%.5f", data[i]);
			lenghts[i] = strlen(buf[i]);
		}

		int highest = 0;

		for (int i = 0; i < 4; i++)
			if (highest < lenghts[i]) highest = lenghts[i];

		std::string ret;
		ret.reserve(4 * 64);

		for (int j = 0; j < 2; j++) {
			for (int i = 0; i < 2; i++) {
				for (int q = highest + 1; q > lenghts[i + j * 2]; q--)
					ret += ' ';
				ret += buf[i + j * 2];
			}
			ret += '\n';
		}
		ret += '\n';

		return ret;
	}*/

	template <typename S>
	mat2T<S> operator *(const mat2T<S>& m) const{
		mat2T<S> ret;

		ret._11 = _11 * m._11 + _12 * m._21;
		ret._12 = _11 * m._12 + _12 * m._22;

		ret._21 = _21 * m._11 + _22 * m._21;
		ret._22 = _21 * m._12 + _22 * m._22;

		return m;
	}

	template <typename S>
	vec2T<S> operator *(const vec2T<S>& v) const {
		return vec2T<S>(_11 * v.x + _12 * v.y,
		                _21 * v.x + _22 * v.y);
	}

	T det() const {
		return _11 * _22 - _12 * _21;
	}

	mat2T<T> transpose() const{
		mat2T<T> m;

		m._11 = _11; m._12 = _21;
		m._21 = _12; m._22 = _22;

		return m;
	}

	mat2T<T> inverse() const{
		mat2T<T> m;

		T d = det();
		m._11 =  _22 / d;  m._12 = -_12 / d;
		m._21 = -_21 / d;  m._22 =  _11 / d;

		return m;
	}

	mat2T<T> operator -(){
		mat2T<T> temp;

		for (int i = 0; i < 4; i++)
			temp.data[i] = -data[i];

		return temp;
	}

	template <typename S>
	mat2T<T> operator *(const mat2T<S>& m){
		mat2T<T> temp;

		temp._11 = _11 * m._11 + _12 * m._21;  temp._12 = _11 * m._12 + _12 * m._22;
		temp._21 = _21 * m._11 + _22 * m._21;  temp._22 = _21 * m._12 + _22 * m._22;

		return temp;
	}

	template <typename S>
	mat2T<T> operator +(const mat2T<S>& m){
		mat2T<T> temp;

		for (int i = 0; i < 4; i++)
			temp.data[i] = data[i] + m.data[i];

		return temp;
	}

	template <typename S>
	mat2T<T> operator -(const mat2T<S>& m){
		mat2T<T> temp;

		for (int i = 0; i < 4; i++)
			temp.data[i] = data[i] - m.data[i];

		return temp;
	}


	template <typename S>
	mat2T<T> operator *(S f){
		mat2T<T> temp;

		for (int i = 0; i < 4; i++)
			temp.data[i] = data[i] * f;

		return temp;
	}

	template <typename S>
	mat2T<T> operator /(S f) {
		mat2T<T> temp;

		for (int i = 0; i < 4; i++)
			temp.data[i] = data[i] / f;

		return temp;
	}



	template <typename S>
	void operator *=(const mat2T<S>& m){
		mat2T<T> temp;

		temp._11 = _11 * m._11 + _12 * m._21;  temp._12 = _11 * m._12 + _12 * m._22;
		temp._21 = _21 * m._11 + _22 * m._21;  temp._22 = _21 * m._12 + _22 * m._22;

		memcpy(data, temp.data, 4 * sizeof(float));
	}

	template <typename S>
	void operator +=(const mat2T<S>& m){
		for (int i = 0; i < 4; i++)
			data[i] += m.data[i];
	}

	template <typename S>
	void operator -=(const mat2T<S>& m){
		for (int i = 0; i < 4; i++)
			data[i] -= m.data[i];
	}


	template <typename S>
	void operator *=(S f){
		for (int i = 0; i < 4; i++)
			data[i] *= f;
	}

	template <typename S>
	void operator /=(S f) {
		for (int i = 0; i < 4; i++)
			data[i] /= f;
	}

	/*void setTranslation(const vec4& v, bool erase = false);

	void setRotation(float theta){
		float c = cos(theta);
		float s = sin(theta);

		_11 = c; _12 = -s;
		_21 = s; _22 = c;
	}*/
};

typedef mat2T<float> mat2;
typedef mat2T<double> dmat2;

#endif
