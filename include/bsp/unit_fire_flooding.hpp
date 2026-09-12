#pragma once
// Ship fire, flooding and damage control. docs/UNIT_FIRE_AND_REPAIR.md,
// reports/unit_fire_flooding.json. Packet cc2_fire_flooding, Ghidra read-only.
//
// Every descriptive name here is a hypothesis, not a recovered symbol. The rules
// below are projections of the native routines named in each comment; they are
// not binary-compatible replacements and no native layout is reproduced. The
// health fields come from include/bsp/unit_damage.hpp and are not redeclared.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Where the state lives. The repair task is embedded in the unit instance at
// unit+A20h: 008160DC does ADD ECX,0xA20 before the tick call at 008160E5,
// 008AD699 does LEA ECX,[ESI+0xA20] before the setter 00939FE0, and 008ADA11
// writes [ESI+0xA48h], the slot the hull step reads as task+28h.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kUnitRepairTaskOffset = 0xA20;

// Offsets inside the task. Add kUnitRepairTaskOffset for the unit-relative one.
inline constexpr std::size_t kRepairTaskOffOwner = 0x00;            // 0093C770, 0093CA20
inline constexpr std::size_t kRepairTaskOffFailuresBegin = 0x18;    // 0093C520
inline constexpr std::size_t kRepairTaskOffFailuresEnd = 0x1C;      // 0093C520
inline constexpr std::size_t kRepairTaskOffFailuresCapacity = 0x20; // 0093B690
inline constexpr std::size_t kRepairTaskOffPriority = 0x24;         // all five steps
inline constexpr std::size_t kRepairTaskOffEffectivity = 0x28;      // 008ADA11 writes
inline constexpr std::size_t kRepairTaskOffFireDamageRate = 0x2C;   // 0093C120 reads
inline constexpr std::size_t kRepairTaskOffWaterDamageRate = 0x30;  // 0093C210 reads
inline constexpr std::size_t kRepairTaskOffFireSeconds = 0x34;      // 0093C120 writes
inline constexpr std::size_t kRepairTaskOffWaterSeconds = 0x38;     // 0093C210 writes
inline constexpr std::size_t kRepairTaskOffWaterExpiredSlot = 0x3C; // 0093C210 clears
inline constexpr std::size_t kRepairTaskOffFireExpiredSlot = 0x40;  // 0093C120 clears
inline constexpr std::size_t kRepairTaskOffReported = 0x44;         // 0093CADC sets
inline constexpr std::size_t kRepairTaskOffHullRepairEnabled = 0x45;// 0093C770 reads
inline constexpr std::size_t kRepairTaskOffFailureRepairEnabled = 0x46; // 00939FE0

// Unit fields outside the task that damage control uses.
inline constexpr std::size_t kUnitRepairEffectivityOffset = 0xA48;  // = A20h + 28h
inline constexpr std::size_t kUnitRepairTimeDelayOffset = 0x114C;   // 008ADBB6..008ADBDD
inline constexpr std::size_t kUnitRepairLevelArrayOffset = 0x1134;  // 0080E130, 00827960
inline constexpr std::size_t kUnitLeakManagerOffset = 0x10D4;       // 00891788, 008919D8
inline constexpr std::size_t kUnitPartDescriptorVectorOffset = 0x344; // 008AD025

// The leak manager, read only as far as GetLeaks and GetWaterLoad need it.
inline constexpr std::size_t kLeakManagerOffCount = 0x14;      // 0074E8F0, 0074E9C0
inline constexpr std::size_t kLeakManagerOffLeakArray = 0x18;  // 0074E9C0
inline constexpr std::size_t kLeakManagerOffWaterArray = 0x1C; // 0074E8F0

// The parts object's displacement divisor, 0092BEB0 over [unit+1018h].
inline constexpr std::size_t kUnitPartsDisplacementOffset = 0x84;

