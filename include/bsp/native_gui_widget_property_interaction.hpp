#pragma once
#include "bsp/native_gui_widget_property_prefix.hpp"
#include "bsp/native_gui_widget_transform.hpp"
#include "bsp/native_gui_widget_visibility.hpp"

namespace bsp {
// Required implementations for current targets other than the two recovered
// base entries A9E110 and AA8530. Receivers are actual widget storage; never
// substitute a base result or logical owner for an unknown derived target.
class NativeGuiWidgetPropertyWidgetDispatch {
public:
    virtual ~NativeGuiWidgetPropertyWidgetDispatch() = default;
    virtual std::uint32_t call_type_5c(std::uint32_t target, void* widget) = 0;
    virtual void call_visible_34(std::uint32_t target, void* widget,
        std::uint8_t requested) = 0;
};
struct NativeGuiWidgetPropertyInteractionContext {
    NativeGuiWidgetPropertyBindings& properties;
    const char* visible_00d5c1e4;
    const char* mouse_block_00d5c1d8;
    const char* mouse_hit_00d5c1cc;
    NativeGuiWidgetPropertyWidgetDispatch& widgets;
    NativeGuiWidgetVisibilityContext& visibility;
    NativeGuiWidgetTransformScratch& transform_scratch;
    NativeGuiWidgetTransformBindings& transform;
};
// Exactly [AAABD6,AAACD4), 254B of AAA710. Same actual widget, visitor and
// scratch as the preceding prefix. Current type1 skips Visible and widget+34;
// MouseBlock true stores MouseHit1 without its reader call; transform always
// follows. Each virtual call reads its current raw profile and exact target.
// Boolean fallback payloads preserve only the meaningful low byte. The upper
// three bytes, native return-address/stack reuse and outer machine registers
// are outside this new ABI, as in the prefix scratch contract. The 7F8h locals
// remain in the SAME scratch, including metadata+28 and stale string header.
void continue_native_gui_widget_property_interaction_00aaabd6(void* widget,
    void* visitor, NativeGuiWidgetPropertyScratch&,
    NativeGuiWidgetPropertyInteractionContext&);
// Compose the existing prefix and this stage through the transform. Still a
// partial [AAA710,AAACD4) projection: no key enumeration, children, epilogue,
// FH3/SEH, binary ABI or application binding. Genuine callbacks must return.
void read_native_gui_widget_properties_through_transform_00aaa710(void* widget,
    void* visitor, NativeGuiWidgetPropertyScratch&,
    NativeGuiWidgetPropertyInteractionContext&);
} // namespace bsp
