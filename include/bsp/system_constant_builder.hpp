#pragma once
#include "bsp/camera_frame_state.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {
struct CameraAxesCrtAccess;
struct SystemTimeBindings;
struct SystemLightingScene;
class SystemInvalidParameterRuntime;

// Native00B46A70 reserves a local 77-register prefix and never clears it.
// The caller must supply an initialized preimage. Its values are an explicit
// host choice; this type does not synthesize native stack contents or defaults.
using SystemConstantPrefix = std::array<float, 77 * 4>;

struct SystemConstantBuilderEnvironment {
    SystemTimeBindings& time; // actual live clock/service/global owner slots
    const CameraAxesCrtAccess& axes_crt;
    const float* parameters_0108fc30; // actual sixteen consecutive global words
    D3D9StateCache* const volatile& renderer_00f8d394;
    const volatile std::uint32_t& register_count_00e13078;
    const std::uint32_t& exponent_00cfad80;
    SystemInvalidParameterRuntime* invalid_parameter_runtime{};
};

// Original ECX camera, EAX [camera+43C], RET. Borrowed optional float4.
const CameraPlane* get_camera_context_depth_scale_00b6feb0(
    const CameraFrameState&) noexcept;
//00B475B4..00B475FC: two actual pointer reads when initially nonnull, then
// four raw MOVSS read/store pairs. No clearing when absent, no scene gate.
bool write_system_context_constants_00b475b4(const CameraFrameState&,
    SystemConstantPrefix&, std::string&);
//00B475FD..00B4762D: count then renderer loaded independently for VS and PS.
// Zero count skips the native wrapper's guard/device/counters. A failed VS
// HRESULT never suppresses PS. Checked capacity/binding errors retain effects.
bool upload_system_constant_prefix_00b475fd(const SystemConstantPrefix&,
    D3D9StateCache* const volatile& renderer_00f8d394,
    const volatile std::uint32_t& count_00e13078,
    HRESULT& accumulated_device_result, std::string&);

//00B46A70: ECX optional scene, EDX camera; no stack args, RET. This typed
// body composes all ordered field/cache writers with the SAME actual owners.
// No native lifecycle, object/vtable ABI or global initialization is implied.
// Owner/runtime adapters remain required where selected. Their exceptions
// propagate with prior writes/callback effects retained; this is a new host API.
// device_result accumulates COM failures; false denotes checked host bindings.
bool build_and_upload_system_constants_00b46a70(const SystemLightingScene*,
    CameraFrameState&, SystemConstantPrefix& initialized_prefix,
    SystemConstantBuilderEnvironment&, HRESULT& device_result, std::string&);
}