// ---------------------------------------------------------------------------
// Game-settings offsets, all through 00424C40(). The default values are
// contract: unread; only the offsets and their roles are established.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kSettingsOffHullRepairScale = 0x3B4;      // 0093C770
inline constexpr std::size_t kSettingsOffSubObjectRepairScale = 0x3B8; // 0093C860
inline constexpr std::size_t kSettingsOffFirePriorityDivisor = 0x3C8;  // 0093C120
inline constexpr std::size_t kSettingsOffWaterPriorityDivisor = 0x3CC; // 0093C210
inline constexpr std::size_t kSettingsOffFailurePriorityRate = 0x3D0;  // 0093C520
inline constexpr std::size_t kSettingsOffHullPriorityRate = 0x3D4;     // 0093C770
inline constexpr std::size_t kSettingsOffSubObjectPriorityRate = 0x3D8;// 0093C860
inline constexpr std::size_t kSettingsOffFailureChanceNumerator = 0x3DC;   // 0093BF5F
inline constexpr std::size_t kSettingsOffFailureChanceDenominator = 0x3E0; // 0093BF66
inline constexpr std::size_t kSettingsOffFailureTableBegin = 0x3E8;    // 0093C320
inline constexpr std::size_t kSettingsOffFailureTableEnd = 0x3EC;      // 0093C320
inline constexpr std::size_t kSettingsOffMaxRepairLevel = 0x46C;       // 00827960

// Literal constants the rules use, with the address each was read from.
inline constexpr float kRepairUnitScale = 1.0f;            // 00D7A24C
inline constexpr float kRepairZero = 0.0f;                 // 00D7A218
inline constexpr float kRepairTimeDelayCeiling = 1.0e10f;  // 00CE4970
inline constexpr double kLeakZoneCount = 6.0;              // 00CE6628
inline constexpr double kLeakLoadScale = 10000.0;          // 00CE4BD8
inline constexpr double kLeakLoadFactor = 1.5;             // 00CE3D78
inline constexpr int kLeakZoneBins = 6;
inline constexpr std::size_t kFailureRecordStride = 0x10;  // 0093C520 `>> 4`

// ---------------------------------------------------------------------------
// The repair priority at task+24h, 0..4. Each value boosts exactly one step.
// The names are the effect each value has, not recovered identifiers; the
// message A0h handler that would name them is contract: unread.
// ---------------------------------------------------------------------------
enum class RepairPriority : int {
    kHull = 0,       // 0093C770 takes settings[3D4h]
    kFailures = 1,   // 0093C520 takes settings[3D0h]
    kSubObjects = 2, // 0093C860 takes settings[3D8h]
    kFlooding = 3,   // 0093C210 divides water damage by settings[3CCh]
    kFire = 4,       // 0093C120 divides fire damage by settings[3C8h]
};

// The crew-level categories 0081AD40 maps names to. Index 0 is the unallocated
// pool and is also the answer for any unrecognised name.
enum class RepairCategory : int {
    kPool = 0,             // "all", and the default
    kHull = 1,             // "hull", 00D09600
    kFireFighting = 2,     // "firefighting"
    kEngineAndSteering = 3,// "engine and steering", "engine", "steering"
    kWeapons = 4,          // "weapons", 00D095D4
    kSpecial = 5,          // "spec" 00D095CC, "runway", "airsupply"
};
inline constexpr int kRepairCategoryCount = 6;

// ---------------------------------------------------------------------------
// The records. Both are contract: producer unread for the task itself; the
// failure record's producer 0093BC30 is called from all three sites, so its
// layout is settled by its writers (0093BED0, 0093C300, 0093BD80).
// ---------------------------------------------------------------------------
struct ActiveFailure {
    std::int32_t id = 0;         // +00h: 5 for EngineJam, -2 for a random pick
    std::string name;            // +04h length, +08h characters
    float seconds_remaining = 0; // +0Ch
};

struct RepairTaskState {
    std::vector<ActiveFailure> failures;   // +18h..+20h
    RepairPriority priority = RepairPriority::kHull; // +24h
    float effectivity = 0.0f;              // +28h, the hull coefficient
    float fire_damage_rate = 0.0f;         // +2Ch, per second
    float water_damage_rate = 0.0f;        // +30h, per second
    float fire_seconds = 0.0f;             // +34h
    float water_seconds = 0.0f;            // +38h
    std::uint32_t water_expired_slot = 0;  // +3Ch
    std::uint32_t fire_expired_slot = 0;   // +40h
    bool reported_repaired = false;        // +44h
    bool hull_repair_enabled = false;      // +45h
    bool failure_repair_enabled = false;   // +46h
};

