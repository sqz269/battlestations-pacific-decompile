#pragma once

// Rebinding one input slot and testing it against the conflict tables, over the
// host InputSettings model. Evidence and divergences: docs/CC13_SCENE_BROWSER.md
// ("Input rebinding and conflicts").

#include "bsp/input_settings.hpp"

#include <cstdint>
#include <string>

namespace bsp {

// Address: 006A9A40 (body 006A9A40..006A9E62).
// Original ABI: __thiscall, ECX = input settings (0x540 object), [ESP+4] =
// device name NativeString*, [ESP+8] = input name NativeString*, [ESP+0Ch] =
// slot, [ESP+10h] = 14h-byte binding*, [ESP+14h] = float scale, RET 14h.
// Caller 00560430 (control binding screen update).
//
// No-op when settings+50h (DEVINPUTS) is set. Otherwise writes the binding to
// devices[device].runtime_bindings[input][slot] and sets the slot's
// runtime_reverse bit when scale < 0 (COMISS 0.0, scale; NaN clears it). Then,
// for every device AxisPair (devRec+5Ch) naming `input` on either side, the
// other name is the partner: an axis binding (00699BF0 test) is copied to the
// partner's same slot with the reverse bit set when !(scale < 0.0) (the
// constant at 00D7A218 is 0.0f; NaN clears it), so the partner reads the same
// axis in the opposite direction; a non-axis binding clears the partner slot
// to {-1, 0, 0, -1, false} when that slot currently holds an axis, and clears
// its reverse bit. The loop does not stop at the first matching pair.
// Native indexes every map with operator[] and fails the checked vector index
// on a short vector; the host throws std::out_of_range for a missing entry.
void set_input_binding_slot_006a9a40(InputSettings& settings, const std::string& device,
                                     const std::string& input, std::int32_t slot,
                                     const KeyboardInputBinding& binding, float scale);

// Address: 006A9F70 (body 006A9F70..006AA0xx).
// Original ABI: __thiscall, ECX = input settings, [ESP+4] = device name,
// [ESP+8] = input name, RET 8; AL = result. Only caller 006AAC50.
// True when any AxisPair of the device names `input` on either side.
bool input_in_axis_pair_006a9f70(const InputSettings& settings, const std::string& device,
                                 const std::string& input);

// Address: 006AAC50 (body 006AAC50..006AB67D).
// Original ABI: __thiscall, ECX = input settings, [ESP+4] = device name,
// [ESP+8] = input name, [ESP+0Ch] = slot, [ESP+10h] = out NativeString* (may
// be null), RET 10h; AL = result. Callers 0055C200, 0055C5A0, 0055D380.
//
// Returns true at once when DEVINPUTS is set, false for an unbound slot
// (class -1). Finds the first Conflicts.Groups entry g naming (device, input)
// (none: false), then visits g followed by every group that Conflicts.Pairs
// links to g, in pair order. For every entry (device2, input2) and slot s in
// 0..1, the other binding conflicts when it is bound, equals this binding in
// class, index and key (the +8 dword and the slider byte are ignored), has
// the same runtime_reverse bit, and is not excluded: at s == slot, the input
// itself is excluded, and so is an axis binding of an input that shares an
// AxisPair with `input` on the same device (only when `input` is in any pair).
// On the first conflict `out` (when given) becomes
// input2 + "|. (|" + device2 + "|.)|" and the result is true.
bool find_input_binding_conflict_006aac50(const InputSettings& settings,
                                          const std::string& device, const std::string& input,
                                          std::int32_t slot, std::string* out);

} // namespace bsp
