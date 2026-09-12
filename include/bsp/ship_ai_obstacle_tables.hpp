#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/ship_ai_states.hpp"        // ShipAiControlBlock, ShipAiSteeringMode,
                                         // ShipAiThrottleDirection
#include "bsp/ship_ai_throttle_ring.hpp" // ShipAiRudderLawHost, the rudder law, the hop

// The middle of 009F3F80: the obstacle sectors, the reverse-manoeuvre arms and
// the throttle ceiling.
//
// docs/SHIP_AI_THROTTLE_TO_RING.md projected the head of `BSP_ShipAi_DriveOrderRing`
// (009F3F80, body 009F3F80-009F4D06, `this` = blk) and its tail, the hop into the
// order ring. Everything between them, 009F40CA..009F4B98, was one record. This
// header projects that record: what the routine does to blk+1D0h (desired
// throttle) and blk+1D4h (desired rudder) before the hop reads them.
//
// The shape of the middle, in the routine's own order:
//
//   1. 009F40CA..009F4166  four booleans out of the latched direction blk+35Ch,
//                          the committed direction blk+364h, blk+36Ch and the
//                          hull's signed speed.
//   2. 009F4168..009F4392  the danger level blk+0A84h: a ramp of the clearance
//                          ratio blk+37Ch / unit+9CCh, stepped toward through
//                          two dwell timers (blk+0A88h, blk+0A8Ch), then a load
//                          latch raised on the unit.
//   3. 009F4394..009F44B2  the throttle: zero when stopped, otherwise
//                          009EC7C0's ceiling, limited by blk+344h, then snapped
//                          by the 65-bin profile at blk+4h (009D6B40).
//   4. 009F44B3..009F4500  the sector direction index, then the rudder law
//                          009DA250 when the steering mode is not Rudder.
//   5. 009F4502..009F487C  the obstacle sectors: the rudder is limited by
//                          blk+348h, a bucket of the rudder picks one of twelve
//                          sectors at blk+808h, and a blocked sector steers to
//                          the sector's avoidance bearing and either reverses
//                          the throttle band or arms the one-second astern
//                          latch blk+380h.
//   6. 009F487D..009F4B98  the escape manoeuvre: when every inner sector on the
//                          current side is blocked, the committed direction
//                          blk+364h is flipped on a dwell timer so the hull
//                          rocks ahead and astern out of the block.
//
// The two "2Ch-stride tables at blk+81Ch and blk+848h" of the earlier packet are
// one array: twelve 0x2C-byte sectors based at blk+808h, so blk+81Ch is
// sector[0]+14h and blk+848h is sector[1]+14h, the same field of the next
// sector. See docs/SHIP_AI_OBSTACLE_TABLES.md, "Corrections".
//
// The producer of the sectors is 009EB660, reached once per sector from
// 009EF230 (chain slot at 009F51FA, immediately before 009F4DA0 at 009F5248,
// which tail-calls 009F3F80). 009EF230 refreshes three of the twelve sectors
// per frame on the round-robin counter blk+0A18h and 009EB660 fills the rest of
// each record. Neither is reconstructed here.
//
// Every offset and constant below carries the address it was read at. Names are
// hypotheses, not recovered symbols. These are semantic interfaces for MSVC
// Win32, not drop-in binary replacements.

