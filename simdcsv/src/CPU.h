#ifndef WK_CPU_H
#define WK_CPU_H

#include <stdint.h>
#include "Macros.h"
#include <string>

namespace Wikinger {

class CPU {
public:
  struct ISA {
    enum Enum : uint64_t {
      os_avx = WK_BIT(1),  // Does the os copy the avx registers when context-switching?
      os_avx512 = WK_BIT(2),  // Does the os copy the avx512 registers when context-switching?
      hw_mmx = WK_BIT(3),  // Officially the abbreviation is whithout meaning but inofficially its (among others) MultiMedia eXtension
      hw_mmxext = WK_BIT(4),  // MMX extensions for SSE
      hw_abm = WK_BIT(5),  // Advanced Bit Manipulation (lzcnt and popcnt)
      hw_lzcnt = WK_BIT(6),  // Count leadint zero bits
      hw_popcnt = WK_BIT(7),  // Count number of bits set to 1
      hw_rdrand = WK_BIT(8),  // on-chip randon number generation
      hw_rdseed = WK_BIT(9),  // on-chip randon seed generation
      hw_bmi1 = WK_BIT(10), // Bit Manipulation Instruction set 1
      hw_bmi2 = WK_BIT(11), // Bit Manipulation Instruction set 2
      hw_tbm = WK_BIT(12), // Trailing bit manipulation
      hw_mpx = WK_BIT(13), // MPX (Memory Protection Extensions)
      hw_adx = WK_BIT(14), // ADX (Multi-Precision Add-Carry Instruction Extensions)
      hw_clmul = WK_BIT(15), // Carry-less multiplication
      hw_fma3 = WK_BIT(16), // Fused multiply-add accumulate, 3 operands
      hw_fma4 = WK_BIT(17), // Fused multiply-add accumulate, 4 operands
      hw_sse = WK_BIT(18), // Streaming SIMD Extension (SIMD: Single Instruction Multiple Data)
      hw_sse2 = WK_BIT(19), // Streaming SIMD Extension 2
      hw_sse3 = WK_BIT(20), // Streaming SIMD Extension 3
      hw_ssse3 = WK_BIT(21), // Supplemental Streaming SIMD Extension 3
      hw_sse41 = WK_BIT(22), // Streaming SIMD Extension 41
      hw_sse42 = WK_BIT(23), // Streaming SIMD Extension 42
      hw_sse4a = WK_BIT(24), // Streaming SIMD Extension 4a
      hw_aes = WK_BIT(25), // Advanced Encryption Standard
      hw_aesni = WK_BIT(26), // Advanced Encryption Standard New Instructions
      hw_vaes = WK_BIT(27), // Vector AES Instructions
      hw_sha = WK_BIT(28), // SHA Extension
      hw_xop = WK_BIT(29), // eXtended Operations
      hw_avx = WK_BIT(30), // Advanced Vector Extension
      hw_avx2 = WK_BIT(31), // Advanced Vector Extension 2
      hw_avx512_f = WK_BIT(32), // AVX512 Foundation
      hw_avx512_pf = WK_BIT(33), // AVX512 Prefetch
      hw_avx512_er = WK_BIT(34), // AVX512 Exponential and Reciprocal
      hw_avx512_cd = WK_BIT(35), // AVX512 Conflict Detections
      hw_avx512_vl = WK_BIT(36), // AVX512 Vector Length Extensions
      hw_avx512_bw = WK_BIT(37), // AVX512 Byte and Word Extensions
      hw_avx512_dq = WK_BIT(38), // AVX512 Doubleword and Quadword Instructions
      hw_avx512_ifma = WK_BIT(39), // AVX512 Integer Fused Multiply-Add Instructions
      hw_avx512_vbmi = WK_BIT(40), // AVX512 Vector Bit Manipulation Instructions
      hw_avx512_vbmi2 = WK_BIT(41), // AVX512 Vector Bit Manipulation Instructions 2
      hw_avx512_bitalg = WK_BIT(42), // AVX512 Bit algorithm Instructions
      hw_avx512_vnni = WK_BIT(43), // AVX512 Vector Neural Network Instructions
      hw_avx512_4vnniw = WK_BIT(44), // AVX512 4-register Vector Neural Network Instructions
      hw_avx512_4fmaps = WK_BIT(45), // AVX512 4-register Fused multiply-add accumulate
      hw_avx512_vp2intersect = WK_BIT(46), // AVX512 Compute intersection between pairs of mask registers
      hw_avx512_vpopcntdq = WK_BIT(47),  // AVX512 Vector Population Count Doubleword and Quadword

