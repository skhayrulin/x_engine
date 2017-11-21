#ifndef X_ERROR
#define X_ERROR
#include <sstream>
#include <stdexcept>
#include <string>
namespace x_engine {
class ocl_error : public std::exception {
public:
  ocl_error(const char *msg) : msg(msg) {}
  virtual const char *what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};
class parser_error : public std::exception {
public:
  parser_error(const char *msg) : msg(msg) {}
  virtual const char *what() const throw() { return msg.c_str(); }

private:
  std::string msg;
};

template <typename T> std::string make_msg(const std::string &msg, T val) {
  std::stringstream ss;
  std::string format_str(msg);
  ss << " " << val;
  format_str += ss.str();
  return format_str;
}
template <typename T, typename... Args>
std::string make_msg(const std::string &msg, T val, Args... args) {
  std::stringstream ss;
  std::string format_str(msg);
  ss << " " << val;
  format_str += ss.str();
  format_str = make_msg(format_str, args...);
  return format_str;
}
}

#endif
