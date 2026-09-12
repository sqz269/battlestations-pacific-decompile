#pragma once
// Explosion radial damage: 0084BAD0's argument contract and per-record fill,
// the gather 00904470, the spatial index's sphere query 0098C630 with the
// recursive node overlap 0098C510, and the node-level sphere test 0098AAE0 that
// the blast path reaches through the collision node's vtable slot 0Ch.
//
// Hypotheses, not recovered symbols. The evidence for every rule is in
// docs/EXPLOSION_RADIAL_DAMAGE.md. The hit record itself is HitRecordFill in
// bsp/hit_narrowphase.hpp and the consumer projection is HitRecord in
// bsp/unit_hit_path.hpp; neither is redeclared here.
#include <cstddef>

#include "bsp/hit_narrowphase.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Globals and constants the blast path reads.
// ---------------------------------------------------------------------------

// [00E188A8 + 1FE4h]. 0084BAD0 gathers nothing when the world mode is 2, but
// the 00428800 call ahead of the test still runs.
inline constexpr std::size_t kWorldOffBlastSuppressMode = 0x1FE4; // 0084BB0A
inline constexpr int kWorldBlastSuppressedMode = 0x02;            // CMP ..,2

// [00E188A8 + 19CCh] is loaded into ECX at 0084BB3B, but 00904470 overwrites
// ECX from the stack at its first instruction and takes the index from 0042E630
// instead, so the load is dead. Kept because the doc's host table names it.
inline constexpr std::size_t kWorldOffBlastIndexUnusedEcx = 0x19CC; // 0084BB3B

// [00E188A8 + 648h], the visit stamp 0098C630 writes into node+4h so a node
// that sits in several cells is tested once per query.
inline constexpr std::size_t kWorldOffSphereQueryStamp = 0x648; // 0098C653
inline constexpr std::size_t kCollisionNodeOffVisitStamp = 0x04; // 0098C762

// The direction 0084BAD0 hands to the queue for every blast hit: +Y only, from
// the 1.0f at 00D7A24C with the other two lanes zeroed by XORPS.
inline constexpr float kBlastQueueDirectionY = 1.0f; // 00D7A24C, 0084BBD9

// Virtual slots. The collision node's interface is four slots wide; the shape
// interface is three, and its sphere slot is __purecall (00BF698E) in the base
// vtable 00CE89DC, so every concrete shape has to define one.
inline constexpr std::size_t kCollisionNodeVtableTraceSegment = 0x08; // 0098B9C0
inline constexpr std::size_t kCollisionNodeVtableOverlapSphere = 0x0C; // 0098AAE0
inline constexpr std::size_t kCollisionShapeVtableTraceSegment = 0x00;
inline constexpr std::size_t kCollisionShapeVtableOverlapSphere = 0x04;

// sourceEntity->vtable[B0h]() at 00904482 returns the collision node the query
// must skip; 0098C510 compares it against the node by pointer.
inline constexpr std::size_t kEntityVtableCollisionNode = 0xB0; // 0090447C

// ---------------------------------------------------------------------------
// The argument contract of 0084BAD0, __fastcall, RET 10h.
//   ECX      centre        const float3*
//   EDX      radius        const float*   (dereferenced, never written)
//   [ESP+4]  damage        const float*   (dereferenced, never written)
//   [ESP+8]  ignore_falloff int, low byte only (MOV AL,byte ptr [ESP+40h])
//   [ESP+Ch] source_entity  the entity the burst never queues a hit on
//   [ESP+10h] shot          stored through 00470350 into record+4h
// ---------------------------------------------------------------------------

struct BlastArgs {
    HitQueryPoint centre{};       // ECX, copied into record+8h..+10h
    float radius{0.0f};           // *EDX, copied into record+24h
    float damage{0.0f};           // *[ESP+4], copied into record+28h
    bool ignore_falloff{false};   // [ESP+8], copied into record+2Ch
    const void* source_entity{nullptr}; // [ESP+Ch], the queue gate
    const void* shot{nullptr};          // [ESP+10h]
};

// ---------------------------------------------------------------------------
// Pure rules.
// ---------------------------------------------------------------------------

