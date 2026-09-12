// 0095EB40, the routine the ship AI asks for a firepower rating, and the two
// callers that shape its query: 0095F080 (a 60-sample range profile) and
// 009E6240 (the avoidance refresh).
//
// Packet cc_ai_bearing_rating, worker agent/cc-ai-bearing-rating.
// Project C:/Users/sqz269/bsp.gpr, program /battlestationspacific.exe; Ghidra
// was READ-ONLY for this packet. Every descriptive name here is a hypothesis,
// not a recovered symbol. docs/SHIP_AI_BEARING_RATING.md carries the evidence,
// address by address, the Coverage table and the Corrections.
//
// What 0095EB40 actually rates (docs/SHIP_AI_RING_SCAN.md called it "the ship
// class's rating of a bearing" and left the body unread): it is a damage
// estimate, not an arc score. `this` is the UNIT, not the class descriptor
// (009E5DC4 `MOV ECX,[ESI]`, 009E635B `MOV ECX,[EDI+14h]`). It walks the unit's
// twelve gunnery-category lists, keeps every operational turning gun whose
// category the query enables and whose range the query is inside, and sums an
// expected damage over a time window: direct damage past the target's armour,
// plus flooding damage, plus fire damage. The bearing (word 5) enters only
// through the optional per-mount `can this mount bear` test 0085B7D0, which the
// ring-scan path turns on (009E8171) and the range-profile path turns off
// (009F2ECB).
//
// This header builds on bsp/gunnery_tables.hpp (GunneryCategory, the category
// record and list-node offsets, kDeviceKindGun, kUnitOffAnyWeaponMaxRange),
// bsp/unit_gunnery_pass.hpp (kUnitOffCategoryRecords, kUnitOffCategoryRanges,
// kUnitGunneryListNodeOff*), bsp/unit_weapons.hpp (NativeHandle) and
// bsp/projectile_kinds.hpp (kWeaponClassOffDamageMin .. kWeaponClassOffFireChance
// and the kProjectileSubType* ids). It redefines none of their names.
//
// The seventeen-dword query block is the same storage bsp/ship_ai_ring_scan.hpp
// carries as the raw ShipAiRingScanClassQuery; that struct stays as it is and
// this header adds the typed view of the same 44h bytes.

#pragma once

#include <cstddef>
#include <cstdint>

#include "bsp/gunnery_tables.hpp"
#include "bsp/projectile_kinds.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------

// 0095EB40, `float __thiscall(Unit* this, ShipAiFirepowerQuery* block)`,
// RET 4 at 0095EB99, 0095F065 and 0095F077; body 0095EB40-0095F079.
// The float result comes back in ST0.
inline constexpr std::uint32_t kShipAiFirepowerRatingAddress = 0x0095EB40u;

// 0095F080, `void __thiscall(Unit* this, ShipAiFirepowerQuery block /*by value,
// 44h bytes*/, float* out60, int prefer_long_range)`, RET 4Ch at 0095F165;
// body 0095F080-0095F167.
inline constexpr std::uint32_t kShipAiFirepowerRangeProfileAddress = 0x0095F080u;

// 009E6240, the avoidance refresh that also calls 0095EB40 (009E635E). Its
// contract is documented, not reconstructed, by this packet.
inline constexpr std::uint32_t kShipAiFirepowerAvoidanceRefreshAddress = 0x009E6240u;

// The producer of all seventeen dwords: 009F1BC0 fills nested+127Ch at
// 009F2A04..009F2ED2 and 009F2EA1..009F2ED2 before 009E7FC0 overwrites words 5,
// 6, 7 and the two bytes at 009E813D..009E8178. 009F3090 runs them in that
// order: 009F309B calls 009F1BC0, 009F30A2 calls 009E7FC0.
inline constexpr std::uint32_t kShipAiFirepowerQueryProducerAddress = 0x009F1BC0u;

