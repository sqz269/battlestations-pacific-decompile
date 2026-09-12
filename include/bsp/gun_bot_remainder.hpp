// The parts of the gun-bot family docs/GUN_BOT_TICKS.md left partial: the
// torpedo intercept solver, the entity lead-point predictor behind vtable slot
// 100h, DepthChargeBot's whole tick, AAGunnerBot's fire byte and the two-
// dimensional crossing test the torpedo run is checked against.
//
// Evidence: docs/GUN_BOT_REMAINDER.md. Every routine here is a projection of one
// native body and the coverage of each is recorded in that document's routine
// table. Names are hypotheses, not recovered symbols, except the per-level
// parameter names, which are Lua key string literals in the image
// (docs/ROBOT_CONFIG.md).
//
// Reused rather than redeclared: gun_bot_ticks.hpp for GunBotClass and the
// shared bot constants, bot_fire_target.hpp for GunAimAngles.
#ifndef BSP_GUN_BOT_REMAINDER_HPP
#define BSP_GUN_BOT_REMAINDER_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/gun_bot_ticks.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kTorpedoInterceptPointAddress = 0x008FBB00u;
inline constexpr std::uint32_t kTorpedoInterceptTimeAddress = 0x008FB8D0u;
inline constexpr std::uint32_t kEntityLeadPointDefaultAddress = 0x0042D810u;
inline constexpr std::uint32_t kShipLeadPointAddress = 0x00816650u;
inline constexpr std::uint32_t kShipSectionListFindAddress = 0x0093A570u;
inline constexpr std::uint32_t kDepthChargeBotTickAddress = 0x008FC080u;
inline constexpr std::uint32_t kSegmentCrossingXZAddress = 0x004F3730u;
inline constexpr std::uint32_t kSubmarineIsSubmergedAddress = 0x008527E0u;
inline constexpr std::uint32_t kEntityAsKind6Address = 0x008FE140u;
inline constexpr std::uint32_t kTorpedoSpreadFlipAddress = 0x00951FC0u;
inline constexpr std::uint32_t kAimResolveRayToWorldAddress = 0x00957740u;
inline constexpr std::uint32_t kDeviceAimDirectionToPairAddress = 0x00955830u;
inline constexpr std::uint32_t kDirectionFromPitchYawAddress = 0x004B4D80u;

// Entity vtable slots this packet settles.
inline constexpr std::size_t kEntitySlotWorldVelocity = 0x34;  // 00470BA0 / 00812090 / 007BBB70
inline constexpr std::size_t kEntitySlotLeadPoint = 0x100;     // 0042D810 / 00816650

// ---------------------------------------------------------------------------
// Constants, each with the address it was read from
// ---------------------------------------------------------------------------
inline constexpr float kInterceptCoefficientEpsilon = 1.0e-4f;     // 00D7A268 / 00D0D098, doubles
inline constexpr float kInterceptDiscriminantFactor = 4.0f;        // 00D7A328, a double
inline constexpr float kDepthChargeTargetDepthCeiling = -2.0f;     // 00CE7D7C
inline constexpr float kDepthChargeSinkDepthBias = 15.0f;          // 00CF3F20, a double
inline constexpr float kDepthChargeRadiusFloorSquared = 10000.0f;  // 00CE3D64
inline constexpr float kDepthChargeThinkPeriod = 0.1f;             // 00D17D3C
inline constexpr float kSubmarineSubmergedBias = 3.0f;             // 00D7A2B0, a double
inline constexpr float kSegmentCrossingParameterMax = 1.0f;        // 00D7A24C
inline constexpr float kAAGunnerFireAngleSum = 0.0872664675f;      // 00CF0098, a double, five degrees
inline constexpr float kShipLeadHullTaperStart = 0.6f;             // 00CE3D30
inline constexpr float kShipLeadHullTaperEnd = 1.0f;               // FLD1 at 0081692E
inline constexpr float kShipLeadHullTaperFloor = 0.1f;             // 00D7A2F0
inline constexpr float kShipLeadHalfExtentScale = 0.5f;            // 00D7A280, a double
inline constexpr float kShipLeadVerticalExtentScale = 0.25f;       // 00D7A348, a double
inline constexpr float kAAGunnerLeadBoxWide = 0.8f;                // 00CE74F8
inline constexpr float kAAGunnerLeadBoxUp = 0.5f;                  // 00CE3800
inline constexpr float kAAGunnerSectionChanceOverride = -1.0f;     // 00D7A260
inline constexpr float kTorpedoFriendlyScanRadiusSquared = 4000000.0f; // 00D09FE8, a double
inline constexpr float kTorpedoFriendlyProjectionSeconds = 1000.0f;    // 00CE47A0, a double
inline constexpr float kTorpedoRangeOwnerScale = 1.5f;             // 00CE3D78, a double
inline constexpr float kTorpedoRangeBotBias = 100.0f;              // 00D7A220, a double

