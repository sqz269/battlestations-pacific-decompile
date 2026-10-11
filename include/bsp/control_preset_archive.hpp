#pragma once

#include "bsp/game_settings.hpp"
#include "bsp/input_settings.hpp"

namespace bsp {

// Address: 006A5640 (body 006A5640..006A5A93). Sole caller: the control
// binding screen update 00560430 at 00560649, which passes the settings
// singleton from 005547D0 and a writer built for the "CONTROL_PRESET" literal
// (00CEE75C) by 00BD5D60/00BD6DB0.
// Original ABI: ECX=InputSettings*, [ESP+4]=writer*, __thiscall, RET 4.
// Host C++ interface only: not a binary-compatible object or native vtable.
//
// Opens a "Preset" section (00CEE818) through the writer's virtual +4h,
// emits the same per-device inputSettings/sensitivitySettings body as
// 006A51C0, then closes the section through virtual +8h. Assembly differs
// from 006A51C0 only in that wrapper and in testing the slider/reverse
// booleans inline instead of through the 00BD5680 variant compare; both omit
// a false value, so the emitted rows are identical.
// Evidence and uncertainty: docs/CC13_SETTINGS_SAVEDATA.md.
void write_control_preset_006a5640(const InputSettings& settings,
                                   SettingsWriter& writer);

} // namespace bsp
