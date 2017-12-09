#include "util/ocl_helper.h"
#include "util/x_error.h"
#include <algorithm>
#include <iostream>

using x_engine::ocl_error;

void show_platform_info(const cl::Platform &);
void init_cl_devices(std::priority_queue<std::shared_ptr<device>> &);

std::priority_queue<std::shared_ptr<device>> get_dev_queue() {
  std::priority_queue<std::shared_ptr<device>> q;
  init_cl_devices(q);
  if (q.size() < 1) {
    throw ocl_error("No OpenCL devices were found");
  }
  return q;
}

size_t get_device_count(const cl::Platform &p) {
  std::vector<cl::Device> devices;
  p.getDevices(CL_DEVICE_TYPE_ALL, &devices);
  return devices.size();
}

void init_cl_devices(std::priority_queue<std::shared_ptr<device>> &q) {
  cl_int err;
  std::vector<cl::Platform> platform_list;
  err = cl::Platform::get(
      &platform_list); // TODO make check that returned value isn't error
  if (platform_list.size() < 1 || err != CL_SUCCESS) {
    throw ocl_error("No OpenCL platforms were found");
  }
  // std::for_each(platform_list.begin(), platform_list.end(),
  // show_platform_info);
  auto it =
      std::max_element(platform_list.begin(), platform_list.end(),
                       [](const cl::Platform &p1, const cl::Platform &p2) {
                         return get_device_count(p1) < get_device_count(p2);
                       });
  std::cout << "Use platform" << std::endl;
  show_platform_info(*it);
  std::vector<cl::Device> devices;
  it->getDevices(CL_DEVICE_TYPE_ALL, &devices);
  for (size_t i = 0; i < devices.size(); ++i) {
    std::shared_ptr<device> d(new device(devices[i], 0, i));
    q.push(d);
    std::cout << "Init device " << d->name << std::endl;
  }
}

void show_platform_info(const cl::Platform &platform) {
  // Get OpenCL platform name and version
  std::cout << "Platform Name: " << platform.getInfo<CL_PLATFORM_NAME>()
            << std::endl;
  std::cout << "Platform Vendor: " << platform.getInfo<CL_PLATFORM_VENDOR>()
            << std::endl;
  std::cout << "Platform Version: " << platform.getInfo<CL_PLATFORM_VERSION>()
            << std::endl;
  std::cout << "Devices: " << get_device_count(platform) << std::endl;
  std::cout << "===============================================" << std::endl;
}