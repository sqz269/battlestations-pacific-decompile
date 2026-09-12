#pragma once
#include <array>

#include "bsp/ship_ai_sector_scan.hpp" // ShipAiObstacleNode, kShipAiSector* constants,
                                       // length_2d_00414c60, wrapped_angle_add_00438aa0

// 009EAE20 and 009EAFC0, the two box refreshes 009F0EA0 runs on every surviving
// neighbour node once a frame. Evidence, ABI and uncertainty:
// docs/SHIP_AI_NEIGHBOUR_BOX.md.
//
// docs/SHIP_AI_SECTOR_SCAN.md established the node layout from its *readers*
// (009D80C0, 009D8160, 009DD010, 009DD540, 009D84E0). These two routines are the
// *writers*, so they settle what each field means. Two names in
// ShipAiObstacleNode came out of the readers and read backwards against the
// producer; this header keeps the type and marks the fields at their uses rather
// than redeclaring the record. See the doc's Corrections.
//
//   +28h/+2Ch  the observed hull's FORWARD axis, cos/sin of its own heading
//              (009EAE9D, 009EAEA8). ShipAiObstacleNode calls it axis_beam_*.
//   +30h/+34h  the BEAM axis, (+2Ch, -(+28h)) (009EAEB8, 009EAEC0).
//              ShipAiObstacleNode calls it axis_forward_*.
//   +38h       the half extent along +28h, so the hull's half LENGTH plus the
//              lookahead (009EAF9C). ShipAiObstacleNode calls it near_half_beam.
//   +3Ch       the half extent along +30h, the hull's half BEAM (009EAFAB).
//              ShipAiObstacleNode calls it near_half_length.
//
// The two routines run back to back inside the ageing loop of 009F0EA0
// (009F104D, then 009F10FF), so 009EAFC0 always reads the pose 009EAE20 has just
// written for the same frame.

namespace bsp {

// ---------------------------------------------------------------------------
// Constants
// ---------------------------------------------------------------------------
// 009EAE20's two box scales and its lookahead cap.
inline constexpr double kShipAiNeighbourBoxLookaheadLengthFraction = 0.25;               // 00D7A348, 009EAEE1
inline constexpr double kShipAiNeighbourBoxHalfLengthScale = 0.550000011920929;          // 00CEC8F0, 009EAF92
inline constexpr double kShipAiNeighbourBoxHalfBeamScale = 0.60000002384185791;          // 00CEFF98, 009EAFA5

// 009EAFC0.
inline constexpr float  kShipAiNeighbourBoxNoBounds = -1000.0f;                          // 00D7A240, 009E5380 seeds, 009EB633 restores
inline constexpr float  kShipAiNeighbourBoxCollapsedExtent = 1.0f;                       // 00D7A24C, 009EB064, 009EB611
inline constexpr float  kShipAiNeighbourBoxMinClosingSpeed = 1.0f;                       // FLD1 at 009EB172
inline constexpr float  kShipAiNeighbourBoxMinTravel = 1.0f;                             // 00D7A24C, 009EB231
inline constexpr double kShipAiNeighbourBoxReferenceSpeedFraction = 0.05000000074505806; // 00D7A270, 009EB246
inline constexpr double kShipAiNeighbourBoxBeamShrinkGain = 6.0;                         // 00CE6628, 009EB31F
inline constexpr double kShipAiNeighbourBoxSpeedRatioBias = 1.0;                         // 00D7A210, 009EB38D
inline constexpr double kShipAiNeighbourBoxSpeedRatioScale = 0.5;                        // 00D7A280, 009EB393
inline constexpr double kShipAiNeighbourBoxMinProjectedTravel = 0.10000000149011612;     // 00D7A3A0, 009EB3F6
inline constexpr float  kShipAiNeighbourBoxTurnLimit = 1.5707963705062866f;              // 00CE3C64 / 00CE3CCC, 009EB40A
inline constexpr double kShipAiNeighbourBoxArcThreshold = 0.052359877559829890;          // 00D1A8A0, 3 degrees, 009EB466

// ---------------------------------------------------------------------------
// The node fields ShipAiObstacleNode does not carry
// ---------------------------------------------------------------------------
// The record is 0x90 bytes (operator new(0x90) at 009F0E2A, constructed by
// 009E52E0). These four are written by the two refreshes and are not in
// ShipAiObstacleNode, so they travel beside it rather than redeclaring it.
struct ShipAiNeighbourNodeMotion {
    // +40h, the observed hull's heading this frame. 009EAE48 is the only writer.
    // 009EAFC0 reads it at 009EB4D8 as the base of the arc projection, and the
    // near-box copy tail mirrors it into +64h at 009EB2FB.
    float heading_40{0.0f};