// ---------------------------------------------------------------------------
// The 44h-byte query block
// ---------------------------------------------------------------------------
//
// Offsets are the displacements 0095EB40 uses off EDI. Every producer address
// in a comment is a store in 009F1BC0 or 009E7FC0 (checklist rule 4: the
// meaning comes from the writer, not from this reader's use).
struct ShipAiFirepowerQuery {
    // +00h. Range to the target, metres. 0095EB68 rejects the whole call unless
    // range < unit+494h; 0095EBCB rejects a category unless range <= its
    // unit+430h entry; 0095ECE5 rejects a mount unless range <= the projectile
    // class's +60h. Producer: 009F2A04 from nested+11E0h; 0095F080 overwrites it
    // with the sweep value.
    float range = 0.0f;

    // +04h. The target's hull length, the x axis of the hit-accuracy profile.
    // Producer: 009F2A54 from [target+538h]+A0h ("Length", docs/SHIP_CLASS_FIELDS.md),
    // default 100.0f at 009F2AB9 when there is no target.
    float target_length = 0.0f;

    // +08h. Per-shot damage cap and, scaled, the cap on every output.
    // Producer: 009F2A44 from [target+370h], default 10000.0f at 009F2AA1.
    float damage_cap = 0.0f;

    // +0Ch. The damage a shot must beat for every projectile sub-type except
    // torpedo. Producer: 009F2A2C from [target+538h]+4Ch, 0.0f at 009F2A91.
    float armour = 0.0f;

    // +10h. The same for sub-type 0Ah (torpedo). Producer: the virtual
    // [[target+538h]]->vtable[24h]() at 009F2A3C, 0.0f at 009F2A99.
    float armour_torpedo = 0.0f;

    // +14h. Bearing in radians. 0095EBA1 wraps it in place to [-pi, pi] before
    // the loop. Producer: 009F2A18 and 009E8153 from nested+11DCh; 009E5DA0
    // replaces it with wrap(word5 - slot.angle_08) at 009E5DB4.
    float bearing = 0.0f;

    // +18h. The damage window in seconds. 20.0f on both paths (009F2EA1 and
    // 009E814B, both from 00CE3930). It is the numerator of the shot rate, the
    // ceiling on the flooding and fire terms, and the scale of the output cap.
    float window_seconds = 0.0f;

    // +1Ch. The horizon the ready-round count uses. 30.0f on the range-profile
    // path (009F2EB1 from 00CE38C8) and 60.0f on the ring path (009E8161 from
    // 00CEB4B0). Torpedo mounts use 0.0f instead (0095EC6A).
    float ready_horizon_seconds = 0.0f;

    // +20h. Divisor of the fire term. Producer: 009F2A6B from [target+538h]+6B8h
    // ("DamageThreshold", docs/SHIP_CLASS_FIELDS.md), default 10000.0f at
    // 009F2A7F and 009F2AB1.
    float damage_threshold = 0.0f;

    // +24h. Written by 009F1BC0 (009F2B95, 009F2C6B, 009F2CF6) and never read by
    // 0095EB40.
    float unused_word9 = 0.0f;

    // +28h..+34h, outputs. 0095EB4E..0095EB5D zeroes all four on entry;
    // 0095EFEF..0095F043 writes them capped. 009E5DA0 copies them into
    // slot+24h, +20h, +1Ch and +28h at 009E5DCC..009E5DED.
    float out_artillery = 0.0f;    // +28h, sub-types 4, 5, 6, 7
    float out_small_calibre = 0.0f; // +2Ch, sub-types 1, 2, 3, 10h
    float out_torpedo = 0.0f;      // +30h, sub-type 0Ah
    float out_depth_charge = 0.0f; // +34h, sub-type 0Bh

    // +38h. Written by 009F1BC0 (009F2A10, 009F2E9B) and never read by 0095EB40.
    float unused_word14 = 0.0f;

