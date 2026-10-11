// Scene-renderer object [00F8D39C] leaves, segment 82 render half (packet
// cc13_scene_renderer). Evidence, ABI and uncertainty:
// include/bsp/scene_renderer_cc13.hpp and docs/CC13_SCENE_RENDERER.md.

#include "bsp/scene_renderer_cc13.hpp"

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

}  // namespace bsp::scene_renderer
