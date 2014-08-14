#ifndef VEC_MATH_BASE_MAT2_H
#define VEC_MATH_BASE_MAT2_H

#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>

class vec2;

class mat2 {
	public:
		union {
				struct {
						//use these for referencing elements in the matrix
						float _11, _12;
						float _21, _22;
				};

				float data[4]; //vertical layout
		};

		mat2() {
			memset(data, 0, 4 * sizeof(float));
		}

		mat2(const mat2& m) {
			memcpy(data, m.data, 4 * sizeof(float));
		}

		void operator =(const mat2& m) {
			memcpy(data, m.data, 4 * sizeof(float));
		}

		std::string represent() {
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
		}

		vec2 operator *(const vec2& v) const {
			return vec2(data[0] * v.x + data[2] * v.y,
			/*        */data[1] * v.x + data[3] * v.y);
		}

		float det() const {
			return _11 * _22 - _12 * _21;
		}

		//manipulate
		void transpose(){
			float temp = _12;
			_12 = _21;
			_21 = temp;
		}

		void invert(){
			float inv = 1.0f / det();
			mat2 temp;

			temp._11 = _22 * inv;   temp._12 = -_21 * inv;
			temp._21 = -_12 * inv;  temp._22 = _11 * inv;

			memcpy(data, temp.data, 4 * sizeof(float));
		}

		void operator *=(const mat2& m){
			mat2 temp;

			temp._11 = _11 * m._11 + _12 * m._21;  temp._12 = _11 * m._12 + _12 * m._22;
			temp._21 = _21 * m._11 + _22 * m._21;  temp._22 = _21 * m._12 + _22 * m._22;

			memcpy(data, temp.data, 4 * sizeof(float));
		}

		void operator +=(const mat2& m){
			for (int i = 0; i < 4; i++)
				data[i] += m.data[i];
		}
		void operator -=(const mat2& m){
			for (int i = 0; i < 4; i++)
				data[i] -= m.data[i];
		}

		void operator *=(float f){
			for (int i = 0; i < 4; i++)
				data[i] *= f;
		}
		void operator /=(float f) {
			for (int i = 0; i < 4; i++)
				data[i] /= f;
		}

		//completely change to
		void setIdentity(){
			_11 = 1.0f; _12 = 0.0f;
			_21 = 0.0f; _22 = 1.0f;
		}
		void setTranslation(const vec4& v, bool erase = false);

		void setRotation(float theta){
			float c = cos(theta);
			float s = sin(theta);

			_11 = c; _12 = -s;
			_21 = s; _22 = c;
		}
};

inline std::ostream& operator<<(std::ostream& stream, mat2& m) {
	stream << m.represent();
	return stream;
}

#endif /* VEC_MATH_BASE_MAT2_H */