// ---------------------------------------------------------------------------
// The intercept, 008FB8D0 and 008FBB00
// ---------------------------------------------------------------------------
// 008FB8D0 solves a quadratic in the run time. It is NOT the exact constant-
// bearing intercept: the exact equation is
//     (|v|^2 - speed^2) t^2 - 2 dot(d, v) t + |d|^2 = 0,  d = shooter - target,
// and the native builds
//     (|v|^2 - speed^2) t^2 -   dot(d, v) t + |d|^2 = 0
// then applies the textbook formula with a full `b^2 - 4ac` discriminant
// (00D7A328). The factor of two on the linear term is missing, so every
// solution under-leads a target that is not stationary and not crossing exactly
// abeam. See docs/GUN_BOT_REMAINDER.md section 2.
struct TorpedoInterceptRoots {
    int root_count = 0;    // the routine's EAX: 0, 1 or 2
    float first = 0.0f;    // *param_5
    float second = 0.0f;   // *param_6, only written when root_count is 2
};

// 008FB8D0, __fastcall(ECX shooter, EDX target, float speed, const float3* target_velocity,
// float* t1, float* t2) -> int, RET 10h, body 008FB8D0..008FBAFB.
TorpedoInterceptRoots torpedo_intercept_time_008fb8d0(const std::array<float, 3>& shooter,
                                                      const std::array<float, 3>& target,
                                                      float speed,
                                                      const std::array<float, 3>& target_velocity) noexcept;

// 008FBB00, __fastcall(ECX shooter, EDX target, float speed, const float3* target_velocity,
// float3* out) -> bool, RET 0Ch, body 008FBB00..008FBC08. One root uses `first`,
// two roots use `second`; the point is target + t * target_velocity.
bool torpedo_intercept_point_008fbb00(const std::array<float, 3>& shooter,
                                      const std::array<float, 3>& target,
                                      float speed,
                                      const std::array<float, 3>& target_velocity,
                                      std::array<float, 3>& out) noexcept;

// The exact intercept the native formula approximates, for the difference the
// test pins. Returns a negative time when no forward solution exists.
float torpedo_intercept_time_exact(const std::array<float, 3>& shooter,
                                   const std::array<float, 3>& target,
                                   float speed,
                                   const std::array<float, 3>& target_velocity) noexcept;

// ---------------------------------------------------------------------------
// Entity vtable slot 100h, the lead point
// ---------------------------------------------------------------------------
// Both implementations are `__thiscall float3* (float3* out, const float3* box,
// const float3* origin, float section_chance, float w0, float w1, float w2)`,
// RET 1Ch, and both return `out`. Every caller passes the zero global 00F87574
// as `origin` or an all-zero stack triple, so `out` is an offset in the
// target's own frame.

// 00816650's three named sections. `id` is the value the routine passes to
// 0093A570; `present` is the byte that follows the point in the ship.
struct ShipLeadSection {
    std::array<float, 3> point{{0.0f, 0.0f, 0.0f}};
    bool present = false;
    int id = 0;
};

// The three records, in the order 00816650 accumulates their weights.
struct ShipLeadSections {
    ShipLeadSection engine_room; // ship+0A88h, flag +0A94h, id 5,  weight EngineRoomWeight
    ShipLeadSection magazine;    // ship+0A68h, flag +0A74h, id 8,  weight MagazineWeight
    ShipLeadSection fuel_tank;   // ship+0A78h, flag +0A84h, id 6,  weight FueltankWeight
};

