#ifndef VEC_MATH_BASE_MAT3_H
#define VEC_MATH_BASE_MAT3_H

#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>

#include "vec.h"

template<typename T>
class mat3T {
	public:
		union {
				struct {
						/* row major
						T _11, _12, _13;
						T _21, _22, _23;
						T _31, _32, _33;*/ 

						//column major
						T _11, _21, _31;
						T _12, _22, _32;
						T _13, _23, _33;
				};

				T cols[3][3];
				T data[9];
		};

		mat3T() {
			memset(data, 0, 9 * sizeof(T));
		}

		template <typename S>
		mat3T(const mat3T<S>& m) {
			for (int i = 0; i < 9; i++) data[i] = m.data[i];
		}

		template <typename S>
		void operator =(const mat3T<S>& m) {
			for (int i = 0; i < 9; i++) data[i] = m.data[i];
		}

		std::string represent() {
			char buf[9][64];
			int lenghts[9];

			for (int i = 0; i < 9; i++) {
				snprintf(buf[i], 64, "%.5f", data[i]);
				lenghts[i] = strlen(buf[i]);
			}

			int highest = 0;

			for (int i = 0; i < 9; i++)
				if (highest < lenghts[i]) highest = lenghts[i];

			std::string ret;
			ret.reserve(9 * 64);

			for (int j = 0; j < 3; j++) {
				for (int i = 0; i < 3; i++) {
					for (int q = highest + 1; q > lenghts[i + j * 3]; q--)
						ret += ' ';
					ret += buf[i + j * 3];
				}
				ret += '\n';
			}
			ret += '\n';

			return ret;
		}

		//get
		template <typename S>
		vec2T<T> multVec2(const vec2T<S>& v, T vz = 1) const {
			return vec4(_11 * v.x + _12 * v.y + _13 * vz,
			            _21 * v.x + _22 * v.y + _23 * vz,
			            _31 * v.x + _32 * v.y + _33 * vz);
		}

		template <typename S>
		vec3T<T> operator *(const vec3T<S>& v) const {
			return vec4(_11 * v.x + _12 * v.y + _13 * v.z,
			            _21 * v.x + _22 * v.y + _23 * v.z,
			            _31 * v.x + _32 * v.y + _33 * v.z);
		}

		template <typename S>
		operator mat3T<S>(){
			mat3T<S> m;
			for (int i = 0; i < 9; i++) m.data[i] = data[i];
			return m;
		}

		template<typename S>
		mat3T<T> operator *(const mat3T<S>& m) const {
			mat3T<T> ret;

			ret._11 = _11 * m._11 + _12 * m._21 + _13 * m._31;
			ret._12 = _11 * m._12 + _12 * m._22 + _13 * m._32;
			ret._13 = _11 * m._13 + _12 * m._23 + _13 * m._33;

			ret._21 = _21 * m._11 + _22 * m._21 + _23 * m._31;
			ret._22 = _21 * m._12 + _22 * m._22 + _23 * m._32;
			ret._23 = _21 * m._13 + _22 * m._23 + _23 * m._33;

			ret._31 = _31 * m._11 + _32 * m._21 + _33 * m._31;
			ret._32 = _31 * m._12 + _32 * m._22 + _33 * m._32;
			ret._33 = _31 * m._13 + _32 * m._23 + _33 * m._33;

			return ret;
		}

		template<typename S>
		mat3T<T> operator *(S f) const {
			mat3T<T> ret;
			for (int i = 0; i < 9; i++){
				ret.data[i] = data[i] * f;
			}

			return ret;
		}

		template<typename S>
		mat3T<T> operator /(S f) const {
			mat3T<T> ret;
			for (int i = 0; i < 9; i++){
				ret.data[i] = data[i] / f;
			}

			return ret;
		}

		vec3T<T> getCol(int col) const {
			assert(col >= 0 and col < 3);
			//return vec4T<T>(rows[0][col], rows[1][col], rows[2][col]); //row major
			return vec4T<T>(cols[col][0], cols[col][1], cols[col][2]); //column major
		}

		vec3T<T> getRow(int row) const {
			assert(row >= 0 and row < 3);
			//return vec4T<T>(rows[row][0], rows[row][1], rows[row][2]); //row major
			return vec4T<T>(cols[0][row], cols[1][row], cols[2][row]); //column major
		}

