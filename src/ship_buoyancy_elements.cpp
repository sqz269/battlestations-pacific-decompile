// The producer of the vehicle class's buoyancy element list at class+52Ch.
// See include/bsp/ship_buoyancy_elements.hpp and docs/SHIP_BUOYANCY_ELEMENTS.md for the
// addresses, the evidence and the uncertainty behind every rule below.

#include "bsp/ship_buoyancy_elements.hpp"

#include <cstddef>
#include <vector>

namespace bsp {

float ship_buoyancy_element_water_line(const ShipBuoyancyElement& element) noexcept {
    return element.level_top;
}

float ship_buoyancy_element_deck_level(const ShipBuoyancyElement& element) noexcept {
    return element.level_draft;
}

float ship_buoyancy_element_bottom_level(const ShipBuoyancyElement& element) noexcept {
    return element.level_base;
}

float ship_buoyancy_element_section_height(const ShipBuoyancyElement& element) noexcept {
    return element.unread_10;
}

float ship_buoyancy_element_draught(const ShipBuoyancyElement& element) noexcept {
    return element.unread_14;
}

// 0082A920. The walk starts at `first` and the entry test is
// `if (first->z <= z)`; the else branch returns `first->y`. Inside, the loop advances
// while `z >= next->z` and interpolates on the first span whose far end is past z; when
// the walk reaches `last` it returns `last[-1].y`.
float ship_buoyancy_sample_profile_0082a920(const OceanVec3* first, const OceanVec3* last,
                                            float z) noexcept {
    if (first == nullptr || last == nullptr || first >= last) {
        return 0.0f;
    }
    if (z < first->z) {
        return first->y;
    }
    for (const OceanVec3* p = first; p + 1 != last; ++p) {
        const OceanVec3* q = p + 1;
        if (z < q->z) {
            const float span = q->z - p->z;
            const float t = (z - p->z) / span;
            return (1.0f - t) * p->y + q->y * t;
        }
    }
    return (last - 1)->y;
}

// 0082D21F..0082D365. Take the first strictly-smallest z, append, erase, repeat.
std::vector<OceanVec3> ship_buoyancy_sort_profile_by_z_0082d21f(std::vector<OceanVec3> points) {
    std::vector<OceanVec3> sorted;
    sorted.reserve(points.size());
    while (!points.empty()) {
        // 0082D2A7: the running minimum starts at the +FLT_MAX at 00D7A248 and the test is
        // a strict `<`, so the first point always wins and a later equal z never displaces
        // an earlier one.
        std::size_t best = 0;
        for (std::size_t i = 1; i < points.size(); ++i) {
            if (points[i].z < points[best].z) {
                best = i;
            }
        }
        sorted.push_back(points[best]);
        points.erase(points.begin() + static_cast<std::ptrdiff_t>(best));
    }
    return sorted;
}

// 0082D500..0082D54E.
float ship_buoyancy_station_z_0082d500(int index, int segments, float length) noexcept {
    if (segments < 2) {
        return 0.0f;
    }
    const float start = -length * kShipBuoyancyHalfExtent;
    return (static_cast<float>(index) * length) / static_cast<float>(segments - 1) + start;
}

// 0082D5E4..0082D651. The listing's order, term by term.
ShipBuoyancyElement ship_buoyancy_make_element_0082d5e4(float deck_level, float bottom_level,
                                                        float water_line_ratio, float lateral,
                                                        float station_z, float mass,
                                                        int segments) noexcept {
    ShipBuoyancyElement element{};

    // 0082D61A..0082D622: deck * (1 - ratio) + bottom * ratio.
    const float water_line =
        deck_level * (1.0f - water_line_ratio) + bottom_level * water_line_ratio;

    // 0082D604..0082D608 and 0082D629..0082D630.
    const float section_height = deck_level - bottom_level;
    const float draught = water_line - bottom_level;

    element.coefficient = ((mass * kShipBuoyancyMassToDisplacement * kShipBuoyancyHalfExtent) /
                           draught) /
                          static_cast<float>(segments);  // 0082D637..0082D651
    element.level_top = water_line;                      // +04h
    element.level_draft = deck_level;                    // +08h
    element.level_base = bottom_level;                   // +0Ch
    element.unread_10 = section_height;                  // +10h
    element.unread_14 = draught;                         // +14h
    element.position.x = lateral;                        // +18h
    element.position.y = deck_level;                     // +1Ch
    element.position.z = station_z;                      // +20h
    return element;
}

// 0082D4CA..0082D690.
std::vector<ShipBuoyancyElement> ship_buoyancy_build_list_0082d4ca(
    const ShipBuoyancyHullInputs& hull, const std::vector<OceanVec3>& deck_points,
    const std::vector<OceanVec3>& bottom_points) {
    std::vector<ShipBuoyancyElement> elements;
    if (hull.hull_segments <= 0) {
        return elements;  // 0082D4DC JLE
    }

    const std::vector<OceanVec3> deck = ship_buoyancy_sort_profile_by_z_0082d21f(deck_points);
    const std::vector<OceanVec3> bottom =
        ship_buoyancy_sort_profile_by_z_0082d21f(bottom_points);

    const float half_beam = hull.width * kShipBuoyancyHalfExtent;  // 0082D507..0082D513
    elements.reserve(static_cast<std::size_t>(hull.hull_segments) * 2u);

    for (int i = 0; i < hull.hull_segments; ++i) {
        const float station_z =
            ship_buoyancy_station_z_0082d500(i, hull.hull_segments, hull.length);

        // 0082D585: the deck line first, into +08h and the point's y.
        const float deck_level = ship_buoyancy_sample_profile_0082a920(
            deck.data(), deck.data() + deck.size(), station_z);
        // 0082D5D8: the bottom line, into +0Ch.
        const float bottom_level = ship_buoyancy_sample_profile_0082a920(
            bottom.data(), bottom.data() + bottom.size(), station_z);

        // 0082D658 then 0082D67C: the same record pushed twice, the second with +18h
        // negated at 0082D65D..0082D675.
        elements.push_back(ship_buoyancy_make_element_0082d5e4(
            deck_level, bottom_level, hull.hull_water_line_ratio, half_beam, station_z,
            hull.mass, hull.hull_segments));
        elements.push_back(ship_buoyancy_make_element_0082d5e4(
            deck_level, bottom_level, hull.hull_water_line_ratio, -half_beam, station_z,
            hull.mass, hull.hull_segments));
    }
    return elements;
}

// 0082FEA9..0082FEE8.
bool ship_buoyancy_build_from_model_0082fea9(ShipBuoyancyElementHost& host,
                                             const ShipBuoyancyHullInputs& hull,
                                             std::vector<ShipBuoyancyElement>& out) {
    std::vector<OceanVec3> deck_points;
    if (!host.find_model_profile_points(kShipBuoyancyDeckLineNode,
                                        kShipBuoyancyProfileNodeKind, deck_points)) {
        return false;  // 0082FEBA
    }
    std::vector<OceanVec3> bottom_points;
    if (!host.find_model_profile_points(kShipBuoyancyBottomLineNode,
                                        kShipBuoyancyProfileNodeKind, bottom_points)) {
        return false;  // 0082FECA
    }
    out = ship_buoyancy_build_list_0082d4ca(hull, deck_points, bottom_points);  // 0082FEE3
    return true;
}

}  // namespace bsp
