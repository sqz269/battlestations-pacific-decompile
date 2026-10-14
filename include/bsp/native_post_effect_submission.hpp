#pragma once
#include <cstdint>

namespace bsp {
struct NativeCameraFrameCommandContext;
struct NativeRendererFrameTargetsContext;
struct NativeLogicalBufferMappingContext;
struct NativeMaterialPassExecutionContext;
struct NativeMaterialPassExecutionFrame;

// Complete B6FDE0..B6FDE6: ECX actual camera, EAX current DWORD at +180h,
// plain RET. The address is borrowed and may be null; the camera storage must
// be readable. No validation, retention, viewport registration or copying.
// Pass the actual 458h camera region, never its NativeCameraOwner companion.
// This one-argument Win32 fastcall leaf preserves the physical ECX/EAX/RET
// behavior; whole-program ABI compatibility and gameplay remain unproved.
void* __fastcall get_native_camera_viewport_00b6fde0(
    const void* actual_camera) noexcept;

// Same immutable original scalar cells used by the native MOVSS loads. These
// are borrowed bit patterns, not computed/default geometry constants. Keeping
// current volatile loads at their native positions also preserves alias order.
struct NativePostEffectVertexConstants {
    const volatile std::uint32_t& word_00d7a260;
    const volatile std::uint32_t& word_00ce3800;
    const volatile std::uint32_t& word_00d7a24c;
    const volatile std::uint32_t& word_00ce3d50;
    const volatile std::uint32_t& word_00ce3850;
    const volatile std::uint32_t& word_00ce3854;
    const volatile std::uint32_t& word_00ce7d7c;
    const volatile std::uint32_t& word_00ce3958;
};

// Borrow the SAME actual renderer publication, synchronization, frame/surface,
// camera/viewport, logical/physical mapping and material execution domains.
// Original numeric profile words are current reads, never host callbacks.
struct NativePostEffectExecutionContext {
    const NativeCameraFrameCommandContext& camera;
    NativeRendererFrameTargetsContext& targets;
    NativeLogicalBufferMappingContext& mapping;
    NativeMaterialPassExecutionContext& materials;
    const volatile std::uint32_t* surface_profile_00d619a0;
    const volatile std::uint32_t* vertex_profile_00d61d6c;
    NativePostEffectVertexConstants vertices;
};
struct NativePostEffectDrawFrame {
    NativeMaterialPassExecutionFrame* material{};
    void* captured_stream{};
    void* captured_mapping{};
    std::uint32_t active_native_site{};
    bool started{};
    bool mapped{};
    bool vertices_written{};
    bool unmapped{};
    bool completed{};
    bool failed{};
};

// Full B4DCC0..B4DEEA[555]. ECX actual20h post effect; one UNUSED stacked
// DWORD, RET4. Read color0 twice; first surface height then second width.
// Current camera viewport dimensions; current renderer+A0; capture renderer
// and table before the next viewport getter, then captured+A4; current+98.
// Actual model geometry0/stream0; capture count before map, reread it for
// triangle(3)/quad stores. Real B49980/B49A80 bracket exact bit-copy/store order.
// After unlock, reload material/effect/pass and entry; genuine current pass08
// dispatch owns its existing persistent frame. No local EH/unmap/rollback.
void draw_native_post_effect_00b4dcc0(void* actual_post, std::uint32_t unused,
    NativePostEffectExecutionContext&, NativePostEffectDrawFrame&);

// Complete B4DFB0..B4DFBA[11]. ECX current pass owner, RET. Load owner+08,
// push unused0, call the full concrete B4DCC0 above. No ownership or fallback.
void draw_native_post_effect_owner_00b4dfb0(void* actual_owner,
    NativePostEffectExecutionContext&, NativePostEffectDrawFrame&);

// New Source context/frame ABI. Valid raw extents/producer-created owners and
// current D619A0/D61D6C/D5F0A8 plus established material profiles are required.
// Integer bit copies preserve native MOVSS/XORPS values and load/store order;
// original incidental XMM/register/private-stack/FH3/fault ABI is not claimed.
// Separate B4DEF0 submission remains unimplemented; its native call schedule
// is recorded in NATIVE_POST_EFFECT_SUBMISSION_BV.md.

} // namespace bsp
