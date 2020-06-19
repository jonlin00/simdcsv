#include "../Error.h"

#include "Filepath.h"
#include "FileInfo.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>

#if !WK_CRT_NONE
# if WK_PLATFORM_WINDOWS
#   include <direct.h>
#   define WINDOWS_LEAN_AND_MEAN
#   include <Windows.h>
# else
#   include <getcwd>
# endif
#endif

#if WK_COMPILER_MSVC
#include <sys/stat.h>
#endif// WK_COMPILER_MSVC

#if WK_PLATFORM_WINDOWS
extern "C" __declspec(dllimport) unsigned long __stdcall GetTempPathA(unsigned long max, char* ptr);
#endif

namespace Wikinger {

#pragma region Platform dependant

bool stat(Fileinfo& outFileInfo, const Filepath& filePath) {
#if WK_CRT_NONE
  return false;
#else
  outFileInfo.size = 0;
  outFileInfo.type = Filetype::Count;
# if WK_COMPILER_MSVC
  struct ::_stat64 st;
  int32_t result = ::_stat64(filePath.getPtr(), &st);

  if(0 != result)
    return false;

  if(0 != (st.st_mode & _S_IFREG))
    outFileInfo.type = Filetype::File;
  else if(0 != (st.st_mode & _S_IFDIR))
    outFileInfo.type = Filetype::Dir;

# else
  struct ::stat st;
  int32_t result = ::stat(filePath.getPtr(), &st);

  if(0 != result)
    return false;

  if(0 != (st.st_mode & _S_IFREG))
    outFileInfo.type = Filetype::File;
  else if(0 != (st.st_mode & _S_IFDIR))
    outFileInfo.type = Filetype::Dir;

# endif// WK_COMPILER_MSVC

  outFileInfo.size = st.st_size;
  return true;

#endif// WK_CRT_NONE
}

bool getEnv(char* out, size_t* inOutSize, const std::string_view& nameView) {
  const size_t nameMax = nameView.length();
  char* name = (char*)alloca(nameMax + 1);
  strncpy_s(name, nameMax + 1, nameView.data(), nameMax);
  name[nameMax] = 0;

#if WK_PLATFORM_WINDOWS
  DWORD len = ::GetEnvironmentVariableA(name, out, (DWORD)*inOutSize);
  bool result = len != 0 && len < *inOutSize;
  *inOutSize = len;
  return result;

#elif WK_PLATFORM_EMSCRIPTEN  \
   || WK_PLATFORM_PS4         \
   || WK_PLATFORM_XBOXONE     \
   || WK_PLATFORM_WINRT       \
   || WK_CRT_NONE
  return false;

#else
  const char* ptr = ::getenv(name);
  uint32_t len = 0;
  bool result = false;
  if(nullptr != ptr) {
    len = (uint32_t)strlen(ptr);
    result = len != 0 && len < *inOutSize
      if(len < *inOutSize)
        strncpy(out, ptr, *inOutSize);
  }
  *inOutSize = len;
  return result;

#endif// WK_PLATFORM_*
}

#pragma endregion

#pragma region StaticMemory
class StaticMemory {
public:
  StaticMemory(char* dst, size_t dstSize) :
    mem(dst), len(0), alloc(dstSize) {}

  size_t write(char ch, bool& err);
  size_t write(const char* str, bool& err);

