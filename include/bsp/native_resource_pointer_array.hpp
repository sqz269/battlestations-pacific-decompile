#pragma once
#include <cstdint>

namespace bsp {
// Actual0Ch header: dataDWORD+0, signed countDWORD+4, signed capacityDWORD+8.
// This is the physical layout already declared by NativeRenderPointerArrayStorage;
// these entries borrow its raw address without constructing another header/type.
// Elements are4-byte pointer words. No pointee methods, AddRef or release occur.
//
// B872F0/B87350: ECX actual header; signed capacity stack; RET4. Minimum16;
// signed capacity test, wrapping byte count, forward current-header copy,
// free current old data before publishing replacement data/capacity.
void reserve_native_resource_item_pointers_00b872f0(void* actual_header,
    std::int32_t requested_capacity);
void reserve_native_hierarchy_item_pointers_00b87350(void* actual_header,
    std::int32_t requested_capacity);

// B873C0/B87410: ECX actual header; signed count stack; RET4. Reserve if needed,
// zero added slots using current data, decrement current count when shrinking,
// then store requested count. Negative count/capacity and DWORD wrap remain.
void resize_native_resource_item_pointers_00b873c0(void* actual_header,
    std::int32_t requested_count);
void resize_native_hierarchy_item_pointers_00b87410(void* actual_header,
    std::int32_t requested_count);

// Full B87B20..B87B36 / B87B40..B87B56: ECX header, RET. Resize0 then free
// current backing data. Leave data/capacity stale after free. Negative signed
// capacity can cause a64-byte reserve allocation during this cleanup.
void destroy_native_resource_item_pointers_00b87b20(void* actual_header);
void destroy_native_hierarchy_item_pointers_00b87b40(void* actual_header);

// Existing concrete source allocation/free services only. All reached raw
// addresses must be valid under native32-bit arithmetic; no overflow/bounds
// guards, corrupt-header repair, rollback or noexcept contract is added.
// These C++ interfaces have no meaningful result and are not native ABI/FH3
// replacements. Backing-array cleanup does not complete resource ownership,
// cache removal, hierarchy-pool return, item destruction or gameplay.
} // namespace bsp
