// Packet cc9_building_pad_model. See bsp/building_pads.hpp and
// docs/SHIP_AI_OPEN_ITEMS.md sections 74.5 and 75.
#include "bsp/building_pads.hpp"

#include <cfloat>

#include "bsp/vector_helpers.hpp"

namespace bsp {
namespace {

// The squared 3-D distance as the image sums it, rounded to float32 once.
float squared_distance(const float a[3], const float b[3]) {
    const double dx = static_cast<double>(a[0]) - b[0];
    const double dy = static_cast<double>(a[1]) - b[1];
    const double dz = static_cast<double>(a[2]) - b[2];
    return static_cast<float>(dz * dz + dy * dy + dx * dx);
}

const std::vector<int> kNoPads;

}  // namespace

bool building_pad_in_range_006f5cc0(const float building[3], const float pad[3],
                                    std::int32_t landing_point_range) {
    const double dx = static_cast<double>(pad[0]) - building[0];  // 006F6603-006F661A
    const double dy = static_cast<double>(pad[1]) - building[1];  // 006F661E-006F662A
    const double dz = static_cast<double>(pad[2]) - building[2];  // 006F662E-006F663A
    const float d = static_cast<float>((dy * dy + dx * dx) + dz * dz);  // 006F665A
    // 006F6617 IMUL ECX,EAX: a 32-bit product, wrapping as the image's does.
    const std::int32_t product = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(landing_point_range)
        * static_cast<std::uint32_t>(landing_point_range));
    return !(static_cast<double>(d) > static_cast<double>(product));  // 006F6670 JA
}

int BuildingPadModel::intern_pad(const Pad& pad) {
    for (std::size_t i = 0; i < pads_.size(); ++i) {
        if (pads_[i].marker_id == pad.marker_id) return static_cast<int>(i);
    }
    pads_.push_back(pad);
    return static_cast<int>(pads_.size() - 1);
}

void BuildingPadModel::adopt_landing_pads_006f5cc0(int building,
    const float building_position[3], std::int32_t landing_point_range,
    const std::vector<Pad>& landing_points_in_list_order) {
    std::vector<int>& vector = vectors_[building];
    for (const Pad& candidate : landing_points_in_list_order) {
        if (!building_pad_in_range_006f5cc0(building_position, candidate.position,
                                            landing_point_range)) {
            continue;
        }
        const int index = intern_pad(candidate);
        vector.push_back(index);          // 006F66FF-006F670B
        pads_[static_cast<std::size_t>(index)].owner = building;  // 006F6711, pad+220h
    }
}

int BuildingPadModel::pick_006f2e60(int building, int unit, const float unit_position[3],
                                    bool ignore_held) const {
    if (unit < 0) return -1;  // 006F2E6A
    int best = -1;
    float best_d = FLT_MAX;   // 00D7A248
    for (const int index : pads_of(building)) {
        const Pad& p = pads_[static_cast<std::size_t>(index)];
        if (p.occupant < 0) {
            const float d = squared_distance(unit_position, p.position);
            if (best < 0 || d < best_d) {
                best = index;
                best_d = d;
            }
        } else if (!ignore_held && p.occupant == unit) {
            return index;
        }
    }
    return best;
}

void BuildingPadModel::release_unit_pads_006f2de0(int building, int unit) {
    for (const int index : pads_of(building)) {
        if (pads_[static_cast<std::size_t>(index)].occupant == unit) {
            set_occupant_006ac490(index, -1);
        }
    }
}

void BuildingPadModel::assign_006f2fb0(int building, int unit, int pad) {
    if (pad >= 0 && pads_[static_cast<std::size_t>(pad)].occupant == unit) return;
    release_unit_pads_006f2de0(building, unit);
    if (pad >= 0) set_occupant_006ac490(pad, unit);
}

void BuildingPadModel::set_occupant_006ac490(int pad, int unit) {
    if (pad < 0 || static_cast<std::size_t>(pad) >= pads_.size()) return;
    pads_[static_cast<std::size_t>(pad)].occupant = unit;
}

void BuildingPadModel::forget_unit(int unit) {
    for (Pad& p : pads_) {
        if (p.occupant == unit) p.occupant = -1;
    }
}

void BuildingPadModel::begin_landing_0074a990(int ship, int pad, int building, float draw,
                                              std::int32_t spawn_phase_1208) {
    Lander& l = landers_[ship];
    l.building_1204 = building;       // 0074A9AA
    l.pad_1200 = pad;                 // 0074A9B0
    set_occupant_006ac490(pad, ship); // 0074A9B6
    // 0074A9D2-0074A9E6: (U(0, 0.75f) FIADD +1208h) FMUL double 1.5, FSTP float.
    l.landing_time_1210 = static_cast<float>(
        (static_cast<double>(draw) + static_cast<double>(spawn_phase_1208)) * 1.5);
}

