# Listbox row state over canonical widget owners

Addresses: `00A9B340`, `00A9BA40`, `00A9BA90`.

The three helpers use the same `GuiWidgetOwner`, Text/Icon companions, direct
child ownership vector, and `GuiListboxRuntime::rows_` as the existing GUI.
There is no copied row tree, selection index, color cache, Lua table, or fallback
state callback. Names are descriptive hypotheses. Evidence was verified against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; each live wrapper calls
the existing project/program verifier before its batch.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| A9B340..A9B3ED widget state | ECX unused; row pointer and state DWORD stack; RET8 | Complete normal body for actual current type5C profiles; delegates Text/Icon behavior to existing companions |
| A9BA40..A9BA8D any selectable row | ECX Listbox; AL Boolean; RET | Complete valid FC list domain; no native checked-STL diagnostic ABI |
| A9BA90..A9BB24 row state | ECX Listbox; checked iterator `{container,node}` and state stack; RETC | Complete valid iterator domain; caller supplies the actual dereferenced FC row |

## Producer-backed Text exception

`A9B356` compares **NativeWideString length at Text+EC** to one. It is not a
special-text mode flag. `ABA98D` assigns the resolved wide string to `this+EC`
through `4C5E20`; that routine reads the source length and calls `4C53E0`,
whose `4C5482` stores the exact length. The same canonical `text.text.size()`
represents this field. `ABAF12` resizes the narrow source at `this+F4`, then
`ABAF27` copies it. The row helper reads that same `text.source`.

Only length one plus a case-insensitive match of source to `globals.live`
selects current50. The source is borrowed through an 8-byte header transport
into existing `equal_native_string_header_00425850`; its actual `_stricmp`
and current CRT locale remain authoritative. The live bytes at `CEF718`
are `globals.live\0`. No Lua source is re-evaluated or substituted: the existing
Text providers already own the current resolved text and cached source.

`A9B38B` performs one MOVSS from live `D7A24C`, then copies it into all four
lanes before current50. The pointer is required only when this branch is
reached, so unrelated rows need no invented constant binding. Installed bits
were `3F800000`; callers still supply the live alias. Other Text rows call
their actual current80 with the entire state DWORD. Its established domain
supports non-hidden states0..3 and the hidden color path; unsupported raw
state indexing remains an explicit existing companion error.

`A9B3C7` queries current5C a second time after a non-Text first query. Type6
calls Icon current88 with low16(state), partial display type0 and **literal
FLD1**, independent of live D7A24C. The existing Icon companion performs its
actual select/rebuild behavior and retains all texture/material requirements.
Other established current types have no row-state side effects. Unsupported
current5C profiles remain explicit owner-dispatch boundaries.

## Iteration and callers

`A9BA90` first queries the actual row's current5C. A Group2 iterates its direct
`+64` child list and calls A9B340; nested Groups are inert through that helper.
Every other row calls A9B340 directly. The C++ direct-child vector transport
re-finds the same current actual child after the call before advancing. This
observes appended children and removal of other children while surviving
vector reallocation. Removing the current child is outside the native valid
iterator domain and throws. The row, group, current child and same owner
domain must stay alive across callbacks. Full checked-STL layout and error
handler behavior are excluded, and moving the current node across containers
does not become a supported iteration operation.

The parent retains its private canonical FC iterator and current selected
iterator. A9CD20 compares the **current** selected node each iteration and
passes state1 for it, state3 for every other row. A9D030's A9D2BD site instead
passes1 for selected,3 for a hidden nonselected row,0 otherwise. This helper
therefore accepts the entire state argument; it does not infer selection or
replace the frame path. `RET0C` at A9BB1A/A9BB22 establishes the three original
stack words despite pseudocode rendering them as plain parameters.

A9BA40 performs a callback-free FC sentinel walk and returns AL1 at the first
actual row77 zero. Empty/all-hidden returns AL0. It does not call current38 or
check ancestor visibility. Base constructor AA946C clears77; the source reads
the existing `scene_flags().hidden`. A friend declaration allows a linear walk
of the same private `rows_`, avoiding a second collection or ordinal rescans.

## Current calls and validation

| Native sites | Current target | Contract |
| --- | --- | --- |
| A9B34F, A9B3C7, A9BABF | A9E110 | ECX actual owner; EAX current+60 type; RET |
| A9B36A | 425850 | ECX borrowed source header; literal string stack; AL Boolean; RET4 |
| A9B382 | AB7200 | Text current80, signed state DWORD stack; RET4 |
| A9B3B7 | AB6B50 | Text current50, borrowed four-float pointer stack; RET4 |
| A9B3E5 | AB1710 | Icon current88, low16 state/mode0/literal1 stack; RETC |
| A9BAF6, A9BB11 | A9B340 | Actual direct child or actual row and original state; RET8 |
| A9BA58, A9BA66, A9BA79 | BF6713 | Native invalid-iterator paths, excluded from valid list domain |
| A9BAA2, A9BAB0, A9BADA, A9BAE8, A9BB00 | BF6713 | Native checked-iterator paths, including tautological container checks; excluded |

Current table reads established Text `D5C718 -> AB6B50`, `D5C724 -> A9E110`,
`D5C748 -> AB7200`, Icon `D5C51C -> A9E110`, `D5C548 -> AB1710`, and base
`D5C18C -> A9E110`. BA40 had no Ghidra function initially; the installed PE
listing established exactly78 bytes through its final RET at A9BA8D. The
parent subsequently created and saved that exact definition under its write
lock; the call verifier now checks its numeric sites. BF6713's existing
library name is retained; its body forwards five zero arguments to BF66EF
and cleans14h stack bytes. No invented termination contract is used here.

Strict MSVC Win32 `/W4 /WX /fp:strict` compilation passed. One local extension
of the existing actual Listbox/Group owner fixture passed empty/all-hidden/
mixed77 checks, direct-child-only Group traversal, and the parent's full
current60 true fast path/false nonzero-auto-control tail, alongside its prior
canonical attachment, selection, layout, callback and retirement checks.
It links this TU and the parent's current runtime/type TUs against the parent
baseline library; input hashes are in the report. No permanent tests were
added. The fixture does not validate Text/Icon resource effects, rendering,
native-byte differential behavior, binary ABI, or game execution. Parent
integration owns the aggregate build, current60 composition and annotations.

`reports/gui_listbox_row_control.json` carries every native CALL row and the
coverage/correction/evidence boundaries.
