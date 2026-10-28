#pragma once
// bsp_game.exe milestone 2t: the gun chain, as process bindings.
//
// Addresses: 00864bd0 (the gunnery pass attach onto the unit's tick element
// unit+310h, the object at unit+6DCh), 00864fe0 (BSP_UnitGunneryAi_Tick, the
// two-second think), 00956c20 (the twelve per-category gun lists at unit+394h
// and the ranges at unit+430h), 00727bd0 (the rank table at 00E19BF8 from the
// authored preference lists at 00E092C8), 008624c0 (the director bridge),
// 00863990 / 008633d0 / 00862820 (the candidate gates), 00727f10 / 00728000
// (the per-gun fire-target assign and clear), 0085aba0 (SetTargetAngles),
// 0085ad80 (the aim step), 007f5fc0 / 007f60a0 / 007f6530 (the traverse and
// fire windows and the route around a blocked one), 0072d2c0 (the trigger
// latch gun+454h), 0072d130 (BSP_Gun_FixedStepTick and the 0ADh fire opcode),
// 0072d860 (the 0ADh arm), 00727e30 (FireIfReady), 00729a80 / 0085a830
// (CanFire), 00730160 (BSP_Gun_Fire), 0072cf00 (the barrel reload write),
// 007298d0 (the next ready barrel), 006e8430 (the projectile launch velocity),
// 006e7670 / 006e65c0 (the two integration slots), 0084bf00 (the segment
// sweep), 0084bc60 (the impact), 009239a0 (the queued-hit dispatch),
// 00826f10 (the ship's hit handler), 0092d1f0 (the per-part health
// subtraction), 008777d0 (the base hit record), 00879070 (the damage rule),
// 00877b90 (the health write), 0077ce60 (the damage attribution) and
// 0091bda0 (the kill credit).
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of a bsp:: host already on main - bsp::UnitGunneryPassHost,
// bsp::UnitWeaponCategoryIndexHost, bsp::GunFireRequestHost,
// bsp::ProjectileImpactHost, bsp::ProjectileHitDispatchHost,
// bsp::ShipHitRecordHost and bsp::KillCreditHost - satisfied either by a
// reconstruction already on main or by the explicit unimplemented policy in
// GameHostLog.
//
// What this process does not hold, and what stands in for it, is stated on
// every field below and in the milestone 2t section of docs/GAME_EXECUTABLE.md:
//
//   * There is no model hierarchy, so no gun device exists as a scene node.
//     The guns are built from the authored `VehicleClass[id].Platforms` table
//     that BSP_VehicleClass_ReadLuaFields 009610F6 reads, through the live Lua
//     state the mission machine already holds. Each platform's `Gun[1]` device
//     class id selects the `DeviceClass` row whose `Function` key 007327B0
//     turns into the weapon category, and that row's `Bullet[1].Bullet` selects
//     the `BulletClass` row. Every number below is authored data read back
//     through the interpreter, not an invented constant.
//   * A platform has no authored position: the mount point is a model node.
//     Every gun therefore fires from its unit's own pose translation, raised by
//     the class `Height`, and its traverse angles are hull relative. The
//     traverse and fire windows are the authored ones and are exact.
//   * There is no recon slot object, so [recon+DE8h] is stood in for by the
//     enemy-side members of the world registry's per-class unit lists that
//     docs/RECON_SLOT_LISTS.md rule (a) scans, filtered by rule (b)'s four gate
//     bytes. Rule (c), the detection value, IS applied as of packet
//     cc8_recon_sensor_pass_binding: this host owns the one
//     bsp::ReconSensorPassState in the process and steps 008073C0 on 008079B0's
//     3-second cadence before the gunnery pass, so a target the sensor pass
//     published `none` for is in no contact list. What is still stood in for is
//     the slot object itself, so the pass walks sides rather than a slot's own
//     triple and the drain stays folded per target.
//     docs/RECON_SENSOR_PASS_BINDING.md.
//
// Evidence: docs/UNIT_GUNNERY_PASS.md, docs/GUNNERY_TABLES.md,
// docs/GUN_BOT_TICKS.md, docs/GUN_AIMING.md, docs/GUN_PLATFORM_ARC.md,
// docs/PROJECTILE_HELPERS.md, docs/PROJECTILE_IMPACT.md,
// docs/SHIP_HIT_RECORD.md, docs/UNIT_HIT_PATH.md, docs/UNIT_DAMAGE_AND_DEATH.md,
// docs/KILL_CREDIT.md, docs/GAME_EXECUTABLE.md.
//
// cc11_health_guard: the partial 00877B90 callback binding carries the existing
// UnitHealthWrite::dispatch_health_changed to the five damage/death endpoints.
// It is ON after the qualified cc11_health_pair simulation comparison. Callback
// timing remains at the host's hit/pass end; this does not provide native
// per-write callback ordering or binary ABI compatibility. See the guard and
// pair documents for the finite-request scope and retained NaN storage limit.

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/gun_aiming.hpp"
#include "bsp/camera_projection.hpp"
#include "bsp/gun_pending_timers.hpp"
#include "bsp/ordnance_kinds.hpp"
#include "bsp/gun_platform_arc.hpp"
#include "bsp/gunnery_tables.hpp"
#include "bsp/projectile_impact.hpp"
#include "bsp/rocket_homing_cc13.hpp"
#include "bsp/rocket_proximity_fuze_cc13.hpp"
#include "bsp/unit_gunnery_pass.hpp"

namespace bsp {
// Packet cc9_gunnery_pass_byte_7d, docs/GUNNERY_OPEN_ITEMS.md section 103. The
// gunnery pass byte +7Dh (ctor 1 at 008645D6) that 00865809..00865820 reads: the
// torpedo category takes the director's own fire target only while it is set.
// 00863780 (__thiscall(pass)(char), RET 4) stores it; a change to 0 calls
// 00728000 on every gun of [[pass+50h]+3ECh] when the owner answers
// vtable[5Ch](5). Its callers are the ship AI's 009F3016 (every approach frame:
// +12BAh && +12B4h + 50.0 > +127Ch), 009E6491 (approach exit, 1) and 009F4F2B
// (the formation-follower arm of 009F4DA0, 1). True: the pass reads the stored
// byte and a change to 0 clears the torpedo guns. False: the pass answers the
// constructor's 1; the byte is tracked and the clears counted, nothing else.
// ON by the pairs of 2026-10-05 (docs/GUNNERY_OPEN_ITEMS.md section 103.4).
inline constexpr bool kGunneryPassByte7dBound = true;
class ReconSensorPassState;
}

