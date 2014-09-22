#ifndef VEC_MATH_VEC_H
#define VEC_MATH_VEC_H

#include <cmath>
#include <sstream>
#include <cstdio>

#include <functional>

/*================================================================*/
/*================================================================*/
/*================================================================*/
/*================================================================*/
#define UNARY_SCALAR_OP(op) \
	template <typename S> \
	void operator op(const S& f){ \
		x op f; \
		y op f; \
	}

#define UNARY_CORRESPONDING_OP(op) \
	template <typename S> \
	void operator op(const vec2T<S>& v2){ \
		x op v2.x; \
		y op v2.y; \
	}

#define CORRESPONDING_OP(op) \
	template <typename S> \
	vec2T<T> operator op(const vec2T<S>& v2) const { \
		return vec2T<T>(x op v2.x, y op v2.y); \
	}

#define SCALAR_OP(op) \
	template <typename S> \
	vec2T<T> operator op(const S& f) const { \
		return vec2T<T>(x op f, y op f); \
	}

#define TEST_OP(op, connect) \
	bool operator op(const vec2T<T>& v2) const { \
		return (x op v2.x connect y op v2.y); \
	}

template<typename T>
struct vec2T{
	union{
		T data[2];
		struct{
			T x, y;
		};
	};

	vec2T(){
		x = y = T(0);
	}

	template<typename X>
	vec2T(const X& _f){
		x = _f;
		y = _f;
	}

	template<typename X, typename Y>
	vec2T(const X& _x, const Y& _y){
		x = _x;
		y = _y;
	}

	template<class S>
	vec2T(const vec2T<S>& v) {
		x = v.x;
		y = v.y;
	}

	UNARY_CORRESPONDING_OP(=)

	void set(T xx, T yy) {
		x = xx;
		y = yy;
	}

	void normalize() {
		*this /= length();
	}

	UNARY_CORRESPONDING_OP(+=)
	UNARY_CORRESPONDING_OP(-=)
	UNARY_SCALAR_OP(*=)
	UNARY_SCALAR_OP(/=)

	template<class S>
	operator vec2T<S>(){
		return vec2T<S>((S)x, (S)y);
	}

	T length() const{
		return sqrt(sqrLength());
	}

	T sqrLength() const {
		return x * x + y * y;
	}

	vec2T<T> operator -() const {
		return vec2T<T>(-x, -y);
	}

	vec2T<T> normal() const {
		return *this / length();
	}

	vec2T<T> abs() const {
		return vec2T<T>(x < 0? -x : x, y < 0? -y : y);
	}	

	CORRESPONDING_OP(+)
	CORRESPONDING_OP(-)
	SCALAR_OP(*)
	SCALAR_OP(/)

	T operator *(const vec2T<T>& v2) const {
		return (x * v2.x + y * v2.y);
	}

	template<class S>
	T distanceTo(const vec2T<S>& v2) const {
		return (v2 - *this).length();
	}

	T angleWith(vec2T<T>& v2) {
		return acos((*this * v2) / (length() * v2.length()));
	}

	TEST_OP(==, and)
	TEST_OP(!=, or)
	TEST_OP(<, and)
	TEST_OP(<=, and)
	TEST_OP(>, and)
	TEST_OP(>=, and)

	T minComponent() const{
		return std::min(x, y);
	}

	T maxComponent() const{
		return std::max(x, y);
	}
};

/*================================================================*/
/*================================================================*/
/*================================================================*/
/*================================================================*/

#undef UNARY_SCALAR_OP
#define UNARY_SCALAR_OP(op) \
	template <typename S> \
	void operator op(const S& f){ \
		x op f; \
		y op f; \
		z op f; \
	}

#undef UNARY_CORRESPONDING_OP
#define UNARY_CORRESPONDING_OP(op) \
	template <typename S> \
	void operator op(const vec3T<S>& v2){ \
		x op v2.x; \
		y op v2.y; \
		z op v2.z; \
	}

#undef CORRESPONDING_OP
#define CORRESPONDING_OP(op) \
	template <typename S> \
	vec3T<T> operator op(const vec3T<S>& v2) const { \
		return vec3T<T>(x op v2.x, y op v2.y, z op v2.z); \
	}

#undef SCALAR_OP
#define SCALAR_OP(op) \
	template <typename S> \
	vec3T<T> operator op(const S& f) const { \
		return vec3T<T>(x op f, y op f, z op f); \
	}

#undef TEST_OP
#define TEST_OP(op, connect) \
	bool operator op(const vec3T<T>& v2) const { \
		return (x op v2.x connect y op v2.y connect z op v2.z); \
	}

template<typename T>
struct vec3T{
	union{
		T data[3];
		struct{
			T x, y, z;
		};
		struct{
			vec2T<T> xy;
			T __pad;
		};
		struct{
			T ___pad;
			vec2T<T> yz;
		};
	};

	vec3T(){
		x = y = z = T(0);
	}

	template<typename X>
	vec3T(const X& _f){
		x = _f;
		y = _f;
		z = _f;
	}

