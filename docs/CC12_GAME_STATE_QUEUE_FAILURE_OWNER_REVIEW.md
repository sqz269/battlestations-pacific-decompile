# CC12 game-state queue failure owner review

`004CFE20` has a complete, read-only body review. Its 105 bytes at
`004CFE20..004CFE88` decode to 23 instructions, matching all 23 live listing
rows and the retained original executable. It constructs a local string and
logic-error owner, changes the owner's first word, and calls the external
throw provider. **No C++ GO or production queue binding is approved.** The
unopened registered handler `00C65D08` is the smallest caller-specific next
frontier: the body establishes a state transition but does not establish the
resulting unwind cleanup.

Only this function was freshly analyzed. The three called providers, handler,
message bytes, vtable, exception metadata, cleanup funclets and continuation
were not opened. No C++, build, fixture, Native execution, SDK/OS/game action,
Ghidra annotation, listing repair or project save occurred. Existing library
and reconstruction names are retained evidence labels, not newly recovered
symbols or proof of exception interoperability.

The review is based on worktree commit
`a91ae29f05582e4430d82ba0637699a4e7405136`. Its complete 4,206-file `src/` and
`include/` tree is unchanged from the allocator review. The immutable capture
initially recorded that allocator worker as awaiting Root acceptance. Separate
whole Git artifacts now pin Root commit
`b7afc77af12e2f3d9a3667820e963b55eac533d7`, accepting worker
`edfc4323125f15ce48daafe25149dd30b2117088` with status
`accepted_readiness_only_no_production_binding`. That acceptance supersedes
the capture's scheduling status; it does not rewrite the frozen evidence or
change this packet's Source epoch.

## Exact local behavior and stack frame

Let `S` be entry ESP. `PUSH -1` at `004CFE20` creates a state word at `S-4`;
`PUSH 00C65D08` at `004CFE22` places the literal handler entry at `S-8`.
`004CFE27` reads `FS:[0]`, `004CFE2D` pushes the old link at `S-12`, and
`004CFE2E` publishes `S-12` to `FS:[0]`. `SUB ESP,44h` at `004CFE35` then
reserves 68 local bytes, leaving ESP at `S-80`. No local restoration of that
registration link appears in the body. Provider changes to thread exception
state remain outside the local instruction proof.

Using the explicitly retained `RET8` and `RET4` preparation-provider contracts,
the raw local types fit that reservation exactly:

| Entry-relative extent | Size | Retained Source interpretation |
| --- | ---: | --- |
| `S-80..S-53` | `1Ch` | `NativeLegacySboStringStorage` temporary |
| `S-52..S-13` | `28h` | `NativeLegacyExceptionStorage` owner |
| `S-12..S-1` | three DWORDs | previous link, handler, state |

These type sizes come from the frozen Source, not from a fresh Native provider
inspection. The two local extents total `44h` and do not overlap the observed
registration record.

`004CFE38` pushes count 17 and `004CFE3A` pushes literal address `00CE78A0`.
After those pushes, `LEA ECX,[ESP+8]` sets the string receiver to `S-80`.
The body initializes precisely these fields, in this order:

| Instruction | Effective address | Width | Value |
| --- | --- | ---: | ---: |
| `004CFE43` | `S-56`, string `+18h` | DWORD | 15 |
| `004CFE4B` | `S-60`, string `+14h` | DWORD | 0 |
| `004CFE53` | `S-76`, string `+04h` | byte | 0 |

The leading DWORD and the unused inline bytes are not locally initialized.
Aggregate zero initialization would add behavior absent from this body.
Pseudocode renders the literal as `deque<T> too long`; only the immediate
address and count are independently established here. The literal's storage
bytes were not read as a separate Native data range.

At `004CFE58`, the routine calls `00408720`
(`BSP_CharacterString_AssignCounted`) with ECX as destination and the literal
pointer/count on the stack. Its existing Source/documentation contract is
`RET8`, returning the destination in EAX. On that contract, normal provider
return leaves ESP at `S-80`.

