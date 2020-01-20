#pragma once

#include "config.hpp"
#include "core.hpp"
#include "feature.hpp"

namespace xht {

	namespace comptime {


		template<typename T>
		constexpr xht_forceinline T round_up_to_pot(T value) {
			static_assert(std::is_unsigned_v<T>);
			T acc = 1;
			while (acc < value) {
				acc *= 2;
			}
			return acc;
		}


	}


	template<uword Alignment, class T>
	T align(T val) {
		static_assert(Alignment > 0 && !(Alignment & (Alignment - 1)));
		return val & ~(Alignment - 1);
	}


#if defined(XHT_COMPILER_MSVC)
#include <intrin.h>

	inline xht_forceinline u32 intlog2(u32 val) {
		unsigned long r;
		_BitScanReverse(&r, val);
		return r;
	}

	inline xht_forceinline u64 intlog2(u64 val) {
#if defined(XHT_PLATFORM_WIN64)
		unsigned long r;
		_BitScanReverse64(&r, val);
		return r;
#else
		unsigned long lo, hi;

		char high_set = _BitScanReverse(&hi, (u32)(val >> 32));
		_BitScanReverse(&lo, (u32)val);
		hi |= 32;

		return high_set ? hi : lo;
#endif
	}
#else
	inline xht_forceinline u32 intlog2(u32 val) {
		return 31 - __builtin_clz(val);
	}

	inline xht_forceinline u64 intlog2(u64 val) {
		return 63 - __builtin_clzll(val);
	}
#endif

}
