#pragma once
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_type_dispatch.hpp"

namespace bsp {
// The ONE type16 companion's native+EC..108 fields. Ordinary00ACE0A0 and
// copy00ACE0F0 both leave ALL these bytes unwritten; do not value-initialize
// the member in its owner. The reader writes border_width before update24,
// and update24 establishes all borrowed material sources before consumption.
struct GuiClipBoxFields {
    float center_ec[2];
    float border_f4[4]; // half width/height, then authored border width pair
    float border_width_104[2];
};

// Complete00ACE120 operation schedule over the SAME base transform and derived
// fields. Calls existing00AA6750 resolved-position kernel, then reproduces the
// native x87 stack, every float spill and the two interleaved raw border moves.
// Does not use scale/rotation/aspect globals. BorderWidth must already exist.
// Native ECX widget; RET at00ACE226/1, end00ACE226. New C++ semantic ABI.
void update_gui_clip_box_00ace120(GuiClipBoxFields&, const GuiWidgetTransform&) noexcept;

// Derived continuation AFTER existing00AAA710 has bound base properties and
// loaded children. Missing/nil BorderWidth -> both lanes00D7A2F0 (0.1f),
// present Vec2 uses existing00BD63B0 conversions. Then current+24 immediately.
// Wrong aggregate shapes fail explicitly, preserving the native destination
// preimage; they are not converted into invented clipping values.
// Native ECX widget; stack visitor; RET4 at00ACE6C4/3, end00ACE6C6.
void read_gui_clip_box_properties_00ace650(GuiClipBoxFields&,
    const GuiWidgetTransform&, const GuiTable& evaluated_table,
    const bool& crt_sse2_conversion);

//00ACE0F0 invokes ONLY actual base copy00AA9520 before stamping its type16
// table. Base copy owns real node cloning/empty-child-list behavior and must
// preserve every derived+EC..108 byte, plus base+E8. It is unresolved in the
// current unique-owner GUI runtime, so the real base-copy operation is required.
// No C++ fieldwise copy or fallback ownership is supplied here.
using GuiClipBoxBaseCopy = std::function<void(GuiWidgetOwner& destination,
    const GuiWidgetOwner& source)>;
void copy_gui_clip_box_00ace0f0(GuiWidgetOwner& destination,
    const GuiWidgetOwner& source, const GuiClipBoxBaseCopy& actual_base_copy_00aa9520);

//00ACE0A0: base00AA9390(type16), then table00D5D058, RET00ACE0B3/1.
// GuiWidgetOwnerRuntime has constructed that SAME base before this companion.
// Base+74/+78/visibility slots match GuiGroupTypeImplementation; only reader
// and current+24 differ in the retained subset. No extra tree/count/clip cache.
class GuiClipBoxTypeImplementation final : public GuiGroupTypeImplementation {
public:
    GuiClipBoxTypeImplementation(GuiWidgetOwner&, const bool& crt_sse2_conversion);
    GuiClipBoxTypeImplementation(const GuiClipBoxTypeImplementation&) = delete;
    GuiClipBoxTypeImplementation& operator=(const GuiClipBoxTypeImplementation&) = delete;
    void properties_bound(GuiWidgetOwner&, const GuiTable&) override;
    void update24_00ace120(GuiWidgetOwner&);
    GuiClipBoxFields& fields() noexcept { return fields_; }
private:
    GuiWidgetOwner& owner_;
    const bool& crt_sse2_conversion_; // same live mode supplied by type dispatch
    GuiClipBoxFields fields_; // intentionally not value-initialized
};

// Concrete GuiMaterialBindingServices.clip_box_sources target. Resolves the
// SAME type16 implementation in owner.implementation(); no registry or copied
// sources. Successful property binding/update must precede material packing.
GuiClipBoxParameterSources gui_clip_box_parameter_sources(GuiWidgetOwner&);
} // namespace bsp
