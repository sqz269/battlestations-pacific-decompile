#pragma once

// The scene-renderer object [00F8D39C] ("render service"), segment 82 render
// half 00B0CF80..00B1BD20 (packet cc13_scene_renderer,
// docs/CC13_SCENE_RENDERER.md). Not particle code. Semantic MSVC Win32 C++ over
// the native storage (raw pointer + byte offsets); names are hypotheses.
// Construction, parameters, textures and resources of this object are covered
// by the existing NATIVE_RENDER_SERVICE_* and render-resources work; this file
// only adds what those leave out.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

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

// ---------------------------------------------------------------------------
// Packet cc13_scene_spine (docs/CC13_SCENE_RENDERER.md section 5 onward).
// ---------------------------------------------------------------------------

// 00B0E9B0 `void __thiscall(service, const uint32* block)`, RET 4: copies an
// 11-DWORD block to +84h..+ACh, the tenth entry as a BYTE at +A8h. The scene
// header reader 004E8510 fills it from the "HDR" property bag: BrightPassTreshold,
// BrightPassOffset, MiddleGray, 1 - AdaptationPercent/100, BloomScale,
// MinLuminance, MaxLuminance, NewHDRParam0, NewHDRParam1, AntiBandingON (byte),
// ParticleLuminanceMul. Other callers: 00503510, 00504130, 0067F080.
void set_hdr_params_00b0e9b0(void* service, const std::uint32_t* block);
// 00B0EA20 `void __thiscall(service, const uint32 block[8])`, RET 4: +230h..+24Ch.
// 004E8510 ("MotionBlur" bag): MaxVelocity, VelocityScale, MinSpeed, Divider,
// MaxBlur, then three zero words (the velocity impulse 00B101C0 consumes).
// BSP_LuaBinding_SetMotionBlurParams (0088E560) passes {1.0, 0.5, a0,
// (a1 - a0) / a2, a2, 0, 0, 0}.
void set_motion_blur_params_00b0ea20(void* service, const std::uint32_t* block);
// 00B0EA70 `void __thiscall(service, const uint32 block[8])`, RET 4: REP MOVSD
// to +28Ch..+2A8h, then the shader-order copy +628h..+644h = block[2], [1], [0],
// [3], [6], [5], [4], [7]. 004E8510 ("UnderWaterWave" bag): HorizSpeed,
// HorizScale, HorizPhase, HorizAmplitude/1000, VertSpeed, VertScale, VertPhase,
// VertAmplitude/1000.
void set_underwater_wave_00b0ea70(void* service, const std::uint32_t* block);

// 00B172E0 `void __thiscall(effect, int32 priority)`, RET 4: effect+B0h, the
// signed material sort priority (docs/MATERIAL_SORT_METADATA.md). Only caller
// BSP_GuiModel_SetupMaterialConstants (00ACC8A0) passes 0 for each mesh
// section's material+7Ch effect.
void set_effect_priority_00b172e0(void* effect, std::int32_t priority);
// 00B17310 `void __thiscall(material, int32 value)`, RET 4: material+104h, the
// alpha reference read back by BSP_Material_GetWord104 (00B17320; 00B43410
// sets D3DRS_ALPHAREF when it is > -1). Callers: two map/formation HUD
// screens (005A8F50, 005A9770 under 005A9DB0; 0093DE10).
void set_material_alpha_ref_00b17310(void* material, std::int32_t value);

// 00B19A60 `void __thiscall(registry)`, RET: for each 2Ch entry of the
// registry's vector (begin +8h, count +0Ch) call entry+28h's vtable +28h.
// The registry (004DE4B0, ledger name BSP_ParticleClock_GetSingleton) holds
// texture sources: 00B1B4D0 looks samplers up in it, and the Caustics/ShoreWave
// sources (vtables D64478/D644B4) put BSP_TextureSource_ClearChildren
// (C303B0, release the frame textures) in slot +28h; the base D5E554 slot is a
// bare RET. 00B19A10 drives slot +18h, BSP_TextureSource_SetTime.
class TextureSourceRegistryHost {
public:
    virtual ~TextureSourceRegistryHost() = default;
    virtual void* registry_004de4b0() = 0;
    virtual void source_clear_children_slot28(void* source) = 0;
};
void clear_texture_source_children_00b19a60(void* registry, TextureSourceRegistryHost& host);

