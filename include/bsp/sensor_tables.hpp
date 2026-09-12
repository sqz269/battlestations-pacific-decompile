#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// The sensor tables behind unit detection: the record `[[unit+538h]+B4h]` that
// `008048A0 BSP_Recon_EvaluateSensors` walks, the resolver `00808F90` that hands
// one out per `ReconClass` id, the loader `008082A0` that fills it from the
// shipped `ReconClass` Lua table, the seven observer/subject categories that
// index it, and the two mission-scripted range multipliers on the weather
// config at `[game+21C4h]`.
//
// docs/SENSOR_TABLES.md carries the evidence. Every name below is a hypothesis,
// not a recovered symbol.
//
// bsp/recon_slot_lists.hpp already declares the CONSUMER side: `ReconSensorEntry`
// (the 1Ch row as 008048A0 reads it), kReconSensorBlockOffset,
// kReconSensorTablePointerOffset, kReconSensorSignatureOffset,
// kReconSensorTableArrayOffset, kReconSensorTableStride,
// kReconSensorCategoriesPerObserver, kReconSensorEntryStride,
// kReconSensorMaskAll and the range-scale rules. None of those is redefined
// here; this header adds the producer, the record, the categories and the
// tuning scalars. `SensorTableEntry` below is the same 1Ch row named from the
// writer, and it carries the `+0h` field the reader never touches.
//
// bsp/weather_config.hpp declares the 7Ch object at `[game+21C4h]`. Its `+74h`
// and `+78h` members are named for the amplitude maps; the Lua bindings prove
// otherwise (see kSimplifiedReconMultiplierLuaName below and the corrections
// section of docs/SENSOR_TABLES.md). That header is not owned by this packet
// and is left unchanged.

