# CC13 input rebinding and the segment-6 scene classes

Worker cc13-settings2 (Claude lineage cc13), successor of cc13-settings1. Two parts: the two
input-settings routines settings1 surveyed but did not reconstruct (segment 29), and partition
segment 6 (`00465610..0047C680`).

Every name here is a hypothesis, not a recovered symbol. Status words: *exported* (pseudocode in
`exports/`), *reconstructed* (C++ in `src/`), *build-tested* (compiles in the MSVC Win32 build),
*game-validated* (observed in a run). Nothing in this document is game-validated.

## Input rebinding and conflicts

Host: `include/bsp/input_rebind.hpp`, `src/input_rebind.cpp`, over the `InputSettings` model in
`include/bsp/input_settings.hpp`. Status: reconstructed, build-tested, not ABI-compatible.

### `006A9A40` `BSP_InputSettings_SetBindingSlot`

`__thiscall`, `ECX` = settings, stack: device name, input name, slot, 14h-byte binding pointer,
float scale; `RET 14h` (`006A9E62`). Only caller `00560430` (`BSP_ControlBindingScreen_Update`).

1. Returns at once when settings+50h (DEVINPUTS) is set (`006A9A58`).
2. `devices[device]` (`0055C110` on settings+8), then `runtime_bindings[input]` (`006A45C0` on
   devRec+0Ch) slot `slot` receives the five dwords of the binding (`006A9AB1..006A9AE8`).
3. The slot's bit in `runtime_reverse[input]` (`006A4CA0` on devRec+6Ch) is set when
   `COMISS 0.0, scale; JBE` is not taken, that is when `scale < 0`; NaN clears it
   (`006A9AC1..006A9AF3`).
4. Walks the device's AxisPair vector (devRec+5Ch, begin +60h, end +64h, 10h-byte elements, each
   a vector of at least two pooled strings). A pair names the input when its first string equals
   it (length compare, then `_stricmp`), giving the second as partner, or else when its second
   string equals it (`BSP_NativeString_EqualsInsensitive`), giving the first. The loop does not
   stop at a match.
5. The partner name is copied (`006A9C24`) and `00699BF0` (axis test: class 1 key >= 8, class 2
   key >= 3Ch, unsigned) is applied to the **new** binding (`006A9C60`, `ECX = EBX` = the binding):
   - axis: the partner's same slot receives the same binding; its reverse bit is set when
     `COMISS scale, [00D7A218]; JC` is not taken. `00D7A218` holds `00 00 00 00` (0.0f), so the
     partner is reversed when `scale >= 0` and NaN clears it. The two halves of a pair read one
     physical axis in opposite directions.
   - not an axis: when the partner's same slot holds an axis, it becomes `{-1, 0, 0, -1, 0}`
     (`006A9DC4..006A9DD2`; the byte at +10h is written from a stack dword whose low byte was
     zeroed) and its reverse bit is cleared. A partner holding a button is left alone.

### `006A9F70` `BSP_InputSettings_IsInputInAxisPair`

`__thiscall`, `ECX` = settings, device name, input name; `RET 8`, `AL`. True when any AxisPair
of `devices[device]` has the input on either side. Only caller `006AAC50` (`006AAF8C`).

### `006AAC50` `BSP_InputSettings_FindBindingConflict`

`__thiscall`, `ECX` = settings, stack: device name, input name, slot, out string pointer (may be
null); `RET 10h`, `AL` (`006AB659` returns 1, `006AB67D` returns 0). Ghidra's `__fastcall`
prototype shows a spurious `EDX` parameter; the four stack arguments are `param_3..param_6`.
Callers `0055C200`, `0055C5A0`, `0055D380` (the control binding screens).

1. DEVINPUTS set: returns 1 (`006AAC68..006AAC82`).
2. The slot's binding; class -1 returns 0. The slot's reverse bit is captured (`006AACF9..
   006AAD0A`, `0048F230`).
3. Finds the first group in Conflicts.Groups (settings+30h, begin +34h) holding an entry
   `(device, input)`; none returns 0. `00552250` is the 10h-element vector size.
4. Builds a list: that group, then for each Conflicts.Pairs row (settings+40h, begin +44h) the
   other index of a row that names the group (first column checked first) (`00442190` push_back).
5. `paired = 006A9F70(device, input)`.
6. For each listed group, for each entry `(device2, input2)`:
   `same_device`, `same_input` by case-insensitive compare; `shares_axis` only when `paired` and
   `same_device`: some AxisPair of `devices[device]` holds the input and `input2`
   (`006AB0F0..006AB2BE`). Then for slots 0 and 1 (`stride 14h, < 28h`): the other binding
   `devices[device2].runtime_bindings[input2][s]` conflicts when it is bound, it is not excluded
   (at `s == slot`, excluded when `same_device && same_input`, or when `shares_axis` and the
   other binding is an axis), its class, +4 and key equal this binding's (+8 and the byte at
   +10h are not compared), and its reverse bit equals this binding's.
7. On the first conflict: when `out` is non-null it becomes `input2 + "|. (|" + device2 + "|.)|"`
   (`00CF81D0`, `00CF81D8`; concat order from `006AB508..006AB551`, where the first left operand
   is the input slot and the second the device slot of the stack frame); returns 1.

### Divergences and uncertainty

- Native indexes every map with `operator[]` (inserting a missing key) and fails the checked
  vector index on a short vector (`00BF6713`). The host uses `at()` and throws
  `std::out_of_range`.
- The partner-reverse rule (`!(scale < 0)`) and the own-slot rule (`scale < 0`) differ at
  exactly 0.0: a zero scale leaves the slot unreversed and the partner reversed.
- The `|. (|`/`|.)|` markup reads as "input (device)" in the GUI text system; its rendering was
  not observed.
- `include/bsp/input_settings.hpp`: `InputNameRecord::primary`/`secondary` were renamed to
  `device_type`/`key`, on the `0069E9E0` evidence settings1 recorded (`0069EDBB..0069EDCC`).
