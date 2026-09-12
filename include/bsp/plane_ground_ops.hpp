#pragma once
#include <cstdint>

// Projection of the plane's ground, water, takeoff and landing behaviour: the eight-value
// state enum at unit+900h, the ground-roll arm 007CBFA0 -> 007DCCF0, the water-surface arm
// 007CBA50 -> 007DCDD0, the runway steering band of 007DA380, the arrestor-wire band of
// 007DB680 and the two lift-off requests 007CBFA0 issues.
//
// docs/PLANE_GROUND_OPS.md carries the evidence, the coverage table and the corrections.
// Every name here is a hypothesis, not a recovered symbol, EXCEPT the state labels: those
// are the game's own debug strings, reached through the jump table at 007CCBC0. Nothing in
// this header is a binary-compatible layout; the offset constants are the native ones.
//
// Contracts named but not reconstructed: the arrestor-wire consumer of ctl+ACh (the value is
// produced here, the pose that uses it is docs/DYN_PHYSICS_SUBSTEP.md); the air-operations
// slot release on landing (006C65B0 BSP_AirOps_ReleaseSquadronSlot, docs/AIR_OPERATIONS.md);
// the catapult shot itself (MCatapult::Fire 006EC8E0, same doc); the pilot bots that fly the
// approach (docs/BOT_TASKS.md `land`); the body of the water law past its hull sampling.