namespace bsp::game {

class GameHostLog;
class GameMissionLuaHost;
class GameUnitsHost;
class GameShipAiHost;

// One authored `BulletClass` row, as the shot needs it.
struct GameBulletClassRow {
    int id{-1};
    bool found{false};
    std::string name;
    std::string type;            // "Artillery", "Torpedo", ...
    float muzzle_speed{0.0f};    // "V0", classDesc+50h at 006E8430
    // The 006E9890 products for an artillery class (sub-types 4..7): +50h =
    // sqrt(9.81 * Range), +5Ch = V0 / that, and +54h FlyTime (FLT_MAX unless
    // authored). docs/CC13_VER_21.md D3.
    bool artillery_class{false};
    float class_speed{0.0f};
    float time_scale{1.0f};
    float fly_time{3.4028234663852886e+38f};
    float range{0.0f};           // "Range", the falloff divisor and the life bound
    float damage_min{0.0f};      // "DamageMin"
    float damage_max{0.0f};      // "DamageMax"
    float water_damage{0.0f};    // "WaterDamage", weapon->vtable[10h]
    float fire_damage{0.0f};     // "FireDamage", weapon->vtable[14h]
    float fire_chance{0.0f};     // "FireChance", weapon->vtable[18h]
    float blast_damage_min{0.0f};// "Blast.BlastDamageMin", classDesc+B4h
    float blast_damage_max{0.0f};// "Blast.BlastDamageMax", classDesc+B8h
    float blast_range{0.0f};     // "Blast.BlastRange", classDesc+70h
    float mass{0.0f};            // "Mass"
    // "NoGravity", classDesc+20h (docs/WEAPON_CLASS_DESCRIPTOR.md). 006E65C0 and
    // 006E7670 skip the gravity term for a class that sets it. Packet
    // cc9_gun_ballistics.
    bool no_gravity{false};
    // MTorpedo only. 008566B0 reads "WaterTravelSpeed" into classDesc+0E4h, and
    // it is a different quantity from V0 at +50h: the torpedo bot's intercept
    // solver takes +0E4h at 0090022B while the AA flak bot's call at 009031CF
    // takes +50h, so the choice is deliberate. The swim speed is that value
    // scaled by the double at 00D0C5E0 (0.5999994277954102), stored into the
    // torpedo record's +470h at 0085786D..00857875.
    // docs/TORPEDO_LAUNCH_ACCURACY.md.
    float water_travel_speed{0.0f};   // "WaterTravelSpeed", classDesc+0E4h
    float swim_speed{0.0f};           // record+470h = WaterTravelSpeed * 0.6
    // 008568E0's two water-entry limits: the round breaks up if it hits the sea
    // faster than MaxWaterHitVel, or if it was released above MaxFall. This is
    // why a torpedo bomber releases low and slow. docs/TORPEDO_TICK.md.
    float max_water_hit_vel{0.0f};    // "MaxWaterHitVel", classDesc+0DCh
    float max_fall{0.0f};             // "MaxFall", behind classDesc+0ECh
    // "HeadingTurn", classDesc+0E8h, degrees per second: 00857061's turn rate
    // toward the commanded heading. Packet cc9_usn02_sameside_torpedoes.
    float heading_turn{0.0f};
    // "HomingHorzTurnSpeed" / "HomingVertTurnSpeed", classDesc+0F0h / +0F4h
    // (008566B0, default 0): 00856BB0's homing turn rates, rad/s. Only this
    // installation's "Torpedo Fido Homing Airplane" sets them (0.5 each).
    float homing_horz_turn_speed{0.0f};
    float homing_vert_turn_speed{0.0f};
    // Packet cc13-homing1: the MRocket class fields 0080A000 reads.
    float ignition_delay{0.0f};       // "IgnitionDelay", classDesc+E0h
    float rocket_vmax{0.0f};          // "VMax", classDesc+D8h
    float rocket_acceleration{0.0f};  // "Acceleration", classDesc+DCh
    // Packet cc13-rocketfuze: "AntiAir", classDesc+E4h (00809CB0 stores it at
    // 00809D9F), the proximity fuze gate of 0080A55D. Read for every class in both
    // builds; only a rocket under kRocketProximityFuzeCc13Bound uses it.
    bool anti_air{false};
};

// One authored `DeviceClass` row, as the gun needs it.
struct GameDeviceClassRow {
    int id{-1};
    bool found{false};
    std::string name;
    std::string function;        // "Function", 007327B0's literal
    int category{-1};            // gunnery_category_from_function_name
    float horz_rot_speed{0.0f};  // "HorzRotSpeed", descriptor+88h, rad/s
    float vert_rot_speed{0.0f};  // "VertRotSpeed", descriptor+8Ch
    int bullet_class{-1};        // "Bullet[1].Bullet"
    float reload_time{0.0f};     // "Bullet[1].ReloadTime"
    float barrel_delay_time{0.0f};  // "Bullet[1].BarrelDelayTime"
    float throw_amount{0.0f};    // "Bullet[1].Throw", the spread
    int ammo{0};                 // "Bullet[1].Ammo"
    int barrel_num{1};           // the number of `Bullet` entries
};

// One gun this process built, which is one `Platforms[k]` of the unit's class.
// The live fields are the native gun's own: +480h/+484h/+494h/+498h the two
// angle pairs, +454h the trigger latch, +414h the barrel reload timers,
// +450h barrelDelayTime, +478h the stagger, +44Ch nextFireBarrel.
struct GameGunRow {
    std::size_t unit_index{0};
    float last_fire_seconds{-1.0f};   // gun+474h, the clock at the last shot
    std::string unit_name;
    int platform_key{0};              // the Lua `Platforms` key
    std::string platform_name;        // "Name"
    // Packet cc9_plane_attacker_weight: platform +0Ch PilotFires, +18h the Gun
    // list's size, and whether the device answers 25h (a bomb platform).
    bool platform_pilot_fires{false};
    int platform_gun_count{1};
    bool device_bomb_platform{false};
    int device_class{-1};
    std::string device_name;
    std::string function;
    int category{-1};                 // 0..0Bh, the gunnery pass's loop index
    int bullet_class{-1};
    // What this gun's projectile descriptor answers to vtable[8], from the
    // authored `Bullets` row's `Type`. docs/ORDNANCE_KIND_IDENTITY.md.
    bsp::OrdnanceKindSet ordnance{};
    // The bullet class record's +8h AFTER 006E9890's rewrite, a different id
    // space from `ordnance` above: 006E9890 turns the constructor's 1 into 2 or
    // 3 by the "AA" name test and its 4 into 5, 6 or 7 by the damage bands, and
    // 009FE270 switches on the result to pick a BulletTypeAccuracy row.
    // docs/AI_TARGET_WEIGHT_TERMS.md, docs/BULLET_ENGAGEMENT_RANGE.md.
    int bullet_sub_type{0};
    // gun[+3F8h]+4h, the fire record's `Throw` (007313E0): the cone half-angle
    // in radians before 0073031D's seat multiplier. docs/BULLET_THROW.md.
    float throw_amount{0.0f};
    float wind_amount{0.0f};          // gun[+3F8h]+8h, `Wind` (006E50E3, the rack's drift)
    float max_range{0.0f};            // 00731020's answer, the bullet `Range`
    float muzzle_speed{0.0f};
    // classDesc+50h and +5Ch AFTER 006E9890: for an artillery class (sub-types
    // 4..7) 006E98BC makes them sqrt(9.81 * Range) and V0 / that; otherwise
    // V0 and 1. `muzzle_speed` above keeps the authored V0. docs/CC13_VER_21.md D3.
    float class_speed{0.0f};
    float class_time_scale{1.0f};
    bool artillery_class{false};      // sub-type 4..7 with a usable class_speed
    float water_travel_speed{0.0f};   // MTorpedo classDesc+0E4h, 008566B0
    float swim_speed{0.0f};           // record+470h, WaterTravelSpeed * 0.6
    // gun+120h/+124h, the list 0072AD40 ages at the top of every fixed step.
    // Nothing in this reconstruction pushes to it yet - the producer was not
    // found - so it stays empty and the aging sweep is a no-op. It is carried
    // rather than omitted so the shape is in place, and so the counter below
    // would notice the moment a producer does appear.
    std::vector<bsp::GunPendingTimerRecord> pending_timers;
    std::vector<bsp::GunFiringArc> arcs;   // platform+3Ch, from `Windows`
    float rest_horz{0.0f};            // "RestAngles[1]", platform+94h
    float rest_vert{0.0f};            // "RestAngles[2]", platform+90h
    // False when the platform authors no RestAngles table: the image's +94h then
    // keeps FLT_MAX (00D7A278) and 0085AD00 / 0085A3D0 take their unauthored arms.
    bool rest_authored{false};
    // bot+54h, 008FBCE0's idle timer (kGunIdleRestBound), seeded as 0072BBD0 does.
    float idle_elapsed{999.0f};        // == kGunIdleTimerSeed, declared below
    bsp::GunTurningAngles angles{};   // +480h/+484h/+494h/+498h
    bsp::GunRotationSpeeds speeds{};  // descriptor+88h/+8Ch
    bsp::GunFireRequestState fire{};  // +454h/+478h/+450h/+414h
    int barrel_num{1};                // +448h
    // Packet cc9_ship_platform_attachment: the platform frame's origin in the
    // ship model (0095F500 slot pass -> platform+4Ch -> 0072DD20 -> the device
    // model's root node, 0072E9A1-0072E9AA). Valid when mount_known.
    bool mount_known{false};
    float mount_local[3]{0.0f, 0.0f, 0.0f};
    int next_fire_barrel{0};          // +44Ch
    float reload_time{0.0f};
    float barrel_delay_time{0.0f};
    // What 00727F10 last handed this gun, and what 00728000 last cleared.
    std::size_t target_unit{0};       // one based; 0 means no target
    std::string target_name;
    bool target_is_fire_target{false};// 00865804, the assigned flag
    // Counters.
    unsigned long long assigns{0};    // 00727F10
    unsigned long long clears{0};     // 00728000
    unsigned long long angle_sets{0}; // 0085ABA0 accepted
    unsigned long long angle_refusals{0};  // 0085ABA0 refused: no traverse window
    unsigned long long aim_steps{0};  // 0085AD80 bodies that moved an angle
    unsigned long long trigger_rises{0};   // 0072D2C0 latched true
    unsigned long long fire_messages{0};   // 0072D130 sent the 0ADh opcode
    unsigned long long can_fire_refusals{0};  // 00729A80 / 0085A830 said no
    unsigned long long arc_blocks{0};  // 007F60A0 refused the current angles
    unsigned long long arc_unsolved{0};// 006DFA60's gate: s > 1, out of reach
    unsigned long long shots{0};      // 00730160 bodies
    float first_shot_seconds{-1.0f};
    // Packet cc9_player_gun_seat (docs/PLAYER_GUN_SEAT.md). [gun+1ACh], the
    // gun's own seat: 8 (PLAYER_AI) until message 79h hands it to the unit's
    // role-2 holder. seat_horz/seat_vert are the angles that message's arm
    // gives 0085ABA0; seat_trigger its vtable[1E8h] argument.
    std::int32_t seat_1ac{8};
    float seat_horz{0.0f};
    float seat_vert{0.0f};
    bool seat_trigger{false};
    // Packet cc9_navigator_force_torpedo: a pending vtable[1D8h](0, 0, 0), the
    // direct 00730160 BSP_Gun_Fire NavigatorForceTorpedo (008A7200) makes.
    bool force_fire_008a7200{false};
    // Packet cc9_fire_function_guns_now: 009E2B60's vtable[1F0h], queued for the
    // gun's next aim-and-fire pass.
    bool immediate_fire_009e2b60{false};
    // Packet cc9_player_torpedo_group (message 79h group 4, 0095A1CC): the
    // launch order 007311B0 that 0072AC20 leaves on the tube. Its heading
    // (order+4, the message's world yaw msg+2Ch) is what 00856637 installs as
    // the torpedo's record+46Ch; 10000 = no order.
    float player_order_heading{10000.0f};
    unsigned long long seat_handovers{0};
    unsigned long long seat_returns{0};
    // Packet cc9_player_gun_seat_artillery. Message 79h's group 3 arm
    // (00959F72..0095A1C7) calls 0085ABA0 on every accepted gun, AI-held or
    // not, from the session drain (fan-out row 9, 00875E91), after the gun
    // waves: the pair is the last command before the next wave-1 step.
    bool seat_cmd_pending{false};
    float seat_cmd_horz{0.0f};
    float seat_cmd_vert{0.0f};
    // Packet cc13-gunB (kGunDestroyedFailureCc13Bound, include/bsp/
    // gun_destroyed_failure_cc13.hpp): the gun as a damageable device. 0087BCC0
    // seeds +368h, +36Ch and +370h from the device class's Armour and HP; read only
    // when the switch is on.
    float armour_368{0.0f};
    float max_health_36c{100.0f};
    float health_370{100.0f};
    bool destroyed_60{false};         // entity +60h, 00926C80 / 00926390
    bool destroyed_5d{false};         // entity +5Dh, 00926390 (a wreck, not deleted)
    bool destroyed_3b8{false};        // 0072B5BC
    int damage_level_358{0};          // 00876EC0 through 00877B00
    unsigned long long damage_calls{0};
    unsigned long long damage_refused{0};
};

// Packet cc9_player_gun_seat. ON binds message 79h's group 1/2 arm
// (00959C20) and the gun bots' side gate 008FFA99 on the gun's own seat.
// Committed OFF until its pair runs.
inline constexpr bool kPlayerGunSeatBound = true;

// Packet cc9_player_gun_seat_artillery. ON binds message 79h's group 3 arm
// of 00959C20 (00959F72..0095A1C7): the aim point 00957740, the gravity arc
// 00955630 and the 0085ABA0 turn of every Function 2/3/4/6 gun, AI-held
// included; the trigger only on a player-held gun. docs/PLAYER_GUN_SEAT.md
// section 7. ON by the pair in section 7.4.
inline constexpr bool kPlayerGunSeatArtilleryBound = true;

// cc9-gunnery53 (SHIP_AI_OPEN_ITEMS 296.8). A 4Bh MT_ROLEOWNER that changes a
// role holder (00780120's loop, 00780393) is handed to the unit's HandleMessage:
// 007803A8 0077FE80, whose default 0078002F calls vtable[164h] = 00821E80, whose
// 4Bh arm forces the base 0095ABE0. Its device walks at 0095AC1D run on this
// host's gun rows: mask & 4 or mask == 8, 00953F60 (a release only: Function
// 1/5/6 through 00729F70, keeping a Function 6 when [unit+1BCh] is the slot);
// mask & 10h, 00954000 (Function 2/3/4/6: take vtable[154h](0, slot), plus
// 0072D5B0(0, 0) on Function 6, a record; release 00729F70). Masks 20h and 80h
// (009540D0 / 00954170) stay with the group 4/5 models. This is what puts the
// player's artillery in the seat in single player: 0095A05B, the hand-over in
// 00959C20, needs a hosted session (game+1FE4h == 1) and a remote holder.
// True: the walk runs. False: role takes leave every gun AI-held.
// ON (cc9-gunnery53) by mechanism: pairs on d91c9e54c, reference env. USN01, with
// 0x9D / 0x99 input at 500/900/1300: takes 8, releases 66, held 410-1823.
// Northampton's turret shots 113/113/109 -> 95/95/90, because the AI does not fire
// a player-held mount. 0 player shells: the HUD camera aims at the sea and
// 0085ABA0 refuses. Deaths identical. USN04 idle: no walk, gameplay identical.
inline constexpr bool kRoleOwnerDeviceWalkBound = true;

// Packet s53_wreck_debris (docs/SHIP_AI_OPEN_ITEMS.md 263). 00825A59 step 8 of
// the unit update calls 00935540 once per dead unit (ShipSystems+88h one-shot)
// when ShipGlobals.DebrisStruct.DebrisEnabled (settings+680h) is set and both
// hull-end heights are below -5.0; kinds 0Eh and 8 spawn nothing, and each of
// _ftol(DebrisNumPerMeter * (int)z extent) pieces takes eight stream-1 00BD2F10
// draws. The z extent is the hull body's LOCAL box (B+38h/B+44h, the box the
// broad phase rotates at 00C5715C). The pieces themselves (00447510) are not
// made. True: the setting is read and the draws taken. False: recorded only.
// ON by docs/SHIP_AI_OPEN_ITEMS.md 263: USN01 reference mode gameplay identical;
// default mode moves from 204 s (7 wrecks, 359 pieces, 2872 draws).
inline constexpr bool kSinkingWreckDebrisDrawsBound = true;

// Packet cc9_player_gun_seat_segment_query. ON binds 00957D79..00957DD2 of
// 00957BD0: with no target and the camera off the x = z = 0 axis, the
// 1000-unit segment (00CE47A0, double) from the camera along the aim
// direction goes through 0098ADD0 with kind 0, excluding the unit's
// [unit+360h] (vtable slot 20h, 006D1E30); a hit with y > 0 (00D7A218) is
// every gun's aim point, skipping the range sphere and the sea cut. This host
// answers the query over its own units (query_segment_units; no islands or
// terrain in it). ON: the USN02 and E2 pairs held (docs/PLAYER_GUN_SEAT.md 6.4).
inline constexpr bool kPlayerGunSeatSegmentQueryBound = true;

// Packet g53_seat_target_intercept (cc9-gunnery53, docs/PLAYER_GUN_SEAT.md 7.6).
// The 79h target, both ends:
// - The HUD gate at 005488FE..00548975 sets has_target (+36h) and the target's
//   +174h id (+38h) for a picked unit that is not on the player's side (+54h)
//   and whose vtable[10Ch] answers true. That slot is 0074E1A0 on the plane
//   classes ([unit+B1Ch] speed > 3.0, 00CE3854) and 0042BB40
//   (false) on MBattleship, LandingShip, MSubmarine and the dummy kamikaze. So
//   only a flying plane is ever sent.
// - 00959C91..00959CA8 resolves the id with 00521E30. 00957BD0 then aims every
//   group 1/2 gun at the 00901C20 intercept (00957C29..00957CAB): shooter at
//   gun+FCh, speed [[gun+3F8h]+34h]+50h, and shooter velocity = the unit's
//   vtable[34h] when [unit+4A0h] (00953B80: the OR of AddsSpeed over the unit's
//   Function 0/1/5/6 guns) is set, else zero.
// The group 3 arm (00957740) never reads the target.
// The marks' gunner range (00548EA9 / 00548ED0, [unit+49Ch]) is bound with it;
// without it 0054900D clears the pick before the gate.
// False: the gate answers false and the intercept is recorded only.
// ON by mechanism (PLAYER_GUN_SEAT 7.6): with a tracked Kate, USN04's player AA
// leads it and the Lexington kills it 2.9 s earlier; idle USN04/USN21 identical.
inline constexpr bool kPlayerGunSeatTargetInterceptBound = true;

// Packet cc9_unit_death_route, docs/UNIT_DEATH_ROUTE.md. True: a sunk or
// wrecked unit goes through 00959450 BSP_Unit_OnDestroyed's model
// (bsp::unit_death_00959450) after the sink: the report gate (clock > 1.0,
// KillReason 1, world+4ACh), the kind-20h children's vt[1E8h](0), the loss
// report 009813A0 (a named record: the warning manager has no entry for it),
// the kill credit 0091BDA0 when the gate passes, and the limbo page 00565FB0
// for the controlled unit (a named record). False: the sink goes straight to
// the kill credit. ON: the USN02 and E2 pairs held (docs/UNIT_DEATH_ROUTE.md s4).
inline constexpr bool kUnitDeathRouteBound = true;

// Packet cc9_death_route_destroy, docs/CONSTRUCT_WORLD.md section 22. True: the
// death route calls the Destroy step at the kill, as the image's death step does
// (00958A30 -> vt[70h](1) = 0077D1A0 -> 00926C80, +60h = 1 at 00926CD5), for a
// ship. The row-15 flush (kSunkShipFlushBound, src/game_hosts_ready.cpp) keeps
// setting +5Dh and finds +60h already set. Planes already get both bytes in the
// units host (kPlaneDeathFlagsBound). False: +60h lands at row 15, the flush's
// labelled substitution.
// Committed false for the pairs; USN02 and E2 kept gameplay identical, and the
// HUD markers stop drawing a dead ship one frame earlier, as in the image. ON.
inline constexpr bool kDeathRouteDestroyBound = true;

// Packet cc9_ship_weapon_director_enable, docs/SENTITY_INIT_PASSES.md section 7.
// True: a ship's weapon director carries the four enables its pass B sets from
// the scene bag (00822C20 -> 007214C0 at 00823A9F -> vt[3Ch] 00835690 ->
// 007219C0 at +220h..+223h), so the stance 008624C0 pushes masks off every
// category the ship's bag disables; this installation's ship.props defaults
// TorpedoDirector to false. SUBSTITUTION: read at the stance push from the
// scene host's per-name table rather than stored on the controller at creation.
// Packet cc9_ship_torpedo_mask_read (section 8): the torpedo byte has two
// runtime writers on these missions, both session message 5Ah sub-kind 5
// (0071E0D0 -> 0071C25B), bound with this switch: the Lua native TorpedoEnable
// 0089C8F0 (usn_2_java.lua 354 and 706 at difficulty 1) and CLOSEATTACK's tail
// 00A11AF0 (every ship of the own group, every tick). SUBSTITUTION: both store
// at the send instead of through the session. False: all four stay the
// constructor's 1 (007202FD) and TorpedoEnable stays unimplemented.
inline constexpr bool kShipDirectorEnablesBound = true;

// Packet cc13-fix15 (docs/CC13_FIX15.md). The Lua native ArtilleryEnable
// 0089C590: when the entity's vtable[114h] director exists, 0071DFD0 routes
// session message 5Ah with sub-kind 3 (msg+20h) and the flag (msg+24h) through
// 0077C2A0 to 00721A40, whose 5Ah arm (00721A93) calls director vtable[38h]
// (00835610 JMP / 00835640 tail) into 0071C1E0, which stores the flag at
// director+220h "artilleryEnabled" (0071C231). The constructor's 1 (007202FD)
// is the default. Readers on the fire path: 008624C0 pushes it through
// 00861CD0 into bit 1 ("may engage a non-plane") of every artillery
// category's mask (0086258B), which 008633D0 tests before a target is
// admitted; 009F1BC0 caches it as the ship AI's +12B9h (009F2B18, 009F2E20).
// True: the native writes the byte into the scene director table by name and
// the stance push reads it for non-ship units too (ships already read the
// whole table under kShipDirectorEnablesBound, and the ship AI's +12B9h reads
// the same table). SUBSTITUTION: stored at the call, not delivered through the
// session. False: the native stays unimplemented and every director keeps 1.
// ON since cc9-ships58 (docs/SHIP_AI_OPEN_ITEMS.md section 285).
inline constexpr bool kLuaArtilleryEnableBound = true;

// Packet cc13-fix17 (docs/CC13_FIX17.md). The Lua native ForceFireDepthCharge
// 0089B9D0: argument 0 through 00888AA0 (no null test), then the unit's child
// list from +48h (next +44h, read after the call, 0089BB1B). A child that answers
// vtable[5Ch](24h) (MSTGun and its MDepthChargeLauncher 27h) and whose
// descriptor [child+3F4h]+80h is 8 ("DEPTHCHARGE") gets vtable[1F0h]() at
// 0089BB19. For MSTGun that slot is 006FDC50 = vtable[1E8h](1) = 006FDCD0: the
// fire request 0072D2C0(1), and +4D4h = 0 when +454h is 0. No RNG draw and no
// result. The request is then the gun's: 006FE0D0's salvo test drops it after
// the last barrel, and DepthChargeBot 008FC080 only rewrites it on a think with
// a fire target (it returns at 008FC13C without one).
// True: each call is queued and, at the next fixed step, raises the trigger of
// every category-8 gun of the unit; a raised trigger survives the host's
// no-target drop until its salvo ends or the bot has a target again. LABELLED:
// category 8 stands in for the class test (every DEPTHCHARGE device in this
// installation's deviceclasses.lua is a Single_Turning_Gun), the raise lands at
// the next fixed step rather than in the script phase, and a player-held seat
// keeps its own trigger. False: the native stays unimplemented.
// ON (cc9-lua67, SQUADRON_LAND_TASK 5ii): PRCP01 pair, Puffin fires 124 charges.
inline constexpr bool kLuaForceFireDepthChargeBound = true;

// Packet cc9_torpedo_stock (docs/UNIT_WEAPON_DEVICES.md, "Torpedo stock, bound").
// True: a ship's torpedo tubes draw on the class's MaxTorpedoStock (class+7A0h).
// 0081F8B0 at pass C (008201B8) sets the spare unit+104Ch = stock - loaded
// barrels (00810E90 over the category-7 list at unit+3ECh); every fired
// torpedo barrel then goes through 0072D520's provider pair, the unit's
// vtable[1F4h] 00810D80 (kind 7: spare != 0) and [1F8h] 00810DA0 (kind 7:
// spare > 0 -> spare - 1). With no spare left the barrel is pinned at FLT_MAX
// and never fires again. SUBSTITUTION: the pass C set is made at the ship's
// first torpedo shot (every barrel is still loaded then); the supply tick
// 00825450 and the unload 0081DCB0 (stock < loaded) are not bound. False:
// tubes reload without limit, as before.
inline constexpr bool kTorpedoStockBound = true;

// Packet cc13_natives1 (docs/CC13_WIN_PATH_NATIVES.md section 1.4). The ship's
// jam bytes unit+9E4h (steering) and +9E5h (engine), set by 008198A0's message
// 6Bh / 6Ch arms (0081999D / 008199C7) and cleared by 008137B0's 6Dh / 6Eh arms,
// which 00813660 (vtable[19Ch]) routes when 0093C520 retires the record. The
// motion reads them at 00826754 (engine gate, target speed 0) and 00826B2E (the
// rudder slew 0092E8C0 skipped). True: the units host copies them into the ship
// motion state before each 00825F20 step. False: the motion never sees a jam.
inline constexpr bool kShipJamMotionBound = true;  // ON: n1 pairs, faithful (GUNNERY 197-199, cc9-gunnery38)

// Packet cc9_torpedo_supply_tick (docs/UNIT_WEAPON_DEVICES.md, "The supply
// tick"). True: every ship runs 00825450 once per fixed step, and the stock
// set of kTorpedoStockBound unloads random loaded barrels (0081DCB0, shared
// generator stream 1) while the stock is below the loaded count. False: no
// tick, and a stock below the loaded count only clears the spare.
// LABELLED SUBSTITUTION: the supply-area registry 00F874F0 stays empty.
// 00809880 registers an area only for a non-empty `RepairZoneArea`
// (00809BC0 with a mode-1 bag), and every one of the 83 `RepairZoneArea`
// rows in this installation's .scn files is "", as are the ship.props and
// landfort.props defaults. A savegame bag (mode 2) could still carry one; the
// harness never loads one.
inline constexpr bool kTorpedoSupplyTickBound = true;

// Packet cc9_ship_set_torpedo_stock (docs/UNIT_WEAPON_DEVICES.md,
// "ShipSetTorpedoStock, wired"). True: the Lua native ShipSetTorpedoStock
// (0089EEE0) runs 0081F8B0 on the unit's torpedo state. That means the pass C
// set first when the unit has not had it yet, then the unload loop or the
// spare, then 0072D520(gun, barrel, 1) on every pinned barrel of its torpedo
// guns. LABELLED SUBSTITUTION: the script host queues the call and the gunnery
// host applies it at the start of its next fixed step (the image calls
// 0081F8B0 inside the native). False: the native stays an unimplemented record.
inline constexpr bool kShipSetTorpedoStockBound = true;

// Packet p17_reload_torpedoes (docs/OCEAN_WAVE_FIELD.md section 66). True: the Lua
// native ReloadTorpedoes(unit) (008C3CE0) walks the unit's device list ([+48h], next
// +44h) and, for each torpedo gun (vtable[5Ch](20h) and [[gun+3F4h]+80h] == 7),
// calls 0072CF00(gun, barrel, 0.0f, 0) on every barrel 0..[gun+448h) (008C3E32):
// every tube is loaded and ready at once, fired tubes included, and the torpedo
// stock is not drawn. LABELLED SUBSTITUTIONS: a torpedo gun is a gun row of
// kUnitGunneryTorpedoCategory; the call is queued by the script host and applied at
// the start of the gunnery host's next fixed step; 0072CF00's torpedo-model tail is
// presentation. False: the native stays an unimplemented record.
inline constexpr bool kReloadTorpedoesBound = true;  // cc9-sweep2b: prune flip, zero reach on u01/b9/b6i/l10/p01/u12/j8v3

// Packet cc9_gun_rest_angles (docs/GUN_REST_ANGLES.md). True: a targetless gun
// follows the image's idle rule instead of commanding its rest angles every
// tick. 008FBCE0, step 3 of every gun-bot prologue, keeps a per-bot timer
// seeded 999.0 (00CF4888 at 0072BBD0), held at 0 while the bot has a target and
// otherwise advanced by dt; past NoTargetTimeUntilRest, with the side gate
// passing, it is pinned at FLT_MAX and a gun answering IsKindOf(22h) gets ONE
// 0085AD00, which does nothing when the platform's RestAngles were not
// authored (+94h == FLT_MAX, 00D7A278). A failing side gate winds a pinned timer
// back to a quarter of the limit (00D7A348). Every other targetless tick issues
// no angle command: the gun holds. An unauthored turning gun spawns at the
// clamped midpoint of its first traverse-and-fire arc (0085A3D0), not at 0.
// LABELLED SUBSTITUTIONS: the limit is kGunNoTargetTimeUntilRest below, not a
// robots.lua descriptor read; the side gate is the host's player-seat test; the
// 22h class test is the category rule (not PLANEGUN, BOMBPLATFORM, CATAPULT),
// exact on this installation's device tables; one timer per gun row stands in
// for one per bot. False: every targetless gun commands rest_horz/rest_vert
// each tick, rest angles default to 0 and the gun spawns there.
// ON by the pairs of 2026-09-27 (docs/GUN_REST_ANGLES.md section 9).
inline constexpr bool kGunIdleRestBound = true;

// Packet cc9_difficulty_multipliers (docs/DIFFICULTY_MULTIPLIERS.md). True: damage
// taken follows the image's difficulty scaling. Every health write the host routes
// as the unit's AddDamage (vtable[1ACh]: 008777D0's hull pass 008778C6 and part
// pass 00877A37, the delayed explosions, damage control's water and fire) takes
// 0095DA00's config+4Ch[level] (1 / PlayerCheatMultipliers) when the local player
// holds role 0, then 0087D730's config+1Ch[level] (1 / HPMultipliers) when the
// unit's party (+54h) is the local player's. The ship hit record's own multiply
// at 008270AD (the failure roll and the effect count) uses the same party test and
// +1Ch. The tables come from the loaded Globals.Difficulty at run time (0087D7B0).
// LABELLED: the local player's party (record +28h) is taken as the controlled
// unit's party; single player only (game+1FE4h == 0); the level is game+6ACh.
// False: no damage is scaled, as before.
// ON by the pairs of 2026-09-28 (docs/DIFFICULTY_MULTIPLIERS.md section 6.3).
inline constexpr bool kDifficultyMultipliersBound = true;

// Packet cc9_unit_fire_cooldown (docs/GUNNERY_OPEN_ITEMS.md section 11). True: the
// two unit-wide fire cooldowns CanFire tests 5 and 6 read. 0072F830, per shot: an
// artillery kind (2/3/4/6, 006D1E50) sets unit+6F8h = U(0.075, 0.225) on stream 1
// (00CEED68 / 00CFE2B4, 0072FB4F..0072FB78); a torpedo (kind 7) sets unit+6FCh =
// 00836EB0: ShipGlobals SubTorpedoDelay for a submarine (IsKindOf(8), settings
// +764h), else ShipTorpedoDelay (+768h). 00953CC0 counts both down by the step while
// positive. 00729ADD..00729B26: an artillery gun refuses while +6F8h > 0, a torpedo
// gun while +6FCh > 0. The constructor seeds both 0 (0095CF50/58, XORPS 0095CD7D).
// LABELLED: the host counts down at the start of its gun step, the image in the
// unit's own tick. False: neither test binds, as before.
// ON by the pairs of 2026-09-28 (docs/GUNNERY_OPEN_ITEMS.md section 12).
inline constexpr bool kUnitFireCooldownBound = true;
// Packet cc9_repair_enable_route (docs/GUNNERY_OPEN_ITEMS.md section 82). RepairEnable
// on an IsKindOf(6) entity (008AD330) builds message 9Fh with the flag at +1Ch
// (008AD48B..008AD4B7) and routes it through 0077C2A0 (008AD4CD, flags 7); the unit's
// handler 00821E80 takes it at 008220D7 and calls 00939FD0 on the repair task at
// unit+A20h, which stores the byte at task+45h. 0093C770 is the only reader in the
// task block (0093C776): with +45h clear the hull repair rate is 0. True: the flag
// reaches the gunnery host's task and gates its 0093C770 step. LABELLED: stored at
// the Lua call, not queued to the next step's session pump. False: the flag is
// counted and dropped, and every task keeps 0093BCC0's +45h = 1.
// ON by the pairs of 2026-09-30 (docs/GUNNERY_OPEN_ITEMS.md section 82.4).
inline constexpr bool kHullRepairEnableRouteBound = true;
// robots.lua (this installation, 2025-06-01): NoTargetTimeUntilRest = 20.0 for
// AAFlakBot, TailGunnerBot, AAGunnerBot, ArtilleryGunnerBot, TorpedoBot and
// DepthChargeBot alike; descriptor +4h, read by 008FBCE0 as [[bot+30h]+4h].
inline constexpr float kGunNoTargetTimeUntilRest = 20.0f;
// 00CF4888, the idle timer's seed at 0072BBD0 (bot+54h).
inline constexpr float kGunIdleTimerSeed = 999.0f;

// The queue between the script host's ShipSetTorpedoStock and the gunnery host.
// `unit_index` is the units host's index. Process-wide, drained by
// GameGunneryHost::fixed_step.
void queue_ship_set_torpedo_stock_0089eee0(std::size_t unit_index, std::int32_t stock);
// Packet p17_reload_torpedoes: a ReloadTorpedoes call (008C3CE0), applied at the next
// fixed step under kReloadTorpedoesBound.
void queue_reload_torpedoes_008c3ce0(std::size_t unit_index);
// Packet cc9_bsm01_state_natives: an ExplodeToParts call (0088E1B0), applied as
// 00935C70 over the unit's hull segments at the next fixed step.
void queue_explode_to_parts_0088e1b0(std::size_t unit_index);
// Packet cc13-fix17: a ForceFireDepthCharge call (0089B9D0), applied at the next
// fixed step under kLuaForceFireDepthChargeBound.
void queue_force_fire_depth_charge_0089b9d0(std::size_t unit_index);

// Session message 79h as 00954A10 builds it in 005484F0 (section 32 of
// docs/SHIP_SCREEN_UPDATE.md): +1Ch group, +20h..+28h the camera mover's
// world position, +2Ch yaw and +30h pitch of its forward row (00521370),
// +34h action 99h held, +35h 99h pressed, +36h has-target, +38h target id.
// `forward` is the mover's forward row the two angles are taken from.
struct GunAimMessage79 {
    int group{0};
    float camera[3]{};
    float forward[3]{};
    bool held_34{false};
    bool pressed_35{false};
    bool has_target_36{false};
    std::uint16_t target_38{0};
    // msg+2Ch, the group 4 arm's world yaw (0095A210), in this host's world
    // heading convention (atan2(x, z)).
    float yaw_2c{0.0f};
    // cc15-smA: msg+30h, the pitch 00954A10 clamped. Set, with yaw_2c, when
    // the value is the copy 0076E520 rebuilt (HostSessionMessages::
    // queue_gun_aim_79); 00959C20's arms then take +2Ch/+30h as the image
    // does instead of recomputing them from `forward`.
    float pitch_30{0.0f};
    bool angles_carried{false};
};

// One projectile in flight, as 006E8430 created it.
struct GameProjectileRow {
    std::size_t gun_row{0};
    // Packet cc9_projectile_team_id: [shot+1Ch] (projectile+18Ch,
    // kProjectileOffTeamId), the team 0072BF10 stamps at 0072C0FB / 0072C14B
    // and the hit event's attacker_player_index reads. -1: no stamp traced.
    int team_id_1c{-1};
    std::size_t owner_unit{0};        // one based
    int owner_side{0};
    int bullet_class{-1};
    bsp::ProjectileFlightState flight{};
    float position[3]{};
    float life{0.0f};
    // Packet cc9_bomb_drift: 006E4D50's drift (rack+4ECh, copied by 006E1F00
    // into block+38h with y zeroed), the acceleration 006E1510 adds at
    // record+348h. Zero unless kBombDropDriftBound.
    float bomb_drift[3]{};
    bool alive{false};
    // Diagnostic (BSP_SHELL_FATE): how the round ended. 0 none, 1 land, 2 entity,
    // 3 flak burst, 4 water, 5 expired.
    int fate{0};
    bool swimming{false};             // past the water crossing, on the swim
    // Packet s57_torpedo_from_sub: projectile+354h as 006E2E10 samples it at the
    // activation (y at or below the water, 006E2E7F), which 00857780 copies to the
    // torpedo's +49Ch at 008578B0. Set at the swim entry from a launch that was
    // already under the surface (kSubmergedTorpedoLaunchSwimBound's entry).
    bool started_under_water{false};
    // Packet cc9_depth_charge_in_water (kDepthChargeInWaterBound): a round of a
    // "Depthcharge" class, its water byte (006E6450 at the entry 006FD660), and
    // the activate 006FD9B0's record+468h dragvert and +470h divedepth.
    bool depth_charge{false};
    bool depth_charge_in_water{false};
    float depth_charge_drag{0.0f};
    float depth_charge_depth{0.0f};
    // record+46Ch, the commanded world heading (atan2(x, z)) the launch command
    // 007311B0 carries as `heading`; 10000 (00D0C310) means none, and the steer
    // then holds the heading it swims on. Packet cc9_usn02_sameside_torpedoes.
    float commanded_heading{10000.0f};
    // record+47Ch "swimdepth": 00856050's -2.5 (00D0C314), or launch params +8h
    // (0072C970's 00852410 prediction for a submarine firing at a submarine).
    // Read only under kTorpedoSwimDepthHomingBound.
    float swim_depth{-2.5f};
    // Packet cc8_torpedo_closest_approach. Tracked only while swimming: the
    // smallest horizontal distance this round reached to any unit of another
    // side, and when. Centre to centre - this host has no oriented hull box for
    // a ship - so read it against the target's own Length.
    float min_enemy_distance{-1.0f};
    float min_enemy_time{-1.0f};
    std::size_t min_enemy_unit{0};    // one based, 0 = never measured
    // Packet cc8_torpedo_aim_census. The same tracking against the ORDERED
    // target rather than the nearest unit, which is what decides whether the
    // ship a round came nearest to is the ship it was aimed at.
    std::size_t ordered_target{0};       // one based, the owner's command target
    float target_pos_release[3]{};       // that target's position at the drop
    float ordered_min_distance{-1.0f};
    float ordered_min_time{-1.0f};
    float target_pos_at_min[3]{};
    // |wrapped(round track - target heading)| at the closest approach. A scalar
    // distance cannot separate "abeam and clear" from "inside the bow line";
    // this is what does.
    float crossing_angle{0.0f};
    // Packet cc8_torpedo_retire item 5: the same geometry at the DROP, so the
    // run-in can be compared with the closest approach without re-deriving it.
    // Both are hull POSE headings (the unit's vtable[50h] row 2), the same
    // quantity `course` above uses - never the ship-ai step heading.
    float drop_owner_heading{0.0f};
    float drop_target_heading{0.0f};
    float drop_crossing_angle{-1.0f};   // -1 = no ordered target at the drop
    // Packet cc8_torpedo_release item 1. The horizontal centre-to-centre range
    // to the ordered target AT THE DROP. This is the quantity the image's
    // aspect gate scales: 009D2008 and 009D203D both compare the interpolated
    // release distance against approach+90h, which 009D3519/009D3552 build
    // from exactly this X/Z difference. No census recorded it - the drop line
    // prints the drop ALTITUDE (origin[1]), not a range - so the aspect gate
    // could not be measured at all. INSTRUMENTATION, no behaviour.
    // -1 = no ordered target at the drop.
    float drop_ordered_range{-1.0f};
    // Packet cc8_torpedo_swim, item 1: the drop index this round was filed
    // under, so every later line about it can be keyed to the id on its own
    // `torpedo drop N` line rather than matched on a bare number across the
    // three id spaces a run prints. 0 = not a torpedo drop and not traced.
    unsigned long long torpedo_trace_id{0};
    // Packet cc9_ship_torpedo_response. A run-unique serial the ship AI keys its
    // torpedo tracks on (the image's track+48h holds the entity itself), and the
    // time in the water, the torpedo record's +488h that 0085748A advances by
    // dt on every swim step. The serial is assigned at the first flight step.
    unsigned long long serial{0};
    float swim_seconds{0.0f};
    // Packet cc9_flak_proximity_burst (docs/FLAK_PROXIMITY_BURST.md): the flak
    // round's lock byte (+288h, ESI+44h at 0070C661), its locked entity (+294h,
    // ESI+50h, one based here) and the distance left to the burst (+28Ch,
    // ESI+48h, written at 0070C6D5 and run down at 0070C7AF).
    bool flak_locked{false};
    std::size_t flak_target{0};
    float flak_remaining{0.0f};
    // Packet cc9_aa_lethality_audit: the round's "distErr" (+290h), the firing
    // AAFlakBot's bot+60h when kAaFlakAimErrorBound is on; 0 otherwise.
    float flak_dist_err{0.0f};
    // Packet cc8_dive_glide. A bomb released by the dive-bomb task, and the
    // predicted impact point approach+D8h/+E0h carried at the release tick so
    // the run can print predicted against actual. INSTRUMENTATION; the image
    // keeps neither flag nor point on the round.
    bool is_bomb{false};
    // kTorpedoRackDropBound: a torpedo dropped through 006E4D50's bomb arm, whose
    // bomb_drift (006E1510) applies while it is airborne.
    bool rack_drift{false};
    float predicted_impact[3]{};
    // Packet cc13-homing1: a rocket record's motion state (0080A000) and its
    // homing block (0080AA20). docs/CC13_ROCKET_HOMING.md.
    bool is_rocket{false};
    bsp::RocketFlight rocket{};
    bsp::RocketClassParams rocket_class{};
    // Packet cc8_dive_aim item 2. The aimdive summary's `tf=` is LAST-SAMPLED -
    // it is read after the aircraft has left the dive, so it is not the fall
    // time that was in force when the round left. These four are sampled AT THE
    // RELEASE TICK and at no other time, which is the whole point of them: a
    // release-tick `tf` that is 3x the measured fall puts the defect in
    // 007BCC80 or its feed, and one that matches puts it in the geometry.
    // `release_fall_time` is 009C7D71's own tf (fall time + 0.1, 00D7A3A0).
    float release_fall_time{-1.0f};
    float target_speed_release{0.0f};    // 0092D730 forward speed, at the drop
    float target_heading_release{0.0f};  // vtable[50h] hull heading, at the drop
    // Sampled when the round DIES, not at the drop: where the target had got to
    // while the bomb fell, which is what the miss is actually against.
    float target_pos_impact[3]{};
    float target_heading_impact{0.0f};
};

// Packet cc8_dive_glide. One released bomb's impact, kept after the round is
// gone. `predicted` is approach+D8h/+E0h at the release tick - the point
// 009C7D71 computed as where a bomb dropped then would land - so the two
// errors below say whether the CCIP solution the dive steers on is the one the
// released round actually flies to.
struct GameBombImpactRow {
    std::string owner_name;
    float predicted[3]{};
    float actual[3]{};
    float target_release[3]{};
    float predicted_error{-1.0f};  // planar |actual - predicted|
    float target_error{-1.0f};     // planar |actual - target at release|
    float life{0.0f};              // seconds of flight
    // Packet cc8_dive_aim item 2, all carried from the round. Every one of
    // these says in its own name WHEN it was sampled, because the trap this
    // census exists to avoid is a last-sampled column read as a release one.
    float release_fall_time{-1.0f};      // 009C7D71's tf AT THE RELEASE TICK
    float target_speed_release{0.0f};
    float target_heading_release{0.0f};
    std::string target_name;             // the ORDERED TARGET, not the bomber
    float target_pos_impact[3]{};        // where the target was AT IMPACT
    float target_heading_impact{0.0f};
    float impact_error{-1.0f};           // planar |actual - target AT IMPACT|
    // The miss against the target at impact, resolved in the target's own
    // frame: +along is ahead of the ship on its course, +across is to its
    // starboard. This is the decomposition the integrator asked for, and it is
    // what separates "the bomb fell short of a moving ship" from "the aim point
    // was off the hull line".
    float miss_along{0.0f};
    float miss_across{0.0f};
    // A round the entity sweep killed dies where it met the hull, above the
    // sea; one that reached the water crossing dies at or below y = 0. This
    // host keeps no hit flag on the round, so that height IS the discriminator
    // and is reported as such rather than as a recovered field.
    bool died_above_water{false};
};

// One swimming round's closest approach, kept after the round is gone.
struct GameTorpedoApproachRow {
    std::string owner_name;
    std::string nearest_name;
    float min_distance{-1.0f};
    float min_time{-1.0f};
    float life_at_end{0.0f};
    bool hit{false};
    bool expired{false};
    // Packet cc8_torpedo_aim_census.
    std::string ordered_name;         // "-" when the owner had no command target
    float ordered_min_distance{-1.0f};
    float ordered_min_time{-1.0f};
    float target_travel{0.0f};        // how far the ordered target moved, drop to closest
    float crossing_angle{0.0f};       // radians, 0 = the round runs along the target's course
    // Packet cc8_torpedo_retire item 5, carried from the round.
    float drop_owner_heading{0.0f};
    float drop_target_heading{0.0f};
    float drop_crossing_angle{-1.0f};
    // Packet cc8_torpedo_release item 1, carried from the round.
    float drop_ordered_range{-1.0f};
};

// Per unit, what the chain did to it and what it did with its guns.
struct GameGunneryUnitRow {
    std::size_t unit_index{0};
    std::string name;
    int side{0};
    int type_id{-1};
    std::size_t guns{0};
    std::array<int, bsp::kUnitGunneryCategoryCount> category_guns{};
    std::array<float, bsp::kUnitGunneryCategoryCount> category_ranges{};
    // Packet cc8_ship_ai_firepower_inputs: the two extra maxima 00956C20
    // writes beside unit+430h, which the rebuild binding used to discard.
    // 0095EB40 gates its whole body on unit+494h at 0095EB62, so the ship AI
    // cannot rate a range without it.
    float artillery_max_range{0.0f};  // unit+490h, Function 2, 3, 4 and 6 only
    float any_weapon_max_range{0.0f}; // unit+494h, every category
    bool pass_attached{false};
    bool pass_enabled{false};         // pass+58h
    unsigned long long think_bodies{0};    // 00864FE0 bodies past the throttle
    unsigned long long ticks{0};      // 00864FE0 calls
    unsigned long long candidates{0}; // recon-sweep survivors, summed
    float nearest_enemy{0.0f};        // the closest 00863990 measured
    float best_range{0.0f};           // the largest unit+430h row read
    unsigned long long assigns{0};
    unsigned long long clears{0};
    std::string fire_target;          // [this+5Ch]->vtable[4], director+238h
    std::string command_target;       // 0071EBF0 then 00521EA0
    unsigned long long shots{0};
    unsigned long long hits_taken{0};
    unsigned long long hits_dealt{0};
    float damage_taken{0.0f};
    float damage_dealt{0.0f};
    float health{0.0f};
    float max_health{0.0f};
    unsigned long long fires_started{0};   // hit+4Ch, the fire rate record
    unsigned long long floods_started{0};  // hit+50h, the flood rate record
    bool sunk{false};
    float sunk_seconds{-1.0f};
    std::string killed_by;
    unsigned long long kill_credits{0};    // 0091BDA0 bodies naming this unit
};

struct GameGunnerySummary {
    // Packet cc13-homing1 (kRocketHomingBound): rocket launches and 0080A000.
    unsigned long long rocket_launches{0};
    unsigned long long rocket_guided_launches{0};
    unsigned long long rocket_ignitions{0};
    unsigned long long rocket_homing_steps{0};
    unsigned long long rocket_homing_lock_lost{0};
    std::size_t units_with_guns{0};
    std::size_t guns{0};
    std::size_t passes_attached{0};
    std::size_t device_rows{0};
    std::size_t bullet_rows{0};
    unsigned long long pass_ticks{0};
    unsigned long long pass_bodies{0};
    unsigned long long bridge_applies{0};   // 008624C0
    unsigned long long recon_sweeps{0};
    unsigned long long candidates{0};
    unsigned long long candidates_rejected{0};
    unsigned long long assignment_passes{0};
    unsigned long long gun_evaluations{0};
    unsigned long long gun_slot_rejects{0};
    unsigned long long assigns{0};
    unsigned long long clears{0};
    // Diagnostic provenance for step 8.8's choice. docs/GUNNERY_CANDIDATE_ORDER.md
    // proves the walk runs order[count-1] down to order[0], so step 8.7's unsorted
    // appends are tried BEFORE the recon sweep's ranked inserts. These separate the
    // two sources so a run can say which one a gun actually took and at what
    // fraction of its own reach, instead of inferring it from totals.
    unsigned long long assigns_from_arm{0};     // the director's fire/command target
    unsigned long long assigns_from_recon{0};   // the ranked recon sweep
    double arm_reach_fraction_sum{0.0};         // distance / max_range at assignment
    double recon_reach_fraction_sum{0.0};
    unsigned long long arm_assigns_beyond_half{0};   // fraction > 0.5
    unsigned long long recon_assigns_beyond_half{0};
    // Guns whose engagement range came from 00855A90's water-travel rule rather
    // than the authored `Range`. Zero here means the Bullets table was not
    // reachable and category 7 is still refused on the 10.0f seed.
    // Which gate the recon-contact stand-in drops a unit at, so a zero contact
    // count can be attributed instead of guessed.
    // Ticks on which the gun actually held a target, split out of the
    // per-gun-per-tick angle tallies which carry no target information.
    unsigned long long angle_refusals_targeted{0};
    // Packet cc9_gun_rest_angles: targetless ticks that issued no angle command,
    // 0085AD00 calls made, calls that found RestAngles unauthored, the side
    // gate's wind-backs, and the 0085A3D0 spawn seeds (and seeds with no arc).
    unsigned long long idle_holds{0};
    unsigned long long idle_rests{0};
    unsigned long long idle_rests_unauthored{0};
    unsigned long long idle_windbacks{0};
    unsigned long long rest_seeds{0};
    unsigned long long rest_seed_no_arc{0};
    unsigned long long want_fire_no_accept{0};
    unsigned long long want_fire_no_settle{0};
    unsigned long long want_fire_no_window{0};
    unsigned long long contact_considered{0};
    unsigned long long contact_reject_side{0};
    unsigned long long contact_reject_visible{0};
    unsigned long long contact_reject_dead{0};
    unsigned long long contact_reject_kind{0};
    // docs/RECON_SLOT_LISTS.md rule (c): the sensor pass published `none` for
    // this (side, target), so the target is in no published contact list.
    // Counted apart from the rule (a) and (b) rejections above so a run can
    // attribute a contact drop to the detection rule rather than to a gate.
    unsigned long long contact_reject_recon_level{0};
    unsigned long long contact_admit_ship{0};
    unsigned long long contact_admit_plane{0};
    unsigned long long bullet_ranges_derived{0};   // 006E9890 gave the gun a range
    // Guns kept out of the AI weapon-facts row because their bullet class never
    // resolved, the CATAPULT case. docs/AI_TARGET_WEIGHT_TERMS.md.
    unsigned long long ai_barrels_unresolved_skipped{0};
    // 0072AD40's sweep. Both stay 0 while no producer fills gun+120h; a
    // non-zero `live` is the first sign that one has appeared.
    unsigned long long gun_pending_timers_expired{0};
    unsigned long long gun_pending_timers_live{0};
    // Packet p17_reload_torpedoes: ReloadTorpedoes calls applied, and the barrels
    // they found still reloading or empty.
    unsigned long long reload_torpedoes_calls{0};
    unsigned long long reload_torpedoes_barrels{0};
    unsigned long long torpedo_ranges_derived{0};
    unsigned long long torpedo_swims_started{0};   // water crossings that became a swim
    // Packet cc8_torpedo_release_spawn. Where a torpedo-carrying gun stops on
    // the way to a shot, one counter per conjunct of the same `want_fire` the
    // gun loop builds. A gun counts as a torpedo gun when its bullet class
    // derived a swim speed, which is the identical test the water crossing
    // uses to decide that a round swims rather than dies at the surface, so a
    // gun counted here is exactly a gun whose shot could reach the swim model.
    // These are gate counters, not reconstructions: no native address produces
    // them. docs/TORPEDO_RELEASE_SPAWN.md.
    unsigned long long torpedo_gun_ticks{0};
    unsigned long long torpedo_gun_targeted{0};    // have_target
    unsigned long long torpedo_gun_accepted{0};    // + 0085ABA0 accepted the angles
    unsigned long long torpedo_gun_settled{0};     // + within the 0.1 deg fire band
    unsigned long long torpedo_gun_window{0};      // + 007F60A0 allowed the bearing
    unsigned long long torpedo_gun_sent{0};        // + 0072D130 sent the 0ADh arm
    unsigned long long torpedo_gun_shots{0};       // + 00730160 made a projectile
    // Packet cc9_g34_torpedo_bot_ready, both states: category-7 ticks with a
    // target, those 008FFF20's own test (009003F8..0090042F) would launch on,
    // those the generic test launches on, and image launches 0085ABA0 refused.
    unsigned long long torpedo_ready_targeted{0};
    unsigned long long torpedo_ready_image{0};
    unsigned long long torpedo_ready_generic{0};
    unsigned long long torpedo_ready_refused_angles{0};
    unsigned long long torpedo_friendly_scans{0};  // 008FFF20 0090058A walked the own list
    unsigned long long torpedo_friendly_holds{0};  // 009007F6 held the launch for a friendly
    // The air drop. 007BBBA0 accepted the request and a round left the plane.
    // Packet cc8_torpedo_gun_assignment. Why a torpedo-category gun is never
    // given a target. The image excludes category 7 from the recon sweep at
    // 008651F5 (`CMP ESI,7 / JE 00865442`), so its only candidate sources are
    // the director's command target and fire target at step 8.7. These count
    // that path on units that actually carry a torpedo-category gun, then each
    // rejection reason inside 00863990.
    // Packet cc9_plane_forced_target_read: pass ticks on a plane or squadron
    // (008636A0 installs the null provider 00861B90 there), how many of them
    // found a stored ship-AI fire target, and how many the switch dropped.
    // Packet cc9_hull_roll_torque: 00827126..00827329 on a torpedo hit of a hull
    // over 500 mass, the message-93h torques posted, and those delivered to the
    // units host one fixed step later (row 9 of the image's step, after 00875E0C).
    unsigned long long roll_torque_calls{0};
    unsigned long long roll_torque_posted{0};
    unsigned long long roll_torque_delivered{0};
    unsigned long long roll_torque_undelivered{0};
    double roll_torque_max_magnitude{0.0};
    unsigned long long plane_null_provider_ticks{0};
    unsigned long long plane_fire_target_reads{0};
    unsigned long long plane_fire_target_nulled{0};
    unsigned long long torpedo_cat_pass_ticks{0};
    unsigned long long torpedo_cat_with_command_target{0};
    unsigned long long torpedo_cat_with_fire_target{0};
    unsigned long long torpedo_cat_score_calls{0};
    unsigned long long torpedo_cat_reject_unknown{0};   // target is not a unit row
    unsigned long long torpedo_cat_reject_liveness{0};  // 00862820
    unsigned long long torpedo_cat_reject_class{0};     // class id below zero
    unsigned long long torpedo_cat_reject_rank{0};      // the rank table answered 0
    unsigned long long torpedo_cat_reject_mask{0};      // 008633D0
    unsigned long long torpedo_cat_reject_range{0};     // distance vs the category range
    unsigned long long torpedo_cat_score_accepted{0};
    unsigned long long torpedo_drops{0};
    unsigned long long torpedo_drop_refusals{0};   // no torpedo-capable gun on the unit
    // Packet cc8_dive_glide, the kind 2Ah drop.
    unsigned long long bomb_drops{0};
    unsigned long long bomb_drop_refusals{0};      // no kind 2Ah gun on the unit
    unsigned long long paratrooper_releases{0};    // packet cc13-para2: 31h rounds released
    // Packet cc8_torpedo_breakoff: drops that cleared the owner's torpedo kind
    // 2Bh bit, so approach+132h goes false on the next approach update.
    unsigned long long torpedo_loadout_cleared{0};
    unsigned long long water_entry_breakups{0};    // 008568E0's two limits rejected the entry
    unsigned long long torpedo_heading_snaps{0};   // 007F6190 snapped the heading onto a window edge
    unsigned long long angle_sets{0};
    unsigned long long angle_refusals{0};
    unsigned long long aim_steps{0};
    unsigned long long trigger_rises{0};
    unsigned long long fire_messages{0};
    unsigned long long fire_if_ready{0};
    unsigned long long can_fire_refusals{0};
    unsigned long long arc_blocks{0};
    unsigned long long arc_unsolved{0};
    unsigned long long shots{0};
    unsigned long long projectiles{0};
    unsigned long long projectile_steps{0};
    unsigned long long sweeps{0};
    unsigned long long water_crossings{0};
    unsigned long long impacts_entity{0};
    unsigned long long impacts_static{0};
    unsigned long long expired{0};
    unsigned long long queued_hits{0};
    unsigned long long dispatched_hits{0};
    unsigned long long ship_hit_records{0};
    unsigned long long part_damages{0};
    unsigned long long hull_damages{0};
    unsigned long long fire_messages_9e{0};   // the 9Eh fire / flood arm
    unsigned long long flood_messages_9e{0};
    unsigned long long attributions{0};       // 0077CE60
    unsigned long long deaths{0};
    unsigned long long kill_credits{0};
    float damage_total{0.0f};
    float first_shot_seconds{-1.0f};
    float first_hit_seconds{-1.0f};
};

// The gun chain for one mission run, owned for the whole run.
// Borrowed immutable asset metadata from the same cache that supplies firing.
// mesh remains valid while this host/cache lives; no gun pose is stored here.
struct GameDeviceVisualMetadata {
    std::size_t unit_index{};
    int platform_key{};
    int device_class{-1};
    const std::string* mesh{};
    std::size_t hierarchy_items{};
    bool source_loaded{};
    bool mount_known{};
};

class GameGunneryHost {
public:
    GameGunneryHost(GameHostLog& log, GameUnitsHost& units, GameMissionLuaHost& lua);
    ~GameGunneryHost();
    GameGunneryHost(const GameGunneryHost&) = delete;
    GameGunneryHost& operator=(const GameGunneryHost&) = delete;

