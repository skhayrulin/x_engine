#ifndef OW_SOLVER_CONTAINER
#define OW_SOLVER_CONTAINER
#include "isolver.h"
#include "ocl_const.h"
#include "ocl_solver.hpp"
#include "sph_model.hpp"
#include "util/ocl_helper.h"
#include "util/x_error.h"
#include <string>
#include <vector>
namespace x_engine {
namespace solver {
using std::shared_ptr;
using model::sph_model;
using x_engine::solver::ocl_solver;
template <class T = float> class solver_container {
  typedef shared_ptr<sph_model<T>> model_ptr;

public:
  solver_container(const solver_container &) = delete;
  solver_container &operator=(const solver_container &) = delete;
  /** Classic Maer's singleton
   */
  static solver_container &instance(model_ptr &model, size_t devices_number = 1,
                                    SOLVER_TYPE s_t = OCL) {
    static solver_container s(model, devices_number, s_t);
    return s;
  }

private:
  solver_container(model_ptr &model, size_t devices_number = 1,
                   SOLVER_TYPE s_type = OCL) {
    try {
      std::priority_queue<std::shared_ptr<device>> dev_q = get_dev_queue();
      int i = 0;
      while (!dev_q.empty()) {
        std::shared_ptr<ocl_solver<T>> solver(
            new ocl_solver<T>(model, dev_q.top()));
        dev_q.pop();
        _solvers.push_back(solver);
      }
    } catch (x_engine::ocl_error &err) {
      throw;
    }
  }
  ~solver_container() {}
  std::vector<std::shared_ptr<i_solver>> _solvers;
};
}
}

#endif