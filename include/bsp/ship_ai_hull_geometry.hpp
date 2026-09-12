#pragma once

#include "bsp/camera_affine.hpp"
#include "bsp/hit_narrowphase.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/ship_ai_sector_scan.hpp"
#include "bsp/system_camera_axes.hpp"

#include <array>
#include <cstdint>

namespace bsp {
// Complete geometry produced by009DE2F0. A semantic output, not the native
// block layout: untouched +1B4 and input +1B8 are deliberately not duplicated.
// Original ECX block, no stack arguments, RET. Names are hypotheses.
struct ShipAiHullGeometry {
    std::array<float, 2> bow_174;
    std::array<float, 2> stern_17c;
    std::array<float, 2> position_184;
    std::array<float, 2> shoulder_18c;
    std::array<float, 2> shoulder_194;
    std::array<float, 2> beam_19c;
    std::array<float, 2> opposite_beam_1a4;
    std::array<float, 2> forward_1ac;
    float max_y_1bc;
    float min_y_1c0;
};

class ShipAiHullUnitAccess {
public:
    virtual ~ShipAiHullUnitAccess() = default;
    // Three reads, at009DE321,009DE37E,009DE3C5. Resolve the current unit and
    // honor its actual C8 gate before returning its CC world matrix. A host
    // with only cache-valid matrix access may supply that actual matrix and
    // reject a dirty unsupported pose. Do not fabricate parent/local fields.
    virtual const CameraMatrix& unit_world_pose_3fc() = 0;
    // Read-only view of the ACTUAL optional damage-model/collision node's
    // world bounds at+13C/+148. The native MDestroyer target006D1E30 returns
    // unit+360, populated from descriptor+50 by0087BCC0. Unknown model data
    // is not evidence of absence. Called twice when the first result is nonnull.
    // This getter does not refresh the node's frame-stamped bounds.
    virtual const HitQueryBounds* unit_model_vtable20() = 0;
};

// Canonical native refresh path for hosts with real borrowed pose fields.
// Existing cache-only runtime bindings can implement the read contract above.
const CameraMatrix& ship_ai_hull_world_pose(PoseRefreshView&);

// Complete0098A8E0 copy schedule over the existing semantic bounds type.
// Native ECX collision node; stack minXYZ,maxXYZ;RET8. Six x87 load/stores;
// source/output overlap outside the actual caller's disjoint stack buffers
// is not supported by this C++ projection.
void ship_ai_hull_copy_bounds_0098a8e0(
    const HitQueryBounds&, HitQueryPoint& minimum, HitQueryPoint& maximum) noexcept;

// Complete009DE2F0 through required actual pose/model/CRT inputs. distance3E4
// and shoulder1B8 are read where native reads them; constructor009E4330 is
// their producer. No heading shortcut, finite-value repair, or fallback axis.
// The null model branch writes the native50 and-10 vertical bounds.
void ship_ai_hull_geometry_009de2f0(ShipAiHullGeometry&,
    const float& distance_3e4, const float& shoulder_offset_1b8,
    ShipAiHullUnitAccess&, const CameraAxesCrtAccess&);

class ShipAiHullPreStepAccess : public ShipAiHullUnitAccess {
public:
    virtual std::uint32_t class_reference_0570() = 0;
    virtual float unit_reference_speed_0080fc30() = 0;
    virtual float unit_turn_circle_00811a30(float fraction) = 0;
    virtual float unit_full_beam_09cc() = 0;
};

// Bind actual existing storage. The native parent owns these fields; this
// view creates no alternative control block or default initialization.
struct ShipAiHullPreStepView {
    std::uint32_t& class_reference_168;
    float& reference_speed_3c4;
    ShipAiHullGeometry& geometry;
    const float& distance_3e4;
    const float& shoulder_offset_1b8;
    std::array<ShipAiObstacleSector, 12>& sectors_808;
    std::uint8_t& flag_3e8;
    std::uint8_t& flag_3e9;
    std::uint8_t& flag_3ea;
    std::array<std::uint8_t, 65>& profile_004;
    std::uint8_t& profile_valid_045;
};

// Complete009E0270 pre-step, original ECX block and one UNUSED stack word,
// RET4. Constructor passes1; controller passes a word with its low byte set
// to a boolean and upper bytes retained from a float temporary. It is not dt.
// Stores class reference/reference speed, calls geometry, writes sector shape
// fields using stored3E4, clears3EA/3E9/3E8, and if45==0 sets45=1 then clears
// the65-byte profile. Other sector fields and nonzero45 byte are preserved.
void ship_ai_hull_pre_step_009e0270(ShipAiHullPreStepView&,
    ShipAiHullPreStepAccess&, std::uint32_t ignored_native_argument,
    const CameraAxesCrtAccess&);
} // namespace bsp
