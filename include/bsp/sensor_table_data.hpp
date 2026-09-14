#pragma once

#include <cstddef>
#include <iterator>

#include "bsp/sensor_tables.hpp"

// The AUTHORED data behind rule (c) of the contact list.
//
// bsp/sensor_tables.hpp already declares the producer: the 2A8h record, the
// 1Ch entry, the 20h GUIRange ring, the seven categories, the loader
// 008082A0 and its two per-row conversions. bsp/recon_slot_lists.hpp and
// bsp/gunnery_recon_detection.hpp declare the consumer, 008048A0. Nothing
// from either is redeclared here.
//
// What this header adds is the shipped content those rules run against: the
// twelve `ReconClass` rows of
//   scripts/datatables/classtables/arcade/reconclasses.lua     (ArcadeTable)
//   scripts/datatables/classtables/realistic/reconclasses.lua  (RealisticTable)
// read out of the installed game and kept in their AUTHORED form - the Lua
// numbers as the scripts write them, before 008082A0 squares, halves and
// clamps. build_sensor_class_table_008082a0 then applies the loader's own
// arithmetic (sensor_entry_from_lua_row_008085c0 and
// sensor_ring_from_lua_row_00808791), so nothing here re-derives a value the
// producer already computes.
//
// docs/SENSOR_TABLE_DATA.md carries the evidence and the provenance, including
// the fact that the installation these rows were read from is modded. The two
// reconclasses.lua files carry the untouched-datatables timestamp; no pristine
// retail copy was available to diff against.
//
// The Lua key names in the data are RECOVERED STRINGS, cited at their
// addresses in the doc. The C++ identifiers below are hypotheses.

namespace bsp {

// scripts/datatables/autoload/reconclasses.lua: `GameMode == 1` loads the
// realistic file and aliases RealisticTable onto the global ReconClass;
// anything else loads the arcade file. The installation read here has
// `GameMode = 0` in its root gamemode.lua.
enum class ReconTableVariant : int { arcade = 0, realistic = 1 };

// One authored row of a `Subject...` list, with the two category keys it sits
// under. `row` is the Lua row verbatim; 008085C0..00808A6A converts it.
struct ReconAuthoredEntry {
    SensorCategory observer;
    SensorCategory subject;
    SensorLuaEntryRow row;
};

// One authored row of a `GUIRange` list. The list lives per observer category
// at record + 5Ch + observer * 60h (008087B2), which is column 7 of the
// observer's group of eight headers; 008048A0 never reads it.
struct ReconAuthoredRing {
    SensorCategory observer;
    SensorLuaRingRow row;
};

// One `ReconClass[id]` row of the shipped table. `script_comment` is the
// comment the script itself carries above the row.
struct ReconClassSource {
    int recon_class_id;
    const char* script_comment;
    const ReconAuthoredEntry* entries;
    std::size_t entry_count;
    const ReconAuthoredRing* rings;
    std::size_t ring_count;
};

// Both shipped files define ids 1..12 and nothing else.
inline constexpr std::size_t kReconClassCount = 12;
inline constexpr int kReconClassIdMin = 1;
inline constexpr int kReconClassIdMax = 12;

// The base ranges the two files differ by; every other byte of the two files is
// the same. Metres, as authored.
struct ReconBaseRanges {
    float normal_ship_vision;
    float radar_ship_radar;
    float sonar_ship_sonar;
    float radar_and_sonar_ship_radar;
    float radar_and_sonar_ship_sonar;
    float normal_plane_vision;
    float recon_plane_vision;
    float recon_plane_sonar;
    float normal_sub_vision;
    float normal_sub_sonar;
    float ground_unit_vision;
    float ground_observer_unit_vision;
    float ground_radar_unit_radar;
    float ground_radar_unit_vision;
    float small_radar_unit_radar;
};

inline constexpr ReconBaseRanges kReconBaseRangesArcade{
    4000.0f, 6000.0f, 1000.0f, 5000.0f, 1000.0f, 4000.0f, 5000.0f, 750.0f,
    4000.0f, 1800.0f, 4000.0f, 6000.0f, 10000.0f, 4000.0f, 5000.0f};

inline constexpr ReconBaseRanges kReconBaseRangesRealistic{
    10000.0f, 20000.0f, 4000.0f, 10000.0f, 4000.0f, 10000.0f, 10000.0f, 1000.0f,
    9000.0f, 4000.0f, 10000.0f, 10000.0f, 20000.0f, 10000.0f, 20000.0f};

// DEG(45) from scripts/global/luamw_init.lua line 434, the only angle the
// shipped tables author. bsp/sensor_tables.hpp already names the float form
// as kSonarShipBeamHalfAngle; this is the same value.
inline constexpr double kReconAuthoredBeamHalfAngle = 0.78539816339744828;

// The authored rows of one class, or nullptr when the id is outside 1..12.
const ReconClassSource* recon_class_source(ReconTableVariant variant,
                                           int recon_class_id) noexcept;

// The record 008082A0 builds for that class: every authored row run through
// the loader's own conversions and filed under sensor_list_index_008085aa.
// An unknown id gives the empty record the loader leaves behind when
// row.Details is not a table (008082FE/00808305).
SensorClassTable build_sensor_class_table_008082a0(ReconTableVariant variant,
                                                   int recon_class_id);

} // namespace bsp
