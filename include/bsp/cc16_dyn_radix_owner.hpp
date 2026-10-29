#pragma once

#include "bsp/cc16_dyn_radix_entries.hpp"

namespace bsp::cc16_dyn_radix_owner {

struct Heap {
    void* (*operator_new_00bf681b)(std::uint32_t bytes);
    cc16_dyn_radix_entries::Heap entries;
};

// Complete normal body 00C55D30..00C55D93, RET 0Ch at 00C55D91.
// Native ABI: ECX = manager (saved in ESI); stack arguments are a dword,
// a pointer to six dwords, and the low byte of a third stack slot. Returns
// the allocated object in EAX, preserving ESI/EDI and other callee-saved regs.
//
// Allocate 24h bytes, then write object+00h, byte +1Ch, and six sequential
// dwords at +04h..+18h. Compose 00C37180 with the actual manager/object and
// required heap providers; that helper writes the object's +20h low16 index.
// Padding stays untouched. A null object allocation still reaches the helper
// with null, just as the original does; no failure guard is supplied.
//
// Raw Win32 storage and usable providers required. Explicit C++ transport,
// not a native ABI adapter or an activated runtime path. Names and field roles
// are descriptive hypotheses. See docs/CC16_DYN_RADIX_OWNER.md.
void* create_object_00c55d30(void* manager, std::uint32_t object_word_00,
    const void* source_six_words, std::uint8_t flag_1c, const Heap& heap);

} // namespace bsp::cc16_dyn_radix_owner
