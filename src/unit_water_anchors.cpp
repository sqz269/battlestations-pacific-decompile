#include "bsp/unit_water_anchors.hpp"

// The bow and stern water anchors and 004842C0. Evidence, the native listings
// these rules were taken from, and the uncertainties are in
// docs/UNIT_WATER_ANCHORS.md.

namespace bsp {

OceanVec3 unit_publish_anchor_point_004842c0(bool has_parent,
                                             const OceanVec3& world_point,
                                             UnitWaterAnchorHost& host) noexcept
{
    // 004842C6..004842D7: the node call comes first and is made with the
    // caller's buffer. 00B6DAE0 only reads it, so the value cached below is
    // the value the caller passed in.
    host.node_set_world_position(world_point);

    // 004842D9..004842EA: the parent test is the compiler's NEG/SBB/TEST
    // idiom, a plain null test.
    if (!has_parent) {
        // 00834320..0048433B, three float stores of the point as it came in.
        return world_point;
    }
    // 004842EC..00484313: through the parent's inverse world matrix, then
    // three float stores of the result.
    return host.parent_relative_point(world_point);
}

UnitAnchorWaterPoints unit_anchor_water_points_00825946(const OceanVec3& bow_from_matrix,
                                                        const OceanVec3& stern_from_matrix,
                                                        float bow_water_height,
                                                        float stern_water_height) noexcept
{
    UnitAnchorWaterPoints points{};
    points.bow = bow_from_matrix;
    points.stern = stern_from_matrix;

    // 0082594C: COMISS against 00CFBC84, JBE past the whole lift. The bow
    // height decides for both points, and an unordered compare skips the lift.
    if (!(bow_from_matrix.y > kUnitAnchorWaterGate)) {
        return points;
    }
    // 00825977 and 008259A4: the lift is added on the x87 stack as a double
    // and each result is stored as a float.
    points.bow.y =
        static_cast<float>(static_cast<double>(bow_water_height) + kUnitAnchorWaterLift);
    points.stern.y =
        static_cast<float>(static_cast<double>(stern_water_height) + kUnitAnchorWaterLift);
    return points;
}

bool unit_water_anchors_created_00823f09(bool template_present,
                                         const OceanVec3& bow_anchor_position) noexcept
{
    // 00823F12..00823F21: no template, no anchors.
    if (!template_present) {
        return false;
    }
    // 00823F27..00823F58: three UCOMISS/LAHF/TEST AH,0x44 pairs against zero.
    // The jump into the creation block is taken on "not equal", which an
    // unordered compare also satisfies, so a NaN component creates the anchors.
    const bool x_zero = (bow_anchor_position.x == 0.0f);
    const bool y_zero = (bow_anchor_position.y == 0.0f);
    const bool z_zero = (bow_anchor_position.z == 0.0f);
    return !(x_zero && y_zero && z_zero);
}

void unit_effect_set_scalar_00815370(const UnitEffectScalarArrays& arrays,
                                     float value,
                                     UnitEffectScalarHost& host) noexcept
{
    // 00815375..008153A8: the end pointer is begin + count * 4, and every
    // element is null-tested before its virtual is called.
    if (arrays.primary != nullptr) {
        for (std::int32_t i = 0; i < arrays.primary_count; ++i) {
            UnitEffectScalarTarget* const target = arrays.primary[i];
            if (target == nullptr) {
                continue; // 00815392
            }
            host.target_set_scalar(target, value); // 008153A1
        }
    }
    // 008153AA..008153D6: the same walk with no null test at all. A null
    // element here is a native crash; this reconstruction skips it and
    // docs/UNIT_WATER_ANCHORS.md records the difference.
    if (arrays.auxiliary != nullptr) {
        for (std::int32_t i = 0; i < arrays.auxiliary_count; ++i) {
            UnitEffectScalarTarget* const target = arrays.auxiliary[i];
            if (target == nullptr) {
                continue;
            }
            host.target_set_scalar(target, value); // 008153CF
        }
    }
}

} // namespace bsp