// 00B33A10 `void __thiscall(renderer)`, RET: renderer vtable 00D5F0A8 slot
// +E8h, called by BSP_Game_TeardownSessionState (004DAD4A). 00B19A60 on the
// registry, then byte +B4h = 0 on the effect (+28h) of every 2Ch entry of the
// renderer's effect cache (+1A9Ch begin, +1AA0h count). Material construction
// 00B18900 sets that byte to 1, so it reads as "effect in use".
void release_texture_source_frames_00b33a10(void* renderer, TextureSourceRegistryHost& host);

// What the scene-render spine calls. Methods are named after the original
// callee; "slotNN" is a vtable offset. Every pointer is borrowed native storage.
class SceneRenderFrameHost {
public:
    virtual ~SceneRenderFrameHost() = default;

    // Renderer [00F8D394].
    virtual bool device_can_render_slot2c() = 0;
    virtual void device_set_frame_targets_slot98(void* targets) = 0;  // null unbinds
    virtual void* device_depth_surface_slot12c() = 0;
    virtual void device_slot114(const std::uint32_t words[3]) = 0;
    virtual void device_set_texture_slot130(std::uint32_t stage, void* texture) = 0;
    virtual std::uint8_t renderer_guard_mode_0108d6dc() = 0;
    // [01090AB0]: vtable +1Ch returns {int64 elapsed, int64 frequency}; +4 is a float.
    virtual const std::int64_t* clock_interval_slot1c() = 0;
    virtual float clock_float_04() = 0;

    // Command, camera and transform.
    virtual void* command_camera_00b1bf40(void* command) = 0;  // command+28h -> +8h
    virtual void camera_set_near_plane_00b6fbf0(void* camera, float near_plane) = 0;
    virtual std::uint32_t camera_clear_color_00b6fe60(void* camera) = 0;  // camera+190h
    virtual void camera_set_clear_color_00b6fe50(void* camera, std::uint32_t color) = 0;
    virtual void camera_set_clear_flags_00b6fe10(void* camera, std::uint32_t flags) = 0;
    virtual void camera_set_clear_stencil_00b6fe70(void* camera, std::uint32_t stencil) = 0;  // camera+194h
    virtual void camera_clear_00b700a0(void* camera) = 0;  // renderer +8h(0, 0, +188h, &+190h, +18Ch, +194h)
    virtual void camera_execute_frame_command_00b71360(void* camera) = 0;
    virtual void camera_set_render_mode_00b6fdf0(void* camera, std::int32_t mode) = 0;
    virtual const float* camera_projected_y_axis_00b70ea0(void* camera) = 0;
    virtual const float* camera_projected_x_axis_00b70fe0(void* camera) = 0;
    virtual const float* camera_view_projection_00b70490(void* camera) = 0;
    virtual void camera_set_extra_clip_plane_00b70410(void* camera, const float plane[4]) = 0;
    virtual void camera_clear_extra_clip_plane_00b70470(void* camera) = 0;
    virtual void transform_refresh_world_00b6db70(void* node) = 0;  // when !(node+5Ch & 2)
    virtual void invert_affine_00b63b30(float* out, const float* source) = 0;
    virtual void multiply4x4_00413920(const float* left, float* out, const float* right) = 0;
    virtual void node_set_visibility_factor_00b6da70(void* node, float factor, std::uint8_t recurse) = 0;
    virtual void normalize3_00419510(const float in[3], float out[3]) = 0;
    virtual float length3_00419440(const float v[3]) = 0;
    virtual const float* invert4x4_00b632d0(const float* source, float* out) = 0;  // returns out
    virtual const float* build_perspective_00b642f0(float* out, float fov, float aspect, float near_plane,
                                                    float far_plane) = 0;
    virtual const float* transform4_00b62d10(const float source[4], float* out, const float* matrix) = 0;
    virtual void upload_system_prefix_00b46a70(void* owner, void* camera) = 0;  // ECX owner, EDX camera

