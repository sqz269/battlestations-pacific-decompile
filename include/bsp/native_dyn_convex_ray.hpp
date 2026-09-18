#pragma once
#include "bsp/dyn_dispatch_initialization.hpp"

namespace bsp {
// Complete 00C44780..00C47A8A normal body. Native ECX owner, stack
// result/shape/start/end, RET10h, AL hit. Result is six floats: point then normal.
// The shape needs its actual body/transform and callable double-support slot0C.
// Mutates the owner's simplex and ray scratch; calls sharing an owner must be
// serialized. Preserve native x87 order, tolerances, ties and stale scratch.
// This explicit interface supplies the borrowed CRT service; it does not supply
// native exception metadata, private-stack aliases or arbitrary invalid inputs.
bool intersect_native_dyn_convex_ray_00c44780(DynConvexRayIntersectionStorage&,
    void* result,const void* shape,const float* start,const float* end,
    const CameraAxesCrtAccess&);

// Complete one-slot source table for the actual A0h owner. Keep this runtime
// and its borrowed CRT pointees at stable addresses while scenes use the table.
class NativeDynConvexRayRuntime final {
public:
    explicit NativeDynConvexRayRuntime(const CameraAxesCrtAccess&) noexcept;
    NativeDynConvexRayRuntime(const NativeDynConvexRayRuntime&)=delete;
    NativeDynConvexRayRuntime& operator=(const NativeDynConvexRayRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[1];
    CameraAxesCrtAccess crt_;
    static bool __fastcall intersect(void*,void*,void*,const void*,const float*,const float*);
};
} // namespace bsp
