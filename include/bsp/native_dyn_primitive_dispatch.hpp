#pragma once
#include "bsp/dyn_dispatch_initialization.hpp"

namespace bsp {
// Actual sphere/box storage: body pointer+4, kind+8, transform+34,
// radius+210 (sphere), half-extents+210/+214/+218 (box). Records and borrowed
// CRT state must remain valid throughout the call. Original results use AL.
// Collision output is count plus nine floats; ray output is six floats.
bool intersect_native_dyn_spheres_00c518d0(void* result,const void* shape_a,
    const float* matrix_a,const void* shape_b,const float* matrix_b,const CameraAxesCrtAccess&);
bool intersect_native_dyn_box_sphere_00c48330(void* result,const void* shape_a,
    const float* matrix_a,const void* shape_b,const float* matrix_b,const CameraAxesCrtAccess&);
bool intersect_native_dyn_sphere_ray_00c50740(void* result,const void* shape,
    const float* start,const float* end,const CameraAxesCrtAccess&);
bool intersect_native_dyn_box_ray_00c50dd0(void* result,const void* shape,
    const float* start,const float* end,const CameraAxesCrtAccess&);
// Existing float-sqrt boundary: original stack float, ST0 result, RET4.
// Preserve its explicit float store/reload after the recovered CRT ST0 service.
float sqrt_native_dyn_float_004011d0(float,const CameraAxesCrtAccess&);

// Four complete, independent one-slot tables for actual static dispatch owners.
// The runtime owns context copies, borrows CRT pointees, and must remain stable
// and alive while any owner or scene uses its tables. No invocation globals.
class NativeDynPrimitiveDispatchRuntime final {
public:
    explicit NativeDynPrimitiveDispatchRuntime(const CameraAxesCrtAccess&) noexcept;
    NativeDynPrimitiveDispatchRuntime(const NativeDynPrimitiveDispatchRuntime&)=delete;
    NativeDynPrimitiveDispatchRuntime& operator=(const NativeDynPrimitiveDispatchRuntime&)=delete;
    const void* sphere_sphere_table() const noexcept {return &sphere_sphere_.method;}
    const void* box_sphere_table() const noexcept {return &box_sphere_.method;}
    const void* sphere_ray_table() const noexcept {return &sphere_ray_.method;}
    const void* box_ray_table() const noexcept {return &box_ray_.method;}
private:
    struct Entry {std::uintptr_t method;const CameraAxesCrtAccess* crt;};
    static_assert(offsetof(Entry,method)==0);
    CameraAxesCrtAccess crt_;
    Entry sphere_sphere_,box_sphere_,sphere_ray_,box_ray_;
    static const CameraAxesCrtAccess& context(const void*) noexcept;
    static bool __fastcall sphere_sphere(void*,void*,void*,const void*,const float*,const void*,const float*);
    static bool __fastcall box_sphere(void*,void*,void*,const void*,const float*,const void*,const float*);
    static bool __fastcall sphere_ray(void*,void*,void*,const void*,const float*,const float*);
    static bool __fastcall box_ray(void*,void*,void*,const void*,const float*,const float*);
};
} // namespace bsp
