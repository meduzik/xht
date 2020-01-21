#pragma once

namespace xht {

template<typename TKey>
xht_forceinline const TKey& SimplifyKeyExt(const TKey& key) {
	return key;
}

template<typename TKey>
xht_forceinline decltype(auto) SimplifyKey(const TKey& key) {
	return SimplifyKeyExt(key);
}

struct BasicSimplify
{
	template<typename T>
	using SimpleKey = std::decay_t<decltype(xht::SimplifyKey(std::declval<T>()))>;

	template<typename TKey, typename TInKey, typename TSimpleKeyType>
	static xht_forceinline void SimplifyVerify() {
		static_assert(
			std::is_trivial_v<TKey>
			&&
			std::is_convertible_v<TSimpleKeyType, TKey>,
			"Implicit conversion to simplified key type failed"
		);
	}

	template<typename TKey, typename TInKey>
	static xht_forceinline decltype(auto) SimplifyKey(const TInKey& inKey)
	{
		using TSimpleKey = SimpleKey<TKey>;

		if constexpr (std::is_same_v<TKey, TInKey>)
		{
			return inKey;
		}
		else
		{
			decltype(auto) key = xht::SimplifyKey(inKey);
			using TSimpleInKey = std::decay_t<decltype(key)>;

			if constexpr (std::is_same_v<TSimpleInKey, TSimpleKey>)
			{
				return key;
			}
			else
			{
				SimplifyVerify<TSimpleKey, TInKey, TSimpleInKey>();
				return static_cast<TSimpleKey>(key);
			}
		}
	}

};

namespace ext {
	template<typename T>
	struct Simplifier {
		using Type = BasicSimplify;
	};
}

template<class K>
using DefaultSimplifier = typename ext::Simplifier<K>::Type;

}
