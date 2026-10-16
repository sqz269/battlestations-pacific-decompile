// cc13_fix8: segment-89 camera capture routines. See include/bsp/cc13_fix8.hpp and
// docs/CC13_FIX8.md. The stack offsets below are from the function's body ESP after its
// register pushes (00B83F30: S = the 64-byte-aligned EBP frame minus 1C0h, EH state at
// [S+1BCh]; 00B848C0: B, EH state at [B+74h]; 00B84A90: B, EH state at [B+70h]).

#include "bsp/cc13_fix8.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {

// FISTP QWORD under the truncating control word, then only the low dword is read.
std::uint32_t fistp_trunc_low32(double value) {
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
}

// FILD of a dword the code treats as unsigned: +2^32 (float 00CE3978) when the sign bit is
// set, then FSTP to a float.
float unsigned_to_float(std::uint32_t value) {
    return static_cast<float>(static_cast<double>(value));
}

// The inline NativeString release: when data is non-null, 00419CC0 then
// 00BD1510(data, length + 1, 1).
void release_string(CameraCaptureHost& host, const CameraCaptureString& text) {
    if (text.data != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_block_00bd1510(pool, text.data, text.length + 1u, 1u);
    }
}

// The reference release at object+4 (InterlockedDecrement, then vtable[0] at zero).
void release_reference(CameraCaptureHost& host, void* object) {
    if (host.interlocked_decrement_ce2220(object) == 0) {
        host.object_vslot0(object);
    }
}

constexpr double kWorldUnitsPerTexel = 9.375;    // double 00D5D410
constexpr float kEyeHeight = 10000.0f;           // float 00CE3D64
constexpr std::uint32_t kMaxSurfaceSide = 0x1000u;

}  // namespace

