#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/dyn_lcp_impulse_math.hpp"

// The Dyn scene's collision pass, 00C57070 ("Collide"), the producer of the manifold
// list the substep's contact phase then reads.
//
// docs/DYN_COLLISION_PASS.md carries the addresses, the original ABI and the
// uncertainty. Everything here is a semantic C++ interface for MSVC Win32, not a
// drop-in binary replacement, and every descriptive name is a hypothesis rather than a
// recovered symbol. Two names are the image's own: the RTTI spells
// Dyn::CollisionSystem::IntersectTask2 (vtable 00D7A110, TypeDescriptor 00E17244) for
// the narrow-phase task, and Dyn::Task (vtable 00D7A080) for its base. The scope labels
// "Collide", "BroadPhase", "BroadPhaseUpdate", "ManifoldUpdate", "IntersectLoop" and
// "GetManifold" are image strings.
//
// The pass is six steps against the scene at world+444h:
//
//   1. BroadPhase      00C5712A..00C5747A   refresh every awake body's world AABB
//   2. BroadPhaseUpdate 00C574C2..00C574CB  manager vslot 3, the SAP radix sort
//   3. ManifoldUpdate  00C57565..00C5756C   00C549D0, refresh and retire manifolds
//   4. the pair count  00C5759C..00C575A5   manager vslot 4
//   5. IntersectLoop   00C575AF..00C577F3   collect pairs (vslots 5 and 6), partition
//                                            them into IntersectTask2 batches, fork-join
//   6. the notifications 00C57827          00C35480, the per-body contact callbacks
//
// The narrow phase itself runs under 00403CC0 (IntersectTask2 vslot 0) which calls
// 00C44090 with the scene in ESI, the first pair index in EAX and the last pushed.

