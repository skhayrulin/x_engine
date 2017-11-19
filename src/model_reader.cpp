/*#include "model_reader.h"
#include "util/x_error.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <set>

using x_engine::model::sph_model;
static enum LOADMODE { NOMODE = -1, PARAMS, MODEL, POS, VEL };
static std::set<std::string> NAMES = {"particles", "x_max", "x_min", "y_max",
                                      "y_min",     "z_max", "z_min"};


template <class T>
void x_engine::read_model(sph_model<T> &model, const std::string &model_file) {
  std::ifstream file(model_file.c_str(), std::ios_base::binary);
  LOADMODE mode = NOMODE;
  bool is_model_mode = false;
  if (file.is_open()) {
    while (file.good()) {
      std::string cur_line;
      getline(std::cin, cur_line);
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
        std::regex rgx("(\\w+) : (\\d*) .*");
        std::smatch matches;
        if (std::regex_search(cur_line, matches, rgx)) {
          std::cout << "Match found\n";
          for (size_t i = 0; i < matches.size(); ++i) {
            std::cout << i << ":" << matches[i].str() << "\n";
          }
        } else {
          throw parser_error(
              "Problem with parsing parametrs. Please check parametrs");
        }
      }
    }
  }
}
*/