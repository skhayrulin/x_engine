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
#ifndef X_SPHMODEL
#define X_SPHMODEL

#include "particle.h"
#include "util/x_error.h"
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <vector>

namespace x_engine {
namespace model {
enum LOADMODE { NOMODE = -1, PARAMS, MODEL, POS, VEL };

template <class T = float, class container = std::vector<particle<T>>>
class sph_model {
  typedef std::map<std::string, size_t> sph_config;

public:
  sph_model(const std::string &config_file) {
    config = {{"particles", 0}, {"x_max", 0}, {"x_min", 0}, {"y_max", 0},
              {"y_min", 0},     {"z_max", 0}, {"z_min", 0}};
    read_model(config_file);
    std::cout << "Model was loaded: " << particles.size() << " partticles."
              << std::endl;
  }
  const sph_config &get_config() const { return config; }

private:
  container particles;
  sph_config config;
  std::map<std::string, T> phys_consts;
  std::shared_ptr<std::array<T, 4>> get_vector(const std::string &line) {
    std::shared_ptr<std::array<T, 4>> v(new std::array<T, 4>());
    std::stringstream ss(line);
    ss >> (*v)[0] >> (*v)[1] >> (*v)[2] >> (*v)[3]; // TODO check here!!!
    return v;
  }
  void read_model(const std::string &model_file) {
    std::ifstream file(model_file.c_str(), std::ios_base::binary);
    LOADMODE mode = NOMODE;
    bool is_model_mode = false;
    int index = 0;
    if (file.is_open()) {
      while (file.good()) {
        std::string cur_line;
        std::getline(file, cur_line);
        if (cur_line.compare("parametrs[") == 0) {
          mode = PARAMS;
          continue;
        } else if (cur_line.compare("model[") == 0) {
          mode = MODEL;
          is_model_mode = true;
          continue;
        } else if (cur_line.compare("position[") == 0) {
          mode = POS;
          continue;
        } else if (cur_line.compare("velocity[") == 0) {
          mode = VEL;
          continue;
        } else if (cur_line.compare("]") == 0) {
          mode = NOMODE;
          continue;
        }
        if (mode == PARAMS) {
          std::regex rgx("[\\t ]*(\\w+) *: *(\\d+) *([//]*.*)");
          std::smatch matches;
          if (std::regex_search(cur_line, matches, rgx)) {
            if (matches.size() > 2) {
              if (config.find(matches[1]) != config.end()) {
                config[matches[1]] =
                    static_cast<size_t>(stoi(matches[2].str()));
                continue;
              }
            } else {
              std::string msg = x_engine::make_msg(
                  "Problem with parsing parametrs:", matches[0].str(),
                  "Please check parametrs.");
              throw parser_error(msg);
            }
          } else {
            throw parser_error(
                "Please check parametrs section there are no parametrs.");
          }
        }
        if (is_model_mode) {
          switch (mode) {
          case POS: {
            particle<T> p;
            p.pos = *get_vector(cur_line);
            particles.push_back(p);
            break;
          }
          case VEL: {
            if (index >= particles.size())
              throw parser_error(
                  "Config file problem. Velocitie mode than partiles is.");
            particles[index].vel = *get_vector(cur_line);
            ++index;
            break;
          }
          default: { break; }
          }
        }
      }
    }
    file.close();
  }
};
}
}
#endif // X_SPHMODEL
