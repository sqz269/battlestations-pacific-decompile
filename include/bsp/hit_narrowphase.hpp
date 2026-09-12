#pragma once
// Hit narrowphase: the spatial index's segment query 0098ADD0, the per-entity
// pass 0098AC20, the producer side of the hit record, the scene-node flags that
// gate the projectile sweep, and the flak detonation 0070C210.
//
// Hypotheses, not recovered symbols. Every name below is descriptive; the
// evidence for each rule is in docs/HIT_NARROWPHASE.md. The record offsets and
// the two damage formulas are reused from bsp/unit_hit_path.hpp rather than
// redeclared.
#include <cstddef>

#include "bsp/unit_hit_path.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The grid. 0098AD60 maps a point to a cell; 0098ADD0 walks the rectangle.
// ---------------------------------------------------------------------------

inline constexpr int kSpatialGridDim = 0x96;      // CMP with 95h at 0098AE55
inline constexpr int kSpatialGridMaxIndex = 0x95; // the clamp, inclusive
inline constexpr int kSpatialGridHalf = 0x4B;     // ADD EAX,4Bh inside 0098AD60
inline constexpr std::size_t kSpatialGridHeadsOffset = 0x84;    // index+84h
inline constexpr std::size_t kSpatialGridLooseArrayOffset = 0x08; // index+8h
inline constexpr std::size_t kSpatialGridLooseCountOffset = 0x80; // index+80h

// Collision node fields 0098ADD0 and 0098AC20 read. The node is a proxy: the
// entity the record names is node+4Ch, not the node.
inline constexpr std::size_t kCollisionNodeOffOwnerEntity = 0x4C;  // 0098AC88
inline constexpr std::size_t kCollisionNodeOffShapeArray = 0xD0;   // 0098AC51
inline constexpr std::size_t kCollisionNodeOffShapeCount = 0xF8;   // 0098AC4B
inline constexpr std::size_t kCollisionNodeOffChildArray = 0xFC;   // 0098ACC5
inline constexpr std::size_t kCollisionNodeOffChildCount = 0x100;  // 0098ACCB
inline constexpr std::size_t kCollisionNodeOffBoundsMin = 0x13C;   // 0098AEEE
inline constexpr std::size_t kCollisionNodeOffBoundsMax = 0x148;   // 0098AF00
inline constexpr std::size_t kCollisionNodeShapeSlots = 0x0A;      // (F8h-D0h)/4
inline constexpr std::size_t kCollisionCellNodeOffNext = 0x04;     // 0098AFBB
inline constexpr std::size_t kCollisionCellNodeOffEntity = 0x08;   // 0098AEB4

// Hit record offsets bsp/unit_hit_path.hpp does not carry, all producer-side.
inline constexpr std::size_t kHitRecordOffEntity = 0x00;      // 00470370
inline constexpr std::size_t kHitRecordOffPosition = 0x08;    // the shape vtable[0]
inline constexpr std::size_t kHitRecordOffBlastCentre = 0x18; // 00904470
inline constexpr std::size_t kHitRecordOffShapeKind = 0x30;   // 0087FF5D
inline constexpr std::size_t kHitRecordOffPartCapacity = 0x44; // 00470470 only
inline constexpr std::size_t kHitRecordOffDirection = 0x54;   // 00926F48
inline constexpr std::size_t kHitRecordSize = 0x54;           // the blast vector stride

inline constexpr int kHitRecordResetIndex = -1; // +30h and +34h at 00470473
inline constexpr int kHitRecordShapeKindSimple = 0x0A; // the only kind in the program

// Scene-node flag bytes. The projectile derives from this base at offset 0, so
// bsp/projectile_impact.hpp's kProjectileOffSweepEnabled and ...OffBindGate name
// the same two bytes from the projectile's side.
inline constexpr std::size_t kSceneNodeOffActive = 0x5C;    // 00922F30 / 00922F80
inline constexpr std::size_t kSceneNodeOffTornDown = 0x5D;  // 00922FD0, 009263C0
inline constexpr std::size_t kSceneNodeOffDestroyed = 0x5E; // the guard both test
inline constexpr std::size_t kSceneNodeOffRemoved = 0x5F;   // 009263C0

// ---------------------------------------------------------------------------
// Pure geometry. 0098ADD0's own rules, with no host behind them.
// ---------------------------------------------------------------------------

