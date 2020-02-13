#pragma once

#if defined(XHT_CONFIG_FILE)
#include XHT_CONFIG_FILE
#endif

#if !defined(XHT_STATIC) && !defined(XHT_SHARED)
	#define XHT_STATIC
#endif

#if defined(XHT_INCLUDE_IMPLEMENTATION) && !defined(XHT_BUILD)
	#define XHT_BUILD
#endif

#if !defined(XHT_SSE2)
	#if defined(__SSE2__) || (defined(_M_IX86_FP) && (_M_IX86_FP+0 == 2)) || defined(_M_X64)
		#define XHT_SSE2 1
	#else
		#define XHT_SSE2 0
	#endif
#endif

#if !defined(XHT_SSSE3)
	// just assume ssse3 on msvc
	#if __SSSE3__ || (defined(_M_IX86_FP) && (_M_IX86_FP+0 == 2)) || defined(_M_X64)
		#define XHT_SSSE3 1
	#else
		#define XHT_SSSE3 0
	#endif
#endif

#if XHT_SSSE3 && !XHT_SSE2
	#error Incorrect SSE settings
#endif

namespace xht {
}
