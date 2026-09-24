#pragma once
#include "bsp/native_string.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
// Exact current visitor+0C target, actual visitor and actual contiguous 8-byte
// key/field/fallback pairs. Required binding: no target substitution or no-op.
// For BD68D0 bind read_native_lua_field_or_default_00bd68d0 with the SAME actual
// reader/Lua state, raw pool, initialized lookup and StoreValue scratch.
class NativeGuiWidgetPropertyDispatch {
public:
    virtual ~NativeGuiWidgetPropertyDispatch() = default;
    virtual void call_virtual0c(std::uint32_t actual_target, void* actual_visitor,
        void* actual_key_pair, void* actual_field_pair,
        void* actual_fallback_pair) = 0;
};
struct NativeGuiWidgetPropertyLiterals {
    // Pos, Size, Pivot, Scale, Rotate, Color, LowColor, HighColor,
    // BlendFactor, WideScreenAlign. Stable actual C-string addresses.
    const char* keys[10];
    const char* empty_00ce3a0c;
    const char* left_00ce92e4;
    const char* right_00ce92dc;
};
struct NativeGuiWidgetPropertyBindings {
    // Binding addresses remain fixed for the call; the borrowed cells and
    // pointed-to bytes may change through real reader/Lua callbacks.
    const NativeGuiWidgetPropertyLiterals& literals;
    const volatile float& one_00d7a24c;
    volatile std::uint32_t& white_guard_00f8bcf0;
    void* white_00f8bce0;
    volatile std::uint32_t& black_guard_00f8bcdc;
    void* black_00f8bccc;
    void* const volatile& platform_0109cf04;
    const volatile double& widescreen_offset_00d5c118;
    NativeStringRawPoolContext& strings;
    NativeGuiWidgetPropertyDispatch& dispatch;
};
// Initialized, stable caller-owned preimage. Native S is ESP after the four
// register saves: bytes represent [S-18h,S+808h). Argument pairs are +0/+8/+10;
// +18..27 are an untouched ABI gap; native 7F8h locals start at +28. Metadata
// +28, reused vec2/string header +2C/+30, X spill +34, Pos defaults +40/+44/+48.
// This new ABI excludes native saved-register/return-address/FH3 bookkeeping
// and subsequent CRT argument reuse. Remaining allocated local bytes persist.
struct alignas(4) NativeGuiWidgetPropertyScratch { std::byte bytes[0x820]; };
// PARTIAL PROJECTION of AAA710: exactly [AAA710,AAABD6), the first ten reads,
// string cleanup and widescreen X fixup. Excludes [AAABD6,AAAECE] inclusive:
// widget virtuals, later properties, children and whole-function epilogue.
// Native ECX widget, stack visitor, full RET4; this is an explicit C++ ABI.
// Callback-visible globals/pairs/stores remain current; no logical widget or
// Lua projection. No null validation, native FH3/SEH or game integration claim.
void read_native_gui_widget_property_prefix_00aaa710(void* actual_widget,
    void* actual_visitor, NativeGuiWidgetPropertyScratch&,
    NativeGuiWidgetPropertyBindings&);
} // namespace bsp
