#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/fixed_step_job_waves.hpp"

// What a tick element actually does per fixed step: the six-slot interface at
// 00D0DEC8, the seven concrete vtables that override it, and the per-step slot
// bodies of the four registering classes.
//
// docs/FIXED_STEP_JOB_WAVES.md reconstructed the driver and left the element's
// virtuals as opaque host methods ("what a concrete +4h, +8h or +0Ch override
// does is a different packet"). This header is that packet.
// docs/TICK_ELEMENT_OVERRIDES.md carries the evidence.
//
// Every name here is a hypothesis, not a recovered symbol.

namespace bsp {

// ---------------------------------------------------------------------------
// The interface, 00D0DEC8
//
// bsp/fixed_step_job_waves.hpp already declares the element's own layout
// (kTickElement*Offset, kTickElementSize) and the three slots the waves call
// (kTickElementStepSlot 04h, kTickElementPreSlot 08h, kTickElementPostSlot 0Ch).
// Only the two slots no wave reaches are new here.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kTickElementDestroySlot = 0x00;   // scalar deleting dtor
inline constexpr std::size_t kTickElementUnusedSlot = 0x10;    // 0042BBA0, RET 4
inline constexpr std::size_t kTickElementPredicateSlot = 0x14; // 0042BBB0, XOR AL,AL + RET

inline constexpr int kTickElementVtableSlots = 6;              // 00D0DEE0 is zero
inline constexpr std::uint32_t kTickElementBaseVtable = 0x00d0dec8;

// 00875890 writes 00D7A260 into element+30h (kTickElementTimerOffset). The
// constant is -1.0f, and 00811AB0 scales the step by it only when it is above
// 1.0f (COMISS against 00D7A24C = 1.0f at 00811AB9), so the default disables
// the scale: element+30h is a per-element time-scale, not a countdown.
inline constexpr float kTickElementTimeScaleDisabled = -1.0f;  // 00D7A260
inline constexpr float kTickElementTimeScaleFloor = 1.0f;      // 00D7A24C
inline constexpr float kTickElementFixedStep = 0.05f;          // 00D0DE84

// The five base stubs. Four are one instruction, the fifth is two; none of the
// seven derived tables overrides the last two.
inline constexpr std::uint32_t kTickElementBaseStepStub = 0x0042bb70;      // RET 4
inline constexpr std::uint32_t kTickElementBaseAdvanceStub = 0x0042bb80;   // RET 4
inline constexpr std::uint32_t kTickElementBaseCommitStub = 0x0042bb90;    // RET, no argument
inline constexpr std::uint32_t kTickElementBaseUnusedStub = 0x0042bba0;    // RET 4
inline constexpr std::uint32_t kTickElementBasePredicateStub = 0x0042bbb0; // returns false

// The role of each slot, established from the job bodies of
// docs/FIXED_STEP_JOB_WAVES.md and from every override read in this packet.
enum class TickElementSlot : std::uint8_t {
    Destroy,     // +0h   scalar deleting dtor
    PlacePose,   // +4h   float: put the visual pose at t into the current step
    AdvanceSim,  // +8h   float: advance the simulation by the step
    CommitPose,  // +0Ch  void: store the pose as this step's fixed-step pose
    Unused,      // +10h  float, no override in this build
    Predicate,   // +14h  bool, default false, no override in this build
};

std::size_t tick_element_slot_offset(TickElementSlot slot) noexcept;

// Wave 1 calls PlacePose(0.05f) then CommitPose(); the interpolation wave calls
// PlacePose(leftover) and nothing else; wave 3 calls AdvanceSim(0.05f). Wave 2
// touches no slot of the element itself (it drains the sub-list).
bool tick_element_wave_calls(JobWavePhase phase, TickElementSlot slot) noexcept;

// ---------------------------------------------------------------------------
// The concrete vtables
// ---------------------------------------------------------------------------

struct TickElementVtable {
    std::uint32_t address;
    std::uint32_t slot[kTickElementVtableSlots];
};

// True when the table's slot is not the base stub.
bool tick_element_slot_is_override(const TickElementVtable& table, TickElementSlot slot) noexcept;

enum class TickElementCoverage : std::uint8_t {
    Complete,       // every overriding slot body read end to end
    Partial,        // at least one overriding slot body only partly read
    ContractUnread, // an overriding slot body is another packet's
};

struct TickElementClassRow {
    const char* name;            // hypothesis
    std::uint32_t constructor;   // the routine that installs the vtable
    std::uint32_t vtable;
    std::size_t node_offset;     // where the element sub-object sits in the object
    int group;                   // the value element+14h has at the first splice
    int class_id;                // the dword at object+C4h, where the class has one
    TickElementCoverage coverage;
};

inline constexpr int kTickElementClassCount = 8;
std::size_t tick_element_class_count() noexcept;
const TickElementClassRow& tick_element_class(std::size_t index) noexcept;
const TickElementVtable& tick_element_vtable(std::size_t index) noexcept;

// ---------------------------------------------------------------------------
// Unit, node at unit+310h, group 1
//
// Slot +4h is BSP_UnitInstance_RunShiftedControllerUpdate 00811AB0, already
// reconstructed as unit_shifted_update_00811ab0 in src/unit_rudder.cpp; it is
// not repeated here. Slot +8h is 00953CC0 at level 4 and
// BSP_UnitInstance_UpdateShipMotion 00825F20 at levels 5 and 6, which belongs
// to another packet and is a contract here.
// ---------------------------------------------------------------------------

// 006D1FC0, RET (no argument), body 006D1FC0..006D1FEE.
struct MatrixCopyRequest {
    std::size_t destination_offset; // from the element node
    std::size_t source_offset;      // from the element node
};

// The commit is unconditional; only the source changes.
inline constexpr std::size_t kUnitTickCommitDestination = 0x364;  // unit+674h
inline constexpr std::size_t kUnitTickCommitLiveSource = 0x29c;   // subtracted: unit+74h
inline constexpr std::size_t kUnitTickCommitAlternateSource = 0x1d0; // unit+4E0h
inline constexpr std::size_t kUnitTickAlternateSourceFlag = 0x1c8; // byte, unit+4D8h

MatrixCopyRequest unit_tick_commit_pose_006d1fc0(bool use_alternate_source) noexcept;

// 00953CC0, __thiscall void(node, float step), RET 4, body 00953CC0..00953D9D.
struct UnitTickAdvanceState {
    std::int32_t notify_code_528h = -1; // node+218h; the virtual runs when >= 0
    std::int32_t flag_634h = 0;         // node+324h
    bool gate_byte_61h = false;         // node-2AFh
    float timer_6f8h = 0.0f;            // node+3E8h, counts down
    float timer_6fch = 0.0f;            // node+3ECh, counts down
    std::int32_t role_1ach = 8;         // unit+1ACh, 8 skips the role check
    bool enabled_520h = true;           // node+210h byte
};

struct UnitTickAdvanceHost {
    virtual ~UnitTickAdvanceHost() = default;

