#pragma once

#include <cstdint>

namespace bsp::cc16_dyn_radix_entries {

// Required native heap bindings; no allocator implementation or fallback is
// supplied. Both callbacks receive the original 32-bit values.
struct Heap {
    void* (*operator_new_00bf55be)(std::uint32_t bytes);
    void (*free_00bf6989)(void* storage);
};

// Complete normal body 00C37180..00C37202 (RET 4 starts at 00C37200).
// Original ABI: ESI = manager, one stack object argument, callee pops 4 bytes;
// EBX/EDI/EBP/ESI preserved. EAX holds object on normal return, but a semantic
// return value is not established. This explicit C++ interface returns void.
//
// Raw Win32 layout: manager+04h = entry storage, +08h = uint32 count,
// +0Ch = uint32 capacity. Each entry is 8 bytes; only its +04h object pointer
// is written on append. The object's +20h uint16 receives the old count's low
// 16 bits. Other fields and the first entry dword are not interpreted.
//
// Retains wrapping count/byte arithmetic, capacity publication before the heap
// call, per-row null destination checks during copy, and free-before-storage
// publication. Valid writable native storage and usable heap callbacks are
// required. In particular the original does not guard a null object or recover
// from a null allocation. No native ABI adapter or runtime activation supplied.
// Descriptive names are hypotheses; see docs/CC16_DYN_RADIX_ENTRIES.md.
void append_object_00c37180(void* manager, void* object, const Heap& heap);

} // namespace bsp::cc16_dyn_radix_entries
