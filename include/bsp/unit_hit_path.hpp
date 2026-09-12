#pragma once
// The path from an impact record to a health change, and the damage-control
// repair tick that shares the same helper.
//
// Addresses: 008777D0 007BBCF0 00470510 004705C0 00879810 0093C770 0093C860
//            0093CA20 0087BCC0 00934150 0080E440 0080E410 00878340.
// Evidence and coverage: docs/UNIT_HIT_PATH.md, reports/unit_hit_path.json.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The
// native routines are __thiscall on a raw pointer; these are new C++ interfaces
// over the same arithmetic, not drop-in binary replacements.
#include <cstddef>
#include <cstdint>

#include "bsp/unit_damage.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Float literals the listing loads by address.
// ---------------------------------------------------------------------------

// 00D7A208, the operand of MOVSS at 00879810. The listing computes -0.0f - x.
inline constexpr float kUnitHitNegativeZero = -0.0f;
// 00D7A218, the 0.0f the part pass compares the winning damage against.
inline constexpr float kUnitHitZero = 0.0f;
// 00D7A244 (FF 7F FF FF), the seed of the "largest part damage so far" slot.
inline constexpr float kUnitHitWorstPartSeed = -3.402823466e+38f;
// 00CED9E0, the detail value 0087BCC0 passes for an IsKindOf(1Bh) unit.
inline constexpr float kUnitPartDetailKind1B = 0.949999988f;

// ---------------------------------------------------------------------------
// IsKindOf class ids, the literals pushed into vtable[5Ch].
// ---------------------------------------------------------------------------

inline constexpr int kUnitKindAlternateArmourSource = 0x06; // 008777ED
inline constexpr int kUnitKindOwnerScopedModifier = 0x20;   // 00877824
inline constexpr int kUnitKindRepairableChild = 0x04;       // 0093C8FC
inline constexpr int kUnitKindRepairExcluded = 0x0F;        // 0093C90F
inline constexpr int kUnitKindReducedPartDetail = 0x1B;     // 0087BE0D
inline constexpr int kUnitHitPartEntryAlternateArmour = 4;  // CMP [EAX],4 at 0087790F

// ---------------------------------------------------------------------------
// Offsets this packet adds to the unit instance. The health offsets themselves
// live in include/bsp/unit_damage.hpp and are not repeated.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kUnitPartTableOffset = 0x344;     // 0087BD4F, 0087BDBC
inline constexpr std::size_t kUnitPartInstanceOffset = 0x360;  // 0087BEA4
inline constexpr std::size_t kUnitArmourOffset = 0x368;        // 0087BCF4, read 00877811
inline constexpr std::size_t kUnitOneMarkerOffset = 0x164;     // 0087BD09, set to 1.0f
inline constexpr std::size_t kUnitConditionFlagOffset = 0x378; // 0087BD19, read 0093C91D
inline constexpr std::size_t kUnitOwnerScopedUnitOffset = 0x3F0; // 00877835
inline constexpr std::size_t kUnitArmourProviderOffset = 0x354;  // vehicle-class descriptor
inline constexpr std::size_t kUnitChildListHeadOffset = 0x48;    // 0093C8EC
inline constexpr std::size_t kUnitChildListNextOffset = 0x44;    // 0093C9F5

// Vehicle-class descriptor fields 0087BCC0 reads. docs/VEHICLE_CLASS_FIELDS.md
// attributes +48h to the Lua key HP and +4Ch to Armour.
inline constexpr std::size_t kVehicleClassOffPartVectorHeader = 0x18; // begin +1Ch, end +20h
inline constexpr std::size_t kVehicleClassOffPartVectorBegin = 0x1C;
inline constexpr std::size_t kVehicleClassOffPartVectorEnd = 0x20;
inline constexpr std::size_t kVehicleClassOffHitPoints = 0x48;
inline constexpr std::size_t kVehicleClassOffArmour = 0x4C;
inline constexpr std::size_t kVehicleClassOffPartSet = 0x50;
inline constexpr std::size_t kVehicleClassPartDescStride = 0x30; // IMUL 2AAAAAABh / SAR 3
inline constexpr std::size_t kUnitPartInstanceSize = 0x1AC;      // PUSH 1ACh at 0087BE15

