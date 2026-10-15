#pragma once

#include "bsp/avoid_zone_owner.hpp"
#include "bsp/cc13_avoid1.hpp"

#include <cstddef>

namespace bsp {
// Complete 0041C490..0041C4C7. Native ECX points to one owner pointer cell;
// no stack arguments; RET. Capture its zone; null skips every write/free.
// Otherwise reuse the exact 0041A1E0 clear+buffer destruction, then separately
// free the zone OBJECT through actual 00BF65AC and finally null the owner cell.
// free_record_00bf65ac is the existing untyped actual CRT-free endpoint; this
// call frees the zone object, distinct from records and its pointer buffer.
// No C++ delete, unique_ptr policy, extra capacity reset or invented allocator.
//
// Zone records, pointer buffer and object must satisfy the existing actual
// matching ownership/free contracts. An opaque-prefix copy does not establish
// them. The owner cell must remain writable through object free and final null,
// outside every freed allocation. No concurrent mutation or callback invalidation.
// New C++ interface, not a native ABI replacement. Names are hypotheses.
void avoid_zone_owner_destroy_0041c490(AvoidZoneNativeStorage*& owner,
    const AvoidZoneAllocationAccess&) noexcept;

static_assert(offsetof(Cc13AvoidZoneOptionalVector, x) == 0x00);
static_assert(offsetof(Cc13AvoidZoneOptionalVector, z) == 0x04);
static_assert(offsetof(Cc13AvoidZoneOptionalVector, valid) == 0x08);

// Complete 0041A4E0..0041A532. Recovered native __fastcall ABI: ECX=out,
// EDX=a0, stack a1,b0,b1; EAX=out, RET0Ch. Pseudocode's __thiscall form
// misses the live EDX input. Each endpoint points to two readable binary32
// words; native 004F3730 handles crossing, degeneracy and unordered rejection.
// The existing 0Ch optional-vector storage is reused as an optional POINT.
// Success copies two result WORD REPRESENTATIONS (MOVSS), then writes valid=1.
// Failure writes ONLY valid=0. All three trailing padding bytes stay untouched.
// Returns the same output object. The temporary result is separate, so inputs
// may overlap each other and output: all input reads finish before output writes.
// Input storage must be readable/stable during the call; output must be writable.
// New typed C++ interface; register/exception ABI, packet-native differential
// equivalence and gameplay remain unverified. See the packet doc/report.
Cc13AvoidZoneOptionalVector& segment_intersection_optional_point_0041a4e0(
    Cc13AvoidZoneOptionalVector& output, const float* a0, const float* a1,
    const float* b0, const float* b1) noexcept;
} // namespace bsp