    // Diagnostics label [00F8D39C]+684h.
    virtual void assign_global_command_label(const char text[4]) = 0;  // inline NativeString assign
    virtual void reset_command_label_00b13510() = 0;                  // label := "X"

    // Batch lists command+0Ch / +10h.
    virtual std::int32_t batch_count_00b51b20(void* list) = 0;
    // 004C1130 pool, 00B0FFB0 job; pool+4 vtable +4(job, list).
    virtual void frame_job_pool_submit(void* list) = 0;
    virtual void frame_job_pool_wait_slot8(std::uint32_t value) = 0;  // pool+4 vtable +8
    virtual void batch_list_prepare_slotc(void* list, std::uint32_t index) = 0;
    virtual void batch_list_execute_slot8(void* list, std::uint32_t index, void* camera) = 0;

    // Render textures, targets and passes.
    virtual void* holder_texture_00b4cb10(void* holder) = 0;  // holder+8h
    virtual std::uint32_t texture_width_slot3c(void* texture) = 0;
    virtual std::uint32_t texture_height_slot40(void* texture) = 0;
    virtual void* holder_primary_surface_00b4cb20(void* holder) = 0;  // holder+0Ch
    virtual void holder_resolve_00b4caf0(void* holder) = 0;
    virtual void* shadow_map_texture_holder_00b4d170(void* map) = 0;
    virtual void* bloom_output_holder_00b54cd0(void* bloom) = 0;
    virtual void targets_set_color_00b1fab0(void* targets, std::uint32_t index, void* surface) = 0;
    virtual void targets_set_depth_00b1fb00(void* targets, void* surface) = 0;
    virtual void targets_set_srgb_write_00b1f700(void* targets, std::uint8_t value) = 0;
    virtual void* pass_material_00b4cba0(void* pass) = 0;
    virtual void material_set_texture_00b189f0(void* material, std::uint32_t slot, void* texture) = 0;
    virtual void pass_assign_targets_00b4e2b0(void* pass, void* targets) = 0;
    virtual void pass_draw_00b4dcc0(void* pass, std::uint32_t unused) = 0;
    virtual void object_slot10(void* object) = 0;  // vtable +10h on +18h/+1Ch/+28h/+2Ch/+60h
    virtual void surface_stretch_from_00b3cda0(void* destination, void* source) = 0;  // StretchRect, linear
    virtual bool cockpit_node_visible_00b3c550(void* cockpit) = 0;  // cockpit helper +8h node, node+44h == 0
    virtual void cockpit_begin_frame_00b3c960(void* cockpit, void* camera, void* targets) = 0;
    virtual void cockpit_draw_00b3c6e0(void* cockpit, void* camera, void* targets) = 0;
    virtual void luminance_measure_00b50050(void* luminance, float value) = 0;
    virtual void tonemap_set_params_draw_00b548c0(void* pass, void* holder, float a, float b, float c, float d,
                                                  float e) = 0;
    virtual void distortion_render_00b4f1c0(void* distortion, void* camera) = 0;
    virtual void distortion_extent_00b4ed30(void* distortion, std::uint32_t out[2]) = 0;
    virtual void gui_aspect_extent_00aa1fe0(float out[2]) = 0;
    virtual std::uint32_t viewport_width_slot48(void* viewport) = 0;
    virtual std::uint32_t viewport_height_slot4c(void* viewport) = 0;
    virtual void render_old_film_00b529a0(void* state, void* targets, void* params, std::uint8_t flag,
                                          float extent_y, float amount) = 0;

