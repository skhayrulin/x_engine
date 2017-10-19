#ifndef OW_PARTICLE
#define OW_PARTICLE

#include <algorithm>
#include <array>
#include <sstream>

namespace openworm {
namespace sibernetic {
namespace model {
// TODO write the docs
// Write why alligment on 16 bytes is important!!
template <class T, size_t dim = 4> struct alignas(16) particle {
  typedef std::array<T, dim> container;
  container pos;
  container vel;
  size_t type;
  size_t cell_id;
  size_t get_dim() const { return dim; }
  T density;
  T pressure;
  std::string pos_str() {
    std::stringstream s;
    std::for_each(pos.begin(), pos.end(), [&s](T c) { s << c << ' '; });
    s << '\n';
    return s.str();
  }
};
}
}
}

#endif