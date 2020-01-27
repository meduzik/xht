#include <xht/xht.hpp>
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
	#if XHT_SSE2
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
	#else
		constexpr i8 msb1 = 0b10000000u;
		constexpr i8 lsb0 = 0b11111110u;

		for (u8 i = 0; i < GroupSize; i++) {
			group[i] = (Ctrl)((0 > group[i]) ? msb1 : lsb0);
		}
	#endif
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
