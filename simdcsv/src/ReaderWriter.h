#ifndef WK_READER_WRITER_H
#define WK_READER_WRITER_H

#include <stdint.h>

namespace Wikinger {

class Error;

enum class Whence {
  Current = 0,
  Begin = 1,
  End = 2
};

class ReaderSeekerI {
public:
  virtual size_t readbin(Error& err, void* data, size_t size) = 0;
  virtual int64_t seek(Error& err, int64_t offset = 0, Whence whence = Whence::Current) = 0;
};

}

#endif// WK_READER_WRITER_H