#pragma once

#include "bsp/gui_lua_reader.hpp"
#include "bsp/input_settings.hpp"

namespace bsp {

// Host boundary for the input-manager calls made by 006aa640. Implementations
// must supply real behavior; there are no successful no-op defaults.
struct KeyboardRuntimeHost {
    virtual ~KeyboardRuntimeHost() = default;
    // 00a92260 tests the native 30h-stride action record's byte +0, with a
    // bounds check. Apply calls this once for action 128h before all devices.
    virtual bool action_registered_00a92260(std::uint32_t action) = 0;
    // 006aa090 inspects AxisPairs and compares the pair's input descriptions
    // through 0069e860. That helper remains a boundary, not guessed equality.
    virtual bool use_alternate_axis_slots_006aa090(const InputSettings& settings,
        const std::string& device_name, const std::string& input_name) = 0;
    // 00a93750 grows the native action's bindings, copies the 14h binding and
    // float scale, then calls 00a91e80 to resolve device references.
    virtual void bind_action_00a93750(std::int32_t action, std::int32_t slot,
        const KeyboardInputBinding& binding, float scale) = 0;
};

// Native ECX=settings, stack reader*, RET 4; body 006aba50..006ac025.
// Reads the already-entered keyboardSetup table, then calls runtime apply.
// Existing device/action/sensitivity names determine what is read. Missing
// devices/actions/sensitivities preserve values; a missing numeric slot 0/1
// resets {-1,0,0,-1,false} but preserves its independent reverse flag. The
// unarchived binding+8 word is preserved for a present slot.
void read_keyboard_setup_006aba50(InputSettings& settings, GuiLuaReader& reader,
    KeyboardRuntimeHost& host);

// Native ECX=settings, no stack arguments, RET; body 006aa640..006aac43.
// DEVINPUTS or an unset +5 latch skips all host calls. The latch is not cleared.
// Rebuilds effective sensitivities in a copy, then installs two slots per
// described input code. Short native arrays throw std::out_of_range here;
// sensitivity category outside 0..4 with nonempty codes throws
// std::invalid_argument rather than reading a stale/uninitialized temporary.
// New host interfaces, not binary-compatible replacement objects. Evidence,
// rounding boundaries and dependencies: docs/KEYBOARD_RESTORE.md.
void apply_keyboard_setup_006aa640(InputSettings& settings, KeyboardRuntimeHost& host);

} // namespace bsp
