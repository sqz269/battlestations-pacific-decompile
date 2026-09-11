#include "bsp/hud_root_rows.hpp"

namespace bsp {
namespace {
std::uint32_t signed_index_as_unsigned(std::uint16_t index) noexcept {
    return index < 0x8000u ? index : (0xffff0000u | index);
}
std::uint16_t find_index(HudUnitListView list, std::uint32_t unit) noexcept {
    // The native signed BX comparison terminates when BX becomes 8000h.
    for (std::uint32_t i = 0; i < list.count && i < 0x8000u; ++i)
        if (list.units[i] == unit) return static_cast<std::uint16_t>(i);
    return 0xffff;
}
std::uint16_t next_index(std::uint16_t index, std::uint32_t count) noexcept {
    index = static_cast<std::uint16_t>(index + 1u);
    return signed_index_as_unsigned(index) >= count ? 0 : index;
}
std::uint16_t previous_index(std::uint16_t index, std::uint32_t count) noexcept {
    index = static_cast<std::uint16_t>(index - 1u);
    return index == 0xffffu ? static_cast<std::uint16_t>(count - 1u) : index;
}
}

HudUnitSelection hud_root_find_primary_unit(HudUnitListView primary,
    std::uint32_t unit, HudRootSelectionHost& host) {
    if (!unit) return {};
    if (host.unit_is_group_member(unit)) unit = host.group_leader(unit);
    return {false, find_index(primary, unit)};
}
HudUnitSelection hud_root_find_unit(HudUnitListView primary, HudUnitListView secondary,
    std::uint32_t unit, HudRootSelectionHost& host) {
    if (!unit) return {};
    if (host.unit_is_group_leader(unit)) return hud_root_find_primary_unit(primary, unit, host);
    if (!host.unit_is_group_member(unit)) return {};
    const auto index = find_index(secondary, unit);
    return {index != 0xffffu && secondary.count > 1, index};
}
void hud_root_poll_selection(HudRootSelectionState& state, HudUnitListView primary,
    HudUnitListView secondary, HudRootSelectionHost& host) {
    state.primary_previous = host.input_action_pressed(0x8d);
    state.primary_next = host.input_action_pressed(0x8c);
    state.secondary_previous = host.input_action_pressed(0x8e);
    state.secondary_next = host.input_action_pressed(0x8f);
    if (host.base_screen_active() && !host.panel_54_active() && host.controlled_unit()) {
        state.primary_previous = state.primary_next = false;
        state.secondary_previous = state.secondary_next = false;
    }
    const auto controlled = host.controlled_unit();
    if ((primary.count > 1 || (controlled && !host.unit_virtual_124(controlled))) &&
        (state.primary_next || state.primary_previous)) {
        if (state.selection.secondary)
            state.selection = hud_root_find_primary_unit(primary, host.selected_unit(), host);
        state.selection.index = state.primary_next
            ? next_index(state.selection.index, primary.count)
            : previous_index(state.selection.index, primary.count);
        state.selection.secondary = false;
        state.pending = true;
    }
    if (secondary.count <= 1 || state.pending) return;
    if (state.secondary_next) {
        state.selection.index = state.selection.secondary
            ? next_index(state.selection.index, secondary.count) : 1;
    } else if (state.secondary_previous) {
        state.selection.index = state.selection.secondary
            ? previous_index(state.selection.index, secondary.count)
            : static_cast<std::uint16_t>(secondary.count - 1u);
    } else return;
    if (state.selection.index == 0)
        state.selection = hud_root_find_primary_unit(primary, secondary.units[0], host);
    else state.selection.secondary = true;
    state.pending = true;
}

void hud_root_toggle_closed(HudClosedRowsState& state, HudClosedRowsHost& host) {
    if (state.closed == 0) {
        if (state.interface_id == 0x22) {
            const auto unit = host.controlled_unit();
            const auto member = host.first_member(unit);
            if (host.unit_is_kind(member, 0x10) || host.unit_is_kind(member, 0x16)) {
                state.closed = 1;
                host.push_interface_request(0x23, host.first_member(unit));
            }
        } else if (state.interface_id == 0x23) {
            const auto unit = host.controlled_unit();
            if (!host.unit_flag_379(unit)) {
                const auto owner = host.unit_owner_1a8(unit);
                if (owner == 9 || owner == host.local_team()) {
                    state.closed = 1;
                    host.push_interface_request(0x22, host.first_member(unit));
                }
            }
        }
    } else if (state.closed == 1) {
        state.closed = 0;
        if (state.restore_payload)
            host.push_interface_request(state.interface_id, state.restore_payload);
    }
}

float hud_root_healthy_width(long double health, const HudHealthTerms& terms) noexcept {
    const float first = static_cast<float>(terms.powerup_first * terms.scale_3c8);
    const float second = static_cast<float>(terms.powerup_second * terms.scale_3cc);
    const long double part_b = static_cast<long double>(terms.metric_939fb0) * terms.metric_939f80 / first;
    const long double part_a = static_cast<long double>(terms.metric_939fc0) * terms.metric_939f70 / second;
    return static_cast<float>(health - (part_b + part_a) / terms.divisor_36c);
}
std::uint32_t hud_root_command_icon_state(bool special, std::uint32_t descriptor) noexcept {
    if (special) return 2;
    switch (descriptor) {
    case 0x00e08f10: case 0x00e08f18: case 0x00e08f20: case 0x00e08f28:
    case 0x00e08f30: case 0x00e08f38: case 0x00e08f40: case 0x00e08f48:
    case 0x00e08f50: case 0x00e08f58: case 0x00e08f78: return 3;
    case 0x00e08f60: case 0x00e08f68: case 0x00e08f80: return 1;
    case 0x00e08f70: return 2;
    case 0x00e08fa0: return 4;
    default: return 0;
    }
}

namespace {
HudWeaponSource weapon_source(std::uint32_t unit, bool& special, HudRootRowsHost& host) {
    HudWeaponSource result;
    if (host.unit_is_kind(unit, 0x18) || host.unit_is_kind(unit, 0x0f)) {
        const auto weapon_unit = host.unit_is_kind(unit, 0x18) ? host.first_member(unit) : unit;
        if (!weapon_unit) return result;
        special = host.panel_byte(0x6c, 5) && host.panel_byte(0x6c, 0xbd);
        if (special) {
            if (host.weapon_probe(weapon_unit, HudWeaponProbe::Bomb))
                return {"ingame.selector_bomb", true, host.weapon_count(weapon_unit, HudWeaponCount::Payload)};
            if (host.weapon_probe(weapon_unit, HudWeaponProbe::Torpedo))
                return {"ingame.selector_torpedo", true, host.weapon_count(weapon_unit, HudWeaponCount::Payload)};
            if (host.weapon_probe(weapon_unit, HudWeaponProbe::DepthCharge))
                return {"ingame.selector_depthcharge", true, host.weapon_count(weapon_unit, HudWeaponCount::Payload)};
            if (host.weapon_probe(weapon_unit, HudWeaponProbe::Rocket))
                return {"ingame.selector_rocket", true, host.weapon_count(weapon_unit, HudWeaponCount::Rocket)};
        }
        result.key = host.unit_machinegun_byte(weapon_unit) ? "ingame.selector_machinegun" : ".";
    } else if (host.panel_byte(0x50, 5)) {
        switch (host.panel_50_weapon_mode()) {
        case 1: result.key = "ingame.selector_aagun"; break;
        case 2: result.key = "ingame.selector_aaflak"; break;
        case 3: result.key = "ingame.selector_atrillery"; break; // native spelling
        case 4:
            result.key = "ingame.selector_torpedo";
            if (host.unit_is_kind(unit, 6)) {
                result.counted = true;
                result.count = host.weapon_count(unit, HudWeaponCount::SubmarineTorpedo);
            }
            break;
        case 5: result.key = "ingame.selector_depthcharge"; break;
        default: break;
        }
    }
    return result;
}
void hide_weapon_info(HudRootRowsHost& host) {
    host.stop_weapon_animation(0);
    host.widget_scalar(HudRowWidget::WeaponInfo, 0.0f);
}
void update_health(std::uint32_t selected, std::uint32_t health_unit,
    HudRootRowState& state, HudRootRowsHost& host) {
    if (!health_unit) return;
    if (host.unit_is_kind(health_unit, 0x45)) {
        host.widget_width(HudRowWidget::HpDamage, static_cast<float>(host.kind_45_health(health_unit)), 1.0f);
        return;
    }
    if (!host.unit_is_kind(health_unit, 5)) return;
    host.widget_shown(HudRowWidget::HpDamage, true);
    host.widget_width(HudRowWidget::HpDamage, static_cast<float>(host.unit_health(health_unit)), 1.0f);
    if (!host.unit_is_kind(health_unit, 6)) {
        host.widget_width(HudRowWidget::HpHealthy, static_cast<float>(host.unit_health(health_unit)), 1.0f);
        host.widget_width(HudRowWidget::HpDamage, 0.0f, 1.0f);
        return;
    }
    HudHealthTerms terms;
    terms.metric_939f70 = host.health_metric(selected, 0x00939f70);
    terms.metric_939f80 = host.health_metric(selected, 0x00939f80);
    terms.metric_939fc0 = host.health_metric(selected, 0x00939fc0);
    terms.metric_939fb0 = host.health_metric(selected, 0x00939fb0);
    const auto mode = host.unit_field_a44(selected);
    if (mode == 4) terms.scale_3c8 = host.tuning_float(0x3c8);
    else if (mode == 3) terms.scale_3cc = host.tuning_float(0x3cc);
    terms.powerup_first = host.powerup_multiplier(3, selected);
    terms.powerup_second = host.powerup_multiplier(3, selected);
    terms.divisor_36c = host.unit_float_36c(selected);
    const auto width = hud_root_healthy_width(host.unit_health(health_unit), terms);
    if (terms.metric_939f70 == 0.0f && terms.metric_939f80 == 0.0f) {
        host.widget_width(HudRowWidget::HpHealthy, static_cast<float>(host.unit_health(health_unit)), 1.0f);
    } else {
        if (state.previous_healthy_width != width)
            host.widget_width(HudRowWidget::HpHealthy, width, 1.0f);
        state.previous_healthy_width = width;
    }
}
}

void hud_root_update_rows(HudRootRowState& state, HudRootRowsHost& host) {
    host.rebuild_unit_lists();
    auto unit = host.controlled_unit();
    if (!unit) unit = host.selected_unit();
    if (const auto override_unit = host.panel_74_override_unit()) unit = override_unit;
    if (unit) {
        if (host.unit_is_kind(unit, 0x18) && !host.first_member(unit)) return;
        if (!host.panel_byte(0x54, 5)) {
            bool special = false;
            const auto source = weapon_source(unit, special, host);
            if (unit != state.previous_unit || host.weapon_source_differs(source)) {
                if (host.unit_is_kind(unit, 0x45) || host.unit_is_kind(unit, 0x46)) {
                    hide_weapon_info(host);
                } else {
                    host.widget_scalar(HudRowWidget::WeaponInfo, 1.0f);
                    if (!special) {
                        const auto animation = host.weapon_animation(0);
                        host.animation_interval(animation, 0.0f, 1.0f);
                        host.animation_field_4(animation, 1.0f);
                    }
                }
            }
            host.set_weapon_source(source, true);
            host.cache_weapon_source(source);
        }
    }
    state.previous_unit = unit;
    std::uint32_t payload = 3;
    if (unit) {
        bool special = false;
        if (host.unit_is_kind(unit, 0x18)) special = host.special_command_state(host.first_member(unit), 1);
        else if (host.unit_is_kind(unit, 0x0f)) special = host.special_command_state(unit, 1);
        const auto descriptor = special ? 0 : host.command_descriptor(unit);
        host.widget_state(HudRowWidget::Command, hud_root_command_icon_state(special, descriptor), 0, 1.0f);
        host.unit_name_source(host.unit_name(unit), -1.0f, true);
        if (const auto controlled = host.controlled_unit()) {
            if (host.controlled_unit_country(controlled) != 0) {
                host.widget_shown(HudRowWidget::FlagUS, false);
                host.widget_shown(HudRowWidget::FlagJP, true);
            } else {
                host.widget_shown(HudRowWidget::FlagJP, false);
                host.widget_shown(HudRowWidget::FlagUS, true);
            }
        }
        if (host.unit_is_kind(unit, 0x1c)) payload = 0;
        else {
            const auto class_unit = host.unit_is_kind(unit, 0x18) ? host.first_member(unit) : unit;
            payload = host.class_payload_icon(host.unit_class(class_unit));
        }
        const auto health_unit = host.unit_is_kind(unit, 0x18) ? host.first_member(unit) : unit;
        update_health(unit, health_unit, state, host);
    }
    host.widget_state(HudRowWidget::Payload, payload, 0, 1.0f);
    if (host.panel_byte(0x54, 4)) hide_weapon_info(host);
}
}
