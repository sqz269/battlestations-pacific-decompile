#pragma once

#include <cstdint>

namespace bsp {

// TWO DISTINCT complete raw class00/01 vtable+5Ch leaves: 63B/20 instructions.
// ECX actual receiver, unused EDX, stacked DWORD, full EAX0/1, RET4. Ordered
// fixed words precede ONE fresh DWORD at the SAME actual receiver+C4h.
// Admit stable nonnull backing>=C8h with a genuinely live aligned uint32 cell
// exactly+C4h; this common domain covers each dynamic path. Fixed-word paths
// do not read ECX. No added guard/default/cache/census or class-routing policy.
// Invalid/fault/concurrency/lifetime paths outside this admission unvalidated.
// Numeric names retain the descriptive native class-name hypotheses.
// Native profiles remain UNCALLABLE Source data. Source field compatibility
// proves no original class ctor, group membership, virtual admission, class
// lifetime, allocation/observer/world/private EH or gameplay binding.

std::uint32_t __fastcall native_entity_root_is_kind_0042b8f0(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_entity_root_is_kind_0047f190(const void*, void*, std::uint32_t);

} // namespace bsp
