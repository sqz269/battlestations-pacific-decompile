#include "bsp/ship_ai_layer_selection.hpp"

#include "bsp/avoid_zone_manager_queries.hpp"
#include "bsp/unit_rudder.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
// Each operation below ends at a native FSTP32 boundary. Keeping these in x87
// preserves the caller's precision/rounding controls and avoids SSE arithmetic
// or fused expressions crossing the original spills.
float spill(float value) noexcept {
    __asm {
        fld value
        fstp value
    }
    return value;
}
float add(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fadd right
        fstp result
    }
    return result;
}
float subtract(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fsub right
        fstp result
    }
    return result;
}
float multiply(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fmul right
        fstp result
    }
    return result;
}
float divide(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fdiv right
        fstp result
    }
    return result;
}
float negate_divide(float left, float right) noexcept {
    float result;
    __asm {
        fld left
        fchs
        fdiv right
        fstp result
    }
    return result;
}
bool x87_above(float left, float right) noexcept {
    std::uint8_t result;
    __asm {
        fld right
        fld left
        fcomip st(0), st(1)
        fstp st(0)
        seta result
    }
    return result != 0;
}
bool x87_carry(float left, float right) noexcept {
    std::uint8_t result;
    __asm {
        fld right
        fld left
        fcomip st(0), st(1)
        fstp st(0)
        setb result
    }
    return result != 0;
}
bool sse_carry(float left, float right) noexcept {
    std::uint8_t result;
    __asm {
        movss xmm0, left
        comiss xmm0, right
        setb result
    }
    return result != 0;
}
bool sse_below_equal(float left, float right) noexcept {
    std::uint8_t result;
    __asm {
        movss xmm0, left
        comiss xmm0, right
        setbe result
    }
    return result != 0;
}
float squared_length(const std::array<float, 2>& value) noexcept {
    const float* p = value.data();
    float result;
    __asm {
        mov ecx, p
        fld dword ptr [ecx + 4]
        fld dword ptr [ecx]
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp result
    }
    return result;
}
std::array<float, 2> difference(const std::array<float, 2>& a,
    const std::array<float, 2>& b) noexcept {
    const float x = subtract(a[0], b[0]);
    const float z = subtract(a[1], b[1]);
    return {x, z};
}
std::int32_t signed_key(std::uint32_t bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}
std::uint32_t key_bits(std::int32_t value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    return bits;
}
float magnitude_bits(float value) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof bits);
    bits &= 0x7fffffffu; //009ECBB6, after ratio FSTP32.
    std::memcpy(&value, &bits, sizeof value);
    return value;
}
std::int32_t clamp_key(std::int32_t value, std::int32_t first,
    std::int32_t last) noexcept {
    if (value < first) return first;
    return value > last ? last : value;
}
const AvoidZoneLayerGroup* select_group(const AvoidZoneTable& table, std::int32_t key) {
    const auto index = avoid_zone_group_for_layer_004120d0(table, key);
    return index < 0 ? nullptr : &table.groups[static_cast<std::size_t>(index)];
}
// A native singleton call PRECEDES each live key load. Reference arguments
// defer the read until after that callback; C++ argument evaluation order must
// not turn select_group(host.manager(), read_key()) into a premature read.
const AvoidZoneLayerGroup* select_live_group(ShipAiLayerSelectionHost& host,
    const std::uint32_t& key) {
    const auto& manager = host.manager_004218e0();
    return select_group(manager, signed_key(key));
}
std::int32_t next_live_layer(ShipAiLayerSelectionHost& host, const std::uint32_t& key) {
    const auto& manager = host.manager_004218e0();
    return avoid_zone_next_layer_004121b0(manager, signed_key(key));
}
std::int32_t previous_live_layer(ShipAiLayerSelectionHost& host, const std::uint32_t& key) {
    const auto& manager = host.manager_004218e0();
    return avoid_zone_previous_layer_00417d60(manager, signed_key(key));
}
const AvoidZoneLayerGroup& require_group(const AvoidZoneLayerGroup* group) {
    if (group == nullptr)
        throw std::domain_error("navigation layer query requires the selected native group");
    return *group;
}
bool contains(const AvoidZoneLayerGroup* group, const std::array<float, 2>& point) {
    return avoid_zone_first_containing_004178f0(require_group(group), point) >= 0;
}
std::array<float, 2> offset(const AvoidZoneLayerGroup* group,
    const std::array<float, 2>& point, ShipAiLayerSelectionHost& host) {
    const auto& actual = require_group(group);
    const auto native = host.native_group(actual);
    std::array<float, 2> out;
    avoid_zone_group_offset_00417b10(actual, native, out, point, 3.0f, 1);
    return out;
}
std::array<float, 2> goal(const ShipAiLayerSelectionView& v) noexcept {
    return {v.goal_x_1dc, v.goal_z_1e0};
}
} // namespace

