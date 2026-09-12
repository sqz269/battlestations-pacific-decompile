#include "bsp/tick_element_overrides.hpp"

// Slot tables and per-step slot bodies of the four classes that register a tick
// element. Evidence in docs/TICK_ELEMENT_OVERRIDES.md.

namespace bsp {
namespace {

constexpr std::size_t kSlotOffsets[kTickElementVtableSlots] = {
    kTickElementDestroySlot, kTickElementStepSlot, kTickElementPreSlot,
    kTickElementPostSlot, kTickElementUnusedSlot, kTickElementPredicateSlot,
};

constexpr std::uint32_t kBaseStubs[kTickElementVtableSlots] = {
    0x00875920, kTickElementBaseStepStub, kTickElementBaseAdvanceStub,
    kTickElementBaseCommitStub, kTickElementBaseUnusedStub,
    kTickElementBasePredicateStub,
};

// The eight tables, in the order of kClasses below.
constexpr TickElementVtable kVtables[kTickElementClassCount] = {
    // base, 00D0DEC8
    {0x00d0dec8, {0x00875920, 0x0042bb70, 0x0042bb80, 0x0042bb90, 0x0042bba0, 0x0042bbb0}},
    // unit level 3, 00D0DF28: only the destructor differs
    {0x00d0df28, {0x0087a4f0, 0x0042bb70, 0x0042bb80, 0x0042bb90, 0x0042bba0, 0x0042bbb0}},
    // unit level 4, 00D1A654
    {0x00d1a654, {0x00959c10, 0x0042bb70, 0x00953cc0, 0x006d1fc0, 0x0042bba0, 0x0042bbb0}},
    // unit level 5, 00D09630
    {0x00d09630, {0x0081f380, 0x00811ab0, 0x00825f20, 0x006d1fc0, 0x0042bba0, 0x0042bbb0}},
    // unit level 6 (MDestroyer), 00CFC38C: same slots, new destructor
    {0x00cfc38c, {0x006fe510, 0x00811ab0, 0x00825f20, 0x006d1fc0, 0x0042bba0, 0x0042bbb0}},
    // plane squadron, 00D0877C
    {0x00d0877c, {0x007efae0, 0x0042bb70, 0x007f3ba0, 0x0042bb90, 0x0042bba0, 0x0042bbb0}},
    // projectile, 00CF9D78
    {0x00cf9d78, {0x006e7bf0, 0x006e6750, 0x006e6490, 0x006e7d50, 0x0042bba0, 0x0042bbb0}},
    // tickable game entity, 00D194C4
    {0x00d194c4, {0x0092a030, 0x00929cb0, 0x0092b350, 0x0042bb90, 0x0042bba0, 0x0042bbb0}},
};

constexpr TickElementClassRow kClasses[kTickElementClassCount] = {
    {"TickElementBase", 0x00875890, 0x00d0dec8, 0x00, 0, -1,
     TickElementCoverage::Complete},
    {"UnitTickableEntity (level 3)", 0x0087b670, 0x00d0df28, 0x310, 0, 4,
     TickElementCoverage::Complete},
    {"UnitGameObject (level 4)", 0x0095cc90, 0x00d1a654, 0x310, 0, 5,
     TickElementCoverage::Complete},
    {"UnitVehicle (level 5)", 0x0081ed40, 0x00d09630, 0x310, 1, 6,
     TickElementCoverage::ContractUnread}, // +8h is 00825F20, another packet
    {"MDestroyer (level 6)", 0x006fe460, 0x00cfc38c, 0x310, 1, 7,
     TickElementCoverage::ContractUnread},
    {"PlaneSquadron", 0x007f2c60, 0x00d0877c, 0x310, 3, 0x18,
     TickElementCoverage::Complete},
    {"Projectile", 0x006e7b00, 0x00cf9d78, 0x244, 0, 0x29,
     TickElementCoverage::Complete},
    {"TickableGameEntity", 0x00929e50, 0x00d194c4, 0x170, 0, 0x58,
     TickElementCoverage::Partial}, // +8h 0092B350 read as a call sequence only
};

} // namespace

std::size_t tick_element_slot_offset(TickElementSlot slot) noexcept {
    return kSlotOffsets[static_cast<std::size_t>(slot)];
}

bool tick_element_wave_calls(JobWavePhase phase, TickElementSlot slot) noexcept {
    switch (phase) {
    case JobWavePhase::kWave1:
        // 008750A0: vtable[+4h](0.05f) then vtable[+0Ch]().
        return slot == TickElementSlot::PlacePose || slot == TickElementSlot::CommitPose;
    case JobWavePhase::kWave3:
        // 00874FE0: vtable[+8h](0.05f).
        return slot == TickElementSlot::AdvanceSim;
    case JobWavePhase::kInterpolation:
        // 00875160: vtable[+4h](leftover), nothing else.
        return slot == TickElementSlot::PlacePose;
    case JobWavePhase::kWave2:
    default:
        // 00875B90 -> 008759B0 walks the sub-list; no slot of the element runs.
        return false;
    }
}

bool tick_element_slot_is_override(const TickElementVtable& table,
                                   TickElementSlot slot) noexcept {
    const std::size_t index = static_cast<std::size_t>(slot);
    return table.slot[index] != kBaseStubs[index];
}

std::size_t tick_element_class_count() noexcept { return kTickElementClassCount; }

const TickElementClassRow& tick_element_class(std::size_t index) noexcept {
    return kClasses[index < kTickElementClassCount ? index : 0];
}

const TickElementVtable& tick_element_vtable(std::size_t index) noexcept {
    return kVtables[index < kTickElementClassCount ? index : 0];
}

// ---------------------------------------------------------------------------
// Unit
// ---------------------------------------------------------------------------

MatrixCopyRequest unit_tick_commit_pose_006d1fc0(bool use_alternate_source) noexcept {
    MatrixCopyRequest request{};
    request.destination_offset = kUnitTickCommitDestination;
    // 006D1FC9 LEA EAX,[ECX-0x29C] against 006D1FDC LEA EDX,[ECX+0x1D0]; the
    // destination is ECX+364h on both paths.
    request.source_offset = use_alternate_source ? kUnitTickCommitAlternateSource
                                                 : kUnitTickCommitLiveSource;
    return request;
}

void unit_tick_advance_sim_00953cc0(UnitTickAdvanceState& state, float step,
                                    UnitTickAdvanceHost& host) {
    // 00953CC4: JL skips the call, so the test is signed and >= 0 runs it. The
    // pushed code is the literal 9 at 00953CDD, not the field.
    if (state.notify_code_528h >= 0) {
        host.unit_virtual_5c_notify(9);
    }

    host.unit_virtual_1f0_advance(step);

    // 00953CFF..00953D11: the flag survives only while the gate byte is set.
    if (state.flag_634h != 0 && !state.gate_byte_61h) {
        state.flag_634h = 0;
    }

    // 00953D1B and 00953D3F: each timer counts down only while it is positive,
    // and is allowed to go negative on the step that crosses zero.
    if (state.timer_6f8h > 0.0f) {
        state.timer_6f8h -= step;
    }
    if (state.timer_6fch > 0.0f) {
        state.timer_6fch -= step;
    }

    // 00953D60: role 8 skips the check entirely.
    if (state.role_1ach != 8) {
        if (!host.unit_role_still_available_00927f10(state.role_1ach)) {
            state.enabled_520h = false;
        }
    }

    if (!state.enabled_520h) {
        return;
    }
    host.unit_virtual_1d8_advance(step);
}

// ---------------------------------------------------------------------------
// Plane squadron
// ---------------------------------------------------------------------------

void squadron_tick_advance_sim_007f3ba0(SquadronTickState& state, float step,
                                        SquadronTickHost& host) {
    // 007F3BAA: the whole timer block is behind step > 0.0f (00D7A218).
    if (step > 0.0f) {
        for (int i = 0; i < kSquadronTickTimerCount; ++i) {
            if (!state.timers.frozen[i]) {
                state.timers.value[i] -= step;
            }
        }
        if (state.notify_flag_3ech) {
            host.squadron_notify_007ee7f0(0);
        }
    }

    // 007F3C0B..007F3C29: morale += (1 - morale) * step * 0.25.
    state.morale_3e8h += (1.0f - state.morale_3e8h) * step * kSquadronTickMoraleRate;

    // 007F3C3F: the countdown fires when the step reaches it, and reloads with
    // the period minus the overshoot rather than with the period.
    if (step < state.countdown_3c4h) {
        state.countdown_3c4h -= step;
    } else {
        state.countdown_3c4h = (state.period_3c0h - step) + state.countdown_3c4h;
        host.squadron_periodic_007ee790();
    }

    // 007F3C60: node+A8h is copied into node+A0h before the member loop, which
    // then ORs each member's answer into node+A0h.
    state.status_3b0h = state.status_3b8h;

    for (std::int32_t index = 0; index < state.member_count; ++index) {
        const SquadronMemberView member = host.member(index);

        // 007F3C84: the squadron's rank is a floor for every member's.
        if (state.rank_2ech > member.rank_2ech) {
            host.set_member_rank(index, state.rank_2ech);
        }
        if (!member.active_904h) {
            continue;
        }

        const bool answered = host.member_update_007b8ad0(index);
        state.status_3b0h = state.status_3b0h || answered;
        if (state.status_3b0h && member.marker_source != 0) {
            state.marker_3b4h = member.marker_source;
        }

        // 007F3CC3: state 1 with the member still active retires it. The native
        // loop then steps the index back so the shrunken array is not skipped;
        // the host owns the array, so it reports the new count through
        // member_count on the next read.
        if (member.state_900h == 1) {
            host.member_release_00926d90(index, 5);
            host.squadron_drop_member_007f3970(index, 0);
            --index;
            --state.member_count;
        }
    }

    // 007F3D02: the tail runs only when node-8h is not exactly 0.0f.
    if (state.finalize_gate_308h != 0.0f) {
        host.squadron_finalize_0077a650();
    }
}

// ---------------------------------------------------------------------------
// Projectile
// ---------------------------------------------------------------------------

void projectile_tick_commit_pose_006e7d50(ProjectilePoseSnapshot& snapshot,
                                          ProjectileSnapshotHost& host) {
    // 006E7D53..006E7D66: previous <- current, before the refresh.
    snapshot.previous = snapshot.current;
    if (!host.world_pose_valid()) {
        host.refresh_world_pose_00414db0();
    }
    snapshot.current = host.world_translation();
}

void projectile_tick_place_pose_006e6750(ProjectileTickState& state, float step,
                                         ProjectileTickHost& host) {
    const float scaled = state.class_time_scale * step;
    const bool alternate = host.motion_mode_alternate_2c();
    host.projectile_place_pose(alternate, scaled);

    if (!host.has_attached_node()) {
        return;
    }
    if (!state.world_pose_valid) {
        host.refresh_world_pose_00414db0();
    }
    host.attached_node_set_transform_34();
}

void projectile_tick_advance_sim_006e6490(ProjectileTickState& state, float step,
                                          ProjectileTickHost& host) {
    const float scaled = state.class_time_scale * step;
    // 006E64B8: the accumulation happens before the mode query, so the scaled
    // step is counted even when the advance below does nothing.
    state.flight_time += scaled;

    const bool alternate = host.motion_mode_alternate_2c();
    host.projectile_advance(alternate, scaled);

    if (state.tracer_enabled) {
        const bool world_mode_flag = !host.world_mode_is_two();
        if (!state.world_pose_valid) {
            host.refresh_world_pose_00414db0();
        }
        host.emit_tracer_0084c430(state.snapshot.previous, host.world_translation(),
                                  world_mode_flag);
    }

    // 006E6584: expiry compares the accumulated flight time with the class's
    // [+54h], not with a per-instance field.
    if (state.flight_time > state.class_max_life) {
        host.projectile_expire_00696350();
        host.projectile_release_00926d90(2);
    }
}

// ---------------------------------------------------------------------------
// Tickable game entity
// ---------------------------------------------------------------------------

float game_entity_tick_alpha(float step) noexcept {
    // 00929CD2 FDIV against 00D0DE84.
    return step / kTickElementFixedStep;
}

void game_entity_tick_place_pose_00929cb0(float step, GameEntityTickHost& host) {
    if (!host.has_physics_body()) {
        return;
    }
    host.interpolate_body_transform_00c43ea0(game_entity_tick_alpha(step));

    const TickPoint3 pivot = host.transform_pivot_0042d0d0();
    TickPoint3 translation = host.interpolated_translation();
    translation.x -= pivot.x;
    translation.y -= pivot.y;
    translation.z -= pivot.z;

    host.entity_set_interpolated_matrix_00741e90(translation);
    host.attached_node_set_transform_34(translation);
}

} // namespace bsp
