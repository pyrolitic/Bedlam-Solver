#ifndef VEC_MATH_MAT4_H
#define VEC_MATH_MAT4_H

#include <cstring>
#include <cassert>

#include <string>

#define _USE_MATH_DEFINES 1 //for msvcc, to have M_PI and its brethren defined (hopefully the same as for)
#include <cmath>

#include "vec3.h"

//#include "quat.h"
//class quat;

class mat4 {
	public:
		union {
				struct {
						//opengl uses this kind of layout
						/*float _11, _21, _31, _41;
						float _12, _22, _32, _42;
						float _13, _23, _33, _43;
						float _14, _24, _34, _44;*/

						
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
			mat4 ret;

			ret._11 = _11 * m._11 + _12 * m._21 + _13 * m._31 + _14 * m._41;
			ret._12 = _11 * m._12 + _12 * m._22 + _13 * m._32 + _14 * m._42;
			ret._13 = _11 * m._13 + _12 * m._23 + _13 * m._33 + _14 * m._43;
			ret._14 = _11 * m._14 + _12 * m._24 + _13 * m._34 + _14 * m._44;

			ret._21 = _21 * m._11 + _22 * m._21 + _23 * m._31 + _24 * m._41;
			ret._22 = _21 * m._12 + _22 * m._22 + _23 * m._32 + _24 * m._42;
			ret._23 = _21 * m._13 + _22 * m._23 + _23 * m._33 + _24 * m._43;
			ret._24 = _21 * m._14 + _22 * m._24 + _23 * m._34 + _24 * m._44;

			ret._31 = _31 * m._11 + _32 * m._21 + _33 * m._31 + _34 * m._41;
			ret._32 = _31 * m._12 + _32 * m._22 + _33 * m._32 + _34 * m._42;
			ret._33 = _31 * m._13 + _32 * m._23 + _33 * m._33 + _34 * m._43;
			ret._34 = _31 * m._14 + _32 * m._24 + _33 * m._34 + _34 * m._44;

			ret._41 = _41 * m._11 + _42 * m._21 + _43 * m._31 + _44 * m._41;
			ret._42 = _41 * m._12 + _42 * m._22 + _43 * m._32 + _44 * m._42;
			ret._43 = _41 * m._13 + _42 * m._23 + _43 * m._33 + _44 * m._43;
			ret._44 = _41 * m._14 + _42 * m._24 + _43 * m._34 + _44 * m._44;

			return ret;
		}

		vec3 operator *(const vec3& v) const {
			return vec3f(_11 * v.x + _12 * v.y + _13 * v.z + _14, 
			             _21 * v.x + _22 * v.y + _23 * v.z + _24,
			             _31 * v.x + _32 * v.y + _33 * v.z + _34);
		}

		vec3 getCol(int col) const {
			assert(col >= 0 and col < 4);
			return vec3f(rows[0][col], rows[1][col], rows[2][col]);
		}

		vec3 getRow(int row) const {
			assert(row >= 0 and row < 4);
			return vec3f(rows[row][0], rows[row][0], rows[row][0]);
		}

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

			float tmp[16];
			mat4 m;

			tmp[0] = _33 * _44;
			tmp[1] = _43 * _34;
			tmp[2] = _23 * _44;
			tmp[3] = _43 * _24;
			tmp[4] = _23 * _34;
			tmp[5] = _33 * _24;
			tmp[6] = _13 * _44;
			tmp[7] = _43 * _14;
			tmp[8] = _13 * _34;
			tmp[9] = _33 * _14;
			tmp[10] = _13 * _24;
			tmp[11] = _23 * _14;

			m._11 = tmp[0] * _22 + tmp[3] * _32 + tmp[4] * _42 - (tmp[1] * _22 + tmp[2] * _32 + tmp[5] * _42);
			m._12 = tmp[1] * _12 + tmp[6] * _32 + tmp[9] * _42 - (tmp[0] * _12 + tmp[7] * _32 + tmp[8] * _42);
			m._13 = tmp[2] * _12 + tmp[7] * _22 + tmp[10] * _42 - (tmp[3] * _12 + tmp[6] * _22 + tmp[11] * _42);
			m._14 = tmp[5] * _12 + tmp[8] * _22 + tmp[11] * _32 - (tmp[4] * _12 + tmp[9] * _22 + tmp[10] * _32);
			m._21 = tmp[1] * _21 + tmp[2] * _31 + tmp[5] * _41 - (tmp[0] * _21 + tmp[3] * _31 + tmp[4] * _41);
			m._22 = tmp[0] * _11 + tmp[7] * _31 + tmp[8] * _41 - (tmp[1] * _11 + tmp[6] * _31 + tmp[9] * _41);
			m._23 = tmp[3] * _11 + tmp[6] * _21 + tmp[11] * _41 - (tmp[2] * _11 + tmp[7] * _21 + tmp[10] * _41);
			m._24 = tmp[4] * _11 + tmp[9] * _21 + tmp[10] * _31 - (tmp[5] * _11 + tmp[8] * _21 + tmp[11] * _31);

