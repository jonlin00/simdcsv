#include "../RuntimeDispatch.h"

// this dependency is not used but provided because it is nice
// remove this include and the detail::csv::readCSV function at your own discretion
#include <vector>

#include <immintrin.h>

namespace Wikinger {

Error& CSVReader::open(Error& err, const Filepath& path) {
  row = 0;
  column = 0;
  Error& res = reader.open(err, path);
  req_alignment = reader.getAlignment(path);
  return res;
}

void CSVReader::close() {
  reader.close();
}

bool CSVReader::isOpen() const {
  return reader.isOpen();
}

char CSVReader::getSep() const {
  return seperator;
}

void CSVReader::setSep(char s) {
  seperator = s;
}

namespace detail {
namespace csv {

typedef CSVReader::Callback csvclb;

class CSVFileReader {
public:
  CSVFileReader(UnbufferedFileReader& base, size_t alignment);
  ~CSVFileReader();

  char* pushCache(Error& err, size_t sz, uint64_t& read);
  void settk(char* tk) { tkprev = tk; }
  char* getCurr() const { return curr; }
  char* getPrev() const { return tkprev; }
  bool eof() const { return reader.eof(); }

  void seek(Error& err, int64_t off, Whence wh = Whence::Current) { reader.seek(err, off, wh); }
  bool isOpen() const { return reader.isOpen(); }
  size_t getCacheAlignment() const { return alignment; }
  size_t getCacheSize() const { return size; }

  int64_t getRemBytes() const { return size + cache - tkprev; }

  bool hasDangling() const { return eof() && tkprev >= end; }
  std::string_view getDangling() const { return std::string_view(tkprev, end - tkprev); }

private:
  void createCache(size_t alignment);
  void destroyCache();

  UnbufferedFileReader& reader;
  char* cache;
  size_t alignment;
  size_t size;
  char* curr;
  char* end;
  char* tkprev;
};

CSVFileReader::CSVFileReader(UnbufferedFileReader& base, size_t _alignment) :
  reader(base), cache(nullptr), alignment(0), size(0),
  curr(nullptr), end(nullptr), tkprev(nullptr) {
  createCache(_alignment);
}

CSVFileReader::~CSVFileReader() {
  destroyCache();
}

void CSVFileReader::createCache(size_t align) {
  if(cache == nullptr) {
    size_t i_want_to_be_size = 1024 * 1024 * 4;

    // size is now an integer of alignment.
    i_want_to_be_size += align / 2;
    i_want_to_be_size = i_want_to_be_size - i_want_to_be_size % align;

    if(i_want_to_be_size == 0)
      i_want_to_be_size = align;

    cache = (char*)_aligned_malloc(i_want_to_be_size, align);
    size = i_want_to_be_size;
    alignment = align;
    curr = cache;
    end = cache;
    tkprev = cache;
  }
}

void CSVFileReader::destroyCache() {
  if(cache != nullptr) {
    _aligned_free(cache);
    cache = nullptr;
    tkprev = nullptr;
    curr = nullptr;
    end = nullptr;
    size = 0;
    alignment = 0;
  }
}

char* CSVFileReader::pushCache(Error& err, size_t sz, uint64_t& read) {
  read = sz;
  if(curr >= end) {
    ptrdiff_t cpy = end - tkprev;
    ptrdiff_t aligned_cpy = (1 + (cpy - 1) / alignment) * alignment;
    ptrdiff_t off = aligned_cpy - cpy;
    memcpy(cache + off, tkprev, cpy);
    size_t batch = reader.readbin(err, cache + aligned_cpy, size - aligned_cpy);

    read   = batch < sz ? batch : sz;
    curr   = cache + aligned_cpy;
    tkprev = cache + off;
    end    = cache + aligned_cpy + batch;
  }

  char* res = curr;
  curr += sz;
  return res;
}

// Represents the context of the csv parser.
struct CSV_Context {

  // This carry keeps track of whether the last iteration ended
  // inside a quote and as such if the current iteration should
  // begin with one.
  uint64_t quote_carry;

  // This carry keeps track of whether the last iteration ended on
  // a backslash character, and as such if the first character in this
  // iteration should be escaped.
  uint64_t esc_carry;

