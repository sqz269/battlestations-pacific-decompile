#pragma once
#include "bsp/gui_text_lifetime.hpp"
#include <array>

namespace bsp {
// Resolve the existing borrowed +1B0 OWNER slot through the supplied runtime
// and its one Text companion association. Null stays null. Nonnull must be a
// live canonical Text in that same runtime; this is never a native-pointer cast.
GuiTextLifetime* resolve_gui_text_glyph_reference(
    GuiTextLifetime&, GuiWidgetOwnerRuntime&);

// 00531130, ECX Text, stack (UTF16 wrapper, listener, reference Text), RET0Ch.
// New C++ canonical-string projection: source has no embedded terminator and
// may be the SAME string_1a4 object. Self assignment skips string work but still
// publishes listener then reference. Existing +1AC remains a borrowed opaque
// listener; +1B0 receives the canonical owner pointer (nullable). Neither is
// retained. String-pool allocation/callback/SEH and raw-header alias ABI are
// excluded; no geometry, visibility, offset or ownership operation is added.
void set_gui_text_glyph_reference_00531130(GuiTextLifetime&,
    const std::u16string& source, void* listener,
    GuiWidgetOwner* reference_text, GuiWidgetOwnerRuntime&);

// 00531380 direct-store fragments only. The first block at531903 computes
// x=+0/current doubleCEC380, stores byte1B4=1, then spills x and publishes1B8
// and current floatCED318 into1BC. Return that SAME local x for the other
// three blocks, which run after intervening current50 calls. These helpers
// do not clear the listener or perform those earlier/later calls.
float begin_gui_text_prompt_glyph_offsets_00531380_fragment(
    GuiTextLifetime&, const volatile double& width_00cec380,
    const volatile float& y_00ced318);
void copy_gui_text_prompt_glyph_offsets_00531380_fragment(
    GuiTextLifetime&, float captured_first_x,
    const volatile float& y_00ced318);

struct GuiTextGlyphPositionConstants {
    const volatile double& x_subtract_00d7a358;
    const volatile double& y_subtract_00d5c7a8;
    const volatile double& z_subtract_00d7a220;
    const volatile double& input_x_bias_00d06880;
    const volatile double& input_x_divisor_00cec380;
};

// ONLY AB98F0 tail9EC2..9FAD, after the actual child has completed construction,
// recursive content, attachment and current30 pivot. No preceding operation
// is bypassed or claimed complete. Re-resolve the current reference, get its
// resolved position, inspect live mode/length/offsets, read original arg2.x
// only in the long-reference branch, and preserve native x87 float spills.
// Then use existing AA8240 arithmetic and actual owner recompose/bounds, in
// that order. Current child reparenting is respected by the inverse transform.
// Offset mode requires glyph_offsets_written; failure retains owners/state.
// All borrowed inputs, owners and trees must remain live for this call.
void position_gui_text_glyph_child_00ab98f0_fragment(
    GuiTextLifetime& parent, GuiTextLifetime& child,
    const std::array<float, 3>& original_position_2,
    GuiWidgetOwnerRuntime&, const GuiTextGlyphPositionConstants&);
} // namespace bsp