`004CFE5D` forms the completed temporary's address, `004CFE60` pushes it,
and `004CFE61` sets ECX to owner `S-52`. **The state word becomes 0 at
`004CFE65`, before the logic-error construction call at `004CFE6D`.**
That call targets `00411700` (`BSP_NativeLegacyLogicError_Construct`), whose
retained contract is ECX owner, one source-string stack argument, `RET4`, and
EAX owner. Its normal return restores ESP to `S-80` under that contract.

`004CFE72` then pushes literal metadata address `00D83F98`; `004CFE77`
forms the same local owner pointer and `004CFE7B` pushes it. At `004CFE7C`,
the body overwrites the owner's first DWORD with literal `00D69260`.
This publication follows successful logic-owner construction. Frozen Source
calls this constant `length_error_vtable`, and pseudocode labels it
`std::length_error::vftable`; the table itself is unopened.

The final instruction is `CALL 00BF6885` at `004CFE84`, with owner first and
metadata second in stack-argument order. Its retained name is
`__CxxThrowException@8`. The body ends at the call's final byte `004CFE88`.
There is no `RET`, branch, local epilogue or returning exit. If the external
throw call returned, execution would continue at `004CFE89`, outside this
packet; no bytes or invented continuation are substituted for that frontier.
Ghidra's nonreturn label is not an independent proof of that provider's
behavior.

## ABI, state and ownership limits

No incoming integer register or stack parameter is consumed as a semantic
argument by the body. EAX is first overwritten by the `FS:[0]` read; ECX is
overwritten with a local address before provider use; EDX is not explicitly
referenced. EBX, ESI, EDI and EBP are not locally referenced. The function
requires incoming stack/thread exception context, but it does not read a
queue receiver, capacity/count registers, current-game publication or any
profile callback slot. No output register, return-value or normal-return
cleanup convention is inferred for this non-returning local body.

The audit records all 23 instructions, six explicit memory-write operands
(thread link, three string fields, state and vtable word), one explicit
memory-read operand (`FS:[0]`), eight pushes, three calls, and the address-only
LEAs. Implicit stack accesses and provider effects are accounted for separately.
There is one straight-line route to an external throw boundary and zero
in-body normal-return paths.

Preparation cleanup sizes are retained contracts, not freshly analyzed callee
ABI. The stack audit also keeps those quantities symbolic. If counted
assignment pops `a` bytes, the subsequent constructor receiver is `S-60+a`
and the state store is `S-12+a`; reaching the registered state at `S-4`
requires `a=8`. If the constructor pops `b`, the later throw owner/vtable
address is `S-64+a+b`; matching the constructor receiver requires `b=4`.
At the throw call, the resulting ESP is `S-100+a+b`, or `S-88` with the
retained contracts. This consistency calculation supports the frame mapping;
it does not independently establish either provider's complete ABI.

The observed state is `-1` during counted assignment, then `0` during
logic-owner construction and the final throw call. The body contains no
additional state transition registering a completed exception owner. The
unopened handler and its FuncInfo/unwind records determine what those states
mean for cleanup. This review does not invent a caller destructor, catch,
rethrow, rollback, frame removal, or ownership transfer solely from a state
number or a library name.

Existing `00411700` Source and documentation describe base-only cleanup if
its member copy fails. That is a callee-local obligation; it does not close
this caller's temporary lifetime. Exception metadata identity, native catch
matching, runtime allocation/copy/destruction, thread unwind and throw-provider
ownership remain unverified Native boundaries.

## Current Source reuse and coverage

`include/bsp/native_legacy_sbo_string.hpp:9` defines the raw `1Ch` string
storage with a preserved first DWORD, 16-byte inline buffer, length at `14h`
and capacity at `18h`. Its trivial storage does not add an automatic
destructor. `src/native_legacy_sbo_string.cpp:162` provides the existing
counted-assignment route. On the ordinary valid-storage path, initial
capacity 15 and count 17 invoke its existing growth provider. This is an
observable allocation/failure boundary before the logic-owner construction.

`include/bsp/native_legacy_exception_owner.hpp:11` defines the raw `28h`
owner; its documented construction contract starts at line 29.
`src/native_legacy_exception_owner.cpp:53` constructs the base, publishes the
logic-error vtable, initializes member string fields in order, and applies a
base-only guard around member copying. The existing derived vtable constant
includes `00D69260`. These two existing raw types and their providers are the
appropriate reuse candidates; no duplicate string or exception owner is
needed for a later bounded adapter.