namespace bsp {

// ---------------------------------------------------------------------------
// The obstacle sector array at blk+808h
// ---------------------------------------------------------------------------
// 009F45B0 IMUL EAX,EAX,0x2C and 009F45B3 CMP byte ptr [EAX + ESI + 0x81C],0:
// the index is ((direction_index * 6) + bucket) and the stride is 0x2C. The
// fields below are the ones 009F3F80 reads (009F45B3, 009F4653, 009F4662,
// 009F4669) and the ones the producer writes (009EF230 at 009EF32F, 009EB660 at
// 009EB6A3, 009EB6A7, 009EBECC, 009EBEDB, 009EBF6E, 009EBF83, 009EC0DE,
// 009EC1A3). Offsets are relative to
// the record; the record base in blk is kShipAiObstacleSectorBase.
inline constexpr std::uint32_t kShipAiObstacleSectorBase = 0x808u;   // 009F45B3, 0x81C - 0x14
inline constexpr std::uint32_t kShipAiObstacleSectorStride = 0x2Cu;  // 009F45B0, 009F4650
inline constexpr int kShipAiObstacleGroupSize = 6;                   // 009F45A8 LEA EDX,[EDX+EDX*2]; ADD EDX,EDX
inline constexpr int kShipAiObstacleSectorCount = 12;                // two groups of six

// The neighbour the AI keeps in the list at blk+608h (count blk+604h). Only the
// fields this packet read; `009EB660` picks the record and 009F3F80 and
// 009D8B90 read it. The two nested objects are named after the tests, not after
// any recovered symbol.
struct ShipAiNeighbourOwner {
    bool gone_5e{false};      // +5Eh, 009F474D and 009D8BA3: the record is skipped when set
    bool avoidance_184{false}; // +184h, 009F4768: the speed test runs only when set
};

struct ShipAiNeighbourRecord {
    ShipAiNeighbourOwner* owner{nullptr}; // +14h, 009F4740, 009D8B96
    float x{0.0f};                        // +44h, 009D8BDE
    float z{0.0f};                        // +48h, 009D8BE8
    float forward_x{0.0f};                // +54h, 009D8BCA
    float forward_z{0.0f};                // +58h, 009D8BD5
    bool disabled_68{false};              // +68h, 009D8BB1
    int pass_side_88{0};                  // +88h, 009F475B: the side this neighbour picked
};

// 009EB660's output for one sector. `blocked` is the byte at +14h that
// 009F3F80 tests as blk+81Ch / blk+848h.
struct ShipAiObstacleSector {
    std::uint8_t kind{0};                     // +0h,  009EB6B4, 009EB9B3, 009EBF20
    float half_width{0.0f};                   // +4h,  009EB69F, 009EB979
    float braking_distance{0.0f};             // +8h,  009EF32F writes, 009EB783 reads
    float reach{0.0f};                        // +0Ch, 009EB7B1, 009EB862
    float lateral{0.0f};                      // +10h, 009EB6B7, 009EB9BC
    bool blocked{false};                      // +14h, 009EB6A3 clears, 009EBECC sets
    float hit_x{0.0f};                        // +18h, 009EBF6E
    float hit_z{0.0f};                        // +1Ch, 009EBF7A
    float avoid_bearing{0.0f};                // +20h, 009EC1A3, blk+828h at 009F4653
    ShipAiNeighbourRecord* blocker{nullptr};  // +24h, 009EBEDB, blk+82Ch at 009F4662
    int pass_side{0};                         // +28h, 009EBF83 (1 or 2), blk+830h at 009F4669
};

// ---------------------------------------------------------------------------
// 009D6B40: the 65-bin throttle profile at blk+4h
// ---------------------------------------------------------------------------
// float* __thiscall(blk+4h)(float* value, float low, float high), RET 0Ch, body
// 009D6B40-009D6D57, complete. Both call sites pass `this` = blk+4h
// (009F44A7 and 009F4878 LEA ECX,[ESI+4]; 009E10D9 ADD ESI,4), so the profile is
// one object per control block. The bytes are a cost profile over the throttle
// axis: bin i is the throttle i * 0.0625 - 2.0 (009D6CE0 FILD, FMUL 00CEF290,
// FSUBRP against the 2.0 at 00D7A308), so bin 0 is -2.0 and bin 64 is +2.0.
// A zero byte is the free bin; the routine walks outward from the bin the
// current value lands in and moves the value to the cheapest reachable bin.
inline constexpr int kShipAiThrottleProfileBins = 65;              // 009D6CCD MOV ECX,0x40, plus bin 64
inline constexpr double kShipAiThrottleProfileScale = 16.0;        // 00CED9F8, 009D6B87
inline constexpr double kShipAiThrottleProfileStep = 0.0625;       // 00CEF290, 009D6CE8
inline constexpr double kShipAiThrottleProfileOrigin = 2.0;        // 00D7A308, 009D6B7F
inline constexpr double kShipAiThrottleProfileRound = 0.5;         // 00D7A280, 009D6B8F

struct ShipAiThrottleProfile {
    std::array<std::uint8_t, kShipAiThrottleProfileBins> bin{};
    bool bypass_41{false}; // this+41h, 009D6B4F: when set the routine only clamps
};

// 009D6B87..009D6B99 and the two repeats at 009D6BA0 and 009D6BAF: the bin index
// is (int)(value * 16.0 + 2.0 * 16.0 + 0.5) computed as ((value + 2.0) * 16.0 +
// 0.5) truncated by 00BF7420. Not clamped by the routine; the caller's window
// does that.
int ship_ai_throttle_profile_index_009d6b40(float value) noexcept;

// 009D6CE0..009D6CF0, the inverse: bin * 0.0625 - 2.0.
float ship_ai_throttle_profile_value_009d6b40(int bin) noexcept;

// The whole of 009D6B40. Returns the new value; `value` is the routine's
// in/out first argument.
//   - this+41h set             -> clamp into [low, high] only          (009D6B5D)
//   - low >= high              -> (low + high) * 0.5, then the clamp   (009D6B63)
//   - bin[clamped current] == 0 -> the value is left alone             (009D6BDF)
//   - otherwise the search below, then the clamp into [low, high].
float ship_ai_apply_throttle_profile_009d6b40(const ShipAiThrottleProfile& profile,
                                              float value, float low, float high) noexcept;

// ---------------------------------------------------------------------------
// blk+0A84h, the danger level, and its two dwell timers
// ---------------------------------------------------------------------------
// 009F4168..009F41A7: the target is a ramp of the clearance ratio
// blk+37Ch / unit+9CCh through 00419010, InterpolateClamped(1.0, 1.0, 4.0, 0.0,
// ratio): at or below one half-width of clearance the danger is 1.0, at four it
// is 0.0. blk+37Ch is written by 009EF910, which 009F4D10 calls one chain slot
// earlier; unit+9CCh is the half-width docs/SHIP_AI_ORDER_CONSUMER.md reads.
inline constexpr float kShipAiDangerClearanceMin = 1.0f; // 009F4199 FLD1
inline constexpr float kShipAiDangerClearanceMax = 4.0f; // 00CE3D34, 009F418F
// 009F41CD and 009F424E: the target has to leave a +/-0.05 band before either
// dwell timer starts.
inline constexpr double kShipAiDangerBand = 0.05;        // 00D7A270
// 009F41FE and 009F42C8: rising, the timer at blk+0A88h fills at 2/s and the
// step is 2 * dt; falling, blk+0A8Ch fills at 1/s and the step is dt; inside the
// band the step is 2 * dt and both timers drain at 1/s.
inline constexpr double kShipAiDangerRiseRate = 2.0;     // 00D7A308, 009F41DB
inline constexpr float kShipAiDangerArmed = 1.0f;        // 00D7A24C, 009F41B1
// 009F4337: the danger also raises the unit's turn-assist load latch.
inline constexpr float kShipAiDangerLoadFloor = 0.01f;   // 00D7A238
inline constexpr float kShipAiLoadLatchHigh = 1.5f;      // 00CE380C
inline constexpr float kShipAiLoadLatchLow = 0.5f;       // 00CE3800

// The blk fields of the middle that ShipAiControlBlock does not carry. Offsets
// are relative to blk = brain+8h.
struct ShipAiObstacleState {
    // The sectors and the profile.
    std::array<ShipAiObstacleSector, kShipAiObstacleSectorCount> sector{}; // +808h
    ShipAiThrottleProfile profile{};                                      // +4h
    // The danger level and its dwell timers.
    float danger_a84{0.0f};      // +0A84h, 009F41AB, the value 009EC7C0 is given
    float danger_rise_a88{0.0f}; // +0A88h, 009F41E3
    float danger_fall_a8c{0.0f}; // +0A8Ch, 009F420C
    float clearance_37c{0.0f};   // +37Ch,  009F416E, written by 009EF910
    // The limits 009F4DA0 writes one step earlier (brain+34Ch and brain+350h).
    float throttle_limit_344{1.0f}; // +344h, 009F43F4, 009F4448
    float rudder_limit_348{1.0f};   // +348h, 009F4525, 009F452D
    float published_33c{0.0f};      // +33Ch, 009F4514, published by 009F4D10
    // The astern latch and the escape manoeuvre. The dwell timer blk+368h and
    // the flip marker blk+374h are ShipAiControlBlock::timer_368 and
    // ::direction_value_374; this struct does not duplicate them.
    float backoff_timer_380{-1.0f}; // +380h, 009F47A7 sets 1.0f, the head counts down
    // +384h. ship_ai_states.hpp declares the same field as
    // `int direction_counter_384`; that type is wrong. Every writer is a float
    // store (009F3FE3 MOVSS, 009F47EE FSTP, 009F4AFF MOVSS) and 009F489E
    // compares it against the 10.0f at 00CE38B8. Use this one on this path and
    // see docs/SHIP_AI_OBSTACLE_TABLES.md, "Corrections".
    float stall_time_384{0.0f};
    bool escape_enabled_36c{false}; // +36Ch, 009F4141, 009F498C, 009F4A62
    int escape_mode_370{0};         // +370h, 009F4999, 009F4A02
    int escape_latch_378{0};        // +378h, 009F4A10, 009F4B26, 009F4B8B
    // The hull's own position, for 009D8B90.
    float position_x{0.0f}; // +184h, 009F4775
    float position_z{0.0f}; // +188h
    float reference_speed_3c4{1.0f}; // +3C4h, 009F478B and 009EC9AB
};

// 009F41AB..009F4332. Steps blk+0A84h toward `target` and runs the two dwell
// timers. `target` is the ramp above.
void ship_ai_step_danger_009f41ab(ShipAiObstacleState& obs, float target, float dt) noexcept;

// ---------------------------------------------------------------------------
// 009EC7C0 BSP_UnitBot_ComputeThrottleCeiling
// ---------------------------------------------------------------------------
// float10 __thiscall(blk)(float heading_error, float danger, float cap), RET 0Ch,
// body 009EC7C0-009ECA1A, complete. Five 00419010 stages over the tuning block
// 00424C40. The Lua names are docs/GAMEPLAY_SETTINGS.md's.
struct ShipAiAutoThrustSettings {
    float hdg_diff_value_min_slow{0.0f}; // +6CCh, Navigator.AutoThrust.HdgDiffValueMin_Slow, DEG(25)
    float hdg_diff_value_max_slow{0.0f}; // +6D0h, Navigator.AutoThrust.HdgDiffValueMax_Slow, DEG(75)
    float thrust_min_slow{0.0f};         // +6D4h, Navigator.AutoThrust.ThrustMin_Slow, 0.5
    float hdg_diff_value_min_fast{0.0f}; // +6E0h, Navigator.AutoThrust.HdgDiffValueMin_Fast, DEG(45)
    float hdg_diff_value_max_fast{0.0f}; // +6E4h, Navigator.AutoThrust.HdgDiffValueMax_Fast, DEG(90)
    float thrust_min_fast{0.0f};         // +6E8h, Navigator.AutoThrust.ThrustMin_Fast, 0.75
    float hdg_diff_danger_mul{1.0f};     // +6ECh, Navigator.AutoThrust.HdgDiffDangerMul, 6.0
};

// The two live reads 009EC7C0 makes on the unit, separated so the rule stays
// pure. `live_throttle` is |unit+980h|, the order ring's live throttle
// (docs/CRUISE_COMMAND.md: ring+148h). `commanded_speed` and
// `commanded_speed_enabled` are *(unit+73Ch)+24h and +28h, the cruise setting
// whose enable is the >= 0.0f test at 009EC9A1.
struct ShipAiThrottleCeilingInputs {
    float live_throttle{0.0f};            // 009EC97B, unit+980h
    float commanded_speed{0.0f};          // 009EC9A4, *(unit+73Ch)+24h
    bool commanded_speed_enabled{false};  // 009EC99C, *(unit+73Ch)+28h >= 0.0f
    float reference_speed{1.0f};          // 009EC9AB, blk+3C4h, the divisor
};

// 009EC9DB and 009EC9E5: astern, the ceiling is negated and floored at -0.625.
inline constexpr float kShipAiAsternThrottleFloor = -0.625f; // 00D21A78, 00D21A80

// `committed_ahead_364` is blk+364h != 0, tested at 009EC9CD:
// the committed direction is ahead when the byte is set and astern when it is
// clear, so a clear byte negates the whole ceiling.
float ship_ai_throttle_ceiling_009ec7c0(const ShipAiAutoThrustSettings& settings,
                                        const ShipAiThrottleCeilingInputs& inputs,
                                        bool committed_ahead_364,
                                        float heading_error, float danger, float cap) noexcept;

// ---------------------------------------------------------------------------
// 009D8B90, the neighbour's closing speed
// ---------------------------------------------------------------------------
// float10 __thiscall(neighbour)(const float* our_position, int pass_side),
// RET 8, body 009D8B90-009D8C51, complete. One call site, 009F477E.
// `neighbour_speed` is 0092D730 on the neighbour's controller (009D8BC1,
// ECX = [[this+14h]+1018h]); the caller reads the same value again at 009F47B8.
float ship_ai_neighbour_closing_speed_009d8b90(const ShipAiNeighbourRecord& neighbour,
                                               float our_x, float our_z, int pass_side,
                                               float neighbour_speed) noexcept;

// ---------------------------------------------------------------------------
// The sector index
// ---------------------------------------------------------------------------
// 009F454C..009F4592, the five rudder buckets. The edges are read in order and
// every test is a strict COMISS/JBE, so an unordered compare falls through to
// the last bucket.
inline constexpr float kShipAiRudderBucketHard = 0.65f;   // 00D07FC4, 009F4554
inline constexpr float kShipAiRudderBucketSoft = 0.25f;   // 00CE3868, 009F4561
inline constexpr float kShipAiRudderBucketSoftNeg = -0.25f; // 00CF00A8, 009F4571
inline constexpr float kShipAiRudderBucketHardNeg = -0.65f; // 00D21B34, 009F4581

// 009F4594: when the hull is moving astern the bucket is mirrored.
int ship_ai_rudder_bucket_009f454c(float rudder, bool speed_above_astern_floor) noexcept;

// 009F43A9, FLD 00D7A264: the half turn the obstacle arm's reference bearing
// takes when the hull is moving against the latched direction.
inline constexpr float kShipAiObstacleHalfTurn = 3.1415927410125732f; // 00D7A264
// 009F4791, FMUL double 00CEC9D8: the fraction of the hull's own reference
// speed a neighbour has to beat before the AI backs off.
inline constexpr double kShipAiNeighbourSpeedFraction = 0.75; // 00CEC9D8
// 009F47A7 and 009F47E2: the astern latch lasts one second, and a neighbour
// slower than this adds the difference to the stall accumulator.
inline constexpr float kShipAiObstacleBackoffSeconds = 1.0f; // 00D7A24C
inline constexpr float kShipAiStallNeighbourSpeed = 2.0f;    // 00CE3958
// 009F489E and 009F4A33, COMISS against 00CE38B8: the stall the escape
// manoeuvre waits for.
inline constexpr float kShipAiStallThreshold = 10.0f; // 00CE38B8
// 009F45CD: the hold blk+354h is raised to while a sector is blocked.
inline constexpr float kShipAiObstacleHold = 3.0f; // 00CE3854

// 009F45A4..009F45B0.
int ship_ai_sector_index_009f45a4(int direction_index, int bucket) noexcept;

// ---------------------------------------------------------------------------
// The host the executable must implement
// ---------------------------------------------------------------------------
// One pure virtual per native call site the projection cannot make pure.
struct ShipAiObstacleHost {
    virtual ~ShipAiObstacleHost() = default;