// ---------------------------------------------------------------------------
// The hit record, as its consumers read it. The producer is unread, so this is
// a consumer-side layout (docs/WORKER_VERIFICATION_CHECKLIST.md rule 4 is not
// satisfied) and every meaning is provisional.
// ---------------------------------------------------------------------------

inline constexpr std::size_t kHitRecordOffWeaponDesc = 0x04;
inline constexpr std::size_t kHitRecordOffArmourSelector = 0x0C;
inline constexpr std::size_t kHitRecordOffHullDamageBase = 0x14;
inline constexpr std::size_t kHitRecordOffFalloffRange = 0x24;
inline constexpr std::size_t kHitRecordOffPartDamageBase = 0x28;
inline constexpr std::size_t kHitRecordOffIgnoreFalloff = 0x2C;
inline constexpr std::size_t kHitRecordOffHullSegment = 0x34;
inline constexpr std::size_t kHitRecordOffSourceRecord = 0x38;
inline constexpr std::size_t kHitRecordOffPartHits = 0x3C;
inline constexpr std::size_t kHitRecordOffPartHitCount = 0x40;
inline constexpr std::size_t kHitRecordOffAppliedDamage = 0x48;

inline constexpr std::size_t kHitPartEntryStride = 0x10; // EBP advances by 10h, 00877900..008779FA
inline constexpr std::size_t kHitPartEntryOffKind = 0x00;
inline constexpr std::size_t kHitPartEntryOffPartIndex = 0x04;
inline constexpr std::size_t kHitPartEntryOffDistance = 0x0C;

inline constexpr int kHitRecordNoHullSegment = -1; // OR EDI,0FFFFFFFFh at 008777DA
inline constexpr int kHitPartEntryNoPart = -1;

// One 10h-byte entry of the array at hit+3Ch.
struct HitPartEntry {
    int kind{0};          // +0h; kUnitHitPartEntryAlternateArmour selects the other armour
    int part_index{kHitPartEntryNoPart}; // +4h; -1 skips the entry
    float distance{0.0f}; // +0Ch, the falloff numerator
};

// The fields of the impact record the hull and part passes read.
struct HitRecord {
    float hull_damage_base{0.0f};   // +14h
    float part_damage_base{0.0f};   // +28h
    float falloff_range{0.0f};      // +24h, the divisor; zero divides as the native does
    bool ignore_falloff{false};     // +2Ch
    float armour_selector{0.0f};    // +0Ch, negative picks the alternate armour
    int hull_segment{kHitRecordNoHullSegment}; // +34h
    float weapon_scale{1.0f};       // [hit+4h]->vtable[58h](), or 1.0f when +4h is null
    float owner_modifier{1.0f};     // ProductForUnit(1, owner), or 1.0f
    const HitPartEntry* part_hits{nullptr}; // +3Ch
    int part_hit_count{0};                  // +40h
};

// ---------------------------------------------------------------------------
// The two damage formulas.
// ---------------------------------------------------------------------------

// 00470510. Not floored at zero: armour above the weapon's base makes this
// negative, and 008777D0 then skips the AddDamage call.
float hull_damage_00470510(const HitRecord& hit, float armour_scaled) noexcept;

// 004705C0. The falloff is 1 - distance/range, replaced by 1.0f when the
// record's ignore flag is set and the falloff is still positive; the result is
// floored at zero before the weapon scale.
float part_damage_004705c0(const HitRecord& hit, float armour_scaled, int part_hit_index) noexcept;

// 00879810. Negates the delta and hands it to 00879070, so a positive delta is
// a repair. The float the native pushes is exactly -0.0f - delta.
float health_delta_to_damage_00879810(float delta) noexcept;

// ---------------------------------------------------------------------------
// 0087BCC0's health block: the maximum health and the armour a unit starts with
// come straight from the vehicle-class descriptor.
// ---------------------------------------------------------------------------

struct UnitInitialCondition {
    float max_health{0.0f}; // +36Ch and +370h, both from desc+48h
    float health{0.0f};
    float armour{0.0f};     // +368h, from desc+4Ch
    float one_marker{1.0f}; // +164h
    bool condition_flag{true}; // +378h
    std::size_t part_count{0}; // (desc+20h - desc+1Ch) / 30h
};

