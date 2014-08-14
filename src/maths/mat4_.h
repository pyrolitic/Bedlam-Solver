#ifndef VEC_MATH_MAT4_H
#define VEC_MATH_MAT4_H

#include <sstream>
#include <cstring>
#include <cassert>

#include <string>

#include "misc.h"
#include "quat.h"

class vec3f;
class quat;

class mat4 {
	public:
		union {
				struct {
						//row vectors
						//float _11, _21, _31, _41;
						//float _12, _22, _32, _42;
						//float _13, _23, _33, _43;
						//float _14, _24, _34, _44;

						//column vectors
						float _11, _12, _13, _14;
						float _21, _22, _23, _24;
						float _31, _32, _33, _34;
						float _41, _42, _43, _44;
				};

				float rows[4][4];
				float data[16]; //vertical layout
		};

		mat4() {
			memset(data, 0, 16 * sizeof(float));
		}

		mat4(const mat4& m) {
			memcpy(data, m.data, 16 * sizeof(float));
		}

		void operator =(const mat4& m) {
			memcpy(data, m.data, 16 * sizeof(float));
		}

		std::string represent() {
			char buf[16][64];
			int lenghts[16];

			for (int i = 0; i < 16; i++) {
				snprintf(buf[i], 64, "%.5f", data[i]);
				lenghts[i] = strlen(buf[i]);
			}

			int highest = 0;

			for (int i = 0; i < 16; i++)
				if (highest < lenghts[i]) highest = lenghts[i];

			std::string ret;
			ret.reserve(16 * 64);

			for (int j = 0; j < 4; j++) {
				for (int i = 0; i < 4; i++) {
					for (int q = highest + 1; q > lenghts[i + j * 4]; q--)
						ret += ' ';
					ret += buf[i + j * 4];
				}
				ret += '\n';
			}
			ret += '\n';

			return ret;
		}

		//get
		/*mat3 rotationMatrix() const {
			mat3 temp;

			memcpy(&temp._11, &_11, 3 * sizeof(float));
			memcpy(&temp._21, &_21, 3 * sizeof(float));
			memcpy(&temp._31, &_31, 3 * sizeof(float));

			return temp;
		}*/

		mat4 operator *(const mat4& m) const {
			mat4 m;

			m._11 = _11 * m._11 + _12 * m._21 + _13 * m._31 + _14 * m._41;
			m._12 = _11 * m._12 + _12 * m._22 + _13 * m._32 + _14 * m._42;
			m._13 = _11 * m._13 + _12 * m._23 + _13 * m._33 + _14 * m._43;
			m._14 = _11 * m._14 + _12 * m._24 + _13 * m._34 + _14 * m._44;

			m._21 = _21 * m._11 + _22 * m._21 + _23 * m._31 + _24 * m._41;
			m._22 = _21 * m._12 + _22 * m._22 + _23 * m._32 + _24 * m._42;
			m._23 = _21 * m._13 + _22 * m._23 + _23 * m._33 + _24 * m._43;
			m._24 = _21 * m._14 + _22 * m._24 + _23 * m._34 + _24 * m._44;

			m._31 = _31 * m._11 + _32 * m._21 + _33 * m._31 + _34 * m._41;
			m._32 = _31 * m._12 + _32 * m._22 + _33 * m._32 + _34 * m._42;
			m._33 = _31 * m._13 + _32 * m._23 + _33 * m._33 + _34 * m._43;
			m._34 = _31 * m._14 + _32 * m._24 + _33 * m._34 + _34 * m._44;

			m._41 = _41 * m._11 + _42 * m._21 + _43 * m._31 + _44 * m._41;
			m._42 = _41 * m._12 + _42 * m._22 + _43 * m._32 + _44 * m._42;
			m._43 = _41 * m._13 + _42 * m._23 + _43 * m._33 + _44 * m._43;
			m._44 = _41 * m._14 + _42 * m._24 + _43 * m._34 + _44 * m._44;

			return m;
		}
		/*vec3f operator *(const vec3f& v) const {
			return vec3f(_11 * v.x + _12 * v.y + _13 * v.z + _14, _21 * v.x + _22 * v.y + _23 * v.z + _24, _31 * v.x + _32 * v.y + _33 * v.z + _34);
		}*/