namespace bsp {

// ---------------------------------------------------------------------------
// The scene fields the pass uses. Offsets are native, inside the 0E8h-byte scene the
// world holds at world+444h.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDynSceneBodyContainerOffset = 0xa8;    // 00C5712A
inline constexpr std::size_t kDynSceneBodyListHeadOffset = 0x204;    // 00C57130
inline constexpr std::size_t kDynSceneBodyListSentinelOffset = 0x208;
inline constexpr std::size_t kDynSceneBroadPhaseOffset = 0xac;       // 00C574C2
inline constexpr std::size_t kDynScenePairArrayOffset = 0xb4;        // 00C57688
inline constexpr std::size_t kDynScenePairCountOffset = 0xb8;        // 00C5764A
inline constexpr std::size_t kDynScenePairCapacityOffset = 0xbc;     // 00C57656
inline constexpr std::size_t kDynSceneNarrowTaskArrayOffset = 0xc0;  // 00C577A1
inline constexpr std::size_t kDynSceneNarrowTaskCapacityOffset = 0xc4;  // 00C57777
inline constexpr std::size_t kDynSceneEventArrayOffset = 0xd8;       // 00C354A3
inline constexpr std::size_t kDynSceneEventCountOffset = 0xdc;       // 00C575AF zeroes it
inline constexpr std::size_t kDynSceneEventCapacityOffset = 0xe0;
inline constexpr std::size_t kDynSceneEventSpinLockOffset = 0xe4;    // LOCK CMPXCHG
inline constexpr std::size_t kDynSceneNarrowTaskStride = 0x14;       // 00C38292

// The body fields the broad-phase refresh reads, on top of the transform
// bsp/rigid_body_integration.hpp declares.
inline constexpr std::size_t kDynBodyLocalBoundsMinOffset = 0x38;  // 00C5715C
inline constexpr std::size_t kDynBodyLocalBoundsMaxOffset = 0x44;  // 00C5716F
inline constexpr std::size_t kDynBodyBroadPhaseProxyOffset = 0x60;  // 00C5715F
inline constexpr std::size_t kDynBodyShapeListOffset = 0x70;     // 00C440C0, 00C440E8
inline constexpr std::size_t kDynBodyContactCallbackOffset = 0x68;  // 00C35551, 00C35574
inline constexpr std::size_t kDynBodySceneListNextOffset = 0x84;  // 00C57467
inline constexpr std::size_t kDynBodySolverStampOffset = 0x58;   // 00C4DEE7
inline constexpr std::size_t kDynBodySolverIndexOffset = 0x5c;   // 00C4DEDB

// B+50h bit 3. The broad phase refreshes a body's world AABB only while it is set and
// bit 4 (kDynBodyFlagNoIntegrate) is clear: 00C5714C tests 8, 00C57154 tests 10h.
inline constexpr std::uint32_t kDynBodyFlagBroadPhaseActive = 0x8u;

// The manifold container fields 00C549D0 uses that docs/DYN_CONTACT_SOLVER.md did not
// list: the free list it pushes empty manifolds onto, and the doubly-linked list's
// backward pointer.
inline constexpr std::size_t kDynManifoldContainerFreeListOffset = 0x0c;  // 00C54A5B
inline constexpr std::size_t kDynManifoldPrevOffset = 0xd8;               // 00C54A3C

// One shape of a body. The list is threaded through +208h from B+70h.
inline constexpr std::size_t kDynShapeTypeOffset = 0x08;         // 00C44118, 00C4411B
inline constexpr std::size_t kDynShapeGroupOffset = 0x2c;        // 00C4410A, 00C4410D
inline constexpr std::size_t kDynShapeMaskOffset = 0x30;         // 00C44104, 00C44107
inline constexpr std::size_t kDynShapeRestitutionOffset = 0x24;  // 00C4419D
inline constexpr std::size_t kDynShapeFrictionOffset = 0x28;     // 00C44154
inline constexpr std::size_t kDynShapeListNextOffset = 0x208;    // 00C440C5 reaches the head
// 00C44121 multiplies shape A's type by six before adding shape B's, so the dispatch
// table the scene holds is six by six.
inline constexpr std::int32_t kDynShapeTypeCount = 6;

// The 0.5f the AABB refresh and the restitution combine share, the double at 00D7A280.
inline constexpr float kDynAabbHalf = 0.5f;
// 00D7A2D8, the squared distance below which 00C3F650 calls a new contact point the same
// point as an existing one and keeps its accumulated impulses (00C3F668). 0.0025f is
// 0.05 squared.
inline constexpr float kDynContactPointMatchDistanceSq = 0.0025f;
// 00D7A270, the same 0.05 the restitution threshold uses: 00C3F767..00C3F790 rejects a
// candidate whose normal is not unit to within this much of one in the squared length.
inline constexpr float kDynContactNormalUnitTolerance = 0.05f;
// 00C3F943 CMP ECX,4: a manifold never holds more than four points.
inline constexpr std::int32_t kDynManifoldMaxPoints = 4;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// The world AABB of one body, 00C5715C..00C5745E. The local box is turned into a centre
// and a half extent, the centre goes through the 3x4 transform and the half extent
// through the same matrix with every element replaced by its magnitude. That is the
// standard conservative box rotation and it is why a rotating hull's proxy grows.
struct DynWorldBounds {
    float min[3]{};  // proxy+04h
    float max[3]{};  // proxy+10h
};
DynWorldBounds dyn_body_world_bounds_00c5715c(const DynSolverBodyInput& body,
                                              const float local_min[3],
                                              const float local_max[3]) noexcept;

// 00C576E0..00C57705: a pair survives only when at least one of its two bodies is awake. The test
// is `(flagsA & flagsB & 10h) == 0`, so two sleeping bodies are dropped and any other
// combination is kept.
bool dyn_pair_survives_sleep_filter(std::uint32_t flags_a, std::uint32_t flags_b) noexcept;

// 00C44104..00C44110: two shapes are tested when either one's mask selects the other's
// group. The relation is deliberately asymmetric in the fields and symmetric in effect.
bool dyn_shapes_overlap_filter(std::uint32_t group_a, std::uint32_t mask_a,
                               std::uint32_t group_b, std::uint32_t mask_b) noexcept;

// 00C44118..00C44128: the index into the scene's six-by-six narrow-phase dispatch table.
std::int32_t dyn_narrow_phase_dispatch_index(std::int32_t type_a,
                                             std::int32_t type_b) noexcept;

// 00C3F767..00C3F790: a candidate whose normal is not unit within the tolerance is
// dropped before anything else happens.
bool dyn_contact_normal_is_unit(const float normal[3]) noexcept;

// 00C3F650: the index of the first existing point the candidate matches, or
// `point_count` when it matches none. A match on EITHER body's local point is enough,
// which is what lets the warm start survive a contact sliding along one surface.
std::int32_t dyn_match_contact_point_00c3f650(const DynSolverContactPoint* points,
                                              std::int32_t point_count,
                                              const float local_a[3],
                                              const float local_b[3]) noexcept;

// 00C3F8B0..00C3F93C: the penetration depth a point carries is the normal component of
// the gap between the two bodies' world versions of the two local points. It is positive
// while they overlap.
float dyn_contact_depth_00c3f93c(const DynSolverBodyInput& body_a,
                                 const DynSolverBodyInput& body_b,
                                 const DynSolverContactPoint& point) noexcept;

// 00C3F760 in full for one candidate. A matched point keeps its two accumulated impulses
// and takes the new geometry; a new point is appended with both impulses zeroed; a fifth
// candidate against a full manifold takes the reduction branch this packet did not read
// and is reported through `hit_full_manifold` instead of being inserted.
struct DynContactInsertResult {
    bool rejected_normal{false};   // the unit-normal guard rejected it
    bool matched_existing{false};  // an existing point was refreshed in place
    bool appended{false};          // a new point was appended
    bool hit_full_manifold{false}; // four points already, the unread reduction branch
    std::int32_t index{-1};        // which point, when it was matched or appended
};
DynContactInsertResult dyn_insert_contact_point_00c3f760(
    DynSolverContactPoint* points, std::int32_t& point_count,
    const DynSolverBodyInput& body_a, const DynSolverBodyInput& body_b,
    const float local_a[3], const float local_b[3], const float normal[3]) noexcept;

// ---------------------------------------------------------------------------
// The pass as a sequence
//
// One pure-virtual method per native call site inside 00C57070 and 00C35480. The two
// broad-phase managers the shipped scene can hold are behind vslots 3 to 6 of the object
// at scene+0ACh; this packet read the call sites and their return contracts, not the
// SAP radix manager's bodies, so those four methods are the pass's boundary.
// ---------------------------------------------------------------------------
struct DynCollisionPassHost {
    virtual ~DynCollisionPassHost() = default;

