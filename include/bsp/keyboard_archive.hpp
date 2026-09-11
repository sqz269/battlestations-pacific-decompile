#pragma once

#include "bsp/game_settings.hpp"
#include "bsp/input_settings.hpp"

namespace bsp {

// Addresses: 006A51C0 (body 006A51C0..006A563D), with runtime storage seeded
// by 006A8067..006A80B1 and 006A8448 inside 006A7BE0.
// Original ABI: ECX=InputSettings*, [ESP+4]=writer*, __thiscall, RET 4.
// Host C++ interface only: not a binary-compatible object or native vtable.
//
// Writes the body of the already-open keyboardSetup section, in native map
// order. Does not call options.txt persistence or open keyboardSetup itself.
// Emits exactly slots 0 and 1; disabled device_type==-1 slots are omitted.
// Short binding arrays or missing/short reverse arrays for enabled slots are
// invalid native state; this implementation throws std::out_of_range instead
// of continuing after the native checked-container failure.
// Evidence, original offsets and remaining runtime work:
// docs/KEYBOARD_SETTINGS_ARCHIVE.md.
void write_keyboard_setup_006a51c0(const InputSettings& settings,
                                 SettingsWriter& writer);

} // namespace bsp
