#include "Log/Log.h"
#include "Error.h"

namespace Wikinger {

inline void Error::raise(Code c) {
  reset();
  mcode = c;
  mhandled = false;
}

template<typename ...Args>
void Error::raise(Code c, const char* path, unsigned int line, const char* fmt, Args ...args) {
  reset();
  mcode = c;
  mpath = path;
  mline = line;
  mhandled = false;
  msg = fmt::format(fmt, args...);
}

inline void Error::reset() {
  log();
  setEmpty();
}

inline void Error::silence(Code c) {
  if(mcode == c || mcode == Code::OK)
    mhandled = true;
}

inline void Error::log() const {
  if(!isHandled()) {
    const std::string& msg = getMsg();
    if(msg.empty())
      Log::dblog(Log::V_ERROR, getPath(), getLine(), "Error '{}'", getCodeStr());
    else
      Log::dblog(Log::V_ERROR, getPath(), getLine(), "Error '{}:{}'", getCodeStr(), msg);
  }
}

inline bool Error::isOk() const {
  return getCode() == Error::OK;
}

inline bool Error::peekOk() const {
  return mcode == Error::OK;
}

inline bool Error::isHandled() const {
  return mhandled;
}

inline Error::Code Error::getCode() const {
  setHandled();
  return mcode;
}

inline const char* Error::getCodeStr() const {
  static const char* errorcodestr[] = {
    "Ok",
    "Generic",
    "NotSupported",
    "ErrorAccess",
    "NotADirectory",
    "CannotOpen",
    "StreamEOF",
    "AlreadyOpen",
    "NotOpen",
    "ReaderWriter_Write",
    "ReaderWriter_Read",
    "OutOfMemory",
    "NotImplemented",
    "StreamIllFormat",
    "InvalidFormat"
  };
  WK_STATIC_ASSERT(WK_COUNTOF(errorcodestr) == Error::Invalid,
                   "Error::Code must have the same amount of entries as the string representation equivalent");
  int idx = static_cast<int>(mcode);
  // should be mcode instead of getCode since getCode has the side effect
  // of setting mhandled to true which should not happen due to getCodeStr
  // shouldn't be used for error handling.
  if(idx < 0 || idx >= WK_COUNTOF(errorcodestr))
    return "???";
  else
    return errorcodestr[idx];
}

inline const char* Error::getPath() const {
  return mpath;
}

inline unsigned int Error::getLine() const {
  return mline;
}

inline const char* Error::getMsg() const {
  return msg.c_str();
}

inline Error::operator bool() const {
  return isOk();
}

inline Error::operator Code() const {
  return getCode();
}

inline void Error::setHandled() const {
  const_cast<Error*>(this)->mhandled = true;
}

inline void Error::setEmpty() {
  mcode = Error::OK;
  mpath = "???";
  mline = 0;
  mhandled = true;
  msg = "";
}

inline Error::Error() {
  setEmpty();
}
inline Error::~Error() {
  log();
}

template<>
struct fmt::formatter<Error> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template<typename FormatContext>
  auto format(const Error& e, FormatContext& ctx) {
    const std::string& msg = e.getMsg();
    if(msg.empty())
      return format_to(ctx.out(), "Error '{}' At '{}:{}'", e.getCodeStr(), e.getPath(), e.getLine());
    else
      return format_to(ctx.out(), "Error '{}:{}' At '{}:{}'", e.getCodeStr(), msg, e.getPath(), e.getLine());
  }
};

}