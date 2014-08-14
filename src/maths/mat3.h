#ifndef VEC_MATH_BASE_MAT3_H
#define VEC_MATH_BASE_MAT3_H

#include <cstdlib>
#include <cstring>
#include <ostream>
#include <string>

class vec4;

#define _SWAP$(a, b) do {temp = a; a = b; b = temp;} while(false)

class mat3 {
	public:
		union {
				struct {
						float _11, _12, _13;
						float _21, _22, _23;
						float _31, _32, _33;
				};

				float data[9];
		};

		mat3() {
		}

		mat3(const mat3& m) {
			memcpy(data, m.data, 9 * sizeof(float));
		}

		void operator =(const mat3& m) {
			memcpy(data, m.data, 9 * sizeof(float));
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
		vec4 operator *(const vec4& v) const {
			return vec4(_11 * v.x + _12 * v.y + _13 * v.z,
			/*         */_21 * v.x + _22 * v.y + _23 * v.z,
			/*         */_31 * v.x + _32 * v.y + _33 * v.z);
		}

		float det() const {
			float a = _11 * _22 * _33;
			float b = _11 * _23 * _32;
			float c = _12 * _21 * _33;
			float d = _12 * _23 * _31;
			float e = _13 * _21 * _32;
			float f = _13 * _22 * _31;

			return a - b - c + d + e - f;
		}

		//manipulate
		void transpose() {
			float temp;
			_SWAP$(_12, _21);
			_SWAP$(_13, _31);
			_SWAP$(_23, _32);
		}

		void adjoint() {
			mat3 temp;

			temp._11 = _22 * _33 - _32 * _23;
			temp._12 = _12 * _33 - _32 * _13;
			temp._13 = _12 * _23 - _22 * _13;
			temp._21 = _21 * _33 - _31 * _23;
			temp._22 = _11 * _33 - _31 * _13;
			temp._23 = _11 * _23 - _21 * _13;
			temp._31 = _21 * _32 - _31 * _22;
			temp._32 = _11 * _32 - _31 * _12;
			temp._33 = _11 * _22 - _21 * _12;

			memcpy(data, temp.data, 9 * sizeof(float));
		}

		void invert() {
			mat3 temp;
			float inv = 1.0f / det();

			temp._11 = (_22 * _33 - _32 * _23) * inv;
			temp._12 = (-_12 * _33 + _32 * _13) * inv;
			temp._13 = (_12 * _23 - _22 * _13) * inv;
			temp._21 = (-_21 * _33 + _31 * _23) * inv;
			temp._22 = (_11 * _33 - _31 * _13) * inv;
			temp._23 = (-_11 * _23 + _21 * _13) * inv;
			temp._31 = (_21 * _32 - _31 * _22) * inv;
			temp._32 = (-_11 * _32 + _31 * _12) * inv;
			temp._33 = (_11 * _22 - _21 * _12) * inv;

			memcpy(data, temp.data, 9 * sizeof(float));
		}

		void operator *=(const mat3& m) {
			mat3 temp;

			temp._11 = _11 * m._11 + _12 * m._21;
			temp._12 = _11 * m._12 + _12 * m._22;
			temp._21 = _21 * m._11 + _22 * m._21;
			temp._22 = _21 * m._12 + _22 * m._22;

			memcpy(data, temp.data, 9 * sizeof(float));
		}

		void operator +=(const mat3& m) {
			for (int i = 0; i < 9; i++)
				data[i] += m.data[i];
		}

		void operator -=(const mat3& m) {
			for (int i = 0; i < 9; i++)
				data[i] -= m.data[i];
		}

		void operator *=(float f) {
			for (int i = 0; i < 9; i++)
				data[i] *= f;
		}

		void operator /=(float f) {
			for (int i = 0; i < 9; i++)
				data[i] /= f;
		}

		//completely change to
		void setIdentity() {
			memset(data, 0, 9 * sizeof(float));
			_11 = 1.0f;
			_22 = 1.0f;
			_33 = 1.0f;
		}

		void setTranslation(const vec4& v, bool erase = false) {
			//static_assert(false, "unimplemented");
			//TODO: use mat3 as a 2d mat4?
		}

		void setOrientation(const mat2& m) {
			_11 = m._11;
			_12 = m._12;
			_21 = m._21;
			_22 = m._22;
		}

		void setRotX(float theta) {
			float c = cos(theta);
			float s = sin(theta);

			_11 = 1.0f;
			_12 = 0.0f;
			_13 = 0.0f;
			_21 = 0.0f;
			_22 = c;
			_23 = -s;
			_31 = 0.0f;
			_32 = s;
			_33 = c;
		}

		void setRotY(float theta) {
			float c = cos(theta);
			float s = sin(theta);

			_11 = c;
			_12 = 0.0f;
			_13 = s;
			_21 = 0.0f;
			_22 = 1.0f;
			_23 = 0.0f;
			_31 = -s;
			_32 = 0.0f;
			_33 = c;
		}

		void setRotZ(float theta) {
			float c = cos(theta);
			float s = sin(theta);

			_11 = c;
			_12 = -s;
			_13 = 0.0f;
			_21 = s;
			_22 = c;
			_23 = 0.0f;
			_31 = 0.0f;
			_32 = 0.0f;
			_33 = 1.0f;
		}

		void setRotation(float rotX, float rotY, float rotZ) {
			float sp = sin(rotX);
			float cp = cos(rotX);

			float sh = sin(rotY);
			float ch = cos(rotY);

			float sr = sin(rotZ);
			float cr = cos(rotZ);

			_11 = ch * cp;
			_12 = sh * cp;
			_13 = -sp;
			_21 = ch * sp * sr - sh * cr;
			_22 = sh * sp * sr + ch * cr;
			_23 = cp * sr;
			_31 = ch * sp * cr + sh * sr;
			_32 = sh * sp * cr - ch * sr;
			_33 = cp * cr;
		}
		void setRotation(const vec4& axis, float theta) {
			vec4 a = (axis).normal();

			float c = cos(theta);
			float s = sin(theta);
			float t = 1.0f - c;

			float a1 = a.x * a.y * t;
			float b1 = a.z * s;

			float a2 = a.x * a.z * t;
			float b2 = a.y * s;

			float a3 = a.y * a.z * t;
			float b3 = a.x * s;

			_11 = c + a.x * a.x * t;
			_12 = a1 - b1;
			_13 = a2 + b2;
			_21 = a1 + b1;
			_22 = c + a.y * a.y * t;
			_23 = a3 - b3;
			_31 = a2 - b2;
			_32 = a3 + b3;
			_33 = c + a.z * a.z * t;
		}

		void setPointTo(const vec4& forward, const vec4& up) {
			vec4 f = forward.normal();
			vec4 U = up.normal();

			vec4 s = f ^ U;
			vec4 u = s ^ f;

			_11 = s.x;
			_12 = u.x;
			_13 = f.x;
			_21 = s.y;
			_22 = u.y;
			_23 = f.y;
			_31 = s.z;
			_32 = u.z;
			_33 = f.z;
		}
		void setVecs(const vec4& a, const vec4& b, const vec4& c) {
			_11 = a.x;
			_12 = b.x;
			_13 = c.x;
			_21 = a.y;
			_22 = b.y;
			_23 = c.y;
			_31 = a.z;
			_32 = b.z;
			_33 = c.z;
		}
};

inline std::ostream& operator<<(std::ostream& stream, mat3& m) {
	stream << m.represent();
	return stream;
}

#endif /* VEC_MATH_BASE_MAT3_H */
