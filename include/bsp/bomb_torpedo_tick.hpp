#pragma once
// The bomb-family tick and the torpedo swim model.
//
// Addresses: 006E1300 (tick advance), 006E1060 (place pose), 006E1010 (commit),
// 006E1510 (air advance), 006E27F0 (bomb/rocket water advance), 006E2800 (sweep
// flags), 006E2570 (water query), 006E6900 (owner grace), 006FCD20 (depth-charge
// dive), 0080A000 (rocket ignition and thrust), 00855B00 (torpedo commit),
// 00855A90 (torpedo class finalise), 00855F00 (torpedo water query), 00857480
// (torpedo swim), 00856BB0 (torpedo steering), 008568E0 (torpedo water entry).
//
// Evidence lives in docs/BOMB_FAMILY_TICK.md and docs/TORPEDO_TICK.md. Every
// name below is a hypothesis, not a recovered symbol. These are new C++
// interfaces over the recovered rules, not binary-compatible replacements: the
// native routines are __thiscall methods on the projectile record and reach the
// world through virtual slots, and nothing here reproduces that ABI.
#include <cstddef>
#include <cstdint>

#include "bsp/projectile_impact.hpp"
#include "bsp/projectile_kinds.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Constants, each with the address its bytes were read from.
// ---------------------------------------------------------------------------

// 00D7A390 / 00D7A3A0, the two doubles of the pitch blend in 006E1510. Both are
// float literals widened by the compiler (0.9f and 0.1f).
inline constexpr float kBombPitchBlendKeep = 0.9f;
inline constexpr float kBombPitchBlendAdd = 0.1f;

// 00D7A268, the squared-length gate on the air trail segment in 006E1060.
inline constexpr float kBombTrailMinSegmentSquared = 1.0e-4f;

// 00CFBC80 and 00CFBC84, read by 006FCD20. The first is a squared radius, so
// the depth-charge proximity radius is 25 m; the second is the depth at which
// the scan starts and is reused as the torpedo's vertical speed floor.
inline constexpr float kDepthChargeProximityRadiusSquared = 625.0f;
inline constexpr float kDepthChargeScanDepth = -5.0f;

// 00D7A2B0, the double of the levelling term in 00857480 (3.0, not a widened
// float), and the 00CE3850 / 00CFBC84 pair that clamps the torpedo's rise and
// sink rate in 00856BB0.
inline constexpr float kTorpedoLevelRate = 3.0f;
inline constexpr float kTorpedoMaxRiseSpeed = 5.0f;
inline constexpr float kTorpedoMaxSinkSpeed = -5.0f;

// 00D0C310, the sentinel 00857480 tests the commanded heading against before it
// seeds the heading from the hull's own yaw.
inline constexpr float kTorpedoHeadingUnset = 10000.0f;

// 00CEFF98, the factor 00855A90 folds into the cached torpedo range.
inline constexpr float kTorpedoRangeFactor = 0.6f;

// 008E6430 keys: 0Eh for the wake gate in 00857480, 0Fh for the heading-rate
// modifier in 00856BB0. Both are indices into the gameplay modifier table.
inline constexpr int kTorpedoWakeModifierKey = 0x0E;
inline constexpr int kTorpedoHeadingModifierKey = 0x0F;

// The capability tag the homing scan passes to the candidate's vtable[5Ch] at
// 00856C5F and re-tests at 00856E4D.
inline constexpr int kTorpedoHomingTargetTag = 8;

// ---------------------------------------------------------------------------
// Record offsets. The bomb family record is the bullet record with 1A0h bytes
// inserted before the shot interface, so every shared field below is the
// bsp::kProjectileOff* value plus 1A0h; the static_asserts in the .cpp check
// that against docs/PROJECTILE_IMPACT.md. Offsets are given from the record
// base even where the native code addresses them through the tick element at
// +3E4h with a negative displacement.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kBombRecordShotInterfaceShift = 0x1A0;