namespace bsp {

// ---------------------------------------------------------------------------
// unit+900h. The switch at 007CCB30 (Ghidra body 007CCB30-007CCBBF) formats the
// field for the debug dump: `cmp edx,7 / ja default / jmp [edx*4 + 7CCBC0]`, and
// the eight-entry table at 007CCBC0 selects one string per value. Value 3 is the
// only in-range value routed to the out-of-range arm, so the shipped build has no
// label for it; `Launching` is this packet's hypothesis from its side effects.
// ---------------------------------------------------------------------------
enum class PlaneFlightState : int {
    NotApplicable = 0,  // "N/A plane state" 00D05E74; the constructor's value
    Inside = 1,         // "Inside" 00D05E6C; stowed in the hangar
    Locked = 2,         // "Locked" 00D05E64; held on the catapult or the deck spot
    Launching = 3,      // no label (falls to "huh?" 00D05E34); the setter forces full throttle
    Runway = 4,         // "Runway" 00D05E5C; free ground roll
    RunwayOnPath = 5,   // "Runway on Path" 00D05E4C; ground roll driven along a path
    Water = 6,          // "Water" 00D05E44; floating or ditched
    Flying = 7,         // "Flying" 00D05E3C; airborne
};

// The exact debug string 007CCB30 assigns, or nullptr for a value outside 0..7.
const char* flight_state_debug_label_007ccb30(int state);

inline constexpr int kPlaneGroundStateCount = 8;

namespace plane_ground_off {
// Unit fields. unit+900h itself is plane_unit_off::kGroundWaterMode in plane_flight.hpp.
inline constexpr int kFlightState = 0x900;       // the enum above
inline constexpr int kLandedAfterFlight = 0x904; // byte; set on entering 4/5 when the clock > 5s
inline constexpr int kAirborneClock = 0x908;     // float seconds; 007CEC4E accumulates it
inline constexpr int kWaterAccumulator = 0x90C;  // float; argument 2 of the water law
inline constexpr int kStateByte910 = 0x910;      // byte; cleared on entering 6 and 7
inline constexpr int kSplashLatch = 0x911;       // byte; 007CBC5E one-shot for "splash"
inline constexpr int kSpeed = 0xB1C;             // float; compared with AirField/PlayerControlSpd
inline constexpr int kGroundContactOwner = 0xBF4;// the object the plane rests on
inline constexpr int kGroundContact = 0xBF8;      // byte; non-zero while resting on it
inline constexpr int kGroundHeight = 0xBFC;       // float; height against classDesc+1FCh
inline constexpr int kPendingByteC01 = 0xC01;    // byte; 007CC2AD writes 2, write-only in this scan
inline constexpr int kStateChangeStamp = 0xC04;  // float; every state change stores -1.0f
inline constexpr int kGroundSteerGate = 0xC0C;   // byte; the runway steering authority
inline constexpr int kStateEnterMarker = 0xC18;  // 3 on entering 4/5
inline constexpr int kWreckFuse = 0xC44;         // float seconds; counts down while unit+5Dh
inline constexpr int kOutOfActionFlag = 0x5D;    // byte; gates the fuse and the splash test

// Flight controller fields (the object at unit+AB0h).
inline constexpr int kControllerMode = 0xFC;     // 0 free flight, 1 ground roll, 2 water
inline constexpr int kWireRopeAccumulator = 0xAC;// ctl+ACh, the arrestor-wire / brake build-up
inline constexpr int kGroundFrame = 0x80;        // four floats written by 007DCCF0
inline constexpr int kForwardSpeed = 0x6C;       // ctl+6Ch, the speed the runway band reads
inline constexpr int kWheelLoad = 0x68;          // ctl+68h, clamped by the wheel-friction band

// Class descriptor fields (unit+538h, ctl+0Ch).
inline constexpr int kClassMinWaterSpd = 0x198;  // MinWaterSpd; the wheeled-plane discriminator
inline constexpr int kClassMaxWaterSpd = 0x19C;  // read by the splash test
inline constexpr int kClassStallSpd = 0x184;     // StallSpd
inline constexpr int kClassYawSpd = 0x1B0;       // YawSpd
inline constexpr int kClassWheelBrake = 0x1E0;   // WheelBrake, 80
inline constexpr int kClassGroundRef = 0x1FC;    // subtracted from unit+BFCh at 007CC1CC
inline constexpr int kClassGroundPitch = 0x200;  // GroundPitch; the ground frame's angle

// Game tuning offsets (the singleton 0042E740 returns).
inline constexpr int kTuningPlayerControlSpd = 0x18C;   // AirField/PlayerControlSpd, KMH(80)
inline constexpr int kTuningLevelFlightMul = 0x24C;     // Dynamics/SpdMultipliers/LevelFlight, 1.8
inline constexpr int kTuningRunwaySmoothStrength = 0x2A4;
inline constexpr int kTuningRunwayYawTurnSpdLimit = 0x2A8;
inline constexpr int kTuningRunwayYawTurnSpdLimitDup = 0x2AC;
inline constexpr int kTuningRunwayYawTurnSpdMul = 0x2B0;
inline constexpr int kTuningWaterMaxVSpd = 0x2B4;
inline constexpr int kTuningWaterMaxDownPitch = 0x2B8;
inline constexpr int kTuningWaterMaxUpPitch = 0x2BC;
inline constexpr int kTuningWaterMaxRoll = 0x2C0;
inline constexpr int kTuningWireRope = 0x518;           // Pilot/Landing/WireRope, 7.25
inline constexpr int kTuningMaxWireRope = 0x51C;        // Pilot/Landing/MaxWireRope, 100
inline constexpr int kTuningWheelFriction = 0x290;
inline constexpr int kTuningWheelFrictionAccelLo = 0x29C;
inline constexpr int kTuningWheelFrictionAccelHi = 0x2A0;
}  // namespace plane_ground_off

// The installed values the ground and water arms read, from
// docs/GAME_TUNING_SINGLETON.md and docs/PLANE_CLASS_FIELDS.md.
inline constexpr float kPlaneGroundLandedClockThreshold = 5.0f;   // 00CE3850
inline constexpr float kPlaneGroundSpawnedAirborneClock = 3600.0f;// 00CFDEB0
inline constexpr float kPlaneGroundStateChangeStamp = -1.0f;      // 00D7A260
inline constexpr float kPlaneGroundLiftOffEpsilon = 0.1f;         // 00D7A3A0, a double
inline constexpr float kPlaneGroundLandedYawScale = 0.5f;         // 00D7A280, a double
inline constexpr float kPlaneGroundYawTurnNumerator = 0.872665f;  // 00D057E0, DEG(50)
inline constexpr float kPlaneGroundWireDeckShortLen = 160.0f;     // 00D04A10
inline constexpr float kPlaneGroundWireDeckShortMul = 1.4f;       // 00D06874
inline constexpr float kPlaneGroundWireDeckLongLen = 250.0f;      // 00CE77B0
inline constexpr float kPlaneGroundWireDeckLongMul = 1.0f;        // fld1 at 007DC055
inline constexpr float kPlaneGroundInstalledWheelBrake = 80.0f;
inline constexpr float kPlaneGroundInstalledWireRope = 7.25f;
inline constexpr float kPlaneGroundInstalledMaxWireRope = 100.0f;
inline constexpr float kPlaneGroundInstalledRunwaySmoothStrength = 4.0f;
inline constexpr float kPlaneGroundInstalledRunwayYawTurnSpdMul = 2.2f;
inline constexpr float kPlaneGroundInstalledLevelFlightMul = 1.8f;
inline constexpr float kPlaneGroundInstalledWheelFrictionAccelLo = 0.0f;
inline constexpr float kPlaneGroundInstalledWheelFrictionAccelHi = 8.0f;

// The session message kinds the ground and water arms build.
inline constexpr int kPlaneGroundTakeoffMessageKind = 0xC6;      // 00762A00 stores it at +10h
inline constexpr int kPlaneGroundStateChangeMessageKind = 0xC3;  // 007CBD9x, 0075B430

// ---------------------------------------------------------------------------
// The motion dispatch, 007CEC30-007CECB4. This supersedes
// select_motion_arm_007ce040 in plane_flight.hpp: the surface arm's gate at
// 007CEC99 reads [ESI+5F0h] where ESI is unit+310h, so the field is unit+900h,
// not a second enum at unit+5F0h. See the corrections in docs/PLANE_GROUND_OPS.md.
// ---------------------------------------------------------------------------
enum class PlaneGroundMotionArm {
    None,        // 007CECA0 skips the 0085DC80 commit as well
    FreeFlight,  // 007CEC6E 007CC2F0
    GroundRoll,  // 007CEC92 007CBFA0
    Water,       // 007CECAF 007CBA50
};

PlaneGroundMotionArm select_motion_arm_007cec30(bool control_mode_gate, int flight_state);

// ---------------------------------------------------------------------------
// The controller mode at ctl+FCh, written by whichever law runs.
// ---------------------------------------------------------------------------
enum class PlaneControllerMode : int {
    FreeFlight = 0,  // 007DC841
    GroundRoll = 1,  // 007DCD24
    Water = 2,       // 007DCDD3
};

// ---------------------------------------------------------------------------
// 007C1430 BSP_Plane_SetFlightState, Ghidra body 007C1430-007C156C,
// __thiscall(unit, int state), RET 4. The jump table at 007C1550 is indexed by
// state - 3 over five entries; states 0, 1, 2 and anything above 7 share the tail.
// ---------------------------------------------------------------------------
struct PlaneSetStateInputs {
    int requested_state{0};
    int current_state{0};
    float airborne_clock{0.0f};   // unit+908h
    float class_min_water_spd{0.0f};  // classDesc+198h; zero disables the 4/5 arm
    bool landed_after_flight{false};  // unit+904h on entry
};

struct PlaneSetStateEffects {
    bool changed{false};             // false means the setter returned 0 and stored nothing
    int state{0};
    bool zero_throttle{false};       // 007C143C, on a requested state of 2 even when unchanged
    bool force_full_throttle{false}; // 007C1532, state 3
    bool write_state_change_stamp{false};  // unit+C04h = -1.0f
    bool write_landed_after_flight{false};
    bool landed_after_flight{false};
    bool clear_landed_after_flight{false};
    bool clear_byte_910{false};
    bool clear_water_accumulator{false};
    bool write_airborne_clock{false};
    float airborne_clock{0.0f};
    bool write_state_enter_marker{false};  // unit+C18h = 3
    bool notify{false};                    // 007C11E0(0)
};

PlaneSetStateEffects set_flight_state_007c1430(const PlaneSetStateInputs& in);

// ---------------------------------------------------------------------------
// 007C6340, the placement chooser: the only routine that decides between Water
// and Flying when a plane is put into the world. Ghidra body 007C6340-007C64F5,
// __fastcall(unit); its only caller is 007F2920. It writes unit+900h directly,
// bypassing the setter, and notifies with 007C11E0(1) rather than (0).
// ---------------------------------------------------------------------------
struct PlaneSpawnPlacementInputs {
    int current_state{0};
    float height_probe{0.0f};         // unit+A8h at 007C6370
    float class_height_reference{0.0f};  // classDesc+A8h at 007C636A
    float class_min_water_spd{0.0f};  // non-zero forces Flying
};

struct PlaneSpawnPlacement {
    bool launch_arm{false};  // current_state == 3: no state write, only the pose refresh
    PlaneFlightState state{PlaneFlightState::Flying};
    float airborne_clock{0.0f};
};

PlaneSpawnPlacement choose_spawn_placement_007c6340(const PlaneSpawnPlacementInputs& in);

// ---------------------------------------------------------------------------
// 007C7110, the become-airborne routine. Ghidra body 007C7110-007C71DF,
// __fastcall(unit). It inlines the setter's state-7 arm and, on the authority,
// notifies the surface it is leaving.
// ---------------------------------------------------------------------------
inline constexpr int kPlaneGroundNetModeClient = 2;  // game+1FE4h

struct PlaneBeginFlyingInputs {
    int current_state{0};
    int net_mode{0};
    bool has_ground_contact_owner{false};  // unit+BF4h != 0
    bool free_flight_gate{false};          // (*(unit+72Ch))->vtable[+38h]() on the client
};

struct PlaneBeginFlyingEffects {
    bool notify_surface_of_departure{false};  // (owner+4h)->+3Ch->vtable[+28h](unit)
    bool run_client_fallback{false};          // 007C6F50(0)
    bool changed{false};
    float airborne_clock{0.0f};  // 0 from Runway or Launching, 3600 otherwise
};

PlaneBeginFlyingEffects begin_flying_007c7110(const PlaneBeginFlyingInputs& in);

// ---------------------------------------------------------------------------
// 007DCCF0, the ground-roll law. Ghidra body 007DCCF0-007DCDCB,
// __thiscall(ctl, float step, int flag), RET 8. The flag is tested as a byte at
// 007DCD20, so only its low byte matters; the call site at 007CC155 sets it with
// SETZ on `unit+900h == 2`.
// ---------------------------------------------------------------------------
struct PlaneGroundLawInputs {
    bool ground_contact{false};  // unit+BF8h
    int flight_state{0};
    bool state_is_locked{false};  // the flag argument, `flight_state == 2`
    float class_ground_pitch{0.0f};
    float tuning_runway_smooth_strength{0.0f};
};

struct PlaneGroundLawResult {
    bool took_ground_arm{false};  // the return value in AL
    // Free-flight fallback: 007DC830(step) ran and nothing below was written.
    PlaneControllerMode mode{PlaneControllerMode::FreeFlight};
    bool clear_mode_bytes{false};    // ctl+4h and ctl+5h
    float core_law_arg_a{0.0f};      // argument 3 of 007DB680
    float ground_frame[4]{0.0f, 0.0f, 0.0f, 0.0f};  // ctl+80h..8Ch
};

PlaneGroundLawResult ground_roll_law_007dccf0(const PlaneGroundLawInputs& in);

// ---------------------------------------------------------------------------
// 007CC0C9-007CC12C: the runway steering authority, unit+C0Ch.
// ---------------------------------------------------------------------------
struct PlaneGroundSteerGateInputs {
    bool landed_after_flight{false};  // unit+904h
    int flight_state{0};
    bool has_squadron{false};                // unit+9D4h != 0
    bool squadron_override{false};           // squadron+360h
    float speed{0.0f};                       // unit+B1Ch
    float tuning_player_control_spd{0.0f};   // AirField/PlayerControlSpd
};

bool runway_steer_gate_007cc0c9(const PlaneGroundSteerGateInputs& in);

// ---------------------------------------------------------------------------
// 007CBFF0-007CC043: the wreck fuse. While unit+5Dh is set the fuse counts down;
// without steering authority it is first clamped down to 5 seconds.
// ---------------------------------------------------------------------------
struct PlaneWreckFuseResult {
    float fuse{0.0f};
    bool explode{false};
};

PlaneWreckFuseResult wreck_fuse_007cbff0(float fuse, float step, bool steer_gate);

// ---------------------------------------------------------------------------
// 007DA540-007DA616, the runway steering band of 007DA380 (Ghidra body
// 007DA380-007DA707). Below RunwayYawTurnSpdLimit the yaw rate is replaced by a
// low-speed tail-wheel term; 0047B850 restricts the halving to classes 10h / 16h.
// ---------------------------------------------------------------------------
struct PlaneRunwayYawInputs {
    float forward_speed{0.0f};          // ctl+6Ch
    float class_yaw_spd{0.0f};          // classDesc+1B0h
    float tuning_yaw_turn_spd_limit{0.0f};      // +2A8h
    float tuning_yaw_turn_spd_limit_dup{0.0f};  // +2ACh; the shipped duplicate
    float tuning_yaw_turn_spd_mul{0.0f};        // +2B0h
    bool class_is_10h_or_16h{false};    // 0047B850
    bool landed_after_flight{false};    // unit+904h
};

struct PlaneRunwayYawResult {
    bool runway_band_applies{false};  // false above the limit; the caller keeps its own term
    float yaw_rate{0.0f};
    float blend{0.0f};                // the InterpolateClamped result at 007DA611
};

PlaneRunwayYawResult runway_yaw_factor_007da540(const PlaneRunwayYawInputs& in);

// ---------------------------------------------------------------------------
// 007DBEEE-007DC088, the wheel-brake and arrestor-wire band of 007DB680
// (BSP_PlaneFlight_CoreLaw, Ghidra body 007DB680-007DC82A).
// ---------------------------------------------------------------------------
struct PlaneWireRopeInputs {
    bool brake_flag{false};       // [esp+13h] at 007DBEEE; zero resets the accumulator
    float wheel_brake{0.0f};      // classDesc+1E0h
    float load_max{0.0f};         // the max() the band feeds the brake with
    float accumulator{0.0f};      // ctl+ACh on entry
    float ground_speed_xz{0.0f};  // BSP_Vector2f_LengthWithCutoff of the two velocity taps
    float scale{0.0f};            // [esp+5Ch]
    float tuning_wire_rope{0.0f};
    float tuning_max_wire_rope{0.0f};
    bool has_ground_contact_owner{false};  // 006049F0(unit) != 0
    float owner_deck_length{0.0f};         // owner+B4h
};

struct PlaneWireRopeResult {
    float brake_term{0.0f};   // wheel_brake * load_max, [esp+60h]
    float accumulator{0.0f};  // ctl+ACh after the band
    bool reset{false};        // the 007DC085 arm stored 0.0f
    bool held{false};         // the 007DBF22 arm left the accumulator untouched
};

PlaneWireRopeResult wire_rope_band_007dbeee(const PlaneWireRopeInputs& in);

// ---------------------------------------------------------------------------
// 007CC1B3-007CC2B3, the lift-off request. Two paths raise the same takeoff
// message; when neither fires and ground contact is gone the arm only stamps
// unit+C01h.
// ---------------------------------------------------------------------------
enum class PlaneLiftOffAction {
    None,
    SendTakeoffMessage,  // 00762A00 then 0077C2A0(unit, msg, 1, 0)
    StampPendingByte,    // unit+C01h = 2
};

struct PlaneLiftOffInputs {
    int flight_state{0};
    int net_mode{0};
    float ground_height{0.0f};        // unit+BFCh
    float class_ground_reference{0.0f};  // classDesc+1FCh
    float vertical_rate{0.0f};        // unit+ACCh
    bool ground_contact{false};       // unit+BF8h
    bool owner_is_class_9{false};     // (owner+4h)+7Ch ->vtable[+5Ch](9)
};

PlaneLiftOffAction lift_off_request_007cc1b3(const PlaneLiftOffInputs& in);

// ---------------------------------------------------------------------------
// The water-impact test of 007CBA50 (Ghidra body 007CBA50-007CBF93), the block
// that ends at the "splash" push 007CBB91. Any one condition makes the contact a
// crash landing: "splash" is raised and unit+911h latches so it fires once.
// ---------------------------------------------------------------------------
struct PlaneWaterImpactInputs {
    float speed{0.0f};                 // unit->vtable[+204h]()
    float class_max_water_spd{0.0f};   // classDesc+19Ch
    float vertical_rate{0.0f};         // unit+ACCh
    float pitch{0.0f};                 // unit+C64h
    float roll{0.0f};                  // unit+C68h
    float tuning_water_max_vspd{0.0f};
    float tuning_water_max_down_pitch{0.0f};
    float tuning_water_max_up_pitch{0.0f};
    float tuning_water_max_roll{0.0f};
};

bool water_impact_is_splash_007cbb91(const PlaneWaterImpactInputs& in);

// 007CBD79-007CBE00: the engine-drowning test. The water law's returned depth is
// compared with Dynamics/Water/MaxDepth scaled by a speed blend whose upper x is
// LevelFlight * StallSpd; when the depth wins and the net mode is 0 or 1 the
// "powerlost" effect fires instead of any takeoff request.
struct PlaneWaterDrownInputs {
    float law_depth{0.0f};                  // the float10 007DCDD0 leaves in ST0
    float speed{0.0f};                      // unit->vtable[+204h]()
    float class_stall_spd{0.0f};            // classDesc+184h
    float tuning_level_flight_mul{0.0f};    // +24Ch, 1.8
    float tuning_normal_yaw_control_spd{0.0f};  // +2C8h, KMH(45)
    float tuning_water_max_depth{0.0f};     // +2D4h, 6.0
    float interpolate_y1{0.0f};             // 00CE3800, the blend's upper y
};

struct PlaneWaterDrownResult {
    float threshold{0.0f};
    bool engine_drowns{false};
};

PlaneWaterDrownResult water_drown_test_007cbd79(const PlaneWaterDrownInputs& in);

// 007CBE7A-007CBED1: the submersion accumulator. It grows only while the plane is
// below MinWaterSpd and the law reports depth.
float water_accumulator_step_007cbe7a(float accumulator, float law_depth, float speed,
                                      float class_min_water_spd);

// The water arm's state request, 007CBED9-007CBF80. On the authority it routes the
// state-change message; on the client it applies state 7 directly, and only from
// Water, Runway or RunwayOnPath.
enum class PlaneWaterTakeoffAction {
    None,
    SendStateChangeMessage,  // kind C3h carrying {7, current}
    SetFlyingDirectly,       // 007C1430(7)
};

PlaneWaterTakeoffAction water_takeoff_request_007cbed9(int flight_state, int net_mode);

// ---------------------------------------------------------------------------
// The host the ground-roll step drives: one method per native call site inside
// 007CBFA0 (Ghidra body 007CBFA0-007CC2C5, __thiscall(unit, float step)).
// ---------------------------------------------------------------------------
class PlaneGroundOpsHost {
public:
    virtual ~PlaneGroundOpsHost() = default;

