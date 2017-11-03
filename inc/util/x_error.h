#ifndef X_ERROR
#define X_ERROR
#include <stdexcept>

namespace x_engine
{
class ocl_error : public std::exception
{
  public:
    ocl_error(const char *msg) {}
};
}

#endif