    // Queue used by 00B101C0 / 00B10500.
    virtual void* render_command_new_00b1f170(void* scene, void* camera, void* owner) = 0;  // vtable D5E13C
    virtual void command_queue_append_00b1eb80(void* command, std::uint32_t flags) = 0;
    virtual void command_queue_set_last_metadata_00b1cb50(const std::uint32_t words[3]) = 0;
    // Queue singleton 004C11F0. 00B1F5E0 builds a 44h command (scene = camera+A4h,
    // camera, targets) and, when node+48h & camera+19Ch, collects the node into it.
    virtual void command_queue_push_node_00b1f5e0(void* scene, void* camera, void* targets, void* node) = 0;
    virtual void command_queue_execute_last_00b1e6d0() = 0;
    virtual void camera_retain(void* camera) = 0;   // InterlockedIncrement(camera+4)
    virtual void camera_release(void* camera) = 0;  // InterlockedDecrement; vtable +0 at zero

    // Direct callees reconstructed in this file (hosts may route to them).
    virtual void camera_history_00b0db00(void* history, float time, void* camera) = 0;
    virtual void frustum_corners_00b0eb10(void* service, void* camera, float* out16) = 0;
    virtual void excluded_node_pass_00b10500(void* service, void* command, void* targets) = 0;
    virtual void stats_overlay_00b151e0(void* service) = 0;
};

// 00B0EB10 `void __thiscall(service unused, camera, float out[16])`, RET 8.
// Far-plane corner rays: perspective(camera +1C4h fov, +1C8h aspect, near =
// +1D4h or 0.5 when 0.5 > near, +1D8h far), inverted; NDC (-1,-1), (-1,1),
// (1,-1), (1,1) at z 0 transformed and divided by w, scaled by far/near,
// rotated by the camera world 3x3 (+F0h rows). out = four float4, w = 1.0.
// Callers 00B156C0 (out = service+B4h) and 00BA0090.
void frustum_corners_00b0eb10(void* service, void* camera, float* out16, SceneRenderFrameHost& host);

// 00B101C0 `void __thiscall(service, scene, camera)`, RET 8: motion-blur
// tracking of node +1C0h, then queue the scene-render command (vtable D5E13C,
// 00B17240) for (scene, camera, +1D4h). Callers BSP_FrontEndPreview_Draw
// (00503510) and 0068A0D0.
void submit_scene_render_00b101c0(void* service, void* scene, void* camera, SceneRenderFrameHost& host);

// 00B10500 `void __thiscall(service, command, targets)`, RET 8: when +22Ch,
// draw the tracked node +1C0h on its own (visibility 1.0, clear flags 0), then
// hide it again (0.0) and restore the camera clear flags.
void excluded_node_pass_00b10500(void* service, void* command, void* targets, SceneRenderFrameHost& host);

// 00B156C0 `void __thiscall(service, command)`, RET 4: the scene render
// (labels DRSH, SCNE, POST). See docs/CC13_SCENE_RENDERER.md section 6.
void render_scene_00b156c0(void* service, void* command, SceneRenderFrameHost& host);

// Typed projection of the 12Ch render-statistics record at service +690h..+694h.
// Not the native layout: native names are NativeString {length, text} at
// +11Ch (the pass label current when the producer 00B16F80 created the
// record) and +124h (label + effect name, the sort key); records swap by deep
// copy. Counters per render mode, added by 00B16F80
// (docs/NATIVE_MATERIAL_DIAGNOSTICS_DRAW_CN.md).
struct RenderStatsRecord {
    const void* effect{};                 // +0
    std::int32_t draw_calls[14]{};        // +4,   "DP"
    std::int32_t primitives[14]{};        // +3Ch, "PRM"
    std::int32_t vertices[14]{};          // +74h, not printed
    std::int32_t vs_float4[14]{};         // +ACh, "VSF4"
    std::int32_t ps_float4[14]{};         // +E4h, "PSF4"
    std::string name;                     // +11Ch, label (DRSH/SCNE/POST)
    std::string sort_key;                 // +124h, label + effect name
};

class StatsOverlayHost {
public:
    virtual ~StatsOverlayHost() = default;
    virtual bool enabled_681() = 0;                                  // service byte +681h
    virtual std::vector<RenderStatsRecord>& records_690() = 0;       // vector +690h..+694h
    virtual const std::vector<std::string>& filter_69c() = 0;        // vector +6A0h..+6A4h
    virtual std::string effect_name_00b172d0(const void* effect) = 0;
    // 004C14C0 sink + 004B47E0: the print is a bare RET in this image.
    virtual void debug_print_004b47e0(float x, float y, const std::string& text) = 0;
};