      sse4 = hw_sse41 | hw_sse42 | os_avx,
      mmx = hw_mmx | os_avx,
      mmxext = hw_mmxext | os_avx,
      abm = hw_abm | os_avx,
      lzcnt = hw_lzcnt | os_avx,
      popcnt = hw_popcnt | os_avx,
      rdrand = hw_rdrand,
      rdseed = hw_rdseed,
      bmi1 = hw_bmi1,
      bmi2 = hw_bmi2,
      tbm = hw_tbm,
      mpx = hw_mpx,
      adx = hw_adx,
      clmul = hw_clmul,
      fma3 = hw_fma3 | os_avx,
      fma4 = hw_fma4 | os_avx,
      sse = hw_sse | os_avx,
      sse2 = hw_sse2 | os_avx,
      sse3 = hw_sse3 | os_avx,
      ssse3 = hw_ssse3 | os_avx,
      sse41 = hw_sse41 | os_avx,
      sse42 = hw_sse42 | os_avx,
      sse4a = hw_sse4a | os_avx,
      aes = hw_aes | os_avx,
      aesni = hw_aesni | os_avx,
      vaes = hw_vaes | os_avx,
      sha = hw_sha | os_avx,
      xop = hw_xop | os_avx,
      avx = hw_avx | os_avx,
      avx2 = hw_avx2 | os_avx,
      avx512_f = hw_avx512_f | os_avx512,
      avx512_pf = hw_avx512_pf | os_avx512,
      avx512_er = hw_avx512_er | os_avx512,
      avx512_cd = hw_avx512_cd | os_avx512,
      avx512_vl = hw_avx512_vl | os_avx512,
      avx512_bw = hw_avx512_bw | os_avx512,
      avx512_dq = hw_avx512_dq | os_avx512,
      avx512_ifma = hw_avx512_ifma | os_avx512,
      avx512_vbmi = hw_avx512_vbmi | os_avx512,
      avx512_vbmi2 = hw_avx512_vbmi2 | os_avx512,
      avx512_bitalg = hw_avx512_bitalg | os_avx512,
      avx512_vnni = hw_avx512_vnni | os_avx512,
      avx512_4vnniw = hw_avx512_4vnniw | os_avx512,
      avx512_4fmaps = hw_avx512_4fmaps | os_avx512,
      avx512_vp2intersect = hw_avx512_vp2intersect | os_avx512,
      avx512_vpopcntdq = hw_avx512_vpopcntdq | os_avx512,
    };
  };

