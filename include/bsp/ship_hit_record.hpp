#pragma once
// The ship family's own hit-record handler: the vtable[ECh] override every
// shell, torpedo and depth-charge hit on a destroyer, cruiser, battleship,
// submarine, mothership, cargo, landing ship or torpedo boat runs before the
// shared damage formula sees the record.
//
// Addresses: 00826F10 0092D1F0 0080FA50 0080FF80 0080FFD0 0092CED0 004155B0
//            00BD2F10 0064B170 009553D0.
// Evidence and coverage: docs/SHIP_HIT_RECORD.md, reports/ship_hit_record.json.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// native routines are __thiscall on raw pointers; these are new C++ interfaces
// over the same arithmetic, not drop-in binary replacements.
//
// The damage formulas themselves are not restated: hull_damage_00470510 and
// part_damage_004705c0 live in bsp/unit_hit_path.hpp and this handler calls
// them unchanged. The component-failure roll it makes at 00827450 is
// bsp/unit_fire_flooding.hpp's 0093BED0.
#include <cstddef>
#include <cstdint>

#include "bsp/unit_hit_path.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Literals the listing loads by address.
// ---------------------------------------------------------------------------

// 00D7A220, the double the hull and part passes compare the class Mass against
// before they will subtract a part's health.
inline constexpr double kShipHitPartDamageMassFloor = 100.0;
// 00CE3840, the double the roll-torque branch compares the class Mass against.
inline constexpr double kShipHitRollTorqueMassFloor = 500.0;
// 00CE3DC0, the divisor that turns applied damage into an effect count.
inline constexpr double kShipHitEffectDamagePerUnit = 10.0;
// 00D099A8 (float) and 00D099A0 (the same value as a double): the ceiling on
// that count. Six bits' worth, so a hit can never ask for more than 63.
inline constexpr float kShipHitEffectCountMax = 63.0f;
// 00D19620, the health 0092D1F0 parks in a part's slot once it is destroyed,
// and the value it treats as "already destroyed" on the way in.
inline constexpr float kShipPartDestroyedHealth = -10000.0f;

// ---------------------------------------------------------------------------
// IsKindOf class ids, the literals pushed into vtable[5Ch]. The ids are
// docs/ENTITY_CLASS_IDS.md's; only the two the handler tests are named here.
// ---------------------------------------------------------------------------

// 00826F51, 008274B4, 00827677: MSubmarine. A submarine is the only ship a
// depth charge may hurt, and the only one that shows no hull impact effect.
inline constexpr int kShipHitKindSubmarine = 0x08;
// 00826F44: MDepthCharge, the shot kind vetoed against every other hull.
inline constexpr int kShipHitKindDepthCharge = 0x2C;
// 008270E5: MTorpedo, the only shot kind that rolls a hull.
inline constexpr int kShipHitKindTorpedo = 0x2B;

// 00826FAC and 008275C1: the hull-segment kind and the part-hit entry kind that
// select a part-health subtraction. The same enum in both places.
inline constexpr int kShipHitSegmentKindBreakable = 0x0D;

// ---------------------------------------------------------------------------
// Offsets, named by the routine that reads them.
// ---------------------------------------------------------------------------

// On the 0x1188 unit instance of docs/UNIT_INSTANCE_LAYOUT.md.
inline constexpr std::size_t kShipOffOwnerId = 0x054;          // 0082706B
inline constexpr std::size_t kShipOffWorldValidFlag = 0x0C8;   // owner side, 00827471
inline constexpr std::size_t kShipOffWorldMatrix = 0x0CC;      // 00827706
inline constexpr std::size_t kShipOffWorldTranslation = 0x0FC; // = 0CCh + 30h, 0082748A
inline constexpr std::size_t kShipOffInverseValidFlag = 0x10C; // 008276F6, 00827712
inline constexpr std::size_t kShipOffInverseMatrix = 0x110;    // 0082770C, 0082772E
inline constexpr std::size_t kShipOffArmour = 0x368;           // 00826F8B, 008275A0
inline constexpr std::size_t kShipOffClassDescriptor = 0x538;  // 00826F78; 009553D0 writes it
inline constexpr std::size_t kShipOffRepairTask = 0xA20;       // 0082744A
inline constexpr std::size_t kShipOffPartsObject = 0x1018;     // 00827026
inline constexpr std::size_t kShipOffHitAccumulator = 0x10D0;  // 008270BE clears it

// On the vehicle-class descriptor. +B0h is docs/VEHICLE_CLASS_FIELDS.md's Mass.
inline constexpr std::size_t kShipClassOffMass = 0x0B0; // 00826FD0

// On the breakable-parts object 0092D1F0 walks.
inline constexpr std::size_t kShipPartsOffHealthVector = 0x30C; // 0092D227
inline constexpr std::size_t kShipPartsOffHealthBegin = 0x310;  // 0092D21E
inline constexpr std::size_t kShipPartsOffHealthEnd = 0x314;    // 0092D22F
inline constexpr std::size_t kShipPartsOffEnableFlags = 0x34C;  // 0092D210
inline constexpr std::size_t kShipPartsOffOwner = 0x1C;         // 0092D2C2
inline constexpr std::size_t kShipPartsOffRollBody = 0x2C;      // 00827195