int BuildingPadModel::lander_pad_1200(int ship) const {
    const auto found = landers_.find(ship);
    return found != landers_.end() ? found->second.pad_1200 : -1;
}

const BuildingPadModel::Lander* BuildingPadModel::lander(int ship) const {
    const auto found = landers_.find(ship);
    return found != landers_.end() ? &found->second : nullptr;
}

BuildingPadModel::Pad* BuildingPadModel::mutable_pad(int index) {
    if (index < 0 || static_cast<std::size_t>(index) >= pads_.size()) return nullptr;
    return &pads_[static_cast<std::size_t>(index)];
}

bool refresh_pad_line_006ac5d0(BuildingPadModel::Pad& pad, int layer,
                               PadLineZoneQueries& zones) {
    ShipAiLandPadLine& line = pad.line;
    if (line.zone_group_208 == layer) return false;            // 006AC5F2
    const float fx = pad.facing_x;                             // 006AC260
    const float fz = pad.facing_z;
    line.zone_group_208 = layer;
    line.origin_x_20c = pad.position[0];
    line.origin_y_210 = pad.position[1];
    line.origin_z_214 = pad.position[2];
    line.astern_clearance_218 = 1000.0f;                       // 00CE3804
    line.ahead_run_21c = 800.0f;                               // 00CE3950
    const std::uint32_t group = zones.group_for_layer_004120d0(layer);
    if (group == 0u) return true;
    float p[2] = {pad.position[0], pad.position[2]};
    float hit[2] = {0.0f, 0.0f};
    bool struck = false;
    const std::uint32_t zone = zones.zone_containing_004178f0(group, p[0], p[1]);
    if (zone == 0u) {
        // Outside every zone: cast from 100000 ahead [00CF81F0] back to the pad.
        const float far_point[2] = {static_cast<float>(fx * 100000.0) + p[0],
                                    static_cast<float>(100000.0 * fz) + p[1]};
        struck = zones.group_segment_hit_0041b4e0(group, p, far_point, hit);
    } else {
        // Inside one: cast from 100000 astern [00CF81F8] against that zone.
        const float far_point[2] = {static_cast<float>(fx * -100000.0) + p[0],
                                    static_cast<float>(-100000.0 * fz) + p[1]};
        struck = zones.zone_segment_hit_00416dd0(zone, p, far_point, hit);
    }
    if (struck) {
        p[0] = hit[0] - fx;
        p[1] = hit[1] - fz;
    }
    line.origin_x_20c = p[0];
    line.origin_y_210 = 0.0f;
    line.origin_z_214 = p[1];
    // The astern clearance: cast from one clearance behind back to the origin.
    const float back[2] = {p[0] - line.astern_clearance_218 * fx,
                           p[1] - line.astern_clearance_218 * fz};
    float out[2] = {0.0f, 0.0f};
    if (zones.group_segment_hit_0041b4e0(group, p, back, out)) {
        line.astern_clearance_218 = length_2d_00414c60(
            std::array<float, 2>{p[0] - out[0], p[1] - out[1]});
    }
    // The run ahead: from 5 [00D7A370] ahead of the origin toward one run ahead.
    const float start[2] = {static_cast<float>(fx * 5.0) + p[0],
                            static_cast<float>(fz * 5.0) + p[1]};
    const float end[2] = {p[0] + line.ahead_run_21c * fx, p[1] + fz * line.ahead_run_21c};
    if (zones.group_segment_hit_0041b4e0(group, start, end, out)) {
        line.ahead_run_21c = static_cast<float>(
            static_cast<double>(length_2d_00414c60(
                std::array<float, 2>{p[0] - out[0], p[1] - out[1]}))
            * 0.89999997615814209);                            // 00D7A390
    }
    return true;
}

BuildingPadModel& building_pad_model() {
    static BuildingPadModel model;
    return model;
}

void BuildingPadModel::nearest_pad_xz_006f3af0(int building, const float building_position[3],
    const float from[3], float out_xz[2]) const {
    out_xz[0] = building_position[0];
    out_xz[1] = building_position[2];
    float best_d = FLT_MAX;
    for (const int index : pads_of(building)) {
        const Pad& p = pads_[static_cast<std::size_t>(index)];
        const float d = squared_distance(p.position, from);
        if (d < best_d) {
            best_d = d;
            out_xz[0] = p.position[0];
            out_xz[1] = p.position[2];
        }
    }
}

const std::vector<int>& BuildingPadModel::pads_of(int building) const {
    const auto found = vectors_.find(building);
    return found != vectors_.end() ? found->second : kNoPads;
}

const BuildingPadModel::Pad* BuildingPadModel::pad(int index) const {
    if (index < 0 || static_cast<std::size_t>(index) >= pads_.size()) return nullptr;
    return &pads_[static_cast<std::size_t>(index)];
}

void BuildingPadModel::clear() {
    pads_.clear();
    vectors_.clear();
    landers_.clear();
}

}  // namespace bsp
