#pragma once

#include "bsp/ship_ai_approach_update.hpp"
#include "bsp/system_camera_axes.hpp"

namespace bsp {

// Names are hypotheses. Full address/ABI evidence: docs/SHIP_AI_NAV_CIRCLE_TANGENT.md.
// 00E0830C: 17 B7 D1 38. Relative comparisons are STRICT, with a scale floor of 1.
inline constexpr float kShipAiCircleRelativeTolerance = 0.0001f;

// 004F3560-004F3626; native two float stack arguments, RET 8, AL flag.
// Keeps the x87/SSE comparison schedule, including its unordered branches.
bool __stdcall circle_relative_equal_004f3560(float first, float second) noexcept;

enum class ShipAiCircleIntersection : int {
    Coincident = -2,
    Contained = -1,
    Disjoint = 0,
    ExternalTangent = 1,
    Pair = 2,
};

// 004F4520-004F47AD, native ECX/EDX centers, radii then two output pointers
// on the stack, RET 10h. This interface adds explicit borrowed CRT access.
// All branches reconstructed. Negative/zero results leave BOTH outputs alone;
// Pair also includes internal tangency (two identical points). No radius repair.
// Output order is base - perp(second-first)*height, then base + that offset.
ShipAiCircleIntersection circle_intersections_004f4520(
    const ShipAiAttackMoveXZ& first_center, const ShipAiAttackMoveXZ& second_center,
    float first_radius, float second_radius,
    ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second,
    const CameraAxesCrtAccess& crt);

// 004F47B0-004F47D1, native ECX/EDX three-float circles, two outputs, RET 8.
// Sole native caller supplies {query.x, query.z, clearance} as the first circle.
ShipAiCircleIntersection circle_intersections_004f47b0(
    const ShipAiCircleTangentCircle& first_circle,
    const ShipAiCircleTangentCircle& second_circle,
    ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second,
    const CameraAxesCrtAccess& crt);

// Actual stream-1 RNG draw supplied by the caller; no new PRNG or global state.
using ShipAiCircleRandomDraw = float (*)(void* context, float low, float high);

// Concrete geometry binding for the existing 009D68B0 control-flow projection.
// All borrowed references and random_draw must remain valid. Its circle must
// be the same circle passed to ship_ai_circle_tangent_009d68b0.
class ShipAiCircleGeometryHost final : public ShipAiCircleTangentHost {
public:
    ShipAiCircleGeometryHost(const ShipAiCircleTangentCircle& circle,
        const CameraAxesCrtAccess& crt, ShipAiCircleRandomDraw random_draw,
        void* random_context) noexcept;

    bool tangent_points_009d6550(const ShipAiAttackMoveXZ& point,
        ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second) override;
    void offset_points_004f47b0(const ShipAiAttackMoveXZ& point, float clearance,
        ShipAiAttackMoveXZ& first, ShipAiAttackMoveXZ& second) override;
    float random_stream1_00bd2f10(float low, float high) override;
    float sqrt_00bf7030(float value) override;

private:
    const ShipAiCircleTangentCircle& circle_;
    const CameraAxesCrtAccess& crt_;
    ShipAiCircleRandomDraw random_draw_;
    void* random_context_;
};

} // namespace bsp
