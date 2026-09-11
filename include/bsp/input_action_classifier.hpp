#pragma once

#include <cstdint>
#include <vector>

namespace bsp {

struct InputActionListener;
class InputDevice;

// Image defaults at 00e12f20, 00e12f24 and 00e12f28. Names describe the
// comparisons in 00a91a50; they are hypotheses, not recovered symbols.
struct InputActionTimingThresholds {
    float quick_edge_and_hold{0.25f};
    float repeat_delay{0.5f};
    float repeat_step{0.1f};
};

// Native: ECX=listener, stack=(float seconds, byte previous, byte current),
// RET 0Ch. Explicit state adapter, not an ABI-compatible listener layout.
// Uses ordered comparisons, including for NaN, and one repeat subtraction per
// update. The existing listener names since_press/since_release are historical:
// +14h actually measures time since release, +18h time since press.
void update_input_action_listener_00a91a50(InputActionListener& listener,
    float seconds, bool down_previous, bool down_current,
    const InputActionTimingThresholds& thresholds) noexcept;

// The 14h-stride modifier record's fields read/written by 00a91e80. Native
// +0Ch input code and +10h are not touched by this routine and are not modelled.
struct InputActionModifierBinding {
    std::int32_t device_class{0};       // +00h
    std::uint32_t device_index{0};      // +04h, unsigned vector index
    InputDevice* cached_device{nullptr}; // +08h
};

// Fields of the 34h-stride binding at action+10h/count+14h touched by 00a91e80.
// Required/forbidden roles are supported by the +20h virtual predicates in
// 00a92370; their cached pointers are the only modifier fields rebind changes.
struct InputActionBinding {
    bool resolved{false};                 // +00h
    std::int32_t device_class{-1};         // +04h; -1 skips, retaining cached_device
    std::uint32_t device_index{0};         // +08h
    InputDevice* cached_device{nullptr};   // +0Ch
    std::vector<InputActionModifierBinding> required_modifiers;  // +18h/count+1Ch
    std::vector<InputActionModifierBinding> forbidden_modifiers; // +24h/count+28h
};

// Vectors at backend+6Ch+class*24h: begin +04h, end +08h, pointer stride 4.
// This is separate from InputDeviceTable's fixed 3x8 attachment table.
using InputBindingDeviceGroups = std::vector<std::vector<InputDevice*>>;

// Native: ECX=30h action record, no stack arguments, RET. Precondition: every
// device_class actually visited indexes groups; native only special-cases -1
// on the primary binding and does not bounds-check the class. Device indices
// are checked. Arrays must be valid and stable for the duration of this call.
// Resolution stops at the first missing device within each binding; later
// cached pointers remain untouched. No device virtual methods are called.
void rebind_input_action_00a91e80(std::vector<InputActionBinding>& bindings,
    const InputBindingDeviceGroups& groups) noexcept;

} // namespace bsp