    // +64h, the heading the projected box is drawn at. 009EB4F0 writes it on the
    // arc arm, 009EB2FB on the copy tail; the straight arm (009EB472..009EB4CE)
    // leaves it at the previous frame's value. Not seeded by 009E52E0 and no
    // reader was located anywhere in the image.
    float projected_heading_64{0.0f};

    // +80h / +84h, the observed hull's cached world bounds on Y: maximum in
    // +80h, minimum in +84h (0098A8E0 writes minimum first, maximum second;
    // 009EB014 takes the second buffer's Y, 009EB022 the first). Seeded to
    // -1000.0f by 009E5380/009E5388 and restored to it at 009EB63B/009EB643.
    // Read only by 009EAFC0's own vertical-overlap gate at 009EB033/009EB049.
    float bounds_max_y_80{kShipAiNeighbourBoxNoBounds};
    float bounds_min_y_84{kShipAiNeighbourBoxNoBounds};
};

// ---------------------------------------------------------------------------
// 009EAE20, the near box refresh
// ---------------------------------------------------------------------------
// void __thiscall(node)(const float* self_velocity_xz), RET 4 at 009EAFB2, body
// 009EAE20-009EAFB4, read whole. Sole call site 009F104D in 009F0EA0. The stack
// argument is the caller's clamped hull velocity pair (009F1046 LEA ECX,
// [ESP+30h]; 009F104A PUSH ECX) and **no instruction in the body reads it**: the
// largest ESP displacement the body touches is +1Ch, the argument sits at +24h
// (+28h after the PUSH EDI at 009EAE82).
//
// What it refreshes is the observed ship's own oriented box, swept forward over
// a lookahead:
//
//   heading  = unit->vtable[50h]()                          ; 009EAE46
//   node+40h = heading                                      ; 009EAE48
//   speed    = 0092D730(unit+1018h)                         ; 009EAE54, signed
//   a        = pi/2 - heading ; if (a < 0) a += 2*pi         ; 009EAE60..009EAE7A
//   node+28h = cos(a) ; node+2Ch = sin(a)                    ; 009EAE9D, 009EAEA8
//   node+30h = node+2Ch ; node+34h = -0.0f - node+28h        ; 009EAEB8, 009EAEC0
//   advance  = min(settings+1A8h * speed, unit+9C8h * 0.25)  ; 009EAECA..009EAF07
//   if (unit+C8h == 0) 00414DB0(unit)                        ; 009EAF07, 009EAF30
//   node+20h = unit+FCh  + advance * node+28h                ; 009EAF7F
//   node+24h = unit+104h + advance * node+2Ch                ; 009EAF86
//   node+38h = unit+9C8h * 0.55 + |advance|                  ; 009EAF9C
//   node+3Ch = unit+9CCh * 0.60                              ; 009EAFAB
//
// `advance` is a `min`, not a clamp: a ship making sternway has a negative
// body-axis speed, the min keeps the negative product, the centre moves astern
// and +38h still grows by |advance| (009EAF62 AND 7FFFFFFFh). The cap is only
// ever reached going ahead.
struct ShipAiNeighbourNearBoxHost {
    virtual ~ShipAiNeighbourNearBoxHost() = default;

