#ifndef WK_CSVREADER_H
#define WK_CSVREADER_H

#include "../Error.h"
#include "FileReader.h"

#include <string_view>

namespace Wikinger {

class CSVReader {
public:
  class Token;

  typedef void(Callback)(Error& err, uint32_t row, uint32_t col, Token& tk);

public:
  class Token {
  public:
    Token(std::string_view m) : mem(m) {}

    template<typename T>
    T get(Error& err);

    template<>
    std::string_view get(Error& err) {
      return mem;
    }

  private:
    std::string_view mem;
  };

  template<typename F>
  Error& readHeader(Error& err, F& clb);
  template<typename F>
  Error& read(Error& err, F& clb);

  char getSep() const;
  void setSep(char s);

  Error& open(Error& err, const Filepath& path);
  void close();

  bool isOpen() const;

private:
  char seperator = ',';
  uint32_t row = 0;
  uint32_t column = 0;

  UnbufferedFileReader reader;
  size_t req_alignment;
};

}

#include "CSVReader.inl"

#endif// WK_CSVREADER_H
