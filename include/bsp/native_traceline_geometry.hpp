#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/native_material_parameters.hpp"

namespace bsp {
struct TrackedCriticalSection;

struct NativeTracelineGeometryAccess {
    // Reuse the application's canonical actual mesh/section/material companions.
    // The existing class name does not create a separate GUI resource domain.
    GuiNativeGeometryOwners& geometry;
    NativeStringStorage& strings;
    NativeMaterialDestructionAccess& material_lifetime;
    NativeMaterialParameterAccess& parameters;
    const volatile std::uint32_t* material_vtable_00d5e520;
    TrackedCriticalSection* const volatile& lock_00f8c284;
    void* const volatile& renderer_00f8d394;
    const volatile float& unchanged_00d7a260;
    void* (__cdecl* allocate_array_00bf55be)(std::size_t);
};

// Complete normal AF3440..AF3748 sequence. Original ECX actual1BCh Traceline,
// stack(payload80h, actual root), RET8. This new C++ ABI receives its SAME live
// model companion and the existing root view; never cast a raw root pointer to
// RenderNodeRootList. Nonnull roots must come from the canonical root binding.
//
// Renderer CURRENT +5C must be a callable thiscall(count,flags,declaration)
// factory returning one real owned vertex stream, and +48 must implement the
// actual effect acquisition consumed by 535320. Numeric native table addresses
// are not callable host bindings. No fallback stream/effect or second registry.
// Live valid payload/resources and successful allocation form the supported
// domain. Native FH3/hardware-fault unwind and game ABI remain unverified.
void attach_native_traceline_geometry_00af3440(NativeModelOwner&,
    void* actual_payload80h, RenderNodeRootList*, NativeTracelineGeometryAccess&);
} // namespace bsp
