#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "bsp/scene_graph2_cc13.hpp"

// Partition segments 88/89 (00B6D3C0..00B80ED0, 00B80FD0..00B86E10) reconstructions by
// cc13-files1. Evidence, original ABIs and uncertainty: docs/CC13_SCENE_GRAPH_NODES.md.
// New C++ interfaces over host data; the native routines use custom register ABIs
// (XMM0 float returns, EAX/ECX/EDX/ESI/EDI pointer inputs) that are not reproduced.

namespace bsp::cc13_scene {

// 28h-byte animation key.
struct AnimKey {
    std::int32_t type{};   // +00: interpolation of the segment that ENDS at this key
    float value{};         // +04
    float time{};          // +08
    float tension{};       // +0C  (TCB, type 0)
    float continuity{};    // +10
    float bias{};          // +14
    float in_a{};          // +18: type 1/2 in tangent; type 5 in-handle dt
    float in_b{};          // +1C: type 1/2 out tangent; type 5 in-handle dv
    float out_a{};         // +20: type 5 out-handle dt
    float out_b{};         // +24: type 5 out-handle dv
};

// Out-of-range behaviour at track +14h (before the first key) and +18h (after the last).
enum class OutOfRange : std::int32_t { zero = 0, hold = 1, cycle = 2, ping_pong = 3, relative_cycle = 4, linear = 5 };

// Track fields used by the sampler: +14h, +18h, keys +1Ch, count +20h.
struct AnimTrack {
    OutOfRange before{};
    OutOfRange after{};
    std::vector<AnimKey> keys;
};

// 00B75450: wrap `time` into the key range. Returns start + time - floor(time/len)*len
// (len = end - start; start when len == 0). When `cycles` is given it receives
// -trunc((wrapped - time)/len -/+ 0.5).
float wrap_time_00b75450(float time, float start, float end, std::int32_t* cycles);

// 00B75520: cubic Hermite basis for s; outputs are rounded to float like the native stores.
struct HermiteBasis { float p0, p1, t0, t1; };
HermiteBasis hermite_basis_00b75520(float s);

// 00B75820: tangent leaving `key` toward `next` (`prev` may be null).
float outgoing_tangent_00b75820(const AnimKey& key, const AnimKey* prev, const AnimKey& next);
// 00B75A20: tangent arriving at `key` from `before`; `after` may be null.
float incoming_tangent_00b75a20(const AnimKey& key, const AnimKey* after, const AnimKey& before);

// 00B75C10: index i with keys[i].time <= time < keys[i+1].time (interpolation probe plus
// bisection); 0 for fewer than three keys.
std::int32_t find_key_00b75c10(const AnimKey* keys, std::int32_t count, float time);

// 00B75580: cubic Bezier in power form for control values p0..p3 at s, rounded to float.
double bezier_00b75580(float p0, float p1, float p2, float p3, float s);
// 00B75610: bisect s in [lo, hi] until |target - bezier(s)| <= 1e-4. The native loop has no
// iteration limit; this version stops after `max_iterations` (documented deviation).
float bezier_solve_00b75610(float& lo, float& hi, float p0, float p1, float p2, float p3,
    float target, int max_iterations = 200);
// 00B75720: type-5 segment from `key` to `next` at absolute time.
float bezier_segment_00b75720(const AnimKey& key, const AnimKey& next, float time);

// 00B76CE0: sample a track at time.
float sample_track_00b76ce0(const AnimTrack& track, float time);

// 00B77220: nine optional channels (tx,ty,tz, rx,ry,rz, sx,sy,sz); a missing track leaves
// the output component unchanged.
// The track array (+10h) has at least 11 entries: node animation reads channel 10 and the camera
// channel 9 through 00B77430.
struct ChannelGroup { std::vector<const AnimTrack*> channels; };
void sample_channel_group_00b77220(const ChannelGroup& group, float position[3], float rotation[3],
    float scale[3], float time);

// 00B77360 / 00B77430 (ordinary animator vtable 00D62E60 slots +18h/+1Ch). The animator
// supplies translation +08h..+10h, angles +14h..+1Ch and groups +20h/+24h.
struct NodeAnimatorView {
    float translation[3]{};
    float angles[3]{};
    std::vector<const ChannelGroup*> groups;
};
// Fills position/rotation/scale for the matrix the native slot composes through
// 00B64CE0 (scale defaults to 1). Returns false for a missing group.
bool evaluate_animator_pose_00b77360(const NodeAnimatorView& animator, std::uint32_t group,
    float time, float position[3], float rotation[3], float scale[3]);
// Returns false for a missing group or channel; otherwise samples one channel.
bool evaluate_animator_channel_00b77430(const NodeAnimatorView& animator, std::uint32_t group,
    std::uint32_t channel, float time, float& out);


// ---------------------------------------------------------------------------
// Scene-graph node queries. A host node view: name (+54h) and children in sibling order
// (+34h first child, +3Ch next sibling).
struct SceneNodeView {
    std::string name;
    std::vector<SceneNodeView*> children;
};
// 00B6E750: direct children first (names compared case-insensitively), then, when
// recursive, each child's subtree in order.
SceneNodeView* find_child_by_name_00b6e750(SceneNodeView& node, const std::string& name, bool recursive);
// 00B6F9A0: post-order collection of descendants whose name contains `filter` (all when empty).
void collect_descendants_matching_00b6f9a0(SceneNodeView& node, const std::string& filter,
    std::vector<SceneNodeView*>& out);
// 00B6E150 visibility rule: node +ACh = clamp(1 - channel 10 sample, 0, 1); NaN stays NaN.
float node_visibility_from_channel_00b6e150(float sampled) noexcept;
// 00B70DE0: camera +1C4h = 2*atan(1/channel 9 sample) (00BF8490 is CRT atan; FPATAN fallback at 00BF84FF).
float camera_fov_from_channel_00b70de0(float sampled) noexcept;

// ---------------------------------------------------------------------------
// 00B811C0 resource dump table (the CSV save 00BF0A30 is a stub in this build).
struct ResourceDumpEntry { std::string name; std::int32_t size{}; std::int32_t refcount{}; };
// Returns the rows: header {Name, Size, RefCounter, "Size / RefCounter"}, then entries by
// size descending (qsort, so equal sizes have no defined order); size/refcount is integer
// division, which the native code performs even for a zero refcount (here: column left empty).
std::vector<std::vector<std::string>> resource_dump_rows_00b811c0(std::vector<ResourceDumpEntry> entries);
// The file name the native routine passes to the stub.
std::string resource_dump_file_name_00b811c0(const std::string& suffix);

// ---------------------------------------------------------------------------
// cc13-scene3 (docs/CC13_SCENE_3.md): the two large bodies left by docs/CC13_SCENE_2.md, read from
// the listing (capstone over the PE). Build-tested only; not wired into a host.

// 00B708E0 BSP_Camera_RenderHiresTiles, __thiscall(ECX camera, const char* file, int width, int
// height, callback, context) RET 14h -> AL. Camera fields: +180h viewport, +1E0h projection
// (16 floats), +2A0h its published copy, +2F0h flags.
struct HiresCamera {
    float projection_1e0[16]{};
    float projection_copy_2a0[16]{};
    std::uint32_t flags_2f0 = 0;
};
// A tile rectangle in the target surface (left, top, right, bottom).
struct HiresTileRect { std::int32_t left{}, top{}, right{}, bottom{}; };
// The per-tile scene callback: called with ECX = context and no stack arguments; AL is kept.
using HiresTileCallback = std::uint8_t(__fastcall*)(void* context);
struct HiresTileServices {
    // Renderer 00F8D394 virtuals (ECX renderer).
    virtual bool in_scene_2c() = 0;
    virtual void end_scene_14() = 0;
    virtual void begin_scene_0c() = 0;
    // vtable +84h (width, height, 16h, 2): the full-size target surface.
    virtual void* create_surface_84(std::int32_t width, std::int32_t height, std::uint32_t format,
                                    std::uint32_t usage) = 0;
    // vtable +7Ch (surface holder*) -> AL: fill the stack surface holder with the back buffer.
    virtual bool get_back_buffer_7c(void* holder) = 0;
    // 00B1F740 on the camera's viewport (+180h): {width, height}; called twice, as in the listing.
    virtual const std::int32_t* viewport_size_00b1f740() = 0;
    // The stack surface holder (3Ch bytes at frame +90h): 00B3F450 constructs, 00B3F4E0 destroys.
    virtual void holder_construct_00b3f450(void* holder) = 0;
    virtual void holder_destroy_00b3f4e0(void* holder) = 0;
    // Target surface virtuals (ECX surface): +38h (holder*, 0, rect*) copies the back buffer into
    // the rectangle; +34h (file, 0) saves; +4h (1) deletes.
    virtual void surface_copy_from_38(void* surface, void* holder, std::uint32_t zero, const HiresTileRect& rect) = 0;
    virtual void surface_save_34(void* surface, const char* file, std::uint32_t zero) = 0;
    virtual void surface_delete_04(void* surface, std::uint32_t flags) = 0;
protected:
    ~HiresTileServices() = default;
};
// Tiles: tx = width / vw and ty = height / vh (signed IDIV by the viewport size). The target is
// created at (tx*vw, ty*vh). The projection is saved, then P[0] *= tx and P[5] *= ty (FILD x float,
// rounded to float). Columns j < tx outer, rows i < ty inner: P[8] = f(f(tx-1) - 2j),
// P[9] = f(-(f(ty-1) - 2i)) (f(n-1) by CVTSI2SS); the 16 entries are reloaded and stored in place
// (FLD/FSTP: only a signalling NaN changes, to quiet) and copied to +2A0h; flags = (flags &
// FFFFFF4Bh) | 8. Then BeginScene (+0Ch), callback(context) -> AL kept, EndScene (+14h); a stack
// holder gets the back buffer (+7Ch) and, when that succeeds, it is copied into {x, y, x+vw, y+vh}
// (x advances by vw per column, y by vh per row). Afterwards the saved projection is restored to
// +1E0h and +2A0h (MOVSS, bit-exact), the flags updated again, the target saved to `file`
// (+34h, 0) and deleted (+4h, 1); BeginScene runs again when the renderer was in a scene at entry
// (EndScene was called then). Returns the last callback AL (0 when there are no tiles).
std::uint8_t camera_render_hires_tiles_00b708e0(HiresCamera& camera, const char* file, std::int32_t width,
    std::int32_t height, HiresTileCallback callback, void* context, HiresTileServices& s);

// 00B8DD60 BSP_Flare_BuildGeometry, __thiscall(ECX flare, const element vector*) RET 4.
// Layout of a draw section's ranges (+8h..+18h).
struct FlareDrawRanges {
    std::uint32_t primitive_type_08 = 4;
    std::uint32_t first_vertex_0c = 0;
    std::uint32_t vertex_count_10 = 0;
    std::uint32_t first_index_14 = 0;
    std::uint32_t primitive_count_18 = 0;
};
struct FlareGeometryServices {
    // 00B8D850 (ECX flare+18Ch, stack source): assign the element pointer vector.
    virtual void assign_elements_00b8d850(cc13_scene2::FlareView& flare, const void* source) = 0;
    // 00B73B60 (ECX 0BCh) allocation and 00B73D70 construction of the mesh object; null when the
    // allocation returns null.
    virtual void* new_mesh_object_00b73b60() = 0;
    // Renderer 00F8D394 virtuals.
    virtual void* create_vertex_declaration_38(const char* name) = 0;  // ref-counted
    virtual void* create_vertex_buffer_5c(std::uint32_t vertex_count, std::uint32_t usage, void* declaration) = 0;
    // +40h on a stack record {declaration, three words the frame left there, 1}.
    virtual std::uint32_t create_vertex_layout_40(void* declaration) = 0;
    virtual void* create_index_buffer_60(std::uint32_t index_count, std::uint32_t usage, std::uint32_t format) = 0;
    virtual std::uint16_t* lock_indices_0c(void* buffer) = 0;  // buffer vtable +0Ch (0, 0, 0)
    virtual void unlock_indices_10(void* buffer) = 0;          // buffer vtable +10h
    // Intrusive release (InterlockedDecrement of +4h, vtable slot 0 at zero).
    virtual void release(void* object) = 0;
    virtual void geometry_set_vertex_stream_00b73bb0(void* geometry, std::uint32_t slot, void* buffer) = 0;
    virtual void geometry_set_index_stream_00b73b70(void* geometry, void* buffer) = 0;
    // 00AEFB20 on the atlas 00F8C26C; the item's texture is its +8h word.
    virtual const cc13_scene2::FlareAtlasRect* atlas_find_item_00aefb20(const char* name) = 0;
    virtual void* atlas_item_texture(const cc13_scene2::FlareAtlasRect* item) = 0;
    virtual void* material_create_00535320(const char* effect_name) = 0;  // ref-counted
    virtual void material_set_texture_slot_00b189f0(void* material, std::uint32_t slot, void* texture) = 0;
    // 00B85EE0 (ECX 60h) and 00B857F0: a draw section, null when the slot allocation fails.
    virtual void* new_draw_section_00b85ee0() = 0;
    virtual void section_write_ranges(void* section, const FlareDrawRanges& ranges) = 0;
    virtual void section_set_material_00b864c0(void* section, void* material) = 0;
    virtual void* geometry_vertex_stream_00b73260(void* geometry, std::uint32_t slot) = 0;
    virtual void section_append_vertex_stream_00b85b80(void* section, void* stream) = 0;
    virtual void section_set_vertex_layout_00b86650(void* section, std::uint32_t layout) = 0;
    virtual void geometry_append_section_00b73c60(void* geometry, void* section) = 0;
    // 00B75170 (ECX flare, 0, geometry, -1.0f, -1.0f).
    virtual void set_geometry_00b75170(cc13_scene2::FlareView& flare, std::uint32_t slot, void* geometry,
                                       float a, float b) = 0;
    virtual void initialize_occlusion_00b8dac0(cc13_scene2::FlareView& flare) = 0;
protected:
    ~FlareGeometryServices() = default;
};
// Assign the elements (00B8D850); build the mesh object; vertex declaration "simplecolor2.mvfm";
// vertex buffer of 4n vertices (usage 1), declaration released; flare +1CCh = layout; vertex stream 0;
// index buffer of 6n (usage 1, format 65h) locked: element i gets 4i, 4i+1, 4i+2, 4i+2, 4i+3, 4i; per
// element both atlas names (+48h/+50h) resolve to rects[k] (an empty name gives null) and size
// (+34h/+38h) = origin (+8h/+0Ch) by FLD/FSTP. Unlock, index stream, release. Materials "flare.mshd"
// (M0) and "flare2.mshd" (M1). Per element: type 0 and type 1 count n0/n1; the first of either
// (while both counts are 0) sets M0 slot 0 to the texture of its rects[0] (no null test); type 2
// counts n2 and its first sets M1 slots 0/1 from rects[0]/rects[1] (null rect -> null texture); other
// types are skipped. Draw sections (primitive type 4): n0 quads at vertex/index 0 with M0, then n1
// quads with M0, then n2 quads with M1, each starting after the previous section's vertices (4 per
// quad) and indices (6 per quad); vertex count 4n, primitive count 2n; vertex stream 0 and the flare
// layout; appended and released. The sections assume the elements are already grouped by type.
// Release M0 and M1; set geometry (0, mesh, -1, -1); release the mesh; 00B8DAC0.
void flare_build_geometry_00b8dd60(cc13_scene2::FlareView& flare, const void* elements_source,
    std::uint32_t& vertex_layout_1cc, FlareGeometryServices& s);
}  // namespace bsp::cc13_scene