// The blast half of 004705C0: the scalar the part damage is scaled by before
// armour. bsp/unit_hit_path.hpp owns the whole formula; this is only the
// fraction, whose two inputs both come from the blast path (the distance from
// the unread part-hit producer, the range from 00904470's record+24h).
// The native divides without guarding a zero range and applies the ignore flag
// only when the fraction is still positive, so a hit outside the radius stays
// outside it however the flag is set.
float blast_falloff_fraction_004705c0(float distance, float radius,
                                      bool ignore_falloff) noexcept;

// centre +/- radius on each axis, the AABB 0098AAE0 and 0098C630 both build.
HitQueryBounds blast_sphere_bounds(const HitQueryPoint& centre, float radius) noexcept;

// 0098AAE0's reject, in the native's comparison order and sense: the node's
// bounds are [node+13Ch..+144h] and [node+148h..+150h], and each test returns
// false on a strict separation only, so touching bounds still overlap.
bool blast_sphere_overlaps_bounds_0098aae0(const HitQueryBounds& sphere,
                                           const HitQueryBounds& node) noexcept;

// The gather's per-record fill, 009044F4..0090453A. The shot goes through
// 00470350, which is the host's business; the four scalar fields are here.
void blast_gather_record_fill_00904470(HitRecordFill& record,
                                       const HitQueryPoint& centre, float radius,
                                       bool ignore_falloff) noexcept;

// 0084BAD0's per-record fill, 0084BBA1..0084BBD1. Note the order the native
// writes in: the damage first, then the shot, then the centre over the position
// the shape wrote, then the flag. The centre overwrite is why a blast record's
// +8h is the burst centre and not a surface point.
void blast_apply_record_fill_0084bad0(HitRecordFill& record,
                                      const HitQueryPoint& centre, float damage,
                                      bool ignore_falloff) noexcept;

// 0084BBCF. The burst never queues a hit on the entity it came from; every
// other gathered record is queued, including other parts of the same unit that
// carry a different node owner.
bool blast_record_is_queued_0084bad0(const HitRecordFill& record,
                                     const void* source_entity) noexcept;

// ---------------------------------------------------------------------------
// Hosts. One virtual method per native call site; no default implementations,
// because nothing here stands in for unrecovered game behaviour.
// ---------------------------------------------------------------------------

// 0098AAE0, the collision node's vtable slot 0Ch, and the only implementation
// the blast path reaches: 0070F090 (the unit part's override) re-pushes the
// same four arguments and tail-calls it.
struct CollisionNodeSphereHost {
    virtual ~CollisionNodeSphereHost() = default;

    // [node+13Ch..+150h], read field by field at 0098AB4B..0098ABAC.
    virtual HitQueryBounds node_bounds(const void* node) = 0;
    // [node+F8h], the count of the inline shape array at [node+D0h].
    virtual int node_shape_count(const void* node) = 0;
    virtual const void* node_shape(const void* node, int slot) = 0;
    // shape->vtable[4](centre, radius, &record) at 0098ABE3. Both concrete
    // shape classes read in this packet implement it as XOR AL,AL / RET 0Ch.
    virtual bool shape_overlaps_sphere(const void* shape, const HitQueryPoint& centre,
                                       float radius, HitRecordFill& record) = 0;
};

// 0098C510, the recursive node walk, and 0098C630, the grid rectangle.
struct SphereQueryHost {
    virtual ~SphereQueryHost() = default;

