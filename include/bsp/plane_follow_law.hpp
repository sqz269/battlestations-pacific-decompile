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

}  // namespace bsp

#endif  // BSP_PLANE_FOLLOW_LAW_HPP