    // +3Ch..+3Fh, the four category gates. Each enables a fixed set of gunnery
    // categories at 0095EBD7..0095EC1E; a category outside every set can never
    // contribute. Producers: 009F2AE7/009F2E0F, 009F2B20/009F2E29,
    // 009F2BC2/009F2E5C and 009F2B6D/009F2E43, from [[unit+6DCh] queries] and
    // the bytes at [0080E160(unit)+220h..+223h].
    std::uint8_t allow_machine_gun = 0;   // +3Ch, category 1
    std::uint8_t allow_artillery = 0;     // +3Dh, categories 2, 3, 4 and 6
    std::uint8_t allow_torpedo = 0;       // +3Eh, category 7
    std::uint8_t allow_depth_charge = 0;  // +3Fh, categories 8 and 9

    // +40h. When set, every mount must pass 0085B7D0 for this bearing and range
    // (0095EDE2). 1 on the ring path (009E8171), 0 on the range-profile path
    // (009F2ECB), 1 on the avoidance path (009E634F).
    std::uint8_t require_bearing = 0;

    // +41h. When set, a mount contributes only the rounds ready inside
    // ready_horizon_seconds (00727D70); when clear it contributes its whole
    // barrel count and is skipped outright if [device+3B8h] is set (0095EC5F,
    // 0095EC8B). 1 on all three paths (009F2ED2, 009E8178, 009E6240 leaves the
    // caller's byte in place).
    std::uint8_t use_ready_rounds = 0;

    std::uint8_t pad[2] = {0, 0}; // +42h, +43h; the image never reads these.
};

static_assert(sizeof(ShipAiFirepowerQuery) == 0x44, "the block is 17 dwords");
static_assert(offsetof(ShipAiFirepowerQuery, bearing) == 0x14, "word 5");
static_assert(offsetof(ShipAiFirepowerQuery, out_artillery) == 0x28, "word 10");
static_assert(offsetof(ShipAiFirepowerQuery, allow_machine_gun) == 0x3C, "byte 3Ch");
static_assert(offsetof(ShipAiFirepowerQuery, require_bearing) == 0x40, "byte 40h");

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------

inline constexpr int kShipAiFirepowerCategoryCount = 12; // 0095EF83 `CMP ESI,0Ch`

// 0095EC42 pushes 22h, not the 20h the gunnery rebuild 00956C20 pushes at
// 00956C72. 22h is the turning-gun family base (docs/ENTITY_CLASS_IDS.md): the
// rating skips the fixed gun 21h, the bomb platform 25h and the catapult 28h
// even when their category is enabled.
inline constexpr int kShipAiFirepowerDeviceKindTurningGun = 0x22;

// 00D7A280, the double the arc-free excess-damage rule multiplies by.
inline constexpr double kShipAiFirepowerHalf = 0.5;

// 00D7A370, the divisor of the output cap, and 00D7A24C, its floor.
inline constexpr double kShipAiFirepowerCapWindowDivisor = 5.0;
inline constexpr float kShipAiFirepowerCapFloor = 1.0f;

// 0095EC68: sub-type 7 (the torpedo CATEGORY, not the projectile sub-type) asks
// for rounds ready now rather than inside the query horizon.
inline constexpr float kShipAiFirepowerTorpedoReadyHorizon = 0.0f;

// The projectile class field 0095ECE2 and 006EB061 read as the effective max
// range. bsp/projectile_kinds.hpp names +58h, +68h and +70h but not +60h, and
// no Lua reader in docs/WEAPON_CLASS_DESCRIPTOR.md writes it.
inline constexpr std::size_t kShipAiFirepowerWeaponClassOffMaxRange = 0x60;