std::uint32_t ship_ai_unit_navigation_layer_006dfd80(const ShipLeafTuning& tuning) noexcept {
    return tuning.array[0];
}
std::uint32_t ship_ai_submarine_navigation_layer_00852fd0(
    const ShipLeafTuning& tuning, std::int32_t depth_level_1268) {
    if (depth_level_1268 < 0 || depth_level_1268 >= 4)
        throw std::out_of_range("native submarine depthLevel must address class560..56C");
    return tuning.array[depth_level_1268];
}
std::int32_t avoid_zone_first_layer_00412170(const AvoidZoneTable& table) noexcept {
    return table.groups.empty() ? 0 : table.groups.front().layer_key;
}
std::int32_t avoid_zone_last_layer_00412180(const AvoidZoneTable& table) noexcept {
    return table.groups.empty() ? 0 : table.groups.back().layer_key;
}
std::int32_t avoid_zone_next_layer_004121b0(const AvoidZoneTable& table,
    std::int32_t key) noexcept {
    for (const auto& group : table.groups)
        if (group.layer_key > key) return group.layer_key;
    return avoid_zone_last_layer_00412180(table);
}
std::int32_t avoid_zone_previous_layer_00417d60(const AvoidZoneTable& table,
    std::int32_t key) noexcept {
    if (table.groups.empty()) return 0;
    std::size_t index = 0;
    while (index < table.groups.size() && table.groups[index].layer_key < key) ++index;
    return table.groups[index == 0 ? 0 : index - 1].layer_key;
}
void avoid_zone_layer_noop_004121a0(std::int32_t) noexcept {}

void avoid_zone_group_offset_00417b10(const AvoidZoneLayerGroup& group,
    const AvoidZoneClearanceGroupView& native, std::array<float, 2>& output,
    const std::array<float, 2>& point, float push, std::uint8_t test_containment) {
    if (native.count != group.zones.size() || (native.count != 0 && native.zones == nullptr))
        throw std::invalid_argument("navigation group must match actual native zone storage");
    output[0] = spill(point[0]); //00417B1C..26, before reading point[1].
    output[1] = spill(point[1]); //00417B29..30, including input/output identity.
    for (std::uint32_t i = 0; i < native.count; ++i) {
        if (native.zones[i] == nullptr)
            throw std::invalid_argument("native navigation group contains a null zone");
        // Same half-open bounds/contains/closest-offset composition as00417580;
        // each zone receives the result left by the preceding zone,00417BD2/6.
        output = avoid_zone_point_offset_00417580(*native.zones[i], group.zones[i],
            output, push, test_containment != 0);
    }
}

