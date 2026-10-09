#pragma once

#include <cstdint>

namespace bsp {

// Complete 006F58E0..006F591A: ECX is the same actual receiver, the raw query
// DWORD is at entry ESP+4, EAX returns 0/1, and both exits RET4. Incoming EAX
// is overwritten; incoming EDX is unused, explicit in this fastcall spelling.
// Fixed words 1C/1B/5/4/2/1/0 precede ONE late actual DWORD+C4 comparison.
// Normal caller: stable nonnull receiver with genuine aligned uint32_t at+C4
// and readable backing through+C8. When composing raw storage, establish the
// lifetime of that DWORD at its actual offset; do not supply a copied class id.
// Fixed matches avoid the receiver read but establish no lifetime admission.
// Native table/profile values remain numeric DATA. This direct leaf installs
// no virtual routing, constructor, owner, dispatcher or numbering services.
// Invalid placement, faults, concurrent mutation, private EH and game binding
// remain unproved. MCommandBuilding is the existing provisional interpretation.
std::uint32_t __fastcall native_mcommandbuilding_is_kind_006f58e0(
    const void* actual_receiver, void* unused_edx, std::uint32_t class_word);

} // namespace bsp
