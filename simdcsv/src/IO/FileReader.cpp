#include "../Platform.h"
#include "FileReader.h"
#include "../Error.h"

#if WK_PLATFORM_WINDOWS || WK_PLATFORM_XBOXONE || WK_PLATFORM_WINRT
#include <Windows.h>
#endif

#if WK_PLATFORM_POSIX

#endif

namespace Wikinger {

UnbufferedFileReader::UnbufferedFileReader()
  : hnd(INVALID_HANDLE_VALUE), meof(false) {}
  
UnbufferedFileReader::~UnbufferedFileReader() {
  close();
}

Error& UnbufferedFileReader::open(Error& err, const Filepath& path) {
  if(!err.peekOk()) {
    return err;
  }
  else if(isOpen()) {
    WK_RAISE_ERR(err, AlreadyOpen, "FileReader: A file is already open, cannot open '{}'", path);
    return err;
  }
  else {
    hnd = CreateFileA(path.getPtr(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                      OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, nullptr);

    if(hnd == INVALID_HANDLE_VALUE) {
      WK_RAISE_ERR(err, CannotOpen, "FileReader: Couldn't open file '{}'", path);
    }
    else {
      meof = false;
    }

    return err;
  }
}

void UnbufferedFileReader::close() {
  if(isOpen()) {
    CloseHandle(hnd);
    hnd = INVALID_HANDLE_VALUE;
    meof = false;
  }
}

bool UnbufferedFileReader::isOpen() const {
  return hnd != INVALID_HANDLE_VALUE;
}

size_t UnbufferedFileReader::readbin(Error& err, void* data, size_t size) {
  DWORD read = 0;
  bool res = ReadFile(hnd, data, (DWORD)size, &read, nullptr);
  meof = res && read == 0;
  return read;
}

int64_t UnbufferedFileReader::seek(Error& err, int64_t offset, Whence whence) {
  LARGE_INTEGER mv;
  LARGE_INTEGER nw;
  mv.QuadPart = offset;
  if(SetFilePointerEx(hnd, mv, &nw, static_cast<DWORD>(whence))) {
    return nw.QuadPart;
  }
  else {
    return 0;
  }
}

size_t UnbufferedFileReader::getAlignment(const Filepath& path) const {
  char partition[MAX_PATH];
  Filepath tmp = path;

  // retrieve a path to the partition from argument path.
  if(!path.isAbsolute()) {
    tmp = Filepath::Dir::Current;
  }
  else {
    memcpy(partition, "\\\\.\\", 4);
    partition[4] = path.getPtr()[0];
    partition[5] = ':';
    partition[6] = '\0';
  }

  // This will fail with error code 5 (Access denied) if run without admin privileges,
  // it won't prompt the user for access.
  HANDLE phnd = CreateFileA(partition, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING, 0, nullptr);

  if(phnd != INVALID_HANDLE_VALUE) {
    STORAGE_PROPERTY_QUERY query;
    query.PropertyId = StorageAccessAlignmentProperty;
    query.QueryType = PropertyStandardQuery;
    STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR align;
    DWORD actual = 0;
    bool res = DeviceIoControl(phnd, IOCTL_STORAGE_QUERY_PROPERTY,
                               &query, sizeof(query), &align, sizeof(align), &actual, nullptr);

    if(res && actual >= sizeof(align.BytesPerPhysicalSector) +
       WK_OFFSETOF(STORAGE_ACCESS_ALIGNMENT_DESCRIPTOR, BytesPerPhysicalSector)) {
      return align.BytesPerPhysicalSector;
    }

    CloseHandle(phnd);
  }
  else {
    DWORD e = GetLastError();
    WK_DEBUG("FileReader: Create handle exited with code '{}'", e);
  }
  
  // If the physical sector size cannot be retrieved, return the page size instead
  // This assumption that the page size is more restrictive hold true in the general case
  // Most disks have a sector size of 512 bytes but newer hardware may use the "Advanced format" instead
  // this format has a sector size of 4096 bytes.
  // The page size however is 4096 bytes in x86 and x64 applications and 8192 on itanium based systems.
  // As such in general restricting the alignment to the page size is sufficient when the
  // sector size cannot be retrieved.
  // If this assumption doesn't hold true the application should be restarted with admin privileges.

  SYSTEM_INFO info;
  GetNativeSystemInfo(&info);
  return info.dwPageSize;
}

int64_t UnbufferedFileReader::tell() const {
  LARGE_INTEGER mv;
  LARGE_INTEGER nw;
  mv.QuadPart = 0;
  if(SetFilePointerEx(hnd, mv, &nw, FILE_CURRENT)) {
    return nw.QuadPart;
  }
  else {
    return 0;
  }
}

bool UnbufferedFileReader::eof() const {
  return meof;
}

int64_t UnbufferedFileReader::size(Error& err) const {
  UnbufferedFileReader* rd = const_cast<UnbufferedFileReader*>(this);
  int64_t off = rd->tell();
  int64_t start = rd->seek(err, 0, Whence::Begin);
  int64_t end = rd->seek(err, 0, Whence::End);
  rd->seek(err, off, Whence::Begin);
  return end - start;
}

FileReader::FileReader() :
  UnbufferedFileReader(), cache(nullptr), cacheSize(0),
  cacheAlign(0), curr(nullptr), end(nullptr) {}

FileReader::~FileReader() {
  close();
}

Error& FileReader::open(Error& err, const Filepath& path) {
  UnbufferedFileReader::open(err, path);
  if(err.peekOk()) {
    createCache(path);
  }
  return err;
}

void FileReader::close() {
  UnbufferedFileReader::close();

  if(cache != nullptr) {
    _aligned_free(cache);
    cache = nullptr;
    cacheSize = 0;
    cacheAlign = 0;
    curr = nullptr;
    end = nullptr;
  }
}

int64_t FileReader::seek(Error& err, int64_t offset, Whence whence) {
  if(whence == Whence::Current) {
    curr += offset;
    if(curr >= end || curr < cache) {
      curr = cache;
      end = cache;

      return UnbufferedFileReader::seek(err, curr - end, whence);
    }
    return tell();
  }
  else {
    curr = cache;
    end = cache;

    return UnbufferedFileReader::seek(err, curr - end, whence);
  }
}

int64_t FileReader::tell() const {
  return UnbufferedFileReader::tell() + curr - cache;
}

size_t FileReader::readbin(Error& err, void* data, size_t size) {
  char* pdata = reinterpret_cast<char*>(data);
  if(isOpen()) {
    size_t avail = end - curr;
    if(size >= cacheSize) {
      memcpy(pdata, curr, avail);

      size_t read = avail;
      size_t batch = 0;
      size_t fetchIter = size / cacheSize;
      size_t fetchRem = size % cacheSize;

      for(size_t iter = 0; iter < fetchIter; iter++) {
        batch = UnbufferedFileReader::readbin(err, cache, cacheSize);
        if(batch == 0)
          return read;

        memcpy(pdata + read + iter * cacheSize, cache, batch);
        read += batch;
      }
      batch = UnbufferedFileReader::readbin(err, cache, cacheSize);
      size_t lastbit = batch < fetchRem ? batch : fetchRem;

      memcpy(pdata + read, cache, lastbit);
      read += lastbit;

      curr = cache + lastbit;
      end  = cache + batch;

      return read;
    }
    else if(avail >= size) {
      memcpy(pdata, curr, size);
      curr += size;
      return size;
    }
    else { // avail < size && size < cacheSize
      memcpy(pdata, curr, avail);

      size_t batch = UnbufferedFileReader::readbin(err, cache, cacheSize);
      size_t fetchRem = size - avail;
      size_t lastbit = batch < fetchRem ? batch : fetchRem;

      memcpy(pdata + avail, cache, lastbit);

      curr = cache + lastbit;
      end = cache + batch;

      return avail + lastbit;
    }
  }
  else {
    WK_RAISE_ERR(err, NotOpen, "FileReader: no file is open");
    return 0;
  }
}

size_t FileReader::getAlignment() const {
  return cacheAlign;
}

size_t FileReader::getCacheSize() const {
  return cacheSize;
}

char* FileReader::pushCache(Error& err, size_t sz, uint64_t& read) {
  char* res = curr;
  curr += sz;
  read = sz;
  if(curr >= end) {
    res = cache;
    size_t batch = UnbufferedFileReader::readbin(err, cache, cacheSize);
    read = batch < sz ? batch : sz;
    curr = cache + read;
    end  = cache + batch;
  }
  return res;
}

void FileReader::createCache(const Filepath& path) {
  size_t alignment = UnbufferedFileReader::getAlignment(path);
  size_t size = 1024 * 1024 * 4;

  // size is now an integer of alignment.
  size += alignment / 2;
  size = size - size % alignment;

  cache = (char*)_aligned_malloc(size, alignment);
  cacheSize = size;
  cacheAlign = alignment;
  curr = cache;
  end = cache;
}

}