void ship_ai_select_navigation_layer_009eca20(ShipAiLayerSelectionView v,
    float seconds, ShipAiLayerSelectionHost& host) {
    if (!host.has_owner_3fc()) return; //009ECA2D..36; no later input is read.
    if (host.unit_group_leads_00778890()) //009ECA3C.
        v.requested_layer_308 = key_bits(host.group_navigation_layer_0070e450()); //51/6E.
    else
        v.requested_layer_308 = host.unit_navigation_layer_v214(); //66/6E.
    if (host.unit_is_kind_v5c(0x0c) && host.call_unit_v10c() != 0) //7B/8F.
        v.class_floor_16c = 0; //009ECA95, actual native store.
    else
        v.class_floor_16c = host.class_navigation_floor_0560(); //009ECAAD/B3.
    const auto first = avoid_zone_first_layer_00412170(host.manager_004218e0()); //B9/C0.
    const auto last = avoid_zone_last_layer_00412180(host.manager_004218e0()); //C7/CE.
    v.requested_layer_308 = key_bits(clamp_key(signed_key(v.requested_layer_308), first, last));
    v.timer_148 = subtract(v.timer_148, seconds); //009ECB09.
    v.timer_170 = subtract(v.timer_170, seconds); //009ECB17.
    v.timer_314 = subtract(v.timer_314, seconds); //009ECB2F.

    if (!x87_carry(0.0f, v.timer_148)) { //009ECB3B: unordered takes the other arm.
        const auto old_layer = signed_key(v.position_layer_164); //EBX at009ECB41.
        const auto upper_settings = host.settings_00424c40(); //009ECB47, retained EDI.
        const auto lower_settings = host.settings_00424c40(); //009ECB4E.
        const float high = spill(upper_settings.ship_max_200); //009ECB53.
        const float low = spill(lower_settings.ship_min_1fc); //009ECB65.
        const float draw = host.uniform_float_00bd2f10(1, low, high); //009ECB6E.
        v.timer_148 = add(draw, v.timer_148); //009ECB73..7F.
        const float speed = spill(host.unit_forward_speed_0092d730()); //009ECB8B/90.
        const float reference = host.unit_reference_speed_0080fc30(); //009ECBA2.
        const float fraction = magnitude_bits(divide(speed, reference)); //BA7..BBC.
        const float timer_scale = clamped_interpolate_00419010(0.1f, 2.0f, 0.6f, 1.0f,
            fraction); //009ECBEB, callee float-spills its result.
        v.timer_148 = multiply(timer_scale, v.timer_148); //009ECBF0..FC.

        std::array<float, 2> probe = v.hull.position_184; //009ECC02..18, before speed read.
        const float current_speed = spill(host.unit_forward_speed_0092d730()); //24/29.
        const bool reverse = x87_carry(current_speed, 0.0f); //009ECC38 JC includes unordered.
        const float ratio = reverse ? negate_divide(current_speed, v.reference_speed_3c4)
                                    : divide(current_speed, v.reference_speed_3c4);
        const float scale = reverse
            ? clamped_interpolate_00419010(0.1f, 0.0f, 0.4f, 1.0f, ratio) //009ECCBF.
            : clamped_interpolate_00419010(0.1f, 0.0f, 0.5f, 1.5f, ratio); //009ECC6D.
        const auto delta = difference(reverse ? v.hull.stern_17c : v.hull.bow_174,
            v.hull.position_184); //009ECC76..E4, two spills.
        const float dx = multiply(delta[0], scale); //009ECCF6.
        const float dz = multiply(delta[1], scale); //009ECCFE.
        probe[0] = add(dx, probe[0]); //009ECD0A.
        probe[1] = add(dz, probe[1]); //009ECD16.
        const float moved_sq = squared_length(difference(probe,
            {v.last_probe_x_158, v.last_probe_z_15c})); //009ECD46.
        if (x87_above(moved_sq, 1.0f)) { //009ECD54 skips unordered as well.
            v.last_probe_x_158 = probe[0]; //009ECD60.
            v.last_probe_z_15c = probe[1]; //009ECD6E.
            const auto minimum = avoid_zone_first_layer_00412170(host.manager_004218e0()); //D76/7D.
            const auto maximum = avoid_zone_last_layer_00412180(host.manager_004218e0()); //D84/8B.
            v.position_layer_164 = key_bits(clamp_key(signed_key(v.position_layer_164), minimum, maximum));
            const auto* group = select_live_group(host, v.position_layer_164); //DAA/DB8.
            if (contains(group, probe)) { //009ECDC4.
                do {
                    const auto next = previous_live_layer(host, v.position_layer_164); //009ECDD2/DE0.
                    const auto before = signed_key(v.position_layer_164);
                    v.position_layer_164 = key_bits(next); //009ECDED precedes stopping.
                    if (next >= before) break;
                    group = select_group(host.manager_004218e0(), next); //009ECDF5/DFD.
                } while (contains(group, probe)); //009ECE09.
            } else if (signed_key(v.position_layer_164) < signed_key(v.requested_layer_308)) {
                for (;;) { //Native initial bound is not retested after advancing.
                    const auto next = next_live_layer(host, v.position_layer_164); //009ECE31/3F.
                    if (next <= signed_key(v.position_layer_164)) break;
                    group = select_group(host.manager_004218e0(), next); //009ECE4E/56.
                    if (contains(group, probe)) break; //009ECE62.
                    v.position_layer_164 = key_bits(next); //009ECE70.
                }
            }
            if (old_layer != signed_key(v.position_layer_164)) goto select_travel; //009ECE12..1E.
        }
    } else if (!sse_carry(0.0f, v.timer_314)) { //009ECE80: positive/unordered skip goal arm.
        const int mode = static_cast<int>(v.mode_1c4);
        if (mode == 3 || mode == 2) {
            const auto old_layer = signed_key(v.goal_layer_310); //009ECEBF.
            const auto upper_settings = host.settings_00424c40(); //009ECEC5.
            const auto lower_settings = host.settings_00424c40(); //009ECECC.
            const float high = spill(upper_settings.move_max_1f8);
            const float low = spill(lower_settings.move_min_1f4);
            const float draw = host.uniform_float_00bd2f10(1, low, high); //009ECEEC.
            v.timer_314 = add(draw, v.timer_314); //009ECEF7.
            const std::array<float, 2> target{spill(v.goal_x_1dc), spill(v.goal_z_1e0)};
            const float moved_sq = squared_length(difference(target,
                {v.last_goal_x_31c, v.last_goal_z_320})); //009ECF3D.
            bool changed = false;
            if (x87_above(moved_sq, 1.0f)) {
                v.last_goal_x_31c = target[0]; //009ECF57.
                v.last_goal_z_320 = target[1]; //009ECF65.
                v.goal_layer_310 = v.requested_layer_308; //009ECF73.
                auto* group = select_live_group(host, v.goal_layer_310); //F79/F87.
                bool blocked = contains(group, target); //009ECF93.
                while (blocked) {
                    const auto previous = previous_live_layer(host, v.goal_layer_310); //009ECFA0/FAE.
                    if (previous >= signed_key(v.goal_layer_310)) break;
                    v.goal_layer_310 = key_bits(previous); //009ECFBB.
                    group = select_live_group(host, v.goal_layer_310); //FC1/FCF.
                    blocked = contains(group, target); //009ECFDB.
                }
                changed = old_layer != signed_key(v.goal_layer_310); //009ECFE4..EC.
            }
            if (signed_key(v.class_floor_16c) > signed_key(v.goal_layer_310))
                v.goal_layer_310 = v.class_floor_16c; //009ED000; does not change `changed`.
            if (changed) goto select_travel; //009ED008.
        } else {
            v.goal_layer_310 = v.position_layer_164; //009ECEA4.
            v.last_goal_z_320 = 1.0e10f; //009ECEAA,00CE4970 bits501502F9.
            v.last_goal_x_31c = 1.0e10f; //009ECEB2.
        }
    }
    if (sse_below_equal(0.0f, v.timer_170)) return; //009ED014 JBE also returns on unordered.

select_travel:
    const auto lower_settings = host.settings_00424c40(); //009ED01A.
    const float low = spill(lower_settings.travel_min_204); //009ED01F..25 before next lookup.
    const auto upper_settings = host.settings_00424c40(); //009ED029.
    const float high = spill(upper_settings.travel_max_208); //009ED02E..4C.
    const float draw = host.uniform_float_00bd2f10(1, low, high); //009ED04F.
    const auto requested = v.requested_layer_308; //009ED054 before timer store.
    v.timer_170 = spill(draw); //009ED05A.
    v.escaping_160 = false; //009ED060.
    v.travel_layer_30c = requested; //009ED067.
    host.manager_004218e0(); //009ED06D;004121A0 really does nothing.
    avoid_zone_layer_noop_004121a0(signed_key(v.travel_layer_30c)); //009ED07B.
    const auto goal_layer = signed_key(v.goal_layer_310); //ECX at009ED080.
    if (goal_layer < signed_key(v.travel_layer_30c)) {
        const float radius = spill(v.look_ahead_318);
        const float radius_sq = multiply(radius, radius); //009ED0A8.
        const float distance_sq = squared_length(difference(v.hull.position_184, goal(v)));
        if (x87_above(radius_sq, distance_sq)) { //009ED0E5; unordered goes to offset arm.
            v.travel_layer_30c = key_bits(goal_layer); //009ED0E7 uses retained ECX.
            host.manager_004218e0(); //009ED0ED.
            avoid_zone_layer_noop_004121a0(signed_key(v.travel_layer_30c)); //009ED0FB.
        } else {
            host.manager_004218e0(); //009ED105.
            avoid_zone_layer_noop_004121a0(signed_key(v.travel_layer_30c)); //009ED113.
            auto* group = select_live_group(host, v.travel_layer_30c); //D118/126.
            auto shifted = offset(group, goal(v), host); //009ED13F.
            float shifted_sq = squared_length(difference(shifted, v.hull.position_184));
            // Native stores radius_sq as double009ED16C for later iterations;
            // it was already binary32, so this preserves exactly the same value.
            while (x87_above(radius_sq, shifted_sq)) { //009ED18A /230, unordered stops.
                const auto previous = previous_live_layer(host, v.travel_layer_30c); //009ED190/19E.
                const auto floor = signed_key(v.goal_layer_310); //009ED1A3 before store.
                v.travel_layer_30c = key_bits(previous); //009ED1A9.
                if (previous <= floor) break;
                host.manager_004218e0(); //009ED1B5.
                avoid_zone_layer_noop_004121a0(signed_key(v.travel_layer_30c)); //009ED1C3.
                group = select_live_group(host, v.travel_layer_30c); //D1C8/1D6.
                shifted = offset(group, goal(v), host); //009ED1EF, reloads live goal.
                shifted_sq = squared_length(difference(shifted, v.hull.position_184));
            }
        }
    }
    if (signed_key(v.position_layer_164) >= signed_key(v.travel_layer_30c)) return; //009ED242.
    for (;;) {
        const auto next = next_live_layer(host, v.position_layer_164); //009ED250/25E, retained EDI.
        const auto* group = select_group(host.manager_004218e0(), next); //009ED265/26D.
        if (contains(group, v.hull.position_184)) { //009ED275.
            v.escaping_160 = true; //009ED296.
            group = select_group(host.manager_004218e0(), next); //009ED29D/2A5.
            const auto shifted = offset(group, v.hull.position_184, host); //009ED2BE.
            const auto delta = difference(shifted, v.hull.position_184); //009ED2C3..D8.
            const float length = spill(host.vector_length_00414c60(delta)); //009ED2DC/E1.
            v.escape_distance_14c = length; //009ED2E9.
            if (x87_above(1.0f, length)) { //009ED2F3 unordered goes to division.
                v.escape_distance_14c = 1.0f;
                v.escape_x_150 = 0.0f;
                v.escape_z_154 = 0.0f;
            } else {
                const float x = divide(delta[0], length); //009ED32E.
                const float z = divide(delta[1], length); //009ED336.
                v.escape_x_150 = spill(x); //009ED33E.
                v.escape_z_154 = spill(z); //009ED348.
            }
            if (next < signed_key(v.travel_layer_30c)
                || x87_above(v.escape_distance_14c, v.escape_limit_3c8))
                v.escape_distance_14c = spill(v.escape_limit_3c8); //009ED36E.
            break;
        }
        const bool continue_walk = next < signed_key(v.travel_layer_30c); //009ED283 before store.
        v.position_layer_164 = key_bits(next); //009ED289.
        if (!continue_walk) break;
    }
    const auto raw_requested = signed_key(v.requested_layer_308); //EDI009ED374.
    const auto* group = select_live_group(host, v.requested_layer_308); //D37A/388.
    auto ceiling = raw_requested;
    if (group != nullptr) {
        ceiling = group->layer_key; //009ED393.
        if (ceiling < raw_requested) {
            const auto& manager = host.manager_004218e0(); //009ED39A.
            ceiling = avoid_zone_next_layer_004121b0(manager, group->layer_key); //009ED3A5.
        }
    }
    const auto position = signed_key(v.position_layer_164);
    v.travel_layer_30c = key_bits(position < ceiling ? position : ceiling); //009ED3B8.
    host.manager_004218e0(); //009ED3BE.
    avoid_zone_layer_noop_004121a0(signed_key(v.travel_layer_30c)); //009ED3CC.
}
} // namespace bsp