namespace bsp {

// ---------------------------------------------------------------------------
// The seven categories
// ---------------------------------------------------------------------------

// The value `unit->[+1E4h]->vtable[1]()` returns. 008048A0 uses it twice: the
// observer's picks the row (x8) and the target's picks the column.
// The names are the loader's Lua key names at 008083CD..00808474 (Observer...)
// and 00808531..00808734 (Subject...); `unclassified` is the value 6 that
// 004F1740 returns and that no Lua key can address.
enum class SensorCategory : int {
    air = 0,             // 0074E190, "ObserverAir" / "SubjectAir"
    surface = 1,         // 006DFD20 and five siblings, "...Surface"
    periscope_in = 2,    // 00852B90, "...PeriscopeIn"
    periscope_out = 3,   // 00852B90, "...PeriscopeOut"
    underwater = 4,      // 0085EA40 and 00852B90, "...Underwater"
    deep_underwater = 5, // 00852B90, "...DeepUnderwater"
    unclassified = 6,    // 004F1740; no Lua key reaches this row or column
};

// 00808CB0 constructs 7 groups; 008085B2 multiplies the observer index by 8.
inline constexpr std::size_t kSensorCategoryCount = 7;
inline constexpr std::size_t kSensorCategoryStride = 8;

// The 56 lists the record actually holds. Columns 7 of each row are allocated
// and never addressed: no category value reaches 7.
inline constexpr std::size_t kSensorListCount =
    kSensorCategoryCount * kSensorCategoryStride;

constexpr bool sensor_category_is_valid(int value) noexcept {
    return value >= 0 && value < static_cast<int>(kSensorCategoryCount);
}

// 008085AA..008085BC: LEA EAX,[EDX+ECX*8]; LEA EAX,[EAX+EAX*2]; +8, *4.
constexpr std::size_t sensor_list_index_008085aa(SensorCategory observer,
                                                 SensorCategory subject) noexcept {
    return static_cast<std::size_t>(static_cast<int>(observer)) * kSensorCategoryStride +
           static_cast<std::size_t>(static_cast<int>(subject));
}

// ---------------------------------------------------------------------------
// The record: 2A8h, refcounted, 7 groups of 8 lists
// ---------------------------------------------------------------------------

inline constexpr std::size_t kSensorClassRecordSize = 0x2a8;        // 008090F1
inline constexpr std::size_t kSensorClassRecordListsOffset = 0x08;  // 00808CE8
inline constexpr std::size_t kSensorClassRecordGroupSize = 0x60;    // 00808CE6
inline constexpr std::size_t kSensorClassRecordListStride = 0x0c;   // 00808200
inline constexpr std::size_t kSensorClassRecordEntryStride = 0x1c;  // 00808A58
inline constexpr std::uint32_t kSensorClassRecordVtable = 0x00d08fec; // 00808CF4

// The 0Ch list header the loader grows and 008048A0 walks.
inline constexpr std::size_t kSensorListBeginOffset = 0x00;
inline constexpr std::size_t kSensorListCountOffset = 0x04;
inline constexpr std::size_t kSensorListCapacityOffset = 0x08;

// 00808A37..00808A4E: the growth is max(2 * capacity, 1).
constexpr int sensor_list_grow_capacity_00808a37(int capacity) noexcept {
    const int doubled = capacity * 2;
    return doubled < 2 ? 1 : doubled;
}

// ---------------------------------------------------------------------------
// The 1Ch entry, named from the writer at 008085AA..00808A6A
// ---------------------------------------------------------------------------

// The `RawType` the scripts install; 008048A0 tests `mask & (1 << raw_type)`.
// Names and values from scripts/datatables/classtables/*/reconclasses.lua.
enum class SensorRawType : int { vision = 0, radar = 1, sonar = 2 };

struct SensorTableEntry {
    float dist = 0.0f;            // +0h,  Lua "Dist"; written, never read back
    float dist_sq = 0.0f;         // +4h,  dist * dist (008085F1 FMUL ST0)
    float gain_per_second = 0.0f; // +8h,  Lua "Gain" * 0.5
    float max_value = 0.0f;       // +0Ch, from Lua "MaxLevel"
    int raw_type = 0;             // +10h, Lua "RawType", the mask bit index
    float half_angle = 0.0f;      // +14h, Lua "Angle", radians
    bool bearing_limited = false; // +18h, true when "Angle" is not nil
};

// 00808626 FMUL by the double at 00D7A280. The scripts write Gain as a rate in
// detection-value per second; the record halves it.
inline constexpr double kSensorGainScale = 0.5;

// 0080865A pushes the default. A row without "MaxLevel" behaves as 2.
inline constexpr int kSensorMaxLevelDefault = 2;

// 0080866B..00808685 and 0080896B/00808975. The cap is the highest detection
// value this entry may drive the target's accumulator to, which is how it caps
// the published level: 0.25f is exactly the blip threshold and 1.0f the top.
constexpr float sensor_max_value_for_level_0080866b(int max_level) noexcept {
    if (max_level == 0) {
        return 0.0f;
    }
    return max_level == 1 ? 0.25f : 1.0f;
}

// ---------------------------------------------------------------------------
// The "GUIRange" sibling table, 20h, built by the same loop
// ---------------------------------------------------------------------------

// 00808791..00808903 build this and hand it to 00807F60. It sits per observer
// category, not per subject, and 008048A0 never reads it.
struct SensorGuiRangeRing {
    int raw_type = 0;                        // +0h,  Lua "RawType"
    bool has_angle = false;                  // +4h,  true when "Angle" is set
    float range = 0.0f;                      // +8h,  Lua "Range", unsquared
    float half_angle = 0.0f;                 // +0Ch, Lua "Angle", radians
    std::array<std::uint32_t, 4> color{};    // +10h, Lua "Color", four words
};

// ---------------------------------------------------------------------------
// The Lua key names, in the loader's comparison order
// ---------------------------------------------------------------------------

inline constexpr const char* kSensorDetailsKey = "Details";      // 00D08FE4
inline constexpr const char* kSensorGuiRangeKey = "GUIRange";    // 00D08ED8
inline constexpr const char* kSensorEntryDistKey = "Dist";       // 00D08F5C
inline constexpr const char* kSensorEntryGainKey = "Gain";       // 00D08F54
inline constexpr const char* kSensorEntryMaxLevelKey = "MaxLevel"; // 00D08F48
inline constexpr const char* kSensorEntryAngleKey = "Angle";     // 00D08ED0
inline constexpr const char* kSensorEntryRawTypeKey = "RawType"; // 00D08EC8
inline constexpr const char* kSensorRingRangeKey = "Range";
inline constexpr const char* kSensorRingColorKey = "Color";

// Index = the SensorCategory the key selects. Category 6 has no key.
inline constexpr std::array<const char*, 6> kSensorObserverKeys{{
    "ObserverAir",             // 00D08FD8 -> 0
    "ObserverSurface",         // 00D08FC8 -> 1
    "ObserverPeriscopeIn",     // 00D08FB4 -> 2
    "ObserverPeriscopeOut",    // 00D08F9C -> 3
    "ObserverUnderwater",      // 00D08F88 -> 4
    "ObserverDeepUnderwater",  // 00D08F70 -> 5
}};
inline constexpr std::array<const char*, 6> kSensorSubjectKeys{{
    "SubjectAir",              // 00D08F64 -> 0
    "SubjectSurface",          // 00D08F38 -> 1
    "SubjectPeriscopeIn",      // 00D08F24 -> 2
    "SubjectPeriscopeOut",     // 00D08F10 -> 3
    "SubjectUnderwater",       // 00D08EFC -> 4
    "SubjectDeepUnderwater",   // 00D08EE4 -> 5
}};

// The loader matches case-insensitively (00425850) and does NOT reset the index
// when a key matches nothing: an unrecognised `Details` key reuses the previous
// iteration's observer index, and an unrecognised key inside an observer group
// reuses the previous subject index and still appends its rows. Neither slot is
// initialised before its loop (no write to [ESP+10h] or [ESP+14h] precedes the
// comparison chains), so the first unrecognised key of a record reads an
// uninitialised stack word. This rule reproduces that, with `previous` standing
// in for the stack slot.
constexpr int sensor_category_for_key_or_previous(int matched, int previous) noexcept {
    return matched < 0 ? previous : matched;
}

// ---------------------------------------------------------------------------
// The reconstructed record
// ---------------------------------------------------------------------------

struct SensorClassTable {
    // 56 lists, addressed with sensor_list_index_008085aa. Columns 7 stay empty.
    std::array<std::vector<SensorTableEntry>, kSensorListCount> lists{};
    // One ring list per observer category (00807F60's destination).
    std::array<std::vector<SensorGuiRangeRing>, kSensorCategoryCount> gui_rings{};

