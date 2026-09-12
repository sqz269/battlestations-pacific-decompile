#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {
struct CameraAxesCrtAccess;
struct ShipAiPathLateralAnchor;

// Native 24h-byte avoid-zone corner record, reached through path node+10h.
// Producer: 0041CCD0 initializes all nine words; 0041A200 derives geometry;
// 00423190 computes the cached clearance scale. Descriptive names are hypotheses.
// See docs/SHIP_AI_LATERAL_RECORD.md for exact stores and uncertainty.
struct ShipAiPathLateralRecord {
    float x{0.0f};
    float z{0.0f};
    float outgoing_x{0.0f};       // +08h, normalized next - current
    float outgoing_z{0.0f};       // +0Ch
    float offset_dir_x{0.0f};     // +10h, normalized right normal of in + out
    float offset_dir_z{0.0f};     // +14h
    float outgoing_length{0.0f};  // +18h
    float signed_turn{0.0f};      // +1Ch, wrapped outgoing - incoming heading
    float clearance_scale{-1.0f}; // +20h, lazy 00423190 result; not edge length
};
static_assert(sizeof(ShipAiPathLateralRecord) == 0x24);
static_assert(offsetof(ShipAiPathLateralRecord, outgoing_x) == 0x08);
static_assert(offsetof(ShipAiPathLateralRecord, offset_dir_x) == 0x10);
static_assert(offsetof(ShipAiPathLateralRecord, outgoing_length) == 0x18);
static_assert(offsetof(ShipAiPathLateralRecord, signed_turn) == 0x1c);
static_assert(offsetof(ShipAiPathLateralRecord, clearance_scale) == 0x20);

// Only the first eight bytes of the native zone. The allocation capacity,
// layer, associated entity and bounds belong to its larger 24h-byte object.
struct ShipAiPathLateralRecordList {
    ShipAiPathLateralRecord** records;
    std::int32_t count;
};
static_assert(sizeof(ShipAiPathLateralRecordList) == 0x08);
static_assert(offsetof(ShipAiPathLateralRecordList, count) == 0x04);

// Complete 00417610: ECX zone, one signed index on stack, EAX record, RET4.
// Native IDIV gives a signed remainder, not a positive modulo. No guards:
// count must be nonzero, division representable and records[index % count]
// readable; a negative remainder can address before the array.
ShipAiPathLateralRecord* ship_ai_lateral_record_at_00417610(
    const ShipAiPathLateralRecordList& zone, std::int32_t index) noexcept;

// Complete geometry/control-flow projection of 0041A200, ECX zone, one byte
// flag in a stack word, AL result, RET4. Computes all derived fields, sums
// turns in binary32, and reverses/recomputes once if sum>0 and flag!=0.
// Return describes the ORIGINAL pass: false even after reversing successfully.
// Empty input returns true. Nonempty input requires a valid nonnegative count,
// pointer array and records. No count>=3, zero-length, NaN or self-intersection
// repair: the outgoing unit still divides by zero after a cutoff-to-zero length.
// x/z and clearance_scale remain untouched, including during reversal.
// Reuses 00419260 and 00438B10, and 00414EB0's equivalent heading sequence.
// CRT sqrt borrows the required actual access; heading uses the existing host
// CRT binding. This new C++ interface is not a binary replacement; legacy CRT
// exception details and whole-routine native differential parity are unverified.
bool ship_ai_lateral_records_derive_0041a200(ShipAiPathLateralRecordList& zone,
    bool reverse_positive_turn, const CameraAxesCrtAccess& crt);

// Copy into the EXISTING compact follower projection; never reinterpret the
// four-float ShipAiPathLateralAnchor as a packed native 24h record.
ShipAiPathLateralAnchor ship_ai_lateral_anchor_projection(
    const ShipAiPathLateralRecord& record) noexcept;
} // namespace bsp
