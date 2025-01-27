﻿#include "helpers.h"

void
add_source(int N, float * x, float * s, float dt){
	int i, size = (N + 2)*(N + 2);
	for (i = 0; i < size; i++) x[i] += dt*s[i];
}

void
set_boundaries(int N, int b, float * x){
	for (int i = 1; i <= N; i++) {
		x[IX(0, i)]		= b == 0 ? -x[IX(1, i)] : x[IX(1, i)];
		x[IX(N + 1, i)] = b == 0 ? -x[IX(N, i)] : x[IX(N, i)];
		x[IX(i, 0)]		= b == 0 ? -x[IX(i, 1)] : x[IX(i, 1)];
		x[IX(i, N + 1)] = b == -1 ? -x[IX(i, N)] : x[IX(i, N)];
	}
	x[IX(0, 0)]			= 0.5f * (x[IX(1, 0)]		+	x[IX(0, 1)]);
	x[IX(0, N + 1)]		= 0.5f * (x[IX(1, N + 1)]	+	x[IX(0, N)]);
	x[IX(N + 1, 0)]		= 0.5f * (x[IX(N, 0)]		+	x[IX(N + 1, 1)]);
	x[IX(N + 1, N + 1)] = 0.5f * (x[IX(N, N + 1)]	+	x[IX(N + 1, N)]);
}

void
Gauss_Seidel_solve(int N, int b, float * x, float * x0, float a, float c, int iterations){
	int k;

	double h = 1.0f / N;

	for (k = 0; k < iterations; k++) {
		LOOP_CELLS{
			x[IX(i, j)] = (x0[IX(i, j)] + a *
			(x[IX(i - 1, j)] + x[IX(i + 1, j)] +
			x[IX(i, j - 1)] + x[IX(i, j + 1)])) / c;
		}
		set_boundaries(N, b, x);
	}
}

void
Jacobi_solve(int N, int b, float * x, float * x0, float a, float c, int iterations){
	int k;
	int size = (N + 2) * (N + 2);
	float* aux = (float*)malloc(size*sizeof(float));
	double h = 1.0f / (N);

	for (k = 0; k < iterations; k++)
	{
		LOOP_CELLS{
			aux[IX(i, j)] = (x0[IX(i, j)] * (h * h) + a *
			(x[IX(i - 1, j)] + x[IX(i + 1, j)] +
			x[IX(i, j - 1)] + x[IX(i, j + 1)])) / c;
		}

		LOOP_CELLS{
			x[IX(i, j)] = aux[IX(i, j)];
		}
		set_boundaries(N, b, x);
	}

	free(aux);
}

void
Multigrid_solve(int N, float * x, float * x0, int nSmooth)
{
}

void
diffuse(int N, int b, float * x, float * x0, float diff, float dt){
	float a = dt*diff*N*N;
	Gauss_Seidel_solve(N, b, x, x0, a, 1 + 4 * a, 30);
}

vec2
get_velocity(int N, const vec2& position, float * u, float * v){
	float u0, v0;
	u0 = interpolate(N, position.x * N - 0.0f, position.y * N - 0.5f, u);
	v0 = interpolate(N, position.x * N - 0.5f, position.y * N - 0.0f, v);
	return vec2(u0, v0);
}

// Mark:
// TODO: Problems make the particles move to wrong direction
// Desc:
// Runge-Kutta 2nd order integration for ODEs
vec2 rk2(int N, float * u, float * v, const vec2& position, float dt){
	vec2 vel = get_velocity(N, position, u, v);
	vel = get_velocity(N, vec2(position.x + 0.5 * dt * vel.x, position.y + 0.5 * dt * vel.y), u, v);
	return vec2(position.x + dt * vel.x, position.y + dt * vel.y);
}

void
particles_advector(int N, float * u, float * v, Particle* particles, int num_particles, float dt){
	vec2 pos(0.0f, 0.0f);
	for (int i = 0; i != num_particles; i++){
		pos.x = pos.y = 0.0f;
		pos = rk2(N, u, v, vec2(particles[i].x, particles[i].y), dt);
		particles[i].x = pos.x; particles[i].y = pos.y;
		particles[i].vel = get_velocity(N, vec2(particles[i].x, particles[i].y), u, v);
	}
}