    void set_ship_ai(GameShipAiHost* ai) noexcept;

    // Packet cc9_recon_team_lists: the recon slot of party `side` after the last
    // 008073C0 rebuild, one published triple as unit indices in native order.
    // `triple`: 0 own (+DD8h, head +DDCh), 1 enemy (+DE4h, head +DE8h),
    // 2 neutral (+DF0h), 3 unknown (+DFCh, head +E00h), 4 union (+E08h).
    // 004C3CB0 walks 0, 1 and 3 of the player's slot. False (and `out` empty)
    // before the first rebuild or for a side with no slot.
    bool recon_triple_units(int side, int triple, std::vector<std::size_t>& out) const;
    // Packet p15_free_attack_tick: triple 1 (the enemy chain at +DE8h) with, per
    // entry, whether it is the kind-18h group record 00805490 leaves for a
    // squadron (its unit is the squadron's fused leader) rather than a plane's
    // own class-bucket record.
    bool recon_enemy_contacts(int side, std::vector<std::size_t>& units,
                              std::vector<bool>& squadron_record) const;
    // Packet cc13-fapick3, log-only: the 2D distance from `target` to the nearest
    // unit the sensor pass admits as an observer of `side` (008068C2: present,
    // IsKindOf(05h), a sensor table), or -1 when there is none. Reads only.
    float recon_nearest_observer_distance(int side, std::size_t target) const;
    // Packet p15_free_attack_tick: 00804350 BSP_Unit_SensorsDetectUnit,
    // __fastcall(ECX = observer, EDX = target, mask) RET 4, over the same sensor
    // rows, signature and multipliers the recon pass reads.
    bool sensors_detect_unit_00804350(std::size_t observer, std::size_t target,
                                      std::uint32_t mask);
    // Packet cc9_initial_controlled_unit_load: the scene load's step 19 calls
    // 008073C0 on the local slot directly (004E059B), outside 008079B0's
    // periodic countdown, which it leaves untouched. SUBSTITUTION, labelled:
    // this host's pass covers every side at once, not only the local slot.
    // Returns false when no unit is registered (the pass has nothing to scan).
    bool run_recon_pass_at_scene_load_004e059b();
    // Packet cc9_recon_level_table: 00807A50 BSP_Recon_ForceRefreshNow, the
    // whole body of the Lua native ForceRecon (008AADF0). It stores
    // -1.0e-4f (00D08E6C) into the countdown [00F874B8] and runs 008079B0
    // with a zero step, so the pass runs now and the countdown comes back
    // just under the full period. Returns false with no unit registered.
    bool force_recon_refresh_00807a50();