// 0087BCF4..0087BD19 plus the part-count divide at 0087BD20..0087BD43.
UnitInitialCondition unit_initial_condition_0087bcc0(float class_hit_points,
                                                     float class_armour,
                                                     std::size_t part_vector_begin,
                                                     std::size_t part_vector_end) noexcept;

// 0087BE0D..0087BE8E: the detail value handed to vtable[190h].
float unit_part_detail_0087bcc0(bool is_kind_1b) noexcept;

// ---------------------------------------------------------------------------
// The repair tick.
// ---------------------------------------------------------------------------

// Game-settings scalars the repair steps multiply by. The singleton is 00424C40
// and the four offsets are unattributed; they are named by their offset.
struct RepairSettings {
    float hull_scale{1.0f};      // +3B4h, read at 0093C7FB
    float subobject_scale{1.0f}; // +3B8h, read at 0093C938
    float hull_kind_rate{1.0f};  // +3D4h, read at 0093C78C when task+24h == 0
    float subobject_kind_rate{1.0f}; // +3D8h, read at 0093C88C when task+24h == 2
};

// The fields of the damage-control task the two steps read.
struct RepairTask {
    bool hull_enabled{false}; // +45h; when clear the hull rate is 0.0f
    int kind{0};              // +24h
    float hull_rate{0.0f};    // +28h, a fraction of maximum health per second
    float fire_amount{0.0f};  // +34h
    float leak_amount{0.0f};  // +38h
    bool reported_repaired{false}; // +44h
};

inline constexpr int kRepairTaskKindHullSettingsRate = 0; // task+24h == 0
inline constexpr int kRepairTaskKindSubobjectSettingsRate = 2; // task+24h == 2
inline constexpr int kRepairTaskKindLoggedA = 3; // 0093CA7x
inline constexpr int kRepairTaskKindLoggedB = 4;

// 0093C776..0093C7EA. The gameplay modifier is the caller's ProductForUnit(3)
// result, or 1.0f when the modifier system is off.
float hull_repair_rate_0093c770(const RepairTask& task, const RepairSettings& settings,
                                float gameplay_modifier) noexcept;

// 0093C7EA..0093C820: rate * dt * settings.hull_scale * max_health.
float hull_repair_amount_0093c770(const RepairTask& task, const RepairSettings& settings,
                                  float gameplay_modifier, float dt, float max_health) noexcept;

// 0093C929..0093C957 for one child: the same shape with the subobject scalars.
float subobject_repair_amount_0093c860(const RepairTask& task, const RepairSettings& settings,
                                       float gameplay_modifier, float dt,
                                       float child_max_health) noexcept;

// 0093C825..0093C84C: the hull step writes the maximum back only when health is
// strictly above it (FCOMIP / JBE at 0093C83C).
bool repair_overshoot_clamp(float health, float max_health) noexcept;

// 0093C95C..0093C99A: the subobject step tests max <= health instead, so a child
// that lands exactly on its maximum is still written. The asymmetry is in the
// native code, not a simplification here.
bool subobject_repair_complete(float health, float max_health) noexcept;

// 0093CA6C..0093CAD1: the completion test that gates the debug log.
bool repair_completion_logged_0093ca20(const RepairTask& task) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary. One virtual method per native call site the sequences
// below reach. Same pattern as UnitDamageHost in include/bsp/unit_damage.hpp.
// ---------------------------------------------------------------------------

struct UnitHitPathHost {
    virtual ~UnitHitPathHost() = default;

    // vtable[5Ch], the instance IsKindOf (docs/UNIT_TIMED_SUBUPDATES.md).
    // Sites 008777EF, 00877828, 0093C900, 0093C913, 0087BE11.
    virtual bool unit_is_kind_of(std::uint32_t unit, int class_id) = 0;

    // [[unit+354h] vtable+24h] at 00877809 and 0087791F.
    virtual float unit_alternate_armour(std::uint32_t unit) = 0;

    // The plain armour field unit+368h, read at 00877811.
    virtual float unit_armour(std::uint32_t unit) = 0;

    // 008E6430 BSP_GameplayModifiers_ProductForUnit at 0087784F, 00877875,
    // 00877965, 008779A3 (kind 2) and 0093C7C5, 0093C8C5 (kind 3). Returns
    // 1.0f when the modifier system is off; the caller decides the gate.
    virtual float gameplay_modifier_product(int kind, std::uint32_t unit) = 0;