// The ammunition record reached as [[device+354h]+74h]. 0095EE07 divides the
// window by +2Ch, so +2Ch is a period in seconds; 0095ECD4 copy-assigns the
// nested record at +48h over the record itself for the flak alternate; +34h is
// the projectile class.
inline constexpr std::size_t kShipAiFirepowerDeviceOffAmmoOwner = 0x354;
inline constexpr std::size_t kShipAiFirepowerAmmoOwnerOffRecord = 0x74;
inline constexpr std::size_t kShipAiFirepowerAmmoOffCyclePeriod = 0x2C;
inline constexpr std::size_t kShipAiFirepowerAmmoOffProjectileClass = 0x34;
inline constexpr std::size_t kShipAiFirepowerAmmoOffFlakAlternate = 0x48;

// The device fields the loop reads directly.
inline constexpr std::size_t kShipAiFirepowerDeviceOffDestroyed = 0x3B8; // 0095EC8B
inline constexpr std::size_t kShipAiFirepowerDeviceOffBarrelCount = 0x448; // 0095EC98

// 0095F080's sweep: 60 samples of 50 m starting at 50 m. 00D19BDC is the first
// range (float 50.0f, 0095F087), 00CE3938 the step (double 50.0, 0095F140).
inline constexpr int kShipAiFirepowerProfileSamples = 60;
inline constexpr float kShipAiFirepowerProfileFirstRange = 50.0f;
inline constexpr double kShipAiFirepowerProfileStep = 50.0;

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00605070, the in-place angle wrap 0095EBA1 applies to the query bearing:
// fmod by 2*pi (00CE3828), then +2*pi while <= -pi (00CE3D18) or -2*pi while
// > pi (00CE3D28).
float ship_ai_firepower_wrap_angle_00605070(float radians) noexcept;

// 00419010, the clamped linear interpolation. Returns y0 when x0 == x1 exactly
// (00419026 JP) and otherwise clamps y0 + (t-x0)/(x1-x0) * (y1-y0) into
// [min(y0,y1), max(y0,y1)].
float ship_ai_firepower_lerp_clamped_00419010(float x0, float y0,
                                              float x1, float y1,
                                              float t) noexcept;

// The two damage limits a mount is judged against: 0095ED05..0095ED71 takes the
// larger of the direct and blast damage at each end.
struct ShipAiFirepowerDamageBand {
    float lower = 0.0f; // max(DamageMin +ACh, Blast.BlastDamageMin +B4h)
    float upper = 0.0f; // max(DamageMax +B0h, Blast.BlastDamageMax +B8h)
};

// 0095ED75..0095EDB0. The fraction of shots that beat `armour`: 1.0 at the lower
// limit, 0.0 at the upper, clamped. A mount whose fraction is not > 0 is
// dropped at 0095EDB0.
float ship_ai_firepower_kill_fraction(const ShipAiFirepowerDamageBand& band,
                                      float armour) noexcept;

// 0095EE3D..0095EE79. The damage a hit is expected to land past `armour`,
// clamped into [0, damage_cap] by 00415690. Below the lower limit it is the
// band midpoint minus the armour; inside the band it is half the headroom left.
float ship_ai_firepower_excess_damage(const ShipAiFirepowerDamageBand& band,
                                      float armour, float damage_cap) noexcept;

// 0095EE07..0095EE25. Shots inside the window, or 1.0 when the period is not
// positive.
float ship_ai_firepower_shot_rate(float window_seconds, float cycle_period) noexcept;

// 0095EF90..0095EFC8. The ceiling every output and the return value are clamped
// to: damage_cap * max(1.0f, window_seconds / 5.0).
float ship_ai_firepower_output_cap(float damage_cap, float window_seconds) noexcept;

