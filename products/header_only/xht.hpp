#if !defined(XHT_UNITY_HEADER)
#define XHT_UNITY_HEADER
// #### BEGIN config.hpp #### 

#if __has_include("xhtconfig.hpp")
#endif

#if !defined(XHT_STATIC) && !defined(XHT_SHARED)
	#define XHT_STATIC
#endif

#if defined(XHT_INCLUDE_IMPLEMENTATION)
	#define XHT_BUILD
#endif

namespace xht {
}
// #### END config.hpp #### 
// #### BEGIN feature.hpp #### 

#include <cassert>

#if defined(_MSC_VER)
	#define XHT_COMPILER_MSVC
#elif defined(__clang)
	#define XHT_COMPILER_CLANG
#elif defined(__GNUC__)
	#define XHT_COMPILER_GCC
#else
	#error Unknown compiler
#endif

#if !defined(xht_forceinline)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_forceinline __forceinline
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_forceinline __attribute__((always_inline))
	#else
		#error No default for xht_forceinline
	#endif
#endif

#if !defined(xht_noinline)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_noinline __declspec(noinline)
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_noinline __attribute__((noinline))
	#else
		#error No default for xht_noinline
	#endif
#endif

#if !defined(xht_likely)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_likely(x) (x)
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_likely(x) __builtin_expect(x, 1)
	#else
		#error No default for xht_likely
	#endif
#endif
	
#if !defined(xht_unlikely)
	#if defined(XHT_COMPILER_MSVC)
		#define xht_unlikely(x) (x)
	#elif defined(XHT_COMPILER_CLANG) || defined(XHT_COMPILER_GCC)
		#define xht_unlikely(x) __builtin_expect(x, 0)
	#else
		#error No default for xht_unlikely
	#endif
#endif

#if !defined(xht_assert)
	#define xht_assert(c) assert(c)
#endif

#if !defined(XHT_PUBLIC)
	#if defined(XHT_SHARED)
		#if defined(XHT_BUILD)
			#if defined(XHT_COMPILER_MSVC)
				#define XHT_PUBLIC __declspec(dllexport)
			#else
				#error Shared export decorator is not implemented for the compiler
			#endif
		#else
			#if defined(XHT_COMPILER_MSVC)
				#define XHT_PUBLIC __declspec(dllimport)
			#else
				#error Shared import decorator is not implemented for the compiler
			#endif
		#endif
	#else
		#define XHT_PUBLIC
	#endif
#endif

// #### END feature.hpp #### 
// #### BEGIN core.hpp #### 

#include <cstddef>
#include <cstdint>
#include <utility>

namespace xht {

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using uword = std::uintptr_t;

using i8 = std::uint8_t;
using i16 = std::uint16_t;
using i32 = std::uint32_t;
using i64 = std::uint64_t;
using iword = std::intptr_t;

}
// #### END core.hpp #### 
// #### BEGIN intrin.hpp #### 


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


// #### END intrin.hpp #### 
// #### BEGIN traits.hpp #### 

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
// #### END traits.hpp #### 
// #### BEGIN intlog.hpp #### 


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
// #### END intlog.hpp #### 
// #### BEGIN compare.hpp #### 

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
// #### END compare.hpp #### 
// #### BEGIN key.hpp #### 


