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
//     bytes. Rule (c), the detection value, is not applied: no sensor pass runs
//     in this process, so every enemy in class scope is a contact.
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
#include "bsp/gun_platform_arc.hpp"
#include "bsp/gunnery_tables.hpp"
#include "bsp/projectile_impact.hpp"
#include "bsp/unit_gunnery_pass.hpp"

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
    float blast_damage_max{0.0f};// "Blast.BlastDamageMax"
    float blast_range{0.0f};     // "Blast.BlastRange"
    float mass{0.0f};            // "Mass"
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
    float max_range{0.0f};            // 00731020's answer, the bullet `Range`
    float muzzle_speed{0.0f};
    std::vector<bsp::GunFiringArc> arcs;   // platform+3Ch, from `Windows`
    float rest_horz{0.0f};            // "RestAngles[1]", platform+94h
    float rest_vert{0.0f};            // "RestAngles[2]", platform+90h
    bsp::GunTurningAngles angles{};   // +480h/+484h/+494h/+498h
    bsp::GunRotationSpeeds speeds{};  // descriptor+88h/+8Ch
    bsp::GunFireRequestState fire{};  // +454h/+478h/+450h/+414h
    int barrel_num{1};                // +448h
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
    // that ended with at least one gun. Runs once, after create_units.
    void attach_00864bd0();

    // One fixed simulation step: the gunnery pass 00864FE0, the aim ticks and
    // the trigger latch, 0072D130's 0ADh send with BSP_Gun_Fire behind it, and
    // the projectile flight with its sweep, impact, hit record and kill credit.
    void fixed_step(float step_seconds);

    const std::vector<GameGunRow>& guns() const noexcept;
    const std::vector<GameGunneryUnitRow>& unit_rows() const noexcept;
    const GameGunnerySummary& summary() const noexcept;

    void log_sample(unsigned long long step_index, unsigned long long interval);
    void report();

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
