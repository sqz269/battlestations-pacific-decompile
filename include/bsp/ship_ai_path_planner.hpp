#pragma once

#include <array>
#include <cstdint>

#include "bsp/ship_ai_goal_vector.hpp"

namespace bsp {
// The planner the ship AI hands its goal to: 009E3780 and the node graph it
// seeds. Semantic interfaces, not native object layouts. Names are hypotheses,
// not recovered symbols. Addresses, evidence, original ABI and uncertainty:
// docs/SHIP_AI_PATH_PLANNER.md.
//
// 009E3780 is not the search. It is the request-and-revalidate gate in front of
// one path plan block, called four times per navigation tick by 009ED3E0
// BSP_ShipAi_RefreshPathPlan (009ED523, 009ED588, 009ED5FD, 009ED692). On an
// empty block it latches the goal, pushes both endpoints clear of the avoid
// zones, allocates a two-node graph (ship, goal) and moves the block into the
// Seeded state. On a block that already holds a plan it answers one question:
// is that plan still the plan for this goal and this ship position. The
// incremental search that turns the two seed nodes into a path runs in
// 009EC680, one state transition per navigation tick; 009E3C00 walks the
// finished graph for the next path point and 009D9E50 for the remaining length.
//
//   009ED523  009E3780(plan, &pose, &goal, layer, radius)    seed or revalidate
//   009E3821  004218E0 the avoid-zone manager singleton
//   009E3831  00417E40 the zone containing the goal, in this layer
//   009E3851  00417580 push the goal out of that zone by 10 units
//   009E387B  004120D0 the zone group for this layer
//   009E38A2  0041B840 the nearest zone boundary point to the goal
//   009E38EF  00417E40 / 009E390B 00417580, the same pair on the ship's pose
//   009E3927  operator new(54h) and 009D9150, the node at the ship
//   009E394C  operator new(54h) and 009D9150, the node at the goal
//   009E3973  009D9230 links the two and measures the segment
//   009E3980  owner->vtable[50h]() seeds the start node's +48h
//   009ED58D  009EC680(plan, dt) advances the search one state
//   009EE5F4  009E3C00(plan, &record) reads the next point off the graph
//   009EE6AC  009D9E50(plan, &pose) reads the remaining length
//
// include/bsp/ship_ai_goal_vector.hpp owns the producer of the latched goal;
// include/bsp/ship_ai_navigation.hpp owns the output block the walk feeds;
// include/bsp/ship_ai_state_steps.hpp owns 009DE050 and 009DA4E0.

// ---------------------------------------------------------------------------
// Sizes and vtables
// ---------------------------------------------------------------------------

// 009E3925 and 009E3947 both PUSH 54h before operator new.
inline constexpr std::size_t kShipAiPathNodeSize = 0x54u;
// 009D915D MOV dword ptr [EAX], 0xd214f4.
inline constexpr std::uint32_t kShipAiPathNodeVtable = 0x00D214F4u;
// 009DA4E0 clears two field sets 68h apart (+240h..+25Ch and +2A8h..+2C4h) and
// 009E3C00 writes +64h, so the block is 68h bytes and the navigator embeds two.
inline constexpr std::size_t kShipAiPathPlanBlockSize = 0x68u;
// 009D9D0D MOV dword ptr [EAX], 0xd2152c, the block constructor at 009D9CC0.
inline constexpr std::uint32_t kShipAiPathPlanVtable = 0x00D2152Cu;
// The two embedded blocks inside the navigator, 009DA4E0's two field sets.
inline constexpr std::uint32_t kShipAiPathPlanSlotA = 0x0224u;
inline constexpr std::uint32_t kShipAiPathPlanSlotB = 0x028Cu;

// ---------------------------------------------------------------------------
// Constants, each from the instruction that loads it
// ---------------------------------------------------------------------------

// 00CE38B8, pushed at 009E383A: how far outside the zone a goal that fell
// inside one is placed.
inline constexpr float kShipAiPathGoalZoneMargin = 10.0f;
// 00CE3850, pushed at 009E38F8: the same margin for the ship's own pose.
inline constexpr float kShipAiPathPoseZoneMargin = 5.0f;
// 00CE4BC8, pushed at 009E3891: the AABB slack 0041B840 hands each zone.
inline constexpr float kShipAiPathClearanceSearchRadius = 100000.0f;
// FLD1 at 009E3884, the fourth argument of 0041B840; 0041B8BA adds it to the
// measured distance, so the returned point sits one unit past the boundary.
inline constexpr float kShipAiPathClearancePush = 1.0f;
// FLD1 at 009E38CC: a clearance below one unit is not stored.
inline constexpr float kShipAiPathClearanceFloor = 1.0f;
// 00CF58EC, stored at 009E3819, 009D9D6D and 009DA518: "no clearance measured".
inline constexpr float kShipAiPathNoClearance = 1.0e7f;
// 00CE380C, stored at 009E3ACC: the ceiling on the revalidation delay.
inline constexpr float kShipAiPathRevalidateCap = 1.5f;
// 00CE3D08, loaded at 009E3AD3: the ceiling on the radius that delay uses.
inline constexpr float kShipAiPathRevalidateRadiusCap = 100.0f;
// 00CE3800, stored into +14h by the constructor at 009D9CD4.
inline constexpr float kShipAiPathRevalidateInitial = 0.5f;
// 00D05A40, stored into +0Ch by the constructor at 009D9CEE. Read at 009E3A9A
// and 009E3BB0 as a squared radius, so the corridor is 40 units wide.
inline constexpr float kShipAiPathCaptureRadiusSq = 1600.0f;
// 00D20278, stored into +10h at 009D9D22. Read at 009E39CD and 009E3A34, so a
// goal may drift 50 units before the plan is thrown away.
inline constexpr float kShipAiPathGoalDriftSq = 2500.0f;
// 00CE3930, stored into +4h and +8h at 009D9CFB and 009D9D00. 009EE61E reads
// +8h as the width the navigator publishes for the point.
inline constexpr float kShipAiPathPublishedWidth = 20.0f;
// 00D7A24C, compared at 009D92AC: a link shorter than this sets the node's
// +31h flag.
inline constexpr float kShipAiPathShortLinkLimit = 1.0f;
// 00D21530, a double, compared at 009DA5E2: the moveto arrival radius is 80
// units around the goal the active plan was latched to.
inline constexpr double kShipAiPathArrivalRadiusSq = 6400.0;

// ---------------------------------------------------------------------------
// The search state at plan+1Ch
// ---------------------------------------------------------------------------
//
// 009E3780 gates on it (0 seeds, 2 refuses, 1 and 3 test the goal only, above 3
// tests the corridor) and 009EC680 drives it. The names come from 009EC680's
// switch, not from the image.
enum class ShipAiPathSearchState : int {
    Empty = 0,         // 009E3799, the block has no graph: seed it
    Seeded = 1,        // 009E381E, two nodes exist and the search has not run
    Failed = 2,        // 009E3790 refuses; 009EC680 sets it when +3Ch is null
    Searched = 3,      // 009EC685 case 1/5 sets it once 009D5A20 reports done
    Extracting = 4,    // 009EC6A9 case 3, which also clears the node count
    ExtendSearch = 5,  // 009EC6B8 case 4 when 009D96A0 wants another pass
    Extracted = 6,     // 009EC6B8 case 4 otherwise
    Ready = 7,         // 009EC6C4 case 6: the graph is a usable path
};

// 009E37A2 CMP EAX,3 / JG and 009ED5A6 CMP [EAX+1Ch],3 / JLE: a plan is only
// walked, swapped in and corridor-tested once the state is above Searched.
inline constexpr bool ship_ai_path_plan_has_graph(ShipAiPathSearchState state) noexcept {
    return static_cast<int>(state) > static_cast<int>(ShipAiPathSearchState::Searched);
}

// ---------------------------------------------------------------------------
// The 54h-byte path node, laid out by the constructor 009D9150
// ---------------------------------------------------------------------------
//
// Every field below is written by 009D9150 unless its comment says otherwise.
// The graph is undirected: each node carries two links and the walk in
// 009D9E50 and 009E3C00 picks one of them per node through `direction`.
struct ShipAiPathNode {
    std::uint32_t vtable{kShipAiPathNodeVtable}; // +00h, 009D915D
    // +04h. 0 until a walk decides it, then +1 to follow `link_plus` or -1 to
    // follow `link_minus` (009D9E74..009D9EDC, 009E3C4A..009E3C9C).
    std::int32_t direction{0};
    std::uint32_t field_08{0};        // +08h, 009D9170
    std::uint16_t field_0c{0xFFFFu};  // +0Ch, 009D9173 MOV word, 0xffff
    std::uint32_t field_10{0};        // +10h, 009D9179
    std::uint8_t field_14{0};         // +14h, 009D917C
    float x{0.0f};                    // +18h, 009D9165, the point's world X
    float z{0.0f};                    // +1Ch, 009D916D, the point's world Z
    ShipAiPathNode* link_minus{nullptr}; // +20h, 009D9182; 009D9230 writes it
    ShipAiPathNode* link_plus{nullptr};  // +24h, 009D917F
    ShipAiPathNode* back{nullptr};       // +28h, 009D9188; 009D9248 writes it
    std::uint32_t field_2c{0};        // +2Ch, 009D9185
    std::uint8_t field_30{0};         // +30h, 009D91A3
    // +31h, 009D918E. 009D92BB / 009D92C7 set it when `link_minus_length` is
    // below one unit.
    std::uint8_t short_link{0};
    std::uint8_t field_32{0};         // +32h, 009D918B
    float link_minus_length{0.0f};    // +34h, 009D9196; 009D92AF writes it
    float link_plus_length{0.0f};     // +38h, 009D9191
    // +3Ch, 009D919B. The remaining path length from this node onward, summed
    // by the search and read by 009D9E50 and by the walk's cost comparison.
    float remaining_length{0.0f};
    std::uint32_t zone_layer{0};      // +40h, 009D91A9, the constructor's second argument
    std::uint8_t field_44{0};         // +44h, 009D91A6
    // +45h, 009D91AC. 009E3991 sets it on the node built at the ship's pose.
    std::uint8_t is_start{0};
    // +48h, 009D91AF. 009E3982 seeds it from owner->vtable[50h] on the start
    // node only. No routine read here consumes it; the search does.
    float seed{0.0f};
    float cost_minus{0.0f};           // +4Ch, 009D91B9; read by both walks
    float cost_plus{0.0f};            // +50h, 009D91B4; read by both walks
};

// ---------------------------------------------------------------------------
// The 68h-byte plan block, laid out by the constructor 009D9CC0
// ---------------------------------------------------------------------------
//
// The navigator owns two of these (kShipAiPathPlanSlotA, kShipAiPathPlanSlotB)
// and points nav+2F4h at the one in use and nav+2F8h at the one being computed;
// 009ED5BD/009ED5C3 swap the pointers when the second reaches Ready.
struct ShipAiPathPlanBlock {
    std::uint32_t vtable{kShipAiPathPlanVtable};          // +00h, 009D9D0D
    float field_04{kShipAiPathPublishedWidth};            // +04h, 009D9D00
    float published_width{kShipAiPathPublishedWidth};     // +08h, 009D9CFB
    float capture_radius_sq{kShipAiPathCaptureRadiusSq};  // +0Ch, 009D9CEE
    float goal_drift_sq{kShipAiPathGoalDriftSq};          // +10h, 009D9D22
    // +14h, 009D9CD4. Counted down by 009EC680 at 009EC68D and re-armed at
    // 009E3ACC; while it is positive the corridor test is skipped.
    float revalidate_delay{kShipAiPathRevalidateInitial};
    float field_18{0.0f};                                 // +18h, 009D9CDC
    std::int32_t search_state{0};                         // +1Ch, 009D9D19
    ShipAiPathNode* head{nullptr};                        // +20h, 009D9D16
    ShipAiPathNode* goal_node{nullptr};                   // +24h, 009D9D13
    // +28h: not written by the constructor and not touched by any routine read
    // for this packet.
    std::uint32_t field_28{0};
    std::uint32_t search_context{0};                      // +2Ch, 009D9CE1; 009EC697 hands it to 009EC280
    // +30h, 009D9D2D. The goal's clearance from the nearest avoid zone plus one
    // unit, 0 when the goal had to be pushed out of a zone, and
    // kShipAiPathNoClearance when no zone group or no zone was in range.
    float goal_clearance{kShipAiPathNoClearance};
    std::int32_t node_count{0};                           // +34h, 009D9D1C; 009EC6AC clears it
    std::uint32_t zone_layer{0};                          // +38h, 009D9D27
    // +3Ch, 009D9D1F. The ship this plan belongs to. 009E3ABF reads its radius
    // at +9C8h and its class at +538h; 009EC69x refuses to search without it.
    void* owner{nullptr};
    // +40h / +44h. Not initialised by the constructor; 009E37F2 and 009E37FA
    // latch the goal here, and 009DA5A2 reads them for the arrival test.
    float latched_goal_x{0.0f};
    float latched_goal_z{0.0f};
    std::uint32_t field_48{0};                            // +48h, 009D9D2A
    // +4Ch, +50h, +54h, +60h and +64h are written by the walk in 009E3C00 and
    // belong to the ship_ai_path_follower packet; +58h and +5Ch are unread.
    float follower_4c{0.0f};
    float follower_50{0.0f};
    float follower_54{0.0f};
    std::uint32_t field_58{0};
    std::uint32_t field_5c{0};
    float follower_60{0.0f};
    float follower_64{0.0f};
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 009D9150, __thiscall(node)(const float2* xz, layer) -> node, RET 8, body
// 009D9150-009D91BE, complete. Every other field is zero.
ShipAiPathNode ship_ai_path_node_init_009d9150(const std::array<float, 2>& xz,
                                               std::uint32_t zone_layer) noexcept;

// 009D9230, __thiscall(node)(next), RET 4, body 009D9230-009D92CE, complete.
// A null `next` is a no-op (009D923C). Otherwise the link is one-directional in
// `link_minus` with the back pointer on the far node, and the measured length
// uses the same x87 kernel as 00414C60 (009D9276 compares the rounded sum with
// the double 1e-10 at 00CE3820 before calling the CRT square root).
void ship_ai_path_node_link_009d9230(ShipAiPathNode& node, ShipAiPathNode* next) noexcept;

// 009E399E-009E39E2 and 009E3A05-009E3A3B, the same test on both paths: the
// plan survives only while the goal stays within sqrt(+10h) of the goal it was
// latched to. The sum is rounded to float before the comparison (009E39C5
// FSTP / 009E39C9 FLD).
bool ship_ai_path_goal_within_tolerance_009e399e(float goal_x, float goal_z,
                                                 float latched_x, float latched_z,
                                                 float drift_sq) noexcept;

// 009E3ABF-009E3B0B, re-armed whenever the ship leaves the head node's capture
// circle. Without an owner the delay is the cap alone (009E3ACC then 009E3AD1
// JZ). With one it is min(cap, min(radius_cap, owner+9C8h) / class+500h), and
// 00415510 BSP_Math_MinFloatByRef performs both minima.
float ship_ai_path_revalidate_delay_009e3abf(float owner_radius_09c8,
                                             float owner_class_max_speed_0500,
                                             bool has_owner) noexcept;

// Which exit of the corridor test at 009E3A41-009E3BF9 a pose took.
enum class ShipAiPathCorridorVerdict {
    InsideCapture,    // 009E3AB9 JBE: still within sqrt(radius_sq) of the head node
    BehindSegment,    // 009E3BE5: the projection onto the segment is not positive
    InsideCorridor,   // 009E3BBC JBE: within sqrt(radius_sq) of the projected point
    OutsideCorridor,  // 009E3BC0: the ship has left the corridor
};

struct ShipAiPathCorridorResult {
    ShipAiPathCorridorVerdict verdict{ShipAiPathCorridorVerdict::InsideCapture};
    bool plan_still_valid{true};  // false only for BehindSegment and OutsideCorridor
    float projection{0.0f};       // 009E3B4D, the dot of the pose offset on the unit segment
    float segment_length{0.0f};   // 009E3B12, 00414C60 of the raw segment
};

// 009E3A57-009E3BF9. The head node and its chosen neighbour define one segment;
// the ship is on plan while it is inside the head node's capture circle, or
// ahead of the segment start and within the same radius of its perpendicular
// projection onto the segment. A zero-length segment divides by zero here
// exactly as the image does.
ShipAiPathCorridorResult ship_ai_path_corridor_009e3a57(const std::array<float, 2>& pose,
                                                        const std::array<float, 2>& node,
                                                        const std::array<float, 2>& next,
                                                        float capture_radius_sq) noexcept;

// 009D9E50, __thiscall(plan)(const float2* pose) -> float, RET 4, body
// 009D9E50-009D9F8C, complete. Walks at most node_count+1 links from the head,
// deciding each undecided node's direction from the two link costs, falls back
// to the goal node when the walk runs out, and adds the straight-line distance
// from the pose to the node it stopped on. The navigation arm reads this at
// 009EE6AC as the remaining path length.
float ship_ai_path_remaining_length_009d9e50(ShipAiPathPlanBlock& plan,
                                             const std::array<float, 2>& pose) noexcept;

// The direction decision the two walks share, 009D9E6C-009D9EDC and
// 009E3C56-009E3CC7. Returns the value stored into node+4h. A node that already
// carries a nonzero direction keeps it.
std::int32_t ship_ai_path_node_direction_009d9e6c(const ShipAiPathNode& node) noexcept;

// The step the walks take after the decision, 009D9EE6-009D9EFE. Null when
// neither link matches the direction, which ends the walk.
ShipAiPathNode* ship_ai_path_node_next_009d9ee6(const ShipAiPathNode& node) noexcept;

// 009DA590, __thiscall(nav)(const float2* goal) -> bool, RET 4, body
// 009DA590-009DA604, complete. Both navigation states reach it through the
// three-instruction thunk 009DAB10, which is vtable slot 2Ch of the movetopos
// vtable 00D21628 and of the moveonpath vtable 00D21688. The latch at
// nav+2FEh is set by the controls step at 009EF034 and cleared here on a miss.
struct ShipAiPathArrivalResult {
    bool reached{false};      // 009DA5EE MOV AL,1
    bool clears_latch{false}; // 009DA5F6, the latch byte is cleared on a miss
};
ShipAiPathArrivalResult ship_ai_path_arrival_009da590(bool latch_2fe,
                                                      float goal_x, float goal_z,
                                                      float latched_x, float latched_z) noexcept;

// ---------------------------------------------------------------------------
// The host 009E3780 needs
// ---------------------------------------------------------------------------
//
// One method per native call site, in the order the seeding branch runs them.
// The avoid-zone objects stay opaque: this packet does not own them.
struct ShipAiPathPlannerHost {
    virtual ~ShipAiPathPlannerHost() = default;

