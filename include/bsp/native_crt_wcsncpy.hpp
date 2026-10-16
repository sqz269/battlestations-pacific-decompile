#pragma once

namespace bsp {

// Whole Original _wcsncpy at 00BFAEEF..00BFAF2B, under supplied DF=0 and
// equal flat DS/ES ordinary-storage conditions. This is a new interface.
// count0 returns actual_destination without payload access or cursor advances.
// Positive counts require every reached raw WORD/DWORD access and all actual
// terminal advances to be genuine, nonwrapping and within/one-past its allocation.
// Overlap, including odd byte offsets and self-induced later reads, is admitted.
// No Original private-frame/home alias, fault/MMIO/concurrency/lifetime protocol.
void* __cdecl copy_native_crt_wide_units_00bfaeef(
    void* actual_destination, const void* actual_source, unsigned int count);

} // namespace bsp
