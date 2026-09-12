#include "bsp/tracer_parameter_curve_loading.hpp"

#include <cstdlib>
#include <cstring>
#include <new>
#include <new.h>
extern "C" {
#include <lua.h>
}

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Tracer curve loading requires MSVC Win32 x87 semantics.
#endif

namespace bsp {
namespace {
void* allocate_records(void*, std::uint32_t bytes) {
    for (;;) {
        if (void* result = std::malloc(bytes)) return result;
        if (!_callnewh(bytes)) throw std::bad_alloc();
    }
}
void free_records(void*, void* memory) noexcept { std::free(memory); }
const TracerParameterCurveMemory default_memory{nullptr, allocate_records, free_records};

std::int32_t add32(std::int32_t a, std::int32_t b) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) + static_cast<std::uint32_t>(b));
}
TracerParameterCurveRecord* record_at(TracerParameterCurveRecord* base, std::int32_t index) noexcept {
    return reinterpret_cast<TracerParameterCurveRecord*>(reinterpret_cast<std::uintptr_t>(base) +
        static_cast<std::uint32_t>(index) * 12u);
}
void copy_record(TracerParameterCurveRecord* destination, const TracerParameterCurveRecord* source) noexcept {
    // Integer copies preserve even signaling-NaN payloads without x87 loads.
    std::memcpy(destination, source, sizeof(*destination));
}

const std::uint32_t slope_threshold_bits = 0x3a83126fu;
__declspec(naked) void __cdecl update_previous_slope(TracerParameterCurveRecord*, float, float) noexcept {
    __asm {
        mov ecx, dword ptr [esp + 4]
        fld dword ptr [esp + 8]
        fsub dword ptr [ecx]
        fstp dword ptr [esp + 8]
        fld dword ptr [esp + 8]
        fld dword ptr [slope_threshold_bits]
        fcomip st(0), st(1)
        jbe compute_slope // Unordered follows this branch, as at BAC6C1.
        fstp st(0)
        mov dword ptr [ecx + 8], 0
        ret
    compute_slope:
        fld dword ptr [esp + 12]
        fsub dword ptr [ecx + 4]
        fdivrp st(1), st(0)
        fstp dword ptr [ecx + 8]
        ret
    }
}
float scaled_number(NativeLuaObjectStorage& object, float scale) {
    // The getter has already performed native B66280 FSTP32/FLD32.
    const float number = native_lua_number_00b66270(object);
    float result;
    __asm {
        fld number
        fmul scale
        fstp result
    }
    return result;
}
float argument_spill(float input) noexcept {
    float result;
    __asm {
        fld input
        fstp result
    }
    return result;
}
bool is_nil(const NativeLuaObjectStorage& object) {
    // Complete B65FB0 predicate over the canonical actual storage.
    return object.kind_04 == 2 && lua_type(object.owner_00->state_04, object.index_08) == LUA_TNIL;
}
struct ObjectCleanup {
    NativeLuaObjectStorage& object;
    ~ObjectCleanup() { destroy_native_lua_object_00b67700(object); }
};
} // namespace

const TracerParameterCurveMemory& tracer_parameter_curve_memory() noexcept { return default_memory; }

void reserve_tracer_curve_records_00babfe0(TracerParameterCurveStorage& owner,
    std::int32_t capacity, const TracerParameterCurveMemory& memory) {
    if (capacity < 1) capacity = 1;
    if (capacity <= owner.storage_capacity) return;
    auto* const fresh = static_cast<TracerParameterCurveRecord*>(
        memory.allocate(memory.context, static_cast<std::uint32_t>(capacity) * 12u));
    for (std::int32_t i = 0; i < owner.storage_size; i = add32(i, 1)) {
        auto* const destination = record_at(fresh, i);
        if (destination) copy_record(destination, record_at(owner.records, i));
    }
    memory.free(memory.context, owner.records);
    // Disk BAC053..BAC05C supplies these publications after free returns.
    *static_cast<TracerParameterCurveRecord* volatile*>(&owner.records) = fresh;
    *static_cast<volatile std::int32_t*>(&owner.storage_capacity) = capacity;
}