// Problems: Blow up : (
// 2nd order Runge-Kutta ODEs integrator for advection bugs
void
vector_advector_rk2(int N, float * u, float * u0, float * v, float * v0, float * u_tmp, float * v_tmp, float dt){
	// TODO: Runge Kutta integrator (2nd order)
}

void
scalar_advector(int N, float * d, float * d0, float * u, float * v, float dt){
	int i0, j0, i1, j1;
	float x, y, s1, t1, dt0;

	dt0 = dt*N;
	LOOP_CELLS {
		x = i - dt0*u[IX(i, j)];
		y = j - dt0*v[IX(i, j)];

		if (x < 0.5f) x = 0.5f;
		if (x > N + 0.5f) x = N + 0.5f;

		i0 = (int)x;
		i1 = i0 + 1;

		if (y < 0.5f) y = 0.5f;
		if (y > N + 0.5f) y = N + 0.5f;

		j0 = (int)y;
		j1 = j0 + 1;

		s1 = x - i0;
		t1 = y - j0;

		float top_x_dir_lerp = lerp(s1, d0[IX(i0, j0)], d0[IX(i1, j0)]);
		float bottom_x_dir_lerp = lerp(s1, d0[IX(i0, j1)], d0[IX(i1, j1)]);
		d[IX(i, j)] = lerp(t1, top_x_dir_lerp, bottom_x_dir_lerp);
	}
	set_boundaries(N, 0, d);
}

void
vector_advector(int N, float * d, float * d0, float * k, float * k0, float * u, float * v, float dt){
	int i0, j0, i1, j1;
	float x, y, s1, t1, dt0;

	// TODO: Modify this piece of code to archive 2nd Order RK
	dt0 = dt*N;
	LOOP_CELLS {
		x = i - dt0*u[IX(i, j)];
		y = j - dt0*v[IX(i, j)];

		if (x < 0.5f) x = 0.5f;
		if (x > N + 0.5f) x = N + 0.5f;

		i0 = (int)x;
		i1 = i0 + 1;

		if (y < 0.5f) y = 0.5f;
		if (y > N + 0.5f) y = N + 0.5f;

		j0 = (int)y;
		j1 = j0 + 1;

		s1 = x - i0;
		t1 = y - j0;

		d[IX(i, j)] = lerp(s1,
			lerp(t1, d0[IX(i0, j0)], d0[IX(i0, j1)]),
			lerp(t1, d0[IX(i1, j0)], d0[IX(i1, j1)]));

		k[IX(i, j)] = lerp(s1,
			lerp(t1, k0[IX(i0, j0)], k0[IX(i0, j1)]),
			lerp(t1, k0[IX(i1, j0)], k0[IX(i1, j1)]));
	}
	set_boundaries(N, 0, d);
	set_boundaries(N, 0, k);
}

void
project(int N, float * u, float * v, float * p, float * div){
	computeDivergence_unifrom(N, u, v, div);
	set_boundaries(N, 0, div);
	set_boundaries(N, 0, p);
	zeros(N, p);
	scaler(N, div, -1.0f);
	//Multigrid_solve(N, p, div, 10);
	Gauss_Seidel_solve(N, 0, p, div, 1, 4, 30);
	LOOP_CELLS{
		u[IX(i, j)] -= 0.5f*N*(p[IX(i + 1, j)] - p[IX(i - 1, j)]);
		v[IX(i, j)] -= 0.5f*N*(p[IX(i, j + 1)] - p[IX(i, j - 1)]);
	}
	set_boundaries(N, 0, u);
	set_boundaries(N, 0, v);
}

void
MoveScalarProperties(int N, float * x, float * x0, float * u, float * v, float diff, float dt){
	add_source(N, x, x0, dt);
	SWAP(x0, x); diffuse(N, 0, x, x0, diff, dt);
	SWAP(x0, x); scalar_advector(N, x, x0, u, v, dt);
}

