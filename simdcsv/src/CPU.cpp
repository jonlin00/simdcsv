#include "CPU.h"
#include "Macros.h"
#include "Log/Log.h"

namespace Wikinger {

struct {
  uint64_t ext;
  const char* text;
  const char* explanation;
} extensions[] = {
  { CPU::ISA::os_avx, "OS avx", "Does the os copy the avx registers when context-switching?" },
  { CPU::ISA::os_avx512, "OS avx512", "Does the os copy the avx512 registers when context-switching?" },
  { CPU::ISA::hw_mmx, "mmx", "MultiMedia eXtension" },
  { CPU::ISA::hw_mmxext, "mmxext", "MMX extensions for SSE" },
  { CPU::ISA::hw_abm, "abm", "Advanced Bit Manipulation (lzcnt and popcnt)" },
  { CPU::ISA::hw_lzcnt, "lzcnt", "Count leadint zero bits" },
  { CPU::ISA::hw_popcnt, "popcnt", "Count number of bits set to 1" },
  { CPU::ISA::hw_rdrand, "rdrand", "on-chip randon number generation" },
  { CPU::ISA::hw_rdseed, "rdseed", "on-chip randon seed generation" },
  { CPU::ISA::hw_bmi1, "bmi1", "Bit Manipulation Instruction set 1" },
  { CPU::ISA::hw_bmi2, "bmi2", "Bit Manipulation Instruction set 2" },
  { CPU::ISA::hw_tbm, "tbm", "Trailing bit manipulation" },
  { CPU::ISA::hw_mpx, "mpx", "MPX (Memory Protection Extensions)" },
  { CPU::ISA::hw_adx, "adx", "ADX (Multi-Precision Add-Carry Instruction Extensions)" },
  { CPU::ISA::hw_clmul, "clmul", "Carry-less multiplication" },
  { CPU::ISA::hw_fma3, "fma3", "Fused multiply-add accumulate, 3 operands" },
  { CPU::ISA::hw_fma4, "fma4", "Fused multiply-add accumulate, 4 operands" },
  { CPU::ISA::hw_sse, "sse", "Streaming SIMD Extension (SIMD: Single Instruction Multiple Data)" },
  { CPU::ISA::hw_sse2, "sse2", "Streaming SIMD Extension 2" },
  { CPU::ISA::hw_sse3, "sse3", "Streaming SIMD Extension 3" },
  { CPU::ISA::hw_ssse3, "ssse3", "Supplemental Streaming SIMD Extension 3" },
  { CPU::ISA::hw_sse41, "sse41", "Streaming SIMD Extension 41" },
  { CPU::ISA::hw_sse42, "sse42", "Streaming SIMD Extension 42" },
  { CPU::ISA::hw_sse4a, "sse4a", "Streaming SIMD Extension 4a" },
  { CPU::ISA::hw_aes, "aes", "Advanced Encryption Standard" },
  { CPU::ISA::hw_aesni, "aesni", "Advanced Encryption Standard New Instructions" },
  { CPU::ISA::hw_vaes, "vaes", "Vector AES Instructions" },
  { CPU::ISA::hw_sha, "sha", "SHA Extension" },
  { CPU::ISA::hw_xop, "xop", "eXtended Operations" },
  { CPU::ISA::hw_avx, "avx", "Advanced Vector Extension" },
  { CPU::ISA::hw_avx2, "avx2", "Advanced Vector Extension 2" },
  { CPU::ISA::hw_avx512_f, "avx512_f", "AVX512 Foundation" },
  { CPU::ISA::hw_avx512_pf, "avx512_pf", "AVX512 Prefetch" },
  { CPU::ISA::hw_avx512_er, "avx512_er", "AVX512 Exponential and Reciprocal" },
  { CPU::ISA::hw_avx512_cd, "avx512_cd", "AVX512 Conflict Detections" },
  { CPU::ISA::hw_avx512_vl, "avx512_vl", "AVX512 Vector Length Extensions" },
  { CPU::ISA::hw_avx512_bw, "avx512_bw", "AVX512 Byte and Word Extensions" },
  { CPU::ISA::hw_avx512_dq, "avx512_dq", "AVX512 Doubleword and Quadword Instructions" },
  { CPU::ISA::hw_avx512_ifma, "avx512_ifma", "AVX512 Integer Fused Multiply-Add Instructions" },
  { CPU::ISA::hw_avx512_vbmi, "avx512_vbmi", "AVX512 Vector Bit Manipulation Instructions" },
  { CPU::ISA::hw_avx512_vbmi2, "avx512_vbmi2", "AVX512 Vector Bit Manipulation Instructions 2" },
  { CPU::ISA::hw_avx512_bitalg, "avx512_bitalg", "AVX512 Bit algorithm Instructions" },
  { CPU::ISA::hw_avx512_vnni, "avx512_vnni", "AVX512 Vector Neural Network Instructions" },
  { CPU::ISA::hw_avx512_4vnniw, "avx512_4vnniw", "AVX512 4-register Vector Neural Network Instructions" },
  { CPU::ISA::hw_avx512_4fmaps, "avx512_4fmaps", "AVX512 4-register Fused multiply-add accumulate" },
  { CPU::ISA::hw_avx512_vp2intersect, "avx512_vp2intersect", "AVX512 Compute intersection between pairs of mask registers" },
  { CPU::ISA::hw_avx512_vpopcntdq, "avx512_vpopcntdq", "AVX512 Vector Population Count Doubleword and Quadword" }
};

#if WK_PLATFORM_WINDOWS
#include <intrin.h>

void cpuid(int32_t out[4], int32_t x) {
  __cpuidex(out, x, 0);
}

__int64 xgetbv(unsigned int x) {
  return _xgetbv(x);
}
#elif WK_PLATFORM_LINUX
#include <cpuid.h>
void cpuid(int32_t out[4], int32_t x) {
  __cpuid_count(x, 0, out[0], out[1], out[2], out[3]);
}

uint64_t xgetbv(unsigned int x) {
  uint32_t eax, edx;
  __asm__ __volatile__("xgetbx" : "=a"(eax), "=d"(edx) : "c"(x));
  return ((uint64_t)edx << 32) | eax;
}
#endif

bool detectAVX() {
  bool avxSupported = false;
  int cpuInfo[4];
  cpuid(cpuInfo, 1);

  bool osUsesXSAVE_XRSTORE = WK_BIT_GET(cpuInfo[2], WK_BIT(27)) != 0;
  bool cpuAVXSupport = WK_BIT_GET(cpuInfo[2], WK_BIT(28)) != 0;

  if(osUsesXSAVE_XRSTORE && cpuAVXSupport) {
    uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
    avxSupported = (xcrFeatureMask & 0x6) == 0x6;
  }
  return avxSupported;
}

bool detectAVX512() {
  if(!detectAVX()) return false;

  uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
  return (xcrFeatureMask & 0xe6) == 0xe6;
}

void CPU::detectISA() {
  ext |= detectAVX() ?    CPU::ISA::os_avx : 0;
  ext |= detectAVX512() ? CPU::ISA::os_avx512 : 0;

  int32_t info[4];
  cpuid(info, 0x00000000);
  nBIds = info[0];
  cpuid(info, 0x80000000);
  nEIds = info[0];

  if(nBIds >= 0x01) {
    cpuid(info, 0x00000001);

    int32_t efamid = (info[0] >> 19) & 0x7F;
    int32_t emodid = (info[0] >> 15) & 0x0F;
    int32_t famid  = (info[0] >>  7) & 0x0F;
    int32_t mod    = (info[0] >>  3) & 0x0F;
    stepping = info[0] & 0x0F;
    model = (famid == 6 || famid == 15) ? (emodid >> 4) + mod : mod;
    family = famid == 15 ? famid + efamid : famid;

    ext |= WK_BIT_GET(info[2], WK_BIT(0)) != 0 ?  CPU::ISA::hw_sse3 : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(1)) != 0 ? CPU::ISA::hw_clmul : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(9)) != 0 ?  CPU::ISA::hw_ssse3 : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(12)) != 0 ? CPU::ISA::hw_fma3 : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(19)) != 0 ? CPU::ISA::hw_sse41 : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(20)) != 0 ? CPU::ISA::hw_sse42 : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(23)) != 0 ? CPU::ISA::hw_popcnt : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(25)) != 0 ? CPU::ISA::hw_aes : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(28)) != 0 ? CPU::ISA::hw_avx : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(30)) != 0 ? CPU::ISA::hw_rdrand : 0;

    ext |= WK_BIT_GET(info[3], WK_BIT(23)) != 0 ? CPU::ISA::hw_mmx : 0;
    ext |= WK_BIT_GET(info[3], WK_BIT(25)) != 0 ? CPU::ISA::hw_sse : 0;
    ext |= WK_BIT_GET(info[3], WK_BIT(26)) != 0 ? CPU::ISA::hw_sse2 : 0;
  }
  if(nBIds >= 0x00000007) {
    cpuid(info, 0x00000007);

    ext |= WK_BIT_GET(info[1], WK_BIT(3)) != 0 ?  CPU::ISA::hw_bmi1 : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(5)) != 0 ?  CPU::ISA::hw_avx2 : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(8)) != 0 ?  CPU::ISA::hw_bmi2 : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(14)) != 0 ? CPU::ISA::hw_mpx : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(16)) != 0 ? CPU::ISA::hw_avx512_f : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(17)) != 0 ? CPU::ISA::hw_avx512_dq : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(18)) != 0 ? CPU::ISA::hw_rdseed : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(19)) != 0 ? CPU::ISA::hw_adx : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(21)) != 0 ? CPU::ISA::hw_avx512_ifma : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(26)) != 0 ? CPU::ISA::hw_avx512_pf : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(27)) != 0 ? CPU::ISA::hw_avx512_er : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(28)) != 0 ? CPU::ISA::hw_avx512_cd : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(29)) != 0 ? CPU::ISA::hw_sha : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(30)) != 0 ? CPU::ISA::hw_avx512_bw : 0;
    ext |= WK_BIT_GET(info[1], WK_BIT(31)) != 0 ? CPU::ISA::hw_avx512_vl : 0;

    ext |= WK_BIT_GET(info[2], WK_BIT(1)) != 0 ?  CPU::ISA::hw_avx512_vbmi : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(6)) != 0 ?  CPU::ISA::hw_avx512_vbmi2 : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(9)) != 0 ?  CPU::ISA::hw_vaes : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(11)) != 0 ? CPU::ISA::hw_avx512_vnni : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(12)) != 0 ? CPU::ISA::hw_avx512_bitalg : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(14)) != 0 ? CPU::ISA::hw_avx512_vpopcntdq : 0;

    ext |= WK_BIT_GET(info[3], WK_BIT(2)) != 0 ? CPU::ISA::hw_avx512_4vnniw : 0;
    ext |= WK_BIT_GET(info[3], WK_BIT(3)) != 0 ? CPU::ISA::hw_avx512_4fmaps : 0;
    ext |= WK_BIT_GET(info[3], WK_BIT(8)) != 0 ? CPU::ISA::hw_avx512_vp2intersect : 0;
  }
  if(nEIds >= 0x80000001) {
    cpuid(info, 0x80000001);

    ext |= WK_BIT_GET(info[2], WK_BIT(5)) != 0 ?  CPU::ISA::hw_abm | CPU::ISA::hw_lzcnt | CPU::ISA::hw_popcnt: 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(6)) != 0 ?  CPU::ISA::hw_sse4a : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(11)) != 0 ? CPU::ISA::hw_xop : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(16)) != 0 ? CPU::ISA::hw_fma4 : 0;
    ext |= WK_BIT_GET(info[2], WK_BIT(21)) != 0 ? CPU::ISA::hw_tbm : 0;
    ext |= WK_BIT_GET(info[3], WK_BIT(22)) != 0 ? CPU::ISA::hw_mmxext : 0;
    ext |= WK_BIT_GET(info[3], WK_BIT(23)) != 0 ? CPU::ISA::hw_mmx : 0;
  }
}

