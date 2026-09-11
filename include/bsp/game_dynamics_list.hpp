#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/mission_state_frame.hpp"

// The dynamics list at game+30h: detached rigid bodies floating in water.
//
// docs/GAME_DYNAMICS_LIST.md carries the evidence. bsp/mission_state_frame.hpp
// already declares the release side (release_all_dynamics_00447060,
// GameDynamicsList, kDynamicsRecordStride, kDynamicsRecordHandleOffset,
// kDynamicsPendingStride, kDynamicsOwnerOffset); those are reused here, never
// redefined. This header adds the record layout its producer 00447510 writes,
// the display-frame pass 00447b80 and the fixed-step buoyancy pass 004462d0.
//
// Every name below is a hypothesis, not a recovered symbol.

namespace bsp {

// ---------------------------------------------------------------------------
// The record, from its producer 00447510 (00447ade..00447b40)
// ---------------------------------------------------------------------------
// kDynamicsRecordStride (0x20) and kDynamicsRecordHandleOffset (0x0c) come from
// bsp/mission_state_frame.hpp; +0Ch is the rigid body, not an opaque handle.
inline constexpr std::size_t kDynamicsRecordNodeOffset = 0x00;      // 00447ae6
inline constexpr std::size_t kDynamicsRecordLifetimeOffset = 0x04;  // 00447adc
inline constexpr std::size_t kDynamicsRecordSpecFieldOffset = 0x08; // 00447b14, no reader found
inline constexpr std::size_t kDynamicsRecordAnchorOffset = 0x10;    // 00447b0d, x/y/z at +10h/+14h/+18h
inline constexpr std::size_t kDynamicsRecordBuoyancyDivisorOffset = 0x1c; // 00447b37

// The two vectors of the game+30h object, both _SECURE_SCL MSVC vectors
// { proxy, first, last, end }.
inline constexpr std::size_t kDynamicsPendingVectorOffset = 0x00; // first at +4h,  00445db0
inline constexpr std::size_t kDynamicsRecordVectorOffset = 0x10;  // first at +14h, 00447d18

struct DynamicsVec3 {
    float x{};
    float y{};
    float z{};
};

// The 3x4 the physics interpolation produces (00c43ea0) as four rows of three,
// row 3 being the translation. 00c33650 expands it into a 4x4 by inserting a
// zero column and 1.0f at +3Ch.
struct DynamicsTransform34 {
    float m[12]{};
};

// One 20h record.
struct GameDynamicsRecord {
    std::uint32_t node{};             // +00h, the scene node
    float lifetime_seconds{};         // +04h
    float spec_field_08{};            // +08h, written by 00447510, no reader read here
    std::uint32_t body{};             // +0Ch, the rigid body
    DynamicsVec3 anchor{};            // +10h/+14h/+18h, in body space
    float buoyancy_divisor{};         // +1Ch, 004462d0 divides the force by it
};

// One 8h pending entry: a deferred model-group switch.
struct GameDynamicsPending {
    std::uint32_t owner{};  // +0h, an object with a 10h-stride group vector at +16Ch
    float seconds{};        // +4h
};

// The typed list. GameDynamicsList in bsp/mission_state_frame.hpp is the
// release-time projection of the same object and stays as it is.
struct GameDynamicsState {
    std::vector<GameDynamicsPending> pending{}; // +4h/+8h
    std::vector<GameDynamicsRecord> records{};  // +14h/+18h
};

// 00445db0, __thiscall int(list), body 00445db0..00445dc2: the pending count,
// re-read on every pass of the first loop.
std::size_t dynamics_pending_count_00445db0(const GameDynamicsState& list) noexcept;

// ---------------------------------------------------------------------------
// Constants, all read from the image
// ---------------------------------------------------------------------------
inline constexpr float kDynamicsFadeWindowSeconds = 5.0f;      // 00ce3850
inline constexpr double kDynamicsFadeDivisor = 5.0;            // 00d7a370
inline constexpr double kDynamicsKillDepth = -50.0;            // 00ce4938
inline constexpr float kDynamicsExpiringBuoyancyDivisor = 1.5f;// 00ce380c
inline constexpr float kDynamicsBuoyancyDivisorFloor = 1.0f;   // 00d7a24c, the test at 004481bc
inline constexpr double kDynamicsBuoyancyGravity = 10.0;       // 00ce3dc0
inline constexpr double kDynamicsBoxHalfExtent = 0.5;          // 00d7a280
inline constexpr float kDynamicsGroupHideOffsetY = 100000.0f;  // 00cf81f0, inside 00710bb0

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00447d8f: the leftover fixed-step accumulator 00f876ac over the fixed step.
float dynamics_interpolation_alpha(float accumulator, float fixed_step) noexcept;

// 00c43ea0, 00447da7: out[i] = prev[i] + alpha * (cur[i] - prev[i]), twelve
// floats, no clamp. The native routine reads prev at [body+4]+84h and cur at
// body+8h; the rule takes both.
DynamicsTransform34 interpolate_body_transform_00c43ea0(
    const DynamicsTransform34& previous, const DynamicsTransform34& current, float alpha) noexcept;

// 00447e0b..00447eb1: the interpolated translation row minus the anchor rotated
// by the transpose of the interpolated 3x3, which is where the node is placed.
DynamicsVec3 dynamics_node_translation(
    const DynamicsTransform34& interpolated, const DynamicsVec3& anchor) noexcept;

// 00447f99..00447fd2: the world y of the anchor under the node's own world
// matrix, column 1 at node+F4h/+104h/+114h/+124h.
float dynamics_anchor_world_y(const float node_matrix_column_y[4], const DynamicsVec3& anchor) noexcept;

// 00447fd6..00447fec: the anchor above the water plane while the body's own
// interpolated origin is below it.
bool dynamics_water_entry(float anchor_world_y, float body_world_y) noexcept;

// 00448008..0044803a. settings are 00424c40()'s +20h/+24h (velocity thresholds,
// compared negated) and +28h/+2Ch (effect ids, truncated to int). Returns -1
// when neither threshold fires or the chosen id is negative, which is the same
// thing the native code does with its JLE at 0044803a.
struct DynamicsSplashSettings {
    float small_threshold_20{};
    float large_threshold_24{};
    float small_effect_28{};
    float large_effect_2c{};
};
int dynamics_splash_effect_id(const DynamicsSplashSettings& settings, float body_velocity_y) noexcept;

// 0044814a..0044816f: the record leaves the list when its lifetime has run out
// or the body sank past the kill depth.
bool dynamics_should_release(float lifetime_seconds, float body_world_y) noexcept;

// 00448186..004481e5: inside the fade window a divisor below 1.0f is raised to
// 1.5f, so the piece stops floating while it fades. Outside the window the
// divisor is untouched.
float dynamics_expiring_buoyancy_divisor(float lifetime_seconds, float divisor) noexcept;

// 00448220..0044823d: the visibility factor during the last five seconds.
// Outside the window the native code does not call the setter at all; the rule
// returns 1.0f there so a caller can treat it as "fully visible".
float dynamics_fade_factor(float lifetime_seconds) noexcept;

// 004462d0 steps 3 and 4: the fraction of the body's bounding box below the
// water surface, clamped to [0, 1]. `axis` is the transform's +04h/+10h/+1Ch,
// `extent` the box size per axis, `center_y` the transform's +28h.
float dynamics_submersion_fraction(
    const DynamicsVec3& axis, const DynamicsVec3& extent,
    float center_y, float water_height) noexcept;

// 00710bb0's group selection as the caller computes it at 00447c1a..00447c4e:
// the last group, or 0 when the object has none.
std::size_t dynamics_last_model_group(std::size_t group_count) noexcept;

// ---------------------------------------------------------------------------
// The display-frame pass, 00447b80
// ---------------------------------------------------------------------------

struct GameDynamicsFrameHost {
    virtual ~GameDynamicsFrameHost() = default;

