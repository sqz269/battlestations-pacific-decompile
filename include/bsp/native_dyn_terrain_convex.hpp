#pragma once
#include "bsp/dyn_dispatch_initialization.hpp"

namespace bsp {
// Complete 00C53630..00C549C8 normal body. Native owner ECX unused,
// stack result/shapeA/matrixA/shapeB/matrixB, RET14h. One shape is terrain
// (kind5); the other supplies actual 16-byte convex mesh vertices. Output is
// count plus at most eight contacts, each nine floats: pointA, pointB, normal.
// Preserve native inclusive grid bounds, neighbor sampling, interpolation,
// x87 spills, contact order, and reversed-shape point/normal handling.
// Terrain sample storage must cover every accessed right/down neighbor, even
// at an inclusive grid edge. This interface does not add allocation or bounds
// repair and does not supply native exception metadata or private-stack aliases.
bool intersect_native_dyn_terrain_convex_00c53630(void* result,
    const void* shape_a,const float* matrix_a,const void* shape_b,const float* matrix_b,
    const CameraAxesCrtAccess&);

// Complete one-slot table for the actual stateless four-byte owner. Keep the
// runtime and its borrowed CRT pointees alive at stable addresses while used.
class NativeDynTerrainConvexRuntime final {
public:
    explicit NativeDynTerrainConvexRuntime(const CameraAxesCrtAccess&) noexcept;
    NativeDynTerrainConvexRuntime(const NativeDynTerrainConvexRuntime&)=delete;
    NativeDynTerrainConvexRuntime& operator=(const NativeDynTerrainConvexRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    std::uintptr_t methods_[1];
    CameraAxesCrtAccess crt_;
    static bool __fastcall intersect(void*,void*,void*,const void*,const float*,const void*,const float*);
};
} // namespace bsp
