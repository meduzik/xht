#pragma once

#include "config.hpp"
#include "feature.hpp"
#include "core.hpp"

#if defined(XHT_COMPILER_MSVC)
#include <intrin.h>

namespace xht {

	inline xht_forceinline u32 _ctz32(u32 x) {
		unsigned long r = 0;
		_BitScanForward(&r, x);
		return r;
	}

	inline xht_forceinline u32 _clz32(u32 value) {
		unsigned long r = 0;
		_BitScanReverse(&r, value);
		return 31 - r;
	}

	#if defined(_M_X64)
	inline xht_forceinline u64 _ctz64(u64 x) {
		unsigned long r = 0;
		_BitScanForward64(&r, x);
		return r;
	}

	inline xht_forceinline u64 _clz64(u64 value) {
		unsigned long r = 0;
		_BitScanReverse64(&r, value);
		return 63 - r;
	}
	#endif
}

	#define xht_ctz32(x) xht::_ctz32(x)
	#define xht_clz32(x) xht::_clz32(x)

	#if defined(_M_X64)
	#define xht_ctz64(x) xht::_ctz64(x)
	#define xht_clz64(x) xht::_clz64(x)
	#endif

#else

	#if XHT_SSE2
	#include <x86intrin.h>
	#endif

	#define xht_ctz32(x) __builtin_ctz(x)
	#define xht_ctz64(x) __builtin_ctzll(x)
	#define xht_clz32(x) __builtin_clz(x)
	#define xht_clz64(x) __builtin_clzll(x)
#endif


