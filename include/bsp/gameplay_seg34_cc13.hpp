#pragma once

// Segment 34 (006FA540-00701AD0) reference models: the released "parachute
// target" chain and the dummy submarine release. Packet cc13-gpr34;
// docs/CC13_GPR_34.md.
//
//   MDummyTarget (bullet sub-type 0Ch, class record 00CFA5CC, reader 00701060):
//     a bomb-family round. Its fixed-step slot 00701460 keeps the round upright,
//     runs the shared bomb tick 006E1300 and, once OpenAfterTime has passed since
//     the release and the round falls faster than OpenAtFallingSpeed, spawns a
//     DummyTargetVehicle with the round's velocity.
//   DummyTargetVehicle (class id 35h, 740h bytes, vtable 00CFCD60, tick element
//     at +310h with table 00CFCD1C): the chute. 00700660 opens it over OpenTime
//     and slows all three velocity axes by SlowingFactor * openState; 00700F70
//     moves the pose by velocity * dt with the up row forced; 00700770 drives the
//     chute animation and fades the body out below 5 HP.
//   MDummySubmarine (bullet sub-type 0Eh, class record 00CFA678, reader
//     00700050): the fixed-step body at 006FFA40 (no Ghidra function; vtable
//     00CFCAC8 holds it) gives the dropped round +4 m/s upward on release and,
//     after OpenAfterTime, creates the submarine unit (004F05F0) with the
//     round's velocity and reports it to the launching unit.
//
// This installation's rows (scripts/datatables): bulletclasses.lua realistic
// row 142 "Releaseable target" (OpenAfterTime 3, OpenAtFallingSpeed 40,
// DummyTargetVehicleClass 790) and vehicleclasses.lua "Parachute target -
// tutorial" (OpenTime 1, SlowingFactor 2, HP 1).
//
// These are semantic interfaces read from the disk bytes (Capstone) and the
// Ghidra pseudocode, not native layouts or ABI-compatible replacements. Field
// names are hypotheses; the "_NNN" suffix is the native byte offset from the
// entity base E (the tick elements are E+310h and E+3E4h). Every engine
// service the image calls is a host callback, so nothing here invents a
// global or stubs a call. Main-thread x87 runs at 24-bit precision
// (docs/CC13_FPU_PRECISION.md), so the arithmetic is float. Build-tested only;
// not wired, not game-run.
//
// ADOPTION NOTE (cc9 terms; the full table is in docs/CC13_GPR_34.md):
// - Replaces: nothing that exists. No host models MDummyTarget, the class-35h
//   vehicle or MDummySubmarine. A rack holding one of these rounds goes through
//   GameGunneryHost::release_bomb_drop (src/game_hosts_gunnery.cpp:13387) as a
//   plain bomb; the class-35h unit only appears in kind tests
//   (src/game_hosts_gunnery.cpp:6211, src/game_hosts_ship_ai.cpp:7756,
//   src/game_hosts_script_orders.cpp:1455, src/local_player_unit_lists.cpp:168).
//   The sister round MDummyKamikazePlane (006FEC00) is already hosted in
//   src/kamikaze_task_host.cpp and is the template for binding these.
// - Inputs: the bullet row (OpenAfterTime, OpenAtFallingSpeed or the submarine's
//   OpenAfterTime, the spawned class), the vehicle row (OpenTime,
//   SlowingFactor), the round's velocity and pose, the fixed-step clock
//   [00F876A4], the network mode [[00E188A8]+1FE4h] and the owner chain
//   (00923810) the gunnery host already walks for the kamikaze round.
// - RNG: none. No routine modelled here calls 00BD2F10 or any other generator.
// - OFF-switch prediction: with the switch off nothing changes. With it on, a
//   released target round becomes a falling crate whose chute opens 1 s after it
//   spawns and whose sink rate settles at 9.81 * (1 - SlowingFactor * dt) /
//   SlowingFactor once open (the fixed point of v = (v - 9.81 dt)(1 - SF dt));
//   with this installation's SlowingFactor 2 that is about 4.9 m/s at small dt. Ships and guns that already test kind 35h then see a live target. A
//   released dummy-submarine round creates a submarine after OpenAfterTime (1 s
//   default) and adds it to its launcher (message VEHICLE_ADD_LAUNCHED, kind 7).
//   Stream 0 is untouched by either, unless the spawned units draw on their own.

#include <array>
#include <cstdint>
#include <functional>

