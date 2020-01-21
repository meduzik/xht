#pragma once

#include "core.hpp"
#include <cstring>

namespace xht {

struct BasicComparator
{
	template<typename T1, typename T2>
	static xht_forceinline bool Compare(const T1& t1, const T2& t2)
	{
		if constexpr (std::is_floating_point_v<T1>&& std::is_floating_point_v<T2>) {
			using T3 = std::common_type_t<T1, T2>;
			T3 x = t1;
			T3 y = t2;
			return !memcmp(&x, &y, sizeof(T3));
		}
		else {
			return t1 == t2;
		}
	}
};


namespace ext {

	template<typename K>
	struct Comparator {
		using Type = BasicComparator;
	};

}

template<class K>
using DefaultComparator = typename ext::Comparator<K>::Type;


}
