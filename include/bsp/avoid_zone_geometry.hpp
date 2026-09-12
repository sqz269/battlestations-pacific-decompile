#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {
// The avoid-zone manager's geometry: the polygons an AI plan bends around.
//
// Packet cc_ai_avoid_zones, worker agent/cc-ai-avoid-zones. Project
// C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra was
// READ-ONLY for this packet. Every descriptive name here is a hypothesis, not
// a recovered symbol. docs/AVOID_ZONE_GEOMETRY.md carries the evidence address
// by address and the Coverage table saying which bodies are partial.
//
// What this is. docs/SHIP_AI_PATH_SEARCH.md left the zone side of the search a
// host contract: 009E3040 asks 00417E90 whether an edge crosses a zone and,
// on a hit, asks 00422500 for the two corners to detour around, but neither
// body was read, so "the zone polygon's shape stays a host contract" and
// "until it lands, 00417E90 answers not blocked and every plan is a straight
// line". This header is that shape. It carries the three native records (the
// layer table, a layer group, a zone polygon and its corner records), the
// segment-versus-zone test the whole search rests on, the point-in-zone test
// the planner uses, and the tangent walk that picks the two detour corners.
//
// The plane is (x, z). The producer reads world path points as float3 and
// keeps [0] and [2] (0041CCD0 at 0041CD32/0041CE7E), so every float2 here is
// (east, north) in world units and the third component never reaches a zone.
//
// Bearings. Every angle in this file is the game's compass bearing
//   bearing = wrap_2pi(pi/2 - atan2(dy, dx))
// inlined at 004225BF, 00422650, 004227B6, 00422848, 0042298D, 00422C40,
// 00422DC4 and 00422E61, each guarded by a squared-distance gate of 1.0 that
// leaves the bearing at zero for a degenerate pair. It grows clockwise in a
// top-down view, so a polygon wound counter-clockwise on screen accumulates
// -2*pi of turn, which is the winding the producer normalises to.
//
// Call order the executable needs, from the mission load down to one detour:
//
//   004E07B7  004218E0             the manager singleton (owned elsewhere)
//   004E07BE  00424D00             the rebuild (owned by cc2_mission_load_hosts)
//   00424D22  0041F600             clear the 14h group slots
//   00424D7A  _strncmp "AvoidZone" the name gate on every world entity
//   00424D92  _strncmp "AvoidZoneG"
//   00424DAE  _sscanf   "AvoidZoneG %*s %d #%03d"   the layer key
//   00424DBE  00417CA0             find or create that layer's group
//   00424DC5  0041D1E0             add the entity to the group as a zone
//   0041D201  007AC9D0             the entity's path interface
//   0041D206  [entity+54h] == 2    the path kind gate
//   0041D233  0041CCD0             build the polygon from the path points
//   00424DDA  00417CA0(0)          layer 0 always exists after a rebuild
//
//   009E3075  00417E90(manager)(layer, &from, &to, &zone, &edge)
//   00417ECE  004120D0             the layer's group
//   00417ED5  004179D0             the group's segment test
//   00417A0E  00416DD0             one zone
//   00416DE9  004F2B00 -> 0085C910 the zone AABB reject
//   00416EA5  004F3730             one polygon edge
//   009E3105  00422500(zone)(far, edge, hint, side, margin, &l, &r, &li, &ri)
//   00422BAB  00417EF0             the forward corner's pull-back
//   0042315C  00417EF0             the backward corner's pull-back
//
// Ownership. 004218E0, 00417E90, 00417EF0, 004120D0, 00417580, 0041B840 and
// 009E3040 belong to other packets; this header projects only what it leased
// and references the rest. include/bsp/ship_ai_path_search.hpp owns the search
// host whose zone methods these rules make implementable, and
// include/bsp/ship_ai_path_planner.hpp owns the planner host that pushes a
// goal out of a zone. This header redefines none of their types; it reuses
// segment_crossing_004f3730 from bsp/gun_bot_remainder.hpp and
// wrapped_angle_subtract_00438b10 from bsp/unit_rudder.hpp rather than
// restating either rule.

// ---------------------------------------------------------------------------
// Sizes and offsets, from the producers
// ---------------------------------------------------------------------------

