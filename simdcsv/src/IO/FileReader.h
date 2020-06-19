#ifndef WK_FILEREADER_H
#define WK_FILEREADER_H

#include "Filepath.h"
#include "../ReaderWriter.h"

namespace Wikinger {

class UnbufferedFileReader : public virtual ReaderSeekerI {
public:
  UnbufferedFileReader();
  virtual ~UnbufferedFileReader();

  Error& open(Error& err, const Filepath& path);
  void close();
  bool isOpen() const;

  virtual size_t readbin(Error& err, void* data, size_t size) override;
  virtual int64_t seek(Error& err, int64_t offset = 0, Whence whence = Whence::Current) override;

  bool eof() const;
  // Coding while itoxicated makes for interesting code
  size_t getAlignment(const Filepath& path) const;
  int64_t size(Error& err) const;
  int64_t tell() const;

private:
  void* hnd;
  bool meof;
};

class FileReader : public UnbufferedFileReader {
public:
  FileReader();
  virtual ~FileReader();

  Error& open(Error& err, const Filepath& path);
  void close();

  size_t getAlignment() const;
  size_t getCacheSize() const;
  char* pushCache(Error& err, size_t sz, uint64_t& read);

  virtual size_t readbin(Error& err, void* data, size_t size) override;
  virtual int64_t seek(Error& err, int64_t offset = 0, Whence whence = Whence::Current) override;
  int64_t tell() const;

private:
  void createCache(const Filepath& path);

  char* cache;
  size_t cacheSize;
  size_t cacheAlign;
  char* curr;
  char* end;
};

}

#endif// WK_FILEREADER_H