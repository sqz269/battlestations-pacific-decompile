// Packet hud_central_updates, docs/HUD_CENTRAL_UPDATES.md.
//
// The update virtuals of the three central in-mission HUD screens. Every
// constant and every ordering decision below is taken from the disassembly of
// 00649860, 006435D0 and 005C0F20; the decompiler output of all three carries
// register inputs and could not be used directly.
#include "bsp/hud_updates.hpp"

#include <cmath>

namespace bsp {
namespace {

// 00649BC5 and 00649BF4 both index the per-team power-up table by the record id
// held at team_record+28h. Kept here so both loops of the HUD root spell the
// stride once.
constexpr std::size_t team_record_byte_offset(std::size_t field, int record_id) noexcept {
    return field + static_cast<std::size_t>(record_id) * kHudRootTeamRecordStride;
}

} // namespace

// ---------------------------------------------------------------------------
// Slot 44h, the HUD root update (00649860)
// ---------------------------------------------------------------------------

float hud_root_first_row_y(bool platform_inset) noexcept {
    // 006499E3 zeroes the accumulator, 006499EE overwrites it with 00CF5C08
    // when the platform byte is set.
    return platform_inset ? kHudRootFirstRowOffset : 0.0f;
}

float hud_root_advance_row_y(float previous_y, float template_height) noexcept {
    // FLD [size+4]; FADD qword 00CEBDD0; FADD accumulator; FSTP accumulator.
    // The height and the gap are summed before the accumulator is added, so the
    // rounding of the native sequence is reproduced by this grouping.
    return (template_height + kHudRootRowGap) + previous_y;
}

HudGuiPoint hud_root_icon_position(float row_y) noexcept {
    // 00649B42: x = 00CE8190 + 00CF5C00, y = the accumulator, z = 00CE65D8.
    HudGuiPoint p;
    p.x = kHudRootColumnAnchor + kHudRootColumnXOffset;
    p.y = row_y;
    p.z = kHudRootIconDepth;
    return p;
}

HudGuiPoint hud_root_circle_position() noexcept {
    // 00649ED9: the same x, y = 00CE8190 - 00CF0DE8, z = 00CF5BFC. The circle
    // does not follow the row accumulator.
    HudGuiPoint p;
    p.x = kHudRootColumnAnchor + kHudRootColumnXOffset;
    p.y = kHudRootColumnAnchor - kHudRootCircleYOffset;
    p.z = kHudRootCircleDepth;
    return p;
}

bool hud_root_timed_entry_active(float deadline, float mission_clock) noexcept {
    // 00649C71: FLD deadline; FSUB clock; FLDZ; FCOMIP; JBE skips the entry, so
    // the test is strictly greater than zero.
    return (deadline - mission_clock) > 0.0f;
}

float hud_root_circle_fill(float deadline, float mission_clock, float duration) noexcept {
    // 00649F8C: FLD deadline; FSUB clock; FDIV [record+7Ch].
    if (duration == 0.0f) {
        return 0.0f;
    }
    return (deadline - mission_clock) / duration;
}

void hud_root_screen_update(HudRootUpdateState& state, HudRootUpdateHost& host,
                            float seconds) {
    // 00649863..00649898. The body runs only when neither suppression byte is
    // set and only on the frame the counter reaches zero or below.
    bool run_body = false;
    if (!host.game_hud_suppressed()) {
        state.update_countdown -= 1;
        if (state.update_countdown < 1) {
            state.update_countdown = kHudRootUpdateInterval;
            run_body = true;
        }
    }

    if (run_body) {
        // 006498AC..006499CC: both clone vectors are destroyed and emptied
        // before anything is rebuilt.
        host.release_icon_clones();
        host.release_circle_clones();

        const bool inset = host.platform_row_inset();
        float row_y = hud_root_first_row_y(inset);

        if (host.controlled_unit_present()) {
            host.collect_powerups();

            // 00649A3D..00649C21, the untimed entries at +164h.
            const std::size_t untimed = host.untimed_entry_count();
            for (std::size_t i = 0; i < untimed; ++i) {
                if (!host.untimed_entry_applies(i)) {
                    continue;
                }
                const std::uint32_t clone = host.clone_icon_template();
                host.widget_set_shown(clone, true);
                row_y = hud_root_advance_row_y(row_y, host.icon_template_height());
                host.widget_set_resolved_position(clone, hud_root_icon_position(row_y));
                host.icon_add_state_from_entry(clone, i, false);
                host.widget_select_state(clone);
            }

            // 00649C28..00649FAC, the timed entries at +124h with their
            // deadlines at +154h.
            const std::size_t timed = host.timed_entry_count();
            for (std::size_t i = 0; i < timed; ++i) {
                const float deadline = host.timed_entry_deadline(i);
                const float clock = host.mission_clock();
                if (!hud_root_timed_entry_active(deadline, clock)) {
                    // Deadline already reached: the entry is skipped entirely.
                    continue;
                }
                if (!host.timed_entry_applies(i)) {
                    continue;
                }
                const std::uint32_t icon = host.clone_icon_template();
                host.widget_set_shown(icon, true);
                row_y = hud_root_advance_row_y(row_y, host.icon_template_height());
                host.widget_set_resolved_position(icon, hud_root_icon_position(row_y));
                host.icon_add_state_from_entry(icon, i, true);
                host.widget_select_state(icon);

                const std::uint32_t circle = host.clone_circle_template();
                host.widget_set_shown(circle, true);
                host.widget_set_resolved_position(circle, hud_root_circle_position());
                host.circle_set_fill(circle,
                                     hud_root_circle_fill(deadline, clock,
                                                          host.timed_entry_duration(i)));
            }
        }
    }

    // 00649FB3 onwards runs on every call, gated body or not.
    if (state.ticker_timer > 0.0f) {
        state.ticker_timer -= seconds;
    }
    if (state.ticker_timer <= 0.0f && state.ticker_queue_count != 0) {
        host.advance_ticker();
        state.ticker_timer = kHudRootTickerInterval;
    }

    // 0064A00E..0064A101, the pending unit handle at +78h.
    if (state.pending_unit_handle != 0) {
        if (host.pending_unit_selectable(state.pending_unit_handle)) {
            if (host.pending_unit_allowed()) {
                host.commit_pending_unit();
                if (!host.controlled_unit_present()) {
                    if (host.interface_manager_idle()) {
                        host.push_interface_request(kHudRootRequestLimbo, 0);
                    }
                } else {
                    host.notify_camera_and_input();
                    host.push_interface_request(kHudRootRequestScene,
                                                host.controlled_unit_scene_payload());
                }
            }
            state.pending_unit_handle = 0;
        }
    }

    // 0064A108..0064A160.
    host.widget_set_shown(state.selector_widget, true);
    bool needs_fallback = false;
    const std::uint32_t resolved = host.resolve_weapon_info(needs_fallback);
    state.weapon_info_field = needs_fallback ? host.resolve_weapon_info_fallback() : resolved;
    host.update_unit_rows(seconds);
    host.update_medals();

    // 0064A165..0064A205, the closed-HUD toggle.
    if (host.controlled_unit_present() && !host.game_blocks_toggle() &&
        !host.controlled_unit_is_kind(9) && !host.controlled_unit_is_kind(0x45) &&
        !host.controlled_unit_is_kind(0x46)) {
        const bool pressed = host.input_action_pressed(kHudRootToggleInputAction);
        bool toggle = false;
        if (host.controlled_unit_is_kind(0x18) && host.controlled_unit_flag_379()) {
            // 0064A1E9: with the closed-HUD field set the press is swallowed.
            toggle = !state.closed_hud_flag;
        } else {
            toggle = pressed;
        }
        if (toggle) {
            host.toggle_closed_hud();
        }
    }

    // 0064A20B..0064A241, the group-change tail.
    if (state.pending_group_request && host.game_has_group_manager() &&
        host.controlled_unit_present()) {
        const int team = host.controlled_unit_team();
        if (team < 8 && team != host.local_team_index()) {
            host.rebuild_group_rows();
            state.pending_group_request = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Slot 4Dh, the world markers update (006435D0)
// ---------------------------------------------------------------------------

HudMarkerClipRect hud_marker_clip_rect(float gui_width, float gui_height) noexcept {
    // 0064361B: both extents are scaled by 00CF5990 first, then the rectangle
    // is centred on 00D7A280.
    const float half_w = gui_width * kHudMarkerClipHalfExtent;
    const float half_h = gui_height * kHudMarkerClipHalfExtent;
    HudMarkerClipRect rect;
    rect.left = kHudMarkerClipCentre - half_w;   // 00E197D0 and 00E197D8
    rect.right = kHudMarkerClipCentre + half_w;  // 00E197CC and 00E197D4
    rect.top = kHudMarkerClipCentre - half_h;    // 00E197C8
    rect.bottom = kHudMarkerClipCentre + half_h; // 00E197C4
    return rect;
}

bool hud_marker_within_radius(float delta_x, float delta_y, float delta_z, int radius) noexcept {
    // 00643817..00643853. FILD converts the radius from the integer at +7C4h,
    // the sum is built as (dx*dx + dy*dy) + dz*dz and the comparison is
    // radius*radius against the squared distance with JBE skipping.
    float rounded;
    std::uint8_t inside;
    __asm {
        fild radius
        fstp rounded
        fld delta_x
        fld delta_y
        fld delta_z
        fld rounded
        fld st(2)
        fmulp st(3),st(0)
        fld st(3)
        fmulp st(4),st(0)
        fxch st(2)
        faddp st(3),st(0)
        fmul st(0),st(0)
        faddp st(2),st(0)
        fxch st(1)
        fstp rounded
        fld rounded
        fld st(1)
        fmulp st(2),st(0)
        fxch st(1)
        fstp rounded
        fld rounded
        fcomip st(0),st(1)
        fstp st(0)
        seta inside
    }
    return inside != 0;
}

namespace {
float marker_axis_difference(float controlled, float member) noexcept {
    float result;
    __asm {
        fld controlled
        fsub member
        fstp result
    }
    return result;
}
}

bool hud_markers_screen_update(HudMarkersUpdateState& state,
                               HudMarkersUpdateHost& host, float seconds) {
    (void)seconds; // 006435D0 never reads its float argument.

    // 006435D8..0064360C.
    const int team = host.local_team_index();
    if (team < 0 || team > 7 || host.game_hud_suppressed()) {
        return false;
    }

    // 00643612..0064367F.
    float width = 0.0f;
    float height = 0.0f;
    host.gui_extent(width, height);
    host.publish_clip_rect(hud_marker_clip_rect(width, height));

    // 00643685..006436CC.
    state.pool_flag = 0;
    state.camera_unit = host.camera_unit();
    host.reset_marker_pool();
    host.clear_marker_set();
    state.self_marker_unit = host.controlled_unit_present() ? host.self_marker_unit() : 0;
    host.refresh_screen_state();
    host.refresh_marker_layout();

    if (state.markers_disabled) {
        return true;
    }

    // 006436DB, the controlled unit's own marker.
    host.add_marker(state.self_marker_unit, 0, 0, kHudMarkerKindSelf);
    host.note_marker(state.self_marker_unit);

    // 006436FD..0064377C, the interface manager's current target.
    bool target_marked = false;
    const std::uint32_t target = host.interface_target_unit();
    if (state.camera_unit != 0 && target != 0 && target != state.self_marker_unit &&
        host.unit_team_id(target) == host.local_team_record_id() &&
        host.unit_passes_target_filter(target) && host.target_is_selectable(target)) {
        host.add_marker(target, kHudMarkerKindTarget, 0, 0);
        host.note_marker(target);
        target_marked = true;
    }

    // 00643781..00643880, the squad members within their own radius.
    if (host.controlled_unit_present()) {
        for (const void* node = host.squad_first_node(); node;
             node = host.squad_next_node(node)) {
            const std::uint32_t mate = host.squad_node_unit(node);
            auto& member_pose = host.unit_pose(mate);
            if (member_pose.world_valid_c8 == 0) refresh_pose_00414db0(member_pose);
            const auto controlled = host.controlled_unit();
            auto& controlled_pose = host.unit_pose(controlled);
            if (controlled_pose.world_valid_c8 == 0) refresh_pose_00414db0(controlled_pose);
            // Both poses are refreshed before the native x/y/z reads. Keep
            // references across refresh, including an exact shared-pose alias.
            const float dx = marker_axis_difference(controlled_pose.world_cc[12], member_pose.world_cc[12]);
            const float dy = marker_axis_difference(controlled_pose.world_cc[13], member_pose.world_cc[13]);
            const float dz = marker_axis_difference(controlled_pose.world_cc[14], member_pose.world_cc[14]);
            if (hud_marker_within_radius(dx, dy, dz, host.squad_marker_radius(mate))) {
                host.add_marker(mate, 0, 0, kHudMarkerKindSelf);
                host.note_marker(mate);
            }
        }
    }

    // 00643886..0064392A, the screen-centre world pick.
    int vw = 0;
    int vh = 0;
    host.viewport_size(vw, vh);
    float wx = 0.0f;
    float wy = 0.0f;
    float wz = 0.0f;
    const bool picked = host.pick_world_point(state.crosshair_x * static_cast<float>(vw),
                                              state.crosshair_y * static_cast<float>(vh),
                                              wx, wy, wz);
    host.publish_crosshair_point(picked, wx, wy, wz);

    // 00643931..00643A9F and 00643A9F..00643BB9.
    host.sweep_objectives();
    host.sweep_command_units();

    // 00643BB9..00643C8C, the target group.
    state.target_unit = 0;
    if (host.controlled_unit_present() && !target_marked) {
        const std::uint32_t queried = host.query_target_unit();
        state.target_unit = queried;
        if (queried != 0) {
            if (host.unit_is_group_leader(queried)) {
                const std::size_t count = host.group_member_count(queried);
                for (std::size_t i = 0; i < count; ++i) {
                    const std::uint32_t member =
                        i < static_cast<std::size_t>(kHudMarkerGroupMemberLimit)
                            ? host.group_member(queried, i)
                            : 0u;
                    if (member != queried) {
                        host.add_group_marker(member, 1);
                        host.note_marker(member);
                    }
                }
            }
            host.add_group_marker(queried, 0);
            host.note_marker(queried);
        }
    }

    // 00643C8C..00643D3F, the reinforcement sweep behind the detail gate.
    if (state.camera_unit != 0 &&
        host.objective_detail_level() > kHudMarkerObjectiveThreshold) {
        host.sweep_reinforcements();
    }

    host.flush_markers();
    return true;
}

// ---------------------------------------------------------------------------
// Slot 35h, the minimap update (005C0F20)
// ---------------------------------------------------------------------------

float hud_minimap_pulse(float mission_clock) noexcept {
    // 005C0F9E: FLD 00F876A4; FMUL qword 00D7A328; FSIN.
    return std::sin(mission_clock * kHudMinimapPulseRate);
}

float hud_minimap_pulse_alpha(float pulse) noexcept {
    // 005C0FC4: FLD 00D7A280; FMUL ST1 leaves pulse*C; FADDP adds C.
    return kHudMinimapPulseCentre * pulse + kHudMinimapPulseCentre;
}

float hud_minimap_intro_ramp(float animation_clock) noexcept {
    // 005C102D: the clock is scaled by 00D7A328, then FCOMIP against 1.0 keeps
    // the smaller of the two.
    const float scaled = animation_clock * kHudMinimapRampRate;
    return scaled >= 1.0f ? 1.0f : scaled;
}

float hud_minimap_pulse_scale(float ramp, float pulse) noexcept {
    // 005C101D builds 0.1*pulse + 1.0, 005C1067 multiplies it by the ramp.
    return ramp * (pulse * kHudMinimapPulseScale + kHudMinimapPulseBase);
}

HudGuiPoint hud_minimap_icon_position(float world_dx, float world_dz,
                                      float depth) noexcept {
    // 005C1C61: x is multiplied by 00CEDAE8 (1/1024) and y is negated and
    // divided by 00CE42B0 (768), so the two axes do not share a scale.
    HudGuiPoint p;
    p.x = world_dx / kHudMinimapXDivisor;
    p.y = -world_dz / kHudMinimapYDivisor;
    p.z = depth;
    return p;
}

float hud_minimap_icon_rotation(float heading) noexcept {
    // 005C1CBC: FSUBR qword 00CE3830 turns the accumulated heading into pi/2
    // minus that heading.
    return kHudMinimapHeadingBias - heading;
}

bool hud_minimap_within_map(float squared_distance, float half_extent) noexcept {
    // 005C15A0 squares the extent once; 005C16D3 compares it with the squared
    // distance and JC drops the icon when the extent is the smaller value.
    return (half_extent * half_extent) >= squared_distance;
}

void hud_minimap_update_animation(HudMinimapUpdateState& state,
                                  HudMinimapUpdateHost& host, float seconds) {
    host.refresh_transforms(); // 005C0F3F, unconditional.

    bool animate = false;
    if (state.selected_capture != 0) {
        // 005C0F59: a capture point is selected, so the group is shown and the
        // fade latch is armed for the next release.
        state.fade_latch = false;
        host.widget_set_shown(state.capture_group, true);
        state.animation_clock += seconds;
        host.widget_select_state(state.capture_widget,
                                 host.capture_icon_state(state.selected_capture));
        animate = true;
    } else if (host.widget_query(state.capture_group)) {
        // 005C1289: nothing selected but the group is still up, so the same
        // animation runs once from a clock restarted on the first such frame.
        if (!state.fade_latch) {
            state.animation_clock = 0.0f;
            state.fade_latch = true;
        }
        state.animation_clock += seconds;
        animate = true;
    }

    if (!animate) {
        return;
    }

    const float pulse = hud_minimap_pulse(host.mission_clock());
    host.widget_set_color(state.capture_widget, 1.0f, 1.0f, 1.0f,
                          hud_minimap_pulse_alpha(pulse));

    const float ramp = hud_minimap_intro_ramp(state.animation_clock);
    const float scale = hud_minimap_pulse_scale(ramp, pulse);
    const float ring = host.capture_ring_factor(state.selected_capture);
    host.widget_set_scale(state.capture_widget, scale, scale);
    host.widget_set_scale(state.capture_ring_a, ring, ring);
    host.widget_set_scale(state.capture_ring_b, ring, ring);
    host.widget_set_shown(state.marker_widget, true);
}

std::size_t hud_minimap_update_unit_icons(HudMinimapUpdateState& state,
                                          HudMinimapUpdateHost& host) {
    (void)state;

    // 005C154E..005C1578: the same team gate as the markers screen, plus a
    // camera unit.
    const int team = host.local_team_index();
    const std::uint32_t camera = host.camera_unit();
    if (team < 0 || team > 7 || camera == 0) {
        return 0;
    }

    float extent_x = 0.0f;
    float extent_z = 0.0f;
    host.map_half_extents(extent_x, extent_z);

    host.refresh_unit_pose(camera);
    float cx = 0.0f;
    float cy = 0.0f;
    float cz = 0.0f;
    host.unit_position(camera, cx, cy, cz);
    const std::uint32_t self_unit = host.self_marker_unit();

    std::size_t placed = 0;
    const std::size_t count = host.team_unit_count();
    for (std::size_t i = 0; i < count; ++i) {
        const std::uint32_t unit = host.team_unit(i);
        if (unit == camera || !host.unit_drawable(unit)) {
            continue;
        }
        if (unit != self_unit) {
            // 005C1685: the cull uses the icon-space offset, not the world one.
            float dx = 0.0f;
            float dy = 0.0f;
            float dz = 0.0f;
            host.unit_icon_offset(unit, dx, dy, dz);
            if (!hud_minimap_within_map(host.squared_length(dx, dy, dz), extent_x)) {
                continue;
            }
        }
        if (host.icon_missing(unit)) {
            host.create_icon(unit);
        }
        host.attach_icon(unit);
        ++placed;
    }
    return placed;
}

} // namespace bsp
