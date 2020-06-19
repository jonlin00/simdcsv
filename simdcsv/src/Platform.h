#ifndef WK_PLATFORM_H_HEADER_GUARD
#define WK_PLATFORM_H_HEADER_GUARD

 // Architecture
#define WK_ARCH_32BIT 0
#define WK_ARCH_64BIT 0

// Compiler
#define WK_COMPILER_CLANG          0
#define WK_COMPILER_CLANG_ANALYZER 0
#define WK_COMPILER_GCC            0
#define WK_COMPILER_MSVC           0

// Endianess
#define WK_CPU_ENDIAN_BIG    0
#define WK_CPU_ENDIAN_LITTLE 0

// CPU
#define WK_CPU_ARM   0
#define WK_CPU_JIT   0
#define WK_CPU_MIPS  0
#define WK_CPU_PPC   0
#define WK_CPU_RISCV 0
#define WK_CPU_X86   0

// C Runtime
#define WK_CRT_BSD    0
#define WK_CRT_GLIBC  0
#define WK_CRT_LIBCXX 0
#define WK_CRT_MINGW  0
#define WK_CRT_MSVC   0
#define WK_CRT_NEWLIB 0

#ifndef WK_CRT_NONE
#	define WK_CRT_NONE 0
#endif // WK_CRT_NONE

// Platform
#define WK_PLATFORM_BSD        0
#define WK_PLATFORM_EMSCRIPTEN 0
#define WK_PLATFORM_LINUX      0
#define WK_PLATFORM_NX         0
#define WK_PLATFORM_OSX        0
#define WK_PLATFORM_PS4        0
#define WK_PLATFORM_RPI        0
#define WK_PLATFORM_STEAMLINK  0
#define WK_PLATFORM_WINDOWS    0
#define WK_PLATFORM_WINRT      0
#define WK_PLATFORM_XBOXONE    0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  WK_COMPILER_CLANG
#	define WK_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  WK_COMPILER_CLANG_ANALYZER
#		define WK_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  WK_COMPILER_MSVC
#	define WK_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  WK_COMPILER_GCC
#	define WK_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "WK_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__)     \
 || defined(__aarch64__) \
 || defined(_M_ARM)
#	undef  WK_CPU_ARM
#	define WK_CPU_ARM 1
#	define WK_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__)     \
 ||   defined(__mips_isa_rev) \
 ||   defined(__mips64)
#	undef  WK_CPU_MIPS
#	define WK_CPU_MIPS 1
#	define WK_CACHE_LINE_SIZE 64
#elif defined(_M_PPC)        \
 ||   defined(__powerpc__)   \
 ||   defined(__powerpc64__)
#	undef  WK_CPU_PPC
#	define WK_CPU_PPC 1
#	define WK_CACHE_LINE_SIZE 128
#elif defined(__riscv)   \
 ||   defined(__riscv__) \
 ||   defined(RISCVEL)
#	undef  WK_CPU_RISCV
#	define WK_CPU_RISCV 1
#	define WK_CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    \
 ||   defined(_M_X64)     \
 ||   defined(__i386__)   \
 ||   defined(__x86_64__)
#	undef  WK_CPU_X86
#	define WK_CPU_X86 1
#	define WK_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  WK_CPU_JIT
#	define WK_CPU_JIT 1
#	define WK_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__)    \
 || defined(_M_X64)        \
 || defined(__aarch64__)   \
 || defined(__64BIT__)     \
 || defined(__mips64)      \
 || defined(__powerpc64__) \
 || defined(__ppc64__)     \
 || defined(__LP64__)
#	undef  WK_ARCH_64BIT
#	define WK_ARCH_64BIT 64
#else
#	undef  WK_ARCH_32BIT
#	define WK_ARCH_32BIT 32
#endif //

#if WK_CPU_PPC
// _LITTLE_ENDIAN exists on ppc64le.
#	if _LITTLE_ENDIAN
#		undef  WK_CPU_ENDIAN_LITTLE
#		define WK_CPU_ENDIAN_LITTLE 1
#	else
#		undef  WK_CPU_ENDIAN_BIG
#		define WK_CPU_ENDIAN_BIG 1
#	endif
#else
#	undef  WK_CPU_ENDIAN_LITTLE
#	define WK_CPU_ENDIAN_LITTLE 1
#endif // WK_PLATFORM_

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  WK_PLATFORM_XBOXONE
#	define WK_PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64)
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
//  If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset.
#	if defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#		include <winapifamily.h>
#	endif // defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#	if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#		undef  WK_PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if WK_ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // WK_ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define WK_PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  WK_PLATFORM_WINRT
#		define WK_PLATFORM_WINRT 1
#	endif
#elif defined(__STEAMLINK__)
// SteamLink compiler defines __linux__
#	undef  WK_PLATFORM_STEAMLINK
#	define WK_PLATFORM_STEAMLINK 1
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  WK_PLATFORM_RPI
#	define WK_PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  WK_PLATFORM_LINUX
#	define WK_PLATFORM_LINUX 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  WK_PLATFORM_OSX
#	define WK_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	undef  WK_PLATFORM_EMSCRIPTEN
#	define WK_PLATFORM_EMSCRIPTEN 1
#elif defined(__ORBIS__)
#	undef  WK_PLATFORM_PS4
#	define WK_PLATFORM_PS4 1
#elif  defined(__FreeBSD__)        \
	|| defined(__FreeBSD_kernel__) \
	|| defined(__NetBSD__)         \
	|| defined(__OpenBSD__)        \
	|| defined(__DragonFly__)