// 00B83F30. Uncertain: the renderer slot meanings (0Ch / 14h / 2Ch read as scene begin /
// end / in-scene) and 84h's last argument (2) are inferred from their use only. The
// unsigned fix-up of each FILD is exact for any dword, so unsigned_to_float covers it.
// The early return when the assembled surface is null (00B843D3) releases nothing and
// restores nothing (the tile surface, the frame targets reference, the camera, the clear
// flags and the zeroed fog scalars all stay), as in the image.
void* camera_render_to_frame_targets_00b83f30(const float* min_xz, const float* max_xz,
    float scale, void* camera, std::uint32_t format, void* targets_arg,
    CameraCaptureHost& host) {
    // 00B83F64..00B83F84: viewport width [S+40h], then height [S+6Ch].
    const std::uint32_t view_w =
        host.viewport_get_size_00b1f740(host.camera_get_viewport_00b6fde0(camera))[0];
    const std::uint32_t view_h =
        host.viewport_get_size_00b1f740(host.camera_get_viewport_00b6fde0(camera))[1];
    // 00B83F84..00B84000: DC F9 gives (max.x - min.x) / 9.375, then * scale; DE F1 gives
    // (max.z - min.z) / 9.375, then * scale; both truncated (FISTP under CW | 0C00h).
    const std::uint32_t width = fistp_trunc_low32(
        ((static_cast<double>(max_xz[0]) - static_cast<double>(min_xz[0])) /
            kWorldUnitsPerTexel) * static_cast<double>(scale));          // [S+5Ch]
    const std::uint32_t height = fistp_trunc_low32(
        ((static_cast<double>(max_xz[1]) - static_cast<double>(min_xz[1])) /
            kWorldUnitsPerTexel) * static_cast<double>(scale));          // [S+3Ch]

    // 00B84004..00B84054: the camera state saved for the restore.
    const float* const local = host.node_get_local_matrix_00b6db60(camera);
    const float saved_fov = host.camera_fov_1c4(camera);                 // [S+9Ch]
    const float saved_near = host.camera_near_1d4(camera);               // [S+74h]
    const float saved_far = host.camera_far_1d8(camera);                 // [S+2Ch]
    float saved_matrix[16];                                              // [S+140h]
    for (int i = 0; i < 16; ++i) {
        saved_matrix[i] = local[i];
    }
    const float saved_aspect = host.camera_aspect_1c8(camera);           // [S+98h]
    const float saved_fog68 =
        host.fog_load_scalar68_00b84ca0(host.camera_fog_owner_184(camera)); // [S+94h]
    const float saved_fog78 =
        host.fog_load_scalar78_00b84ce0(host.camera_fog_owner_184(camera)); // [S+88h]
    // 00B8407B..00B840A0: the viewport size again, for the tile surface.
    const std::uint32_t tile_w =
        host.viewport_get_size_00b1f740(host.camera_get_viewport_00b6fde0(camera))[0];
    const std::uint32_t tile_h =
        host.viewport_get_size_00b1f740(host.camera_get_viewport_00b6fde0(camera))[1];
    host.fog_set_scalar68_00b84d00(host.camera_fog_owner_184(camera), 0.0f);
    host.fog_set_scalar78_00b84d40(host.camera_fog_owner_184(camera), 0.0f);

    // 00B840C6..00B841BD: tile counts, ceil (00BFA7B0, the CRT ceil) of float ratios. The
    // x ratio divides the exact FILD of the width; the z ratio divides the stored float.
    const float view_wf = unsigned_to_float(view_w);                     // [S+60h]
    const float ratio_x =
        static_cast<float>(static_cast<double>(width) / static_cast<double>(view_wf));
    const std::uint32_t tiles_x = fistp_trunc_low32(
        static_cast<float>(std::ceil(static_cast<double>(ratio_x))));    // [S+24h]
    const float height_f = unsigned_to_float(height);                    // [S+7Ch]
    const float view_hf = unsigned_to_float(view_h);                     // [S+44h]
    const float ratio_z =
        static_cast<float>(static_cast<double>(height_f) / static_cast<double>(view_hf));
    const std::uint32_t tiles_z = fistp_trunc_low32(
        static_cast<float>(std::ceil(static_cast<double>(ratio_z))));    // [S+84h]
    const std::uint32_t saved_clear_flags = host.camera_clear_flags_188(camera); // [S+48h]
    const float units_per_pixel =
        static_cast<float>(kWorldUnitsPerTexel / static_cast<double>(scale)); // [S+68h/78h]
    host.camera_set_clear_flags_00b6fe10(camera, 7u);                    // 00B841DE

    // 00B841E3..00B842C8: orthographic projection at [S+0E8h] (DE F2, DE F9, DE EA, DE F2,
    // DE E2, DE F1 read from the bytes).
    float projection[16] = {};
    projection[0] = static_cast<float>(
        2.0 / (static_cast<double>(view_wf) * static_cast<double>(units_per_pixel)));
    projection[5] = static_cast<float>(
        2.0 / (static_cast<double>(view_hf) * static_cast<double>(units_per_pixel)));
    projection[10] = static_cast<float>(
        1.0 / (static_cast<double>(saved_far) - static_cast<double>(saved_near)));
    projection[14] = static_cast<float>(static_cast<double>(saved_near) /
        (static_cast<double>(saved_near) - static_cast<double>(saved_far)));
    projection[15] = 1.0f;                                               // 00D7A24C
    host.camera_set_projection_matrix_00b6fd60(camera, projection);

    // 00B842CD..00B84346: a caller's targets get a reference; otherwise a new 40h-byte
    // object (EH state 0 around 00B1FBB0) takes the renderer's colour surface 0 and depth
    // surface. No null test on the constructed object before the two setters.
    void* targets = nullptr;                                             // [S+54h]
    if (targets_arg != nullptr) {
        targets = targets_arg;
        host.interlocked_increment_ce221c(targets_arg);
    } else {
        void* const block = host.operator_new_00bf681b(0x40u);
        void* constructed = nullptr;
        if (block != nullptr) {
            constructed = host.frame_targets_construct_00b1fbb0(block);
        }
        void* const color = host.renderer_color_surface_vslot128(host.renderer_f8d394(), 0u);
        targets = constructed;
        host.frame_targets_set_color_00b1fab0(constructed, 0u, color);
        void* const depth = host.renderer_depth_surface_vslot12c(host.renderer_f8d394());
        host.frame_targets_set_depth_00b1fb00(constructed, depth);
    }

    // 00B8434B..00B84369: EDI = the renderer from here on.
    void* const renderer = host.renderer_f8d394();
    const bool was_in_scene = host.renderer_vslot2c(renderer);           // [S+1Eh]
    if (was_in_scene) {
        host.renderer_vslot14(renderer);
    }
    // 00B8436B..00B84389: halve both sides until neither exceeds 1000h (unsigned).
    std::uint32_t out_w = width;
    std::uint32_t out_h = height;
    std::uint32_t shift = 0;                                             // [S+80h]
    while (out_w > kMaxSurfaceSide || out_h > kMaxSurfaceSide) {
        ++shift;
        out_w >>= 1;
        out_h >>= 1;
    }
    void* const assembled =
        host.renderer_create_surface_vslot84(renderer, out_w, out_h, format, 2u); // [S+70h]
    void* const tile =
        host.renderer_create_surface_vslot84(renderer, tile_w, tile_h, format, 2u); // [S+20h]
    if (assembled == nullptr) {
        return nullptr;                                                  // 00B843D3
    }

    // 00B843EC..00B846C3: column i ([S+58h]) with pixel x [S+30h] += view_w; row j
    // ([S+64h]) with pixel y [S+38h], reset per column, advanced to y + view_h.
    std::uint32_t pixel_x = 0;
    for (std::uint32_t i = 0; i < tiles_x; ++i, pixel_x += view_w) {
        std::uint32_t pixel_y = 0;
        for (std::uint32_t j = 0; j < tiles_z; ++j) {
            // 00B84415..00B84447, once per column before the first row: the 0.5 (double
            // 00D7A280) kept in ST0 across the column is consumed here.
            const double x_offset = (static_cast<double>(static_cast<std::int32_t>(i)) *
                static_cast<double>(view_wf) + 0.5 * static_cast<double>(view_wf)) *
                static_cast<double>(units_per_pixel);                    // [S+128h]
            const double z_span =
                static_cast<double>(height_f) * static_cast<double>(units_per_pixel); // [S+130h]
            const double half_view_h = 0.5 * static_cast<double>(view_hf);  // [S+138h]
            // 00B8444E..00B84509: eye (x, 10000, z), target (x, 0, z), up (0, 0, 1).
            const float x = static_cast<float>(static_cast<double>(min_xz[0]) + x_offset);
            const float z = static_cast<float>((z_span -
                (static_cast<double>(static_cast<std::int32_t>(j)) *
                    static_cast<double>(view_hf) + half_view_h) *
                    static_cast<double>(units_per_pixel)) + static_cast<double>(min_xz[1]));
            const float eye[3] = {x, kEyeHeight, z};                     // [S+0DCh]
            const float target[3] = {x, 0.0f, z};                        // [S+0D0h]
            const float up[3] = {0.0f, 0.0f, 1.0f};                      // [S+0C4h]
            host.camera_set_eye_and_target_00b70200(camera, eye, target, up);
            host.renderer_vslot0c(renderer);
            // 00B84517..00B84550: the by-value label "X" (00CE9A38), built inline as
            // 0041DD40(1, 1) and memcpy(data, "X", length + 1) when data is non-null.
            CameraCaptureString label{};
            host.native_string_resize_0041dd40(label, 1u, 1u);
            if (label.data != nullptr) {
                std::memcpy(label.data, "X", label.length + 1u);
            }
            void* const scene = host.camera_scene_a4(camera);
            void* const queue = host.render_queue_004c11f0();
            host.render_queue_submit_scene_00b1f4d0(queue, scene, camera, targets, label);
            host.renderer_vslot14(renderer);
            // 00B8458B..00B845A9: the argument [EBP+14h], not [S+54h], picks the source.
            void* const source = targets_arg != nullptr
                ? host.frame_targets_get_color_00b1f6d0(targets_arg, 0u)
                : host.renderer_color_surface_vslot128(host.renderer_f8d394(), 0u);
            // 00B845AB..00B84650: rectangles, unsigned minima, destination shifted.
            const std::uint32_t right =
                pixel_x + view_w < width ? pixel_x + view_w : width;
            const std::uint32_t next_y = pixel_y + view_h;               // [S+34h]
            const std::uint32_t bottom = next_y < height ? next_y : height;
            const CameraCaptureRect destination{pixel_x >> shift, pixel_y >> shift,
                right >> shift, bottom >> shift};                        // [S+0A0h]
            const CameraCaptureRect source_rect{0u, 0u, view_w < width ? view_w : width,
                view_h < height ? view_h : height};                      // [S+0B4h]
            host.renderer_get_render_target_data_00b1ff70(renderer, source, tile);
            host.surface_copy_vslot38(assembled, tile, &source_rect, &destination);
            pixel_y = next_y;
        }
    }

    // 00B846CB..00B84792: restore.
    if (was_in_scene) {
        host.renderer_vslot0c(renderer);
    }
    if (tile != nullptr) {
        host.surface_vslot4(tile, 1u);
    }
    if (targets != nullptr) {
        release_reference(host, targets);
    }
    host.camera_set_local_matrix_vslot38(camera, saved_matrix);
    host.camera_set_field_of_view_00b6fbb0(camera, saved_fov);
    host.camera_set_near_plane_00b6fbf0(camera, saved_near);
    host.camera_set_far_plane_00b6fc10(camera, saved_far);
    host.camera_set_aspect_ratio_00b6fbd0(camera, saved_aspect);
    host.camera_set_clear_flags_00b6fe10(camera, saved_clear_flags);
    host.fog_set_scalar68_00b84d00(host.camera_fog_owner_184(camera), saved_fog68);
    host.fog_set_scalar78_00b84d40(host.camera_fog_owner_184(camera), saved_fog78);
    return assembled;
}

