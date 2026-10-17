#pragma once
// cc13_fix8: the segment-89 camera capture routines (00B83F30, 00B847C0, 00B848C0,
// 00B84A90). Evidence, ABI and uncertainty per function: docs/CC13_FIX8.md.
//
// Every engine call is a named CameraCaptureHost hook invoked at the binary's point with
// the binary's arguments; the renderer, surfaces, frame targets and the camera are opaque.
// None of the four has a caller in src, and 00B847C0 / 00B84A90 / 00B83F30 have no
// reference in the image other than 00B847C0 -> 00B83F30 (00B848C0 is called from
// 004DBF45). Names are hypotheses, not recovered symbols. Reconstructed and build-tested
// only; not fixture- or game-validated.

#include <cstdint>

namespace bsp {

// The actual 8-byte NativeString header: +0 length, +4 data (null when empty).
struct CameraCaptureString {
    std::uint32_t length{0};
    char* data{nullptr};
};

// A RECT as the surface copy vslot+38h reads it: left, top, right, bottom.
struct CameraCaptureRect {
    std::uint32_t left{0};
    std::uint32_t top{0};
    std::uint32_t right{0};
    std::uint32_t bottom{0};
};

struct CameraCaptureHost {
    virtual ~CameraCaptureHost() = default;

    // Camera (all __thiscall, ECX = camera).
    // 00B6FDE0 BSP_Camera_GetViewport -> viewport; 00B1F740 BSP_Viewport_GetSize(ECX =
    // viewport) -> pointer to {width, height}.
    virtual void* camera_get_viewport_00b6fde0(void* camera) = 0;
    virtual const std::uint32_t* viewport_get_size_00b1f740(void* viewport) = 0;
    // 00B6DB60 BSP_Node_GetLocalMatrix -> 16 floats.
    virtual const float* node_get_local_matrix_00b6db60(void* camera) = 0;
    // Inline reads: [camera+0A4h] scene, [+184h] fog owner, [+188h] clear flags,
    // [+1C4h] field of view, [+1C8h] aspect, [+1D4h] near, [+1D8h] far.
    virtual void* camera_scene_a4(void* camera) = 0;
    virtual void* camera_fog_owner_184(void* camera) = 0;
    virtual std::uint32_t camera_clear_flags_188(void* camera) = 0;
    virtual float camera_fov_1c4(void* camera) = 0;
    virtual float camera_aspect_1c8(void* camera) = 0;
    virtual float camera_near_1d4(void* camera) = 0;
    virtual float camera_far_1d8(void* camera) = 0;
    // 00B6FE10 SetClearFlags, 00B6FD60 SetProjectionMatrix(const float[16]),
    // 00B70200 SetEyeAndTarget(eye, target, up) RET 0Ch, 00B6FBB0 SetFieldOfView,
    // 00B6FBF0 SetNearPlane, 00B6FC10 SetFarPlane, 00B6FBD0 SetAspectRatio (RET 4 each),
    // and camera->vtable[38h](const float[16]) (restores the local matrix).
    virtual void camera_set_clear_flags_00b6fe10(void* camera, std::uint32_t flags) = 0;
    virtual void camera_set_projection_matrix_00b6fd60(void* camera, const float* matrix) = 0;
    virtual void camera_set_eye_and_target_00b70200(void* camera, const float* eye,
        const float* target, const float* up) = 0;
    virtual void camera_set_field_of_view_00b6fbb0(void* camera, float value) = 0;
    virtual void camera_set_near_plane_00b6fbf0(void* camera, float value) = 0;
    virtual void camera_set_far_plane_00b6fc10(void* camera, float value) = 0;
    virtual void camera_set_aspect_ratio_00b6fbd0(void* camera, float value) = 0;
    virtual void camera_set_local_matrix_vslot38(void* camera, const float* matrix) = 0;

    // Fog owner [camera+184h]: 00B84CA0 / 00B84CE0 return ST0 (stored as float);
    // 00B84D00 / 00B84D40 take one float, RET 4.
    virtual float fog_load_scalar68_00b84ca0(void* fog) = 0;
    virtual float fog_load_scalar78_00b84ce0(void* fog) = 0;
    virtual void fog_set_scalar68_00b84d00(void* fog, float value) = 0;
    virtual void fog_set_scalar78_00b84d40(void* fog, float value) = 0;

    // Renderer [00F8D394] and its vtable slots (__thiscall, ECX = renderer).
    virtual void* renderer_f8d394() = 0;
    virtual void renderer_vslot0c(void* renderer) = 0;   // no arguments (scene begin?)
    virtual void renderer_vslot14(void* renderer) = 0;   // no arguments (scene end?)
    virtual bool renderer_vslot2c(void* renderer) = 0;   // -> AL (inside a scene?)
    // vtable[7Ch](surface object), RET 4: fills the local 00B3F450 surface.
    virtual void renderer_capture_vslot7c(void* renderer, void* surface_object) = 0;
    // vtable[84h](width, height, format, 2), RET 10h -> surface (null on failure).
    virtual void* renderer_create_surface_vslot84(void* renderer, std::uint32_t width,
        std::uint32_t height, std::uint32_t format, std::uint32_t arg_4) = 0;
    // vtable[128h](index) -> colour surface; vtable[12Ch]() -> depth surface.
    virtual void* renderer_color_surface_vslot128(void* renderer, std::uint32_t index) = 0;
    virtual void* renderer_depth_surface_vslot12c(void* renderer) = 0;
    // 00B1FF70 BSP_D3D9Renderer_GetRenderTargetData(ECX = renderer; source, destination),
    // RET 8.
    virtual void renderer_get_render_target_data_00b1ff70(void* renderer, void* source,
        void* destination) = 0;

