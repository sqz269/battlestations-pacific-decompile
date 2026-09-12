#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/sensor_tables.hpp"

// Reconstruction of the submarine unit's depth, air and crush-depth model.
// docs/SUBMARINE_MODEL.md carries the evidence, the coverage table and the
// addresses. Every descriptive name is a hypothesis except the eight field names
// the binary itself uses in its save schema at 00853E10, which are recovered.
//
// Nothing here is ABI-compatible. The native fields live in a 1288h instance and
// these are behavioural projections of the rules that read and write them.

namespace bsp {

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

// Instance offsets of the submarine leaf's own fields. The ship unit is 1188h
// bytes and the submarine is a sibling leaf over the same five-level base
// (00852F10 calls 0081ED40, the level the ship leaf 006FE460 also calls), so
// everything the submarine adds starts at 1188h rather than inside the ship.
struct SubmarineUnitOffsets {
    static constexpr std::uint32_t kDepthBands = 0x1200;      // 4 floats, 00853630
    static constexpr std::uint32_t kPeriscopeNode = 0x1214;   // "periszkop", 00853630
    static constexpr std::uint32_t kPeriscopeState = 0x122C;  // recovered "periscopeState"
    static constexpr std::uint32_t kPeriscopeY = 0x1230;      // recovered "periscopeY"
    static constexpr std::uint32_t kPeriscopeOut = 0x1234;    // recovered "periscopeOut"
    static constexpr std::uint32_t kPeriscopeAutoRaise = 0x1235;  // 00464320, 004649B0
    static constexpr std::uint32_t kScanStep = 0x1238;        // 0..0Ch, 00855420
    static constexpr std::uint32_t kClearanceOut = 0x123C;    // read by 00936DC0
    static constexpr std::uint32_t kScanRollOut = 0x1240;
    static constexpr std::uint32_t kScanPitchOut = 0x1244;
    static constexpr std::uint32_t kClearanceAcc = 0x1248;
    static constexpr std::uint32_t kScanRollAcc = 0x124C;
    static constexpr std::uint32_t kScanPitchAcc = 0x1250;
    static constexpr std::uint32_t kRepairDeadline = 0x1254;  // 009373C0
    static constexpr std::uint32_t kRepairClock = 0x1258;
    static constexpr std::uint32_t kRepairClockFast = 0x125C;
    static constexpr std::uint32_t kDepthLevel = 0x1268;      // recovered "depthLevel"
    static constexpr std::uint32_t kSubmergeSeedFlag = 0x126C; // 00853630 = 1
    static constexpr std::uint32_t kSubmergeForce = 0x1270;   // recovered "sullyesztoEro"
    static constexpr std::uint32_t kSubmergeForceAux = 0x1274;
    static constexpr std::uint32_t kAtDepthFlag = 0x1278;     // 00936DC0
    static constexpr std::uint32_t kCatapultSurfaceEnable = 0x1279;  // read by 008522C0
    static constexpr std::uint32_t kAir = 0x127C;             // recovered "air"
    static constexpr std::uint32_t kUnlimitedAir = 0x1280;    // recovered "unlimitedAir"
    static constexpr std::uint32_t kNeedAir = 0x1281;         // recovered "needAir"
    static constexpr std::uint32_t kCrushAccumulator = 0x1284; // 008551C0
    static constexpr std::uint32_t kInstanceSize = 0x1288;    // operator new at 008531AF
};

// Class-descriptor offsets of the thirteen keys 00854230 reads. The ship keys
// this packet also consumes are in include/bsp/ship_class_fields.hpp
// (MaxSpeed +500h, KamikazeDamage +510h, KamikazeBlastDamage +514h) and the base
// dimensions in include/bsp/vehicle_class_fields.hpp (Length +A0h, Width +A4h,
// Height +A8h).
struct SubmarineClassOffsets {
    static constexpr std::uint32_t kPeriscopeWave = 0x80C;       // effect handle
    static constexpr std::uint32_t kPeriscopeDepth = 0x810;      // alias SwimDepth1
    static constexpr std::uint32_t kSwimDepth2 = 0x814;
    static constexpr std::uint32_t kSwimDepth3 = 0x818;
    static constexpr std::uint32_t kPeriscopeMoveRange = 0x81C;
    static constexpr std::uint32_t kUpDownAccel = 0x824;
    static constexpr std::uint32_t kUpDownRotation = 0x828;
    static constexpr std::uint32_t kUpDownStopTime = 0x82C;
    static constexpr std::uint32_t kUpSpeed = 0x830;
    static constexpr std::uint32_t kDownSpeed = 0x834;
    static constexpr std::uint32_t kAirRunOutTime = 0x838;
    static constexpr std::uint32_t kAirReloadTime = 0x83C;
};

// ---------------------------------------------------------------------------
// Depth bands
// ---------------------------------------------------------------------------

// The instance's four target depths, world Y. 00852B90 reads all four and
// docs/SENSOR_TABLES.md names the sensor states they straddle.
enum class SubmarineDepthBand : int {
    surface = 0,    // 00465502, command string "Surface"
    periscope = 1,  // 00465520, "Periscope"
    underwater = 2, // 0046553E, "Underwater"
    max_depth = 3,  // 0046555C, "Maxdepth"
};
inline constexpr int kSubDepthBandCount = 4;

// The four command strings 004654D0 matches case-insensitively against
// command+2Ch. Recovered .rdata literals, in band order.
inline constexpr std::array<const char*, kSubDepthBandCount> kSubDepthBandCommands = {
    "Surface", "Periscope", "Underwater", "Maxdepth",
};

// The four dwords at 00E0B578, what a class that names no depth key gets.
// 00853A63 loads that address and 00853A7B subtracts 80Ch so the table can be
// indexed by the class offset; any Ghidra label at 00E0AD6C is an artifact.
inline constexpr std::array<float, kSubDepthBandCount> kSubDefaultBandDepths = {
    0.0f, -20.0f, -40.0f, -80.0f,
};

// DAT_00D7A208, the negative zero 00853630 subtracts the positive Lua metres
// from, and the same constant the motion tick's jump table uses as its origin.
inline constexpr float kSubDepthSignOrigin = -0.0f;

// The three class depth keys in band order. Index 0 has no key: 00853630's loop
// starts at +80Ch but its i<1 test always takes the default path, so the effect
// handle there is never read as a float.
struct SubmarineClassDepths {
    float periscope_depth{-1.0f}; // +810h, DAT_00D7A260 default, negative when unset
    float swim_depth_2{-1.0f};    // +814h
    float swim_depth_3{-1.0f};    // +818h
};

// 00853630 008539E0..00853A7C. A key left negative by the Lua loader falls back
// to the default image, which is why -1.0 means "unset" rather than "one metre".
std::array<float, kSubDepthBandCount> submarine_build_depth_bands_00853630(
    const SubmarineClassDepths& keys) noexcept;

// 00853630 00853B77..00853BB1. Errors are measured against band 0 as |hullY|,
// then bands 1..3 in order; ties keep the earlier band because the test is a
// strict <.
SubmarineDepthBand submarine_nearest_depth_band_00853630(
    const std::array<float, kSubDepthBandCount>& bands, float hull_world_y) noexcept;

// ---------------------------------------------------------------------------
// The depth command
// ---------------------------------------------------------------------------

// 008528B0's clamp. A kamikaze class (KamikazeDamage or KamikazeBlastDamage
// above zero, ship descriptor +510h/+514h) is pinned to periscope depth and
// cannot be commanded anywhere else; 00853630 gives the same class no periscope
// node at all.
SubmarineDepthBand submarine_clamp_depth_command_008528b0(int requested,
                                                          bool kamikaze_class) noexcept;

// 00852C60's A1h arm. `deeper` is the message payload byte at msg+20h being
// zero. A clamped step returns the current band unchanged, and the native still
// answers AL = 1.
SubmarineDepthBand submarine_step_depth_command_00852c60(SubmarineDepthBand current,
                                                         bool deeper) noexcept;

// The two message kinds 00852C60 adds above the base ceiling of A0h.
inline constexpr std::uint8_t kSubMessageStepDepth = 0xA1;      // 00852C68 SUB EAX,0A1h
inline constexpr std::uint8_t kSubMessageSetDepthLevel = 0xA2;  // 008528B0's echo

// ---------------------------------------------------------------------------
// Air supply
// ---------------------------------------------------------------------------

// The five Submarine.* gameplay settings this model reads, with the values
// docs/GAMEPLAY_SETTINGS.md records as installed. They are global: no class key
// overrides any of them.
struct SubmarineDepthSettings {
    float air_warning_limit{0.35f}; // +4B8h Submarine.SubmarineAirWarningLimit
    float air_need_limit{0.16f};    // +4BCh Submarine.SubmarineAirNeedLimit
    float air_enough_limit{0.50f};  // +4C0h Submarine.SubmarineAirEnoughLimit
    float damage_depth{70.0f};      // +4B0h Submarine.SubmarineDamageDepth
    float depth_damage{9.0f};       // +4ACh Submarine.SubmarineDepthDamage
};

// Class air rates, seconds. Both are used as dt/duration, so `air` is a
// normalised 0..1 charge and the constructor seeds it at 1.0 (DAT_00D7A24C).
struct SubmarineAirRates {
    float run_out_time{120.0f}; // +838h AirRunOutTime, DAT_00D05804 default
    float reload_time{5.0f};    // +83Ch AirReloadTime, DAT_00CE3850 default
};

inline constexpr float kSubAirFull = 1.0f;       // DAT_00D7A24C
inline constexpr float kSubAirSurfaceMargin = 3.0f;  // 00D7A2B0, a double
// 00CF1430. The margin sum is discarded whenever it falls below zero, so for
// every shipped class (periscope depth around -20) the breathing line is this
// flat value and the boat must come nearly to the surface to refill.
inline constexpr float kSubAirBreathingLineFallback = -4.0f;

// 00855256..00855286. The sum survives only for a class whose periscope depth
// is above -3; otherwise the line is kSubAirBreathingLineFallback.
float submarine_air_breathing_line_00855250(float periscope_band_y) noexcept;

// Which warning 00855250 raises on this step. Both edges are downward only and
// the critical edge suppresses the low edge on the same step.
enum class SubmarineAirWarning {
    none,
    low,      // 00977050 BSP_WarningManager_ReportSubmarineAirLow
    critical, // 009771E0 BSP_WarningManager_ReportSubmarineAirCritical
};

struct SubmarineAirState {
    float air{kSubAirFull};    // +127Ch "air"
    bool need_air{false};      // +1281h "needAir", the hysteresis latch
    bool unlimited_air{false}; // +1280h "unlimitedAir"
};

struct SubmarineAirStepResult {
    SubmarineAirState state{};
    SubmarineAirWarning warning{SubmarineAirWarning::none};
    bool drowned{false}; // air went below zero: vtable[70h] DestroyAndBroadcast(1)
    bool submerged{false};
};

// 00855250, complete. `hull_world_y` is unit+100h after the pose refresh and
// `periscope_band_y` is bands[1]: the breathing line is the periscope band plus
// three metres, so a boat at ordered periscope depth is still using up air.
// `already_dead` is unit+5Dh, the guard that stops a sinking boat drowning.
SubmarineAirStepResult submarine_step_air_00855250(const SubmarineAirState& state,
                                                   float dt,
                                                   float hull_world_y,
                                                   float periscope_band_y,
                                                   const SubmarineAirRates& rates,
                                                   const SubmarineDepthSettings& settings,
                                                   bool already_dead) noexcept;

// ---------------------------------------------------------------------------
// Crush depth
// ---------------------------------------------------------------------------

inline constexpr float kSubCrushTickSeconds = 1.0f; // 008551D8 COMISS against 1.0

struct SubmarineCrushStepResult {
    float accumulator{0.0f}; // +1284h
    bool pulsed{false};      // the accumulator passed one second on this step
    float damage{0.0f};      // the argument to vtable[1ACh] BSP_UnitInstance_AddDamage
    bool report{false};      // 00977370 BSP_WarningManager_ReportSubmarineDepthDamage
};

// 008551C0, complete. The limit is the global setting, not a class key: every
// submarine crushes at the same depth and SwimDepth3 only bounds what the player
// can order. The damage scales with the accumulated interval, not the frame dt.
SubmarineCrushStepResult submarine_step_crush_008551c0(
    float accumulator, float dt, float hull_world_y,
    const SubmarineDepthSettings& settings) noexcept;

// ---------------------------------------------------------------------------
// The band the physics actually chases
// ---------------------------------------------------------------------------

// 00936DC0 00936E2B..00936E9E. The commanded band is overridden twice before it
// becomes a target depth. `catapult_wants_surface` is 008522C0: unit+1279h set
// and some child answers IsKindOf(28h) (MCatapult) with child+490h >= 0, i.e. a
// seaplane boat surfacing for a pending launch.
struct SubmarineEffectiveBandInputs {
    SubmarineDepthBand commanded{SubmarineDepthBand::surface};
    bool need_air{false};               // +1281h
    bool catapult_wants_surface{false}; // 008522C0
    bool kamikaze_class{false};         // class +510h/+514h
    bool dead{false};                   // unit+5Dh
};
SubmarineDepthBand submarine_effective_depth_band_00936dc0(
    const SubmarineEffectiveBandInputs& in) noexcept;

// 00936DC0 00936EA5..00936EE6. `clearance_y` is unit+123Ch, the scan output of
// 00855420. A clamped target also raises the depth-curve gain from 1.0 to 1.5
// (DAT_00CE380C), which is what makes the boat pull up harder off the seabed.
inline constexpr float kSubClearanceClampGain = 1.5f;
struct SubmarineDepthTarget {
    float target_y{0.0f};
    float gain{kSubAirFull};
    bool clamped_by_seabed{false};
};
SubmarineDepthTarget submarine_depth_target_00936dc0(float band_y, float clearance_y,
                                                     bool clearance_valid) noexcept;

// 00855420 00855806..0085583C, the one formula of the scan this packet
// reconstructs. `sample_y` is the provider height at the footprint point and
// `class_height` is descriptor +A8h; a zero sample means the provider declined
// and is skipped by the caller.
inline constexpr float kSubClearanceMargin = 3.0f; // 00D7A2B0, a double
float submarine_accumulate_clearance_00855420(float accumulated_y, float sample_y,
                                              float class_height) noexcept;

// ---------------------------------------------------------------------------
// The periscope
// ---------------------------------------------------------------------------

enum class SubmarinePeriscopeState : int {
    stowed = 0,   // 00853CAB at attach
    raised = 1,   // 00854F48
    broken = 2,   // 009373C0 at 009373E9
};

inline constexpr float kSubPeriscopeAutoRaiseWindow = 2.5f; // 00CE3DE0, a double
inline constexpr float kSubPeriscopeOutMargin = 1.0f;       // 00D7A210, a double
inline constexpr float kSubPeriscopeRaiseRate = 5.0f;       // 00D7A370, a double
inline constexpr float kSubPeriscopeLowerRate = 3.0f;       // 00D7A2B0, a double
inline constexpr float kSubPeriscopeMoveRangeDefault = 10.0f; // DAT_00CE38B8
inline constexpr float kSubPeriscopeRepairTimeInstalled = 60.0f; // settings +4C4h

// 00854ED7..00854F4E. The auto-raise fires only for an explicitly enabled boat
// (+1235h, written only by the two reflection setters 00464320 and 004649B0)
// that is ordered to periscope depth and whose hull Y is strictly inside
// bandY +/- kSubPeriscopeAutoRaiseWindow.
bool submarine_periscope_should_auto_raise_00854650(bool auto_raise_enabled,
                                                    SubmarineDepthBand commanded,
                                                    SubmarinePeriscopeState state,
                                                    float hull_world_y,
                                                    float periscope_band_y) noexcept;

// 00855039..00855057. periscopeOut is cleared every frame at 00854B00 and set
// only here, so it is a per-frame output and not a latch. 00852B90 reads it to
// pick PeriscopeOut over PeriscopeIn.
bool submarine_periscope_is_out_00855057(float mast_local_y, float periscope_rest_y,
                                         float periscope_move_range) noexcept;

// 00854E44..00854ECF. The clock snapshots at the break and the deadline is
// PeriscopeRepairTime past it, so this is an absolute comparison and not a
// countdown. dt is scaled by FailureRepairMultiplier (settings +3D0h, installed
// 3) when unit+A44h is 1.
struct SubmarinePeriscopeRepair {
    float deadline{0.0f}; // +1254h
    float clock{0.0f};    // +1258h
    float clock_fast{0.0f}; // +125Ch, advances at twice the rate and is not tested
};
inline constexpr float kSubFailureRepairMultiplierInstalled = 3.0f;
struct SubmarinePeriscopeRepairStep {
    SubmarinePeriscopeRepair repair{};
    bool completed{false}; // 0092BEC0: unhide the mast, restore its collision
};
SubmarinePeriscopeRepairStep submarine_step_periscope_repair_00854650(
    const SubmarinePeriscopeRepair& repair, float dt, bool repair_boosted) noexcept;

// 009373C0 and its reference-free twin 009327F0. No caller of
// 00977500 BSP_WarningManager_ReportSubmarinePeriscopeBroken exists anywhere in
// the image, so the break is silent by design or by omission; see
// docs/SUBMARINE_MODEL.md section 3.
SubmarinePeriscopeRepair submarine_break_periscope_009373c0(
    float clock_now, float periscope_repair_time) noexcept;

// The sensor state 00852B90 answers, expressed against the reconstructed
// fields. The band boundaries are 00852B90's own and stay there; this is only
// the periscopeOut half, which the sensor reads and this packet produces.
SensorCategory submarine_periscope_sensor_state(bool periscope_out) noexcept;

// ---------------------------------------------------------------------------
// The motion tick tail
// ---------------------------------------------------------------------------

// Integration boundary for 00855420's tail, in call order. Each method is one
// native call site; there are no default implementations and nothing here
// stands in for unrecovered behaviour. The whole tick is not modelled: only the
// tail from 008558F1, which is the part whose rules this packet recovered.
struct SubmarineMotionTickHost {
    virtual ~SubmarineMotionTickHost() = default;