void SemiLagAdvance(int N,
	Particle* particles, int num_particles,
	float * fx, float * fy,
	float * psi, float * du, float * dv, float * wn, float *dw, float * w_bar, float * w_star,
	float * u, float * v, float * u0, float * v0,
	float * t, float * t0,
	float visc,
	float dt){
	//// IVOCK advection
	zeros(N, wn);
	zeros(N, w_bar);
	zeros(N, w_star);
	zeros(N, dw);
	zeros(N, psi);
	zeros(N, du);
	zeros(N, dv);
	zeros(N, u0);
	zeros(N, v0);

	// Gravity
	int size = (N + 2) * (N + 2);
	float *g = (float*)malloc(size*sizeof(float));
	zeros(N, g);
	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			g[IX(i, j)] = -9.8;
		}
	}

	add_source(N, u, u0, dt);
	add_source(N, v0, g, dt);
	add_source(N, v, v0, dt);

	particles_advector(N, u, v, particles, num_particles, dt);

	SWAP(u0, u);
	SWAP(v0, v);
	diffuse(N, 0, u, u0, visc, dt);
	diffuse(N, 0, v, v0, visc, dt);
	project(N, u, v, u0, v0);
	zeros(N, u0);
	zeros(N, v0);
	SWAP(u0, u);
	SWAP(v0, v);

	computeCurls_uniform(N, wn, u0, v0);
	scalar_advector(N, w_bar, wn, u0, v0, dt);
	vector_advector(N, u, u0, v, v0, u0, v0, dt);
	computeCurls_uniform(N, w_star, u, v);
	linear_combine_sub(N, dw, w_bar, w_star);
	set_boundaries(N, 0, dw);
	scaler(N, dw, -1.0f);
	Jacobi_solve(N, 0, psi, dw, 1, 4, 30);
	find_vector_potential_2D(N, du, dv, psi);
	//linear_combine_add(N, u, u, du);
	//linear_combine_add(N, v, v, dv);
	free(g);
}

void IVOCKAdvance(int N,
	Particle* particles, int num_particles,
	float * fx, float * fy,
	float * psi, float * du, float * dv, float * wn, float *dw, float * w_bar, float * w_star,
	float * u, float * v, float * u0, float * v0,
	float * t, float * t0,
	float visc,
	float dt){

	// Only for debug

	//// IVOCK advection
	zeros(N, wn);
	zeros(N, w_bar);
	zeros(N, w_star);
	zeros(N, dw);
	zeros(N, psi);
	zeros(N, du);
	zeros(N, dv);
	zeros(N, u0);
	zeros(N, v0);

	// Gravity
	int size = (N + 2) * (N + 2);
	float *g = (float*)malloc(size*sizeof(float));
	zeros(N, g);
	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			g[IX(i, j)] = -9.8;
		}
	}

	add_source(N, u, u0, dt);
	add_source(N, v0, g, dt);
	add_source(N, v, v0, dt);

	particles_advector(N, u, v, particles, num_particles, dt);

	SWAP(u0, u);
	SWAP(v0, v);
	diffuse(N, 0, u, u0, visc, dt);
	diffuse(N, 0, v, v0, visc, dt);
	project(N, u, v, u0, v0);
	zeros(N, u0);
	zeros(N, v0);
	SWAP(u0, u);
	SWAP(v0, v);

	computeCurls_uniform(N, wn, u0, v0);
	scalar_advector(N, w_bar, wn, u0, v0, dt);
	vector_advector(N, u, u0, v, v0, u0, v0, dt);
	computeCurls_uniform(N, w_star, u, v);
	linear_combine_sub(N, dw, w_bar, w_star);
	set_boundaries(N, 0, dw);
	scaler(N, dw, -1.0f);
	Jacobi_solve(N, 0, psi, dw, 1, 4, 30);
	find_vector_potential_2D(N, du, dv, psi);
	linear_combine_add(N, u, u, du);
	linear_combine_add(N, v, v, dv);
	free(g);
}

void add_gravity(int N, float dt, float grav, float * field){
	// Gravity
	int size = (N + 2) * (N + 2);
	float *g = (float*)malloc(size*sizeof(float));
	zeros(N, g);
	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			g[IX(i, j)] = grav;
		}
	}
	add_source(N, field, g, dt);
	if (g) free(g);
}

