# CC12 game-state queue failure handler readiness

The registered selector `00C65D08` and its directly selected metadata/action
now have a bounded, read-only review. The selector is 10 bytes; the selected
cleanup action is 8 bytes; the selected FuncInfo and one unwind row total
44 data bytes. All four exact spans match live bytes and the retained
original PE. **No dormant C++ GO, production queue binding or portable throw
equivalence is approved.**

The fresh evidence establishes the selected cleanup route and its receiver
formula, `ECX = u32(EBP - 50h)`. Identifying that receiver with the caller's
temporary requires the runtime relation **`EBP = caller entry ESP`**. Neither
the selector nor its cleanup action establishes that relation. The generic
runtime target and string-destructor target remain unopened.

The Source baseline is worktree commit
`c6608150334e8cd9f67a9f81f7df9a0b284b8228`, created by merging current main
without changing earlier frozen evidence. The complete 4,206-file Source tree
and all thirteen accepted overlays are unchanged from the preceding failure
review. Root subsequently accepted that failure worker
`5de62af8f519e771ef5bc7ce1ffd7e8498341ef7` in commit
`3129866ef489aadf9a2e90ddcfec228e6a3ecdbd`, with status
`accepted_complete_body_readiness_only_no_production_binding`. Its whole
primary document/report are separately pinned. This acceptance supersedes
the immutable capture's earlier pending status without rewriting the capture.

## Scope and exact selector

The initial `00C65D08` prototype query reported no function definition.
Existing listing instructions were nevertheless available at `00C65D08` and
`00C65D0D`. Their exact sizes and live/PE byte match establish the following
10-byte extent, below the requested 64-byte selector gate:

| Address | Instruction | Local effect |
| --- | --- | --- |
| `00C65D08` | `MOV EAX,00D8E774` | Publish the selected metadata address in EAX |
| `00C65D0D` | `JMP 00BF6B43` | Tail-transfer to the unopened runtime target |

There is no local CALL, RET, stack adjustment, memory load/store or flag
change. ECX, EDX, EBX, ESI, EDI, EBP and ESP are locally unchanged. That says
nothing about downstream preservation or runtime behavior. The selector is
registered through exception data; it has no established ordinary portable
function signature. EAX is the observed metadata carrier into the tail target.

The live xrefs are the retained caller's handler-address push at `004CFE22`
and a data reference at `00D7D780`. The latter's source bytes were not read.
No function creation, instruction repair, rename, comment mutation or project
save was performed to change the missing prototype metadata.

The exact selector and selected FuncInfo ranges were leased before metadata
inspection. The subsequently selected eight-byte unwind row received its own
lease extension before reading. The row identified cleanup `00C65D00`; its
prototype metadata bounded it to `00C65D00..00C65D07`. Root was notified of
that exact eight-byte extent before its body was read and explicitly approved
the single action. Its fresh address/range lease preceded the body read.
No other Native provider body or out-of-scope continuation was opened.

## Directly selected metadata

The selector's immediate addresses the 36-byte record at
`00D8E774..00D8E797`. The retained raw DWORDs are interpreted using the
nine-DWORD x86 MSVC FuncInfo layout:

| Offset | Interpreted field | Raw DWORD |
| --- | --- | --- |
| `00h` | magicNumber | `19930522` |
| `04h` | maxState | `00000001` |
| `08h` | pUnwindMap | `00D8E76C` |
| `0Ch` | nTryBlocks | `00000000` |
| `10h` | pTryBlockMap | `00000000` |
| `14h` | nIPMapEntries | `00000000` |
| `18h` | pIPtoStateMap | `00000000` |
| `1Ch` | pESTypeList | `00000000` |
| `20h` | EHFlags | `00000001` |

Raw values and offsets are independently pinned and replayed. The final
flags word is retained literally; this review derives no hardware-exception,
dispatch or portable C++ policy from it. The zero fields select no local
catch map, IP map or exception-specification type list under that layout.
No corresponding external table is fetched merely to interpret a zero field.

