#include "bsp/sensor_tables.hpp"

#include "bsp/recon_slot_lists.hpp"

#include <cmath>

// The "MaxLevel" cap and the published-level thresholds are the same two
// constants in the image (00CE3868 and 00D7A24C). If they ever drift apart the
// cap stops meaning "this entry may raise the target to at most this level".
static_assert(bsp::sensor_max_value_for_level_0080866b(1) == bsp::kReconDetectionBlipThreshold,
              "MaxLevel 1 caps the accumulator at the blip threshold, 00CE3868");
static_assert(bsp::sensor_max_value_for_level_0080866b(2) == bsp::kReconDetectionValueMax,
              "MaxLevel 2 caps the accumulator at the maximum, 00D7A24C");

// Reconstruction of the sensor tables behind unit detection. Read-only analysis
// of 00808F90, 008082A0, 00808CB0, 00808EA0, the seven +1E4h category accessors
// and the four Lua bindings on [game+21C4h]+74h/+78h. docs/SENSOR_TABLES.md
// carries the evidence. These are new C++ interfaces, not binary-compatible
// replacements: the native record is refcounted and its lists are raw
// {begin, count, capacity} triples.

namespace bsp {
namespace {

// 008085C5..008085F7: Dist is read as a Lua number, stored, then squared.
// 008085FF..00808626: Gain is read and multiplied by the double 0.5.
// The x87 path keeps double precision until the single store, which is what the
// float casts below reproduce.
float narrow(double value) noexcept { return static_cast<float>(value); }

// 00425850 / 00BF7FBF compare case-insensitively.
bool equals_insensitive(const char* candidate, const std::string& key) noexcept {
    for (std::size_t n = 0;; ++n) {
        const unsigned char a = static_cast<unsigned char>(candidate[n]);
        const unsigned char b = n < key.size() ? static_cast<unsigned char>(key[n]) : 0u;
        const unsigned char la = (a >= 'A' && a <= 'Z') ? static_cast<unsigned char>(a + 32) : a;
        const unsigned char lb = (b >= 'A' && b <= 'Z') ? static_cast<unsigned char>(b + 32) : b;
        if (la != lb) {
            return false;
        }
        if (la == 0) {
            return true;
        }
    }
}

int category_index_for_key(const std::array<const char*, 6>& keys,
                           const std::string& key) noexcept {
    for (std::size_t i = 0; i < keys.size(); ++i) {
        if (equals_insensitive(keys[i], key)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace

SensorTableEntry sensor_entry_from_lua_row_008085c0(const SensorLuaEntryRow& row) noexcept {
    SensorTableEntry entry{};
    entry.dist = narrow(row.dist);
    entry.dist_sq = entry.dist * entry.dist;
    entry.gain_per_second = narrow(row.gain * kSensorGainScale);
    entry.max_value = sensor_max_value_for_level_0080866b(row.max_level);
    entry.raw_type = row.raw_type;
    // 00808726: a nil "Angle" stores +0.0f and clears the flag.
    entry.half_angle = row.has_angle ? narrow(row.angle) : 0.0f;
    entry.bearing_limited = row.has_angle;
    return entry;
}

SensorGuiRangeRing sensor_ring_from_lua_row_00808791(const SensorLuaRingRow& row) noexcept {
    SensorGuiRangeRing ring{};
    ring.raw_type = row.raw_type;
    ring.has_angle = row.has_angle;
    ring.range = narrow(row.range);
    ring.half_angle = row.has_angle ? narrow(row.angle) : 0.0f;
    ring.color = row.color;
    return ring;
}

void load_sensor_class_table_008082a0(SensorClassTable& table, SensorTableLuaHost& host) {
    // 008082CD and 008082FE: both tests must pass or the record stays empty.
    if (!host.row_is_table_008082cd()) {
        return;
    }
    if (!host.open_details_008082e9()) {
        return;
    }

    // The two stack slots the native loader never initialises. -1 stands for
    // "uninitialised"; the native code would read whatever the frame held.
    int observer_index = -1;

    for (const std::string& details_key : host.details_keys_0080834a()) {
        observer_index = sensor_category_for_key_or_previous(
            category_index_for_key(kSensorObserverKeys, details_key), observer_index);

        int subject_index = -1;
        for (const std::string& group_key : host.observer_group_keys_008084b5(details_key)) {
            // 00808734: "GUIRange" is checked last and does not touch the
            // subject index.
            if (equals_insensitive(kSensorGuiRangeKey, group_key)) {
                if (sensor_category_is_valid(observer_index)) {
                    auto& rings = table.gui_rings[static_cast<std::size_t>(observer_index)];
                    for (const SensorLuaRingRow& row : host.gui_range_rows_00808786()) {
                        rings.push_back(sensor_ring_from_lua_row_00808791(row));
                    }
                }
                continue;
            }

            subject_index = sensor_category_for_key_or_previous(
                category_index_for_key(kSensorSubjectKeys, group_key), subject_index);
            if (!sensor_category_is_valid(observer_index) ||
                !sensor_category_is_valid(subject_index)) {
                continue;
            }

            auto& list = table.lists[sensor_list_index_008085aa(
                static_cast<SensorCategory>(observer_index),
                static_cast<SensorCategory>(subject_index))];
            for (const SensorLuaEntryRow& row : host.subject_rows_0080858c(group_key)) {
                list.push_back(sensor_entry_from_lua_row_008085c0(row));
            }
        }
    }
}

SensorClassTable* resolve_sensor_class_00808f90(int recon_class_id,
                                                SensorClassRegistryHost& host) {
    // 00808FBE..00808FE8: grow the registry, then read the slot.
    host.reserve_registry_00808fd1(recon_class_id);
    if (SensorClassTable* cached = host.cached_record_00808fe4(recon_class_id)) {
        // 00809002/00809015: a cached record skips every Lua step.
        return cached;
    }

    host.open_recon_class_global_00809041();
    host.open_class_row_0080905e(recon_class_id);
    // 00809085 and 008090C9: the resolver writes back into the script table
    // before it reads anything out of it.
    host.set_row_id_00809085(recon_class_id);
    host.set_row_got_008090c9(true);

    // 00809104: a null allocation skips the constructor, and 00809147 is still
    // reached with a null record.
    SensorClassTable* record = host.allocate_record_0080910d();
    if (record != nullptr) {
        host.load_from_row_00809147(*record);
    }
    host.publish_record_0080915f(recon_class_id, record);
    return record;
}

SensorCategory sensor_category_submarine_00852b90(
    float world_y, const SubmarineDepthBands& bands) noexcept {
    // 00852BAB..00852BD3.
    const float surface_limit =
        (bands.band_1204 + bands.band_1200) / kSubmarineSurfaceDivisor;
    if (world_y > surface_limit) {
        return SensorCategory::surface;
    }
    // 00852BE4..00852C02 and 00852C4C.
    const float periscope_limit = bands.band_1204 - kSubmarinePeriscopeMargin;
    if (periscope_limit <= world_y) {
        return bands.periscope_raised ? SensorCategory::periscope_out
                                      : SensorCategory::periscope_in;
    }
    // 00852C14..00852C4B.
    const float deep_limit = (bands.band_120c + bands.band_1208) * kSubmarineDeepMidpoint;
    return deep_limit <= world_y ? SensorCategory::underwater
                                 : SensorCategory::deep_underwater;
}

float sensor_effective_range_00804947(const SensorTableEntry& entry,
                                      float environment_factor,
                                      float target_signature,
                                      float recon_multiplier,
                                      bool submerged_submarine,
                                      float sonar_multiplier) noexcept {
    // 008048A0 works in squares: distSq / scale <= entry.dist_sq. The unsquared
    // form below is the same comparison with both sides rooted.
    float scale = environment_factor * environment_factor * target_signature *
                  recon_multiplier * recon_multiplier;
    if (submerged_submarine) {
        scale *= sonar_multiplier * sonar_multiplier;
    }
    if (scale <= 0.0f) {
        return 0.0f;
    }
    return entry.dist * std::sqrt(scale);
}

namespace {

// scripts/datatables/classtables/arcade/reconclasses.lua lines 71-108.
// Gain is 1/1 and MaxLevel 2 on every row, so every entry carries
// gain_per_second 0.5f and max_value 1.0f.
SensorClassTable build_sonar_ship_table_arcade() {
    const float sonar = kSonarShipSonarRangeArcade;
    const float vision = kSonarShipVisionRangeArcade;

    auto row = [](double dist, int raw_type, bool has_angle) {
        SensorLuaEntryRow r{};
        r.dist = dist;
        r.gain = 1.0;
        r.max_level = 2;
        r.raw_type = raw_type;
        r.has_angle = has_angle;
        r.angle = has_angle ? static_cast<double>(kSonarShipBeamHalfAngle) : 0.0;
        return r;
    };
    const int vision_type = static_cast<int>(SensorRawType::vision);
    const int sonar_type = static_cast<int>(SensorRawType::sonar);

    SensorClassTable table{};
    const SensorCategory observer = SensorCategory::surface;

    table.list(observer, SensorCategory::air)
        .push_back(sensor_entry_from_lua_row_008085c0(row(vision, vision_type, false)));
    table.list(observer, SensorCategory::surface)
        .push_back(sensor_entry_from_lua_row_008085c0(row(vision, vision_type, false)));

    for (SensorCategory subject :
         {SensorCategory::periscope_out, SensorCategory::periscope_in}) {
        auto& list = table.list(observer, subject);
        list.push_back(sensor_entry_from_lua_row_008085c0(row(sonar, sonar_type, false)));
        list.push_back(
            sensor_entry_from_lua_row_008085c0(row(sonar * 2.5, sonar_type, true)));
    }

    auto& underwater = table.list(observer, SensorCategory::underwater);
    underwater.push_back(
        sensor_entry_from_lua_row_008085c0(row(sonar * 0.66, sonar_type, false)));
    underwater.push_back(
        sensor_entry_from_lua_row_008085c0(row(sonar * 1.66, sonar_type, true)));

    // The three GUIRange rings, in script order. Color is a Lua constant this
    // packet did not decode, so the four words stay zero.
    auto ring = [](double range, int raw_type, bool has_angle) {
        SensorLuaRingRow r{};
        r.range = range;
        r.raw_type = raw_type;
        r.has_angle = has_angle;
        r.angle = has_angle ? static_cast<double>(kSonarShipBeamHalfAngle) : 0.0;
        return r;
    };
    auto& rings = table.gui_rings[static_cast<std::size_t>(observer)];
    rings.push_back(sensor_ring_from_lua_row_00808791(ring(sonar, sonar_type, false)));
    rings.push_back(sensor_ring_from_lua_row_00808791(ring(sonar * 2.5, sonar_type, true)));
    rings.push_back(sensor_ring_from_lua_row_00808791(ring(vision, vision_type, false)));
    return table;
}

} // namespace

const SensorClassTable& sonar_ship_sensor_table_arcade() {
    static const SensorClassTable table = build_sonar_ship_table_arcade();
    return table;
}

} // namespace bsp
