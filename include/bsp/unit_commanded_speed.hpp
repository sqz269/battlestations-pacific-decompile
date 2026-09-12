// The commanded speed on the navigator parameter block at *(unit+73Ch), its two
// producers, and the weapon director's command-stage ladder.
//
// Packet cc_commanded_speed. Every name below is a hypothesis, not a recovered
// symbol. docs/UNIT_COMMANDED_SPEED.md carries the addresses, the evidence and
// the uncertainty for each rule; docs/CRUISE_COMMAND.md carries the order path
// that consumes the pair.
#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/cruise_command.hpp"
#include "bsp/weapon_director.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The navigator parameter block at *(unit+73Ch)
// ---------------------------------------------------------------------------
// The block is a plain heap struct, not a polymorphic object: 00822B70 writes a
// float straight into its +0h, so there is no vtable there. 0081F283..0081F28C
// allocates it in BSP_UnitVehicleBase_Construct and stores the pointer at
// unit+73Ch; 0081F4A7..0081F4B2 frees it in the unit's vector deleting
// destructor. 00818920 and 0082007B register it with the property system under
// the literal name "navigatorParams" (00D094C4), which is where the names here
// come from. It is four bytes past the weapon director pointer at unit+738h and
// is a separate allocation, not part of the director.
inline constexpr std::size_t kUnitOffNavigatorParams = 0x73c;  // 0081F28C

// Offsets with a producer read in this packet. +0h..+18h are refilled from the
// game tuning singleton by 00822B70 and are not interpreted here.
inline constexpr std::size_t kNavigatorParamsOffTuningFirst = 0x00;   // 00822B95
inline constexpr std::size_t kNavigatorParamsOffTuningLast = 0x18;    // 00822C04
inline constexpr std::size_t kNavigatorParamsOffAllowMaxDepth = 0x20; // 008A33FF
inline constexpr std::size_t kNavigatorParamsOffMoveClose = 0x21;     // 008A35A4
inline constexpr std::size_t kNavigatorParamsOffCommandedSpeed = 0x24;      // 00890E86
inline constexpr std::size_t kNavigatorParamsOffCommandedSpeedTime = 0x28;  // 00890E97

// The tuning fields 00822B70 copies, GameSettings +160h..+178h -> block +0h..+18h.
inline constexpr std::size_t kGameSettingsOffNavigatorFirst = 0x160;  // 00822B99
inline constexpr std::size_t kGameSettingsOffNavigatorLast = 0x178;   // 00822C0E

// The commanded-speed pair itself is `CruiseSpeedSetting` in bsp/cruise_command.hpp:
// `.speed` is +24h and `.enable` is +28h. This packet establishes what `.enable`
// holds. It is not a boolean and not a flag: the two producers store the mission
// clock DAT_00F876A4 into it (00890E8B, 008A3906), and 00835C28 subtracts it from
// the clock and compares the difference with 1.0f. `kCruiseSpeedSettingInactive`
// (-1.0f, 00D7A260) is the "no speed commanded" value the constructor, the ship
// AI and the director's stage reset all store.
//
// The consumers only ever test `>= 0.0f` (009E12AC, 00836AC1, 00836E59), so a
// clock of 0.0f at mission start still reads as active.

// The age after which 00835BF0 drops a commanded speed, 00835C3C `FLD1`.
inline constexpr float kNavigatorCommandedSpeedMaxAge = 1.0f;

// 009E12AC / 00836AC1 / 00836E59: `COMISS enable, 00D7A218` with 00D7A218 = 0.0f,
// taking the active branch when the compare is not below.
bool navigator_commanded_speed_active(const CruiseSpeedSetting& setting) noexcept;

// 00890E6F..00890E97 in luaMW_SetShipSpeed and 008A38D5..008A3912 in
// luaMW_NavigatorMoveOnPath, byte for byte the same store:
//   +24h = (requested < 0.0f) ? 0.0f : requested
//   +28h = DAT_00F876A4, the mission clock
// The negative clamp is the `FLDZ / FCOMPI ST(1) / JBE` pair at 00890E73 and
// 008A38D9. Neither binding touches any other field of the block.
CruiseSpeedSetting navigator_commanded_speed_store_00890e6f(float requested,
                                                            float mission_clock) noexcept;