		vec3 operator *(const vec3& v) const {
			return vec3f(_11 * v.x + _12 * v.y + _13 * v.z + _14, _21 * v.x + _22 * v.y + _23 * v.z + _24);
		}

		vec3 getCol(int col) const {
			assert(col >= 0 and col < 4);
			return vec3f(rows[0][col], rows[1][col], rows[2][col]);
		}
		vec3 getRow(int row) const {
			assert(row >= 0 and row < 4);
			return vec3f(rows[row][0], rows[row][0], rows[row][0]);
		}

		//manipulate
		void translateBy(const vec3f& v) {
			//TODO
		}

		void operator +=(const mat4& m) {
			for (int i = 0; i < 16; i++)
				data[i] += m.data[i];
		}
		void operator -=(const mat4& m) {
			for (int i = 0; i < 16; i++)
				data[i] -= m.data[i];
		}

		void operator *=(const mat4& m) {
			mat4 m = *this * m;
			memcpy(data, m.data, 16 * sizeof(float));
		}

		void operator *=(float f) {
			for (int i = 0; i < 16; i++)
				data[i] *= f;
		}
		void operator /=(float f) {
			for (int i = 0; i < 16; i++)
				data[i] /= f;
		}

		//generate...
		void setIdentity() {
			memset(data, 0, 16 * sizeof(float));
			_11 = _22 = _33 = _44 = 1.0f;
		}

		static mat4 translation(const vec3f& v) {
			mat4 m;

			m._11 = 1.0f;
			m._22 = 1.0f;
			m._33 = 1.0f;
			m._44 = 1.0f;

			m._14 = v.x;
			m._24 = v.y;
			m._34 = v.z;

			return m;
		}

		static mat4 XAxisRotation(float theta) {
			float c = cos(theta);
			float s = sin(theta);

			// 1  0  0  0
			// 0  c -s  0
			// 0  s  c  0
			// 0  0  0  1

			mat4 m; //all 0

			m._11 = 1.0f;
			               m._22 = c;     m._23 = -s;
			               m._32 = s;     m._33 = c;
			                                           m._44 = 1.0f;

			return m;
		}

		static mat4 YAxisRotation(float theta) {
			float c = cos(theta);
			float s = sin(theta);

			//  c  0  s  0
			//  0  1  0  0
			// -s  0  c  0
			//  0  0  0  1

			mat4 m; //all 0

			m._11 = c;                  m._13 = s;
			             m._22 = 1.0f;  
			m._31 = -s;                 m._33 = c;
			                                        m._44 = 1.0f;

			return m;
		}

		static mat4 ZAxisRotation(float theta) {
			float c = cos(theta);
			float s = sin(theta);

			// c -s  0  0
			// s  c  0  0
			// 0  0  1  0
			// 0  0  0  1

			mat4 m; //all 0

			m._11 = c;   m._12 = -s;
			m._21 = s;   m._22 = c;  
			                           m._33 = 1.0f;
			                                          m._44 = 1.0f;

			return m;
		}

		static mat4 everyAxisRotation(float rotX, float rotY, float rotZ) {
			//TODO: check angles mapping
			mat4 m;

			float sr = sin(rotX);
			float cr = cos(rotX);

			float sp = sin(rotY);
			float cp = cos(rotY);

			float sy = sin(rotZ);
			float cy = cos(rotZ);

			// cp*cy  sr*sp*cy+cr*-sy  cr*sp*cy*+-sr*-sy  0
			// cp*sy   sr*sp*sy+cr*cy    cr*sp*sy+-sr*cy  0
			//   -sp            sr*cp              cr*cp  0
			//     0                0                  0  1

			m._11 = cp * cy;  m._12 = sr * sp * cy + cr * -sy;  m._13 = cr * sp * cy + -sr * -sy;
			m._21 = cp * sy;  m._22 = sr * sp * sy + cr * cy;   m._23 = cr * sp * sy + -sr * cy;
			m._31 = -sp;      m._32 = sr * cp;                  m._33 = cr * cp;
			                                                                                       m._44 = 1.0f;

			return m;
		}

