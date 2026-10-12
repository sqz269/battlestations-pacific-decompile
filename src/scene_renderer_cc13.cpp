// Scene-renderer object [00F8D39C] leaves, segment 82 render half (packet
// cc13_scene_renderer). Evidence, ABI and uncertainty:
// include/bsp/scene_renderer_cc13.hpp and docs/CC13_SCENE_RENDERER.md.

#include "bsp/scene_renderer_cc13.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace bsp::scene_renderer {
namespace {

template <typename T>
T rd(const void* base, std::uint32_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::uint8_t*>(base) + offset, sizeof value);
    return value;
}

template <typename T>
void wr(void* base, std::uint32_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(base) + offset, &value, sizeof value);
}

}  // namespace

void set_sight_blur_00b0cf80(void* service, std::uint32_t a, float amount) {
    wr<std::uint32_t>(service, 0x250, a);
    wr<float>(service, 0x254, amount);
    // B0CF8A..B0CFB6: FCOMPI 0.0001 vs amount, JBE sets the flag (unordered too).
    wr<std::uint8_t>(service, 0x21b, 0.0001 > static_cast<double>(amount) ? 0 : 1);
}

void get_transition_00b0cfc0(const void* service, float* start, float* end, float* duration) {
    *start = rd<float>(service, 0x238);
    *duration = rd<float>(service, 0x240);
    *end = static_cast<float>(static_cast<double>(rd<float>(service, 0x23c)) * rd<float>(service, 0x240) +
                              rd<float>(service, 0x238));
}

void set_transition_00b0d6a0(void* service, float start, float end, float duration, std::uint32_t a,
                             std::uint32_t b, std::uint32_t c) {
    wr<float>(service, 0x238, start);
    wr<float>(service, 0x240, duration);
    if (duration > 0.0f) {
        wr<float>(service, 0x23c, static_cast<float>((static_cast<double>(end) - start) / duration));
    } else {
        wr<float>(service, 0x23c, 1.0f);
    }
    wr<std::uint32_t>(service, 0x244, a);
    wr<std::uint32_t>(service, 0x248, b);
    wr<std::uint32_t>(service, 0x24c, c);
}

void set_lens_effect_00b0d020(void* service, std::uint8_t enabled, std::uint32_t a, std::uint32_t b,
                              std::uint32_t c, std::uint32_t d, std::uint32_t e) {
    wr<std::uint32_t>(service, 0x258, a);
    wr<std::uint32_t>(service, 0x25c, b);
    wr<std::uint32_t>(service, 0x260, c);
    wr<std::uint32_t>(service, 0x264, d);
    wr<std::uint8_t>(service, 0x21a, enabled);
    wr<std::uint32_t>(service, 0x268, e);
}

void set_film_effect_00b0d080(void* service, std::uint32_t value) { wr<std::uint32_t>(service, 0x220, value); }

void set_value_224_00b0d0b0(void* service, std::uint32_t value) { wr<std::uint32_t>(service, 0x224, value); }

void set_flag_21c_00b0d160(void* service, std::uint8_t value) { wr<std::uint8_t>(service, 0x21c, value); }

void attach_to_listener_00b0d170(void* service, void* a, void* b, SceneRendererHost& host) {
    if (void* listener = rd<void*>(service, 0x30); listener != nullptr) {
        host.listener_attach_00b4ef90(listener, a, b);
    }
}

void invalidate_listener_root_00b0d180(void* service, SceneRendererHost& host) {
    if (void* listener = rd<void*>(service, 0x30); listener != nullptr) {
        host.listener_invalidate_root_00b4ec90(listener);
    }
}

void flush_listener_00b0d190(void* service, SceneRendererHost& host) {
    if (void* listener = rd<void*>(service, 0x30); listener != nullptr) {
        host.listener_flush_00b4f030(listener);
    }
}

void set_motion_blur_and_samples_00b0d580(void* service, std::uint8_t motion_blur, std::int32_t samples,
                                          SceneRendererHost& host) {
    wr<std::uint8_t>(service, 0x219, motion_blur);
    std::uint8_t unused[8];
    host.device_query_slot80(unused);
    if (void* t = rd<void*>(service, 0x44)) host.texture_set_samples_00b4d270(t, samples);
    if (void* t = rd<void*>(service, 0x48)) host.texture_set_samples_00b4d270(t, samples);
    if (void* shadow = rd<void*>(service, 0x64); shadow != nullptr &&
                                                 host.shadow_map_texture_holder_00b4d170(shadow) != nullptr) {
        // B0D5D4..B0D5DF: the holder is fetched a second time for the call.
        host.texture_set_samples_00b4d270(host.shadow_map_texture_holder_00b4d170(rd<void*>(service, 0x64)), samples);
    }
    if (void* t = rd<void*>(service, 0x50)) host.target_set_samples_00b3ce20(t, samples);
    if (void* t = rd<void*>(service, 0x58)) host.target_set_samples_00b3ce20(t, samples);
}

void execute_scene_command_00b17240(void* command, void* service, SceneRendererHost& host) {
    if (!host.can_execute_00b20240()) {
        return;
    }
    const std::uint32_t prefix = host.command_prefix_00b1bf40(command);
    host.upload_system_prefix_00b46a70(rd<void*>(command, 0x04), prefix);
    host.device_slot114(static_cast<std::uint8_t*>(command) + 0x1c);
    host.render_scene_00b156c0(service, command);
}

bool record_name_less_00b0daa0(const void* a, const void* b) {
    if (rd<std::int32_t>(a, 0x124) == 0) {
        return rd<std::int32_t>(b, 0x124) != 0;  // NEG/SBB then SETL
    }
    if (rd<std::int32_t>(b, 0x124) == 0) {
        return false;  // 1 < 0
    }
    return _stricmp(rd<const char*>(a, 0x128), rd<const char*>(b, 0x128)) < 0;  // 00BF7FBF
}

NativeMaterialParameterStorage* register_native_material_float3_00b18ae0(NativeMaterialStorage& material,
                                                                         const void* name_header,
                                                                         const void* source,
                                                                         NativeMaterialParameterAccess& access) {
    return register_native_material_parameter_00b17e10(material, name_header, source, 3, 0, access);
}

