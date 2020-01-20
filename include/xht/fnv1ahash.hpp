#pragma once

#include "config.hpp"
#include "core.hpp"

namespace xht {

	struct FNV1aHash {
		typedef u64 StateType;

		static constexpr u64 Seed = 0xa1807282'fb1a2f9d;

		static xht_forceinline u64 Init() {
			return Seed;
		}

		template<typename T>
		static xht_forceinline u64 Combine(u64 state, T value) {
			static_assert(std::is_integral_v<T>);

			constexpr u64 k = 0x9ddfea08eb382d69;
			u64 m = (u64)(state + (u64)value);
			return m;
		}

		static xht_forceinline u64 Combine(
			u64 state,
			const void* data,
			uword size
		) {
			return Combine(state, HashBytes(data, size));
		}

		static u64 HashBytes(const void* data, uword size) {
			uword hash = 0xcbf29ce484222325;
			for (uword i = 0; i < size; ++i) {
				hash ^= ((const u8*)data)[i];
				hash *= 0x100000001b3;
			}
			return hash;
		}

		static xht_forceinline uword Finalize(u64 state) {
			return state;
		}
	};

}
