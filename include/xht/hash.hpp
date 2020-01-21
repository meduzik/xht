#pragma once

#include "config.hpp"
#include "traits.hpp"
#include "fnv1ahash.hpp"
#include <string_view>

namespace xht {

template<typename TState, typename T>
TState HashExt(TState, ...) = delete;

template<typename TState>
xht_forceinline TState HashExt(TState state, bool value) {
	return state.Combine((u8)value);
}

template<typename TState, typename T>
xht_forceinline TState HashExt(TState state, T* value) {
	uword integer = (uword)value;
	return state.Combine(integer, integer);
}

template<typename TState, typename T>
xht_forceinline auto HashExt(TState state, T value)
	-> std::enable_if_t<std::is_floating_point_v<T>, TState> {
	return state.CombineBytes((const void*)&value, sizeof(value));
}

template<typename TState>
xht_forceinline TState HashExt(TState state, std::string_view value) {
	return state.CombineBytes(value.data(), value.size());
}

template<typename TState, typename T>
xht_forceinline auto HashExt(TState state, const T& value)
	-> std::enable_if_t<xht::meta::HasUniqueRepresentation<T>, TState> {
	if constexpr (sizeof(T) <= 8) {
		u64 bits = 0;
		memcpy(&bits, &value, sizeof(value));
		return state.Combine(bits);
	}
	else {
		return state.CombineBytes((const void*)&value, sizeof(value));
	}
}

template<typename TState, typename A, typename B>
xht_forceinline TState HashExt(TState state, const std::pair<A, B>& value) {
	return state.Combine(value.first, value.second);
}

template<typename TState, typename T>
xht_forceinline TState Hash(TState state, T val) {
	return HashExt(state, val);
}

using DefaultHash = FNV1aHash<meta::Select<sizeof(uword) == 8, u64, u32>>;

template<typename THash>
struct HashState {
	static_assert(
		std::is_trivial_v<typename THash::StateType>,
		"hash state must be trivial");

	typename THash::StateType m_state;

	template<typename T>
	xht_forceinline HashState Combine(const T& value) {
		if constexpr (std::is_integral_v<T>) {
			return { THash::Combine(m_state, value) };
		}
		else {
			return Hash(*this, value);
		}
	}

	template<typename... TArgs>
	xht_forceinline HashState Combine(const TArgs&... args) {
		HashState state = *this;
		((state = state.Combine(args)), ...);
		return state;
	}

	template<typename T>
	xht_forceinline HashState CombineArray(const T* data, iword size) {
		if constexpr (meta::HasUniqueRepresentation<T>) {
			return CombineBytes(data, (uword)size * sizeof(T));
		}
		else {
			HashState state = *this;
			for (iword i = 0; i < size; ++i)
				state = state.Combine(data[i]);
			return state;
		}
	}

	xht_forceinline HashState CombineBytes(const void* data, uword size) {
		return { THash::Combine(m_state, data, size) };
	}
};

template<typename THash>
struct BasicHasher {
	template<typename T>
	static xht_noinline uword Hash(const T& value) {
		HashState<THash> state{ THash::Init() };
		return THash::Finalize(state.Combine(value).m_state);
	}
};

namespace ext {

	template<typename T>
	struct Hasher {
		using Type = BasicHasher<DefaultHash>;
	};

}

template<typename T>
using DefaultHasher = typename ext::Hasher<T>::Type;

}