// 0041D212 PUSH 24h before operator new, the zone record 0041CCD0 constructs.
inline constexpr std::size_t kAvoidZoneRecordSize = 0x24u;
// 0041CD4D and 0041CEEF PUSH 24h, the corner record the same routine appends.
inline constexpr std::size_t kAvoidZoneCornerRecordSize = 0x24u;
// 00417CC7 PUSH 14h, the group 00417CA0 creates on a miss.
inline constexpr std::size_t kAvoidZoneGroupRecordSize = 0x14u;
// 0041F600's loop counter: the table holds 14h inline group pointers at +8h
// and their count at +58h, so the manager's 78h bytes (0042193A) are the
// vtable-free header, twenty slots and the count.
inline constexpr std::int32_t kAvoidZoneGroupSlotCount = 0x14;
inline constexpr std::size_t kAvoidZoneTableSlotsOffset = 0x08u;
inline constexpr std::size_t kAvoidZoneTableCountOffset = 0x58u;

// The zone record, written by 0041CCD0 and read by 00416B50 / 00416DD0.
inline constexpr std::size_t kAvoidZoneCornersOffset = 0x00u;   // 0041CCEF
inline constexpr std::size_t kAvoidZoneCornerCountOffset = 0x04u;
inline constexpr std::size_t kAvoidZoneCornerCapacityOffset = 0x08u;
inline constexpr std::size_t kAvoidZoneLayerOffset = 0x0Cu;     // 0041CD06, 00422BA0
inline constexpr std::size_t kAvoidZoneOwnerOffset = 0x10u;     // 0041D099, unread
inline constexpr std::size_t kAvoidZoneBoundsMinOffset = 0x14u; // 0041CD43, 00416B58
inline constexpr std::size_t kAvoidZoneBoundsMaxOffset = 0x1Cu; // 0041CD73, 004F2B00's LEA

// The corner record. Only +0h, +4h and +1Ch reach the tangent walk; the rest
// is the derived data 0041A200 recomputes after every rebuild.
inline constexpr std::size_t kAvoidZoneCornerPositionOffset = 0x00u;
inline constexpr std::size_t kAvoidZoneCornerEdgeDirOffset = 0x08u;
inline constexpr std::size_t kAvoidZoneCornerNormalOffset = 0x10u;
inline constexpr std::size_t kAvoidZoneCornerEdgeLengthOffset = 0x18u;
inline constexpr std::size_t kAvoidZoneCornerTurnAngleOffset = 0x1Cu;
inline constexpr std::size_t kAvoidZoneCornerSpareOffset = 0x20u;

// The group record: vtable, corner-of-the-table vector and the layer key.
inline constexpr std::size_t kAvoidZoneGroupZonesOffset = 0x04u;
inline constexpr std::size_t kAvoidZoneGroupCountOffset = 0x08u;
inline constexpr std::size_t kAvoidZoneGroupCapacityOffset = 0x0Cu;
inline constexpr std::size_t kAvoidZoneGroupKeyOffset = 0x10u;
// 00417CD5 MOV dword ptr [EAX], 0CE3858h, the group's only vtable; 00417CE4
// starts the key at -1.
inline constexpr std::uint32_t kAvoidZoneGroupVtable = 0x00CE3858u;

// ---------------------------------------------------------------------------
// Constants, each the float or double the listing loads
// ---------------------------------------------------------------------------

