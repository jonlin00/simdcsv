/*
 * Copyright 2010-2019 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef WK_MACROS_H
#define WK_MACROS_H

#include "Platform.h"

#if _DEBUG
#define WK_BUILD_DEBUG 1
#define WK_BUILD_RELEASE 1
#else
#define WK_BUILD_DEBUG 0
#define WK_BUILD_RELEASE 1
#endif// _DEBUG

 ///
#if WK_COMPILER_MSVC
// Workaround MSVS bug...
#	define WK_VA_ARGS_PASS(...) WK_VA_ARGS_PASS_1_ __VA_ARGS__ WK_VA_ARGS_PASS_2_
#	define WK_VA_ARGS_PASS_1_ (
#	define WK_VA_ARGS_PASS_2_ )
#else
#	define WK_VA_ARGS_PASS(...) (__VA_ARGS__)
#endif // WK_COMPILER_MSVC

#define WK_VA_ARGS_COUNT(...) WK_VA_ARGS_COUNT_ WK_VA_ARGS_PASS(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define WK_VA_ARGS_COUNT_(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12, _a13, _a14, _a15, _a16, _last, ...) _last

///
#define WK_MACRO_DISPATCHER(_func, ...) WK_MACRO_DISPATCHER_1_(_func, WK_VA_ARGS_COUNT(__VA_ARGS__) )
#define WK_MACRO_DISPATCHER_1_(_func, _argCount) WK_MACRO_DISPATCHER_2_(_func, _argCount)
#define WK_MACRO_DISPATCHER_2_(_func, _argCount) WK_CONCATENATE(_func, _argCount)

///
#define WK_MAKEFOURCC(_a, _b, _c, _d) ( ( (uint32_t)(_a) | ( (uint32_t)(_b) << 8) | ( (uint32_t)(_c) << 16) | ( (uint32_t)(_d) << 24) ) )

///
#define WK_STRINGIZE(_x) WK_STRINGIZE_(_x)
#define WK_STRINGIZE_(_x) #_x

///
#define WK_CONCATENATE(_x, _y) WK_CONCATENATE_(_x, _y)
#define WK_CONCATENATE_(_x, _y) _x ## _y

///
#define WK_FILE_LINE_LITERAL "" __FILE__ ":" WK_STRINGIZE(__LINE__)

///
#define WK_ALIGN_MASK(_value, _mask) ( ( (_value)+(_mask) ) & ( (~0)&(~(_mask) ) ) )
#define WK_ALIGN_16(_value) WK_ALIGN_MASK(_value, 0xf)
#define WK_ALIGN_256(_value) WK_ALIGN_MASK(_value, 0xff)
#define WK_ALIGN_4096(_value) WK_ALIGN_MASK(_value, 0xfff)

///
#define WK_ALIGNOF(_type) __alignof(_type)
#define WK_OFFSETOF(type, member) ((size_t)(&(((type*)0)->member)))

#if defined(__has_feature)
#	define WK_CLANG_HAS_FEATURE(_x) __has_feature(_x)
#else
#	define WK_CLANG_HAS_FEATURE(_x) 0
#endif // defined(__has_feature)

#if defined(__has_extension)
#	define WK_CLANG_HAS_EXTENSION(_x) __has_extension(_x)
#else
#	define WK_CLANG_HAS_EXTENSION(_x) 0
#endif // defined(__has_extension)

#if WK_COMPILER_GCC || WK_COMPILER_CLANG
#	define WK_ALIGN_DECL(_align, _decl) _decl __attribute__( (aligned(_align) ) )
#	define WK_ALLOW_UNUSED __attribute__( (unused) )
#	define WK_FORCE_INLINE inline __attribute__( (__always_inline__) )
#	define WK_FUNCTION __PRETTY_FUNCTION__
#	define WK_LIKELY(_x)   __builtin_expect(!!(_x), 1)
#	define WK_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#	define WK_NO_INLINE   __attribute__( (noinline) )
#	define WK_NO_RETURN   __attribute__( (noreturn) )
#	define WK_CONST_FUNC  __attribute__( (const) )

#	if WK_COMPILER_GCC >= 70000
#		define WK_FALLTHROUGH __attribute__( (fallthrough) )
#	else
#		define WK_FALLTHROUGH WK_NOOP()
#	endif // WK_COMPILER_GCC >= 70000

#	define WK_NO_VTABLE
#	define WK_PRINTF_ARGS(_format, _args) __attribute__( (format(__printf__, _format, _args) ) )

#	if WK_CLANG_HAS_FEATURE(cxx_thread_local)
#		define WK_THREAD_LOCAL __thread
#	endif // WK_COMPILER_CLANG

#	if (!WK_PLATFORM_OSX && (WK_COMPILER_GCC >= 40200)) || (WK_COMPILER_GCC >= 40500)
#		define WK_THREAD_LOCAL __thread
#	endif // WK_COMPILER_GCC

#	define WK_ATTRIBUTE(_x) __attribute__( (_x) )

#	if WK_CRT_MSVC
#		define __stdcall
#	endif // WK_CRT_MSVC
#elif WK_COMPILER_MSVC
#	define WK_ALIGN_DECL(_align, _decl) __declspec(align(_align) ) _decl
#	define WK_ALLOW_UNUSED
#	define WK_FORCE_INLINE __forceinline
#	define WK_FUNCTION __FUNCTION__
#	define WK_LIKELY(_x)   (_x)
#	define WK_UNLIKELY(_x) (_x)
#	define WK_NO_INLINE __declspec(noinline)
#	define WK_NO_RETURN
#	define WK_CONST_FUNC  __declspec(noalias)
#	define WK_FALLTHROUGH WK_NOOP()
#	define WK_NO_VTABLE __declspec(novtable)
#	define WK_PRINTF_ARGS(_format, _args)
#	define WK_THREAD_LOCAL __declspec(thread)
#	define WK_ATTRIBUTE(_x)
#else
#	error "Unknown WK_COMPILER_?"
#endif

/// The return value of the function is solely a function of the arguments.
///
#if __cplusplus < 201402
#	define WK_CONSTEXPR_FUNC WK_CONST_FUNC
#else
#	define WK_CONSTEXPR_FUNC constexpr WK_CONST_FUNC
#endif // __cplusplus < 201402

///
#define WK_STATIC_ASSERT(_condition, ...) static_assert(_condition, "" __VA_ARGS__)

///
#define WK_ALIGN_DECL_16(_decl) WK_ALIGN_DECL(16, _decl)
#define WK_ALIGN_DECL_256(_decl) WK_ALIGN_DECL(256, _decl)
#define WK_ALIGN_DECL_CACHE_LINE(_decl) WK_ALIGN_DECL(WK_CACHE_LINE_SIZE, _decl)

///
#define WK_MACRO_BLOCK_BEGIN for(;;) {
#define WK_MACRO_BLOCK_END break; }
#define WK_NOOP(...) (void)0

#define WK_COUNTOF(x) (sizeof(x)/sizeof(x[0]))

#define WK_BIT(x)              (1ULL << (x))
#define WK_BIT_SET(n, x)       ((n) | (x))
#define WK_BIT_GET(n, x)       ((n) & (x))
#define WK_BIT_CLEAR(n, x)     ((n) & ~(x))
#define WK_BIT_TOGGLE(n, x)    ((n) ^ (x))
#define WK_BIT_CHANGE(n, x, v) ((n) ^ ((-(v) ^ (n)) & (x)))

///
#define WK_UNUSED_1(_a1)                                          \
	WK_MACRO_BLOCK_BEGIN                                            \
		WK_PRAGMA_DIAGNOSTIC_PUSH();                                  \
		/*WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC("-Wuseless-cast");*/ \
		(void)(true ? WK_NOOP : ( (void)(_a1) ) );                    \
		WK_PRAGMA_DIAGNOSTIC_POP();                                   \
	WK_MACRO_BLOCK_END

