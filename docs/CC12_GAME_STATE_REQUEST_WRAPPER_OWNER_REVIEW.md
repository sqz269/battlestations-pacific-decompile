# CC12 game-state request wrapper owner review

`004D7920` forwards the **incoming** game object's embedded queue at `+5D8h`
and a **pointer to the caller's incoming state-value slot** to `004D3ED0`.
It does not read that value or resolve the current game publication. Its own
`RET 4` is directly proved. The enqueue callee's value-read timing, queue
mutation, allocation, argument cleanup and other effects remain a separate
frontier. There is **no production C++ activation GO**; the next smallest
Native body is `004D3ED0` under a new lease.

The independently pinned Source baseline is
**9fb890ff61064a9b0e4b6b868d1b5627036da3cd**. It contains the accepted title-skip
review and the same nine qualified Source overlays. The pending Atlas startup
C++ edits are outside this epoch. This packet owns only `004D7920`, this
document, its report, and its ignored local evidence directory.

## Complete Native body and ABI

The body is exactly `004D7920..004D7932`, 19 bytes and five instructions. Every
byte and instruction matches the retained original PE, typed live byte query,
and complete live listing. Seven read-only typed queries retained the prototype,
xrefs, callers, callees, bytes, listing and pseudocode. Each verified the
configured `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86
language and image base `00400000`; autostart was disabled.

Let S be entry ESP and A be entry ECX. `[S]` is the wrapper's return address;
`[S+4]` is the incoming DWORD value slot.

| Site | Instruction | Established local effect |
| --- | --- | --- |
| `004D7920` | `LEA EAX,[ESP+4]` | EAX becomes the address S+4. It does not load or copy the value. |
| `004D7924` | `PUSH EAX` | Store that pointer at S-4. |
| `004D7925` | `ADD ECX,5D8h` | Adjust the incoming receiver to the embedded queue A+5D8h. |
| `004D792B` | `CALL 004D3ED0` | Pass that queue and pointer to the enqueue callee. |
| `004D7930` | `RET 4` | Return and discard the incoming value DWORD. |

The live `undefined(void)` prototype and pseudocode omit the incoming ECX
receiver. The assembly establishes the receiver, argument-address conversion
and immediate return cleanup; no Ghidra signature or listing was changed.
An `int` state is a useful descriptive interpretation from the existing
reconstruction, but the wrapper itself applies no signed test, narrowing,
value validation or range restriction to the 32-bit slot.

There is one straight-line path, one direct call, no branch and no explicit
memory-writing instruction. `LEA` has a memory-shaped operand but does not
access its contents. The wrapper has no explicit data-memory read either.
Implicit stack effects are counted separately: `PUSH` writes the pointer,
`CALL` writes its return address, and `RET` reads the wrapper return address.
Zero local data stores do not exclude any callee write or alias effect.

For a normal enqueue call, callee entry ESP is S-8, its return address is at
S-8, and its pointer argument is at S-4. The pointed-to value remains at S+4.
The wrapper performs no caller cleanup after the call. Ordinary balance
therefore requires the enqueue callee to remove its own pointer DWORD, bringing
ESP back to S before the decoded wrapper `RET 4`. The wrapper then returns
with caller ESP at S+8. The enqueue cleanup is a declared external ABI
assumption, not fresh proof of its `RET 4` or exceptional paths. The wrapper's
own immediate return cleanup is directly decoded.

No meaningful return value is assigned after the call; EAX is whatever the
callee leaves. `ADD ECX` updates flags, with subsequent flags and volatile
register behavior delegated. There is no local null, capacity, pending-count,
publication-validity or state-range guard, and no local retry or recovery.

The current metadata contains 28 call xrefs and 15 function-caller rows. Three
xrefs have no containing-function label. The historical repository lookup's
different caller count is not adopted, and no function boundary or listing
repair was performed. These metadata do not prove the absence of indirect
callers or the argument values at uninspected callers.

## Relation to the accepted title caller

The accepted `0068D8A0` body reads game A after resetting current storage,
checks A's pending count, pushes literal 4, and calls this wrapper on A. This
review establishes the exact next boundary: enqueue receives **A+5D8h** and
the address of the caller's slot containing 4.

There is no `00E188A8` operand or other global lookup in the wrapper. It does
not replace A with a newer game or reload a game after enqueue. The accepted
title body performs its own fresh game B load only after this wrapper returns,
then clears B's hold byte. That post-call reload remains necessary unless a
separate contract proves publication identity through delegated calls.

The pointer is borrowed caller stack storage. The wrapper does not snapshot
the value before calling enqueue. The leaf determines when it reads the
pointee, whether intervening allocation or callbacks can occur, and whether
it retains or writes through the pointer. None of those effects are recovered
from this wrapper. A deferred call using that pointer, or an eager by-value
replacement, needs the leaf's actual lifetime and read-order contract.

No allocator, map growth, queue element store, head/count update, hold clear
or completion-slot access appears locally. Those effects remain delegated.
The three accepted profile/storage/settings completion slots remain distinct;
the wrapper does not establish their ordering or retirement. Earlier accepted
outer continuations may have more work after synchronous callback delivery.
Their captured owners must remain valid through those uses.

## Current Source and owner boundaries

`GameStateRequestQueue` in `include/bsp/game_frame_control.hpp:29` uses a
`std::vector<std::unique_ptr<GameStateRequestBlock>>`, a head offset and a
count. Its enqueue helper in `src/game_frame_control.cpp:96` accepts a
`std::uint32_t` value. Map growth uses C++ vector allocation and movement;
new blocks use C++ allocation. This preserves a semantic queue interface,
but does not establish the Native pointer argument, raw embedded layout,
allocator ownership, or delegated reentry behavior.

The header separately documents Native `004D3ED0` as taking a pointer to the
integer to copy. That retained contract is consistent with the newly proved
wrapper boundary. It does not replace a fresh review of the leaf's actual
load timing, writes, growth paths or argument cleanup.

`MenuTitleHandoverHost::request_game_state` at
`src/game_hosts_menu.cpp:621` and `MenuPathHost::request_game_state` at line 1358
enqueue into fixed `owner_.requests`, then mirror the count into
`owner_.state.pending_requests`. The latter releases the hold on the same
fixed state object. `MenuImpl` owns that semantic queue and borrows a
`GameStateSlot`; their association with the actual incoming raw game queue
has not been proved. A log label containing `004D7920` does not supply that
missing ownership or ABI contract.

Useful raw lifetime boundaries already exist in retained Source. Native game
construction at `src/native_game_construction.cpp:132` initializes the queue
map/count/head fields at `+5DC/+5E0/+5E4/+5E8` and the hold byte at `+5EC`.
Native game teardown at `src/native_game_lifetime.cpp:119` passes game+5D8h
to the pointer-block destruction adapter. The adapter at
`src/native_game_container_lifetime.cpp:65` uses raw header words, frees its
blocks and map through the supplied free provider, and clears header fields.
These are retained reconstruction facts, not fresh constructor/destructor
Native analysis or proof of a compatible producer.

`GameNativeGameRuntime` exposes shared construction/lifetime game cells.
Retained lifetime Source clears the publication before later profile and queue
destruction. These facts do not authorize treating a cleared publication as
proof that every captured owner has retired. The four automatic
`MenuPressStartHost` sites at lines 1554, 1564, 1785 and 1848, and the borrowed
`MenuImpl`, still require a durable owner if used for retained callbacks.
Menu shutdown deletes the menu before the frontend. This packet adds no
ownership, shutdown exclusion, callback drain, queue repair or cancellation.

Whole-Source address/name searches find the wrapper's comments, virtual host
interfaces and production log sites. They do not identify a separate bound
raw wrapper producer under those identifiers, nor do they prove the absence
of arbitrary aliases. The current production path still lacks the complete
raw queue and profile/menu/settings owner composition established by the
accepted reviews.

## Evidence and replay

The single retained root contains the complete 4,206-file Source Git and
working archives, manifests and current tree, with all nine accepted overlays
matched independently against actual current Git and working bytes. Source
content is unchanged from the accepted title-skip epoch. The selected closure
has 393 files from 20 explicit roots and 724 quoted include edges with no
missing project include. Eight complete-corpus queries yield 297 matches;
22 Source excerpts and three documentation excerpts are backed by whole files.
External system/SDK includes are recorded separately and are not a compiler
closure.

The previous title worker and primary reports are retained as whole exact Git
artifacts, together with their documents. Their nested Root proof trees are
not walked or replayed. The accepted 46-byte title body is retained and checked
for byte identity against the report and original PE, without new decoding.
Only this wrapper's 19 bytes receive fresh Native instruction analysis.

`reports/cc12_game_state_request_wrapper_owner_review.json` pins the immutable
freeze, audit, this document, and six helper scripts. The default verifier
opens only the new retained root and the report, prohibits writes, subprocesses,
network and dynamic loading, and recomputes the Source closure/searches,
overlays, excerpts, complete instruction facts and declared normal stack path.
The separate original-worktree mode checks actual Git objects, all current
Source working bytes, and the exact two owned output paths.

```powershell
python local/cc12_game_state_request_wrapper_owner_review/verify.py
python local/cc12_game_state_request_wrapper_owner_review/verify.py --original-worktree
python local/cc12_game_state_request_wrapper_owner_review/relocate_verify.py
```

The relocation helper copies every pin and the exact report into a fresh
directory, then runs the guarded artifact-only verifier there. It retains a
receipt and refuses an existing destination. Preserve every referenced
relative path under `local/cc12_game_state_request_wrapper_owner_review/` and
the report. Older absolute paths inside accepted dependency reports are
provenance only.

The next separately leased body is **004D3ED0**. It must establish the actual
pointee read, raw map/block writes, growth/allocation calls, argument cleanup,
failure paths and delegated effects before a genuine production queue adapter
can be selected. Further growth, allocator, storage/menu/prompt, profile,
settings/archive, late Lua and shutdown owners remain bounded dependencies
where that work exposes them.

Validation is artifact/Git identity and evidence replay only. This packet
changes no C++, runs no compiler or C++ tests, executes no Native/game/SDK/OS
path, and makes no Ghidra mutation. It claims no activation, ABI-drop-in,
build, fixture, startup or gameplay equivalence.
