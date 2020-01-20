#pragma once

#include "config.hpp"
#include "feature.hpp"
#include "traits.hpp"

namespace xht {
namespace ext {

template<typename TKey>
xht_forceinline const TKey& SimplifyKey(const TKey& key) {
	return key;
}

}

template<typename T>
using SimpleKey = std::decay_t<decltype(ext::SimplifyKey(std::declval<T>()))>;

template<typename TKey, typename TValue>
struct KeyValuePair
{
	typedef TKey KeyType;
	typedef TValue ValueType;

	TKey Key;
	TValue Value;

	explicit xht_forceinline KeyValuePair(TKey key)
		: Key(std::move(key))
	{
	}

	xht_forceinline KeyValuePair(TKey key, TValue value)
		: Key(std::move(key)), Value(std::move(value))
	{
	}
};

}

template<typename TKey, typename TValue>
constexpr bool xht::meta::IsTriviallyRelocatable<xht::KeyValuePair<TKey, TValue>> =
xht::meta::IsTriviallyRelocatable<TKey> && meta::IsTriviallyRelocatable<TValue>;

