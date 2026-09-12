#pragma once

#include <cstddef>
#include <cstdint>

// 00BA9DA0 and 00BACAA0. Descriptive names are hypotheses; see
// docs/TRACER_PARAMETER_CURVE.md for producer, ABI and coverage evidence.
// This is the concrete curve published by 00869E40, not a curve loader.

namespace bsp {

struct TracerParameterCurveRecord {
    float coordinate;  // +00: first Lua number, scaled by 00869E40's third arg
    float value;       // +04: second Lua number
    float slope;       // +08: 00BAC670 computes this when the next point arrives
};

// Exact 1Ch-byte Win32 storage. No C++ vptr or ownership machinery is added.
// The address at +00 identifies the ORIGINAL image's table; it is not a callable
// replacement vtable in this executable. The caller supplies/owns any records.
struct TracerParameterCurveStorage {
    std::uint32_t native_vtable;
    std::int32_t reference_count;
    TracerParameterCurveRecord* records;
    std::int32_t storage_size;
    std::int32_t storage_capacity;
    std::int32_t point_count;
    std::uint32_t current_segment;

    // 00BACAA0: complete normal-return initialized state, ECX=this, RET, EAX=this.
    // For fresh storage only: overwrites fields and does not release old records.
    // Native EH registration is omitted; its resize(0) on zero storage cannot grow.
    TracerParameterCurveStorage* __thiscall initialize_00bacaa0() noexcept;

    // 00BA9DA0: complete instruction path, ECX=this, one binary32 stack argument,
    // RET 4, x87 ST0 return. The interpolation branch FSTP32/FLD32 is retained.
    // Preserves the caller's x87 control word; no SSE/double approximation.
    // Requires the native valid storage/cache invariants. It does not sanitize
    // malformed data or attach storage. It mutates only current_segment.
    float __thiscall sample_00ba9da0(float argument) noexcept;
};

inline constexpr std::uint32_t kTracerParameterCurveNativeVtable = 0x00d63ffcu;

static_assert(sizeof(TracerParameterCurveRecord) == 0x0c);
static_assert(offsetof(TracerParameterCurveRecord, value) == 0x04);
static_assert(offsetof(TracerParameterCurveRecord, slope) == 0x08);
static_assert(sizeof(void*) == 4, "The native curve storage requires Win32.");
static_assert(sizeof(TracerParameterCurveStorage) == 0x1c);
static_assert(offsetof(TracerParameterCurveStorage, native_vtable) == 0x00);
static_assert(offsetof(TracerParameterCurveStorage, reference_count) == 0x04);
static_assert(offsetof(TracerParameterCurveStorage, records) == 0x08);
static_assert(offsetof(TracerParameterCurveStorage, storage_size) == 0x0c);
static_assert(offsetof(TracerParameterCurveStorage, storage_capacity) == 0x10);
static_assert(offsetof(TracerParameterCurveStorage, point_count) == 0x14);
static_assert(offsetof(TracerParameterCurveStorage, current_segment) == 0x18);

}  // namespace bsp
