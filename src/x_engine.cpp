/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2011, 2013 OpenWorm.
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
#include "solver_container.hpp"
#include <iostream>

using x_engine::model::sph_model;
using x_engine::solver::solver_container;

int usage() {
  std::string version = "0.0.1";
  std::cout
      << "\nx_engine v" << version << "\n  This is a C++/OpenCL "
      << "implementation of the SPH algorithm supplemented with"
      << "with many posibilities"
      << "a set of biomechanics related features"
      << "Usage: ./bin/x_engine [OPTION]\n\n"
      << "    -multi_dev                 Run without on all available devices"
      << "                               but default it will run only one.\n\n"
      << "    -help, -h, -?, --help      Print this information\n\n"
      << "Full documentation at: <https://github.com/openworm/sibernetic>\n"
      << "Please report any bugs/issues "
      << "to: <https://github.com/openworm/sibernetic/issues>\n";
  return EXIT_SUCCESS;
}
int main(int argc, char **argv) {
  try {
    std::shared_ptr<sph_model<float>> model(
        new sph_model<float>("config/demo1"));
    solver_container<float> &s_con = solver_container<float>::instance(model);
  } catch (x_engine::parser_error &e) {
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  } catch (x_engine::ocl_error &e) {
    std::cout << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