// On the hit record, in addition to bsp/unit_hit_path.hpp's HitRecord fields.
inline constexpr std::size_t kShipHitOffImpactPoint = 0x08;   // 0082751F, 0082771E
inline constexpr std::size_t kShipHitOffSegmentKind = 0x30;   // 00826FAC
inline constexpr std::size_t kShipHitOffFireRate = 0x4C;      // 008273F5 writes
inline constexpr std::size_t kShipHitOffFloodRate = 0x50;     // 00827358 writes

// Game-settings floats (the 00424C40 singleton).
inline constexpr std::size_t kShipSettingsOffRollTorqueScale = 0x590; // 00827251
inline constexpr std::size_t kShipSettingsOffRollMassRoot = 0x594;    // 008271D8

// ---------------------------------------------------------------------------
// Session message ids. 0075B430 takes the id; the vtable is the message class.
// ---------------------------------------------------------------------------

// 0080FA50, vtable 00D0334C. The same message BSP_LuaBinding_SetFireDamage
// (0088E320) and BSP_LuaBinding_SetWaterDamage (0088E790) send.
inline constexpr int kShipMessageSetFireOrWaterDamage = 0x9E;
// 0080FF80 and the copy the part loop inlines, vtable 00D034A0.
inline constexpr int kShipMessageHullImpactEffect = 0x90;
// 0080FFD0, vtable 00D034C8.
inline constexpr int kShipMessageAddHullTorque = 0x93;
// 0092CED0, vtable 00D033EC.
inline constexpr int kShipMessagePartDestroyed = 0x99;

// The selector 0080FA50 stores at message+24h. The two Lua setters fix the
// mapping: 0088E320 loads 0 with XOR EBP,EBP and 0088E790 loads 1.
enum class ShipDamageChannel : int {
    kFire = 0,  // 0082740F passes this
    kWater = 1, // 00827372 passes this
};

// All four session routes are 0077C2A0(this, message, 7, 0).
inline constexpr int kShipMessageRouteChannel = 7;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00826F38..00826F59. The one case in which the handler refuses the record and
// hands it back to the dispatcher, which then walks entity+3Ch upwards
// (docs/PROJECTILE_IMPACT.md's 009239A0 step 5). Everything else is handled.
bool ship_hit_vetoed_00826f38(bool shot_present,
                              bool shot_is_depth_charge,
                              bool hull_is_submarine) noexcept;

// 004155B0, the three-pointer clamp: max(lo, min(value, hi)).
float clamp_004155b0(float value, float lo, float hi) noexcept;

// 008274C2..00827513 and the inline copy at 00827685..008276E6. The x87
// rounding mode is forced to truncate-toward-zero for the store, so this is a
// truncation and not a round.
int ship_hit_effect_count(float applied_damage) noexcept;

// 00827056..00827085, the row of the difficulty table a hit is scaled by.
// False means the branch at 00827078 fell through and nothing is scaled: a
// campaign hit on a hull the local player does not own.
bool ship_difficulty_row(int session_mode,
                         bool unit_is_local_players,
                         unsigned campaign_index,
                         unsigned* row) noexcept;

// 00827043..008270B7. The row picked by ship_difficulty_row and the multiply
// at 008270AD. `scaled` false means no table row applied and the damage is
// returned unchanged.
struct ShipDifficultyScaling {
    bool scaled{false};
    unsigned index{0};
    float damage{0.0f};
};
ShipDifficultyScaling ship_difficulty_scaled_hull_damage(float hull_damage,
                                                         int session_mode,
                                                         bool unit_is_local_players,
                                                         unsigned campaign_index,
                                                         const float* difficulty_table,
                                                         std::size_t difficulty_table_size) noexcept;

// 00827263..008272A8: the horizontal cross product of the roll axis and the
// normalised impact direction, reduced to a sign.
int ship_roll_direction_sign(const float roll_axis[3], float impact_dir_x, float impact_dir_z) noexcept;

// 00827126..008272DB. `impact_dir` is the raw vector shot->vtable[34h] returns;
// only its x and z survive the normalise. The result is the vector 0080FFD0
// packs into message 93h.
struct ShipRollTorque {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};
ShipRollTorque ship_roll_torque(const float roll_axis[3],
                                const float impact_dir[3],
                                float hull_damage_no_armour,
                                float class_mass,
                                float torque_scale,
                                float mass_root) noexcept;

// 0092D1F0. `health` is the part's slot on the way in; the result says what the
// slot becomes and whether the destroyed message is sent.
struct ShipPartDamageResult {
    bool applied{false};   // false when the flag byte is negative or the part was already destroyed
    bool destroyed{false}; // the message+99h route at 0092D2D2
    float health{0.0f};
};
ShipPartDamageResult ship_part_damage_0092d1f0(float health,
                                               float damage,
                                               signed char enable_flag) noexcept;

// ---------------------------------------------------------------------------
// The host. One virtual method per native call site the sequence below reaches;
// every method names the callee and the site.
// ---------------------------------------------------------------------------