  inline bool MMX() const { return (GetISA() & ISA::mmx) == ISA::mmx; }
  inline bool MMXEXT() const { return (GetISA() & ISA::mmxext) == ISA::mmxext; }
  inline bool ABM() const { return (GetISA() & ISA::abm) == ISA::abm; }
  inline bool LZCNT() const { return (GetISA() & ISA::lzcnt) == ISA::lzcnt; }
  inline bool POPCNT() const { return (GetISA() & ISA::popcnt) == ISA::popcnt; }
  inline bool RDRAND() const { return (GetISA() & ISA::rdrand) == ISA::rdrand; }
  inline bool RDSEED() const { return (GetISA() & ISA::rdseed) == ISA::rdseed; }
  inline bool BMI1() const { return (GetISA() & ISA::bmi1) == ISA::bmi1; }
  inline bool BMI2() const { return (GetISA() & ISA::bmi2) == ISA::bmi2; }
  inline bool TBM() const { return (GetISA() & ISA::tbm) == ISA::tbm; }
  inline bool MPX() const { return (GetISA() & ISA::mpx) == ISA::mpx; }
  inline bool ADX() const { return (GetISA() & ISA::adx) == ISA::adx; }
  inline bool CLMUL() const { return (GetISA() & ISA::clmul) == ISA::clmul; }
  inline bool FMA3() const { return (GetISA() & ISA::fma3) == ISA::fma3; }
  inline bool FMA4() const { return (GetISA() & ISA::fma4) == ISA::fma4; }
  inline bool SSE() const { return (GetISA() & ISA::sse) == ISA::sse; }
  inline bool SSE2() const { return (GetISA() & ISA::sse2) == ISA::sse2; }
  inline bool SSE3() const { return (GetISA() & ISA::sse3) == ISA::sse3; }
  inline bool SSSE3() const { return (GetISA() & ISA::ssse3) == ISA::ssse3; }
  inline bool SSE41() const { return (GetISA() & ISA::sse41) == ISA::sse41; }
  inline bool SSE42() const { return (GetISA() & ISA::sse42) == ISA::sse42; }
  inline bool SSE4() const { return (GetISA() & ISA::sse4) == ISA::sse4; }
  inline bool SSE4A() const { return (GetISA() & ISA::sse4a) == ISA::sse4a; }
  inline bool AES() const { return (GetISA() & ISA::aes) == ISA::aes; }
  inline bool AESNI() const { return (GetISA() & ISA::aesni) == ISA::aesni; }
  inline bool VAES() const { return (GetISA() & ISA::vaes) == ISA::vaes; }
  inline bool SHA() const { return (GetISA() & ISA::sha) == ISA::sha; }
  inline bool XOP() const { return (GetISA() & ISA::xop) == ISA::xop; }
  inline bool AVX() const { return (GetISA() & ISA::avx) == ISA::avx; }
  inline bool AVX2() const { return (GetISA() & ISA::avx2) == ISA::avx2; }
  inline bool AVX512_F() const { return (GetISA() & ISA::avx512_f) == ISA::avx512_f; }
  inline bool AVX512_PF() const { return (GetISA() & ISA::avx512_pf) == ISA::avx512_pf; }
  inline bool AVX512_ER() const { return (GetISA() & ISA::avx512_er) == ISA::avx512_er; }
  inline bool AVX512_CD() const { return (GetISA() & ISA::avx512_cd) == ISA::avx512_cd; }
  inline bool AVX512_VL() const { return (GetISA() & ISA::avx512_vl) == ISA::avx512_vl; }
  inline bool AVX512_BW() const { return (GetISA() & ISA::avx512_bw) == ISA::avx512_bw; }
  inline bool AVX512_DQ() const { return (GetISA() & ISA::avx512_dq) == ISA::avx512_dq; }
  inline bool AVX512_IFMA() const { return (GetISA() & ISA::avx512_ifma) == ISA::avx512_ifma; }
  inline bool AVX512_VBMI() const { return (GetISA() & ISA::avx512_vbmi) == ISA::avx512_vbmi; }
  inline bool AVX512_VBMI2() const { return (GetISA() & ISA::avx512_vbmi2) == ISA::avx512_vbmi2; }
  inline bool AVX512_BITALG() const { return (GetISA() & ISA::avx512_bitalg) == ISA::avx512_bitalg; }
  inline bool AVX512_VNNI() const { return (GetISA() & ISA::avx512_vnni) == ISA::avx512_vnni; }
  inline bool AVX512_4VNNIW() const { return (GetISA() & ISA::avx512_4vnniw) == ISA::avx512_4vnniw; }
  inline bool AVX512_4FMAPS() const { return (GetISA() & ISA::avx512_4fmaps) == ISA::avx512_4fmaps; }
  inline bool AVX512_VP2INTERSECT() const { return (GetISA() & ISA::avx512_vp2intersect) == ISA::avx512_vp2intersect; }
  inline bool AVX512_VPOPCNTDQ() const { return (GetISA() & ISA::avx512_vpopcntdq) == ISA::avx512_vpopcntdq; }

