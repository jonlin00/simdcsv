#ifndef WK_LOG_H
#define WK_LOG_H

#include "../Macros.h"
#include "../fmt/format.h"

namespace Wikinger {

class Log {
public:
  enum Verbosity {
    V_FATAL = -3,
    V_ERROR = -2,
    V_WARNING = -1,
    V_INFO = 0,
    V_0,
    V_DEBUG = 1,
    V_1 = 1,
    V_2 = 2,
    V_3 = 3,
    V_4 = 4,
    V_5 = 5,
    V_6 = 6,
    V_7 = 7,
    V_8 = 8,
    V_9 = 9
  };

  template<typename ...Args>
  static void dblog(Verbosity verbosity, const char* file, unsigned int line, const char* format, Args... args);
  static void vdblog(Verbosity verbosity, const char* file, unsigned int line, const char* format, const fmt::format_args& args);

  static void init();

private:
  Log() {};
};

template<typename ...Args>
void Log::dblog(Verbosity verbosity, const char* file, unsigned int line, const char* format, Args... args) {
  Log::vdblog(verbosity, file, line, format, fmt::make_format_args(args...));
}

}

#define WK_LOG(verbosity, fmt, ...) ::Wikinger::Log::dblog(WK_CONCATENATE(::Wikinger::Log::V_, verbosity), __FILE__, __LINE__, fmt, __VA_ARGS__)

#define WK_DEBUG(fmt, ...) WK_LOG(DEBUG, fmt, __VA_ARGS__)
#define WK_INFO(fmt, ...) WK_LOG(INFO, fmt, __VA_ARGS__)
#define WK_WARN(fmt, ...) WK_LOG(WARNING, fmt, __VA_ARGS__)
#define WK_ERROR(fmt, ...) WK_LOG(ERROR, fmt, __VA_ARGS__)
#define WK_FATAL(fmt, ...) WK_LOG(FATAL, fmt, __VA_ARGS__)

#define WK_CHECK(expr, verbosity, fmt, ...) (((expr) == 0) ? WK_LOG(verbosity, fmt, __VA_ARGS__) : WK_NOOP);

#endif// WK_LOG_H