// 00CE3830 and 00CE3828, the two doubles every bearing uses. Both hold a
// float-rounded value, so the projection keeps them in float.
inline constexpr float kAvoidZoneHalfPi = 1.5707963705062866f;
inline constexpr float kAvoidZoneTwoPi = 6.2831854820251465f;
// FLD1 at 004225B3 and its seven siblings: a pair closer than one unit leaves
// the bearing at zero instead of calling atan2.
inline constexpr float kAvoidZoneBearingGateSq = 1.0f;
// 00D7A238, the squared distance at which 00416DD0 stops walking edges.
inline constexpr float kAvoidZoneSegmentEndEpsilonSq = 0.01f;
// 00D7A24C, 00422515 COMISS: the tangent margin is floored at one unit.
inline constexpr float kAvoidZoneMarginFloor = 1.0f;
// 00CE38B8, 00422B85 and 00423136 COMISS: below this margin the offset corner
// is published as computed and no pull-back segment is tested.
inline constexpr float kAvoidZonePullbackMargin = 10.0f;
// 00D7A288, 00422A80 and 00423012: a bisector this short means the two edges
// are opposite.
inline constexpr float kAvoidZoneBisectorDegenerateSq = 1.0e-6f;
// 00D7A3A0 and 00CE3928, 004226C1 and 00422CC2: the tolerance that decides
// which side of the edge the query point starts on.
inline constexpr float kAvoidZoneSideTolerance = 0.1f;
// 00CE3880, 0041CEDB: a path point closer than five units to the last kept one
// is dropped, so no zone edge is shorter than that.
inline constexpr float kAvoidZoneMinCornerSpacingSq = 25.0f;
// 00D7A218, 0041A47E: a polygon whose turn sum exceeds this is reversed once.
inline constexpr float kAvoidZoneWindingTurnLimit = 0.0f;
// 00CE3820, 0041A310: below this an edge has no direction and length zero.
inline constexpr float kAvoidZoneEdgeLengthEpsilonSq = 1.0e-10f;
// 00D7A260, 0041CDC0 and 0041CF34: the corner record's +20h starts here and
// 0041A200 never
// writes it. No reader was identified; provisional.
inline constexpr float kAvoidZoneCornerSpareInit = -1.0f;
// 00D7A280, 0085C918: the SAT test halves every extent.
inline constexpr float kAvoidZoneBoxHalf = 0.5f;

// 00424D00's name gate and its two literals live in
// include/bsp/mission_load_hosts.hpp (kAvoidZonePrefix, kAvoidZoneGroupPrefix,
// kAvoidZoneGroupFormat). 0041D206 CMP dword ptr [EDI+54h], 2.
inline constexpr std::int32_t kAvoidZonePathKind = 2;
// 00424DD7 XOR ESI,ESI before the last 00417CA0: layer 0 always exists.
inline constexpr std::int32_t kAvoidZoneDefaultLayer = 0;

// ---------------------------------------------------------------------------
// The records
// ---------------------------------------------------------------------------

// One 24h-byte corner record. 0041CCD0 writes the position and clears the
// rest; 0041A200 fills the derived half for every corner of the polygon.
struct AvoidZoneCorner {
    std::array<float, 2> position{{0.0f, 0.0f}};   // +0h, +4h: (x, z)
    std::array<float, 2> edge_direction{{0.0f, 0.0f}};  // +8h, +0Ch: unit, this corner to the next
    std::array<float, 2> normal{{0.0f, 0.0f}};     // +10h, +14h: unit corner normal
    float edge_length{0.0f};                       // +18h: to the next corner
    float turn_angle{0.0f};                        // +1Ch: signed bearing turn at this corner
    float spare{kAvoidZoneCornerSpareInit};        // +20h: written once, no reader found
};

// One 24h-byte zone record. The native keeps the corners as a hand-grown
// vector of pointers ([zone+0h] base, [zone+4h] count, [zone+8h] capacity,
// doubling at 0041CDDD); the projection keeps them by value in order.
struct AvoidZonePolygon {
    std::vector<AvoidZoneCorner> corners{};
    std::int32_t layer{0};                         // +0Ch, the group key it was built under
    std::array<float, 2> bounds_min{{0.0f, 0.0f}}; // +14h, +18h
    std::array<float, 2> bounds_max{{0.0f, 0.0f}}; // +1Ch, +20h
};

// One 14h-byte group: every zone that shares a layer key.
struct AvoidZoneLayerGroup {
    std::int32_t layer_key{-1};                    // +10h, 00417CE4 starts it at -1
    std::vector<AvoidZonePolygon> zones{};         // +4h / +8h / +0Ch
};

