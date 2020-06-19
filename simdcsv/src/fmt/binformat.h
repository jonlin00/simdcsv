#ifndef WK_BINFORMAT_H
#define WK_BINFORMAT_H

#include "../Error.h"

namespace Wikinger {
namespace fmt {

template<typename ...Args>
inline size_t binformat(Error& err, WriterI* fptr, bool isStreamLittleEndian, const Args&... args);
template<typename ...Args>
inline size_t binparse(Error& err, ReaderI* fptr, bool isStreamLittleEndian, Args&... args);

template<typename T>
inline constexpr bool isPrimitive();

template<typename T>
struct binparser;
// Member functions
//  size_t parse(Error& err, ReaderI* reader, T& value, bool isStreamLittleEndian);

template<typename T>
struct binformatter;
// Member functions
//  size_t write(Error& err, WriterI* writer, T& value, bool isStreamLittleEndian);

}
}

#include "binformat.inl"

#endif// WK_BINFORMAT_H