`maxState=1` and `pUnwindMap=00D8E76C` select exactly one eight-byte row,
`00D8E76C..00D8E773`:

| Row | Signed successor state | Action address |
| --- | ---: | --- |
| 0 | -1 | `00C65D00` |

The table therefore encodes the state-0 cleanup route and successor `-1`.
There is no second selected row for a constructed exception owner. These are
metadata facts under the stated layout; the unopened runtime controls whether
and how the action is invoked and how unwinding progresses.

## Selected action and the frame relation

The selected action is exactly two instructions and eight bytes, below the
requested 32-byte cleanup gate:

| Address | Instruction | Local effect |
| --- | --- | --- |
| `00C65D00` | `LEA ECX,[EBP-50h]` | Form the receiver address without reading it |
| `00C65D03` | `JMP 004072D0` | Tail-transfer to the unopened string destroy target |

Only ECX is locally written. EBP is read; EAX, EDX, EBX, ESI, EDI, EBP, ESP
and flags are locally unchanged. LEA performs 32-bit address arithmetic and
does not load or validate the receiver. ESP is unchanged and the jump creates
no new return address. Any return from the destructor depends on the stack
context prepared by the runtime; this action supplies no independent normal
return or stack-cleanup contract.

The accepted, whole `004CFE20` failure review is the caller-layout dependency.
Its 105 body bytes are retained and checked for identity only; they are not
freshly decoded in this packet. With `S` denoting caller entry ESP, that
review establishes these locations under its retained preparation-provider
cleanup contracts:

| Item | Caller-relative address |
| --- | --- |
| Completed SBO-string temporary | `S-50h` |
| Local exception owner | `S-34h` |
| Registered exception-chain link | `R = S-0Ch` |
| This action's computed receiver | `EBP-50h` |

The action receives the completed temporary precisely when
`u32(EBP-50h) == u32(S-50h)`, which requires `EBP == S` modulo 2^32.
Equivalently, runtime EBP must equal `R+0Ch`. Setting EBP equal to the
registration link `R` alone would instead produce `S-5Ch`, which is a
different address. The retained caller does not itself establish this
runtime EBP rebasing, and the selector/action do not establish it either.

Under the explicit `EBP=S` condition, the selected receiver is the temporary,
not the exception owner, which lies `1Ch` bytes above it. This conditional
mapping is the extent of the caller-cleanup identification established here.
It is not a proof of the generic frame-handler register ABI or actual unwind
execution.

## Retained state order and current Source reuse

The accepted caller report establishes state `-1` during counted assignment
`00408720`. After that provider returns, the caller publishes state `0`
before invoking logic-error construction `00411700`; state remains `0` at
the later external throw call. That order is consistent with selecting the
completed temporary cleanup when the runtime dispatches row 0 with EBP=S.
This packet adds the selected row and receiver formula; it does not silently
promote the runtime assumptions into fresh evidence.

`include/bsp/native_legacy_sbo_string.hpp:41` already declares
`native_legacy_sbo_string_destroy_004072d0`.
`src/native_legacy_sbo_string.cpp:56` implements the existing Source cleanup:
when capacity is at least 16, free `buffer_04.heap` through
`singleton_lifetime_free`; then set capacity to 15, length to 0 and the first
inline byte to zero. It does not additionally clear the preserved leading
DWORD or unused inline bytes. This is a concrete reuse candidate for the
conditionally identified temporary receiver, with no duplicate string owner
or new provider implementation needed.

`src/native_legacy_exception_owner.cpp:69` defines a different owner-wide
destruction path: publish the logic-error vtable, destroy `owner.message_0c`,
then destroy its base. That is not the selected direct temporary receiver.
The existing constructor's base-only guard during failed member copying is
also a different cleanup scope. Neither an owner-wide destructor nor an
extra completed-owner registration is added to this caller by inference.

