#pragma once

// Presentation reconstructions for segments 92-94 (00B999F0-00BD2E60): ocean wave field and
// generators, ocean particles and beams, coast mesh and coastline extractor (shipped but
// unreachable), water tracer, shore-wave texture source, archives (msar/mpkg) under them, cloud
// manager/instances/puffs/class, waterdrops, lightning strands and thunder sprites, model pools.
// Packet cc13-water1, docs/CC13_WATER_1.md. Names are hypotheses, not recovered symbols. Every
// function carries its native address; the ledger records (config/reconstruction) carry the ABI and
// evidence. Build-tested only: nothing is wired into a host, fixture-tested or ABI-compatible.
//
// x87 model: the image runs at 53-bit precision; each FSTP to a float slot is a float cast and the
// intermediates are modelled as double, so results are close to native but not bit-exact.
// RNG: no function here draws random numbers. Functions whose native code draws from 00BD2F10
// (stream 0, the shared gameplay stream) take the drawn values as inputs or through a host callback, and say so.

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "bsp/engine_services_cc13.hpp"

// ================================================================================================
// Part a: 00B999F0-00BA2FFF
// ================================================================================================
// cc13-water1 helper part "a": ocean wave field (00B999F0-00BA2FFF). Semantic C++ over
// modelled inputs; every name is a hypothesis, not a recovered symbol. Evidence per function
// in w1_a.cpp and local/w1_a_records.jsonl. Not ABI-compatible, not wired into any host.
namespace bsp::water_weather_cc13 {

using W1A_OceanMatrix = std::array<float, 16>;  // row-major, native 4x4 float layout

// ---------------------------------------------------------------------------
// Base vtable 00D63B5C stubs (also reused by the derived vtable 00D63E10).

// 00B9A2F0 `void __thiscall(field)`, RET: slot 2, does nothing.
void ocean_field_base_noop_00b9a2f0();
// 00BA2460 `void __thiscall(field, int)`, RET 4: base slot 4 (00BA6BD0 overrides it).
void ocean_field_base_set_quality_noop_00ba2460(int quality);
// 00BA2470 `int __thiscall(field, a, b, c)`, XOR EAX,EAX; RET 0Ch: slot 6.
int ocean_field_base_return_zero_00ba2470(std::uint32_t a, std::uint32_t b, std::uint32_t c);
// 00BA2480 RET 4 (slot 7) and 00BA2490 RET 8 (slot 8).
void ocean_field_base_noop4_00ba2480(std::uint32_t a);
void ocean_field_base_noop8_00ba2490(std::uint32_t a, std::uint32_t b);

// 00B9A3B0 `int* __thiscall(field, int out[2])`, RET 4, EAX = out: slot 5 in both vtables.
struct W1A_OceanTargetSize {
    std::int32_t width{0};
    std::int32_t height{0};
};
W1A_OceanTargetSize ocean_field_reflection_target_size_00b9a3b0();

// ---------------------------------------------------------------------------
// Small setters.

// Reference-counted native object, modelled as an opaque handle with a counter.
struct W1A_OceanRefObject {
    std::int32_t refs_04{0};
};

// 00B9DBB0 `void __thiscall(field, ref, float r, float g, float b, float max_darken)`, RET 14h.
struct W1A_OceanUnderwaterFog {
    W1A_OceanRefObject* fog_ref_2ac{nullptr};
    std::array<float, 4> fog_color_3ac{};  // +3ACh..+3B8h
};
// Returns the object whose count reached zero (the native calls its vtable slot 0), or null.
W1A_OceanRefObject* ocean_field_set_underwater_fog_00b9dbb0(W1A_OceanUnderwaterFog& field,
                                                             W1A_OceanRefObject* fog,
                                                             const std::array<float, 4>& color);

// 00B9DD90 `void __thiscall(field, spectrum)`, RET 4.
struct W1A_OceanSpectrumLink {
    W1A_OceanRefObject ref;   // +4 refcount
    void* owner_field_08{nullptr};
};
W1A_OceanRefObject* ocean_field_set_spectrum_00b9dd90(W1A_OceanSpectrumLink*& spectrum_bc,
                                                       void* field,
                                                       W1A_OceanSpectrumLink& spectrum);

// Texture binding host for 00B9DB50 / 00B9DE30 / 00B9E8B0.
struct W1A_OceanTextureHost {
    virtual ~W1A_OceanTextureHost() = default;
    // [00F8D394] vtable +64h: load a texture by name, flags 0.
    virtual void* load_texture_f8d394_64(const std::string& name) = 0;
    // 00B70DC0-family BSP_Material_SetTextureSlot on material `which` (0 = field+12Ch,
    // 1 = field+130h, 2 = foam model geometry 0 element 0 material +20h).
    virtual void set_texture_slot(int which, int slot, void* texture) = 0;
    // InterlockedDecrement on texture+4 and slot-0 call at zero.
    virtual void release_texture(void* texture) = 0;
};
// 00B9DB50 `void __thiscall(field, const NativeString& name)`, RET 4: slot 2 of both materials.
void ocean_field_set_reflection_cubemap_00b9db50(W1A_OceanTextureHost& host,
                                                 const std::string& name);
// 00B9DE30 same ABI: slot 5 of both materials.
void ocean_field_set_normal_map_texture_00b9de30(W1A_OceanTextureHost& host,
                                                 const std::string& name);

// 00B9E8B0 SetFoam `void __thiscall(field, bool use, float fade_range, float scroll,
// float noise_scroll, float height, float scale, float noise_scale, const NativeString* tex,
// const NativeString* noise_tex)`, RET 24h.
struct W1A_OceanFoamState {
    bool use_foam_84{false};
    float scroll_speed_88{0.0f};
    float fade_range_8c{0.0f};
    float noise_scroll_speed_90{0.0f};
    float height_94{0.0f};
    float scale_98{0.0f};
    float noise_scale_9c{0.0f};
    std::string texture_a0;
    std::string noise_texture_a8;
};
struct W1A_OceanFoamHost : W1A_OceanTextureHost {
    virtual bool foam_model_present_120() = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration on the foam model; root null or field+14h.
    virtual void foam_propagate_root_00b6d890(bool to_field_root_14) = 0;
    // 00B6E680 BSP_Node_SetParent(null) on the foam model.
    virtual void foam_set_parent_null_00b6e680() = 0;
};
struct W1A_OceanFoamArgs {
    bool use_foam{false};
    float fade_range{0.0f};
    float scroll_speed{0.0f};
    float noise_scroll_speed{0.0f};
    float height{0.0f};
    float scale{0.0f};
    float noise_scale{0.0f};
    std::string texture;
    std::string noise_texture;
};
void ocean_field_set_foam_00b9e8b0(W1A_OceanFoamState& foam, W1A_OceanFoamHost& host,
                                   const W1A_OceanFoamArgs& args);

// ---------------------------------------------------------------------------
// Water rings (+4F8h pointer vector, 24h-byte records from 00B9E3F0).

struct W1A_OceanRingRecord {
    float position_00[3]{};
    float radius_0c{0.0f};      // a: triangle inradius
    float b_10{0.0f};           // read as float by 00B9CE96 FMUL dword
    float duration_14{0.0f};
    float start_clock_18{0.0f};
    float rate_1c{0.0f};
    float rate_20{0.0f};
};

// 00B9DED0 `bool __thiscall(vector*, void* const* value)`, RET 4: erase the first equal
// pointer by moving the last one over it (no shrink). Returns true when it erased.
bool ocean_ptr_vector_erase_value_swap_last_00b9ded0(std::vector<void*>& v, void* value);

// 00B9E4A0 `void __thiscall(field, ring*)`, RET 4: erase from +4F8h, then free(ring).
// `free_ring` is the CRT free 00BF65AC, called for any non-null ring, found or not.
void ocean_field_remove_water_ring_00b9e4a0(std::vector<void*>& rings_4f8, void* ring,
                                            void (*free_ring)(void*));

// One 28h-byte vertex of the ring triangle stream.
struct W1A_OceanRingVertex {
    float position[3]{};
    float centre[3]{};
    float radius{0.0f};
    float start_clock{0.0f};
    float scaled_b{0.0f};
    float duration{0.0f};
};
struct W1A_OceanRingMeshHost {
    virtual ~W1A_OceanRingMeshHost() = default;
    // Non-empty path: bounds (0,0,0, 1e10) on +504h (00B6E1C0 family), root registration
    // with 00BA7180's result, identity local matrix (vtable +38h), visibility 1.0.
    virtual void show_ring_model_504() = 0;
    // Empty path: 00B6DA70 visibility 0.
    virtual void hide_ring_model_504() = 0;
    // Element +18h = ring count (triangles), +10h = ring count * 3 (vertices).
    virtual void set_ring_counts(std::uint32_t triangles, std::uint32_t vertices) = 0;
};
// 00B9CB80 `void __thiscall(field)`: rebuilds the stream; returns the vertices written.
std::vector<W1A_OceanRingVertex> ocean_field_update_water_ring_mesh_00b9cb80(
    const std::vector<const W1A_OceanRingRecord*>& rings_4f8, W1A_OceanRingMeshHost& host);

// ---------------------------------------------------------------------------
// 00B9AB60 projective texture matrices and shader constants.

struct W1A_OceanProjectiveHost {
    virtual ~W1A_OceanProjectiveHost() = default;
    // 00B70490 BSP_Camera_GetViewProjectionMatrix on camera field+C8h (0) / field+D0h (1).
    virtual W1A_OceanMatrix camera_view_projection_00b70490(int which_camera) = 0;
    // 00413920 BSP_Matrix_Multiply4x4 (ECX = left, stack dst/right), native x87 schedule.
    virtual W1A_OceanMatrix multiply_00413920(const W1A_OceanMatrix& left,
                                              const W1A_OceanMatrix& right) = 0;
};
struct W1A_OceanProjectiveState {
    // inputs
    float scroll_speed_88{0.0f};
    float fade_range_8c{0.0f};
    float noise_scroll_speed_90{0.0f};
    float height_94{0.0f};
    float scale_98{0.0f};
    float noise_scale_9c{0.0f};
    float value_40c{0.0f};
    float wind_direction_59c{0.0f};
    // outputs
    float clock_3c{0.0f};
    W1A_OceanMatrix refraction_matrix_50c{};
    W1A_OceanMatrix reflection_matrix_54c{};
    std::array<float, 4> constants_58c{};
    std::array<float, 4> foam_scale_5a0{};
    std::array<float, 4> foam_scroll_5b0{};
};
// Texture-space bias: diag (0.5, -0.5, 1, 1), translation (tx, ty, 0, 1).
W1A_OceanMatrix ocean_projective_bias_a(float tx, float ty);
// reflection_size = vtable slot 5 (00B9A3B0, always 512 in both vtables); screen_* =
// [00F8D394] vtable +80h results ([eax] of the first call, [eax+4] of the second);
// global_010909b4 = the float at 010909B4.
void ocean_field_update_projective_matrices_00b9ab60(W1A_OceanProjectiveState& s,
                                                     W1A_OceanProjectiveHost& host,
                                                     std::int32_t reflection_size,
                                                     std::int32_t screen_width,
                                                     std::int32_t screen_height,
                                                     float global_010909b4);

// ---------------------------------------------------------------------------
// 00B9D490 / 00B9E0C0: 2-D grid of 18h-byte cells {width +0, height +4, data +8}.

struct W1A_OceanGridCell {
    std::uint32_t words[6]{};
};
struct W1A_OceanCellGrid {
    std::int32_t width_00{0};
    std::int32_t height_04{0};
    std::vector<W1A_OceanGridCell> cells_08;
};
void ocean_cell_grid_resize_00b9d490(W1A_OceanCellGrid& grid, std::int32_t width,
                                     std::int32_t height);
W1A_OceanCellGrid& ocean_cell_grid_assign_00b9e0c0(W1A_OceanCellGrid& dst,
                                                   const W1A_OceanCellGrid& src);

// ---------------------------------------------------------------------------
// 00B9FE00 RemoveFloatingObject `void __thiscall(field, object*)`, RET 4.

struct W1A_OceanFloatNode {
    void* owner_00{nullptr};
    void* node_04{nullptr};
};
struct W1A_OceanCoverageRecord {  // 30h bytes in the image
    void* owner_00{nullptr};
    std::vector<std::uint8_t> mask_04;   // +4 data, +8 size, +0Ch capacity
    std::uint32_t words_10[8]{};          // +10h..+2Ch copied verbatim
};
struct W1A_OceanFloatHost {
    virtual ~W1A_OceanFloatHost() = default;
    virtual void node_set_parent_null_00b6e680(void* node) = 0;
    virtual void node_propagate_root_null_00b6d890(void* node) = 0;
    virtual void node_unlink_and_release_00b6dfa0(void* node) = 0;
};
void ocean_field_remove_floating_object_00b9fe00(std::vector<void*>& objects_5f4,
                                                 std::vector<W1A_OceanFloatNode>& nodes_614,
                                                 std::vector<W1A_OceanCoverageRecord>& regions_620,
                                                 void* object, W1A_OceanFloatHost& host);

// ---------------------------------------------------------------------------
// 00BA0090 Update fragments (the clock/spectrum step and normal-map scroll are already in
// src/ocean_spectrum.cpp and src/water_render.cpp).

// 00BA0853..00BA0ACC: nine float4 texel offsets at +2C0h..+34Ch.
std::array<std::array<float, 4>, 9> ocean_field_depth_sample_kernel_00ba0090(
    std::int32_t screen_width, std::int32_t screen_height);

struct W1A_OceanWaveScaleState {
    float height_scale_b8{0.0f};
    float wave_scale_28{0.0f};
    float height_scale_2c{0.0f};
    float wave_scale_30{0.0f};
    float amplitude_24{0.0f};
    void* height_target_274{nullptr};
    void* height_target_278{nullptr};
};
// 00BA0B32..00BA0B6D: before 00BA70F0 / 00BA8EE0 / 00B9F0A0 run.
void ocean_field_wave_scale_00ba0090(W1A_OceanWaveScaleState& s, float spectrum_choppy_88);
// 00BA0B90..00BA0BAA: after 00B9F0A0.
void ocean_field_publish_amplitude_00ba0090(W1A_OceanWaveScaleState& s);

// ---------------------------------------------------------------------------
// 00BA2B50 `void __thiscall(field, float out[2], float x, float z)`, RET 0Ch.

struct W1A_OceanSpectrumSampler {
    virtual ~W1A_OceanSpectrumSampler() = default;
    // ECX = field+BCh (the spectrum); (u, v) in [0, 1).
    virtual float sample_00b962e0(float u, float v) = 0;
    virtual float sample_00b96450(float u, float v) = 0;
};
void ocean_field_sample_horizontal_displacement_00ba2b50(float layer_scale_b4,
                                                         W1A_OceanSpectrumSampler& spectrum,
                                                         float x, float z, float out[2]);

// ---------------------------------------------------------------------------
// 00B9F0A0 RenderReflectionAndRefraction fragment (decision and above-water camera).

struct W1A_OceanReflectionCameraSetup {
    bool underwater_cc{false};
    float near_plane{0.0f};
    float far_plane{0.0f};
    float field_of_view{0.0f};
    float aspect{0.0f};
    W1A_OceanMatrix mirrored_world{};
};
// camera_world = camera+F0h; near/far/fov/aspect = camera+1D4h/+1D8h/+1C4h/+1C8h;
// sea_height = 0078C890 at (camera+120h, camera+128h). The mirror is filled only when
// underwater_cc is false (the path this fragment covers).
W1A_OceanReflectionCameraSetup ocean_field_reflection_camera_00b9f0a0(
    const W1A_OceanMatrix& camera_world, float camera_near_1d4, float camera_far_1d8,
    float camera_fov_1c4, float camera_aspect_1c8, float sea_height);

// ---------------------------------------------------------------------------
// 00B9FA60 CollectTerrainMaterials fragment: the per-element name test.
bool ocean_field_is_terrain_effect_00b9fa60(std::string_view effect_name);

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part b: 00BA3000-00BBBFFF
// ================================================================================================
// cc13-water1 helper b: segment 92-93 band 00BA3000-00BBBFFF (ocean height generator and
// particles, skinned water tracer, coastline extractor and coast mesh (shipped but unreachable,
// docs/CC13_WATER_RENDER.md section 5), MSAR/MPKG archive leaves).
// Every function is a hypothesis over modelled inputs, not a recovered symbol, not ABI-compatible,
// not wired into a host, not fixture- or game-validated. x87 model: 53-bit precision, each FSTP to a
// float slot is a float cast, intermediates double.
namespace bsp::water_weather_cc13 {

// ---------------------------------------------------------------------------------------------
// Shared small types (tag b).
struct W1B_Vec2 {
    float x{0.0f};
    float y{0.0f};
};
struct W1B_Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};
struct W1B_Vec4 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float w{0.0f};
};

// ---------------------------------------------------------------------------------------------
// Ocean water particles (world+B0h, vtable 00D63BB4).
struct OceanWaterParticlesState {
    float scale_24{0.0f};
    std::array<float, 8> constants_40{};  // +40h..+5Ch, copied to +ECh..+108h
    W1B_Vec3 volume_offset_60{};           // +60h..+68h
    W1B_Vec3 volume_size_70{};             // +70h..+78h
    float time_c0{0.0f};                   // +C0h
    std::uint32_t enabled_20{0};           // +20h (non-zero: run the visibility tail)
    // outputs
    W1B_Vec3 volume_size_cc{};             // +CCh..+D4h
    float one_d8{0.0f};                    // +D8h
    W1B_Vec3 volume_corner_dc{};           // +DCh..+E4h
    float one_e8{0.0f};                    // +E8h
    std::array<float, 8> constants_ec{};   // +ECh..+108h
    W1B_Vec3 camera_10c{};                 // +10Ch..+114h
    float scaled_time_118{0.0f};           // +118h
};
// Result of the visibility tail: nullopt when +20h == 0 (no SetVisibilityFactor call).
std::optional<float> ocean_water_particles_update_00ba40f0(OceanWaterParticlesState& state,
                                                           const W1B_Vec3& camera_position,
                                                           float time);

// ---------------------------------------------------------------------------------------------
// Ocean wave field height texture (vtable slot 3, float16 upload).
struct OceanHeightTexel {
    std::uint32_t level{0};
    std::uint32_t texel_index{0};  // row*width + column inside the level (pitch ignored)
    W1B_Vec4 value{};              // fed to D3DXFloat32To16Array count 4
};
// level_vertices[level] holds the 6-float vertices of that level (source stride 18h bytes).
std::vector<OceanHeightTexel> ocean_wave_field_height_texels_00ba5030(
    const std::vector<std::vector<std::array<float, 6>>>& level_vertices, std::uint32_t width,
    std::uint32_t height, std::uint32_t level_count);

// Projected grid mesh (projocean.mvfm), 00BA5240 BSP_OceanWaveField_BuildProjectedGridMesh,
// __thiscall(ECX = wave field), RET 0; callers 00BA5610 and 00BA6BD0. Listing
// 00BA5240-00BA5609. Field reads: +FCh width W, +100h height H, +11Ch generated model,
// +124h vertex buffer (released, then replaced), +128h vertex layout, +12Ch material,
// +2BCh height generator. Every call goes through this host at the image's point.
struct OceanProjectedGridHost {
    virtual ~OceanProjectedGridHost() = default;
    // 00B74640 BSP_GeneratedModel_GetGeometry (ECX = model)(0), RET 4 -> geometry.
    virtual void* generated_model_get_geometry_00b74640(void* model, std::uint32_t index) = 0;
    // 00B73C10 BSP_MeshObject_ClearDrawSectionOwners (ECX = geometry).
    virtual void geometry_clear_draw_sections_00b73c10(void* geometry) = 0;
    // 00B73B70 BSP_Geometry_SetIndexStream (ECX = geometry)(stream), RET 4.
    virtual void geometry_set_index_stream_00b73b70(void* geometry, void* stream) = 0;
    // InterlockedDecrement(object+4h) through the import [00CE2220]; when it reaches zero,
    // object->vtable[0]() with ECX = object.
    virtual void ref_release(void* object) = 0;
    // The renderer [00F8D394]: vtable[38h](NativeString* name) -> vertex format (a held
    // reference). The name is a temporary NativeString built from 00D5EDD0
    // "projocean.mvfm" (0041DD40 resize to 0Eh, memcpy 0Fh bytes) and released through
    // 00419CC0/00BD1510 right after the call; the temporary is folded into this hook.
    virtual void* renderer_vertex_format_38(const char* name) = 0;
    // vtable[5Ch](vertex count, 1, format) -> vertex buffer.
    virtual void* renderer_create_vertex_buffer_5c(std::uint32_t count, std::uint32_t flag,
                                                   void* format) = 0;
    // vtable[60h](W*H*2, 1, 65h) -> index buffer.
    virtual void* renderer_create_index_buffer_60(std::uint32_t size, std::uint32_t flag,
                                                  std::uint32_t format) = 0;
    // Vertex buffer vtable[10h](count, 0, 0) -> float* and vtable[14h]() (lock/unlock).
    virtual float* vertex_buffer_lock_10(void* buffer, std::uint32_t count, std::uint32_t a,
                                         std::uint32_t b) = 0;
    virtual void vertex_buffer_unlock_14(void* buffer) = 0;
    // Index buffer vtable[0Ch](0, 0, 0) -> std::uint16_t* and vtable[10h]() (lock/unlock).
    virtual std::uint16_t* index_buffer_lock_0c(void* buffer, std::uint32_t a, std::uint32_t b,
                                                std::uint32_t c) = 0;
    virtual void index_buffer_unlock_10(void* buffer) = 0;
    // 00B85EE0 BSP_DrawSectionPool_AllocateStaticSlot (ECX = 60h) -> block or null.
    virtual void* draw_section_pool_allocate_00b85ee0(std::uint32_t size) = 0;
    // 00B857F0 BSP_DrawSection_Construct (ECX = block) -> section.
    virtual std::uint8_t* draw_section_construct_00b857f0(void* block) = 0;
    // 00B864C0 BSP_DrawSection_SetMaterial (ECX = section)(material), RET 4.
    virtual void draw_section_set_material_00b864c0(std::uint8_t* section, std::uint32_t material) = 0;
    // 00B85B80 BSP_DrawSection_AppendVertexStream (ECX = section)(stream), RET 4.
    virtual void draw_section_append_vertex_stream_00b85b80(std::uint8_t* section,
                                                            std::uint32_t stream) = 0;
    // 00BA7080 BSP_OceanHeightGenerator_GetCurrentStream (ECX = generator) -> stream.
    virtual std::uint32_t ocean_height_generator_current_stream_00ba7080(void* generator) = 0;
    // 00B86650 BSP_DrawSection_SetVertexLayout (ECX = section)(layout), RET 4.
    virtual void draw_section_set_vertex_layout_00b86650(std::uint8_t* section,
                                                         std::uint32_t layout) = 0;
    // 00B73C60 BSP_Geometry_AppendDrawSection (ECX = geometry)(section), RET 4.
    virtual void geometry_append_draw_section_00b73c60(void* geometry, std::uint8_t* section) = 0;
};
// Writes W*H {u, v} float pairs into the locked vertex buffer (u steps by float(1/(W-1)),
// v by float(1/(H-1)), each accumulated in float), and a serpentine strip into the locked
// index buffer: rows 1..H-1, direction flipping every row, 16-bit stores, a closing index
// (r-1)*W - dir + c per row. The draw section gets +8h = 5, +Ch = 0, +10h = W*H, +14h = 0,
// +18h = indices written - 2. The index buffer request is H*W*2 while (H-1)*(2W+1)
// indices are written (the factory's size unit is not established). The draw-section
// block is constructed only when the pool returns one, but the field stores that follow
// have no null test, as in the image.
void ocean_wave_field_projected_grid_00ba5240(std::uint8_t* field, OceanProjectedGridHost& host);