// 00835C28..00835C44: `(float)(int)clock - commanded_at > 1.0f`. The truncation
// is real - `CVTTSS2SI` at 00835C28 converts the clock to a signed int and
// `FILD` at 00835C34 converts it back before the subtract - so the age is
// measured against the whole-second floor of the clock, not the clock itself.
bool navigator_commanded_speed_stale_00835c28(float mission_clock,
                                              float commanded_at) noexcept;

// ---------------------------------------------------------------------------
// The weapon director's command stages
// ---------------------------------------------------------------------------
// 0071BE60 counts the filled command slots. The array is the one
// `kDirectorOffCommandSlots` (0x54) and `kDirectorCommandSlotStride` (0x1Ch)
// already name in bsp/weapon_director.hpp, which is why 008369DB reaches slot i
// as element i+3 of a 1Ch-byte array from the object base: 0x54 == 3 * 0x1C.
// The walk stops at the first null slot or at `kDirectorCommandSlotCount`.
//
// The primary stage is `kCruiseDirectorOffCommandStage` (0x48) and the primary
// command is the first slot's command word; the secondary pair is
// `kCruiseDirectorOffOverrideCommand` (0x188) and the offset below. The unit is
// `kDirectorOffOwnerUnit` (0x24Ch).
inline constexpr std::size_t kDirectorOffSecondaryCommandStage = 0x50;  // 0071D9E0

// The two stage-raising helpers only ever move a stage up, and only stage 2
// sends the completion message (0071D82D, 0071D9FD).
inline constexpr int kDirectorCommandStageIdle = 0;
inline constexpr int kDirectorCommandStageRunning = 1;
inline constexpr int kDirectorCommandStageFinished = 2;

// The vtable at 00D09F58, the derived weapon director. The base vtable is
// 00D09EC0 and its slot 6Ch is 0071C130, which 00835BF0 calls directly.
inline constexpr std::size_t kDirectorVtableSlotResetCommandStage = 0x6c;  // 00D09FC4
inline constexpr std::size_t kDirectorVtableSlotRetargetCommand = 0x70;    // 00D09FC8

// The command class pointers this packet reads, from docs/SCENE_COMMAND_TYPES.md.
inline constexpr std::uint32_t kCommandedSpeedFollowObject = 0x00e08f60u;      // 00836ADC
inline constexpr std::uint32_t kCommandObjectCruise = 0x00e08f70u;      // 00836E8B
inline constexpr std::uint32_t kCommandObjectAttackMove = 0x00e08f78u;  // 00836B45
inline constexpr std::uint32_t kCommandObjectMoveOnPath = 0x00e08f80u;  // 00836BF0
inline constexpr std::uint32_t kCommandObjectStop = 0x00e08f88u;        // 00836A8E

// The squared planar distance at which a queued weapon command abandons the
// running movement command, the double at 00D09FE8 = 4.0e6 (2000 m).
inline constexpr double kDirectorWeaponTargetAbandonDistanceSq = 4000000.0;  // 00836A6C

// The director state the stage rules read. Plain field reads, not call sites.
struct WeaponDirectorCommandState {
    int primary_stage{0};                 // director +48h
    int secondary_stage{0};               // director +50h
    std::uint32_t primary_command{0};     // director +54h
    std::uint32_t secondary_command{0};   // director +188h
    std::uint32_t unit{0};                // director +24Ch, read at 00836972 / 00836E3F
    bool unit_player_controlled{false};   // unit +184h, read at 00836978 / 00836AAD / 00836E45
};

// Which default command the idle tail re-issues.
enum class DirectorDefaultCommand : std::uint32_t {
    None = 0u,
    Follow = kCommandedSpeedFollowObject,
    Stop = kCommandObjectStop,
    Cruise = kCommandObjectCruise,
};

// ---------------------------------------------------------------------------
// The host boundary
// ---------------------------------------------------------------------------
// One pure-virtual method per native call site the sequences below make. Field
// reads travel in `WeaponDirectorCommandState` and `CruiseSpeedSetting` instead,
// so nothing here stands for a plain load.
class WeaponDirectorStageHost {
public:
    virtual ~WeaponDirectorStageHost() = default;

    // 0071BE60, __fastcall(director), body 0071BE60-0071BE87, complete: walks
    // director+54h with stride 1Ch and returns the index of the first null slot,
    // capped at 10. Call sites 0083694E, 0083699A, 00836A9D, 00836B29, 00836DD6,
    // 00836DED.
    virtual int director_filled_command_slots_0071be60() = 0;