    // 009F4386/009F438C, 009F461C/009F462A and 009F4A49/009F4A51: the
    // turn-assist load latch of docs/UNIT_COMMAND_PRODUCERS.md, the inlined
    // body of 009D4FB0. The native compare-and-store pair is the whole
    // operation, so the host must implement the raise:
    //   if (value > unit+102Ch) unit+102Ch = value;
    virtual void raise_turn_assist_load_102c(float value) = 0;

    // 009F45F8/009F45FE, 009F4904/009F4912 and 009F4AAE/009F4AB4: the second
    // load latch at unit+1034h, 009D4FE0 inlined, same raise rule.
    virtual void raise_secondary_load_1034(float value) = 0;

    // 009F44F7 and 009F4694, CALL 009DA250 with ECX = blk: the rudder law. The
    // projection of that law is ship_ai_rudder_from_heading_error_009da250 in
    // bsp/ship_ai_throttle_ring.hpp; this method is its value at these sites.
    virtual float rudder_law_009da250(float heading_error) = 0;

    // 009F47B8 (and 009D8BC1 inside 009D8B90), CALL 0092D730 with
    // ECX = [[sector.blocker+14h]+1018h]: the neighbour's signed forward speed.
    virtual float neighbour_body_axis_speed_0092d730(const ShipAiNeighbourRecord& neighbour) = 0;
};

// ---------------------------------------------------------------------------
// The middle of 009F3F80
// ---------------------------------------------------------------------------
// What the head of 009F3F80 has already computed when the middle starts, all of
// it projected by docs/SHIP_AI_THROTTLE_TO_RING.md.
struct ShipAiObstacleFrame {
    float dt{0.0f};              // the routine's float argument
    float body_axis_speed{0.0f}; // 009F4034, 0092D730 on [unit+1018h]
    float heading{0.0f};         // 009F407B, unit->vtable[50h](), already turned by
                                 // pi at 009F409E when the latch is Astern
    float heading_error{0.0f};   // 009F40BB, 00438B10(blk+324h, heading)
    float unit_half_width_9cc{1.0f}; // 009F4174, the FDIV divisor
};

// 009F4034..009F4166: the four booleans the rest of the middle is written in
// terms of. Exposed because the escape arm reads all four and the doc names
// them.
struct ShipAiObstacleFlags {
    bool speed_above_astern_floor{false}; // [ESP+12h], 009F4043: speed > -0.4
    bool speed_below_ahead_floor{false};  // [ESP+13h], 009F4066: speed < +0.4
    bool hull_with_latch{false};          // [ESP+15h], 009F40E3: the hull is not
                                          // moving against the latched direction
    bool committed_with_latch{false};     // [ESP+14h], 009F410F: blk+364h agrees
                                          // with the latch
    bool moving_or_stopped{false};        // [ESP+16h], 009F4130
    bool direction_mismatch{false};       // BL at 009F4162: blk+36Ch disagrees
                                          // with blk+364h for the latch
};

ShipAiObstacleFlags ship_ai_obstacle_flags_009f40ca(const ShipAiControlBlock& blk,
                                                    const ShipAiObstacleState& obs,
                                                    float body_axis_speed) noexcept;

// 009F40CA..009F4B98, the whole middle, projected operation for operation.
// Writes blk.desired_throttle and blk.desired_rudder, the danger level and its
// timers, the astern latch and the escape latch; raises the two load latches
// through the host. The hop at 009F4B99 is
// ship_ai_order_ring_hop_009f4b99 in bsp/ship_ai_throttle_ring.hpp and runs after
// this, unconditionally: every path here converges on 009F4B99.
void ship_ai_drive_order_ring_middle_009f40ca(ShipAiControlBlock& blk,
                                              ShipAiObstacleState& obs,
                                              const ShipAiObstacleFrame& frame,
                                              const ShipAiAutoThrustSettings& settings,
                                              const ShipAiThrottleCeilingInputs& ceiling_inputs,
                                              ShipAiObstacleHost& host);

} // namespace bsp