	template<typename X, typename Y, typename Z>
	vec3T(const X& _x, const Y& _y, const Z& _z){
		x = _x;
		y = _y;
		z = _z;
	}

	template<class S>
	vec3T(const vec3T<S>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
	}

	template<class S, class Y>
	vec3T(const vec2T<S>& v, const Y& zz) {
		x = v.x;
		y = v.y;
		z = zz;
	}

	template<class S, class Y>
	vec3T(const Y& xx, const vec2T<S>& v) {
		x = xx;
		y = v.x;
		z = v.y;
	}

	UNARY_CORRESPONDING_OP(=)

	void set(T xx, T yy, T zz) {
		x = xx;
		y = yy;
		z = zz;
	}

	void normalize() {
		*this /= length();
	}

	UNARY_CORRESPONDING_OP(+=)
	UNARY_CORRESPONDING_OP(-=)
	UNARY_SCALAR_OP(*=)
	UNARY_SCALAR_OP(/=)

	template<class S>
	operator vec3T<S>(){
		return vec3T<S>((S)x, (S)y, (S)z);
	}

	T length() const{
		return sqrt(sqrLength());
	}

	T sqrLength() const {
		return x * x + y * y + z * z;
	}

	vec3T<T> operator -() const {
		return vec3T<T>(-x, -y, -z);
	}

	vec3T<T> normal() const {
		return *this / length();
	}

	vec3T<T> abs() const {
		return vec3T<T>(x < 0? -x : x, y < 0? -y : y, z < 0? -z : z);
	}

	CORRESPONDING_OP(+)
	CORRESPONDING_OP(-)
	SCALAR_OP(*)
	SCALAR_OP(/)

	T operator *(const vec3T<T>& v2) const {
		return (x * v2.x + y * v2.y + z * v2.z);
	}

	template<class S>
	T distanceTo(const vec3T<S>& v2) const {
		return (v2 - *this).length();
	}

	T angleWith(vec3T<T>& v2) {
		return acos((*this * v2) / (length() * v2.length()));
	}

	vec3T<T> operator ^(const vec3T<T>& v2) const {
		return vec3T<T>(y * v2.z - z * v2.y,
		                z * v2.x - x * v2.z,
		                x * v2.y - y * v2.x);
	}

	TEST_OP(==, and)
	TEST_OP(!=, or)
	TEST_OP(<, and)
	TEST_OP(<=, and)
	TEST_OP(>, and)
	TEST_OP(>=, and)

	T minComponent() const{
		return std::min(std::min(x, y), z);
	}

	T maxComponent() const{
		return std::max(std::max(x, y), z);
	}
};

/*================================================================*/
/*================================================================*/
/*================================================================*/
/*================================================================*/

#undef UNARY_SCALAR_OP
#define UNARY_SCALAR_OP(op) \
	template <typename S> \
	void operator op(const S& f){ \
		x op f; \
		y op f; \
		z op f; \
		w op f; \
	}

#undef UNARY_CORRESPONDING_OP
#define UNARY_CORRESPONDING_OP(op) \
	template <typename S> \
	void operator op(const vec4T<S>& v2){ \
		x op v2.x; \
		y op v2.y; \
		z op v2.z; \
		w op v2.w; \
	}

#undef CORRESPONDING_OP
#define CORRESPONDING_OP(op) \
	template <typename S> \
	vec4T<T> operator op(const vec4T<S>& v2) const { \
		return vec4T<T>(x op v2.x, y op v2.y, z op v2.z, w op v2.w); \
	}

#undef SCALAR_OP
#define SCALAR_OP(op) \
	template <typename S> \
	vec4T<T> operator op(const S& f) const { \
		return vec4T<T>(x op f, y op f, z op f, w op f); \
	}

#undef TEST_OP
#define TEST_OP(op, connect) \
	bool operator op(const vec4T<T>& v2) const { \
		return (x op v2.x connect y op v2.y connect z op v2.z connect w op v2.w); \
	}

template<typename T>
struct vec4T{
	union{
		T data[4];
		struct{
			T x, y, z, w;
		};
		struct{
			vec2T<T> xy;
			vec2T<T> zw;
		};
		struct{
			T __pad;
			vec2T<T> yz;
			T ___pad;
		};
		struct{
			vec3T<T> xyz;
			T ____pad;
		};
		struct{
			T _____pad;
			vec3T<T> yzw;
		};
	};

	vec4T(){
		x = y = z = w = T(0);
	}

	template<typename X>
	vec4T(const X& _f){
		x = _f;
		y = _f;
		z = _f;
	}

	template<typename X, typename Y, typename Z, typename W>
	vec4T(const X& _x, const Y& _y, const Z& _z, const W& _w){
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}