namespace bsp {
namespace gameplay_seg34_cc13 {

struct Vec3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// ---------------------------------------------------------------------------
// The DummyTargetVehicle chute (class id 35h).
// ---------------------------------------------------------------------------

// Vehicle class fields read by 00700E40 (the reader the ledger calls
// BSP_DoorClass_ReadLuaFields; only the dummy target vehicle row carries these
// keys, and 00700660 reads them from the descriptor [E+538h]).
struct DummyTargetVehicleClassFields {
    float open_time_138{1.0f};       // "OpenTime", required (00700330 at 00700F2B)
    float slowing_factor_13c{0.0f};  // "SlowingFactor" (007003B0 at 00700F4E)
};

struct DummyTargetVehicleState {
    float hp_a8{0.0f};               // E+A8h, the unit's hit points
    float open_state_730{0.0f};      // E+730h, 0 closed .. 1 open ("openState")
    Vec3 velocity_734;               // E+734h..73Ch ("velocity"; "Vel3D" at spawn)
};

struct DummyTargetVehicleHost {
    // 00953CC0, the unit game-object advance, called first with the same dt.
    std::function<void(float dt)> base_tick_advance;
};

// 00700660, slot +8h of the tick table 00CFCD1C. __thiscall(E+310h, float dt),
// RET 4. Returns true when the image calls 00926D90(E, 2) (HP below zero).
bool dummy_target_vehicle_tick_advance_00700660(DummyTargetVehicleState& s,
                                                const DummyTargetVehicleClassFields& cls,
                                                float dt,
                                                const DummyTargetVehicleHost& host);

struct DummyTargetVehiclePoseHost {
    // 004134F0(E+74h, E+674h): the pose matrix takes the step matrix.
    std::function<void()> restore_step_matrix;
    // Row 1 (+10h..18h) of the pose matrix := the globals 00F8758C..00F87594.
    std::function<void()> force_up_row;
    // 0085DAD0 on E+74h.
    std::function<void()> orthonormalize_from_row_y;
    // Translation row E+A4h..ACh (pose matrix +30h).
    std::function<Vec3()> read_translation;
    std::function<void(const Vec3&)> write_translation;
    // Clears the bytes E+C8h and E+10Ch, runs 0042ED50 on each child in the
    // E+48h list (next at +44h), then E->vtable[+D8h]().
    std::function<void()> invalidate_pose;
};

// 00700F70, slot +4h of the tick table 00CFCD1C. __thiscall(E+310h, float dt),
// RET 4.
void dummy_target_vehicle_commit_pose_00700f70(const DummyTargetVehicleState& s, float dt,
                                               const DummyTargetVehiclePoseHost& host);

struct DummyTargetVehicleUpdateHost {
    // 00956600, the unit instance timers.
    std::function<void(float dt)> base_update_timers;
    // 00B6DC90(0, 1) on the animation object [E+4A4h]: the chute animation length.
    std::function<float()> chute_animation_length;
    // [[E+4A4h]]->vtable[+24h](0, t).
    std::function<void(float t)> set_chute_animation_time;
    // 00B6DA70(factor, 1) on [E+4A4h].
    std::function<void(float factor)> set_visibility_factor;
};

// 00700770, slot +DCh of 00CFCD60. __thiscall(E, float dt), RET 4.
// Presentation only: the chute animation and the low-HP fade.
void dummy_target_vehicle_update_00700770(const DummyTargetVehicleState& s, float dt,
                                          const DummyTargetVehicleUpdateHost& host);

// ---------------------------------------------------------------------------
// The MDummyTarget round's fixed step: 00701460.
// ---------------------------------------------------------------------------

struct DummyTargetBulletClassFields {
    float open_at_falling_speed_d8{0.0f};  // "OpenAtFallingSpeed", default 0 (00701088)
    float open_after_time_dc{2.0f};        // "OpenAfterTime", default 2.0 (00CE3958)
    // +E0h: the DummyTargetVehicleClass record (00964790 on the named row).
};

struct DummyTargetBulletState {
    float launch_time_480{-1.0f};  // E+480h; 006FF570 sets -1.0 (00D7A260) on activate
    bool spawned_vehicle_49c{false};  // E+49Ch, the observer's target (E+488h + 14h)
    Vec3 velocity_318;             // E+318h..320h, the bomb-family velocity
    std::int32_t party_54{0};      // E+54h, copied from the unit ancestor by 00700520
    std::int32_t race_58{0};       // E+58h
    std::int32_t dummy_object_id_484{0};  // E+484h
};

struct DummyTargetSpawnRequest {
    std::int32_t race{0};
    std::int32_t party{0};          // the round's party inverted: (E+54h == 0)
    std::int32_t dummy_object_id{0};
    std::int32_t owner_player{8};
    Vec3 vel3d;
};

struct DummyTargetBulletHost {
    // 00923810(E, 1) != 0: the round is still held by its owner chain.
    std::function<bool()> attached;
    float now_f876a4{0.0f};          // [00F876A4], the fixed-step clock
    bool network_client{false};      // [[00E188A8]+1FE4h] == 2
    // [[E+464h]+0Ch]->vtable[+24h](0, 0.0f, 1): an animation reset (unread).
    std::function<void()> reset_round_animation;
    // The same three steps 00700F70 uses on E+74h (00F8758C row, 0085DAD0).
    std::function<void()> force_up_row;
    std::function<void()> orthonormalize_from_row_y;
    // 006E1300 on E+3E4h with the step; it may change velocity_318.
    std::function<void(float dt, DummyTargetBulletState&)> bomb_tick;
    // E->vtable[+10h](0, E+74h, bag), 00701170 with the class at classDesc+E0h,
    // the bag reference at vehicle+C0h, and 00700940 on E+488h.
    std::function<void(const DummyTargetSpawnRequest&)> spawn_vehicle;
};

struct DummyTargetBulletStep {
    bool stamped_launch{false};
    bool spawned{false};
};

// 00701460. __thiscall(E+3E4h, float dt), RET 4.
DummyTargetBulletStep dummy_target_bullet_fixed_step_00701460(DummyTargetBulletState& s,
                                                             const DummyTargetBulletClassFields& cls,
                                                             float dt,
                                                             const DummyTargetBulletHost& host);

// ---------------------------------------------------------------------------
// The MDummySubmarine round's fixed step: 006FFA40 (no Ghidra function).
// ---------------------------------------------------------------------------

struct DummySubmarineBulletClassFields {
    float open_after_time_d8{1.0f};  // "OpenAfterTime", default 1.0 (00700078 FLD1)
    // +DCh: the "SubmarineClass" record; its +54h is the type id 004F05F0
    // takes in EDX and its +70h is the "Type" property.
    std::int32_t submarine_type_id_54{0};
    std::int32_t submarine_type_70{0};
};

struct DummySubmarineBulletState {
    float launch_time_480{-1.0f};    // E+480h
    float arming_45c{0.0f};          // E+45Ch (tick element +78h), the sweep countdown
    bool spawned_unit_49c{false};    // E+49Ch
    bool launcher_4b4{false};        // E+4B4h, the observer's target (E+4A0h + 14h)
    bool launcher_is_unit{false};    // [E+4B4h]->vtable[+5Ch](5)
    Vec3 velocity_318;
    std::int32_t party_54{0};
    std::int32_t race_58{0};
    std::int32_t owner_player_180{0};       // E+180h
    std::int32_t skill_4b8{0};              // E+4B8h
    std::int32_t dummy_object_id_484{0};    // E+484h
    bool has_attack_target_47c{false};      // E+47Ch non-null
    std::uint16_t attack_target_word_174{0};  // [[E+47Ch]+174h]
};

struct DummySubmarineSpawnRequest {
    std::int32_t type_id{0};         // EDX for 004F05F0
    std::int32_t type{0};            // "Type"
    std::int32_t skill{0};           // "Skill"
    std::int32_t race{0};            // "Race"
    std::int32_t party{0};           // "Party"
    std::int32_t owner_player{0};    // "OwnerPlayer"
    Vec3 vel3d;                      // "Vel3D"
    float start_speed{0.0f};         // "StartSpeed" = 0042B2F0(|velocity|)
    float invincible{0.0f};          // "Invincible", a float property set to 0.0
    bool has_auto_attack_target{false};
    std::int32_t auto_attack_target{0};  // "AutoAttackTarget"
    std::int32_t dummy_object_id{0};     // "DummyObjectID"
    std::int32_t target_dive{1};         // "TargetDive"
    std::int32_t fire_stance{2};         // "FireStance"
    bool notify_launcher{false};         // VEHICLE_ADD_LAUNCHED (006EB9C0) via 0077C2A0(msg, 7, 0)
};

struct DummySubmarineBulletHost {
    std::function<bool()> attached;          // 00923810(E, 1) != 0
    float now_f876a4{0.0f};
    bool network_client{false};
    // The owner-chain climb to the first vtable[+5Ch](5) unit (or the last
    // owner) and 00694A60/006952A0 on the observer at E+4A0h.
    std::function<void(DummySubmarineBulletState&)> bind_launcher;
    std::function<void(float dt, DummySubmarineBulletState&)> bomb_tick;  // 006E1300
    // 0042B2F0 on the velocity: length, 0 at or below 1e-10.
    std::function<float(const Vec3&)> length;
    std::function<void(const DummySubmarineSpawnRequest&)> spawn_submarine;
};

struct DummySubmarineBulletStep {
    bool stamped_launch{false};
    bool spawned{false};
};

// 006FFA40. __thiscall(E+3E4h, float dt), RET 4.
DummySubmarineBulletStep dummy_submarine_fixed_step_006ffa40(
    DummySubmarineBulletState& s, const DummySubmarineBulletClassFields& cls, float dt,
    const DummySubmarineBulletHost& host);

}  // namespace gameplay_seg34_cc13
}  // namespace bsp