    // 0098AD60 twice at 0098C6AC and 0098C6B9: the min and max corners of the
    // sphere's AABB become inclusive cell indices, clamped to the grid.
    virtual HitQueryCell cell_of_point(const HitQueryPoint& point) = 0;
    // [world+648h] at 0098C653 and the per-node stamp at node+4h.
    virtual int query_stamp() = 0;
    virtual int node_stamp(const void* node) = 0;
    virtual void set_node_stamp(const void* node, int stamp) = 0;
    // The cell list at index+84h: each link carries the node at link+8h and the
    // next link at link+4h.
    virtual const void* cell_first_link(int cell_x, int cell_z) = 0;
    virtual const void* cell_next_link(const void* link) = 0;
    virtual const void* cell_link_node(const void* link) = 0;
    // node+4Ch into record+0h through 00470370 at 0098C570.
    virtual const void* node_owner_entity(const void* node) = 0;
    // node->vtable[0Ch](centre, radius, exclude, &record) at 0098C595.
    virtual bool node_overlaps_sphere(const void* node, const HitQueryPoint& centre,
                                      float radius, const void* exclude,
                                      HitRecordFill& record) = 0;
    // [node+FCh] and [node+100h], walked at 0098C5C0.
    virtual int node_child_count(const void* node) = 0;
    virtual const void* node_child(const void* node, int slot) = 0;
    // 00470470, 0098C460 and 004704B0: the temporary record is reset, appended
    // on a hit and destroyed before the call returns.
    virtual void record_reset(HitRecordFill& record) = 0;
    virtual void append_record(const HitRecordFill& record) = 0;
    virtual void destroy_record(HitRecordFill& record) = 0;
};

struct SphereQueryArgs {
    HitQueryPoint centre{};
    float radius{0.0f};
    const void* exclude{nullptr}; // the source entity's collision node
};

// 0098C630, RET 10h, __thiscall on the spatial index. Returns whether any node
// reported a hit; the records themselves land in the host's vector.
bool spatial_index_query_sphere_0098c630(SphereQueryHost& host,
                                         const SphereQueryArgs& args);

// 0098C510, RET 14h, __thiscall on the same index, recursive over node children.
bool collision_node_overlap_sphere_0098c510(SphereQueryHost& host,
                                            const SphereQueryArgs& args,
                                            const void* node);

// 0098AAE0, RET 10h, __thiscall on the node.
bool collision_node_sphere_test_0098aae0(CollisionNodeSphereHost& host,
                                         const void* node,
                                         const HitQueryPoint& centre, float radius,
                                         HitRecordFill& record);

// 00904470, __stdcall with six stack arguments, RET 18h.
struct BlastGatherHost {
    virtual ~BlastGatherHost() = default;

    // sourceEntity->vtable[B0h]() at 00904482, skipped when the entity is null.
    virtual const void* entity_collision_node(const void* entity) = 0;
    // 0042E630 at 0090449B, a lazy singleton accessor that leaves the four
    // arguments already on the stack for 0098C630.
    virtual const void* spatial_index() = 0;
    // 0098C630 at 009044A2.
    virtual bool query_sphere(const void* index, const SphereQueryArgs& args) = 0;
    // The gathered records, walked with a 54h stride at 009044B4..00904547.
    virtual int gathered_count() = 0;
    virtual HitRecordFill& gathered_record(int slot) = 0;
    // 00470350 at 009044F4.
    virtual void record_set_shot(HitRecordFill& record, const void* shot) = 0;
};

void blast_gather_hit_records_00904470(BlastGatherHost& host, const BlastArgs& args);

// 0084BAD0.
struct BlastDamageHost {
    virtual ~BlastDamageHost() = default;

    // 00428800 at 0084BB00, before the world-mode test and with the radius and
    // the damage pushed as floats by value. contract: partially read - the body
    // squares the radius and sweeps a container at [world+21D0h]+28h comparing
    // that square against a distance from 0085BF90.
    virtual void call_00428800(float radius, float damage) = 0;
    // [world+1FE4h] at 0084BB0A.
    virtual int world_blast_mode() = 0;
    // 00904470 at 0084BB4A.
    virtual void gather_hit_records(const BlastArgs& args) = 0;
    virtual int gathered_count() = 0;
    virtual HitRecordFill& gathered_record(int slot) = 0;
    // 00470350 at 0084BBB1.
    virtual void record_set_shot(HitRecordFill& record, const void* shot) = 0;
    // 00926E80 at 0084BBF9, with the direction (0, 1, 0).
    virtual void queue_hit(HitRecordFill& record, const HitQueryPoint& direction) = 0;
    // 004704B0 at 0084BC32 and 00BF65AC at 0084BC43.
    virtual void destroy_record(HitRecordFill& record) = 0;
    virtual void free_gathered() = 0;
};

void blast_apply_radial_damage_0084bad0(BlastDamageHost& host, const BlastArgs& args);

} // namespace bsp