    // 007CBFC3 007C5AC0(step), the shared pre-pass the free-flight arm also runs.
    virtual void run_pre_pass_007c5ac0(float step) = 0;

    // 007CBFDE / 007CC1A1: on the object at unit+DECh, latch +45h and +4Ch and then
    // raise +11h. Called on the entry pass when unit+904h is clear, and again on the
    // exit pass through the same three stores.
    virtual void arm_ground_subsystem_007cbfd2() = 0;

    // 007CC06C 0041E870 then unit->vtable[+194h](&out, 0): raise a named effect.
    virtual void raise_effect(const char* name) = 0;

    // 007CC13F unit->vtable[+1ECh](step), the per-axis control clamp 007CAF10.
    virtual void clamp_controls_007caf10(float step) = 0;

    // 007CC15E 007DCCF0(unit+AB0h, step, flag), the ground-roll law.
    virtual PlaneGroundLawResult run_ground_law_007dccf0(float step, bool state_is_locked) = 0;

    // 007CC186 (owner+4h)->+3Ch->vtable[+38h](unit): does the surface still hold the
    // plane? Only reached on the exit pass when unit+904h is set.
    virtual bool surface_still_holds_007cc186() = 0;

    // 007CC18E 007B8DA0(unit), taken when the surface answers true.
    virtual void run_surface_hold_007b8da0() = 0;

