#include "bsp/game_dynamics_list.hpp"

#include "bsp/cc13_x87_model.hpp"
#include "bsp/sw1b_switch_reach.hpp"

namespace bsp {
namespace {

// 00448186: the fade window test is COMISS 5.0f, rec+4h with JBE taking the
// skip arm, so the window is strictly "lifetime below 5.0".
bool inside_fade_window(float lifetime_seconds) noexcept
{
    return lifetime_seconds < kDynamicsFadeWindowSeconds;
}

// 004462d0 step 3: the listing writes the absolute value as -0.0f - v, applied
// only when v <= 0. Kept in that shape so a negative zero behaves the way the
// listing makes it behave.
float axis_magnitude(float value) noexcept
{
    return value <= 0.0f ? (-0.0f - value) : value;
}

} // namespace

std::size_t dynamics_pending_count_00445db0(const GameDynamicsState& list) noexcept
{
    return list.pending.size();
}

float dynamics_interpolation_alpha(float accumulator, float fixed_step) noexcept
{
    if (fixed_step == 0.0f) {
        return 0.0f;
    }
    return accumulator / fixed_step; // 00447d93 FDIV, no clamp
}

DynamicsTransform34 interpolate_body_transform_00c43ea0(
    const DynamicsTransform34& previous, const DynamicsTransform34& current, float alpha) noexcept
{
    DynamicsTransform34 out{};
    for (int i = 0; i < 12; ++i) {
        // 00c43eb7..00c43ff5: a + (b - a) * t, twelve times, no clamp on t. The FSUBP, FMULP
        // and FADDP stay on the x87 stack until the FSTP. The callers (00447B80
        // UpdateFrame, 00929CB0, 0092F930) are main-thread or fixed-step code, so each op
        // rounds to a 24-bit significand with the stack's exponent range (cc13-fixture3:
        // plain float overflowed on (b - a) and rounded denormals once where the stack
        // rounds twice).
        using namespace cc13x87;
        out.m[i] = static_cast<float>(x87_add24(
            previous.m[i], x87_mul24(x87_sub24(current.m[i], previous.m[i]), alpha)));
    }
    return out;
}

DynamicsVec3 dynamics_node_translation(
    const DynamicsTransform34& t, const DynamicsVec3& anchor) noexcept
{
    // 00447e0b..00447eb1: the translation row (m[9], m[10], m[11]) minus the
    // anchor multiplied by the transpose of the 3x3, column by column.
    const float dx = anchor.x * t.m[0] + anchor.y * t.m[3] + anchor.z * t.m[6];
    const float dy = anchor.x * t.m[1] + anchor.y * t.m[4] + anchor.z * t.m[7];
    const float dz = anchor.x * t.m[2] + anchor.y * t.m[5] + anchor.z * t.m[8];
    return DynamicsVec3{t.m[9] - dx, t.m[10] - dy, t.m[11] - dz};
}

float dynamics_anchor_world_y(const float node_matrix_column_y[4], const DynamicsVec3& anchor) noexcept
{
    // 00447f99..00447fd2: node+F4h, +104h, +114h then +124h added last, which is
    // the order the x87 stack builds it in.
    const float rotated = node_matrix_column_y[0] * anchor.x +
                          node_matrix_column_y[1] * anchor.y +
                          node_matrix_column_y[2] * anchor.z;
    return node_matrix_column_y[3] + rotated;
}

bool dynamics_water_entry(float anchor_world_y, float body_world_y) noexcept
{
    // 00447fd6 (FCOMIP against zero, JBE skips) and 00447fe9 (COMISS 0, XMM0).
    return anchor_world_y > 0.0f && body_world_y < 0.0f;
}

int dynamics_splash_effect_id(const DynamicsSplashSettings& settings, float body_velocity_y) noexcept
{
    // 00448011..00448036. The thresholds are compared negated, so a faster
    // downward velocity is a more negative y.
    float chosen = 0.0f;
    if (-settings.large_threshold_24 > body_velocity_y) {
        chosen = settings.large_effect_2c; // 0044801a
    } else if (-settings.small_threshold_20 > body_velocity_y) {
        chosen = settings.small_effect_28; // 00448032
    } else {
        return -1; // 0044802c, the branch that spawns nothing
    }
    const int id = static_cast<int>(chosen); // CVTTSS2SI
    return id > -1 ? id : -1;                // 00448037 CMP -1, JLE skips
}

bool dynamics_should_release(float lifetime_seconds, float body_world_y) noexcept
{
    // 00448150: COMISS 0.0f, rec+4h with JNC taking the release arm, so a
    // lifetime of exactly zero releases.
    if (lifetime_seconds <= 0.0f) {
        return true;
    }
    // 0044815f: the double -50.0 at 00ce4938 compared above the body's y.
    return static_cast<double>(body_world_y) < kDynamicsKillDepth;
}

float dynamics_expiring_buoyancy_divisor(float lifetime_seconds, float divisor) noexcept
{
    if (!inside_fade_window(lifetime_seconds)) {
        return divisor;
    }
    // 004481bc: COMISS 1.0f, rec+1Ch with JBE skipping, so a divisor already at
    // or above 1.0f is left alone.
    if (divisor < kDynamicsBuoyancyDivisorFloor) {
        return kDynamicsExpiringBuoyancyDivisor;
    }
    return divisor;
}

float dynamics_fade_factor(float lifetime_seconds) noexcept
{
    if (!inside_fade_window(lifetime_seconds)) {
        return 1.0f;
    }
    // 00448225: the divide is against the double 5.0 at 00d7a370 and the result
    // is stored back as a float, which is what the setter receives.
    return static_cast<float>(static_cast<double>(lifetime_seconds) / kDynamicsFadeDivisor);
}

float dynamics_submersion_fraction(
    const DynamicsVec3& axis, const DynamicsVec3& extent,
    float center_y, float water_height) noexcept
{
    const float half = static_cast<float>(
        (axis_magnitude(axis.x) * extent.x +
         axis_magnitude(axis.y) * extent.y +
         axis_magnitude(axis.z) * extent.z) * kDynamicsBoxHalfExtent);
    const float bottom = center_y - half;
    const float top = center_y + half;
    if (top == bottom) {
        return 0.0f; // the native divide would be by zero; no record reaches it
    }
    float fraction = (water_height - bottom) / (top - bottom);
    if (fraction < 0.0f) {
        return 0.0f;
    }
    if (fraction > kDynamicsBuoyancyDivisorFloor) { // the 1.0f at 00d7a24c
        return kDynamicsBuoyancyDivisorFloor;
    }
    return fraction;
}

std::size_t dynamics_last_model_group(std::size_t group_count) noexcept
{
    return group_count != 0 ? group_count - 1 : 0; // 00447c37, 00447c4e
}

GameDynamicsFrameResult tick_game_dynamics_frame_00447b80(
    GameDynamicsState& list, float scaled_delta, float alpha, GameDynamicsFrameHost& host)
{
    GameDynamicsFrameResult result{};

    // --- loop 1, 00447ba0..00447cf3: the pending model-group switches -------
    // The bound is re-read through 00445db0 on every pass (00447ba3, 00447ce9),
    // and a removal does not advance the index.
    std::size_t i = 0;
    while (i < dynamics_pending_count_00445db0(list)) {
        GameDynamicsPending& entry = list.pending[i];
        entry.seconds -= scaled_delta; // 00447bcb
        if (entry.seconds >= 0.0f) {   // 00447bfa, JBE keeps
            ++i;
            continue;
        }
        const std::size_t groups = host.model_group_count(entry.owner); // 00447c1a
        host.select_visible_model_group_00710bb0(
            entry.owner, dynamics_last_model_group(groups)); // 00447c6d
        ++result.pending_fired;
        // 00447c72..00447ce2: swap-erase, index unchanged.
        list.pending[i] = list.pending.back();
        list.pending.pop_back();
    }

    // --- loop 2, 00447cf6..004482fd: one record per pass --------------------
    std::size_t r = 0;
    while (r < list.records.size()) { // 00447d1b, the bound is re-read too
        GameDynamicsRecord& record = list.records[r];

        const std::uint32_t body = record.body; // 00447d57
        record.lifetime_seconds -= scaled_delta; // 00447d79

        DynamicsTransform34 interpolated =
            host.interpolated_body_transform_00c43ea0(body, alpha); // 00447da7
        // 00c33650 expands the same twelve floats into a 4x4; the extra column
        // carries no information, so the reconstruction keeps the 3x4.

        const DynamicsVec3 body_world{interpolated.m[9], interpolated.m[10], interpolated.m[11]};
        const DynamicsVec3 placed = dynamics_node_translation(interpolated, record.anchor);
        interpolated.m[9] = placed.x;
        interpolated.m[10] = placed.y;
        interpolated.m[11] = placed.z;

        // 00447ede and 00447f0b: the same test and call, twice.
        host.refresh_node_world_matrix_00b6db70(record.node);
        host.refresh_node_world_matrix_00b6db70(record.node);

        float column_y[4]{};
        host.node_world_matrix_column_y(record.node, column_y); // 00447f99
        const float anchor_y = dynamics_anchor_world_y(column_y, record.anchor);

        if (dynamics_water_entry(anchor_y, body_world.y)) {
            const DynamicsVec3 velocity = host.body_linear_velocity_00c31f40(body); // 00447ffe
            const DynamicsSplashSettings settings = host.splash_settings_00424c40(); // 00448003
            const int effect = dynamics_splash_effect_id(settings, velocity.y);
            if (effect >= 0) {
                host.spawn_water_entry_effect(effect, body_world); // 00448046, 0044809a
                ++result.effects_spawned;
            }
        }

        host.set_node_world_transform_vtable34(record.node, interpolated); // 00448130

        if (dynamics_should_release(record.lifetime_seconds, body_world.y)) { // 0044814a
            host.unlink_and_release_node_00b6dfa0(record.node); // 0044826b
            host.queue_body_release_00c34f70(body);             // 00448279
            ++result.records_released;
            // 004482cc..004482fd: swap-erase of the 20h record, index unchanged.
            list.records[r] = list.records.back();
            list.records.pop_back();
            continue;
        }

        if (inside_fade_window(record.lifetime_seconds)) { // 00448191
            record.buoyancy_divisor = dynamics_expiring_buoyancy_divisor(
                record.lifetime_seconds, record.buoyancy_divisor); // 004481e5
            host.set_node_visibility_00b6da70(
                record.node, dynamics_fade_factor(record.lifetime_seconds)); // 0044823d
            ++result.records_faded;
        }
        ++r; // 00448242
    }

    return result;
}

namespace {

// cc13-P4 (004462d0): ON runs the whole body as the image does it - the water
// query before the box query, the image's float operation order with no
// zero-divide guards, and the second damping setter 00c37e00. OFF keeps the
// earlier projection, which live play has run on (with an empty list).
constexpr bool kDynamicsBuoyancyWholeBodyCc13Bound = true;  // ON by pruning (cc9-sweep1b): reach counter 0 differs on every OFF run (sites 0)

// 0044636a..004463e3: `x > 0 ? x : -0.0f - x` against the -0.0f at 00d7a208.
float image_magnitude(float x) noexcept
{
    return x > 0.0f ? x : -0.0f - x;
}

} // namespace

float dynamics_submersion_fraction_image_004462d0(
    const DynamicsVec3& axis, const DynamicsVec3& extent,
    float center_y, float water_height) noexcept
{
    const float y_term = image_magnitude(axis.y) * extent.y; // 004463e3
    const float x_term = image_magnitude(axis.x) * extent.x; // 004463eb
    const float z_term = image_magnitude(axis.z) * extent.z; // 004463f5
    const float half = ((y_term + x_term) + z_term) * 0.5f;  // 004463ff, double 0.5
    const float bottom = center_y - half;                    // 0044641c
    const float top = half + center_y;                       // 00446432
    const float fraction = (water_height - bottom) / (top - bottom); // 00446440
    if (0.0f > fraction) return 0.0f;  // 0044644c, JBE keeps NaN
    if (fraction > 1.0f) return 1.0f;  // 00446468, the 1.0f at 00d7a24c
    return fraction;
}

std::size_t apply_dynamics_buoyancy_004462d0(
    GameDynamicsState& list, float damping_scale, GameDynamicsBuoyancyHost& host)
{
    std::size_t processed = 0;
    // 004462f0: a plain forward walk by index with the bound re-read; nothing is
    // removed here.
    for (std::size_t i = 0; i < list.records.size(); ++i) {
        const GameDynamicsRecord& record = list.records[i];

        if constexpr (kDynamicsBuoyancyWholeBodyCc13Bound) {
            // 0044631d 00c32000, 00446340 0078cf20(x, z) from +24h/+2Ch,
            // 00446355 00c31f90 - water before box.
            DynamicsVec3 axis{};
            DynamicsVec3 center{};
            host.body_world_transform_00c32000(record.body, axis, center);
            const float water = host.water_height_0078cf20(center.x, center.z);
            const DynamicsVec3 extent = host.body_box_extent_00c31f90(record.body);
            const float submersion =
                dynamics_submersion_fraction_image_004462d0(axis, extent, center.y, water);
            // 004464a1..004464c4: ((scalar * 10.0) * submersion) / record+1Ch,
            // each product rounded to float; a zero divisor gives inf or NaN.
            const float scalar = host.body_buoyancy_scalar_00c31fc0(record.body);
            const float force = (scalar * 10.0f) * submersion / record.buoyancy_divisor;
            host.apply_buoyancy_force_00c32050(record.body, force); // {0, force, 0}
            const float damping = damping_scale * submersion; // 004464d5, 004464f2
            host.set_body_damping_00c37de0(record.body, damping);
            host.set_body_damping_00c37e00(record.body, damping);
        } else {
            DynamicsVec3 axis{};
            DynamicsVec3 center{};
            host.body_world_transform_00c32000(record.body, axis, center);
            const DynamicsVec3 extent = host.body_box_extent_00c31f90(record.body);
            const float water = host.water_height_0078cf20(center.x, center.z);

            const float submersion = dynamics_submersion_fraction(axis, extent, center.y, water);
            // sw1b reach: the ON arm also calls 00C37E00, so every site is a difference in
            // calls; differs counts the sites whose submersion or force value differs too.
            const float submersion_on =
                dynamics_submersion_fraction_image_004462d0(axis, extent, center.y, water);

            const float scalar = host.body_buoyancy_scalar_00c31fc0(record.body);
            // The divide by record+1Ch is what the fade path manipulates. The
            // image's x87 divide is masked (inf or NaN, no fault); this arm
            // substitutes 0.
            const float force = record.buoyancy_divisor == 0.0f
                ? 0.0f
                : static_cast<float>(
                      static_cast<double>(scalar) * kDynamicsBuoyancyGravity *
                      static_cast<double>(submersion) / static_cast<double>(record.buoyancy_divisor));
            {
                const float force_on = (scalar * 10.0f) * submersion_on / record.buoyancy_divisor;
                sw1b_note(Sw1bReach::DynamicsBuoyancy,
                          !sw1b_same_bits(submersion_on, submersion)
                              || !sw1b_same_bits(force_on, force));
            }
            host.apply_buoyancy_force_00c32050(record.body, force);
            host.set_body_damping_00c37de0(record.body, damping_scale * submersion);
        }
        ++processed;
    }
    return processed;
}

} // namespace bsp
