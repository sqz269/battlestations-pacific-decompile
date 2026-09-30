// The plane FOLLOW law: what a wing member is commanded to do while it is
// following its flight leader.  Packet `cc8_follow_law`.
//
// ADDRESSES.  The follow tick is `009C1FD0 BSP_BotStateFollow_Tick`, vtable
// 00D20AB8 slot +Ch, `RET 4` on one float (dt).  It runs three bodies in a
// fixed order:
//
//     009C1FEA  CALL 009BFD70   station point; AL=0 means "I am the leader",
//                               and the tick then ends (009C1FF1 JZ 009C234E)
//     009C2068  CALL 009BFEE0   the GEOMETRY step (1795 instructions)
//     009C2077  CALL 009BEE30   the COMMANDING step, handed the tick's own dt
//                               (009C206D FLD [ESP+74h] / 009C2074 FSTP [ESP])
//
// A CORRECTION THIS PACKET OWES.  docs/PLANE_FORMATION.md section 6 and the
// packet brief both describe 009BFEE0 as "the law that FLIES a member to its
// station - the throttle, the heading and the GoodPosition gates".  It is not.
// 009BFEE0 commands nothing.  Its fifteen distinct callees are every one of
// them a math or pose primitive -- 00414DB0 EntityPose_RefreshWorld x13,
// 0042CF10 Geometry_AsinClamped x7, 0042BE90 x4, 00BF7030 CRT sqrt x3,
// 00438B10 Math_SubtractWrappedAngle x3, 00415510/00415550 Min/MaxFloatByRef,
// 00419510 Vector3f_Normalize, 00414C60 Vector2f_LengthWithCutoff, 00419010
// Math_InterpolateClamped, 00419260, 0042B2F0 -- and no heading, altitude or
// speed helper appears anywhere in its body.  It is a pure producer: it writes
// a single float3 STEER POINT at `state+44h/48h/4Ch` and nothing else that
// leaves the object.  009BEE30, listed until now as an unread "third step", is
// the body that issues every command.
//
// THE CONTRACT BETWEEN THEM is that one float3.  009BEE30 opens its
// out-of-position arm with `LEA EBP,[ESI+44h] / PUSH EBP / MOV ECX,EDI /
// CALL 009F9E40` (009BF9EA-009BF9F0), so the steer point 009BFEE0 computed is
// literally the point the pilot is told to head at.
//
// THE GATE is `state+85h`, written by 009BFD70 (009BFDD4, 009BFE95, 009BFEA2,
// 009BFEB1) from the `Pilot/Follow` GoodPositionDist / GoodPositionDir pair.
// BOTH bodies branch on it with the same polarity:
//
//     009BFEEE  CMP byte [ESI+85h],0 / JZ 009C0026   geometry: hold vs fly-to
//     009BEE49  CMP byte [ESI+85h],0 / JZ 009BF9EA   commands: hold vs fly-to
//
// so "in good position" selects the hold arm in the geometry and in the
// commanding in lockstep.  What this header binds is the FLY-TO arm of
// 009BEE30 (009BF9EA-009BFD38), the regime that governs a member catching up
// to its station and a spent bomber in `done` or `prepare`, which reach this
// same tick through 009C7270 and 009D2720.
//
// Names are hypotheses, not recovered symbols.  Field offsets are the image's.
#ifndef BSP_PLANE_FOLLOW_LAW_HPP
#define BSP_PLANE_FOLLOW_LAW_HPP

