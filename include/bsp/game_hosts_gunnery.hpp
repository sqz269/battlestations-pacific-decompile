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

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/gun_aiming.hpp"
#include "bsp/gun_pending_timers.hpp"
#include "bsp/ordnance_kinds.hpp"
#include "bsp/gun_platform_arc.hpp"
#include "bsp/gunnery_tables.hpp"
#include "bsp/projectile_impact.hpp"
#include "bsp/unit_gunnery_pass.hpp"

namespace bsp {
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
    std::string unit_name;
    int platform_key{0};              // the Lua `Platforms` key
    std::string platform_name;        // "Name"
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
    float max_range{0.0f};            // 00731020's answer, the bullet `Range`
    float muzzle_speed{0.0f};
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
};

// One projectile in flight, as 006E8430 created it.
struct GameProjectileRow {
    std::size_t gun_row{0};
    std::size_t owner_unit{0};        // one based
    int owner_side{0};
    int bullet_class{-1};
    bsp::ProjectileFlightState flight{};
    float position[3]{};
    float life{0.0f};
    bool alive{false};
    bool swimming{false};             // past the water crossing, on the swim
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
    // Packet cc8_dive_glide. A bomb released by the dive-bomb task, and the
    // predicted impact point approach+D8h/+E0h carried at the release tick so
    // the run can print predicted against actual. INSTRUMENTATION; the image
    // keeps neither flag nor point on the round.
    bool is_bomb{false};
    float predicted_impact[3]{};
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
    // The air drop. 007BBBA0 accepted the request and a round left the plane.
    // Packet cc8_torpedo_gun_assignment. Why a torpedo-category gun is never
    // given a target. The image excludes category 7 from the recon sweep at
    // 008651F5 (`CMP ESI,7 / JE 00865442`), so its only candidate sources are
    // the director's command target and fire target at step 8.7. These count
    // that path on units that actually carry a torpedo-category gun, then each
    // rejection reason inside 00863990.
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
class GameGunneryHost {
public:
    GameGunneryHost(GameHostLog& log, GameUnitsHost& units, GameMissionLuaHost& lua);
    ~GameGunneryHost();
    GameGunneryHost(const GameGunneryHost&) = delete;
    GameGunneryHost& operator=(const GameGunneryHost&) = delete;

    void set_ship_ai(GameShipAiHost* ai) noexcept;

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

    // One fixed simulation step: the gunnery pass 00864FE0, the aim ticks and
    // the trigger latch, 0072D130's 0ADh send with BSP_Gun_Fire behind it, and
    // the projectile flight with its sweep, impact, hit record and kill credit.
    void fixed_step(float step_seconds);

    const std::vector<GameGunRow>& guns() const noexcept;
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
    bool release_bomb_drop(std::size_t unit_index,
                           const float predicted_impact[3],
                           float release_fall_time);
    const std::vector<GameGunneryUnitRow>& unit_rows() const noexcept;
    // Packet cc9_water_surface_law. The gunnery host owns every death, so the
    // units host asks it two things for an aircraft in the water:
    // * unit_dead(): stands for 007BC5B0's health test `unit+150h <= 0`;
    // * kill_unit_00926d90(): 007CE3A7's BSP_MissionEntity_Kill(unit, 1), the
    //   plane tick's depth kill, through the same funnel a gunfire death takes.
    bool unit_dead(std::size_t unit_index) const noexcept;
    void kill_unit_00926d90(std::size_t unit_index, int cause);
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
    };
    std::vector<LiveTorpedo> live_torpedoes() const;
    // Packet cc9_ship_torpedo_response: 00BD2F10 on stream 1 for the ship AI's
    // torpedo draws (009F0AD0, 009F1316..009F13BE). Default: the shared
    // generator every gunnery draw uses, in call order. BSP_GUNNERY_RNG_STREAMS=1:
    // its own generator keyed by (unit), a measurement substitution.
    float ship_ai_draw(std::size_t unit_index, float low, float high);

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
