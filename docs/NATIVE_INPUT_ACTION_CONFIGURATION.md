# Raw action configuration producers

Addresses: `00A93C80`, `00A92D40`; partial caller evidence at `00698A10`.

This packet implements registration and context activation on the existing
actual24h action owner, actual30h action records, actual34h bindings and
actual24h listeners. It does not implement the larger settings/Lua loader at
00698A10. The two source functions reuse the recovered record allocation,
context-vector assignment, listener allocation, binding poll and listener tick.
They create no parallel action owner or settings projection.

| Entry | Native ABI | Coverage |
| --- | --- | --- |
| A93C80 | ECX owner; stack uint32 action, source12h header pointer, DWORD whose low byte requests listener replacement; no result; RET0Ch atA93D47 | Complete normal flow through inclusiveA93D49, exclusiveA93D4A, under existing actual-storage/provider contracts |
| A92D40 | ECX action; stack owner-context12h header pointer, DWORD enabled low byte, int32 active value; no result; RET0Ch atA92DFE orA92E68 | Complete normal flow through inclusiveA92E6A, exclusiveA92E6B, under the same contracts |
| 698A10 | ECX embedded game input configuration (OnInitOnce passes game+3C); no stack input; raw excluded RET at699B7D | Evidence only: entry, two A93C80 callers, selected parsing/storage sites and excluded tail. No C++ entry or completed parser is supplied |

These are new C++ interfaces, not original calling-convention, FH3/SEH or
hardware-fault replacements. The two implemented bodies have no local EH owner
or rollback. Exceptions from their real providers retain previous mutations.
Their callers must supply valid native ranges, including indices and aliased
source pointers across callbacks. Arbitrary mutation of native argument spills
and asynchronous memory races are outside the source contract.

## Storage and mutation order

The existing A93DA0 owner writes action base/count/capacity at04/08/0C,
context-value base/count/capacity at10/14/18, context-enabled byte1 at1C and
active value1 at20. A93940 constructs each30h action: registered00, enabled01,
context header04, binding header10, previous value/latch1C/20, current24/28 and
retained listener2C. A93500 and697220 already produce the actual binding and
modifier records. A92B70 allocates the actual24h listener, profileD5B610,
reference count1, twelve flag bytes08..13 and four floats14..20.

A93C80 compares the action index against the owner count **unsigned**. A resize
uses index+1 with native DWORD wrap and the existing signed-count provider.
It captures the selected action only after resizing, sets registered=1, and
optionally calls A92B70 for **any nonzero low byte**. The captured action remains
the receiver after listener release and all later allocations.

The source context header is read after listener replacement. A92E70 performs
the real clear/reserve/append assignment; self-alias therefore clears the source
count. A93C80 then reads the source header again and scans its original current
pointer against reloaded endpoints. It skips exactly-1. Other values use the
native signed comparison against the owner's current context count. A growth
captures the old count and new bound, invokes86A430, and explicitly zeros that
interval while reloading the owner base for every store. This redundant zeroing
is retained even though the existing resize initializes normal new entries.
The source count and base are reread after each iteration, in that order, before
incrementing and comparing the captured current pointer. Finally, owner byte1C
and word20 are loaded afresh before context activation.

A92D40 captures the action's context start before clearing enabled. Its scan
has a captured endpoint; -1 enables unconditionally, otherwise a nonzero enable
byte and an equal owner context value enable it. The disabled arm captures
listener2C, clears the action's four value/latch fields, and clears listener
flags and timers in native store order. The mask calculation followed by
`TEST ECX,F8BC00` atA92DA7 tests whether the listener pointer is nonzero; it does
**not** load the global atF8BC00.

The enabled arm calls the real raw A92370 poll, then reloads and captures
listener2C. It computes current-down before previous-down, using the exact
COMISS/JBE test against SSE zero only when each latch is nonzero. Unordered
values do not pass, and the COMISS exception behavior is retained. It supplies
FLD1-spilled delta1 to real A91A50, then performs FLD current value, reads the
current latch byte, FSTP previous value, and writes the previous latch byte.
No listener reference is acquired or released by A92D40.

## Direct providers and caller contracts

