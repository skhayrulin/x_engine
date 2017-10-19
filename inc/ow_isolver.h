#ifndef OW_ISOLVER
#define OW_ISOLVER

namespace sibernetic {
namespace solver {
class i_solver {
public:
  // virtual void interract() = 0;
  virtual void init_ext_particles() = 0;
};
}
}
#endif