// The five rate constants the steps read out of the settings singleton.
struct RepairSettings {
    float hull_repair_scale = 0.0f;       // +3B4h
    float subobject_repair_scale = 0.0f;  // +3B8h
    float fire_priority_divisor = 1.0f;   // +3C8h
    float water_priority_divisor = 1.0f;  // +3CCh
    float failure_priority_rate = 1.0f;   // +3D0h
    float hull_priority_rate = 1.0f;      // +3D4h
    float subobject_priority_rate = 1.0f; // +3D8h
    float failure_chance_numerator = -1.0f;   // +3DCh
    float failure_chance_denominator = 1.0f;  // +3E0h
    float max_repair_level = 0.0f;        // +46Ch
};

// ---------------------------------------------------------------------------
// Pure rules. Every one is a projection of the named routine's arithmetic; the
// inputs are explicit so a headless mission can drive them.
// ---------------------------------------------------------------------------

// The shared three-line preamble of all five steps: the gameplay modifier is
// applied only when 00F88C30 is non-null, 00E0C978 is set and [00F88C30+ACh] is
// non-zero. 0093C553/0093C592 and the same pair in each other step.
float repair_modifier_factor(bool modifier_manager_present,
                             bool modifier_manager_enabled,
                             float modifier_product);

// 0093C770. Returns the health to add this tick; the caller clamps to maximum.
float hull_repair_amount_0093c770(const RepairTaskState& task,
                                  const RepairSettings& settings,
                                  float unit_max_health,
                                  float modifier,
                                  float dt);

// 0093C860, one child. The child list walk and its three kind tests stay with
// the host; this is only the per-child arithmetic.
float subobject_repair_amount_0093c860(const RepairTaskState& task,
                                       const RepairSettings& settings,
                                       float child_max_health,
                                       float modifier,
                                       float dt);

// 0093C520. The per-tick decrement applied to every active failure's remaining
// seconds; zero when failure repair is disabled.
float failure_repair_step_0093c520(const RepairTaskState& task,
                                   const RepairSettings& settings,
                                   float modifier,
                                   float dt);

// 0093C120 and 0093C210. Advances one timer and reports the damage to apply.
struct TimerDamageResult {
    float seconds_remaining = 0.0f;
    float damage = 0.0f;
    bool expired_this_tick = false;
};
TimerDamageResult fire_damage_step_0093c120(const RepairTaskState& task,
                                            const RepairSettings& settings,
                                            float modifier,
                                            float dt);
TimerDamageResult water_damage_step_0093c210(const RepairTaskState& task,
                                             const RepairSettings& settings,
                                             float modifier,
                                             float dt);

// 0093CA20's completion test: priority 3 or 4, both timers exactly zero and the
// report byte clear.
bool repair_completion_ready_0093ca20(const RepairTaskState& task);

// 0093BED0's probability, before the draw. A negative numerator or denominator
// on the component descriptor falls back to the settings pair.
float failure_chance_0093bed0(float component_numerator,
                              float component_denominator,
                              const RepairSettings& settings,
                              float damage);

// 008ADA70. unit+114Ch = clamp(unit+114Ch + delay, 0, 1e10).
float repair_time_delay_0093ada70(float current, float delay);

// 0074E8F0 and 0074E9C0: bin `values` into six zones by an index that advances
// 6/n per element. Returns the six bins in bin order b0..b5.
void leak_zone_sums_0074e8f0(const float* values, int count, float out_bins[kLeakZoneBins]);

// 00891680 and 008918D0 normalise each sum: min(1, sum * 1.5 * 10000 /
// displacement). The binding pushes the total first, then the six zones in
// stack-slot order, which is the permutation below.
float leak_load_fraction_00891680(float sum, float displacement);