void stream(float * f1, float * f2, float * f3, float * f4, 
			float * f5, float * f6, float * f7, float * f8,
			int N){

	int size = (N + 2) * (N + 2);
	float * tmpf1 = (float*)malloc(size * sizeof(float));
	float * tmpf2 = (float*)malloc(size * sizeof(float));
	float * tmpf3 = (float*)malloc(size * sizeof(float));
	float * tmpf4 = (float*)malloc(size * sizeof(float));
	float * tmpf5 = (float*)malloc(size * sizeof(float));
	float * tmpf6 = (float*)malloc(size * sizeof(float));
	float * tmpf7 = (float*)malloc(size * sizeof(float));
	float * tmpf8 = (float*)malloc(size * sizeof(float));

	LOOP_CELLS{
		tmpf1[IX(i, j)] = f1[IX(i - 1, j)];
		tmpf2[IX(i, j)] = f2[IX(i, j + 1)];
		tmpf3[IX(i, j)] = f3[IX(i + 1, j)];
		tmpf4[IX(i, j)] = f4[IX(i, j - 1)];
		tmpf5[IX(i, j)] = f5[IX(i - 1, j + 1)];
		tmpf6[IX(i, j)] = f6[IX(i + 1, j + 1)];
		tmpf7[IX(i, j)] = f7[IX(i + 1, j - 1)];
		tmpf8[IX(i, j)] = f8[IX(i - 1, j - 1)];
	}

	LOOP_CELLS{
		f1[IX(i, j)] = tmpf1[IX(i, j)];
		f2[IX(i, j)] = tmpf2[IX(i, j)];
		f3[IX(i, j)] = tmpf3[IX(i, j)];
		f4[IX(i, j)] = tmpf4[IX(i, j)];
		f5[IX(i, j)] = tmpf5[IX(i, j)];
		f6[IX(i, j)] = tmpf6[IX(i, j)];
		f7[IX(i, j)] = tmpf7[IX(i, j)];
		f8[IX(i, j)] = tmpf8[IX(i, j)];
	}

	if (tmpf1) free(tmpf1);
	if (tmpf2) free(tmpf2);
	if (tmpf3) free(tmpf3);
	if (tmpf4) free(tmpf4);
	if (tmpf5) free(tmpf5);
	if (tmpf6) free(tmpf6);
	if (tmpf7) free(tmpf7);
	if (tmpf8) free(tmpf8);
}

