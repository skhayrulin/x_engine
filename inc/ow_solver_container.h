#ifndef OW_SOLVER_CONTAINER
#define OW_SOLVER_CONTAINER
#include "ow_isolver.h"
#include <cstddef>
#include <string>
#include <vector>
namespace sibernetic {
namespace solver {
enum SOLVER_TYPE { OCL = 1, CUDA, SINGLE, PARALLEL };
enum DEVICE { CPU = 0, GPU = 1, ALL = 2 };
class solver_container {
  struct device {
    DEVICE type;
    std::string name;
    bool is_buisy;
  };

public:
  solver_container(size_t devices_number = 1, SOLVER_TYPE s_t = OCL);
  ~solver_container() { destroy(); }

private:
  void destroy() {
    for (auto s : _solvers) {
      delete s;
    }
  }

  std::vector<i_solver *> _solvers;
  std::vector<device> devices;
};
}
}

#endif