    std::vector<SensorTableEntry>& list(SensorCategory observer, SensorCategory subject) {
        return lists[sensor_list_index_008085aa(observer, subject)];
    }
    const std::vector<SensorTableEntry>& list(SensorCategory observer,
                                              SensorCategory subject) const {
        return lists[sensor_list_index_008085aa(observer, subject)];
    }
};

// ---------------------------------------------------------------------------
// The loader, 008082A0
// ---------------------------------------------------------------------------

// One Lua row as the loader sees it: the raw fields before conversion. `angle`
// is absent when the key is nil (00808726 BSP_LuaObject_IsNil).
struct SensorLuaEntryRow {
    double dist = 0.0;
    double gain = 0.0;
    int max_level = kSensorMaxLevelDefault;
    int raw_type = 0;
    bool has_angle = false;
    double angle = 0.0;
};

struct SensorLuaRingRow {
    double range = 0.0;
    std::array<std::uint32_t, 4> color{};
    bool has_angle = false;
    double angle = 0.0;
    int raw_type = 0;
};

// 008085C0..00808A6A, the whole per-row conversion.
SensorTableEntry sensor_entry_from_lua_row_008085c0(const SensorLuaEntryRow&) noexcept;
SensorGuiRangeRing sensor_ring_from_lua_row_00808791(const SensorLuaRingRow&) noexcept;

// One method per native call site of 008082A0, named by that site. The host owns
// the Lua state; the sequence below models the traversal order and the
// decisions, not the storage. The scope destructors (00B67700, 00B669A0) are
// bookkeeping and are not modelled.
struct SensorTableLuaHost {
    virtual ~SensorTableLuaHost() = default;

