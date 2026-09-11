#pragma once

#include "bsp/effect_admission.hpp"
#include "bsp/point_effect_instance.hpp"

namespace bsp {

// Complete 006FBEB0. Native ECX is a destination pointer slot, stack argument
// points to a source slot, EAX returns the destination slot, RET4. Capture the
// source value first; identity does nothing. Otherwise publish, retain incoming,
// then release captured old. Both slots may be the same actual slot.
RenderCommandReference** assign_point_effect_entry_reference_006fbeb0(
    RenderCommandReference** destination, RenderCommandReference* const* source) noexcept;

// Complete 006CF070. Native ECX points to the owned slot; no stack arguments,
// RET. Release captured old, then clear this slot AFTER any terminal reentry.
// Empty slots are not written. The return registers have no established result.
void clear_point_effect_entry_reference_006cf070(RenderCommandReference** slot) noexcept;

struct PointEffectFactoryRowView {
    const std::uint8_t& gated_10;
    const std::uint8_t& admitted_1c;
};

class PointEffectRowRuntime {
public:
    virtual ~PointEffectRowRuntime() = default;
    // Pure nonthrowing projections of the EXACT current owners/fields. Never
    // snapshot a container, initialize a count, or manufacture mapped owners.
    virtual EffectAdmissionTemplateView template_rows(RenderCommandReference&) noexcept = 0;
    virtual PointEffectFactoryRowView row_fields(void* actual_row) noexcept = 0;
    // Pure current load of [E188A8]+19FC, projected to its canonical transform.
    virtual CameraTransform& reference_e188a8_19fc() noexcept = 0;
    // REQUIRED real CURRENT row virtual+18, native ECX=row, stack=instance.
    // EAX is nullable and transfers exactly one owned reference on success.
    // Map that exact result to its stable canonical companion borrowing the
    // owner's actual+04 and nonthrowing current virtual+00 terminal action.
    // A throwing factory has transferred no result. No default factory exists.
    virtual RenderCommandReference* create_virtual_18(
        void* actual_row, PointEffectInstanceStorage&) = 0;
};

// ONLY 008682D5..0086838C of constructor008680B0, after its world-cache stage:
// resize from current template count; refresh current node then current global
// reference; option gate; capture current template extent once; current row flags
// and required factory; reload output backing, assign, release returned temp.
// The owner and captured row span/slots must survive native reentry. Subsequent
// slots can change; replacing the captured span does not change iteration end.
// Native reference words use the existing borrowed RenderCommandReference
// companions. These APIs are new typed interfaces, never native vtable overlays.
// No manager insertion, argument/member unwind, or full constructor is implied.
void initialize_point_effect_rows_008682d5(
    PointEffectInstanceStorage&, PointEffectRowRuntime&);

} // namespace bsp
