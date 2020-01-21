#pragma once

#include "config.hpp"
#include "core.hpp"

namespace xht {

template<class T>
struct FNV1aHash;

template<>
struct FNV1aHash<u64> {
	using StateType = u64;

	static constexpr StateType Seed = 0xa1807282'fb1a2f9d;

	static xht_forceinline StateType Init() {
		return Seed;
	}

	template<typename T>
	static xht_forceinline StateType Combine(StateType state, T value) {
		static_assert(std::is_integral_v<T>);

		constexpr StateType k = 0x9ddfea08eb382d69;
		StateType m = (StateType)(state + (StateType)value);
		return m;
	}

	static xht_forceinline StateType Combine(
		StateType state,
		const void* data,
		uword size
	) {
		return Combine(state, HashBytes(data, size));
	}

	static StateType HashBytes(const void* data, uword size) {
		StateType hash = 0xcbf29ce484222325;
		for (uword i = 0; i < size; ++i) {
			hash ^= ((const u8*)data)[i];
			hash *= 0x100000001b3;
		}
		return hash;
	}

	static xht_forceinline StateType Finalize(StateType state) {
		return state;
	}
};

template<>
struct FNV1aHash<u32> {
	using StateType = u32;

	static constexpr StateType Seed = 0xfb1a2f9d;

	static xht_forceinline StateType Init() {
		return Seed;
	}

	template<typename T>
	static xht_forceinline StateType Combine(StateType state, T value) {
		static_assert(std::is_integral_v<T>);

		constexpr StateType k = 0x9ddfea08;
		StateType m = (StateType)(state + (StateType)value);
		return m;
	}

	static xht_forceinline StateType Combine(
		StateType state,
		const void* data,
		uword size
	) {
		return Combine(state, HashBytes(data, size));
	}

	static StateType HashBytes(const void* data, uword size) {
		StateType hash = 0x811c9dc5;
		for (uword i = 0; i < size; ++i) {
			hash ^= ((const u8*)data)[i];
			hash *= 0x01000193;
		}
		return hash;
	}

	static xht_forceinline StateType Finalize(StateType state) {
		return state;
	}
};

}