#	undef  WK_PLATFORM_BSD
#	define WK_PLATFORM_BSD 1
#elif defined(__NX__)
#	undef  WK_PLATFORM_NX
#	define WK_PLATFORM_NX 1
#endif //

#if !WK_CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(_MSC_VER)
#		undef  WK_CRT_MSVC
#		define WK_CRT_MSVC 1
#	elif defined(__GLIBC__)
#		undef  WK_CRT_GLIBC
#		define WK_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__MINGW32__) || defined(__MINGW64__)
#		undef  WK_CRT_MINGW
#		define WK_CRT_MINGW 1
#	elif defined(__apple_build_version__) || defined(__ORBIS__) || defined(__EMSCRIPTEN__) || defined(__llvm__) || defined(__HAIKU__)
#		undef  WK_CRT_LIBCXX
#		define WK_CRT_LIBCXX 1
#	elif WK_PLATFORM_BSD
#		undef  WK_CRT_BSD
#		define WK_CRT_BSD 1
#	endif //

#	if !WK_CRT_BSD    \
	&& !WK_CRT_GLIBC  \
	&& !WK_CRT_LIBCXX \
	&& !WK_CRT_MINGW  \
	&& !WK_CRT_MSVC   \
	&& !WK_CRT_NEWLIB
#		undef  WK_CRT_NONE
#		define WK_CRT_NONE 1
#	endif // WK_CRT_*
#endif // !WK_CRT_NONE

///
#define WK_PLATFORM_POSIX (0   \
	||  WK_PLATFORM_BSD        \
	||  WK_PLATFORM_EMSCRIPTEN \
	||  WK_PLATFORM_LINUX      \
	||  WK_PLATFORM_NX         \
	||  WK_PLATFORM_OSX        \
	||  WK_PLATFORM_PS4        \
	||  WK_PLATFORM_RPI        \
	||  WK_PLATFORM_STEAMLINK  \
	)

///
#define WK_PLATFORM_NONE !(0   \
	||  WK_PLATFORM_BSD        \
	||  WK_PLATFORM_EMSCRIPTEN \
	||  WK_PLATFORM_LINUX      \
	||  WK_PLATFORM_NX         \
	||  WK_PLATFORM_OSX        \
	||  WK_PLATFORM_PS4        \
	||  WK_PLATFORM_RPI        \
	||  WK_PLATFORM_STEAMLINK  \
	||  WK_PLATFORM_WINDOWS    \
	||  WK_PLATFORM_WINRT      \
	||  WK_PLATFORM_XBOXONE    \
	)

///
#define WK_PLATFORM_OS_CONSOLE  (0 \
	||  WK_PLATFORM_NX             \
	||  WK_PLATFORM_PS4            \
	||  WK_PLATFORM_WINRT          \
	||  WK_PLATFORM_XBOXONE        \
	)

///
#define WK_PLATFORM_OS_DESKTOP  (0 \
	||  WK_PLATFORM_BSD            \
	||  WK_PLATFORM_LINUX          \
	||  WK_PLATFORM_OSX            \
	||  WK_PLATFORM_WINDOWS        \
	)

///
#define WK_PLATFORM_OS_EMBEDDED (0 \
	||  WK_PLATFORM_RPI            \
	||  WK_PLATFORM_STEAMLINK      \
	)

///
#define WK_PLATFORM_OS_WEB      (0 \
	||  WK_PLATFORM_EMSCRIPTEN     \
	)

///
#if WK_COMPILER_GCC
#	define WK_COMPILER_NAME "GCC "       \
		WK_STRINGIZE(__GNUC__) "."       \
		WK_STRINGIZE(__GNUC_MINOR__) "." \
		WK_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif WK_COMPILER_CLANG
#	define WK_COMPILER_NAME "Clang "      \
		WK_STRINGIZE(__clang_major__) "." \
		WK_STRINGIZE(__clang_minor__) "." \
		WK_STRINGIZE(__clang_patchlevel__)
