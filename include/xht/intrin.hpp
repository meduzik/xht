#pragma once

#include "config.hpp"
#include "feature.hpp"
#include "core.hpp"

#if defined(XHT_COMPILER_MSVC)
#include <intrin.h>

namespace xht {

	inline xht_forceinline u32 _ctz(u32 x) {
		unsigned long r = 0;
		_BitScanForward(&r, x);
		return r;
	}

	inline xht_forceinline u32 _clz(u32 value) {
		unsigned long r = 0;
		_BitScanReverse(&r, value);
		return 31 - r;
	}

}

#define xht_ctz32(x) xht::_ctz(x)
#define xht_clz32(x) xht::_clz(x)
#else
#include <x86intrin.h>
#define xht_ctz32(x) __builtin_ctz(x)
#define xht_clz32(x) __builtin_clz(x)
#endif