    // 008082CD, 00B661B0: the record's Lua row must be a table.
    virtual bool row_is_table_008082cd() = 0;
    // 008082E9, 00B67800: `row.Details`; the result must itself be a table
    // (008082FE, 00B661B0).
    virtual bool open_details_008082e9() = 0;
    // 0080834A, 00B67080: iterate the keys of `Details`.
    virtual std::vector<std::string> details_keys_0080834a() = 0;
    // 008084B5, 00B67080: iterate the keys of one observer group. Empty when the
    // group value is unbound (008084C9, 00B66420).
    virtual std::vector<std::string> observer_group_keys_008084b5(
        const std::string& details_key) = 0;
    // 0080858C, 00B67080: the numbered rows of one subject list, each read
    // through 008085D1 Dist, 00808615 Gain, 00808655 MaxLevel, 008089A6 Angle
    // and 00808A08 RawType.
    virtual std::vector<SensorLuaEntryRow> subject_rows_0080858c(const std::string& key) = 0;
    // 00808786, 00B67080: the numbered rows of the GUIRange list.
    virtual std::vector<SensorLuaRingRow> gui_range_rows_00808786() = 0;
};

// 008082A0 `__thiscall void(this = record, LuaObject* row)`. Returns the filled
// record; on any early exit the record keeps the empty lists 00808CB0 built.
void load_sensor_class_table_008082a0(SensorClassTable&, SensorTableLuaHost&);

// ---------------------------------------------------------------------------
// The resolver, 00808F90
// ---------------------------------------------------------------------------

inline constexpr const char* kSensorReconClassGlobal = "ReconClass"; // 00D08FF8
inline constexpr const char* kSensorReconClassIdKey = "ID";          // 00CE59B4
inline constexpr const char* kSensorReconClassGotKey = "Got";        // 00CE452C
inline constexpr std::uint32_t kSensorClassRegistryAddress = 0x00f874e4; // 00808EA0

// One method per native call site of 00808F90, named by that site, in body order.
struct SensorClassRegistryHost {
    virtual ~SensorClassRegistryHost() = default;