  inline bool OS_AVX() const { return (GetISA() & ISA::os_avx) == ISA::os_avx; }          // Does the os copy the avx registers when context-switching?
  inline bool OS_AVX512() const { return (GetISA() & ISA::os_avx512) == ISA::os_avx512; } // Does the os copy the avx512 registers when context-switching?
  inline bool HW_MMX() const { return (GetISA() & ISA::hw_mmx) == ISA::hw_mmx; }          // Officially the abbreviation is whithout meaning but inofficially its (among others) MultiMedia eXtension
  inline bool HW_MMXEXT() const { return (GetISA() & ISA::hw_mmxext) == ISA::hw_mmxext; } // MMX extensions for SSE
  inline bool HW_ABM() const { return (GetISA() & ISA::hw_abm) == ISA::hw_abm; }          // Advanced Bit Manipulation (lzcnt and popcnt)
  inline bool HW_LZCNT() const { return (GetISA() & ISA::hw_lzcnt) == ISA::hw_lzcnt; }    // Count leadint zero bits
  inline bool HW_POPCNT() const { return (GetISA() & ISA::hw_popcnt) == ISA::hw_popcnt; } // Count number of bits set to 1
  inline bool HW_RDRAND() const { return (GetISA() & ISA::hw_rdrand) == ISA::hw_rdrand; } // on-chip randon number generation
  inline bool HW_RDSEED() const { return (GetISA() & ISA::hw_rdseed) == ISA::hw_rdseed; } // on-chip randon seed generation
  inline bool HW_BMI1() const { return (GetISA() & ISA::hw_bmi1) == ISA::hw_bmi1; }       // Bit Manipulation Instruction set 1
  inline bool HW_BMI2() const { return (GetISA() & ISA::hw_bmi2) == ISA::hw_bmi2; }       // Bit Manipulation Instruction set 2
  inline bool HW_TBM() const { return (GetISA() & ISA::hw_tbm) == ISA::hw_tbm; }          // AMD Trailing bit manipulation
  inline bool HW_MPX() const { return (GetISA() & ISA::hw_mpx) == ISA::hw_mpx; }          // Intel MPX (Memory Protection Extensions)
  inline bool HW_ADX() const { return (GetISA() & ISA::hw_adx) == ISA::hw_adx; }          // Intel ADX (Multi-Precision Add-Carry Instruction Extensions)
  inline bool HW_CLMUL() const { return (GetISA() & ISA::hw_clmul) == ISA::hw_clmul; }    // Carry-less multiplication
  inline bool HW_FMA3() const { return (GetISA() & ISA::hw_fma3) == ISA::hw_fma3; }       // Fused multiply-add accumulate, 3 operands
  inline bool HW_FMA4() const { return (GetISA() & ISA::hw_fma4) == ISA::hw_fma4; }       // Fused multiply-add accumulate, 4 operands
  inline bool HW_SSE() const { return (GetISA() & ISA::hw_sse) == ISA::hw_sse; }          // Streaming SIMD Extension (SIMD: Single Instruction Multiple Data)
  inline bool HW_SSE2() const { return (GetISA() & ISA::hw_sse2) == ISA::hw_sse2; }       // Streaming SIMD Extension 2
  inline bool HW_SSE3() const { return (GetISA() & ISA::hw_sse3) == ISA::hw_sse3; }       // Streaming SIMD Extension 3
  inline bool HW_SSSE3() const { return (GetISA() & ISA::hw_ssse3) == ISA::hw_ssse3; }    // Supplemental Streaming SIMD Extension 3
  inline bool HW_SSE41() const { return (GetISA() & ISA::hw_sse41) == ISA::hw_sse41; }    // Streaming SIMD Extension 41
  inline bool HW_SSE42() const { return (GetISA() & ISA::hw_sse42) == ISA::hw_sse42; }    // Streaming SIMD Extension 42
  inline bool HW_SSE4A() const { return (GetISA() & ISA::hw_sse4a) == ISA::hw_sse4a; }    // Streaming SIMD Extension 4a
  inline bool HW_AES() const { return (GetISA() & ISA::hw_aes) == ISA::hw_aes; }          // Advanced Encryption Standard
  inline bool HW_AESNI() const { return (GetISA() & ISA::hw_aesni) == ISA::hw_aesni; }    // Advanced Encryption Standard New Instructions
  inline bool HW_VAES() const { return (GetISA() & ISA::hw_vaes) == ISA::hw_vaes; }       // Vector AES Instructions
  inline bool HW_SHA() const { return (GetISA() & ISA::hw_sha) == ISA::hw_sha; }          // Intel SHA Extension
  inline bool HW_XOP() const { return (GetISA() & ISA::hw_xop) == ISA::hw_xop; }          // AMD eXtended Operations
  inline bool HW_AVX() const { return (GetISA() & ISA::hw_avx) == ISA::hw_avx; }          // Advanced Vector Extension
  inline bool HW_AVX2() const { return (GetISA() & ISA::hw_avx2) == ISA::hw_avx2; }       // Advanced Vector Extension 2
  inline bool HW_AVX512_F() const { return (GetISA() & ISA::hw_avx512_f) == ISA::hw_avx512_f; }     // AVX512 Foundation
  inline bool HW_AVX512_PF() const { return (GetISA() & ISA::hw_avx512_pf) == ISA::hw_avx512_pf; }  // AVX512 Prefetch
  inline bool HW_AVX512_ER() const { return (GetISA() & ISA::hw_avx512_er) == ISA::hw_avx512_er; }  // AVX512 Exponential and Reciprocal
  inline bool HW_AVX512_CD() const { return (GetISA() & ISA::hw_avx512_cd) == ISA::hw_avx512_cd; }  // AVX512 Conflict Detections
  inline bool HW_AVX512_VL() const { return (GetISA() & ISA::hw_avx512_vl) == ISA::hw_avx512_vl; }  // AVX512 Vector Length Extensions
  inline bool HW_AVX512_BW() const { return (GetISA() & ISA::hw_avx512_bw) == ISA::hw_avx512_bw; }  // AVX512 Byte and Word Extensions
  inline bool HW_AVX512_DQ() const { return (GetISA() & ISA::hw_avx512_dq) == ISA::hw_avx512_dq; }  // AVX512 Doubleword and Quadword Instructions
  inline bool HW_AVX512_IFMA() const { return (GetISA() & ISA::hw_avx512_ifma) == ISA::hw_avx512_ifma; }      // AVX512 Integer Fused Multiply-Add Instructions
  inline bool HW_AVX512_VBMI() const { return (GetISA() & ISA::hw_avx512_vbmi) == ISA::hw_avx512_vbmi; }      // AVX512 Vector Bit Manipulation Instructions
  inline bool HW_AVX512_VBMI2() const { return (GetISA() & ISA::hw_avx512_vbmi2) == ISA::hw_avx512_vbmi2; }   // AVX512 Vector Bit Manipulation Instructions 2
  inline bool HW_AVX512_BITALG() const { return (GetISA() & ISA::hw_avx512_bitalg) == ISA::hw_avx512_bitalg; }// AVX512 Bit algorithm Instructions
  inline bool HW_AVX512_VNNI() const { return (GetISA() & ISA::hw_avx512_vnni) == ISA::hw_avx512_vnni; }      // AVX512 Vector Neural Network Instructions
  inline bool HW_AVX512_4VNNIW() const { return (GetISA() & ISA::hw_avx512_4vnniw) == ISA::hw_avx512_4vnniw; }// AVX512 4-register Vector Neural Network Instructions
  inline bool HW_AVX512_4FMAPS() const { return (GetISA() & ISA::hw_avx512_4fmaps) == ISA::hw_avx512_4fmaps; }// AVX512 4-register Fused multiply-add accumulate
  inline bool HW_AVX512_VP2INTERSECT() const { return (GetISA() & ISA::hw_avx512_vp2intersect) == ISA::hw_avx512_vp2intersect; }  // AVX512 Compute intersection between pairs of mask registers
  inline bool HW_AVX512_VPOPCNTDQ() const { return (GetISA() & ISA::hw_avx512_vpopcntdq) == ISA::hw_avx512_vpopcntdq; } // AVX512 Vector Population Count Doubleword and Quadword

