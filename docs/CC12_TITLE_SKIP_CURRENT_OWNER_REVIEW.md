# CC12 title-skip current-owner review

`0068D8A0` resolves three owners at distinct points: current storage before the
reset, current game after that reset, and current game again after requesting
state 4. The last game publication can differ from the request receiver. This
closes the immediate post-dismiss title-skip body, but does not authorize a
production profile/title binding. The next smallest Native body is `004D7920`,
the actual request wrapper; it needs a separate lease and review.

The Source baseline is **6c010967124a8faa71c92d943d3ae9dc095b9b31**. This packet
owns only `0068D8A0`, this document, its JSON report, and its ignored local
evidence directory. It changes no C++, Ghidra state, startup route, callback,
SDK/OS interface, or game behavior.

## Complete bounded Native result

The retained body is exactly `0068D8A0..0068D8CD`, 46 bytes and ten instructions.
Every instruction is decoded and matched to the retained original PE and the
complete live listing. The configured project and program were checked for
each read-only typed query: `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86, image base `00400000`.

| Site | Local operation and required owner |
| --- | --- |
| `0068D8A0` | Load `ECX = [0109CECC]`, entry-current storage S. |
| `0068D8A6` | Call `00BD3450` on S unconditionally, before any game read. |
| `0068D8AB` | Load `ECX = [00E188A8]`, current game A after reset returns. |
| `0068D8B1` | Compare DWORD `[A+5E8]` with zero. |
| `0068D8B8` | Any nonzero value branches directly to the final `RET`. |
| `0068D8BA` | Push the literal DWORD `4`. |
| `0068D8BC` | Call `004D7920` with the same checked game A in ECX. |
| `0068D8C1` | Reload `[00E188A8]` into EAX, current game B after request returns. |
| `0068D8C6` | Clear exactly one byte at `[B+5EC]`. |
| `0068D8CD` | Plain `RET`. |

The pending arm performs the reset and returns without a local request or hold
clear. The reset's delegated effects are not excluded. The zero arm checks and
requests on A, then clears B's hold byte. Caching A for that final store would
require a separate proof that the publication cannot change during the request.
The body has no null guards, fallback owner, incoming receiver use, stack
argument consumption, retry, or frame scheduling. No meaningful return value
is established.

There is one explicit memory-writing instruction. That count excludes the
implicit stack writes from `PUSH` and `CALL`, and says nothing about delegated
callee effects or aliases. There are no local operands naming the three
completion slots `00F87458`, `00E198F8`, and `00F88958`.

Both normal control-flow paths visit the final return with the entry stack
position restored under the declared external ABI assumptions: reset has no
stack arguments and the request callee removes the pushed DWORD. The caller
does not perform that cleanup. This is not fresh proof of the callee's `RET 4`,
exceptional behavior, or non-returning paths. If a call does not return normally,
the remaining local steps do not follow.

Seven retained xrefs identify six caller functions, including two sites in the
press-start update. One xref is `0067CA57` from the accepted previous callback.
The xref tool labels it `UNCONDITIONAL_CALL`; the separately accepted 29-byte
callback body establishes that site as a tail `JMP`. Caller metadata does not
change that instruction evidence or establish the absence of indirect callers.
No other Native body or data table was opened for this packet.

## Current Source comparison

The production `MenuPressStartHost::request_main_menu_state` at
`src/game_hosts_menu.cpp:543` sets `press_start_handoff_requested`.
`MenuPathHost::input_action_pressed` observes it at line 1350. The main-menu
path then uses separate `TitleSkip` and `RequestShellState` steps in
`src/main_menu_path.cpp:155`. Several steps can run in one frame; this review
does not claim an obligatory one-frame delay. This handoff nevertheless does
not establish the immediate post-dismiss Native call and its current-owner
observations.

The typed path resets storage, tests cached `state.state_request_pending`,
requests state 4, and releases a fixed state hold. Its production methods at
`src/game_hosts_menu.cpp:1355` enqueue into `owner_.requests` and clear
`owner_.state.requests_held`. They do not resolve A after the reset and B after
the request. Returning the typed path to `TitleSkip` while pending also does
not authorize a retry in the completed Native callback.

`GameMenuHost::Impl` borrows a `GameStateSlot` and owns a separate
`GameStateRequestQueue`. The queue uses
`std::vector<std::unique_ptr<GameStateRequestBlock>>`, a head offset, and a
count (`include/bsp/game_frame_control.hpp:29`). That semantic representation
is not a demonstrated alias to the actual Native game fields at `+5D8`,
`+5E8`, and `+5EC`. The retained enqueue declaration describes Native
`004D3ED0` as taking a pointer to an integer, while its C++ helper takes a
value. The fresh title call supplies literal 4 to **004D7920**, a distinct
wrapper. Substituting the existing enqueue helper leaves that wrapper and its
raw owner/queue contract unproved.

Both `MenuPathHost` and `MenuTitleHandoverHost` still log the storage reset as
unimplemented. The concrete `PcStorageBackend::reset_storage_operation_00bd3450`
exists at `src/storage_backend.cpp:103`: it sets readiness and derives the
operation state from the error flag, preserving the operation-code and prompt
fields. Its existence does not bind that backend to the actual current
`0109CECC` publication. No fresh reset body was inspected here.

The existing `GameNativeGameRuntime` checks shared construction/lifetime game
cells, and `GameNativeSettingsProcess` exposes a game cell. These are useful
retained ownership boundaries, but do not supply the missing title/profile
binding. The Native game lifetime reconstruction clears the game publication
before later profile destruction. Publication clearing therefore cannot alone
prove that captured outer profile references remain valid.

## Callback and lifetime contract

The accepted preceding `0067CA40` callback reads current storage, dismisses
prompts on its zero-state arm, and tail-enters this title body after dismissal.
Consequently this body's storage read occurs after any dismissal callbacks and
refresh effects. There is no evidence to reuse the storage or game owner from
the earlier request, driver, profile completion, or menu lookup.

S must remain valid through its reset call, A through the pending test and
request, and B through the final byte store. Outer callers have their own
remaining uses: accepted `007FEFE0` may resume settings application and profile
commit after synchronous delivery returns, and the storage driver can dismiss
its prompt after the continuation. The distinct shared slots and their
clear/transfer/reentrant ordering remain part of the accepted prior contract.
This body neither merges nor clears them.

Four automatic `MenuPressStartHost` objects remain at
`src/game_hosts_menu.cpp:1554`, `:1564`, `:1785`, and `:1848`. Their borrowed
`GameMenuHost::Impl` reference is also insufficient as a deferred lifetime
guarantee. The retained shutdown paths delete the menu before the frontend.
A real composition must assign ownership through the actual remaining accesses,
including nested synchronous delivery, pending callbacks, and shutdown. This
review supplies no queue, retry, cancellation, lockout, automatic draining, or
cleanup reordering to fill those gaps.

## Source epoch and retained evidence

The bundle contains the complete 4,206-file Source Git and working archives,
tree and manifest, plus exactly nine qualified material overlays. Three are
the previously accepted Source3350 particle manager/header and script-order
comment changes. Six are the accepted Source3352 dormant owner changes in
`game_hosts_singletons`, `game_native_string_process`, and
`native_singleton_destruction`, each with its header and implementation.
Their whole retained bytes match the primary Source3352 manifest and this
frozen checkout; Git comparisons normalize line endings only. The complete
manifest is pinned, but unrelated paths inside it are not opened or replayed.
Later Atlas changes to `game_hosts.cpp` or its header are outside this epoch.

The whole-source searches have 272 matches across eight bounded queries. The
selected Source closure contains 402 files from 34 explicit roots and 735
quoted include edges, with no missing project include. The report has 27
whole-file-backed Source excerpts and four retained documentation excerpts.
System and SDK includes are listed separately; this is not a compiler closure.

Four whole prior worker/primary JSON reports and their selected documents are
retained as exact Git artifacts. Their nested proof trees are deliberately not
walked or replayed. Only the actually relied evidence is copied into this
packet's single retained root. The older 29-byte `0067CA40` and 278-byte
`007FEFE0` bodies receive byte-identity checks against the retained image and
prior reports, with no new decoding or behavioral credit.

`reports/cc12_title_skip_current_owner_review.json` pins the immutable freeze,
audit, this document, and the six helper scripts. Portable verification opens
only the retained root and the current report, forbids writes, subprocesses,
network access and dynamic loading, then rebuilds the Source closure/searches,
all Native instruction facts, both declared stack paths and dependency
qualifications. A separate original-worktree mode checks actual Git objects,
the current working Source, and the exact two allowed changed paths.

```powershell
python local/cc12_title_skip_current_owner_review/verify.py
python local/cc12_title_skip_current_owner_review/verify.py --original-worktree
python local/cc12_title_skip_current_owner_review/relocate_verify.py
```

The relocation helper copies every pin plus the exact report to a fresh
directory, then runs the guarded artifact-only verifier there. It is a one-time
copy with a retained receipt, not a command to rerun over an existing result.
Preserve all relative paths under `local/cc12_title_skip_current_owner_review/`
and the report for later portable replay. Old absolute paths in the retained
dependency reports are provenance only.

## Decision

There is **no C++ activation GO**. The next bounded Native owner/ABI review is
`004D7920`, to establish the request wrapper's actual receiver, argument
conversion, queue/allocator behavior, and possible current-publication effects.
The raw storage reset binding, actual menu getter/lifetime, prompt-response
adapter, raw profile/settings/archive composition, late Lua owner resolution,
and shutdown contract remain separate frontiers where required. Nothing in
this packet authorizes a raw startup profile call or a substitute semantic
queue.

Validation is evidence replay and artifact/Git identity only. This packet runs
no compiler, C++ tests, Native functions, game, SDK/OS path, or Ghidra mutation;
it claims no build, fixture, ABI-drop-in, startup, or gameplay equivalence.
