#pragma once

namespace bsp {
// Complete raw 0070D0C0..0070D0E3, 36 bytes: ECX=actual group, stacked
// next and previous, RET8. Unused EDX preserves native stack placement.
// Compare the two arguments, not group+14. Equal arguments leave even a
// third stored leader untouched. Otherwise, nonnull previous calls whole
// raw wake handoff before publishing next at actual group+14; null previous
// skips only the handoff. There is no null-next fallback or callback.
//
// Borrow valid actual group storage and, on a wake path, FAC-byte unit
// prefixes and every reached pose ancestor. A reached parent multiply needs
// an EMPTY x87 stack; other wake paths need one free slot. Production does
// not reset floating state. Full class ABI, construction/lifetime, caller
// type/speed/detach routing, faults, concurrency and game remain external.
void __fastcall set_native_unit_group_leader_0070d0c0(
    void* actual_group, void* unused_edx,
    void* actual_next_unit, void* actual_previous_unit) noexcept;
} // namespace bsp
