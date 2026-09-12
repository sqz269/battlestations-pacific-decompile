// Reconstruction of the gun-side AI bot ticks. Evidence in
// docs/GUN_BOT_TICKS.md; each routine is a projection of one native body and
// the coverage of each is in that document's routine table.
#include "bsp/gun_bot_ticks.hpp"

#include "bsp/unit_rudder.hpp"

#include <cmath>

namespace bsp {

// ---------------------------------------------------------------------------
// 0072C6A0, which bot a weapon descriptor sub-type gets
// ---------------------------------------------------------------------------
GunBotSlotAssignment gun_bot_slots_for_subtype_0072c6a0(int weapon_sub_type,
                                                        bool gun_is_turning,
                                                        bool owner_of_kind_0f) noexcept
{
    GunBotSlotAssignment slots;

    // 0072C6C2: sub-type 1 and gun->vtable[5Ch](22h).
    if (weapon_sub_type == 1 && gun_is_turning) {
        // 0072C6EB: 00922E90(gun, 0Fh) picks between the two classes.
        slots.primary = owner_of_kind_0f ? GunBotClass::kTurret : GunBotClass::kLead;
    }

    // 0072C75D: sub-types 5 and 6, which also destroy whatever gun+390h holds.
    if (weapon_sub_type == 5 || weapon_sub_type == 6) {
        slots.ballistic = GunBotClass::kBallistic;
        slots.destroys_primary = true;
    }

    // 0072C7DB: sub-types 2, 3, 4 and 6.
    if (weapon_sub_type == 2 || weapon_sub_type == 3 || weapon_sub_type == 4 ||
        weapon_sub_type == 6) {
        slots.muzzle = GunBotClass::kMuzzle;
    }

    if (weapon_sub_type == 7) { // 0072C83x
        slots.torpedo = GunBotClass::kTorpedo;
    }
    if (weapon_sub_type == 8) { // 0072C88x
        slots.sub_type_8 = GunBotClass::kSubType8;
    }
    if (weapon_sub_type == 9) { // 0072C8Fx
        slots.muzzle_second = GunBotClass::kMuzzle;
    }
    return slots;
}

// ---------------------------------------------------------------------------
// The shared prologue
// ---------------------------------------------------------------------------
bool gun_bot_side_enabled_00927f10(int side_index, bool side_record_flag) noexcept
{
    // 008FFA93: CMP EAX,8 / JZ skips the call entirely.
    return side_index == kSideIndexNone || side_record_flag;
}

bool gun_bot_target_still_valid_008ffa20(const GunBotTargetValidity& v) noexcept
{
    // 008FFA37..008FFA5F: any failure sets BL and reaches vtable[38h](0).
    return v.target_resolved && v.target_alive && v.gun_present &&
           v.gun_parent_present && v.gun_parent_alive && v.sides_related;
}

GunBotIdleAction gun_bot_idle_timer_008fbce0(GunBotIdleTimer& timer,
                                             bool has_target,
                                             bool side_enabled,
                                             bool gun_is_turning,
                                             float idle_limit,
                                             float dt) noexcept
{
    // 008FBCEB: with either target byte set the timer is pinned at zero.
    if (has_target) {
        timer.elapsed = 0.0f;
        return GunBotIdleAction::kNone;
    }

    timer.elapsed += dt; // 008FBCFx
    GunBotIdleAction action = GunBotIdleAction::kNone;

    // 008FBD10: the rest-angle arm runs only while the side gate passes and the
    // timer has crossed the limit without yet being pinned.
    if (side_enabled && idle_limit < timer.elapsed && timer.elapsed < kGunBotFloatMax) {
        timer.elapsed = kGunBotFloatMax; // 00D7A248
        if (gun_is_turning) {            // vtable[5Ch](22h) at 008FBD4x
            action = GunBotIdleAction::kAimToRest; // 008FBD61
        }
    }

    // 008FBD7A: once the side gate fails a pinned timer is wound back.
    if (!side_enabled && timer.elapsed == kGunBotFloatMax) {
        timer.elapsed = idle_limit * kGunBotIdleRestFraction;
    }
    return action;
}

// ---------------------------------------------------------------------------
// 008FFA20
// ---------------------------------------------------------------------------
bool gun_bot_turret_recompute_due_008ffa20(float& countdown, float dt) noexcept
{
    // 008FFB35: FLD [ESI+6Ch] / FSUB dt, stored back before the test.
    countdown -= dt;
    return countdown < 0.0f; // 008FFB6B FLDZ / FCOMIP / JBE
}

// ---------------------------------------------------------------------------
// 009030C0
// ---------------------------------------------------------------------------
float gun_bot_ballistic_vertical_correction_009030c0(float vert,
                                                     bool target_is_kind_0f) noexcept
{
    if (vert >= 0.0f) { // 009032A8, only a depression is corrected
        return vert;
    }
    if (target_is_kind_0f) { // 009032B5, vtable[5Ch](0Fh)
        return 0.0f;
    }
    if (vert < kBallisticBotDepressionFloor) { // 009032CE, COMISS -0.02 vs v
        // 009032D5..009032E9: v - (0.02 + v) * 0.5.
        return vert - (kBallisticBotDepressionBias + vert) * kGunBotHalf;
    }
    return vert;
}

bool gun_bot_ballistic_fire_009030c0(const BallisticBotFireInputs& in) noexcept
{
    if (!(in.min_range < in.distance)) { // 00903333 FCOMI / JBE
        return false;
    }
    if (!(in.distance < in.max_range)) { // 00903340 FCOMIP / JBE
        return false;
    }
    if (!(std::fabs(in.horz_delta) < kGunBotOneDegree)) { // 0090337C
        return false;
    }
    if (!(std::fabs(in.vert_delta) < kGunBotOneDegree)) { // 009033B9
        return false;
    }
    return !in.player_inhibit; // 009033D2 TEST byte [EAX+634h],1
}

// ---------------------------------------------------------------------------
// 006DF520
// ---------------------------------------------------------------------------
GunAimAngles gun_bot_muzzle_error_006df520(const MuzzleBotErrorEnvelope& envelope,
                                           float period, float countdown) noexcept
{
    GunAimAngles error;
    // 006DF623: InterpolateClamped(0, start, period, end, countdown).
    error.horz = clamped_interpolate_00419010(0.0f, envelope.start_horz, period,
                                              envelope.end_horz, countdown);
    // 006DF651: the same over the vertical envelope.
    error.vert = clamped_interpolate_00419010(0.0f, envelope.start_vert, period,
                                              envelope.end_vert, countdown);
    return error;
}

bool gun_bot_angle_within_tolerance_006dee40(float current, float desired,
                                             float tolerance) noexcept
{
    // 006DEE52 then AND EAX,7FFFFFFFh at 006DEE5B, compared at 006DEE68.
    const float delta = wrapped_angle_subtract_00438b10(current, desired);
    return std::fabs(delta) <= tolerance;
}

int gun_bot_muzzle_inhibit_bit_006df520(int muzzle_kind) noexcept
{
    // 006DFC0x: [[gun+3F8h]+34h]+8h selects the shift.
    if (muzzle_kind == 0x0b) {
        return 3;
    }
    if (muzzle_kind == 0x0a) {
        return 2;
    }
    return 1;
}

void gun_bot_muzzle_arm_fire_006df520(MuzzleBotDelayedFire& state,
                                      bool solver_ok, bool aim_accepted,
                                      float gun_horz, float gun_vert,
                                      const GunAimAngles& commanded,
                                      float draw) noexcept
{
    if (!solver_ok || !aim_accepted) { // 006DFB5C, the AND of both answers
        return;
    }
    if (state.armed) { // 006DFB6x, a shot already queued is not re-armed
        return;
    }
    if (!gun_bot_angle_within_tolerance_006dee40(gun_horz, commanded.horz,
                                                 kGunBotTenthDegree)) {
        return; // 006DFB8C
    }
    if (!gun_bot_angle_within_tolerance_006dee40(gun_vert, commanded.vert,
                                                 kGunBotTenthDegree)) {
        return; // 006DFBB6
    }
    state.delay = draw; // 006DFBD6, 00BD2F10(0, 0.1)
    state.armed = true;
}

bool gun_bot_muzzle_release_fire_006df520(MuzzleBotDelayedFire& state,
                                          bool inhibited, float dt) noexcept
{
    if (!state.armed || inhibited) { // 006DFC4x
        return false;
    }
    const float remaining = state.delay;
    state.delay = remaining - dt;
    if (remaining - dt >= 0.0f) {
        return false;
    }
    state.armed = false; // 006DFC6x, cleared with the vtable[1F0h] call
    return true;
}

// ---------------------------------------------------------------------------
// 00902920
// ---------------------------------------------------------------------------
float gun_bot_lead_error_span_00902920(float skill) noexcept
{
    // 00902B38: InterpolateClamped(0, 1.0, 6.0, 0.2, skill).
    return clamped_interpolate_00419010(0.0f, kLeadBotSpanAtSkillZero,
                                        kLeadBotSkillSpanEnd,
                                        kLeadBotSpanAtSkillEnd, skill);
}

float gun_bot_lead_error_limit_00902920(float distance) noexcept
{
    if (distance == 0.0f) {
        return 0.0f;
    }
    return kLeadBotErrorClampNumerator / distance; // 00902E5x, 25 / range
}

bool gun_bot_lead_fire_00902920(const LeadBotFireInputs& in) noexcept
{
    // 00902FEx: the aim must have been accepted and the range must be inside
    // nine tenths of the weapon's maximum.
    return in.aim_accepted && in.distance < in.max_range * kLeadBotRangeFraction;
}

// ---------------------------------------------------------------------------
// 008FFF20
// ---------------------------------------------------------------------------
bool gun_bot_heading_accepted_008fff20(float filtered_heading) noexcept
{
    // 009003C6 compares the result against the FLT_MAX double at 00D7A278.
    return filtered_heading != kGunBotFloatMax;
}

bool gun_bot_torpedo_ready_008fff20(float filtered_heading, float gun_horz,
                                    bool gun_can_fire) noexcept
{
    if (!gun_bot_heading_accepted_008fff20(filtered_heading)) {
        return false;
    }
    // 009003F8: SubtractWrappedAngle(bot+60h, gun+480h), absolute, one degree.
    const float delta = wrapped_angle_subtract_00438b10(filtered_heading, gun_horz);
    return std::fabs(delta) < kGunBotOneDegree && gun_can_fire;
}

// ---------------------------------------------------------------------------
// 00959C20 and 00954210
// ---------------------------------------------------------------------------
bool unit_gun_aim_accepts_device_00954210(UnitGunAimMessageKind kind,
                                          const UnitGunAimDeviceTests& tests) noexcept
{
    if (!tests.device_present || !tests.device_ready) { // 00954211, 0095421x
        return false;
    }
    switch (kind) {
    case UnitGunAimMessageKind::kAimAtPoint:
        return tests.weapon_sub_type == 1 || tests.weapon_sub_type == 5 ||
               tests.weapon_sub_type == 6;
    case UnitGunAimMessageKind::kAimAtPointWithTarget:
        return tests.weapon_sub_type == 1 || tests.weapon_sub_type == 5 ||
               tests.weapon_sub_type == 6 || tests.accepts_kind2_extra;
    case UnitGunAimMessageKind::kAimAtDirection:
        return tests.accepts_kind3;
    case UnitGunAimMessageKind::kAimAtHeading:
        return tests.weapon_sub_type == 7;
    case UnitGunAimMessageKind::kAimSubType9:
        return tests.accepts_kind5;
    default:
        return false; // 009542A7 XOR AL,AL
    }
}

float unit_gun_aim_trigger_window_00959c20(UnitGunAimMessageKind kind) noexcept
{
    switch (kind) {
    case UnitGunAimMessageKind::kAimAtPoint:
    case UnitGunAimMessageKind::kAimAtPointWithTarget:
    case UnitGunAimMessageKind::kAimSubType9:
        return kUnitGunAimWindowKind12; // 00D1A8A0
    case UnitGunAimMessageKind::kAimAtDirection:
        return kUnitGunAimWindowKind3;  // 00D0C26C
    case UnitGunAimMessageKind::kAimAtHeading:
        return kUnitGunAimWindowKind4;  // 00CEDF5C
    default:
        return 0.0f;
    }
}

// ---------------------------------------------------------------------------
// The three complete ticks as sequences
// ---------------------------------------------------------------------------
namespace {

// The three-float direction every vtable[100h] caller passes as its second
// argument, 00CE3D30 broadcast (008FFBB4, 008FFBBA, 008FFBC0).
std::array<float, 3> predict_direction_scalars()
{
    return {kGunBotPredictDirectionScalar, kGunBotPredictDirectionScalar,
            kGunBotPredictDirectionScalar};
}

std::array<float, 3> subtract(const std::array<float, 3>& a,
                              const std::array<float, 3>& b)
{
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

// The shared prologue of 008FFA20, 00902920, 009030C0 and 008FFF20. Returns
// false when the tick must stop before it aims.
bool run_shared_prologue(GunBotTickHost& host, float dt)
{
    host.resolve_fire_target_00521ea0();               // 008FFA2D
    if (!gun_bot_target_still_valid_008ffa20(host.target_validity())) {
        host.clear_fire_target_slot38();               // 008FFA6C
    }
    host.run_idle_timer_008fbce0(dt);                  // 008FFA85
    return host.side_enabled_00927f10();               // 008FFA99
}

} // namespace

void gun_bot_turret_tick_008ffa20(GunBotTickHost& host, TurretBotState& state,
                                  float dt)
{
    if (!run_shared_prologue(host, dt)) {
        return; // 008FFAA0 JZ 008FFF17
    }

    // 008FFAA8..008FFAE7: the director's artillery flag must be set.
    if (host.resolve_fire_target_00521ea0() != nullptr && host.gun_present() &&
        !host.director_artillery_flag_008ffac9()) {
        host.clear_fire_target_slot38();
    }
    if (!host.gun_present()) {
        return; // 008FFAE9
    }
    if (host.fire_target_entity_slot44() == nullptr) {
        return; // 008FFAFC
    }

    std::array<float, 3> target_origin = host.target_pose_origin(); // 008FFB30

    if (gun_bot_turret_recompute_due_008ffa20(state.aim_countdown, dt)) {
        // 008FFB87: the four skill scalars. The host supplies them because the
        // array base inside the class descriptor was not established.
        host.predict_lead_point_slot100(predict_direction_scalars(), 0.0f, 0.0f,
                                        0.0f, 0.0f); // 008FFBC6
        target_origin = host.target_pose_origin();   // 008FFBF0, re-sampled

        const std::array<float, 3> muzzle = host.muzzle_world_origin(); // 008FFBFD
        state.aim = host.angles_from_world_direction_008fdaf0(
            subtract(target_origin, muzzle)); // 008FFCAF, 008FFCBD

        // 008FFCD9 and 008FFD05: one draw per axis, degrees to radians.
        const float horz_draw = host.random_range_00bd2f10(0.0f, state.aim_error_degrees);
        state.aim.horz += horz_draw * kGunBotDegreesToRadiansNumerator /
                          kGunBotDegreesToRadiansDenominator;
        const float vert_draw = host.random_range_00bd2f10(0.0f, state.aim_error_degrees);
        state.aim.vert += vert_draw * kGunBotDegreesToRadiansNumerator /
                          kGunBotDegreesToRadiansDenominator;

        // 008FFD37: the next aim period.
        state.aim_countdown = host.random_range_00bd2f10(state.aim_period_min,
                                                         state.aim_period_max);
    }

    host.set_target_angles_0085aba0(state.aim); // 008FFD52, cached pair or not

    // 008FFD6E..008FFDB9: the distance from the muzzle to the target origin.
    const std::array<float, 3> muzzle = host.muzzle_world_origin();
    const float distance = host.vector_length_0042b2f0(subtract(target_origin, muzzle));

    // 008FFE16..008FFF03: the hysteresis, which reads the gun's current angles.
    const float horz_error =
        wrapped_angle_subtract_00438b10(host.gun_horz_angle(), state.aim.horz);
    const float vert_error =
        wrapped_angle_subtract_00438b10(host.gun_vert_angle(), state.aim.vert);
    const bool request = gun_bot_wants_fire_008ffa20(
        state.trigger.committed, distance, state.shoot_range, horz_error, vert_error);

    host.trigger_debounce_008fef40(request, dt); // 008FFF12
}

void gun_bot_ballistic_tick_009030c0(GunBotTickHost& host, float dt)
{
    if (!run_shared_prologue(host, dt)) {
        return; // 00903136's JZ
    }
    if (!host.gun_present() || host.fire_target_entity_slot44() == nullptr) {
        return; // 0090314x
    }

    // 00903154 and 00903163 choose which 00901C20 arm runs; both produce the
    // same outputs, so the host answers with the solved pair.
    host.gun_is_kind5_00903154();
    host.descriptor_flag_95h();
    BallisticBotFireInputs fire = host.solve_intercept_00901c20(); // 009031CF / 00903219

    GunAimAngles angles = host.ballistic_angles_009030c0(); // 0090326D, 0090327B
    angles.vert = gun_bot_ballistic_vertical_correction_009030c0(
        angles.vert, host.target_is_kind_0f());

    host.set_target_angles_0085aba0(angles); // 00903302

    fire.horz_delta = wrapped_angle_subtract_00438b10(host.gun_horz_angle(), angles.horz);
    fire.vert_delta = wrapped_angle_subtract_00438b10(host.gun_vert_angle(), angles.vert);

    // 00903410: the tail jump, which always runs, with the composed byte.
    host.set_trigger_slot1e8(gun_bot_ballistic_fire_009030c0(fire));
}

void gun_bot_muzzle_tick_006df520(GunBotTickHost& host, MuzzleBotErrorState& error,
                                  MuzzleBotDelayedFire& fire, float dt)
{
    host.resolve_fire_target_00521ea0(); // 006DF52C
    if (!gun_bot_target_still_valid_008ffa20(host.target_validity())) {
        host.clear_fire_target_slot38();  // 006DF54x
    }
    host.run_idle_timer_008fbce0(dt);     // 006DF563
    if (!host.side_enabled_00927f10()) {
        return;                           // 006DF577's inverted gate
    }
    if (!host.gun_present() || !host.has_fire_target_slot40()) {
        return; // 006DF585
    }

    // 006DF59x: the error period counts down and is re-rolled on expiry.
    error.countdown -= dt;
    if (error.countdown < 0.0f) {
        const float period = host.random_range_00bd2f10(kMuzzleBotErrorPeriodMin,
                                                        kMuzzleBotErrorPeriodMax);
        error.period = period;    // 006DF5D4
        error.countdown = period;
        host.reroll_error_envelope_006deff0(); // 006DF5E9
    }
    error.error = gun_bot_muzzle_error_006df520(host.error_envelope(), error.period,
                                                error.countdown);

    // 006DF694..006DF6D2: the offset walks toward its target at dt * 30.
    host.step_error_offset_0042ac60(dt * kMuzzleBotErrorStepRate);

    GunAimAngles commanded;
    const bool solved = host.solve_gravity_arc_00955630(commanded); // 006DFAD4
    host.store_aim_point_006dfae9();                                // 006DFAE9

    if (solved) {
        // 006DFB1C and 006DFB36: the per-bot errors, wrapped.
        commanded.horz = wrapped_angle_add_00438aa0(commanded.horz, error.error.horz);
        commanded.vert = wrapped_angle_add_00438aa0(commanded.vert, error.error.vert);
        const bool accepted = host.set_target_angles_0085aba0(commanded); // 006DFB54
        // The draw at 006DFBD6 happens only on the arming frame, so the gate is
        // evaluated before the host's random source is touched.
        if (accepted && !fire.armed &&
            gun_bot_angle_within_tolerance_006dee40(host.gun_horz_angle(),
                                                    commanded.horz,
                                                    kGunBotTenthDegree) &&
            gun_bot_angle_within_tolerance_006dee40(host.gun_vert_angle(),
                                                    commanded.vert,
                                                    kGunBotTenthDegree)) {
            const float draw = host.random_range_00bd2f10(0.0f, kMuzzleBotFireDelayMax);
            gun_bot_muzzle_arm_fire_006df520(fire, solved, accepted,
                                             host.gun_horz_angle(),
                                             host.gun_vert_angle(), commanded, draw);
        }
    }

    if (gun_bot_muzzle_release_fire_006df520(fire, host.muzzle_fire_inhibited(), dt)) {
        host.fire_now_slot1f0(); // 006DFC5x
    }
}

} // namespace bsp