    // 00953CDF: unit->vtable[+5Ch](9), reached only when node+218h >= 0.
    virtual void unit_virtual_5c_notify(int code) = 0;
    // 00953CFD: unit->vtable[+1F0h](step). Always runs.
    virtual void unit_virtual_1f0_advance(float step) = 0;
    // 00953D6E: 00927F10(unit, role), skipped when unit+1ACh == 8. False
    // clears the node+210h byte.
    virtual bool unit_role_still_available_00927f10(int role) = 0;
    // 00953D98: unit->vtable[+1D8h](step), only while node+210h is set.
    virtual void unit_virtual_1d8_advance(float step) = 0;
};

void unit_tick_advance_sim_00953cc0(UnitTickAdvanceState& state, float step,
                                    UnitTickAdvanceHost& host);

// ---------------------------------------------------------------------------
// Plane squadron, node at squadron+310h, group 3
//
// Slots +4h and +0Ch stay the base stubs: a squadron carries no pose, so it is
// invisible to wave 1 and to the interpolation wave. Only slot +8h is an
// override. 007F3BA0, __thiscall void(node, float step), RET 4, body
// 007F3BA0..007F3D30 (no Ghidra function).
// ---------------------------------------------------------------------------

inline constexpr int kSquadronTickTimerCount = 4;
inline constexpr float kSquadronTickMoraleRate = 0.25f; // 00D7A348, a double

struct SquadronTickTimers {
    // node+6Ch/70h/74h/78h, each gated by the byte at node+7Ch/7Dh/7Eh/7Fh.
    // A set gate byte freezes its timer. Listing order is 70h, 6Ch, 74h, 78h.
    float value[kSquadronTickTimerCount] = {0.0f, 0.0f, 0.0f, 0.0f};
    bool frozen[kSquadronTickTimerCount] = {false, false, false, false};
};

struct SquadronTickState {
    SquadronTickTimers timers;
    bool notify_flag_3ech = false;  // node+DCh
    float morale_3e8h = 1.0f;       // node+D8h, approaches 1 at 0.25/second
    float period_3c0h = 1.0f;       // node+B0h
    float countdown_3c4h = -1.0f;   // node+B4h
    bool status_3b8h = false;       // node+A8h, copied into node+A0h
    bool status_3b0h = false;       // node+A0h
    std::uint32_t marker_3b4h = 0;  // node+A4h
    std::int32_t rank_2ech = 0;     // node-24h, pushed down onto each member
    std::int32_t member_count = 0;  // node+BCh
    float finalize_gate_308h = 0.0f; // node-8h; the tail runs when non-zero
};

struct SquadronMemberView {
    std::int32_t rank_2ech = 0;      // member+2ECh, raised to the squadron's
    bool active_904h = false;        // member+904h
    std::int32_t state_900h = 0;     // member+900h; 1 with active_904h retires it
    std::uint32_t marker_source = 0; // member+BF4h -> [+4h] -> [+7Ch]
};

struct SquadronTickHost {
    virtual ~SquadronTickHost() = default;

