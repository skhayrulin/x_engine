
/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2011, 2017 OpenWorm.
 * http://openworm.org
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the MIT License
 * which accompanies this distribution, and is available at
 * http://opensource.org/licenses/MIT
 *
 * Contributors:
 *     	OpenWorm - http://openworm.org/people.html
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *******************************************************************************/
 
#ifdef cl_amd_printf
	#pragma OPENCL EXTENSION cl_amd_printf : enable
	#define PRINTF_ON // this comment because using printf leads to very slow work on Radeon r9 290x on my machine
    	              // don't know why
#elif defined(cl_intel_printf)
	#pragma OPENCL EXTENSION cl_intel_printf : enable
	#define PRINTF_ON
#endif

#ifdef _DOUBLE_PRECISION
	#ifdef cl_khr_fp64
		#pragma OPENCL EXTENSION cl_khr_fp64 : enable
		
	#elif defined(cl_amd_fp64)
		#pragma OPENCL EXTENSION cl_amd_fp64 : enable
		#define _DOUBLE_PRECISION
	#else
		#error "Double precision floating point not supported by OpenCL implementation."
	#endif
#endif

#include "inc/ow_cl_struct.h"

typedef struct particle_f{
	float4 pos;
	float4 vel;
	size_t cell_id;
	size_t type_;
	float density;
	float pressure;
} particle_f;

typedef struct particle_d{
	double4 pos;
	double4 vel;
	size_t cell_id;
	size_t type_;
	double density;
	double pressure;
} particle_d;


__kernel void work_with_struct(__global struct extendet_particle * ext_particles, 
							   __global struct 
							   #ifdef _DOUBLE_PRECISION
									particle_d
							   #else
									particle_f
							   #endif 
									* particles){
	int id = get_global_id(0);
#ifdef PRINTF_ON
	if(id == 0){
		printf("sizeof() of particles_f is %d\n", sizeof(particle_f) );
		printf("sizeof() of particles_d is %d\n", sizeof(particle_d) );
	}
#endif
#ifdef _DOUBLE_PRECISION

	particles[id].pos = (double4)(id, id, id, id);
	particles[id].vel = (double4)(id, id, id, id);
#else
	particles[id].pos = (float4)(id, id, id, id);
	particles[id].vel = (float4)(id, id, id, id);
#endif
	particles[id].type_ = id + 1;
}


__kernel void _init_ext_particles(__global struct extendet_particle * ext_particles){
	int id = get_global_id(0);
	ext_particles[id].p_id = id;
	for(int i=0;i<NEIGHBOUR_COUNT;++i){
		ext_particles[id].neigbour_list[i] = -1;
	}
}