// [ship+538h]+0A0h/+0A4h/+0A8h, the hull box the fallback samples.
struct ShipHullExtents {
    float width = 0.0f;  // +0A0h, halved for the z offset
    float length = 0.0f; // +0A4h, halved for the x offset
    float height = 0.0f; // +0A8h, quartered for the y offset
};

// The four random draws 00816650 makes, in the order it makes them, so a caller
// can replay a run without an RNG. `pick` is drawn over [0, total_weight).
struct ShipLeadRandomDraws {
    float section_roll = 0.0f; // rand(0, 1) at 0081667C
    float pick = 0.0f;         // rand(0, sum - 1e-4) at 00816796
    float box_x = 0.0f;        // rand(-box.x, box.x) at 00816883
    float box_y = 0.0f;        // rand(0, box.y) at 008168A0
    float box_z = 0.0f;        // rand(-box.z, box.z) at 008168D5
};

// 0042D810, the base, plane, land-vehicle and land-fort implementation: the
// zero offset, whatever the arguments are.
std::array<float, 3> entity_lead_point_0042d810() noexcept;

// 00816650, the ship implementation. `section_available` is the answer
// 0093A570(ship+0A20h, id) gives, negated at each call site: a section whose id
// is already in that vector is skipped.
std::array<float, 3> ship_lead_point_00816650(const ShipLeadSections& sections,
                                              const ShipHullExtents& hull,
                                              const std::array<float, 3>& box,
                                              const std::array<float, 3>& origin,
                                              float section_chance,
                                              float engine_room_weight,
                                              float magazine_weight,
                                              float fuel_tank_weight,
                                              const ShipLeadRandomDraws& draws,
                                              bool engine_room_available,
                                              bool magazine_available,
                                              bool fuel_tank_available) noexcept;

// 00816913, the taper the x offset is scaled by: 1.0 up to |box_z| = 0.6 and
// 0.1 at |box_z| = 1.0, clamped at both ends by 00419010.
float ship_lead_length_taper_00816941(float box_z_draw) noexcept;

// ---------------------------------------------------------------------------
// 008FC080, DepthChargeBot's tick
// ---------------------------------------------------------------------------
// One 0x14-byte level record, read by 008FD880 from the Lua keys named here.
struct DepthChargeBotLevel {
    float attack_dist = 0.0f;          // +0Ch AttackDist
    float bullet_throw_mul = 0.0f;     // +10h BulletThrowMul, unread by the tick
    float continuous_fire_time = 0.0f; // +14h ContinuousFireTime
    float fire_delay_min = 0.0f;       // +18h FireDelay
    float fire_delay_max = 0.0f;       // +1Ch FireDelay
};

struct DepthChargeBotState {
    float fire_delay = 0.0f;  // bot+60h, counts down every frame
    float think_delay = 0.0f; // bot+5Ch, reset to 0.1f on every pass
};

// 008FC23E: the time a charge dropped now needs to reach the target's depth.
// The bias and the divisor are the weapon descriptor's own sink rate at +0DCh.
float depth_charge_sink_time_008fc23e(float owner_height, float target_height,
                                      float sink_rate) noexcept;

// 008FC2B0..008FC354. Both distances are horizontal (x and z only) and squared;
// the radius is squared and floored at 10000 before the comparison.
struct DepthChargeRangeInputs {
    std::array<float, 3> owner_position{{0.0f, 0.0f, 0.0f}};     // owner+0FCh
    std::array<float, 3> target_position{{0.0f, 0.0f, 0.0f}};    // target+0FCh
    std::array<float, 3> predicted_position{{0.0f, 0.0f, 0.0f}}; // target+0FCh + velocity * t
    float attack_dist = 0.0f;
};
bool depth_charge_in_range_008fc354(const DepthChargeRangeInputs& in) noexcept;

// 008FC356..008FC386: what the byte handed to gun->vtable[1E8h] ends up being.
struct DepthChargeFireInputs {
    bool in_range = false;       // 008FC354
    float fire_delay = 0.0f;     // bot+60h after this frame's decrement
    bool has_muzzle = false;     // [gun+3F0h] != 0
    bool muzzle_inhibited = false; // bit 3 of [[gun+3F0h]+634h]
};
bool depth_charge_fire_byte_008fc386(const DepthChargeFireInputs& in) noexcept;