    // 009E3821, 009E3870 and 009E38E3: 004218E0
    // BSP_AvoidZoneManager_GetSingleton, called once before each query.
    virtual std::uint32_t avoid_zone_manager_004218e0() = 0;

    // 009E3831 (the goal) and 009E38EF (the pose): 00417E40(manager)(pt, layer).
    // The body walks the layer's group with 00416B50 and returns the first zone
    // that contains the point, or zero.
    virtual std::uint32_t zone_containing_point_00417e40(std::uint32_t manager,
                                                         const std::array<float, 2>& point,
                                                         std::uint32_t zone_layer) = 0;

    // 009E3851 (margin 10) and 009E390B (margin 5): 00417580(zone)(&out, pt,
    // margin, 1). The body re-tests containment with 00414F50 and 00416B50 and
    // returns the point unchanged when it is outside; otherwise 00416F30 puts
    // it on the boundary offset by the margin.
    virtual std::array<float, 2> push_point_out_of_zone_00417580(std::uint32_t zone,
                                                                 const std::array<float, 2>& point,
                                                                 float margin) = 0;

    // 009E387B: 004120D0(manager)(layer), the group whose +10h key matches the
    // layer, else the last group below it. Zero when the manager holds none.
    virtual std::uint32_t zone_group_for_layer_004120d0(std::uint32_t manager,
                                                        std::uint32_t zone_layer) = 0;

