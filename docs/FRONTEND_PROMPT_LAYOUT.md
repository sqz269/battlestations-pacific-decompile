# Prompt entry, button preparation and horizontal layout
Addresses: `00531380`, `00532110`, shared tail `00530a60..00530c17` in `00532360`.

Descriptive names are hypotheses, not recovered symbols. The code is in
`include/bsp/frontend_prompt_layout.hpp` and `src/frontend_prompt_layout.cpp`.
It extends the controller in `FRONTEND_PROMPT_SCREEN.md` without changing that
controller's owned files. The primary integrator binds its existing
`FrontEndPromptHost::layout_buttons_00530a60` to this module separately.

## Horizontal layout

`00532360` ends its successful refresh path in a jump to `00530a60`. The latter
has its own ECX-screen prologue and RET at `00530c17`, but Ghidra already stores
these instructions as a shared body range in `00532360`. It is a reconstructed
fragment, not evidence of an undefined function. No function was split or created.
The bridge's ordinary function listing starts at `00532360` and omits the earlier
shared range, so the tail's assembly was decoded from installed disk bytes after
all 440 saved Ghidra bytes were verified equal.

The layout reads these native fields:

| Screen offset | Meaning |
| --- | --- |
| +28h, +2Ch, +30h, +34h | Yes, No, Accept and Checkpoint/Restart text objects |
| +38h | Group whose width comes from `00aa6740`, return value's first float |
| +25Dh, +25Eh, +25Fh, +260h | The four enabled flags in the same field order |
| Button +114h | Measured text width in font units |

It sums only enabled widths, in **Yes, Accept, Restart, No** order. Each width
is divided by the double `960.0` at `00cec380` and stored to float before addition.
The Yes addition uses the double zero at `00d7a258`. Each accumulated sum is also
stored to float. The gap is `(group_width - total_width) / (enabled_count + 1)`.
There is no clamp: an oversized row has negative spacing. No enabled buttons still
causes a group-width read but no position writes.

The first position is the gap. For each enabled button it calls `00aa78d0` to set
local X, then, except for No, rereads the button's current measured width and advances
by that normalized width plus the gap. It rereads subsequent enabled flags after
each setter. Neither the total nor gap is recomputed when a setter changes geometry
or flags. The C++ preserves the float spill points and uses double arithmetic between
spills; unusual x87 precision-control/rounding modes are not reproduced.

`FrontEndPromptLayoutHost` exposes the two concrete GUI calls and measured-width
read. `ActualFrontEndPromptLayoutHost` performs the +114h read on an actual object;
only GetSize/SetLocalX, including their engine notifications, remain required calls.

## Button preparation

`00532110` is ECX=screen, one stack widget argument, RET4. Its scratch vector is at
screen+268h with begin at +26Ch and end at +270h. If nonempty it applies the last
NativeString through `00abaed0(widget, &last, true)`, then rereads the vector and
destroys/removes its **current** last string if it remains nonempty. It does not
pop a captured old item when the localization call changes the vector.

Assembly `00532153..00532187` resolves the hidden receiver and allocator ABI:
ECX is loaded from the stack widget for `00abaed0`; destruction receives the current
last header in ECX and current end in EDX through `00432050`, with two extra ignored
stack arguments and RET8. That helper releases each string's buffer through the
sized pool using length+1. The host uses the existing single-header destruction
body `0041dd20` for the one current item, then pops its `std::vector<NativeString>`.
The vector owns these strings; malformed vectors and debug iterator failures are
outside the host projection. Callers still own cleanup of any remaining scratch.

## Screen entry

`00531380` is ECX=screen, no stack arguments, RET. Registration supplies root+14h,
background+18h, group+38h and listener+10h. Entry performs this sequence:

1. If screen+3Ch is set, call background virtual+34h(true). A clear flag skips it.
2. Recursively find and bind `Gamertag_Text`, `Message_Text`, `Options_Listbox`,
   `Yes_Text`, `No_Text`, `Accept_Text`, `Checkpoint_Text`, in that order, under
   root+14h. Each binding is written before releasing its temporary NativeString.
3. Find `background_Icon`, call its virtual+34h(screen+288h), release the key and
   clear +288h. Show the navigation widget through virtual+34h(true), then call
   group virtual+84h with whether `00e198c4` is nonnull.
4. If previous screen `00e1930c` exists, call virtual+1Ch only when +5h is set,
   then clear its wanted/active bytes and commit through `004f83b0` unconditionally.
   If owner `00e198b4` exists and its +54h screen is active, call that screen's
   +8h interface virtual+0Ch. The host null result represents absent owner; native
   does not tolerate a missing +54h screen inside a present owner.
5. Set input mode +264h to 1, then clear it if `00f88a30` is zero. Otherwise get
   the GUI manager through `004c12b0` and reset screens through `00aa0f70`.
6. Configure **Yes, No, Accept, Restart** in field order. Bind callback text through
   `00531130`, passing listener=screen+10h and event=that same button. Set the native
   button fields below, then call virtual+50h with RGBA `(1,1,1,1)`.

The callback text comes from one-byte constructors `00531030(-59h),(-5Dh),(-5Eh)`
and concatenations. Assembly shows the final bytes are `A2 A3 A7`; `004c5e60`
zero-extends each byte, yielding UTF-16 `00A2 00A3 00A7`, without code-page conversion.
The host passes that established text directly; native temporary allocation and
exception unwinding for this constant text are not claimed.

| Actual button offset | Written value | Evidence |
| --- | --- | --- |
| +DCh | dword 0 | `00aa6bc0(0,0)` first store |
| +79h | byte 0 | `00aa6bc0(0,0)` second store |
| +1B4h | byte 1 | `0053190e` and three repeated blocks |
| +1B8h | float +0 | `00531903..0053191e`, shared spill reread for other buttons |
| +1BCh | float bits `3c360b61`, about 0.01111111138 | constant `00ced318` |

Names for the last three fields' visual role remain uncertain. The implementation
writes their verified offsets rather than claiming a cursor, animation or highlight
interpretation. Raw GUI bindings must refer to correctly sized, writable native
objects. FindChild, flag/color virtuals, previous/overlay callbacks, reset and
`00531130` are required engine services, with no fallback no-ops.

## Validation and limits

- Guarded Ghidra queries/export verified project `bsp`, program
  `/battlestationspacific.exe`, using `C:/Users/sqz269/bsp.gpr`. Read-only throughout.
- Complete saved bytes match installed disk: layout 440 bytes, entry 1906 bytes,
  preparation 128 bytes. SHA-256 values are in `reports/frontend_prompt_layout.json`.
- `python tools/ghidra_export.py verify-seeds` passed; MSVC Win32 Release
  `./scripts/build.ps1` passed both existing CTests, including native math differential.
  Those existing tests are not prompt GUI behavior tests.
- One ignored focused Win32 fixture passed a negative-gap row where the first
  SetLocalX changes Yes width and disables Accept: later calls target Restart then
  No at the expected positions. It exercises the actual +114h adapter read.
  Entry and scratch preparation are build-tested and assembly-reviewed only.

Local artifacts use `local/prompt-layout-*`, plus `local/prompt-enter-native-bytes.txt`
and `local/prompt-prepare-native-bytes.txt`; no permanent test was added.
This is not an ABI-compatible replacement or game validation. GUI construction,
native callback ownership/allocation, localized glyph generation, full native screen
layout and rendering still require the supplied engine services.
