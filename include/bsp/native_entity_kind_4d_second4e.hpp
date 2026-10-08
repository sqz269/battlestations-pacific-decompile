#pragma once

#include <cstdint>

namespace bsp {

// Two DISTINCT complete raw leaves: [004F1800,004F182C) 44B/15 instructions,
// [00435360,00435387) 39B/13. Actual ECX receiver, unused EDX, stacked DWORD,
// full EAX0/1, RET4. Ordered fixed 4D/02/01/00 or 4E/4C/00 precedes ONE fresh
// DWORD read at the SAME actual receiver+C4h. The caller supplies stable
// nonnull backing>=C8h and a genuinely live aligned uint32 cell exactly+C4h.
// Fixed branches do not read the receiver; the common admitted normal domain
// covers dynamic queries. No guard/default/cache/census/router/body collapse.
// Invalid/fault/concurrency/object-lifetime paths outside admission unvalidated.
// SpawnPoint is a descriptive census hint only. Native profile bindings remain
// UNCALLABLE Source DATA; no native class/profile/virtual/group-membership,
// constructor/lifetime/privateEH/world/game binding is established. Closed
// first4E 004351E0 is outside this separate second4E Source provider.

std::uint32_t __fastcall native_entity_is_kind_004f1800(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_entity_is_kind_00435360(const void*, void*, std::uint32_t);

} // namespace bsp