// 00B847C0: format 15h and null targets are pushed for 00B83F30 (00B847E1..00B847E3).
bool camera_save_tiled_screenshot_bmp_00b847c0(const float* min_xz, const float* max_xz,
    float scale, void* camera, const CameraCaptureString* name, CameraCaptureHost& host) {
    void* const surface = camera_render_to_frame_targets_00b83f30(
        min_xz, max_xz, scale, camera, 0x15u, nullptr, host);
    if (surface == nullptr) {
        return false;                                                    // 00B8489D
    }
    CameraCaptureString suffix{};                                        // "_.bmp" 00D63170
    host.native_string_assign_0041e870(suffix, "_.bmp");
    CameraCaptureString path{};
    const CameraCaptureString* const joined =
        host.native_string_concat_004261a0(name, &path, &suffix);
    host.surface_save_vslot34(surface, joined, 0u);
    release_string(host, path);
    release_string(host, suffix);
    host.surface_vslot4(surface, 1u);                                    // 00B84887
    return true;
}

// 00B848C0. ESI = the renderer, read once (00B84901). Format 16h (D3DFMT_X8R8G8B8 if the
// slot forwards a D3DFORMAT; 00B847C0 passes 15h).
void camera_save_screenshot_bmp_00b848c0(void* camera, const CameraCaptureString* name,
    void* targets, CameraCaptureHost& host) {
    const std::uint32_t width =
        host.viewport_get_size_00b1f740(host.camera_get_viewport_00b6fde0(camera))[0];
    const std::uint32_t height =
        host.viewport_get_size_00b1f740(host.camera_get_viewport_00b6fde0(camera))[1];
    void* const renderer = host.renderer_f8d394();
    void* const surface =
        host.renderer_create_surface_vslot84(renderer, width, height, 0x16u, 2u);
    if (surface == nullptr) {
        return;                                                          // 00B84921
    }
    host.renderer_vslot0c(renderer);
    CameraCaptureString label{};                                         // "X" 00CE9A38
    host.native_string_assign_0041e870(label, "X");
    void* const scene = host.camera_scene_a4(camera);
    void* const queue = host.render_queue_004c11f0();
    host.render_queue_submit_scene_00b1f4d0(queue, scene, camera, targets, label);
    host.renderer_vslot14(renderer);
    alignas(4) unsigned char surface_object[0x34];                       // [B+38h]
    host.d3d9_surface_construct_00b3f450(surface_object);
    host.renderer_capture_vslot7c(renderer, surface_object);
    const CameraCaptureRect rect{0u, 0u, width, height};                 // [B+28h]
    host.surface_copy_vslot38(surface, surface_object, &rect, &rect);
    CameraCaptureString suffix{};                                        // ".bmp" 00D63178
    host.native_string_assign_0041e870(suffix, ".bmp");
    CameraCaptureString path{};
    const CameraCaptureString* const joined =
        host.native_string_concat_004261a0(name, &path, &suffix);
    host.surface_save_vslot34(surface, joined, 0u);
    release_string(host, path);
    release_string(host, suffix);
    release_reference(host, surface);
    host.d3d9_surface_destroy_00b3f4e0(surface_object);
}