// The fields of the hit record this handler reads on top of HitRecord's.
struct ShipHitRecordView {
    int segment_kind{0};            // +30h
    float impact_point[3]{};        // +08h, +0Ch, +10h
    bool shot_present{false};       // +04h non-null
    bool shot_is_depth_charge{false};   // shot->vtable[5Ch](2Ch)
    bool shot_is_torpedo{false};        // shot->vtable[5Ch](2Bh)
    bool weapon_present{false};         // shot->vtable[108h]() non-null
};

struct ShipHitRecordHost {
    virtual ~ShipHitRecordHost() = default;

    // --- the hull's own state -------------------------------------------
    virtual bool hull_is_submarine() = 0;            // vtable[5Ch](8) at 00826F51
    virtual float hull_armour() = 0;                 // this+368h at 00826F8B
    virtual float class_armour_virtual() = 0;        // [this+538h]->vtable[24h]() at 00826F83
    virtual float class_mass() = 0;                  // [this+538h]+B0h at 00826FD0
    virtual void clear_hit_accumulator() = 0;        // this+10D0h at 008270BE

    // --- the two damage formulas (bsp/unit_hit_path.hpp) -----------------
    virtual float hull_damage(float armour) = 0;     // 00470510 at 00826FA3, 0082704D, 0082712E
    virtual float part_damage(float armour, int part_hit_index) = 0; // 004705C0 at 008275B5

    // --- difficulty scaling ---------------------------------------------
    virtual int session_mode() = 0;                  // game+1FE4h at 0082705B
    virtual bool unit_is_local_players() = 0;        // this+54h vs the player row at 00827075
    virtual unsigned campaign_difficulty_index() = 0;    // game+6ACh at 00827085
    virtual float difficulty_multiplier(unsigned index) = 0; // 00432650's vector at 008270AD

    // --- the part-health subtraction ------------------------------------
    virtual void apply_part_damage(int part_index, const float direction[3], float damage) = 0;
    // 0092D1F0(this+1018h, index, &dir, damage) at 0082703E and 0082765B
    virtual void impact_direction(float out[3]) = 0;  // shot->vtable[34h] at 00826FF0, 00827144, 0082760D

    // --- the roll torque -------------------------------------------------
    virtual void roll_axis(float out[3]) = 0;         // [[this+1018h]+2Ch]+20h via 00C32000 at 008271B2
    virtual float settings_roll_torque_scale() = 0;   // settings+590h at 00827251
    virtual float settings_roll_mass_root() = 0;      // settings+594h at 008271D8
    virtual void route_add_hull_torque(const ShipRollTorque& torque) = 0;
    // 0080FFD0 then 0077C2A0 at 00827312 and 00827329

    // --- fire and flooding ------------------------------------------------
    virtual float weapon_water_damage() = 0;  // weapon->vtable[10h] at 0082734C, 00827363
    virtual float weapon_fire_damage() = 0;   // weapon->vtable[14h] at 008273E9, 00827400
    virtual float weapon_fire_chance() = 0;   // weapon->vtable[18h] at 008273B1
    virtual float random_unit_float() = 0;    // 00BD2F10(0, 1.0f) at 008273CA
    virtual void record_flood_rate(float rate) = 0; // hit+50h at 00827358
    virtual void record_fire_rate(float rate) = 0;  // hit+4Ch at 008273F5
    virtual void route_set_damage_channel(ShipDamageChannel channel, float amount, bool flag) = 0;
    // 0080FA50 then 0077C2A0 at 00827372 / 00827389 and 0082740F / 00827426

    // --- the component-failure roll (bsp/unit_fire_flooding.hpp) ----------
    virtual void roll_component_failure(float applied_damage) = 0; // 0093BED0 at 00827450

    // --- the local player's damage indicator ------------------------------
    virtual bool is_local_players_unit() = 0;    // this vs 00E188D8 at 00827455
    virtual bool shooter_world_position(float out[3]) = 0;
    // 004704E0 at 0082745F/0082746A, the pose refresh 00414DB0 at 0082747C and owner+FCh
    virtual void push_damage_direction(const float shooter_world_position[3]) = 0; // 0064B170 at 00827497

    // --- the hull and part impact effects ---------------------------------
    virtual void route_hull_impact_effect(int count, const float local_point[3]) = 0;
    // 00414E10 / 004142E0 / 0080FF80 / 0077C2A0 at 00827542..00827576
    virtual void route_part_impact_effect(int count, const float local_point[3]) = 0;
    // the cached-inverse path and the inlined 90h message at 008276F6..008277C1

    // --- the tail ----------------------------------------------------------
    virtual bool apply_base_hit_record() = 0; // 008777D0(this, hit) at 008277FC
};

// 00826F10 in order. Returns what the native leaves in AL: false only on the
// depth-charge veto, otherwise 008777D0's own result.
bool apply_ship_hit_record_00826f10(ShipHitRecordHost& host,
                                    const HitRecord& hit,
                                    const ShipHitRecordView& view) noexcept;

} // namespace bsp
