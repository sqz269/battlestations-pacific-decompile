#pragma once
#include "bsp/dyn_dispatch_initialization.hpp"

namespace bsp {
// Complete native box/box chain, including face clipping and edge contacts.
// Original slot: owner ECX unused, stack result/A/matrixA/B/matrixB, RET14h;
// AL is the hit result. Inputs use actual box transform+34 and extents+210.
// Result capacity: count plus eight nine-float contacts. Valid borrowed box
// records, body transforms and CRT state are required for the whole call.
bool intersect_native_dyn_boxes_00c49a30(void* result,const void* shape_a,
    const float* matrix_a,const void* shape_b,const float* matrix_b,const CameraAxesCrtAccess&);

// Complete one-slot source table for an actual DynStaticDispatchObjectStorage.
// Keep this noncopyable runtime and its borrowed CRT pointees alive and stable
// while any owner or scene uses the table. Invocation scratch is local.
class NativeDynBoxBoxRuntime final {
public:
    explicit NativeDynBoxBoxRuntime(const CameraAxesCrtAccess&) noexcept;
    NativeDynBoxBoxRuntime(const NativeDynBoxBoxRuntime&)=delete;
    NativeDynBoxBoxRuntime& operator=(const NativeDynBoxBoxRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[1];
    CameraAxesCrtAccess crt_;
    static bool __fastcall intersect(void*,void*,void*,const void*,const float*,const void*,const float*);
};
} // namespace bsp