    // The gate 00E0C978 != 0 && [00F88C30+A0h] != 0 (hit path) or
    // [00F88C30+ACh] != 0 (repair path).
    virtual bool gameplay_modifiers_enabled(int kind) = 0;

    // unit+3F0h, read at 00877835 for the owner-scoped modifier.
    virtual std::uint32_t unit_owner_scoped_unit(std::uint32_t unit) = 0;

    // vtable[1ACh] AddDamage, base 0095DA00. Sites 008778D2 and 00877A37.
    virtual void unit_add_damage(std::uint32_t unit, float damage) = 0;

    // hit+48h, written at 008778BF and 00877A24 before each dispatch.
    virtual void write_applied_damage(std::uint32_t hit_record, float damage) = 0;

    // 00879070 BSP_UnitInstance_ApplyDamage at 00879824.
    virtual void unit_apply_damage(std::uint32_t unit, float amount) = 0;

    // 00877B90 BSP_UnitInstance_SetHealth at 0093C844, 0093C99A and 0087834A.
    virtual void unit_set_health(std::uint32_t unit, float health) = 0;

    // 00424C40 BSP_GameSettings_GetSingleton at 0093C787, 0093C7F6, 0093C887 and
    // 0093C933, folded into one read of the four scalars.
    virtual RepairSettings game_settings() = 0;

    // unit+36Ch and unit+370h.
    virtual UnitHealth read_health(std::uint32_t unit) = 0;

    // The subobject walk of 0093C860: unit+48h then +44h, keeping children with
    // IsKindOf(4) true, IsKindOf(0Fh) false and the byte +378h set.
    virtual int repairable_child_count(std::uint32_t unit) = 0;
    virtual std::uint32_t repairable_child(std::uint32_t unit, int index) = 0;
    virtual bool unit_condition_flag(std::uint32_t unit) = 0;

    // 00934150 BSP_UnitParts_DetachPart at 0080E467, through [unit+1018h].
    // The body is a contract; only its ABI is established.
    virtual void detach_part(std::uint32_t unit, int part_index, const float impulse[3]) = 0;

    // 0080E422..0080E438: the condition reset 0080E410 performs after the heal.
    virtual void write_condition_reset(std::uint32_t unit) = 0;
};

// ---------------------------------------------------------------------------
// The sequences.
// ---------------------------------------------------------------------------

// What one call to 008777D0 did, for a caller that wants to assert on it.
struct HitApplication {
    bool hull_applied{false};
    float hull_damage{0.0f};
    bool part_applied{false};
    float part_damage{0.0f};
    int worst_part_hit{-1}; // the index into hit+3Ch, not the part id
};

// 008777D0 BSP_UnitInstance_ApplyHitRecord, complete.
HitApplication apply_hit_record_008777d0(UnitHitPathHost& host, std::uint32_t unit,
                                         std::uint32_t hit_record, const HitRecord& hit);

// 00879810 BSP_UnitInstance_ApplyHealthDelta, complete.
void apply_health_delta_00879810(UnitHitPathHost& host, std::uint32_t unit, float delta);

// 0093C770 BSP_RepairTask_RepairHull, complete.
void repair_hull_0093c770(UnitHitPathHost& host, std::uint32_t unit, const RepairTask& task,
                          float gameplay_modifier, float dt);

// 0093C860 BSP_RepairTask_RepairSubObjects. Partial: the "destroyed" dispatch at
// 0093C9AE..0093C9F5 is not modelled.
void repair_subobjects_0093c860(UnitHitPathHost& host, std::uint32_t unit, const RepairTask& task,
                                float gameplay_modifier, float dt);

// 00878340 BSP_UnitInstance_RestoreFullHealth, complete.
void restore_full_health_00878340(UnitHitPathHost& host, std::uint32_t unit);

// 0080E410 BSP_UnitInstance_ResetCondition, complete.
void reset_condition_0080e410(UnitHitPathHost& host, std::uint32_t unit);

// 0080E440 BSP_UnitInstance_DetachPart, complete: a zero impulse into 00934150.
void detach_part_0080e440(UnitHitPathHost& host, std::uint32_t unit, int part_index);

} // namespace bsp