  void seek(size_t offset);

private:
  char* mem;
  size_t len;
  size_t alloc;
};

size_t StaticMemory::write(char ch, bool& err) {
  if(len + 1 >= alloc) {
    err = true;
    return 0;
  }
  else {
    mem[len++] = ch;
    return 1;
  }
}

size_t StaticMemory::write(const char* str, bool& err) {
  size_t al = strlen(str);
  if(len + al >= alloc) {
    err = true;
    return 0;
  }
  else {
    memcpy(mem, str, al);
    len += al;
    return al;
  }
}

void StaticMemory::seek(size_t offset) {
  len = offset;
}
#pragma endregion

bool isPathSeperator(char ch) {
  return false || '/' == ch || '\\' == ch;
}

size_t normalizeFilepath(char* dst, size_t dstSize, const char* src, size_t num) {
  const size_t len = strnlen(src, num);

  if(0 == num) {
    strncpy_s(dst, dstSize, ".", dstSize);
    return strlen(dst);
  }

  size_t size = 0;
  StaticMemory writer(dst, dstSize);
  bool err = false;

  size_t idx = 0;
  size_t dotdot = 0;

  if(2 <= num && ':' == src[1]) {
    size += writer.write(toupper(src[idx]), err);
    size += writer.write(':', err);
    idx += 2;
    dotdot = size;
  }

  const size_t slashIdx = idx;
  bool rooted = isPathSeperator(src[idx]);
  if(rooted) {
    size += writer.write('/', err);
    idx += 1;
    dotdot = size;
  }

  bool trailingSlash = false;

  while(idx < num && !err) {
    switch(src[idx]) {
      case '/':
      case '\\':
        ++idx;
        trailingSlash = idx == num;
        break;

      case '.':
        if(idx + 1 == num || isPathSeperator(src[idx + 1])) {
          ++idx;
          break;
        }

        if('.' == src[idx + 1] &&
           (idx + 2 == num || isPathSeperator(src[idx + 2]))) {
          idx += 2;
          if(dotdot < size) {
            for(--size; dotdot < size && !isPathSeperator(dst[size]); --size) {
            }
            writer.seek(size);
          }
          else if(!rooted) {
            if(0 < size)
              size += writer.write('/', err);

            size += writer.write("..", err);
            dotdot = size;
          }
          break;
        }
        // Fallthrough

      default:
        if((rooted && slashIdx + 1 != size)
           || (!rooted && 0 != size))
          size += writer.write('/', err);

        for(; idx < num && !isPathSeperator(src[idx]); ++idx)
          size += writer.write(src[idx], err);
        break;
    }
  }

  if(size == 0)
    size += writer.write('.', err);

  if(trailingSlash)
    size += writer.write('/', err);

  writer.write('\0', err);

  return size;
}

bool getEnv(char* out, size_t* inOutSize, const std::string_view& name, Filetype type) {
  size_t len = *inOutSize;
  *out = 0;
  if(getEnv(out, &len, name)) {
    Fileinfo fi;
    if(stat(fi, out) && type == fi.type) {
      *inOutSize = len;
      return true;
    }
  }
  return false;
}

char* pwd(char* buff, size_t size) {
#if WK_PLATFORM_PS4     \
 || WK_PLATFORM_XBOXONE \
 || WK_PLATFORM_WINRT   \
 || WK_CRT_NONE
  return nullptr;

#elif WK_CRT_MSVC
  return ::_getcwd(buff, (int)size);

#else
  return ::getcwd(buff, size);
#endif// WK_PLATFORM_*
}

bool getCurrentPath(char* out, size_t* inOutSize) {
  size_t len = *inOutSize;
  if(nullptr != pwd(out, len)) {
    *inOutSize = strlen(out);
    return true;
  }
  return false;
}

bool getHomePath(char* out, size_t* inOutSize) {
  return false
#if WK_PLATFORM_WINDOWS
    || getEnv(out, inOutSize, "USERPROFILE", Filetype::Dir)
#endif// WK_PLATFORM_WINDOWS
    || getEnv(out, inOutSize, "HOME", Filetype::Dir)
    ;
}

bool getTempPath(char* out, size_t* inOutSize) {
#if WK_PLATFORM_WINDOWS
  size_t len = ::GetTempPathA((DWORD)*inOutSize, out);
  bool result = len != 0 && len < *inOutSize;
  *inOutSize = len;
  return result;

#else
  static const std::string_view stmp[] = {
    "TMPDIR",
    "TMP",
    "TEMP",
    "TEMPDIR",
    "",
  };
  for(const std::string_view* tmp = stmp; !tmp->isEmpty(); ++tmp) {
    uint32_t len = *inOutSize;
    out = 0;
    bool ok = getEnv(out, &len, *tmp, Filetype::Dir);

    if(ok
       && len != 0
       && len < *inOutSize) {
      *inOutSize = len;
      return ok;
    }
  }

  FileInfo fi;
  if(stat(fi, "/tmp")
     && Filetype::Dir == fi.type) {
    strncpy(out, "/tmp", *inOutSize);
    *inOutSize = 4;
    return true;
  }
  return false;

#endif// WK_PLATFORM_*
}

Filepath::Filepath() {
  set("");
}

Filepath::Filepath(Dir dir) {
  set(dir);
}

Filepath::Filepath(const char* rhs) {
  set(rhs);
}

Filepath::Filepath(const std::string_view& filepath) {
  set(filepath);
}

void Filepath::set(Dir dir) {
  char tmp[WK_MAXFILEPATH];
  size_t len = WK_COUNTOF(tmp);

  switch(dir) {
    case Dir::Current:
      getCurrentPath(tmp, &len);
      break;

    case Dir::Temp:
      getTempPath(tmp, &len);
      break;

    case Dir::Home:
      getHomePath(tmp, &len);
      break;

    default:
      len = 0;
      break;
  }

  set(std::string_view(tmp, len));
}

void Filepath::set(const char* path) {
  std::string_view view = path;
  set(view);
}

void Filepath::set(const std::string_view& filepath) {
  normalizeFilepath(
    mpath,
    WK_MAXFILEPATH,
    filepath.data(),
    filepath.length()
  );
}

void Filepath::join(const std::string_view& str) {
  char tmp[WK_MAXFILEPATH];
  strncpy_s(tmp, WK_MAXFILEPATH, mpath, WK_MAXFILEPATH);
  strncat_s(tmp, "/", WK_MAXFILEPATH);
  size_t len = strlen(tmp);
  strncat_s(tmp, str.data(), len + str.length() > WK_MAXFILEPATH ? WK_MAXFILEPATH : str.length());
  set(tmp);
}

void Filepath::clear() {
  if(!isEmpty()) {
    set("");
  }
}

std::string_view Filepath::getPath() const {
  const char* ptr = strrchr(mpath, '/');
  if(ptr != nullptr) {
    size_t len = ptr - mpath;
    return std::string_view(mpath, len);
  }
  else {
    return std::string_view(mpath);
  }
}

std::string_view Filepath::getFileName() const {
  const char* ptr = strrchr(mpath, '/');
  if(ptr != nullptr) {
    return std::string_view(ptr + 1);
  }
  else {
    return std::string_view(mpath);
  }
}

std::string_view Filepath::getBaseName() const {
  const std::string_view fileName = getFileName();
  if(!fileName.empty()) {
    const char* dot = strrchr(fileName.data(), '.');
    if(dot == nullptr) {
      return fileName;
    }
    else {
      size_t len = dot - fileName.data();
      return std::string_view(fileName.data(), len);
    }
  }
  return std::string_view();
}

std::string_view Filepath::getExt() const {
  const std::string_view fileName = getFileName();
  if(!fileName.empty()) {
    const char* dot = strrchr(fileName.data(), '.');
    if(dot == nullptr) {
      return fileName;
    }
    else {
      return std::string_view(dot + 1);
    }
  }
  return std::string_view();
}

std::string_view Filepath::getString() const {
  return std::string_view(mpath);
}

bool Filepath::isAbsolute() const {
  return '/' == mpath[0]
    || (':' == mpath[1] && '/' == mpath[2]);
}

bool Filepath::isEmpty() const {
  return 0 == strcmp(mpath, ".");
}

Filepath::operator std::string_view() const {
  return getString();
}

Filepath::operator std::string() const {
  return std::string(getString());
}

Filepath& Filepath::operator=(Dir rhs) {
  set(rhs);
  return *this;
}

Filepath& Filepath::operator=(const char* rhs) {
  set(rhs);
  return *this;
}

Filepath& Filepath::operator=(const std::string_view& rhs) {
  set(rhs);
  return *this;
}

}