    // 00855250 at 0085591C, ECX = EBP = the unit. Reports and the drown path are
    // the host's: this call site only forwards dt.
    virtual void report_air_low() = 0;      // 00977050 at 00855375
    virtual void report_air_critical() = 0; // 009771E0 in 00855250's critical arm
    // vtable[70h] = 0077D1A0 BSP_UnitInstance_DestroyAndBroadcast(1)
    virtual void destroy_and_broadcast(int recurse) = 0;

    // 008551C0 at 0085592B.
    // vtable[1ACh] = 0095DA00 BSP_UnitInstance_AddDamage(float)
    virtual void add_damage(float amount) = 0;
    virtual void report_depth_damage() = 0; // 00977370 at 0085523C

    // 00414DB0 BSP_EntityPose_RefreshWorld, called by both models when
    // unit+C8h is clear. Must leave unit+100h current.
    virtual float refresh_and_read_hull_world_y() = 0;
};

// The mutable submarine state the tail touches.
struct SubmarineStepState {
    SubmarineAirState air{};
    float crush_accumulator{0.0f}; // +1284h
    bool dead{false};              // unit+5Dh
};

// 00855420 008558F1..00855935: air first, then crush, both with the tick's own
// dt. The clearance scan that precedes them in the same body is not modelled
// here; see docs/SUBMARINE_MODEL.md section 7 for its coverage.
void submarine_run_motion_tick_tail_00855420(SubmarineMotionTickHost& host,
                                             SubmarineStepState& state, float dt,
                                             float periscope_band_y,
                                             const SubmarineAirRates& rates,
                                             const SubmarineDepthSettings& settings);

}  // namespace bsp