		T determinant() const {
			T a = _11 * _22 * _33;
			T b = _11 * _23 * _32;
			T c = _12 * _21 * _33;
			T d = _12 * _23 * _31;
			T e = _13 * _21 * _32;
			T f = _13 * _22 * _31;

			return a - b - c + d + e - f;
		}

		mat3T<T> transpose() const{
			mat3T<T> m;

			m._11 = _11; m._12 = _21; m._13 = _31;
			m._21 = _12; m._22 = _22; m._23 = _32;
			m._31 = _13; m._32 = _23; m._33 = _33;

			return m;
		}

		mat3T<T> inverse(bool& success) const{
			//
			//  -1     T
			// A   = (C ) / det(A)
			//
			// where C is the matrix of cofactors

			mat3T<T> cofactors;

			//in-place transpose
			cofactors._11 =  (_22 * _33 - _32 * _23);
			cofactors._12 = -(_12 * _33 - _32 * _13);
			cofactors._13 =  (_12 * _23 - _22 * _13);

			cofactors._21 = -(_21 * _33 - _31 * _23);
			cofactors._22 =  (_11 * _33 - _31 * _13);
			cofactors._23 = -(_11 * _23 - _21 * _13);

			cofactors._31 =  (_21 * _32 - _31 * _22);
			cofactors._32 = -(_11 * _32 - _31 * _12);
			cofactors._33 =  (_11 * _22 - _21 * _12);
			
			//calculate determinant
			//since we already have the cofactors, just multiply the top row elements
			//by their corresponding cofactors (which are also negated as needed)
			//and add them together
			T det = _11 * cofactors._11 + _12 * cofactors._21 + _13 * cofactors._31;

			//a zero (or close to) determinant means the matrix does not have an inverse
			success = (fabs(det) > 1e-5);
			return cofactors / det;
		}

		void operator +=(const mat3T<T>& m) {
			for (int i = 0; i < 9; i++)
				data[i] += m.data[i];
		}

		void operator -=(const mat3T<T>& m) {
			for (int i = 0; i < 9; i++)
				data[i] -= m.data[i];
		}

		void operator *=(const mat3T<T>& m) {
			*this = *this * m;
		}

		void operator *=(T f) {
			for (int i = 0; i < 9; i++)
				data[i] *= f;
		}

		void operator /=(T f) {
			for (int i = 0; i < 9; i++)
				data[i] /= f;
		}

		//Generators
		static mat3T<T> identity() {
			mat3T<T> m;

			m._11 = T(1);
			               m._22 = T(1);
			                              m._33 = T(1);

			return m;
		}
		
		//2d
		template <typename S>
		static mat3T<T> translation(const vec2T<S>& v) {
			mat3T<T> m;

			m._11 = T(1);                  m._13 = v.x;
			               m._22 = T(1);   m._23 = v.y;
			                               m._33 = T(1);

			return m;
		}

		template <typename S>
		static mat3T<T> scale(const vec2T<S>& v) {
			mat3T<T> m;

			m._11 = v.x;                               
			               m._22 = v.y;              
			                             m._33 = T(1);

			return m;
		}

		static mat3T<T> scale(T s){
			return scale(vec2T<T>(s));
		}

		static mat3T<T> rotation(T theta) {
			T c = cos(theta);
			T s = sin(theta);

			// c -s  0
			// s  c  0
			// 0  0  1

			mat3T<T> m; //all 0

			m._11 = cos(theta);   m._12 = T(0);
			m._21 = s;            m._22 = -sin(theta);  
			                                            m._33 = T(1);

			return m;
		}

		template <typename S>
		static mat3T<T> fromMat4(const mat4T<S>& m4) {
			mat3T<T> m;

			m._11 = m4._11;  m._12 = m4._12;  m._13 = m4._13;
			m._21 = m4._21;  m._22 = m4._22;  m._23 = m4._23;
			m._31 = m4._31;  m._32 = m4._32;  m._33 = m4._33;

			return m;
		}
};

typedef mat3T<float> mat3;
typedef mat3T<double> dmat3;

#endif