    // 007CC212 / 007CC26E: build the takeoff message (00762A00, kind C6h) and route it
    // with 0077C2A0(unit, msg, 1, 0).
    virtual void send_takeoff_message_00762a00() = 0;

    // 007CC2AD unit+C01h = 2.
    virtual void stamp_pending_byte_007cc2ad() = 0;

    // Field reads the sequence needs. Grouped rather than one accessor per offset.
    virtual int flight_state() const = 0;
    virtual bool landed_after_flight() const = 0;
    virtual bool out_of_action() const = 0;  // unit+5Dh
    virtual float wreck_fuse() const = 0;
    virtual void set_wreck_fuse(float value) = 0;
    // unit+C0Ch as the previous step left it; 007CBFFA reads it before 007CC0E5
    // overwrites it, so the fuse clamp uses the older value.
    virtual bool runway_steer_gate() const = 0;
    virtual void set_runway_steer_gate(bool value) = 0;
    virtual int net_mode() const = 0;
    virtual PlaneGroundSteerGateInputs steer_gate_inputs() const = 0;
    virtual PlaneLiftOffInputs lift_off_inputs() const = 0;
};

// What 007CBFA0 did on one step, for a caller that wants to assert on it.
enum class PlaneGroundStepOutcome {
    Exploded,           // the fuse expired and the "explosion" effect was raised
    FuseExpiredQuiet,   // the fuse expired but the net mode suppressed the effect
    RanLaw,             // the law ran and no lift-off was requested
    RequestedTakeoff,   // the law ran and the takeoff message went out
    StampedPending,     // the law ran and only unit+C01h was stamped
};

PlaneGroundStepOutcome run_ground_roll_step_007cbfa0(PlaneGroundOpsHost& host, float step);

}  // namespace bsp