// One virtual per native call site 008FC080 makes.
struct DepthChargeBotHost {
    virtual ~DepthChargeBotHost() = default;

    virtual void* resolve_fire_target_00521ea0() = 0;          // 008FC08A, bot+38h
    virtual GunBotTargetValidity target_validity() = 0;        // 008FC098..008FC0BA
    virtual void clear_fire_target_slot38() = 0;               // 008FC0D2
    virtual void run_idle_rest_timer_008fbce0(float dt) = 0;   // 008FC0E2
    virtual bool side_enabled_00927f10() = 0;                  // 008FC11E
    virtual bool gun_present() = 0;                            // 008FC12B, bot+58h
    virtual void* fire_target_entity_slot44() = 0;             // 008FC13C and four more
    virtual float weapon_sink_rate() = 0;                      // 008FC17F, [[gun+3F8h]+34h]+0DCh
    virtual std::array<float, 3> owner_world_position() = 0;   // 008FC1A1, bot+50h +0FCh
    virtual std::array<float, 3> target_world_position() = 0;  // 008FC1E6, target+0FCh
    virtual std::array<float, 3> target_world_velocity_slot34() = 0; // 008FC28A
    virtual DepthChargeBotLevel level_parameters() = 0;        // [00E19988] + 14h * bot+34h
    virtual bool muzzle_fire_inhibited() = 0;                  // 008FC377, bit 3 of +634h
    virtual void set_trigger_slot1e8(bool held) = 0;           // 008FC232 and 008FC39A
    virtual float random_range_00bd2f10(float low, float high) = 0; // 008FC3E2
};

// 008FC080, body 008FC080..008FC3F2, __thiscall(bot)(float dt), RET 4.
void depth_charge_bot_tick_008fc080(DepthChargeBotHost& host, DepthChargeBotState& state,
                                    float dt);

// ---------------------------------------------------------------------------
// 00902920, AAGunnerBot's fire byte
// ---------------------------------------------------------------------------
// 00902F7A..009030A8, read from the listing. The byte is zero unless every test
// passes; the tail is a jump into gun->vtable[1E8h] with it.
struct AAGunnerFireByteInputs {
    bool weapon_mount_present = false; // [gun+3F8h] != 0
    bool weapon_descriptor_present = false; // [[gun+3F8h]+34h] != 0
    bool aim_accepted = false;         // 0085ABA0's answer at 00902FB0
    float distance = 0.0f;
    float max_range = 0.0f;            // [weapon descriptor +60h]
    float commanded_horz = 0.0f;
    float commanded_vert = 0.0f;
    float gun_horz = 0.0f;             // gun+480h
    float gun_vert = 0.0f;             // gun+484h
    bool has_muzzle = false;           // [gun+3F0h] != 0
    bool muzzle_inhibited = false;     // bit 0 of [[gun+3F0h]+634h]
};
bool aa_gunner_fire_byte_00902920(const AAGunnerFireByteInputs& in) noexcept;

// ---------------------------------------------------------------------------
// The friendly-crossing test, 004F3730
// ---------------------------------------------------------------------------
// __fastcall(ECX a0, EDX a1, float2* b0, float2* b1, float2* out) -> bool,
// RET 0Ch, body 004F3730..004F3801. Two dimensional: only [0] and [1] of each
// argument are read, and 008FFF20 packs (x, z) into them. Both segment
// parameters must land in [0, 1].
struct SegmentCrossingXZ {
    bool crossed = false;
    std::array<float, 2> point{{0.0f, 0.0f}};
};
SegmentCrossingXZ segment_crossing_004f3730(const std::array<float, 2>& a0,
                                            const std::array<float, 2>& a1,
                                            const std::array<float, 2>& b0,
                                            const std::array<float, 2>& b1) noexcept;

// 008527E0, __fastcall(entity) -> bool, RET, body 008527E0..0085281A. The
// submarine is shallow enough to be shot at when its world height is above its
// own +1204h reference minus three metres.
bool submarine_is_shallow_008527e0(float entity_height, float depth_reference) noexcept;

} // namespace bsp

#endif // BSP_GUN_BOT_REMAINDER_HPP
