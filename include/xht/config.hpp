#pragma once

#if __has_include("xhtconfig.hpp")
#include "xhtconfig.hpp"
#endif

#if !defined(XHT_STATIC) && !defined(XHT_SHARED)
	#define XHT_STATIC
#endif

#if defined(XHT_INCLUDE_IMPLEMENTATION)
	#define XHT_BUILD
#endif

namespace xht {
}