// 00B84A90: 00B848C0 with a 100h x 100h surface; ESI = the renderer, read first.
void camera_save_screenshot_bmp_256_00b84a90(void* camera, const CameraCaptureString* name,
    void* targets, CameraCaptureHost& host) {
    void* const renderer = host.renderer_f8d394();
    void* const surface =
        host.renderer_create_surface_vslot84(renderer, 0x100u, 0x100u, 0x16u, 2u);
    if (surface == nullptr) {
        return;                                                          // 00B84AD6
    }
    host.renderer_vslot0c(renderer);
    CameraCaptureString label{};
    host.native_string_assign_0041e870(label, "X");
    void* const scene = host.camera_scene_a4(camera);
    void* const queue = host.render_queue_004c11f0();
    host.render_queue_submit_scene_00b1f4d0(queue, scene, camera, targets, label);
    host.renderer_vslot14(renderer);
    alignas(4) unsigned char surface_object[0x34];                       // [B+34h]
    host.d3d9_surface_construct_00b3f450(surface_object);
    host.renderer_capture_vslot7c(renderer, surface_object);
    const CameraCaptureRect rect{0u, 0u, 0x100u, 0x100u};                // [B+24h]
    host.surface_copy_vslot38(surface, surface_object, &rect, &rect);
    CameraCaptureString suffix{};
    host.native_string_assign_0041e870(suffix, ".bmp");
    CameraCaptureString path{};
    const CameraCaptureString* const joined =
        host.native_string_concat_004261a0(name, &path, &suffix);
    host.surface_save_vslot34(surface, joined, 0u);
    release_string(host, path);
    release_string(host, suffix);
    release_reference(host, surface);
    host.d3d9_surface_destroy_00b3f4e0(surface_object);
}

}  // namespace bsp