// The six caller slots the helper's out parameters land in, in ascending
// address order; derived from the six pushes at 0089176E..00891787 and the
// callee's own cleanup (no ADD ESP before the FLD at 00891793).
inline constexpr int kLeakZoneResultOrder[kLeakZoneBins] = {2, 4, 0, 3, 1, 5};

// 00827960, 00812A70 and 00827A40 over the int[6] at unit+1134h. Index 0 is the
// pool; raising a category pays for it out of the pool, or out of the highest
// other category when the pool is empty.
void repair_level_raise_00827960(int levels[kRepairCategoryCount], int index, float max_level);
bool repair_level_lower_00812a70(int levels[kRepairCategoryCount], int index);
void repair_level_set_00827a40(int levels[kRepairCategoryCount], int index, int target,
                               float max_level);
int repair_category_index_0081ad40(const std::string& name);

// ---------------------------------------------------------------------------
// The host. One virtual method per native call site the sequences below reach;
// the pattern is include/bsp/app_frame.hpp. Every method names its site and
// callee, and the report carries the argument detail.
// ---------------------------------------------------------------------------
struct UnitFireFloodingHost {
    virtual ~UnitFireFloodingHost() = default;

    // The shared preamble. 00424C40 at 0093C553/0093C12C/0093C21C.
    virtual const RepairSettings& game_settings() = 0;
    // 008E6430 at 0093C592/0093C16A/0093C25A, arguments (3, unit).
    virtual bool modifier_manager_present() = 0;   // 00F88C30 non-null
    virtual bool modifier_manager_enabled() = 0;   // 00E0C978 and [00F88C30+ACh]
    virtual float gameplay_modifier_product(int kind, std::uint32_t unit) = 0;

    // The unit's own health, from include/bsp/unit_damage.hpp's fields.
    virtual float unit_max_health(std::uint32_t unit) = 0;   // +36Ch
    virtual float unit_health(std::uint32_t unit) = 0;       // +370h
    // 00879810 at 0093C7EE, a positive argument heals.
    virtual void apply_health_delta(std::uint32_t unit, float delta) = 0;
    // 00877B90, the clamp back to maximum.
    virtual void set_health(std::uint32_t unit, float health) = 0;
    // vtable[1ACh] kEntityVtableSlotAddDamage at 0093C1FA and 0093C2EA.
    virtual void unit_add_damage(std::uint32_t unit, float amount) = 0;

    // 0093C520's expiry path.
    virtual void unit_named_state_dispatch(std::uint32_t unit,
                                           const std::string& name) = 0; // vtable[19Ch], 0093C62E
    virtual void session_route_failure_cleared(std::uint32_t unit,
                                               const std::string& name) = 0; // 0075B430/0077C2A0

    // 0093CA20's completion.
    virtual std::string unit_display_name(std::uint32_t unit) = 0; // vtable[14h], 0093CAC9
    virtual float unit_get_health_0923be0(std::uint32_t unit) = 0; // 00923BE0 at 0093CAAF
    virtual void log_line(const char* format, const std::string& name,
                          double health, double reported) = 0;     // 004254B0 at 0093CAD1
    virtual void repair_completed_notify(std::uint32_t unit) = 0;  // JMP 00914100 at 0093CAF1

    // 0093BED0's roll.
    virtual bool failure_rolls_enabled() = 0; // game+1FE4h in {0,1}
    // 008782A0 at 0093BF1B; returns false when no component resolves.
    virtual bool resolve_component(std::uint32_t owner, int segment,
                                   float* numerator, float* denominator) = 0;
    virtual float random_unit_float() = 0;    // 00BD2F10(0, 1.0f) at 0093BF98
    // 0093AA00 at 0093BFC1; false when the owner has no failure descriptor.
    virtual bool failure_descriptor_for(std::uint32_t owner, std::string* name,
                                        float* duration) = 0;
    virtual bool failure_already_active(const std::string& name) = 0; // 0093A5D0 at 0093C035
    virtual void session_route_failure_started(std::uint32_t owner, int segment) = 0; // 0093C08E
    virtual void warning_fire_failure(std::uint32_t unit, const std::string& name,
                                      float duration) = 0;          // 00982C50 at 0093C0C2
    virtual void failure_side_effect(std::uint32_t unit, std::uint32_t hit,
                                     std::uint32_t owner) = 0;      // 00913D80 at 0093C0DC
    virtual void unit_on_failure(std::uint32_t unit, const std::string& name,
                                 std::uint32_t hit) = 0;            // vtable[21Ch] at 0093C0ED