#elif WK_COMPILER_MSVC
#	if WK_COMPILER_MSVC >= 1920 // Visual Studio 2019
#		define WK_COMPILER_NAME "MSVC 16.0"
#	elif WK_COMPILER_MSVC >= 1910 // Visual Studio 2017
#		define WK_COMPILER_NAME "MSVC 15.0"
#	elif WK_COMPILER_MSVC >= 1900 // Visual Studio 2015
#		define WK_COMPILER_NAME "MSVC 14.0"
#	elif WK_COMPILER_MSVC >= 1800 // Visual Studio 2013
#		define WK_COMPILER_NAME "MSVC 12.0"
#	elif WK_COMPILER_MSVC >= 1700 // Visual Studio 2012
#		define WK_COMPILER_NAME "MSVC 11.0"
#	elif WK_COMPILER_MSVC >= 1600 // Visual Studio 2010
#		define WK_COMPILER_NAME "MSVC 10.0"
#	elif WK_COMPILER_MSVC >= 1500 // Visual Studio 2008
#		define WK_COMPILER_NAME "MSVC 9.0"
#	else
#		define WK_COMPILER_NAME "MSVC"
#	endif //
#endif // WK_COMPILER_

#if WK_PLATFORM_BSD
#	define WK_PLATFORM_NAME "BSD"
#elif WK_PLATFORM_EMSCRIPTEN
#	define WK_PLATFORM_NAME "asm.js "          \
		WK_STRINGIZE(__EMSCRIPTEN_major__) "." \
		WK_STRINGIZE(__EMSCRIPTEN_minor__) "." \
		WK_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif WK_PLATFORM_LINUX
#	define WK_PLATFORM_NAME "Linux"
#elif WK_PLATFORM_NONE
#	define WK_PLATFORM_NAME "None"
#elif WK_PLATFORM_NX
#	define WK_PLATFORM_NAME "NX"
#elif WK_PLATFORM_OSX
#	define WK_PLATFORM_NAME "OSX"
#elif WK_PLATFORM_PS4
#	define WK_PLATFORM_NAME "PlayStation 4"
#elif WK_PLATFORM_RPI
#	define WK_PLATFORM_NAME "RaspberryPi"
#elif WK_PLATFORM_STEAMLINK
#	define WK_PLATFORM_NAME "SteamLink"
#elif WK_PLATFORM_WINDOWS
#	define WK_PLATFORM_NAME "Windows"
#elif WK_PLATFORM_WINRT
#	define WK_PLATFORM_NAME "WinRT"
#elif WK_PLATFORM_XBOXONE
#	define WK_PLATFORM_NAME "Xbox One"
#else
#	error "Unknown WK_PLATFORM!"
#endif // WK_PLATFORM_

#if WK_CPU_ARM
#	define WK_CPU_NAME "ARM"
#elif WK_CPU_JIT
#	define WK_CPU_NAME "JIT-VM"
#elif WK_CPU_MIPS
#	define WK_CPU_NAME "MIPS"
#elif WK_CPU_PPC
#	define WK_CPU_NAME "PowerPC"
#elif WK_CPU_RISCV
#	define WK_CPU_NAME "RISC-V"
#elif WK_CPU_X86
#	define WK_CPU_NAME "x86"
#endif // WK_CPU_

#if WK_CRT_BSD
#	define WK_CRT_NAME "BSD libc"
#elif WK_CRT_GLIBC
#	define WK_CRT_NAME "GNU C Library"
#elif WK_CRT_MSVC
#	define WK_CRT_NAME "MSVC C Runtime"
#elif WK_CRT_MINGW
#	define WK_CRT_NAME "MinGW C Runtime"
#elif WK_CRT_LIBCXX
#	define WK_CRT_NAME "Clang C Library"
#elif WK_CRT_NEWLIB
#	define WK_CRT_NAME "Newlib"
#elif WK_CRT_NONE
#	define WK_CRT_NAME "None"
#else
#	error "Unknown WK_CRT!"
#endif // WK_CRT_

#if WK_ARCH_32BIT
#	define WK_ARCH_NAME "32-bit"
#elif WK_ARCH_64BIT
#	define WK_ARCH_NAME "64-bit"
#endif // WK_ARCH_

#if WK_COMPILER_MSVC
#	define WK_CPP_NAME "C++MsvcUnknown"
#elif defined(__cplusplus)
#	if __cplusplus < 201103L
#		error "Pre-C++11 compiler is not supported!"
#	elif __cplusplus < 201402L
#		define WK_CPP_NAME "C++11"
#	elif __cplusplus < 201703L
#		define WK_CPP_NAME "C++14"
#	elif __cplusplus < 201704L
#		define WK_CPP_NAME "C++17"
#	else
// See: https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b#orthodox-c
#		define WK_CPP_NAME "C++WayTooModern"
#	endif // WK_CPP_NAME
#else
#	define WK_CPP_NAME "C++Unknown"
#endif // defined(__cplusplus)

#endif // WK_PLATFORM_H_HEADER_GUARD

