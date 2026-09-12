#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/ship_ai_settings_block.hpp" // ship_avoidance_side_accepted_009ec79b, kShipAvoidanceSide*
#include "bsp/ship_ai_state_steps.hpp"    // ShipAiAvoidanceRequest (brain+3F4h/+3F8h/+3FCh)
#include "bsp/unit_weapons.hpp"           // kUnitOffWeaponDirector = 0x738
#include "bsp/weapon_director.hpp"        // kDirectorOffTorpedoAvoidance and its two neighbours

// The ship AI avoidance request block: blk+3ECh, blk+3F0h, blk+3F4h, blk+3F5h.
//
// docs/SHIP_AI_STATE_STEPS.md left this as the follow-up `ship_ai_avoidance_request`
// with "the other two have no reader yet". Both have readers. The block is four
// bytes of request that a ship AI state step publishes once per re-plan tick, and
// each of the three request fields is ANDed, at its consumer, with one of the
// three authored booleans on the unit's command controller (`unit+738h`), whose
// literal names the property dump 008362A0 spells out (docs/WEAPON_DIRECTOR.md):
//
//   blk+3ECh  byte   <-> director+240h  torpedoAvoidance        reader 009DA1D0
//   blk+3F0h  int32  <-> director+241h  shipCollisionAvoidance  readers 009EC770,
//                                                               009EF350, 009F0EA0
//   blk+3F4h  byte   <-> director+242h  landCollisionAvoidance  readers 009DA6E0,
//                                                               009EF910
//   blk+3F5h  byte    (no director partner) readers 009ED6B0, 009F3F80
//
// The image addresses the block from `brain`, so every writer's displacement is
// eight higher than the blk-relative offset used here: brain+3F4h is blk+3ECh,
// brain+3F8h is blk+3F0h, brain+3FCh is blk+3F4h, brain+3FDh is blk+3F5h. The
// four unreferenced one-instruction setters at 009DABB0/C0/D0/E0 are the
// compiler's copies of the block's accessors and fix the four widths.
//
// Names are hypotheses, not recovered symbols; the three director names are not.
// These are semantic interfaces for MSVC Win32, not drop-in binary replacements.

