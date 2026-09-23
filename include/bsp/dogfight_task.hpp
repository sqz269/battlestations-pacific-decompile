#pragma once

// The dogfight bot task (kind 2), packet cc9_dogfight_task. docs/DOGFIGHT_TASK.md.
//
// ADDRESSES.  Factory 009AB570; derived constructor 009A9810
// (BSP_BotTaskDogfight_Construct), which runs 009F9980 (step 9) before the
// 007B8AD0 flight-leader test (step 6); the state objects are built by
// 009A94E0 with ESI = task+3F8h; the per-tick arm is primary vtable 00D1F9B0
// slot +64h = 009AB1C0 (no Ghidra function):
//
//   009AB1D1  task+4F8h = FFh
//   009AB1DB  009AAC70(approach = task+3F8h, dt)    ; reads AttackDist (+644h)
//   009AB1EA  009AAFA0(task, dt)                     ; the state transitions
//   009AB202  state(task+310h)->vtable[0Ch](dt)      ; the state tick
//   009AB20A  task+2E4h = task+4F8h
//
// The states (009A94E0, ESI = task+3F8h; names from 009A6A60):
//   moveto     +510h  009C2CA0 -> vtable 00D20B24, tick 009C18C0 (the generic
//                      moveto tick), speed slot +1Ch 009C1BC0
//   follow     +54Ch  009C2980 BSP_BotStateFollow_Construct (tick 009C1FD0)
//   prepare    +5E4h  009C2980, a second follow object
//   aim        +67Ch  vtable 00D1F8D8, tick 009A76E0
//   maneuver   +6A4h  009A84E0
//   attackrun  +6E4h  009A70E0
//   avoid_roll +708h  vtable 00D1F918, tick 009A7E80
//   avoid_turn +730h  vtable 00D1F938, tick 009A80E0

