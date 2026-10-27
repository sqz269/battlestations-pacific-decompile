// The plane `moveonpath` bot task. Packet cc13-moveonpath1; docs/CC13_MOVEONPATH_TASK.md.
// Read from the installed executable's listing (local/mp1_window.asm, Capstone over
// the disk bytes 009BC690-009BDD70). Names are hypotheses.

#include "bsp/moveonpath_task_cc13.hpp"

#include <cmath>

#include "bsp/unit_rudder.hpp"   // clamped_interpolate_00419010
#include "bsp/cc13_x87_model.hpp"
#include "bsp/pc24_planetasks_cc13.hpp"

// kPc24PlaneTasksCc13Bound sites here (pc24w1-sweep3-7): planar_length_00414c60
// (00414C61-00414C6E) and the rule timer reload of 009BD150 (009BD16A-009BD179).

namespace bsp {

namespace {

// 00414C60 BSP_Vector2f_LengthWithCutoff: s = x*x + z*z rounded to float; 0 when
// s <= 1e-10 ([00CE3820], a qword), else sqrt(s) (00BF7030) rounded to float.
float planar_length_00414c60(float dx, float dz) noexcept {
    float s;
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // 00414C66 DC C8 FMUL ST0 x*x, DE CA FMULP z*z, DE C1 FADDP, each PC24.
        s = static_cast<float>(cc13x87::x87_add24(cc13x87::x87_mul24(dz, dz),
                                                  cc13x87::x87_mul24(dx, dx)));
    } else {
        s = static_cast<float>(static_cast<double>(dx) * dx +
                               static_cast<double>(dz) * dz);
    }
    if (!(static_cast<double>(s) > 1e-10)) {
        return 0.0f;
    }
    return static_cast<float>(std::sqrt(static_cast<double>(s)));
}

// 009FD050 applied to the approach and the leg manager.
void apply_leg_009fd050(MoveOnPathApproach& ap, MoveOnPathLegHost& legs, float d,
                        const std::array<float, 3>& point) {
    const MoveOnPathLegUpdate u = moveonpath_leg_weight_009fd050(d);
    if (u.registers_009fc000) {
        legs.leg_register_009fc000(point);   // 009FD078 -> 009FC000(unit, point)
    } else {
        legs.leg_remove_009fc1c0();          // 009FD0BE -> 009FC1C0(unit)
    }
    ap.leg_weight_28 = u.weight_28;          // 009FD0B3 / 009FD0D2
}

// The state machine's exit (vtable[8]) and enter (vtable[4]). Only the follow state
// has bodies; the path and circle states inherit 007B3DB0 / 007B3DC0, a bare RET.
void exit_state(MoveOnPathTaskHost& host, MoveOnPathState s) {
    if (s == MoveOnPathState::kFollow) host.exit_follow_009bde40();
}
void enter_state(MoveOnPathTaskHost& host, MoveOnPathState s) {
    if (s == MoveOnPathState::kFollow) host.enter_follow_009bed80();
}

}  // namespace

MoveOnPathLegUpdate moveonpath_leg_weight_009fd050(float d) noexcept {
    MoveOnPathLegUpdate u;
    // 009FD056 COMISS d, [00D7A218] (0.0), JB; 009FD06A COMISS [00CFA424] (3000), d, JBE.
    // COMISS sets CF and ZF on unordered, so a NaN takes the removal branch.
    if (!(d >= 0.0f) || !(3000.0f > d)) {
        u.registers_009fc000 = false;
        u.weight_28 = -1.0f;                 // [00D7A260]
        return u;
    }
    u.registers_009fc000 = true;
    // 009FD084-009FD0AE: 00419010(x0 = [00CFD714] 1200, y0 = 1.0, x1 = [00CFA424] 3000,
    // y1 = 0.0, x = d), FSTP [ESI+28h].
    u.weight_28 = clamped_interpolate_00419010(1200.0f, 1.0f, 3000.0f, 0.0f, d);
    return u;
}

