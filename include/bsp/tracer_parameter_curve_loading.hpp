#pragma once

#include "bsp/native_lua_objects.hpp"
#include "bsp/tracer_parameter_curve.hpp"

namespace bsp {

// Native CRT services, NOT another curve owner/refcount/domain. A custom
// allocator must return nonnull or throw, and its matching free must not throw.
// Defaults use malloc/_callnewh/bad_alloc and free, like BF681B/BF9DC8.
// Reuse the same bindings for the owner and its records throughout its lifetime.
struct TracerParameterCurveMemory {
    void* context;
    void* (*allocate)(void*, std::uint32_t bytes);
    void (*free)(void*, void*) noexcept;
};
const TracerParameterCurveMemory& tracer_parameter_curve_memory() noexcept;

// BABFE0 / BAC5E0 receive ECX=&owner.records, one signed count, RET4.
// These C++ interfaces take the containing actual 1Ch owner. Capacity is at
// least one on a growing reserve. Resize does not change point_count/cache.
// Integer arithmetic wraps at 32 bits; malformed counts/storage are unchecked.
void reserve_tracer_curve_records_00babfe0(TracerParameterCurveStorage&,
    std::int32_t capacity, const TracerParameterCurveMemory& = tracer_parameter_curve_memory());
void resize_tracer_curve_records_00bac5e0(TracerParameterCurveStorage&,
    std::int32_t size, const TracerParameterCurveMemory& = tracer_parameter_curve_memory());

// BAC670: ECX actual owner, stack coordinate then value, RET8. Updates the
// previous point's slope BEFORE potentially throwing growth. Delta FSTP32,
// threshold bits3A83126F, unordered JBE, FDIVRP and slope FSTP32 are preserved.
void append_tracer_curve_point_00bac670(TracerParameterCurveStorage&, float coordinate,
    float value, const TracerParameterCurveMemory& = tracer_parameter_curve_memory());

// 869E40: three stack args (Lua object*, owner**, binary32 scale), RET0C;
// ECX is unused. Reuses the actual native stack-tracked Lua objects. Publishes
// a constructed owner before reading rows; appends to an existing owner.
// Stops at the FIRST nil row. No validation, sorting, rollback or cache reset.
// Native SEH registration/Lua nonlocal-error ABI is not replaced.
void load_tracer_parameter_curve_00869e40(NativeLuaObjectStorage& table,
    TracerParameterCurveStorage*& output, float scale,
    const TracerParameterCurveMemory& = tracer_parameter_curve_memory());

// BACB10: ECX owner, RET. Resize(0), free records, then write base tableCEB130.
// Other fields (including dangling record pointer/capacity/count/cache) remain.
// BACBB0: ECX owner, stack flags, RET4/EAX same address, free owner iff flags&1.
// These are actual default-curve destructor bindings; neither decrements the
// refcount nor dereferences the original-image native_vtable as host code.
void destroy_tracer_parameter_curve_00bacb10(TracerParameterCurveStorage&,
    const TracerParameterCurveMemory& = tracer_parameter_curve_memory());
TracerParameterCurveStorage* scalar_destroy_tracer_parameter_curve_00bacbb0(
    TracerParameterCurveStorage&, std::uint32_t flags,
    const TracerParameterCurveMemory& = tracer_parameter_curve_memory());

} // namespace bsp
