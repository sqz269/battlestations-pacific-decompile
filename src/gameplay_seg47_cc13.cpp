// Segment 47 reference models: plane damage and crash. Packet cc13-gpr47;
// docs/CC13_GPR_47.md. Each function cites the listing it was read from.
// Hypothesis names; build-tested only, not wired, not game-run.

#include "bsp/gameplay_seg47_cc13.hpp"

#include <cctype>

namespace bsp {
namespace gameplay_seg47_cc13 {
namespace {

bool iequals(const char* a, const char* b) noexcept {
    for (; *a != '\0' && *b != '\0'; ++a, ++b) {
        if (std::tolower(static_cast<unsigned char>(*a)) !=
            std::tolower(static_cast<unsigned char>(*b)))
            return false;
    }
    return *a == *b;
}

// 0077C2A0(msg, 7, 0).
void route(NamedEffectResult& out, FailureMessageKind kind, float value_20,
           std::uint32_t arg_24 = 0) {
    out.routed.push_back(FailureMessage{kind, value_20, arg_24});
}

// The tail every failure arm but engine fire shares: 007BC17B / 007BC241 /
// 007BC2xx / 007BC357 test [00E188A8]+1FE4h and route 007BA8D0's message.
void route_dead_meat_rot(NamedEffectResult& out, int session_mode) {
    if (session_mode != 0) route(out, FailureMessageKind::DeadMeatRot, 0.0f);
}

}  // namespace

NamedEffect named_effect_from_name(const char* name) noexcept {
    if (name == nullptr) return NamedEffect::Other;
    static constexpr struct {
        const char* text;
        NamedEffect effect;
    } kNames[] = {
        {"explosion", NamedEffect::Explosion},
        {"explosion_delayed", NamedEffect::ExplosionDelayed},
        {"splash", NamedEffect::Splash},
        {"enginefire", NamedEffect::EngineFire},
        {"powerlost", NamedEffect::PowerLost},
        {"spinning", NamedEffect::Spinning},
        {"leftspinning", NamedEffect::LeftSpinning},
        {"rightspinning", NamedEffect::RightSpinning},
    };
    for (const auto& n : kNames)
        if (iequals(name, n.text)) return n.effect;
    return NamedEffect::Other;
}

// 007BBFA0-007BC457.
NamedEffectResult named_effect_007bbfa0(const NamedEffectInputs& in,
                                        const PlaneFailureGlobals& g, const Rng& rng) {
    NamedEffectResult out;
    out.timer_c10 = in.timer_c10;

    // 007BBFC7-007BC031: BL. "explosion", or a state outside 4..7, or (the
    // controller gate clear and 0.2777778f > speed; FCOMI/JA, so a NaN speed
    // does not force).
    bool forced = in.effect == NamedEffect::Explosion;
    if (!forced) {
        const int st = in.flight_state_900;
        if (st != 7 && st != 6 && st != 4 && st != 5) {
            forced = true;
        } else if (!(in.controller_gate_72c && in.controller_gate_72c())) {
            forced = kForcedExplosionSpeed > in.cached_speed_b1c;
        }
    }
    out.forced_explosion = forced;

    // 007BC031-007BC055 and 007BC385-007BC3E4: the explosion arm. Only an
    // unforced "explosion_delayed" on a plane that was not rammed (unit+DFCh)
    // draws its delay; everything else explodes at once.
    if (in.effect == NamedEffect::ExplosionDelayed || forced) {
        if (in.effect == NamedEffect::ExplosionDelayed && !forced && !in.rammed_dfc) {
            out.timer_c10 = rng.uniform(0, g.explosion_delay_a_e18710,
                                        g.explosion_delay_b_e1870c);  // 007BC3C7
        } else {
            out.timer_c10 = kImmediateTimer;
        }
        route(out, FailureMessageKind::Explosion, out.timer_c10);  // 007BC3EC
        return out;
    }

    switch (in.effect) {
    case NamedEffect::Splash:  // 007BC06B
        route(out, FailureMessageKind::Splash, kSplashMessageTimer);
        return out;
    case NamedEffect::EngineFire: {  // 007BC09A-007BC12F
        if (in.engine_count_7c <= 0) return out;  // JLE
        std::uint32_t index = in.lua_engine_index
                                  ? static_cast<std::uint32_t>(*in.lua_engine_index)
                                  : 0xFFFFFFFFu;
        if (index == 0xFFFFFFFFu)  // 007BC0FC, ECX = EDI + 2 = 1
            index = rng.next_u32(1) % static_cast<std::uint32_t>(in.engine_count_7c);
        route(out, FailureMessageKind::EngineFire, kEngineFireMessageTimer, index);
        return out;
    }
    case NamedEffect::PowerLost:  // 007BC144-007BC1AD
        out.timer_c10 = kFailureTimer;
        route(out, FailureMessageKind::EnginesLost, kFailureTimer);
        route_dead_meat_rot(out, in.session_mode_1fe4);
        return out;
    case NamedEffect::Spinning:  // 007BC1C6-007BC26B
        out.timer_c10 = kFailureTimer;
        if (!in.spinning_c36) {
            const float draw = rng.uniform(1, 0.0f, 1.0f);  // 007BC1F2
            const std::uint32_t side = kSpinSideSplit > draw ? 1u : 0u;  // FCOMI/JA
            route(out, FailureMessageKind::Spinning, out.timer_c10, side);
        }
        route_dead_meat_rot(out, in.session_mode_1fe4);
        return out;
    case NamedEffect::LeftSpinning:
    case NamedEffect::RightSpinning:  // 007BC270-007BC380
        out.timer_c10 = kFailureTimer;
        if (!in.spinning_c36)
            route(out, FailureMessageKind::Spinning, kFailureTimer,
                  in.effect == NamedEffect::RightSpinning ? 1u : 0u);
        route_dead_meat_rot(out, in.session_mode_1fe4);
        return out;
    default:
        return out;  // 007BC442, AL = 1 with nothing routed
    }
}

FailureQueryResult failure_query_007bc460(NamedEffect effect, bool powerlost_c39,
                                          bool spinning_c36) noexcept {
    FailureQueryResult r;
    switch (effect) {
    case NamedEffect::EngineFire: r.out = false; break;
    case NamedEffect::PowerLost: r.answered = true; r.out = powerlost_c39; break;
    case NamedEffect::Spinning: r.answered = true; r.out = spinning_c36; break;
    default: break;
    }
    return r;
}

// 007D0B80-007D1351.
void state_message_007d0b80(PlaneFailureState& s, const FailureMessage& msg,
                            const StateMessageEnv& env, const StateMessageHost& host) {
    // 007D0B9D-007D0BBE: every kind copies +20h unless the plane is dead
    // outside session mode 2.
    if (env.session_mode_1fe4 == 2 || !s.dead_5d) s.timer_c10 = msg.value_20;
    if (s.removed_5e) return;  // 007D0BCA

    const FailureMessageKind k = msg.kind;
    bool tail = true;
    // 007D0BEB 66h, or 007D0BF9-007D0C2C: 63h with C10h > 0 (COMISS/JBE) and
    // the controller gate: the power-lost arm 007D12D6.
    if (k == FailureMessageKind::EnginesLost ||
        (s.timer_c10 > 0.0f && env.controller_gate_72c && env.controller_gate_72c() &&
         k == FailureMessageKind::Explosion)) {
        host.stop_all_engines();
        s.powerlost_c39 = true;
        if (s.flag_969 && env.has_model_360) s.flag_969 = false;
    } else if (k == FailureMessageKind::Explosion) {  // 007D0C37-007D0D12
        if (env.pos_y_100 > -5.0f) {  // 00CFBC84
            if (env.marker_service_on && !s.removed_5e) {
                if (!s.has_marker_c4c) s.has_marker_c4c = host.create_marker();
                if (s.has_marker_c4c) host.update_marker();
            }
            host.spawn_explosion_effect();
        }
        host.kill();
        s.timer_c10 = kImmediateTimer;
    } else if (k == FailureMessageKind::Splash) {  // 007D0D1C-007D0D3F
        host.bind_crash_effect();
        host.stop_all_engines();
        s.powerlost_c39 = true;
    } else if (k == FailureMessageKind::EngineFire) {  // 007D0D49-007D0D9A
        host.destroy_engine_at(msg.arg_24);
        if (s.dead_5d && !s.removed_5e && !s.has_marker_c4c)
            s.has_marker_c4c = host.create_marker();
        tail = false;  // JMP 007D1339
    } else if (k == FailureMessageKind::Spinning) {  // 007D0DA4-007D12D4
        s.spinning_c36 = true;
        if (!env.has_model_360) {
            tail = false;
        } else {
            const bool right = (msg.arg_24 & 0xFFu) != 0;
            if (right) s.right_wing_c37 = true;  // 007D0DD7
            else s.left_wing_c38 = true;          // 007D104F
            host.break_wing(right);
        }
    } else {
        tail = false;
    }

    // 007D12FC-007D1331.
    if (tail && !s.death_raised_c3a && !s.dead_5d) {
        s.death_raised_c3a = true;
        host.raise_dead();
        if (0.1745329f > s.bank_c68) s.pitch_input_9e8 = 0.0f;  // 00CE3990, COMISS/JBE
    }
}

// 007BBCF0-007BBDBC.
void plane_hit_record_007bbcf0(PlaneHitState& s, const PlaneHitInputs& in,
                               const PlaneHitHost& host) {
    s.hit_latch_520 = false;
    if (in.shooter_is_kind5) s.rammed_dfc = true;
    if (in.player_squadron) host.hud_damage_direction();
    if (!in.death_raised_c3a && !in.dead_5d && in.has_squadron_9d4) host.squadron_on_hit();
    if (in.has_pilot_bot_df4) host.pilot_bot_on_hit();
    if (in.hit_segment_34 != -1) s.spin_side_800 = in.hit_side_38_4;
    host.unit_apply_hit();
}

// 007BCAA0-007BCB27.
void plane_on_destroyed_007bcaa0(PlaneDestroyState& s, const PlaneFailureGlobals& g,
                                 const Rng& rng, const PlaneDestroyHost& host) {
    s.engine_fire_timer_c14 = rng.uniform(0, g.engine_fire_delay_a_e18718,
                                          g.engine_fire_delay_b_e18714);  // 007BCABB
    if (s.kind_1b0 != 8) s.last_kind_748 = s.kind_1b0;
    host.unit_on_destroyed();
    if (s.has_squadron_9d4) host.squadron_remove();
    if (s.has_pilot_bot_df4) {
        host.retire_pilot_bot();
        s.has_pilot_bot_df4 = false;
    }
    if (s.has_sub_6dc) {
        host.retire_sub_6dc();
        s.has_sub_6dc = false;
    }
}

// 007C6C30-007C6E08.
void out_of_action_countdown_007c6c30(ExitCountdownState& s, bool dead_5d, float step,
                                      const ExitCountdownTuning& t,
                                      const ExitCountdownHost& host) {
    if (dead_5d) return;
    // 007C6C5A-007C6C7A: inside the map, or a squadron that is not leaving on
    // purpose: reset (007C6DCC).
    if (!host.outside_map() || (s.has_squadron_9d4 && !host.squadron_leaving())) {
        s.timer_6f0 = t.period_34c;
        s.state_6ec = 1;
        if (!s.flag_9d8 && s.has_squadron_9d4) s.squadron_warn_36c = 0.0f;
        return;
    }
    s.timer_6f0 = s.timer_6f0 - step;  // 007C6C86 FLD/FSUB/FSTP float
    std::int32_t next = 2;
    if (host.beyond_margin(t.margin_348) || 0.0f > s.timer_6f0) {
        next = 3;
        s.timer_6f0 = kImmediateTimer;
    }
    if (!s.flag_9d8) {  // 007C6CF2
        if (s.has_squadron_9d4) {
            if (host.local_player_role())
                s.squadron_warn_36c = s.squadron_warn_36c - step;
            else
                s.squadron_warn_36c = 1.0f;  // 00D7A24C
        }
        // The old state, before the store at 007C6D94.
        if (s.state_6ec == 1 || (s.has_squadron_9d4 && 0.0f > s.squadron_warn_36c)) {
            host.warn_exit_zone();
            if (s.has_squadron_9d4) s.squadron_warn_36c = t.warn_period_350;
        }
    }
    s.state_6ec = next;
    if (0.0f > s.timer_6f0) host.send_exit_zone();  // 007C6D8A-007C6DC1
}

void exit_zone_007ca790(bool has_squadron_9d4, std::int32_t party, const ExitZoneHost& host) {
    host.set_lua_exit_zone_party(party);
    if (has_squadron_9d4) host.squadron_left_map(party);
}

// 007CAAD0-007CAE3x.
std::optional<DebrisSpec> debris_spec_007caad0(bool record_found, const Vec3& node_translation,
                                               const Vec3& plane_velocity,
                                               const Vec3& record_14, const Rng& rng) {
    if (!record_found) return std::nullopt;
    DebrisSpec d;
    // 007CAC5D/007CAC8B/007CACB9: U(-0.001, 0.001) FADD the copied world
    // translation, FSTP float.
    d.position.x = rng.uniform(1, -0.001f, 0.001f) + node_translation.x;
    d.position.y = rng.uniform(1, -0.001f, 0.001f) + node_translation.y;
    d.position.z = rng.uniform(1, -0.001f, 0.001f) + node_translation.z;
    d.velocity_08 = plane_velocity;
    d.spin_14.x = rng.uniform(1, -1.0f, 1.0f);  // 007CAD58
    d.spin_14.y = rng.uniform(1, -1.0f, 1.0f);  // 007CAD78
    d.spin_14.z = rng.uniform(1, -1.0f, 1.0f);  // 007CAD98
    d.record_24 = record_14;
    return d;
}

// 007BBDC0-007BBF07.
bool set_control_factor_007bbdc0(PlaneControlFactors& f, const char* name, std::int32_t index,
                                 float value, std::int32_t engine_count_140,
                                 const ControlFactorHost& host) {
    if (name != nullptr && iequals(name, "roll")) {
        f.roll_bcc = value;  // 007BBE07
    } else if (name != nullptr && iequals(name, "pitch")) {
        f.pitch_bd0 = value;  // 007BBE27
    } else if (name != nullptr && iequals(name, "power")) {
        if (index < 0 || index > 3) return false;  // model limit, see the header
        // 007B8140: store, then a float running sum (FADD/FSTP per element).
        f.engine_power_bd8[index] = value;
        float sum = 0.0f;
        for (std::int32_t i = 0; i < engine_count_140 && i < 4; ++i)
            sum = sum + f.engine_power_bd8[i];
        f.power_mean_bd4 = sum / static_cast<float>(engine_count_140);  // 007B81D8 FIDIV
        if (!(0.0f < value)) {
            if (value == value) host.stop_engine_at(index);  // 007BBE6D COMISS/JB
        }
    } else {
        return false;  // 007BBEF4, AL = 0
    }
    host.broadcast(f);  // 007BBE80-007BBED8
    return true;
}

// 007BBF10-007BBF98.
bool get_control_factor_007bbf10(const PlaneControlFactors& f, const char* name,
                                 std::int32_t index, float& out) {
    if (name == nullptr) return false;
    if (iequals(name, "roll")) {
        out = f.roll_bcc;
    } else if (iequals(name, "Pitch")) {
        out = f.pitch_bd0;
    } else if (iequals(name, "Power")) {
        if (index < 0 || index > 3) return false;  // model limit
        out = f.engine_power_bd8[index];
    } else {
        return false;
    }
    return true;
}

// 007B97F0-007B985A.
void restore_full_health_007b97f0(PlaneControlFactors& f, float (&limits_be8)[3],
                                  const RestoreFullHealthHost& host) {
    host.unit_restore();
    f = PlaneControlFactors{};  // BCCh..BE4h = 1.0f (00D7A24C)
    limits_be8[0] = limits_be8[1] = limits_be8[2] = 0.0f;
    host.engines_reset_all();
}

// 007CC375-007CC41F.
FreeFlightTail free_flight_tail_007cc2f0(FreeFlightTailInputs& in) noexcept {
    if (!in.dead_5d || (in.has_game_e188a8 && in.session_mode_1fe4 == 2))
        return FreeFlightTail::LiveBranch;
    if (in.splashed_911) return FreeFlightTail::Nothing;
    if (!(3.0f > in.pos_y_100)) return FreeFlightTail::Nothing;  // 007CC3BA
    in.splashed_911 = true;  // 007CC418, after the vtable +194h call
    return FreeFlightTail::DeadSplash;
}

}  // namespace gameplay_seg47_cc13
}  // namespace bsp