void moveonpath_approach_update_009bd4d0(MoveOnPathApproach& ap,
                                         const MoveOnPathApproachInputs& in,
                                         MoveOnPathLegHost& legs) {
    if (!in.cursor.present) {
        // 009BD50D-009BD53A: no cursor; the leg is the squadron's own position at
        // weight 1.0 (FLD1), and nothing else is touched.
        apply_leg_009fd050(ap, legs, 1.0f, in.ctl_position_fc);
        return;
    }
    // 009BD56C-009BD594: approach+34h..+3Ch = cursor->vtable[4](&out, -1).
    ap.ref_34 = in.cursor.waypoint;
    // 009BD597 007B1350(&out); 009BD59C-009BD5AD tests its valid byte.
    if (in.cursor.final_valid) {
        // 009BD5E3-009BD618: planar distance squadron -> final point, 00414C60 on
        // (ctl.x - p.x, ctl.z - p.z); 009BD63C 009FD050(len, &p).
        const float dx = static_cast<float>(static_cast<double>(in.ctl_position_fc[0]) -
                                            in.cursor.final_point[0]);
        const float dz = static_cast<float>(static_cast<double>(in.ctl_position_fc[2]) -
                                            in.cursor.final_point[2]);
        apply_leg_009fd050(ap, legs, planar_length_00414c60(dx, dz), in.cursor.final_point);
    } else {
        // 009BD61F-009BD642: 009FD050([00D7A260] -1.0, &ctl+FCh): the leg is dropped.
        apply_leg_009fd050(ap, legs, -1.0f, in.ctl_position_fc);
    }
    // 009BD647-009BD65B: no cruise altitude set (ctl+3ADh == 0) zeroes both offsets.
    if (!in.ctl_alt_set_3ad) {
        ap.travel_alt_offset_30 = 0.0f;
        ap.alt_offset_2c = 0.0f;
    }
    // 009BD660-009BD692: an authored point at or above 10.0 ([00CE38B8]) with no
    // positive leg weight flies at the point's height plus altOffset. COMISS: an
    // unordered y fails `10 > y` (JA not taken) and so takes this arm.
    const float y = ap.ref_34[1];
    if (!(10.0f > y) && !(ap.leg_weight_28 > 0.0f)) {
        ap.ref_34[1] = static_cast<float>(static_cast<double>(ap.alt_offset_2c) + y);
        return;
    }
    // 009BD695-009BD6AC: otherwise the squadron's cruise altitude less travelAltOffset,
    // 009BD6AF-009BD6D2: replaced by the leg manager's altitude while the weight is > 0.
    ap.ref_34[1] = static_cast<float>(static_cast<double>(in.ctl_alt_394) -
                                      ap.travel_alt_offset_30);
    if (ap.leg_weight_28 > 0.0f) {
        ap.ref_34[1] = legs.leg_altitude_009fc260(ap.ref_34, ap.leg_weight_28);
    }
}

bool moveonpath_rule_timer_009bd150(MoveOnPathTask& task, float dt) noexcept {
    // 009BD154-009BD168: FLD +51Ch; FLD dt; FCOMI; JC when dt < timer.
    if (dt < task.rule_timer_51c) {
        // 009BD1BE FSUBP ST(1),ST(0) (DE E9): timer - dt.
        task.rule_timer_51c = static_cast<float>(static_cast<double>(task.rule_timer_51c) - dt);
        return false;
    }
    // 009BD16A-009BD179: (+518h - dt) + timer.
    if constexpr (kPc24PlaneTasksCc13Bound) {
        // D8 AE FSUBR [ESI+518h] period - dt, DE C1 FADDP + timer, each PC24.
        task.rule_timer_51c = static_cast<float>(cc13x87::x87_add24(
            task.rule_timer_51c, cc13x87::x87_sub24(task.rule_period_518, dt)));
    } else {
        task.rule_timer_51c = static_cast<float>(
            (static_cast<double>(task.rule_period_518) - dt) + task.rule_timer_51c);
    }
    return true;
}

MoveOnPathState moveonpath_rule_state_009bd17f(bool leader) noexcept {
    // 009BD17F-009BD18E: a leader takes path (+44Ch), a wingman follow (+464h).
    return leader ? MoveOnPathState::kPath : MoveOnPathState::kFollow;
}

MoveOnPathPlanCommand moveonpath_path_tick_009bc8e0(const std::array<float, 3>& ref,
                                                    float ctl_speed_3a0) noexcept {
    MoveOnPathPlanCommand c;
    c.writes_neutral_26c = true;          // 009BC8ED MOV byte [plan+26Ch],1
    c.speed_2b4 = ctl_speed_3a0;          // 009BC909-009BC921
    c.pitch_altitude = ref[1];            // 009BC931 FLD [ESP+10h], ref.y
    c.pitch_gain = 1.0f;                  // 009BC91F FLD1
    c.heads_to_point_009f9e40 = true;     // 009BC944
    c.head_point = ref;
    return c;
}