namespace bsp {

// 007EEC50's dogfight class (include/bsp/attack_commands.hpp kAttackCmdDogfight).
inline constexpr unsigned int kDogfightCommandClass = 0x00E08F58u;

enum class DogfightState : int {
    kNone = 0,
    kMoveTo,     // +510h
    kFollow,     // +54Ch
    kPrepare,    // +5E4h
    kAim,        // +67Ch
    kManeuver,   // +6A4h
    kAttackRun,  // +6E4h
    kAvoidRoll,  // +708h
    kAvoidTurn,  // +730h
};
inline constexpr int kDogfightStateCount = 9;
const char* dogfight_state_name(DogfightState s) noexcept;

// 009AAFA0's engagement test, used on both of its arms:
//   task+4C8h != 0  ||  (unit+370h == 2  &&  task+4C4h != 0)
bool dogfight_engaged_009aafa0(bool latch_4c8, int control_mode_370,
                               bool target_4c4) noexcept;

// 009AAFA0's arm for a task that is not engaged: moveto for the flight leader
// (007B8AD0 true), follow for a wing member. From moveto/follow it is the
// fall-through at 009AB19x; from any other state it is the 007B8AD0 test after
// 009AB0xx. Returns the state the task is in afterwards.
DogfightState dogfight_unengaged_state_009aafa0(bool is_flight_leader) noexcept;

// STAND-IN, labelled: the moveto state's tick 009C18C0 flies the glide the
// cruise profile 009AAF30 sets up; this host has not read the dogfight speed
// slot 009C1BC0 nor bound 009AAF30. What stands in is its end state: head at the
// commanded target (009F9E40) and pitch toward Pilot/Dogfight/CruisingAlt with
// the image's own point pitch law 009F9ED0, over at least 250 m, so the command
// is an elevation angle capped by the class climb, never a bang-bang cap.
struct DogfightMoveToInputs {
    float own_pos[3] = {0.0f, 0.0f, 0.0f};
    float target_pos[3] = {0.0f, 0.0f, 0.0f};
    float cruising_alt = 1400.0f;       // singleton+640h
    float min_distance = 250.0f;        // the fly-to arm's FollowedPointDist floor
    float class_climb_angle_1e4 = 0.0f; // desc+1E4h
};
struct DogfightMoveToCommand {
    float heading = 0.0f;        // plan+2C0h, mode 2
    float pitch = 0.0f;          // plan+2BCh, mode 2
    float horizontal_range = 0.0f;
};
DogfightMoveToCommand dogfight_moveto_standin(const DogfightMoveToInputs& in) noexcept;

// ===========================================================================
// The engaged half, packet cc9_dogfight_engaged. docs/DOGFIGHT_ENGAGED.md.
// Reconstructed and build-tested; each rule names its addresses. Names are
// hypotheses, not recovered symbols.
// ===========================================================================

// The pilot robots row the approach reads through approach+14h
// (= 00F8A30C + level * 248h + 0Ch, so row+N here is robot_config.hpp's
// suffix N+0Ch). SUBSTITUTION, labelled, as the dive-bomb binding does: the
// PilotBot registry is out of this host's reach, so the values are this
// installation's scripts/datatables/robots.lua SPNormal pilot row (lines
// 554-692).
struct DogfightPilotRow {
    float aim_shoot_distance = 850.0f;       // row+230h, aim_shoot_distance_23c
    float follow_dist = 300.0f;              // row+20Ch, dogfight_follow_dist_218
    float boring_time = 18.0f;               // row+210h, dogfight_boring_time_21c
    float avoid_time = 2.5f;                 // row+214h, dogfight_avoid_time_220
    float maneuver_change_time = 15.0f;      // row+21Ch, dogfight_maneuver_change_time_228
};

// 009AA630 (called only from 009AAC70), one candidate's score. The target
// squadron is approach+CCh (= task+4C4h); its members are +3D0h[0..4] in
// order, the scan stops at the first null, and a member counts only when it is
// live (+5Ch set, +5Dh/+5Eh/+60h clear) and inside the map (0071C4F0). With
// fewer than two members (+3CCh < 2) 009AA630 takes +3D0h[0] unscored.
//   local = the candidate in the shooter's frame (004142E0 with unit+110h),
//           z forward.
//   R = ShootDistance * 0.8 (00CE3D40, double)
//   range = d < R ? interp(100, 0.5, 0.8R, 1.0, d)          (00CE3D08, 00CE3800)
//                 : interp(1.2R, 1.0, 3R, 0.25, d)          (00CEC160, 00D7A2B0, 00CE3868)
//   behind = local.z < 0 ? 0.1 : 1.0                        (00D7A2F0)
//   angle = interp(0.25, 1.0, 1.5, 0.2, |(x, y) / max(z, 1)|) (00CE3868, 00CE380C, 00CE54A0)
//   score = angle * range * behind * 0.7^wingmates           (00CEFFA0, double)
// where `wingmates` counts own-squadron members other than self whose current
// pilot target (007BBC10: [unit+DF4h]+98h+[00F876B8]*1Ch) is the candidate.
// A candidate that is not the current target (approach+B4h) is multiplied by
// 00BD2F10(0.8, 1.0) (00CE74F8); the caller supplies that draw. The highest
// score strictly above 0 wins; none wins -> +3D0h[0].
struct DogfightCandidate {
    float local[3] = {0.0f, 0.0f, 0.0f};
    float distance = 0.0f;        // 3-D, 0042B2F0
    int wingmates_on_it = 0;
    bool is_current_target = false;
};
float dogfight_range_score_009aa630(float distance, float shoot_distance) noexcept;
float dogfight_angle_score_009aa630(const float local[3]) noexcept;
float dogfight_target_score_009aa630(const DogfightCandidate& c, float shoot_distance,
                                     float noncurrent_draw) noexcept;

// 009AAC70, the approach update (approach = task+3F8h), in its order:
//   009AAC76  no target squadron (+CCh == 0): latch +D0h = 0, +D4h = 9999.0
//             (00CE4C04) and return.
//   009AAC9D  timer +E4h -= dt; +E8h += dt.
//   009AAD10  009AA630 re-selects when the target (+B4h) is null or not live,
//             or when the timer went negative AND the PREVIOUS distance +D4h
//             exceeds ShootDistance + 200.0 (00CE4D70, double). 009AA630 re-arms
//             the timer to 00BD2F10(1.0, 3.0) (00CE3854).
//   009AAD60  +D4h = |aim - own| (3-D); 009AAD78 +D8h = the same with y = 0.
//   009AAD83  with a target: latched = d < AttackDist(tuning+644h) *
//             approach+24h + (was latched ? 150.0 (00CE3808) : 0), a STRICT
//             compare (FCOMIP/JBE). Latched: +70h = clamp(d / 007C2610(), 0,
//             30.0) (00CE7630 double / 00CE38C8), 007C2610 being the smallest
//             muzzle speed ([desc+34h]+50h) over the unit's kind-21h parts.
//             Not latched or no target: +D0h = 0, +70h = 0.
//   009AAE35  009FADA0 (the target reference), then +ECh..+F4h = the aim point
//             in the shooter's frame and +F8h/+FCh = x/max(z,1), y/max(z,1).
bool dogfight_needs_reselect_009aac70(bool target_live, float timer_after_dt,
                                      float previous_distance,
                                      float shoot_distance) noexcept;
struct DogfightLatch {
    bool latched = false;         // approach+D0h = task+4C8h
    float time_to_target = 0.0f;  // approach+70h
};
DogfightLatch dogfight_latch_009aac70(bool has_target, float distance,
                                      float attack_dist, float ratio_24,
                                      bool was_latched,
                                      float min_muzzle_speed) noexcept;
// +F8h/+FCh: the off-axis tangents, divided by max(z, 1.0) (00D7A24C).
void dogfight_off_axis_009aac70(const float local[3], float out_xy[2]) noexcept;

// 009AAFA0, the transitions, read from the LISTING (009AAFA0-009AB1B8, RET 4).
// ENG = +4C8h || (squadron+370h == 2 && +4C4h != 0). squadron+370h is
// [task+404h]+370h, the SQUADRON's attack mode (unit+9D4h), not the unit's.
//   moveto/follow: ENG -> 009A9D90; else 007B8AD0 ? moveto : follow.
//   other, !ENG:   007B8AD0 ? moveto : follow.
//   other, ENG:    mode 0 -> prepare.
//                  prepare -> 009A9D90 (009AB030-009AB03C; the earlier doc
//                  missed this edge).
//                  not aim and 009AAA80 true -> aim.
//                  attackrun: +4C8h -> maneuver, zeroing +6BCh (dword) and
//                             +6D8h (float) first.
//                  aim: +6A0h (aim+24h) -> 009A9970's avoid pick;
//                       else 009A98F0 (aim+18h > aim+1Ch) -> 009A8560 on the
//                       maneuver object, then maneuver.
//                  maneuver: 009A9BD0 -> aim.
//                  avoid_roll/avoid_turn: timer +720h/+748h < 0 -> 009A86F0 on
//                             the maneuver object, then maneuver.
// 009A9D90, the engage entry: mode 0 -> prepare; else +4C8h -> maneuver
// (zeroing +6BCh/+6D8h) else attackrun.
// 009A9D50 switches only on a change: old->vtable[8], then new->vtable[4].
struct DogfightTransitionInputs {
    DogfightState current = DogfightState::kNone;
    bool latch_4c8 = false;
    int squadron_mode_370 = 0;
    bool target_squadron_4c4 = false;
    bool is_flight_leader = false;
    bool aaa80 = false;             // 009AAA80's result
    bool aim_too_close_6a0 = false; // aim+24h
    bool aim_bored_98f0 = false;    // 009A98F0
    bool maneuver_on_target_9bd0 = false;  // 009A9BD0
    float avoid_timer = 0.0f;       // +720h or +748h for the current avoid state
    bool avoid_pick_turn = false;   // 009A9970's draw chose avoid_turn
};
struct DogfightTransition {
    DogfightState next = DogfightState::kNone;
    bool reset_maneuver_6bc_6d8 = false;  // attackrun -> maneuver, 009A9D90
    bool maneuver_from_aim_8560 = false;  // 009A8560 before the switch
    bool maneuver_from_avoid_86f0 = false; // 009A86F0 before the switch
};
DogfightTransition dogfight_transition_009aafa0(const DogfightTransitionInputs& in) noexcept;

// 009A9BD0 (maneuver -> aim): approach+F4h (local z) > 1.0 and
// |(+F8h, +FCh)| < 0.8 (00CE3D40, double); a squared length at or below
// 00CE3820 (1e-10, double) counts as 0.
bool dogfight_on_target_009a9bd0(float local_z, float tan_x, float tan_y) noexcept;

// 009A75C0 (aim enter, no Ghidra function; 009A75C0-009A7641, RET):
//   aim+18h = 0; aim+1Ch = 00BD2F10(0.8, 1.4) * BoringTime   (00CE74F8, 00D06874)
//   aim+20h = 00BD2F10(0.5, 0.8) * FollowDist * approach+24h (00CE3800, 00CE74F8)
//   aim+24h = aim+25h = 0.
struct DogfightAimState {
    float bored_18 = 0.0f;
    float bored_limit_1c = 0.0f;
    float too_close_20 = 0.0f;
    bool too_close_24 = false;
    bool head_on_25 = false;
};
DogfightAimState dogfight_aim_enter_009a75c0(const DogfightPilotRow& row, float ratio_24,
                                             float draw_boring, float draw_close) noexcept;

// 009A76E0 (aim tick, 009A76E0-009A79B5), with a target:
//   aim+25h = local z > 1.0 && dot(own vt[34h], target vt[34h]) < 0 (head-on).
//   head-on && d(+D4h) < aim+20h -> aim+24h = 1 (sticky until the next enter).
//   rate = the gun controller's +48h byte ? -4.0 (00CF1430)
//        : d < ShootDistance ? interp(0.25, -1.0, 2.0, 2.0, |(+F8h,+FCh)|) : 0
//   aim+18h = max(0, aim+18h + rate * dt).
//   heading 009F9E40 to the aim point (approach vt[0] = approach+48h);
//   pitch 009F9ED0(aimY - ownY, approach+D8h).
//   not head-on: plan+2B0h = 1, +2D8h = 1, +2B4h = (d - FollowDist) + target
//                vt[38h] (its speed).
//   head-on: 007B4ED0(interp(aim+20h, 0.3, ShootDistance, 1.0, d)), unread.
//   gun controller (approach+1Ch)+40h = tuning+678h Angle_Strafe; approach+DCh
//   = 0; approach+E0h = 1.0; a target-squadron plane at gun+74h that is not
//   the current target becomes the target (009A7650).
struct DogfightAimInputs {
    float distance = 0.0f;       // approach+D4h
    float tan_len = 0.0f;        // |(+F8h, +FCh)|
    float local_z = 0.0f;        // approach+F4h
    bool opposing = false;       // the dot product < 0
    bool gun_locked_48 = false;  // (approach+1Ch)+48h
    float target_speed = 0.0f;
    float dt = 0.0f;
};
struct DogfightAimCommand {
    bool speed_from_target = false;  // plan+2B0h/+2D8h = 1
    float desired_speed = 0.0f;      // plan+2B4h when speed_from_target
    float head_on_fraction = 0.0f;   // 007B4ED0's argument otherwise
};
DogfightAimCommand dogfight_aim_tick_009a76e0(DogfightAimState& st, const DogfightAimInputs& in,
                                              const DogfightPilotRow& row) noexcept;

// 009A8560 (aim -> maneuver): maneuver+18h = 0; maneuver+34h =
// 00BD2F10(0.3, 1.1) * ShootDistance (00CE69C8, 00CE6448).
float dogfight_maneuver_pursuit_range_009a8560(float shoot_distance, float draw) noexcept;

// 009A7DE0 / 009A8020 (avoid enters, no Ghidra functions): the timer +18h =
// 00BD2F10(0.75, 1.5) * AvoidTime (00CEE07C, 00CE380C); the ticks (009A7E80,
// 009A80E0) call 009A7A50(dt) first. 009A9970 picks avoid_turn when
// 00BD2F10(0, wRoll + wTurn) > wRoll, the weights being the two states'
// vtable+1Ch slots 009A7F70 and 009A83B0.
float dogfight_avoid_timer_009a7de0(float avoid_time, float draw) noexcept;

// STAND-IN, labelled: the maneuver tick 009A8B20 (009A8B20-009A92D4) is a
// three-sub-mode machine (+18h: 0 heading servo, 1 roll to the bank +1Ch,
// 2) that is only partly read. What stands in is its mode-0 arm, which the
// read part shows: heading mode 2 at the bearing to the target and pitch mode
// 2 toward the point `+20h` metres up over `+24h` metres, where +20h = aimY -
// ownY and +24h = approach+D8h while d <= 3 * ShootDistance
// (00D7A2B0, double), else +20h = min(Dynamics/Ceiling (tuning+210h) - (ownY +
// 50.0 (00CE3938, double)), 300) (00CE3CA8 double / 00CE3AE8) over +24h = 500
// (00CE397C).
// Avoid STAND-IN, labelled: the avoid ticks are unread past their first call;
// the stand-in holds the heading 90 degrees off the target bearing, turning
// the way the unit already banks, level, until the timer runs out.
struct DogfightSteer {
    float heading = 0.0f;
    float pitch = 0.0f;
};
DogfightSteer dogfight_maneuver_standin(const float own_pos[3], const float aim[3],
                                        float horizontal_range, float shoot_distance,
                                        float ceiling_210, float class_climb_angle) noexcept;


// ===========================================================================
// The task's gun controller, packet cc9_dogfight_gun. docs/DOGFIGHT_GUN.md.
// ===========================================================================
//
// approach+1Ch is task+314h (009F9980), a plain object built by 009FAAD0 and
// ticked by 009FC7C0(sub, dt) from BSP_PilotBot_Update at 00999979, after the
// task arm and only while unit+C24h is set (00999962). Fields used here:
//   +28h search arm (-1.0 each tick end), +2Ch 0.4 (00CE7804), +30h =
//   ShootDistance + 650.0 (00D1F3A0, double), +34h ShootDistance, +38h =
//   max(AimDistortAngle1 * 3.0, 0.08) (0099C864), +3Ch 0 (ctor), +40h the
//   strafe cone a state writes (zeroed each tick end), +44h the range,
//   +48h fire, +49h steer, +4Bh burst on, +4Ch hold timer, +50h burst timer,
//   +5Ch lead point, +68h desired direction (own forward when unset),
//   +74h the auto-selected target (007B96F0 -> [unit+C50h] 007E2090).
//
// The fire decision, read from the pseudocode and checked on the listing for
// the two gates (007BA760: unit+C3Ah or +5Dh set blocks; 007B96D0: unit+C50h
// non-null and 007DEDB0 true re-aims instead):
//   d = |lead - own| > 1 and d < max(+30h, +34h + 200.0 (00CE4D70));
//   envelope = +4Ch > 0, or (lateral < d * +38h and 1 < z < +34h and
//              (lateral > +3Ch (/ 1.8, 00D049A8, with an auto target) or
//               lateral > 2 * +0Ch * +08h * d));
//   steer (009F9FC0) when (1 - cos(lead, +68h)) < +40h and +4Ch <= 0;
//   fire when envelope && !007BA760 && !007B96D0 && (+4Bh || +50h < 0):
//   +48h = 1 and task+2E0h (= plan+2DCh, the gunFire request) = 1.
// Bursts: +50h -= dt every tick; with +4Bh clear, +50h < 0 and a fire this
// tick, +4Bh = 1 and +50h = U(row+234h, row+238h) (AimShootTime, {4.5, 3.0});
// with +4Bh set and +50h < 0, +4Bh = 0 and +50h = U(row+23Ch, row+240h)
// (AimShootDelayTime, {1.6, 0.7}).
struct DogfightGunState {
    bool fire_48 = false;
    bool burst_4b = false;
    float burst_timer_50 = 0.0f;
    float hold_4c = 0.0f;
    int bursts = 0;         // census: rising edges of +4Bh
    int fire_ticks = 0;     // census
};
struct DogfightGunInputs {
    bool has_target = false;     // +74h (a finder substitute in this host)
    float lead_local[3] = {0.0f, 0.0f, 0.0f};  // the lead point in the unit frame
    float shoot_distance = 850.0f;   // +34h
    float search_range_30 = 1500.0f; // +30h
    float lateral_cap_38 = 0.09f;    // +38h
    float dont_shoot_3c = 0.0f;      // +3Ch
    bool unit_disabled = false;      // 007BA760
    bool finder_busy = false;        // 007B96D0
    float burst_draw = 3.75f;        // U(3.0, 4.5) at its midpoint
    float delay_draw = 1.15f;        // U(0.7, 1.6) at its midpoint
    float dt = 0.0f;
};
// Returns true when this tick raises the gunFire request.
bool dogfight_gun_tick_009fc7c0(DogfightGunState& st, const DogfightGunInputs& in) noexcept;

// 007B4ED0(plan, f): throttle slot +278h = clamp(f, 0, 1) and air-brake slot
// +2A8h = clamp(-f, 0, 1), both active (+27Ch, +2ACh = 1), speed mode +2D8h = 0.
// The aim tick's head-on arm and the maneuver tick's tail (f = 1.0 at
// 009A9270) both write this shape.
struct DogfightThrottle {
    float throttle = 0.0f;
    float air_brake = 0.0f;
};
DogfightThrottle dogfight_throttle_007b4ed0(float f) noexcept;

// ===========================================================================
// The unit+C50h neighbour object and its finder, packet cc9_plane_gunfire.
// docs/PLANE_GUNFIRE.md section 3.
// ===========================================================================
//
// 007D621F stores unit+C50h = 007E1E20(unit), a 0xCC-byte object, for a plane
// when [[00E188A8]+1FE4h] is 0 or 1 and unit+54h is not the local player's
// ([[00E188A8]+5FCh]+908h). It keeps three lists: +30h/+34h (everything near,
// the probe 007F0280's), +50h/+54h (enemy aircraft, the finder's) and
// +70h/+74h (friendly aircraft). Its update 007E2010 (vtable 00D06B88 +0Ch,
// no Ghidra function) adds dt to three clocks +7Ch/+80h/+84h and, when +7Ch >=
// +88h (3.0, 00CE3854), calls the refresh 007E11D0 and subtracts 3.0.
// The clocks start at U(0, P) + P for P = 3.0, 1.0 (+80h) and 2.0 (+84h).
//
// 007E11D0: R = (+88h + 3.0 (00D7A2B0)) * 180.0 (00CE3D20, double) = 1080;
// enemy radius max(R, 1200.0 (00CFD714)); friendly radius R, floored at 500
// (00CE3840 / 00CE397C). It drops list members at or beyond their radius, then
// walks the world list ([[00E188A8]+19CCh]+58h) for units other than the owner
// that answer vtable[5Ch](6) or (0Fh), not already listed, within the enemy
// radius: an aircraft (0Fh) of another side (+54h) goes to +50h; one of the
// same side within the friendly radius goes to +70h; anything within R also
// goes to +30h.
struct PlaneNeighbourRadii {
    float enemy_plane = 1200.0f;
    float friendly_plane = 1080.0f;
    float near_any = 1080.0f;
};
PlaneNeighbourRadii plane_neighbour_radii_007e11d0(float refresh_period_88) noexcept;

// 007DEEC0(obj, candidate), the finder's score, with the parameters 007E2090
// stored: +B8h cone (tangent), +BCh range, +C0h inner-cone scale, +C4h near
// ramp. local = the candidate in the owner's frame. 0 outside 1 < z < +BCh or
// outside the cone; else
//   inner = min(+C0h * 0.75 (00CEC9D8), +B8h * 0.5 (00D7A280))
//   ramp  = z <= +C4h ? interp(1, 0, +C4h, 1, z) : interp(+BCh, 0.4, +C4h, 1, z)
//   score = interp(+B8h^2, 0, inner^2, 1, tan^2) * ramp
// and, for a candidate above the owner (dy > 0), times
// interp(0.3, 1.0, 2.0, 0.2, dy / max(1, horizontal)) (00CE69C8, 00CE3958, 00CE54A0).
struct PlaneFinderParams {
    float cone_b8 = 0.4f;
    float range_bc = 1500.0f;
    float inner_c0 = 0.09f;
    float near_c4 = 255.0f;
};
float plane_finder_score_007deec0(const PlaneFinderParams& p, const float local[3],
                                  float dy_above, float horizontal) noexcept;

}  // namespace bsp