// Scene-colour capture predicate (+F9h flat byte, +CCh byte, colour target present).
bool ocean_wave_field_capture_scene_color_runs_00ba51c0(bool flat_f9, bool flag_cc,
                                                        bool target_present);

// Ocean height generator (vtable 00D63E34 / 00D64168).
struct OceanHeightGeneratorState {
    std::array<std::uint32_t, 4> targets_68{};    // +68h.. (GetTarget index)
    std::array<std::uint32_t, 2> streams_78{};    // +78h/+7Ch vertex streams
    std::uint32_t write_index_80{0};              // +80h
    std::uint32_t current_index_84{0};            // +84h
    std::array<float, 6> tap_weights_a0{};        // +A0h..+B4h
    W1B_Vec3 wave_scale_100{};                    // +100h..+108h
};
std::uint32_t ocean_height_generator_current_stream_00ba7080(const OceanHeightGeneratorState& g);
std::uint32_t ocean_height_generator_target_00ba7090(const OceanHeightGeneratorState& g,
                                                     std::uint32_t index);
void ocean_height_generator_set_wave_scale_00ba70f0(OceanHeightGeneratorState& g,
                                                    const W1B_Vec3& scale);
struct OceanViewportSize {
    std::int32_t width{0};
    std::int32_t height{0};
};
// render_scale = dword 00E14454 (data, initial 2).
OceanViewportSize ocean_height_generator_viewport_size_00ba7120(std::int32_t width,
                                                                std::int32_t height,
                                                                std::int32_t render_scale);
// Resize fragment: reported dimensions of the four targets (scale = dword 00E144E8).
std::array<OceanViewportSize, 4> ocean_height_generator_resize_dimensions_00bb3ee0(
    std::int32_t width, std::int32_t height, std::int32_t scale);
// Readback fragment: the double-buffer swap after the copy.
void ocean_height_generator_readback_swap_00bb3ce0(OceanHeightGeneratorState& g);

struct OceanCrtHost {
    virtual ~OceanCrtHost() = default;
    virtual double crt_log_00bfa910(double x) = 0;  // ST0 in/out (provisional _CIlog)
    virtual double crt_exp_00bfeeec(double x) = 0;  // ST0 in/out (provisional _CIexp)
};
// Render fragment: six geometric tap weights from settings+18h, normalised (+A0h..+B4h).
std::array<float, 6> ocean_height_generator_tap_weights_00ba8ee0(OceanCrtHost& host,
                                                                 float ratio_18);

// ---------------------------------------------------------------------------------------------
// Skinned water tracer (wakes).
struct TracerHost {
    virtual ~TracerHost() = default;
    virtual float reciprocal_length_00419260(float x, float y) = 0;
    virtual double crt_sqrt_00bf7030(double x) = 0;
    virtual double crt_acos_00bf9940(double x) = 0;
    virtual double crt_round_00bf85b0(double x) = 0;  // unresolved CRT rounding ABI
    virtual double ocean_sample_height_0078c890(float x, float z) = 0;
    virtual W1B_Vec4 transform_point_00b62d10(const float* position) = 0;  // by node +F0h
};

// Growth rules of the tracer containers.
std::uint32_t tracer_ptr_vector_grow_capacity_00ba9d10(std::uint32_t capacity);
std::uint32_t tracer_ptr_vector_push_capacity_00bac190(std::uint32_t size, std::uint32_t capacity);
std::uint32_t tracer_parameter_curve_push_capacity_00bac2c0(std::int32_t size,
                                                            std::int32_t capacity);

// The 16-float vertex record BuildWidthProfile writes and SampleWaterContact reads.
using TracerVertex = std::array<float, 16>;
// width/alpha samples: count = generator+10h.
TracerVertex tracer_build_width_profile_00baa0f0(TracerHost& host, std::uint32_t count,
                                                 float value_0, float offset, float value_14,
                                                 float value_15);

// Point list node (+1B8h chain, link +28h).
struct TracerPoint {
    float x_00{0.0f};
    float y_04{0.0f};
    float dir_x_08{0.0f};
    float dir_y_0c{0.0f};
    float depth_10{0.0f};
    float segment_length_14{0.0f};
    float value_18{0.0f};
    float width_1c{0.0f};
    float value_20{0.0f};
    float value_24{0.0f};
    int next_28{-1};      // index into the list, -1 = null
    int toward_2c{-1};    // neighbour the direction points at, -1 = toward the origin
};
struct TracerBoneState {
    float curve_limit_4c{0.0f};  // [+190h]+4Ch
    W1B_Vec2 origin_234{};       // +234h/+238h
    float cos_23c{0.0f};         // +23Ch
    float sin_240{0.0f};         // +240h
};
void tracer_update_bones_00baa8f0(TracerHost& host, const TracerBoneState& state,
                                  std::vector<TracerPoint>& points, int head);

struct TracerContactState {
    float first_segment_length{0.0f};  // [+1B8h]+14h
    float scale_1d8{0.0f};             // +1D8h
    float depth_base_1c4{0.0f};        // +1C4h
    float depth_scale_1e4{0.0f};       // +1E4h
    float depth_offset_1e8{0.0f};      // +1E8h
};
// bones = the +1A0h pointer table (00BAC730 order).
float tracer_sample_water_contact_00bab3f0(TracerHost& host, const TracerContactState& state,
                                           const std::vector<TracerPoint>& bones,
                                           const TracerVertex& vertex);
// Per vertex of the tracer mesh: copy the 16 floats, replace [14] with the contact height.
std::vector<TracerVertex> tracer_rebuild_vertices_00bac730(TracerHost& host,
                                                           const TracerContactState& state,
                                                           const std::vector<TracerPoint>& bones,
                                                           const std::vector<TracerVertex>& source);
// Static tracer: copy 8-float vertices, y += 0.5 * height at the transformed (x, z).
std::vector<std::array<float, 8>> tracer_conform_to_water_00bab930(
    TracerHost& host, const std::vector<std::array<float, 8>>& source);

// Render constants (pass id 4 only).
struct TracerRenderRow {
    float x{0.0f};
    float y{0.0f};
    float angle{0.0f};
    float value_1c{0.0f};
    float fade{0.0f};
    float depth{0.0f};
    float width{0.0f};
    float value_20{0.0f};
};
struct TracerRenderState {
    bool static_202{false};
    float value_18c{0.0f}, value_1c4{0.0f}, value_1d8{0.0f}, value_1dc{0.0f}, value_1e4{0.0f},
        value_1e8{0.0f}, value_1ec{0.0f}, value_1f0{0.0f}, value_1f4{0.0f}, value_1fc{0.0f},
        value_204{0.0f}, value_208{0.0f};
    // outputs
    float out_798{0.0f};
    std::array<float, 8> header_268{};  // +268h..+284h
    float out_258{0.0f}, out_25c{0.0f}, out_260{0.0f};
    std::vector<TracerRenderRow> rows;  // +288h (x,y,angle,v) and +508h (fade,depth,width,v)
};
// Returns the native return value (1 for pass 4, else 0).
int tracer_render_00baba50(TracerHost& host, TracerRenderState& state, int pass_id,
                           const std::vector<TracerPoint>& points, int head);

// Static tracer index fragment: rows of ring_counts[r] vertices stitched into a triangle list.
std::vector<std::uint16_t> tracer_static_mesh_indices_00bacbd0(
    const std::vector<std::uint32_t>& ring_counts);

// ---------------------------------------------------------------------------------------------
// Coastline extractor (vtable 00D640DC) - shipped but unreachable.
struct CoastHost {
    virtual ~CoastHost() = default;
    virtual double vector_length_00419440(float x, float y, float z) = 0;
    virtual double crt_round_00bf85b0(double x) = 0;
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;  // CRT operator new
    virtual void free_00bf6989(void* block) = 0;                   // CRT _free
};
struct CoastSegment {  // 1Ch
    W1B_Vec3 a{};
    W1B_Vec3 b{};
    std::uint32_t flags_18{0};  // bit0 start consumed, bit1 end consumed
};
struct CoastPoint {  // 2Ch, built by 00BAED40
    W1B_Vec3 a{};
    W1B_Vec3 b{};
    W1B_Vec3 normal{};
    float u0_24{0.0f};
    float u1_28{0.0f};
};
struct CoastlineState {
    float water_level_2c{0.0f};          // 00BB02C0: 0
    float tolerance_34{0.1f};            // 00BB02C0: float 00D7A2F0 = 0.1
    float time_bc{0.0f};
    std::uint32_t max_points_c0{0};      // +C0h
    float u_scale_fc{0.0f};              // +FCh
    std::vector<CoastSegment> segments;  // +08h/+0Ch/+10h
    std::vector<std::vector<CoastPoint>> polylines;  // +14h/+18h/+1Ch
    std::uint32_t count_30{0};
};
CoastlineState coastline_construct_00bb02c0();
void coastline_set_time_00baec50(CoastlineState& s, float time);
bool coastline_edge_crossing_00baeb20(const CoastlineState& s, const W1B_Vec3& p,
                                      const W1B_Vec3& q, W1B_Vec3& out);
CoastPoint coast_segment_construct_00baed40(CoastHost& host, const W1B_Vec3& a,
                                            const W1B_Vec3& b);
// The native 0Ch growable array the coast code keeps its elements in.
template <class T>
struct CoastNativeVector {
    T* data{nullptr};          // +0h
    std::int32_t size{0};      // +4h
    std::int32_t capacity{0};  // +8h
};
// Reserve, __thiscall(ECX vector, int request) RET 4, nothing returned: the request is floored at
// 20h; when it exceeds the capacity, operator new 00BF55BE(request * element size), the live
// elements copied dword by dword (each skipped when the destination is null), _free 00BF6989 on
// the old data, then data and capacity stored. Elements 4 (00BAEEB0), 1Ch (00BAEFB0), 2Ch
// (00BAF450) bytes.
void coast_ptr_vector_reserve_00baeeb0(CoastHost& host, CoastNativeVector<std::uint32_t>& v,
                                       std::int32_t request);
void coast_segment_vector_reserve_00baefb0(CoastHost& host, CoastNativeVector<CoastSegment>& v,
                                           std::int32_t request);
void coast_point_vector_reserve_00baf450(CoastHost& host, CoastNativeVector<CoastPoint>& v,
                                         std::int32_t request);
// Push back, __thiscall(ECX vector, const T* value) RET 4, nothing returned: when size ==
// capacity, the own reserve with capacity*2 (20h unless above it, signed JG); then the slot
// data+size gets *value unless its address is null (the image's TEST), and size += 1.
// 00BAF350 segments (reserve 00BAEFB0; a/b through the EH vector copy 00453ED0 with the Vec3
// copy constructor 00414220, then +18h), 00BAF3E0 dwords (reserve 00BAEEB0), 00BAFC00 points
// (reserve 00BAF450; a/b as 00BAF350, +18h..+20h FLD/FSTP, +24h/+28h dword moves).
void coast_segment_vector_push_back_00baf350(CoastHost& host, CoastNativeVector<CoastSegment>& v,
                                             const CoastSegment& value);
void coast_ptr_vector_push_back_00baf3e0(CoastHost& host, CoastNativeVector<std::uint32_t>& v,
                                         const std::uint32_t& value);
void coast_point_vector_push_back_00bafc00(CoastHost& host, CoastNativeVector<CoastPoint>& v,
                                           const CoastPoint& value);
void coast_ptr_vector_resize_00baf070(std::vector<std::uint32_t>& v, std::int32_t size);
// 00BB4B40 (0x39 bytes), __thiscall(ECX vector {data +0h, size +4h, capacity +8h},
// const dword* value) RET 4: when size == capacity, 00BB46F0 reserve(capacity*2, or 1 when
// that is <= 1, signed JG); then the slot data+size*4 gets *value unless the slot address
// is null (the image's TEST EAX,EAX), and size += 1. Returns nothing meaningful.
void pak_archive_registry_slots_push_back_00bb4b40(void* vector, const std::uint32_t* value);
// Returns -1 (none), 0 (matched a segment start) or 1 (matched its end); index in out_index.
int coastline_find_nearest_segment_00baf0c0(CoastHost& host, const CoastlineState& s,
                                            const W1B_Vec3& point, std::uint32_t& out_index,
                                            std::uint32_t exclude);
void coastline_clip_triangle_00baf5c0(CoastHost& host, CoastlineState& s, const W1B_Vec3& p0,
                                      const W1B_Vec3& p1, const W1B_Vec3& p2);
void coastline_clip_draw_section_00baf8a0(CoastHost& host, CoastlineState& s,
                                          const std::vector<W1B_Vec3>& positions,
                                          const std::vector<std::uint16_t>& indices,
                                          std::uint32_t first_index, std::uint32_t triangles,
                                          float y_offset);
bool coastline_collect_terrain_section_matches_00baf9a0(std::string_view effect_name);
// Returns the free-end selector (or -1 when no free segment); out_segment = head (or -1).
int coastline_chain_segments_00bafaa0(CoastHost& host, const CoastlineState& s,
                                      std::uint32_t& out_segment);
void coastline_resample_polyline_00bafce0(CoastHost& host, const CoastlineState& s,
                                          std::vector<CoastPoint>& polyline, const W1B_Vec3& a,
                                          const W1B_Vec3& b, float& accumulated);
// Stream reader for coast data (vtable +38h int, +44h float).
struct CoastStreamHost {
    virtual ~CoastStreamHost() = default;
    virtual bool exists_08() = 0;
    virtual std::int32_t read_int_38() = 0;
    virtual float read_float_44() = 0;
};
bool coastline_load_data_00bb0070(CoastStreamHost& stream, CoastlineState& s);
void coastline_build_polylines_00bb05e0(CoastHost& host, CoastlineState& s);
void coastline_build_mesh_00bb07c0(CoastHost& host, CoastlineState& s);
std::uint32_t coastline_build_count_00bb09f0(std::uint32_t polyline_count,
                                             std::uint32_t point_count);

// Coast mesh (vtable 00D640E4).
void coast_mesh_set_time_00bb0a90(float& time_120, float time);
// GenerateDistanceMap fragment: marching-squares cell classification and edge allocation.
struct CoastMarchingCell {
    std::int32_t edge_down{-1};   // vertical edge vertex id (cell +0)
    std::int32_t edge_right{-1};  // horizontal edge vertex id (cell +4)
};
struct CoastMarchingResult {
    std::vector<CoastMarchingCell> cells;  // rows x columns
    std::uint32_t edge_vertex_count_24{0};
    std::uint32_t segment_count_28{0};
};
CoastMarchingResult coast_mesh_classify_cells_00bb11a0(const std::vector<float>& heights,
                                                       std::uint32_t rows_34,
                                                       std::uint32_t columns_38);
// LoadDistanceMap fragment: 'MitCoast' magic and grid match.
bool coast_mesh_distance_map_header_ok_00bb2df0(const std::uint8_t* magic8, std::uint32_t rows,
                                                std::uint32_t columns, std::uint32_t rows_34,
                                                std::uint32_t columns_38);
// Initialize fragment: packed vertex {s16 x, s16 z, s8 nx, s8 nz, u16 d} -> two 3-float points.
struct CoastPackedVertex {
    std::int16_t x{0};
    std::int16_t z{0};
    std::int8_t nx{0};
    std::int8_t nz{0};
    std::uint16_t distance{0};
};
std::array<float, 6> coast_mesh_decode_vertex_00bb3370(const CoastPackedVertex& v, float scale_x_28,
                                                       float scale_z_2c, float outer_58,
                                                       float inner_5c);

// ---------------------------------------------------------------------------------------------
// MSAR archive provider (vtable 00D643C4) and MPKG decode.
struct MsarHeader {
    bool magic_ok{false};          // 'MSAR' (00D643BC)
    std::uint32_t crc_04{0};       // stored CRC
    std::uint32_t toc_size_08{0};
    std::uint32_t data_size_0c{0};
    std::uint32_t header_crc_10{0};
};
MsarHeader msar_read_header_00bba090(const std::uint8_t* bytes16);
struct MsarEntry {  // 18h record
    std::string name;
    std::uint32_t size_08{0};
    std::uint32_t packed_size_0c{0};
    bool compressed_10{false};
    std::uint32_t offset_14{0};
};
std::vector<MsarEntry> msar_read_toc_00bbb890(const std::vector<std::uint8_t>& archive);
bool msar_verify_checksum_00bbbab0(const std::vector<std::uint8_t>& archive,
                                   std::uint64_t stream_position_after,
                                   std::uint64_t stream_length);
bool msar_validate_archive_00bbbb90(const std::vector<std::uint8_t>& archive);
bool msar_path_has_suffix_00bbbbf0(std::string_view path);
struct MsarOpenPlan {
    bool valid{false};
    bool inflate{false};
    std::uint32_t offset{0};
    std::uint32_t length{0};       // stored: copy length; compressed: packed input length
    std::uint32_t output_size{0};
    std::uint32_t inflate_input_buffer{0};
    std::uint32_t inflate_output_buffer{0};
};
MsarOpenPlan msar_open_entry_by_index_00bba500(const std::vector<MsarEntry>& entries,
                                               std::uint32_t index);
// Returns nullopt for write mode (mode bit 0) or when the name is not found.
std::optional<std::uint32_t> msar_open_entry_index_00bba7c0(const std::vector<MsarEntry>& entries,
                                                            std::string_view name,
                                                            std::uint8_t mode);
struct MsarNameFilterHost {
    virtual ~MsarNameFilterHost() = default;
    virtual bool matches_enumeration_filter(std::string_view directory,
                                            const MsarEntry& entry) = 0;
};
std::vector<std::string> msar_enumerate_names_00bbad60(MsarNameFilterHost& filter,
                                                       const std::vector<MsarEntry>& entries,
                                                       std::string_view directory,
                                                       std::string& normalized_directory);
std::vector<std::uint8_t> mpkg_decode_stream_to_memory_00bb9180(
    const std::vector<std::uint8_t>& encoded, const std::uint8_t* key_00e144f0, std::size_t key_size);

// Memory block (seg 93): begin, data end, capacity end, read cursor.
struct MemoryBlockState {
    std::vector<std::uint8_t> storage;
    std::size_t data_end_04{0};
    std::size_t capacity_end_08{0};
    std::size_t cursor_0c{0};
};
MemoryBlockState memory_block_allocate_00bbbcf0(std::size_t size);
std::size_t memory_block_read_00bbbed0(MemoryBlockState& block, std::uint8_t* out,
                                       std::size_t count);