    // 009EAE46, CALL EDX through [[node+14h]]+50h. The concrete slot in the unit
    // vtable 00CFC3D0 is 006DFD60, `FLD dword ptr [ECX+1050h]; RET`: the hull
    // heading in the same convention 006BC0C0 folds.
    virtual float observed_heading_vtable50() = 0;

    // 009EAE54, CALL 0092D730 with ECX = [node+14h]+1018h. Signed: the velocity
    // projected on the body's third axis row, negative when going astern.
    virtual float observed_body_axis_speed_0092d730() = 0;

    // 009EAEC5, CALL 00424C40, then MOVSS from [EAX+1A8h] at 009EAECA.
    // ShipAvoidance.NearbyShip_PosSpeedCorrig, the lookahead in seconds.
    virtual float settings_pos_speed_corrig_1a8() = 0;

    // [node+14h]+9C8h, read twice (009EAEDB before the pose refresh, 009EAF8C
    // after). The hull's full length; see the doc's uncertainty 1.
    virtual float observed_hull_length_09c8() = 0;

    // [node+14h]+9CCh, read once at 009EAF9F. The hull's full beam.
    virtual float observed_hull_beam_09cc() = 0;

    // [node+14h]+C8h at 009EAF07, the pose-valid byte 00414DB0 tests the same
    // way. False means the world matrix has to be rebuilt first.
    virtual bool observed_pose_valid_00c8() = 0;

    // 009EAF30, CALL 00414DB0 with ECX = node+14h. Only reached when
    // observed_pose_valid_00c8() is false.
    virtual void refresh_observed_pose_00414db0() = 0;

    // [node+14h]+FCh and +104h at 009EAF35/009EAF45, the world translation, read
    // after the refresh above. Two methods would let a host read them before it,
    // which the native cannot, so it is one call returning the pair.
    virtual std::array<float, 2> observed_world_position_xz_00fc() = 0;
};

// Returns false when the entry gate 009EAFC6..009EAFD7 rejects the node, in
// which case nothing at all is written (009EAE2B / 009EAE3B jump straight to the
// epilogue at 009EAFAE).
bool ship_ai_neighbour_near_box_refresh_009eae20(ShipAiObstacleNode& node,
                                                 ShipAiNeighbourNodeMotion& motion,
                                                 ShipAiNeighbourNearBoxHost& host);

// ---------------------------------------------------------------------------
// 009EAFC0, the avoid box refresh and the side flag
// ---------------------------------------------------------------------------
// The ten settings rows the routine reads through the `ADD EDI,180h` base at
// 009EB081/009EB087. Names and loader sites: docs/SHIP_AI_SETTINGS_BLOCK.md
// section 2; this packet only re-checked the displacements.
struct ShipAiNeighbourAvoidSettings {
    float arrive_time_min_1a0{0.0f};             // [EDI+20h], 009EB18C
    float arrive_dist_min_1a4{0.0f};             // [EDI+24h], 009EB0BA
    float est_pos_dist_limit_mul_1ac{0.0f};      // [EDI+2Ch], 009EB340
    float est_pos_min_ship_length_1b0{0.0f};     // [EDI+30h], 009EB334
    float est_pos_ship_length_limit_mul_1b4{0.0f}; // [EDI+34h], 009EB350
    float est_pos_ship_spd_mul_1bc{0.0f};        // [EDI+3Ch], 009EB20F
    float est_pos_size_dec_mul_1c0{0.0f};        // [EDI+40h], 009EB28F
    float est_pos_size_dec_min_1c4{0.0f};        // [EDI+44h], 009EB288
    float go_away_spd_add_1d0{0.0f};             // [EDI+50h], 009EB167
};

// The nine stack arguments, in push order. 009F0EA0 builds them at
// 009F10A0..009F10FA; the slots are named from that listing, not from their use
// here.
struct ShipAiNeighbourAvoidBoxInputs {
    float self_x{0.0f};           // arg0, 009F10EE  <- [blk+3FCh]+FCh  (009F0ECB)
    float self_z{0.0f};           // arg1, 009F10FA  <- [blk+3FCh]+104h (009F0EE2)
    float self_velocity_x{0.0f};  // arg2, 009F10C0  <- the floored hull velocity
    float self_velocity_z{0.0f};  // arg3, 009F10D2

