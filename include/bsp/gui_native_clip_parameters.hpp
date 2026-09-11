#pragma once
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_material_parameters.hpp"

namespace bsp {

// Complete supported00AA9F10 body against the SAME native material storage,
// parameter pool, widget+E8 and canonical ClipBox fields. Native ECX widget,
// stack material pointer, RET4; this is a new C++ interface, not a native ABI.
// Supported retained GUI types expose their fixed current+5C identity through
// layout.transform.type_id. The nearest self/parent type16 supplies its actual
// initialized ClipBox center/border fields; no rectangle or material snapshot.
//
// Writes+E8, constructs/registers/releases cClip, then reloads+E8. Active only:
// cClipCenter(float2), cClipBorder(float4), cAspectRatio(float), in that order.
// Inactive leaves earlier center/border/aspect parameters untouched. Names use
// actual NativeString allocation/release; source addresses stay borrowed.
// Storage, owner/ancestor instances and actual service domains must survive
// callbacks and every later parameter consumer. No widget retain is added;
// native00B18A40 is a separate caller operation. Corrupt/missing ownership and
// native pool/SEH failure behavior are not emulated. See the matching doc/report.
void register_native_gui_clip_parameters_00aa9f10(GuiWidgetOwner&,
    NativeMaterialStorage&, GuiWidgetOwnerRuntime&,
    const float& aspect_ratio_00e12fc0, NativeMaterialParameterAccess&);

} // namespace bsp
