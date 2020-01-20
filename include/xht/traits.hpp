#pragma once

#include "config.hpp"
#include <type_traits>

namespace xht::meta {

	namespace impl {
		template<bool>
		struct Select;

		template<>
		struct Select<0> {
			template<typename, typename T>
			using X = T;
		};

		template<>
		struct Select<1> {
			template<typename T, typename>
			using X = T;
		};
	}


	template<bool TCondition, typename T1, typename T0>
	using Select = typename impl::Select<TCondition>::template X<T1, T0>;


	template<typename T>
	constexpr bool IsTriviallyRelocatable = std::is_trivially_copyable_v<T>;

	template<typename T>
	constexpr bool IsZeroConstructible = std::is_trivially_constructible_v<T>;

	template<typename T>
	constexpr bool HasUniqueRepresentation = std::has_unique_object_representations_v<T>;

	template<typename T>
	using ConstructType = Select<IsZeroConstructible<T>, u8, T>;

	template<typename T>
	using DestroyType = Select<std::is_trivially_destructible_v<T>, u8, T>;

	template<typename T>
	using CopyType = Select<std::is_trivially_copyable_v<T>, u8, T>;
}