namespace bsp {

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

// blk = brain+8h (docs/SHIP_AI_STATES.md). Every writer below stores through
// `brain`; subtract this to read the table in blk-relative terms.
inline constexpr std::size_t kShipAiBrainToBlkDelta = 0x008;

inline constexpr std::size_t kShipAiBlkOffTorpedoAvoidanceRequest = 0x3EC; // 009E4695, 009DA21D
inline constexpr std::size_t kShipAiBlkOffShipAvoidancePartyFilter = 0x3F0; // 009E468B, 009F1052
inline constexpr std::size_t kShipAiBlkOffLandAvoidanceRequest = 0x3F4;    // 009E469C, 009DA6E6
inline constexpr std::size_t kShipAiBlkOffDriveBypass = 0x3F5;             // 009E46A3, 009ED9FF

// The one-instruction setters. No call site or vtable slot in the image refers
// to any of them, so they are the compiler's unused COMDAT copies of the inline
// accessors every writer inlined. Their widths are the block's widths.
inline constexpr std::uint32_t kShipAiSetTorpedoAvoidanceRequestUnused = 0x009dabb0u; // byte
inline constexpr std::uint32_t kShipAiSetShipAvoidancePartyUnused = 0x009dabc0u;      // dword
inline constexpr std::uint32_t kShipAiSetLandAvoidanceRequestUnused = 0x009dabd0u;    // byte
inline constexpr std::uint32_t kShipAiSetDriveBypassUnused = 0x009dabe0u;             // byte

// The whole four-byte group. The first three fields are the existing
// ShipAiAvoidanceRequest of bsp/ship_ai_state_steps.hpp, kept by composition so
// that the record has one declaration: `enable_3f4` is blk+3ECh,
// `side_filter_3f8` is blk+3F0h and `flag_3fc` is blk+3F4h. The fourth byte is
// the one bsp/ship_ai_states.hpp already models as
// ShipAiControlBlock::early_out_3f5.
struct ShipAiAvoidanceRequestBlock {
    ShipAiAvoidanceRequest request{};
    bool early_out_3f5{false}; // blk+3F5h = brain+3FDh
};

// The constructor's values, 009E468B..009E46A3 in BSP_ShipAi_NavigationBlockConstruct:
// `MOV [ESI+3F0h],3`, `MOV byte [ESI+3ECh],1`, `MOV byte [ESI+3F4h],1`,
// `MOV byte [ESI+3F5h],BL` with BL = 0.
ShipAiAvoidanceRequestBlock ship_ai_avoidance_request_constructed_009e468b() noexcept;

// 009F1B7B..009F1B9A, the tail of BSP_ShipAi_BrainPrePass, which runs as chain
// step 3 immediately before the state step. It rewrites all four bytes every
// re-plan tick with no condition of its own: `MOV byte [EDI+3F4h],1`,
// `MOV byte [EDI+3FCh],1`, `MOV byte [EDI+3FDh],0` and the side filter from
// `CMP byte [EAX+4],CL / SETNZ CL / LEA ECX,[ECX*4-1]` on the settings
// singleton's AvoidAllShipCollision byte. The state step then overrides it, so
// this is the value a state that writes nothing leaves behind.
ShipAiAvoidanceRequestBlock ship_ai_avoidance_request_prepass_009f1b7b(
    bool avoid_all_ship_collision) noexcept;

// ---------------------------------------------------------------------------
// What the state steps publish
// ---------------------------------------------------------------------------

// 009E15BB..009E15FB, the two arms of BSP_ShipAi_StopStateStep, selected by the
// `state+8h` latch (009E15B4 `CMP byte [EDI+8],BL`, `JNZ 009E15E0`). Both arms
// set blk+3ECh; the moving arm asks for ship avoidance and the avoid zones, the
// stopped arm asks for neither. blk+3F5h is untouched.
void ship_ai_stop_step_request_009e15bb(ShipAiAvoidanceRequest& request,
                                        bool making_way) noexcept;

// 009E2588..009E2669 in BSP_ShipAi_AttackMoveEngageSubStateStep and the
// identically shaped 009E21F0..009E22D8 in BSP_ShipAi_KamikazeAttackStateStep.
// Both load `[[brain+0AA8h]+54h]`, the owning unit's own Party, into the side
// filter: while attacking, a ship avoids only ships of its own side. The latch
// at `sub+8h` then picks the avoid-zone request (1 on the goal arm, 0 on the
// heading arm), and blk+3ECh is not written on either arm, so it keeps whatever
// the pre-pass left.
void ship_ai_attack_step_request_009e2588(ShipAiAvoidanceRequest& request, int own_party,
                                          bool run_mode) noexcept;

// 009E1C28..009E1EC2, the two arms of BSP_ShipAi_LandStateStep. The arm that
// reaches 009DE050 asks for ship avoidance against every side and the avoid
// zones; the other asks for neither. blk+3ECh is not written on either arm.
void ship_ai_land_step_request_009e1c28(ShipAiAvoidanceRequest& request,
                                        bool navigating) noexcept;

// The three arms of BSP_ShipAi_CruiseStateStep, 009E1170. Only this step writes
// blk+3F5h, and only on its first arm.
enum class ShipAiCruiseAvoidanceArm {
    HelmHeldByPlayer, // 009E13B4, the group's slot 1 is not AI held
    UnitPlayerFlag,   // 009E11D6, unit+184h is set
    CruiseRule,       // 009E12EB, the arm that actually cruises
};

// The five bytes the arm selector reads, each from its own call site.
struct ShipAiCruiseAvoidanceInputs {
    // 009E119F/009E11A5: `[[unit+740h]+50h]+1B0h`, index 1 of the slot array
    // 00521E70 indexes. 8 means no slot; the identity of the object holding it
    // is `contract: unread` and docs/CRUISE_COMMAND.md reads the same arm.
    bool group_slot_unassigned{false};
    // 009E11AD, 00927F10 BSP_PartySlot_IsAiHeld on that slot.
    bool group_slot_ai_held{false};
    // 009E11C2, the unit's player-controlled byte at unit+184h.
    bool unit_player_controlled{false};
    // 009E12E4, 00521E70(unit, 0) reading `[unit+1ACh]`: true when that slot is
    // 8, which short-circuits the AI-held test inside the callee.
    bool own_slot_unassigned{false};
    // 009E12E4 again, the 00927F10 half of the same callee.
    bool own_slot_ai_held{false};
};

ShipAiCruiseAvoidanceArm ship_ai_cruise_step_arm_009e11a5(
    const ShipAiCruiseAvoidanceInputs& in) noexcept;

// Writes only the fields the selected arm writes: the first arm is the only one
// that touches blk+3F5h, and no arm leaves a field of the block half written.
ShipAiCruiseAvoidanceArm ship_ai_cruise_step_request_009e11d6(
    ShipAiAvoidanceRequestBlock& block, const ShipAiCruiseAvoidanceInputs& in) noexcept;

// ---------------------------------------------------------------------------
// blk+3F0h, the ship-collision party filter
// ---------------------------------------------------------------------------

// 009EC770 BSP_ShipAi_ObstacleSideAccepted whole, and the two copies the
// compiler inlined at 009F1052..009F1092 (009F0EA0) and 009EF368..009EF3A9
// (009EF350). bsp/ship_ai_settings_block.hpp projects the 009EC79B tail only
// and says the three gates before it need a host; these are the three gates.
// `director_ship_collision_avoidance` is the byte at
// `0080E160(blk+3FCh)+241h` (009EC787, 009F106C, 009EF37B).
bool ship_ai_avoidance_party_accepted_009ec770(
    int party_filter, int other_party, bool director_ship_collision_avoidance,
    bool avoid_all_ship_collision) noexcept;

// 009EAFDE in 009EAFC0: `node+69h = (accept == 0)`, the only effect the filter
// has on a neighbour. The list itself is built by 009F0D20, which never reads
// the block, so the request never changes which ships enter the list at
// blk+608h: it only marks each of them. 009EAFC0's early return for a destroyed
// entity sets node+68h and node+69h together (009EAFD0).
bool ship_ai_neighbour_excluded_009eafde(bool accepted) noexcept;

// ---------------------------------------------------------------------------
// blk+3ECh, the torpedo-avoidance request: 009DA1D0
// ---------------------------------------------------------------------------

// -15.0, the double at 00CE3D58, compared against the unit's world Y at
// unit+100h by `FLD [EDI+100h]; FLD double [00CE3D58]; FCOMIP ST0,ST1; JA`, so
// the predicate fails when -15.0 is strictly greater than the depth, and an
// unordered compare does not take the JA.
inline constexpr double kShipAiAvoidanceMinimumDepth = -15.0;
// 009DA1E7 `PUSH 0Eh`, the argument to the unit's vtable[5Ch] kind predicate.
inline constexpr int kShipAiAvoidanceExcludedKind = 0x0E;

struct ShipAiAvoidanceSteerGateHost {
    virtual ~ShipAiAvoidanceSteerGateHost() = default;
    // 009DA1E9, `[unit]->vtable[5Ch](0Eh)`. A true answer ends the predicate
    // false. contract: unread, the vtable-5Ch predicate family is its own packet.
    virtual bool unit_is_kind_vtable_005c(int kind) = 0;
    // 009DA1FA, the unit's pose-valid byte at unit+0C8h.
    virtual bool unit_pose_valid_00c8() = 0;
    // 009DA205, 00414DB0 BSP_EntityPose_RefreshWorld(unit).
    virtual void refresh_unit_pose_00414db0() = 0;
    // 009DA20A, the unit's world Y at unit+100h, read after the refresh.
    virtual float unit_world_y_0100() = 0;
    // 009DA22C, 0080E160 on blk+3FCh, then the byte at director+240h
    // `torpedoAvoidance`.
    virtual bool director_torpedo_avoidance_0080e160_240() = 0;
};

// 009DA1D0 whole, `__thiscall(blk) -> bool`, RET 0, body 009DA1D0-009DA244.
// Callers 009DE5B0 (the heading override at 009DE8F8) and 009E04E0 (the
// contact-track loop at 009E061B, over the list at blk+404h counted by
// blk+400h). It is the only reader of blk+3ECh in the image.
bool ship_ai_avoidance_steer_gate_009da1d0(const ShipAiAvoidanceRequest& request,
                                           ShipAiAvoidanceSteerGateHost& host);

// ---------------------------------------------------------------------------
// blk+3F4h, the land-avoidance request: 009DA6E0 and the searchers it drives
// ---------------------------------------------------------------------------

// The three 20h-byte avoid-zone searcher records the constructor seeds at
// 009E4401..009E4449 with `+0h` = 1, `+14h` = -1, `+18h` = 0 and `+1Ch` = 0.
inline constexpr std::size_t kShipAiBlkOffAvoidZoneSearchers = 0xA24; // 009DA713
inline constexpr std::size_t kShipAiAvoidZoneSearcherStride = 0x20;   // 0A24h, 0A44h, 0A64h
inline constexpr std::size_t kShipAiAvoidZoneSearcherCount = 3;
inline constexpr std::size_t kShipAiAvoidZoneSearcherOffEnabled = 0x00;  // 009DA70D
inline constexpr std::size_t kShipAiAvoidZoneSearcherOffBox = 0x04;      // 009D707B
inline constexpr std::size_t kShipAiAvoidZoneSearcherOffLayerKey = 0x14; // 009DA729
inline constexpr std::size_t kShipAiAvoidZoneSearcherOffList = 0x18;     // 009DA721

// 00D7A2B0 = 3.0 and 00CE3CA8 = 300.0, both doubles; 00CE3AE8 = 300.0f.
inline constexpr double kShipAiAvoidZoneQuerySpan = 3.0;
inline constexpr double kShipAiAvoidZoneQueryFloorCompare = 300.0;
inline constexpr float kShipAiAvoidZoneQueryFloor = 300.0f;

struct ShipAiAvoidZoneSearcher {
    bool enabled{true};              // +0h
    float min_x{0.0f};               // +4h
    float min_z{0.0f};               // +8h
    float max_x{0.0f};               // +0Ch
    float max_z{0.0f};               // +10h
    std::int32_t layer_key{-1};      // +14h, -1 from the constructor
};

struct ShipAiAvoidZoneSearcherSet {
    std::array<ShipAiAvoidZoneSearcher, kShipAiAvoidZoneSearcherCount> searchers{};
};

// The five dwords 009DA6E0 builds on its stack at 009DA81B..009DA850 and hands
// to 009D7050 by pointer: the hull position, the half extents (the same value
// twice) and the ship class's avoid-zone layer key.
struct ShipAiAvoidZoneQuery {
    float x{0.0f};                  // blk+184h
    float z{0.0f};                  // blk+188h
    float half_width{0.0f};         // +8h
    float half_height{0.0f};        // +0Ch, the same value
    std::int32_t layer_key{0};      // blk+168h
};

// What 009DA6E0 reads off the block besides the request byte.
struct ShipAiAvoidZoneSearcherInputs {
    float hull_x{0.0f};            // blk+184h, 009DA81B
    float hull_z{0.0f};            // blk+188h, 009DA833
    float look_ahead_3c8{0.0f};    // blk+3C8h, 009DA7EB
    std::int32_t layer_key_168{0}; // blk+168h, 009DA823
};

struct ShipAiAvoidZoneSearcherHost {
    virtual ~ShipAiAvoidZoneSearcherHost() = default;
    // 009DA6F6, 009DA73F, 009DA78C and 009DA7DD: 0080E160 on blk+3FCh, then the
    // byte at director+242h `landCollisionAvoidance`. The routine re-reads it
    // once per searcher and once more for the query, so four call sites.
    virtual bool director_land_avoidance_0080e160_242() = 0;
    // 009DA724, 009DA76E and 009DA7BB: 004158A0
    // BSP_AvoidZoneSegmentList_Clear on searcher+18h. Runs only on the
    // enabled-to-disabled edge.
    virtual void avoid_zone_segment_list_clear_004158a0(std::size_t searcher_index) = 0;
    // 009DA854, 009D7050 on searcher 0 with the query above. Body read from
    // pseudocode: it keeps the cached box when searcher+14h still equals the
    // query's layer key and both diagonal corners are inside it
    // (00414F50 BSP_Geometry_ContainsPoint on searcher+4h), and otherwise grows
    // the box to max(half extent * 1.2, half the old box, 500.0) either side of
    // the point, stores the layer key and refills the selected segment runs
    // through 00419FA0 and 004224C0.
    virtual void avoid_zone_query_refresh_009d7050(std::size_t searcher_index,
                                                   const ShipAiAvoidZoneQuery& query) = 0;
};

// 009DA7EB..009DA81B: `w = (float)((double)blk+3C8h * 3.0)` stored to float32,
// then `FLD double [00CE3CA8]; FCOMIP ST0,ST1; JBE` keeps w when 300.0 is below
// or equal to it or the compare is unordered, and substitutes the float 300.0f
// only when 300.0 is strictly greater. A NaN look-ahead therefore survives.
float ship_ai_avoid_zone_query_half_extent_009da7eb(float look_ahead) noexcept;

// 009DA6E0 whole, `__thiscall(blk)(float seconds)`, RET 4, body
// 009DA6E0-009DA860. Chain step 9, call site 009F51D5. The float argument is
// never read.
void ship_ai_refresh_avoid_zone_searchers_009da6e0(
    ShipAiAvoidZoneSearcherSet& searchers, const ShipAiAvoidanceRequest& request,
    const ShipAiAvoidZoneSearcherInputs& inputs, ShipAiAvoidZoneSearcherHost& host);

} // namespace bsp