  // This is a mask where every bit represents whether
  // the corresponding byte in strBuff equals an quote character.
  uint64_t quote_mask;

  // This is a mask where every bit represents whether
  // the corresponding byte in strBuff equals the seperation character.
  uint64_t sep_mask;

  // This is a mask where every bit represents whether
  // the corresponding byte in strBuff equals an newline character.
  uint64_t nl_mask;

  // This is a mask where every bit represents whether
  // the corresponding byte in strBuff equals a backslash character.
  uint64_t esc_mask;

  // An example of the different bitmasks
  //    strBuff => "TheBrown",Fox,\"jumps over\",the lazy","dog"\n
  // quote_mask => 1________1______1___________1_________1_1___1_
  //   sep_mask => __________1___1______________1_________1______
  //    nl_mask => _____________________________________________1
  //   esc_mask => _______________1___________1__________________
  // bit n in a mask corresponds to byte n in strBuff.
  // (fun but ultimatly unnecessary note: the example string is only
  //  48 bytes long and is missing an additional 16 bytes,
  //  and the same goes for the masks).

  // The current row which is being parsed.
  // This value begins at zero and counts up.
  // Note that this does not necessarily equal the amount of lines in the document
  // since if newlines are encased in quotes then they are not counted.
  uint32_t& row;

  // The current column which is being parsed.
  // This value begins at zero and counts up. It is reset back
  // to zero whenever the row value whenever the row value increments.
  // A column is delimited by the seperation character.
  uint32_t& column;

