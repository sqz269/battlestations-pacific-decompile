#pragma once

#include "bsp/point_effect_row_ownership.hpp"

namespace bsp {

struct PointEffectRestartRowView {
    const std::uint8_t& gated_10;
    const std::uint32_t& sample_14;
};
class PointEffectChildRows {
public:
    virtual ~PointEffectChildRows() = default;
    // Pure projection of the exact current row's fields, no snapshot/callback.
    virtual PointEffectRestartRowView restart_fields(void* actual_row) noexcept = 0;
    // REQUIRED current component virtual+08, AL result; no default predicate.
    virtual std::uint8_t restart_virtual_08(void* actual_row) = 0;
};
struct PointEffectChildView {
    std::uint8_t& active_0c;
    const std::uint32_t& type_18;
};
class PointEffectChildEvents {
public:
    virtual ~PointEffectChildEvents() = default;
    // Pure projection of the exact existing event and its actual fields.
    virtual PointEffectChildView child_fields(RenderCommandReference&) noexcept = 0;
    // REQUIRED current native virtuals; references borrow each actual+04.
    virtual void update_virtual_28(RenderCommandReference&, float delta,
        void* actual_reference_node) = 0;
    virtual std::uint8_t complete_virtual_08(RenderCommandReference&) = 0;
    // Capture CURRENT virtual+30, clear actual active+0C, THEN invoke the
    // captured implementation. Caller already tested the live active byte.
    virtual void deactivate_virtual_30(RenderCommandReference&) = 0;
};

// Complete00867790..008679C0. ECX actual114h point, stack(float delta,
// reference-node pointer), RET8. Captures the primary slot span and definition
// row backing once. Callbacks may replace slot values/header fields; captured
// spans must remain allocated. Auxiliary end is reloaded after each callback.
// No entry retain/lock; actual callers must keep owner/definition/spans alive.
// Known native template/row/event preconditions are required (including a
// nonnull current event after its update callback). No invalid-state recovery.
void update_point_effect_children_00867790(PointEffectInstanceStorage&, float delta,
    void* actual_reference_node, PointEffectRowRuntime&, PointEffectChildRows&,
    PointEffectChildEvents&);

} // namespace bsp