void resize_tracer_curve_records_00bac5e0(TracerParameterCurveStorage& owner,
    std::int32_t size, const TracerParameterCurveMemory& memory) {
    if (owner.storage_capacity < size) reserve_tracer_curve_records_00babfe0(owner, size, memory);
    for (auto i = owner.storage_size; i < size; i = add32(i, 1)) {
        if (auto* const record = record_at(owner.records, i)) std::memset(record, 0, sizeof(*record));
    }
    while (size < owner.storage_size)
        *static_cast<volatile std::int32_t*>(&owner.storage_size) = add32(owner.storage_size, -1);
    owner.storage_size = size;
}

void append_tracer_curve_point_00bac670(TracerParameterCurveStorage& owner,
    float coordinate, float value, const TracerParameterCurveMemory& memory) {
    TracerParameterCurveRecord point;
    std::memcpy(&point.coordinate, &coordinate, 4);
    std::memcpy(&point.value, &value, 4);
    point.slope = 0.0f;
    if (owner.point_count != 0)
        update_previous_slope(record_at(owner.records, add32(owner.point_count, -1)), coordinate, value);
    if (owner.storage_size == owner.storage_capacity) {
        auto capacity = add32(owner.storage_capacity, owner.storage_capacity);
        if (capacity <= 1) capacity = 1;
        reserve_tracer_curve_records_00babfe0(owner, capacity, memory);
    }
    if (auto* const record = record_at(owner.records, owner.storage_size)) copy_record(record, &point);
    *static_cast<volatile std::int32_t*>(&owner.storage_size) = add32(owner.storage_size, 1);
    *static_cast<volatile std::int32_t*>(&owner.point_count) = add32(owner.point_count, 1);
}

void load_tracer_parameter_curve_00869e40(NativeLuaObjectStorage& table,
    TracerParameterCurveStorage*& output, float scale, const TracerParameterCurveMemory& memory) {
    auto* owner = output; // EDI is captured once; callbacks do not retarget it.
    if (!owner) {
        void* const fresh = memory.allocate(memory.context, 0x1c);
        owner = fresh ? (::new(fresh) TracerParameterCurveStorage)->initialize_00bacaa0() : nullptr;
        output = owner;
    }
    NativeLuaObjectStorage unused_first, unused_second;
    construct_native_lua_object_00b65f50(&unused_first);
    ObjectCleanup first_cleanup{unused_first};
    construct_native_lua_object_00b65f50(&unused_second);
    ObjectCleanup second_cleanup{unused_second};
    for (std::int32_t index = 1;; index = add32(index, 1)) {
        NativeLuaObjectStorage row;
        native_lua_get_by_index_00b67720(table, &row, index);
        ObjectCleanup row_cleanup{row};
        if (is_nil(row)) break;
        float coordinate;
        {
            NativeLuaObjectStorage number;
            native_lua_get_by_index_00b67720(row, &number, 1);
            ObjectCleanup number_cleanup{number};
            coordinate = scaled_number(number, scale);
        }
        float value;
        {
            NativeLuaObjectStorage number;
            native_lua_get_by_index_00b67720(row, &number, 2);
            ObjectCleanup number_cleanup{number};
            value = argument_spill(native_lua_number_00b66270(number));
        }
        // Native loads/stores value, then coordinate, to its RET8 callee args.
        value = argument_spill(value);
        coordinate = argument_spill(coordinate);
        append_tracer_curve_point_00bac670(*owner, coordinate, value, memory);
    }
}

void destroy_tracer_parameter_curve_00bacb10(TracerParameterCurveStorage& owner,
    const TracerParameterCurveMemory& memory) {
    resize_tracer_curve_records_00bac5e0(owner, 0, memory);
    memory.free(memory.context, owner.records);
    owner.native_vtable = 0x00ceb130u; // BD30F0, reached at disk BACB57.
}
TracerParameterCurveStorage* scalar_destroy_tracer_parameter_curve_00bacbb0(
    TracerParameterCurveStorage& owner, std::uint32_t flags, const TracerParameterCurveMemory& memory) {
    auto* const result = &owner;
    destroy_tracer_parameter_curve_00bacb10(owner, memory);
    if (flags & 1u) memory.free(memory.context, result);
    return result;
}
} // namespace bsp
