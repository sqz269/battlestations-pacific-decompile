#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native pending tick group initialization requires MSVC Win32.
#endif

namespace bsp {

// Borrowed Source translation of the accepted 00CD27C0..00CD2858 initializer
// fragment: five groups at stride 0x68, with each tail at head+0x34. The saved
// Native evidence has no function/listing at the entry; this is a new Source
// interface, not a recovered Native symbol or a drop-in fixed-address entry.
//
// actual_first_group_head supplies ECX. In ascending group order, write only:
//   head+4 = 0; head+8 = actual tail; tail+8 = 0; tail+4 = actual head.
// The caller supplies writable backing for those twenty DWORDs and retains
// the actual identities for all consumers. The last write ends at base+0x1DF;
// 0x1E0 is a direct-access bound, not proof of a full five*0x68 object. No
// profile/count/other-field initialization, allocation, ownership or production
// lifecycle is supplied. Reinitializing live links is not made safe here.
//
// The declared Source interface takes one pointer in ECX and uses EDX as
// scratch. The Native fragment takes no argument and leaves ECX/EDX intact.
// Both store schedules are qualified separately; Native ABI, emitted Source,
// startup and gameplay equivalence remain unproved. Deliberately no noexcept.
void __fastcall initialize_native_pending_tick_group_sentinels_00cd27c0(
    void* actual_first_group_head);

} // namespace bsp