    // arg4, 009F10E8 <- [blk+3FCh]+9C8h * 0.55 (009F0FCB, the double 00CEC8F0).
    float self_half_length{0.0f};

    // arg5, 009F10DD <- [blk+3FCh]+9CCh * 0.75 (009F0FE3, the double 00CEC9D8).
    // **Never read.** The body touches no epoch-B [ESP+44h]; see the doc's
    // Corrections.
    float self_half_beam_unread{0.0f};

    float self_bounds_max_y{0.0f}; // arg6, 009F10CA <- blk+1BCh (009F10BA)
    float self_bounds_min_y{0.0f}; // arg7, 009F10B4 <- blk+1C0h (009F10A4)

    // arg8, 009F10B0 PUSH EDX of the byte at [ESP+18h]. The inlined party filter
    // 009EC770 at 009F1052..009F1092 sets it. 009EB02A tests the low byte only.
    bool avoidance_accepted{false};
};

// The result the two callers of the projection can observe without reading the
// node back: which arm of 009EAFC0 ran.
enum class ShipAiNeighbourAvoidArm {
    owner_gone,        // 009EAFD9, node+68h and +69h both set, nothing else written
    no_vertical_overlap, // 009EB611
    near_box_copy,     // 009EB2C9, the avoid box is the near box
    straight,          // 009EB472, the projection is a line along +28h/+2Ch
    arc                // 009EB4D1, the projection rides a turning circle
};

struct ShipAiNeighbourAvoidBoxHost {
    virtual ~ShipAiNeighbourAvoidBoxHost() = default;

    // 009EAFED and 009EB005, CALL EDX through [[node+14h]]+20h, twice. The
    // concrete slot in vtable 00CFC3D0 is 006D1E30, `MOV EAX,[ECX+360h]; RET`.
    // The first call is the presence test (009EAFF1 TEST EAX,EAX), the second
    // supplies the `this` the bounds copier reads. Returning null on the second
    // call is native-invalid.
    virtual const void* observed_model_vtable20() = 0;

    // 009EB009, CALL 0098A8E0 with ECX = the second model pointer, RET 8. Six
    // x87 copies out of [model+13Ch..144h] into `minimum` and
    // [model+148h..150h] into `maximum`; it refreshes nothing.
    virtual void observed_world_bounds_0098a8e0(const void* model,
                                                std::array<float, 3>& minimum,
                                                std::array<float, 3>& maximum) = 0;

    // 009EB05F, CALL 00424C40. EDI is written once, at 009EB081/009EB087, and
    // the only POP EDI before 009EB334 is on a path 009EB2B7 skips, so every
    // [EDI+disp] in the body is in this singleton's +180h epoch.
    virtual ShipAiNeighbourAvoidSettings settings_ship_avoidance_180() = 0;

    // 009EB0ED, CALL 00419260 with ECX = the two-float delta. Returns
    // 1/length, which the body multiplies both components by.
    virtual float reciprocal_length_00419260(const std::array<float, 2>& delta) = 0;

    // 009EB119, CALL EDX through [[node+14h]]+34h with one out-pointer, RET 4.
    // The concrete slot in vtable 00CFC3D0 is 00812090: the body axis
    // (unit+94h, +98h, +9Ch) scaled by 0092D730(unit+1018h), returned in the
    // caller's buffer. Only [0] and [2] are read back (009EB11B, 009EB125).
    virtual std::array<float, 3> observed_velocity_vtable34() = 0;

    // 009EB1FE and 009EB259, CALL 0092D730 with ECX = [node+14h]+1018h, twice.
    // Signed at the first site (it drives the travel's sign), taken absolute at
    // the second (009EB269).
    virtual float observed_body_axis_speed_0092d730() = 0;