		void setRotation(const vec3f& axis, float theta) {
			mat4 m;
			vec3f a = axis.normal();

			float c = cos(theta);
			float s = sin(theta);
			float sum = 1.0f - c;

			m._11 = a.x * a.x * sum + c;       m._12 = a.x * a.y * sum - a.z * s; m._13 = a.x * a.z * sum + a.y * s; m._14 = 0.0f;
			m._21 = a.y * a.x * sum + a.z * s; m._22 = a.y * a.y * sum + c;       m._23 = a.y * a.z * sum - a.x * s; m._24 = 0.0f;
			m._31 = a.z * a.x * sum - a.y * s; m._32 = a.z * a.y * sum + a.x * s; m._33 = a.z * a.z * sum + c;       m._34 = 0.0f;
			m._41 = 0.0f;                      m._42 = 0.0f;                      m._43 = 0.0f;                      m._44 = 1.0f;

			return m;
		}
		
		/*void setQuat(const quat& q) {
			//TODO
			memset(data, 0, 16 * sizeof(float));
			_44 = 1.0f;

			float x2 = q.x * q.x;
			float y2 = q.y * q.y;
			float z2 = q.z * q.z;

			float xx = q.x * x2;
			float xy = q.x * y2;
			float xz = q.x * z2;

			float yy = q.y * y2;
			float yz = q.y * z2;
			float zz = q.z * z2;

			float wx = q.w * x2;
			float wy = q.w * y2;
			float wz = q.w * z2;

			data[0] = 1.0f - (yy + zz);
			data[1] = xy - wz;
			data[2] = xz + wy;

			data[4] = xy * wz;
			data[5] = 1.0f - (xx + zz);
			data[6] = yz - wx;

			data[8] = wx - wy;
			data[9] = yz + wx;
			data[10] = 1.0f - (xx + yy);
		}*/

		mat4 transpose() {
			mat4 m;

			m._11 = _11; m._12 = _21; m._13 = _31; m._14 = _41;
			m._21 = _12; m._22 = _22; m._23 = _32; m._24 = _42;
			m._31 = _13; m._32 = _23; m._33 = _33; m._34 = _43;
			m._41 = _14; m._42 = _24; m._43 = _34; m._44 = _44;

			return m;
		}

