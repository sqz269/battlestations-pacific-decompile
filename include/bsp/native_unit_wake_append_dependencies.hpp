#pragma once

namespace bsp::game { class GameNativeGeometryGlobals; }

namespace bsp {

// Raw 0042B2F0: ECX points at three readable float cells; ST0 returns the
// float-spilled length, ECX returns its raw float bits and RET consumes no stack arguments.
// Preserve the actual Y/X/Z load order, x87 arithmetic, float spills and
// FCOMI cutoff: squared length <= double 1e-10 or unordered returns +0.
// The other arm calls the genuine current CRT _CIsqrt with its input in ST0.
// Four free x87 slots are needed by this body; CRT entry requirements and its
// dispatch/diagnostic/exception policy remain external. No FP reset is added.
// Storage lifetime, validity and synchronization belong to the caller. This
// is a raw Win32 entry, not a typed trail adapter or Original CRT replacement.
float __fastcall native_unit_wake_length_0042b2f0(
    const float* actual_vector) noexcept;

// Raw 00810160: ECX=destination, [ESP+4]=source, EAX=destination on return,
// RET4. The unused EDX parameter preserves the native source stack position;
// it is not an object/context argument. ECX becomes source; EDX is untouched.
// Six ascending FLD/FSTP pairs access 24 readable source / writable destination
// bytes. Overlap and self-copy retain native sequential propagation, SNaN
// quieting, rounding/status effects and ambient x87 exception behavior.
// One free x87 slot is required. No snapshot, byte copy, overlap guard or FP
// reset is introduced. Caller owns valid storage and synchronization; this
// leaf constructs no ring, owner, reset vector or entity and updates no head.
void* __fastcall copy_native_unit_wake_sample_00810160(
    void* actual_destination, void* unused_edx, const void* actual_source) noexcept;

// Ordinary Source interface for the complete 00810190 algorithm; this is not
// the Original binary entry ABI. Borrow a genuine live raw wake (988 bytes):
// samples +8, 40 records of 24 bytes; head +0x3C8 in 0..39 at every read;
// full-byte flag +0x3CC and mutable residual +0x3D0. ShipAiWakeTrail has a
// different layout and must not be reinterpreted or copied into this API.
// Position supplies three readable float cells. Pass the actual canonical
// process instance from game::game_native_geometry_globals(); its mutable
// reset cells are read at each original observation, never snapshotted.
// Caller owns lifetimes and synchronization. Valid external regions may
// alias, but may not overlap active wrapper/kernel frames or formal slots.
// Scalar arguments are passed by value and copied as bits into native slots.
// Four free local x87 slots are required, excluding genuine CRT interiors.
// No FP-environment reset or Original/current CRT policy equivalence is
// supplied. This API constructs no storage, entity, owner or application flow.
void append_native_unit_wake_00810190(
    void* actual_wake, const float* actual_world_position,
    float heading, float yaw_rate,
    game::GameNativeGeometryGlobals& actual_geometry_globals);

} // namespace bsp
