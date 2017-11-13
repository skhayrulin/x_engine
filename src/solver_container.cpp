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

#include "ow_solver_container.h"
#include "ow_oclsolver.h"
#include <iostream>
#include <memory>
#include "util/x_error.h"

using x_engine::solver::solver_container;
using x_engine::solver::SOLVER_TYPE;

solver_container::solver_container(size_t devices_number, SOLVER_TYPE s_type)
{
  _solvers.reserve(devices_number);
  try
  {
    for (int i = 0; i < devices_number; ++i)
    {
      std::shared_ptr<i_solver> s;
      std::shared_ptr<device> d(new device{CPU, "", false});
      switch (s_type)
      {
      case OCL:
      {
        s = std::make_shared<ocl_solver>(d);
        devices.push_back(d);
        _solvers.push_back(s);
        break;
      }
      default:
        break;
      };
    }
  }
  catch (ocl_error &err)
  {
    throw;
  }
}