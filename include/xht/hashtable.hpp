#pragma once

#include "config.hpp"
#include "feature.hpp"
#include <cstring>
#include <memory>
#include <algorithm>

#include "intrin.hpp"
#include "traits.hpp"
#include "intlog.hpp"
#include "traits.hpp"
#include "compare.hpp"
#include "key.hpp"


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
#if XHT_SSSE2
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
#else
	struct Group
	{
		xht_forceinline explicit Group(const Ctrl* ctrl)
		{
			memcpy(m_ctrl, ctrl, sizeof(m_ctrl));
		}

		xht_forceinline u32 Match(Ctrl h2) const
		{
			u32 mask = 0;
			for (uint8_t i = 0; i < sizeof(m_ctrl); i++) {
				mask |= ((h2 == m_ctrl[i]) << i);
			}
			return mask;
		}

		xht_forceinline u32 MatchEmpty() const
		{
			return Match(Ctrl_Empty);
		}

		xht_forceinline u32 MatchFree() const
		{
			u32 mask = 0;
			for (uint8_t i = 0; i < sizeof(m_ctrl); i++) {
				mask |= ((Ctrl_End > m_ctrl[i]) << i);
			}
			return mask;
		}

		xht_forceinline u32 CountLeadingFree() const
		{
			u32 mask = MatchFree();
			return mask == 0 ? GroupSize : xht_ctz32(mask + 1);
		}

		Ctrl m_ctrl[16];
	};
#endif

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
					TSimplify::template SimplifyKey<TKey>(
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
					TSimplify::template SimplifyKey<TKey>(
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
				TSimplify::template SimplifyKey<TKey>(
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
					TSimplify::template SimplifyKey<TKey>(
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

			decltype(auto) key = TSimplify::template SimplifyKey<TKey>(inKey);
			using KeyParamType = decltype(key);

			return (T*)impl::hashtable::Find<
				TKey, TKeyTraits, TSimplify, TCompare, KeyParamType>(
					&m.core, sizeof(T), THash::Hash(key), key);
		}

		template<typename TInKey>
		xht_forceinline InsertResult<T> Insert(const TInKey& inKey,
			InsertCallback insertCallback, const void* insertContext)
		{
			decltype(auto) key = TSimplify::template SimplifyKey<TKey>(inKey);
			using KeyParamType = decltype(key);

			auto result = impl::hashtable::Insert<TKey, TKeyTraits, TSimplify, TCompare, KeyParamType>(
				&m.core, sizeof(T), THash::Hash(key), key, insertCallback, insertContext);

			return InsertResult<T>{ (T*)result.Element, result.Inserted };
		}

		template<typename TInKey>
		xht_forceinline T* Remove(const TInKey& inKey)
		{
			decltype(auto) key = TSimplify::template SimplifyKey<TKey>(inKey);
			using KeyParamType = decltype(key);

			return (T*)impl::hashtable::Remove<
				TKey, TKeyTraits, TCompare, KeyParamType>(
					&m.core, sizeof(T), THash::Hash(key), key);
		}


		AllocatorLayer<StorageLayer<Core, T, LocalCapacity>, TAllocatorHandle> m;
	};

}