  uint64_t GetISA() const { return ext; }

  // Gets the vendor string, i.e. an string representing the manufacturer of the CPU
  // sample result is 'AuthenticAMD' or 'GenuineIntel'
  std::string Vendor() const;
  // str must have allocated at least 12 bytes
  size_t Vendor(char* str) const;

  // Gets the cpu brand string, i.e. an string representing the CPU
  // below are a couple of sample strings returned by this function
  // Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz
  // Intel(R) Core(TM) i9-9900K CPU @ 3.60GHz
  // AMD Ryzen 5 3600 6-Core Processor
  std::string Brand() const;
  // str must have allocated at least 48 bytes
  size_t Brand(char* str) const;

  // Does the cpu report max/base frequency
  bool HasFrequency() const;
  // Returns the maximum frequency of the cpu in MHz or 0
  // The value is retrieved using CPUID which means that
  // the resulting value is what was 'written on the box'
  // and does not necessarily represent the real value.
  uint32_t MaxFrequency() const;
  // Returns the base frequency of the cpu in MHz or 0
  // The value is retrieved using CPUID which means that
  // the resulting value is what was 'written on the box'
  // and does not necessarily represent the real value.
  uint32_t BaseFrequency() const;
  // Returns the cpu load across all cores. This function has persistant data
  // and should therefore only be called periodically and it is not thread safe.
  static double Load();

  // prints all supported features along with vendor, brand and frequencies.
  void print() const;

  // Construct a CPU object. This will run the detection for every object created
  // However it should be noted that this will still result in less cpu instructions
  // than I have fingers so there's that. It's very cheap.
  CPU();

private:
  void detectISA();

  uint64_t ext;
  int32_t nBIds;
  int32_t nEIds;

  int32_t family;
  int32_t model;
  int32_t stepping;
};

}

#endif// WK_CPU_H