    // Surfaces.
    // surface->vtable[38h](source surface, const RECT* source, const RECT* destination),
    // RET 0Ch; surface->vtable[34h](const NativeString* path, 0), RET 8;
    // surface->vtable[4](1), RET 4.
    virtual void surface_copy_vslot38(void* surface, void* source,
        const CameraCaptureRect* source_rect, const CameraCaptureRect* destination_rect) = 0;
    virtual void surface_save_vslot34(void* surface, const CameraCaptureString* path,
        std::uint32_t arg_2) = 0;
    virtual void surface_vslot4(void* surface, std::uint32_t arg) = 0;
    // 00B3F450 BSP_D3D9Surface_ConstructDefault / 00B3F4E0 BSP_NativeSurfaceOwner_Destroy
    // on a 34h-byte stack object (ECX = object).
    virtual void d3d9_surface_construct_00b3f450(void* object) = 0;
    virtual void d3d9_surface_destroy_00b3f4e0(void* object) = 0;

    // Reference counts at object+4: InterlockedIncrement [00CE221C] /
    // InterlockedDecrement [00CE2220] (-> the new count), and object->vtable[0]() with
    // no stack argument when the decrement reaches zero.
    virtual long interlocked_increment_ce221c(void* object) = 0;
    virtual long interlocked_decrement_ce2220(void* object) = 0;
    virtual void object_vslot0(void* object) = 0;

    // Frame targets: operator new (00BF681B, cdecl) then 00B1FBB0 BSP_FrameTargets_Construct
    // (ECX = block) -> the object; 00B1FAB0 SetColorSurface(index, surface) RET 8;
    // 00B1FB00 SetDepthSurface(surface) RET 4; 00B1F6D0 GetColorSurface(index) RET 4.
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void* frame_targets_construct_00b1fbb0(void* block) = 0;
    virtual void frame_targets_set_color_00b1fab0(void* targets, std::uint32_t index,
        void* surface) = 0;
    virtual void frame_targets_set_depth_00b1fb00(void* targets, void* surface) = 0;
    virtual void* frame_targets_get_color_00b1f6d0(void* targets, std::uint32_t index) = 0;

    // 004C11F0 BSP_RenderCommandQueue_GetSingleton; 00B1F4D0 SubmitScene(ECX = queue;
    // scene, camera, targets, NativeString label BY VALUE), RET 14h: the callee owns and
    // frees the label (the callers set their EH state to -1 before the call).
    virtual void* render_queue_004c11f0() = 0;
    virtual void render_queue_submit_scene_00b1f4d0(void* queue, void* scene, void* camera,
        void* targets, CameraCaptureString label) = 0;

    // NativeString helpers: 0041E870 (ECX = header; const char*) RET 4, constructing the
    // header from text; 0041DD40 BSP_NativeString_Resize(ECX = header; length, preserve)
    // RET 8; 004261A0 BSP_NativeString_Concat(ECX = left; out, right) RET 8 -> out;
    // a release is 00419CC0 (pool singleton) then 00BD1510 ReturnBlock(ECX = pool; data,
    // length + 1, 1) RET 0Ch, skipped when data is null.
    virtual void native_string_assign_0041e870(CameraCaptureString& header,
        const char* text) = 0;
    virtual void native_string_resize_0041dd40(CameraCaptureString& header,
        std::uint32_t length, std::uint32_t preserve) = 0;
    virtual CameraCaptureString* native_string_concat_004261a0(const CameraCaptureString* left,
        CameraCaptureString* out, const CameraCaptureString* right) = 0;
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size,
        std::uint32_t arg_3) = 0;
};

// 00B83F30, ECX = const float min_xz[2], EDX = const float max_xz[2], stack (float scale,
// camera, format, targets), EBP frame, RET 10h -> EAX = the assembled surface or null.
// Renders the world rectangle top-down in viewport-sized tiles and copies each tile into
// one surface (halved until both sides are <= 1000h).
void* camera_render_to_frame_targets_00b83f30(const float* min_xz, const float* max_xz,
    float scale, void* camera, std::uint32_t format, void* targets, CameraCaptureHost& host);

// 00B847C0 (no Ghidra function; after 00B83F30's padding), ECX/EDX passed through as
// min_xz / max_xz, stack (float scale, camera, const NativeString* name), RET 0Ch -> AL.
// 00B83F30(..., 15h, null), then saves "<name>_.bmp" and releases the surface.
bool camera_save_tiled_screenshot_bmp_00b847c0(const float* min_xz, const float* max_xz,
    float scale, void* camera, const CameraCaptureString* name, CameraCaptureHost& host);

// 00B848C0, ECX = camera, EDX = const NativeString* name, stack targets, RET 4.
// Viewport-sized X8R8G8B8 capture saved as "<name>.bmp".
void camera_save_screenshot_bmp_00b848c0(void* camera, const CameraCaptureString* name,
    void* targets, CameraCaptureHost& host);

// 00B84A90, ECX = camera, EDX = const NativeString* name, stack targets, RET 4.
// The same with a fixed 100h x 100h surface and no viewport query.
void camera_save_screenshot_bmp_256_00b84a90(void* camera, const CameraCaptureString* name,
    void* targets, CameraCaptureHost& host);

}  // namespace bsp
