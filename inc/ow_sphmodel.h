#ifndef OW_SPHMODEL
#define OW_SPHMODEL

#include "ow_particle.h"
#include <memory>
#include <vector>
namespace openworm {
namespace sibernetic {
namespace model {
template <class T = float, class container = std::vector<particle<T>>>
class sph_model {
public:
  sph_model(const sph_config &cfg);

private:
  container particles;
};
}
}
}
#endif // OW_SPHMODEL