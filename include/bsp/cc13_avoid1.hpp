#pragma once

// cc13_avoid1: avoid-zone navigation leftovers of segment 1 (packet cc13-avoid1,
// docs/CC13_AVOID1.md). Every name is a hypothesis, not a recovered symbol.
// These are new C++ interfaces over the native records (or over the existing
// AvoidZoneBoundaryZoneView snapshots), not drop-in replacements of the
// original ABI. Nothing here is wired into the game; the ship lane binds.

#include <array>
#include <cstdint>
#include <vector>

#include "bsp/avoid_zone_boundary.hpp"
#include "bsp/avoid_zone_clearance.hpp"
#include "bsp/avoid_zone_dyn_hull.hpp"
#include "bsp/avoid_zone_owner.hpp"
#include "bsp/camera_projection.hpp"
#include "bsp/ship_ai_lateral_record.hpp"

namespace bsp {

struct CameraAxesCrtAccess;

// ---------------------------------------------------------------------------
// Native records used below
// ---------------------------------------------------------------------------

// The 14h-byte zone group the manager's twenty slots point at. 0041DE70 stores
// vtable 00CE3858; 0041D0C0/0041D150 walk +4h/+8h; 0041DE70 frees +4h with the
// array free 00BF6989. +0Ch is the hand-grown capacity, +10h the layer key
// (AvoidZoneLayerGroup's comment, 00417CE4).
struct Cc13AvoidZoneNativeGroup {
    std::uint32_t vtable;                  // +00h
    AvoidZoneNativeStorage** zones;        // +04h
    std::uint32_t count;                   // +08h
    std::uint32_t capacity;                // +0Ch
    std::int32_t layer_key;                // +10h
};
static_assert(sizeof(Cc13AvoidZoneNativeGroup) == 0x14);

// The 0Ch-byte "optional direction" 0041B290 writes and 0041B700 merges:
// (x, z) and a valid byte at +8h.
struct Cc13AvoidZoneOptionalVector {
    float x;                               // +00h
    float z;                               // +04h
    std::uint8_t valid;                    // +08h
    std::uint8_t untouched[3];
};
static_assert(sizeof(Cc13AvoidZoneOptionalVector) == 0x0c);

// A std::list node of the manager's +6Ch hull list (0041C0F0 builds one).
struct Cc13AvoidZoneHullListNode {
    Cc13AvoidZoneHullListNode* next;       // +00h
    Cc13AvoidZoneHullListNode* previous;   // +04h
    AvoidZoneDynHullHandle value;          // +08h
};
static_assert(sizeof(Cc13AvoidZoneHullListNode) == 0x10);

// The 78h-byte avoid-zone manager [00E17624], as 00421500 lays it out.
struct Cc13AvoidZoneManagerNative {
    std::uint32_t vtable;                  // +00h, 00CE38F4
    void* critical_section;                // +04h, 00BD1860 result
    void* group_slots[20];                 // +08h..+57h
    std::uint32_t group_count;             // +58h
    std::uint32_t vector_proxy_5c;         // +5Ch, never written
    void* vector_first;                    // +60h
    void* vector_last;                     // +64h
    void* vector_end;                      // +68h
    std::uint32_t list_proxy_6c;           // +6Ch, never written
    Cc13AvoidZoneHullListNode* list_head;  // +70h
    std::uint32_t list_size;               // +74h
};
static_assert(sizeof(Cc13AvoidZoneManagerNative) == 0x78);

// The 28h-byte TerrainGridLayer (00424680 constructs it: vtable 00CE38CC,
// +14h = tan(angle), grid vector +18h..+24h).
struct Cc13TerrainGridLayerNative {
    std::uint32_t vtable;                  // +00h
    float half_extent;                     // +04h
    std::int32_t dimension;                // +08h
    float cell_size;                       // +0Ch
    float file_scalar;                     // +10h
    float slope_limit;                     // +14h
    std::uint32_t grid_proxy_18;           // +18h
    std::uint8_t* grid_first;              // +1Ch
    std::uint8_t* grid_last;               // +20h
    std::uint8_t* grid_end;                // +24h
};
static_assert(sizeof(Cc13TerrainGridLayerNative) == 0x28);

struct Cc13TerrainGridLayerListNode {
    Cc13TerrainGridLayerListNode* next;    // +00h
    Cc13TerrainGridLayerListNode* previous;// +04h
    Cc13TerrainGridLayerNative* value;     // +08h
};

// The 14h-byte avoid-zone registry [00E17620] (004C17D0, operator new(14h)).
struct Cc13AvoidZoneRegistryNative {
    std::uint32_t vtable;                  // +00h, 00CE38E4
    std::uint32_t list_proxy_04;           // +04h
    Cc13TerrainGridLayerListNode* head;    // +08h
    std::uint32_t size;                    // +0Ch
    float max_cell_height;                 // +10h, 0041E000; the constructor leaves it
};
static_assert(sizeof(Cc13AvoidZoneRegistryNative) == 0x14);

// The 00423C50 scratch object 004235A0 destroys: four vectors (first word of
// each at +10h, +20h, +30h, +58h) and one vector of 10h-byte vectors at +44h.
struct Cc13AvoidZoneRawVector {
    std::uint32_t proxy;                   // +00h, never written here
    void* first;                           // +04h
    void* last;                            // +08h
    void* end;                             // +0Ch
};
static_assert(sizeof(Cc13AvoidZoneRawVector) == 0x10);
struct Cc13AvoidZoneDraftScratch {
    std::uint8_t unread_00[0x10];
    void* vector10[3];                     // +10h first/last/end
    std::uint8_t unread_1c[4];
    void* vector20[3];                     // +20h
    std::uint8_t unread_2c[4];
    void* vector30[3];                     // +30h
    std::uint8_t unread_3c[8];
    Cc13AvoidZoneRawVector vectors44;      // +44h: first/last/end at +48h..+50h
    std::uint8_t unread_54[4];
    void* vector58[3];                     // +58h
};
static_assert(sizeof(Cc13AvoidZoneDraftScratch) == 0x64);

// ---------------------------------------------------------------------------
// Callees reached through named hooks
// ---------------------------------------------------------------------------

class Cc13AvoidDebugDrawHost {
public:
    virtual ~Cc13AvoidDebugDrawHost() = default;
    // 0085FB30, __fastcall(ECX p0, EDX p1; r, g, b, a).
    virtual void line_0085fb30(const float p0[3], const float p1[3], float r, float g, float b,
                               float a) = 0;
    // 00860030, __thiscall(ECX rgba; x0, y0, x1, y1), RET 10h.
    virtual void screen_line_00860030(const float rgba[4], float x0, float y0, float x1,
                                      float y1) = 0;
    // 0085FB70, __fastcall(ECX p0, EDX p1; rgba0 by value, rgba1 by value), RET 20h.
    virtual void arrow_0085fb70(const float p0[3], const float p1[3], const float rgba0[4],
                                const float rgba1[4]) = 0;
    // 00860BE0, __fastcall(ECX centre, EDX rgba; radius, rgba*, detailed byte).
    virtual void wire_sphere_00860be0(const float centre[3], const float rgba_edx[4],
                                      float radius, const float* rgba_stack,
                                      std::uint8_t detailed) = 0;
};

class Cc13AvoidCrtHost {
public:
    virtual ~Cc13AvoidCrtHost() = default;
    // 00BF7030, the CRT sqrt on ST0.
    virtual double crt_sqrt_00bf7030(double value) = 0;
};

class Cc13AvoidZoneSimplifyHost : public Cc13AvoidCrtHost {
public:
    // 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), RET 14h, ST0
    // (stored as a float by the caller).
    virtual float interpolate_clamped_00419010(float x0, float y0, float x1, float y1,
                                               float x) = 0;
    // 00BF6713, the checked-iterator report. Handler return/failure policy is
    // external; returning requires valid state for the caller's continuation.
    virtual void invalid_parameter_00bf6713() = 0;
};

// The object 00423640 hands to 0041EA30: a vector of 0Ch-byte points whose
// first/last words sit at +10h/+14h. Only those two words are read or written.
struct Cc13AvoidZonePathNative {
    std::uint8_t unread_00[0x10];
    std::array<float, 3>* first;           // +10h
    std::array<float, 3>* last;            // +14h
};

class Cc13AvoidZoneManagerHost {
public:
    virtual ~Cc13AvoidZoneManagerHost() = default;
    // 0041BF50, the list's head-node allocator (_Buynode()).
    virtual Cc13AvoidZoneHullListNode* buy_head_node_0041bf50(Cc13AvoidZoneManagerNative&) = 0;
    // 00BD1860, creates the tracked critical section.
    virtual void* create_critical_section_00bd1860() = 0;
    // 0041F600, ECX manager: zero +58h, delete the twenty group slots.
    virtual void clear_groups_0041f600(Cc13AvoidZoneManagerNative&) = 0;
    // 0041CC80, ECX = &manager+4h.
    virtual void destroy_owned_section_0041cc80(void** section_slot) = 0;
    // 0041E200, ECX = &manager+6Ch: unlink and free every hull node.
    virtual void clear_hull_list_0041e200(Cc13AvoidZoneManagerNative&) = 0;
    // The store to the singleton slot [00E17624].
    virtual void store_manager_singleton_00e17624(void* value) = 0;
};

class Cc13AvoidZoneRegistryHost {
public:
    virtual ~Cc13AvoidZoneRegistryHost() = default;
    // 0041BFD0, the layer list's head-node allocator.
    virtual Cc13TerrainGridLayerListNode* buy_head_node_0041bfd0(Cc13AvoidZoneRegistryNative&) = 0;
    // 00BF681B operator new(28h); may return null only through the new handler.
    virtual void* allocate_layer_00bf681b(std::uint32_t bytes) = 0;
    // 00BF65AC, the unwind release of that block when the constructor throws.
    virtual void free_layer_00bf65ac(void* block) noexcept = 0;
    // 00424680, __thiscall(layer, float angle), RET 4.
    virtual Cc13TerrainGridLayerNative* construct_layer_00424680(void* block, float angle) = 0;
    // 0041C1F0, __thiscall(list)(next, previous, const value&), RET 0Ch.
    virtual Cc13TerrainGridLayerListNode* buy_node_0041c1f0(Cc13AvoidZoneRegistryNative&,
        Cc13TerrainGridLayerListNode* next, Cc13TerrainGridLayerListNode* previous,
        Cc13TerrainGridLayerNative* const& value) = 0;
    // 00420140, __thiscall(list)(1): the length check and ++size.
    virtual void increment_size_00420140(Cc13AvoidZoneRegistryNative&, std::uint32_t count) = 0;
    // 0041DED0, ECX registry: delete every layer and pop it.
    virtual void clear_layers_0041ded0(Cc13AvoidZoneRegistryNative&) = 0;
    // 0041D710, ECX = &registry+4h: the list destructor.
    virtual void destroy_layer_list_0041d710(Cc13AvoidZoneRegistryNative&) = 0;
    // The store to the singleton slot [00E17620].
    virtual void store_registry_singleton_00e17620(void* value) = 0;
    // 00BF6713, the CRT invalid-parameter report from the checked iterators.
    // Handler return/failure policy is external; the wrapper has a plain RET.
    // Returning requires valid state for the caller's continuation.
    virtual void invalid_parameter_00bf6713() = 0;
};

// ---------------------------------------------------------------------------
// Debug drawing (callers BSP_ShipAiDebug_Draw 009DDCC0 and two vtable slots)
// ---------------------------------------------------------------------------

// 004166E0, __thiscall(ECX = &selected list head; const float rgba[4]*), RET 4.
// Draws every selected segment start->end at y = 0 in one colour, following
// next, or next_run when closes_run is set or next is null.
void cc13_avoid_draw_selected_segments_004166e0(AvoidZoneSelectedSegment* const& head,
    const float rgba[4], Cc13AvoidDebugDrawHost& host);

// 00416840, __thiscall(ECX zone; const CameraMatrix*), RET 4. Projects every
// vertex (x, 0, z) through the matrix (004142E0) and draws white screen lines
// from the previous vertex (the ring starts at the last one) to (t.x / 666.0,
// -t.z / 500.0). No empty guard: the last vertex is read first.
void cc13_avoid_draw_zone_screen_00416840(const AvoidZoneBoundaryZoneView& zone,
    const CameraMatrix& matrix, Cc13AvoidDebugDrawHost& host);

// 004169A0, __thiscall(ECX zone; const float rgba[4]*), RET 4. Arrows at
// y = 3.0 from the previous vertex to each vertex, and once, after the first
// arrow, a red/yellow radius-5 wire sphere at the first vertex.
void cc13_avoid_draw_zone_arrows_004169a0(const AvoidZoneBoundaryZoneView& zone,
    const float rgba[4], Cc13AvoidDebugDrawHost& host);

// 004178B0, __thiscall(ECX group; rgba*), RET 4: 004169A0 on every zone.
void cc13_avoid_draw_group_arrows_004178b0(const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const float rgba[4], Cc13AvoidDebugDrawHost& host);
// 00417990, __thiscall(ECX group; matrix*), RET 4: 00416840 on every zone.
void cc13_avoid_draw_group_screen_00417990(const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const CameraMatrix& matrix, Cc13AvoidDebugDrawHost& host);

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

// 0041AC60, __thiscall(zone)(const float2* centre, float radius, float
// start_bearing, float sweep), RET 10h, AL. Rejects a centre outside the
// bounds grown by radius (half-open) or a zone with no vertex; otherwise true
// when any 004F3BA0 crossing of the circle with an edge has a bearing
// (00414EB0's formula, inlined) whose 00438B10 difference from start_bearing
// lies strictly between 0 and sweep (either sign).
bool cc13_avoid_zone_arc_hit_0041ac60(const AvoidZoneBoundaryZoneView& zone,
    const std::array<float, 2>& centre, float radius, float start_bearing, float sweep,
    const CameraAxesCrtAccess& crt);
// 0041B510, __thiscall(group)(same four), RET 10h: 0041AC60 on each zone in
// order, true on the first hit.
bool cc13_avoid_group_arc_hit_0041b510(const std::vector<AvoidZoneBoundaryZoneView>& zones,
    const std::array<float, 2>& centre, float radius, float start_bearing, float sweep,
    const CameraAxesCrtAccess& crt);

// 0041B120, __thiscall(zone)(const float2* point, float slack), RET 8, ST0.
// FLT_MAX outside the slack-grown half-open bounds, 0 inside the polygon
// (00416B50), else the smallest 00419AB0 edge distance (ring from the last
// vertex; no empty guard).
float cc13_avoid_zone_distance_0041b120(AvoidZoneBoundaryHost& host,
    const AvoidZoneBoundaryZoneView& zone, const std::array<float, 2>& point, float slack);
// 0041B680, __thiscall(group)(point*, slack), RET 8, ST0: the minimum of
// 0041B120 over the zones, FLT_MAX for none.
float cc13_avoid_group_distance_0041b680(AvoidZoneBoundaryHost& host,
    const std::vector<AvoidZoneBoundaryZoneView>& zones, const std::array<float, 2>& point,
    float slack);

// 0041B290, __thiscall(zone)(Cc13AvoidZoneOptionalVector* out, const float2*
// point, float slack), RET 0Ch. Outside the grown bounds: out.valid = 0 only.
// Else n = normalize(point - 00416F30(point, push 0)) and len its length
// (0 at or below 1e-10 squared), negated (n and len) when the point is inside
// (00416B50). len > slack (or ordered greater): valid = 0. Otherwise valid = 1
// and out = (d * n.z, -0.0f - d * n.x) with d = slack - len: the boundary
// tangent scaled by the remaining slack.
void cc13_avoid_zone_tangent_0041b290(AvoidZoneBoundaryHost& host, Cc13AvoidCrtHost& crt,
    const AvoidZoneBoundaryZoneView& zone, Cc13AvoidZoneOptionalVector& out,
    const std::array<float, 2>& point, float slack);

// 0041B700, __fastcall(ECX accumulator, EDX addend), RET. Invalid
// accumulator: copy a valid addend (x, z, valid). Both valid: flip the addend
// when the dot product is <= 0 (ordered), then add it and set valid.
void cc13_avoid_merge_tangent_0041b700(Cc13AvoidZoneOptionalVector& accumulator,
    const Cc13AvoidZoneOptionalVector& addend) noexcept;

// 00423520, __thiscall(zone)(float2* out, int index, float distance), RET 0Ch.
// record = 00417610(zone, index); 00423190(zone, record); s = min(record
// clearance scale, distance) (the cache when it is not greater); out =
// record position + s * record offset direction.
std::array<float, 2>& cc13_avoid_corner_offset_point_00423520(const AvoidZoneNativeStorage& zone,
    std::array<float, 2>& out, std::int32_t index, float distance,
    AvoidZoneClearanceAccess& clearance, const AvoidZoneAllocationAccess& allocation,
    const CameraAxesCrtAccess& crt);

// 0041EA30, __stdcall(path*, params*), RET 8. Simplifies a closed ring of at
// least ten points in place. index[0..n] = 0..n; each pass walks from the kept
// anchor and extends the candidate while every skipped original point lies
// within the tolerance of the line through the candidate (signed distance
// along the perpendicular (u, v) of anchor - candidate, negative sides scaled
// by -1.2); the first violation backs off one candidate. Tolerance per pass is
// 00419010(10, 3, 30, params+4, n - removed). Passes repeat while a pass
// removed something and more than ten points remain; then the dropped points
// are erased from the vector (checked erase, x87 float copies).
void cc13_avoid_simplify_ring_0041ea30(Cc13AvoidZonePathNative* path,
    const float& tolerance_at_30, Cc13AvoidZoneSimplifyHost& host,
    const AvoidZoneAllocationAccess& allocation);

// ---------------------------------------------------------------------------
// Groups, manager and registry lifetime
// ---------------------------------------------------------------------------

// 0041D0C0, __fastcall(ECX group): free every zone (its records, its record
// array, the zone itself), null each slot, then count = 0. The slot array stays.
void cc13_avoid_group_clear_0041d0c0(Cc13AvoidZoneNativeGroup& group,
    const AvoidZoneAllocationAccess& allocation) noexcept;
// 0041D150, __thiscall(group)(void* owner), RET 4. owner 0: 0041D0C0. Else
// free each zone whose +10h equals owner and fill its slot with the last one.
void cc13_avoid_group_remove_owner_0041d150(Cc13AvoidZoneNativeGroup& group, void* owner,
    const AvoidZoneAllocationAccess& allocation) noexcept;
// 0041DE70, __fastcall(ECX group): the group destructor body.
void cc13_avoid_group_destroy_0041de70(Cc13AvoidZoneNativeGroup& group,
    const AvoidZoneAllocationAccess& allocation) noexcept;

// 0041C0F0, __stdcall(next, previous, const handle&), RET 0Ch, EAX node:
// operator new(10h), link words, copy the hull handle (00C40F50); a throwing
// copy frees the node and rethrows.
Cc13AvoidZoneHullListNode* cc13_avoid_hull_list_buy_node_0041c0f0(
    Cc13AvoidZoneHullListNode* next, Cc13AvoidZoneHullListNode* previous,
    const AvoidZoneDynHullHandle& value, const AvoidZoneAllocationAccess& allocation,
    const AvoidZoneDynHullMemory& hull_memory);

// 00421500, __fastcall(ECX manager), EAX manager: the manager constructor.
Cc13AvoidZoneManagerNative& cc13_avoid_manager_construct_00421500(
    Cc13AvoidZoneManagerNative& manager, Cc13AvoidZoneManagerHost& host);
// 0041F980, __fastcall(ECX manager): the manager destructor.
void cc13_avoid_manager_destroy_0041f980(Cc13AvoidZoneManagerNative& manager,
    Cc13AvoidZoneManagerHost& host, const AvoidZoneAllocationAccess& allocation);

// 004235A0, __fastcall(ECX scratch): the 00423C50 scratch destructor.
void cc13_avoid_draft_scratch_destroy_004235a0(Cc13AvoidZoneDraftScratch& scratch,
    const AvoidZoneAllocationAccess& allocation) noexcept;

// 00424730, __fastcall(ECX registry), EAX registry: the registry constructor.
// Three default layers, 10, 20 and 70 degrees ([00CE3990], [00CE398C],
// [00CE3988]), each operator new(28h) + 00424680, appended in that order.
Cc13AvoidZoneRegistryNative& cc13_avoid_registry_construct_00424730(
    Cc13AvoidZoneRegistryNative& registry, Cc13AvoidZoneRegistryHost& host);
// 0041F0D0, __fastcall(ECX registry): the registry destructor.
void cc13_avoid_registry_destroy_0041f0d0(Cc13AvoidZoneRegistryNative& registry,
    Cc13AvoidZoneRegistryHost& host);
// 0041E000, __fastcall(ECX registry): +10h = max(10.0, every cell height of
// the FIRST layer), cell (i, j) = grid[n * clamp(j) + clamp(i)] * layer+10h.
void cc13_avoid_registry_end_load_0041e000(Cc13AvoidZoneRegistryNative& registry,
    Cc13AvoidZoneRegistryHost& host);

}  // namespace bsp