MoveOnPathPlanCommand moveonpath_circle_tick_009bca50(const std::array<float, 3>& ref,
                                                      float desc_speed_18c,
                                                      float desc_radius_268,
                                                      bool clockwise_18,
                                                      bool has_director) noexcept {
    MoveOnPathPlanCommand c;
    c.speed_2b4 = desc_speed_18c;         // 009BCA70-009BCA88
    c.pitch_altitude = ref[1];            // 009BCA98
    c.pitch_gain = 1.0f;                  // 009BCA86 FLD1
    c.circles_009fbb20 = true;            // 009BCADC, result discarded (FSTP ST0)
    c.circle_center_x = ref[0];
    c.circle_center_z = ref[2];
    c.circle_radius = desc_radius_268;    // 009BCACC FLD [desc+268h]
    c.circle_clockwise = clockwise_18;    // 009BCAA9 MOVZX EDX, byte [state+18h]
    c.ends_command_0071e430 = has_director;  // 009BCAE3-009BCB0E
    c.writes_gun_cone_40 = true;          // 009BCB13-009BCB23
    c.calls_direction_009fabe0 = true;    // 009BCB26-009BCB46
    return c;
}

MoveOnPathCruiseResult moveonpath_cruise_profile_009bd6e0(const MoveOnPathCruiseInputs& in,
                                                          MoveOnPathRandom& rng) {
    MoveOnPathCruiseResult r;
    // 009BD6E4-009BD705: task+428h (approach+30h) = U(0, tuning+364h), every call.
    r.travel_alt_offset_30 = rng.uniform_00bd2f10("30", 0.0f, in.travel_alt_random_364);
    // 009BD70B-009BD718: 007B8AD0; a wingman stops here.
    if (!in.flight_leader) {
        return r;
    }
    r.leader_block_ran = true;
    // 009BD71F-009BD739: random = tuning+364h * [00CEFF98] (a qword, 0.6000000238...),
    // stored to a float slot.
    const float random_part =
        static_cast<float>(static_cast<double>(in.travel_alt_random_364) * 0.6000000238418579);

    // UCOMISS against [00D7A218] 0.0 with LAHF / TEST AH,44h / JNP: equal goes to the
    // plain arm. The MoveOn slots are int32, so only 0 and 80000000h compare equal.
    auto move_on = [](std::int32_t bits) noexcept {
        return (static_cast<std::uint32_t>(bits) & 0x7FFFFFFFu) != 0u;
    };
    bool ordnance_arm = false;
    float alt = 0.0f;
    if (in.has_torpedo_007b93f0) {                       // 009BD73D
        if (move_on(in.torpedo_move_on_43c)) {           // 009BD74B
            ordnance_arm = true;
            alt = static_cast<float>(static_cast<double>(in.torpedo_cruising_alt_430) +
                                     random_part);        // 009BD794-009BD79E
        }
    } else if (in.has_depth_charge_007b94f0) {           // 009BD7B3
        if (move_on(in.depth_move_on_4b4)) {             // 009BD7C1
            ordnance_arm = true;
            alt = static_cast<float>(static_cast<double>(in.depth_cruising_alt_4a4) +
                                     random_part);        // 009BD80A
        }
    } else if (in.has_general_bomb_007b9320) {           // 009BD829
        if (in.level_bomber_0047b850) {                  // 009BD83C
            if (move_on(in.level_move_on_454)) {
                ordnance_arm = true;
                alt = static_cast<float>(static_cast<double>(in.level_cruising_alt_444) +
                                         random_part);    // 009BD893
            }
        } else if (move_on(in.dive_move_on_4d4)) {       // 009BD8B0
            ordnance_arm = true;
            alt = static_cast<float>(static_cast<double>(in.dive_cruising_alt_4c0) +
                                     random_part);        // 009BD8F5
        }
    }
    if (!ordnance_arm) {
        // 009BD90D-009BD913: 0047B880; SmallPlaneTravelAlt (+35Ch) when it answers
        // true, LargePlaneTravelAlt (+360h) otherwise. No random part.
        alt = in.small_turn_roll_0047b880 ? in.small_plane_travel_alt_35c
                                          : in.large_plane_travel_alt_360;
    }
    // The channel, identical in every arm: ctl+38Dh set returns at once (009BD988);
    // ctl+380h < 0 with ctl+3A9h clear writes ctl+394h and ctl+3ADh = 1; ctl+3A9h is
    // cleared written or not (009BD985).
    if (in.cruise.locked) {
        return r;
    }
    if (bot_task_altitude_write_applies(in.cruise)) {
        r.writes_alt_394 = true;
        r.alt_394 = alt;
    }
    r.clears_one_shot_3a9 = true;
    return r;
}