    // Packet cc9_hud_ray_pick (docs/HUD_PICK_SEGMENT_QUERY.md section 4): 0098ADD0
    // over this host's units, as 009043A0 calls it with kind filter 0. `exclude`
    // is one-based (0 = none) and stands for ignore->vtable[B0h]. True with the
    // nearest hit's one-based unit and point. Read-only: the two trace counters
    // the binding bumps are restored, so no gunnery summary line moves.
    bool query_segment_units(const float from[3], const float to[3], std::size_t exclude,
        std::size_t& hit_unit, float hit_point[3]) const;

    // Packet cc9_landscape_segment_query: 00904400(kind, from, to, record, entity),
    // RET 14h, ECX set by callers but unread. Every caller (007C255E,
    // 008647F4, 009F25FA) passes kind 44h (Landscape) and entity 0, so the
    // 0098ADD0 exclude is 0 (a non-null entity would give entity->vtable[B0h]).
    // True when the segment from -> to hits the kind-44h filter; this host has
    // no islands or terrain, so it answers over its own units, as
    // line_of_sight_00864680 does. Read-only: the binding's mesh and 0085CDB0
    // trace counters are restored. No caller is bound yet.
    bool landscape_segment_hit_00904400(const float from[3], const float to[3]) const;

    // Packet cc9_unit_sight_queries, for the ship AI's 009E7FC0.
    // 009E8116 CALL 00864FD0 (MOV ECX,[ECX+68h]; JMP 00864D90) on [unit+6DCh]:
    // unit `unit`'s own gunnery-pass visibility cache asked about `target`. It is
    // GunneryPassBinding::visible_00864d90 itself, so a miss appends the same
    // cache entry with the same TTL draw the pass would, and it follows the same
    // OFF arm under kGunneryLineOfSightBound. False for an index out of range.
    // Const like the other queries, but it writes the pass cache through impl_.
    bool unit_sees_unit_00864d90(std::size_t unit, std::size_t target) const;
    // 009E8130 CALL 00864BA0 on [unit+6DCh]: MOV ECX,[ECX+68h], the three floats
    // copied to a local, CALL 00864680, RET 0Ch; no cache entry. 00864680 raises
    // the cache OWNER ([cache+0] = [pass+50h], the unit, stored at 00864C12..0F):
    // pose +FCh..+104h (008646F0..00864708) plus class+A8h and Globals+94h
    // (00864793..008647B6). The passed point is used as given. 00904400(44h,
    // &owner point, &point, record, 0) at 008647F4; a hit farther than 25 m
    // (00CFBC80 = 625.0 squared) from the PASSED point (008647FD..00864852) hides
    // it. LABELLED as line_of_sight_00864680 does: 0081DE10 answers 0, the
    // section-span raise branch is not taken, the 0.5 s owner-point cache
    // (cache+10h, 00CE3800) is not kept. Read-only.
    bool unit_sees_point_00864680(std::size_t unit, const float point[3]) const;

