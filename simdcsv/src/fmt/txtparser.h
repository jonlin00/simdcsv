#ifndef WK_TXTPARSER_H
#define WK_TXTPARSER_H

#include "../mem/ReaderWriter.h"

namespace Wikinger {
namespace fmt {

template<typename ...Args>
inline size_t txtparse(Error& err, ReaderSeekerI* fptr, bool isStreamLittleEndian, Args&... args);

template<typename T>
struct txtparser;
// member functions
//   size_t parse(Error& err, ReaderSeekerI* reader, T& value);

}
}

#include "txtparser.inl"

#endif// WK_TXTPARSER_H