| Site | Callee | Receiver/arguments and source |
| --- | --- | --- |
| A93C95 | A93C10 | owner+4, signed index+1, RET4; native_input_action_records |
| A93CB3 | A92B70 | captured action, no stack input; native_input_action_listener_owner |
| A93CC0 | A92E70 | action+4, borrowed source header, RET4/EAX destination; native_input_binding_storage |
| A93CED | 86A430 | owner+10, signed context+1, RET4; native_input_binding_storage |
| A93D3F | A92D40 | captured action, owner+10, reloaded byte1C/word20, RET0Ch; this packet |
| A92E03 | A92370 | captured action, no stack input; native_input_action_binding_runtime |
| A92E54 | A91A50 | listener captured after poll; delta1, previous/current low bytes, RET0Ch; native_input_action_tick |

Both incoming A93C80 sites belong to698A10. At699258 the stack has the current
Lua-derived action id, actual local context header and the `press` low byte.
At699A31 it has index128h, a local header containing context1Eh, and zero for
listener replacement. The receiver comes from lazy004BEC00 at69903A, captured
in EDI and its stack spill, then restored after nested iteration. The other
A92D40 caller, A9304C within A93020, passes each actual30h row plus owner+10,
fresh context-enabled byte and fresh active value; the loop reloads the action
array endpoint afterward. The report includes every listed CALL and these
incoming sites, rather than treating the decompiler's unused EDX as an input.

## Remaining 698A10 boundary

OnInitOnce calls698A10 at4DD745 only when game+55C is zero and passes game+3C.
The first call698A2F enters698730, which scans action ids0..128, unregisters
existing entries and clears multiple actual configuration containers. That
normal cleanup and its truncated free fallthrough remain unresolved here.
698A36 then tests actual configuration byte4C8; the first load opens the
embedded native Lua owner with mask1. It executes `X360COMP=true/false`, runs
`Scripts\datatables\Inputs.lua`, sets4C8, gets globals, and parses modifier,
group, input and helper records. The real application VFS/Lua services and
typed InputScriptStartup exist, but the latter is a distinct input-settings
projection and cannot be cast to this configuration or native Lua owner.

Ghidra currently ends the698A10 body at699AD4, after the CALL to returning free
at699AD0. Disk decoding and matching live bytes establish the excluded range
699AD5..699B7D (exclusive699B7E), including:

- 699AD8 lazy004BEC00; 699ADF rebindA922A0.
- 699AEA fresh lazy004BEC00; 699AF1 A92C40 with x87-spilled zero delta.
- 699B01 settings getter5547D0 and699B08 apply6AB820 on the non-X360 arm.
- Four Lua-object destructor calls, byte configuration+520=1, EH restoration,
  ADD ESP,164h and plain RET699B7D.

The sampled final-tail CALL sites have no Ghidra containing function and are
recorded separately with their bytes; the worker did not widen/repair any body.
The export also lists free CALL69982B outside the actual Ghidra body; live proto
and bytesE859D15500 confirm that it too needs separate raw qualification.
Internal free-related omissions remain in this large routine. Its body-wide CALL
inventory is mechanical evidence only, not a claim that every parser/callee or
EH state was reconstructed. The exact raw configuration constructor/storage
owner,698730 cleanup, schema-to-actual bindings and complete normal/EH Lua
cleanup must precede an application698A10 implementation. No successful wrapper
or callback stub hides these requirements.

## Validation

Win32 Release builds with MSVC, eight original seeds match, and both existing
CTests pass. One ignored manifested native/source differential copies the two
complete native spans, verifies live bytes against the installed image and
relocates seven direct CALL operands; branch instructions remain unchanged.
Both paths use the real recovered storage, listener allocation/deletion, raw
binding poll and listener tick providers.

The four phases cover action-array growth, listener release mutating the context
source and owner state, context-vector growth, nonempty polling, disabled flag
and timer clearing, and context-header self-alias. Both paths have three device
calls, action count3, context count5 and active value bits3F000000. The device
callbacks are explicitly controlled fixture services. No SDK, window, hardware,
force, cursor or game run occurred. No permanent tests were added.

Runner: `local/run_native_action_configuration_probe_ag.ps1 -PrimaryWorktree <tree>`.
It compiles only the ignored fixture against that tree's headers and archive.
Reports distinguish the two complete producers from the partial caller evidence.
