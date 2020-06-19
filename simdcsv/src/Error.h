#ifndef WK_ERROR_H
#define WK_ERROR_H

#include "Macros.h"
#include "fmt/format.h"

#define WK_RAISE_ERR(err, c, fmt, ...) err.raise(WK_CONCATENATE(Error::, c), __FILE__, __LINE__, fmt, __VA_ARGS__)

namespace Wikinger {

class Error {
public:
  enum Code {
    OK,
    Generic,
    NotSupported,
    ErrorAccess,
    NotADirectory,
    CannotOpen,
    StreamEOF,
    AlreadyOpen,
    NotOpen,
    ReaderWriter_Write,
    ReaderWriter_Read,
    OutOfMemory,
    InvalidFormat,
    NotImplemented,
    StreamIllFormed,
    Invalid
  };

  void raise(Code c);
  void reset();
  void silence(Code c);

  template<typename ...Args>
  void raise(Code c, const char* path, unsigned int line, const char* fmt, Args ...args);

  bool isOk() const;
  bool peekOk() const;
  bool isHandled() const;
  Code getCode() const;
  const char* getCodeStr() const;
  const char* getPath() const;
  unsigned int getLine() const;
  const char* getMsg() const;

  operator bool() const;
  operator Code() const;

  void log() const;

  Error();
  ~Error();

private:
  void setHandled() const;
  void setEmpty();

  Code mcode;
  bool mhandled;
  const char* mpath;
  unsigned int mline;
  std::string msg;
};

template<>
struct fmt::formatter<Error>;

}

#include "Error.inl"

#endif// WK_ERROR_H