// 00B105B0 `iterator* __thiscall(service, iterator* out, const NativeString* name)`,
// RET 8: linear case-insensitive find in the +69Ch name vector (same length,
// then _stricmp). Returns end when absent.
std::size_t find_filter_name_00b105b0(const std::vector<std::string>& filter, const std::string& name);

// 00B151E0 `void __fastcall(service)`: the per-effect render-statistics overlay.
// 00B148E0 (std::sort, 12Ch elements) with 00B13790/00B13BF0/00B13C90/00B13D00/
// 00B13F80/00B14400 ordered by record_name_less_00b0daa0 on the sort key.
void stats_overlay_00b151e0(StatsOverlayHost& host);

// 00B0DB00 `void __thiscall(history = service+314h, float time, camera)`, RET 8.
// Delayed camera: an 8-slot history of {world 4x4, fov, time} (48h each, from
// +8h, count +4h, delay +0h); a jump beyond 20.0 [00CE3930] from the last
// position (+288h) restarts it. Interpolates the pair bracketing time - delay
// and stores inverse(view) * perspective at +248h (the previous view-projection
// the scene render copies to service+5A8h).
void camera_history_00b0db00(void* history, float time, void* camera, SceneRenderFrameHost& host);

// ---------------------------------------------------------------------------
// Packet cc13_seg83_queue: render command queue helpers (segment 83) and the
// cockpit helper frame calls. Queue layout as docs/RENDER_COMMAND_QUEUE.md:
// command pointers data/count/capacity +14h/+18h/+1Ch; world-label rows
// data/count/capacity +24h/+28h/+2Ch, 14h each {NativeString text, float pos[3]}.
// ---------------------------------------------------------------------------

class RenderQueueHost {
public:
    virtual ~RenderQueueHost() = default;
    // operator new(44h) + 00B1F170(scene, camera, targets); null when the allocation fails.
    virtual void* render_command_new_00b1f170(void* scene, void* camera, void* targets) = 0;
    virtual void command_pointers_reserve_00b1c6c0(void* vector14, std::int32_t capacity) = 0;
    virtual void node_collect_slot20(void* node, void* context, float a, float b, std::uint32_t c) = 0;
    virtual void instance_groups_upload_00b1e990(void* command) = 0;
    virtual void native_string_assign(void* destination, const std::uint32_t source[2]) = 0;  // inline 0041DD40 + memcpy
    virtual void native_string_free(const std::uint32_t source[2]) = 0;  // pool return of the by-value argument
    virtual void scene_traverse_00b72190(void* scene, void* context) = 0;
    virtual void command_execute_slot0(void* command) = 0;
    virtual void command_destroy_00b1ddd0(void* command) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    // 00B1EF60.
    virtual const float* camera_view_projection_00b70490(void* camera) = 0;
    virtual const float* transform4_00b62d10(const float source[4], float* out, const float* matrix) = 0;
    virtual void label_rows_resize_00b1e7f0(void* rows24, std::uint32_t count) = 0;
    virtual void debug_print_004b47e0(float x, float y, const char* format, const char* text) = 0;  // bare RET
};

// 00B1BF40 `camera __thiscall(command)`, RET: command+28h (context) -> +8h.
// (SceneRendererHost::command_prefix_00b1bf40 above is this camera.)

// 00B1F5E0 `void __thiscall(queue, scene, camera, targets, node)`, RET 10h:
// append a new command for (scene, camera, targets); when node+48h &
// camera+19Ch, node vtable +20h(command+28h, 0.0, 1.0, 0) collects the node and
// 00B1E990 uploads it. A failed allocation appends null and then faults on
// null+28h in the native. Callers 00B10500, 00B3C6E0, 00A8E180.
void submit_drawable_00b1f5e0(void* queue, void* scene, void* camera, void* targets, void* node,
                              RenderQueueHost& host);