namespace xht {

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

// #### END key.hpp #### 
// #### BEGIN simplify.hpp #### 

namespace xht {

template<typename TKey>
xht_forceinline const TKey& SimplifyKeyExt(const TKey& key) {
	return key;
}

template<typename TKey>
xht_forceinline const TKey& SimplifyKey(const TKey& key) {
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
		if constexpr (std::is_same_v<TKey, TInKey>)
		{
			return inKey;
		}
		else
		{
			decltype(auto) key = xht::SimplifyKey(inKey);
			using SimpleKeyType = std::decay_t<decltype(key)>;

			if constexpr (std::is_same_v<SimpleKeyType, TKey>)
			{
				return key;
			}
			else
			{
				SimplifyVerify<TKey, TInKey, SimpleKeyType>();
				return static_cast<TKey>(key);
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
// #### END simplify.hpp #### 
// #### BEGIN hashtable.hpp #### 

#include <cstring>
#include <memory>
#include <algorithm>



namespace xht::impl::hashtable {

	enum Ctrl : i8
	{
		Ctrl_Empty = (i8)0b10000000,
		Ctrl_Tomb = (i8)0b11111110,
		Ctrl_End = (i8)0b11111111,
	};

	constexpr iword GroupSize = 16;
	extern const Ctrl EmptyGroup[];

	//ARCH: SSE2
	struct Group
	{
		xht_forceinline explicit Group(const Ctrl* ctrl)
		{
			m_ctrl = _mm_loadu_si128((const __m128i*)ctrl);
		}

		xht_forceinline u32 Match(Ctrl h2) const
		{
			return _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_set1_epi8(h2), m_ctrl));
		}

		xht_forceinline u32 MatchEmpty() const
		{
			//ARCH: SSSE3
#if XHT_SSSE3
			return _mm_movemask_epi8(_mm_sign_epi8(m_ctrl, m_ctrl));
#else
			return Match(Ctrl_Empty);
#endif
		}

		xht_forceinline u32 MatchFree() const
		{
			return _mm_movemask_epi8(_mm_cmpgt_epi8(
				_mm_set1_epi8(Ctrl_End), m_ctrl));
		}

		xht_forceinline u32 CountLeadingFree() const
		{
			__m128i end = _mm_set1_epi8(Ctrl_End);
			u32 mask = _mm_movemask_epi8(_mm_cmpgt_epi8(end, m_ctrl));
			return mask == 0 ? GroupSize : xht_ctz32(mask + 1);
		}

		__m128i m_ctrl;
	};

	struct Probe
	{
		xht_forceinline Probe(uword hash, uword mask)
		{
			m_mask = mask;
			m_offset = hash & mask;
			m_index = 0;
		}

		xht_forceinline iword Offset(iword offset)
		{
			return (m_offset + offset) & m_mask;
		}

		xht_forceinline void Next()
		{
			m_index += GroupSize;
			m_offset += m_index;
			m_offset &= m_mask;
		}

		uword m_mask;
		uword m_offset;
		uword m_index;
	};

#define XHT_HT_HASH1(hash) ((hash) >> 7)
#define XHT_HT_HASH2(hash) ((Ctrl)((hash) & 0x7F))

#define XHT_HT_SLOTS(ctrl, capacity) ((u8*)((ctrl) + (capacity) + 1 + GroupSize))


	struct IteratorCore
	{
		Ctrl* m_ctrl;
		u8* m_slot;

		template<uword TElementSize>
		void SkipFreeSlots()
		{
			Ctrl* ctrl = m_ctrl;
			u8* slot = m_slot;

			while (*ctrl < Ctrl_End)
			{
				u32 shift = Group(ctrl).CountLeadingFree();

				ctrl += shift;
				slot += shift * TElementSize;
			}

			m_ctrl = ctrl;
			m_slot = slot;
		}

		template<uword TElementSize>
		void Advance()
		{
			m_ctrl += 1;
			m_slot += TElementSize;
			SkipFreeSlots<TElementSize>();
		}

		template<uword TElementSize>
		static IteratorCore Begin(Ctrl* ctrl, iword capacity)
		{
			IteratorCore iterator;
			iterator.m_ctrl = ctrl;
			iterator.m_slot = XHT_HT_SLOTS(ctrl, capacity);
			iterator.SkipFreeSlots<TElementSize>();
			return iterator;
		}

		static IteratorCore End(Ctrl* ctrl, iword capacity)
		{
			IteratorCore iterator;
			iterator.m_ctrl = ctrl + capacity;
			return iterator;
		}
	};

	template<typename T>
	class BasicIterator : IteratorCore
	{
	public:
		xht_forceinline BasicIterator(IteratorCore iterator)
			: IteratorCore(iterator)
		{
		}

		xht_forceinline T& operator*() const
		{
			return *(T*)m_slot;
		}

		xht_forceinline T* operator->() const
		{
			return (T*)m_slot;
		}

		xht_forceinline BasicIterator& operator++()
		{
			Advance<sizeof(T)>();
			return *this;
		}

		xht_forceinline BasicIterator operator++(int)
		{
			auto it = *this;
			Advance<sizeof(T)>();
			return it;
		}

		xht_forceinline bool operator==(const BasicIterator& other) const
		{
			return m_ctrl == other.m_ctrl;
		}

		xht_forceinline bool operator!=(const BasicIterator& other) const
		{
			return m_ctrl != other.m_ctrl;
		}
	};


	struct Core
	{
		Ctrl* ctrl;
		iword size;
		iword free;
		iword capacity;
	};

	constexpr xht_forceinline uword GetBufferSize(iword capacity, uword elemSize)
	{
		return (uword)(capacity + 1 + GroupSize) + elemSize * (uword)capacity;
	}

	constexpr xht_forceinline iword GetMaxSize(iword capacity)
	{
		return capacity - (iword)((uword)capacity / 8);
	}

	template<bool THasLocalStorage>
	xht_forceinline bool IsDynamic(Core* table, Ctrl* ctrl)
	{
		if constexpr (THasLocalStorage)
		{
			return ctrl != (Ctrl*)(table + 1);
		}
		else
		{
			return ctrl != EmptyGroup;
		}
	}

	xht_forceinline void InitCtrl(Ctrl* ctrl, iword capacity) {
		uword ctrlSize = capacity + 1 + GroupSize;
		xht_assert(ctrlSize % GroupSize == 0);
		memset(ctrl, Ctrl_Empty, ctrlSize);
		ctrl[capacity] = Ctrl_End;
	}

	template<bool THasLocalStorage>
	void ConstructTable(Core* table, iword localCapacity)
	{
		if constexpr (THasLocalStorage)
		{
			Ctrl* ctrl = (Ctrl*)(table + 1);
			InitCtrl(ctrl, localCapacity);

			table->ctrl = ctrl;
			table->size = 0;
			table->free = GetMaxSize(localCapacity);
			table->capacity = localCapacity;
		}
		else
		{
			table->ctrl = (Ctrl*)EmptyGroup;
			table->size = 0;
			table->free = 0;
			table->capacity = 0;
		}
	}

	template<typename TDestroy, typename TAlloc, bool THasLocalStorage>
	void DestroyTable(Core* table, uword elemSize, TAlloc alloc)
	{
		Ctrl* ctrl = table->ctrl;
		iword capacity = table->capacity;

		if constexpr (!std::is_trivially_destructible_v<TDestroy>)
		{
			if (table->size > 0)
			{
				u8* slot = XHT_HT_SLOTS(ctrl, capacity);
				for (iword slotIndex = 0; slotIndex < capacity;
					++slotIndex, slot += elemSize)
				{
					if (ctrl[slotIndex] >= 0)
						std::destroy_at((TDestroy*)slot);
				}
			}
		}

		if (IsDynamic<THasLocalStorage>(table, ctrl))
		{
			alloc.free((u8*)ctrl, GetBufferSize(capacity, elemSize));
		}
	}

	struct KeyTraits
	{
		template<typename TKey, typename T>
		static xht_forceinline const TKey* GetKey(const T* slot)
		{
			return (const TKey*)slot;
		}
	};

	xht_forceinline void SetCtrl(Ctrl* ctrl,
		iword capacity, iword slotIndex, Ctrl h2)
	{
		ctrl[slotIndex] = h2;
		ctrl[(slotIndex - GroupSize & capacity) + GroupSize] = h2;
	}

	inline iword FindFreeSlot(const Core* core, uword hash)
	{
		Ctrl* ctrl = core->ctrl;
		iword capacity = core->capacity;

		//TODO: move args ops in?
		Probe probe(XHT_HT_HASH1(hash), (uword)capacity);

		while (true)
		{
			Group group(ctrl + probe.m_offset);

			if (u32 mask = group.MatchFree())
				return probe.Offset(xht_ctz32(mask));

			xht_assert(probe.m_index < (uword)capacity);
			probe.Next();
		}
	}

	template<typename T>
	struct InsertResult
	{
		T* Element;
		bool Inserted;
	};

	typedef InsertResult<void> InsertCallback(
		Core* core, uword elemSize, uword hash, const void* context);

	//typedef FunctionRef<InsertCallbackSignature> InsertCallback2;

	XHT_PUBLIC
		InsertResult<void> FailCallback(Core* core,
			uword elemSize, uword hash, const void* context);

	XHT_PUBLIC
		InsertResult<void> InsertSlot(Core* table,
			uword elemSize, uword hash, iword slotIndex);

	XHT_PUBLIC
		InsertResult<void> InsertWithCallback(
			Core* table, uword elemSize, uword hash,
			InsertCallback insertCallback, const void* insertContext);

	XHT_PUBLIC
		void RemoveSlot(Core* table, iword slotIndex);

	XHT_PUBLIC
		void ConvertTombToEmptyAndFullToTomb(Ctrl* ctrl, iword capacity);

	xht_forceinline iword GetProbeIndex(
		iword slotIndex, uword hash, iword capacity)
	{
		uword offset = (uword)slotIndex - XHT_HT_HASH1(hash) & (uword)capacity;
		return (iword)(offset / (uword)GroupSize);
	}

	template<typename TKey, typename TKeyTraits, typename TSimplify, typename TCompare, typename TInKey>
	xht_noinline
		void* Find(const Core* table, uword elemSize, uword hash, TInKey key)
	{
		Ctrl* ctrl = table->ctrl;
		iword capacity = table->capacity;
		u8* slots = XHT_HT_SLOTS(ctrl, capacity);

		Probe probe(XHT_HT_HASH1(hash), (uword)capacity);

		for (Ctrl h2 = XHT_HT_HASH2(hash);;)
		{
			Group group(ctrl + probe.m_offset);

			for (u32 mask = group.Match(h2); mask != 0; mask &= mask - 1)
			{
				u8* slot = slots + elemSize * probe.Offset(xht_ctz32(mask));

				if (xht_likely(TCompare::Compare(key,
					TSimplify::template SimplifyKey<typename TSimplify::template SimpleKey<TKey>>(
						*TKeyTraits::template GetKey<TKey>(slot)))))
				{
					return slot;
				}
			}

			if (xht_likely(group.MatchEmpty()))
				return nullptr;

			probe.Next();
		}
	}

	template<typename TKey, typename TKeyTraits, typename TSimplify, typename THash>
	static void Rehash(Core* dst, Core* src, uword elemSize)
	{
		Ctrl* dstCtrl = dst->ctrl;
		iword dstCapacity = dst->capacity;
		u8* dstSlots = XHT_HT_SLOTS(dstCtrl, dstCapacity);

		Ctrl* srcCtrl = src->ctrl;
		iword srcCapacity = src->capacity;

		u8* srcSlot = XHT_HT_SLOTS(srcCtrl, srcCapacity);
		for (iword srcSlotIndex = 0; srcSlotIndex < srcCapacity;
			++srcSlotIndex, srcSlot += elemSize)
		{
			if (srcCtrl[srcSlotIndex] >= 0)
			{
				uword hash = THash::Hash(
					TSimplify::template SimplifyKey<typename TSimplify::template SimpleKey<TKey>>(
						*TKeyTraits::template GetKey<TKey>(srcSlot)));

				iword dstSlotIndex = FindFreeSlot(dst, hash);
				SetCtrl(dstCtrl, dstCapacity, dstSlotIndex, XHT_HT_HASH2(hash));

				u8* dstSlot = dstSlots + elemSize * dstSlotIndex;
				memcpy(dstSlot, srcSlot, elemSize);
			}
		}
	}

	template<typename TKey, typename TKeyTraits,
		typename TSimplify, typename THash, typename TAllocatorHandle, bool THasLocalStorage>
		static void Resize(Core* table, uword elemSize, iword newCapacity, TAllocatorHandle allocator)
	{
		xht_assert(newCapacity > 0 && (newCapacity & newCapacity + 1) == 0);
		xht_assert(GetMaxSize(newCapacity) >= table->size);

		Ctrl* ctrl = (Ctrl*)allocator.allocate(GetBufferSize(newCapacity, elemSize));

		InitCtrl(ctrl, newCapacity);

		iword size = table->size;

		Core newTable;
		newTable.ctrl = ctrl;
		newTable.size = size;
		newTable.free = GetMaxSize(newCapacity) - size;
		newTable.capacity = newCapacity;

		if (size > 0)
		{
			Rehash<TKey, TKeyTraits, TSimplify, THash>(&newTable, table, elemSize);
		}

		ctrl = table->ctrl;
		if (IsDynamic<THasLocalStorage>(table, ctrl))
		{
			allocator.free((u8*)ctrl, GetBufferSize(table->capacity, elemSize));
		}

		*table = newTable;
	}

	template<typename TKey, typename TKeyTraits, typename TSimplify, typename THash>
	static void ReuseTombs(Core* table, uword elemSize)
	{
		Ctrl* ctrl = table->ctrl;
		iword capacity = table->capacity;

		ConvertTombToEmptyAndFullToTomb(ctrl, capacity);

		u8* slots = XHT_HT_SLOTS(ctrl, capacity);

		u8* slot = slots;
		for (iword slotIndex = 0; slotIndex < capacity; ++slotIndex, slot += elemSize)
		{
			if (ctrl[slotIndex] != Ctrl_Tomb)
				continue;

			uword hash = THash::Hash(
				TSimplify::template SimplifyKey<typename TSimplify::template SimpleKey<TKey>>(
					*TKeyTraits::template GetKey<TKey>(slot)));

			iword newSlotIndex = FindFreeSlot(table, hash);

			iword probeIndex = GetProbeIndex(slotIndex, hash, capacity);
			iword newProbeIndex = GetProbeIndex(newSlotIndex, hash, capacity);

			if (xht_likely(newProbeIndex == probeIndex))
			{
				SetCtrl(ctrl, capacity, slotIndex, XHT_HT_HASH2(hash));
				continue;
			}

			u8* newSlot = slots + elemSize * newSlotIndex;
			if (ctrl[newSlotIndex] == Ctrl_Empty)
			{
				SetCtrl(ctrl, capacity, newSlotIndex, XHT_HT_HASH2(hash));
				memcpy(newSlot, slot, elemSize);
				SetCtrl(ctrl, capacity, slotIndex, Ctrl_Empty);
			}
			else
			{
				xht_assert(ctrl[newSlotIndex] == Ctrl_Tomb);
				SetCtrl(ctrl, capacity, newSlotIndex, XHT_HT_HASH2(hash));

				u8 temp[256];
				memcpy(temp, newSlot, elemSize);
				memcpy(newSlot, slot, elemSize);
				memcpy(slot, temp, elemSize);

				--slotIndex;
			}
		}
	}

	template<typename TKey, typename TKeyTraits,
		typename TSimplify, typename THash, typename TAllocatorHandle, bool THasLocalStorage>
		static InsertResult<void> GrowCallback(Core* table,
			uword elemSize, uword hash, const void* allocator)
	{
		iword capacity = table->capacity;
		if (capacity == 0)
		{
			Resize<TKey, TKeyTraits, TSimplify, THash, TAllocatorHandle, THasLocalStorage>(
				table, elemSize, GroupSize - 1, *(const TAllocatorHandle*)allocator);
		}
		else if (table->size <= GetMaxSize(capacity) / 2)
		{
			ReuseTombs<TKey, TKeyTraits, TSimplify, THash>(table, elemSize);
		}
		else
		{
			Resize<TKey, TKeyTraits, TSimplify, THash, TAllocatorHandle, THasLocalStorage>(
				table, elemSize, capacity * 2 + 1, *(const TAllocatorHandle*)allocator);
		}

		--table->free;

		iword slotIndex = FindFreeSlot(table, hash);
		return InsertSlot(table, elemSize, hash, slotIndex);
	}

	template<typename TKey, typename TKeyTraits, typename TSimplify, typename TCompare, typename TInKey>
	static xht_noinline InsertResult<void> Insert(
		Core* table, uword elemSize, uword hash, TInKey key,
		InsertCallback insertCallback, const void* insertContext)
	{
		Ctrl* ctrl = table->ctrl;
		iword capacity = table->capacity;
		u8* slots = XHT_HT_SLOTS(ctrl, capacity);

		Probe probe(XHT_HT_HASH1(hash), (uword)capacity);

		for (Ctrl h2 = XHT_HT_HASH2(hash);;)
		{
			Group group(ctrl + probe.m_offset);

			for (u32 mask = group.Match(h2); mask != 0; mask &= mask - 1)
			{
				u8* slot = slots + elemSize *
					probe.Offset(xht_ctz32(mask));

				if (xht_likely(TCompare::Compare(key,
					TSimplify::template SimplifyKey<typename TSimplify::template SimpleKey<TKey>>(
						*TKeyTraits::template GetKey<TKey>(slot)))))
				{
					return { slot, false };
				}
			}

			if (xht_likely(group.MatchEmpty()))
			{
				return InsertWithCallback(table,
					elemSize, hash, insertCallback, insertContext);
			}

			probe.Next();
		}
	}

	//TODO: make sure the elem is destroyed on the outside...
	template<typename TKey, typename TKeyTraits, typename TCompare, typename TInKey>
	static void* Remove(Core* table, uword elemSize, uword hash, TInKey key)
	{
		void* slot = Find<
			TKey, TKeyTraits, TCompare, TInKey>(
				table, elemSize, hash, key);

		if (slot != nullptr)
		{
			//TODO: do something cleverer than integer division
			RemoveSlot(table, ((u8*)slot - XHT_HT_SLOTS(
				table->ctrl, table->capacity)) / elemSize);
		}

		return slot;
	}

	template<typename TDestroy>
	void Clear(Core* table, uword elemSize)
	{
		iword capacity = table->capacity;
		iword maxSize = GetMaxSize(capacity);

		if (table->free < maxSize)
		{
			Ctrl* ctrl = table->ctrl;
			if constexpr (!std::is_trivially_destructible_v<TDestroy>)
			{
				if (table->size > 0)
				{
					u8* slot = XHT_HT_SLOTS(ctrl, capacity);
					for (iword slotIndex = 0; slotIndex < capacity;
						++slotIndex, slot += elemSize)
					{
						if (ctrl[slotIndex] >= 0)
							Destroy((TDestroy*)slot);
					}
				}
			}
			InitCtrl(ctrl, capacity);

			table->size = 0;
			table->free = maxSize;
		}
	}



	template<typename TKey, typename TKeyTraits,
		typename THash, typename TAllocatorHandle, bool THasLocalStorage>
		static void Reserve(Core* table, uword elemSize, iword minCapacity, TAllocatorHandle allocator)
	{
		if (table->capacity < minCapacity) {
			uword po2 = 1;
			uword target_cap = minCapacity + 1;
			while (po2 < target_cap) {
				po2 = po2 << (uword)1;
			}
			Resize<TKey, TKeyTraits, THash, TAllocatorHandle, THasLocalStorage>(
				table, elemSize, (iword)po2 - 1, allocator);
		}
	}

	template<typename TCore, typename T, iword TCapacity,
		bool = (TCapacity > 0) && (alignof(T) > alignof(TCore))>
		struct StorageLayer;

	template<typename TCore, typename T>
	struct StorageLayer<TCore, T, 0, 0>
	{
		TCore core;
	};

	template<typename TCore, typename T, iword TCapacity>
	struct StorageLayer<TCore, T, TCapacity, 0>
	{
		TCore core;
		std::aligned_storage_t<
			GetBufferSize(TCapacity, sizeof(T)),
			alignof(T)
		> mutable storage;
	};

	template<typename TCore, typename T, iword TCapacity>
	struct StorageLayer<TCore, T, TCapacity, 1>
	{
		std::aligned_storage_t<
			alignof(T) - alignof(TCore),
			1
		> padding;

		TCore core;
		std::aligned_storage_t<
			GetBufferSize(TCapacity, sizeof(T)),
			alignof(T)
		> mutable storage;
	};

	template<typename TStorage, typename TAllocatorHandle>
	struct AllocatorLayer : TAllocatorHandle, TStorage
	{
		AllocatorLayer() = default;

		xht_forceinline AllocatorLayer(const TAllocatorHandle& allocator)
			: TAllocatorHandle(allocator)
		{
		}
	};

	template<uword TSize>
	struct TrivialKey
	{
		//TODO: compute minimum alignment
		std::aligned_storage_t<TSize, 1> m_buffer;

		xht_forceinline bool operator==(const TrivialKey& other)
		{
			return memcmp(m_buffer, other.m_buffer, TSize) == 0;
		}
	};

	template<typename TCompare, typename TKey>
	using CompareType = meta::Select<
		std::is_same_v<TCompare, BasicComparator> &&
		meta::HasUniqueRepresentation<TKey>, TrivialKey<sizeof(TKey)>, TKey>;

	template<
		typename T,
		typename TKey,
		typename TKeyTraits,
		typename TSimplify,
		typename THash,
		typename TCompare,
		typename TAllocatorHandle,
		iword TCapacity
	>
	struct HashTable
	{
		static_assert(xht::meta::IsTriviallyRelocatable<T>);

		static_assert(sizeof(T) <= 256, "See ReuseTombs implementation");

		static constexpr iword LocalCapacity =
			TCapacity > 0
			?
			(iword)comptime::round_up_to_pot((uword)std::max(TCapacity, GroupSize)) - 1
			:
			0
			;

		static_assert(TCapacity == 0 || LocalCapacity + 1 >= alignof(T));

	public:
		typedef BasicIterator<      T> Iterator;
		typedef BasicIterator<const T> ConstIterator;


		HashTable()
		{
			ConstructTable<(TCapacity > 0)>(&m.core, { LocalCapacity });
		}

		HashTable(TAllocatorHandle allocator)
			: m(std::move(allocator))
		{
			ConstructTable<(TCapacity > 0)>(&m.core, { LocalCapacity });
		}

		HashTable(HashTable&& src);
		HashTable& operator=(HashTable&& src);

		HashTable(const HashTable&) = delete;
		HashTable& operator=(const HashTable&) = delete;

		~HashTable()
		{
			DestroyTable<meta::DestroyType<T>, TAllocatorHandle, (TCapacity > 0)>(
				&m.core, sizeof(T), static_cast<const TAllocatorHandle&>(m));
		}


		xht_forceinline iword Size() const
		{
			return m.core.size;
		}

		xht_forceinline iword Capacity() const
		{
			return m.core.capacity;
		}

		xht_forceinline Iterator begin()
		{
			return IteratorCore::Begin<sizeof(T)>(m.core.ctrl, m.core.capacity);
		}

		xht_forceinline ConstIterator begin() const
		{
			return IteratorCore::Begin<sizeof(T)>(m.core.ctrl, m.core.capacity);
		}

		xht_forceinline Iterator end()
		{
			return IteratorCore::End(m.core.ctrl, m.core.capacity);
		}

		xht_forceinline ConstIterator end() const
		{
			return IteratorCore::End(m.core.ctrl, m.core.capacity);
		}

	protected:
		template<typename TInKey>
		xht_forceinline T* Find(const TInKey& inKey) const
		{
			//TODO: collapse TCompare=DefaultCompare and TKey has unique repr to
			// dummy layout type

			decltype(auto) key = TSimplify::template SimplifyKey<typename TSimplify::template SimpleKey<TKey>>(inKey);
			using KeyParamType = decltype(key);

			return (T*)impl::hashtable::Find<
				TKey, TKeyTraits, TSimplify, TCompare, KeyParamType>(
					&m.core, sizeof(T), THash::Hash(key), key);
		}

		template<typename TInKey>
		xht_forceinline InsertResult<T> Insert(const TInKey& inKey,
			InsertCallback insertCallback, const void* insertContext)
		{
			decltype(auto) key = TSimplify::template SimplifyKey<typename TSimplify::template SimpleKey<TKey>>(inKey);
			using KeyParamType = decltype(key);

			auto result = impl::hashtable::Insert<TKey, TKeyTraits, TSimplify, TCompare, KeyParamType>(
				&m.core, sizeof(T), THash::Hash(key), key, insertCallback, insertContext);

			return InsertResult<T>{ (T*)result.Element, result.Inserted };
		}

		template<typename TInKey>
		xht_forceinline T* Remove(const TInKey& inKey)
		{
			decltype(auto) key = TSimplify::template SimplifyKey<typename TSimplify::template SimpleKey<TKey>>(inKey);
			using KeyParamType = decltype(key);

			return (T*)impl::hashtable::Remove<
				TKey, TKeyTraits, TCompare, KeyParamType>(
					&m.core, sizeof(T), THash::Hash(key), key);
		}


		AllocatorLayer<StorageLayer<Core, T, LocalCapacity>, TAllocatorHandle> m;
	};

}
// #### END hashtable.hpp #### 
// #### BEGIN fnv1ahash.hpp #### 


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
// #### END fnv1ahash.hpp #### 
// #### BEGIN hash.hpp #### 

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

using DefaultHash = FNV1aHash;

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
// #### END hash.hpp #### 
// #### BEGIN hashmap.hpp #### 


#include <utility>

namespace xht {

	using KeyTraits = xht::impl::hashtable::KeyTraits;


	template<
		typename TKey,
		typename TValue,
		typename TSimplifier,
		typename THash,
		typename TCompare,
		typename TAllocatorHandle,
		iword TCapacity
	>
		class HashMap : public xht::impl::hashtable::HashTable<
			KeyValuePair<TKey, TValue>,
			TKey,
			KeyTraits,
			TSimplifier,
			THash,
			TCompare,
			TAllocatorHandle,
			TCapacity
		>
	{
		using B = xht::impl::hashtable::HashTable<
			KeyValuePair<TKey, TValue>,
			TKey,
			KeyTraits,
			TSimplifier,
			THash,
			TCompare,
			TAllocatorHandle,
			TCapacity
		>;

	public:
		using ElementType = KeyValuePair<TKey, TValue>;

		using KeyType = TKey;
		using ValueType = TValue;

		using InsertResult = xht::impl::hashtable::InsertResult<ElementType>;

		using B::B;

		template<typename TInKey>
		xht_forceinline bool Contains(const TInKey& key) const
		{
			return B::Find(key) != nullptr;
		}

		template<typename TInKey>
		xht_forceinline ElementType* Find(const TInKey& key)
		{
			return B::Find(key);
		}

		template<typename TInKey>
		xht_forceinline const ElementType* Find(const TInKey& key) const
		{
			return B::Find(key);
		}

		template<typename TInKey>
		xht_forceinline TValue* FindValue(const TInKey& key)
		{
			if (ElementType* element = B::Find(key))
				return &element->Value;
			return nullptr;
		}

		template<typename TInKey>
		xht_forceinline const TValue* FindValue(const TInKey& key) const
		{
			if (ElementType* element = B::Find(key))
				return &element->Value;
			return nullptr;
		}

		template<typename TInKey>
		xht_forceinline TValue FindDef(const TInKey& key) const
		{
			if (ElementType* element = B::Find(key))
				return element->Value;
			return TValue{};
		}

		template<typename TInKey>
		xht_forceinline TValue& operator[](const TInKey& key)
		{
			ElementType* element = B::Find(key);
			xht_assert(element != nullptr);
			return element->Value;
		}

		template<typename TInKey>
		xht_forceinline const TValue& operator[](const TInKey& key) const
		{
			ElementType* element = B::Find(key);
			xht_assert(element != nullptr);
			return element->Value;
		}


		template<typename TInKey>
		xht_forceinline InsertResult Insert(TInKey&& key)
		{
			InsertResult result = B::Insert(key, &impl::hashtable::GrowCallback<
				TKey, KeyTraits, THash, TAllocatorHandle, (TCapacity > 0)>,
				& static_cast<const TAllocatorHandle&>(B::m));

			if (result.Inserted)
			{
				new (result.Element) ElementType(std::forward<TInKey>(key));
			}
			return result;
		}

		template<typename TInKey, typename TInValue>
		xht_forceinline InsertResult Insert(TInKey&& key, TInValue&& value)
		{
			InsertResult result = B::Insert(key, &impl::hashtable::GrowCallback<
				TKey, KeyTraits, TSimplifier, THash, TAllocatorHandle, (TCapacity > 0)>,
				& static_cast<const TAllocatorHandle&>(B::m));

			if (result.Inserted)
			{
				new (result.Element) ElementType(std::forward<TInKey>(key), std::forward<TInValue>(value));
			}
			return result;
		}

		template<typename TInKey, typename TInValue>
		xht_forceinline InsertResult Set(TInKey&& key, TInValue&& value)
		{
			InsertResult result = B::Insert(key, &impl::hashtable::GrowCallback<
				TKey, KeyTraits, THash, TAllocatorHandle, (TCapacity > 0)>,
				& static_cast<const TAllocatorHandle&>(B::m));

			if (result.Inserted)
			{
				new (result.Element) ElementType(std::forward<TInKey>(key), std::forward<TInValue>(value));
			}
			else
			{
				result.Element->Value = std::forward<TInValue>(value);
			}
			return result;
		}


		template<typename TInKey>
		xht_forceinline bool Remove(const TInKey& key)
		{
			ElementType* element = B::Remove(key);
			if (element != nullptr)
			{
				std::destroy_at(element);
				return true;
			}
			return false;
		}

		template<typename TInKey>
		xht_forceinline TValue Extract(const TInKey& key) {
			ElementType* element = B::Remove(key);
			xht_assert(element);
			TValue value = element->Value;
			std::destroy_at(element);
			return value;
		}

		xht_forceinline void Clear()
		{
			impl::hashtable::Clear<meta::DestroyType<ElementType>>(&B::m.core, sizeof(ElementType));
		}

		xht_forceinline void Reserve(iword minCapacity)
		{
			impl::hashtable::Reserve<TKey, KeyTraits,
				THash, TAllocatorHandle, (TCapacity > 0)>(
					&B::m.core, sizeof(ElementType), minCapacity,
					static_cast<const TAllocatorHandle&>(B::m));
		}
	};


	struct Mallocator {
		u8* allocate(uword size)
		{
			return (u8*)::malloc(size);
		}

		void free(u8* ptr, uword size)
		{
			::free(ptr);
		}
	};

	template<
		class K,
		class V,
		class TSimplifier = DefaultSimplifier<K>,
		class THash = DefaultHasher<K>,
		class TCompare = DefaultComparator<K>
	>
	using StdHashMap = HashMap<K, V, TSimplifier, THash, TCompare, Mallocator, 16>;
}

// #### END hashmap.hpp #### 
// ##### BEGIN IMPLEMENTATION #### 
#if defined(XHT_INCLUDE_IMPLEMENTATION)
// #### BEGIN hashtable.cpp #### 

using namespace xht;

namespace xht::impl::hashtable {

	const Ctrl EmptyGroup[] = {
		Ctrl_End,   Ctrl_Empty, Ctrl_Empty, Ctrl_Empty,
		Ctrl_Empty, Ctrl_Empty, Ctrl_Empty, Ctrl_Empty,
		Ctrl_Empty, Ctrl_Empty, Ctrl_Empty, Ctrl_Empty,
		Ctrl_Empty, Ctrl_Empty, Ctrl_Empty, Ctrl_Empty,
	};

	void RemoveSlot(Core* table, iword slotIndex) {
		Ctrl* ctrl = table->ctrl;

		iword leftIndex = (slotIndex - GroupSize) & (uword)table->capacity;

		u32 leftMask = Group(ctrl + leftIndex).MatchEmpty();
		u32 slotMask = Group(ctrl + slotIndex).MatchEmpty();

		bool reuse = leftMask && slotMask &&
			(xht_ctz32(leftMask) + xht_clz32(slotMask)) < GroupSize;

		SetCtrl(ctrl, table->capacity, slotIndex, reuse ? Ctrl_Empty : Ctrl_Tomb);
		table->free += reuse ? 1 : 0;
		--table->size;
	}

	InsertResult<void> FailCallback(
		Core* core, uword elemSize, uword hash, const void* context) {
		return { nullptr, false };
	}

	InsertResult<void> InsertWithCallback(
		Core* table, uword elemSize, uword hash,
		InsertCallback insertCallback, const void* insertContext) {
		iword slotIndex = FindFreeSlot(table, hash);

		Ctrl* ctrl = table->ctrl;
		iword free = table->free;
		Ctrl slotCtrl = ctrl[slotIndex];

		if (xht_unlikely(free == 0 && slotCtrl != Ctrl_Tomb))
			return insertCallback(table, elemSize, hash, insertContext);

		table->free = free - (slotCtrl == Ctrl_Empty ? 1 : 0);
		return InsertSlot(table, elemSize, hash, slotIndex);
	}

	InsertResult<void> InsertSlot(
		Core* table, uword elemSize, uword hash, iword slotIndex
	) {
		++table->size;
		Ctrl* ctrl = table->ctrl;
		iword capacity = table->capacity;

		SetCtrl(ctrl, capacity, slotIndex, XHT_HT_HASH2(hash));

		uword slotOffset = (uword)slotIndex * elemSize;
		return { XHT_HT_SLOTS(ctrl, capacity) + slotOffset, true };
	}

	static void ConvertSpecialToEmptyAndFullToTomb(Ctrl* group) {
		__m128i ctrl = _mm_loadu_si128((const __m128i*)group);

		__m128i msb1 = _mm_set1_epi8(0b10000000u);
		__m128i lsb0 = _mm_set1_epi8(0b11111110u);

#if XHT_SSSE3
		__m128i result = _mm_or_si128(_mm_shuffle_epi8(lsb0, ctrl), msb1);
#else
		__m128i zero = _mm_setzero_si128();
		__m128i mask = _mm_cmpgt_epi8(zero, ctrl);
		__m128i result = _mm_or_si128(msb1, _mm_andnot_si128(mask, lsb0));
#endif

		_mm_storeu_si128((__m128i*)group, result);
	}

	void ConvertTombToEmptyAndFullToTomb(Ctrl* ctrl, iword capacity) {
		xht_assert(ctrl[capacity] == Ctrl_End);

		Ctrl* end = ctrl + capacity + 1;
		for (Ctrl* group = ctrl; group != end; group += GroupSize)
			ConvertSpecialToEmptyAndFullToTomb(group);

		memcpy(end, ctrl, GroupSize);
		ctrl[capacity] = Ctrl_End;
	}


}
// #### END hashtable.cpp #### 
#endif
// ##### END IMPLEMENTATION #### 
#endif
