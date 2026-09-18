#pragma once
#include "bsp/native_dyn_shape_lifetime.hpp"
#include <cstdint>

namespace bsp {
// Complete normal support query342B, native EAX mesh,stack output/direction,
// RET8. Select a signed16 seed using the original CRT direction conversions,
// then climb vertex adjacency only when candidate > current + native epsilon.
// Mesh/adjacency/seed provenance and a valid nonempty graph are required.
// Preserve x87 spills, comparisons and actual mutable0109EEA4 conversion mode.
// Arbitrary nonfinite directions, invalid seeds/private stack aliases and
// native exception/register ABI are not supplied by these explicit interfaces.
void support_native_dyn_hull_00c358a0(const AvoidZoneDynHullData&,OceanVec3& output,
    const OceanVec3& direction,const volatile std::uint32_t& actual_0109eea4) noexcept;
// Complete279B/299B methods, ECX shape,three stack pointers,RET0C. Transform
// direction, query borrowed mesh210, transform result including translation.
// The double interface first narrows direction and final results to binary32.
void support_native_dyn_convex_shape_00c386e0(const DynConvexShapeStorage&,OceanVec3&,
    const OceanVec3&,const float* matrix12,const volatile std::uint32_t&) noexcept;
void support_native_dyn_convex_shape_00c385b0(const DynConvexShapeStorage&,double* output3,
    const double* direction3,const float* matrix12,const volatile std::uint32_t&) noexcept;

// Complete four-entry callable source table for this concrete class. Borrow
// the real convex pool and mutable CRT state for the entire lifetime of every
// shape using table(). Stable,noncopyable owner; no native RTTI/EH metadata.
// Bounds/scalar slots reuse C57C40/4062C0; support slots use the bodies above.
class NativeDynConvexShapeRuntime final {
public:
    NativeDynConvexShapeRuntime(DynConvexShapePoolStorage&,const volatile std::uint32_t&) noexcept;
    NativeDynConvexShapeRuntime(const NativeDynConvexShapeRuntime&)=delete;
    NativeDynConvexShapeRuntime& operator=(const NativeDynConvexShapeRuntime&)=delete;
    const void* table() const noexcept {return methods_;}
private:
    const std::uintptr_t methods_[4];
    DynConvexShapePoolStorage* const pool_;
    const volatile std::uint32_t* const conversion_;
    static const NativeDynConvexShapeRuntime& owner(const void*) noexcept;
    static void __fastcall bounds(void*,void*);
    static void* __fastcall scalar(void*,void*,std::uint32_t);
    static void __fastcall support32(void*,void*,OceanVec3*,const OceanVec3*,const float*) noexcept;
    static void __fastcall support64(void*,void*,double*,const double*,const float*) noexcept;
};
} // namespace bsp
