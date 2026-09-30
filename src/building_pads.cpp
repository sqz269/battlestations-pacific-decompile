// Packet cc9_building_pad_model. See bsp/building_pads.hpp and
// docs/SHIP_AI_OPEN_ITEMS.md sections 74.5 and 75.
#include "bsp/building_pads.hpp"

#include <cfloat>

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
}

}  // namespace bsp