    // 00447c6d, 00710bb0(owner, group). Hides every model group but `group` by
    // parking its nodes 100000 units up, and shows `group`.
    virtual void select_visible_model_group_00710bb0(std::uint32_t owner, std::size_t group) = 0;

    // 00447c1a..00447c32: the owner's 10h-stride group vector at +16Ch/+170h.
    virtual std::size_t model_group_count(std::uint32_t owner) = 0;

    // 00447da7, 00c43ea0 with the alpha; the host owns both physics states.
    virtual DynamicsTransform34 interpolated_body_transform_00c43ea0(
        std::uint32_t body, float alpha) = 0;

    // 00447ede and 00447f0b, 00b6db70, skipped when node+5Ch bit 1 is set. The
    // listing performs the test and the call twice through two loads of the
    // same pointer; the reconstruction keeps both.
    virtual void refresh_node_world_matrix_00b6db70(std::uint32_t node) = 0;

    // 00447f99..00447fd2: column 1 of the node's world matrix, the four cells
    // at node+F4h, +104h, +114h, +124h.
    virtual void node_world_matrix_column_y(std::uint32_t node, float out[4]) = 0;

    // 00447ffe, 00c31f40(body, &out), the linear velocity.
    virtual DynamicsVec3 body_linear_velocity_00c31f40(std::uint32_t body) = 0;