    // 0093C300's random pick.
    virtual std::uint32_t random_next_u32() = 0;                    // 00BD2FC0 at 0093C351
    // The settings failure table at +3E8h; false when the index is disabled.
    virtual bool failure_table_row(int index, std::string* name, float* duration,
                                   bool* enabled) = 0;
    virtual int failure_table_size() = 0;

    // The bindings.
    virtual std::uint32_t unit_parts_object(std::uint32_t unit) = 0;     // 0080E490
    virtual float parts_displacement(std::uint32_t parts) = 0;           // 0092BEB0
    virtual int leak_manager_count(std::uint32_t unit) = 0;              // [unit+10D4h]+14h
    virtual const float* leak_manager_leaks(std::uint32_t unit) = 0;     // +18h
    virtual const float* leak_manager_water(std::uint32_t unit) = 0;     // +1Ch
    virtual void lua_push_number(float value) = 0;                       // 00B66480
    virtual bool unit_is_kind_of(std::uint32_t unit, int class_id) = 0;  // vtable[5Ch]
    virtual void unit_max_repair(std::uint32_t unit) = 0;                // vtable[1B8h]
    virtual int group_member_count(std::uint32_t unit) = 0;              // unit+3CCh
    virtual std::uint32_t group_member(std::uint32_t unit, int index) = 0; // unit+3D0h[i]
    virtual int roster_size(std::uint32_t unit) = 0;                     // unit+398h..+39Ch
    virtual std::uint32_t roster_member(std::uint32_t unit, int index) = 0;
};

// ---------------------------------------------------------------------------
// The sequences.
// ---------------------------------------------------------------------------

// 0093CA20, the whole tick. Runs the five steps in order and then the
// completion report. Steps 1 and 2 are inherited from docs/UNIT_HIT_PATH.md.
void repair_task_update_0093ca20(UnitFireFloodingHost& host, std::uint32_t unit,
                                 RepairTaskState& task, float dt);

// 0093C520. Decrements every failure and swap-erases the expired ones; the
// swapped-in entry is retried on the same pass, as the native loop does.
void repair_failures_0093c520(UnitFireFloodingHost& host, std::uint32_t unit,
                              RepairTaskState& task, float dt);

// 0093BED0, called from 00827450 when a hit applied damage above zero.
void roll_component_failure_0093bed0(UnitFireFloodingHost& host, std::uint32_t unit,
                                     RepairTaskState& task, std::uint32_t hit,
                                     std::uint32_t owner, int segment, float damage);

// 0093C300, the random pick from the settings table.
void pick_random_failure_0093c300(UnitFireFloodingHost& host, std::uint32_t unit,
                                  RepairTaskState& task);

// 00891680 and 008918D0. Pushes the total then the six zones in result order.
void lua_get_water_load_00891680(UnitFireFloodingHost& host, std::uint32_t unit);
void lua_get_leaks_008918d0(UnitFireFloodingHost& host, std::uint32_t unit);

// 008C6DD0, the cheat's class fan-out.
void lua_cheat_max_repair_008c6dd0(UnitFireFloodingHost& host, std::uint32_t unit);

// The class ids 008C6DD0 tests through vtable[5Ch].
inline constexpr int kCheatMaxRepairShipClassId = 0x04;
inline constexpr int kCheatMaxRepairGroupClassId = 0x18;
inline constexpr int kCheatMaxRepairRosterClassId = 0x1A;
inline constexpr int kCheatMaxRepairGroupMemberLimit = 4; // `CMP EDI,0x4 / JA` at 008C6F68

}  // namespace bsp