		mat4 inverse() {
			// from Intel: Streaming SIMD Extensions - Inverse of 4x4 Matrix

			mat4 src;
			src.setTransposeOf(m);

			float tmp[16];

			tmp[0] = m.data[10] * m.data[15];
			tmp[1] = m.data[14] * m.data[11];
			tmp[2] = m.data[6] * m.data[15];
			tmp[3] = m.data[14] * m.data[7];
			tmp[4] = m.data[6] * m.data[11];
			tmp[5] = m.data[10] * m.data[7];
			tmp[6] = m.data[2] * m.data[15];
			tmp[7] = m.data[14] * m.data[3];
			tmp[8] = m.data[2] * m.data[11];
			tmp[9] = m.data[10] * m.data[3];
			tmp[10] = m.data[2] * m.data[7];
			tmp[11] = m.data[6] * m.data[3];

			data[0] = tmp[0] * m.data[5] + tmp[3] * m.data[9] + tmp[4] * m.data[13] - (tmp[1] * m.data[5] + tmp[2] * m.data[9] + tmp[5] * m.data[13]);
			data[1] = tmp[1] * m.data[1] + tmp[6] * m.data[9] + tmp[9] * m.data[13] - (tmp[0] * m.data[1] + tmp[7] * m.data[9] + tmp[8] * m.data[13]);
			data[2] = tmp[2] * m.data[1] + tmp[7] * m.data[5] + tmp[10] * m.data[13] - (tmp[3] * m.data[1] + tmp[6] * m.data[5] + tmp[11] * m.data[13]);
			data[3] = tmp[5] * m.data[1] + tmp[8] * m.data[5] + tmp[11] * m.data[9] - (tmp[4] * m.data[1] + tmp[9] * m.data[5] + tmp[10] * m.data[9]);
			data[4] = tmp[1] * m.data[4] + tmp[2] * m.data[8] + tmp[5] * m.data[12] - (tmp[0] * m.data[4] + tmp[3] * m.data[8] + tmp[4] * m.data[12]);
			data[5] = tmp[0] * m.data[0] + tmp[7] * m.data[8] + tmp[8] * m.data[12] - (tmp[1] * m.data[0] + tmp[6] * m.data[8] + tmp[9] * m.data[12]);
			data[6] = tmp[3] * m.data[0] + tmp[6] * m.data[4] + tmp[11] * m.data[12] - (tmp[2] * m.data[0] + tmp[7] * m.data[4] + tmp[10] * m.data[12]);
			data[7] = tmp[4] * m.data[0] + tmp[9] * m.data[4] + tmp[10] * m.data[8] - (tmp[5] * m.data[0] + tmp[8] * m.data[4] + tmp[11] * m.data[8]);

			tmp[0] = m.data[8] * m.data[13];
			tmp[1] = m.data[12] * m.data[9];
			tmp[2] = m.data[4] * m.data[13];
			tmp[3] = m.data[12] * m.data[5];
			tmp[4] = m.data[4] * m.data[9];
			tmp[5] = m.data[8] * m.data[5];
			tmp[6] = m.data[0] * m.data[13];
			tmp[7] = m.data[12] * m.data[1];
			tmp[8] = m.data[0] * m.data[9];
			tmp[9] = m.data[8] * m.data[1];
			tmp[10] = m.data[0] * m.data[5];
			tmp[11] = m.data[4] * m.data[1];

			data[8] = tmp[0] * m.data[7] + tmp[3] * m.data[11] + tmp[4] * m.data[15] - (tmp[1] * m.data[7] + tmp[2] * m.data[11] + tmp[5] * m.data[15]);
			data[9] = tmp[1] * m.data[3] + tmp[6] * m.data[11] + tmp[9] * m.data[15] - (tmp[0] * m.data[3] + tmp[7] * m.data[11] + tmp[8] * m.data[15]);
			data[10] = tmp[2] * m.data[3] + tmp[7] * m.data[7] + tmp[10] * m.data[15] - (tmp[3] * m.data[3] + tmp[6] * m.data[7] + tmp[11] * m.data[15]);
			data[11] = tmp[5] * m.data[3] + tmp[8] * m.data[7] + tmp[11] * m.data[11] - (tmp[4] * m.data[3] + tmp[9] * m.data[7] + tmp[10] * m.data[11]);
			data[12] = tmp[2] * m.data[10] + tmp[5] * m.data[14] + tmp[1] * m.data[6] - (tmp[4] * m.data[14] + tmp[0] * m.data[6] + tmp[3] * m.data[10]);
			data[13] = tmp[8] * m.data[14] + tmp[0] * m.data[2] + tmp[7] * m.data[10] - (tmp[6] * m.data[10] + tmp[9] * m.data[14] + tmp[1] * m.data[2]);
			data[14] = tmp[6] * m.data[6] + tmp[11] * m.data[14] + tmp[3] * m.data[2] - (tmp[10] * m.data[14] + tmp[2] * m.data[2] + tmp[7] * m.data[6]);
			data[15] = tmp[10] * m.data[10] + tmp[4] * m.data[2] + tmp[9] * m.data[6] - (tmp[8] * m.data[6] + tmp[11] * m.data[10] + tmp[5] * m.data[2]);

			float det = m.data[0] * data[0] + m.data[4] * data[1] + m.data[8] * data[2] + m.data[12] * data[3];

			for (int i = 0; i < 16; i++)
				data[i] /= det;
		}

