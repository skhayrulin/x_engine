#ifndef X_OCLHELPER
#define X_OCLHELPER
#include "x_device.h"
#include <memory>
#include <queue>
#if defined(__APPLE__) || defined(__MACOSX)
#include "../inc/OpenCL/cl.hpp"
//	#include <OpenCL/cl_d3d10.h>
#else
#include <CL/cl.hpp>
#endif
std::priority_queue<std::shared_ptr<device>> get_dev_queue();
#endif // X_OCLHELPER