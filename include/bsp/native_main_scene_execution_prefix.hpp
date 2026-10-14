#pragma once
#include <cstdint>

namespace bsp {
struct NativeCameraAxesContext;
struct NativeCameraHistoryContext;
struct NativeCameraFarCornersContext;

struct NativeMainSceneExecutionPrefixContext {
    void* const volatile& actual_renderer_00f8d394;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    void* const volatile& actual_clock_01090ab0;
    const NativeCameraAxesContext& axes;
    const NativeCameraHistoryContext& history;
    const volatile double& direction_scale_00cec9e0;
    const volatile std::uint32_t& near_plane_00ce3800;
};

// Fresh persistent external continuation state, disjoint from actual native
// storage, for the ORIGINAL later restore. Do not reuse a completed state.
// near_bits is the original camera+1D4 word; color_word is the saved +190 word
// with its high byte replaced by FF exactly as B1573D..B15741. The existing
// service/camera mutations survive failure and are never rolled back here.
struct NativeMainSceneExecutionPrefixState {
    std::uint32_t near_bits{};
    std::uint32_t color_word{};
    bool frame_active{};
    bool near_changed{};
    bool reached_texture_stage{};
    bool reached_bloom_stage{};
};

// Complete raw leaves. Native ECX command / camera. B1BF40 uses RET and returns
// current [[command+28]+08]. B6FE60 has stack outputDWORD, RET4, EAX output.
void* __fastcall get_native_scene_command_camera_00b1bf40(const void* actual_command);
void* __fastcall get_native_camera_clear_color_00b6fe60(const void* actual_camera,
    void* unused_edx, void* actual_output_word);

// Genuine B156C0 entry through B1586C, ending BEFORE B1586D texture-holder
// acquisition. Includes current renderer slot2C frame-active gating, near/color
// capture, raw axes/velocity projection, full B0DB00 history, delayed VP copy,
// and current raw camera VP inverse/copy. Returns true only at that boundary;
// false means native frame-inactive exit. This is NOT whole command success.
//
// No renderer, clock, camera, history, owner or callable table is created. The
// raw service/command graph and the SAME actual CRT/constants must be supplied.
// Each B1BF40 call reloads the current camera; no camera snapshot is substituted.
// B17240 system-prefix preparation and the remaining B156C0 texture/target/
// batch/material/post stages are unresolved here. D5E13C must stay UNADMITTED
// in the existing queue execution domain. This API does not install a route.
bool execute_native_main_scene_prefix_00b156c0(void* actual_service,
    void* actual_command, NativeMainSceneExecutionPrefixContext&,
    NativeMainSceneExecutionPrefixState&);

struct NativeMainSceneTextureConstantsContext {
    const volatile std::uint32_t* actual_texture_profile_00d61948;
    const volatile float* unsigned_bias_00ce3978;
    const volatile double* half_00d7a280;
    const volatile std::uint32_t* negative_one_00d7a260;
    const volatile std::uint32_t* one_00d7a24c;
    const NativeCameraFarCornersContext* far_corners;
};

// Continue the SAME actual service/command and persistent prefix state through
// B1586D..B15BD1, ending BEFORE B15BD2 bloom selection. Repeated genuine B4CB10
// holder getters and captured D61948 current slots3C/40 use B3CE50/B3CE60.
// Retains unsigned x87 conversion, half/full texel stores, all nine XY offsets,
// and current-camera full B0EB10 far rays. No owner allocation or ref changes.
// Only the established valid profile/raw-domain contract is admitted; throwing
// across the naked arithmetic helper is not supported. Never whole-command
// success: D5E13C remains unadmitted and all later renderer stages are required.
void continue_native_main_scene_texture_constants_00b1586d(void* actual_service,
    void* actual_command, const NativeMainSceneTextureConstantsContext&,
    NativeMainSceneExecutionPrefixState&);
} // namespace bsp