// 00B1F4D0 `void __thiscall(queue, scene, camera, targets, NativeString label)`,
// RET 14h: as 00B1F5E0 but command+14h = label, the whole scene is traversed
// (00B72190(scene, command+28h)), and the by-value label is freed.
void submit_scene_00b1f4d0(void* queue, void* scene, void* camera, void* targets, const std::uint32_t label[2],
                           RenderQueueHost& host);
// 00B1E6D0 `void __thiscall(queue)`, RET: run the last command (vtable +0),
// destroy and free it, drop it from the count.
void execute_last_00b1e6d0(void* queue, RenderQueueHost& host);
// 00B1EF60 `void __thiscall(queue, camera)`, RET 4: project each world label
// through the camera view-projection; inside the clip volume (|x|,|y| <= w,
// 0 <= z <= w) print "%s\n" at (x/w*0.5+0.5)*1024, (0.5-y/w*0.5)*768 (the
// print is a bare RET); then clear the rows. Null camera clears only.
void draw_world_labels_00b1ef60(void* queue, void* camera, RenderQueueHost& host);

class CockpitHost {
public:
    virtual ~CockpitHost() = default;
    virtual void release(void* object) = 0;  // InterlockedDecrement(+4), vtable +0 at zero
    virtual void* camera_viewport_00b6fde0(void* camera) = 0;
    virtual void camera_set_fov_00b6fbb0(void* camera, float fov) = 0;
    virtual void camera_set_fog_owner_00b71940(void* camera, void* owner) = 0;
    virtual void camera_set_aspect_00b6fbd0(void* camera, float aspect) = 0;
    virtual const float* camera_view_matrix_00b6fcb0(void* camera) = 0;
    virtual void camera_set_view_matrix_00b71490(void* camera, const float* matrix) = 0;
    virtual void camera_set_render_mode_00b6fdf0(void* camera, std::int32_t mode) = 0;
    virtual void transform_refresh_world_00b6db70(void* node) = 0;
    virtual void node_set_world_slot34(void* node, const float* matrix) = 0;
    virtual void node_set_local_slot38(void* node, const float* matrix) = 0;
    virtual const float* node_bounds_slot44(void* node, float* out) = 0;  // +0Ch radius
    virtual void node_set_visibility_factor_00b6da70(void* node, float factor, std::uint8_t recurse) = 0;
    virtual void* scene_lighting_owner_00b72110(void* scene) = 0;
    virtual const std::uint8_t* render_options_00f8bbf0() = 0;  // +0Ch, +0Dh shadow switches
    virtual void* first_light_node(void* lighting) = 0;  // list +1Ch head ->next; CRT check when empty
    virtual void* light_shadow_map_owner_00b7aab0(void* light) = 0;
    virtual void shadow_render_single_view_00a8e180(void* shadow, void* node, void* camera, const float min[3],
                                                    const float max[3]) = 0;
    virtual void submit_drawable_00b1f5e0(void* scene, void* camera, void* targets, void* node) = 0;
    virtual void execute_last_00b1e6d0() = 0;
};

// 00B3C960 `void __thiscall(helper, main camera, targets)`, RET 8. Helper +8h
// node, +0Ch cockpit camera. A hidden node (+44h) is released and cleared.
// Otherwise: cockpit viewport size, fov, fog owner and aspect from the main
// camera; node world translation = node - main camera position; cockpit view
// = main view with zero translation; node visibility 1.0; with both shadow
// switches on, the first light's shadow owner renders the node over the
// cockpit camera position +- the node's bound radius and the command runs.
void cockpit_begin_frame_00b3c960(void* helper, void* main_camera, void* targets, CockpitHost& host);
// 00B3C6E0 `void __thiscall(helper, main camera, targets)`, RET 8: hidden node
// released; else visibility 1.0, cockpit camera render mode 12, submit
// (main camera+A4h scene, cockpit camera, targets, node) and run it,
// visibility 0.0, node vtable +38h(identity).
void cockpit_draw_00b3c6e0(void* helper, void* main_camera, void* targets, CockpitHost& host);

}  // namespace bsp::scene_renderer