	template<class S>
	vec4T(const vec4T<S>& v) {
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	UNARY_CORRESPONDING_OP(=)

	void set(T xx, T yy, T zz, T ww) {
		x = xx;
		y = yy;
		z = zz;
		w = ww;
	}

	void normalize() {
		*this /= length();
	}

	UNARY_CORRESPONDING_OP(+=)
	UNARY_CORRESPONDING_OP(-=)
	UNARY_SCALAR_OP(*=)
	UNARY_SCALAR_OP(/=)

	template<class S>
	operator vec4T<S>(){
		return vec4T<S>((S)x, (S)y, (S)z, (S)w);
	}

	T length() const{
		return sqrt(sqrLength());
	}

	T sqrLength() const {
		return x * x + y * y + z * z + w * w;
	}

	vec4T<T> operator -() const {
		return vec4T<T>(-x, -y, -z, -w);
	}

	vec4T<T> normal() const {
		return *this / length();
	}

	vec4T<T> abs() const {
		return vec4T<T>(x < 0? -x : x, y < 0? -y : y, z < 0? -z : z, w < 0? -w : w);
	}

	CORRESPONDING_OP(+)
	CORRESPONDING_OP(-)
	SCALAR_OP(*)
	SCALAR_OP(/)

	T operator *(const vec4T<T>& v2) const {
		return (x * v2.x + y * v2.y + z * v2.z + w * v2.w);
	}

	TEST_OP(==, and)
	TEST_OP(!=, or)
	TEST_OP(<, and)
	TEST_OP(<=, and)
	TEST_OP(>, and)
	TEST_OP(>=, and)

	T minComponent() const{
		return std::min(std::min(x, y), std::min(z, w));
	}

	T maxComponent() const{
		return std::max(std::max(x, y), std::max(z, w));
	}
};

/*================================================================*/
/*================================================================*/
/*================================================================*/
/*================================================================*/

typedef vec2T<float> vec2;
typedef vec3T<float> vec3;
typedef vec4T<float> vec4;

typedef vec2T<double> dvec2;
typedef vec3T<double> dvec3;
typedef vec4T<double> dvec4;

typedef vec2T<int> ivec2;
typedef vec3T<int> ivec3;
typedef vec4T<int> ivec4;


template<typename T>
inline vec2T<T> minVec(const vec2T<T>& v1, const vec2T<T>& v2) {
	vec2T<T> v(v1);
	if (v2.x < v1.x) v.x = v2.x;
	if (v2.y < v1.y) v.y = v2.y;

	return v;
}

template<typename T>
inline vec2T<T> maxVec(const vec2T<T>& v1, const vec2T<T>& v2) {
	vec2T<T> v(v1);
	if (v2.x > v1.x) v.x = v2.x;
	if (v2.y > v1.y) v.y = v2.y;

	return v;
}

template<typename T>
inline vec3T<T> minVec(const vec3T<T>& v1, const vec3T<T>& v2) {
	vec3T<T> v(v1);
	if (v2.x < v1.x) v.x = v2.x;
	if (v2.y < v1.y) v.y = v2.y;
	if (v2.z < v1.z) v.z = v2.z;

	return v;
}

template<typename T>
inline vec3T<T> maxVec(const vec3T<T>& v1, const vec3T<T>& v2) {
	vec3T<T> v(v1);
	if (v2.x > v1.x) v.x = v2.x;
	if (v2.y > v1.y) v.y = v2.y;
	if (v2.z > v1.z) v.z = v2.z;

	return v;
}

template<typename T>
inline vec4T<T> minVec(const vec4T<T>& v1, const vec4T<T>& v2) {
	vec4T<T> v(v1);
	if (v2.x < v1.x) v.x = v2.x;
	if (v2.y < v1.y) v.y = v2.y;
	if (v2.z < v1.z) v.z = v2.z;
	if (v2.w < v1.w) v.w = v2.w;

	return v;
}

template<typename T>
inline vec4T<T> maxVec(const vec4T<T>& v1, const vec4T<T>& v2) {
	vec4T<T> v(v1);
	if (v2.x > v1.x) v.x = v2.x;
	if (v2.y > v1.y) v.y = v2.y;
	if (v2.z > v1.z) v.z = v2.z;
	if (v2.w > v1.w) v.w = v2.w;

	return v;
}


namespace std {
	//specify a hashing function for vec3
	//this gives good results for small numbers for ivec3, see hash_analysis.py
	//TODO: this will fail for vec3 since shifting is not defined for floats, so type specify this function
	template <typename T> 
	struct hash<vec3T<T>>{
		#ifndef ROTATE_LEFT
		#define ROTATE_LEFT(x, amount) ((x << amount) | (x >> (sizeof(x) * 8 - amount)))
		#endif

		std::size_t operator()(const vec3T<T>& k) const {
			//x = 31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 
			//y = 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00 31 30 29 28 27 26 25 24 23 22 
			//z = 10 09 08 07 06 05 04 03 02 01 00 31 30 29 28 27 26 25 24 23 22 31 20 19 18 17 16 15 14 13 12 11 
			using std::hash;
			return hash<T>()((k.x) ^ ROTATE_LEFT(k.y, 10) ^ ROTATE_LEFT(k.z, 21));
		}
	};
}
#endif

