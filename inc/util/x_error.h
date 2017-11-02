#ifndef X_ERROR
#define X_ERROR
#include <stdexcept>

namespace x_engine
{
class ocl_error : public std::exception
{
};
}

#endif