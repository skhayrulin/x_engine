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
#ifndef X_OCLSOLVER
#define X_OCLSOLVER

#if defined(_WIN32) || defined(_WIN64)
#pragma comment(lib, "opencl.lib") // opencl.lib
#endif

#if defined(__APPLE__) || defined(__MACOSX)
#include "OpenCL/cl.hpp"
#else
#include <CL/cl.hpp>
#endif

#include "isolver.h"
#include "ocl_const.h"
#include "ocl_struct.h"
#include "particle.h"
#include "sph_model.hpp"
#include "util/x_error.h"
#include "x_device.h"
#include <fstream>
#include <iostream>
#include "../X_ENGINE/measurement.hpp"


namespace x_engine {
	namespace solver {

		typedef struct {
			int width;
			int height;
			float* elements;
		} Matrix;

		void matrix_multiply(Matrix A, Matrix B, Matrix C, const cl::Context context,
			cl::Kernel matMulKernel,
			const cl::CommandQueue queue, uint32_t factor);

		using std::cout;
		using std::endl;
		using std::shared_ptr;
		using x_engine::ocl_error;
		using x_engine::model::sph_model;
		using x_engine::model::particle;
		using x_engine::model::partition;
		// OCL constans block
#define QUEUE_EACH_KERNEL 1

		const int LOCAL_NDRANGE_SIZE = 256;

		template <class T = float> class ocl_solver : public i_solver {
			typedef shared_ptr<sph_model<T>> model_ptr;

		private:
			float get_random_value(float min, float max) {
				return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
			}
		public:
			ocl_solver(model_ptr &m, shared_ptr<device> d) : model(m), dev(d) {
				try {
					this->initialize_ocl();
				}
				catch (ocl_error &ex) {
					throw;
				}
			}
			virtual void init_model(const partition &p) {
				this->p = p;
				init_buffers();
				init_kernels();
			}


			~ocl_solver() {}
			virtual void run_neighbour_search() {}
			virtual void run_physic() {}



			void benchmark_solver() {
				srand(time(NULL));
				Matrix A;
				Matrix B;
				Matrix C;
				const float min_val = 0.0;
				const float max_val = 500.0;
				const int height = 2048;
				const int width = 2048;

				A.elements = (float*)malloc(height * width * sizeof(float));
				for (int i = 0; i < height; ++i) {
					for (int j = 0; j < width; ++j) {
						A.elements[i * height + j] = get_random_value(min_val, max_val);
					}
				}
				//A.elements = elements;
				A.height = height;
				A.width = width;

				B.elements = (float*)malloc(height * width * sizeof(float));
				for (int i = 0; i < height; ++i) {
					for (int j = 0; j < width; ++j) {
						B.elements[i * height + j] = get_random_value(min_val, max_val);
					}
				}
				//B.elements = elements;
				B.height = height;
				B.width = width;

				C.elements = (float*)malloc(height * width * sizeof(float));
				for (int i = 0; i < height; ++i) {
					for (int j = 0; j < width; ++j) {
						C.elements[i * height + j] = 0;
					}
				}
				//C.elements = elements;
				C.height = height;
				C.width = width;

				cl::Kernel mat_mul_kernel;
				create_ocl_kernel("MatMulKernel", mat_mul_kernel);
				matrix_multiply(A, B, C, dev->context, mat_mul_kernel, queue, 1);

			}

		private:
			model_ptr model;
			partition p;
			shared_ptr<device> dev;
			std::string msg = dev->name + '\n';
			const std::string cl_program_file = "cl_code\\sph_cl_code.cl";
			cl::Kernel k_init_ext_particles;
			cl::Buffer b_particles;
			cl::Buffer b_ext_particles;
			cl::CommandQueue queue;
			cl::Program program;


			//Initialization device-side, setting size and naming
			void init_buffers() {
				create_ocl_buffer("particles", b_particles, CL_MEM_READ_WRITE,
					model->size() * sizeof(particle<T>));
				create_ocl_buffer("ext_particles", b_ext_particles, CL_MEM_READ_WRITE,
					model->size() * sizeof(extendet_particle));
			}


			void init_kernels() {}

			virtual void init_ext_particles() {}

			void initialize_ocl() {
				int err;
				queue = cl::CommandQueue(dev->context, dev->dev, 0, &err);
				if (err != CL_SUCCESS) {
					throw ocl_error(msg + "Failed to create command queue");
				}
				std::ifstream file(cl_program_file);
				if (!file.is_open()) {
					throw ocl_error(msg + "Could not open file with OpenCL program check "
						"input arguments oclsourcepath: " +
						cl_program_file);
				}
				std::string programSource(std::istreambuf_iterator<char>(file),
					(std::istreambuf_iterator<char>()));
				if (0) {
					programSource =
						"#define _DOUBLE_PRECISSION\n" +
						programSource; // not now it needs double extension check on device
				}
				cl::Program::Sources source(
					1, std::make_pair(programSource.c_str(), programSource.length() + 1));
				program = cl::Program(dev->context, source);
#if defined(__APPLE__)
				err = program.build("-g -cl-opt-disable -I .");
#else
#if INTEL_OPENCL_DEBUG
				err = program.build(OPENCL_DEBUG_PROGRAM_PATH + "-g -cl-opt-disable -I .");
#else
				err = program.build("");
#endif
#endif
				if (err != CL_SUCCESS) {
					std::string compilationErrors;
					compilationErrors = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev->dev);
					msg += make_msg(msg, "Compilation failed: ", compilationErrors);
					throw ocl_error(msg);
				}
				std::cout
					<< msg
					<< "OPENCL program was successfully build. Program file oclsourcepath: "
					<< cl_program_file << std::endl;
				return;
			}