NativeMaterialParameterStorage* register_native_material_matrix_00b18b40(NativeMaterialStorage& material,
                                                                         const void* name_header,
                                                                         const void* source,
                                                                         NativeMaterialParameterAccess& access) {
    return register_native_material_parameter_00b17e10(material, name_header, source, 16, 1, access);
}

// ---------------------------------------------------------------------------
// Packet cc13_scene_spine.
// ---------------------------------------------------------------------------

namespace {

std::uint8_t* at(void* base, std::uint32_t offset) { return static_cast<std::uint8_t*>(base) + offset; }

bool flag(const void* base, std::uint32_t offset) { return rd<std::uint8_t>(base, offset) != 0; }

void refresh_world(void* node, SceneRenderFrameHost& host) {
    if ((rd<std::uint8_t>(node, 0x5c) & 2) == 0) {
        host.transform_refresh_world_00b6db70(node);
    }
}

// FLDZ/FCOMI then COMISS against 1.0: 0 > v gives 0, v > 1 gives 1, NaN passes.
float clamp_unit(float v) {
    if (0.0f > v) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

// Textures report unsigned sizes (FILD plus 2^32 [00CE3978] when negative);
// x87 extended division, one float rounding (double here).
float over_size(double numerator, std::uint32_t size) {
    return static_cast<float>(numerator / static_cast<double>(size));
}

void set_slot(void* pass, std::uint32_t slot, void* texture, SceneRenderFrameHost& host) {
    host.material_set_texture_00b189f0(host.pass_material_00b4cba0(pass), slot, texture);
}

// B15F63..B16066 and B161F5..B162FC: the two batch lists command+0Ch (index 0)
// and +10h (index 1). Over 50 batches in both and guard mode 0 runs the
// preparation on the frame job pool and waits; otherwise each list prepares
// itself (vtable +0Ch). The lists execute (vtable +8h) in order either way.
void execute_batch_lists(void* command, SceneRenderFrameHost& host) {
    if (host.renderer_guard_mode_0108d6dc() == 0 && host.batch_count_00b51b20(rd<void*>(command, 0x0c)) > 0x32 &&
        host.batch_count_00b51b20(rd<void*>(command, 0x10)) > 0x32) {
        wr<std::uint32_t>(rd<void*>(command, 0x0c), 0x8, 0);
        host.frame_job_pool_submit(rd<void*>(command, 0x0c));
        wr<std::uint32_t>(rd<void*>(command, 0x10), 0x8, 1);
        host.frame_job_pool_submit(rd<void*>(command, 0x10));
        host.frame_job_pool_wait_slot8(1);
    } else {
        host.batch_list_prepare_slotc(rd<void*>(command, 0x0c), 0);
        host.batch_list_execute_slot8(rd<void*>(command, 0x0c), 0, host.command_camera_00b1bf40(command));
        host.batch_list_prepare_slotc(rd<void*>(command, 0x10), 1);
        host.batch_list_execute_slot8(rd<void*>(command, 0x10), 1, host.command_camera_00b1bf40(command));
        return;
    }
    host.batch_list_execute_slot8(rd<void*>(command, 0x0c), 0, host.command_camera_00b1bf40(command));
    host.batch_list_execute_slot8(rd<void*>(command, 0x10), 1, host.command_camera_00b1bf40(command));
}

// Camera render mode 0 selects if0, 3 selects if3, anything else is kept.
void select_render_mode(void* command, std::int32_t if0, std::int32_t if3, SceneRenderFrameHost& host) {
    const std::int32_t mode = rd<std::int32_t>(host.command_camera_00b1bf40(command), 0x198);
    if (mode == 0) {
        host.camera_set_render_mode_00b6fdf0(host.command_camera_00b1bf40(command), if0);
    } else if (rd<std::int32_t>(host.command_camera_00b1bf40(command), 0x198) == 3) {
        host.camera_set_render_mode_00b6fdf0(host.command_camera_00b1bf40(command), if3);
    }
}

void upload_prefix(void* command, SceneRenderFrameHost& host) {
    void* camera = host.command_camera_00b1bf40(command);
    host.upload_system_prefix_00b46a70(rd<void*>(camera, 0xa4), host.command_camera_00b1bf40(command));
}

// Float3 lerp of BSP_Float3_LerpRounded (005803E0): difference, product and sum
// each rounded to float.
void lerp3(const float* from, const float* to, float f, float* out) {
    for (int i = 0; i < 3; ++i) {
        const float d = to[i] - from[i];
        const float p = d * f;
        out[i] = from[i] + p;
    }
}

// B0E3B7..B0E42D pattern: squares rounded to float, summed in the listed order,
// rounded, square-rooted (_CIsqrt), rounded; 1/len stored as float, 0 when
// len <= 0 (or NaN, which takes the JBE).
float inverse_length(float s0, float s1, float s2) {
    const float a = s0 * s0;
    const float b = s1 * s1;
    const float c = s2 * s2;
    const float sum = static_cast<float>(static_cast<double>(a) + b + c);
    const float len = static_cast<float>(std::sqrt(static_cast<double>(sum)));
    if (!(len > 0.0f)) return 0.0f;
    return static_cast<float>(1.0 / len);
}

// a*b - c*d with products exact in double, one rounding to float.
float cross_term(float a, float b, float c, float d) {
    return static_cast<float>(static_cast<double>(a) * b - static_cast<double>(c) * d);
}

}  // namespace

void set_hdr_params_00b0e9b0(void* service, const std::uint32_t* block) {
    for (std::uint32_t i = 0; i < 9; ++i) {
        wr<std::uint32_t>(service, 0x84 + 4 * i, block[i]);
    }
    wr<std::uint8_t>(service, 0xa8, static_cast<std::uint8_t>(block[9] & 0xff));
    wr<std::uint32_t>(service, 0xac, block[10]);
}

void set_motion_blur_params_00b0ea20(void* service, const std::uint32_t* block) {
    for (std::uint32_t i = 0; i < 8; ++i) {
        wr<std::uint32_t>(service, 0x230 + 4 * i, block[i]);
    }
}

void set_underwater_wave_00b0ea70(void* service, const std::uint32_t* block) {
    for (std::uint32_t i = 0; i < 8; ++i) {
        wr<std::uint32_t>(service, 0x28c + 4 * i, block[i]);
    }
    static constexpr std::uint32_t order[8] = {2, 1, 0, 3, 6, 5, 4, 7};
    for (std::uint32_t i = 0; i < 8; ++i) {
        wr<std::uint32_t>(service, 0x628 + 4 * i, rd<std::uint32_t>(service, 0x28c + 4 * order[i]));
    }
}

void set_effect_priority_00b172e0(void* effect, std::int32_t priority) { wr<std::int32_t>(effect, 0xb0, priority); }

void set_material_alpha_ref_00b17310(void* material, std::int32_t value) { wr<std::int32_t>(material, 0x104, value); }

void clear_texture_source_children_00b19a60(void* registry, TextureSourceRegistryHost& host) {
    // B19A7D..B19A89: the end is recomputed from +8h/+0Ch after every call.
    for (std::uint8_t* entry = rd<std::uint8_t*>(registry, 0x8);
         entry != rd<std::uint8_t*>(registry, 0x8) + 0x2c * rd<std::uint32_t>(registry, 0xc); entry += 0x2c) {
        host.source_clear_children_slot28(rd<void*>(entry, 0x28));
    }
}

void release_texture_source_frames_00b33a10(void* renderer, TextureSourceRegistryHost& host) {
    clear_texture_source_children_00b19a60(host.registry_004de4b0(), host);
    for (std::uint8_t* entry = rd<std::uint8_t*>(renderer, 0x1a9c);
         entry != rd<std::uint8_t*>(renderer, 0x1a9c) + 0x2c * rd<std::uint32_t>(renderer, 0x1aa0); entry += 0x2c) {
        wr<std::uint8_t>(rd<void*>(entry, 0x28), 0xb4, 0);
    }
}

void frustum_corners_00b0eb10(void*, void* camera, float* out16, SceneRenderFrameHost& host) {
    float near_plane = rd<float>(camera, 0x1d4);
    if (0.5 > static_cast<double>(near_plane)) {  // B0EB54 FCOMIP 0.5 [00D7A280]
        near_plane = 0.5f;                         // [00CE3800]
    }
    const float far_plane = rd<float>(camera, 0x1d8);
    float projection[16];
    float inverse[16];
    host.invert4x4_00b632d0(host.build_perspective_00b642f0(projection, rd<float>(camera, 0x1c4),
                                                            rd<float>(camera, 0x1c8), near_plane, far_plane),
                            inverse);
    // B0EB3E..B0EC1B: eight NDC points; the loop reads only the four at z 0.
    static constexpr float ndc[4][2] = {{-1.0f, -1.0f}, {-1.0f, 1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}};
    float corner[4][3];
    for (int i = 0; i < 4; ++i) {
        const float source[4] = {ndc[i][0], ndc[i][1], 0.0f, 1.0f};
        float r[4];
        host.transform4_00b62d10(source, r, inverse);
        const double inv_w = 1.0 / static_cast<double>(r[3]);
        for (int k = 0; k < 3; ++k) {
            corner[i][k] = static_cast<float>(r[k] * inv_w);
        }
    }
    const float scale = static_cast<float>(static_cast<double>(far_plane) / near_plane);
    // B0EE87..B0EF3B rotates the four near points and the four scaled ones in
    // place; only the scaled ones reach the output. The world refresh check
    // runs per point and can only call once.
    refresh_world(camera, host);
    for (int i = 0; i < 4; ++i) {
        const double x = static_cast<float>(corner[i][0] * scale);
        const double y = static_cast<float>(corner[i][1] * scale);
        const double z = static_cast<float>(corner[i][2] * scale);
        out16[4 * i + 0] = static_cast<float>((x * rd<float>(camera, 0xf0) + y * rd<float>(camera, 0x100)) +
                                              z * rd<float>(camera, 0x110));
        out16[4 * i + 1] = static_cast<float>((x * rd<float>(camera, 0xf4) + y * rd<float>(camera, 0x104)) +
                                              z * rd<float>(camera, 0x114));
        out16[4 * i + 2] = static_cast<float>((x * rd<float>(camera, 0xf8) + y * rd<float>(camera, 0x108)) +
                                              z * rd<float>(camera, 0x118));
        out16[4 * i + 3] = 1.0f;  // [00D7A24C]
    }
}

void submit_scene_render_00b101c0(void* service, void* scene, void* camera, SceneRenderFrameHost& host) {
    void* node = rd<void*>(service, 0x1c0);
    wr<std::uint8_t>(service, 0x22c, (node != nullptr && rd<std::uint8_t>(node, 0x44) == 0) ? 1 : 0);
    if (flag(service, 0x22c)) {
        // B1020A..B10228: dt = elapsed / frequency from the clock [01090AB0] vtable +1Ch.
        const std::int64_t* interval = host.clock_interval_slot1c();
        const float dt = static_cast<float>(static_cast<double>(interval[0]) / static_cast<double>(interval[1]));
        node = rd<void*>(service, 0x1c0);
        refresh_world(node, host);
        float velocity[3];
        for (std::uint32_t i = 0; i < 3; ++i) {
            const float delta = rd<float>(node, 0x120 + 4 * i) - rd<float>(service, 0x1b4 + 4 * i);
            const float rate = static_cast<float>(static_cast<double>(delta) / dt);
            velocity[i] = rd<float>(service, 0x244 + 4 * i) + rate;  // impulse +244h..+24Ch
        }
        for (std::uint32_t i = 0; i < 3; ++i) {
            wr<float>(service, 0x19c + 4 * i, velocity[i]);
            wr<float>(service, 0x244 + 4 * i, 0.0f);
        }
        wr<float>(service, 0x1b0, host.length3_00419440(velocity));
        node = rd<void*>(service, 0x1c0);
        refresh_world(node, host);
        for (std::uint32_t i = 0; i < 3; ++i) {
            wr<float>(service, 0x1b4 + 4 * i, rd<float>(node, 0x120 + 4 * i));
        }
    }
    // B1034E..B103B2: on above MinSpeed (+238h) with motion blur (+219h); stays on
    // until the speed drops to MinSpeed * (1 - 0.05 [00D5E1C8]).
    const bool tracked = flag(service, 0x22c);
    bool active = false;
    if (tracked) {
        const float speed = rd<float>(service, 0x1b0);
        const float min_speed = rd<float>(service, 0x238);
        if (speed > min_speed && flag(service, 0x219)) {
            active = true;
        } else if (flag(service, 0x22d) &&
                   static_cast<double>(speed) > static_cast<double>(min_speed) - static_cast<double>(min_speed) * 0.05) {
            active = true;
        }
    }
    wr<std::uint8_t>(service, 0x22d, active ? 1 : 0);
    if (active) {
        host.node_set_visibility_factor_00b6da70(rd<void*>(service, 0x1c0), 0.0f, 0);
    } else if (tracked) {
        host.node_set_visibility_factor_00b6da70(rd<void*>(service, 0x1c0), 1.0f, 0);
    }
    void* command = host.render_command_new_00b1f170(scene, camera, rd<void*>(service, 0x1d4));
    refresh_world(camera, host);
    if (-5.0f > rd<float>(camera, 0x124)) {  // [00CFBC84]: camera below y = -5
        const float plane[4] = {0.0f, -1.0f, 0.0f, 2.0f};  // [00D7A260], [00CE3958]
        host.camera_set_extra_clip_plane_00b70410(camera, plane);
    } else {
        host.camera_clear_extra_clip_plane_00b70470(camera);
    }
    host.command_queue_append_00b1eb80(command, 1);
    const std::uint32_t metadata[3] = {0, 0x24, 0x5c};
    host.command_queue_set_last_metadata_00b1cb50(metadata);
    if (flag(service, 0xb0) && rd<void*>(service, 0x10) != camera) {
        void* previous = rd<void*>(service, 0x10);
        wr<void*>(service, 0x10, camera);
        host.camera_retain(camera);
        if (previous != nullptr) {
            host.camera_release(previous);
        }
    }
}

void excluded_node_pass_00b10500(void* service, void* command, void* targets, SceneRenderFrameHost& host) {
    if (!flag(service, 0x22c)) {
        return;
    }
    const std::uint32_t flags = rd<std::uint32_t>(host.command_camera_00b1bf40(command), 0x188);
    host.node_set_visibility_factor_00b6da70(rd<void*>(service, 0x1c0), 1.0f, 0);
    host.camera_set_clear_flags_00b6fe10(host.command_camera_00b1bf40(command), 0);
    void* scene = rd<void*>(host.command_camera_00b1bf40(command), 0xa4);
    host.command_queue_push_node_00b1f5e0(scene, host.command_camera_00b1bf40(command), targets,
                                          rd<void*>(service, 0x1c0));
    host.command_queue_execute_last_00b1e6d0();
    host.camera_set_clear_flags_00b6fe10(host.command_camera_00b1bf40(command), flags);
    host.node_set_visibility_factor_00b6da70(rd<void*>(service, 0x1c0), 0.0f, 0);
}

void render_scene_00b156c0(void* service, void* command, SceneRenderFrameHost& host) {
    if (!host.device_can_render_slot2c()) {
        return;
    }
    auto camera = [&] { return host.command_camera_00b1bf40(command); };

    // --- Frame constants -------------------------------------------------
    const float saved_near = rd<float>(camera(), 0x1d4);
    host.camera_set_near_plane_00b6fbf0(camera(), 0.5f);  // [00CE3800]
    // B1572A..B15741: the clear colour is saved with its top byte forced to FFh.
    const std::uint32_t saved_color = (host.camera_clear_color_00b6fe60(camera()) & 0x00ffffffu) | 0xff000000u;
    float y_axis[3];
    float x_axis[3];
    std::memcpy(y_axis, host.camera_projected_y_axis_00b70ea0(camera()), sizeof y_axis);
    std::memcpy(x_axis, host.camera_projected_x_axis_00b70fe0(camera()), sizeof x_axis);
    float velocity[3];
    std::memcpy(velocity, at(service, 0x19c), sizeof velocity);
    float direction[3];
    host.normalize3_00419510(velocity, direction);
    const float dot_y = static_cast<float>((static_cast<double>(direction[0]) * y_axis[0] +
                                            static_cast<double>(direction[1]) * y_axis[1]) +
                                           static_cast<double>(direction[2]) * y_axis[2]);
    const float dot_x = static_cast<float>((static_cast<double>(direction[0]) * x_axis[0] +
                                            static_cast<double>(direction[1]) * x_axis[1]) +
                                           static_cast<double>(direction[2]) * x_axis[2]);
    wr<float>(service, 0x1a8, static_cast<float>(-0.5 * dot_x));  // [00CEC9E0]
    wr<float>(service, 0x1ac, static_cast<float>(-0.5 * dot_y));

    host.camera_history_00b0db00(at(service, 0x314), host.clock_float_04(), camera());
    std::memcpy(at(service, 0x5a8), at(service, 0x55c), 64);  // 004134F0, delayed view-projection
    float inverse[16];
    std::memcpy(at(service, 0x5e8), host.invert4x4_00b632d0(host.camera_view_projection_00b70490(camera()), inverse),
                64);

    // B1586D..B15952: half and full texel of the +4Ch target. The height comes
    // from the first fetched texture, the width from the second.
    {
        void* height_source = host.holder_texture_00b4cb10(rd<void*>(service, 0x4c));
        void* width_source = host.holder_texture_00b4cb10(rd<void*>(service, 0x4c));
        const float half_w = over_size(0.5, host.texture_width_slot3c(width_source));
        const float half_h = over_size(0.5, host.texture_height_slot40(height_source));
        wr<float>(service, 0x4, half_w);
        wr<float>(service, 0x8, half_h);
        height_source = host.holder_texture_00b4cb10(rd<void*>(service, 0x4c));
        width_source = host.holder_texture_00b4cb10(rd<void*>(service, 0x4c));
        const float texel_w = over_size(1.0, host.texture_width_slot3c(width_source));
        const float texel_h = over_size(1.0, host.texture_height_slot40(height_source));
        wr<float>(service, 0x184, texel_w);
        wr<float>(service, 0x188, texel_h);
    }
    // B15958..B15BBF: 3x3 kernel offsets, float4 each from +F4h, scaled by the texel.
    static constexpr float kernel[9][2] = {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {0, 0},
                                           {1, 0},   {-1, 1}, {0, 1},  {1, 1}};
    for (std::uint32_t i = 0; i < 9; ++i) {
        const std::uint32_t entry = 0xf4 + 0x10 * i;
        wr<float>(service, entry + 0x0, kernel[i][0] * rd<float>(service, 0x184));
        wr<float>(service, entry + 0x4, kernel[i][1] * rd<float>(service, 0x188));
        wr<float>(service, entry + 0x8, 0.0f);
        wr<float>(service, entry + 0xc, 0.0f);
    }
    {
        float corners[16];
        host.frustum_corners_00b0eb10(service, camera(), corners);
        std::memcpy(at(service, 0xb4), corners, sizeof corners);
    }
    // B15BD2..B15D1B: lens effect constants from the five +258h words.
    if (flag(service, 0x21a)) {
        void* height_source = host.holder_texture_00b4cb10(host.bloom_output_holder_00b54cd0(rd<void*>(service, 0x2c)));
        void* width_source = host.holder_texture_00b4cb10(host.bloom_output_holder_00b54cd0(rd<void*>(service, 0x2c)));
        const float half_w = over_size(0.5, host.texture_width_slot3c(width_source));
        const float half_h = over_size(0.5, host.texture_height_slot40(height_source));
        wr<float>(service, 0x18c, half_w);
        wr<float>(service, 0x190, half_h);
        constexpr double scale = 20000.0;  // [00CE3CB8]
        wr<float>(service, 0x26c, static_cast<float>(rd<float>(service, 0x258) / scale));
        wr<float>(service, 0x270, static_cast<float>(1.0 / (rd<float>(service, 0x25c) / scale)));
        wr<float>(service, 0x274, clamp_unit(static_cast<float>(1.0 - rd<float>(service, 0x264))));
        wr<float>(service, 0x278, static_cast<float>(1.0 / (rd<float>(service, 0x260) / scale)));
        wr<float>(service, 0x27c, clamp_unit(static_cast<float>(1.0 - rd<float>(service, 0x268))));
    }
    // B15D2E..B15DD4: clip-to-texture matrix with the half-texel offset.
    static constexpr float uv_matrix[16] = {0.5f, 0, 0, 0, 0, -0.5f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f};
    for (std::uint32_t i = 0; i < 16; ++i) {
        wr<float>(service, 0x1d8 + 4 * i, uv_matrix[i]);
    }
    wr<float>(service, 0x208, static_cast<float>(rd<float>(service, 0x4) + 0.5));
    wr<float>(service, 0x20c, static_cast<float>(0.5 + rd<float>(service, 0x8)));

    // --- DRSH: the three-target geometry pass ----------------------------
    const std::uint32_t saved_flags = rd<std::uint32_t>(camera(), 0x188);
    host.camera_set_clear_flags_00b6fe10(camera(), 7);
    host.camera_set_clear_color_00b6fe50(camera(), 0xffffffffu);
    host.targets_set_color_00b1fab0(rd<void*>(service, 0x1c8), 0,
                                    host.holder_primary_surface_00b4cb20(rd<void*>(service, 0x38)));
    host.targets_set_color_00b1fab0(rd<void*>(service, 0x1c8), 1,
                                    host.holder_primary_surface_00b4cb20(rd<void*>(service, 0x3c)));
    host.targets_set_color_00b1fab0(rd<void*>(service, 0x1c8), 2,
                                    host.holder_primary_surface_00b4cb20(rd<void*>(service, 0x40)));
    host.device_set_frame_targets_slot98(rd<void*>(service, 0x1c8));
    host.camera_execute_frame_command_00b71360(camera());
    const std::int32_t saved_mode = rd<std::int32_t>(camera(), 0x198);
    host.camera_set_render_mode_00b6fdf0(camera(), 6);
    upload_prefix(command, host);
    host.assign_global_command_label("DRSH");
    execute_batch_lists(command, host);
    if (flag(service, 0x22d)) {
        host.excluded_node_pass_00b10500(service, command, rd<void*>(service, 0x1c8));
    }
    host.camera_set_render_mode_00b6fdf0(camera(), saved_mode);
    host.reset_command_label_00b13510();
    host.camera_set_clear_flags_00b6fe10(camera(), saved_flags);
    host.camera_set_near_plane_00b6fbf0(camera(), saved_near);
    host.object_slot10(rd<void*>(service, 0x60));
    upload_prefix(command, host);
    host.camera_set_clear_color_00b6fe50(camera(), saved_color);

    // --- SCNE: the colour pass -------------------------------------------
    host.assign_global_command_label("SCNE");
    host.targets_set_color_00b1fab0(
        rd<void*>(service, 0x1cc), 0,
        host.holder_primary_surface_00b4cb20(rd<void*>(service, flag(service, 0x22d) ? 0x48 : 0x44)));
    host.device_set_frame_targets_slot98(rd<void*>(service, 0x1cc));
    host.camera_execute_frame_command_00b71360(camera());
    execute_batch_lists(command, host);
    host.holder_resolve_00b4caf0(rd<void*>(service, flag(service, 0x22d) ? 0x48 : 0x44));
    for (std::uint32_t stage = 0; stage < 8; ++stage) {
        host.device_set_texture_slot130(stage, nullptr);
    }
    host.reset_command_label_00b13510();

    // --- POST ------------------------------------------------------------
    wr<std::uint8_t>(service, 0x680, 0);
    host.assign_global_command_label("POST");
    if (flag(service, 0x22d)) {
        host.device_set_frame_targets_slot98(nullptr);
        host.targets_set_color_00b1fab0(rd<void*>(service, 0x1cc), 0,
                                        host.holder_primary_surface_00b4cb20(rd<void*>(service, 0x44)));
        host.pass_assign_targets_00b4e2b0(rd<void*>(service, 0x664), rd<void*>(service, 0x1cc));
        set_slot(rd<void*>(service, 0x664), 0, host.holder_texture_00b4cb10(rd<void*>(service, 0x48)), host);
        host.pass_draw_00b4dcc0(rd<void*>(service, 0x664), 0);
        const std::uint32_t flags = rd<std::uint32_t>(camera(), 0x188);
        host.camera_set_clear_flags_00b6fe10(camera(), 2);
        host.camera_clear_00b700a0(camera());
        host.camera_set_clear_flags_00b6fe10(camera(), flags);
        host.excluded_node_pass_00b10500(service, command, rd<void*>(service, 0x1cc));
        host.holder_resolve_00b4caf0(rd<void*>(service, 0x44));
    }
    upload_prefix(command, host);
    host.device_set_frame_targets_slot98(nullptr);
    host.targets_set_color_00b1fab0(rd<void*>(service, 0x1cc), 0, rd<void*>(service, 0x58));
    host.device_set_frame_targets_slot98(rd<void*>(service, 0x1cc));
    select_render_mode(command, 8, 9, host);
    host.camera_set_clear_color_00b6fe50(camera(), 0);
    host.camera_set_clear_flags_00b6fe10(camera(), 7);
    host.camera_set_clear_stencil_00b6fe70(camera(), 0);
    host.camera_clear_00b700a0(camera());
    host.batch_list_prepare_slotc(rd<void*>(command, 0x10), 1);
    host.batch_list_execute_slot8(rd<void*>(command, 0x10), 1, camera());
    host.camera_set_render_mode_00b6fdf0(camera(), saved_mode);
    host.surface_stretch_from_00b3cda0(rd<void*>(service, 0x5c), rd<void*>(service, 0x58));
    host.device_set_frame_targets_slot98(nullptr);
    host.camera_set_clear_color_00b6fe50(camera(), 0x01000000u);
    // B1660E..B16998: the cockpit helper +0Ch brackets the second list-1
    // pass when its node is visible; the passes between are the same.
    void* cockpit = rd<void*>(service, 0x0c);
    const bool bracket = cockpit != nullptr && host.cockpit_node_visible_00b3c550(cockpit);
    if (bracket) {
        host.cockpit_begin_frame_00b3c960(cockpit, camera(), rd<void*>(service, 0x1cc));
    }
    host.targets_set_color_00b1fab0(
        rd<void*>(service, 0x1cc), 0,
        host.holder_primary_surface_00b4cb20(host.shadow_map_texture_holder_00b4d170(rd<void*>(service, 0x64))));
    host.targets_set_depth_00b1fb00(rd<void*>(service, 0x1cc), host.device_depth_surface_slot12c());
    host.device_set_frame_targets_slot98(rd<void*>(service, 0x1cc));
    set_slot(rd<void*>(service, 0x68), 0, host.holder_texture_00b4cb10(rd<void*>(service, 0x44)), host);
    host.pass_assign_targets_00b4e2b0(rd<void*>(service, 0x68), rd<void*>(service, 0x1cc));
    host.pass_draw_00b4dcc0(rd<void*>(service, 0x68), 0);
    host.camera_set_clear_flags_00b6fe10(camera(), 6);
    host.camera_set_clear_stencil_00b6fe70(camera(), 0);
    host.camera_clear_00b700a0(camera());
    set_slot(rd<void*>(service, 0x6c), 0, rd<void*>(service, 0x54), host);
    set_slot(rd<void*>(service, 0x6c), 1,
             host.holder_texture_00b4cb10(host.shadow_map_texture_holder_00b4d170(rd<void*>(service, 0x60))), host);
    host.pass_assign_targets_00b4e2b0(rd<void*>(service, 0x6c), rd<void*>(service, 0x1cc));
    host.pass_draw_00b4dcc0(rd<void*>(service, 0x6c), 0);
    select_render_mode(command, 10, 11, host);
    if (bracket) {
        upload_prefix(command, host);
    }
    host.batch_list_prepare_slotc(rd<void*>(command, 0x10), 1);
    host.batch_list_execute_slot8(rd<void*>(command, 0x10), 1, camera());
    host.camera_set_render_mode_00b6fdf0(camera(), saved_mode);
    if (bracket) {
        host.cockpit_draw_00b3c6e0(rd<void*>(service, 0x0c), camera(), rd<void*>(service, 0x1cc));
    }
    host.holder_resolve_00b4caf0(host.shadow_map_texture_holder_00b4d170(rd<void*>(service, 0x64)));
    host.camera_set_clear_flags_00b6fe10(camera(), 7);
    if (bracket) {
        host.device_set_frame_targets_slot98(nullptr);
    }

    // B16998..B16B84: post objects, luminance, tone map and the composite.
    host.object_slot10(rd<void*>(service, 0x18));
    const std::uint32_t words[3] = {0, 0x10, 0x70};
    host.device_slot114(words);
    if (flag(service, 0x21a) || flag(service, 0x21b)) {
        host.object_slot10(rd<void*>(service, 0x1c));
    }
    if (flag(service, 0x21a) || flag(service, 0x21b)) {
        host.object_slot10(rd<void*>(service, 0x2c));
    }
    host.luminance_measure_00b50050(rd<void*>(service, 0x20), rd<float>(service, 0x90));
    host.tonemap_set_params_draw_00b548c0(rd<void*>(service, 0x24), rd<void*>(rd<void*>(service, 0x20), 0x23c),
                                          rd<float>(service, 0x84), rd<float>(service, 0x88),
                                          rd<float>(service, 0x8c), rd<float>(service, 0x98),
                                          rd<float>(service, 0x9c));
    host.object_slot10(rd<void*>(service, 0x28));
    void* composite = nullptr;
    if (flag(service, 0x21a)) {
        composite = rd<void*>(service, 0x74);
        set_slot(composite, 2, host.holder_texture_00b4cb10(rd<void*>(rd<void*>(service, 0x20), 0x23c)), host);
        set_slot(composite, 3, host.holder_texture_00b4cb10(host.bloom_output_holder_00b54cd0(rd<void*>(service, 0x2c))),
                 host);
        set_slot(composite, 4, host.holder_texture_00b4cb10(rd<void*>(service, 0x38)), host);
    } else if (flag(service, 0x21b)) {
        composite = rd<void*>(service, 0x80);
        set_slot(composite, 2, host.holder_texture_00b4cb10(rd<void*>(rd<void*>(service, 0x20), 0x23c)), host);
        set_slot(composite, 3, host.holder_texture_00b4cb10(host.bloom_output_holder_00b54cd0(rd<void*>(service, 0x2c))),
                 host);
    } else {
        composite = rd<void*>(service, 0x70);
        set_slot(composite, 2, host.holder_texture_00b4cb10(rd<void*>(rd<void*>(service, 0x20), 0x23c)), host);
    }
    host.targets_set_srgb_write_00b1f700(rd<void*>(composite, 0x8), 1);  // 00B4CB90 = pass+8h
    host.pass_draw_00b4dcc0(composite, 0);

    if (void* distortion = rd<void*>(service, 0x30)) {
        host.distortion_render_00b4f1c0(distortion, camera());
        std::uint32_t extent[2];
        host.distortion_extent_00b4ed30(rd<void*>(service, 0x30), extent);
        wr<std::uint32_t>(service, 0x194, extent[0]);  // x87 float copies
        wr<std::uint32_t>(service, 0x198, extent[1]);
    }

    // B16BBD..B16F45: the final copy to the back-buffer targets +1D4h; the
    // old-film variant when the mode +220h asks for it.
    const std::int32_t film = rd<std::int32_t>(service, 0x220);
    const float amount = rd<float>(service, 0x224);
    // B16D04..B16D2E: the distortion output replaces the +668h texture unless
    // the cockpit node is visible. 00B3C550 is called on +0Ch without a null
    // check here (the native would fault on a null +0Ch).
    auto distortion_texture = [&](bool check_cockpit) -> void* {
        if (rd<void*>(service, 0x30) != nullptr &&
            (!check_cockpit || !host.cockpit_node_visible_00b3c550(rd<void*>(service, 0x0c)))) {
            return host.holder_texture_00b4cb10(rd<void*>(rd<void*>(service, 0x30), 0x18));  // 00B4EC50
        }
        return rd<void*>(service, 0x668);
    };
    if (film != 0 && (amount > 0.0f || film >= 2)) {
        const std::uint8_t strong = (amount > 0.0f || film == 3) ? 1 : 0;
        float extent[2];
        host.gui_aspect_extent_00aa1fe0(extent);
        wr<float>(service, 0x228,
                  static_cast<float>(extent[1] / (1.0 - (static_cast<double>(amount) + amount))));
        const double half_w =
            static_cast<double>(host.texture_width_slot3c(host.holder_texture_00b4cb10(rd<void*>(service, 0x4c)))) * 0.5;
        wr<float>(service, 0x648,
                  static_cast<float>(half_w / host.viewport_width_slot48(rd<void*>(service, 0x67c))));
        const double half_h =
            static_cast<double>(host.texture_height_slot40(host.holder_texture_00b4cb10(rd<void*>(service, 0x4c)))) *
            0.5;
        wr<float>(service, 0x64c,
                  static_cast<float>(half_h / host.viewport_height_slot4c(rd<void*>(service, 0x67c))));
        const bool alt = flag(service, 0x21c);
        void* pass = rd<void*>(service, alt ? 0x65c : 0x658);
        host.pass_assign_targets_00b4e2b0(pass, rd<void*>(service, 0x1d4));
        set_slot(pass, 0, host.holder_texture_00b4cb10(rd<void*>(service, 0x4c)), host);
        set_slot(pass, 1, distortion_texture(true), host);
        set_slot(pass, 2, rd<void*>(service, alt ? 0x678 : 0x674), host);
        host.pass_draw_00b4dcc0(pass, 0);
        host.render_old_film_00b529a0(rd<void*>(service, 0x34), rd<void*>(service, 0x1d4), at(service, 0x2ac),
                                      strong, extent[1], amount);
    } else {
        const bool alt = flag(service, 0x21c);
        void* pass = rd<void*>(service, alt ? 0x654 : 0x650);
        host.pass_assign_targets_00b4e2b0(pass, rd<void*>(service, 0x1d4));
        set_slot(pass, 0, host.holder_texture_00b4cb10(rd<void*>(service, 0x4c)), host);
        // B16E64: the +654h variant skips the cockpit test.
        set_slot(pass, 1, distortion_texture(!alt), host);
        set_slot(pass, 2, rd<void*>(service, alt ? 0x670 : 0x66c), host);
        host.pass_draw_00b4dcc0(pass, 0);
    }
    host.stats_overlay_00b151e0(service);
    host.reset_command_label_00b13510();
}

std::size_t find_filter_name_00b105b0(const std::vector<std::string>& filter, const std::string& name) {
    for (std::size_t i = 0; i < filter.size(); ++i) {
        if (filter[i].size() == name.size() && (name.empty() || _stricmp(name.c_str(), filter[i].c_str()) == 0)) {
            return i;
        }
    }
    return filter.size();
}

void stats_overlay_00b151e0(StatsOverlayHost& host) {
    if (!host.enabled_681()) {
        return;
    }
    std::uint32_t y = 20;
    std::int32_t column = 0;
    std::int32_t total_draws = 0;
    std::int32_t total_primitives = 0;
    std::int32_t total_vs = 0;
    std::int32_t total_ps = 0;
    std::vector<RenderStatsRecord>& records = host.records_690();
    // 00B148E0 is MSVC's introsort; equal keys may land in another order here.
    std::sort(records.begin(), records.end(), [](const RenderStatsRecord& a, const RenderStatsRecord& b) {
        if (a.sort_key.empty()) return !b.sort_key.empty();
        if (b.sort_key.empty()) return false;
        return _stricmp(a.sort_key.c_str(), b.sort_key.c_str()) < 0;
    });
    const std::vector<std::string>& filter = host.filter_69c();
    char line[512];
    for (const RenderStatsRecord& record : records) {
        if (find_filter_name_00b105b0(filter, record.name) == filter.size()) {
            continue;
        }
        std::int32_t x = column * 500;
        for (std::int32_t mode = 0; mode < 14; ++mode) {
            // Effect name after its last '/' [00CE7898], at most 1000 bytes.
            std::string effect = host.effect_name_00b172d0(record.effect);
            if (const std::size_t slash = effect.rfind('/'); slash != std::string::npos) {
                effect = effect.substr(slash + 1, 1000);
            }
            if (record.draw_calls[mode] != 0) {
                std::snprintf(line, sizeof line, "%s : %s - RM(%i) - DP(%i) - PRM(%i) - VSF4(%i) - PSF4(%i)",
                              record.name.c_str(), effect.c_str(), mode, record.draw_calls[mode],
                              record.primitives[mode], record.vs_float4[mode], record.ps_float4[mode]);
                host.debug_print_004b47e0(static_cast<float>(static_cast<std::uint32_t>(x)),
                                          static_cast<float>(y), line);
                y += 0x12;
                total_draws += record.draw_calls[mode];
                total_primitives += record.primitives[mode];
                total_vs += record.vs_float4[mode];
                total_ps += record.ps_float4[mode];
            }
            if (y > 599) {
                ++column;
                x += 500;
                y = 20;
            }
        }
    }
    std::snprintf(line, sizeof line, "Total - DP(%i) - PRM(%i) - VSF4(%i) - PSF4(%i)", total_draws, total_primitives,
                  total_vs, total_ps);
    host.debug_print_004b47e0(static_cast<float>(static_cast<std::uint32_t>(column * 500)), static_cast<float>(y),
                              line);
}

void camera_history_00b0db00(void* history, float time, void* camera, SceneRenderFrameHost& host) {
    constexpr std::uint32_t stride = 0x48;  // world 4x4, fov +40h, time +44h
    auto entry = [&](std::uint32_t index) { return at(history, 0x8 + stride * index); };
    refresh_world(camera, host);
    // B0DB1E..B0DB84: restart when the camera moved more than 20.0 since the last call.
    {
        float delta[3];
        for (std::uint32_t i = 0; i < 3; ++i) {
            delta[i] = rd<float>(history, 0x288 + 4 * i) - rd<float>(camera, 0x120 + 4 * i);
        }
        if (host.length3_00419440(delta) > 20.0f) {  // [00CE3930]
            wr<std::uint32_t>(history, 0x4, 0);
        }
    }
    if (rd<std::uint32_t>(history, 0x4) == 8) {  // full: drop the oldest (x87 float copies)
        std::memmove(entry(0), entry(1), stride * 7);
        wr<std::uint32_t>(history, 0x4, 7);
    }
    refresh_world(camera, host);
    const std::uint32_t n = rd<std::uint32_t>(history, 0x4);
    std::memcpy(entry(n), at(camera, 0xf0), 64);
    wr<float>(entry(n), 0x40, rd<float>(camera, 0x1c4));
    wr<float>(entry(n), 0x44, time);
    const float target = time - rd<float>(history, 0x0);
    wr<std::uint32_t>(history, 0x4, n + 1);
    // B0E1EC..B0E222: newest pair; the walk only moves back while
    // time[c] < target <= time[d], which increasing times never satisfy.
    std::uint32_t c = n;
    std::uint32_t d = n;
    if (n != 0) {
        d = n - 1;
        while (rd<float>(entry(c), 0x44) < target && rd<float>(entry(d), 0x44) >= target && d != 0) {
            --d;
            --c;
        }
    }
    const float td = rd<float>(entry(d), 0x44);
    const float fraction = clamp_unit(static_cast<float>((static_cast<double>(target) - td) /
                                                         (static_cast<double>(rd<float>(entry(c), 0x44)) - td)));
    auto column = [&](std::uint32_t index, std::uint32_t first, float* out) {
        out[0] = rd<float>(entry(index), first);
        out[1] = rd<float>(entry(index), first + 0x10);
        out[2] = rd<float>(entry(index), first + 0x20);
    };
    float pos_d[3], pos_c[3], z_d[3], z_c[3], y_d[3], y_c[3];
    for (std::uint32_t i = 0; i < 3; ++i) {
        pos_d[i] = rd<float>(entry(d), 0x30 + 4 * i);
        pos_c[i] = rd<float>(entry(c), 0x30 + 4 * i);
    }
    column(d, 0x8, z_d);
    column(c, 0x8, z_c);
    column(d, 0x4, y_d);
    column(c, 0x4, y_c);
    float position[3], z[3], y[3];
    lerp3(pos_d, pos_c, fraction, position);
    lerp3(z_d, z_c, fraction, z);
    float inv = inverse_length(z[0], z[1], z[2]);
    for (float& v : z) v = inv * v;
    lerp3(y_d, y_c, fraction, y);
    inv = inverse_length(y[0], y[1], y[2]);
    for (float& v : y) v = inv * v;
    // x = normalize(y cross z), then y = normalize(z cross x). Both squared
    // sums add y*y + x*x first (B0E584..B0E5A8, B0E660..B0E684).
    float x[3] = {cross_term(z[2], y[1], z[1], y[2]), cross_term(y[2], z[0], z[2], y[0]),
                  cross_term(y[0], z[1], z[0], y[1])};
    inv = inverse_length(x[1], x[0], x[2]);
    for (float& v : x) v = v * inv;
    float y2[3] = {cross_term(z[1], x[2], z[2], x[1]), cross_term(z[2], x[0], x[2], z[0]),
                   cross_term(x[1], z[0], z[1], x[0])};
    inv = inverse_length(y2[1], y2[0], y2[2]);
    for (float& v : y2) v = v * inv;
    const float world[16] = {x[0], y2[0], z[0], 0.0f, x[1], y2[1], z[1], 0.0f,
                             x[2], y2[2], z[2], 0.0f, position[0], position[1], position[2], 1.0f};
    const float fov_d = rd<float>(entry(d), 0x40);
    const float fov = static_cast<float>((static_cast<double>(rd<float>(entry(c), 0x40)) - fov_d) * fraction + fov_d);
    float projection[16], view[16], view_projection[16];
    host.build_perspective_00b642f0(projection, fov, rd<float>(camera, 0x1c8), rd<float>(camera, 0x1d4),
                                    rd<float>(camera, 0x1d8));
    host.invert_affine_00b63b30(view, world);
    host.multiply4x4_00413920(view, view_projection, projection);
    std::memcpy(at(history, 0x248), view_projection, sizeof view_projection);
    refresh_world(camera, host);
    for (std::uint32_t i = 0; i < 3; ++i) {
        wr<float>(history, 0x288 + 4 * i, rd<float>(camera, 0x120 + 4 * i));
    }
}

}  // namespace bsp::scene_renderer
