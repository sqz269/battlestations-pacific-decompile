# Actual scene-command record construction

Addresses: `004690D0`, `00C61A70`, `00C61A7B`.

This packet adds whole ordinary C++ construction against actual storage and
qualified explicit-cell mappings of the two cleanup actions. The existing
`SceneCommandRecord` / `std::string` implementation in `scene_deferred_refs.cpp`
is a semantic projection; it does not implement these raw headers, live input
cells, current saved-record cleanup or publication order. It remains available
to its existing consumers.

| Entry | Coverage | Original input/result |
|---|---|---|
| 004690D0 | Complete 209 listed bytes / 85 instructions | ECX record20; owner, command, target stack DWORDs; RET0Ch; EAX original record |
| 00C61A70 | Qualified whole 11B / 3 instructions | Current saved record at native EBP-10h; add4; tail destruction |
| 00C61A7B | Qualified whole 11B / 3 instructions | Independently reload current saved record; add0Ch; tail destruction |

The constructor occupies `[004690D0,0046915D)` and
`[00469160,004691A4)`. Its 212-byte envelope includes three unreachable bytes
between those ranges; those bytes receive no executed-instruction credit.
Both action spans are contiguous. All 234 envelope bytes match live Ghidra and
the installed PE; the reconstructed listed spans total 231 bytes / 91
instructions. The native EBX, EBP, ESI and EDI values are preserved. These facts
do not establish a drop-in C++ binary interface.

## Actual storage and read order

`NativeSceneCommandRecordConstructFrame` binds live `actual_parameters12` and
writable `actual_saved_record4`. At entry ESP=P the native ranges are
P+4..P+0F and P-10..P-0D respectively. They are distinct native stack ranges.
Source binding addresses stay stable; frame/context metadata are disjoint from
payload and actual backing cells. Payload aliases with record20, arguments and
the saved word remain admitted when every reached load/store and the existing
raw providers are valid. No prezeroed record is required. No compiler/native,
child/helper argument-home, return, spill, register-save, EH or asynchronous
stack alias identity is inferred from the new ordinary interface.

1. Capture the ECX record value. Both the native PUSH ECX and explicit saved
   record MOV write that value into the saved word before the header stores.
2. Write zero to record+4 and +8, arm state0, then zero record+0Ch and +10h.
   These are two actual8h length/data headers, not owning host strings.
3. Capture the command DWORD from the current second argument **before** reading
   the owner DWORD from the first argument. Arm state1, then write owner at +0.
4. Null command measures zero. Otherwise walk current bytes one at a time,
   including the terminating byte; cursor and final pointer difference use
   DWORD wrap. There is no `strlen` snapshot or string copy before the walk.
5. Use complete raw resize with preserve0 on record+4. After it returns, read
   current data at +8. Only when nonnull read current length at +4, then invoke
   the genuine current SDK copy boundary with the retained command pointer.
   The count excludes the terminator already handled by the resize provider.
6. Only after that whole resize/copy sequence read the current third argument.
   Retain that target pointer through its own byte walk, raw preserve0 resize
   at record+0Ch, current data at +10h and conditional current length at +0Ch.
7. Return the original captured record pointer. There is no normal member
   cleanup, global empty-string read, mapped owner acquisition or extra guard.

The actual native copy entries are BF7680. The Source-only tail boundary jumps
to genuine current SDK `memmove`, with a nonintrinsic call even for reached
zero counts or a null source. It preserves overlap and current zero-count
service behavior. It is not a new native provider or a claimed native CALL to
`memmove`. The complete raw resize/destructor and raw pool context are reused
from `native_string.hpp/.cpp`; their existing storage and provider limits are
inherited. No callback, allocator simulation or unresolved stub is added.

## Native transfers and caller

| Verified listing | Actual containing function |
|---|---|
| `00469135 CALL 0041DD40` | 004690D0 |
| `00469146 CALL 00BF7680` | 004690D0 |
| `00469174 CALL 0041DD40` | 004690D0 |
| `00469185 CALL 00BF7680` | 004690D0 |
| `00C61A76 JMP 0041DD20` | 00C61A70 |
| `00C61A81 JMP 0041DD20` | 00C61A7B |
| `00469654 CALL 004690D0` | 00469610 |