The complete current Source census has 51 matches for the selector/cleanup
query, identifying existing string-destroy reuse sites. None contains
`00C65D08`, `00C65D00`, `00D8E774`, `00D8E76C` or `00BF6B43`; no existing
Source implementation of this runtime selector or frame rebasing is thereby
established. All other retained game/profile/queue searches and owner/lifetime
checks are replayed against the same complete Source tree.

The accepted allocator and growth reports remain direct whole-Git dependencies.
Root's queue coverage correction still qualifies the existing enqueue/growth
helpers as semantic fragments whose general raw provider, ordering and owner
binding is open. This packet does not change queue activation, automatic
adapters, current/captured profile ownership, cancellation, deferred execution
or shutdown lifetime. The thirteen accepted Source overlays and both newer
startup/typed-model manifests are unchanged. The historical 70-of-80
application-object census qualification remains in force; this packet earns
no build, fixture or unchanged-object comparison credit.

A host scope guard can model a chosen temporary-cleanup sequence. It would
not establish the observed EH register context, native metadata consumption,
native catch matching, throw-provider ownership or exception-owner destruction.
The selected temporary row does not close any of those boundaries. No host
throw shortcut, dormant adapter, generic runtime shim or production binding
is approved by this review.

## Evidence and reproduction

The report is
`reports/cc12_game_state_queue_failure_handler_readiness.json`. Preserve that
report and the complete relative tree
`local/cc12_queue_failure_handler_selector_readiness/` for standalone replay.
The evidence requires no earlier packet directories, game installation,
Ghidra or Git in portable mode.

Nine retained typed CLI queries cover selector prototype/xrefs/bytes/exact
listing context, the selected FuncInfo and unwind-row bytes, and the selected
action prototype/bytes/complete listing. Each uses the client verification
for configured `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 and image base `00400000`; autostart is
explicitly disabled. The configured GPR path is checked configuration, not a
separately exposed server path. Only the four authorized spans are fetched in
the captured Native batch.

The complete original PE, baseline Source Git/working archives, current Source
tree, thirteen overlays and accepted manifests are pinned. The selected Source
closure has 401 files, 26 roots and 734 quoted include edges. The separate
typed-model-header closure has 40 files and 57 edges. Thirteen complete-corpus
queries retain 1,155 matches; 42 Source excerpts and 13 documentation excerpts
are recomputed exactly. Sixteen whole dependency Git files, twenty support
Git files and two separately accepted Root Git files are retained directly.
Prior Root proof trees and unrelated nested artifact paths are not traversed.

The fresh Native replay decodes all four instructions, checks their register,
memory/address-only, flag and stack effects, independently checks the relative
jump targets, parses every selected metadata DWORD and the signed unwind row,
and recomputes the conditional frame equation. The 18 code bytes and 44 data
bytes must match the live captures and original PE. The retained 105-byte
caller is checked only for identity against the accepted prior report and PE.

Run `python local/cc12_queue_failure_handler_selector_readiness/verify.py`
for the guarded artifact-only replay. It rejects file reads outside the one
retained root/report, and rejects writes, subprocesses, sockets, dynamic
library loading and filesystem mutations after guard installation. The audit
helper is executed with its final write/print statements removed; the complete
result must equal the retained audit JSON.

Run the separate `--original-worktree` mode to verify all actual Git blobs,
all 4,206 current working Source files, accepted Root ancestry/report identity
and the exact two owned output paths. The physical relocation helper copies
the pinned artifacts and exact report into a fresh directory and runs the
same guarded verifier there. These are evidence checks, not application tests
or Native execution.

The remaining decision is whether to retain the runtime frame relation as an
explicit external contract or commission a separate bounded runtime-entry
review. This packet does not invent or lease a further Native range. Generic
EH setup, actual unwind invocation, native throw ownership and general raw
queue binding remain open. No C++, compiler, new test, fixture, Native runtime,
SDK/OS/game operation or Ghidra mutation occurred.
