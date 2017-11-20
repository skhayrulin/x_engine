#ifndef X_ERROR
#define X_ERROR
#include <stdexcept>
#include <string>
namespace x_engine {
class ocl_error : public std::exception {
public:
  ocl_error(const char *msg) {}
};
class parser_error : public std::exception {
public:
  parser_error(const char *msg) : msg(msg) {}
  virtual const char *what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};
}

#endif
