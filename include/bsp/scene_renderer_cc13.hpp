#pragma once

// The scene-renderer object [00F8D39C] ("render service"), segment 82 render
// half 00B0CF80..00B1BD20 (packet cc13_scene_renderer,
// docs/CC13_SCENE_RENDERER.md). Not particle code. Semantic MSVC Win32 C++ over
// the native storage (raw pointer + byte offsets); names are hypotheses.
// Construction, parameters, textures and resources of this object are covered
// by the existing NATIVE_RENDER_SERVICE_* and render-resources work; this file
// only adds what those leave out.

#include <cstdint>

#include "bsp/native_material_parameters.hpp"

namespace bsp::scene_renderer {

// 00B0CF80 `void __thiscall(service, uint32 a, float amount)`, RET 8: +250h =
// a, +254h = amount, +21Bh = !(0.0001 [00D7A290] > amount) (NaN enables).
// Called from the ship screens' pipe-sight blur block (docs/SHIP_SCREEN_UPDATE.md).
void set_sight_blur_00b0cf80(void* service, std::uint32_t a, float amount);

// 00B0CFC0 `void __thiscall(service, float* start, float* end, float*
// duration)`, RET 0Ch: start = +238h, duration = +240h, end = float(+23Ch *
// +240h + +238h) (x87, one rounding).
void get_transition_00b0cfc0(const void* service, float* start, float* end, float* duration);

// 00B0D6A0 `void __thiscall(service, float start, float end, float duration,
// a, b, c)`, RET 18h: +238h = start, +240h = duration, +23Ch = (end - start) /
// duration when duration > 0 (COMISS against 0.0 [00D7A218]; NaN takes the
// else) else 1.0 [00D7A24C]; +244h..+24Ch = a, b, c. The GameTuning caller
// 00608C30 passes tuning +D8h/+DCh/+E0h.
void set_transition_00b0d6a0(void* service, float start, float end, float duration, std::uint32_t a,
                             std::uint32_t b, std::uint32_t c);

// 00B0D020 `void __thiscall(service, uint8 enabled, a, b, c, d, e)`, RET 18h:
// +258h..+268h = a..e, +21Ah = enabled. The binocular lens effect
// (00452B80 on 00E081A0; include/bsp/hud_warning_screen.hpp).
void set_lens_effect_00b0d020(void* service, std::uint8_t enabled, std::uint32_t a, std::uint32_t b,
                              std::uint32_t c, std::uint32_t d, std::uint32_t e);

// 00B0D080 `void __thiscall(service, value)`, RET 4: +220h, the old-film mode
// (docs/OPTIONS_SETTINGS_COMMIT.md item 23).
void set_film_effect_00b0d080(void* service, std::uint32_t value);
// 00B0D0B0 `void __thiscall(service, value)`, RET 4: +224h. The movie screens'
// exit (005CDB00, 005CDD70) store 0.0; meaning open.
void set_value_224_00b0d0b0(void* service, std::uint32_t value);
// 00B0D160 `void __thiscall(service, uint8)`, RET 4: +21Ch; only caller 00BA0090.
void set_flag_21c_00b0d160(void* service, std::uint8_t value);

// What 00B0D170/00B0D180/00B0D190/00B0D580/00B17240 reach.
class SceneRendererHost {
public:
    virtual ~SceneRendererHost() = default;
    // Tail calls on the render listener service+30h (skipped when null).
    virtual void listener_attach_00b4ef90(void* listener, void* a, void* b) = 0;
    virtual void listener_invalidate_root_00b4ec90(void* listener) = 0;  // BSP_RenderListener_InvalidateRootRegistration
    virtual void listener_flush_00b4f030(void* listener) = 0;
    // [00F8D394] vtable +80h(out 8 bytes); the result is not read by 00B0D580.
    virtual void device_query_slot80(void* out8) = 0;
    // 00B4D270(target, samples): render texture sample count (recreates on change).
    virtual void texture_set_samples_00b4d270(void* target, std::int32_t samples) = 0;
    // 00B3CE20(target, samples): target+24h = samples.
    virtual void target_set_samples_00b3ce20(void* target, std::int32_t samples) = 0;
    // 00B4D170 BSP_NativeShadowMap_GetTextureHolder(shadow map).
    virtual void* shadow_map_texture_holder_00b4d170(void* shadow_map) = 0;
    // 00B17240's calls.
    virtual bool can_execute_00b20240() = 0;                   // [00F8D394], BSP_D3D9Renderer_CanExecuteCommand
    virtual std::uint32_t command_prefix_00b1bf40(void* command) = 0;
    virtual void upload_system_prefix_00b46a70(void* target, std::uint32_t value) = 0;  // ECX target, EDX value
    virtual void device_slot114(void* argument) = 0;           // [00F8D394] vtable +114h
    virtual void render_scene_00b156c0(void* service, void* command) = 0;
};

// 00B0D170 `__thiscall(service, a, b)`, RET 8: listener +30h ? JMP 00B4EF90(a, b).
// 00AF63A0 passes (particle model, mesh B); BSP_Waterdrops_Spawn also calls it.
void attach_to_listener_00b0d170(void* service, void* a, void* b, SceneRendererHost& host);
// 00B0D180 / 00B0D190 `__thiscall(service)`, RET: listener ? JMP 00B4EC90 / 00B4F030.
void invalidate_listener_root_00b0d180(void* service, SceneRendererHost& host);
void flush_listener_00b0d190(void* service, SceneRendererHost& host);

// 00B0D580 `void __thiscall(service, uint8 motion_blur, int samples)`, RET 8:
// +219h = motion_blur; device vtable +80h(local); samples to the render
// textures +44h, +48h (00B4D270), the shadow map +64h's texture holder (when
// both exist), and the targets +50h, +58h (00B3CE20). Settings_ApplyAll passes
// settings +8Dh (MotionBlur) and +58h (Antialias).
void set_motion_blur_and_samples_00b0d580(void* service, std::uint8_t motion_blur, std::int32_t samples,
                                          SceneRendererHost& host);

// 00B17240 `void __thiscall(command)`, RET, the function pointer at D5E13C:
// when the renderer can execute: 00B46A70(command+4, 00B1BF40(command)),
// device +114h(command+1Ch), then 00B156C0(service [00F8D39C], command).
void execute_scene_command_00b17240(void* command, void* service, SceneRendererHost& host);

// 00B0DAA0 `bool __thiscall(a, b)`, RET 4: name order of two records by
// +124h (length) and +128h (text): empty a sorts before non-empty b; empty b
// never after; else _stricmp(a, b) < 0. Only caller 00B13790.
bool record_name_less_00b0daa0(const void* a, const void* b);

// 00B18AE0 / 00B18B40: the float3 and matrix siblings of the recovered
// 00B18AA0/00B18B00/00B18B20 wrappers, ECX material, name/source stack, RET 8:
// 00B17E10(material, name, source, 3, 0) and (..., 16, 1).
NativeMaterialParameterStorage* register_native_material_float3_00b18ae0(NativeMaterialStorage&,
                                                                         const void* name_header,
                                                                         const void* source,
                                                                         NativeMaterialParameterAccess&);
NativeMaterialParameterStorage* register_native_material_matrix_00b18b40(NativeMaterialStorage&,
                                                                         const void* name_header,
                                                                         const void* source,
                                                                         NativeMaterialParameterAccess&);

}  // namespace bsp::scene_renderer