// The projectile class fields one mount contributes. Offsets from
// bsp/projectile_kinds.hpp except max_range (see the constant above).
struct ShipAiFirepowerProjectileClass {
    // The [ammo+34h] pointer itself (0095ECDB). 006EB060 takes it in ECX and
    // 0085B7D0 takes it as its first stack argument, so the handle travels with
    // the fields.
    NativeHandle handle = 0;
    int sub_type = 0;        // +08h,  kWeaponClassOffSubType
    float max_range = 0.0f;  // +60h
    float damage_min = 0.0f; // +ACh,  kWeaponClassOffDamageMin
    float damage_max = 0.0f; // +B0h,  kWeaponClassOffDamageMax
    float blast_damage_min = 0.0f; // +B4h, kWeaponClassOffBlastDamageMin
    float blast_damage_max = 0.0f; // +B8h, kWeaponClassOffBlastDamageMax
    float water_damage = 0.0f;     // +BCh, kWeaponClassOffWaterDamage
    float fire_damage = 0.0f;      // +C0h, kWeaponClassOffFireDamage
    float fire_chance = 0.0f;      // +C4h, kWeaponClassOffFireChance
};

ShipAiFirepowerDamageBand ship_ai_firepower_damage_band(
    const ShipAiFirepowerProjectileClass& projectile) noexcept;

// The two gameplay-settings scalars 0095EEC2 and 0095EEEA multiply by.
// docs/GAMEPLAY_SETTINGS.md: +3B0h `WaterTickDamage` (default 100),
// +3ACh `FireTickDamage` (default 40).
struct ShipAiFirepowerTickDamage {
    float water_tick_damage = 0.0f; // settings+3B0h
    float fire_tick_damage = 0.0f;  // settings+3ACh
};

// 0095EE29..0095EEFA, one mount's contribution. `shots` is the shot rate,
// `hit_probability` the 006EB060 result, `rounds` the barrel or ready-round
// count that 0095EE31 FIMULs in as an integer.
float ship_ai_firepower_contribution(const ShipAiFirepowerQuery& query,
                                     const ShipAiFirepowerProjectileClass& projectile,
                                     const ShipAiFirepowerTickDamage& tick,
                                     float shots,
                                     float hit_probability,
                                     int rounds) noexcept;

// 0095EBD7..0095EC1E. True when the query's four gates let this gunnery
// category through. Categories 0, 5, 0Ah and 0Bh are never enabled.
bool ship_ai_firepower_category_enabled(const ShipAiFirepowerQuery& query,
                                        int category) noexcept;

// 0095EEF0..0095EF63. Which output a projectile sub-type accumulates into.
enum class ShipAiFirepowerBucket {
    kNone,          // 0095EF63 FSTP ST0, the contribution is dropped
    kArtillery,     // +28h
    kSmallCalibre,  // +2Ch
    kTorpedo,       // +30h
    kDepthCharge,   // +34h
};

ShipAiFirepowerBucket ship_ai_firepower_bucket(int projectile_sub_type) noexcept;

// ---------------------------------------------------------------------------
// The host
// ---------------------------------------------------------------------------

struct ShipAiFirepowerResult {
    float total = 0.0f;         // the ST0 return, capped at 0095F04A
    float artillery = 0.0f;     // query +28h
    float small_calibre = 0.0f; // query +2Ch
    float torpedo = 0.0f;       // query +30h
    float depth_charge = 0.0f;  // query +34h
};

// One pure-virtual per native call site or unit/device field read inside
// 0095EB40, in call order. docs/SHIP_AI_BEARING_RATING.md's host table repeats
// the list with the call-site address of each.
class ShipAiFirepowerHost {
public:
    virtual ~ShipAiFirepowerHost() = default;

    // [unit+494h], 0095EB62. kUnitOffAnyWeaponMaxRange.
    virtual float unit_max_weapon_range() = 0;

    // [unit+394h + category*0Ch], 0095EBB3. kUnitOffCategoryRecords with
    // kUnitGunneryCategoryRecordOffGate: the list's element count.
    virtual int category_device_count(int category) = 0;

    // [unit+430h + category*4], 0095EBC4. kUnitOffCategoryRanges.
    virtual float category_max_range(int category) = 0;

    // [unit+398h + category*0Ch], 0095EC24. The list head; 0 ends the walk.
    virtual NativeHandle category_list_head(int category) = 0;