			void matrix_multiply(Matrix A, Matrix B, Matrix C, const cl::Context context,
				cl::Kernel matMulKernel,
				const cl::CommandQueue queue, uint32_t factor)
			{
				const int BLOCK_SIZE = 16;
				// Load A and B to device memory
				size_t size = A.width * A.height * sizeof(float);
				cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, size);
				queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, size, A.elements);
				size = B.width * B.height * sizeof(float);
				cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, size);
				queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, size, B.elements);
				// Allocate C in device memory
				size = C.width * C.height * sizeof(float);
				cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, size);
				queue.enqueueWriteBuffer(buffer_C, CL_TRUE, 0, size, C.elements);


				cl::Buffer resBuf(context, CL_MEM_READ_WRITE, sizeof(int));
				// Invoke kernel
				int i = 0;


				matMulKernel.setArg(i++, A.width);
				matMulKernel.setArg(i++, A.height);
				matMulKernel.setArg(i++, buffer_A);
				matMulKernel.setArg(i++, B.width);
				matMulKernel.setArg(i++, B.height);
				matMulKernel.setArg(i++, buffer_B);
				matMulKernel.setArg(i++, C.width);
				matMulKernel.setArg(i++, C.height);
				matMulKernel.setArg(i++, buffer_C);
				matMulKernel.setArg(i++, factor);

				cl::NDRange global(C.height, C.width);
				queue.enqueueNDRangeKernel(matMulKernel, cl::NullRange, global, cl::NullRange);
				refreshTime();
				queue.finish();
				std::string naming("");
				naming += (*dev).name + " becnh results: \t%9.3f ms\n";
				watch_report(naming.c_str());

				float* res_1 = (float*)malloc(C.height * C.width * sizeof(float));;
				queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, size, res_1);
				//queue.enqueueReadBuffer(resBuf, 1, 0, C.height * C.width, res_1);
			}

			void create_ocl_buffer(const char *name, cl::Buffer &b,
				const cl_mem_flags flags, const int size) {
				int err;
				b = cl::Buffer(dev->context, flags, size, NULL, &err);
				if (err != CL_SUCCESS) {
					std::string error_m = "Buffer creation failed: ";
					error_m.append(name);
					throw ocl_error(error_m);
				}
			}


			void create_ocl_kernel(const char *name, cl::Kernel &k) {
				int err;
				k = cl::Kernel(program, name, &err);
				if (err != CL_SUCCESS) {
					std::string error_m = "Kernel creation failed: ";
					error_m.append(name);
					throw ocl_error(error_m);
				}
			}
		};
	}
}
#endif // X_OCLSOLVER