    // 009E38A2: 0041B840(group)(&out, pt, search_radius, push). The body asks
    // 0041AEA0 for each zone's distance, keeps the smallest, and returns the
    // point moved onto that boundary and `push` units past it. The point comes
    // back unchanged when no zone is in range.
    virtual std::array<float, 2> nearest_zone_boundary_0041b840(std::uint32_t group,
                                                                const std::array<float, 2>& point,
                                                                float search_radius,
                                                                float push) = 0;

    // 009E3927 and 009E394C: operator new(54h). Zero on failure.
    virtual ShipAiPathNode* allocate_path_node_00bf681b(std::size_t size) = 0;

    // 009E3980: CALL EAX = owner->vtable[50h](), the float stored into the start
    // node's +48h. Callee body unread: contract unread.
    virtual float owner_seed_vtable50() = 0;

    // 009E3ADB, owner+9C8h, the unit radius 009E3C00 reads at the same offset.
    virtual float owner_radius_09c8() = 0;

    // 009E3AF0, [owner+538h]+500h. 00828F20 BSP_ShipClass_DeriveTurnFields
    // writes class+500h as the class maximum speed.
    virtual float owner_class_max_speed_0500() = 0;

    // 009D9D4D inside 009D9D40: CALL EDX = head->vtable[0](1), the scalar
    // deleting destructor of the whole node list. 009ED649 inlines the same
    // call. Callee body unread: contract unread.
    virtual void release_node_list_vtable0(ShipAiPathNode* head) = 0;
};

// 009D9D40, __thiscall(plan), RET 0, body 009D9D40-009D9D73, complete. Releases
// the node list through the host and returns the block to Empty. 009ED642-
// 009ED66A is the same sequence inlined into 009ED3E0.
void ship_ai_path_plan_reset_009d9d40(ShipAiPathPlanBlock& plan,
                                      ShipAiPathPlannerHost& host) noexcept;

// ---------------------------------------------------------------------------
// 009E3780 itself
// ---------------------------------------------------------------------------

// Which exit of 009E3780 a call took. The image returns AL only; the caller
// 009ED3E0 tests it at 009ED602 and tears the plan down on false.
enum class ShipAiPathPlanOutcome {
    SearchFailed,        // 009E3790, state Failed
    LayerChanged,        // 009E37A0, plan+38h does not match the requested layer
    Seeded,              // 009E399B, a fresh two-node graph
    GoalDrifted,         // 009E39D4 and 009E3A3B
    SearchInProgress,    // 009E39DC, states Seeded and Searched with a goal node
    GraphMissing,        // 009E39F3, reset and accepted
    RevalidateDeferred,  // 009E3A51, plan+14h has not run out
    CorridorHeld,        // 009E3BEE with the byte still 1
    CorridorLeft,        // 009E3BC0 and 009E3BE5
    NodeAllocationFailed // see the note below: a deviation, not a native exit
};

struct ShipAiPathPlanRequestResult {
    bool accepted{false};                                    // the AL the image returns
    ShipAiPathPlanOutcome outcome{ShipAiPathPlanOutcome::SearchFailed};
    bool seeded{false};                                      // the block was rebuilt on this call
};

// 009E3780, __thiscall(plan)(const float2* pose, const float2* goal, layer,
// float radius), RET 10h, body 009E3780-009E3BFB, complete.
//
// The fourth stack argument is the unit radius the caller loads from
// [ship+9C8h] (009ED504, 009ED563, 009ED5D7, 009ED673). Nothing in the body
// reads it; 009E3ADB reads the same field through plan+3Ch instead. It is kept
// in the signature because the four call sites push it and the RET is 10h.
//
// Deviation from the image: when operator new returns null, 009E3973 and
// 009E3986 dereference the null head. This reconstruction stops with
// NodeAllocationFailed instead and leaves the block Seeded with whatever nodes
// it did get.
ShipAiPathPlanRequestResult ship_ai_path_plan_request_009e3780(
    ShipAiPathPlanBlock& plan,
    const std::array<float, 2>& pose,
    const std::array<float, 2>& goal,
    std::uint32_t zone_layer,
    float unused_owner_radius,
    ShipAiPathPlannerHost& host);

}  // namespace bsp
