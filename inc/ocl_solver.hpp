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
#include <iostream>
#include <fstream>
#include "isolver.h"
#include "ocl_const.h"
#include "util/x_error.h"
#include "sph_model.h"
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
  ocl_solver(model_ptr m, shared_ptr<device> d) : model(m) {
    try {
      this->initialize_ocl(d);
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
  cl::Context context;
  cl::CommandQueue queue;
  cl::Program program;
  const std::string program_name ="cl_code//sph_cl_code.cl";
  model_ptr model;
  virtual void init_ext_particles() {}
  void initialize_ocl(shared_ptr<device> dev) {
    cl_int err;
    std::vector<cl::Platform> platformList;
    std::vector<cl::Device> devices;
    err = cl::Platform::get(
        &platformList); // TODO make check that returned value isn't error
    if (platformList.size() == 0 || err != CL_SUCCESS) {
      throw ocl_error("No OpenCL platforms found");
    }
    char _name[1024];
    cl_platform_id
    cl_pl_id[10]; // TODO make this optional case it could be many platforms 
    cl_uint n_pl;
    clGetPlatformIDs(10, cl_pl_id, &n_pl); // TODO make 10 - number of platforms optional
     for (cl_uint i = 0; i < n_pl; i++) {
      // Get OpenCL platform name and version
      err = clGetPlatformInfo(cl_pl_id[i], CL_PLATFORM_VERSION, sizeof(_name),
                              _name, NULL);
      if (err == CL_SUCCESS) {
        cout << "CL_PLATFORM_VERSION [" << i << "]: \t" << _name << endl;
      } else {
        std::cerr << "Error " << err << " in clGetPlatformInfo Call" << endl;
      }
    }
    // 0-CPU, 1-GPU // depends on the time order of system OpenCL drivers
    // installation on your local machine
    // CL_DEVICE_TYPE
    cl_device_type type;
    unsigned int device_type[] = {CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_GPU,
                                  CL_DEVICE_TYPE_ALL};

    int pl_list =
        -1; // selected platform index in platformList array [choose CPU
            // by default]
    // added autodetection of device number corresonding to preferrable device
    // type (CPU|GPU) | otherwise the choice will be made from list of existing
    // devices
    cl_uint cl_device_count = 0;
    cl_device_id *devices_t;
    bool b_passed = true, find_device = false;
    cl_int result, device_coumpute_unit_num,
        device_coumpute_unit_num_current = 0;
    size_t dev_num = 0;
    // Selection of more appropriate device
    while (!find_device) {
      for (cl_uint id = 0; id < n_pl; ++id) {
        clGetDeviceIDs(cl_pl_id[id], device_type[dev->type], 0, NULL,
                       &cl_device_count);
        if ((devices_t = static_cast<cl_device_id *>(
                 malloc(sizeof(cl_device_id) * cl_device_count))) == NULL)
          b_passed = false;
        if (b_passed) {
          result = clGetDeviceIDs(cl_pl_id[id], device_type[dev->type],
                                  cl_device_count, devices_t, &cl_device_count);
          if (result == CL_SUCCESS) {
            for (cl_uint i = 0; i < cl_device_count; ++i) {
              clGetDeviceInfo(devices_t[i], CL_DEVICE_TYPE, sizeof(type), &type,
                              NULL);
              if (type & device_type[dev->type]) {
                clGetDeviceInfo(devices_t[i], CL_DEVICE_MAX_COMPUTE_UNITS,
                                sizeof(device_coumpute_unit_num),
                                &device_coumpute_unit_num, NULL);
                if (device_coumpute_unit_num_current <=
                    device_coumpute_unit_num) {
                  pl_list = id;
                  device_coumpute_unit_num_current = device_coumpute_unit_num;
                  find_device = true;
                  dev_num = i;
                }
              }
            }
          }
          free(devices_t);
        }
      }
      if (!find_device) {
        dev_num = 0;
        std::string deviceTypeName =
            (dev->type == ALL) ? "ALL" : (dev->type == CPU) ? "CPU" : "GPU";
        cout << "Unfortunately OpenCL couldn't find device " << deviceTypeName
             << endl;
        cout << "OpenCL try to init existing device " << endl;
        if (dev->type != ALL)
          dev->type = ALL;
        else
          throw ocl_error("Sibernetic can't find any OpenCL devices. "
                          "Please check you're environment "
                          "configuration.");
      }
    }
    cl_context_properties cprops[3] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)(platformList[pl_list])(),
        0};
    context = cl::Context(device_type[dev->type], cprops, NULL, NULL, &err);
    devices = context.getInfo<CL_CONTEXT_DEVICES>();
    if (devices.size() < 1) {
      throw std::runtime_error("No OpenCL devices were found");
    }
    // Print some information about chosen platform
    size_t compUnintsCount, memoryInfo, workGroupSize;
    result = devices[dev_num].getInfo(CL_DEVICE_NAME,
                                      &_name); // CL_INVALID_VALUE = -30;
    if (result == CL_SUCCESS) {
      cout << "CL_CONTEXT_PLATFORM [" << pl_list << "]: CL_DEVICE_NAME ["
           << dev_num << "]:\t" << _name << "\n"
           << endl;
    }
    if (strlen(_name) < 1024) {
      dev->name = _name;
    }
    result = devices[dev_num].getInfo(CL_DEVICE_TYPE, &_name);
    if (result == CL_SUCCESS) {
      cout << "CL_CONTEXT_PLATFORM [" << pl_list << "]: CL_DEVICE_TYPE ["
           << dev_num << "]:\t"
           << ((_name[0] == CL_DEVICE_TYPE_CPU) ? "CPU" : "GPU") << endl;
    }
    result =
        devices[dev_num].getInfo(CL_DEVICE_MAX_WORK_GROUP_SIZE, &workGroupSize);
    if (result == CL_SUCCESS) {
      cout << "CL_CONTEXT_PLATFORM [" << pl_list
           << "]: CL_DEVICE_MAX_WORK_GROUP_SIZE [" << dev_num << "]: \t"
           << workGroupSize << endl;
    }
    result =
        devices[dev_num].getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &compUnintsCount);
    if (result == CL_SUCCESS) {
      cout << "CL_CONTEXT_PLATFORM [" << pl_list
           << "]: CL_DEVICE_MAX_COMPUTE_UNITS [" << dev_num << "]: \t"
           << compUnintsCount << endl;
    }
    result = devices[dev_num].getInfo(CL_DEVICE_GLOBAL_MEM_SIZE, &memoryInfo);
    if (result == CL_SUCCESS) {
      cout << "CL_CONTEXT_PLATFORM [" << pl_list
           << "]: CL_DEVICE_GLOBAL_MEM_SIZE [" << dev_num << "]: \t" << dev_num
           << endl;
    }
    result =
        devices[dev_num].getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, &memoryInfo);
    if (result == CL_SUCCESS) {
      cout << "CL_CONTEXT_PLATFORM [" << pl_list
           << "]: CL_DEVICE_GLOBAL_MEM_CACHE_SIZE [" << dev_num << "]:\t"
           << memoryInfo << endl;
    }
    result = devices[dev_num].getInfo(CL_DEVICE_LOCAL_MEM_SIZE, &memoryInfo);
    if (result == CL_SUCCESS) {
      cout << "CL_CONTEXT_PLATFORM " << pl_list
           << ": CL_DEVICE_LOCAL_MEM_SIZE [" << dev_num << "]:\t" << memoryInfo
           << endl;
    }
    queue = cl::CommandQueue(context, devices[dev_num], 0, &err);
    if (err != CL_SUCCESS) {
      throw std::runtime_error("Failed to create command queue");
    }
    std::ifstream file(ocl_solver::program_name.c_str());
    if (!file.is_open()) {
      throw ocl_error("Could not open file with OpenCL program check "
                      "input arguments oclsourcepath: ");
    }
    std::string programSource(std::istreambuf_iterator<char>(file),
                              (std::istreambuf_iterator<char>()));
    cl::Program::Sources source(
        1, std::make_pair(programSource.c_str(), programSource.length() + 1));
    program = cl::Program(context, source);
#if defined(__APPLE__)
    err = program.build(devices, "-g -cl-opt-disable");
#else
#if INTEL_OPENCL_DEBUG
    err = program.build(devices,
                        OPENCL_DEBUG_PROGRAM_PATH + "-g -cl-opt-disable");
#else
    err = program.build(devices, "");
#endif
#endif
    if (err != CL_SUCCESS) {
      std::string compilationErrors;
      compilationErrors =
          program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(devices[0]);
      std::cerr << "Compilation failed: " << endl << compilationErrors << endl;
      throw ocl_error("Failed to build program");
    }
    cout
        << "OPENCL program was successfully build. Program file oclsourcepath: "
        << endl;
  }
};
}
}
#endif