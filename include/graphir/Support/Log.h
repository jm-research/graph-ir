#ifndef GRAPHIR_SUPPORT_LOG_H
#define GRAPHIR_SUPPORT_LOG_H

#include <cassert>
#include <cstdlib>
#include <iostream>

#define graphir_unreachable(MSG) assert(false && "Unreachable statement: " #MSG)

namespace graphir {
namespace log {

inline std::ostream& Error() { return std::cerr << "[Error] "; }

inline std::ostream& Verbose() { return std::cout << "[Verbose] "; }

inline std::ostream& Debug() { return std::cout << "[Debug] "; }

}  // end namespace Log

class Diagnostic {
  bool abort_;

 public:
  Diagnostic() : abort_(false) {}

  std::ostream& Warning() { return log::Error() << "Warning: "; }

  std::ostream& Error() {
    abort_ = true;
    return log::Error() << "Error: ";
  }

  ~Diagnostic() {
    if (abort_) {
      std::exit(1);
    }
  }
};

}  // namespace graphir

#endif  // GRAPHIR_SUPPORT_LOG_H