struct HitQueryPoint {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct HitQueryBounds {
    HitQueryPoint min{};
    HitQueryPoint max{};
};

struct HitQueryCell {
    int x{0};
    int z{0};
};

// 00722B20: component-wise min and max of the two endpoints.
HitQueryBounds segment_bounds_00722b20(const HitQueryPoint& from,
                                       const HitQueryPoint& to) noexcept;

// 0098AD60: floor(p.x / cell_size) + 4Bh and floor(p.z / cell_size) + 4Bh. The
// y component is not indexed. cell_size is the float at 00CE3D90.
HitQueryCell cell_of_point_0098ad60(const HitQueryPoint& p, float cell_size) noexcept;

// 0098AE33..0098AE60: both cell pairs are clamped into [0, 95h] before the walk,
// the low pair from below and the high pair from above.
HitQueryCell clamp_cell_0098add0(const HitQueryCell& cell, bool clamp_low) noexcept;

// 0098AEEE..0098AF60: the six float compares, in listing order. Returns false as
// soon as one separates, which is what the six JA branches do.
bool bounds_overlap_0098add0(const HitQueryBounds& segment,
                             const HitQueryBounds& box) noexcept;

// 0085CAD0: the six-axis separating-axis test between the box and the segment
// treated as a degenerate box. Three face axes, then the three cross products.
bool segment_overlaps_box_0085cad0(const HitQueryBounds& box,
                                   const HitQueryPoint& from,
                                   const HitQueryPoint& to) noexcept;

// ---------------------------------------------------------------------------
// The hit record, producer side. HitRecord in bsp/unit_hit_path.hpp is the
// consumer's projection; this is what the producers actually write, and
// consumer_view() is the bridge between the two.
// ---------------------------------------------------------------------------

struct HitRecordFill {
    const void* entity{nullptr};   // +0h
    const void* shot{nullptr};     // +4h
    HitQueryPoint position{};      // +8h..+10h
    float hull_damage_base{0.0f};  // +14h
    HitQueryPoint blast_centre{};  // +18h..+20h
    float falloff_range{0.0f};     // +24h
    float part_damage_base{0.0f};  // +28h
    bool ignore_falloff{false};    // +2Ch
    int shape_kind{kHitRecordResetIndex};   // +30h
    int hull_segment{kHitRecordResetIndex}; // +34h
    const void* source_record{nullptr};     // +38h, no writer found
    const HitPartEntry* part_hits{nullptr}; // +3Ch, contract: unread
    int part_hit_count{0};                  // +40h, contract: unread
    int part_capacity{0};                   // +44h
    float applied_damage{0.0f};             // +48h, written back by the consumer
};

// 00470470. Note the two indices reset to -1 while every other field resets to
// zero, so an untouched record already reads as "no shape, no hull segment".
void hit_record_reset_00470470(HitRecordFill& record) noexcept;

// 00470350. The hull damage base is the shot's own vtable[54h]; a null shot
// leaves +14h alone, which after a reset means zero.
void hit_record_set_shot_00470350(HitRecordFill& record, const void* shot,
                                  float shot_hull_damage) noexcept;

// 00470370 at 0098ACB4: the entity written is the node's owner, node+4Ch.
void hit_record_set_entity_00470370(HitRecordFill& record, const void* owner_entity) noexcept;

// The tail every known shape vtable[0] shares (0087FF4x, 0087FFCx, 00929C4x):
// the hit point, the owning entity, the shape kind and "no hull segment".
void shape_hit_fill_0087fec0(HitRecordFill& record, const HitQueryPoint& hit_position,
                             const void* owner_entity) noexcept;

// 00904470's per-record prefill on the blast path: the centre, the radius the
// part falloff divides by, and the ignore flag.
void blast_record_prefill_00904470(HitRecordFill& record, const void* shot,
                                   float shot_hull_damage, const HitQueryPoint& centre,
                                   float radius, bool ignore_falloff) noexcept;

// 0084BAD0's second pass: the part damage base, the shot again, the hit position
// set to the burst centre, and the ignore flag again.
void blast_record_finish_0084bad0(HitRecordFill& record, const void* shot,
                                  float shot_hull_damage, const HitQueryPoint& centre,
                                  float damage, bool ignore_falloff) noexcept;

// 0084BBCx: the burst never queues a hit on the entity that produced it.
bool blast_record_queues_0084bad0(const HitRecordFill& record, const void* source_entity) noexcept;

// The consumer projection, so the two damage formulas of bsp/unit_hit_path.hpp
// can be run over a record this packet filled.
HitRecord consumer_view(const HitRecordFill& record, float weapon_scale,
                        float owner_modifier) noexcept;

// ---------------------------------------------------------------------------
// Part selection.
// ---------------------------------------------------------------------------

struct WorstPartResult {
    int index{kHitPartEntryNoPart}; // -1 when nothing scored above zero
    float damage{0.0f};
};

// 004706D0. Seeded at 0.0f with a strict compare, so a zero or negative part
// never wins and the first entry wins a tie.
WorstPartResult worst_part_004706d0(const HitRecord& hit, float armour_scaled) noexcept;

// 00470740: the same loop without the index.
float max_part_damage_00470740(const HitRecord& hit, float armour_scaled) noexcept;

// ---------------------------------------------------------------------------
// The scene-node flags 006E64EF and 006E6BB0 gate on.
// ---------------------------------------------------------------------------

struct SceneNodeFlags {
    bool active{false};    // +5Ch
    bool torn_down{false}; // +5Dh
    bool destroyed{false}; // +5Eh
    bool removed{false};   // +5Fh
};

void scene_node_enable_00922f30(SceneNodeFlags& node) noexcept;  // and vtable[68h]
void scene_node_disable_00922f80(SceneNodeFlags& node) noexcept; // and vtable[6Ch]
void scene_node_kill_00922fd0(SceneNodeFlags& node) noexcept;    // and vtable[84h]
void scene_node_remove_009263c0(SceneNodeFlags& node) noexcept;  // and vtable[80h]

bool projectile_sweep_enabled_006e64ef(const SceneNodeFlags& node) noexcept;
bool projectile_may_bind_006e6bb0(const SceneNodeFlags& node) noexcept;

// ---------------------------------------------------------------------------
// The segment query as a sequence over an injected host. One virtual method per
// native call site; nothing here stands in for unrecovered behaviour.
// ---------------------------------------------------------------------------

struct SegmentQueryHost {
    virtual ~SegmentQueryHost() = default;

