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

BuildingPadModel::Lander* BuildingPadModel::mutable_lander(int ship) {
    const auto found = landers_.find(ship);
    return found != landers_.end() ? &found->second : nullptr;
}

bool landing_ship_ramp_latch_0074afcc(BuildingPadModel::Lander& l, bool ground_1011,
                                      float clock, float dt) {
    // x87 intermediates are extended precision; here double.
    const double c = static_cast<double>(clock);
    const bool held_before =
        static_cast<double>(l.last_ground_11a8) > (c - 1.0) - static_cast<double>(dt);
    if (ground_1011) l.last_ground_11a8 = clock;                       // 0074B006
    const bool held_now = static_cast<double>(l.last_ground_11a8) > c - 1.0;
    if (held_now != held_before) {                                     // 0074B028
        l.ramp_timer_11ac = l.ramp_delay_11b0;                         // 0074B02E
        return false;
    }
    if (!(l.ramp_timer_11ac > 0.0f)) return false;                     // 0074B044 COMISS, JBE
    l.ramp_timer_11ac = static_cast<float>(
        static_cast<double>(l.ramp_timer_11ac) - static_cast<double>(dt));  // 0074B053..5F
    if (0.0f < l.ramp_timer_11ac) return false;                        // 0074B067, JB
    if (l.ramp_down_1188) return false;                                // 0074B06D -> 0074B109
    if (!held_now) return false;                                       // 0074B07A TEST CL,CL
    l.ramp_down_1188 = true;                                           // 0074A420
    return true;
}

bool landing_ship_ramp_unload_0074b109(BuildingPadModel::Lander& l, float dt) {
    const float r = l.ramp_rotation_11a4;                               // 0074B111
    const double step = static_cast<double>(dt)
        / static_cast<double>(l.ramp_rotation_time_1190);
    if (l.ramp_down_1188) {                                             // 0074B10F
        if (1.0f <= r) {                                                // 0074B133, JBE
            if (l.landing_118a || l.fast_landing_118b) return false;    // 0074B16E, 0074B17B
            l.fast_landing_118b = true;                                 // 0074B18F
            l.landed_118c = true;                                       // 0074AD90
            return true;
        }
        const float up = static_cast<float>(step + static_cast<double>(r));  // 0074B14F FSTP
        l.ramp_rotation_11a4 = 1.0f > up ? up : 1.0f;                   // 006F22B0(ECX = 1)
        return false;
    }
    if (!(r > 0.0f)) return false;                                      // 0074B2F4, JBE
    const float down = static_cast<float>(static_cast<double>(r) - step);  // 0074B313 FSTP
    l.ramp_rotation_11a4 = down > 0.0f ? down : 0.0f;                   // 006F22F0(ECX = 0)
    return false;
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
    if constexpr (kPadApproachPointImageCc13Bound) {
        zones.refresh_pad_pose_00414db0(pad);                  // 006AC620
    }
    line.origin_x_20c = pad.position[0];
    line.origin_y_210 = pad.position[1];
    line.origin_z_214 = pad.position[2];
    line.astern_clearance_218 = 1000.0f;                       // 00CE3804
    line.ahead_run_21c = 800.0f;                               // 00CE3950
    const std::uint32_t group = zones.group_for_layer_004120d0(layer);
    if (group == 0u) return true;
    if constexpr (kPadApproachPointImageCc13Bound) {
        zones.refresh_pad_pose_00414db0(pad);                  // 006AC68B
    }
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
    unloads_.clear();
}

