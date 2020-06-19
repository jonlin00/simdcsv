#ifndef FILEPATH_H
#define FILEPATH_H

#include "../Macros.h"
#include "../fmt/format.h"
#include <string_view>

namespace Wikinger {

struct Fileinfo;

class Filepath {
public:
  enum Dir {
    Current,
    Temp,
    Home,
    Count
  };

  Filepath();
  Filepath(Dir dir);
  Filepath(const char* path);
  Filepath(const std::string_view& view);

  void set(Dir dir);
  void set(const char* path);
  void set(const std::string_view& view);

  void join(const std::string_view& view);

  const char* getPtr() const { return mpath; }
  std::string_view getPath() const;
  std::string_view getFileName() const;
  std::string_view getBaseName() const;
  std::string_view getExt() const;
  std::string_view getString() const;

  bool isAbsolute() const;
  bool isEmpty()    const;

  void clear();

  Filepath& operator=(Dir dir);
  Filepath& operator=(const char* path);
  Filepath& operator=(const std::string_view& view);

  Filepath operator+=(const Filepath& other) { join(other); return *this; }

  operator std::string_view() const;
  operator std::string() const;

private:
  char mpath[WK_MAXFILEPATH];
};

inline Filepath operator+(const Filepath& lhs, const Filepath& rhs) {
  Filepath cpy = lhs;
  cpy.join(rhs);
  return cpy;
}

inline bool operator==(const Filepath& lhs, const Filepath& rhs) {
  return strcmp(lhs.getPtr(), rhs.getPtr()) == 0;
}

template<>
struct fmt::formatter<Filepath> {
  auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template<typename FormatContext>
  auto format(const Filepath& path, FormatContext& ctx) {
    return format_to(ctx.out(), "{}", path.getPtr());
  }
};

}
#endif // FILEPATH_H