			tmp[0] = _31 * _42;
			tmp[1] = _41 * _32;
			tmp[2] = _21 * _42;
			tmp[3] = _41 * _22;
			tmp[4] = _21 * _32;
			tmp[5] = _31 * _22;
			tmp[6] = _11 * _42;
			tmp[7] = _41 * _12;
			tmp[8] = _11 * _32;
			tmp[9] = _31 * _12;
			tmp[10] = _11 * _22;
			tmp[11] = _21 * _12;

			m._31 = tmp[0] * _24 + tmp[3] * _34 + tmp[4] * _44 - (tmp[1] * _24 + tmp[2] * _34 + tmp[5] * _44);
			m._32 = tmp[1] * _14 + tmp[6] * _34 + tmp[9] * _44 - (tmp[0] * _14 + tmp[7] * _34 + tmp[8] * _44);
			m._33 = tmp[2] * _14 + tmp[7] * _24 + tmp[10] * _44 - (tmp[3] * _14 + tmp[6] * _24 + tmp[11] * _44);
			m._34 = tmp[5] * _14 + tmp[8] * _24 + tmp[11] * _34 - (tmp[4] * _14 + tmp[9] * _24 + tmp[10] * _34);
			m._41 = tmp[2] * _33 + tmp[5] * _43 + tmp[1] * _23 - (tmp[4] * _43 + tmp[0] * _23 + tmp[3] * _33);
			m._42 = tmp[8] * _43 + tmp[0] * _13 + tmp[7] * _33 - (tmp[6] * _33 + tmp[9] * _43 + tmp[1] * _13);
			m._43 = tmp[6] * _23 + tmp[11] * _43 + tmp[3] * _13 - (tmp[10] * _43 + tmp[2] * _13 + tmp[7] * _23);
			m._44 = tmp[10] * _33 + tmp[4] * _13 + tmp[9] * _23 - (tmp[8] * _23 + tmp[11] * _33 + tmp[5] * _13);

			float det = _11 * _11 + _21 * _12 + _31 * _13 + _41 * _14;
			for (int i = 0; i < 16; i++)
				m.data[i] /= det;

			return m;
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
			*this = *this * m;
		}

		void operator *=(float f) {
			for (int i = 0; i < 16; i++)
				data[i] *= f;
		}

		void operator /=(float f) {
			for (int i = 0; i < 16; i++)
				data[i] /= f;
		}

		//Generators
		
		static mat4 identity() {
			mat4 m;

			m._11 = 1.0f;
			               m._22 = 1.0f;
			                              m._33 = 1.0f;
			                                             m._44 = 1.0f;

			return m;
		}
		
		static mat4 translation(const vec3f& v) {
			mat4 m;

			m._11 = 1.0f;                                m._14 = v.x;
			               m._22 = 1.0f;                 m._24 = v.y;
			                              m._33 = 1.0f;  m._34 = v.z;
			                                             m._44 = 1.0f;

			return m;
		}

		static mat4 scale(float s){
			mat4 m;

			m._11 = s;                      
			            m._22 = s;                
			                        m._33 = s;
			                                    m._44 = 1.0f;

			return m;
		}

