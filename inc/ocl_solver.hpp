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
#ifndef OW_OCLSOLVER
#define OW_OCLSOLVER

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
#include "sph_model.hpp"
#include "util/x_error.h"
#include "x_device.h"
#include <fstream>
#include <iostream>
namespace x_engine {
namespace solver {
using std::cout;
using std::endl;
using std::shared_ptr;
using x_engine::ocl_error;
using x_engine::model::sph_model;
template <class T = float> class ocl_solver : public i_solver {
  typedef shared_ptr<sph_model<T>> model_ptr;

public:
  ocl_solver(model_ptr &m, shared_ptr<device> d) : model(m), dev(d) {
    try {
      this->initialize_ocl();
    } catch (ocl_error &ex) {
      throw;
    }
  }
  ~ocl_solver(){};
  virtual void run_neighbour_search() {}
  virtual void run_physic() {}

private:
  cl::Kernel k_init_ext_particles;
  cl::Buffer b_particles;
  cl::Buffer b_ext_particles;
  cl::CommandQueue queue;
  cl::Program program;
  shared_ptr<device> dev;
  const std::string cl_program_file = "cl_code//sph_cl_code.cl";
  model_ptr model;
  virtual void init_ext_particles() {}
  void initialize_ocl() {
    int err;
    queue = cl::CommandQueue(dev->context, dev->dev, 0, &err);
    if (err != CL_SUCCESS) {
      throw std::runtime_error("Failed to create command queue");
    }
    std::ifstream file(cl_program_file);
    if (!file.is_open()) {
      throw std::runtime_error("Could not open file with OpenCL program check "
                               "input arguments oclsourcepath: ./test.cl");
    }
    std::string programSource(std::istreambuf_iterator<char>(file),
                              (std::istreambuf_iterator<char>()));
    if (0) {
      programSource = "#define _DOUBLE_PRECISSION\n" + programSource;
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
    err = program.build("-I .");
#endif
#endif
    if (err != CL_SUCCESS) {
      std::string compilationErrors;
      compilationErrors = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev->dev);
      std::cerr << "Compilation failed: " << std::endl
                << compilationErrors << std::endl;
      throw std::runtime_error("failed to build program");
    }
    std::cout
        << "OPENCL program was successfully build. Program file oclsourcepath: "
        << cl_program_file << std::endl;
    return;
  }
};
}
}
#endif