// The manager's geometry, without the critical section and the singleton
// bookkeeping that mission_load_hosts.hpp already records. 00417CA0 keeps the
// groups sorted by ascending key and 0041F600 empties all twenty slots.
struct AvoidZoneTable {
    std::vector<AvoidZoneLayerGroup> groups{};
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// wrap_2pi(pi/2 - atan2(dy, dx)), or 0 when the pair is within one unit.
// Inlined at 004225BF and seven more sites in 00422500 and twice in 0041A200.
float avoid_zone_bearing(const std::array<float, 2>& from,
                         const std::array<float, 2>& to) noexcept;

// 0085C910, __fastcall(ECX min, EDX max, b0, b1) -> bool in AL, RET 8, body
// 0085C910-0085CAC7, reached through the adjustor thunk 004F2B00 which turns
// the zone's +14h into the (min, max) pair. A 2D separating-axis test between
// the box and the segment: the two box axes and the segment's perpendicular.
bool avoid_zone_box_meets_segment_0085c910(const std::array<float, 2>& box_min,
                                           const std::array<float, 2>& box_max,
                                           const std::array<float, 2>& b0,
                                           const std::array<float, 2>& b1) noexcept;

// 00416B50, __thiscall(zone)(const float2*) -> bool in AL, RET 4, body
// 00416B50-00416CC4, complete. The AABB rejects first (half-open on the max
// side), then a crossing-number walk over the closed polygon counting edges
// whose interpolated x lies left of the point.
bool avoid_zone_contains_point_00416b50(const AvoidZonePolygon& zone,
                                        const std::array<float, 2>& point) noexcept;

// 00416DD0, __thiscall(zone)(const float2* toward, const float2* start,
// float2* running, int* edge) -> bool in AL, RET 10h, body
// 00416DD0-00416F20, complete. The running point is the segment end that
// moves: it starts at *start, and every edge that the segment (running,
// toward) crosses moves it to the crossing and records that edge, so the
// value left behind is the crossing nearest `toward`. The edge index is the
// crossing edge's first corner, negative indices wrapped by the corner count
// at 00416F11. The caller shares one running point across a whole group, and
// 004179D0 passes the same pointer as `start` and `running`.
bool avoid_zone_segment_hit_00416dd0(const AvoidZonePolygon& zone,
                                     const std::array<float, 2>& toward,
                                     std::array<float, 2>& running,
                                     std::int32_t& edge_index) noexcept;

struct AvoidZoneGroupHit {
    bool hit = false;
    std::int32_t zone_index = -1;   // index into AvoidZoneLayerGroup::zones
    std::int32_t edge_index = -1;   // the crossing edge's first corner
    std::array<float, 2> point{{0.0f, 0.0f}};
};

// 004179D0, __thiscall(group)(const float2* toward, const float2* from,
// float2* running, zone** hit_zone, int* edge) -> bool in AL, RET 14h, body
// 004179D0-00417A39, complete. It copies *from into the running point once
// (004179D4..004179E6) and then hands the SAME running point to every zone in
// the group, so a zone can only improve on a crossing an earlier zone found:
// what comes back is the crossing nearest `toward` over the whole group, and
// the zone recorded is the one that produced it.
AvoidZoneGroupHit avoid_zone_group_segment_hit_004179d0(const AvoidZoneLayerGroup& group,
                                                        const std::array<float, 2>& toward,
                                                        const std::array<float, 2>& from) noexcept;

// 004120D0, __thiscall(manager)(int key) -> group*, RET 4, body
// 004120D0-00412119, complete, the lookup 00417E90 / 00417EF0 / 009E3780 use.
// Exact key, else the last group whose key is below it, else slot 0. It never
// returns null and never checks the count before reading slot 0, so a table
// with no groups reads uninitialised memory; after a rebuild slot 0 always
// exists (00424DDA). Returns an index into AvoidZoneTable::groups, or -1 when
// the table is empty, which is where the projection stops short of the native.
std::int32_t avoid_zone_group_for_layer_004120d0(const AvoidZoneTable& table,
                                                 std::int32_t layer) noexcept;

// 00412120, the same lookup written the other way round (it records every key
// at or below the target and breaks on the exact one), body
// 00412120-0041216B, complete. 00417E40 uses this one. For the ascending order
// 00417CA0 maintains the two agree on every input.
std::int32_t avoid_zone_group_for_layer_00412120(const AvoidZoneTable& table,
                                                 std::int32_t layer) noexcept;

// 004178F0, __thiscall(group)(const float2*) -> zone*, RET 4, body
// 004178F0-0041793B, complete. The first zone of the group that contains the
// point, in slot order; the zone behind 00417E40's answer. Returns an index
// into group.zones, or -1 for none.
std::int32_t avoid_zone_first_containing_004178f0(const AvoidZoneLayerGroup& group,
                                                  const std::array<float, 2>& point) noexcept;

// 00417CA0, __thiscall(manager)(int key) -> group*, RET 4, body
// 00417CA0-00417D58, complete. Find the group with that key, else create one
// (operator new(14h), vtable 00CE3858, empty vector, key) and insert it before
// the first higher key, shifting the tail up one slot. Nothing bounds the
// insert against the twenty slots. Returns the index of the group.
std::int32_t avoid_zone_group_find_or_create_00417ca0(AvoidZoneTable& table,
                                                      std::int32_t layer) noexcept;

// 0041F600, __fastcall(manager), RET, body 0041F600-0041F639, complete for the
// slot loop: count = 0 and every one of the twenty slots gets its vtable[0]
// scalar-deleting destructor called with 1 and is nulled. The tail call
// 0041E960 is unread.
void avoid_zone_table_clear_0041f600(AvoidZoneTable& table) noexcept;

// ---------------------------------------------------------------------------
// The tangent walk, 00422500
// ---------------------------------------------------------------------------

// The offset direction at one corner: the normalised sum of the unit vectors
// from the corner to its two neighbours, which points out of the polygon at a
// convex corner. When that sum is degenerate (00422A80, |sum|^2 <= 1e-6, the
// two edges are opposite) the native falls back to the perpendicular of the
// corner-to-previous vector, (dy, -dx) normalised, where (dx, dy) is that
// vector. `forward` selects which neighbour plays the role of "previous":
// the forward pass at 004229D6 takes next then previous, the backward pass at
// 00422F56 takes previous then next.
std::array<float, 2> avoid_zone_corner_offset_direction(const AvoidZonePolygon& zone,
                                                        std::int32_t corner,
                                                        bool forward) noexcept;

struct AvoidZoneTangentCorners {
    // The native leaves the caller's points untouched when a pass finds
    // nothing; 009E3082 pre-fills both with the sentinel at 00CE4C04, so the
    // caller can tell "no corner" from "both corners" even though the return
    // value is 0 for each.
    bool has_forward = false;
    bool has_backward = false;
    std::array<float, 2> forward_point{{0.0f, 0.0f}};    // param_9, 00422B78
    std::array<float, 2> backward_point{{0.0f, 0.0f}};   // param_8, 00423129
    std::int32_t forward_index = -1;                     // param_11, 00422BCA
    std::int32_t backward_index = -1;                    // param_10, 00423173
    // The return value: +1 only the forward corner, -1 only the backward one,
    // 0 for both and also for neither (00422BD2 sets the flag, 00423186
    // subtracts one from it). 009E3256 TEST/JG and 009E32AA TEST/JL read it.
    std::int32_t side_code = 0;
};

// 00422500, __thiscall(zone)(float far_x, float far_z, int edge_index,
// int near_corner_hint, int far_side_hint, float margin, float2* out_backward,
// float2* out_forward, int* out_backward_index, int* out_forward_index) -> int
// in EAX, RET 28h, body 00422500-0042318F, complete.
//
// Two mirrored walks from the corner the blocked edge starts at. Each tracks
// one angle: the bearing from the walking corner to `far_point` minus the
// bearing of the edge leaving it, carried from corner to corner by subtracting
// (forward) or adding (backward) each corner's +1Ch turn. A corner is a
// candidate when that angle changes sign, and the candidate is accepted when
// the exactly recomputed angle has the right sign; that corner is the tangent
// point from `far_point` to the polygon on that side. The published point is
// the corner pushed `margin` units along its offset direction, and when the
// margin exceeds ten units the push is tested against the zone's own layer
// (00417EF0) and pulled back to the boundary crossing nearest the corner.
//
// `far_side_hint` gates the passes: >= 0 runs the forward one, <= 0 the
// backward one, so 0 runs both. `near_corner_hint` >= 0 stops a pass after
// that many corners, the distance from `edge_index` to the hint measured the
// way that pass walks. Both are node fields at the search's call site
// (009E30B6 MOVSX from the far node's +14h byte, 009E30C3 MOVSX from the near
// node's +0Ch word) and are -1 / 0 when the node does not carry them.
//
// `table` stands in for the 004218E0 singleton the native fetches at 00422B95
// and 00423146 for the pull-back; the zone's own +0Ch layer selects the group.
AvoidZoneTangentCorners avoid_zone_tangent_corners_00422500(
    const AvoidZoneTable& table,
    const AvoidZonePolygon& zone,
    const std::array<float, 2>& far_point,
    std::int32_t edge_index,
    std::int32_t near_corner_hint,
    std::int32_t far_side_hint,
    float margin) noexcept;

// ---------------------------------------------------------------------------
// The producer: a mission's zones
// ---------------------------------------------------------------------------

// 0041A200's per-corner pass, run once at the end of 0041CCD0 (0041D063).
// Fills every corner's outgoing edge direction and length, its normal and its
// turn angle, sums the turns, and when the sum is above zero reverses the
// corner order and runs again with the reverse disabled, so a finished polygon
// always has a turn sum at or below zero. Evidence only: 0041A200 is leased to
// another packet, so this projection claims no ledger record for it and
// docs/AVOID_ZONE_GEOMETRY.md lists it as read-only evidence.
void avoid_zone_rebuild_corner_data(AvoidZonePolygon& zone) noexcept;

// 0041CCD0's body, the zone constructor: keep path point 0, then every later
// point more than five units from the last kept one, in the (x, z) plane;
// grow the AABB over them (00415010 at 0041CFB9); finish with the corner pass
// above. Same note as that routine: 0041CCD0 is leased to another packet and
// this is a projection from its body as evidence, not an owned reconstruction.
// The world-box step 0041A540 at 0041D053 and the owner query 00923810 at
// 0041D06D are not projected.
AvoidZonePolygon avoid_zone_from_path_points(const std::vector<std::array<float, 3>>& path_points,
                                             std::int32_t layer);

// 00424D00's name gate, 00424D7A / 00424D92 / 00424DAE. Returns the layer key
// for a world entity's name, or -1 when the name is not an avoid zone. A name
// that starts with "AvoidZone" but not "AvoidZoneG" (sixteen of the shipped
// ones, e.g. "AvoidZone1") keeps layer 0, and so does a "AvoidZoneG" name the
// format does not parse, because 00424D8E zeroes the slot before the sscanf.
std::int32_t avoid_zone_layer_from_entity_name(const std::string& name) noexcept;

// One pure-virtual per native call site the rebuild makes outside this packet.
struct AvoidZoneSceneHost {
    virtual ~AvoidZoneSceneHost() = default;