		static mat4 scale(const vec3f& v) {
			mat4 m;

			m._11 = v.x;                               
			               m._22 = v.y;              
			                              m._33 = v.z;
			                                            m._44 = 1.0f;

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

		static mat4 flightRotation(float pitch, float yaw, float roll){
			float sr = sin(roll);
			float cr = cos(roll);

			float sp = sin(pitch);
			float cp = cos(pitch);

			float sy = sin(yaw);
			float cy = cos(yaw);

			mat4 m;

			/* not sure where I got this from, but it looks wrong
			m._11 = cp * cy;  m._12 = sr * sp * cy + cr * -sy;  m._13 = cr * sp * cy + -sr * -sy;
			m._21 = cp * sy;  m._22 = sr * sp * sy + cr * cy;   m._23 = cr * sp * sy + -sr * cy;
			m._31 = -sp;      m._32 = sr * cp;                  m._33 = cr * cp;
			                                                                                       m._44 = 1.0f;*/

			//this one I made from YRot(yaw) * XRot(pitch) * ZRot(roll) premultiplied
			m._11 =  cy * cr + sy * sp * sr;  m._12 =    cy * (-sr) + sy * sp * cr;  m._13 = sy * cp;  m._14 = 0.0f;  
			m._21 =                 cp * sr;  m._22 =                      cp * cr;  m._23 =     -sp;  m._24 = 0.0f;  
			m._31 = -sy * cr + cy * sp * sr;  m._32 = (-sy) * (-sr) + cy * sp * cr;  m._33 = cy * cp;  m._34 = 0.0f;  
			m._41 =                    0.0f;  m._42 =                         0.0f;  m._43 =    0.0f;  m._44 = 1.0f;

			return m;
		}

		static mat4 axisRotation(const vec3f& axis, float theta) {
			vec3f a = axis.normal();
			float c = cos(theta);
			float s = sin(theta);
			float mc = 1.0f - c;

			mat4 m;

			m._11 = a.x * a.x * mc + c;        m._12 = a.x * a.y * mc - a.z * s;  m._13 = a.x * a.z * mc + a.y * s;  m._14 = 0.0f;
			m._21 = a.y * a.x * mc + a.z * s;  m._22 = a.y * a.y * mc + c;        m._23 = a.y * a.z * mc - a.x * s;  m._24 = 0.0f;
			m._31 = a.z * a.x * mc - a.y * s;  m._32 = a.z * a.y * mc + a.x * s;  m._33 = a.z * a.z * mc + c;        m._34 = 0.0f;
			m._41 = 0.0f;                      m._42 = 0.0f;                      m._43 = 0.0f;                      m._44 = 1.0f;

			return m;
		}

		/*static mat4 quternionRotation(const quat& q) {
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

			mat4 m;

			m._11 = 1.0f - (yy + zz);  m._12 = xy - wz;           m._13 = xz + wy;
			m._21 = xy * wz;           m._22 = 1.0f - (xx + zz);  m._23 = yz - wx;
			m._31 = wx - wy;           m._32 = yz + wx;           m._33 = 1.0f - (xx + yy);
			                                                                                 m._44 = 1.0f;

			return m;
		}*/

		//projections
		static mat4 perspectiveProjection(float fovY, float aspect, float nZ, float fZ) {
			float halfHeight = tan(M_PI / 180.0f * (fovY / 2.0f)) * nZ;
			float halfWidth = halfHeight * aspect;

			return frustumProjection(-halfWidth, halfWidth, -halfHeight, halfHeight, nZ, fZ);
		}

		static mat4 frustumProjection(float left, float right, float bottom, float top, float nZ, float fZ) {
			mat4 m;

			float w = right - left;
			float h = top - bottom;
			float d = fZ - nZ;

			m._11 = (2.0f * nZ) / w;     m._12 = 0.0f;                m._13 = 0.0f;                    m._14 = 0.0f;
			m._21 = 0.0f;                m._22 = (2.0f * nZ) / h;     m._23 = 0.0f;                    m._24 = 0.0f;
			m._31 = (right + left) / w;  m._32 = (top + bottom) / h;  m._33 = -(fZ + nZ) / d;          m._34 = -1.0f;
			m._41 = 0.0f;                m._42 = 0.0f;                m._43 = -(2.0f * fZ * nZ) / d;   m._44 = 0.0f;

			return m;
		}

		static mat4 orthographicProjection(float left, float right, float bottom, float top, float nZ = -1.0f, float fZ = 1.0f) {
			mat4 m;

			float w = right - left;
			float h = top - bottom;
			float d = fZ - nZ;

			m._11 = 2.0f / w;             m._12 = 0.0f;                 m._13 = 0.0f;            m._14 = 0.0f;
			m._21 = 0.0f;                 m._22 = 2.0f / h;             m._23 = 0.0f;            m._24 = 0.0f;
			m._31 = 0.0f;                 m._32 = 0.0f;                 m._33 = -2.0f / d;       m._34 = 0.0f;
			m._41 = -(right + left) / w;  m._42 = -(top + bottom) / h;  m._43 = -(fZ + nZ) / d;  m._44 = 1.0f;

			return m;
		}

		static mat4 viewLookAt(const vec3f& eye, const vec3f& center, const vec3f& up = vec3f(0.0f, 1.0f, 0.0f)) {
			vec3f dir = (center - eye).normal();
			vec3f right = (dir ^ up).normal();
			vec3f u = right ^ dir;
			//printf("view look at u length: %f\n", u.length()); //debug

			mat4 m;

			//in-place translate by -eye
			m._11 = right.x;  m._12 = right.y;  m._13 = right.z;  m._14 = -(m._11 * eye.x + m._12 * eye.y + m._13 * eye.z);
			m._21 = u.x;      m._22 = u.y;      m._23 = u.z;      m._24 = -(m._21 * eye.x + m._22 * eye.y + m._23 * eye.z);
			m._31 = -dir.x;   m._32 = -dir.y;   m._33 = -dir.z;   m._34 = -(m._31 * eye.x + m._32 * eye.y + m._33 * eye.z);
			m._41 = 0.0f;     m._42 = 0.0f;     m._43 = 0.0f;     m._44 = 1.0f;

			return m;
		}

		static mat4 viewBillboard(const vec3f& eye, const vec3f& dir, const vec3f& up = vec3f(0.0f, 1.0f, 0.0f)) {
			float angle = up * dir;
			vec3f u = (up - dir * angle).normal();
			vec3f right = up ^ dir;

			mat4 m;

			m._11 = right.x;  m._12 = right.y;  m._13 = right.z;  m._14 = 0.0f;
			m._21 = u.x;      m._22 = u.y;      m._23 = u.z;      m._24 = 0.0f;
			m._31 = dir.x;    m._32 = dir.y;    m._33 = dir.z;    m._34 = 0.0f;
			m._41 = eye.x;    m._42 = eye.y;    m._43 = eye.z;    m._44 = 1.0f;

			return m;
		}
};

#endif
