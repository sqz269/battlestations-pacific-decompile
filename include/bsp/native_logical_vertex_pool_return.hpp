#pragma once

namespace bsp {

// Complete B49570..B495D9. Native ECX actual pool, one stack raw-slot pointer,
// RET4, no semantic EAX result and no local exception cleanup. The canonical
// pool is 0108FE18; this function borrows the caller's actual initialized pool.
//
// Enter the saved pool+0C critical section, increment its tracked depth+24,
// read slot+74 and the current pool+28 slab table, and return the low WORD of
// signed wrapped (slot-slab)/120 to the current slab+F40 free-stack position.
// Reload/increment that WORD count after the store, update the unsigned lowest
// slab index at pool+34, decrement current depth, and leave the saved section.
// Every reached raw storage access must be valid. No allocation, destructor,
// canonical singleton lookup, bounds guard or rollback is introduced.
//
// New MSVC Win32 fastcall interface places raw_slot in EDX, unlike the native
// stack argument. This does not close logical-vertex destruction or game proof.
void __fastcall return_native_logical_vertex_pool_slot_00b49570(
    void* actual_pool, const void* raw_slot);

} // namespace bsp