    // Packet cc9_component_failures: true while unit `unit_index` carries an active
    // failure of that name in its repair task (task+18h), started by 0093BED0 and
    // retired by 0093C520. "SteeringJam" and "EngineJam" are the ones 008198A0
    // turns into unit+9E4h / unit+9E5h through messages 6Bh / 6Ch, the flags the
    // ship motion reads; this host applies "Explosion" and "Fire" itself.
    bool unit_failure_active(std::size_t unit_index, const char* name) const;
    // Packet cc13_natives1: the ship SetFailure virtual 008132C0 (vtable[194h]) as
    // the Lua binding 0088EFF0 reaches it. "Explosion" schedules message 6Ah's
    // object with msg+30h = explosion_damage; "SteeringJam" sets +9E4h (6Bh only,
    // no record); "EngineJam" sets +9E5h (6Ch) and 0093BD80 pushes {5, "EngineJam",
    // engine_jam_seconds}; "Periscope" is recorded. Names compare case-blind.
    // Returns the arm: 1 Explosion, 2 SteeringJam, 3 EngineJam, 4 Periscope, 0 none.
    int script_set_failure_008132c0(std::size_t unit_index, const std::string& name,
                                    float engine_jam_seconds, float explosion_damage);
    // unit+9E4h (steering) or +9E5h (engine), what 008135C0 answers for the two
    // jam names and what 00826754 / 00826B2E read.
    bool ship_jam_flag(std::size_t unit_index, bool steering) const;

