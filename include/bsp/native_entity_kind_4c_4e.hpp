#pragma once

#include <cstdint>

namespace bsp {

// TWO DISTINCT complete raw leaves: [0042C010,0042C032) 34B/11 instructions,
// [004351E0,00435207) 39B/13. ECX actual receiver, unused EDX, stacked DWORD,
// full EAX0/1, RET4. Ordered fixed 4C/00 or 4E/4C/00 precedes ONE fresh
// DWORD at the SAME actual receiver+C4h. The caller supplies stable nonnull
// backing>=C8h and a genuinely live aligned uint32 cell exactly+C4h. This
// common admitted domain covers dynamic probes; fixed words do not read ECX.
// No added guard/default/cache/census/router or ancestor/body collapse.
// Invalid/fault/concurrency/object-lifetime paths outside admission unvalidated.
// Descriptive native class names remain hypotheses. Native profile bindings
// are UNCALLABLE Source DATA; field compatibility proves no class constructor,
// virtual/group-membership/lifetime/world/EH/game binding. Second4E 00435360
// is entirely outside this provider.

std::uint32_t __fastcall native_entity_is_kind_0042c010(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_entity_is_kind_004351e0(const void*, void*, std::uint32_t);

} // namespace bsp
