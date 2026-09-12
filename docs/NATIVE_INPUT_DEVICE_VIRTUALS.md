# Native input device virtual leaves and scans

Addresses: 00A93E80, 00A93E90, 00A93EA0, 00A95BE0, 00A96360, 00A9A100,
00A93F30, 00A93F60, 00A94440, 00A95ED0, 00A95F00, 00A95F10,
00A9AAB0, 00A9AAE0, 00A9AB10, 00A9A380.

`native_input_device_virtuals.hpp/.cpp` implements these bodies over actual raw
device allocations. It adds source interfaces and retains the existing typed
implementations as separate reconstruction fragments. It does not construct a
runtime owner, reinterpret storage as a C++ `InputDevice`, or introduce a GUID
container. Complete source control flow is covered within the explicit raw
dispatch and readable-storage contracts below. These source interfaces are not
drop-in binary vtables; native upper-register bits and stack layouts are not
claimed.

## Routines and ABI

All entries receive the native device in ECX; no entry consumes EDX as an
argument. Except the two noted leaves, there are no native stack arguments and
the return is `RET` (zero popped argument bytes). The ignored ECX leaves do not
require readable storage. End addresses are inclusive, not an inferred extent
to the next aligned symbol. No routine has an omitted cleanup tail or native EH
map. Added C++ providers can throw through scans; there is no source catch or
invented successful return.

| Entry | Source behavior / native return | Inclusive end | Coverage |
|---|---|---|---|
| A93E80 | Reset leaf; ECX ignored, no writes, return void | A93E80 | Complete |
| A93E90 | Relative setter leaf; ECX and two DWORD stack slots ignored, `RET 8`, void | A93E92 | Complete bytes; missing Ghidra start |
| A93EA0 | Relative query false; ECX/code ignored, `XOR AL,AL; RET 4` | A93EA4 | Complete |
| A95BE0 | EAX = borrowed D5B6AC `GameController` literal | A95BE5 | Complete bytes; missing Ghidra start |
| A96360 | EAX = borrowed D5B6FC `Keyboard` literal | A96365 | Complete |
| A9A100 | EAX = borrowed D5B8A4 `Mouse` literal | A9A105 | Complete |
| A93F30 | AL: any queried-active code 0..89 | A93F53 | Complete |
| A93F60 | AL: any queried-active code 0..59 | A93F83 | Complete |
| A95ED0 | AL: any queried-active code 0..255 | A95EF6 | Complete |
| A9AAB0 | AL: any queried-active code 0..16 | A9AAD3 | Complete |
| A9AAE0 | AL: any queried-active code 0..7 | A9AB03 | Complete |
| A95F00 | Tail current vslot28; same ECX, preserve returned AL | A95F06 | Complete bytes; missing Ghidra start |
| A95F10 | EAX: first queried-active code 0..255, otherwise -1 | A95F39 | Complete |
| A94440 | EAX: first strict greatest magnitude among active codes 0..89, else -1 | A944F1 | Complete |
| A9AB10 | EAX: same selection over codes 0..16 | A9ABC1 | Complete |
| A9A380 | `FLD float[ECX+238h]; RET`, ST0 mouse double-click interval | A9A386 | Complete |

The A95F00 final instruction is the two-byte `JMP EDX` at A95F05. Its callee
returns directly to the native caller. The source wrapper deliberately preserves
an arbitrary nonzero AL rather than converting it to Boolean 1. Other any-active
scans normalize a successful query to AL=1, and return AL=0 when all fail.

## Raw dispatch, producer identity and finite profiles

The actual allocation's first DWORD is the only dispatch identity. Every scan
loads it at its native query point. Selection loads it again after a successful
query and before fetching the value; the query is allowed to change that DWORD.
The required provider receives the captured profile and the same allocation
identity as receiver. It must resolve that captured profile, not silently read a
new profile or cast the allocation to a typed companion. The code argument is
one DWORD stack slot for native vslots20/24; their native callees use `RET 4`.
Vslot28 receives no stack argument. Existing AD providers establish these bodies:

