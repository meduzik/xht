#pragma once

#include "config.hpp"
#include "hashtable.hpp"
#include "traits.hpp"
#include "key.hpp"
#include "hash.hpp"
#include "compare.hpp"

#include <utility>

namespace xht {

	using KeyTraits = xht::impl::hashtable::KeyTraits;


	template<typename TKey, typename TValue, typename THash,
		typename TCompare, typename TAllocatorHandle, iword TCapacity>
		class HashMap : public xht::impl::hashtable::HashTable<KeyValuePair<TKey, TValue>,
		TKey, KeyTraits, THash, TCompare, TAllocatorHandle, TCapacity>
	{
		using B = xht::impl::hashtable::HashTable<
			KeyValuePair<TKey, TValue>,
			TKey,
			KeyTraits,
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
				TKey, KeyTraits, THash, TAllocatorHandle, (TCapacity > 0)>,
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
		u8* allocate(uword size) {
			return (u8*)::malloc(size);
		}

		void free(u8* ptr, uword size) {
			::free(ptr);
		}
	};

	template<
		class K,
		class V,
		class Hash = Hasher<K>,
		class Eq = DefaultCompare
	>
	using StdHashMap = HashMap<K, V, Hash, Eq, Mallocator, 16>;
}