#define WK_UNUSED_2(_a1, _a2) WK_UNUSED_1(_a1); WK_UNUSED_1(_a2)
#define WK_UNUSED_3(_a1, _a2, _a3) WK_UNUSED_2(_a1, _a2); WK_UNUSED_1(_a3)
#define WK_UNUSED_4(_a1, _a2, _a3, _a4) WK_UNUSED_3(_a1, _a2, _a3); WK_UNUSED_1(_a4)
#define WK_UNUSED_5(_a1, _a2, _a3, _a4, _a5) WK_UNUSED_4(_a1, _a2, _a3, _a4); WK_UNUSED_1(_a5)
#define WK_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6) WK_UNUSED_5(_a1, _a2, _a3, _a4, _a5); WK_UNUSED_1(_a6)
#define WK_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7) WK_UNUSED_6(_a1, _a2, _a3, _a4, _a5, _a6); WK_UNUSED_1(_a7)
#define WK_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8) WK_UNUSED_7(_a1, _a2, _a3, _a4, _a5, _a6, _a7); WK_UNUSED_1(_a8)
#define WK_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9) WK_UNUSED_8(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8); WK_UNUSED_1(_a9)
#define WK_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10) WK_UNUSED_9(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9); WK_UNUSED_1(_a10)
#define WK_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11) WK_UNUSED_10(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10); WK_UNUSED_1(_a11)
#define WK_UNUSED_12(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11, _a12) WK_UNUSED_11(_a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9, _a10, _a11); WK_UNUSED_1(_a12)

#if WK_COMPILER_MSVC
// Workaround MSVS bug...
#	define WK_UNUSED(...) WK_MACRO_DISPATCHER(WK_UNUSED_, __VA_ARGS__) WK_VA_ARGS_PASS(__VA_ARGS__)
#else
#	define WK_UNUSED(...) WK_MACRO_DISPATCHER(WK_UNUSED_, __VA_ARGS__)(__VA_ARGS__)
#endif // WK_COMPILER_MSVC