    // Packet cc9_blast_element_parts: the hull segments (0..19, controller+310h
    // slots) of unit `unit_index` that 0092D1F0 drove to zero, in the order they
    // went. Each one is a message 99h the image routes to 0080E440 DetachPart
    // (00934150: debris, effects and the dynamics list); the physics side is the
    // ship motion's to apply.
    std::vector<int> destroyed_hull_segments(std::size_t unit_index) const;

    // 00727BD0 over the authored preference lists, then the twelve per-unit
    // category lists through 00956C20's sequence, then 00864BD0 on every unit
    // that ended with at least one gun. Runs once, after the first create_units.
    void attach_00864bd0();

    // Packet cc8_gunnery_host. The same per-unit work for the units a later
    // create_units appended, and nothing else: the image builds the 558h-byte
    // object at unit+6DCh once per unit in 00810DD0's creation block and has
    // no per-batch refresh, so an already-built unit keeps its guns, its
    // health, its throttle and its category state. Calling attach_00864bd0()
    // a second time instead would duplicate every existing gun and throw the
    // whole host's accumulated state away. Safe to call with no new units.
    void register_new_units_00864bd0();

    // Packet cc9_player_gun_seat: 0077C2A0 delivering message 79h to a unit,
    // the group 1/2 arm of BSP_Unit_ApplyGunAimMessage 00959C20. Other
    // groups are a record. docs/PLAYER_GUN_SEAT.md.
    void apply_gun_aim_message_00959c20(std::size_t unit_index, const GunAimMessage79& message);
    // 0095AC1D's two device walks (00953F60 / 00954000) for a role-owner
    // message whose holder changed; kRoleOwnerDeviceWalkBound.
    void role_owner_device_walk_0095ac1d(std::size_t unit_index, std::uint32_t mask,
                                         std::int32_t slot, bool take);
    // 0080E290: 0085AD00 on each of the unit's turning guns (IsKindOf 22h), which
    // commands an authored rest pair through 0085ABA0. Packet cc13-frontier; the
    // callers run it only under kHelmReleaseGunsToRestCc13Bound.
    void aim_unit_turning_guns_to_rest_0080e290(std::size_t unit_index);
    // 00953B80's [unit+49Ch]: the square of the largest bullet range over the
    // unit's Function 0/1/5/6 guns (Function 6 takes its Bullet[2]).
    float unit_gun_range_sq_049c(std::size_t unit_index) const;

    // One fixed simulation step: the gunnery pass 00864FE0, the aim ticks and
    // the trigger latch, 0072D130's 0ADh send with BSP_Gun_Fire behind it, and
    // the projectile flight with its sweep, impact, hit record and kill credit.
    void fixed_step(float step_seconds);

    // Packet cc13-hitq (bsp/cc13_hitq_switches.hpp): the deferred hit queue
    // 00F899C0 the drain 00926700 walks. Empty unless
    // kHitDeferredDispatchCc13Bound. The count is [00F899C8]; back() is
    // head[+4h] (00926714 / 0092675B), an opaque node with the victim index the
    // drain's subject is (record+0h), or null when empty; the dispatch is
    // 00926750's 009239A0 call; the pop removes whatever head[+4h] is now.
    std::size_t deferred_hit_count() const noexcept;
    const void* deferred_hit_back(std::size_t& victim) const noexcept;
    void dispatch_deferred_hit_009239a0(const void* node);
    void pop_deferred_hit_back();

    const std::vector<GameGunRow>& guns() const noexcept;
    // Render-only read seam. A nonnull empty Mesh is an authored meshless
    // device; null Mesh means its existing asset-cache provider is absent.
    bool device_visual_metadata(std::size_t gun_index,
        GameDeviceVisualMetadata& out) const noexcept;
    // Recompute one original hierarchy item's current world matrix from the
    // SAME cached authored hierarchy, live gun angles/mount and unit_pose as
    // firing. No persistent pose, firing counter or diagnostic is changed.
    // False leaves no usable output; never substitute an identity matrix.
    bool device_visual_node_world(std::size_t gun_index,
        std::size_t hierarchy_item, bsp::CameraMatrix& out) const;
    // Packet cc8_ship_ai_firepower_inputs: the gun indices 00956C20 put in one
    // unit's category list, in insertion order. The ship AI's firepower host
    // walks these where the image walks the list at unit+398h + category*0Ch.
    const std::vector<std::size_t>* unit_category_guns(
        std::size_t unit_index, int category) const noexcept;
    // The authored Bullets row a gun fires, by GameGunRow::bullet_class.
    const GameBulletClassRow* bullet_class_row(int id) const noexcept;
    // Packet cc8_torpedo_release_spawn. The air drop: put one torpedo-capable
    // round of this unit into the air with the plane's own pose and speed.
    //
    // SUBSTITUTION, labelled. The native release site is unread. What is
    // established is that a bomb platform is a Gun subclass (00730B80 calls
    // BSP_Gun_Construct at 00730B88 before installing vtable 00CFE308), so the
    // round a plane drops is made by the same BSP_Gun_SpawnShotAndEffects
    // 0072F830 this host already implements for a ship's tube. This method
    // therefore reuses that spawn and substitutes the release geometry: the
    // plane's position and its forward axis at its own forward speed, because a
    // drop inherits the aircraft's velocity rather than a muzzle speed. The
    // aiming, the barrel timers and the stock decrement are NOT reproduced.
    //
    // Returns true when a round was created.
    bool release_ordnance_drop(std::size_t unit_index);