namespace bsp {

// Inputs to 009BEE30's fly-to arm, each at the offset the listing reads it
// from.  `state` is the follow-state object (ESI); `block` is the cached
// `Pilot/Follow` tuning block at `state+6Ch`; the pose fields are the unit's
// world matrix, which 009BEE30 refreshes through 00414DB0 before each read.
//
// THE BLOCK BASE.  `state+6Ch` points INTO the game tuning singleton at its
// `+380h`, so `block+NNh` is `singleton+(380h + NNh)`.  That is fixed by two
// offsets docs/BOMBER_AFTER_TASK.md already established from their use --
// block+14h GoodPositionDir and block+18h GoodPositionDist -- landing exactly
// on singleton+394h and +398h, which docs/GAME_TUNING_SINGLETON.md names as
// those same two keys.  Every offset below is named through that base, so the
// authored values are this installation's own
// (scripts/datatables/planeglobals.lua, mtime 2024-10-29 12:54:18).
struct PlaneFollowFlyToInputs {
    // state+30h/34h/38h, the station point 007F23A0 produced.  Note +34h is
    // its Y: the tail of 009BFEE0 writes it at 009C17F1.
    float station[3] = {0.0f, 0.0f, 0.0f};
    // state+44h/48h/4Ch, the steer point 009BFEE0 produced.  +48h is its Y and
    // is the one the tail re-clamps against the leader (009C1811/27/3B).
    float steer_point[3] = {0.0f, 0.0f, 0.0f};
    // pose+FCh/100h/104h, the unit's own world position.
    float unit_pos[3] = {0.0f, 0.0f, 0.0f};
    // pose+ECh and pose+F4h, the two horizontal components 009BFC26-009BFC3B
    // dots against the direction to the station.
    float unit_forward_x = 0.0f;
    float unit_forward_z = 0.0f;
    // 009BFC58 `CALL [[state+2Ch]]+38h`, a virtual on the leader observer at
    // state+2Ch taking no stack argument and returning a float in ST0.  The
    // leader's speed: it is the y0 of the distance ramp, i.e. what a member
    // standing exactly on its station is told to fly.
    float leader_speed = 0.0f;
    // 009BFC41 `CALL 007C47F0 BSP_PlaneClass_LevelFlightSpeed(classDesc)`,
    // then 009BFC49 `FMUL double [00D7A390]` = 0.9 (read as a double).
    float level_flight_speed = 0.0f;
    // classDesc+188h, the floor 009BFC7D compares the leader speed against.
    float class_min_speed = 0.0f;
    // block+18h, GoodPositionDist.  Established in docs/BOMBER_AFTER_TASK.md
    // from its use at 009BFE21; this installation authors 100.
    float good_position_dist = 0.0f;
    // block+24h and block+28h, the two endpoints of the alignment ramp at
    // 009BFD04/009BFCF5: Pilot/Follow/MaxFollowSpdTargetDir and
    // MinFollowSpdTargetDir, authored DEG(30) and DEG(100).
    //
    // A REAL ASYMMETRY, not a misreading: the ramp's INPUT is a cosine
    // (009BFC3B dots two horizontal unit vectors) while these endpoints are
    // radians.  Since a cosine never exceeds 1 and the high endpoint is 1.745,
    // the y1 end is unreachable: even a perfectly aligned member lands at
    // (1 - 0.524) / (1.745 - 0.524) = 0.39 of the way from the distance ramp
    // toward level flight speed.  The arithmetic below is the image's; this
    // note records that the image's own units disagree here.
    float align_ramp_lo = 0.0f;
    float align_ramp_hi = 0.0f;
    // block+4Ch = singleton+3CCh, which docs/GAME_TUNING_SINGLETON.md row
    // `+3cc` records as DERIVED: 007E908D copies it from `+330`
    // Dynamics/SpdMultipliers/TurboMultiplier.  So the far end of the distance
    // ramp is the leader's speed on TURBO -- a member more than
    // GoodPositionDist from its station is told to use the turbo multiplier to
    // close, which is what makes the ramp a catch-up law rather than a trim.
    float catchup_speed_scale = 0.0f;
    // block+00h = singleton+380h, Pilot/Follow/FollowedPointDist, authored 250
    // in this installation.  009BFBD0 floors the distance handed to 009F9ED0
    // at it.
    float min_command_dist = 0.0f;
};

struct PlaneFollowFlyToCommand {
    bool produced = false;
    // Horizontal ranges 0042B2F0 computes at 009BFA42 and 009BFAA7.
    float distance_to_steer_point = 0.0f;
    float distance_to_station = 0.0f;
    // 009BFAAC-009BFB02: lerp(stationY, steerY, min(dStation/dSteer, 1)).
    float commanded_altitude = 0.0f;
    // 009BFC0C-009BFC1E, the first argument of 009F9ED0: commanded minus own.
    float altitude_error = 0.0f;
    // 009BFBD0's max(distance_to_station, block+00h), the second argument.
    float command_distance = 0.0f;
    // 009BFC3B, own forward dotted with the unit direction to the station.
    float alignment_dot = 0.0f;
    // 009BFCD1's result, the distance ramp between the leader's speed and the
    // catch-up speed.
    float distance_ramp_speed = 0.0f;
    // 009BFD0F `FSTP [EBX+2B4h]`, the field this host already carries as
    // GameUnitSlot::plane_desired_speed_2b4.
    float desired_speed_2b4 = 0.0f;
};

// 009BEE30's fly-to arm, 009BF9EA-009BFD38.  Pure: no globals, no allocation.
// Original ABI: `__thiscall void(this = ESI, float dt)`, RET 4 at 009BFD65;
// dt is not used by the arithmetic below, which is why it is not a parameter.
// The heading command itself is 009F9E40's, whose body this does not
// reconstruct -- the caller steers at `steer_point`, which is the argument the
// image passes.
PlaneFollowFlyToCommand plane_follow_flyto_command_009bee30(
    const PlaneFollowFlyToInputs& in) noexcept;

// 009BFA1B-009BFB02 on its own, so the altitude blend can be tested and read
// without the speed law.  t is clamped ABOVE at 1.0 only (009BFAC5-009BFAE1
// compares against [00D7A24C] = 1.0f); there is no lower clamp in the image,
// and a dSteer of zero reaches an FDIV the listing does not guard.
float plane_follow_blended_altitude_009bfaac(float station_y, float steer_y,
                                             float distance_to_station,
                                             float distance_to_steer) noexcept;

// ---------------------------------------------------------------------------
// 009BFEE0, the GEOMETRY step: the producer of the steer point.
//
// THE REFERENCE DIRECTION, and it is the key to the whole body.  009C00C8
// calls unit virtual slot +50h on the LEADER (state+2Ch) for its heading, and
// 009C01B6 calls the SAME slot on the own unit (009C0026 `[[ESI+4]+4]`).  That
// slot is the atan2-over-pose-row-2 heading getter this host already documents
// at the 00835AC0 latch in src/game_hosts_units.cpp.  Then
//
//     009C00F7  T   = InterpolateClamped(block+44h, block+3Ch,
//                                        block+48h, block+40h, R)
//     009C0109  G   = 007D7DA0(leader+0AB0h) * T
//     009C0128  ref = SubtractWrappedAngle(leaderHeading, G)
//
// and block+3Ch/+40h/+44h/+48h are singleton +3BCh/+3C0h/+3C4h/+3C8h, which
// docs/GAME_TUNING_SINGLETON.md names `Pilot/Follow/LeaderHeadingSpdTime/1,2`
// = 0.5, 4.0 and `.../LeaderHeadingSpdDist/1,2` = 100, 500.  So T is a TIME
// ramped by the member's range to its station, and `ref` is the leader's
// heading LAGGED by it: a member 500 m out of position follows the track its
// leader held 4 s ago, one at 100 m the track of 0.5 s ago.  The tuning key's
// own name says the same thing, which is why this is a reading and not a
// guess.  `leader_turn_rate` below is 007D7DA0's result; its body is unread,
// and its dimension (rad/s) is fixed by this use.
//
// THE FRAME.  009C0139 is `_CIatan2` in its x87-argument form (no stack
// adjustment), giving atan2(dz, dx) over `own - station`; 009C0142-009C0164
// rewrites it as wrap(pi/2 - that) into [0, 2pi), i.e. the compass bearing
// station->aircraft, since this image's heading convention is
// direction = (sin h, cos h).  With A0 = wrap(bearing - ref):
//
//     009C018D  V     = R * sin(A0)    the CROSS-TRACK offset, +ve to the
//                                      right of the lagged track
//     009C01A9  along = R * cos(A0)    the ALONG-TRACK offset
//     009C01CE  A     = wrap(ownHeading - ref)   the heading error
//
// PHASE A, 009C0251-009C0EE0, read whole (packet cc9_follow_approach_arm,
// docs/PLANE_FOLLOW_PHASE_A.md section 8).  It is a turn planner: from the
// frame above and a turn model (w = TurnMul * classDesc+270h, the class turn
// rate 007DB4D0 stores; r = classDesc+18Ch TravelSpeed / w) it plans the turns
// that bring the member onto the lagged track behind its moving station, and
// leaves exactly two values for the dispatch: the regime byte BL and a TIME
// (base-0Ch), which 009C0F00 turns into the leader's distance flown during the
// plan (time * the leader's horizontal speed).  The BL values:
//
//   1                 lead pursuit       009C08CD, 009C0BC7
//   2 / 4             abeam, left/right  009C08F5 / 009C0814, reached from
//                                        009C0800 (V >= 0 -> 2) and from
//                                        009C08E7 (V >= 0 -> 4): the two joins
//                                        test the V-sign byte base-21h with
//                                        OPPOSITE polarity
//   (V<0 ? 2 : 4)|8   the turn-circle    009C0ECB-009C0EDF
//                     lead-in
//
// Checked against the image's own bytes: tools-free emulation of 009BFEE0 on
// 5000 sampled geometries matches every output of the fly-to arm (section 8).
// One branch, 009C0829-009C0861 plus 009C08DF (the quadrant-1 "second arc is
// shorter" exit), was never reached by the samples and is transcribed from
// the listing only.
//
// THE THREE REGIMES, all read from the listing:
//
//   lead pursuit   BL&1     009C107B-009C123C, then JMP 009C16C0
//   abeam          !BL&8    009C15C0-009C16CF, entered by JE at 009C1247
//   turn circle    BL&8     009C124D-009C15BB: the point L = time * speed
//                           ahead of the station along the lagged track, a
//                           circle of radius r beside it (side by the V sign),
//                           the tangent from the aircraft to that circle
//                           (004F4840), and the steer point at least
//                           FollowedPointDist along that tangent.  It jumps to
//                           the tail and does not copy +44h..4Ch to +60h..68h.
//
// THE TAIL, 009C16D2-009C1846, then clamps BOTH the station Y and the steer Y
// into one leader-relative band; see section 5.9 of docs/PLANE_FOLLOW_LAW.md.
// The floor is max(min(leaderY + LeaderFollowAlt, state+88h), leaderY - 120)
// (009C1734-009C175A, [00D1F3F8] = 120.0 double); the old binding had only the
// min.  The lower bound is applied when `run_phase_a` is set, so the switch-off
// path keeps its band.
struct PlaneFollowGeometryInputs {
    // The own unit: pose +FCh/+100h/+104h, and virtual slot +50h.
    float own_pos[3] = {0.0f, 0.0f, 0.0f};
    float own_heading = 0.0f;
    // The own pose row 2 (+ECh/+F0h/+F4h): the abeam regime's nose, through
    // 00419260 at 009C15F1.
    float own_forward[3] = {0.0f, 0.0f, 1.0f};
    // state+30h/34h/38h, the station 007F23A0 produced.
    float station[3] = {0.0f, 0.0f, 0.0f};
    // The leader, state+2Ch: pose +FCh/+100h/+104h, virtual slot +50h, and
    // pose row 2 +ECh/+F0h/+F4h (its forward basis).
    float leader_pos[3] = {0.0f, 0.0f, 0.0f};
    float leader_heading = 0.0f;
    float leader_forward[3] = {0.0f, 0.0f, 0.0f};
    // 007D7DA0(leader+0AB0h) at 009C0109, in rad/s.  SUBSTITUTION when the
    // caller has no rate: zero makes `ref` the leader's instantaneous heading,
    // which is the lag law's own zero-turn-rate limit.
    float leader_turn_rate = 0.0f;
    // The `Pilot/Follow` block at state+6Ch = singleton+380h.
    float followed_point_dist = 250.0f;     // block+00h
    float leader_heading_time_1 = 0.5f;     // block+3Ch
    float leader_heading_time_2 = 4.0f;     // block+40h
    float leader_heading_dist_1 = 100.0f;   // block+44h
    float leader_heading_dist_2 = 500.0f;   // block+48h
    // The tail's band, 009C16E8-009C17B9.  Floor L = min(leaderY + block+04h,
    // state+88h); ceiling = min(singleton+210h, leaderY + 120.0).
    float band_floor_offset = 0.0f;         // block+04h
    float state_88 = 0.0f;                  // state+88h
    float band_ceiling_210 = 0.0f;          // singleton+210h
    // False leaves both Y values unclamped and says so, rather than inventing
    // a band out of defaults.
    bool band_inputs_available = false;

