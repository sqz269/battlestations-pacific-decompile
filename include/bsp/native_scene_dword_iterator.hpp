#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw DWORD iterators require MSVC Win32.
#endif

namespace bsp {
// Complete 00467A80. Native ECX iterator8 {owner,current}; one stacked delta,
// RET4/EAX captured iterator. actual_delta4 binds that live DWORD, read only
// AFTER the initial owner diagnostic returns. Scale modulo2^32, compare proposed
// current against owner.end8 then owner.begin4 (skip begin when above end).
// A returning bounds diagnostic is followed by CURRENT iterator.current +=
// captured scaled delta. There is no owner/bounds recheck or recovery clamp.
void* advance_native_scene_dword_iterator_00467a80(
    void* actual_iterator8, const void* actual_delta4);

// Complete 00467D50. Native ECX left iterator8; stacked other-iterator pointer,
// RET4. Capture left.owner BEFORE reading actual_other_parameter4. Capture
// other pointer before diagnostic; owner0 skips the other.owner read. After a
// returning diagnostic, read current left.current then captured other.current.
// Raw EAX has left.current's upper24 bits and inequality in AL. No live caller
// establishes a wider semantic return type; the interface exposes exact bits.
std::uint32_t unequal_native_scene_dword_iterators_00467d50(
    const void* actual_left_iterator8, const void* actual_other_parameter4);

// Actual parameter cells and payload may alias where every reached access is
// valid; binding pointers are stable. These are explicit new Source interfaces.
// Raw owner links use offsets4/8; no nominal collection class is inferred.
// Original private-stack/register/CRT ABI and game behavior remain unproved.
} // namespace bsp
