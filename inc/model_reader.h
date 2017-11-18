#ifndef MODEL_READER
#define MODEL_READER
#include "sph_model.hpp"
#include <string>
namespace x_engine {
template <class T> void read_model(model::sph_model<T> &, const std::string &);
}
#endif // MODEL_READER