Host `std::length_error` sites exist in other Source owners, but the complete
frozen searches find no `004CFE20` or `00C65D08` identifier and no deque text
in those host-error matches. Directly throwing a host error would skip the
observed temporary assignment, possible allocation, logic-owner construction,
derived-word publication and their failure order. The existing SBO error
helper's `string too long` path and the documented `004CE780` list-length
factory are distinct routines and do not substitute for this body.

Root's current `CC12_GAME_STATE_QUEUE_SOURCE_COVERAGE` correction explicitly
qualifies the queue growth/enqueue code as compiled semantic fragments whose
general raw Native provider, order and owner binding remains open. The prior
allocator review closes its local count-times-four and `RET0` behavior;
`singleton_lifetime_allocate` remains an existing service candidate. Neither
that acceptance nor this failure review closes the matching raw queue owner,
copy/free providers, or late capacity/map publication contract.

The retained game/menu Source still has the previously reviewed automatic
adapter sites, current/captured profile owners, borrowed implementation,
semantic menu queue and shutdown ordering. This packet changes none of their
activation, queue, cancellation or deferred/reentrant lifetime policies.
No new production call site is approved.

All 4,206 current Source files retain the same thirteen accepted overlays:
three Source3350, six Source3352, two particle-startup files and two typed
particle-model headers. The current typed-model primary pins the unchanged
`Source3352-particle-model-typed-views` manifest. Its separate application-object
census correction limits earlier 70-object comparisons to a subset of 80
current objects. This review takes no new build, unchanged-object or fixture
credit from either count.

## Retained evidence and replay

The report is `reports/cc12_game_state_queue_failure_owner_review.json`.
Its complete artifact set is retained beneath
`local/cc12_game_state_queue_failure_owner_review/`. Preserve this one root's
relative paths and the report to run the portable replay; older packet roots,
the original game installation, Ghidra and Git are unnecessary for that mode.
Seven captured typed queries verify the configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 and `00400000`
image base through the live client. They retain prototype, xrefs, callers,
callees, all body bytes, complete disassembly and complete pseudocode.
Autostart was explicitly disabled. The current caller name is
`BSP_Game_GrowStateRequestMap`; older dependencies retain their original
metadata epochs.

The Source capture retains complete baseline Git/working archives, the
complete current Git tree, thirteen overlay records and their accepted
manifests. Its selected provider/game closure contains 401 project files,
26 roots and 734 quoted include edges. A separately replayed closure for the
two accepted typed-model headers contains 40 files and 57 edges. Twelve
complete-corpus queries retain 1,104 matches; 40 Source excerpts and 13
documentation excerpts are recomputed exactly. Twelve whole dependency Git
files, twenty support Git files and two separately accepted Root Git files
are pinned. Whole prior reports are dependencies; their nested proof trees
and artifact paths are not recursively traversed.

The full original PE is retained. Only this packet's 105 bytes are freshly
decoded. Opaque allocator `004B8C90` (85 bytes) and growth `004D20D0`
(341 bytes) dependencies are checked for byte identity against their prior
reports and that PE, without fresh instruction decoding or scope expansion.
Every local field, call, frame transition and Source assertion is reproduced
by the retained audit helper. Its write/print tail is removed by the portable
verifier before recomputation; the resulting audit must equal the complete
retained JSON.

Run `python local/cc12_game_state_queue_failure_owner_review/verify.py` for
the guarded artifact-only replay. It rejects reads outside this retained
root/report and rejects writes, subprocesses, network access, dynamic-library
loads and filesystem mutations after guard installation. Run the separate
`--original-worktree` mode for all actual Git blobs, the complete current
4,206-file working Source comparison, accepted Root ancestry/blob checks and
the exact two owned output paths. The physical relocation helper copies all
pinned artifacts plus the exact report to a new directory and invokes the
same guarded verifier there. These are evidence-integrity checks, not Native
execution or application tests.

The next independently leased Native review should first establish the
function/data footprint of registered handler `00C65D08`, then its bounded
FuncInfo/unwind routing. This report invents no address range for that work.
A future dormant adapter may reuse the existing raw types after caller
cleanup and throw-provider ownership are explicit. General production queue
binding and native exception interoperability remain outside this review.
