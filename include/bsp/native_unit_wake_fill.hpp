#pragma once

namespace bsp::game { class GameNativeGeometryGlobals; }

namespace bsp {

// Ordinary C++ interface to the complete native 00810020 wake fill.
// actual_wake supplies writable native storage: 40 records at +8, stride 0x18,
// head at +0x3C8 and flag at +0x3CC. Position addresses three readable floats
// and may alias writable wake storage; head/flag stores precede position reads.
// Fills XYZ/heading/segment in slots 39..0, preserving every yaw, residual,
// prefix, padding and other byte. The caller owns the preserved preimage,
// lifetime and synchronization. No written count or owner model is introduced.
// Requires four free x87 slots. Preserves ambient FP settings and the exact
// FCOS/FSIN, spill, read/store and arithmetic status/exception sequence.
void fill_native_unit_wake_00810020(void* actual_wake,
    const float* position, float heading);

// Ordinary Source constructor interface for complete 00815600. Borrow actual
// caller-owned writable 988-byte Native wake storage, distinct from the active
// frames and canonical geometry cells. This neither allocates the wake nor
// creates a C++ owner class. ShipAiWakeTrail and UnitPoseHistoryRing both have
// different physical layouts and must not be cast or copied into this API.
// Pass the genuine process instance from game::game_native_geometry_globals().
// Its mutable seed cells are observed by the actual fill and then read afresh
// for residual; the pointer is retained across both real helper calls.
// Preserve the unknown yaw/padding preimage. Storage must not already own a
// live section. The created section is the real malloc-backed 0x1C allocation
// and is published at +4 before fill. Four free local x87 slots are required;
// no FP reset is added. Caller owns storage lifetime and synchronization.
// Admitted domain is normal successful construction; no invented rollback,
// Original CRT/SEH/fault equivalence or Original no-context thiscall ABI.
void* construct_native_unit_wake_00815600(void* actual_wake,
    game::GameNativeGeometryGlobals& actual_geometry_globals);

// Complete plain 00818100: Native vptr word D09480, ECX+=4, tail jump to the
// actual owned tracked-section release. End all borrowers before destruction;
// release requires the owning/quiescent thread and the genuine section from
// the matching constructor. It drains positive depth, deletes/frees the real
// section and clears its slot. The wake allocation root is never freed here.
void __fastcall destroy_native_unit_wake_00818100(void* actual_wake);

} // namespace bsp