struct InflateStreamPositions {
    std::uint64_t base_offset_10{0};
    std::uint32_t total_14{0};
    std::uint32_t total_18{0};
    std::uint32_t produced_24{0};
    std::uint32_t remaining_2c{0};
    std::uint32_t remaining_30{0};
};
// Returns the source seek target (+10h); resets +24h and +2Ch/+30h.
std::uint64_t inflate_stream_rewind_00bbbde0(InflateStreamPositions& s);
struct InflateStateConfig {
    std::size_t allocation_size{0};
    int window_bits{0};
    std::string_view version;
    int stream_size{0};
};
InflateStateConfig inflate_state_create_config_00bbbe60();

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part c: 00BBC000-00BC2FFF
// ================================================================================================
// cc13-water1 part c: 00BBC000-00BC2FFF (texture-source tails, cloud manager, atmosphere,
// cloud instance, sky box, application factories, lightning storm teardown, camera
// waterdrops and their fixed-block pool). Names are hypotheses, not recovered symbols.
// C++ interfaces over modelled inputs: not ABI-compatible, not wired into any host,
// not fixture- or game-validated.
namespace bsp::water_weather_cc13 {

// ---------------------------------------------------------------------------
// Native growable arrays of segment 93/94: {data +0, count +4, capacity +8}, int32 count
// and capacity. 00BBD0B0/00BBD110/00BBD230/00BBF390 (4-byte elements) and 00BC1860
// (34h elements) share one reserve policy, 00BBD170/00BBD1D0/00BBD640 one resize policy,
// 00BBD540/00BBD580 one push_back policy.
template <class T>
struct W1C_NativeArray {
    std::vector<T> storage;   // +00 data; storage.size() is the allocated element count
    std::int32_t count{};     // +04
    std::int32_t capacity{};  // +08
};

// reserve: n < 1 becomes 1; when capacity < n, allocate exactly n, copy count elements,
// free the old block, capacity = n. Never shrinks.
template <class T>
void w1c_array_reserve_c(W1C_NativeArray<T>& a, std::int32_t n) {
    if (n < 1) n = 1;
    if (a.capacity < n) {
        std::vector<T> next(static_cast<std::size_t>(n));
        for (std::int32_t i = 0; i < a.count; ++i) next[static_cast<std::size_t>(i)] = a.storage[static_cast<std::size_t>(i)];
        a.storage.swap(next);
        a.capacity = n;
    }
}

void cloud_class_vector_reserve_00bbd0b0(W1C_NativeArray<std::uint32_t>& a, std::int32_t n);
void cloud_instance_vector_reserve_00bbd110(W1C_NativeArray<std::uint32_t>& a, std::int32_t n);
void cloud_pointer_array_reserve_00bbd230(W1C_NativeArray<std::uint32_t>& a, std::int32_t n);
void sky_colour_strip_reserve_00bbf390(W1C_NativeArray<std::uint32_t>& a, std::int32_t n);

// resize: reserve(n) only when capacity < n (so n <= 0 never allocates), zero-fill
// [count, n), count = n (shrinking just drops the tail; no element destructor).
void cloud_class_vector_resize_00bbd170(W1C_NativeArray<std::uint32_t>& a, std::int32_t n);
void cloud_instance_vector_resize_00bbd1d0(W1C_NativeArray<std::uint32_t>& a, std::int32_t n);
void cloud_pointer_array_resize_00bbd640(W1C_NativeArray<std::uint32_t>& a, std::int32_t n);

// push_back: when count == capacity grow to capacity*2, or 1 when that is < 2.
void cloud_class_vector_push_back_00bbd540(W1C_NativeArray<std::uint32_t>& a, std::uint32_t v);
void cloud_instance_vector_push_back_00bbd580(W1C_NativeArray<std::uint32_t>& a, std::uint32_t v);

// 00BBD880 __thiscall(vec, const value*), RET 4 -> AL: first match is overwritten by
// the last element (unless it is the last), count-1; false when absent.
bool cloud_pointer_vector_remove_swap_last_00bbd880(W1C_NativeArray<std::uint32_t>& a, std::uint32_t value);

// ---------------------------------------------------------------------------
// Water texture sources (vtables 00D64478 caustics, 00D644B4 shore wave).

struct CausticsTextureSourceC {
    std::array<float, 4> colour_20{};  // +20..+2C
    float scale_30{};                  // +30
};

// 00BBCBA0 __thiscall(src, const float rgba[4]*), RET 4: dword copy of four floats.
void caustics_texture_source_set_colour_00bbcba0(CausticsTextureSourceC& s, const std::array<float, 4>& rgba) noexcept;
// 00BBCBC0 __thiscall(src, float), RET 4.
void caustics_texture_source_set_scale_00bbcbc0(CausticsTextureSourceC& s, float scale) noexcept;
// 00BBCBE0 / 00BBCBF0: shore-wave overrides of the same two slots are bare RET 4.
void shore_wave_texture_source_ignore_colour_00bbcbe0(const std::array<float, 4>& rgba) noexcept;
void shore_wave_texture_source_ignore_scale_00bbcbf0(float scale) noexcept;

// Runtime type tables: dword[0] own token, dword[1..3] base tokens, dword[4] name.
// 00BBC690 (table 0109E9D4..0109E9E3, loader zero-fill) and 00BBE2E0 (01090910..0109091F)
// scan the four tokens: __thiscall(ignored, token), RET 4 -> AL.
bool water_texture_source_accepts_type_token_00bbc690(const std::array<std::uint32_t, 4>& table_0109e9d4,
                                                     std::uint32_t token) noexcept;

// ---------------------------------------------------------------------------
// Cloud manager (A8h bytes, vtable 00D644F8, [atmosphere+3Ch]).

struct CloudManagerC {
    std::uint32_t refcount_04{1};
    std::uint32_t owner_08{};                       // the atmosphere
    W1C_NativeArray<std::uint32_t> classes_0c;      // cloud classes (refcounted)
    W1C_NativeArray<std::uint32_t> instances_18;    // cloud instances
    W1C_NativeArray<std::uint32_t> array_24;        // third pointer array
    std::uint32_t group_node_30{};                  // "cloud group" node
    float field_34{2000.0f};                        // 00CFFD60
    std::array<float, 3> field_38{-99999.0f, -99999.0f, -99999.0f};  // 00D1A648
    std::array<float, 3> vector_44{1.0f, 0.0f, 0.0f};                // 00D7A24C
    std::array<float, 3> vector_50{0.0f, 1.0f, 0.0f};
    std::array<float, 4> colour_60{};  // bottom light, ctor (7,42,98)/255, alpha 0
    std::array<float, 4> colour_70{};  // ambient light, ctor 0.2 grey, alpha 0
    std::array<float, 4> colour_80{};  // sun light, ctor 0.8 grey, alpha 0
    std::uint32_t light_94{};          // object whose +1E0h is the light-0 direction
    std::uint8_t enabled_98{1};
    std::uint8_t wind_enabled_99{1};
    // +9C data / +A0 count / +A4 capacity: registered nodes (growth capacity*2+2).
    std::vector<std::uint32_t> nodes_9c_storage;
    std::uint32_t nodes_count_a0{};
    std::uint32_t nodes_capacity_a4{};
};

class CloudManagerHostC {
public:
    virtual ~CloudManagerHostC() = default;
    virtual void set_visibility_factor_00b6da70(std::uint32_t node, float value, std::uint8_t recurse) = 0;
    virtual void ensure_lighting_00bbe640(std::uint32_t instance) = 0;
    // operator_new(90h) then 00BC4A60(source, manager, mesh, texture, multiplier); 0 if
    // the allocation failed (the native pushes the 0 too).
    virtual std::uint32_t construct_cloud_class_00bc4a60(std::uint32_t source, std::uint32_t mesh,
                                                         std::uint32_t texture, float multiplier) = 0;
    // [010909B0] vtable+04(group node, arg).
    virtual std::uint32_t factory_create_instance_010909b0(std::uint32_t group_node, std::uint32_t arg) = 0;
    virtual void unlink_and_release_00b6dfa0(std::uint32_t node) = 0;
    // inline InterlockedDecrement(obj+4), vtable slot 0 when it reaches zero.
    virtual void release_ref_interlocked_c(std::uint32_t object) = 0;
};

// 00BBDA50 __thiscall(mem, owner), RET 4: field defaults. The "cloud group" node
// (BSP_GroupPool_AllocateStaticRawSlot + BSP_Group_Construct, then
// BSP_Node_PropagateRootRegistration with the atmosphere source) is passed in.
CloudManagerC cloud_manager_construct_00bbda50(std::uint32_t owner, std::uint32_t group_node);

// Accessors (__thiscall). Vector getters copy with FLD/FSTP (a signalling NaN would be
// quieted); colour getters/setters are dword copies; setters RET 10h take four floats.
std::array<float, 3> cloud_manager_get_vector50_00bbccb0(const CloudManagerC& m) noexcept;
std::array<float, 3> cloud_manager_get_vector44_00bbccd0(const CloudManagerC& m) noexcept;
std::array<float, 4> cloud_manager_get_colour60_00bbcd30(const CloudManagerC& m) noexcept;
void cloud_manager_set_colour60_00bbcd50(CloudManagerC& m, float r, float g, float b, float a) noexcept;
std::array<float, 4> cloud_manager_get_colour70_00bbcd70(const CloudManagerC& m) noexcept;
void cloud_manager_set_colour70_00bbcd90(CloudManagerC& m, float r, float g, float b, float a) noexcept;
std::array<float, 4> cloud_manager_get_colour80_00bbcdb0(const CloudManagerC& m) noexcept;
void cloud_manager_set_colour80_00bbcde0(CloudManagerC& m, float r, float g, float b, float a) noexcept;
void cloud_manager_set_light94_00bbce30(CloudManagerC& m, std::uint32_t light) noexcept;
void cloud_manager_set_wind_enabled_00bbce60(CloudManagerC& m, std::uint8_t enabled) noexcept;

// 00BBCFE0 __thiscall(byte), RET 4.
void cloud_manager_set_enabled_00bbcfe0(CloudManagerC& m, CloudManagerHostC& host, std::uint8_t enabled);
// 00BBD470 __thiscall(node), RET 4.
void cloud_manager_register_node_00bbd470(CloudManagerC& m, std::uint32_t node);
// 00BBD510 __fastcall(manager).
void cloud_manager_ensure_instance_lighting_00bbd510(const CloudManagerC& m, CloudManagerHostC& host);
// 00BBD6C0 __thiscall(mesh, texture, float multiplier), RET 0Ch -> EAX class.
std::uint32_t cloud_manager_create_cloud_class_00bbd6c0(CloudManagerC& m, CloudManagerHostC& host,
                                                        std::uint32_t atmosphere_source_0c, std::uint32_t mesh,
                                                        std::uint32_t texture, float multiplier);
// 00BBD770 __thiscall(arg), RET 4 -> EAX instance or 0.
std::uint32_t cloud_manager_create_instance_00bbd770(CloudManagerC& m, CloudManagerHostC& host, std::uint32_t arg);
// 00BBD910 __thiscall(instance), RET 4.
void cloud_manager_release_instance_00bbd910(CloudManagerC& m, CloudManagerHostC& host, std::uint32_t instance);
// 00BBD950 __fastcall(manager): release order of the destructor body.
void cloud_manager_destruct_body_00bbd950(CloudManagerC& m, CloudManagerHostC& host);

// ---------------------------------------------------------------------------
// Atmosphere (40h bytes, vtable 00D64518, game+19E8h).

struct AtmosphereC {
    std::uint32_t source_0c{};
    std::uint32_t fog_receiver_10{};
    std::uint32_t field_14{};
    std::uint32_t sky_box_18{};
    float field_1c{40000.0f};         // 00D64520
    float time_of_day_20{0.5f};       // 00CE3800
    float field_24{1.0f};             // 00D7A24C
    float blend_28{0.1f};             // 00D7A2F0
    std::array<float, 3> wind_direction_2c{};
    float wind_speed_38{0.6f};        // 00CE3D30
    std::uint32_t cloud_manager_3c{};
};

class AtmosphereHostC {
public:
    virtual ~AtmosphereHostC() = default;
    virtual void add_ref_interlocked_c(std::uint32_t object) = 0;      // InterlockedIncrement(+4)
    virtual void release_ref_interlocked_c(std::uint32_t object) = 0;  // decrement, vslot 0 at zero
    // operator_new(130h) then 00BBF8A0(source, atmosphere, arg); 0 when allocation failed.
    virtual std::uint32_t construct_sky_box_00bbf8a0(std::uint32_t source, std::uint32_t arg) = 0;
    // operator_new(A8h) then 00BBDA50(atmosphere); 0 when allocation failed.
    virtual std::uint32_t construct_cloud_manager_00bbda50() = 0;
    virtual void update_frame_and_instances_00bbd310(std::uint32_t manager, std::uint32_t arg, float dt) = 0;
    virtual void weak_owner_destroy_c() = 0;  // BSP_WeakOwner_Destroy tail
};

// 00419440 order: float(x*x), float(y*y), their sum kept in 53-bit, z*z rounded to float,
// sum rounded to float, LIBCRT_sqrt (00BF7030) rounded to float.
float atmosphere_vector3_length_c(float x, float y, float z) noexcept;

// 00BBDFF0 __thiscall(mem, source*, sky arg), RET 8.
AtmosphereC atmosphere_construct_00bbdff0(AtmosphereHostC& host, std::uint32_t source, std::uint32_t sky_arg);
// 00BBDDD0 __thiscall(arg, float dt), RET 8.
void atmosphere_update_clouds_00bbddd0(const AtmosphereC& a, AtmosphereHostC& host, std::uint32_t arg, float dt);
// 00BBDF60 __thiscall(sky arg), RET 4.
void atmosphere_replace_sky_box_00bbdf60(AtmosphereC& a, AtmosphereHostC& host, std::uint32_t sky_arg);
// 00BBE1A0 __fastcall(atmosphere).
void atmosphere_destruct_body_00bbe1a0(AtmosphereC& a, AtmosphereHostC& host);

// ---------------------------------------------------------------------------
// Cloud instance (GeneratedModel "CloudInstance", vtable 00D64528).

struct CloudInstanceC {
    std::uint32_t cloud_class_184{};
    std::uint32_t vertex_buffer_1a8{};
    std::uint32_t lighting_out_1ac{};   // written by 00BC3930 for light 0 (address passed)
    std::uint32_t lighting_out_1b0{};   // for light 1
    std::uint8_t lit_1b4{};
    std::array<float, 3> scroll_1b8{};  // ctor 0
    float field_1c4{1.0f};              // ctor 00D7A24C
    float scale_1c8{};                  // not written by the constructor
    float alpha_1d8{};                  // not written by the constructor
    std::uint8_t affected_by_wind_1e8{1};
};

// Class-wide statics written by the scene-property setters.
struct CloudInstanceStaticsC {
    float blur_radius_01090904{};
    float density_01090908{};
    std::array<float, 4> ambient_01090970{};
    std::array<float, 4> diffuse_01090980{};
    std::array<float, 4> specular_01090990{};
    std::array<float, 4> inside_010909a0{};
};

struct CloudInstanceTypeInfoC {      // 01090910
    std::uint32_t token_00{};
    std::array<std::uint32_t, 3> base_04{};
    const char* name_10{};
};

class CloudInstanceHostC {
public:
    virtual ~CloudInstanceHostC() = default;
    virtual void set_visibility_factor_00b6da70(std::uint32_t node, float value, std::uint8_t recurse) = 0;
    virtual std::uint32_t class_manager_00bc3030(std::uint32_t cloud_class) = 0;
    virtual std::array<float, 3> light_direction_1e0(std::uint32_t light) = 0;  // &light+1E0h
    // 00419510 normalize, then 0042D0D0 by the 00B6E0D0(1) inverse world matrix.
    virtual std::array<float, 3> to_local_direction_0042d0d0(std::array<float, 3> world) = 0;
    virtual void compute_lighting_00bc3930(std::uint32_t cloud_class, std::int32_t light,
                                           const std::array<float, 3>& local_dir, float density,
                                           float blur_radius, std::uint32_t* out) = 0;
    virtual std::uint32_t lock_vertices_vslot10(std::uint32_t vb) = 0;  // vtable+10(0,0,0)
    virtual void upload_puffs_00bc33b0(std::uint32_t cloud_class, std::uint32_t mapped) = 0;
    virtual void unlock_vertices_vslot14(std::uint32_t vb) = 0;         // vtable+14
    virtual void initialize_3d_object_type_info_c() = 0;                // BSP_TypeInfo_Initialize3dObject
};

// 00BBE7D0 __fastcall(mem): only the instance's own fields (GeneratedModel base excluded).
void cloud_instance_construct_00bbe7d0(CloudInstanceC& c) noexcept;
std::uint32_t cloud_instance_type_token_00bbe2b0(const CloudInstanceTypeInfoC& t) noexcept;
const char* cloud_instance_type_name_00bbe2c0(const CloudInstanceTypeInfoC& t) noexcept;
bool cloud_instance_accepts_type_token_00bbe2e0(const CloudInstanceTypeInfoC& t, std::uint32_t token) noexcept;
// 00BBE770 __fastcall(&type info): one-shot (flag 0109090C).
void cloud_instance_init_type_info_00bbe770(CloudInstanceTypeInfoC& t, bool& initialized_0109090c,
                                            const std::array<std::uint32_t, 3>& base_01090034,
                                            std::uint32_t& type_counter, CloudInstanceHostC& host);
// 00BBE310 __thiscall(ignored), RET 4 -> AL 0.
bool cloud_instance_show_return_false_00bbe310(std::uint32_t self_node, CloudInstanceHostC& host);
std::uint32_t cloud_instance_get_class_00bbe330(const CloudInstanceC& c) noexcept;
// 00BBE340 / 00BBE360 __thiscall(out*), RET 4: the manager's +50h / +44h vector.
std::array<float, 3> cloud_instance_get_manager_vector50_00bbe340(const CloudManagerC& manager_via_class) noexcept;
std::array<float, 3> cloud_instance_get_manager_vector44_00bbe360(const CloudManagerC& manager_via_class) noexcept;
void cloud_instance_upload_puffs_00bbe380(const CloudInstanceC& c, CloudInstanceHostC& host);
void cloud_instance_set_scale_00bbe470(CloudInstanceC& c, float v) noexcept;
void cloud_instance_set_alpha_00bbe490(CloudInstanceC& c, float v) noexcept;
void cloud_instance_set_ambient_colour_00bbe4b0(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept;
void cloud_instance_set_diffuse_colour_00bbe4e0(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept;
void cloud_instance_set_specular_colour_00bbe510(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept;
void cloud_instance_set_inside_colour_00bbe540(CloudInstanceStaticsC& s, float r, float g, float b, float a) noexcept;
void cloud_instance_set_density_00bbe570(CloudInstanceStaticsC& s, float v) noexcept;
void cloud_instance_set_blur_radius_00bbe590(CloudInstanceStaticsC& s, float v) noexcept;
void cloud_instance_set_affected_by_wind_00bbe5c0(CloudInstanceC& c, std::uint8_t v) noexcept;
// 00BBE640 __fastcall(instance): lazy two-light pass, then puff upload, lit = 1.
void cloud_instance_ensure_lighting_00bbe640(CloudInstanceC& c, const CloudInstanceStaticsC& s,
                                             std::uint32_t manager_light_94, CloudInstanceHostC& host);
// 00BBE760 __thiscall(ignored), RET 4: lit = 0 then 00BBE640.
void cloud_instance_invalidate_lighting_00bbe760(CloudInstanceC& c, const CloudInstanceStaticsC& s,
                                                 std::uint32_t manager_light_94, CloudInstanceHostC& host);

// ---------------------------------------------------------------------------
// Sky box (130h, built by 00BBF8A0).

struct SkyBoxC {
    float luminance_30{};
};

struct SkyBoxConstructHostC {
    virtual ~SkyBoxConstructHostC() = default;
    virtual void* call_004c1400() = 0;
    virtual void* call_00b80d70(void* manager, std::string_view name) = 0;
    virtual void* resource_vslot_08(void* resource, std::uint32_t kind, float scale) = 0;
    virtual void refcounted_vslot_00(void* object) = 0;
    virtual void call_00b6d890(void* node, void* source) = 0;
    virtual void* global_pointer_00f8d394() = 0;
    virtual void* texture_manager_vslot_64(void* manager, std::string_view name, std::uint32_t flags) = 0;
    virtual std::uint32_t texture_vslot_3c(void* texture) = 0;
    virtual std::uint32_t texture_vslot_40(void* texture) = 0;
    virtual void* call_00b74640(void* model, std::uint32_t index) = 0;
    virtual std::uint32_t call_00b74650(void* model) = 0;
    virtual void call_00b72b50(void* geometry) = 0;
    virtual void call_00b72650(void* geometry, const std::array<float, 4>& bounds) = 0;
    virtual std::uint32_t call_00b72b40(void* geometry) = 0;
    virtual void* call_00b732c0(void* geometry, std::uint32_t index) = 0;
    virtual const void* call_00b33e40(void* texture) = 0;
    virtual void call_00b18b20(void* material, std::string_view name, float* scalar) = 0;
    virtual void call_00b18ac0(void* material, std::string_view name, float* vector, std::uint32_t count) = 0;
    virtual void call_00b189f0(void* material, std::uint32_t slot, void* texture) = 0;
};
// 00BBF8A0: native ECX destination130h, stack source, atmosphere, NativeString*
// name, RET0Ch/EAX destination. Complete normal raw-storage/host-call body.
// Native string/pool/FH3 mechanics and original binary ABI are not reproduced.
// Requires valid resource/node/mesh/material identities and texture slot0 as
// native does. The four Vec4 no-op constructors retain unwritten bytes, including
// cTextureSize.z/w. x87 texel math retains DC C9's ST1 destination and float spills.
void* sky_box_construct_00bbf8a0(void* destination, void* source, void* atmosphere,
    std::string_view name, SkyBoxConstructHostC&);

class SkyBoxHostC {
public:
    virtual ~SkyBoxHostC() = default;
    // 00B646E0 rotation about Y, 004134F0 copy, [[sky+10h]+0Ch] vtable+34h(matrix).
    virtual void set_node_rotation_y_00b646e0(float radians) = 0;
};

// 00BBF1A0 radians = float(deg * 3.141592025756836 / 180.0) in 53-bit precision.
float sky_box_rotation_radians_00bbf1a0(float degrees) noexcept;
void sky_box_set_rotation_degrees_00bbf1a0(SkyBoxHostC& host, float degrees);
void sky_box_set_luminance_00bbf200(SkyBoxC& s, float v) noexcept;

// ---------------------------------------------------------------------------
// Lightning storm teardown (00BC04C0).

struct LightningStormHandlesC {
    std::array<std::uint32_t, 7> handles_08{};  // refcounted
    std::array<std::uint32_t, 7> slots_24{};    // cleared alongside
};

class LightningStormHostC {
public:
    virtual ~LightningStormHostC() = default;
    virtual void release_ref_interlocked_c(std::uint32_t object) = 0;
};

void lightning_storm_destroy_00bc04c0(LightningStormHandlesC& s, std::uint32_t& active_storm_010909b4,
                                      LightningStormHostC& host);

// ---------------------------------------------------------------------------
// Camera waterdrops (GeneratedModel "waterdrops", vtable 00D64760; pooled vtable 00D647D0).

struct WaterdropsDropC {             // 34h bytes
    float x_00{};
    float y_04{};
    float size_08{};
    float height_0c{};
    float slide_10{};
    float dir_x_14{};
    float dir_y_18{};
    float life_total_1c{};
    float life_left_20{};
    float orient_x_24{};
    float orient_y_28{};
    float stick_2c{};
    std::int32_t variant_30{};       // FILD dword: signed
};

struct WaterdropsC {
    W1C_NativeArray<WaterdropsDropC> drops_184;
    std::int32_t requested_190{};    // not written by the constructor
    float min_size_194{};
    float max_size_198{};
    float min_life_19c{};
    float max_life_1a0{};
    float sticky_percent_1a4{};      // not written by the constructor
    std::int32_t live_1a8{};         // not written by the constructor
    float gravity_1c4{};
    std::uint32_t field_1c8{};       // texture slot read by 00BC2D50
    float variant_step_1d0{};
    std::uint8_t apply_at_bottom_only_1d4{};
    std::uint8_t start_top_off_screen_1d5{};
    std::uint8_t apply_at_top_only_1d6{};
    std::uint8_t no_camera_move_1d7{};
};

struct WaterdropsVertexC {           // 18h bytes
    float x{};
    float y{};
    float z{};
    std::uint32_t colour{};
    float u{};
    float v{};
};

class WaterdropsHostC {
public:
    virtual ~WaterdropsHostC() = default;
    virtual float reciprocal_length_00419260(float x, float y) = 0;
    virtual void render_vslot60() = 0;  // model vtable+60h
};

// The field stores of 00BC2030 on the semantic projection (no GeneratedModel base, no vtable);
// the 00BC2D00 model below uses it. The native routine is waterdrops_construct_00bc2030 further down.
void waterdrops_construct_fields(WaterdropsC& w) noexcept;
bool waterdrops_is_finished_00bc1150(const WaterdropsC& w) noexcept;
void waterdrops_set_size_range_00bc1160(WaterdropsC& w, float lo, float hi) noexcept;
void waterdrops_set_life_range_00bc1180(WaterdropsC& w, float lo, float hi) noexcept;
void waterdrops_set_count_00bc11a0(WaterdropsC& w, std::int32_t n) noexcept;
void waterdrops_set_gravity_00bc11b0(WaterdropsC& w, float v) noexcept;
void waterdrops_set_sticky_percent_00bc11d0(WaterdropsC& w, float v) noexcept;
void waterdrops_set_apply_at_bottom_only_00bc11f0(WaterdropsC& w, std::uint8_t v) noexcept;
void waterdrops_set_start_top_off_screen_00bc1200(WaterdropsC& w, std::uint8_t v) noexcept;
void waterdrops_set_apply_at_top_only_00bc1210(WaterdropsC& w, std::uint8_t v) noexcept;
void waterdrops_set_no_camera_move_00bc1220(WaterdropsC& w, std::uint8_t v) noexcept;
// 00BC1230 __thiscall(a, b), RET 8 -> AL.
bool waterdrops_accept_render_00bc1230(const WaterdropsC& w, WaterdropsHostC& host);
void waterdrop_vector_reserve_00bc1860(W1C_NativeArray<WaterdropsDropC>& a, std::int32_t n);

struct WaterdropsFillCountsC {
    std::int32_t locked_vertices{};   // vtable+10h(live*4, 0, 0)
    std::int32_t element_10{};        // live*4
    std::int32_t element_18{};        // live*2
};

// 00BC2340 __fastcall(waterdrops): appends four vertices per drop whose life_left is not
// exactly 0.0 (a NaN life_left is drawn).
WaterdropsFillCountsC waterdrops_fill_vertices_00bc2340(const WaterdropsC& w, WaterdropsHostC& host,
                                                        std::vector<WaterdropsVertexC>& out);

// Fixed-block pool at 010909BC: 32 objects of 1DCh per 3BC4h block.
struct WaterdropsPoolBlockC {
    std::array<std::uint32_t, 32> owner_index_1d8{};  // slot trailer at slot*1DCh + 1D8h
    std::array<std::uint16_t, 32> free_stack_3b80{};
    std::uint16_t free_count_3bc0{};
};

struct WaterdropsPoolC {
    WaterdropsPoolC* list_prev_04{};
    WaterdropsPoolC* list_next_08{};
    std::int32_t lock_depth_24{};
    std::vector<WaterdropsPoolBlockC> blocks_28;  // size() == count +2C (native: block pointers)
    std::uint32_t block_capacity_30{};
    std::uint32_t current_34{0xFFFFFFFFu};
};

struct WaterdropsPoolSlotC {
    std::uint32_t block{};
    std::uint32_t slot{};
};

void waterdrops_pool_init_block_00bc2190(WaterdropsPoolBlockC& b, std::uint32_t block_index) noexcept;
// 00BC2A30: links at the head of the allocator list 00E188B4, capacity 32.
void waterdrops_pool_construct_00bc2a30(WaterdropsPoolC& p, WaterdropsPoolC*& list_head_00e188b4);
WaterdropsPoolSlotC waterdrops_pool_allocate_00bc2bb0(WaterdropsPoolC& p);
// s.block is the slot trailer (+1D8h), s.slot = (object - block) / 1DCh (signed IDIV).
void waterdrops_pool_free_00bc2880(WaterdropsPoolC& p, WaterdropsPoolSlotC s);
void waterdrops_pool_free_global_00bc29f0(WaterdropsPoolC& pool_010909bc, WaterdropsPoolSlotC s);
void waterdrops_pool_trim_free_blocks_00bc2b10(WaterdropsPoolC& p);
// 00BC27C0: returns how many LeaveCriticalSection calls the unbalanced-depth loop made.
std::int32_t waterdrops_pool_destroy_00bc27c0(WaterdropsPoolC& p, WaterdropsPoolC*& list_head_00e188b4);
// 00BC2D00: allocate from 010909BC, construct, vtable 00D647D0.
WaterdropsPoolSlotC waterdrops_create_00bc2d00(WaterdropsPoolC& pool_010909bc, WaterdropsC& object);

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part d: 00BC3000-00BD2E60
// ================================================================================================
// cc13-water1 helper part "d": 00BC3000-00BD2E60. Semantic C++ over modelled inputs; every name is
// a hypothesis, not a recovered symbol. Not ABI-compatible, not wired into any host.
namespace bsp::water_weather_cc13 {

// ---------------------------------------------------------------------------------------------
// Coverage: 00BC3000-00BD2E60 (particle cloud class helpers, cloud puff, cloud render
// model update, the three 16/4/8-slot model pools, lightning strand model: segments, strands,
// bursts, thunder sprites). Hypotheses, not recovered symbols. Every model below is a C++
// interface over explicit inputs; nothing reads a native global, nothing draws a random number
// itself (draws come back through a host callback), nothing is wired into a host or validated
// against the game. x87 model: 53-bit precision, intermediates as double, every FSTP to a float
// slot is a float cast. Evidence per function in local\w1_d_records.jsonl.
// ---------------------------------------------------------------------------------------------

struct W1D_Vec3 {
    float x{};
    float y{};
    float z{};
};

// Row-major 4x4, element [r][c] at index r * 4 + c (the native matrix builders 00B64640/00B646E0/
// 00B64780 and multiply 00413920 use this layout; translation in row 3).
struct W1D_Mat4 {
    std::array<float, 16> m{};
};

// ---- particle cloud puff (vtable 00D64940; constructed by 00BC6430 from factory 00BC03C0) ----
// +0 vtable, +4 reference count (00BC7B60 base); the fields below keep their native offsets.
struct CloudPuffState {
    std::uint32_t slot_08{};          // index of the puff's 4-vertex block in the vertex buffer
    W1D_Vec3 position_0c{};
    float angle_18{};                 // roll of the quad in its plane, radians
    float size_1c{};                  // half extent of the quad
    std::array<float, 2> light_20{};  // per light, written by 00BC7AF0
    std::array<float, 2> shade_28{};  // per light, written by 00BC7AD0; packed into R and G
    std::uint8_t alpha_30{};          // written by 00BC7B10
    float sort_key_34{};              // written by 00BC7A90
};

// 00BC6430 __thiscall(ECX = puff, slot, const Vec3* position, float angle, float size), RET 10h.
// Writes +8..+1Ch only. The native leaves light/shade/alpha/sort key as allocator contents; the
// model value-initialises them.
CloudPuffState cloud_puff_construct_00bc6430(std::uint32_t slot, const W1D_Vec3& position, float angle,
    float size) noexcept;

// 00BC7A70 __thiscall(ECX = puff, Vec3* out), RET 4.
W1D_Vec3 cloud_puff_get_position_00bc7a70(const CloudPuffState& puff) noexcept;
// 00BC7A90 __thiscall(ECX = puff, float key), RET 4.
void cloud_puff_set_sort_key_00bc7a90(CloudPuffState& puff, float key) noexcept;
// 00BC7AD0 __thiscall(ECX = puff, int light, float value), RET 8. No bounds check natively.
void cloud_puff_set_shade_00bc7ad0(CloudPuffState& puff, int light, float value) noexcept;
// 00BC7AF0 __thiscall(ECX = puff, int light, float value), RET 8.
void cloud_puff_set_light_00bc7af0(CloudPuffState& puff, int light, float value) noexcept;
// 00BC7B10 __thiscall(ECX = puff, float alpha), RET 4: byte = low byte of FISTP dword of
// alpha * 255.0 (double 00CE4B48) under a chop control word.
void cloud_puff_set_alpha_00bc7b10(CloudPuffState& puff, float alpha) noexcept;
// 00BC7B40 / 00BC7B50 __thiscall(ECX = puff, int light), RET 4 -> ST0.
float cloud_puff_get_light_00bc7b40(const CloudPuffState& puff, int light) noexcept;
float cloud_puff_get_shade_00bc7b50(const CloudPuffState& puff, int light) noexcept;

// One vertex of the puff quad, 20h bytes: position, ARGB colour, rotated corner offset, uv.
struct CloudPuffVertex {
    W1D_Vec3 position_00{};
    std::uint32_t colour_0c{};
    float offset_x_10{};
    float offset_y_14{};
    float u_18{};
    float v_1c{};
};

// 00BC64C0 (puff vtable 00D64940 slot +8) __thiscall(ECX = puff, void* out, a, b), RET 0Ch; a and
// b are not read. Corners (-1,1),(1,1),(1,-1),(-1,-1) (floats 00D7A260 = -1, 00D7A24C = 1) scaled
// by size and rotated by angle; uv (0,0),(1,0),(1,1),(0,1); colour A = alpha, R = shade[0]*255,
// G = shade[1]*255, B = 0 with the native's chop-truncated subtract/shift packing.
void cloud_puff_write_quad_vertices_00bc64c0(const CloudPuffState& puff,
    std::array<CloudPuffVertex, 4>& out) noexcept;

// 00BC33B0 (ledger name BSP_WeatherLayerSet_ForEachLayerSlot8 - the receiver is the particle cloud
// class: puff pointer array +1Ch, count +20h) __thiscall(ECX = class, void* vertices), RET 4:
// for every puff, virtual slot +8 (00BC64C0) writes 4 vertices at vertices + puff.slot * 80h.
// The class fields +6Ch / +84h are passed as the two unread arguments. `vertices` must hold
// (max slot + 1) * 4 entries; the model returns false and writes nothing when it does not.
bool cloud_class_write_puff_quads_00bc33b0(const std::vector<CloudPuffState>& puffs,
    std::vector<CloudPuffVertex>& vertices) noexcept;

// 00BC3410 __thiscall(ECX = class, unsigned index), RET 4: puffs[index]->slot (00BC7A60 reads +8).
std::uint32_t cloud_class_puff_slot_00bc3410(const std::vector<CloudPuffState>& puffs,
    std::uint32_t index) noexcept;

// One CloudSystem sphere of the cloud class (vector +30h, count +34h, 10h records).
struct CloudEdgeSphere {
    W1D_Vec3 center{};
    float radius{};
};

// 00BC3580 __thiscall(ECX = class, const Vec3* point), RET 4 -> ST0. No reference to it exists in
// the image (dead code). For every sphere with |center - point| <= 1.2 * radius it clamps
// f = 1 - (1.2 r - d) / (0.2 r) to [0, 1] and keeps the minimum; a second containing sphere
// returns 0 at once. The running minimum starts at 0.0, so the result is 0 for every ordered
// input; only a NaN f (r = 0 with d = 0, or NaN data) survives as NaN.
float cloud_class_puff_edge_factor_00bc3580(const std::vector<CloudEdgeSphere>& spheres,
    const W1D_Vec3& point) noexcept;

// ---- cloud resource readers (structured text nodes) ----
struct CloudResourceReaderHost {
    virtual ~CloudResourceReaderHost() = default;
    // 00BE9A40 on the file root before its children are walked (00BC4480 only).
    virtual void read_control_word_00be9a40() = 0;
    // 00715BF0: the current parent has another child.
    virtual bool has_remaining_00715bf0() = 0;
    // 00BEA680: opens the next child; returns its name (child +14h), or nullptr.
    virtual const char* open_child_00bea680() = 0;
    // 00BE9C40: skip and detach the open child.
    virtual void skip_child_00be9c40() = 0;
    // 00BE9ED0: release the open child handle.
    virtual void release_child_00be9ed0() = 0;
    // 00BC42E0 (src/weather_render.cpp cloud_read_spheres_00bc42e0) on a "CloudSystem" child.
    virtual void read_cloud_system_00bc42e0() = 0;
    // 00BC43D0 on a "Resource" child; the model routes this to cloud_class_read_resource_00bc43d0.
    virtual void enter_resource_child() = 0;
};

// 00BC43D0 __thiscall(ECX = class, node* parent), RET 4: children named "CloudSystem" (00D64854,
// _stricmp) are read as sphere lists, every other child is skipped; each child is released.
void cloud_class_read_resource_00bc43d0(CloudResourceReaderHost& host);
// 00BC4480 __thiscall(ECX = class, node* root), RET 4: control word, then "Resource" children are
// read by 00BC43D0 and every other child is skipped.
void cloud_class_read_resource_file_00bc4480(CloudResourceReaderHost& host);

struct CloudResourceLoadHost {
    virtual ~CloudResourceLoadHost() = default;
    // File system singleton DAT_0109CEEC vtable +4 (path, mode 2): returns a stream handle with
    // one reference. The native does not test the result.
    virtual void* open_stream_vfs_vt4(const std::string& path, int mode) = 0;
    // 00BEA210 reader construct + 00BEA4B0 assign stream; the reader takes its own reference.
    virtual void attach_reader_00bea4b0(void* stream) = 0;
    // InterlockedDecrement of stream +4 and the deleting destructor on zero.
    virtual void release_stream(void* stream) = 0;
    // 00BEA5B0 root, 00BC4480 on it, 00BE9ED0 release, 00BEA330 reader destroy.
    virtual void read_root_with_00bc4480() = 0;
};

// 00BC4530 __stdcall(const NativeString* path)-shaped body (EH frame 00CC51A0): opens the cloud
// resource with mode 2, attaches a structured reader, drops the local stream reference and walks
// the root with 00BC4480.
void cloud_class_load_resource_00bc4530(CloudResourceLoadHost& host, const std::string& path);

// ---- cloud render model (vtable 00D64878 +5Ch) ----
struct CloudRenderModelState {
    float alpha_0ac{};              // model alpha; 0 skips the update
    W1D_Vec3 world_position_120{};  // node world position (refreshed by 00B6DB70 when stale)
    W1D_Vec3 last_eye_18c{};        // camera position at the last resort
    W1D_Vec3 last_view_dir_198{};   // camera direction in model space at the last resort
};

struct CloudRenderModelUpdateHost {
    virtual ~CloudRenderModelUpdateHost() = default;
    // 00419440 BSP_Vector3f_Length (x87 spill pattern, result left in ST0).
    virtual double vector_length_00419440(const W1D_Vec3& v) = 0;
    // 0042D0D0 with the inverse world matrix from 00B6E0D0 and normalise flag 0: camera direction
    // (camera +110h) into model space.
    virtual W1D_Vec3 camera_dir_to_model_0042d0d0() = 0;
    // 004142E0 with the inverse world matrix: camera position into model space.
    virtual W1D_Vec3 camera_pos_to_model_004142e0(const W1D_Vec3& camera_world) = 0;
    // 00B71530 camera sphere test against vtable +48h bounds; 2 rejects.
    virtual int classify_bounds_00b71530() = 0;
    // 00BC3430 on the cloud class (+184h): re-sort the puff pointer array along the local eye.
    virtual void sort_puffs_00bc3430(const W1D_Vec3& local_eye) = 0;
    // 00BC3100 puff count and 00BC3410 slot of the i-th sorted puff.
    virtual std::uint32_t puff_count_00bc3100() = 0;
    virtual std::uint32_t sorted_puff_slot_00bc3410(std::uint32_t index) = 0;
};

// 00BC4D20 __thiscall(ECX = model, camera*), RET 4. Skips when alpha == 0.0 (00D7A218; NaN goes
// on), when |camera - model| < 100.0 (double 00D7A220; NaN goes on), when
// dot(last_dir, dir) > 0.9f (double 00D7A390 = (double)0.9f; NaN goes on) or when the bounds
// test returns 2. Otherwise stores dir and eye, re-sorts and rewrites 6 indices per puff
// (s*4, s*4+1, s*4+2, s*4+2, s*4+3, s*4) into the locked index buffer; returns true then.
bool cloud_render_model_update_00bc4d20(CloudRenderModelState& model, const W1D_Vec3& camera_world,
    CloudRenderModelUpdateHost& host, std::vector<std::uint16_t>& indices);

// ---- fixed-block model pools: CloudRenderModel (pool 010909F4), CloudPlaneModel (01090A2C),
// LightningModel (01090A68). Same template as the 00BC2BB0 waterdrop pool and the SlotPool of
// include/bsp/band_0067_cc13.hpp; this models the block bookkeeping including the trim policy and
// the 2n+2 block-array capacity. Critical section +0Ch and lock depth +24h stay with the host.
struct CloudPoolGeometry {
    std::uint32_t slot_bytes{};
    std::uint16_t slots_per_block{};
    std::uint32_t block_bytes{};
    std::uint32_t page_index_offset{};   // slot field holding the block index
    std::uint32_t free_stack_offset{};   // uint16 stack of free slot indices
    std::uint32_t free_count_offset{};   // uint16 free count
};
// 1F0h slots, 16 per 1F24h block, page +1ECh, stack +1F00h, count +1F20h.
CloudPoolGeometry cloud_render_model_pool_geometry_00bc53a0() noexcept;
// 190h slots, 4 per 64Ch block, page +18Ch, stack +640h, count +648h.
CloudPoolGeometry cloud_plane_model_pool_geometry_00bc5f10() noexcept;
// 1C0h slots, 8 per 0E14h block, page +1BCh, stack +0E00h, count +0E10h.
CloudPoolGeometry lightning_model_pool_geometry_00bc7310() noexcept;

class CloudPagedPool {
public:
    explicit CloudPagedPool(std::uint16_t slots_per_block) noexcept;

    struct Slot {
        std::uint32_t block{};
        std::uint16_t index{};
    };

    // 00BC53A0 / 00BC5F10 / 00BC7310 __fastcall(ECX = pool) -> slot pointer.
    Slot allocate();
    // 00BC5070 / 00BC5BE0 / 00BC6DF0 __thiscall(ECX = pool, slot*), RET 4. `slot.block` is the
    // page index the slot carries (+1ECh/+18Ch/+1BCh).
    void free(const Slot& slot) noexcept;
    // 00BC5300 / 00BC5E70 / 00BC7270 __fastcall(ECX = pool).
    void trim_free_blocks() noexcept;

    [[nodiscard]] std::uint32_t first_free_block() const noexcept { return first_free_34_; }
    [[nodiscard]] std::uint32_t block_count() const noexcept { return static_cast<std::uint32_t>(blocks_.size()); }
    [[nodiscard]] std::uint32_t block_capacity() const noexcept { return capacity_30_; }
    [[nodiscard]] std::uint16_t free_count(std::uint32_t block) const noexcept;
    [[nodiscard]] std::uint32_t slot_page_index(std::uint32_t block, std::uint16_t index) const noexcept;

private:
    struct Block {
        std::vector<std::uint16_t> free_stack;
        std::vector<std::uint32_t> page_index;  // per slot
        std::uint16_t free_count{};
    };
    // 00BC4B70 / 00BC5970 / 00BC6B40 __thiscall(ECX = block, page index), RET 4.
    Block init_block(std::uint32_t page_index) const;

    std::uint16_t slots_per_block_{};
    std::vector<Block> blocks_;          // +28h array, +2Ch count
    std::uint32_t capacity_30_{};        // +30h, grown to 2n+2
    std::uint32_t first_free_34_{0xFFFFFFFFu};
};

// 00BC4B70 / 00BC5970 / 00BC6B40 as a free function: free stack {N-1, ..., 0}, every slot's page
// index = page, free count N.
void cloud_pool_init_block_00bc4b70(std::uint16_t slots_per_block, std::uint32_t page,
    std::vector<std::uint16_t>& free_stack, std::vector<std::uint32_t>& page_index,
    std::uint16_t& free_count);

// ---- lightning strand model (vtable 00D64A50; GeneratedModel "Lightning") ----
// Segment record, 30h bytes (copied by 00BC7F00, appended by 00BC8150).
struct LightningSegment {
    W1D_Vec3 dir_00{};        // direction from the eye; the quad sits 4500 units along it
    float half_width_0c{};
    float half_height_10{};
    float alpha_14{};
    float u0_18{};
    float v0_1c{};
    float u1_20{};
    float v1_24{};
    float anchor_x_28{};      // 0.5 = centred
    float anchor_y_2c{};      // -0.5 = centred
};

// Strand / thunder timer record, 8 bytes (vectors +194h and +1ACh).
struct LightningStrand {
    float timer_00{};
    float fade_rate_04{};     // per second, subtracted from the paired segment's alpha (+194h only)
};

// Atlas item of the definition (+24h, 14h records); the texture rectangle lives in the atlas
// entry resolved by 00AEFB20 in 00BC7460.
struct LightningAtlasItem {
    float anchor_x_00{};
    float anchor_y_04{};
    float tex_u0_14{};        // item +10h -> entry +14h..+20h
    float tex_v0_18{};
    float tex_u1_1c{};
    float tex_v1_20{};
};

// The definition the model keeps at +184h.
struct LightningDefinition {
    float life_min_00{};
    float life_max_04{};
    float height_min_08{};
    float height_max_0c{};
    float aspect_10{};
    std::uint32_t max_bursts_14{};
    float burst_interval_max_18{};
    std::vector<LightningAtlasItem> items_24{};   // count at +28h (read as signed int)
};

struct LightningModelState {
    std::vector<LightningSegment> lightning_188{};  // element 0 ("lightning.mshd"), at most 49
    std::vector<LightningStrand> strands_194{};     // parallel to lightning_188
    std::vector<LightningSegment> thunder_1a0{};    // element 1 ("thunder.mshd"), at most 49
    std::vector<LightningStrand> thunder_1ac{};     // parallel to thunder_1a0
    float burst_timer_1b8{};
};

// Every random value the strand model consumes; all native draws are 00BD2F10 stream 0
// (XOR ECX,ECX). The model never draws itself.
struct LightningRandomSource {
    virtual ~LightningRandomSource() = default;
    virtual float uniform_stream0_00bd2f10(float lo, float hi) = 0;
};

struct LightningVertex {
    W1D_Vec3 position_00{};
    float alpha_0c{};
    float u_10{};
    float v_14{};
};

// 00BC66B0 __stdcall(void* out, const LightningSegment*), RET 8. side = (0,1,0) x dir (004F9B30,
// not normalised); centre = dir * 4500.0 (double 00D63408); corners
// (-2ax*w, -2ay*h), (2(1-ax)*w, -2ay*h), (2(1-ax)*w, -2(ay+1)*h), (-2ax*w, -2(ay+1)*h) along
// side and up (doubles 00D19660 = -2.0, 00D7A308 = 2.0); uv (u0,v0),(u1,v0),(u1,v1),(u0,v1).
void lightning_segment_write_quad_vertices_00bc66b0(const LightningSegment& segment,
    std::array<LightningVertex, 4>& out) noexcept;

struct LightningElementFill {
    std::vector<LightningVertex> vertices{};   // 4 per segment, written through the locked stream
    std::uint32_t vertex_count_10{};           // element +10h = 4n
    std::uint32_t primitive_count_18{};        // element +18h = 2n
    bool locked{};                             // vertex stream vtable +10h/+14h ran
};

// 00BC6FA0 __fastcall(ECX = model): element 0 from lightning_188, element 1 from thunder_1a0.
void lightning_model_fill_segment_vertices_00bc6fa0(const LightningModelState& model,
    LightningElementFill& element0, LightningElementFill& element1);

// 00BC9160 __thiscall(ECX = model, float dt), RET 4: removes expired thunder records (timer <= 0;
// NaN is kept) with the native ascending swap-with-last order, then ages the rest by dt.
void lightning_model_age_and_remove_expired_00bc9160(LightningModelState& model, float dt);

// 00BC84A0 __thiscall(ECX = model, float dt), RET 4. Draws from 00BD2F10 stream 0 through `rng`.
void lightning_model_update_bursts_00bc84a0(LightningModelState& model, const LightningDefinition& def,
    LightningRandomSource& rng, float dt);

// 00BC9390 __thiscall(ECX = model, unused, float dt), RET 8: 00BC9160 then 00BC84A0.
void lightning_model_tick_00bc9390(LightningModelState& model, const LightningDefinition& def,
    LightningRandomSource& rng, float dt);

// 00BC7170 (vtable 00D649B4 slot) __thiscall(ECX = model, unused, float dt), RET 8: 00BC9390 then
// 00BC6FA0.
void lightning_model_tick_and_fill_00bc7170(LightningModelState& model, const LightningDefinition& def,
    LightningRandomSource& rng, float dt, LightningElementFill& element0, LightningElementFill& element1);

// Fragment of 00BC7460: the shared 300-entry index buffer (vertex buffer of 200 vertices, 50
// quads): (i, i+1, i+2, i+2, i+3, i) for i = 0, 4, ..., 196.
std::vector<std::uint16_t> lightning_model_static_indices_00bc7460();

// ---- matrix helpers used by the strand model and 00BC7C10 ----
W1D_Mat4 lightning_matrix_rotation_x_00b64640(float angle) noexcept;
W1D_Mat4 lightning_matrix_rotation_y_00b646e0(float angle) noexcept;
W1D_Mat4 lightning_matrix_rotation_z_00b64780(float angle) noexcept;
// left * right, each element summed in double over k = 0..3 and stored as float. The native
// 00413920 kernel has its own x87 schedule (src/camera_multiply.cpp); results can differ in the
// last float bit for general inputs.
W1D_Mat4 lightning_matrix_multiply_w1d(const W1D_Mat4& left, const W1D_Mat4& right) noexcept;

// 00BC7C10 __thiscall(ECX = node, const node* source), RET 4. No reference to it exists in the
// image (dead code). world = scale(50, 50, 1) * rotZ(roll) * translate(0, 0, 40) * source world
// (+0F0h); the result goes to the node's vtable +34h. Native draws roll = Uniform(0, 2*pi)
// (float 00CE3D9C) from 00BD2F10 stream 0; the model takes it as input.
W1D_Mat4 scene_node_random_roll_world_00bc7c10(const W1D_Mat4& source_world, float roll) noexcept;

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part e (cc13-water2b, docs/CC13_WATER_2B.md): container plumbing and small shells left by
// cc13-water1, modelled on the ACTUAL native storage ({data, count, capacity} arrays, intrusive
// reference slots, eight-byte strings) and read from the capstone listing, because the live Ghidra
// decompile drops memory stores in these bodies (2026-10-13). Allocation is the
// singleton_lifetime_allocate/free boundary; strings go through NativeStringStorage. Bodies that
// are byte-identical to engine_services_cc13 shapes call those. Build-tested only.
// ================================================================================================
namespace bsp::water_weather_cc13 {

using W2_Array = engine_services_cc13::NativeArrayHeader;
// How an element is copied when an array grows or appends: DWORD moves, or FLD/FSTP per float
// (which quiets a signalling NaN).
enum class W2_Copy { dwords, floats };
// A reserve routine of a specific vector, bound by the caller when this file does not reconstruct it.
using W2_Reserve = void (*)(W2_Array&, std::int32_t);
using W2_Resize = void (*)(W2_Array&, std::int32_t);

// POD vector reserve, __thiscall(ECX array, int n) RET 4: n is raised to 1; when capacity < n,
// operator new(n * size) (00BF55BE), copy count elements (each destination tested for null), free the
// old data (00BF6989), store data and capacity.
// 4 bytes: 00B9D1E0 BSP_OceanPtrVector_Reserve, 00B9D3F0 BSP_OceanDwordVector_Reserve,
//          00BC3180 BSP_CloudPuffPtrVector_Reserve; 8 bytes: 00B9D240 BSP_OceanPairVector_Reserve,
//          00BC8070 BSP_LightningStrandVector_Reserve; 1 byte: 00B9D2B0 BSP_OceanByteVector_Reserve;
//          16 bytes as floats: 00BC31E0 BSP_CloudSphereVector_Reserve.
void w2_pod_reserve(W2_Array& array, std::int32_t requested, std::uint32_t element_size, W2_Copy copy);
// Resize, __thiscall(ECX array, int n) RET 4: reserve when n > capacity, zero the new elements
// (each address tested), count down to n, store n.
// 00B9D310 (4, via 00B9D1E0), 00B9E040 (4, via 00B9D3F0), 00BC3270 (4, via 00BC3180),
// 00B9D360 (8, via 00B9D240), 00B9DF80 (1, via 00B9D2B0).
void w2_pod_resize(W2_Array& array, std::int32_t count, std::uint32_t element_size, W2_Copy copy);
// Push back, __thiscall(ECX array, const element*) RET 4: when full, reserve max(capacity * 2, 1); copy
// the element to data + count * size unless that address is null; count++.
// 00B9DE90 (4), 00BC3DE0 (4), 00B9DF40 (8), 00BC8210 (8), 00B9D3B0 (1), 00BC3E20 (16 as floats).
void w2_pod_push_back(W2_Array& array, const void* element, std::uint32_t element_size, W2_Copy copy);
// 00B9DFD0 BSP_OceanPtrVector_PushBack_0073eac0: the 4-byte push with the reserve 0073EAC0 (bound).
void w2_push_back_dword_via_00b9dfd0(W2_Array& array, const std::uint32_t* value, W2_Reserve reserve_0073eac0);
// Destroy, __fastcall(ECX array): resize to 0, free data. 00B9E560 (00B9D310), 00B9E580 (00B9D360),
// 00B9E6D0 (00B9E040), 00BC4600 (00BC3270) use the POD resize above.
void w2_pod_destroy(W2_Array& array, std::uint32_t element_size);
// The same over a resize this file does not reconstruct (bound by the caller): 00BA13C0 (00B9EF20),
// 00BA13E0 (00B9EFA0), 00BAFBE0 (00BAF070), 00BB4DC0 (00BB47A0), 00BBD7D0 (00BBD170), 00BBD860 (00BBD1D0),
// 00BBD930 (00BBD640).
void w2_destroy_via(W2_Array& array, W2_Resize resize);
// 00B9E5D0 BSP_OceanByteVector_Assign, __thiscall(ECX, const array*) RET 4, EAX this: resize 0, reserve
// the source count, then append each source byte with the push growth rule.
W2_Array& w2_byte_vector_assign_00b9e5d0(W2_Array& self, const W2_Array& source);
// Inline clear, __fastcall(ECX array): when capacity < 0 call reserve(0) (unreachable for a valid
// array); count down to 0; count = 0; the Destroy forms then free data (00BF6989).
// Clear: 00BAFBA0 (00BAEFB0), 00BAFCA0 (00BAF450), 00BBF600 (00BBF390), 00BC1950 (00BC1860), 00BC3E80
// (00BC31E0), 00BC8280 (00BC7FA0), 00BC82C0 (00BC8070). Destroy: 00BB0240 (00BAEFB0), 00BB0280 (00BAF450),
// 00BBF690 (00BBF390), 00BC1EF0 (00BC1860), 00BC4620 (00BC31E0), 00BC8300 (00BC7FA0), 00BC8340 (00BC8070).
void w2_clear_counting_down(W2_Array& array, W2_Reserve reserve, bool free_data);
// 00BB0590 BSP_CoastPointVector_Delete, __fastcall(ECX slot): when *slot is non-null, the Destroy above
// on it, then free the object (00BF6989).
void w2_coast_point_vector_delete_00bb0590(W2_Array** slot, W2_Reserve reserve);
// 00B9D460 / 00B9E090 BSP_OceanVector_FreeStorage_*, __fastcall(ECX): free +8h when non-null and clear
// it; +4h = 0; +0h = 0.
void w2_free_storage_at_8(void* object) noexcept;
// 00BC6CA0 BSP_PointerArray_Grow_00bc6ca0: capacity * 2 + 2 growth (the 00BF2EE0 body).
void w2_pointer_array_grow_00bc6ca0(W2_Array& array);

// Reference slots (count at +4h; vtable slot 0 at zero).
// 00B9A760, 00B9A780, 00BA2AB0, 00BA2AD0 BSP_OceanVector_FreeData_*: free (00BF65AC) and clear when non-null.
void w2_free_data(void** slot) noexcept;
// 00B9D6C0 (EAX slot) and 00BA2DA0, 00BA2DD0, 00BA2E00, 00BA2E30: release and clear when non-null.
void** w2_ref_release_return_self_00b9d6c0(void** slot);
void w2_ref_release(void** slot);
// 00BA2B10 BSP_RefHandle_AddRef_00ba2b10, __fastcall(ECX slot) -> EAX slot: InterlockedIncrement(+4h).
void** w2_ref_add_00ba2b10(void** slot);
// 00BA2D20, 00BA2D60, 00BB0B90, 00BB8E90, __thiscall(ECX slot, object) RET 4, EAX slot: release and clear
// the old object, store the new one without adding a reference.
void** w2_ref_assign(void** slot, void* object);
// 00B9AB40 BSP_NodeHandle_Release_00b9ab40, __fastcall(ECX slot): when *slot is non-null, the bound
// unlink-and-release routine (BSP_Node_UnlinkAndRelease) on it.
void w2_node_handle_release_00b9ab40(void** slot, void (*unlink_and_release)(void* node));

// Strings and archive records.
// 00BB9030, 00BBA890, 00BBABF0 BSP_NativeString_ReleaseStorage_*: release the string at +0h (0041DD20).
void w2_release_string(void* header, NativeStringStorage& strings) noexcept;
// 00BB5CD0 / 00BB5E50 BSP_ArchiveNameRecord_Destroy_*: free +18h when non-null; +18h..+20h = 0; release
// the string at +0h. (00BB5CD0 is __fastcall ECX; 00BB5E50 takes the record on the stack.)
void w2_archive_name_record_destroy(void* record, NativeStringStorage& strings) noexcept;
// 00BB64A0 BSP_NativeString_CopyConstruct_00bb64a0, __thiscall(ECX, const string*) RET 4 -> EAX: string
// copy, then +8h..+10h = 0.
void* w2_name_record_copy_construct_00bb64a0(void* record, const NativeString& name, NativeStringStorage& strings);
// 00BBA8F0 BSP_ArchiveEntryRecord_Construct, __thiscall(ECX, string*, a, b, BYTE, c) RET 14h -> EAX:
// string copy, +8h = a, +0Ch = b, +10h BYTE, +14h = c. 00BBA950 CopyConstruct copies those fields.
void* w2_archive_entry_record_construct_00bba8f0(void* record, const NativeString& name, std::uint32_t a,
    std::uint32_t b, std::uint8_t flag, std::uint32_t c, NativeStringStorage& strings);
void* w2_archive_entry_record_copy_00bba950(void* record, const void* source, NativeStringStorage& strings);

// Streams.
// 00BB8530 BSP_StreamBuffer_ReadWhole, __thiscall(ECX buffer, stream, size) RET 8 -> EAX: +0h = operator
// new(size); stream vtable +24h (data, size, &buffer+4h); +8h = +0h.
void* w2_stream_buffer_read_whole_00bb8530(void* buffer, void* stream, std::uint32_t size);
// 00BB8570 BSP_StreamBuffer_Destruct and 00BBBD20 BSP_MemoryBlock_Free, __fastcall(ECX): free(+0h).
void w2_free_first_word(void* object) noexcept;
// 00BB9100 BSP_MemoryStream_CreateFromBacking_00bb9100, __fastcall(ECX backing) -> EAX: operator new(14h)
// (00BF681B); ref-counted base 00CEB130 count 1, +8h..+10h = 0, vtable 00D64328 (a null allocation is
// still dereferenced below); release a previous +8h (none); +8h = backing with an added reference;
// +10h = backing +8h; +0Ch = backing +0Ch + backing +8h.
void* w2_memory_stream_create_00bb9100(void* backing);

// 30h-byte float records (lightning segments).
// 00BC7F00 BSP_Record30_Copy, __thiscall(ECX, const record*) RET 4 -> EAX: 12 floats by FLD/FSTP.
void* w2_record30_copy_00bc7f00(void* destination, const void* source) noexcept;
// 00BC81D0 BSP_Record30Array_RemoveSwapLast, __thiscall(ECX array, int index) RET 4: index -1 does
// nothing; otherwise copy the last record over index unless it is the last, then count--.
void w2_record30_remove_swap_last_00bc81d0(W2_Array& array, std::int32_t index) noexcept;

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part f (cc13-left1, docs/CC13_LEFT_1.md): bodies left open by Part e, read from the live decompile
// (reliable, lead correction 2026-10-13) with the listing checked for x87 copies and register
// arguments. Native storage as in Part e. Build-tested only.
// ================================================================================================
namespace bsp::water_weather_cc13 {

// Native 2-D cell grid (0Ch): +0h width, +4h height, +8h data of width*height 18h-byte cells. These are
// the native-storage forms of 00B9D490 / 00B9E0C0, whose semantic models are ocean_cell_grid_* above;
// the vectors below need them on native storage.
// 00B9D490 __thiscall(ECX grid, int width, int height) RET 8: operator new(width*height*18h, saturated
// to FFFFFFFFh on 32-bit overflow); copy the min(width) x min(height) overlap row by row (DWORD moves);
// free the old data when non-null; store width, height, data.
void w3_cell_grid_resize_00b9d490(void* grid, std::int32_t width, std::int32_t height);
// 00B9E0C0 __thiscall(ECX grid, const grid*) RET 4 -> EAX grid: self-assignment returns; free data and
// zero width/height; resize to the source dimensions; copy every cell.
void* w3_cell_grid_assign_00b9e0c0(void* self, const void* source);
// 00B9E7A0 BSP_OceanVector12_Reserve, __thiscall(ECX array, int n) RET 4: element is the 0Ch grid. n is
// raised to 1; when capacity < n: operator new(n*0Ch), zero-construct then assign (00B9E0C0) each old
// grid into the new block (null destinations skipped), free each old grid's data (00B9E090 shape),
// free the old block, store data and capacity.
void w3_grid_vector_reserve_00b9e7a0(W2_Array& array, std::int32_t requested);
// 00B9EF20 BSP_OceanVector12_Resize, __thiscall(ECX array, int n) RET 4: reserve when n > capacity;
// zero the new grids; destroy (free +8h, zero +0h/+4h) the grids above n from the top; count = n.
void w3_grid_vector_resize_00b9ef20(W2_Array& array, std::int32_t count);

// Native coverage region (30h): +0h owner, +4h byte vector {data, count, capacity}, +10h..+2Ch eight
// DWORDs copied verbatim (W1A_OceanCoverageRecord is the semantic model).
// 00B9E6B0 BSP_OceanCoverageRegion_DestroyMask, __fastcall(ECX region): byte vector +4h resize(0)
// (00B9DF80) and free its data.
void w3_coverage_region_destroy_mask_00b9e6b0(void* region);
// 00B9E6F0 BSP_OceanCoverageRegion_Assign, __thiscall(ECX, const region*) RET 4 -> EAX: +0h, byte
// assign (00B9E5D0) of +4h, then +10h..+2Ch. No self-assignment test.
void* w3_coverage_region_assign_00b9e6f0(void* self, const void* source);
// 00B9ED40 BSP_OceanCoverageRegion_CopyConstruct: as the assign, with +4h..+0Ch zeroed first.
void* w3_coverage_region_copy_construct_00b9ed40(void* self, const void* source);
// 00B9EDA0 BSP_OceanCoverageRegionVector_Reserve, RET 4: n raised to 1; when capacity < n operator
// new(n*30h), copy-construct each old region (null destinations skipped), destroy each old mask (the
// inline 00B9DF80 resize(0) then free), free the old block, store data and capacity.
void w3_coverage_region_vector_reserve_00b9eda0(W2_Array& array, std::int32_t requested);
// 00B9EFA0 BSP_OceanCoverageRegionVector_Resize, RET 4: reserve when n > capacity; zero +0h..+0Ch of
// each new region (the eight trailing DWORDs are left as allocated); destroy masks above n from the top.
void w3_coverage_region_vector_resize_00b9efa0(W2_Array& array, std::int32_t count);
// 00B9FFF0 BSP_OceanCoverageRegionVector_PushBack, __thiscall(ECX array, const region*) RET 4: when full
// reserve max(capacity*2, 1); copy-construct at the end unless that address is null; count++.
void w3_coverage_region_vector_push_back_00b9fff0(W2_Array& array, const void* region);
// 00B9ECE0 BSP_OceanCoverageRegionVector_EraseAt, RET 4: index -1 does nothing; otherwise assign the
// last region over index unless it is the last, destroy the last region's mask, count--.
void w3_coverage_region_vector_erase_at_00b9ece0(W2_Array& array, std::int32_t index);

// 30h lightning segments (twelve floats, copied by FLD/FSTP; the listing at 00BC7FEC / 00BC817E).
// 00BC7FA0 BSP_LightningSegmentVector_Reserve: w2_pod_reserve(array, n, 30h, floats).
void w3_lightning_segment_vector_reserve_00bc7fa0(W2_Array& array, std::int32_t requested);
// 00BC8150 BSP_LightningSegmentVector_Append: w2_pod_push_back(array, segment, 30h, floats).
void w3_lightning_segment_vector_append_00bc8150(W2_Array& array, const void* segment);

}  // namespace bsp::water_weather_cc13

namespace bsp {
struct TrackedCriticalSection;
}

namespace bsp::water_weather_cc13 {

// Runtime plumbing at the end of segment 94 (00BD15D0-00BD2B30).
// 00BD15D0 BSP_GameAllocator_Destroy, __fastcall(ECX allocator): vtable 00D68204; the shared small-return
// shutdown gate 01090AA4 = 1; drain the positive SIGNED depth at +0B89AB0h (LeaveCriticalSection on the
// section at +0B89A98h each step); DeleteCriticalSection; the publication 01090AAC = 0; base vtable
// 00CE3818. The second pool owner, twin of destroy_native_string_pool_00bd14c0 (whose section sits at
// +8AD484h and publication at 01090AA8). No unregister or free.
void w3_game_allocator_destroy_00bd15d0(void* allocator, volatile std::uint32_t& small_returns_disabled_01090aa4,
                                        void* volatile& published_01090aac) noexcept;
// 00BD1890 BSP_CountedCriticalSection_Create: byte-identical to 00BD1860 (operator new(1Ch),
// InitializeCriticalSection, depth +18h = 0; null when the allocation is null). No caller or data xref.
TrackedCriticalSection* w3_counted_critical_section_create_00bd1890();
// 00BD18F0 BSP_Event_Destroy, __fastcall(ECX event): vtable 00D6821C; CloseHandle(+4h); vtable 00D68208.
// No caller or data xref in the image.
void w3_event_destroy_00bd18f0(void* event) noexcept;
// String builder appends into the builder's first eight-byte string (native_vfs_open_logging.hpp
// layout: +0h text, +8h "%d" format, +10h "%.3f" format; data pointers at +0Ch and +14h).
// 00BD1B10 BSP_StringBuilder_AppendChar, __thiscall(ECX builder, char) RET 4 -> EAX: a two-byte pooled
// block (00419CC0 + 00BD1120(2)) holding {c, 0}; resize the text by one, preserving (0041DD40); store
// the byte; return the block (00BD1510(2)).
void* w3_string_builder_append_char_00bd1b10(void* builder, char c, NativeStringStorage& strings);
// 00BD1D90 BSP_StringBuilder_AppendFloat, __thiscall(ECX builder, float) RET 4 -> EAX: sprintf into a
// 64-byte local with the format at +14h (01090AB4 when null) and the value widened to double; the
// pooled temporary/append/release schedule of 00BD1BB0. Output that does not fit 64 bytes overruns
// the native stack buffer; this source rejects it, as append_native_log_int_00bd1bb0 does.
void* w3_string_builder_append_float_00bd1d90(void* builder, float value, NativeStringStorage& strings,
                                              const char* null_format_01090ab4);
// 00BD1E80 BSP_StringBuilder_AppendPointer, __thiscall(ECX builder, void*) RET 4 -> EAX: as above with
// the fixed format "0x%p".
void* w3_string_builder_append_pointer_00bd1e80(void* builder, const void* value, NativeStringStorage& strings);
// 00BD2180 BSP_Singleton0109DBDC_Get, no arguments -> EAX: the error reporter singleton. When 0109DBDC is
// null: under the manager's section (00415350 +10h, with its +18h depth), recheck, operator new(18h),
// BSP_ErrorReporter_Construct 00BF2C40 (null stays null), register with the manager (00BD0C30).
void* w3_error_reporter_singleton_00bd2180(void*& published_0109dbdc, SingletonLifetimeDomain& domain,
                                           engine_services_cc13::EngineServiceCalls& calls);
// 00BD2390 BSP_NativeString_CopyConstructInto_00bd2390, __thiscall(ECX source, string* destination)
// RET 4 -> EAX destination: zero the destination; unless it is the source, resize it to the source
// length (preserving) and copy that many bytes when the length is nonzero.
void* w3_native_string_copy_construct_into_00bd2390(const void* source, void* destination,
                                                    NativeStringStorage& strings);
// 00BD26B0 BSP_LogMessage_AppendCString, __thiscall(ECX message, const char*) RET 4 -> EAX message: a
// temporary from the C text (0041E870: zero, strlen, resize preserving, copy length + 1), appended to
// the message text at +20h when nonempty, then released (length + 1 bytes).
void* w3_log_message_append_cstring_00bd26b0(void* message, const char* text, NativeStringStorage& strings);

}  // namespace bsp::water_weather_cc13

namespace bsp {
struct AllocatorListElement;
class AllocatorListDomain;
}

namespace bsp::water_weather_cc13 {

// Small shells. A routine these bodies call and this file does not reconstruct on native storage is
// passed in by the caller, named after its address.
using W3_Body = void (*)(void* object);

// Deleting destructors, __thiscall(ECX object, BYTE flags) RET 4 -> EAX object: store vtable (when
// nonzero), call the body (when present), then dispose of the object when flags bit 0 is set.
//   00BC51F0 BSP_CloudRenderModel_DeletingDestructor  vtable 00D64878, body 00BBE2D0, dispose pool 010909F4 free 00BC5070
//   00BC5D60 BSP_CloudPlaneModel_DeletingDestructor   vtable 00D648E0, body 00BC7BB0, dispose pool 01090A2C free 00BC5BE0
//   00BC6F70 BSP_LightningModel_DeletingDestructor    vtable 00D64958, body 00BC8380, dispose pool 01090A68 free 00BC6DF0
//   00BC2A00 BSP_WaterdropsPooled_DeletingDestructor  vtable 00D647D0, body 00BC1F30, dispose pool 010909BC free 00BC2880
//   00BBE600 BSP_CloudInstance_DeletingDestructor     vtable 00D64528, body 00B750C0, dispose 00B748C0
//   00BC7BE0 BSP_GeneratedModel_ScalarDeletingDestructor_00bc7be0 vtable 00D649E8, body 00B750C0, dispose 00B748C0
//   00BC93C0 BSP_LightningModel_DeletingDestructor    no vtable store, body 00BC8380, dispose 00B748C0
//   00BC2010 BSP_Waterdrops_DeletingDestructor        no vtable store, body 00BC1F30, dispose 00B748C0
//   00BBC460 BSP_ProceduralTextureFactory_ScalarDeletingDestructor vtable 00D64468, no body, dispose
//            00BF65AC (the flag is tested before the store; the order is unobservable)
void* w3_deleting_destructor(void* object, std::uint32_t flags, std::uint32_t vtable, W3_Body body, W3_Body dispose);

// "SUB ECX,4; JMP target" adjustor thunks: 00BA24A0 (-> 00BA29F0) and 00BA6F70 (-> 00BA6F90), the
// secondary-base slots of the wave field base and wave field vtables. Flags stay on the stack.
void* w3_adjustor_thunk_minus4(void* secondary, std::uint32_t flags, void* (*target)(void* object, std::uint32_t flags));

// 00BBE7C0 BSP_CloudInstance_TypeInfoThunk, __fastcall(ECX) -> EAX ECX: call 00BBE770 (once-only type info
// "cCloudInstance"), return the object.
void* w3_cloud_instance_type_info_thunk_00bbe7c0(void* object, W3_Body init_type_info_00bbe770);

// 00BBC4A0 / 00BBC4C0 BSP_ProceduralFactorySlot_DeleteIfSet_*, __fastcall(ECX slot): when *slot is non-null,
// its vtable slot 0 (scalar deleting destructor) with flag 1, then *slot = 0.
void w3_delete_slot_if_set(void** slot);

// 00BC7B60 BSP_RefCountedBase_Construct_00bc7b60, __fastcall(ECX) -> EAX: vtable 00CEB130, count +4h = 1,
// vtable 00D649DC.
void* w3_ref_counted_base_construct_00bc7b60(void* object) noexcept;
// 00BBC670 BSP_CausticsTextureSource_Construct / 00BBC7C0 BSP_ShoreWaveTextureSource_Construct,
// __fastcall(ECX) -> EAX: BSP_TextureSource_Construct 00C30470, then vtable 00D64478 / 00D644B4.
void* w3_caustics_texture_source_construct_00bbc670(void* object) noexcept;
void* w3_shore_wave_texture_source_construct_00bbc7c0(void* object) noexcept;
// 00BC2160 BSP_WaterdropsResource_Construct, __fastcall(ECX) -> EAX: BSP_Waterdrops_Construct 00BC2030,
// then the pooled vtable 00D647D0.
void* w3_waterdrops_pooled_construct_00bc2160(void* object, W3_Body waterdrops_construct_00bc2030);
// GeneratedModel subclasses, __fastcall(ECX) -> EAX: a pooled temporary name, BSP_GeneratedModel_Construct
// 00B75030(object, &name), release the name, store the vtable.
//   00BC7DB0 BSP_CloudPlaneModel_ConstructBase: "cloudplane", vtable 00D649E8.
//   00BC93E0 BSP_LightningModel_ConstructBase: "Lightning", vtable 00D64A50, then +188h..+1B4h = 0 (the
//            strand and segment vector headers).
using W3_ConstructGeneratedModel = void* (*)(void* object, const void* name_header);
void* w3_cloud_plane_model_construct_base_00bc7db0(void* object, NativeStringStorage& strings,
                                                  W3_ConstructGeneratedModel construct_00b75030);
void* w3_lightning_model_construct_base_00bc93e0(void* object, NativeStringStorage& strings,
                                                W3_ConstructGeneratedModel construct_00b75030);
// 00BC2030 BSP_Waterdrops_Construct, __fastcall(ECX) -> EAX: the pooled name "waterdrops" (00D647C4,
// length 0Ah), BSP_GeneratedModel_Construct 00B75030(object, &name), release the name, vtable 00D64760,
// then +184h/+188h/+18Ch = 0, +194h = 0.01f (00D7A238), +198h = 0.025f (00D1FBBC), +19Ch = 0.5f (00CE3800),
// +1A0h = 4.0f (00CE3D34), +1C8h = 0, bytes +1D5h..+1D7h = 0, +1C4h = 0.95f (00CED9E0), byte +1D4h = 1.
void* waterdrops_construct_00bc2030(void* object, NativeStringStorage& strings,
                                    W3_ConstructGeneratedModel construct_00b75030);

// 00BB8690 BSP_MemoryStream_CastFromStream_00bb8690, __fastcall(ECX stream) -> EAX: null stays null;
// otherwise the stream's vtable +0Ch (IsKindOf) on the memory file type (00BEF480 returns 0109DBA0); the
// stream when that returns nonzero AL, else null.
void* w3_memory_stream_cast_00bb8690(void* stream, void* memory_file_type_0109dba0);
// 00BB8E70 BSP_MpkgProvider_OpenEntry, __thiscall(ECX provider, name, flags) RET 8: tail jump to
// BSP_MPKG_OpenNamedEntry 00BB8D60 with ECX = provider +14h.
void* w3_mpkg_provider_open_entry_00bb8e70(void* provider, const void* name, std::uint32_t flags,
    void* (*open_named_entry_00bb8d60)(void* state, const void* name, std::uint32_t flags));
// 00BB5330 BSP_MpakDirectory_MakeMemberIterator: ECX out, EDX name (a register input; the body pushes it
// as the third argument of 00BB4F40), stack (container, first, unused, end) RET 10h -> EAX out:
// out +4h = BSP_MpakDirectoryVector_FindMember 00BB4F40(ECX first, EDX end, name); out +0h = container.
// No caller or data xref in the image.
void* w3_mpak_directory_make_member_iterator_00bb5330(void* out, const void* name, void* container, void* first,
    void* end, void* (*find_member_00bb4f40)(void* first, void* end, const void* name));
// 00BACA80 BSP_TracerParameterCurve_DestroyRecords, __fastcall(ECX records): resize 00BAC5E0(0), free
// the data at +0h (00BF6989); the w2_destroy_via shape.
void w3_tracer_curve_destroy_records_00baca80(W2_Array& records, W2_Resize resize_00bac5e0);

// Fixed-block model pools (Mit::allocator_list_elem, list head 00E188B4), 38h bytes, the same layout as
// DirectionalLightPoolStorage. CloudPagedPool above models the block bookkeeping; these are the native
// constructor and destructor shells.
struct W3_ModelPoolStorage {
    std::uint32_t native_vtable_00;
    void* previous_04;
    void* next_08;
    alignas(4) std::byte critical_section_0c[24];
    std::int32_t recursion_24;
    std::byte** blocks_28;
    std::uint32_t block_count_2c;
    std::uint32_t table_capacity_30;
    std::uint32_t first_free_34;
};
static_assert(sizeof(W3_ModelPoolStorage) == 0x38);
// Construct, __fastcall(ECX) -> EAX: base vtable 00D7A0C0, prepend to 00E188B4; vtable; InitializeCriticalSection
// +0Ch; +24h..+30h = 0, +34h = -1; reserve the block table to 32 entries (operator new(80h), copy, free).
//   00BC5220 BSP_CloudRenderModelPool_Construct (vtable 00D648D8, global 010909F4),
//   00BC5D90 BSP_CloudPlaneModelPool_Construct (00D6493C, 01090A2C),
//   00BC7190 BSP_LightningModelPool_Construct (00D649B8, 01090A68).
void w3_model_pool_construct(W3_ModelPoolStorage& pool, AllocatorListDomain& list_00e188b4, std::uint32_t vtable);
// Destroy, __fastcall(ECX): vtable; free every block, then the table when non-null; drain the positive
// lock depth (LeaveCriticalSection); DeleteCriticalSection; base vtable and unlink from 00E188B4.
//   00BC4FB0 (00D648D8), 00BC5B20 (00D6493C), 00BC6D10 (00D649B8).
void w3_model_pool_destroy(W3_ModelPoolStorage& pool, AllocatorListDomain& list_00e188b4, std::uint32_t vtable) noexcept;
// Free thunks, __fastcall(ECX object): the pool free on the global pool (RET 4 callee).
//   00BC51E0 (010909F4, 00BC5070), 00BC5D50 (01090A2C, 00BC5BE0), 00BC6F60 (01090A68, 00BC6DF0).
void w3_model_pool_free_thunk(void* object, void* pool, void (*pool_free)(void* pool, void* object));

// Destructor bodies and the cloud class constructor (segment 94).
// 00BC8380 BSP_LightningModel_DestructBody, __fastcall(ECX): vtable 00D64A50; the inline clear-and-free
// of the four vectors in the order +1ACh (8-byte strands, reserve 00BC8070), +1A0h (30h segments,
// 00BC7FA0), +194h (strands), +188h (segments); BSP_Model_Destroy 00B750C0.
void w3_lightning_model_destruct_body_00bc8380(void* model, W3_Body model_destroy_00b750c0);
// 00BC4660 BSP_CloudClass_DestructBody, __fastcall(ECX): vtable 00D64860; release and clear each puff
// reference in the pointer vector at +1Ch; release and clear +68h; release the texture name string at
// +60h; clear-and-free the 16-byte sphere vector at +30h (reserve 00BC31E0); resize the puff vector to
// 0 (00BC3270) and free it; release the name string at +0Ch; BSP_RefCounted_DestroyBase 00BD30F0.
void w3_cloud_class_destruct_body_00bc4660(void* cloud_class, NativeStringStorage& strings,
                                          W3_Body destroy_base_00bd30f0);
// 00BC4A60 BSP_CloudClass_Construct, __thiscall(ECX, manager_18, value_14, const string* name, const
// string* texture, owner_08) RET 14h -> EAX: vtable 00CEB130, count 1, vtable 00D64860; +8h = owner;
// name copied into +0Ch (the 00BD2390 copy body); +14h, +18h; the puff and sphere vector headers (+1Ch..
// +24h, +30h..+38h) = 0; texture copied into +60h; BYTE +28h = BYTE +40h = 0; BSP_CloudClass_Load
// 00BC47D0 (ECX this).
void* w3_cloud_class_construct_00bc4a60(void* cloud_class, std::uint32_t manager_18, std::uint32_t value_14,
    const void* name, const void* texture, std::uint32_t owner_08, NativeStringStorage& strings,
    W3_Body load_00bc47d0);
// 00BBF6D0 BSP_SkyBox_DestructBody, __fastcall(ECX): vtable 00D64618; release and clear the three layer
// references at +84h, +0D8h, +12Ch (54h stride); release and clear +8h and +10h; clear-and-free the
// 4-byte vectors at +24h then +18h (the inline 4-byte reserve-to-1 when capacity < 0); base 00BD30F0.
void w3_sky_box_destruct_body_00bbf6d0(void* sky_box, W3_Body destroy_base_00bd30f0);

// Release-sequence destructors. Each body below is a fixed list of steps over the object's fields,
// run in listing order; the step kinds are the inline idioms of these destructors:
//   ref           release (InterlockedDecrement +4h, slot 0 at zero) and clear when non-null
//   node          when non-null BSP_Node_UnlinkAndRelease 00B6DFA0 (ECX node), then clear
//   node_keep     the same without the clear
//   node_always   00B6DFA0 without a null test or clear
//   free_clear    when non-null free (00BF6989) and clear;  free_only: free without the clear
//   string        release the eight-byte string (0041DD20 shape: length + 1 bytes)
//   destroy_via   resize(0) then free the data of a {data, count, capacity} header
//   clear_vector  w2_clear_counting_down (reserve(0) when capacity < 0, count down, free)
//   owner_null    BSP_Material_SetParameterOwner 00B18A40(ECX field, 0, 0)
//   parent_null   BSP_Node_SetParent 00B6E680(ECX field, 0)
// A resize or reserve this file does not reconstruct is supplied in W3_ReleaseCalls.
struct W3_ReleaseCalls {
    W3_Body node_unlink_and_release_00b6dfa0;
    void (*node_set_parent_00b6e680)(void* node, void* parent);
    void (*material_set_parameter_owner_00b18a40)(void* material, void* a, void* b);
    W2_Resize bound_resize;    // the one external resize a body uses (named per body)
    W2_Reserve bound_reserve;  // the one external reserve a body uses (named per body)
    W3_Body base_destructor;   // the final base call (named per body)
    NativeStringStorage* strings;
};
// 00BA24B0 BSP_OceanWaveFieldBase_Destruct, __fastcall(ECX): vtables 00D63B5C / 00D63B54 (+4h); 34 field
// steps; vectors +620h (00B9EFA0), +614h (00B9D360), +5F4h (bound 0059FCE0), +4F8h (00B9D310), +2B0h
// (00B9EF20), +27Ch (00B9E040); strings +0A8h, +0A0h; base BSP_WeakOwner_Destroy 00925540.
void w3_ocean_wave_field_base_destruct_00ba24b0(void* field, const W3_ReleaseCalls& calls);
// 00BA2E60 BSP_OceanBeams_Destruct: vtable 00D63B88; base 00925540.
void w3_ocean_beams_destruct_00ba2e60(void* beams, const W3_ReleaseCalls& calls);
// 00BA42A0 BSP_OceanWaterParticles_Destruct: vtable 00D63BB4; base 00925540.
void w3_ocean_water_particles_destruct_00ba42a0(void* particles, const W3_ReleaseCalls& calls);
// 00BA71C0 BSP_OceanHeightGenerator_Destruct: vtable 00D63E34; 30 field steps; base 00BD30F0.
void w3_ocean_height_generator_destruct_00ba71c0(void* generator, const W3_ReleaseCalls& calls);
// 00BAC8A0 BSP_WaterTracerResource_Destruct: vtable 00D63F94; vectors +24h, +18h over bound 00468050;
// base 00BD30F0.
void w3_water_tracer_resource_destruct_00bac8a0(void* resource, const W3_ReleaseCalls& calls);
// 00BAE170 BSP_WaterSprayResource_Destruct: vtable 00D64094; +0Ch, +8h; base 00BD30F0.
void w3_water_spray_resource_destruct_00bae170(void* resource, const W3_ReleaseCalls& calls);
// 00BAE210 BSP_WaterSprayObject_Destruct: vtable 00D6409C; SetParent(+8h, 0); unlink +8h (no test);
// +0Ch; base 00BD30F0.
void w3_water_spray_object_destruct_00bae210(void* object, const W3_ReleaseCalls& calls);
// 00BB03D0 BSP_Coastline_Destruct: vtable 00D640DC; vector +14h over bound 00BAF070, vector +8h clear
// over bound reserve 00BAEFB0; base 00BD30F0.
void w3_coastline_destruct_00bb03d0(void* coastline, const W3_ReleaseCalls& calls);
// 00BB0C90 BSP_CoastMesh_Destruct: vtable 00D640E4; base 00925540.
void w3_coast_mesh_destruct_00bb0c90(void* mesh, const W3_ReleaseCalls& calls);
// 00BBB660 BSP_MsarProvider_Destroy, __fastcall(ECX): vtable 00D643C4; release +14h; when the entry
// vector's begin (+1Ch) is non-null: destroy [begin, end +20h) (00BBB020: ECX first, EDX last, stack
// &vector +18h, tag) and operator delete (00BF65AC) the block; +1Ch..+24h = 0; BSP_FileProviderBase_Destroy
// 00BB5380.
void w3_msar_provider_destroy_00bbb660(void* provider, const W3_ReleaseCalls& calls,
    void (*destroy_range_00bbb020)(void* first, void* last, void* vector));

// Beams and water particles constructors. Each builds a default scene-record texture layer in its
// 0A4h-byte frame, copies the settings out of it and releases the record's string (+8h length,
// +0Ch data).
struct W3_SettingsCalls {
    W3_Body weak_owner_construct_00925490;         // ECX object
    W3_Body construct_default_layer;               // 004C63E0 (beams) / 004C62A0 (particles), ECX record
    void (*assign_settings)(void* object, const void* record);  // 0078D9F0 / 0078D940, ECX object, RET 4
    NativeStringStorage* strings;
};
// 00BA3A60 BSP_OceanBeams_Construct, __thiscall(ECX, ref* a, ref* b) RET 8 -> EAX: weak owner; vtable
// 00D63B88; +0Ch = *a, +10h = *b (each with an added reference); +20h..+28h, +34h, +38h, +0E4h, +0E8h,
// +14h = 0; the (null) +24h and +20h release-and-clear; settings; +0F0h = 0.0f.
void* w3_ocean_beams_construct_00ba3a60(void* beams, void* const* a, void* const* b, const W3_SettingsCalls& calls);
// 00BA43B0 BSP_OceanWaterParticles_Construct, __thiscall(ECX, ref* a) RET 4 -> EAX: weak owner; vtable
// 00D63BB4; +0Ch = *a (added reference); +14h..+1Ch, +28h, +2Ch, +10h = 0; the (null) +18h and +14h
// release-and-clear; settings; +0C0h = 0.0f.
void* w3_ocean_water_particles_construct_00ba43b0(void* particles, void* const* a, const W3_SettingsCalls& calls);
// 00BBB5B0 BSP_MsarProvider_Construct, __thiscall(ECX, name) RET 4 -> EAX: BSP_FileProviderBase_Construct
// 00BB5590 (ECX this, name); vtable 00D643C4; +1Ch..+24h = 0; +14h = file system 0109CEEC vtable +4h (ECX fs,
// this +8h, 2), the backing stream.
void* w3_msar_provider_construct_00bbb5b0(void* provider, void* name, void* file_system_0109ceec,
    void (*file_provider_base_construct)(void* provider, void* name));

// Factories: allocate, then construct when the allocation is non-null (EAX 0 otherwise).
// 00BA6FB0 BSP_OceanWaveField_Create, __fastcall(ECX world, EDX flat byte) -> EAX: operator new(6BCh)
// (00BF681B), BSP_OceanWaveField_Construct 00BA6C40(obj, world, flat).
void* w3_ocean_wave_field_create_00ba6fb0(void* world, std::uint32_t flat,
    void* (*construct_00ba6c40)(void* field, void* world, std::uint32_t flat));
// 00BB3C60 BSP_OceanHeightGenerator_Create, __fastcall(ECX a, EDX b, c, d) RET 8 -> EAX: operator new(118h),
// BSP_OceanHeightGenerator_ConstructDerived 00BB3AB0(obj, a, b, c, d).
void* w3_ocean_height_generator_create_00bb3c60(void* a, void* b, void* c, void* d,
    void* (*construct_00bb3ab0)(void* generator, void* a, void* b, void* c, void* d));
// Application vtable 00CFEAB0 factories (ECX application unused):
//   00BC02E0 +1Ch, RET 8: pool 010909F4 allocate (00BC54E0), BSP_CloudRenderModel_Construct 00BC54F0(slot, a, b).
//   00BC0440 +28h, RET 8: pool 01090A68 allocate (00BC7450), BSP_LightningModel_Construct 00BC7460(slot, a, b).
void* w3_application_create_pooled2(void* (*pool_allocate)(), void* a, void* b,
    void* (*construct)(void* slot, void* a, void* b));
//   00BC0350 +20h, RET 10h: pool 01090A2C allocate (00BC6050), BSP_CloudPlaneModel_Construct 00BC6060(slot, a, b, c, d).
void* w3_application_create_cloud_plane_model_00bc0350(void* a, void* b, void* c, void* d, void* (*pool_allocate_00bc6050)(),
    void* (*construct_00bc6060)(void* slot, void* a, void* b, void* c, void* d));
//   00BC03C0 +24h, RET 10h: operator new(38h), 00BC6430(obj, a, b, float c, float d); the two floats are
//   re-stored through FLD/FSTP on the way.
void* w3_application_create_cloud_puff_00bc03c0(void* a, void* b, float c, float d,
    void* (*construct_00bc6430)(void* object, void* a, void* b, float c, float d));

// D3D builders. They call the renderer device (the object at 00F8D394) through its native vtable:
//   +80h  size* (size* out): writes the back-buffer {width, height} and returns out
//   +88h  texture* (width, height, levels, format, usage): a render-target texture
//   +94h  surface* (width, height, format, a, float b, c): a depth-stencil surface
// Slot meanings are read from these call sites; the D3DFMT values (15h A8R8G8B8, 17h R5G6B5, 71h
// A16B16G16R16F, 4Bh D24S8, 50h D16) are the arguments the listing passes.
// 00B9DC20 BSP_OceanWaveField_CreateReflectionTargets, __thiscall(ECX field, format, const size*) RET 8:
// release and clear +0E4h, +0E8h, +288h; +0E4h = +88h(size.w, size.h, 1, format, 10h); +0E8h =
// +88h(bb.w, bb.h, 1, 71h, 10h); +288h = +94h(bb.w, bb.h, format == 17h ? 50h : 4Bh, 0, 0.0f, 1). Each
// back-buffer pair calls +80h twice, taking the width from the first result and the height from the
// second (listing 00B9DCD6..00B9DD79).
void w3_ocean_wave_field_create_reflection_targets_00b9dc20(void* field, std::uint32_t format,
                                                            const std::uint32_t* size, void* device_00f8d394);

// Engine routines the builders call that have no native-storage reconstruction here. Each method is the
// native routine at the address in its name, with the native register/stack arguments in order.
class W3_EngineCalls {
public:
    virtual ~W3_EngineCalls() = default;
    // 00B4C700 BSP_D3D9Renderer_CreateGeneratedModel: ECX name*, EDX geometry file*, stack (shader file*,
    // a, b, c, d) -> EAX model.
    virtual void* create_generated_model_00b4c700(const void* name, const void* geometry, const void* shader,
                                                  std::uint32_t a, std::uint32_t b, std::uint32_t c, std::uint32_t d) = 0;
    // 00B6D890 BSP_Node_PropagateRootRegistration, __thiscall(ECX node, root) RET 4.
    virtual void node_propagate_root_00b6d890(void* node, void* root) = 0;
    // 00B189F0 BSP_Material_SetTextureSlot, __thiscall(ECX material, slot, texture) RET 8.
    virtual void material_set_texture_slot_00b189f0(void* material, std::uint32_t slot, void* texture) = 0;
    // 00B86500 BSP_DrawSection_SetVertexStream, __thiscall(ECX section, index, stream) RET 8.
    virtual void draw_section_set_vertex_stream_00b86500(void* section, std::uint32_t index, void* stream) = 0;
    // 00B1FBB0 BSP_FrameTargets_Construct, __fastcall(ECX 40h block) -> EAX; 00B1FAB0 SetColorSurface,
    // __thiscall(ECX, index, surface) RET 8; 00B1FB00 SetDepthSurface, __thiscall(ECX, surface) RET 4.
    virtual void* frame_targets_construct_00b1fbb0(void* block) = 0;
    virtual void frame_targets_set_color_00b1fab0(void* targets, std::uint32_t index, void* surface) = 0;
    virtual void frame_targets_set_depth_00b1fb00(void* targets, void* surface) = 0;
    // Model assembly. ECX carries the byte size into the three raw-slot allocators.
    virtual void* material_allocate_raw_slot_00b18780(std::uint32_t size) = 0;
    virtual void* material_construct_with_effect_00b18900(void* slot, void* effect) = 0;           // RET 4
    virtual void material_set_parameter_owner_00b18a40(void* material, void* owner, void* b) = 0;  // RET 8
    virtual void* mesh_allocate_raw_slot_00b73b60(std::uint32_t size) = 0;
    virtual void* mesh_construct_00b73d70(void* slot) = 0;
    virtual void mesh_set_vertex_stream_00b73bb0(void* mesh, std::uint32_t index, void* stream) = 0;  // RET 8
    virtual void mesh_set_index_stream_00b73b70(void* mesh, void* stream) = 0;                         // RET 4
    virtual void mesh_append_draw_section_00b73c60(void* mesh, void* section) = 0;                     // RET 4
    virtual void* draw_section_create_00533fa0() = 0;
    virtual void draw_section_rebuild_layout_00b865a0(void* section, void* mesh) = 0;                  // RET 4
    virtual void draw_section_set_material_00b864c0(void* section, void* material) = 0;                // RET 4
    virtual void* model_allocate_raw_slot_00b74eb0(std::uint32_t size) = 0;
    virtual void* generated_model_construct_00b75030(void* slot, const void* name) = 0;                // RET 4
    // 00B75170 BSP_GeneratedModel_SetGeometry, __thiscall(ECX model, a, mesh, float, float) RET 10h.
    virtual void generated_model_set_geometry_00b75170(void* model, std::uint32_t a, void* mesh, float x, float y) = 0;
    virtual void node_set_parent_00b6e680(void* node, void* parent) = 0;                               // RET 4
    virtual void generated_model_set_local_bounds_00b74390(void* model, const float* bounds) = 0;      // RET 4
    // Cameras and viewports.
    virtual void* camera_allocate_raw_slot_00b71930(std::uint32_t size) = 0;  // ECX size
    virtual void* camera_construct_00b71a80(void* slot, const void* name) = 0;                         // RET 4
    virtual void camera_set_clear_flags_00b6fe10(void* camera, std::uint32_t flags) = 0;               // RET 4
    virtual void camera_set_clear_color_00b6fe50(void* camera, std::uint32_t argb) = 0;                // RET 4
    virtual void* viewport_construct_00b1f850(void* block) = 0;
    virtual void camera_set_viewport_00b71990(void* camera, void* viewport) = 0;                       // RET 4
    virtual void viewport_set_dimensions_00b1f940(void* viewport, const std::uint32_t* size) = 0;      // RET 4
    virtual void* material_create_for_effect_name_00535320(const void* name) = 0;  // ECX name -> EAX
    virtual void* particle_index_stream_00af10a0(void* resources) = 0;             // ECX resources -> EAX
    virtual void mesh_set_bounds_00b72650(void* mesh, const float* bounds) = 0;    // RET 4
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float factor, std::uint32_t b) = 0;  // RET 8
    virtual void draw_section_append_vertex_stream_00b85b80(void* section, void* stream) = 0;              // RET 4
    // Wave field pieces this part does not reconstruct.
    virtual const std::uint32_t* wave_field_reflection_target_size_00b9a3b0(void* field, std::uint32_t* out) = 0;
    virtual void wave_field_initialize_render_resources_00ba5610(void* field) = 0;
};
// Further device and texture slots used below: device +38h declaration* (const string* file); device
// +5Ch vertex buffer* (count, 1, declaration); texture +30h surface* (level, 0); surface +1Ch width,
// +20h height.
// Field getters these builders call, inlined because each is one load: 00B74640 model geometry [+180h]
// (RET 4, its argument unused); 00B732C0 element i [[+54h] + 4i]; 00B73260 vertex stream i [+64h + 4i];
// 00B72B40 element count [+58h]; 00BA7080 current height stream [+78h + 4 * [+84h]]; 00BA7190 [+44h];
// 00BA7170 [+8h]; 00B9A500 BYTE [+10h].

// 00BA4EB0 BSP_OceanWaveField_BindHeightStream, __fastcall(ECX field). When BYTE [field +10h] is set:
// the height generator's (+2BCh) texture [+44h] goes to slot 8 of the materials +12Ch and +130h and to
// slot 0 of the first element's material of model +4A0h. Otherwise: generator vtable +8h, then for each
// element of model +11Ch's geometry (count reread every pass) set vertex stream 1 to the generator's
// current height stream.
void w3_ocean_wave_field_bind_height_stream_00ba4eb0(void* field, W3_EngineCalls& calls);

// 00BB3DB0 BSP_OceanHeightGenerator_CreateStreams, __thiscall(ECX, width, height) RET 8: release +78h and
// +7Ch; declaration = device +38h("oceanheightmap.mvfm"); +78h and +7Ch = device +5Ch(width * height, 1,
// declaration); release the declaration (no null test).
void w3_ocean_height_generator_create_streams_00bb3db0(void* generator, std::int32_t width, std::int32_t height,
                                                      void* device_00f8d394, NativeStringStorage& strings);
// 00BA94F0 BSP_OceanHeightGenerator_Construct, __thiscall(ECX, owner, width, height, const float[4]) RET 10h
// -> EAX: vtable 00CEB130, count 1, +88h = 0, vtable 00D63E34; fields zeroed; +8Ch..+98h = the four
// DWORDs; +88h = owner; +68h..+7Ch = 0; render targets through device +88h: +68h (s*w, s*h, 1, 71h, 10h),
// +6Ch (s*w, s*h, 1, 15h, 10h) with s = the global 00E14454 (read per call), +44h (w, h, 1, 71h, 10h) with
// surface +48h, +60h (w, h, 1, 71h, 2) with surface +64h, surfaces +70h / +74h of +68h / +6Ch, +10h (80h,
// 80h, 1, 15h, 10h) with surface +14h; then 00BB3DB0(w, h) and BSP_OceanHeightGenerator_InitializePasses
// 00BA7530(w, h).
void* w3_ocean_height_generator_construct_00ba94f0(void* generator, void* owner, std::int32_t width,
    std::int32_t height, const std::uint32_t* params, const volatile std::int32_t& scale_00e14454,
    void* device_00f8d394, NativeStringStorage& strings,
    void (*initialize_passes_00ba7530)(void* generator, std::int32_t width, std::int32_t height));
// 00BB3AB0 BSP_OceanHeightGenerator_ConstructDerived, __thiscall(ECX, a, b, c, d) RET 10h -> EAX: 00BA94F0
// (a, b, c, d); vtable 00D64168; depth = device +94h(width, height, 4Bh, 0, 0.0f, 1) of the surface +70h
// (vtable +20h height is called first, then +1Ch width); three frame targets
// (operator new(40h) + 00B1FBB0, null stays null) at +40h, +5Ch, +2Ch with colour surfaces +70h, +48h,
// +74h and that depth; release the depth when non-null.
// 00BAE900 BSP_WaterSprayObject_Construct, __thiscall(ECX, parent, const resource*) RET 8 -> EAX. No static
// reference (cc13-water1). Ref-counted base, vtable 00D6409C, +8h = +0Ch = 0; material (110h slot,
// constructed with resource +0Ch) at +0Ch, owner this, texture resource +10h in slot 0; a mesh (0BCh slot)
// with vertex stream 0 of resource +8h and its index stream (+60h); one draw section {+8h 4, +0Ch 0,
// +10h resource +14h, +14h 0, +18h resource +18h} with the material, appended and released (no null test);
// model "WaterSpray" (184h slot) at +8h with that geometry (00B75170 with 0 and -1.0f twice, 00D7A260),
// parent and local bounds (resource +1Ch); release the mesh; +10h = +14h = 0.0f.
void* w3_water_spray_object_construct_00bae900(void* object, void* parent, const void* resource,
                                               NativeStringStorage& strings, W3_EngineCalls& calls);
// 00BB0E30 BSP_CoastMesh_Construct, __thiscall(ECX, ref* a, ref* b, ref* c) RET 0Ch -> EAX. No static
// reference (cc13-water1). Weak owner 00925490; vtable 00D640E4; +0Ch/+10h/+14h = *a/*b/*c with added
// references; +18h..+20h, +68h..+7Ch = 0 (the null +20h/+1Ch/+18h release-and-clear); the tuning DWORDs
// below, copied from .rdata; strings "foamwave.dds" +70h, "foamcoast.dds" +78h, "coast1.dat" +68h (resize
// WITHOUT preserve, copy exactly length bytes); +0D8h = +120h = 0.
// 00BA6C40 BSP_OceanWaveField_Construct, __thiscall(ECX, world, BYTE flat) RET 8 -> EAX: base 00BA1400
// (this, world); BYTE +0F9h = flat; vtables 00D63E10 / 00D63E08 (+4h); reflection targets (00B9DC20) with
// format 71h when BYTE 00F8D398 is set, else 15h, at the 00B9A3B0 size; 00BA5610; +498h = device
// +88h(400h, 400h, 1, 15h, 10h); camera "OceanShadowRenderCamera" (458h slot) at +49Ch, clear flags 7,
// clear colour 0, a new viewport (34h) sized {texture +3Ch width, +40h height} of +498h and released
// (no null test); depth = device +94h(width, height, 4Bh, 0, 0.0f, 1); frame targets +60Ch (new) with
// the +498h level-0 surface and that depth; then the existing +604h / +608h get the surfaces of
// +0E4h / +0E8h with the depth +288h. Each surface is released without a null test, the depth with one.
void* w3_ocean_wave_field_construct_00ba6c40(void* field, void* world, std::uint8_t flat,
    const volatile std::uint8_t& hdr_00f8d398, void* device_00f8d394, NativeStringStorage& strings,
    W3_EngineCalls& calls, void* (*base_construct_00ba1400)(void* field, void* world));
// 00BC2D50 BSP_Waterdrops_BuildGeometry, __fastcall(ECX waterdrops): a mesh (0BCh slot); declaration
// "simplecolor.mvfm" (device +38h); vertex buffer device +5Ch(4 * [+190h], 1000h, declaration) as stream 0;
// one draw section {+8h 4, rest 0} with material 00535320("waterdrops.mshd") textured from +1C8h (slot 0);
// the shared index stream 00AF10A0 of the particle resources object at 00F8C280; geometry on the model
// (0, -1.0f, -1.0f); bounds {0, 0, 0, 1.0e10f (00CE4970)} on the mesh (00B72650) and the model (00B74390).
// Declaration, buffer, material, section and mesh are released without null tests.
void w3_waterdrops_build_geometry_00bc2d50(void* waterdrops, void* particle_resources_00f8c280,
    void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls);
// 00BC54F0 BSP_CloudRenderModel_Construct, __thiscall(ECX slot, parent, const cloud class*) RET 8 -> EAX:
// cloud instance 00BBE7D0; +184h = class; vtable 00D64878; the shared image block at 01090904..010909AF:
// sixteen 1.0f DWORDs at 01090970..010909AC (listing order below), 01090908 = 0.5f, 01090904 = 3.0f
// (00CE3854); +1C8h = +1D8h = 1.0f; parent (00B6E680); a mesh (0BCh) with declaration "cloud.mvfm",
// vertex buffer +1A8h = device +5Ch(4 * class +20h, 1, declaration) as stream 0 (one reference
// dropped, pointer kept), index buffer device +60h(6 * class +20h, 1, 65h); a draw section {4, 0,
// 4n, 0, 2n} with a clone (00B18B60, 110h slot) of the class material +68h; geometry (0, -1.0f, -1.0f);
// bounds {0, 0, 0, class +2Ch through FLD/FSTP}; tail: BYTE +188h = 0, +198h..+1A0h = 0.0f,
// +18Ch..+194h = 99999.0f (00D119AC), BYTE +1A4h = BYTE +1B4h = 0.
void* w3_cloud_render_model_construct_00bc54f0(void* model, void* parent, void* cloud_class,
    std::uint32_t* image_01090904, void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls,
    W3_Body cloud_instance_construct_00bbe7d0, void* (*material_clone_construct_00b18b60)(void* slot, void* source));
// 00BC6060 BSP_CloudPlaneModel_Construct, __thiscall(ECX slot, owner, parent, const string* effect, const
// string* texture) RET 10h -> EAX: base 00BC7DB0; vtable 00D648E0; +184h = +188h = 0.0f; parent; visibility
// (0.0f, 0); material 00535320(effect) with texture device +64h(texture, 0) in slot 0 and parameter owner
// `owner`; a mesh with geometry (0, -1.0f, -1.0f) and bounds {0, 0, 0, 1.0e10f}; a 4-vertex buffer
// (declaration "cloud.mvfm", device +5Ch(4, 1, decl)) appended to a draw section {4, 0, 4, 0, 2} with the
// material; the buffer is filled with a unit quad {x, y, 0, FFFFFFFFh, 10x, 10y, u, v} (constants 1.0f,
// -1.0f, 10.0f 00CE38B8, -10.0f 00CE6848); a 6-index buffer device +60h(6, 1, 65h) with {0, 1, 2, 2, 3, 0}
// (Ghidra's body ends at 00BC63AB; the tail was read from the PE). Every object is released without a
// null test.
void* w3_cloud_plane_model_construct_00bc6060(void* model, void* owner, void* parent, const void* effect,
    const void* texture, void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls,
    W3_ConstructGeneratedModel construct_00b75030);
void* w3_coast_mesh_construct_00bb0e30(void* mesh, void* const* a, void* const* b, void* const* c,
                                       NativeStringStorage& strings, W3_Body weak_owner_construct_00925490);
void* w3_ocean_height_generator_construct_derived_00bb3ab0(void* generator, void* owner, std::int32_t width,
    std::int32_t height, const std::uint32_t* params, void* device_00f8d394, W3_EngineCalls& calls,
    void* (*construct_00ba94f0)(void* generator, void* owner, std::int32_t width, std::int32_t height,
                                const std::uint32_t* params));

// 00B9EA50 BSP_OceanWaveField_CreateWaveDampObject, __thiscall(ECX field, const float* a, const float* b,
// const string* texture) RET 0Ch -> EAX model. No static reference (cc13-water1 census). Builds the
// "WaveDampObject" model from "pf43uf42.mvfm" / "oceanwavedamp.mshd" (00B4C700 with 5, 4, 2, 0),
// registers it under [field +2BCh] +8h (the 00BA7170 getter), locks the vertex stream (vtable +10h with
// 0, 0, 0) and writes four 5-float vertices {x, 0, z, u, v} spanning a..b with u, v in {0, 1}
// (1.0f = 00D7A24C; MOVSS copies), unlocks (+14h), loads the texture through the device vtable +64h
// (name, 0), binds it to slot 0 of the first element's material (+20h) and releases it.
void* w3_ocean_wave_field_create_wave_damp_object_00b9ea50(void* field, const float* a, const float* b,
    const void* texture, void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls);

}  // namespace bsp::water_weather_cc13

// ================================================================================================
// Part g (cc13-left2, docs/CC13_LEFT_2.md): the bodies Part f left open. Read from the live
// decompile with the listing checked for register arguments, pushes and x87. Build-tested only.
// ================================================================================================
namespace bsp {
struct SingletonLifetimeCallbacks;
}

namespace bsp::water_weather_cc13 {

// The map<string, string> instantiation at 0048xxxx/0049xxxx (traffic config and the assertion record
// below) is a byte twin of the physical-index instantiation at 00BDxxxx: the bytes match except for
// rel32 call targets (each pairing to the twin's callee) and SEH handler addresses (local/l2_twin.py).
// Node 20h: links +0h/+4h/+8h, key string +0Ch, value string +14h, colour +1Ch, isnil +1Dh; the tree
// header is {allocator word +0h, head +4h, size +8h}. Each wrapper calls the reconstructed twin.
// 00485F50 STL_TrafficStringTree_Allocate: cdecl -> EAX node; twin 00BDABA0.
void* w4_string_tree_allocate_node_00485f50();
// 00480620 STL_TreeIter_Inc: __fastcall(ECX iterator {owner, node}); twin 00BD9860. A null owner or an
// increment of the head reports 00BF6713, which returns.
void w4_string_tree_iterator_increment_00480620(void* iterator, const SingletonLifetimeCallbacks& callbacks);
// 00494EF0 STL_Tree_Erase: __thiscall(ECX tree, root) RET 4; twin 00BDF7A0. Right recursion, then the
// pair (00489D50: value string, then key) and the node are released along the left spine.
void w4_string_tree_erase_subtree_00494ef0(void* tree, void* root, NativeStringStorage& strings) noexcept;
// 00494C30 (tagged STL_xlen_throw, actually erase(iterator)): __thiscall(ECX tree, out, owner, node) RET 0Ch
// -> EAX out; twin 00BDFD80.
void* w4_string_tree_erase_iterator_00494c30(void* tree, void* output, void* owner, void* node,
                                             NativeStringStorage& strings, const SingletonLifetimeCallbacks& callbacks);
// 0049A230 STL_Tree_EraseRange: __thiscall(ECX tree, out, first owner, first node, last owner, last node)
// RET 14h -> EAX out; twin 00BE0C30. [begin, end) erases the whole tree from the root and resets the head;
// any other range erases one node at a time through 00494C30.
void* w4_string_tree_erase_range_0049a230(void* tree, void* output, void* first_owner, void* first_node,
                                          void* last_owner, void* last_node, NativeStringStorage& strings,
                                          const SingletonLifetimeCallbacks& callbacks);

// Assertion record (2Ch), built by 00BD29C0 and consumed by 00BD2B30. 00BD26B0 (Part f) appends to its
// message. Nothing in the image calls 00BD29C0, 00BD2760 or 00BD2B30, and no data references them, so
// the record is dead machinery in this build; the layout is read from the bodies alone.
struct W4_AssertionRecord {
    std::uint32_t file_length;        // +0h  file string
    char* file_data;                  // +4h
    std::int32_t line;                // +8h  never written by 00BD29C0
    std::uint32_t expression_length;  // +0Ch expression string
    char* expression_data;            // +10h
    std::uint32_t tree_allocator;     // +14h map<string, string> of context values
    void* tree_head;                  // +18h
    std::uint32_t tree_size;          // +1Ch
    std::uint32_t message_length;     // +20h message string
    char* message_data;               // +24h
    std::int32_t level;               // +28h 0 WARNING, 1 DEBUG, 2 ERROR, 3 FATAL
};
static_assert(sizeof(W4_AssertionRecord) == 0x2C);

// 00BD29C0 BSP_AssertionRecord_Construct, __thiscall(ECX record, const char* expression) RET 4 -> EAX:
// file = {0, 0}; expression = 0041E870(expression); a new head node (00485F50) with isnil 1 and all three
// links to itself; size 0; message = {0, 0}; level 1.
W4_AssertionRecord* w4_assertion_record_construct_00bd29c0(W4_AssertionRecord* record, const char* expression,
                                                          NativeStringStorage& strings);
// 00BD2A50 BSP_AssertionRecord_Destroy, __fastcall(ECX record): release the message; erase [begin, end)
// (0049A230); free the head; head = size = 0; release the expression, then the file.
void w4_assertion_record_destroy_00bd2a50(W4_AssertionRecord* record, NativeStringStorage& strings,
                                          const SingletonLifetimeCallbacks& callbacks);
// 00BD23E0 BSP_AssertionRecord_FormatContext, __thiscall(ECX record, string* out) RET 4 -> EAX out: out =
// {0, 0}; for each entry in order "<" key "=" value "> " (the literals through pooled temporaries, the key
// and value appended directly when nonempty); then the message. Iterator checks as in the image: a null
// or foreign owner, and each of the two dereferences of the head, report 00BF6713.
void* w4_assertion_record_format_context_00bd23e0(const W4_AssertionRecord* record, void* out,
                                                  NativeStringStorage& strings,
                                                  const SingletonLifetimeCallbacks& callbacks);
// 00BD2760 BSP_AssertionRecord_FormatReport, __thiscall(ECX record, string* out) RET 4 -> EAX out: a log
// builder (004264A0) receives "<LEVEL>" (only for levels 0..3), " level assertion failed\n",
// "Expression: " expression "\n", "File: " file " (" line ")\n", the message, then "\t" key ": " value "\n"
// per entry (one owner check per pass and two head checks per dereference); out is copied from the
// builder's text (the 00BD2390 shape) and the builder destroyed (00425F80).
void* w4_assertion_record_format_report_00bd2760(const W4_AssertionRecord* record, void* out,
                                                 NativeStringStorage& strings,
                                                 const SingletonLifetimeCallbacks& callbacks,
                                                 const char* null_format_01090ab4);
// 00BD2B30 BSP_ErrorReporter_FailWithLocation, __fastcall(ECX record). When the handler 01090AB8 is set:
// text = 00BD23E0(record); handler(ECX &text) (a register argument: the listing loads ECX with the
// result and CALLs [01090AB8] with nothing pushed); release text. Otherwise: ")", "\", line: " and
// " (file: \"" (0041E870 in that order), a copy of the file (00BD2390), the context text (00BD23E0),
// then context + " (file: \"" + file + "\", line: " + line (004263B0) + ")" (004261A0), reported through
// the 0109DBDC singleton (00BD2180) and BSP_ErrorReporter_Fail 00BF1F50; the ten temporaries are
// released newest first. Both paths end by destroying the record (00BD2A50).
struct W4_FailCalls {
    // Loader zero-fill; its one writer is BSP_AutoTest_ParseTestName (004278C2). ECX text at the call.
    void (*handler_01090ab8)(const void* text);
    void* (*reporter_singleton_00bd2180)();
    void (*reporter_fail_00bf1f50)(void* reporter, const void* text);
};
void w4_error_reporter_fail_with_location_00bd2b30(W4_AssertionRecord* record, const W4_FailCalls& calls,
                                                   NativeStringStorage& strings,
                                                   const SingletonLifetimeCallbacks& callbacks);

}  // namespace bsp::water_weather_cc13

namespace bsp {
struct RandomState;
}

namespace bsp::water_weather_cc13 {

// Builders of Part g. Engine routines beyond W3_EngineCalls, with the native operands in order.
class W4_EngineCalls : public W3_EngineCalls {
public:
    // 00B86650 BSP_DrawSection_SetVertexLayout, __thiscall(ECX section, layout) RET 4.
    virtual void draw_section_set_vertex_layout_00b86650(void* section, void* layout) = 0;
    // 00B18AC0 BSP_Material_RegisterFloat4ArrayParameter, __thiscall(ECX material, const string* name,
    // float* values, count) RET 0Ch. 00B18AA0 BSP_Material_RegisterFloat4Parameter, (ECX, name, value) RET 8.
    virtual void material_register_float4_array_00b18ac0(void* material, const void* name, void* values,
                                                         std::uint32_t count) = 0;
    virtual void material_register_float4_00b18aa0(void* material, const void* name, void* value) = 0;
};
// Device slots used here beyond those listed in Part f: +40h layout* (const W4_DeclarationList*) and
// +48h effect* (const string* name). Vertex buffers: +10h lock (a, b, c) -> data, +14h unlock; the locked
// writer fields +8h data, +0Ch stride, +34h packed colour offset (negative when absent), +38h..+44h float
// colour offsets. Index buffers: +0Ch lock (0, 0, 0) -> WORD*, +10h unlock. Declarations: +0CCh stride.
struct W4_DeclarationList {
    void* declarations[4];  // only [0] is stored; the image leaves [1..3] as stack garbage
    std::uint32_t count;    // 1
};

// 00BAE2E0 BSP_WaterSprayResource_Construct, __thiscall(ECX resource, const definition*) RET 4 -> EAX.
// Caller 0086D180. The definition holds six {data, count, capacity} arrays: +0h and +0Ch Vec2, +18h, +24h
// and +30h floats, +3Ch DWORD counts, the last one's count (+40h) bounding every loop. Ref-counted base,
// vtable 00D64094, +8h = +0Ch = 0; +14h = sum 4c (vertices) and +18h = sum 2c (triangles), with the
// total sum T kept. Declaration "waterspray.mvfm"; vertex buffer device +5Ch(+14h, 1, decl); the
// declaration released (no null test). The buffer is locked (+10h with 0, 0, 0) and filled with four 30h
// vertices per sub-element j of each element i: {P.x, 0, P.y, s, t, Q.x, Q.y, A, B, 1.0f, r, j/c} where
// P = +0h[i], Q = +0Ch[i], C = +18h[i], A = +24h[i], B = +30h[i], (s, t) = (-C, C), (-C, -C), (C, -C),
// (C, C), r = one draw per sub-element of BSP_Random_UniformFloatRange 00BD2F10 on stream 0 (ECX 0 at
// 00BAE4BD) over [-2pi, 2pi] (00D640C8 / 00CE3D9C), and j/c an x87 quotient of the unsigned j and c stored
// as a float. Unlock (+14h); index buffer device +60h(6T, 1, 65h) filled {4n, 4n+1, 4n+2, 4n+2, 4n+3, 4n};
// a mesh (0BCh slot) at +8h with vertex stream 0 and that index stream, each released after (no null
// test); +0Ch = device +48h("waterspray.mshd"); +1Ch..+24h = 0.0f, +28h = 1.0e10f (00CE4970); +10h =
// device +64h("p_bubbles_0002.dds", 0). The draw is routed through `random` (cc9 RNG rule): the image
// shares the gameplay stream here, the source does not.
struct W4_PresentationRandom {
    void* context;
    float (*uniform_range_00bd2f10)(void* context, float low, float high);
};
void* w4_water_spray_resource_construct_00bae2e0(void* resource, const void* definition, void* device_00f8d394,
                                                 NativeStringStorage& strings, W4_EngineCalls& calls,
                                                 const W4_PresentationRandom& random);

// 00BA3BD0 BSP_OceanBeams_Initialize, __fastcall(ECX beams). Nothing when N (+2Ch) is 0. +0F8h = 6N,
// +0F4h = 4N; texture device +64h(&+34h, 0) in slot 0 of material 00535320("beam.mshd") (texture released
// when non-null); declaration "beam.mvfm"; +28h = device +40h({decl, count 1}); +24h = vertex buffer
// device +5Ch(+0F4h, 1, decl) (old value released and cleared); declaration released; +20h = index buffer
// device +60h(+0F8h, 1, 65h) (likewise); +1Ch = a draw section with the material (material released),
// {+8h 4, +0Ch 0, +10h +0F4h, +14h 0, +18h 2N}; +14h = model "UnderwaterBeam" (184h slot), its root from
// the owner (+0Ch, its +8h when non-null) and +138h &= ~3; indices {4n, 4n+1, 4n+2, 4n+1, 4n+3, 4n+2};
// +18h = mesh with vertex stream 0 = +24h, index stream +20h; the section gets vertex stream +24h, layout
// +28h, is rebuilt for the mesh (00B865A0) and appended; geometry (0, mesh, -1.0f, -1.0f). Beam records:
// +0E0h = N, +0DCh = 0, +0E4h = operator new(N*14h), +0E8h = operator new(N*4) (each size saturated to
// FFFFFFFFh on overflow) of pointers to the records, each record's +0Ch = +10h = -1.0f. Then +0DCh = N,
// +0CCh = +7Ch, +0D0h = +80h, +0D4h / +0D8h = their x87 reciprocals when positive (0 otherwise), +50h
// raised to 0.01f (00D7A238) when negative, +0F0h = +0ECh = -1.0f.
void w4_ocean_beams_initialize_00ba3bd0(void* beams, void* device_00f8d394, NativeStringStorage& strings,
                                        W4_EngineCalls& calls);

// 00BA44F0 BSP_OceanWaterParticles_Initialize, __fastcall(ECX particles). Nothing when N (+20h) is 0.
// +0C8h = 6N, +0C4h = 4N; texture device +64h(&+28h, 0); material 00535320("WaterParticle.mshd") with
// float4 arrays (count 1) "cVolumeSize" +0CCh, "cVolumeCorner" +0DCh, "cTex1" +0ECh, "cTex2" +0FCh,
// "cMyTime" +10Ch and float4 "cColor" +30h; parameter owner (particles, 0); slot 0 = texture, slot 1 =
// null; texture released (no null test). Declaration "WaterParticle.mvfm"; +1Ch = device +40h layout;
// +18h = vertex buffer device +5Ch(+0C4h, 1, decl) (old released and cleared), zeroed through its lock
// for decl +0CCh * +0C4h bytes and unlocked; declaration released; +14h = index buffer (6N, 1, 65h);
// a draw section {4, 0, +0C4h, 0, 2N} with the material; +10h = model "WaterParticles" with its root;
// indices {4n, 4n+1, 4n+2, 4n+1, 4n+3, 4n+2}; a mesh with vertex stream 0 = +18h and index stream +14h; the
// section gets vertex stream +18h and layout +1Ch and is appended (no rebuild); geometry (0, mesh, -1.0f,
// -1.0f). Then the vertex buffer is locked again and every particle i draws three bytes from the module's
// own Mersenne state 00E13A88 (seeded 4357 by 00CD88B0; random_next_u32_00ba2c20 inlined, guard first):
// b = trunc(float(v * 2^-32) * 255.0) (x87, chop). Each of its four vertices k gets the colour b2 | b1 << 8
// | b0 << 16 | (k << 6) << 24 at the packed offset (+34h) when it is not negative, else four floats
// b0/255, b1/255, b2/255, ((k << 6) & 0FFh)/255 at +38h..+44h (the buffer object is re-read per vertex).
void w4_ocean_water_particles_initialize_00ba44f0(void* particles, RandomState& state_00e13a88,
                                                  void* device_00f8d394, NativeStringStorage& strings,
                                                  W4_EngineCalls& calls);

}  // namespace bsp::water_weather_cc13

namespace bsp::water_weather_cc13 {

// CRT routines the tracer builders call through x87 ST0, bound by the caller. 00BFA910 is _CIlog (its x87
// path 00BFA968 is FLDLN2/FYL2X); 00BFEEEC is the second SSE2-dispatching _CI routine, applied to k*log(x)
// so taken as _CIexp (its x87 path goes through the generic dispatcher 00C080F1; not confirmed from its
// body); 00BF85B0 is the routine other reconstructions bind as floor.
struct W4_CrtMath {
    double (*log_00bfa910)(double);
    double (*exp_00bfeeec)(double);
    double (*floor_00bf85b0)(double);
};
// 00BAA0F0 BSP_SkinedWaterTracer_BuildWidthProfile, __thiscall(ECX resource, float* out, s, w, product, side)
// RET 14h: writes one 40h vertex (its semantic model is tracer_build_width_profile_00baa0f0 above).
using W4_TracerVertexWriter = void (*)(void* resource, float* out, float s, float w, float product, float side);

// 00BAD150 BSP_WaterTracerResource_BuildSkinnedMesh, __thiscall(ECX resource, N, float k, BYTE skip_curves,
// counts*, curve_b**, curve_a**) RET 18h. The one caller (00BADC70) passes skip_curves = 1.
// +30h += N; declaration "watertracerskined.mvfm"; vertex buffer device +5Ch(+30h, 1, decl), declaration
// released (no null test), locked (+10h with 0, 0, 0). step = float(1 / (N - 1)) (signed FILD); t starts at
// 0. Ring i < N: e = k > 1.0 ? float(exp(float(float(log(t)) * k))) : t; m = max(float(resource +10h - 1,
// unsigned), 1.0f); w = float(-e * m). Vertex j < counts[i] (re-read): s = float((j / (c - 1) - 0.5) * 2)
// (unsigned j, signed FIDIV); product = 0, or, when skip_curves is 0, float(curve_a(e) * curve_b(s > 0 ?
// s : -0.0f - s)) through each curve's vtable +8h (ECX curve, float) -> ST0; then 00BAA0F0 writes one
// vertex with side -1.0f and one with 1.0f when s == 0 (UCOMISS, unordered counts as unequal), else one
// with side 0 > s ? -1 : 1. t = float(t + step); T += c_i + c_(i+1) - 2 for i < N - 1. Index buffer device
// +60h(3T, 1, 65h): strips between rings r and r+1 from (a, b, a+1) with a = sum(c + 1) of the rings
// before and b = a + c_r + 1, each next triangle (y, z, y+1) except at mid = (c_r + c_(r+1) - 2) / 2 - 1
// (signed), where it is (y+1, z+1, y+2). For each tracer point q < +10h: the leading triangles whose
// three vertices all have +10h, +14h, +18h <= q (FCOMI against float(q)) are counted into +24h[q], and
// +18h[q] = 1 + the highest index read (including the failing vertex), or 0 when none counted. Unlock
// both; +0Ch = mesh (0BCh slot) with vertex stream 0 and the index stream, each released after.
void w4_water_tracer_build_skinned_mesh_00bad150(void* resource, std::uint32_t rings, float k,
    std::uint8_t skip_curves, const W2_Array* counts, void* const* curve_b, void* const* curve_a,
    void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls, const W4_CrtMath& crt,
    W4_TracerVertexWriter write_vertex_00baa0f0);

// 00BADC70 BSP_WaterTracerResource_Initialize, __thiscall(ECX resource, 44h-byte parameter block by value)
// RET 44h -> EAX. Caller BSP_EffectWaterTracer_ReadLua 0086D180. The block, by stack offset: +0h points P,
// +8h BYTE static, +0Ch the address of the Width curve holder (component + 68h; kept at +48h), +10h / +14h
// the HeightX / HeightY curve holders (component + 6Ch / + 70h), +1Ch float k_mesh, +20h base
// count n0, +24h float k_rings, +28h..+40h seven floats (+34h kept at +8h, +38h at +4Ch). How 0086D180
// fills each word is in water_tracer_read_lua_0086d180 (gameplay_seg53_cc13.hpp). +4h and +18h
// are not read. Ref-counted base, vtable 00D63F94; +14h = 0; vectors +18h and +24h zeroed then resized to
// P (00468050, bound) and cleared; effect +14h = device +48h("watertracerstatic.mshd" or
// "watertracerskined.mshd"); +10h = P; R = 2P - 2 ring counts into a local vector: step = 1 / (R - 1) kept
// as a double (unsigned FILD); x = float(i * step); count = n0 when k_rings == 0 (UCOMISS), else M =
// max(float((1 - float(exp(float(float(log(x)) * k_rings)))) * n0), 3.0f), F = float(floor(M)), rounded
// up by 1.0 when |float(M - F)| > 0.5, then _ftol2; +30h sums the counts. Static: 00BACBD0(resource, R,
// k_mesh, 1, &counts, +0Ch, +10h, +14h, the seven floats); skinned: 00BAD150(resource, R, k_mesh, 1,
// &counts, +10h, +14h). Then a draw section (60h, 00B85EE0 / 00B857F0; dereferenced without a null test)
// {+8h 5, +0Ch..+18h 0} rebuilt for the mesh +0Ch, appended to it and released; the local vector freed
// (its inlined shrink branches are dead for a non-negative capacity).
using W4_TracerStaticBuilder = void (*)(void* resource, std::uint32_t rings, float k, std::uint32_t one,
    W2_Array* counts, std::uint32_t a, std::uint32_t b, std::uint32_t c, float f0, float f1, float f2, float f3,
    float f4, float f5, float f6);
struct W4_TracerCalls {
    W2_Resize resize_00468050;
    W4_TracerStaticBuilder build_static_mesh_00bacbd0;
    void (*build_skinned_mesh_00bad150)(void* resource, std::uint32_t rings, float k, std::uint8_t skip_curves,
                                        const W2_Array* counts, void* const* curve_b, void* const* curve_a);
    void* (*draw_section_allocate_00b85ee0)(std::uint32_t size);  // ECX size
    void* (*draw_section_construct_00b857f0)(void* slot);
};
void* w4_water_tracer_resource_initialize_00badc70(void* resource, const std::uint32_t* parameters,
    void* device_00f8d394, NativeStringStorage& strings, W3_EngineCalls& calls, const W4_CrtMath& crt,
    const W4_TracerCalls& tracer);

}  // namespace bsp::water_weather_cc13

namespace bsp::water_weather_cc13 {

// 00BA1400 BSP_OceanWaveFieldBase_Construct, __thiscall(ECX field, world) RET 4 -> EAX. Caller 00BA6C40.
// Listing read in full (temporaries tracked by stack slot). Engine routines beyond W4_EngineCalls:
struct W4_WaveFieldBaseCalls {
    W3_Body weak_owner_construct_00925490;                                        // ECX field +4h
    std::uint8_t (*check_vertex_texture_format_00b20190)(void* device, std::uint32_t format);  // RET 4 -> AL
    void* (*scene_construct_00b724e0)(void* block, const void* name);              // 24h block, RET 4
    void (*create_floating_particles_00af0f20)(void* manager);                     // ECX [00F8C274]
    void* (*fog_owner_construct_00b84e50)(void* block);                            // 94h block
    void (*camera_set_fog_owner_00b71940)(void* camera, void* fog);                // RET 4
    void* (*scene_resource_construct_00b83c50)(void* block, const void* name);     // 3Ch block, RET 4
    void (*scene_set_resource_00b723f0)(void* scene, void* resource);              // RET 4
    void* (*scene_lighting_owner_00b72110)(void* scene);
    void* (*directional_light_allocate_00b7bd40)(std::uint32_t size);              // ECX 1ECh
    void* (*directional_light_construct_00b7c6b0)(void* slot, const void* name);   // RET 4
    void* (*light_shadow_owner_00b7aab0)(void* light);
    void (*light_set_shadow_owner_00b7bdf0)(void* light, void* owner);             // RET 4
    void (*material_register_matrix_00b18b40)(void* material, const void* name, void* value);  // RET 8
    void (*invalid_iterator_00bf6713)();
};
// Fields, in listing order: vtables 00D63928, weak owner (+4h), 00D63B5C / 00D63B54 (+4h); +14h = 0, BYTE +84h
// = 0, +0A0h..+0ACh, +0BCh..+0C8h, BYTE +0CCh, +0D0h, +0E4h, +0E8h, +0F0h, +0F4h = 0, BYTE +0F8h = 1, +114h =
// 20.0f, +118h = 0.5f, +11Ch..+124h, +12Ch, +130h, +27Ch..+288h, +2ACh..+2BCh, +498h..+4A0h, +4E4h, +4E8h,
// +4F0h, +4F8h..+504h, +5F4h..+604h, +60Ch..+628h = 0; BYTE +10h = 00B20190(device, 71h); +66Ch = 5000.0f,
// +670h..+678h = 0; +0D4h = 0.5f, +20h = 1.0f, +0D8h = 0.05f, +34h..+40h = 1.0f, +0E0h = +0DCh = 0.
// +14h = scene "FloatingObjectScene"; floating particles (00AF0F20 on [00F8C274]) rooted under it. Cameras
// "WaterReflection" (+0C8h, frame targets +604h) and "WaterRefraction" (+0D0h, +608h): clear flags 7, a
// viewport of 200h x 200h and a system fog owner, each released (no null test). +0B0h = 300.0f, +0B4h =
// 0.0033333334f, +0B8h = 1.4f, +24h = 0.0f; world into +0C0h (reference swap); +278h = +274h = 0; model
// "OceanSurface" (+11Ch) rooted under the world; +28Ch = +294h = +29Ch = +2A4h = 0, +290h = +298h = +2A0h =
// 1.0f, +2A8h = 3.0f. +4A0h = 00B4C700("ShadowRender", "position.mvfm", "oceanshadow_vsampler.mshd" when
// BYTE +10h else "oceanshadow.mshd", 4, 3, 1, 0), rooted under +14h, with "cStaticShot_Size_OffsetX_OffsetY"
// (float4 array +66Ch, 1) and "cCameraViewMat" (matrix +67Ch) on its first element's material, and its
// vertex stream 0 written with the triangle {0, 0, 50000; 50000, 0, -50000; -50000, 0, -50000}. Scene
// resource "AllLights" (3Ch) set on +14h and released when non-null. +600h = directional light "TheSun"
// (1ECh pool slot) with the shadow owner of the first light of the world's lighting owner (list +1Ch; an
// empty list reports 00BF6713); +600h and +4A0h vtable +50h(lighting owner of +14h, 1). +120h = "OceanFoam"
// ("position.mvfm", "oceanfoam.mshd", 4, 3, 1, 0) with the same triangle; +504h = "Splashes"
// ("pf43nf43uf44.mvfm", "oceansplash.mshd", 4, 0, 0, 0); +4E4h = scene "EmptyReflection"; +104h..+110h =
// -1.0e10f; "OceanVelAndDepth" ("position.mvfm", "ocean_vel_and_depth.mshd", 4, 3, 1, 0) is rooted under the
// world and given the triangle but not stored.
void* w4_ocean_wave_field_base_construct_00ba1400(void* field, void* world, void* device_00f8d394,
    void* floating_particles_00f8c274, NativeStringStorage& strings, W4_EngineCalls& calls,
    const W4_WaveFieldBaseCalls& base);

}  // namespace bsp::water_weather_cc13

namespace bsp::water_weather_cc13 {

// 00BA7530 BSP_OceanHeightGenerator_InitializePasses, __thiscall(ECX generator, width, height) RET 8. Caller
// 00BA94F0. Listing read in full; the normal pass's x87 vertex block traced symbolically. s = the global
// 00E14454 (read at each use). Each pass draws one full-screen quad of four vertices at (-1, -1), (-1, 1),
// (1, -1), (1, 1), z 0.5, w 1, uv (0, 1), (0, 0), (1, 1), (1, 0), built through a declaration, a vertex
// buffer device +5Ch(4, 1, decl) (declaration released, no null test), a mesh (0BCh) with vertex stream 0
// (buffer released), a material with parameter owner (generator, 0), a draw section {5, 0, 4, 0, 2} with
// that material, rebuilt for and appended to the mesh, and a model with geometry (0, mesh, -1.0f, -1.0f)
// rooted under the pass scene; section and mesh released (no null tests).
// 1. scene "oceanheightmapscene" +30h; camera "OceanHeigtmapGen" +34h, clear flags 0, 00B6FDF0(0), viewport
//    {s*w, s*h}; "oceanheightmapgen.mvfm" with 6-float vertices {x, y, z, w, u, v}; material
//    "oceanheightmapgen.mshd" +3Ch with float4 arrays (count 1) cCorner0..3 = +0C0h, +0D0h, +0E0h, +0F0h,
//    float cLayerScale +9Ch (00B18B20), float3 cWaveScale +100h and cEyePos +10Ch (00B18AE0); model
//    "OceanHeightmapgen" +38h under +30h.
// 2. scene "oceannormalgenscene" +1Ch; camera "OceanNormalgenCamera" +24h, clear flags 7, viewport
//    {s*w, s*h}; "oceannormalmapgen.mvfm" with 14-float vertices {x, y, z, w, u, v, u-dx, v, u+dx, v, u,
//    v-dy, u, v+dy}, dx = float(1 / (s*w)), dy = float(1 / (s*h)) (unsigned FILD), kept at +0B8h / +0BCh;
//    material "oceannormalgen.mshd" +28h with cCorner0..3 from the owner (+88h, read once) +0h..+30h and
//    float2 cInvMapSize +0B8h (00B18B00); model "OceanNormalGen" +20h under +1Ch.
// 3. scene "oceanHeightScalescene" +4Ch; camera "OceanHeightScaleCam" +54h, clear flags 0, viewport {w, h};
//    the pass-1 declaration and quad; material "oceanheightscale.mshd" +58h (no constants); model
//    "OceanHeightScale" +50h under +4Ch.
// 4. scene "wave_damp_scene" +8h; camera "wave_damp_camera" +0Ch with clear colour FFFFFFFFh (00B6FE50),
//    clear flags 1 and a viewport {80h, 80h}; frame targets (40h) +18h with colour surface 0 = +14h.
// Viewports are released when non-null. Finally +80h = 0, +84h = 1.
struct W4_HeightGeneratorCalls {
    void* (*scene_construct_00b724e0)(void* block, const void* name);
    void (*camera_set_render_mode_00b6fdf0)(void* camera, std::uint32_t mode);       // RET 4
    void (*material_register_float_00b18b20)(void* material, const void* name, void* value);  // RET 8
    void (*material_register_float3_00b18ae0)(void* material, const void* name, void* value);  // RET 8
    void (*material_register_float2_00b18b00)(void* material, const void* name, void* value);  // RET 8
};
void w4_ocean_height_generator_initialize_passes_00ba7530(void* generator, std::int32_t width, std::int32_t height,
    const volatile std::int32_t& scale_00e14454, void* device_00f8d394, NativeStringStorage& strings,
    W4_EngineCalls& calls, const W4_HeightGeneratorCalls& passes);

}  // namespace bsp::water_weather_cc13