    // [node+4h], 0095EF6D. kUnitGunneryListNodeOffNext.
    virtual NativeHandle list_next(NativeHandle node) = 0;

    // [node+8h], 0095EC3A. kUnitGunneryListNodeOffPayload.
    virtual NativeHandle list_device(NativeHandle node) = 0;

    // 0095EC46, the virtual [[device]+5Ch](22h). True for a turning gun.
    virtual bool device_is_turning_gun(NativeHandle device) = 0;

    // 0095EC52, 00729F10. [[device+3F0h]+720h], [device+3B8h] and [device+5Dh]
    // all clear.
    virtual bool device_is_operational(NativeHandle device) = 0;

    // 0095EC84, 00727D70. Counts the [device+448h] floats at [device+414h] that
    // are <= `horizon`.
    virtual int device_ready_rounds(NativeHandle device, float horizon) = 0;

    // [device+3B8h], 0095EC8B. Only consulted when use_ready_rounds is clear.
    virtual bool device_is_destroyed(NativeHandle device) = 0;

    // [device+448h], 0095EC98. Only consulted when use_ready_rounds is clear.
    virtual int device_barrel_count(NativeHandle device) = 0;

    // [[device+3F4h]+80h], 0095ECAA. The weapon Function, GunneryCategory.
    virtual int device_weapon_function(NativeHandle device) = 0;

    // [[device+354h]+74h], 0095ECB7. The ammunition record.
    virtual NativeHandle device_ammo_record(NativeHandle device) = 0;

    // 0095ECD4, 0095CF80. Copy-assigns the nested record at ammo+48h over the
    // ammunition record, so the projectile class read next is the flak
    // alternate. Only for Function 6 with allow_artillery clear and
    // allow_machine_gun set (0095ECB0..0095ECCC).
    virtual void ammo_select_flak_alternate(NativeHandle ammo) = 0;

    // [ammo+34h] then its fields, 0095ECDB and 0095ECE2..0095ED4B.
    virtual ShipAiFirepowerProjectileClass ammo_projectile_class(NativeHandle ammo) = 0;

    // [ammo+2Ch], 0095EE07.
    virtual float ammo_cycle_period(NativeHandle ammo) = 0;

    // 0095EDC9, 006EB060 with ECX = the projectile class. Returns 0 when range
    // is at or past the class's +60h, else the WeaponHitAccuracy profile for the
    // class's Function at (target_length, range / max_range).
    virtual float weapon_hit_probability(NativeHandle projectile_class,
                                         float range,
                                         float target_length) = 0;

    // 0095EDFA, 0085B7D0 with ECX = the device. Only when require_bearing.
    virtual bool device_can_bear(NativeHandle device,
                                 NativeHandle projectile_class,
                                 float bearing,
                                 float range) = 0;

    // 0095EEAD and 0095EED8, 00424C40 then [settings+3B0h] and [settings+3ACh].
    virtual ShipAiFirepowerTickDamage gameplay_tick_damage() = 0;
};

// 0095EB40 whole. Writes the four outputs into `query` and returns them with the
// total. `query.bearing` is wrapped in place, as the image does at 0095EBA1.
ShipAiFirepowerResult ship_ai_firepower_rating_0095eb40(ShipAiFirepowerQuery& query,
                                                        ShipAiFirepowerHost& host);

// 0095F080 whole. Takes the block by value as the image does (RET 4Ch), wraps
// word 5 inline (0095F083..0095F0E2), then sweeps `range` over the 60 samples
// and stores each rating. With `prefer_long_range` the sample at index i is
// penalised by (60 - i) whole units (0095F11A FISUB), which biases the profile
// toward longer ranges.
void ship_ai_firepower_range_profile_0095f080(ShipAiFirepowerQuery query,
                                              float out[kShipAiFirepowerProfileSamples],
                                              bool prefer_long_range,
                                              ShipAiFirepowerHost& host);

}  // namespace bsp