    // The scene's body list, [scene+0A8h]+204h through +208h, linked by B+84h.
    virtual std::int32_t body_count() = 0;
    virtual std::uint32_t body_flags(std::int32_t body) = 0;
    virtual DynSolverBodyInput body_transform(std::int32_t body) = 0;
    virtual void body_local_bounds(std::int32_t body, float local_min[3],
                                   float local_max[3]) = 0;
    // 00C57404..00C5745E, the six stores into the proxy at B+60h.
    virtual void set_body_world_bounds(std::int32_t body, const DynWorldBounds& bounds) = 0;

    // 00C574C2..00C574CB, `[[scene+0ACh]+0Ch]()`, the "BroadPhaseUpdate" scope. The SAP radix
    // manager re-sorts its axis lists and rebuilds the overlapping-pair set.
    virtual void broad_phase_update_vslot3() = 0;
    // 00C5759C..00C575A5, `[[scene+0ACh]+10h]()`. The number of pairs the manager holds; zero
    // skips the whole IntersectLoop scope.
    virtual std::uint32_t broad_phase_pair_count_vslot4() = 0;
    // 00C575AF, `scene+0DCh = 0`. Not a call, but the narrow-phase tasks append to that
    // array under the spin lock at scene+0E4h and step 6 reads it, so the sequence has
    // to clear it in the right place: after the pair count and before the loop.
    virtual void clear_contact_events() = 0;
    // 00C576F0..00C576FB, `[[scene+0ACh]+14h]()`, the first pair, and 00C5772B..00C57737,
    // `[[scene+0ACh]+18h](pair)`, the next. A pair's first two fields are pointers to
    // the two bodies' proxies; the body is the first field of each.
    virtual std::int32_t first_pair_vslot5() = 0;
    virtual std::int32_t next_pair_vslot6(std::int32_t pair) = 0;
    virtual std::uint32_t pair_body_flags(std::int32_t pair, int which) = 0;
    // 00C5771B, the surviving pairs are compacted into scene+0B4h in order.
    virtual void keep_pair(std::int32_t slot, std::int32_t pair) = 0;

    // 00C57565..00C5756C, 00C549D0 against the manifold container at scene+0B0h.
    virtual void manifold_update_00c549d0() = 0;

    // 00C577A1..00C577E4, the two ints of each IntersectTask2 at scene+0C0h.
    virtual void set_narrow_task_range(std::int32_t task_index, std::int32_t first_pair,
                                       std::int32_t last_pair) = 0;
    virtual std::int32_t narrow_task_capacity() = 0;
    // 00C577EE, Dyn_TaskScheduler_RunBatchAndWait. Fork-join: the pass is single
    // threaded again on return.
    virtual void run_narrow_phase_batch_00c33140(std::int32_t task_count) = 0;

    // 00C57827, 00C35480. Walks scene+0D8h and calls each body's own callback at B+68h.
    virtual void dispatch_contact_events_00c35480() = 0;
};

// 00C5712A..00C577F8. Returns the number of pairs that survived the sleep filter, which
// is what the narrow-phase partition is sized from.
std::int32_t dyn_run_collision_pass_00c57070(DynCollisionPassHost& host);

}  // namespace bsp