    // Packet cc8_dive_glide, edited under the integrator's hunk arbitration of
    // 2026-09-19. The dive bomber's release, 009C60F1 and 009C5777, could not
    // use release_ordnance_drop above: that method selects the unit's
    // torpedo-capable rows (`swim_speed > 0`) and clears the kind 2Bh bit on a
    // drop, and a dive bomber carries kind 2Ah - so the selection found
    // nothing and `bombs_spawned` stayed 0 however many releases were counted.
    // The spawn is right and the predicate was wrong: a bomb platform is a Gun
    // subclass (00730B80 calls BSP_Gun_Construct at 00730B88), so its round is
    // made by the same 0072F830 a tube's is. This selects on
    // ordnance_has_general_bomb_2ah - kind 2Ah excluding 2Ch/31h/2Bh/33h/2Dh,
    // the same set 007B9320 tests for BSP_WeaponController -
    // and clears 2Ah rather than 2Bh.
    //
    // `predicted_impact` is approach+D8h/+E0h at the release tick, which
    // 009C7D71 has already made the predicted impact point of a bomb dropped
    // this tick; it is carried on the round only so the run can print
    // predicted against actual. It is not an image field.
    // `release_fall_time` is the caller's 009C7D71 tf for THIS tick, carried
    // for the same reason and with the same caveat (packet cc8_dive_aim item 2:
    // the aimdive census `tf=` column is last-sampled and cannot be read as a
    // release value). Pass a negative when the caller has none.
    // `skill_row` is the dropping plane's PilotBot level (0..5) for 006E4D50's
    // ThrowMul (packet cc9_bomb_drop_scatter, GUNNERY 130); -1 asks the units host.
    // Packet cc13-para2: with `paratrooper` non-null the release is 006E4D50's
    // for the unit's first 31h (MParatrooper) gun instead of its 2Ah one: the
    // same 006E0A70 velocity and 006E1F00 cone and drift (the four stream-1
    // draws only under bsp::kParatrooperReleaseScatterDrawBound), written to
    // *paratrooper instead of a bomb shot; the units host owns the round.
    struct ParatrooperRelease {
        float position[3]{};
        float velocity[3]{};
        float drift[3]{};          // block+38h -> round+348h; y = 0
        int bullet_class{-1};
        int owner_side{0};
    };
    bool release_bomb_drop(std::size_t unit_index,
                           const float predicted_impact[3],
                           float release_fall_time, int skill_row = -1,
                           ParatrooperRelease* paratrooper = nullptr);
    // True when 006E4D50's scatter draws are taken (kBombDropScatterBound).
    static bool bomb_drop_scatter_bound() noexcept;
    // Packet cc13-homing1: spawn one rocket from the unit's Rocket-type gun row
    // (the one firing `bullet_class` when it is >= 0) and, under
    // kRocketHomingBound only, fill its homing block as 0080AA20 does. The
    // caller's own switch gates the call (kOpportunisticRocketBound).
    // `target_plus_one` 0 or a target failing IsKindOf(5) leaves it unguided.
    // SUBSTITUTION, labelled: the launch geometry (the rack's mount and the
    // platform release vtable 00CFE308 are unread): the plane's origin and
    // rows, at the plane's forward speed, like release_ordnance_drop.
    bool release_rocket_0080aa20(std::size_t unit_index, std::size_t target_plus_one,
                                 const float body_point[3], float homing_angle,
                                 int bullet_class = -1);
    const std::vector<GameGunneryUnitRow>& unit_rows() const noexcept;
    // Packet cc9_water_surface_law. The gunnery host owns every death, so the
    // units host asks it two things for an aircraft in the water:
    // * unit_dead(): stands for 007BC5B0's health test `unit+150h <= 0`;
    // * kill_unit_00926d90(): 007CE3A7's BSP_MissionEntity_Kill(unit, 1), the
    //   plane tick's depth kill, through the same funnel a gunfire death takes.
    bool unit_dead(std::size_t unit_index) const noexcept;
    // Packet cc9_lua51_kill_args: the victim's kill attribution as 0077CE60 left it,
    // for 009813A0's callback arguments and lastAttacker filter: the last attacker
    // (victim+2C4h/+2C8h, one based, 0 = none) and the credited slot (+2D8h).
    std::size_t kill_last_attacker(std::size_t unit_index) const noexcept;
    int kill_credited_slot(std::size_t unit_index) const noexcept;
    // Packet cc9_command_building_capture_bind (ship AI host's capture):
    // * refresh_unit_side(): re-reads unit+54h into the unit's gunnery row after a
    //   CommandBuilding's neutralize or flip (00928F50 SetPartyRace);
    // * repair_unit_to_fraction(): 006F47F0's hull arm, health += max * fraction,
    //   clamped to max (the parts arm is repair_unit_guns_006f47f0, packet cc13-gunD).
    void refresh_unit_side(std::size_t unit_index);
    void repair_unit_to_fraction(std::size_t unit_index, float fraction);
    // Packet cc9_lua_hit_listeners: one record per gunnery hit that reached the
    // victim's health (the post-multiplier amount lost), queued at the hit dispatch
    // after 0077CE60's attribution. take_hit_events() drains it. Nothing drains it
    // yet, so it is gameplay-inert.
    struct GameGunneryHitEvent {
        std::size_t victim;
        std::size_t shooter;
        int bullet_class;
        float damage;
        // For the Lua hit listeners' per-(victim, attacker) rate limit (00988510).
        // Inert here: the Lua host reads them only behind its own switches.
        // [src+1Ch], src = the ordnance's vtable[108h](); -1 with no ordnance.
        // Packet cc9_projectile_team_id: the shot's GameProjectileRow::team_id_1c.
        int attacker_player_index{-1};
        // [hit+0h] when it is a live child of the victim other than the victim
        // itself; 0 otherwise. No producer yet: the host builds no device entities.
        std::uint32_t device_entity{0};
        float fire_caused{0.0f};   // hit record +4Ch (008273F5), 0 when no fire started
        float leak_caused{0.0f};   // hit record +50h (00827358), 0 when no leak started
        // [[src+4h]+8h], the ordnance kind 00988510's rate limit keys on. Filled
        // with the fired class's sub-type (classDesc+8h, the host's
        // bullet_sub_type); that the two are the same field is a hypothesis.
        int ordnance_kind{-1};
    };
    std::vector<GameGunneryHitEvent> take_hit_events();
    void kill_unit_00926d90(std::size_t unit_index, int cause);
    // Packet cc9_airfield_destruction_rule: 006D41E6..006D41EF, the airfield's
    // destroy arm, vtable[70h](0) = 0077D1A0 BSP_UnitInstance_DestroyAndBroadcast
    // with recurse 0. Through the same death funnel; its own counter.
    void destroy_unit_0077d1a0(std::size_t unit_index, int recurse);
    // Packet cc9_set_invincible_floor (docs/LUA_BINDING_MISSION.md "SetInvincible,
    // 00897A50"; docs/UNIT_DAMAGE_AND_DEATH.md). unit+150h, the invincibility
    // float 0042ED80 (vtable[F4h]) stores and fans to the unit's children. The
    // Lua host calls the setter for SetInvincible; a fraction of maximum health,
    // 1.0 for `true`, 0.0 to release. Kept per unit index before and after the
    // unit's gunnery row exists. With kUnitInvincibilityFloorBound it floors
    // every 00879070 damage write and refuses the 008110F0 sink; the query is
    // IsInvincible's `inv > 0` (00897CB0) and 007BC5B0's `unit+150h <= 0` test.
    void set_unit_invincibility(std::size_t unit_index, float value);
    // Packet cc9_override_hp: the Lua native OverrideHP 008C1930, unit+36Ch = value
    // then 00877B90(value). Behind kLuaOverrideHpBound; counted either way.
    void override_hp_008c1930(std::size_t unit_index, float value);
    // 0095DA00 -> 0087D730 -> 00879070 for a script-sourced amount (AddDamage,
    // 0088E000): the party multiplier, the invincibility floor and the death funnel,
    // as a hit's damage takes them. No caller yet (packet cc9_difficulty_multipliers).
    void apply_script_damage_0095da00(std::size_t unit_index, float amount);
    // Packet cc13-gunC C1b (include/bsp/ship_damaged_gfx_remove_cc13.hpp): vtable[1B4h]
    // as SetDamagedGFXLevel 0088F710 calls it; 00819CC0 for a ship. Returns the gun rows
    // destroyed and detached. Nothing happens unless kShipDamagedGfxRemoveCc13Bound.
    int set_damaged_gfx_level_00819cc0(std::size_t unit_index, int level);
    // Packet cc13-gunB: 00728080 on one gun row (index into guns()), through
    // 00442AE0, 0087D730, 00879070 and, at zero health, the 0072B5A0 destroyed
    // failure. A no-op unless kGunDestroyedFailureCc13Bound. No caller yet (packet C).
    void apply_gun_damage_00728080(std::size_t gun, float amount);
    // Packet cc13-gunD: 006F47F0's gun loop on one unit's guns, `fraction` =
    // Repair[level] * a / GunRepairTime. A no-op unless kGunSubobjectRepairCc13Bound.
    void repair_unit_guns_006f47f0(std::size_t unit_index, float fraction);
    // Packet p16_torpedo_stock_property: 00815850 for a ship, the value
    // GetProperty(ship, "TorpedoStock") pushes. False for an index the host
    // does not hold.
    bool torpedo_stock_total_00815850(std::size_t unit_index, std::int32_t& total) const;
    float unit_invincibility(std::size_t unit_index) const noexcept;
    // Packet cc9_ship_fire_stance: the weapon director arm of UnitSetFireStance
    // 008A6490 / UnitHoldFire 008A6AC0 (stance 0) for a ship: 0071BE80, whose
    // 5Ah messages leave director+3Ch = 0071D560(stance) and +3Dh =
    // 0071D580(stance). Always recorded; the two readers answer the stored
    // values only with kShipFireStanceBound, else 008363E0's default true.
    void set_director_fire_stance_0071be80(std::size_t unit_index, int stance);
    // Packet cc9_repair_enable_route: the 9Fh arm of 00821E80 (008220D7), which calls
    // 00939FD0 to store the flag at task+45h. Always counted; applied only with
    // kHullRepairEnableRouteBound.
    void set_hull_repair_enabled_00939fd0(std::size_t unit_index, bool enabled);
    bool director_allow_fire_3c(std::size_t unit_index) const noexcept;
    bool director_allow_move_3d(std::size_t unit_index) const noexcept;   // 009F5614's read
    // Packet cc13-jm08: 00720450's raw copy of director+3Ch / +3Dh (0072045D..
    // 00720468) from src to dst, no 00836210 side effect. An unset src row is
    // copied as unset, i.e. 008363E0's 1/1.
    void copy_director_allow_bytes_00720450(std::size_t dst, std::size_t src);
    // 007BC5B0's tail, past its flight-state and unit+AA0h tests: true when
    // [unit+150h] <= 0 (not invincible) or the unit is player-held (+1ACh not 8
    // and 00927F10 false). With kUnitInvincibilityFloorBound off it keeps the
    // earlier substitution, unit_dead() (packet cc9_water_invincible_test).
    bool water_gate_007bc5b0(std::size_t unit_index, bool player_held) const noexcept;
    // 007C2610 BSP_Unit_MinKind21ComponentSpeed (body 007C2610-007C2674, RET, the
    // float in ST0): the minimum round V0 [[gun+3F8h]+34h]+50h over the unit's
    // child list (unit+48h, next +44h) of components answering IsKindOf(21h),
    // MRFSGun, the fixed forward guns. It starts at FLT_MAX (00D7A248,
    // 7F7FFFFFh), so a unit with no such gun answers FLT_MAX; the running
    // minimum is a float and a tie keeps the later gun's equal value.
    // SUBSTITUTION, labelled: this host's category 0 (PLANEGUN) rows stand for
    // the class 21h test, and each row's V0 is its first ammunition record's.
    // Packet cc9_plane_death_modes, for docs/FIGHTER_GUNFIRE_RATE.md section 3.
    float min_fixed_gun_muzzle_speed_007c2610(std::size_t unit_index) const noexcept;
    // Packet cc9_pilot_surface_climbout: a death-mode draw for the units host,
    // 00BD2F10(stream, low, high). Stream 1 is the shared generator the gunnery
    // draws also use (007CA914, the mode choice); stream 0 is its own generator
    // (007BBFA0's ExplosionExplosionDelay draw, 007BCABB's engine-fire delay).
    // Under BSP_GUNNERY_RNG_STREAMS=1, a MEASUREMENT option, each (stream, unit)
    // key gets its own deterministic generator instead.
    float death_mode_draw_00bd2f10(int stream, std::size_t unit_index, float low, float high);
    // Packet cc13-sinkfix: 0074EC50's per-leak U(0, 1) (00BD2F10 ECX = 1 at
    // 0074ECA5) for the units host. On the shared generator it is the same call
    // as death_mode_draw_00bd2f10(1, ...); under BSP_GUNNERY_RNG_STREAMS=1 it has
    // its own key (sink_leak, ship), so it shares no keyed stream with the
    // death-mode draws (Lua BreakShip / SetDeadMeat) on the same ship.
    float sink_leak_draw_0074eca5(std::size_t ship_index, float low, float high);
    // Packet g43_plane_spin_side: unit+800h as 007BBCF0 last stored it from a
    // direct hit's element kind (0 lwing, 1 rwing), -1 (007CFDB4) before any or
    // with kPlaneSpinSideBound OFF.
    int plane_spin_side_800(std::size_t unit_index) const;
    // Packet cc9_damage_smoke_draws (docs/GUNNERY_OPEN_ITEMS.md 127): 008227E0 on
    // the unit's damage-smoke controller (unit+10A0h) for one update step 7, with
    // the units host's scaled delta. False when kDamageSmokeDrawsBound is off: the
    // caller keeps its record.
    bool damage_smoke_tick_008227e0(std::size_t unit_index, float delta);
    // Packet s52_sink_draws: 00825F20's two ShipSystems spawners on a sinking ship
    // for one motion step, 00933DA0 (grid) and/or 00933BB0 (spray); their stream-1
    // draws are taken only when kShipSystemsSinkDrawsBound is on.
    void ship_systems_sink_draws_00933da0_00933bb0(std::size_t unit_index,
        bool grid_00933da0, bool spray_00933bb0);
    // Packet s53_sub_wake_draws: 00854650's stream-1 pick (008546E4) for a
    // submarine whose hull y is in (-15, -3), and on 1 in 15 the 00933BB0 spray
    // (008546FC); the draws are taken only when kSubmarineWakeDrawsBound is on.
    void submarine_wake_draws_008546df(std::size_t unit_index, float hull_y);
    // Packet s53_wreck_debris: 00935540's stream-1 draws for one sinking wreck
    // (eight per piece); `class_kind` is unit+C4h, `z_extent` the body's local box
    // length (B+44h - B+38h, z), `num_per_meter` ShipGlobals DebrisNumPerMeter.
    // Taken only when kSinkingWreckDebrisDrawsBound is on.
    void sinking_wreck_debris_draws_00935540(std::size_t unit_index, std::int32_t class_kind,
        float z_extent, float num_per_meter);
    // unit+36Ch, the maximum health, for a writer outside this host: 006F38E0's
    // CommandBuilding level-up stores HP[level] / 100 * class HP there and leaves
    // the current health alone (SHIP_AI 169). No-op for an unbuilt index.
    void set_unit_max_health_036c(std::size_t unit_index, float value);
    // Packet cc9_command_building_garrison (SHIP_AI_OPEN_ITEMS 175): 006F3660's
    // re-create of a dead garrison member, as a revive in place (SUBSTITUTION:
    // the image builds a new entity). Clears the death, restores health to the
    // unit's maximum, turns its guns' triggers back on, clears its attacker and
    // targets and resets its damage-control and damage-smoke state. False when
    // the unit is not dead. The caller restores the units-host scene flags and
    // the party (then refresh_unit_side).
    bool revive_unit_garrison_006f3660(std::size_t unit_index);
    // Packet cc9_shipyard_production (bsp::kShipyardProductionBound): the strategic
    // map's purchase at a shipyard, 00673A10. `shipyard` is the shipyard's unit
    // index; `entry` is 1-based, or <= 0 for the first idle entry (006534E0). The
    // screen sends A7h (byte 1), repeated until the entry holds `vehicle_class`
    // (one A7h when it is <= 0), then for a plane class `count - 1` A8h (byte 1,
    // screen+334h; they cycle the entry's Equipments index, not a unit count),
    // then A9h (byte 1), which builds at once when a hangar is free. Each order
    // builds one unit. `chosen_entry` is 1-based; `reason` is "built", "queued"
    // or why it was refused. docs/GUNNERY_OPEN_ITEMS.md section 136.
    bool shipyard_order(std::size_t shipyard, int entry, int count, int& chosen_entry,
                        std::string& reason, int vehicle_class = 0);
    // Packet cc9_player_weapon_group_fire (kPlayerWeaponGroupFireBound in the
    // .cpp): the player's weapon-group fire, message 79h (005484F0 -> 00959C20).
    // Only group 5 (Function 8/9 launchers, the depth charges) is bound. `held`
    // is the 99h byte; it stays on the launchers until a call with held=false,
    // as a held key sends it every frame. docs/GUNNERY_OPEN_ITEMS.md 140.
    bool player_fire_weapon_group(std::size_t unit, int group, bool held, std::string& reason);
    // Packet cc9_player_torpedo_group (kPlayerTorpedoGroupFireBound in the .cpp):
    // message 79h group 4 (0095A1CC), the player's torpedo tubes. The aim is the
    // world yaw from the unit to `target` (a unit name), or `yaw_radians` when
    // `target` is empty, in this host's heading convention (atan2(x, z)). `held`
    // is the 99h byte (+34h); +35h, pressed, is its rising edge per unit. A
    // surface unit fires while it is held and a tube is trained; a submarine or
    // a torpedo boat fires one ready tube per press. GUNNERY_OPEN_ITEMS 152.
    bool player_fire_torpedo_group(std::size_t unit, const std::string& target, float yaw_radians,
                                   bool held, std::string& reason);
    // unit+368h, the instance armour, for a writer outside this host: 006F38E0
    // stores Armor[level] / 100 * class Armour there (SHIP_AI 169). The direct-hit
    // passes read it under kCommandBuildingGunfireArmourBound (GUNNERY 129).
    void set_unit_armour_0368(std::size_t unit_index, float value);
    // Packet cc9_gun_aim_terms (docs/GUN_AIM_TERMS.md). The dogfight fine aim's
    // distortion, 009FA7E0 on dogfight-gun+4h, called at 009FCCD7 only on a tick
    // where the fine aim runs. One wander per unit, created on the first call
    // (009FA620's four draws) and stepped with two draws per call, all on the
    // shared 00BD2F10 stream 1 (key Draw::aim_wander under the option). `out`
    // receives what 009FCD41-009FCD8A adds to the aim angles: (+20h, +24h),
    // divided by tuning+648h FighterAimMulVersusAI when `owner_is_fighter`.
    // SUBSTITUTION, labelled: the +64Ch FighterAimMulVersusPlayer arm needs the
    // target's +DF4h slot record, and every target here is AI-held.
    void fighter_aim_distortion_009fa7e0(std::size_t unit_index, float dt,
                                         bool owner_is_fighter, float out[2]);
    const GameGunnerySummary& summary() const noexcept;

