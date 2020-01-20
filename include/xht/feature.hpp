#pragma once

#include "config.hpp"
#include <cassert>

#if defined(_MSC_VER)
	#define XHT_COMPILER_MSVC
#elif defined(__clang)
	#define XHT_COMPILER_CLANG
#elif defined(__GNUC__)
	#define XHT_COMPILER_GCC
#else
	#error Unknown compiler
#endif

#if !defined(xht_forceinline)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_forceinline __forceinline
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_forceinline __attribute__((always_inline))
	#else
		#error No default for xht_forceinline
	#endif
#endif

#if !defined(xht_noinline)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_noinline __declspec(noinline)
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_noinline __attribute__((noinline))
	#else
		#error No default for xht_noinline
	#endif
#endif

#if !defined(xht_likely)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_likely(x) (x)
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_likely(x) __builtin_expect(x, 1)
	#else
		#error No default for xht_likely
	#endif
#endif
	
#if !defined(xht_unlikely)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_unlikely(x) (x)
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_unlikely(x) __builtin_expect(x, 0)
	#else
		#error No default for xht_unlikely
	#endif
#endif

#if !defined(xht_assert)
	#define xht_assert(c) assert(c)
#endif

#if !defined(XHT_PUBLIC)
	#if defined(XHT_SHARED)
		#if defined(XHT_BUILD)
			#if defined(XHT_COMPILER_MSVC)
				#define XHT_PUBLIC __declspec(dllexport)
			#else
				#error Shared export decorator is not implemented for the compiler
			#endif
		#else
			#if defined(XHT_COMPILER_MSVC)
				#define XHT_PUBLIC __declspec(dllimport)
			#else
				#error Shared import decorator is not implemented for the compiler
			#endif
		#endif
	#else
		#define XHT_PUBLIC
	#endif
#endif

