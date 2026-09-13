#pragma once
#include "bsp/legacy_crt_math.hpp"

namespace bsp {
// Borrow the owning runtime's actual CRT dispatch word and original constants.
// The complete C27489 provider is fixed; its LegacyCrtMathRuntime must already
// bind actual E16BD0 and the runtime's errno accessor. No binding is changed here.
struct NativeCameraAxesContext final {
    const CameraAxesCrtAccess length_access;
    const volatile std::uint32_t* const one_00d7a24c;
    const volatile std::uint32_t* const fallback_limit_00ce3800;
    NativeCameraAxesContext(const volatile std::uint32_t& actual_0109dd78,
        const volatile std::uint32_t& actual_00d7a24c,
        const volatile std::uint32_t& actual_00ce3800) noexcept
        : length_access{&actual_0109dd78, &legacy_crt_87except_00c27489},
          one_00d7a24c(&actual_00d7a24c),
          fallback_limit_00ce3800(&actual_00ce3800) {}
};
static_assert(offsetof(NativeCameraAxesContext, length_access) == 0);
static_assert(offsetof(NativeCameraAxesContext, one_00d7a24c) == 8);
static_assert(offsetof(NativeCameraAxesContext, fallback_limit_00ce3800) == 12);
static_assert(sizeof(NativeCameraAxesContext) == 16);

// Complete B70EA0/B70FE0: original ECX=actual camera, no stack arguments,
// EAX=camera+440/+44C, RET. EDX adds the concrete borrowed context in this API.
// Bit100 at +2F0 skips all work, including context reads. Otherwise refresh
// raw world storage through full B6DB70 if low-byte +5C bit2 is clear, derive
// both projected axes from current world+20 (camera+110), store via x87 and
// apply the native ordered length<current0.5 fallback before setting bit100.
// Required actual layout: raw node prefix (+30 actual parent pointer or zero,
// +5C flags, +B0 local64, +F0 world64), +2F0 flags and +440/+44C float3 caches.
// Pass actual camera storage, never a CameraFrameState/CameraTransform or an
// owner companion. No missing prefix is initialized, no cycle validation is
// added, and unrelated cache flags/bytes remain untouched.
float* __fastcall get_native_camera_axis_y_00b70ea0(
    void* actual_camera, const NativeCameraAxesContext*);
float* __fastcall get_native_camera_axis_x_00b70fe0(
    void* actual_camera, const NativeCameraAxesContext*);

// Complete B6FEB0: ECX=actual camera, EAX=raw borrowed pointer at +43C, RET.
// No retain, dereference, cache validation or null substitution. The optional
// float4 interpretation comes from B46A70's consumer; ownership is not inferred.
const void* __fastcall get_native_camera_context_00b6feb0(const void* actual_camera);

// Descriptive names remain hypotheses. Original stack/register instructions
// are documented separately from these explicit C++ entry/context interfaces;
// this does not claim a drop-in game binary or gameplay validation.
} // namespace bsp
