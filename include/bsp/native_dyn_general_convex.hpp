#pragma once
#include "bsp/dyn_dispatch_initialization.hpp"

namespace bsp {
// Actual 1F8h work record, produced by the dispatcher and composed-intersect
// path. Calls require real shapes with callable double-support slot+0C, valid
// body transforms, matching mesh data and the borrowed CRT state/handler.
// These explicit C++ interfaces preserve the original normal x87 schedule;
// they do not supply native exception metadata or private-stack aliasing.
std::int32_t search_native_dyn_convex_00c51ef0(void* work,const CameraAxesCrtAccess&);
void select_native_dyn_convex_direction_00c51c20(void* work) noexcept;
void project_native_dyn_convex_contact_00c48be0(void* work,const CameraAxesCrtAccess&);
// work+1F0 must point to the constructor's 26 double support directions.
// Result is count plus nine floats per candidate; this path writes one contact
// on a hit and leaves the result unchanged on an early miss. Only AL is the
// original hit result. Native entry: ECX matrixA, EDX shapeB, EAX matrixB,
// stack work/result/shapeA, RET0C.
bool intersect_native_dyn_convex_00c53010(void* work,void* result,
    void* shape_a,const float* matrix_a,void* shape_b,const float* matrix_b,
    const CameraAxesCrtAccess&);
// Complete original dispatcher scratch allocation and invocation contract:
// ECX owner, stack result/shapeA/matrixA/shapeB/matrixB, RET14.
bool dispatch_native_dyn_general_convex_00c535e0(DynGeneralConvexIntersectStorage&,
    void* result,void* shape_a,const float* matrix_a,void* shape_b,const float* matrix_b,
    const CameraAxesCrtAccess&);

// Complete one-slot GeneralConvexIntersect table for the existing actual owner.
// Keep this runtime and its borrowed CRT pointees alive at stable addresses
// while the owner/scenes use the table. Per-call scratch/context is local.
class NativeDynGeneralConvexRuntime final {
public:
    explicit NativeDynGeneralConvexRuntime(const CameraAxesCrtAccess&) noexcept;
    NativeDynGeneralConvexRuntime(const NativeDynGeneralConvexRuntime&)=delete;
    NativeDynGeneralConvexRuntime& operator=(const NativeDynGeneralConvexRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[1];
    CameraAxesCrtAccess crt_;
    static bool __fastcall intersect(void*,void*,void*,void*,const float*,void*,const float*);
};
} // namespace bsp