    // The cell size at 00CE3D90, read by 0098AD60.
    virtual float grid_cell_size() = 0;
    // The node list head of one cell of the grid at index+84h, and the walk of
    // one list through node+4h / node+8h.
    virtual const void* cell_first_node(int cell_x, int cell_z) = 0;
    virtual const void* cell_next_node(const void* cell_node) = 0;
    virtual const void* cell_node_entity(const void* cell_node) = 0;
    // The unbucketed array at index+8h with its count at index+80h.
    virtual int loose_entity_count() = 0;
    virtual const void* loose_entity(int slot) = 0;

    // entity[+4Ch] and entity[+4Ch]->vtable[5Ch](kind), 0098AECD.
    virtual const void* entity_owner(const void* entity) = 0;
    virtual bool entity_is_kind(const void* owner_entity, int kind) = 0;
    // entity+13Ch and entity+148h, 0098AEEE.
    virtual HitQueryBounds entity_bounds(const void* entity) = 0;

    // entity+F8h shapes at entity+D0h and the shape's own vtable[0], 0098AC82.
    virtual int shape_count(const void* entity) = 0;
    virtual bool shape_trace_segment(const void* entity, int shape_slot,
                                     const HitQueryPoint& from, const HitQueryPoint& to,
                                     HitRecordFill& record) = 0;
    // entity+100h children at entity+FCh, 0098ACC5.
    virtual int child_count(const void* entity) = 0;
    virtual const void* child_entity(const void* entity, int slot) = 0;
};

struct SegmentQueryArgs {
    HitQueryPoint from{};
    HitQueryPoint to{};
    const void* exclude_entity{nullptr}; // arg3, an entity pointer, not a mask
    int kind_filter{0};                  // arg5, 0 means no filter
};

// 0098AC20. Returns whether anything was hit; on a hit the record holds the
// nearest hit and `to` has been pulled back to it.
bool test_entity_0098ac20(SegmentQueryHost& host, const void* entity,
                          const SegmentQueryArgs& args, HitQueryPoint& to,
                          HitRecordFill& record) noexcept;

// 0098ADD0. The grid rectangle, then the loose array; the record ends up holding
// the closest hit because the segment shortens on every accepted hit.
bool query_segment_0098add0(SegmentQueryHost& host, const SegmentQueryArgs& args,
                            HitRecordFill& record) noexcept;

// ---------------------------------------------------------------------------
// The flak burst.
// ---------------------------------------------------------------------------

// The class-descriptor fields 0070C210 reads, by offset.
struct FlakBurstClass {
    float damage_min{0.0f};          // +B4h
    float damage_max{0.0f};          // +B8h
    float radius{0.0f};              // +70h
    const void* burst_effect{nullptr}; // +2Ch, null skips the effect
};

// The arguments 0070C280 hands 0084BAD0, in that routine's own order.
struct FlakBurstParameters {
    HitQueryPoint centre{};              // projectile+FCh
    float radius{0.0f};                  // classDesc+70h
    float damage{0.0f};                  // Random(classDesc+B4h, classDesc+B8h)
    bool ignore_falloff{true};           // PUSH 1 at 0070C26E
    const void* excluded_entity{nullptr}; // projectile+238h, the owner
    const void* shot{nullptr};            // the projectile itself
};

struct FlakDetonationHost {
    virtual ~FlakDetonationHost() = default;
    virtual void refresh_world_pose() = 0;                       // 00414DB0
    virtual bool world_pose_valid() = 0;                         // projectile+C8h
    virtual HitQueryPoint world_translation() = 0;               // projectile+FCh
    virtual float random_in_range(float low, float high) = 0;    // 00BD2F10
    virtual void apply_explosion(const FlakBurstParameters& burst) = 0; // 0084BAD0
    virtual void spawn_burst_effect(const void* definition,
                                    const HitQueryPoint& at) = 0;      // 008685E0
    virtual const void* attached_effect() = 0;                   // projectile+198h
    virtual void move_attached_effect(const void* effect,
                                      const HitQueryPoint& at) = 0;    // 004842C0
    virtual void kill_projectile(int mode) = 0;                  // BSP_MissionEntity_Kill(1)
};

inline constexpr int kFlakKillMode = 1; // PUSH 1 at 0070C355

// 0070C210, in listing order. The projectile is always killed, even when the
// class descriptor carries no effect.
void flak_detonate_0070c210(FlakDetonationHost& host, const FlakBurstClass& desc,
                            const void* owner_entity, const void* projectile) noexcept;

} // namespace bsp
