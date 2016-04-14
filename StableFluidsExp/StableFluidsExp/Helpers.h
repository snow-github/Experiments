#include <iostream>
#include <iomanip>
#include <math.h>
#include "Utility\UTL_General.h"

using namespace std;

#define IXCURL(i,j) ((i) * (N + 2) + (j))
#define IX(i,j) ((i)*(N+2)+(j))
#define FOR_EACH_CELL for ( i=1 ; i<=N ; i++ ) { for ( j=1 ; j<=N ; j++ ) {
#define END_FOR }}

#ifdef __cplusplus__
#include <cstdlib>
#else
#include <stdlib.h>
#endif

void displayField(int row, int col, float * field);
void displayField(int row, int col, float * field, float * fieldOrigin);
void displayVectorField(int row, int col, float * u, float * v);
void displayVectorField(int row, int col, float * u, float * v, float * u0, float * v0);
void zeros(int N, float * field);
void scaler(int N, float *field, float factor);
double relative_error(int N, float * curr, float * prev);
void computeCurls_uniform(int N, float * w, float * u, float * v);
void find_vector_potential_2D(int N, float * u, float * v, float * psi);
void linear_combine_add(int N, float * f_out, float * f, float * f0);
void linear_combine_sub(int N, float * f_out, float * f, float * f0);
void computeDivergence_unifrom(int N, float * u, float * v, float * div);
float lerp(float t, float x0, float x1);
float bilerp(float t, float s, float x0, float x1, float y0, float y1);
float interpolate(int N, float x, float y, float * field);

template<class S, class T>
inline S _lerp(const S& value0, const S& value1, T f)
{
	return (1 - f)*value0 + f*value1;
}

template<class S, class T>
inline S _bilerp(const S& v00, const S& v10,
				const S& v01, const S& v11,
				T fx, T fy){
	return _lerp(_lerp(v00, v10, fx), _lerp(v01, v11, fx),  fy);
}

template<class T>
inline void get_barycentric(T x, int& i, T& f, int i_low, int i_high)
{
	T s = std::floor(x);
	i = (int)s;
	if (i<i_low){
		i = i_low;
		f = 0;
	}
	else if (i>i_high - 2){
		i = i_high - 2;
		f = 1;
	}
	else
		f = (T)(x - s);
}

template<class T>
const T& max(const T& a, const T& b)
{
	return (a < b) ? b : a;
}

template<class T>
const T& min(const T& a, const T& b)
{
	return (b < a) ? b : a;
}

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return max(lower, min(n, upper));
}

typedef struct _vec2{
	_vec2(float _x, float _y){
		x = _x; y = _y;
	}

	float x, y;
}vec2;

typedef struct _vec3{
	_vec3(float _x, float _y, float _z){
		x = _x; y = _y; z = _z;
	}
	float x, y, z;
}vec3;

typedef struct {
	float x, y;
	vec2 vel;
	vec3 color;
}Particle;