    // 00424D28: the intrusive world list at [[00E188A8]+19CCh]+370h walked
    // through node+4h with the element at node+8h. The projection takes it as
    // a count and an index rather than a link walk.
    virtual std::int32_t world_entity_count() = 0;

    // 00424D35..00424D6A: the element's name is its +154h length and +158h
    // buffer, with "<null name>" for a zero length and the empty string at
    // 00E17654 for a null buffer.
    virtual std::string world_entity_name(std::int32_t index) = 0;

    // 0041D201 007AC9D0 then 0041D206 CMP [entity+54h], 2. True when the
    // entity offers a path interface AND that field is 2; 0041D20C skips the
    // entity otherwise. Which of the two the field belongs to was not read, so
    // the host answers for the pair.
    virtual bool world_entity_is_zone_path(std::int32_t index) = 0;

    // 0041CD1B: the path's point count is ([path+0Ch] - [path+8h]) >> 2, and
    // 0041CD32 / 0041CE7E read point i as a float3 through 007AF800.
    virtual std::vector<std::array<float, 3>> world_entity_path_points(std::int32_t index) = 0;
};

// 00424D00's scan, 00424D0F..00424DDF. Clears the table, walks the world,
// builds one polygon per named path entity into its layer's group, and makes
// sure layer 0 exists. Partial: the tail 00424DDF-00425487 registers the ship
// tuning block's draft keys as further layers through 004223B0 and 00423C50
// and is not projected here.
void avoid_zone_rebuild_from_scene_00424d00(AvoidZoneSceneHost& host, AvoidZoneTable& table);

} // namespace bsp