void collision(float * f0,
			   float * f1, float * f2, float * f3, float * f4,
			   float * f5, float * f6, float * f7, float * f8,
			   int N, float tau, float * out_u, float * out_v, float dt){

	assert(out_u != NULL && out_v != NULL);
	
	float rho, rho_u, rho_v, _u, _v;
	float eq0, eq1, eq2, eq3, eq4, eq5, eq6, eq7, eq8;
	float h = 1.f / N;
	float c = h / dt;
	float c2 = pow(c, 2);

	LOOP_CELLS{
		rho = f0[IX(i, j)] + 
			  f1[IX(i, j)] + f2[IX(i, j)] + f3[IX(i, j)] + f4[IX(i, j)] +
			  f5[IX(i, j)] + f6[IX(i, j)] + f7[IX(i, j)] + f8[IX(i, j)];

		rho_u = (f1[IX(i, j)] - f3[IX(i, j)] + f5[IX(i, j)] - f6[IX(i, j)] - f7[IX(i, j)] + f8[IX(i, j)]);
		rho_v = (f2[IX(i, j)] - f4[IX(i, j)] + f5[IX(i, j)] + f6[IX(i, j)] - f7[IX(i, j)] - f8[IX(i, j)]);
		
		_u = rho_u / rho;
		_v = rho_v / rho;

		out_u[IX(i, j)] = _u;
		out_v[IX(i, j)] = _v;

		float coef_vel = 1.5 * (pow(_u, 2) + pow(_v, 2));
		eq0 = rho * (4.f / 9.f) * (1.f - coef_vel);
		eq1 = rho * (1.f / 9.f) * (1.f + (3.f * _u) + (4.5f * pow(_u, 2)) - coef_vel);
		eq2 = rho * (1.f / 9.f) * (1.f + (3.f * _v) + (4.5f * pow(_v, 2)) - coef_vel);
		eq3 = rho * (1.f / 9.f) * (1.f - (3.f * _u) + (4.5f * pow(_u, 2)) - coef_vel);
		eq4 = rho * (1.f / 9.f) * (1.f - (3.f * _v) + (4.5f * pow(_v, 2)) - coef_vel);
		eq5 = rho * (1.f / 36.f) * (1.f + 3.f * ( _u + _v) + 4.5f * pow( _u + _v, 2) - coef_vel);
		eq6 = rho * (1.f / 36.f) * (1.f + 3.f * (-_u + _v) + 4.5f * pow(-_u + _v, 2) - coef_vel);
		eq7 = rho * (1.f / 36.f) * (1.f + 3.f * (-_u - _v) + 4.5f * pow(-_u - _v, 2) - coef_vel);
		eq8 = rho * (1.f / 36.f) * (1.f + 3.f * ( _u - _v) + 4.5f * pow( _u - _v, 2) - coef_vel);
		  
		f0[IX(i, j)] = (1.0f - 1.f / tau) * f0[IX(i, j)] + (1.f / tau) * eq0;
		f1[IX(i, j)] = (1.0f - 1.f / tau) * f1[IX(i, j)] + (1.f / tau) * eq1;
		f2[IX(i, j)] = (1.0f - 1.f / tau) * f2[IX(i, j)] + (1.f / tau) * eq2;
		f3[IX(i, j)] = (1.0f - 1.f / tau) * f3[IX(i, j)] + (1.f / tau) * eq3;
		f4[IX(i, j)] = (1.0f - 1.f / tau) * f4[IX(i, j)] + (1.f / tau) * eq4;
		f5[IX(i, j)] = (1.0f - 1.f / tau) * f5[IX(i, j)] + (1.f / tau) * eq5;
		f6[IX(i, j)] = (1.0f - 1.f / tau) * f6[IX(i, j)] + (1.f / tau) * eq6;
		f7[IX(i, j)] = (1.0f - 1.f / tau) * f7[IX(i, j)] + (1.f / tau) * eq7;
		f8[IX(i, j)] = (1.0f - 1.f / tau) * f8[IX(i, j)] + (1.f / tau) * eq8;
	}
}

void bounce_back_BC_LBM(float * f0,
					float * f1, float * f2, float * f3, float * f4,
					float * f5, float * f6, float * f7, float * f8,
					int N, int * solid_mask){
	assert(solid_mask != NULL);
	
	float f1_prev, f2_prev, f3_prev, f4_prev, f5_prev, f6_prev, f7_prev, f8_prev;

	for (int i = 1; i <= N; i++){
		for (int j = 1; j <= N; j++){
			if (0 == solid_mask[IX(i, j)]){
				f1_prev = f1[IX(i, j)];
				f2_prev = f2[IX(i, j)];
				f3_prev = f3[IX(i, j)];
				f4_prev = f4[IX(i, j)];
				f5_prev = f5[IX(i, j)];
				f6_prev = f6[IX(i, j)];
				f7_prev = f7[IX(i, j)];
				f8_prev = f8[IX(i, j)];

				f1[IX(i, j)] = f3_prev;
				f2[IX(i, j)] = f4_prev;
				f3[IX(i, j)] = f1_prev;
				f4[IX(i, j)] = f2_prev;
				f5[IX(i, j)] = f7_prev;
				f6[IX(i, j)] = f8_prev;
				f7[IX(i, j)] = f5_prev;
				f8[IX(i, j)] = f6_prev;
			}
		}
	}
}

void open_boundary_LBM(float * f0,
					   float * f1, float * f2, float * f3, float * f4,
					   float * f5, float * f6, float * f7, float * f8,
					   int N){
	//TODO: Process when fluid leave the domain
}

// Keep adding source.
// This was set to default by keeping source in vertical direction.
void init_state_LBM(float * f2, float * f5, float * f6, int N, int src_range, float init_mag_vel, float rho, float dt){
	
	float h = 1.f / N;
	float c = h / dt;
	float c2 = pow(c, 2);
	int emit_idx = IX(N / 2, 10);

	for (int i = N / 2 - src_range; i < N / 2 + src_range; i++){
		f2[emit_idx] = rho * 1.f / 9.f * (1.0f +  3.0f * init_mag_vel + 4.5f * pow(init_mag_vel, 2));
		f5[emit_idx] = rho * 1.f / 36.f * (1.0f + 3.0f * init_mag_vel + 4.5f * pow(init_mag_vel, 2));
		f6[emit_idx] = rho * 1.f / 36.f * (1.0f + 3.0f * init_mag_vel + 4.5f * pow(init_mag_vel, 2));
	}
}

