#pragma once

#include "bsp/input_action_classifier.hpp"

#include <cstdint>
#include <vector>

namespace bsp {

struct InputActionRecord;

// Five native dwords passed by value after action/slot to 00a93750. The
// caller translates the native +08h pointer word into an actual InputDevice*
// in this process; integer words must not be cast into fabricated host objects.
struct InputBindingInstallSource {
    std::int32_t device_class{-1};
    std::uint32_t device_index{};
    InputDevice* cached_device{};
    std::uint32_t input_code{};
    std::uint32_t flag_word{};
};

// Native ECX=input manager, stack=uint32 action, EAX=0/1, RET4. The unsigned
// bounds check precedes the byte+00h registration check (not enabled+01h).
bool input_action_registered_00a92260(
    const std::vector<InputActionRecord>& actions, std::uint32_t action) noexcept;

// Native ECX=destination 34h record, stack=source*, EAX=destination, RET4.
// Copies the projected fields and independently owns both modifier arrays.
// Scale follows the original x87 FLD/FSTP (including NaN conversion).
InputActionBinding copy_input_action_binding_00a93100(const InputActionBinding&);

// Native ECX={begin,count,capacity}, stack=signed count, RET4. Capacity is
// clamped to at least one and existing bindings are copied, including modifiers.
// Host vector allocation/capacity and exception ABI replace native allocation.
void reserve_input_action_bindings_00a93220(
    std::vector<InputActionBinding>&, std::int32_t capacity);

// Native ECX={begin,count,capacity}, stack=signed count, RET4. New slots have
// class=-1, scale=1, cleared scalar fields and empty modifiers; shrink discards
// trailing records. A negative size throws here instead of native invalid access.
void resize_input_action_bindings_00a93500(
    std::vector<InputActionBinding>&, std::int32_t count);

// Native ECX=input manager, stack=(action,slot,five source dwords,float scale),
// RET20h. Grows the selected action's array through slot+1, replaces only the
// source fields/response flag/scale, preserves modifiers, and rebinds ALL its
// bindings using the existing 00a91e80. Class==2 enables response_curve.
// Native copies flag_word to +14h; only its low byte is semantically modeled.
// Invalid action/slot throws before mutation; native has no such bounds check.
// The groups and every class visited by rebind must meet its existing contract.
void install_input_action_binding_00a93750(
    std::vector<InputActionRecord>& actions, std::int32_t action,
    std::int32_t slot, const InputBindingInstallSource& source, float scale,
    const InputBindingDeviceGroups& groups);

} // namespace bsp
