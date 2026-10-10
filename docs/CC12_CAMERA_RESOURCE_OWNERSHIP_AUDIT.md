# Camera-resource ownership and listing audit

The three camera stores belong to one **observed local instruction sequence**
at `00CD8390..00CD83DE`, but Ghidra records no function at or containing any of
its 16 instruction starts. `00CD8380` is not an entry: it is inside the instruction
at `00CD837F`, within Bone's recorded body. No complete initializer, original ABI,
CRT owner, or production Source implementation is admitted by this audit.

This two-file packet follows `CC12_CAMERA_RESOURCE_WRITER_DISCOVERY.md` and
the typed-flow runtime review at baseline `b6fc9f628`. Root remains the sole
Ghidra writer. There were no POST requests, scripts, configuration changes,
restarts, GPR writes, C++/CMake changes, or ledger edits.

## Inspection and verification scope

Root initially authorized exactly `00CD8380..00CD83EF` inclusive: **112 original-image
bytes**, with matching current Ghidra bytes and existing listing metadata. This
is an inspection window, not a function extent. The lease includes that range,
the requested stores/candidate and `01090288`; no bytes at the latter data cell
were opened. Root subsequently approved only `00CE35BC..00CE35BF`, **four more
bytes**, after the incoming reference was identified. The lease was refreshed
for that pointer and candidate `00CD8390` before this extension was opened.

Six named `ghidra typed-flow` captures retained 42 full raw HTTP responses.
All pass schema, explicit null-state, complete AddressSet, query, actual
`C:/Users/sqz269/bsp.gpr` marker, program, language, image-base, and before/after
identity checks. Every response and batch reports modification **3**. The raw
receipts were replayed through the pure validator when assembling the audit.
Package-loaded class CodeSource remains unattested; that qualification is
separate from the verified runtime getter contract.

The installed original PE still has 12,223,752 bytes and SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The selected 112 bytes equal the saved Ghidra memory bytes byte-for-byte;
window SHA-256 is
`12a741915ff4588bae13cebe404304ff217f88151fc05947dcac4e9678be283e`.
This is analysis-program memory evidence, not live game execution.

Typed metadata proves `00CD8380` is the interior of a three-byte instruction
starting at `00CD837F`. Its first byte is outside scope and was not opened.
The first complete decoded start in scope is `00CD8382`; Capstone therefore
decodes only the remaining **110 bytes**. Across these, 24 existing instruction
starts have matching original-byte lengths. The six `CC` bytes at `00CD838F`,
`00CD83DF`, and `00CD83EC..00CD83EF` are currently undefined Data code units,
with explicit null instruction/flow states; their disk `INT3` decoding was not
promoted into the Ghidra listing.

No whole-function listing call was used, because it could fetch bytes outside
scope. The POST-based instruction-context helper was also not used. Listing
agreement here means exact typed instruction starts/lengths plus identical
original/Ghidra bytes; it is not a retained Ghidra assembly-text comparison.

## Boundaries and function ownership

| In-window interval | Evidence and limit |
| --- | --- |
| `00CD8380..00CD8381` | Suffix of instruction `00CD837F`; not a new decode origin |
| `00CD8382..00CD838E` | Four existing instructions ending in `RET`, inside recorded Bone function |
| `00CD838F` | Undefined `CC` byte |
| `00CD8390..00CD83DE` | 16 contiguous existing instructions, 79 bytes, no recorded owner; local guard path ends in `RET` |
| `00CD83DF` | Undefined `CC` byte |
| `00CD83E0..00CD83EB` | Separate four-instruction push/call/pop/return sequence, also without recorded owner |
| `00CD83EC..00CD83EF` | Four undefined `CC` bytes |

The complete returned Bone AddressSet is exactly `00CD8340..00CD838E`, one
79-byte range; its current flags are NoReturn=false and non-thunk. This is
stored ownership metadata, not a new inspection of the rest of Bone's body.
It excludes all three camera stores. No function record was synthesized from
the nearest symbol, alignment, or matching size.