inline constexpr std::size_t kBombOffLocalMatrix = 0x74;        // 006E1510, 00857480
inline constexpr std::size_t kBombOffLocalForwardRow = 0x94;    // +74h row 2
inline constexpr std::size_t kBombOffPoseValid = 0xC8;          // 006E10F9
inline constexpr std::size_t kBombOffWorldMatrix = 0xCC;        // 006E1010
inline constexpr std::size_t kBombOffWorldForwardRow = 0xEC;    // 00855F0B, 00856D95
inline constexpr std::size_t kBombOffWorldTranslation = 0xFC;   // 006E1412
inline constexpr std::size_t kBombOffTrailWaterEffect = 0x334;  // 006E1236
inline constexpr std::size_t kBombOffTrailAirEffect = 0x338;    // 006E10E4
inline constexpr std::size_t kBombOffAcceleration = 0x348;      // 006E153E
inline constexpr std::size_t kBombOffFlightTime = 0x364;        // 006E147C
inline constexpr std::size_t kBombOffSweepExtra = 0x368;        // 006E1418
inline constexpr std::size_t kBombOffSnapshotCurrent = 0x370;   // 006E1013
inline constexpr std::size_t kBombOffSnapshotPrevious = 0x37C;  // 006E1017
inline constexpr std::size_t kBombOffOwnerGrace = 0x3A4;        // 006E6900 via +310h
inline constexpr std::size_t kBombOffOwner = 0x3D8;             // 006E1400
inline constexpr std::size_t kBombOffTickElement = 0x3E4;       // the vtable tables
inline constexpr std::size_t kBombOffLaunchDelay = 0x410;       // +3E4h +2Ch
inline constexpr std::size_t kBombOffCachedMatrix = 0x418;      // +3E4h +34h
inline constexpr std::size_t kBombOffCachedTranslation = 0x448; // +3E4h +64h
inline constexpr std::size_t kBombOffActive = 0x458;            // +3E4h +74h
inline constexpr std::size_t kBombOffArmingDelay = 0x45C;       // +3E4h +78h

// Depth-charge-only tail of the 474h record (006FCD28, 006FCFA6). Their
// producer is unread: the create 006FD210 only allocates and zeroes.
inline constexpr std::size_t kDepthChargeOffDragCoefficient = 0x468;
inline constexpr std::size_t kDepthChargeOffDetonationDepth = 0x470;

// Rocket-only tail of the 498h record (0080A024, 0080A02E).
inline constexpr std::size_t kRocketOffIgnitionTimer = 0x490;
inline constexpr std::size_t kRocketOffIgnited = 0x494;

// Torpedo-only tail of the 51Ch record. Only +47Ch has a known producer, the
// Lua binding 008A2710; the rest are read by 00857480 and 00856BB0 and no
// producer was found in the create, the constructor or the water entry.
inline constexpr std::size_t kTorpedoOffCommandedHeading = 0x46C;    // 00857490
inline constexpr std::size_t kTorpedoOffWakeParameter = 0x470;       // 008576C6
inline constexpr std::size_t kTorpedoOffAxialDrag = 0x474;           // 00857603
inline constexpr std::size_t kTorpedoOffLateralDrag = 0x478;         // 008575C4
inline constexpr std::size_t kTorpedoOffDepthGain = 0x480;           // 00856FF2
inline constexpr std::size_t kTorpedoOffSurfaceGain = 0x484;         // 00856FF8
inline constexpr std::size_t kTorpedoOffRunTime = 0x488;             // 0085748A
inline constexpr std::size_t kTorpedoOffCommitMirrorSource = 0x4A0;  // 00855B08
inline constexpr std::size_t kTorpedoOffCommitMirrorMatrix = 0x4A4;  // 00855B16
inline constexpr std::size_t kTorpedoOffTargetObserver = 0x4FC;      // 00856C12
inline constexpr std::size_t kTorpedoOffTarget = 0x510;              // 00856C09
inline constexpr std::size_t kTorpedoOffScanInterval = 0x514;        // 00856BEE
inline constexpr std::size_t kTorpedoOffScanCountdown = 0x518;       // 00856BD6

// 00855A90 caches two derived fields on the class descriptor.
inline constexpr std::size_t kTorpedoClassOffCachedRange = 0x60;      // 00855AAB
inline constexpr std::size_t kTorpedoClassOffTerminalFallSpeed = 0xEC; // 00855AD3