bool CPU::HasFrequency() const {
  return nBIds >= 0x16;
}

uint32_t CPU::MaxFrequency() const {
  int32_t info[4];
  if(HasFrequency()) {
    cpuid(info, 0x16);
    return info[1] & 0x7fff;
  }
  return 0;
}

uint32_t CPU::BaseFrequency() const {
  int32_t info[4];
  if(HasFrequency()) {
    cpuid(info, 0x16);
    return info[0] & 0x7fff;
  }
  return 0;
}

#if WK_PLATFORM_WINDOWS
#include <Windows.h>

__int64 FileTimeToInt64(FILETIME& ft) {
  ULARGE_INTEGER foo;

  foo.LowPart = ft.dwLowDateTime;
  foo.HighPart = ft.dwHighDateTime;

  return (foo.QuadPart);
}

double CPU::Load() {
  FILETIME IdleTime, KernelTime, UserTime;
  static unsigned long long PrevTotal = 0;
  static unsigned long long PrevIdle = 0;
  static unsigned long long PrevUser = 0;
  unsigned long long ThisTotal;
  unsigned long long ThisIdle, ThisKernel, ThisUser;
  unsigned long long TotalSinceLast, IdleSinceLast, UserSinceLast;


  // GET THE KERNEL / USER / IDLE times.  
  // And oh, BTW, kernel time includes idle time
  GetSystemTimes(&IdleTime, &KernelTime, &UserTime);

  ThisIdle = FileTimeToInt64(IdleTime);
  ThisKernel = FileTimeToInt64(KernelTime);
  ThisUser = FileTimeToInt64(UserTime);

  ThisTotal = ThisKernel + ThisUser;
  TotalSinceLast = ThisTotal - PrevTotal;
  IdleSinceLast = ThisIdle - PrevIdle;
  UserSinceLast = ThisUser - PrevUser;
  double Headroom;
  Headroom = (double)IdleSinceLast / (double)TotalSinceLast;
  double Load;
  Load = 1.0 - Headroom;
  Load *= 100.0;  // percent

  PrevTotal = ThisTotal;
  PrevIdle = ThisIdle;
  PrevUser = ThisUser;

  return Load;
}
#else
#pragma message("Currently only windows supports retrieval of CPUimpl load percentages")
double CPUimpl::Load() const {
  return 0.0;
}
#endif