// 0074AF50..0074B567; ordinary path including every early return, latch,
// capture warning, one-shot publication, ramp record and final debug getter.
// Evidence: complete saved listing; the body ends in RET 4 at 0074B565.
// 006F22B0/006F22F0 were inspected and lowered to their exact comparisons.
// The native FH3 cleanup states are not reproduced. Arithmetic preserves the
// float stores; C++ double temporaries do not establish x87 bit equivalence.
void landing_ship_update_0074af50(std::uint8_t* ship, float dt,
                                const LandingShipUpdateGlobals& globals,
                                LandingShipUpdateHost& host) {
    const auto byte = [](void* object, std::size_t offset) -> std::uint8_t& {
        return *(static_cast<std::uint8_t*>(object) + offset);
    };
    const auto word = [](void* object, std::size_t offset) -> std::uint32_t& {
        return *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(object) + offset);
    };
    const auto value = [](void* object, std::size_t offset) -> float& {
        return *reinterpret_cast<float*>(static_cast<std::uint8_t*>(object) + offset);
    };
    const auto pointer = [&](void* object, std::size_t offset) -> std::uint8_t* {
        return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(word(object, offset)));
    };
    host.call_008255b0(ship, dt);                                      // 0074AF76
    if (byte(ship, 0x5C) == 0 || byte(ship, 0x5D) != 0
        || byte(ship, 0x60) != 0 || byte(ship, 0x5E) != 0) return;

    if (word(globals.game_00e188a8, 0x1FE4) != 2 && word(ship, 0x1200) != 0) {
        const float previous_contact = value(ship, 0x11A8);
        const double previous_threshold = static_cast<double>(globals.time_00f876a4) - 1.0 - dt;
        const bool was_recent = previous_contact > previous_threshold;
        if (byte(ship, 0x1011) != 0) value(ship, 0x11A8) = globals.time_00f876a4;
        const bool recent = value(ship, 0x11A8) > static_cast<double>(globals.time_00f876a4) - 1.0;
        if (recent != was_recent) {
            value(ship, 0x11AC) = value(ship, 0x11B0);
        } else if (value(ship, 0x11AC) > 0.0f) {
            value(ship, 0x11AC) = value(ship, 0x11AC) - dt;
            if (value(ship, 0x11AC) <= 0.0f && byte(ship, 0x1188) == 0 && recent) {
                host.call_0074a420(ship);                             // 0074B080
                alignas(4) std::uint8_t message[0x5C];
                void* constructed = host.call_00749aa0(message);       // 0074B089
                host.call_0077c2a0(ship, constructed, 4, 0);            // 0074B0A0
            }
        }
    }

    if (byte(ship, 0x1188) == 0 && word(ship, 0x1200) == 0
        && host.vcall_240(ship) && host.vcall_174(ship) != nullptr) {
        void* object = host.vcall_174(ship);                           // 0074B0EC (second call)
        if (!host.vcall_08(object))
            host.call_00977da0(globals.warning_manager_00f8a0c4, nullptr, ship); // 0074B104
    }

    const float old_fraction = value(ship, 0x11A4);
    if (byte(ship, 0x1188) != 0) {
        // COMISS 1,fraction / JBE also takes the completion arm for NaN.
        if (1.0f > old_fraction) {
            const float candidate = static_cast<float>(static_cast<double>(dt)
                / value(ship, 0x1190) + old_fraction);
            value(ship, 0x11A4) = candidate < 1.0f ? candidate : 1.0f;
        } else if (byte(ship, 0x118A) == 0 && byte(ship, 0x118B) == 0) {
            byte(ship, 0x118B) = 1;
            if (word(ship, 0x1200) != 0) {
                std::uint8_t* descriptor = pointer(ship, 0x538);
                const float* matrix = host.call_0042d7e0(ship);        // 0074B1A2
                float scaled[3];
                const float* result = host.call_00414260(matrix + 8, scaled,
                                                         value(descriptor, 0xA0)); // 0074B1B9
                float half[3] = {static_cast<float>(result[0] * 0.5),
                                 static_cast<float>(result[1] * 0.5),
                                 static_cast<float>(result[2] * 0.5)};
                const float* position = host.call_00427eb0(ship);      // 0074B1E0
                const float unload[3] = {position[0] + half[0], position[1] + half[1],
                                         position[2] + half[2]};
                host.call_006ac370(pointer(ship, 0x1200), unload);     // 0074B228
                if (byte(pointer(ship, 0x538), 0x808) == 0) {
                    void* model = host.call_00476b90(ship);            // 0074B243
                    void* node = host.call_0071ad50(model, "soldiers"); // 0074B24A
                    if (node != nullptr) host.call_00b6da70(node, 0.0f, 0); // 0074B25D
                }
            }
            void* lock_owner = host.call_004c1570();                   // 0074B262
            alignas(4) std::uint8_t guard[8];
            alignas(4) std::uint8_t name[8];
            alignas(4) std::uint8_t lua_object[20];
            host.call_00411eb0(guard, pointer(lock_owner, 4));          // 0074B26F
            host.call_0041e870(name, "LandingStarted");               // 0074B288
            void* self = host.call_00927b40(ship, lua_object);          // 0074B29B
            host.call_00b673a0(self, name, true);                       // 0074B2B1
            host.call_00b67700(lua_object);                            // 0074B2C1
            host.call_0041dd20(name);                                  // 0074B2D2
            host.call_0074ad90(ship);                                  // 0074B2D9
            host.call_00411ee0(guard);                                 // 0074B2E9
        }
    } else if (old_fraction > 0.0f) {
        const float candidate = static_cast<float>(static_cast<double>(old_fraction)
            - static_cast<double>(dt) / value(ship, 0x1190));
        value(ship, 0x11A4) = 0.0f < candidate ? candidate : 0.0f;
    }

    if (old_fraction != value(ship, 0x11A4)) {
        const auto count = [&]() -> std::uint32_t {
            const auto bytes = static_cast<std::int32_t>(word(ship, 0x119C) - word(ship, 0x1198));
            return static_cast<std::uint32_t>(bytes / 0x4C);
        };
        const auto checked = [&](std::uint32_t index) -> std::uint8_t* {
            if (word(ship, 0x1198) == 0 || index >= count()) host.invalid_iterator_00bf6713();
            return pointer(ship, 0x1198) + index * 0x4Cu;
        };
        for (std::uint32_t index = 0; word(ship, 0x1198) != 0 && index < count(); ++index) {
            float* angle = &value(checked(index), 0x44);
            std::uint8_t* from = checked(index);
            std::uint8_t* to = checked(index);
            (void)checked(index);
            *angle = static_cast<float>((static_cast<double>(value(to, 0x48)) - value(from, 0x44))
                * value(ship, 0x11A4) + value(pointer(ship, 0x1198) + index * 0x4Cu, 0x44));
            void* node = pointer(checked(index), 0);
            (void)checked(index);
            const float negative_angle = -0.0f - *angle;               // SUBSS 0074B4D6
            std::uint8_t* record = pointer(ship, 0x1198) + index * 0x4Cu;
            float rotation[16], copied[16], combined[16];
            const float* built = host.call_00b64640(rotation, &negative_angle); // 0074B4F3
            host.call_004134f0(copied, built);                         // 0074B4FD
            std::uint8_t* selected_slot = pointer(node, 0) + 0x38;     // 0074B506/0074B518
            const float* matrix = host.call_00413920(copied, combined,
                reinterpret_cast<const float*>(record + 4));          // 0074B51B
            host.vcall_38(node, matrix, word(selected_slot, 0));        // 0074B524/0074B527
        }
    }
    if (byte(globals.game_00e188a8, 0x19C4) == 0) host.call_0051f460();  // 0074B54B
}

}  // namespace bsp