void LBMAdvance(float * f0,
			    float * f1, float * f2, float * f3, float * f4,
			    float * f5, float * f6, float * f7, float * f8,
			    int N, float tau, float * out_u, float * out_v,
				Particle* particles, int num_particles, float dt){
	//particles_advector(N, out_u, out_v, particles, num_particles, dt);
	stream(f1, f2, f3, f4, f5, f6, f7, f8, N);
	init_state_LBM(f4, f7, f8, N, 2, 0.04f, 1.0f, 1.0);
	collision(f0, f1, f2, f3, f4, f5, f6, f7, f8, N, tau, out_u, out_v, dt);
}

// Poisson Equation Laplace(Psi) = f(x);
// Ex.1
// 5x5 with interior field 3x3 and outter boundaries
/*
A =
-4	1	0	1	0	0	0	0	0
1	-4	1	0	1	0	0	0	0
0	1	-4	0	0	1	0	0	0
1	0	0	-4	1	0	1	0	0
0	1	0	1	-4	1	0	1	0
0	0	1	0	1	-4	0	0	1
0	0	0	1	0	0	-4	1	0
0	0	0	0	1	0	1	-4	1
0	0	0	0	0	1	0	1	-4

b' = 5 0 0 0 6 0 0 0 0

10 Times Gauss Seidel solution is:
Step 1					Step 2				Step 3				Step 4				Step 5
-1.25000000000000	-1.40625000000000	-1.64257812500000	-1.75708007812500	-1.81387329101563
-0.312500000000000	-0.785156250000000	-1.01416015625000	-1.12774658203125	-1.18442535400391
-0.0781250000000000	-0.304687500000000	-0.417968750000000	-0.474609375000000	-0.502929687500000
-0.312500000000000	-0.785156250000000	-1.01416015625000	-1.12774658203125	-1.18442535400391
-1.65625000000000	-2.10937500000000	-2.33593750000000	-2.44921875000000	-2.50585937500000
-0.433593750000000	-0.657714843750000	-0.770690917968750	-0.827293395996094	-0.855608940124512
-0.0781250000000000	-0.304687500000000	-0.417968750000000	-0.474609375000000	-0.502929687500000
-0.433593750000000	-0.657714843750000	-0.770690917968750	-0.827293395996094	-0.855608940124512
-0.216796875000000	-0.328857421875000	-0.385345458984375	-0.413646697998047	-0.427804470062256
The exact solution for this system is:
(-1.8705   -1.2411   -0.5313   -1.2411   -2.5625   -0.8839   -0.5313   -0.8839   -0.4420)

Step 6					Step 7					Step 8				Step 9				Step 10
-1.84221267700195	-1.85637521743774	-1.86345559358597	-1.86699566990137	-1.86876569408923
-1.21275043487549	-1.22691118717194	-1.23399133980274	-1.23753138817847	-1.23930140887387
-0.517089843750000	-0.524169921875000	-0.527709960937500	-0.529479980468750	-0.530364990234375
-1.21275043487549	-1.22691118717194	-1.23399133980274	-1.23753138817847	-1.23930140887387
-2.53417968750000	-2.54833984375000	-2.55541992187500	-2.55895996093750	-2.56072998046875
-0.869768500328064	-0.876848503947258	-0.880388533696532	-0.882158552063629	-0.883043561683735
-0.517089843750000	-0.524169921875000	-0.527709960937500	-0.529479980468750	-0.530364990234375
-0.869768500328064	-0.876848503947258	-0.880388533696532	-0.882158552063629	-0.883043561683735
-0.434884250164032	-0.438424251973629	-0.440194266848266	-0.441079276031815	-0.441521780841867
The exact solution for this system is:
(-1.8705   -1.2411   -0.5313   -1.2411   -2.5625   -0.8839   -0.5313   -0.8839   -0.4420)
*/