std::string CPU::Vendor() const {
  std::string res;
  res.resize(13);
  Vendor(res.data());
  return res;
}

size_t CPU::Vendor(char* str) const {
  int32_t CPUInfo[4];
  cpuid(CPUInfo, 0);
  memcpy(str + 0, &CPUInfo[1], 4);
  memcpy(str + 4, &CPUInfo[3], 4);
  memcpy(str + 8, &CPUInfo[2], 4);
  str[12] = 0;
  return 13;
}

std::string CPU::Brand() const {
  std::string res;
  res.resize(48);
  size_t actual = Brand(res.data()) - 1;
  res.erase(actual, 48 - actual);
  return res;
}

size_t CPU::Brand(char* str) const {
  int32_t info[4];

  if(nEIds < 0x80000004) {
    memcpy(str, "UNKNOWN", 8);
    return 8;
  }

  for(int i = 0; i < 3; i++) {
    cpuid(info, 0x80000002 + i);
    memcpy(str + sizeof(info) * i, info, sizeof(info));
  }
  return strlen(str) + 1;
}

void CPU::print() const {
  char buff[48];
  Vendor(buff);
  WK_INFO("CPU vendor: {}", buff);
  Brand(buff);
  WK_INFO("CPU brand: {}", buff);
  if(HasFrequency()) {
    WK_INFO("CPU base frequency: {}MHz", BaseFrequency());
    WK_INFO("CPU Max frequency: {}MHz", MaxFrequency());
  }

  WK_INFO("instruction set name:CPU/OS support Explanation");
  for(int i = 0; i < WK_COUNTOF(extensions); i++) {
    WK_INFO("{:>20}:{:<14} {}",
            extensions[i].text,
            (ext & extensions[i].ext) == extensions[i].ext ? "true" : "false",
            extensions[i].explanation);
  }
}

CPU::CPU() :
  ext(0), nBIds(0), nEIds(0),
  family(0), model(0), stepping(0) {
  detectISA();
}

}