    // 009EB241, CALL 0080FC30 with ECX = node+14h: unit+9C0h scaled by the
    // difficulty factor. Multiplied by 0.05 at 009EB246.
    virtual float observed_reference_speed_0080fc30() = 0;

    // [node+14h]+9C8h at 009EB337, the same full hull length 009EAE20 reads.
    virtual float observed_hull_length_09c8() = 0;

    // 009EB428, CALL 00811940 with ECX = node+14h: unit+984h through 00811890.
    // Negated at 009EB42D before it is scaled by the projection time.
    virtual float observed_command_yaw_rate_00811940() = 0;

    // 009EB4F6, CALL 006BC0C0 with ECX = the out pair and the heading on the
    // stack, RET 4: out[0] = cos(fold(pi/2 - h)), out[1] = sin(fold(...)). The
    // same kernel 009EAE20 inlines at 009EAE83..009EAE99.
    virtual std::array<float, 2> heading_to_direction_006bc0c0(float heading) = 0;
};

// void __thiscall(node)(nine stack dwords), RET 24h at 009EAFE5, 009EB305,
// 009EB4CE, 009EB60E and 009EB64F; bytes 009EAFC0-009EB651, read whole.
// Sole call site 009F10FF in 009F0EA0.
//
// Ghidra splits the arc arm 009EB4D1-009EB610 into a separate zero-caller
// function FUN_009EB4D1, so 009EAFC0's Ghidra body excludes it. It is one arm of
// one routine: 009EB470 JBE is the only way in, 009EB5FA..009EB60E is 009EAFC0's
// own epilogue, and the frame only balances across the two. The projection keeps
// it inline; the doc's Coverage table records both addresses.
ShipAiNeighbourAvoidArm ship_ai_neighbour_avoid_box_refresh_009eafc0(
    ShipAiObstacleNode& node,
    ShipAiNeighbourNodeMotion& motion,
    const ShipAiNeighbourAvoidBoxInputs& inputs,
    ShipAiNeighbourAvoidBoxHost& host);

// ---------------------------------------------------------------------------
// The pure rules, exposed so the doc's numbers can be checked without a host
// ---------------------------------------------------------------------------
// 009EAECA..009EAF07: the lookahead the near box centre is pushed along its own
// forward axis, and which +38h grows by. `min`, not `clamp`.
float ship_ai_neighbour_lookahead_009eaeca(float settings_pos_speed_corrig_1a8,
                                           float body_axis_speed,
                                           float hull_length_09c8) noexcept;

// 009EB14D..009EB182: the closing speed the arrival times divide by. The dot of
// the relative velocity on the unit vector from the node's near centre to the
// observer, plus ShipAvoidance.NearbyShip_GoAwaySpdAdd, floored at 1.0f.
float ship_ai_neighbour_closing_speed_009eb14d(const std::array<float, 2>& relative_velocity,
                                               const std::array<float, 2>& direction_to_self,
                                               float go_away_spd_add_1d0) noexcept;

// 009EB277..009EB2AB: the factor both avoid-box extents are scaled by. Above
// zero it shrinks the box the further ahead the projection reaches; at or below
// zero 009EAFC0 gives up and sets node+68h.
float ship_ai_neighbour_extent_shrink_009eb277(float body_axis_speed,
                                               float reference_speed_0080fc30,
                                               float near_half_length_38,
                                               float projection_time,
                                               float est_pos_size_dec_mul_1c0,
                                               float est_pos_size_dec_min_1c4) noexcept;

// 009EB334..009EB399: the cap on how far ahead the other ship is projected.
float ship_ai_neighbour_travel_limit_009eb334(float slack,
                                              float hull_length_09c8,
                                              float self_speed,
                                              float closing_speed,
                                              const ShipAiNeighbourAvoidSettings& settings) noexcept;

} // namespace bsp