    // 00448003, 00424c40(), the settings singleton.
    virtual DynamicsSplashSettings splash_settings_00424c40() = 0;

    // 00448046 and 0044809a as one step: acquire the effect handle for `id`
    // through 00870cd0 (the id travels in EDX, the stack argument is 1), spawn
    // it through 008685e0 with ECX from [00E188A8]+19ECh, then set the result's
    // +9h byte to 1. The ref-count pair 00ce221c/00ce2220 around it is the
    // handle's own lifetime and is not modelled.
    virtual void spawn_water_entry_effect(int effect_id, const DynamicsVec3& position) = 0;

    // 00448130, node->vtable[34h](matrix4x4). The reconstruction hands over the
    // corrected 3x4; the native call passes the 4x4 that 00c33650 built from it.
    virtual void set_node_world_transform_vtable34(
        std::uint32_t node, const DynamicsTransform34& transform) = 0;

    // 0044826b, 00b6dfa0.
    virtual void unlink_and_release_node_00b6dfa0(std::uint32_t node) = 0;

    // 00448279, 00c34f70(owner, body) with owner = [00E188A8]+18h, the same
    // deferred-release queue 00447060 pushes to.
    virtual void queue_body_release_00c34f70(std::uint32_t body) = 0;

    // 0044823d, 00b6da70(node, factor, 0).
    virtual void set_node_visibility_00b6da70(std::uint32_t node, float factor) = 0;
};

struct GameDynamicsFrameResult {
    std::size_t pending_fired{};     // entries whose countdown ran out
    std::size_t records_released{};  // records removed this frame
    std::size_t effects_spawned{};   // water-entry effects
    std::size_t records_faded{};     // records inside the fade window
};

// 00447b80, __thiscall void(list, float), body 00447b80..0044831c, RET 4.
// Both loops swap-erase and do not advance the index on a removal, and the
// first loop re-reads its bound through 00445db0 on every pass; both are
// reproduced. `alpha` is dynamics_interpolation_alpha of the leftover the
// fixed-step driver left in 00f876ac.
GameDynamicsFrameResult tick_game_dynamics_frame_00447b80(
    GameDynamicsState& list, float scaled_delta, float alpha, GameDynamicsFrameHost& host);

// ---------------------------------------------------------------------------
// The fixed-step pass, 004462d0
// ---------------------------------------------------------------------------

struct GameDynamicsBuoyancyHost {
    virtual ~GameDynamicsBuoyancyHost() = default;
    // 00c32000(body): the world transform. The rule needs its axis lengths at
    // +04h/+10h/+1Ch and the center y at +28h, plus x and z at +24h/+2Ch for
    // the water query.
    virtual void body_world_transform_00c32000(
        std::uint32_t body, DynamicsVec3& axis, DynamicsVec3& center) = 0;
    // 00c31f90(&min, &max): the body's box, as extents per axis.
    virtual DynamicsVec3 body_box_extent_00c31f90(std::uint32_t body) = 0;
    // 0078cf20(x, z): the water height there.
    virtual float water_height_0078cf20(float x, float z) = 0;
    // 00c31fc0(body): the scalar the buoyancy force is built from.
    virtual float body_buoyancy_scalar_00c31fc0(std::uint32_t body) = 0;
    // 00c32050(&force): apply it.
    virtual void apply_buoyancy_force_00c32050(std::uint32_t body, float force) = 0;
    // 00c37de0 and 00c37e00, both with 00e0819c * submersion.
    virtual void set_body_damping_00c37de0(std::uint32_t body, float damping) = 0;
};

// 004462d0, __thiscall void(list, float step), body 004462d0..00446525. Called
// once per fixed step from 00875e24, never removes anything. `damping_scale` is
// the global at 00e0819c, currently 1.0f. Returns the number of records
// processed.
std::size_t apply_dynamics_buoyancy_004462d0(
    GameDynamicsState& list, float damping_scale, GameDynamicsBuoyancyHost& host);

} // namespace bsp
