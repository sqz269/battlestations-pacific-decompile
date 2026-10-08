#pragma once

#include <cstdint>

namespace bsp {

// THREE DISTINCT complete raw class02/04/05 vtable+5Ch leaves: 132B/45inst.
// ECX actual receiver, unused EDX, stacked DWORD, full EAX0/1, RET4. Exact
// ordered fixed words precede ONE fresh DWORD at the SAME actual receiver+C4.
// Admit stable nonnull backing>=C8h with a genuinely live aligned uint32 cell
// exactly+C4h; the common domain covers the dynamic path. Fixed-word paths
// do not read ECX. No new guard/default/cache/census or class-routing policy.
// Invalid/fault/concurrency/lifetime paths outside this admission unvalidated.
// Numeric names preserve the descriptive native class-name hypotheses.
// Native profiles remain UNCALLABLE Source data. No original class ctor,
// allocation/observer/world/EH/virtual type admission or gameplay closure.

std::uint32_t __fastcall native_entity_base_is_kind_004f1750(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_entity_base_is_kind_006d1610(const void*, void*, std::uint32_t);
std::uint32_t __fastcall native_entity_base_is_kind_006d1650(const void*, void*, std::uint32_t);

} // namespace bsp