///
#if WK_COMPILER_CLANG
#	define WK_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()     _Pragma("clang diagnostic push")
#	define WK_PRAGMA_DIAGNOSTIC_POP_CLANG_()      _Pragma("clang diagnostic pop")
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x) _Pragma(WK_STRINGIZE(clang diagnostic ignored _x) )
#else
#	define WK_PRAGMA_DIAGNOSTIC_PUSH_CLANG_()
#	define WK_PRAGMA_DIAGNOSTIC_POP_CLANG_()
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG(_x)
#endif // WK_COMPILER_CLANG

#if WK_COMPILER_GCC && WK_COMPILER_GCC >= 40600
#	define WK_PRAGMA_DIAGNOSTIC_PUSH_GCC_()       _Pragma("GCC diagnostic push")
#	define WK_PRAGMA_DIAGNOSTIC_POP_GCC_()        _Pragma("GCC diagnostic pop")
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)   _Pragma(WK_STRINGIZE(GCC diagnostic ignored _x) )
#else
#	define WK_PRAGMA_DIAGNOSTIC_PUSH_GCC_()
#	define WK_PRAGMA_DIAGNOSTIC_POP_GCC_()
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_GCC(_x)
#endif // WK_COMPILER_GCC

#if WK_COMPILER_MSVC
#	define WK_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()     __pragma(warning(push) )
#	define WK_PRAGMA_DIAGNOSTIC_POP_MSVC_()      __pragma(warning(pop) )
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x) __pragma(warning(disable:_x) )
#else
#	define WK_PRAGMA_DIAGNOSTIC_PUSH_MSVC_()
#	define WK_PRAGMA_DIAGNOSTIC_POP_MSVC_()
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_MSVC(_x)
#endif // WK_COMPILER_CLANG

#if WK_COMPILER_CLANG
#	define WK_PRAGMA_DIAGNOSTIC_PUSH              WK_PRAGMA_DIAGNOSTIC_PUSH_CLANG_
#	define WK_PRAGMA_DIAGNOSTIC_POP               WK_PRAGMA_DIAGNOSTIC_POP_CLANG_
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG
#elif WK_COMPILER_GCC
#	define WK_PRAGMA_DIAGNOSTIC_PUSH              WK_PRAGMA_DIAGNOSTIC_PUSH_GCC_
#	define WK_PRAGMA_DIAGNOSTIC_POP               WK_PRAGMA_DIAGNOSTIC_POP_GCC_
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC WK_PRAGMA_DIAGNOSTIC_IGNORED_GCC
#elif WK_COMPILER_MSVC
#	define WK_PRAGMA_DIAGNOSTIC_PUSH              WK_PRAGMA_DIAGNOSTIC_PUSH_MSVC_
#	define WK_PRAGMA_DIAGNOSTIC_POP               WK_PRAGMA_DIAGNOSTIC_POP_MSVC_
#	define WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG_GCC(_x)
#endif // WK_COMPILER_

///
#define WK_CLASS_NO_DEFAULT_CTOR(_class) \
	private: _class()

#define WK_CLASS_NO_COPY(_class) \
	private: _class(const _class& _rhs)

#define WK_CLASS_NO_ASSIGNMENT(_class) \
	private: _class& operator=(const _class& _rhs)

#define WK_CLASS_ALLOCATOR(_class)              \
	public: void* operator new(size_t _size);   \
	public: void  operator delete(void* _ptr);  \
	public: void* operator new[](size_t _size); \
	public: void  operator delete[](void* _ptr)

#define WK_CLASS_1(_class, _a1) WK_CONCATENATE(WK_CLASS_, _a1)(_class)
#define WK_CLASS_2(_class, _a1, _a2) WK_CLASS_1(_class, _a1); WK_CLASS_1(_class, _a2)
#define WK_CLASS_3(_class, _a1, _a2, _a3) WK_CLASS_2(_class, _a1, _a2); WK_CLASS_1(_class, _a3)
#define WK_CLASS_4(_class, _a1, _a2, _a3, _a4) WK_CLASS_3(_class, _a1, _a2, _a3); WK_CLASS_1(_class, _a4)

#if WK_COMPILER_MSVC
#	define WK_CLASS(_class, ...) WK_MACRO_DISPATCHER(WK_CLASS_, __VA_ARGS__) WK_VA_ARGS_PASS(_class, __VA_ARGS__)
#else
#	define WK_CLASS(_class, ...) WK_MACRO_DISPATCHER(WK_CLASS_, __VA_ARGS__)(_class, __VA_ARGS__)
#endif // WK_COMPILER_MSVC

// static_assert sometimes causes unused-local-typedef...
WK_PRAGMA_DIAGNOSTIC_IGNORED_CLANG("-Wunused-local-typedef")

#define WK_MAXFILEPATH 1024

#endif // WK_MACROS_H
