#pragma once

// Decorator for public symbols.
// Should set to __declspec(dllexport) / __declspec(dllimport) for DLL linkage on Windows
// #define XHT_PUBLIC

// Defines if the hashtable is built as a shared or static library.
// Assumes static library by default
// #define XHT_SHARED
// #define XHT_STATIC

// Define if the hash table implementation is being built.
// Makes XHT_PUBLIC resolve into __declspec(dllexport) as opposed to __declspec(dllimport) by default
// Automatically defined if XHT_INCLUDE_IMPLEMENTATION is defined
// #define XHT_BUILD

// A function decorator that forces the function to be inlined by the compiler.
// #define xht_forceinline

// A function decorator that forces the function to not be inlined by the compiler.
// #define xht_noinline

// Must evaluate to x. Should hint that x is likely to be true.
// #define xht_likely(x)

// Must evaluate to x. Should hint that x is likely to be false.
// #define xht_unlikely(x)

// An assertion that can be checked at runtime.
// Defined as stdlib assert(c) by default.
// #define xht_assert(c)


// #define XHT_SSE2 1
// #define XHT_SSSE3 1