A bounded GET xref query at `00CD8390` returns one `DATA` reference from
`00CE35BC`. The approved four-byte extension confirms original and Ghidra bytes
`90 83 CD 00`, the little-endian pointer value `00CD8390`. Its SHA-256 is
`de87dc4f708dc813162d4510183fd488bff07e08a841e3555e09615bee2ab85d`.
Only this selected pointer was opened; the surrounding table was not. The
pointer does not establish a CRT slot, callable entry ABI, execution order, or
production caller. With no function record,
function-based caller ownership remains unavailable rather than proven empty.

## Actual register origins and local control flow

The original bytes and typed effective flow agree on the following sequence.
Statements about the later stores apply to the path where both calls return.

| Instruction(s) | Direct observation |
| --- | --- |
| `00CD8390`, `00CD8397` | Compare byte `[01090266]` with zero; nonzero branches directly to `RET` at `00CD83DE` |
| `00CD8399` | Set ECX to immediate address `01090210` |
| `00CD839E` | Store byte value 1 to `01090266` |
| `00CD83A5` | Store literal `00D6327C` to dword cell `01090294` |
| `00CD83AF` | Call `00B869C0` with that observed ECX value; next local instruction is `00CD83B4` |
| `00CD83B4`, `00CD83BF` | Load EAX from `[01090210]`, then store EAX to `0109028C` |
| `00CD83B9`, `00CD83C4` | Load ECX from `[01090214]`, then store this ECX to `01090290`; no intervening call |
| `00CD83CA` | Call `006FAC20`; next local instruction is `00CD83CF` |
| `00CD83CF`, `00CD83D2` | Load ECX from `[EAX+4]` using post-call EAX; compute EDX=ECX+1 modulo 32 bits |
| `00CD83D5`, `00CD83D8` | Store EDX back to `[EAX+4]`, then store the old ECX value to `01090288` |
| `00CD83DE` | Plain `RET`, without an immediate stack-cleanup operand |

The two ECX stores therefore have distinct origins. `00CD83C4` copies a dword
from the absolute cell `01090214`; `00CD83D8` records the preincrement dword at
post-call EAX plus four. The uninspected callee may change EAX; its production
of, or obligations concerning, that value are not proved here. The first ECX value is not merely carried
through the second call. The guard and literal writes precede the first call;
the two copy stores precede the second call; the pointed-to increment precedes
the final camera-cell store. No local rollback path appears in this region.
Exceptions, child failures, and nonreturning execution are not established by
these caller bytes.

Both calls have FlowOverride=NONE, equal default/effective unconditional-call
flow, and equal non-overridden fallthroughs. Their exact target functions are
non-thunks with NoReturn=false. The current descriptive labels are
`BSP_InitializeSceneResource_00b869c0` and `BSP_TypeId_GetCounterSingleton`;
their names do not substitute for uninspected body/ABI contracts. All 24
in-window decoded instructions have no flow or fallthrough override.

The following sequence at `00CD83E0` pushes `00CE0EE0`, calls the function
currently named `_atexit` at `00BF6FF5`, pops ECX, and returns at `00CD83EB`.
Its call also has ordinary flow, NoReturn=false and non-thunk target metadata.
The preceding `RET` and undefined `CC` separate it from the local camera path.
Neither the callback body nor the callee body was opened, and no registration
or camera-destruction ownership is inferred solely from this adjacency.

## Remaining gate and next scoped evidence

The bytes establish the guard operation, four store targets and their order,
but not a complete descriptor layout or the pointed-to literal's meaning.
Guard/name storage, parent/counter bodies, stack/register ABI obligations,
caller order, and any CRT table remain unopened. No local stack frame or explicit
argument pushes occur in the 16 camera instructions; that alone cannot recover
their original function signature or the two callees' effects.

The approved four-byte pointer check is complete. A useful next request is a
**metadata-only xref query at `00CE35BC`** to seek its actual consumer, and exact
body-range metadata at **`00B869C0` and `006FAC20`** before proposing any child
instruction windows. These further queries were not run. The current pointer
alone supplies no evidence for selecting a wider CRT-table interval; do not
expand by adjacency. Any child, data, callback, or broader physical inspection
still needs its own exact range and reason first.

The audit stops with a fully observed local sequence and unresolved genuine
function entry/ownership contracts. No function creation, listing repair,
Source implementation, production backing, build, or gameplay claim follows.