| Profile / producer | Query20 | Value24 | Activity28 | Buttons2C | Select30 | Name0C |
|---|---|---|---|---|---|---|
| D5B904 / keyboard A9A3E0, actual310h | A95E70 | A95E90 | A95ED0 | A95F00 | A95F10 | A96360 |
| D5B8B0 / mouse A9A290, actual23Ch | A99F70 | A99FE0 | A9AAB0 | A9AAE0 | A9AB10 | A9A100 |
| D5BB48 / XInput A9A5A0, actual240h | A9A610 | A9A660 | A93F30 | A93F60 | A94440 | A95BE0 |
| D5B7F0 / joystick A99940, actualB48h | A98BD0 | A98C50 | A93F30 | A93F60 | A94440 | A992E0 (existing) |
| D5B670 / common A95D70, actual220h prefix | BF698E purecall | BF698E purecall | A93F30 | A93F60 | A94440 | A95BE0 |

The keyboard table ends at slot34. Bytes at D5B93C are not a keyboard slot38.
Mouse slot38 is A9A380 with an ST0 return, while common/XInput/joystick slot38
is the force-output interface. This packet neither truncates these tables to a
uniform type nor invents empty behavior for the common abstract query/value.
The parent runtime must provide finite profile dispatch and retain the original
purecall behavior for that abstract profile. Existing providers are in
`native_keyboard_mouse`, `native_gamepad_xinput` and `native_joystick`; this
packet deliberately borrows their dispatch rather than creating another owner.

The name leaves return canonical borrowed literal pointers supplied through
`NativeInputDeviceNameLiterals`; they neither allocate nor release a string.
The mouse +238 float is written by A9A290 from the platform double-click interval
using the existing CRT conversion. The getter is a naked Win32 fastcall leaf so
its ECX/FLD/ST0/RET sequence adds no SSE conversion.

## Selection arithmetic

A9446E..A944D7 and A9AB3E..A9ABA7 use a binary32 spill of the current vslot24
ST0 result, an x87 ordered-positive comparison against `FLDZ`, then `SUBSS` from
the live D7A208 negative-zero word for the nonpositive/unordered arm. The prior
selected signed value is classified with `COMISS` against the live D7A218 zero
word; its nonpositive/unordered arm performs the same negative-zero subtraction.
An x87 `FCOMIP` and strict-greater branch compare the resulting magnitudes.
Only a strict increase replaces the selected index and its **original signed
value**. First ties persist, NaNs do not win, and zero-only scans retain -1.

`NativeInputSelectionConstants` borrows both live words. Inline MSVC x86 assembly
preserves the two distinct sign tests, their subtraction instructions and load
timing. There is no `fabs`, generic maximum, fixed injected zero global, or clamp.
This establishes the source arithmetic path, not arbitrary native call-stack
compatibility or a game-validated numerical differential result.

## Evidence and remaining analysis metadata

Read-only Ghidra batches verified `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. Thirteen defined bodies were refreshed and inspected
through their final instructions. All ten indirect CALL addresses have live
containing-function confirmation. The report records each CALL plus the missing
entry's indirect tail JMP. There are **zero direct CALL instructions** in these
16 bodies; the mechanical report tool skips the eleven explicitly indirect rows.
That skip is not evidence resolving every possible runtime profile.

Three aligned entry decodes have no current Ghidra function membership:
A93E90..A93E92 (`RET 8`), A95BE0..A95BE5 (`MOV EAX,D5B6AC; RET`), and
A95F00..A95F06 (`MOV EAX,[ECX]; MOV EDX,[EAX+28h]; JMP EDX`). Their next bytes
are CC padding at A93E93, A95BE6 and A95F07 respectively. Source coverage of those
bytes is complete; metadata/body membership remains qualified until the primary
agent defines the functions after lease release. This worker made no Ghidra writes.

## Validation

MSVC Win32 Release build passed after all eight native seeds matched disk; both
existing CTests passed. One ignored `/MANIFEST:EMBED` probe exercised the actual
AD keyboard query over raw310h state (512 queries, index255), preserved tail AL128,
unchanged no-op leaf storage and literal pointer identity. Explicit fixture-only
query/value callbacks changed raw profiles between calls: 90 gamepad and 17 mouse
iterations each observed the new profile, selected index7, retained the first
equal magnitude, and ignored NaN/zero contenders. The raw mouse getter returned
0.375 from +238h. Logs and the reusable strict archive runner are in worker `local/`.
No permanent tests were added. These results do not establish controller hardware,
SDK polling, full raw owner composition, application dispatch or gameplay behavior.