    // docs/RECON_SLOT_LISTS.md rule (c). This host owns the one
    // bsp::ReconSensorPassState in the process because it owns the tick that
    // steps it (008073C0 runs before the gunnery pass, not inside it). The
    // ship AI's union-list read reaches it through
    // GameShipAiHost::bind_gunnery, which GameGunneryHost::set_ship_ai already
    // calls; it is the same binding the firepower host uses for the category
    // gun lists. docs/RECON_SENSOR_PASS_BINDING.md.
    const bsp::ReconSensorPassState& recon_sensor_pass_state() const noexcept;

    void log_sample(unsigned long long step_index, unsigned long long interval);
    void report();

    struct Impl;

    // Packet cc9_ship_torpedo_response: every live round of a class with a
    // WaterTravelSpeed (an MTorpedo), for the ship AI's world torpedo walk
    // (world+21Ch/+220h, 00856360's registrar). Heading is atan2(vx, vz), the
    // units host's pose convention, standing in for record+46Ch: a host round
    // runs straight on its launch heading. SUBSTITUTION, labelled.
    struct LiveTorpedo {
        unsigned long long serial{0};
        std::size_t owner_unit{0};    // one based
        int owner_side{0};
        float position[3]{};
        float velocity[3]{};
        float swim_seconds{0.0f};
        bool swimming{false};
        float water_travel_speed{0.0f};
        bool started_under_water{false};   // torpedo+49Ch (s57_torpedo_from_sub)
        // record+46Ch: 00856637's commanded heading, 10000.0 [00D0C310] until
        // 008574A0 seeds it from the hull yaw on the first swim step (the round's
        // commanded_heading, kept by torpedo_swim3d_step under
        // kTorpedoSwimDepthHomingBound). Packet cc13-torpgyro.
        float commanded_heading_46c{10000.0f};
    };
    std::vector<LiveTorpedo> live_torpedoes() const;
    // Packet cc9_navigator_force_torpedo. 008A7200's device walk: every gun of
    // the unit (the direct children at unit+48h answering IsKindOf(20h)) whose
    // Function ([[dev+3F4h]+80h]) is 7 gets vtable[1D8h](0, 0, 0), 00730160,
    // stopping after the first when `first_only`. LABELLED: the host's gun rows
    // in their build order stand for the child list, and the fire is queued for
    // the unit's next aim-and-fire pass, where it takes 00730160's arm without
    // the bot, the fire request 0072D2C0/0072D130 or CanFire 0085A830.
    // Answers the number of guns marked.
    int force_torpedo_fire_008a7200(std::size_t unit_index, bool first_only);
    // Packet cc9_gunnery_pass_byte_7d. 00863780 on the unit's pass ([unit+6DCh]):
    // store `value` at pass+7Dh; when it changed to 0 and the owner is a unit
    // (vtable[5Ch](5)), 00728000 on each gun of the category-7 list unit+3ECh.
    // `site` is the caller's call address, for the census. The byte is tracked both
    // ways; with kGunneryPassByte7dBound false the pass still answers 1 and the
    // clears are only counted.
    void set_pass_byte_7d_00863780(std::size_t unit_index, bool value, std::uint32_t site);
    // Packet cc9_fire_function_guns_now. 009E2B60 (__thiscall on the ship-AI state,
    // body 009E2B60-009E2BA4, RET 0): for each direct child of the unit
    // ([[state+4]+AA8h]+48h, siblings at +44h) that answers IsKindOf(24h)
    // (009E2B76) and whose weapon Function [[child+3F4h]+80h] is 8 (009E2B86), call
    // child->vtable[1F0h]() (009E2B99): 006FDF60 sets gun+4D4h on a class-24h gun,
    // 006FDC50 is vtable[1E8h](1) on the class-27h depth-charge launcher (its
    // parent is 24h). The image hard-codes 8; `function` is the caller's, so the
    // image's call is function == 8. LABELLED: the host's gun rows in build order
    // stand for the child list, the device Type (gun_turning_class) stands for the
    // IsKindOf(24h) test, and the slot is applied at the gun's next pass, after
    // the gun's own request for that step. Answers whether any gun was marked.
    // Inert until a caller exists (the ship AI's attack sub-states record it).
    bool fire_function_guns_now_009e2b60(std::size_t unit_index, int function);
    // Packet cc9_torpedo_gate_bytes (docs/SENTITY_INIT_PASSES.md section 10).
    // 008637D0, 00863840, 00863920 and 008638B0 are one loop over the category
    // lists 00E0A510 (AA {1,5,6}), 00E0A4F8 (artillery {1,2,3,4,6}), 00E0A520
    // (torpedo {7}) and 00E0A528 (depth charge {8,9}): true at the first
    // category whose byte +70h+c is set, whose [+60h]->vtable[4](c) gate passes
    // and that 008633D0(c, target) accepts. `list_address` is one of the four
    // kUnitGunnery*GroupAddress constants; any other answers false. Reads the
    // unit's live pass state; a unit with no pass answers false.
    bool group_accepts_target_008637d0(std::size_t unit_index, std::uint32_t list_address,
                                       std::size_t target_index) const;
    // Packet cc9_ship_torpedo_response: 00BD2F10 on stream 1 for the ship AI's
    // torpedo draws (009F0AD0, 009F1316..009F13BE). Default: the shared
    // generator every gunnery draw uses, in call order. BSP_GUNNERY_RNG_STREAMS=1:
    // its own generator keyed by (unit), a measurement substitution.
    float ship_ai_draw(std::size_t unit_index, float low, float high);
    // 00816650's 00BD2F10(ECX=1) draws for 009FA260's hull aim pick, on the same shared
    // generator (include/bsp/hull_aim_shared_stream_cc13.hpp). BSP_GUNNERY_RNG_STREAMS=1:
    // keyed by (shooter unit).
    float hull_aim_point_draw_00816650(std::size_t unit_index, float low, float high);
    // 009F6A20 (BSP_WeaponDirector_AutoTargetConstruct) at 009F6A8F: 00BD2F10(ECX=1,
    // 0.0f, 1.0f), negated into the auto-target think countdown, once per director.
    // Default: the shared stream-1 generator. BSP_GUNNERY_RNG_STREAMS=1: its own
    // generator keyed by (unit), a measurement substitution. Returns the draw.
    float auto_target_phase_draw_009f6a8f(std::size_t unit_index);
    // Packet cc9_movie_interface_and_reseed: 00BD2FD0(stream, seed) as 005CD1A0
    // makes it (005CD1B0 stream 1 = 12345, 005CD1BC stream 0 = 54321).
    // SUBSTITUTION, labelled: the image seeds MT19937 (sgenrand, docs/RANDOM_STREAMS.md);
    // this host's stream-1 stand-in is the shared LCG, whose state takes the
    // seed. It reaches only the default shared path: under
    // BSP_GUNNERY_RNG_STREAMS=1 every draw is key-local and none reads it.
    // Stream 0 has no host generator. Returns whether a generator took the seed.
    bool reseed_shared_stream_00bd2fd0(int stream, std::uint32_t seed);

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