int moveonpath_command_still_valid_009bd280(bool has_ctl, std::uint32_t current_command,
                                            std::uintptr_t command_object,
                                            std::uintptr_t path_object) noexcept {
    if (!has_ctl) {
        return 2;                                        // 009BD2D4
    }
    if (current_command != kMoveOnPathCommandClass) {    // 009BD2A1
        return 0;
    }
    return command_object == path_object ? 1 : 0;       // 009BD2C3
}

void moveonpath_task_construct_009bce50(MoveOnPathTask& task, MoveOnPathTaskHost& host,
                                        MoveOnPathRandom& rng, float max_alt_offset_540) {
    task = MoveOnPathTask{};
    // 009BC780 (from 009BCC60): approach+2Ch = U(0, tuning+540h MaxAltOffset) at
    // 009BC7C0, approach+30h = 50.0. 009F9CE0 (approach head) is host-side.
    task.ap.alt_offset_2c = rng.uniform_00bd2f10("2c", 0.0f, max_alt_offset_540);
    task.ap.travel_alt_offset_30 = 50.0f;
    task.ap.leg_weight_28 = -1.0f;                        // 009F9D6E
    // 009BCCD6 009C2980 (shared follow, arg [00CE3D08] 100.0): +74h = -U(0, 0.6).
    task.follow_stagger_74 = -rng.uniform_00bd2f10("74", 0.0f, 0.6f);
    // 009BCCE7 009BC960: circle state+18h = ([00CE3800] 0.5 <= U(0, 1)).
    task.circle_clockwise_18 = rng.uniform_00bd2f10("18", 0.0f, 1.0f) >= 0.5f;
    // 009BCE93-009BCED5: task+518h = 1.0, task+51Ch = -U(0, 1).
    task.rule_period_518 = 1.0f;
    task.rule_timer_51c = -rng.uniform_00bd2f10("51c", 0.0f, 1.0f);
    // 009BCEDB-009BCF01: leader -> path, wingman -> follow; state->vtable[4]().
    task.state = host.flight_leader_007b8ad0() ? MoveOnPathState::kPath
                                               : MoveOnPathState::kFollow;
    enter_state(host, task.state);
    // 009BCF06 009F9980 (approach bind) is host-side.
}

void moveonpath_task_arm_009bd990(MoveOnPathTask& task, MoveOnPathTaskHost& host, float dt) {
    // 009BD9A1: 009BD4D0 on the approach.
    moveonpath_approach_update_009bd4d0(task.ap, host.approach_inputs(), host);
    // 009BD9B0: 009BD150, the rule; 007B8AD0 is asked only when the period elapses.
    if (moveonpath_rule_timer_009bd150(task, dt)) {
        const MoveOnPathState next =
            moveonpath_rule_state_009bd17f(host.flight_leader_007b8ad0());
        if (next != task.state) {            // 009BD19A
            exit_state(host, task.state);    // 009BD1A7 old->vtable[8]
            task.state = next;               // 009BD1A9
            enter_state(host, task.state);   // 009BD1B6 new->vtable[4]
        }
    }
    // 009BD9B5-009BD9C8: current state->vtable[0Ch](dt).
    switch (task.state) {
    case MoveOnPathState::kPath:
        host.apply_plan(moveonpath_path_tick_009bc8e0(task.ap.ref_34, host.ctl_speed_3a0()));
        break;
    case MoveOnPathState::kFollow:
        host.tick_follow_009c1fd0(dt);
        break;
    case MoveOnPathState::kCircle:
        host.apply_plan(moveonpath_circle_tick_009bca50(task.ap.ref_34, host.desc_speed_18c(),
                                                        host.desc_radius_268(),
                                                        task.circle_clockwise_18,
                                                        host.has_director()));
        break;
    }
}

void moveonpath_task_stop_009bd120(MoveOnPathTask& task, MoveOnPathTaskHost& host) {
    // 009BD120-009BD138: 009FD050(-1.0, &[00F87574]) on the approach: the leg goes.
    apply_leg_009fd050(task.ap, host, -1.0f, std::array<float, 3>{});
    // 009BD13D-009BD14D: current state->vtable[8], a tail jump.
    exit_state(host, task.state);
}

}  // namespace bsp