The two action JMPs have default unconditional-jump flow and the existing
CALL_RETURN override; effective flow is CALL_TERMINATOR. This packet does not
change that analysis. All other listed transfers are ordinary direct calls
with no override. The sole current incoming call was checked within the full
115B / 34-instruction queue wrapper: allocation14h precedes every input read,
then current target, command and owner are loaded in that order and pushed
target/command/owner before the constructor's RET0Ch. The wrapper is caller
evidence only; its Source is not included in this packet. Its remaining raw
pushback dependency is separate work.

## Cleanup state and limits

Handler `00C61A86` selects FuncInfo `00D8909C`: magic19930522,
maxstate2, unwind map `00D8908C`. State0 goes to -1 through the command action;
state1 goes to0 through the target action. The body arms state0 between the
header initializations and state1 after the command/owner reads, before owner
publication. It never performs a normal member cleanup.

The ordinary Source guard advances its state before each action. On a body
exception it first invokes target cleanup, whose saved-record load is current;
after a returning target cleanup it independently rereads the saved word for
command cleanup. If pool activity changes that cell, the second header base
changes accordingly. Neither action caches the constructor's original record.
Each raw destructor captures its current header data, conditionally reads
length+1 with DWORD wrap, and resolves the actual current raw pool for release.
The public action interfaces also retain their direct ordinary-call failure
propagation. The guard is `noexcept(false)`; secondary failures during true C++
unwinding use the current compiler/runtime rules. Native FH3/SEH, secondary
exception transport, faults, private frame, CRT/heap identity and game execution
remain unproved. No Original action-entry or private-handler ABI credit is
assigned to these ordinary explicit-cell functions.

## Validation

Whole native listings, producer layout, real callee Source, sole current
incoming setup/continuation, typed body/transfer metadata and live/installed
bytes are checked. Existing descriptive and compiler-unwind names are retained
with prior evidence/comments; locked annotation/save/export/readback is recorded
in the report. Source/header and provider hashes identify this draft.

The primary independently reviewed the whole native body, both actions and
complete Source/header; no implementation mismatch was found. A later header
comment only clarifies stable binding addresses versus disjoint metadata.
The actual primary normal `scripts/build.ps1` passed at
`efa7febae744d66c5665e61b64bf8c67d36ff3f6`: MSVC Release Win32, all three
enabled existing CTests, zero failures, 9.55 seconds. No duplicate worker build
was run. Registration is unique, with this Source once in the generated project
and one identical archive body. Reviewed Source/header and all six concrete
provider hashes match the own tree, primary tree and tested revision. The exact
immutable primary receipt/log hashes are retained in this report. The build
retains the unrelated Lua `spawn_request_id_matches` LNK4006 duplicate-symbol
warning; no packet compilation warning occurred.

The exact retained object is 5,841 bytes, SHA256
`93de29c50ab2fd180d410ec428b916733aec87d96e2b6d22c1328d066a4265be`.
Full constructor/helper/action and compiler-unwind disassembly was inspected.
Both saved stores are at `+45/+47`, current command/owner reads at `+63/+66`,
actual guard state1 before owner store at `+72/+79`, raw resize at `+96/+D5`
and genuine SDK boundary at `+AC/+E9`. Current data and conditional length are
read after each resize; the target slot is not read until `+B7`, after the
first copy. The captured original record returns through EAX at `+F1`.
The SDK tail resolves to external `_memmove` without an extra count/null guard.

The emitted unwind funclet dispatches to the real ordinary guard destructor.
It advances state0 before reading current saved-record at `+17`, adds0Ch and
destroys target at `+1D`; after return it advances -1 and independently reloads
current saved-record at `+38`, adds4 and destroys command at `+3E`. Standalone
ordinary action functions each retain their own current saved-word load before
the raw destructor tail. The compiler removes the Source guard's earlier state0
assignment across nonthrowing header stores and arms emitted C++ EH before the
real providers. This gives no native header-fault, exact-state/flags/register,
private-frame, FH3 or SEH equivalence. Constructor SourceReady applies to the
whole ordinary current-provider body; the two native action mappings remain
separately qualified.

No Source, header, registration or tests changed for this validation update.
The existing native math seeds do not execute this packet. Build and static
evidence establish neither packet-specific native execution nor original ABI,
result ownership/lifetime, private heap or game validation.
