#pragma once

namespace bsp {

// 004B4F10: original ECX actual complete owner; plain RET. EDX adds the
// borrowed address of the actual F8D420 publication cell. Clear that cell
// unconditionally BEFORE stamping the owner's primary profile CE3818.
// This is the B1B680 state0 unwind action as well as its inline normal tail.
void __fastcall clear_native_particle_clock_publication_base_004b4f10(
    void* actual_complete_owner,
    void* volatile& actual_publication_00f8d420) noexcept;

// No compare/exchange, manager unregister, free, owner-domain lookup or replay
// state is added. The extra publication-cell argument is a new source ABI.

} // namespace bsp