    // 007F3C02: 007EE7F0(squadron, 0), only while node+DCh is set.
    virtual void squadron_notify_007ee7f0(int argument) = 0;
    // 007F3C5B: 007EE790(squadron), on the step the countdown expires.
    virtual void squadron_periodic_007ee790() = 0;
    // The member array is [node+C0h][index], node+BCh entries.
    virtual SquadronMemberView member(std::int32_t index) = 0;
    virtual void set_member_rank(std::int32_t index, std::int32_t rank) = 0;
    // 007F3CA0: 007B8AD0(member), only while member+904h is set. The result is
    // OR-ed into node+A0h.
    virtual bool member_update_007b8ad0(std::int32_t index) = 0;
    // 007F3CD9 then 007F3CE7: 00926D90(member, 5) then 007F3970(squadron,
    // member, 0). The pair retires a member and shrinks the array.
    virtual void member_release_00926d90(std::int32_t index, int code) = 0;
    virtual void squadron_drop_member_007f3970(std::int32_t index, int flag) = 0;
    // 007F3D1C: 0077A650(squadron), only when node-8h is not 0.0f.
    virtual void squadron_finalize_0077a650() = 0;
};

void squadron_tick_advance_sim_007f3ba0(SquadronTickState& state, float step,
                                        SquadronTickHost& host);

// ---------------------------------------------------------------------------
// Projectile, node at projectile+244h, group 0
//
// All three per-step slots are overridden: 006E6750 (+4h), 006E6490 (+8h),
// 006E7D50 (+0Ch). 0070CAE0 installs a second table, 00CFD554, that keeps
// 006E6750 and 006E7D50 and replaces +8h with 0070C370 (not read).
// ---------------------------------------------------------------------------

struct TickPoint3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// 006E7D50, RET, body 006E7D50..006E7D96. Pure: the node carries the two
// positions the interpolation reads.
struct ProjectilePoseSnapshot {
    TickPoint3 previous; // node-68h..-60h, projectile+1DCh
    TickPoint3 current;  // node-74h..-6Ch, projectile+1D0h
};

struct ProjectileSnapshotHost {
    virtual ~ProjectileSnapshotHost() = default;
    // 006E7D74: BSP_EntityPose_RefreshWorld(projectile), only when the byte at
    // projectile+C8h is clear.
    virtual void refresh_world_pose_00414db0() = 0;
    // projectile+FCh..104h, the world matrix translation row, read after the
    // refresh.
    virtual TickPoint3 world_translation() = 0;
    virtual bool world_pose_valid() = 0; // byte projectile+C8h
};

void projectile_tick_commit_pose_006e7d50(ProjectilePoseSnapshot& snapshot,
                                          ProjectileSnapshotHost& host);

struct ProjectileTickState {
    float flight_time = 0.0f;      // node-80h, projectile+1C4h
    float class_time_scale = 1.0f; // [projectile+174h]+5Ch
    float class_max_life = 0.0f;   // [projectile+174h]+54h
    bool tracer_enabled = false;   // byte projectile+5Ch
    bool world_pose_valid = false; // byte projectile+C8h
    ProjectilePoseSnapshot snapshot;
};

struct ProjectileTickHost {
    virtual ~ProjectileTickHost() = default;