    // 00808FBE, 00808EA0: the lazily created registry singleton, then 00808FD1,
    // 00808C20: grow it to hold `id`.
    virtual void reserve_registry_00808fd1(int recon_class_id) = 0;
    // 00808FE4: the slot's record, or nullptr. A hit skips every Lua step.
    virtual SensorClassTable* cached_record_00808fe4(int recon_class_id) = 0;
    // 0080902C, 00B67980: the embedded Lua owner's globals, then 00809041,
    // 00B67800: globals.ReconClass.
    virtual void open_recon_class_global_00809041() = 0;
    // 0080905E, 00B67720: ReconClass[id], the record's Lua row.
    virtual void open_class_row_0080905e(int recon_class_id) = 0;
    // 00809085, 00B67460: row.ID = id (key assigned at 00809071, 0041E870).
    virtual void set_row_id_00809085(int recon_class_id) = 0;
    // 008090C9, 00B673A0: row.Got = true (key assigned at 008090B5, 0041E870).
    virtual void set_row_got_008090c9(bool value) = 0;
    // 0080910D, 00808CB0: construct the 2A8h record allocated at 008090F6.
    virtual SensorClassTable* allocate_record_0080910d() = 0;
    // 00809147, 008082A0: fill it from the row.
    virtual void load_from_row_00809147(SensorClassTable&) = 0;
    // 0080915F, 00808C20: grow the registry again, then publish into the slot,
    // releasing whatever was there (00809164..0080919B).
    virtual void publish_record_0080915f(int recon_class_id, SensorClassTable*) = 0;
};

// 00808F90 `__fastcall SensorClassTable**(ECX = out, EDX = reconClassId)`,
// RET. A cached record short-circuits every Lua step. Returns the record the
// vehicle class stores at `class+B4h`.
SensorClassTable* resolve_sensor_class_00808f90(int recon_class_id,
                                                SensorClassRegistryHost&);

// ---------------------------------------------------------------------------
// The category accessors
// ---------------------------------------------------------------------------

// The fixed implementations, keyed by the routine that returns the value.
inline constexpr SensorCategory kSensorCategoryDefault_004f1740 = SensorCategory::unclassified;
inline constexpr SensorCategory kSensorCategoryPlane_0074e190 = SensorCategory::air;
inline constexpr SensorCategory kSensorCategoryShip_006dfd20 = SensorCategory::surface;
inline constexpr SensorCategory kSensorCategoryLandVehicle_0074dd50 = SensorCategory::surface;
inline constexpr SensorCategory kSensorCategoryAirfield_006d1d30 = SensorCategory::surface;
inline constexpr SensorCategory kSensorCategoryLandFort_006f57b0 = SensorCategory::surface;
inline constexpr SensorCategory kSensorCategoryTorpedo_0085ea40 = SensorCategory::underwater;

// 007ECFE0, the plane squadron: the category of the object at `+3D0h`, or
// `unclassified` when that pointer is null.
constexpr SensorCategory sensor_category_delegated_007ecfe0(
    bool has_owner, SensorCategory owner_category) noexcept {
    return has_owner ? owner_category : SensorCategory::unclassified;
}

// 00852B90, the submarine: a state, not a class constant. `y` is the world Y of
// the hull frame (+100h, refreshed through 00414DB0 when +C8h is clear); the
// three depth words come from the instance at +1200h, +1204h, +1208h, +120Ch and
// the periscope flag from +1234h.
struct SubmarineDepthBands {
    float band_1200 = 0.0f;
    float band_1204 = 0.0f;
    float band_1208 = 0.0f;
    float band_120c = 0.0f;
    bool periscope_raised = false; // +1234h
};

inline constexpr float kSubmarineSurfaceDivisor = 3.0f;   // 00D7A2B0, double 3.0
inline constexpr float kSubmarinePeriscopeMargin = 5.0f;  // 00D7A370, double 5.0
inline constexpr float kSubmarineDeepMidpoint = 0.5f;     // 00D7A280, double 0.5

SensorCategory sensor_category_submarine_00852b90(float world_y,
                                                  const SubmarineDepthBands&) noexcept;

// ---------------------------------------------------------------------------
// The two mission-scripted range multipliers, [game+21C4h]+74h and +78h
// ---------------------------------------------------------------------------

inline constexpr std::size_t kSimplifiedReconMultiplierOffset = 0x74; // 008B243C
inline constexpr std::size_t kSimplifiedSonarMultiplierOffset = 0x78; // 008B270C

// 004456AD/004456B2: BSP_WeatherConfig_LoadGlobals ends by storing 1.0f into
// both, unconditionally and after every Lua read. Nothing in the shipped data
// tables sets them; only the four Lua bindings below do.
inline constexpr float kSimplifiedReconMultiplierDefault = 1.0f;
inline constexpr float kSimplifiedSonarMultiplierDefault = 1.0f;

inline constexpr const char* kSimplifiedReconMultiplierLuaName =
    "SimplifiedReconMultiplier"; // Get 008B24A0 / Set 008B2320
inline constexpr const char* kSimplifiedSonarMultiplierLuaName =
    "SimplifiedSonarMultiplier"; // Get 008B2770 / Set 008B25F0

// The recon multiplier scales the sensor range linearly. 008048A0 divides the
// squared distance by `env * env * signature * m74 * m74` and compares against
// `dist_sq`, so the entry reaches `dist * m74 * env * sqrt(signature)`. The
// sonar multiplier applies the same way, and only to a submerged submarine
// target. This is the rule, in unsquared terms, for one entry.
float sensor_effective_range_00804947(const SensorTableEntry& entry,
                                      float environment_factor,
                                      float target_signature,
                                      float recon_multiplier,
                                      bool submerged_submarine,
                                      float sonar_multiplier) noexcept;

// ---------------------------------------------------------------------------
// One installed class: ReconClass 3, the sonar ship
// ---------------------------------------------------------------------------

// scripts/datatables/classtables/arcade/reconclasses.lua lines 71-108,
// `ArcadeTable[3]`, reached through scripts/datatables/autoload/reconclasses.lua
// when GameMode is not 1. 31 vehicle classes carry `["ReconClass"] = 3`,
// including VehicleClass[23] "Fletcher 1943" and VehicleClass[11]
// "Allen M. Sumner 1945". The realistic table has the identical shape with
// SonarShip_recon_SonarRange = 4000 and NormalShip_recon_VisionRange = 10000.
inline constexpr int kSonarShipReconClassId = 3;
inline constexpr float kSonarShipSonarRangeArcade = 1000.0f;
inline constexpr float kSonarShipVisionRangeArcade = 4000.0f;
inline constexpr float kSonarShipSonarRangeRealistic = 4000.0f;
inline constexpr float kSonarShipVisionRangeRealistic = 10000.0f;

// DEG(45) from scripts/global/luamw_init.lua line 434: 45 * pi / 180.
inline constexpr float kSonarShipBeamHalfAngle = 0.785398163397448f;

// The record the arcade table installs. Every row sits under ObserverSurface,
// so 55 of the 56 lists stay empty. Each row is {dist, gain, max_level,
// raw_type, has_angle, angle}; the reconstructed record is built from these by
// sensor_entry_from_lua_row_008085c0.
const SensorClassTable& sonar_ship_sensor_table_arcade();

} // namespace bsp
