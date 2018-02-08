
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

#include "inc/ocl_struct.h"

#define NO_PARTICLE_ID -1

typedef struct particle_f{
	float4 pos;
	float4 vel;
	size_t cell_id;
	size_t type_;
	float density;
	float pressure;
} particle_f;
#ifdef _DOUBLE_PRECISION
typedef struct particle_d{
	double4 pos;
	double4 vel;
	size_t cell_id;
	size_t type_;
	double density;
	double pressure;
} particle_d;
#endif

/** Just for test
*/
__kernel void work_with_struct(__global struct extendet_particle * ext_particles, 
							   __global struct 
							   #ifdef _DOUBLE_PRECISION
									particle_d
							   #else
									particle_f
							   #endif 
									* particles){
	int cell_id_to_find = get_global_id(0);
#ifdef PRINTF_ON
	if(cell_id_to_find == 0){
		printf("sizeof() of particles_f is %d\n", sizeof(particle_f) );
		#ifdef _DOUBLE_PRECISION
		printf("sizeof() of particles_d is %d\n", sizeof(particle_d) );
		#endif
	}
#endif
#ifdef _DOUBLE_PRECISION

	particles[cell_id_to_find].pos = (double4)(cell_id_to_find, cell_id_to_find, cell_id_to_find, cell_id_to_find);
	particles[cell_id_to_find].vel = (double4)(cell_id_to_find, cell_id_to_find, cell_id_to_find, cell_id_to_find);
#else
	particles[cell_id_to_find].pos = (float4)(cell_id_to_find, cell_id_to_find, cell_id_to_find, cell_id_to_find);
	particles[cell_id_to_find].vel = (float4)(cell_id_to_find, cell_id_to_find, cell_id_to_find, cell_id_to_find);
#endif
	particles[cell_id_to_find].type_ = cell_id_to_find + 1;
}

/**Initialization of neighbour list by -1 
* what means that it's no neighbours. 
*/
__kernel void _ker_init_ext_particles(__global struct extendet_particle * ext_particles){
	int cell_id_to_find = get_global_id(0);
	ext_particles[cell_id_to_find].p_id = cell_id_to_find;
	for(int i=0;i<NEIGHBOUR_COUNT;++i){
		ext_particles[cell_id_to_find].neigbour_list[i] = -1;
	}
}

__kernel void _ker_grid_cell_indexing(__global struct 
							   #ifdef _DOUBLE_PRECISION
									particle_d
							   #else
									particle_f
							   #endif 
									* particles,
									uint PARTICLE_COUNT,
									__global uint* grid_cell_index,
									uint grid_cell_count){
	int cell_id_to_find = get_global_id(0);

	//check for trivial cases
	if (cell_id_to_find > grid_cell_count){
		return;
	}
    if (PARTICLE_COUNT == 0) {
        /* array is empty */
        return;
    } else if (particles[0].cell_id > cell_id_to_find) {
        /* cell_id_to_find is lower than everyone else */
        return;
    } else if (particles[PARTICLE_COUNT - 1].cell_id < cell_id_to_find) {
        /* cell_id_to_find is greater than everyone else */
        return;
    }

	if (cell_id_to_find == 0){
		grid_cell_index[cell_id_to_find] = 0;
		return;
	}
	if (cell_id_to_find == grid_cell_count){
		grid_cell_index[cell_id_to_find] = PARTICLE_COUNT;
		return;		
	}
	//end check

	//binary search
	int low = 0;
	int high = PARTICLE_COUNT - 1;
	bool converged = false;
	int particle_id = NO_PARTICLE_ID;
	int cur_part_cell_id = -1;

	while (!converged){
		if (low > high){
			converged = true;
			particle_id = NO_PARTICLE_ID;
			continue;
		}

		int middle = low + (high - low) / 2;
		cur_part_cell_id = particles[middle].cell_id;
		particle_id = middle;
		if (cell_id_to_find <= cur_part_cell_id){
			high = middle;
		} else {
			low = middle + 1;
		}

	}
	//end binary search

	//find lowest particle_id that gives cell_id_to_find and init grid_cell_index value
	if (!converged && particles[last].cell_id == cell_id_to_find){
		particle_id = last;
		if (low == high){
			particle_id = low;
		}
		//[QUEST] Is there no need to check if last == 0 ? I think so because if last == 0 it means that we wanted to find cell_id_to_find == 0. That case was handled in trivial cases

		//find lowest particle_id that gives cell_id_to_find
		// [QUEST] is the while condition below correct?
		while (particles[particle_id - 1] && particles[particle_id - 1].cell_id == cell_id_to_find){
			particle_id--;
		}
	} else {
		particle_id = NO_PARTICLE_ID;
	}
 	grid_cell_index[ cell_id_to_find ] = particle_id;
}

/** Calc current cell cell_id_to_find for each particles
*/
__kernel void _ker_calc_cell_id(__global struct 
							   #ifdef _DOUBLE_PRECISION
									particle_d
							   #else
									particle_f
							   #endif 
									* particles){

}

/** Searchin for neigbours foe each particles
*/
__kernel void _ker_neighbour_search(__global struct extendet_particle * ext_particles, 
							   __global struct 
							   #ifdef _DOUBLE_PRECISION
									particle_d
							   #else
									particle_f
							   #endif 
									* particles){

}

int cellId(
		   int4 cellFactors_,
		   uint gridCellsX,
		   uint gridCellsY,
		   uint gridCellsZ//don't use
		   )
{
	int cellId_ = cellFactors_.x + cellFactors_.y * gridCellsX
		+ cellFactors_.z * gridCellsX * gridCellsY;
	return cellId_;
}
/** Caculation spatial hash cellId for every particle
 *  Kernel fill up particleIndex buffer.
 */
 int4 cellFactors(
				 __global struct 
				#ifdef _DOUBLE_PRECISION
					particle_d
				#else
					particle_f
				#endif 
					* particle,
				 float xmin,
				 float ymin,
				 float zmin,
				 float hashGridCellSizeInv
				 )
{
	//xmin, ymin, zmin
	int4 result;
	result.x = (int)( particle.pos.x *  hashGridCellSizeInv );
	result.y = (int)( particle.pos.y *  hashGridCellSizeInv );
	result.z = (int)( particle.pos.z *  hashGridCellSizeInv );
	return result;
}
__kernel void hashParticles(
							__global struct 
							#ifdef _DOUBLE_PRECISION
								particle_d
							#else
								particle_f
							#endif 
								* particle,
							uint gridCellsX,
							uint gridCellsY,
							uint gridCellsZ,
							float hashGridCellSizeInv,
							float xmin,
							float ymin,
							float zmin,
							__global uint2 * particleIndex,
							uint   PARTICLE_COUNT
							)
{
	int cell_id_to_find = get_global_id( 0 );
	if( cell_id_to_find >= PARTICLE_COUNT ) return;
	float4 _position = particle[ cell_id_to_find ];
	int4 cellFactors_ = cellFactors( _position, xmin, ymin, zmin, hashGridCellSizeInv );
	int cellId_ = cellId( cellFactors_, gridCellsX, gridCellsY, gridCellsZ ) & 0xffffff; // truncate to low 16 bits
	uint2 result;
	PI_CELL_ID( result ) = cellId_;
	PI_SERIAL_ID( result ) = cell_id_to_find;
	particleIndex[ cell_id_to_find ] = result;
}