    // 0071D810, __thiscall(director)(int stage), RET 4, body 0071D810-0071D87D:
    // raises director+48h to `stage` only when it is currently lower, and on
    // stage 2 outside session mode 2 routes a completion message built by
    // 0071C730(1,0). Call sites 00836985, 00836A7C, 00836AD2, 00836B3B,
    // 00836BB6, 00836BE6, 00836D12.
    virtual void director_raise_primary_stage_0071d810(int stage) = 0;

    // 0071D9E0, __thiscall(director)(int stage), RET 4: the same rule on
    // director+50h, with 0071C730(0,0) as the message. Call sites 00836993,
    // 00836DC4.
    virtual void director_raise_secondary_stage_0071d9e0(int stage) = 0;

    // 0071C130, __thiscall(director)(char primary), body 0071C130-0071C14D,
    // complete: `primary` clears director+44h and director+48h, otherwise
    // director+4Ch and director+50h. Call site 00835BFA.
    virtual void director_clear_command_slot_0071c130(bool primary) = 0;

    // 00822B70, __thiscall(unit)(char reset), body 00822B70-00822C15, complete:
    // when `reset` is non-zero it sets the block's +20h to 1 and refills +0h..+18h
    // from the game tuning singleton 00424C40 +160h..+178h; when it is zero the
    // whole body is skipped. Call site 00835C65, which passes a literal 0.
    virtual void navigator_params_reset_from_tuning_00822b70(bool reset) = 0;

    // The store into the block's +28h at 00835C54 and 009E13F8.
    virtual void set_navigator_commanded_speed_time(float value) = 0;

    // The director's own vtable[6Ch] = 00835BF0, invoked indirectly at 00836E0B
    // with a literal 1.
    virtual void director_reset_command_stage_vtable6c(bool primary) = 0;

    // 007788B0 BSP_Entity_ControllerBelongsToAnother, __fastcall(entity), body
    // 007788B0-007788C7, complete: `ctrl = [entity+284h]; ctrl != 0 && [ctrl+14h]
    // != entity`, i.e. the unit is in a group someone else owns. Call site
    // 00836E13.
    virtual bool unit_controller_belongs_to_another_007788b0() = 0;

    // 007788D0, __fastcall(entity), body 007788D0-007788E4, complete: returns
    // `[entity+284h] ? [[entity+284h]+14h] : 0`, the entity that owns the
    // controller. Call sites 00836AF2, 00836B03, 00836B1A, 00836E28.
    virtual std::uint32_t unit_controller_owner_007788d0() = 0;

    // 00465080, __thiscall(target)(object, float range), body 00465080-004650C9,
    // complete: fills a SceneCommandTarget - kind byte 1 with the ordinal at
    // [object+174h] when `object` is non-null, otherwise kind 0 and ordinal 0 -
    // copies the read-only zero vector at 00F87574 into its +8h..+10h, and stores
    // `range` at +14h. Call sites 00836E32, 00836E6D, 00836E85.
    virtual std::uint32_t make_command_target_00465080(std::uint32_t object,
                                                       float range) = 0;

    // 0071ECF0 BSP_WeaponDirector_IssueCommand, __thiscall(director)(command,
    // target), RET 8. Call site 00836E92.
    virtual void director_issue_command_0071ecf0(std::uint32_t command,
                                                 std::uint32_t target) = 0;
};

// ---------------------------------------------------------------------------
// The sequences
// ---------------------------------------------------------------------------

// 00836941..00836997, the step's pre-pass. Coverage: complete.
// Returns the local flag the tail reads at 00836DE0 ([ESP+0Bh]), which is set
// when the director has no primary command and no filled slot at all.
//   flag = (primary_command == 0 && filled == 0)
//   if (primary_stage == 1 && (filled > 1 || player_controlled)) raise primary 2
//   if (secondary_stage == 1) raise secondary 2
bool weapon_director_step_prepass_00836941(const WeaponDirectorCommandState& state,
                                           WeaponDirectorStageHost& host);

