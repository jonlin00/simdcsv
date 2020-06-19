#include <stdlib.h>

namespace Wikinger {
namespace fmt {

template<typename T>
inline size_t txtparse(Error& err, ReaderSeekerI* reader, T& first) {
  txtparser<T> p;
  return p.parse(err, reader, first);
}

template<typename T, typename ...Args>
inline size_t txtparse(Error& err, ReaderSeekerI* reader, T& first, Args&... args) {
  size_t res = txtparse(err, reader, first);
  res += txtparse(err, reader, args...);
  return res;
}


inline bool isSpace(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}
inline bool isChar(char c) {
  return c >= 0x20 && c <= 0x7E; // basically is c not a control character
}
inline bool isNumerical(char c) {
  return c >= '0' && c <= '9';
}

inline void eatSpaces(Error& err, ReaderSeekerI* reader) {
  alignas(8) char buff[32];

  do {
    size_t read = reader->readbin(err, buff, WK_COUNTOF(buff));
    if(!err.peekOk()) return;

    for(int i = 0; i < read; i++) {
      if(!isSpace(buff[i])) {
        reader->seek(err, i - read);
        goto RTN_LBL;
      }
    }
  } while(err.peekOk());
RTN_LBL:
  return; // an statement is apparently required...
}

template<>
struct txtparser<std::string> {
  size_t parse(Error& err, ReaderSeekerI* reader, std::string& value) {
    if(!err.peekOk()) return false;

    value.clear();
    char c = 0;
    eatSpaces(err, reader);
    size_t i = 0;
    while(reader->readbin(err, &c, sizeof(c)) == sizeof(c) && !isSpace(c)) {
      if(!err.peekOk()) return false;
      value.push_back(c);
      ++i;
    }
    reader->seek(err, -1, Whence::Current);
    return i;
  }


  size_t parseLine(Error& err, ReaderSeekerI* reader, std::string& value) {
    if(!err.peekOk()) return false;

    value.clear();
    char c = 0;
    eatSpaces(err, reader);
    size_t i = 0;
    while(reader->readbin(err, &c, sizeof(c)) == sizeof(c) && c != '\n') {
      if(!err.peekOk()) return false;
      value.push_back(c);
      ++i;
    }
    reader->seek(err, -1, Whence::Current);
    return i;
  }
};

inline size_t _readTxtNumBaseStart(Error& err, ReaderSeekerI* reader, char* tmp, size_t count) {
  if(!err.peekOk())
    return SIZE_MAX;

  char c = 0;
  eatSpaces(err, reader);
  size_t len = reader->readbin(err, tmp, count - 1);
  reader->seek(err, -(int64_t)len);
  if(!err.peekOk()) return SIZE_MAX;
  tmp[len] = 0;

  if((tmp[0] == '-' && isNumerical(tmp[1])) || isNumerical(tmp[0]))
    return len;

  WK_RAISE_ERR(err, InvalidFormat, "Read: illegal format, NaN");
  return SIZE_MAX;
}

inline size_t _readTxtNumBaseEnd(Error& err, ReaderSeekerI* reader, char* tmp, char* endptr) {
  int64_t off = endptr - tmp;
  reader->seek(err, off, Whence::Current);
  return off;
}

template<>
struct txtparser<int64_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, int64_t& value) {
    char tmp[128];
    size_t len = _readTxtNumBaseStart(err, reader, tmp, WK_COUNTOF(tmp));
    if(len == SIZE_MAX) return 0;
    char* endptr = nullptr;
    value = strtoll(tmp, &endptr, 0);
    return _readTxtNumBaseEnd(err, reader, tmp, endptr);
  }
};

template<>
struct txtparser<int32_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, int32_t& value) {
    int64_t d;
    txtparser<int64_t> p;
    size_t res = p.parse(err, reader, d);
    if(err.peekOk()) value = (int32_t)d;
    return res;
  }
};

template<>
struct txtparser<int16_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, int16_t& value) {
    int64_t d;
    txtparser<int64_t> p;
    size_t res = p.parse(err, reader, d);
    if(err.peekOk()) value = (int16_t)d;
    return res;
  }
};

template<>
struct txtparser<int8_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, int8_t& value) {
    int64_t d;
    txtparser<int64_t> p;
    size_t res = p.parse(err, reader, d);
    if(err.peekOk()) value = (int8_t)d;
    return res;
  }
};

template<>
struct txtparser<uint64_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, uint64_t& value) {
    char tmp[128];
    size_t len = _readTxtNumBaseStart(err, reader, tmp, WK_COUNTOF(tmp));
    if(len == SIZE_MAX) return 0;
    if(tmp[0] == '-') {
      WK_RAISE_ERR(err, InvalidFormat, "txtparser<uint64>: signed value cannot casted to unsigned");
      return 0;
    }
    char* endptr = nullptr;
    value = strtoul(tmp, &endptr, 0);
    return _readTxtNumBaseEnd(err, reader, tmp, endptr);
  }
};

template<>
struct txtparser<uint32_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, uint32_t& value) {
    uint64_t d;
    txtparser<uint64_t> p;
    size_t res = p.parse(err, reader, d);
    if(err.peekOk()) value = (uint32_t)d;
    return res;
  }
};

template<>
struct txtparser<uint16_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, uint16_t& value) {
    uint64_t d;
    txtparser<uint64_t> p;
    size_t res = p.parse(err, reader, d);
    if(err.peekOk()) value = (uint16_t)d;
    return res;
  }
};

template<>
struct txtparser<uint8_t> {
  size_t parse(Error& err, ReaderSeekerI* reader, uint8_t& value) {
    uint64_t d;
    txtparser<uint64_t> p;
    size_t res = p.parse(err, reader, d);
    if(err.peekOk()) value = (uint8_t)d;
    return res;
  }
};

template<>
struct txtparser<float> {
  size_t parse(Error& err, ReaderSeekerI* reader, float& data) {
    char tmp[128];
    size_t len = _readTxtNumBaseStart(err, reader, tmp, WK_COUNTOF(tmp));
    if(len == UINT32_MAX) return 0;
    char* endptr = nullptr;
    data = strtof(tmp, &endptr);
    return _readTxtNumBaseEnd(err, reader, tmp, endptr);
  }
};

template<>
struct txtparser<double> {
  size_t parse(Error& err, ReaderSeekerI* reader, double& data) {
    char tmp[128];
    size_t len = _readTxtNumBaseStart(err, reader, tmp, WK_COUNTOF(tmp));
    if(len == UINT32_MAX) return 0;
    char* endptr = nullptr;
    data = strtod(tmp, &endptr);
    return _readTxtNumBaseEnd(err, reader, tmp, endptr);
  }
};

}
}