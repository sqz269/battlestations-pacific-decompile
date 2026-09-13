# Actual Inputs and binding construction

Addresses: 00698A10 (prefix through00699AD7), 00A93350, 00A93070,
00A93620, 00A93830. Prior stages: docs/NATIVE_INPUT_CONFIGURATION_LOAD.md and
docs/NATIVE_INPUT_CONFIGURATION_MODIFIERS.md.

The new source extends the concrete script/modifier prefix through the complete
`Inputs` parsing loop and the additional action128h registration. It operates
on the actual24h action owner, actual30h actions, actual34h bindings, actual14h
descriptors and actual0Ch count/capacity arrays. It does not substitute the older
projected InputActionBinding or a second Lua owner.

`load_native_input_configuration_actions_prefix_00698a10` stops at699AD8,
before final rebind, zero-delta update and settings application. It retains the
four real Lua objects in caller-supplied stable storage: globals, Inputs, key
and value. The remaining native tail still owns them at that point. The caller
releases them in value/key/Inputs/globals order after its subsequent work. All
owned objects unwind here if parsing throws. This prefix does not set byte520
and must not replace a complete698A10 call.

The parser gets and retains the actual owner once at69903A. For each Inputs key,
it converts the action id, iterates `groups` values into a raw DWORD array, and
reads `helper` (nil means false). It calls the existing concrete A93C80 action
configuration before reading bindings. That producer can activate the action;
its required device/timing services remain real application dependencies.

Bindings are read from lowercase `inputs`. Each consecutive numeric entry is
first fetched, tested for nil and destroyed. A present entry is then fetched
**again** for parsing; the first nil ends the sequence. Lua metatable effects
make this repeated lookup observable. Each binding reads `press`, then optional
`and` and `not` tables, using the established descriptor decoder for every row.
The vectors copy all five descriptor words in order, including scratch padding.

General and map swap selection read live F889C1 and F889C4 respectively and
search the configuration's corresponding checked vectors by action id. A general
match skips the map read. The descriptor subsequently applies its separate
F88A30/configuration4C9-or4CC gate. The parser reads the live one bits before the
`mul` lookup; a nonnil multiplier uses the real Lua number conversion. Camera
and plane inversion then check current bytes4CA and4CB and their checked lists,
in that order. Each match multiplies the binary32 value by the live D7A250 double
using x87 and stores binary32. The dead CMP ESI,ESI validation calls remain
unreachable; returning diagnostics retain captured iterators and native reloads.

The recovered binding path consists of four complete normal-flow bodies:

| Entry | Native ABI | Behavior |
| --- | --- | --- |
| A93350,156 bytes | ECX fresh34h; descriptor, required, forbidden, float; RET10h/EAX fresh | Clear byte0, copy descriptor words to+4, construct/copy required then forbidden arrays, store raw scale bits, reread source class to set byte1 for class2. Padding2..3 is retained. |
| A93070,109 bytes | ECX binding; RET | Resize/free forbidden, then required. The first failure unwinds required; normal required cleanup is not retried. |
| A93620,111 bytes | ECX action; descriptor, required, forbidden, float; RET10h | Construct a temporary34h binding, append through existing A93440, then destroy the temporary. An append exception destroys the owned temporary. |
| A93830,46 bytes | ECX owner; action id, descriptor, required, forbidden, float; RET14h | Spill scale through x87, address current owner base plus id*30h, then call A93620. No invented action bounds check or rebind. |

A93350 acquires only the required-array cleanup state before constructing the
forbidden array. A93620 acquires temporary ownership only after its constructor
returns and resets that state before normal destruction. The three compiler
handlers and their one-state maps were verified, defined where missing and
saved. Loader states15..34 separately confirm the Lua/array cleanup ordering.

Ghidra previously truncated A93070 after the first returning free call. The
explicit tail repair decoded through its RET atA930DC but did not extend stored
body metadata. A separate byte-verified recreation retained the old name/comment
and established the complete109-byte body with34 instructions and zero gaps.
Both receipts are retained in the flow/definition reports; decoding alone is
not presented as the completed body repair.

After all Inputs entries, the parser creates a context array containing1Eh,
configures action128h with helper false, and releases that array. It leaves the
four outer Lua objects alive for the remaining166-byte native tail. Actual
storage is populated, but application integration still needs that tail and its
raw settings dependencies.

One ignored fixture executes the original4296-byte loader prefix, descriptor,
four binding producer bodies and previously verified nested-vector instructions.
It shares the existing concrete Lua, array, action activation and file-loading
leaves. Installed fundamentals.lua and Inputs.lua are supplied through external
fixture streams. Its explicit observation/destruction/return tail ends execution
before final rebind/update/settings; unsupported library paths fail if reached.

All14015 compared words match across three prefix calls: installed data with
normal and swapped/inverted settings, plus a metatable case proving the repeated
lookup and first-nil stop with required/forbidden modifiers and both inversions.
The controlled constructor case also compares descriptor padding and retained
binding padding with identical preimages. Arbitrary native/C++ stack padding
preimages are masked in the full-parser comparison. Lua objects, arrays, owner
records, listeners, strings and streams follow their concrete cleanup paths;
no device provider was called. The210 CALL rows and eight disk/live seeds pass.
Final merged source/archive/build evidence is pinned in the associated report.

This is component evidence. Original register/FH3/SEH compatibility, malformed
negative array sizes, arbitrary stack aliases, native physical/archive VFS
routing, final settings application and gameplay remain unvalidated. No game
launch, single-instance bypass or hardware polling occurred.