// 008369A1..00836A7E, the "a queued weapon command is too far" test. Coverage:
// complete for the stage decision; the two world positions and the squared
// planar distance are supplied by the caller because 00427EB0 and 00521EA0 are
// read by other packets.
//   runs only when filled > 1 and primary_stage < 1 and the primary command's
//   category is neither 1 nor 2 and the last queued command's category is 1 or 2
//   and its target resolves; then raise primary 2 when dx*dx + dz*dz > 4.0e6.
bool weapon_director_abandon_for_far_weapon_target_008369a1(int filled_slots,
                                                            int primary_stage,
                                                            int primary_category,
                                                            int queued_category,
                                                            bool queued_target_resolves,
                                                            double planar_distance_sq,
                                                            WeaponDirectorStageHost& host);

// 00836A8B..00836AD7, the `stop` arm. Coverage: complete. Returns true when it
// raised the primary stage to 2.
//   only when primary_command == 00E08F88 `stop` and primary_stage != 2
//   raise when filled > 1, or the unit is player controlled, or a commanded
//   speed is active. A commanded speed therefore ends a standing `stop`.
bool weapon_director_stop_arm_00836a8b(const WeaponDirectorCommandState& state,
                                       const CruiseSpeedSetting& commanded_speed,
                                       WeaponDirectorStageHost& host);

// 00836DC9..00836EA7, the idle tail that re-issues a default command.
// Coverage: complete.
//   gate: (primary_stage == 2 && filled <= 1) || prepass_flag
//   then: filled > (primary_stage == 2 ? 1 : 0) -> nothing
//   otherwise vtable[6Ch](1) runs first, which is what can expire the commanded
//   speed before the choice below reads it:
//     the controller belongs to another entity -> `follow` on its owner
//     else player controlled or a commanded speed is active -> `cruise`
//     else -> `stop`
//   every one of the three targets is built with a range of 0.0f; `follow` takes
//   the controller's owner as its object, `cruise` and `stop` take the unit.
DirectorDefaultCommand weapon_director_idle_reissue_00836dc9(
        const WeaponDirectorCommandState& state,
        bool prepass_flag,
        const CruiseSpeedSetting& commanded_speed_after_reset,
        WeaponDirectorStageHost& host);

// 00835BF0, director vtable[6Ch], __thiscall(director)(char primary), RET 4,
// body 00835BF0-00835C6D, no Ghidra function. Coverage: complete.
//   0071C130(primary) clears the matching stage pair
//   then, on the block at *(unit+73Ch):
//     primary and active and stale     -> +28h = -1.0f
//     not primary                      -> +28h = -1.0f
//     primary and active and not stale -> kept
//     inactive and primary             -> kept (already -1.0f)
//     inactive and not primary         -> +28h = -1.0f again
//   and when `primary` is set it finally calls 00822B70(unit, 0), which its own
//   body turns into a no-op because the literal argument is zero.
// Returns the pair as it stands after the routine.
CruiseSpeedSetting weapon_director_reset_command_stage_00835bf0(
        bool primary,
        const CruiseSpeedSetting& commanded_speed,
        float mission_clock,
        WeaponDirectorStageHost& host);

// ---------------------------------------------------------------------------
// The ship AI cruise state's arm selection, 009E1188..009E11CE
// ---------------------------------------------------------------------------
// Read only, for the commanded speed. 009E1170 itself is leased to another
// owner and carries no ledger record from this packet.
enum class ShipCruiseStateArm : int {
    Return = 0,    // 009E1188 / 009E1196, a null controller or a null unit+740h
    Detached = 1,  // 009E13B4, the gate at 009E119F rejected
    Player = 2,    // 009E11CE, unit+184h set
    Cruise = 3,    // 009E1265, the AI arm docs/CRUISE_COMMAND.md reconstructs
};
// `gate_accepts` is "[[unit+740h]+50h]+1B0h == 8, or 00927F10 accepted it"
// (009E119F..009E11B4).
ShipCruiseStateArm ship_cruise_state_arm_009e1188(bool controller_present,
                                                  bool unit_740h_present,
                                                  bool gate_accepts,
                                                  bool unit_player_controlled) noexcept;

// The `Detached` arm is the only one that touches the commanded speed: 009E13F8
// stores -1.0f into +28h. The `Player` arm leaves it alone and the `Cruise` arm
// only reads it.
CruiseSpeedSetting ship_cruise_state_commanded_speed_after_arm_009e13f8(
        ShipCruiseStateArm arm, const CruiseSpeedSetting& before) noexcept;

} // namespace bsp