		//projections
		static mat4 perspectiveProjection(float fovY, float aspect, float zNear, float zFar) {
			float range = tan(mathConstants::piDiv180 * fovY / 2.0f) * zNear;
			float left = -range * aspect;
			float right = range * aspect;
			float bottom = -range;
			float top = range;

			mat4 m;

			data[0] = (2.0f * zNear) / (right - left);
			data[1] = 0.0f;
			data[2] = 0.0f;
			data[3] = 0.0f;

			data[4] = 0.0f;
			data[5] = (2.0f * zNear) / (top - bottom);
			data[6] = 0.0f;
			data[7] = 0.0f;

			data[8] = 0.0f;
			data[9] = 0.0f;
			data[10] = -(zFar + zNear) / (zFar - zNear);
			data[11] = -1.0f;

			data[12] = 0.0f;
			data[13] = 0.0f;
			data[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
			data[15] = 0.0f;
		}

		static mat4 perspectiveProjection(float fov, float width, float height, float zNear, float zFar) {
			float rad = mathConstants::piDiv180 * fov;
			float h = cos(0.5f * rad) / sin(0.5f * rad);
			float w = h * height / width;

			data[0] = w;
			data[1] = 0.0f;
			data[2] = 0.0f;
			data[3] = 0.0f;

			data[4] = 0.0f;
			data[5] = h;
			data[6] = 0.0f;
			data[7] = 0.0f;

			data[8] = 0.0f;
			data[9] = 0.0f;
			data[10] = (zFar + zNear) / (zFar - zNear);
			data[11] = 1.0f;

			data[12] = 0.0f;
			data[13] = 0.0f;
			data[14] = -(2.0f * zFar * zNear) / (zFar - zNear);
			data[15] = 0.0f;

			return m;
		}

		static mat4 orthographicProjection(float left, float right, float bottom, float top, float zFar = 1.0f, float zNear = -1.0f) {
			data[0] = 2.0f / (right - left);
			data[1] = 0.0f;
			data[2] = 0.0f;
			data[3] = 0.0f;

			data[4] = 0.0f;
			data[5] = 2.0f / (top - bottom);
			data[6] = 0.0f;
			data[7] = 0.0f;

			data[8] = 0.0f;
			data[9] = 0.0f;
			data[10] = -2.0f / (zFar - zNear);
			data[11] = 0.0f;

			data[12] = -(right + left) / (right - left);
			data[13] = -(top + bottom) / (top - bottom);
			data[14] = -(zFar + zNear) / (zFar - zNear);
			data[15] = 1.0f;

			return m;
		}

		static mat4 frustum(float left, float right, float bottom, float top, float nearVal, float farVal) {
			mat4 m;

			data[0] = (2.0f * nearVal) / (right - left);
			data[1] = 0.0f;
			data[2] = 0.0f;
			data[3] = 0.0f;

			data[4] = 0.0f;
			data[5] = (2.0f * nearVal) / (top - bottom);
			data[6] = 0.0f;
			data[7] = 0.0f;

			data[8] = (right + left) / (right - left);
			data[9] = (top + bottom) / (top - bottom);
			data[10] = -(farVal + nearVal) / (farVal - nearVal);
			data[11] = -1.0f;

			data[12] = 0.0f;
			data[13] = 0.0f;
			data[14] = -(2.0f * farVal * nearVal) / (farVal - nearVal);
			data[15] = 0.0f;

			return m;
		}

		static mat4 lookAt(const vec3f& eye, const vec3f& center, const vec3f& up = vec3f(0.0f, 1.0f, 0.0f)) {
			vec3f dir = (center - eye).normal();
			float angle = up * dir;
			vec3f u = (up - dir * angle).normal();
			vec3f right = up ^ dir;

			data[0] = right.x;
			data[1] = right.y;
			data[2] = right.z;
			data[3] = 0.0f;

			data[4] = u.x;
			data[5] = u.y;
			data[6] = u.z;
			data[7] = 0.0f;

			data[8] = dir.x;
			data[9] = dir.y;
			data[10] = dir.z;
			data[11] = 0.0f;

			data[12] = eye.x;
			data[13] = eye.y;
			data[14] = eye.z;
			data[15] = 1.0f;

			return m;
		}

		static mat4 billboard(const vec3f& eye, const vec3f& dir, const vec3f& up = vec3f(0.0f, 1.0f, 0.0f)) {
			float angle = up * dir;
			vec3f u = (up - dir * angle).normal();
			vec3f right = up ^ dir;

			mat4 m;

			data[0] = right.x;
			data[1] = right.y;
			data[2] = right.z;
			data[3] = 0.0f;

			data[4] = u.x;
			data[5] = u.y;
			data[6] = u.z;
			data[7] = 0.0f;

			data[8] = dir.x;
			data[9] = dir.y;
			data[10] = dir.z;
			data[11] = 0.0f;

			data[12] = eye.x;
			data[13] = eye.y;
			data[14] = eye.z;
			data[15] = 1.0f;

			return m;
		}
};

#endif
