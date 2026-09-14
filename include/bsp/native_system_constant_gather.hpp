#pragma once
#include "bsp/native_camera_axes.hpp"
#include "bsp/native_sampler_loader_context.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"
#include <cstdint>

namespace bsp {
// Borrow actual publication cells, raw owners and current original numeric
// profiles. No CameraFrameState, D3D9StateCache or shadow/texture projection.
struct NativeSystemConstantGatherContext {
    void* const volatile* actual_service_00f8d39c;
    void* const volatile* actual_renderer_00f8d394;
    void* const volatile* actual_timer_01090ab0;
    void* volatile* actual_sampler_00f8d420;
    void* volatile* actual_manager_01090aa0;
    void* const volatile* actual_foliage_manager_00f8c274;
    void* const volatile* actual_foliage_00f8c210;
    const volatile std::uint32_t* actual_parameters_0108fc30;
    const volatile std::uint32_t* actual_register_count_00e13078;
    const volatile std::uint32_t* actual_exponent_00cfad80;
    const volatile float* actual_unsigned_bias_00ce3978;
    const NativeCameraAxesContext* actual_axes;
    NativeRendererSynchronizationGlobals* actual_synchronization;
    const volatile std::uint32_t* actual_timer_profile_00d68d50;
    void* const volatile* actual_shadow_target_00f8bbf0;
    const volatile std::uint32_t* actual_shadow_profile_00d5b5d8;
    const volatile std::uint32_t* actual_texture_profile_00d61948;
};

// One fresh persistent frame for each call. preimage points to 1232 initialized
// bytes supplied by the caller, representing native local prefix stack contents.
// It is copied bitwise before native writes; never default-cleared or synthesized.
// Keep the frame alive if the concrete sampler getter fails: its operation owns
// diagnostic obligations and must not be retried or implicitly discarded.
struct NativeSystemConstantGatherFrame {
    const void* prefix_preimage;
    NativeSamplerLoaderOperation sampler;
};

// Full B46A70..B47638, original ECX optional scene, EDX camera, RET. Source
// adds two stacked borrowed arguments/RET8. Every original branch and x87/SSE
// store is retained. Unwritten prefix words preserve the supplied preimage.
// Current count and renderer are separately reloaded for VS and PS. HRESULTs
// do not suppress the following upload. No outer rollback/cleanup is added.
// Accepted current profiles: timer D68D50/+1C=BEE070, shadow D5B5D8/+08=A8FCF0,
// and texture D61948/+3C=B3CE50/+40=B3CE60. Other targets fail at dispatch;
// no numeric game address is called as a host function pointer.
// Each dispatch uses its one original owner-profile capture and reads the
// selected borrowed table at the reached slot; it never reloads owner[0].
// Original private stack aliases, CRT/FH3/hardware-fault ABI and game validation
// remain outside this new source interface. Raw storage extents must be valid.
void __fastcall gather_native_system_constants_00b46a70(void* actual_scene,
    void* actual_camera, const NativeSystemConstantGatherContext&,
    NativeSystemConstantGatherFrame&);

void* __fastcall get_raw_shadow_depth_texture_00a8fcf0(const void* actual_shadow,
    void* const volatile* actual_global_target);
void* __fastcall get_raw_shadow_color_texture_00a8fd10(const void* actual_shadow,
    void* const volatile* actual_global_target);
void* __fastcall get_raw_shadow_target_color_00a8fd90(const void*) noexcept;
void* __fastcall get_raw_shadow_target_depth_00a8fdb0(const void*) noexcept;
std::uint32_t __fastcall get_raw_texture_width_00b3ce50(const void*) noexcept;
std::uint32_t __fastcall get_raw_texture_height_00b3ce60(const void*) noexcept;
float __fastcall get_raw_foliage_manager_time_00af0460(const void*);
float __fastcall get_raw_foliage_time_00ad5700(const void*);
std::uint8_t __fastcall get_raw_foliage_byte_00ad5740(const void*) noexcept;
void* __fastcall get_raw_light_shadow_00b7aab0(const void*) noexcept;
const void* __fastcall get_raw_timer_interval_00bee070(const void*) noexcept;
} // namespace bsp