// ---------------------------------------------------------------------------
// Plain value types. Vector3 is a local projection: the native records hold
// three consecutive floats, not a class.
// ---------------------------------------------------------------------------
struct BombVector3 {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// A 4x4 row-major affine matrix as the native code addresses it: row 0 is the
// right axis at +0h, row 1 the up axis at +10h, row 2 the forward axis at +20h
// and row 3 the translation at +30h (006E11B8 copies 10h dwords, 00855B16 and
// 006E1058 copy through 004134F0, whose ECX is the destination).
struct BombMatrix4x3 {
    BombVector3 right{};
    BombVector3 up{};
    BombVector3 forward{};
    BombVector3 translation{};
};

// The four descriptor fields 006E1300 and 006E1510 read, plus the two the
// timeout branch needs. Offsets are the kWeaponClassOff* values.
struct BombFlightClass {
    float time_scale{1.0f};   // +5Ch
    float fly_time{0.0f};     // +54h
    bool no_gravity{false};   // +20h
    bool has_timeout_effect{false}; // +D4h non-zero
};

// The mode 006E2570 and 00855F00 answer with. Zero selects the air advance at
// vtable[19Ch]; anything else selects the water advance at vtable[1A0h]. The
// torpedo's extra value 2 is produced but never distinguished by either caller.
enum class BombWaterMode : int {
    Air = 0,
    Water = 1,
    TorpedoBelowTwiceSwimDepth = 2,
};

// ---------------------------------------------------------------------------
// Pure rules. Each takes the step the tick already scaled by the class time
// scale, except bomb_scaled_step itself.
// ---------------------------------------------------------------------------

// 006E1341: the tick and the place pose both rescale the incoming step by the
// class time scale before anything else.
float bomb_scaled_step(float step, float time_scale) noexcept;

// 006E2570 and 00855F00. `swim_depth` and `world_y` are only read on the
// torpedo path; `in_water` is the shot interface byte at +44h.
BombWaterMode bomb_water_mode(bool in_water) noexcept;
BombWaterMode torpedo_water_mode(bool in_water, float world_y, float swim_depth) noexcept;

// 006E1510, the advance every bomb-family kind but the rocket runs in air.
// Gravity is skipped when the class sets NoGravity; the extra acceleration at
// record+348h is added unconditionally inside the same branch.
BombVector3 bomb_air_velocity(const BombVector3& velocity,
                              const BombVector3& acceleration,
                              bool no_gravity,
                              float dt) noexcept;

// 006E15EC: the forward row's vertical component is blended toward the
// normalised velocity's, so the model noses over as it falls. The caller then
// re-orthonormalises the matrix through 0085DC80, which this does not model.
float bomb_pitch_blend(float forward_y, const BombVector3& velocity) noexcept;

// 006FCD28: the depth charge keeps gravity under water and adds linear drag on
// all three axes, so it settles at 9.81 / drag_coefficient metres per second.
BombVector3 depth_charge_dive_velocity(const BombVector3& velocity,
                                       float drag_coefficient,
                                       float dt) noexcept;
float depth_charge_terminal_sink_speed(float drag_coefficient) noexcept;

// 006FCFA6: the charge fires when it is deeper than the magnitude of its
// detonation depth. The scan that precedes it only runs below 5 m (00CFBC84).
bool depth_charge_should_detonate(float world_y, float detonation_depth) noexcept;
bool depth_charge_scan_enabled(float world_y) noexcept;
bool depth_charge_in_proximity(const BombVector3& delta) noexcept;

// 0080A17C: once lit, the rocket accelerates along its world forward axis until
// the axial component of its velocity reaches VMax. Returns the new axial speed.
float rocket_axial_speed(float axial_speed, float acceleration, float v_max, float dt) noexcept;
BombVector3 rocket_thrust_velocity(const BombVector3& velocity,
                                   const BombVector3& world_forward,
                                   float axial_speed,
                                   float acceleration,
                                   float v_max,
                                   float dt) noexcept;
// 0080A06E: the pre-ignition slice of the step, and whether the motor lights on
// this step. `elapsed` is record+490h before the step.
float rocket_pre_ignition_slice(float elapsed, float ignition_delay, float dt) noexcept;
bool rocket_ignites_this_step(float elapsed, float ignition_delay, float dt) noexcept;

// 008574F0: the torpedo's forward row is pulled toward horizontal every step.
float torpedo_level_forward_y(float forward_y, float dt) noexcept;

// 00857540: the velocity is split along the hull axis and damped with two
// different coefficients, then recombined.
BombVector3 torpedo_swim_velocity(const BombVector3& velocity,
                                  const BombVector3& forward,
                                  float axial_drag,
                                  float lateral_drag,
                                  float dt) noexcept;

// 00856FA9: the depth controller. `surface_height` is what
// BSP_GameWorld_SampleWaterHeight returns for the hull's x and z. The result is
// the new vertical velocity, already clamped to the +-5 m/s pair.
float torpedo_depth_velocity(float vertical_velocity,
                             float world_y,
                             float swim_depth,
                             float surface_height,
                             float depth_gain,
                             float surface_gain,
                             float dt) noexcept;
float torpedo_clamp_vertical_speed(float vertical_velocity) noexcept;

// 00856E6C and 00856EE8: the homing turn per step. Both native clamps are
// already in radians for the step, so the rate is multiplied by dt here.
float torpedo_homing_turn(float own_angle, float target_angle, float turn_speed, float dt) noexcept;

// 008570D1: the heading controller's rate. HeadingTurn is authored in degrees
// per second and scaled by the gameplay modifier for key 0Fh.
float torpedo_heading_rate(float heading_turn_degrees, float modifier) noexcept;
float torpedo_heading_turn(float current_yaw, float commanded_yaw, float rate, float dt) noexcept;

// 00856C50: the scan keeps the candidate with the smallest squared distance.
bool torpedo_prefers_candidate(float current_distance_squared, float candidate_distance_squared) noexcept;
// 00856BD6: the countdown that gates the scan, and the value it is reset to.
bool torpedo_scan_due(float countdown, float dt) noexcept;
float torpedo_advance_scan_countdown(float countdown, float interval, float dt) noexcept;

// 00855A90: the two fields the class finalise hook caches.
float torpedo_cached_range(float water_travel_speed, float fly_time) noexcept;
float torpedo_terminal_fall_speed(float max_fall) noexcept;

// 008568E0 and 006FD660: the same water-entry rule for both kinds. The
// projectile breaks up when it is too fast or was dropped from too high.
bool water_entry_destroys(float impact_speed,
                          float vertical_speed,
                          float max_water_hit_velocity,
                          float terminal_fall_speed) noexcept;

// 006E1010 and 00855B00. The commit is a shift register: the previous snapshot
// takes the current one, the current one takes the matrix cached by the
// previous commit, and only then is the matrix re-cached from the world pose.
struct BombStepSnapshot {
    BombVector3 previous{};
    BombVector3 current{};
    BombVector3 cached{};
};
BombStepSnapshot bomb_commit_snapshot(const BombStepSnapshot& snapshot,
                                      const BombVector3& world_translation) noexcept;

// 006E1300 step 6: the arming countdown gates the collision sweep, and world
// mode 2 forces both flags 006E2800 sets to zero.
struct BombSweepFlags {
    bool first{false};  // CL at 006E1457
    bool second{false}; // DL at 006E1453
};
BombSweepFlags bomb_sweep_flags(int world_mode) noexcept;
bool bomb_sweep_armed(float arming_delay) noexcept;

// ---------------------------------------------------------------------------
// Hosts. One virtual method per native call site; no defaults, because nothing
// here stands in for unrecovered game behaviour.
// ---------------------------------------------------------------------------

// The state 006E1300 reads and writes on the record it is the tick element of.
struct BombTickState {
    float launch_delay{0.0f};  // +410h, negative until the drop is released
    bool active{false};        // +458h
    float arming_delay{0.0f};  // +45Ch
    float flight_time{0.0f};   // +364h
    bool sweep_enabled{false}; // +5Ch
    bool pose_valid{false};    // +C8h
};

struct BombTickHost {
    virtual ~BombTickHost() = default;
    // 006E1361 / 006E10A1, the shot interface's vtable[2Ch]: 006E2570 for the
    // bomb, the depth charge and the rocket, 00855F00 for the torpedo.
    virtual BombWaterMode query_water_mode() = 0;
    // 006E138B, the record's own vtable[19Ch] and [1A0h] with the scaled step.
    virtual void advance_in_air(float dt) = 0;
    virtual void advance_in_water(float dt) = 0;
    // 006E13D3, vtable[1A4h] = 006E2800 for all four kinds.
    virtual BombSweepFlags query_sweep_flags() = 0;
    // 006E1407 / 006E1049, 00414DB0 BSP_EntityPose_RefreshWorld.
    virtual void refresh_world_pose() = 0;
    // 006E145B, 0084C430 BSP_Projectile_SweepStepSegment. The eleven dwords are
    // in docs/BOMB_FAMILY_TICK.md; this carries only the two register bytes.
    virtual void sweep_step_segment(const BombSweepFlags& flags) = 0;
    // 006E14C7, 0084B6F0 with the class descriptor's TimeOutEfx slot and the
    // refreshed world position. Contract: the effect spawn is not reconstructed.
    virtual void spawn_timeout_effect() = 0;
    // 006E14D2 and 006E14DB, 00696350(record, 0) then 00926D90(record, 2).
    virtual void expire_projectile() = 0;
    virtual void release_projectile() = 0;
    // 006E14F4, 006E6900 on the shot interface: the owner grace countdown that
    // unregisters the shooter's observer pair when it runs out.
    virtual void advance_owner_grace(float dt) = 0;
    // 006E13D5, *(00E188A8)+1FE4h.
    virtual int world_mode() = 0;
};

// 006E1300, the whole tick in order. Returns false when the projectile expired.
bool bomb_family_tick_006e1300(BombTickState& state,
                               const BombFlightClass& flight_class,
                               BombTickHost& host,
                               float step);

// The steering step 00856BB0 reaches the world through these; 00857480 adds the
// wake gate. Every one is a single native call site.
struct TorpedoSwimHost {
    virtual ~TorpedoSwimHost() = default;
    // 00856DB4 and 00856C7A, 00414DB0 on the torpedo and on the target.
    virtual void refresh_world_pose() = 0;
    // 00857513 and 0085E880's tail, 0085DC80: re-orthonormalise a matrix after
    // a row has been written. Contract: the basis rebuild is not reconstructed.
    virtual void orthonormalize(BombMatrix4x3& matrix) = 0;
    // 0085E880, rotate the local matrix about one of its own rows.
    virtual void rotate_about_right(float angle) = 0;
    virtual void rotate_about_up(float angle) = 0;
    // 00856C31 through 00856D6B: the candidate walk. Returns the nearest entity
    // that answers vtable[5Ch](8), or null. Contract: the list is the recon
    // slot's +DE8h chain reached through 008053C0.
    virtual void* acquire_nearest_target() = 0;
    virtual void* current_target() = 0;
    // 00856D59 and 00856C1C, 00694A60 / 006952A0 on the observer at +4FCh.
    virtual void bind_target(void* target) = 0;
    // 00856E4F and 00856E5F: the two re-tests before the homing turn.
    virtual bool target_is_homing_kind(void* target) = 0;
    virtual bool target_suppresses_homing(void* target) = 0;
    // 00856D9F and 00856E14, 00521370: decompose a direction into pitch (asin
    // of its vertical component) and yaw (atan2).
    virtual void direction_to_pitch_yaw(const BombVector3& direction, float& pitch, float& yaw) = 0;
    virtual BombVector3 direction_to_target(void* target) = 0;
    // 00856FC6, 0078CF20 BSP_GameWorld_SampleWaterHeight.
    virtual float sample_water_height(float x, float z) = 0;
    // 008570B8 and 008576A4, 008E6430 BSP_GameplayModifiers_ProductForUnit.
    virtual float gameplay_modifier(int key) = 0;
    virtual BombMatrix4x3 world_matrix() = 0;
    virtual BombMatrix4x3 local_matrix() = 0;
    virtual void set_local_matrix(const BombMatrix4x3& matrix) = 0;
};

// The torpedo instance state 00857480 and 00856BB0 read and write.
struct TorpedoSwimState {
    float run_time{0.0f};         // +488h
    float commanded_heading{kTorpedoHeadingUnset}; // +46Ch
    float scan_countdown{0.0f};   // +518h
    float scan_interval{0.0f};    // +514h
    float swim_depth{0.0f};       // +47Ch
    float axial_drag{0.0f};       // +474h
    float lateral_drag{0.0f};     // +478h
    float depth_gain{0.0f};       // +480h
    float surface_gain{0.0f};     // +484h
    BombVector3 velocity{};       // +318h
    bool pose_valid{false};       // +C8h
};

// The three descriptor fields the torpedo steering reads.
struct TorpedoSwimClass {
    float heading_turn_degrees{0.0f};   // +E8h
    float homing_horz_turn_speed{0.0f}; // +F0h
    float homing_vert_turn_speed{0.0f}; // +F4h
};

// 00856BB0. Returns true when the homing branch took both turns and returned
// early, which is the case in which neither the depth controller nor the
// heading controller runs this step.
bool torpedo_steer_00856bb0(TorpedoSwimState& state,
                            const TorpedoSwimClass& swim_class,
                            TorpedoSwimHost& host,
                            float dt);

// 00857480, the torpedo's water advance in order. The wake tail after
// 008576D6 is not reconstructed.
void torpedo_swim_00857480(TorpedoSwimState& state,
                           const TorpedoSwimClass& swim_class,
                           TorpedoSwimHost& host,
                           float dt);

} // namespace bsp