    // 006E6774 / 006E64C2: [projectile+170h]->vtable[+2Ch](), the motion-mode
    // query that picks which of the two body virtuals runs.
    virtual bool motion_mode_alternate_2c() = 0;
    // 006E679E: projectile->vtable[+118h] when the query is true, else
    // vtable[+114h]; the scaled step is the only argument.
    virtual void projectile_place_pose(bool alternate, float scaled_step) = 0;
    // 006E64EA: projectile->vtable[+120h] / [+11Ch], same rule.
    virtual void projectile_advance(bool alternate, float scaled_step) = 0;
    // 006E67B5 / 006E652D / 006E7D74: BSP_EntityPose_RefreshWorld(projectile).
    virtual void refresh_world_pose_00414db0() = 0;
    virtual TickPoint3 world_translation() = 0;
    // 006E67C8: [[node+3Ch]+0Ch]->vtable[+34h](projectile+CCh), the attached
    // scene node taking the refreshed matrix. Skipped when node+3Ch is null.
    virtual bool has_attached_node() = 0;
    virtual void attached_node_set_transform_34() = 0;
    // 006E657F: 0084C430, the tracer segment from the snapshot's previous
    // position to the refreshed world translation. Only while projectile+5Ch.
    virtual void emit_tracer_0084c430(const TickPoint3& from, const TickPoint3& to,
                                      bool world_mode_flag) = 0;
    // 006E64FA: [00E188A8]+1FE4h == 2 clears the flag the tracer receives.
    virtual bool world_mode_is_two() = 0;
    // 006E659F then 006E65A9: 00696350(projectile, 0) then
    // 00926D90(projectile, 2), the expiry pair.
    virtual void projectile_expire_00696350() = 0;
    virtual void projectile_release_00926d90(int code) = 0;
};

// 006E6750, __thiscall void(node, float step), RET 4, body 006E6750..006E67CC.
void projectile_tick_place_pose_006e6750(ProjectileTickState& state, float step,
                                         ProjectileTickHost& host);

// 006E6490, __thiscall void(node, float step), RET 4, body 006E6490..006E65B4.
void projectile_tick_advance_sim_006e6490(ProjectileTickState& state, float step,
                                          ProjectileTickHost& host);

// ---------------------------------------------------------------------------
// Tickable game entity, node at entity+170h, group 0
//
// Slot +4h is 00929CB0 (complete), slot +8h is 0092B350 (the x87 body is not
// projected), slot +0Ch stays the base stub.
// ---------------------------------------------------------------------------

// 00929CB0, __thiscall void(node, float step), RET 4, body 00929CB0..00929D5F.
struct GameEntityTickHost {
    virtual ~GameEntityTickHost() = default;

    // node+1E4h, the physics body. Null skips the whole body.
    virtual bool has_physics_body() = 0;
    // 00929CEA then 00929CF1: 00C43EA0 (already reconstructed as
    // interpolate_body_transform_00c43ea0) then 00C33650, both physics-library
    // contracts. alpha is step / 0.05f.
    virtual void interpolate_body_transform_00c43ea0(float alpha) = 0;
    // 00929D07: BSP_Vector3f_TransformDirectionOptionalNormalize with the pivot
    // at node+1F4h and no normalise.
    virtual TickPoint3 transform_pivot_0042d0d0() = 0;
    // 00929D41: 00741E90(entity, matrix), the entity taking the interpolated
    // matrix with the pivot subtracted from its translation row.
    virtual void entity_set_interpolated_matrix_00741e90(const TickPoint3& translation) = 0;
    // 00929D56: [node+1E0h]->vtable[+34h](matrix), the attached scene node.
    virtual void attached_node_set_transform_34(const TickPoint3& translation) = 0;
    // The translation row of the interpolated matrix, before the pivot.
    virtual TickPoint3 interpolated_translation() = 0;
};

float game_entity_tick_alpha(float step) noexcept; // step / 0.05f, 00929CD2

void game_entity_tick_place_pose_00929cb0(float step, GameEntityTickHost& host);

} // namespace bsp
