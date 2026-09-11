#pragma once

#include "bsp/point_effect_children.hpp"
#include "bsp/effect_admission.hpp"

namespace bsp {

class PointEffectAdvanceNodes {
public:
    virtual ~PointEffectAdvanceNodes() = default;
    // Pure mapping to this EXACT existing native node's storage. Never create
    // another owner/count or return diagnostic CameraTransform storage.
    virtual NativeNodeStorage& parent_storage(CameraTransform&) noexcept = 0;
};

struct PointEffectAdvanceRuntime {
    PointEffectRowRuntime& rows;
    PointEffectChildEvents& events;
    EffectManager* volatile& actual_manager;
    EffectManagerLifetimeAccess& manager_lifetime;
    PointEffectInstanceLinks& links;
    SceneAttachmentRuntime& scenes;
    PointEffectAdvanceNodes& nodes;
};

// Complete00867D00..00867EDA: ECX actual114h owner, stack(float delta,
// unused reference DWORD), RET8. Uses the SAME actual nodes, rows, events and
// manager domain as construction/child updates. Captured owners must remain
// alive across callbacks; native adds no temporary retain. Exceptions retain
// prior writes and propagate; the consumed-parent terminal domain is noexcept.
// Recovered source interface, not a native object/vtable binary replacement.
void advance_point_effect_00867d00(PointEffectInstanceStorage&, float delta,
    void* unused_reference, PointEffectAdvanceRuntime&);

namespace detail {
// Borrowed sampling fields shared with the older sample-only semantic helper.
// Full advancement creates this over actual114h storage, BEFORE node refresh;
// references remain live so callback writes to timer/count/flags are observed.
struct PointEffectSampleFields {
    float& timer;
    const float& interval;
    float* previous_xyz;
    float* current_xyz;
    float* velocity_xyz;
    float* displacement_xyz;
    const std::uint32_t& sample_count;
    const std::uint32_t& track_velocity;
    const std::uint32_t& track_displacement;
};
void advance_point_effect_age(float& actual_age, const float& delta) noexcept;
// 867D8F..867DC6: full DWORD flags; unspilled x87 elapsed comparison. True
// means previous XYZ were copied through x87 and sampling-node refresh follows.
bool begin_point_effect_sample(PointEffectSampleFields, const float& delta) noexcept;
// 867DDC..867EC4: capture all3 source DWORDs before any currentXYZ store,
// signed count>1/COMISS-positive delta, explicit numerator/span/quotient spills.
// Re-reads current timer AFTER refresh. Every successful sample resets timer.
void finish_point_effect_sample(PointEffectSampleFields, const float& delta,
    const float* actual_sampled_xyz) noexcept;
}
} // namespace bsp