  // Constructs a context object.
  // The buff pointer must be 64 bytes, the foo pointer cannot be null and pr, pc must be valid references.
  CSV_Context(uint32_t& pr, uint32_t& pc);
};

// Constructs a context object.
CSV_Context::CSV_Context(uint32_t& pr, uint32_t& pc) :
  quote_carry(0), esc_carry(0),
  quote_mask(0), sep_mask(0),
  nl_mask(0), esc_mask(0),
  row(pr), column(pc) {}

// Count trailing zeroes using the BMI1 instruction set extension.
// This function has to exist becouse the compiler intrinsic inside
// has no function address and cannot therefore be pointed at by the
// template arguments.
WK_FORCE_INLINE uint64_t tzcnt_bmi(uint64_t v) {
  return _tzcnt_u64(v);
}

// Count trailing zeroes using regular x64 instructions.
// This is *MUCH* slower than the bmi method.
WK_FORCE_INLINE uint64_t tzcnt_x64(uint64_t v) {
  uint64_t res = 0;
  while((v >> res & 1) == 0 && res < 64) {
    res++;
  }
  return res;
}

typedef uint64_t(tzcntSig)(uint64_t);

// Performs an bitwise fused not and operation using the
// BMI1 instruction set extension.
// This function has to exist becouse the compiler intrinsic inside
// has no function address and cannot therefore be pointed at by the
// template arguments.
WK_FORCE_INLINE uint64_t andn_bmi(uint64_t a, uint64_t b) {
  return _andn_u64(a, b);
}

// Performs an bitwise fused not and operation using the x64 instructions.
WK_FORCE_INLINE uint64_t andn_x64(uint64_t a, uint64_t b) {
  return ~a & b;
}

typedef uint64_t(andnSig)(uint64_t, uint64_t);

// This csv parser implements an finite state machine in order to parse a file.
// This implementation differs from the other implementations in how it deals with
// escape characters, because here the proceeding backslash is removed.
// The latter implementations does not remove any backslash.
// This is succeded by the later implementations and is actually never used by any other function.
// It is kept for no other reason than that it is good code.
template<bool rtnOnNL, typename F>
Error& readCSV(Error& err, F& clb, uint32_t& row, uint32_t& column, char seperator, FileReader& reader) {
  // Check whether the arguments are valid...
  if(!err.peekOk())
    return err;

  if(!reader.isOpen()) {
    WK_RAISE_ERR(err, NotOpen, "CSVReader: no file open to read");
    return err;
  }

  // Definitions of all the different states this state machine can assume
  enum StateEnum : uint8_t {
    // the text state is the "normal" one since it is used when:
    //   - no quote has been encountered
    //   - no charcters been escaped
    text,

    // the escaped state is only used when the proceeding character is
    // an backslash. The state returned to when the current character
    // has been parsed is governed by the variable escapeRetState
    escaped,

    // This state is for when quoted text has been encountered.
    // What seperates this state from the text state is that this
    // one will happily gobble up any character that isn't an
    // non-escaped quote character.
    quoted
  };

  // Current char the state machine is pondering about.
  char chr;
  StateEnum state = text;
  StateEnum escapeRetState = text;

  std::vector<char> mem;
  mem.reserve(4096);

  CSVReader::Token tk = std::string_view();

  while(reader.readbin(err, &chr, 1) == 1 && err.peekOk()) {
    switch(state) {
      case text:
        if(chr == '\n') {
          tk = std::string_view(mem.data(), mem.size());
          clb(err, row, column, tk);
          row++;
          column = 0;
          mem.clear();

          if(rtnOnNL) {
            return err;
          }
        }
        else if(chr == seperator) {
          tk = std::string_view(mem.data(), mem.size());
          clb(err, row, column, tk);
          column++;
          mem.clear();
        }
        else if(chr == '\\') {
          escapeRetState = text;
          state = escaped;
        }
        else if(chr == '"') {
          state = quoted;
        }
        else {
          mem.push_back(chr);
        }
        break;
      case quoted:
        if(chr == '\\') {
          state = escaped;
          escapeRetState = quoted;
        }
        else if(chr == '"') {
          state = text;
        }
        else {
          mem.push_back(chr);
        }
        break;
      case escaped:
        if(chr == seperator) {
          mem.push_back(chr);
        }
        else if(chr == '\\') {
          mem.push_back(chr);
        }
        else if(chr == '"') {
          mem.push_back(chr);
        }
        else {
          mem.push_back('\\');
          mem.push_back(chr);
        }
        state = escapeRetState;
        break;
    }
  }

  if(mem.size() != 0) {
    tk = std::string_view(mem.data(), mem.size());
    clb(err, row, column, tk);
  }

  return err;
}

// This function is the base implementation of the csv parser.
// It is parsing and invoking the callback once it finds a token.
// The template argument rtnOnNL is spelled out to Return On NewLine
// When set to true the function will return after a single newline
// character has been found.
// function returns true if rtnOnNL is true and an newline was encountered.
// The other two template arguments, tzcnt and andn, can be one of
// tzcnt_bmi, tzcnt_x64, andn_bmi or andn_x64 depending on architecture.
// The function arguments ctx is the current context this parser is in.
// The read argument is the amount of bytes successfully read and as such no
// more than read bytes should be parsed from the context.
// (yes a *minor* code explosion is taking place here)
template<bool rtnOnNL, tzcntSig tzcnt, andnSig andn, typename F>
bool readCSV_Impl(Error& err, F& clb, CSV_Context& ctx, uint64_t read, CSVFileReader& reader) {
  CSVReader::Token tk = std::string_view();

  // Since the amount of data read can be less than the expected 64
  // so must any dangling bits be cleared.
  // A bit less intuitivly but equally true, this clearing also
  // ensures that all data read in the do while loop later on is valid.
  uint64_t read_mask = 0xffffffffffffffff >> (64 - read);
  ctx.esc_mask &= read_mask;
  ctx.nl_mask &= read_mask;
  ctx.quote_mask &= read_mask;
  ctx.sep_mask &= read_mask;

  uint64_t quote_mask_fill;
  // since the last bit in the esc_mask is useless (in this iteration) due to the fact
  // that a backslash by definition proceeds something.
  // So whats being done here is the esc_carry from the last iteration is inserted here.
  // the current esc_mask must be shifted since the esc_carry might escape a backslash in
  // the current iteration
  uint64_t esc_cpy = ctx.esc_mask << 1 | ctx.esc_carry;

  // TODO: fix esc_carry, an esc_carry should only be generated if the below magic statement overflows.
  ctx.esc_carry = 0;

  // Now now, This is going to be explained, somehow, alright it's blood magic this expression
  // references here https://youtu.be/wlvKAT7SZIQ?t=2029
  // It's somewhere in the code at simdjson's github but I don't know where.
  // What it does is it takes the all the backslashes and tell which character is escaped and which isn't
  // An example
  //  strBuff => "\\\"Nam[{": [ 116,"\\\\"
  // esc_mask => _111________________1111_
  //  esc_cpy => ____1____________________
  auto ovfDetect = [&ctx](uint64_t a, uint64_t b) -> uint64_t {
    uint64_t sum = a + b;
    ctx.esc_carry |= (sum < a);
    return sum;
  };
  // when this statement takes 5% of this functions runtime you know it's pretty darn optimized.
  esc_cpy = andn(0xaaaaaaaaaaaaaaaa, andn(esc_cpy, ovfDetect(esc_cpy, (andn(esc_cpy << 1, esc_cpy) & 0xaaaaaaaaaaaaaaaa)))) |
    (andn(esc_cpy, ovfDetect(esc_cpy, (andn(esc_cpy << 1, esc_cpy) & 0x5555555555555555))) & 0xaaaaaaaaaaaaaaaa);
  ctx.esc_mask = esc_cpy >> 1;

  // Remove any escaped quotes from the mask
  ctx.quote_mask = andn(ctx.esc_mask, ctx.quote_mask);

  // Fill in the gaps of the quote_mask, this in turns gives a mask
  // denoting string contents.
  // An example
  //    strBuff => CSV,"Yay","Comments",take,"too",long,"to",write
  // quote_mask => ____1___1_1________1______1___1______1__1______
  // quote_fill => ____11111_1111111111______11111______1111______
  quote_mask_fill = ctx.quote_mask | ctx.quote_carry;
  quote_mask_fill = quote_mask_fill ^ (quote_mask_fill << 1);
  quote_mask_fill = quote_mask_fill ^ (quote_mask_fill << 2);
  quote_mask_fill = quote_mask_fill ^ (quote_mask_fill << 4);
  quote_mask_fill = quote_mask_fill ^ (quote_mask_fill << 8);
  quote_mask_fill = quote_mask_fill ^ (quote_mask_fill << 16);
  quote_mask_fill = quote_mask_fill ^ (quote_mask_fill << 32);

  // since the actual quote characters are of no use they are removed
  // An example
  //    strBuff => CSV,"Yay","Comments",take,"too",long,"to",write
  // quote_mask => ____1___1_1________1______1___1______1__1______
  // quote_fill => _____111___11111111________111________11_______
  quote_mask_fill = andn(ctx.quote_mask, quote_mask_fill);

  // remove any seperation or newlines inside quotes
  ctx.sep_mask = andn(quote_mask_fill, ctx.sep_mask);
  ctx.nl_mask = andn(quote_mask_fill, ctx.nl_mask);

  // is the last bit inside quotes? is so it carries into
  // the next iteration
  ctx.quote_carry = quote_mask_fill >> 63;

  // This here is what parses out the tokens from strBuff using
  // the masks that have been computed previously.
  // It also invokes the func on any token found.

  // the amount of bits in the sep_mask until the next set bit, or 64
  uint64_t next_sep;
  // the amount of bits in the nl_mask until the next set bit, or 64
  uint64_t next_nl;
  // the amount of bits in the quote_mask_fill until the next set bit, or 64
  uint64_t next_quote;
  // the amount of set bits counted from next_quote in the quote_mask_fill mask, or 0
  uint64_t quote_len;

  uint64_t prev = 0;
  char* tk_start;
  char* p_rel = reader.getCurr() - 64;

  do {
    next_sep   = tzcnt(ctx.sep_mask);
    next_nl    = tzcnt(ctx.nl_mask);
    next_quote = tzcnt(quote_mask_fill >> prev) + prev;
    quote_len  = tzcnt(~(quote_mask_fill >> next_quote));

    tk_start = reader.getPrev();

    if(next_sep < next_nl && next_sep < 64) {
      if(next_quote < next_sep) {
        tk = std::string_view(p_rel + next_quote, quote_len);
      }
      else {
        tk = std::string_view(tk_start, p_rel + next_sep - tk_start);
      }
      clb(err, ctx.row, ctx.column, tk);
      ctx.column++;
      prev = next_sep + 1;
      reader.settk(p_rel + next_sep + 1);
      // clears the sep_mask so that we don't find this same instance on the next iteration
      ctx.sep_mask = andn(WK_BIT(next_sep), ctx.sep_mask);
    }
    else if(next_nl < 64) {
      if(next_quote < next_nl) {
        tk = std::string_view(p_rel + next_quote, quote_len);
        //ctx.mem.writebin(err, ctx.strBuff + next_quote, quote_len);
      }
      else {
        tk = std::string_view(tk_start, p_rel + next_nl - tk_start);
        //ctx.mem.writebin(err, ctx.strBuff + prev, next_nl - prev);
      }
      clb(err, ctx.row, ctx.column, tk);
      ctx.row++;
      ctx.column = 0;
      prev = next_nl + 1;
      reader.settk(p_rel + next_nl + 1);
      // clears the nl_mask so that we don't find this same instance on the next iteration
      ctx.nl_mask = andn(WK_BIT(next_nl), ctx.nl_mask);

      if(rtnOnNL) {
        // TODO: seek
        reader.seek(err, -reader.getRemBytes());
        return true;
      }
    }
    else {
      //// if no delimiter was found copy the data into the buffer and exit.
      //if(next_quote < 64) {
      //  //ctx.mem.writebin(err, ctx.strBuff + next_quote, quote_len);
      //  reader.settk(p_rel + next_quote);
      //}
      //else {
      //  //ctx.mem.writebin(err, ctx.strBuff + prev, read - prev);
      //  reader.settk(p_rel + prev);
      //}
      break;
    }
  } while(true);

  return false;
}

// parses an csv file depending using only x64 and optionally bmi1
// rtnOnNL decides whether or not to return on the first encountered newline
template<bool rtnOnNL, tzcntSig tzcnt, andnSig andn, typename F>
Error& readCSV_bmi1(Error& err, F& clb, uint32_t& row, uint32_t& column, char seperator, CSVFileReader& reader) {
  if(!err.peekOk())
    return err;

  if(!reader.isOpen()) {
    WK_RAISE_ERR(err, NotOpen, "CSVReader: no file open to read");
    return err;
  }

  char sep = seperator;
  char esc = '\\';
  char quote = '\"';
  char nl = '\n';

  CSV_Context ctx(row, column);

  uint64_t read = 0;

  while(!reader.eof() && err.peekOk()) {
    for(int i = 0; i < 64; i++) {
      uint64_t rn = 0;
      char c = *reader.pushCache(err, 1, rn);
      read += rn;

      ctx.sep_mask |= (uint64_t)(c == sep) << i;
      ctx.quote_mask |= (uint64_t)(c == quote) << i;
      ctx.esc_mask |= (uint64_t)(c == esc) << i;
      ctx.nl_mask |= (uint64_t)(c == nl) << i;
    }

    if(readCSV_Impl<rtnOnNL, tzcnt, andn, F>(err, clb, ctx, read, reader)) {
      return err;
    }
  }

  //if(ctx.mem.size() > 0) {
  //  CSVReader::Token tk = ctx.mem;
  //  clb(err, row, column, tk);
  //}

  if(reader.hasDangling()) {
    CSVReader::Token tk = reader.getDangling();
    clb(err, row, column, tk);
  }

  return err;
}

// parses an csv file depending using only sse, sse2 and optionally bmi1
// rtnOnNL decides whether or not to return on the first encountered newline
template<bool rtnOnNL, tzcntSig tzcnt, andnSig andn, typename F>
Error& readCSV_SSE2(Error& err, F& clb, uint32_t& row, uint32_t& column, char seperator, CSVFileReader& reader) {
  if(!err.peekOk())
    return err;

  if(!reader.isOpen()) {
    WK_RAISE_ERR(err, NotOpen, "CSVReader: no file open to read");
    return err;
  }

  __m128i sep = _mm_set1_epi8(seperator);
  __m128i esc = _mm_set1_epi8('\\');
  __m128i quote = _mm_set1_epi8('\"');
  __m128i nl = _mm_set1_epi8('\n');

  CSV_Context ctx(row, column);

  uint64_t read = 0;

  bool readAligned = false;
  // As of the creation of this file the FileReader uses an unbuffered strategy
  // to increase performance. A part of this unbuffered implementation is the
  // memory alignment according to the physical sector size of the underlying IO
  // Historically this has been 512 bytes but on more recent hardware it has been
  // 4096 bytes instead. This is however still a lot more than the required 16
  // below and as such it should always be read aligned. The unaligned version has
  // worse performance as it load a singular byta at a time opposed to 16 using memcpy.
  // Funnily enough on my hardware the memcpy from the Reader buffer to strBuff takes
  // more CPU time than the IO operations.
  readAligned = reader.getCacheAlignment() % 16 == 0;
  readAligned &= reader.getCacheSize() % 16 == 0 && reader.getCacheSize() != 0;

  if(readAligned) {
    while(!reader.eof() && err.peekOk()) {
      read = 0;
      for(int i = 0; i < 4; i++) {
        uint64_t pushed = 0;
        __m128i strBuff = _mm_load_si128((const __m128i*)reader.pushCache(err, 16, pushed));
        read += pushed;

        __m128i sepField = _mm_cmpeq_epi8(sep, strBuff);
        __m128i quoteField = _mm_cmpeq_epi8(quote, strBuff);
        __m128i escField = _mm_cmpeq_epi8(esc, strBuff);
        __m128i nlField = _mm_cmpeq_epi8(nl, strBuff);

        ctx.sep_mask |= (uint64_t)((uint32_t)_mm_movemask_epi8(sepField)) << (16 * i);
        ctx.quote_mask |= (uint64_t)((uint32_t)_mm_movemask_epi8(quoteField)) << (16 * i);
        ctx.esc_mask |= (uint64_t)((uint32_t)_mm_movemask_epi8(escField)) << (16 * i);
        ctx.nl_mask |= (uint64_t)((uint32_t)_mm_movemask_epi8(nlField)) << (16 * i);
      }

      if(readCSV_Impl<rtnOnNL, tzcnt, andn, F>(err, clb, ctx, read, reader)) {
        return err;
      }
    }
  }
  else {
    // If it is not aligned then why do I have a dedicated reader just for csv files...
    // *Throws temper tantrum*
    WK_RAISE_ERR(err, Generic, "CSVReader: Memory not aligned.");
  }

  //if(ctx.mem.size() > 0) {
  //  CSVReader::Token tk = ctx.mem;
  //  clb(err, row, column, tk);
  //}

  if(reader.hasDangling()) {
    CSVReader::Token tk = reader.getDangling();
    clb(err, row, column, tk);
  }

  return err;
}

// parses an csv file depending using only avx, avx2 and optionally bmi1
// rtnOnNL decides whether or not to return on the first encountered newline
template<bool rtnOnNL, tzcntSig tzcnt, andnSig andn, typename F>
Error& readCSV_AVX2(Error& err, F& clb, uint32_t& row, uint32_t& column, char seperator, CSVFileReader& reader) {
  if(!err.peekOk())
    return err;

  if(!reader.isOpen()) {
    WK_RAISE_ERR(err, NotOpen, "CSVReader: no file open to read");
    return err;
  }

  __m256i sep = _mm256_set1_epi8(seperator);
  __m256i esc = _mm256_set1_epi8('\\');
  __m256i quote = _mm256_set1_epi8('\"');
  __m256i nl = _mm256_set1_epi8('\n');

  CSV_Context ctx(row, column);

  uint64_t read = 0;

  bool readAligned = false;
  readAligned = reader.getCacheAlignment() % 32 == 0;
  readAligned &= (reader.getCacheSize() % 32 == 0) && reader.getCacheSize() != 0;

  if(readAligned) {
    while(!reader.eof() && err.peekOk()) {
      read = 0;
      for(int i = 0; i < 2; i++) {
        uint64_t pushed = 0;
        __m256i strBuff = _mm256_load_si256((const __m256i*)reader.pushCache(err, 32, pushed));
        read += pushed;

        __m256i sepField = _mm256_cmpeq_epi8(sep, strBuff);
        __m256i quoteField = _mm256_cmpeq_epi8(quote, strBuff);
        __m256i escField = _mm256_cmpeq_epi8(esc, strBuff);
        __m256i nlField = _mm256_cmpeq_epi8(nl, strBuff);

        ctx.sep_mask |= (uint64_t)((uint32_t)_mm256_movemask_epi8(sepField)) << (32 * i);
        ctx.quote_mask |= (uint64_t)((uint32_t)_mm256_movemask_epi8(quoteField)) << (32 * i);
        ctx.esc_mask |= (uint64_t)((uint32_t)_mm256_movemask_epi8(escField)) << (32 * i);
        ctx.nl_mask |= (uint64_t)((uint32_t)_mm256_movemask_epi8(nlField)) << (32 * i);
      }

      if(readCSV_Impl<rtnOnNL, tzcnt, andn, F>(err, clb, ctx, read, reader)) {
        return err;
      }
    }
  }
  else {
    // If it is not aligned then why do I have a dedicated file reader just for parsing csv files...
    // *Throws temper tantrum*
    WK_RAISE_ERR(err, Generic, "CSVReader: Memory not aligned.");
  }

  //if(ctx.mem.size() > 0) {
  //  CSVReader::Token tk = ctx.mem;
  //  clb(err, row, column, tk);
  //}

  if(reader.hasDangling()) {
    CSVReader::Token tk = reader.getDangling();
    clb(err, row, column, tk);
  }

  return err;
}

} // namespace csv
} // namespace detail


// Runtime dispatch, select one of the defined functions (most restrictive first)
// using the specified flags.
// This will at runtime select a function to use (before invoking the main method)
template<typename F>
Error& CSVReader::readHeader(Error& err, F& clb) {
  namespace dcsv = detail::csv;
  dcsv::CSVFileReader cread(reader, req_alignment);
  static RuntimeDispatch<Error& (Error&, F&, uint32_t&, uint32_t&, char, dcsv::CSVFileReader&)> dispatchHeader{
    { dcsv::readCSV_AVX2<true, dcsv::tzcnt_bmi, dcsv::andn_bmi, F>, CPU::ISA::avx2 | CPU::ISA::avx | CPU::ISA::bmi1 },
    { dcsv::readCSV_SSE2<true, dcsv::tzcnt_bmi, dcsv::andn_bmi, F>, CPU::ISA::sse2 | CPU::ISA::sse | CPU::ISA::bmi1 },
    { dcsv::readCSV_bmi1<true, dcsv::tzcnt_bmi, dcsv::andn_bmi, F>, CPU::ISA::bmi1 },
    { dcsv::readCSV_AVX2<true, dcsv::tzcnt_x64, dcsv::andn_x64, F>, CPU::ISA::avx2 | CPU::ISA::avx },
    { dcsv::readCSV_SSE2<true, dcsv::tzcnt_x64, dcsv::andn_x64, F>, CPU::ISA::sse2 | CPU::ISA::sse },
    { dcsv::readCSV_bmi1<true, dcsv::tzcnt_x64, dcsv::andn_x64, F>, 0 }
  };
  return dispatchHeader(err, clb, row, column, seperator, cread);
}

// Runtime dispatch, select one of the defined functions (most restrictive first)
// using the specified flags.
// This will at runtime select a function to use (before invoking the main method)
template<typename F>
Error& CSVReader::read(Error& err, F& clb) {
  namespace dcsv = detail::csv;
  dcsv::CSVFileReader cread(reader, req_alignment);
  static RuntimeDispatch<Error&(Error&, F&, uint32_t&, uint32_t&, char, dcsv::CSVFileReader&)> dispatch{
    { dcsv::readCSV_AVX2<false, dcsv::tzcnt_bmi, dcsv::andn_bmi, F>, CPU::ISA::avx2 | CPU::ISA::avx | CPU::ISA::bmi1 },
    { dcsv::readCSV_SSE2<false, dcsv::tzcnt_bmi, dcsv::andn_bmi, F>, CPU::ISA::sse2 | CPU::ISA::sse | CPU::ISA::bmi1 },
    { dcsv::readCSV_bmi1<false, dcsv::tzcnt_bmi, dcsv::andn_bmi, F>, CPU::ISA::bmi1 },
    { dcsv::readCSV_AVX2<false, dcsv::tzcnt_x64, dcsv::andn_x64, F>, CPU::ISA::avx2 | CPU::ISA::avx },
    { dcsv::readCSV_SSE2<false, dcsv::tzcnt_x64, dcsv::andn_x64, F>, CPU::ISA::sse2 | CPU::ISA::sse },
    { dcsv::readCSV_bmi1<false, dcsv::tzcnt_x64, dcsv::andn_x64, F>, 0 }
  };
  return dispatch(err, clb, row, column, seperator, cread);
}

} // namespace Wikinger
