// Gun platform traverse arcs and the gun's fire-request chain.
// Packet cc2_gun_platform_arc. See docs/GUN_PLATFORM_ARC.md for the evidence.

#include "bsp/gun_platform_arc.hpp"

#include <cmath>

namespace bsp {
namespace {

// The traverse-bit test the three window walks share, on top of the bound test
// gun_arc_contains_007f5fc0 already models (007F5FC0, 007F6840, 007F6530).
bool traverse_window(const GunFiringArc& arc, float horz, float vert) noexcept {
    return gun_arc_contains_007f5fc0(arc, horz, vert) && (arc.flags & kGunArcFlagTraverse) != 0;
}

// The horizontal-only form 007F6530's two walks and 0085B0F0's search use. The
// flag test is separate there, so it is not folded in.
bool traverse_window_horizontal(const GunFiringArc& arc, float horz) noexcept {
    return gun_arc_contains_horizontal_007f5960(arc, horz) &&
           (arc.flags & kGunArcFlagTraverse) != 0;
}

// 007F6B10's bound clamp, 00D08BA0/00D08B98 against 00D08BA8/00D08BAC.
float clamp_arc_bound(float radians) noexcept {
    if (!(-kGunArcBoundLimit <= radians)) {
        return -kGunArcBoundLimit;
    }
    if (kGunArcBoundLimit < radians) {
        return kGunArcBoundLimit;
    }
    return radians;
}

// The circular step 007F6530 takes through the window list.
std::size_t advance(std::size_t index, int direction, std::size_t count) noexcept {
    if (direction < 0) {
        return index == 0 ? count - 1 : index - 1;
    }
    std::size_t next = index + 1;
    return next == count ? 0 : next;
}

} // namespace

std::ptrdiff_t gun_find_traverse_window_007f6840(const GunPlatformArcs& arcs,
                                                 float horz,
                                                 float vert) noexcept {
    if (arcs.first == nullptr) {
        return -1;
    }
    for (std::size_t i = 0; i < arcs.count; ++i) {
        if (traverse_window(arcs.first[i], horz, vert)) {
            return static_cast<std::ptrdiff_t>(i);
        }
    }
    return -1;
}

GunArcRouteOutcome gun_arc_route_deltas_007f6530(const GunPlatformArcs& arcs,
                                                 float horz,
                                                 float vert,
                                                 float target_horz,
                                                 float target_vert) noexcept {
    GunArcRouteOutcome out{};
    // 007F6547 and 007F6564: both out parameters are written before any window
    // is looked at, so an unroutable gun still gets the shortest-path deltas.
    out.deltas.horz = gun_wrap_angle_0085abbd(target_horz - horz);
    out.deltas.vert = gun_wrap_angle_0085abbd(target_vert - vert);

    if (arcs.first == nullptr || arcs.count == 0) {
        return out;
    }

    // 007F65A5-007F6608: the window the gun is traversing in right now,
    // horizontal bounds only.
    std::size_t current = 0;
    bool found = false;
    for (std::size_t i = 0; i < arcs.count; ++i) {
        if (traverse_window_horizontal(arcs.first[i], horz)) {
            current = i;
            found = true;
            break;
        }
    }
    if (!found) {
        return out; // 007F6617's early return
    }
    out.current_window_known = true;

    // 007F6621: the direction of travel is the sign of the horizontal delta.
    int direction = out.deltas.horz < 0.0f ? -1 : 1;

    // 007F664E and 007F673F: walk that way until a window holds the target or a
    // window forbids traversal.
    std::size_t stop = current;
    bool stopped = false;
    for (;;) {
        const GunFiringArc& arc = arcs.first[stop];
        if (gun_arc_contains_horizontal_007f5960(arc, target_horz) ||
            (arc.flags & kGunArcFlagTraverse) == 0) {
            stopped = true;
            break;
        }
        stop = advance(stop, direction, arcs.count);
        if (stop == current) {
            break; // a full circle, 007F66B1's fall-through
        }
    }
    if (!stopped) {
        return out;
    }

    // 007F6694: a blocked window sends the gun the other way round. The native
    // code computes -0.0f - delta (00D7A208), which is a negation.
    if ((arcs.first[stop].flags & kGunArcFlagTraverse) == 0) {
        out.deltas.horz = -out.deltas.horz;
        direction = -direction;
        out.routed_around = true;
    }
    if (stop == current) {
        return out; // 007F66B3
    }

    // 007F66B9-007F681B: every window on the chosen path must hold the wanted
    // elevation; the first that does not clamps the vertical delta to its bound.
    const float wanted_vert = vert + out.deltas.vert;
    std::size_t walk = current;
    for (;;) {
        const GunFiringArc& arc = arcs.first[walk];
        if (wanted_vert + kGunAimArcEpsilon < arc.min_vert ||
            arc.max_vert < wanted_vert - kGunAimArcEpsilon) {
            out.deltas.vert = wanted_vert < arc.min_vert
                                  ? gun_wrap_angle_0085abbd(arc.min_vert - vert)
                                  : gun_wrap_angle_0085abbd(arc.max_vert - vert);
            out.vertical_clamped = true;
            return out;
        }
        walk = advance(walk, direction, arcs.count);
        if (walk == stop) {
            return out;
        }
    }
}

bool gun_snap_angle_into_window_0085b0f0(const GunPlatformArcs& arcs,
                                         float angle,
                                         float& accepted) noexcept {
    // 0085B2CF, 0085B38A, 0085B441: the same horizontal search three times, on
    // the angle itself and then half a degree either side of it.
    const float candidates[3] = {
        angle,
        gun_wrap_angle_0085abbd(angle + kGunAimArcEpsilon),
        gun_wrap_angle_0085abbd(angle - kGunAimArcEpsilon),
    };
    for (float candidate : candidates) {
        for (std::size_t i = 0; arcs.first != nullptr && i < arcs.count; ++i) {
            if (traverse_window_horizontal(arcs.first[i], candidate)) {
                accepted = candidate;
                return true;
            }
        }
    }
    return false;
}

void gun_split_insert_arc_007f5a10(std::vector<GunFiringArc>& windows,
                                   const GunFiringArc& arc) {
    // 007F5A1C onward: the last window whose horizontal bounds hold both of
    // the new window's bounds. The native routine dereferences the result with
    // no null guard; refusing is the one deliberate divergence here.
    std::size_t hit = 0;
    bool found = false;
    for (std::size_t i = 0; i < windows.size(); ++i) {
        if (gun_arc_contains_horizontal_007f5960(windows[i], arc.min_horz) &&
            gun_arc_contains_horizontal_007f5960(windows[i], arc.max_horz)) {
            hit = i;
            found = true;
        }
    }
    if (!found) {
        return;
    }

    const GunFiringArc saved = windows[hit];
    std::size_t insert_at = hit + 1;

    // 007F5B96: the left remainder, kept only when it is wider than half a degree.
    if (std::fabs(gun_wrap_angle_0085abbd(saved.min_horz - arc.min_horz)) >=
        kGunAimArcEpsilon) {
        windows[hit].max_horz = arc.min_horz;
        windows.insert(windows.begin() + static_cast<std::ptrdiff_t>(insert_at), arc);
        insert_at += 1;
    } else {
        windows[hit] = arc;
    }

    // 007F5DA8: the right remainder carries the old window's flags and elevation.
    if (std::fabs(gun_wrap_angle_0085abbd(saved.max_horz - arc.max_horz)) >=
        kGunAimArcEpsilon) {
        GunFiringArc tail = saved;
        tail.min_horz = arc.max_horz;
        tail.max_horz = saved.max_horz;
        windows.insert(windows.begin() + static_cast<std::ptrdiff_t>(insert_at), tail);
    }
}

void gun_add_authored_arc_007f6b10(std::vector<GunFiringArc>& windows, GunFiringArc arc) {
    // 007F6B1F onward: an all-zero record is not a window and is dropped.
    if (arc.min_horz == 0.0f && arc.max_horz == 0.0f && arc.min_vert == 0.0f &&
        arc.max_vert == 0.0f) {
        return;
    }

    // 007F6BA0 area: a span narrower than 0.01 rad is padded by 0.01 rad each side
    // and both bounds are re-wrapped through 00605070.
    float span = arc.min_horz - arc.max_horz;
    if (span <= 0.0f) {
        span = -span;
    }
    if (span < kGunArcMinimumSpan) {
        arc.min_horz = gun_wrap_angle_0085abbd(arc.min_horz - kGunArcSpanPadding);
        arc.max_horz = gun_wrap_angle_0085abbd(arc.max_horz + kGunArcSpanPadding);
    }

    // the clamp between the 00605070 calls and 007F6C4B: pull both bounds a shade inside +-pi.
    arc.min_horz = clamp_arc_bound(arc.min_horz);
    arc.max_horz = clamp_arc_bound(arc.max_horz);

    // 007F6C4B: a window that straddles the seam becomes two.
    if (arc.max_horz < arc.min_horz) {
        const float saved_max = arc.max_horz;
        arc.max_horz = kGunArcBoundLimit;
        gun_split_insert_arc_007f5a10(windows, arc);
        arc.min_horz = -kGunArcBoundLimit;
        arc.max_horz = saved_max;
    }
    gun_split_insert_arc_007f5a10(windows, arc);
}

bool gun_set_fire_request_0072d2c0(GunFireRequestState& state,
                                   GunFireRequestHost& host,
                                   bool want_fire) {
    // 0072D2DC-0072D2EC: a detached or suppressed gun cannot want to fire.
    bool want = want_fire;
    if (!host.has_owning_unit() || host.unit_suppressed() || host.gun_suppressed()) {
        want = false;
    }

    // 0072D311: dropping the request drops the observer pair on the fire target.
    if (!want && state.has_target_ref) {
        host.release_fire_target_ref_006952a0();
        state.has_target_ref = false;
        state.target_network_id = 0;
    }

    // 0072D31E: the latch comparison; everything below it is edge-triggered.
    if (state.fire_requested != want) {
        if (host.is_rapid_fixed_slave_006e3d50(kGunClassIdRapidFixedSlave)) {
            host.send_fixed_slave_fire_message_0077c7b0(want);
        }
        state.fire_requested = want;
        if (want) {
            state.fire_stagger = host.random_stagger_00bd2f10(0.0f, kGunArcFireStaggerMax);
        } else {
            host.stop_firing_0072b4c0();
        }
    }
    return state.fire_requested;
}

bool gun_fixed_step_tick_0072d130(GunFireRequestState& state,
                                  GunFireRequestHost& host,
                                  float dt) {
    // 0072D14D: the reference at gun+41Ch is released on the step that takes
    // its count at gun+420h to zero.
    if (state.effect_ref_count > 0) {
        state.effect_ref_count -= 1;
        if (state.effect_ref_count == 0 && state.holds_effect_ref) {
            host.release_effect_ref();
            state.holds_effect_ref = false;
        }
    }

    host.base_tick_0072ad40(dt);
    state.fire_stagger -= dt;
    state.barrel_delay_time -= dt;

    // 0072D1C1-0072D1D9: three gates, all of which must be clear.
    if (host.unit_fire_blocked() || host.gun_disabled() || host.gun_suppressed()) {
        return false;
    }

    // 0072D1FA: only a timer that has not already gone negative is advanced.
    for (std::size_t i = 0; i < state.barrel_timers.size(); ++i) {
        if (state.barrel_timers[i] >= 0.0f) {
            const float next = state.barrel_timers[i] - dt;
            host.set_barrel_reload_timer_0072cf00(static_cast<int>(i), next);
            state.barrel_timers[i] = next; // the store lives inside 0072CF00
        }
    }

    // 0072D22F: the fire latch turns into one opcode 0ADh message per step.
    if (!state.fire_requested) {
        return false;
    }
    host.send_fire_message_0ad(state.has_target_ref ? state.target_network_id
                                                    : static_cast<std::uint16_t>(0));
    return true;
}

void gun_ring_muzzle_origin_006fe160(const GunRingOffsetInputs& in, float out[3]) noexcept {
    // 006FE26A: the native routine divides by the muzzle node count without a
    // guard. A count of zero cannot reach it because the descriptor always has
    // at least one node; the guard here keeps the model total.
    const float angle = in.muzzle_count > 0
                            ? (static_cast<float>(in.barrel_index) * kGunAimTwoPi) /
                                  static_cast<float>(in.muzzle_count)
                            : 0.0f;
    const float c = std::cos(angle);
    const float s = std::sin(angle);
    for (int k = 0; k < 3; ++k) {
        out[k] = in.origin[k] + in.radius * c * in.basis_right[k] +
                 in.radius * s * in.basis_up[k];
    }
}

} // namespace bsp
