#include "Log.h"
#include "Loguru.h"

namespace Wikinger {

void Log::vdblog(Verbosity verbosity, const char* file, unsigned int line, const char* format, const fmt::format_args& args) {
  loguru::vlog(verbosity, file, line, format, args);
}

void Log::init() {
  loguru::init();
}

}