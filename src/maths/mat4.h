#ifndef VEC_MATH_MAT4_H
#define VEC_MATH_MAT4_H

#include <cstring>
#include <cassert>

#include <string>

#define _USE_MATH_DEFINES 1 //for msvcc, to have M_PI and its brethren defined (hopefully the same as for gcc)
#include <cmath>

#include "vec.h"
#include "quat.h"
//class quat;

template<typename T>
struct mat4T {
	public:
		union {
				struct {
						//opengl uses column major
						T _11, _21, _31, _41;
						T _12, _22, _32, _42;
						T _13, _23, _33, _43;
						T _14, _24, _34, _44;
						
						/*row major
						T _11, _12, _13, _14;
						T _21, _22, _23, _24;
						T _31, _32, _33, _34;
						T _41, _42, _43, _44;*/
				};

				//T rows[4][4];
				T cols[4][4];
				T data[16]; //don't reference this
		};

		mat4T() {
			memset(data, 0, 16 * sizeof(T));
		}

		template <typename S>
		mat4T(const mat4T<S>& m) {
			for (int i = 0; i < 16; i++) data[i] = m.data[i];
		}

		template <typename S>
		void operator =(const mat4T<S>& m) {
			for (int i = 0; i < 16; i++) data[i] = m.data[i];
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

		template <typename S>
		operator mat4T<S>(){
			mat4T<S> m;
			for (int i = 0; i < 16; i++) m.data[i] = data[i];
			return m;
		}

		template<typename S>
		mat4T<T> operator *(const mat4T<S>& m) const {
			mat4T<T> ret;

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

		template<typename S>
		vec3T<S> multVec3(const vec3T<S>& v, T vw = 1) const {
			return vec3T<S>(_11 * v.x + _12 * v.y + _13 * v.z + _14 * vw, 
			                _21 * v.x + _22 * v.y + _23 * v.z + _24 * vw,
			                _31 * v.x + _32 * v.y + _33 * v.z + _34 * vw);
		}

		template<typename S>
		vec4T<S> operator *(const vec4T<S>& v) const {
			return vec4T<S>(_11 * v.x + _12 * v.y + _13 * v.z + _14 * v.w, 
			                _21 * v.x + _22 * v.y + _23 * v.z + _24 * v.w,
			                _31 * v.x + _32 * v.y + _33 * v.z + _34 * v.w,
			                _41 * v.x + _42 * v.y + _43 * v.z + _44 * v.w); 
		}

		template<typename S>
		mat4T<T> operator *(S f) const {
			mat4T<T> ret;
			for (int i = 0; i < 16; i++){
				ret.data[i] = data[i] * f;
			}

			return ret;
		}

		template<typename S>
		mat4T<T> operator /(S f) const {
			mat4T<T> ret;
			for (int i = 0; i < 16; i++){
				ret.data[i] = data[i] / f;
			}

			return ret;
		}

		vec4T<T> getCol(int col) const {
			assert(col >= 0 and col < 4);
			//return vec4T<T>(rows[0][col], rows[1][col], rows[2][col], rows[3][col]); //row major
			return vec4T<T>(cols[col][0], cols[col][1], cols[col][2], cols[col][3]); //column major
		}

		vec4T<T> getRow(int row) const {
			assert(row >= 0 and row < 4);
			//return vec4T<T>(rows[row][0], rows[row][1], rows[row][2], rows[row][3]); //row major
			return vec4T<T>(cols[0][row], cols[1][row], cols[2][row], cols[3][row]); //column major
		}

		mat4T<T> transpose() const{
			mat4T<T> m;

			m._11 = _11; m._12 = _21; m._13 = _31; m._14 = _41;
			m._21 = _12; m._22 = _22; m._23 = _32; m._24 = _42;
			m._31 = _13; m._32 = _23; m._33 = _33; m._34 = _43;
			m._41 = _14; m._42 = _24; m._43 = _34; m._44 = _44;

			return m;
		}

		mat4T<T> inverse(bool& success) const{
			//
			//  -1     T
			// A   = (C ) / det(A)
			//
			// where C is the matrix of cofactors

			mat4T<T> cofactors;

			//in-place transpose
			cofactors._11 =  ((_22 * _33 * _44 + _32 * _43 * _24 + _42 * _23 * _34) - (_42 * _33 * _24 + _32 * _23 * _44 + _22 * _43 * _34));
			cofactors._12 = -((_12 * _33 * _44 + _32 * _43 * _14 + _42 * _13 * _34) - (_42 * _33 * _14 + _32 * _13 * _44 + _12 * _43 * _34));
			cofactors._13 =  ((_12 * _23 * _44 + _22 * _43 * _14 + _42 * _13 * _24) - (_42 * _23 * _14 + _22 * _13 * _44 + _12 * _43 * _24));
			cofactors._14 = -((_12 * _23 * _34 + _22 * _33 * _14 + _32 * _13 * _24) - (_32 * _23 * _14 + _22 * _13 * _34 + _12 * _33 * _24));

			cofactors._21 = -((_21 * _33 * _44 + _31 * _43 * _24 + _41 * _23 * _34) - (_41 * _33 * _24 + _31 * _23 * _44 + _21 * _43 * _34));
			cofactors._22 =  ((_11 * _33 * _44 + _31 * _43 * _14 + _41 * _13 * _34) - (_41 * _33 * _14 + _31 * _13 * _44 + _11 * _43 * _34));
			cofactors._23 = -((_11 * _23 * _44 + _21 * _43 * _14 + _41 * _13 * _24) - (_41 * _23 * _14 + _21 * _13 * _44 + _11 * _43 * _24));
			cofactors._24 =  ((_11 * _23 * _34 + _21 * _33 * _14 + _31 * _13 * _24) - (_31 * _23 * _14 + _21 * _13 * _34 + _11 * _33 * _24));

			cofactors._31 =  ((_21 * _32 * _44 + _31 * _42 * _24 + _41 * _22 * _34) - (_41 * _32 * _24 + _31 * _22 * _44 + _21 * _42 * _34));
			cofactors._32 = -((_11 * _32 * _44 + _31 * _42 * _14 + _41 * _12 * _34) - (_41 * _32 * _14 + _31 * _12 * _44 + _11 * _42 * _34));
			cofactors._33 =  ((_11 * _22 * _44 + _21 * _42 * _14 + _41 * _12 * _24) - (_41 * _22 * _14 + _21 * _12 * _44 + _11 * _42 * _24));
			cofactors._34 = -((_11 * _22 * _34 + _21 * _32 * _14 + _31 * _12 * _24) - (_31 * _22 * _14 + _21 * _12 * _34 + _11 * _32 * _24));

			cofactors._41 = -((_21 * _32 * _43 + _31 * _42 * _23 + _41 * _22 * _33) - (_41 * _32 * _23 + _31 * _22 * _43 + _21 * _42 * _33));
			cofactors._42 =  ((_11 * _32 * _43 + _31 * _42 * _13 + _41 * _12 * _33) - (_41 * _32 * _13 + _31 * _12 * _43 + _11 * _42 * _33));
			cofactors._43 = -((_11 * _22 * _43 + _21 * _42 * _13 + _41 * _12 * _23) - (_41 * _22 * _13 + _21 * _12 * _43 + _11 * _42 * _23));
			cofactors._44 =  ((_11 * _22 * _33 + _21 * _32 * _13 + _31 * _12 * _23) - (_31 * _22 * _13 + _21 * _12 * _33 + _11 * _32 * _23));
			
			//calculate determinant
			//since we already have the cofactors, just multiply the top row elements
			//by their corresponding cofactors (which are also negated as needed)
			//and add them together
			T det = _11 * cofactors._11 + _12 * cofactors._21 + _13 * cofactors._31 + _14 * cofactors._41;

			//a zero (or close to) determinant means the matrix does not have an inverse
			success = (fabs(det) > 1e-5);
			return cofactors / det;
		}

		void operator +=(const mat4T<T>& m) {
			for (int i = 0; i < 16; i++)
				data[i] += m.data[i];
		}

		void operator -=(const mat4T<T>& m) {
			for (int i = 0; i < 16; i++)
				data[i] -= m.data[i];
		}

		void operator *=(const mat4T<T>& m) {
			*this = *this * m;
		}

		void operator *=(T f) {
			for (int i = 0; i < 16; i++)
				data[i] *= f;
		}

		void operator /=(T f) {
			for (int i = 0; i < 16; i++)
				data[i] /= f;
		}

		//Generators
		
		static mat4T<T> identity() {
			mat4T<T> m;

			m._11 = T(1);
			               m._22 = T(1);
			                              m._33 = T(1);
			                                             m._44 = T(1);

			return m;
		}
		
		static mat4T<T> translation(const vec3& v) {
			mat4T<T> m;

			m._11 = T(1);                                m._14 = v.x;
			               m._22 = T(1);                 m._24 = v.y;
			                              m._33 = T(1);  m._34 = v.z;
			                                             m._44 = T(1);

			return m;
		}

		static mat4T<T> scale(const vec3& v) {
			mat4T<T> m;

			m._11 = v.x;                               
			               m._22 = v.y;              
			                              m._33 = v.z;
			                                            m._44 = T(1);

			return m;
		}

		static mat4T<T> scale(T s){
			return scale(vec3T<T>(s));
		}

		static mat4T<T> XAxisRotation(T theta) {
			T c = cos(theta);
			T s = sin(theta);

			// 1  0  0  0
			// 0  c -s  0
			// 0  s  c  0
			// 0  0  0  1

			mat4T<T> m; //all 0

			m._11 = T(1);
			               m._22 = c;     m._23 = -s;
			               m._32 = s;     m._33 = c;
			                                           m._44 = T(1);

			return m;
		}

		static mat4T<T> YAxisRotation(T theta) {
			T c = cos(theta);
			T s = sin(theta);

			//  c  0  s  0
			//  0  1  0  0
			// -s  0  c  0
			//  0  0  0  1

			mat4T<T> m; //all 0

			m._11 = c;                  m._13 = s;
			             m._22 = T(1);  
			m._31 = -s;                 m._33 = c;
			                                        m._44 = T(1);

			return m;
		}

		static mat4T<T> ZAxisRotation(T theta) {
			T c = cos(theta);
			T s = sin(theta);

			// c -s  0  0
			// s  c  0  0
			// 0  0  1  0
			// 0  0  0  1

			mat4T<T> m; //all 0

			m._11 = c;   m._12 = -s;
			m._21 = s;   m._22 = c;  
			                           m._33 = T(1);
			                                          m._44 = T(1);

			return m;
		}

		static mat4T<T> flightRotation(T heading, T attitude, T bank){
			T sr = sin(bank);
			T cr = cos(bank);

			T sp = sin(attitude);
			T cp = cos(attitude);

			T sy = sin(heading);
			T cy = cos(heading);

			//mat4T<T> m;

			/* not sure where I got this from, but it looks wrong
			m._11 = cp * cy;  m._12 = sr * sp * cy + cr * -sy;  m._13 = cr * sp * cy + -sr * -sy;
			m._21 = cp * sy;  m._22 = sr * sp * sy + cr * cy;   m._23 = cr * sp * sy + -sr * cy;
			m._31 = -sp;      m._32 = sr * cp;                  m._33 = cr * cp;
			                                                                                       m._44 = T(1);*/

			/*//this one I made from YRot(heading) * XRot(attitude) * ZRot(bank) premultiplied
			m._11 =  cy * cr + sy * sp * sr;  m._12 =    cy * (-sr) + sy * sp * cr;  m._13 = sy * cp;  m._14 = T(0);  
			m._21 =                 cp * sr;  m._22 =                      cp * cr;  m._23 =     -sp;  m._24 = T(0);  
			m._31 = -sy * cr + cy * sp * sr;  m._32 = (-sy) * (-sr) + cy * sp * cr;  m._33 = cy * cp;  m._34 = T(0);  
			m._41 =                    T(0);  m._42 =                         T(0);  m._43 =    T(0);  m._44 = T(1);*/

			//return m;

			//http://www.euclideanspace.com/maths/geometry/rotations/euler/index.htm
			return YAxisRotation(heading) * ZAxisRotation(attitude) * XAxisRotation(bank);
		}

		static mat4T<T> axisRotation(const vec3& axis, T theta) {
			vec3 a = axis.normal();
			T c = cos(theta);
			T s = sin(theta);
			T mc = T(1) - c;

			mat4T<T> m;

			m._11 = a.x * a.x * mc + c;        m._12 = a.x * a.y * mc - a.z * s;  m._13 = a.x * a.z * mc + a.y * s;  m._14 = T(0);
			m._21 = a.y * a.x * mc + a.z * s;  m._22 = a.y * a.y * mc + c;        m._23 = a.y * a.z * mc - a.x * s;  m._24 = T(0);
			m._31 = a.z * a.x * mc - a.y * s;  m._32 = a.z * a.y * mc + a.x * s;  m._33 = a.z * a.z * mc + c;        m._34 = T(0);
			m._41 = 0   ;                      m._42 = T(0);                      m._43 = T(0);                      m._44 = T(1);

			return m;
		}

		template <typename S>
		static mat4T<T> quaternionRotation(const quatT<S>& q) {
			T xx = q.x * q.x;
			T xy = q.x * q.y;
			T xz = q.x * q.z;

			T yy = q.y * q.y;
			T yz = q.y * q.z;
			T zz = q.z * q.z;

			T wx = q.w * q.x;
			T wy = q.w * q.y;
			T wz = q.w * q.z;

			mat4T<T> m;

			m._11 = 1 - 2 * (yy + zz);  m._12 = 2 * (xy - wz);      m._13 = 2 * (xz + wy);
			m._21 = 2 * (xy + wz);      m._22 = 1 - 2 * (xx + zz);  m._23 = 2 * (yz - wx);
			m._31 = 2 * (xz - wy);      m._32 = 2 * (yz + wx);      m._33 = 1 - 2 * (xx + yy);
			                                                                                    m._44 = 1;

			return m;
		}

		//projections
		static mat4T<T> perspectiveProjection(T fovY, T aspect, T nZ, T fZ) {
			T halfHeight = tan(M_PI / T(180) * (fovY / T(2))) * nZ;
			T halfWidth = halfHeight * aspect;

			return frustumProjection(-halfWidth, halfWidth, -halfHeight, halfHeight, nZ, fZ);
		}

		static mat4T<T> frustumProjection(T left, T right, T bottom, T top, T nZ, T fZ) {
			mat4T<T> m;

			T w = right - left;
			T h = top - bottom;
			T d = fZ - nZ;

			m._11 = (2 * nZ) / w;     m._12 = T(0);             m._13 = (right + left) / w;      m._14 = T(0);
			m._21 = T(0);             m._22 = (2 * nZ) / h;     m._23 = (top + bottom) / h;      m._24 = T(0);
			m._31 = T(0);             m._32 = T(0);             m._33 = -(fZ + nZ) / d;          m._34 = -(2 * fZ * nZ) / d;
			m._41 = T(0);             m._42 = T(0);             m._43 = T(-1);                   m._44 = T(0);

			return m;
		}

		static mat4T<T> orthographicProjection(T left, T right, T bottom, T top, T nZ = T(-1), T fZ = T(1)) {
			mat4T<T> m;

			T w = right - left;
			T h = top - bottom;
			T d = fZ - nZ;

			m._11 = T(2) / w;   m._12 = T(0);      m._13 = T(0);       m._14 = -(right + left) / w;
			m._21 = T(0);       m._22 = T(2) / h;  m._23 = T(0);       m._24 = -(top + bottom) / h;
			m._31 = T(0);       m._32 = T(0);      m._33 = T(-2) / d;  m._34 = -(fZ + nZ) / d;
			m._41 = T(0);       m._42 = T(0);      m._43 = T(0);       m._44 = T(1);

			return m;
		}

		static mat4T<T> viewLookAt(const vec3T<T>& eye, const vec3T<T>& center, const vec3T<T>& up = vec3T<T>(0, 1, 0)) {
			/*vec3 dir = (center - eye).normal();
			vec3 right = (dir ^ up).normal();
			vec3 u = right ^ dir;

			mat4T<T> m;

			//in-place translate by -eye
			m._11 = right.x;  m._12 = right.y;  m._13 = right.z;  m._14 = -(m._11 * eye.x + m._12 * eye.y + m._13 * eye.z);
			m._21 = u.x;      m._22 = u.y;      m._23 = u.z;      m._24 = -(m._21 * eye.x + m._22 * eye.y + m._23 * eye.z);
			m._31 = -dir.x;   m._32 = -dir.y;   m._33 = -dir.z;   m._34 = -(m._31 * eye.x + m._32 * eye.y + m._33 * eye.z);
			m._41 = T(0);     m._42 = T(0);     m._43 = T(0);     m._44 = T(1);

			return m;*/


			/*vec3 dir = (eye - center).normal();
			vec3 right = (up ^ dir).normal();
			vec3 u = dir ^ right;*/

			vec3 dir = (center - eye).normal();
			vec3 right = (dir ^ up).normal();
			vec3 u = right ^ dir;

			mat4T<T> m;

			m._11 = right.x;  m._12 = right.y;  m._13 = right.z;  m._14 = T(0);
			m._21 = u.x;      m._22 = u.y;      m._23 = u.z;      m._24 = T(0);
			m._31 = -dir.x;   m._32 = -dir.y;   m._33 = -dir.z;   m._34 = T(0);
			m._41 = T(0);     m._42 = T(0);     m._43 = T(0);     m._44 = T(1);

			return m * mat4T<T>::translation(-eye);
		}

		static mat4T<T> viewBillboard(const vec3T<T>& eye, const vec3T<T>& dir, const vec3T<T>& up = vec3(0, 1, 0)) {
			T angle = up * dir;
			vec3 u = (up - dir * angle).normal();
			vec3 right = up ^ dir;

			mat4T<T> m;

			m._11 = right.x;  m._12 = right.y;  m._13 = right.z;  m._14 = T(0);
			m._21 = u.x;      m._22 = u.y;      m._23 = u.z;      m._24 = T(0);
			m._31 = -dir.x;   m._32 = -dir.y;   m._33 = -dir.z;   m._34 = T(0);
			m._41 = eye.x;    m._42 = eye.y;    m._43 = eye.z;    m._44 = T(1);

			return m;
		}
};

typedef mat4T<float> mat4;
typedef mat4T<double> dmat4;

#endif

