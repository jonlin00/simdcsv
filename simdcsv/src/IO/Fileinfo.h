#ifndef FILEINFO_H
#define FILEINFO_H

#include "Filepath.h"

namespace Wikinger {
enum class Filetype : uint8_t {
  File,
  Dir,
  Count
};

struct Fileinfo {
  Filepath  filePath;
  uint64_t  size = 0;
  Filetype  type = Filetype::Count;
};

}

#endif// FILEINFO_H
