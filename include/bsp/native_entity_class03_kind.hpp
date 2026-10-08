#pragma once

#include <cstdint>

namespace bsp {

// COMPLETE raw class03 vtable+5Ch leaf: [00888EA0,00888EC7), 39B/13 instructions.
// ECX actual receiver, unused EDX, stacked DWORD, full EAX0/1, RET4. Ordered
// fixed words 03/01/00 precede ONE fresh DWORD at the SAME actual receiver+C4.
// Admit stable nonnull backing>=C8h with a genuinely live aligned uint32 cell
// exactly+C4h; this common domain covers the dynamic path. Fixed words do not
// read ECX. No added guard/default/cache/census or routing policy. Invalid,
// fault, concurrency and object-lifetime paths outside admission unvalidated.
// Numeric name retains the descriptive native class-name hypothesis. D11138
// remains UNCALLABLE Source profile DATA. Source field compatibility proves
// no original class ctor/group membership/virtual/lifetime/world/EH/game binding.

std::uint32_t __fastcall native_entity_class03_is_kind_00888ea0(const void*, void*, std::uint32_t);

} // namespace bsp