    // Phase A.  When set, the image's selector picks the regime and the
    // `regime` argument is ignored.
    bool run_phase_a = false;
    // The leader's vtable +34h, 007BBB70: the world velocity unit+AC8h..AD0h.
    // Phase A uses its horizontal length (009C025E-009C02A9).
    float leader_velocity[3] = {0.0f, 0.0f, 0.0f};
    // 009C02CF / 009C02DE: the own unit answers vtable +5Ch(10h) or +5Ch(16h)
    // (the 0047B850 pair) -> block+10h LargePlaneTurnMul, else block+0Ch
    // SmallPlaneTurnMul.
    bool own_large_turn_class = false;
    float small_plane_turn_mul = 1.1f;      // block+0Ch
    float large_plane_turn_mul = 1.1f;      // block+10h
    // classDesc = [[state+4]+8]: +270h, the turn rate 007DB4D0 stores, and
    // +18Ch TravelSpeed.
    float class_turn_rate_270 = 0.0f;
    float class_travel_speed_18c = 0.0f;
};

// Which of the three regimes produced the point.  `kLeadPursuit` is what the
// switch-off host always asks for; with `run_phase_a` Phase A decides.
enum class PlaneFollowRegime { kLeadPursuit, kAbeam, kOffsetPoint009c1328 };

// Phase A's inputs, each the frame slot it reads.
struct PlaneFollowPhaseAInputs {
    float heading_error = 0.0f;   // base-1Ch, A
    float cross_track = 0.0f;     // base+0Ch, V
    float along_track = 0.0f;     // base+10h
    int quadrant_bl = 0;          // BL from 009C01D3-009C024F, 1..4
    float leader_speed = 0.0f;    // base-08h after 009C029E
    float inv_turn_rate = 0.0f;   // base-18h, 1 / w
    float turn_radius = 0.0f;     // base-20h, r
};

struct PlaneFollowPhaseA {
    int bl = 0;          // BL at 009C0EE1
    float time = 0.0f;   // base-0Ch at 009C0EE1, seconds
};

// 009C0251-009C0EE0 from its inputs.  Pure.
PlaneFollowPhaseA plane_follow_phase_a_009c0251(
    const PlaneFollowPhaseAInputs& in) noexcept;

// 004F4840 -> 004F4430 -> 004F3810 (packet cc9_pilot_moveto_task part 4): the
// two tangent points of the circle (c, r) seen from q.  004F3810 returns the
// chord midpoint m = c + (r*r/d) u and b = h (-u.z, u.x), h = r sqrt(d*d -
// r*r) / d, u = (q - c) / d; 004F4430 writes out0 = m + b, out1 = m - b.
// False, outs untouched, unless r < d (004F3887).  xz pairs.
bool circle_tangent_points_004f4840(const float c[2], float r, const float q[2],
                                    float out0[2], float out1[2]) noexcept;

struct PlaneFollowGeometry {
    bool produced = false;
    // state+44h/48h/4Ch after the tail.
    float steer_point[3] = {0.0f, 0.0f, 0.0f};
    // state+34h after the tail, which is the station Y 009BEE30 then blends
    // from.  The tail writes it at 009C17F1.
    float station_y = 0.0f;
    PlaneFollowRegime regime = PlaneFollowRegime::kLeadPursuit;
    // The reading above, exposed so a caller can log it rather than re-derive.
    float reference_heading = 0.0f;   // 009C0128
    float lag_time = 0.0f;            // 009C00F7
    float cross_track = 0.0f;         // 009C018D, V
    float along_track = 0.0f;         // 009C01A9
    float heading_error = 0.0f;       // 009C01CE, A
    int quadrant_bl = 0;              // 009C01D3-009C024F, Phase A's input
    float range_horizontal = 0.0f;    // 009C00A2, R
    float range_3d = 0.0f;            // 009C10A9, D
    bool band_applied = false;
    // Phase A (only with run_phase_a).
    int phase_a_bl = 0;               // BL at 009C0EE1
    float phase_a_time = 0.0f;        // base-0Ch at 009C0EE1
    float lead_distance = 0.0f;       // base-0Ch after 009C0F00
    float turn_radius = 0.0f;         // base-20h
    // The auxiliary state fields the arm writes.  +5Ch is -1 (009C0EF4)
    // except in the turn-circle regime (r, 009C1262); +50h..58h the circle
    // centre (x, own Y, z), turn-circle regime only; +60h..68h a copy of the
    // steer point (009C16C0), or in the turn-circle regime the tangent point
    // with the own Y.  Consumers not identified.
    float state_50[3] = {0.0f, 0.0f, 0.0f};
    bool state_50_written = false;
    float state_5c = -1.0f;
    float state_60[3] = {0.0f, 0.0f, 0.0f};
};

// 009BFEE0's fly-to arm, 009C0026-009C16D1 plus the tail 009C16D2-009C1846.
// Original ABI: `__thiscall void(this = ESI)`, RET 0 at 009C1846; every output
// is a field of the follow state, which this returns by value instead.  Pure:
// the only global it reads are the four never-written bit constants at
// 00E0E2F8-00E0E2FB, which are folded into the branch structure here.
// Without `in.run_phase_a`, `regime` selects which of the three the caller
// wants and the lead distance stands in as D (the switch-off binding).
PlaneFollowGeometry plane_follow_geometry_009bfee0(
    const PlaneFollowGeometryInputs& in, PlaneFollowRegime regime) noexcept;

}  // namespace bsp

#endif  // BSP_PLANE_FOLLOW_LAW_HPP
