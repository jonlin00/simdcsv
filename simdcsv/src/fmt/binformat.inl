#include <stdint.h>
#include "../mem/ReaderWriter.h"
#include "../Math/Util.h"

namespace Wikinger {
namespace fmt {

template<typename T>
inline constexpr bool isPrimitive() { return false; }
template<>
inline constexpr bool isPrimitive<int64_t>() { return true; }
template<>
inline constexpr bool isPrimitive<int32_t>() { return true; }
template<>
inline constexpr bool isPrimitive<int16_t>() { return true; }
template<>
inline constexpr bool isPrimitive<int8_t>() { return true; }
template<>
inline constexpr bool isPrimitive<uint64_t>() { return true; }
template<>
inline constexpr bool isPrimitive<uint32_t>() { return true; }
template<>
inline constexpr bool isPrimitive<uint16_t>() { return true; }
template<>
inline constexpr bool isPrimitive<uint8_t>() { return true; }
template<>
inline constexpr bool isPrimitive<double>() { return true; }
template<>
inline constexpr bool isPrimitive<float>() { return true; }

template<typename T>
inline size_t binformat(Error& err, WriterI* fptr, bool isStreamLittleEndian, const T& value) {
  binformatter<T> form;
  return form.write(err, fptr, value, isStreamLittleEndian);
}

template<typename T, typename ...Args>
inline size_t binformat(Error& err, WriterI* fptr, bool isStreamLittleEndian, const T& first, const Args&... args) {
  size_t res = binformat(err, fptr, isStreamLittleEndian, first);
  res += binformat(err, fptr, isStreamLittleEndian, args...);
  return res;
}

template<typename T>
inline size_t binparse(Error& err, ReaderI* fptr, bool isStreamLittleEndian, T& value) {
  binparser<T> form;
  return form.parse(err, fptr, value, isStreamLittleEndian);
}

template<typename T, typename ...Args>
inline size_t binparse(Error& err, ReaderI* fptr, bool isStreamLittleEndian, T& first, Args&... args) {
  size_t res = binparse(err, fptr, isStreamLittleEndian, first);
  res += binparse(err, fptr, isStreamLittleEndian, args...);
  return res;
}

inline uint8_t endianSwap(uint8_t in) { return in; }
inline int8_t endianSwap(int8_t in) { return endianSwap(static_cast<uint8_t>(in)); }

inline uint16_t endianSwap(uint16_t in) { return (in >> 8) | (in << 8); }
inline int16_t endianSwap(int16_t in) { return endianSwap(static_cast<uint16_t>(in)); }

inline uint32_t endianSwap(uint32_t in) {
  return (in >> 24) | (in << 24) | ((in & 0x00ff0000) >> 8) | ((in & 0x0000ff00) << 8);
}
inline int32_t endianSwap(int32_t in) { return endianSwap(static_cast<uint32_t>(in)); }

inline uint64_t endianSwap(uint64_t in) {
  return   (in >> 56) | (in << 56)
    | ((in & UINT64_C(0x00ff000000000000)) >> 40) | ((in & UINT64_C(0x000000000000ff00)) << 40)
    | ((in & UINT64_C(0x0000ff0000000000)) >> 24) | ((in & UINT64_C(0x0000000000ff0000)) << 24)
    | ((in & UINT64_C(0x000000ff00000000)) >> 8) | ((in & UINT64_C(0x00000000ff000000)) << 8);
}
inline int64_t endianSwap(int64_t in) { return endianSwap(static_cast<uint64_t>(in)); }

template <typename T>
inline T toLittleEndian(T in) {
#if WK_CPU_ENDIAN_BIG
  return endianSwap(in);
#else
  return in;
#endif// WK_CPU_ENDIAN_BIG
}

template <typename T>
inline T toBigEndian(T in) {
#if WK_CPU_ENDIAN_LITTLE
  return endianSwap(in);
#else
  return in;
#endif// WK_CPU_ENDIAN_LITTLE
}

template<typename T>
inline T toHostEndian(T in, bool isStreamLittleEndian) {
#if WK_CPU_ENDIAN_LITTLE
  return isStreamLittleEndian ? in : endianSwap(in);
#else
  return isStreamLittleEndian ? endianSwap(in) : in;
#endif// WK_CPU_ENDIAN_LITTLE
}

template<typename T>
struct binparser {
  size_t parse(Error& err, ReaderI* reader, T& value, bool isStreamLittleEndian) {
    WK_STATIC_ASSERT(isPrimitive<T>(), "binReadformatter: Default binary read formatter only works with primitive datatypes");
    alignas(sizeof(T)) char buff[sizeof(T)];
    size_t res = reader->readbin(err, buff, sizeof(T));
    if(err.peekOk()) {
      value = *reinterpret_cast<T*>(buff);
      value = toHostEndian(value, isStreamLittleEndian);
    }
    return res;
  }
};

template<>
struct binparser<float> {
  size_t parse(Error& err, ReaderI* reader, float& value, bool isStreamLittleEndian) {
    alignas(sizeof(float)) char buff[sizeof(float)];
    size_t res = reader->readbin(err, buff, sizeof(float));
    if(err.peekOk()) {
      uint32_t i = *reinterpret_cast<uint32_t*>(buff);
      value = Math::bitsToFloat(toHostEndian(i, isStreamLittleEndian));
    }
    return res;
  }
};

template<>
struct binparser<double> {
  size_t parse(Error& err, ReaderI* reader, double& value, bool isStreamLittleEndian) {
    alignas(sizeof(double)) char buff[sizeof(double)];
    size_t res = reader->readbin(err, buff, sizeof(double));
    if(err.peekOk()) {
      uint64_t i = *reinterpret_cast<uint64_t*>(buff);
      value = Math::bitsToDouble(toHostEndian(i, isStreamLittleEndian));
    }
    return res;
  }
};

template<typename T>
struct binformatter {
  size_t write(Error& err, WriterI* writer, const T& value, bool isStreamLittleEndian) {
    WK_STATIC_ASSERT(isPrimitive<T>(), "binWriteformatter: Default binary write formatter only works with primitive datatypes");
    T e = toHostEndian(value, isStreamLittleEndian);
    char* buff = reinterpret_cast<char*>(const_cast<T*>(&e));
    return writer->writebin(err, buff, sizeof(T));
  }
};

template<>
struct binformatter<float> {
  size_t write(Error& err, WriterI* writer, const float& value, bool isStreamLittleEndian) {
    uint32_t e = toHostEndian(Math::floatToBits(value), isStreamLittleEndian);
    char* buff = reinterpret_cast<char*>(const_cast<uint32_t*>(&e));
    return writer->writebin(err, buff, sizeof(float));
  }
};

template<>
struct binformatter<double> {
  size_t write(Error& err, WriterI* writer, const double& value, bool isStreamLittleEndian) {
    uint64_t e = toHostEndian(Math::doubleToBits(value), isStreamLittleEndian);
    char* buff = reinterpret_cast<char*>(const_cast<uint64_t*>(&e));
    return writer->writebin(err, buff, sizeof(double));
  }
};

}
}
