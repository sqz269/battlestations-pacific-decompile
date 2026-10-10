# Profile request production owner review

`MenuPressStartHost::request_read_007ff100` is not ready for a production binding.
The adapter is a stack object recreated during frame/title work, while the
existing storage driver can retain a callback. The durable profile/storage
owner and three shared completion slots are absent from production, and the
existing semantic callback captures a profile reference where retained Native
completion evidence observes the current global game profile. Neither a capture
of `this` nor a borrowed reference to the menu implementation settles those
lifetime and identity requirements. No C++ GO follows from this review.

## Scope and retained inputs

Source is frozen at accepted main `9f7365c76`. Its complete tracked
`src/`/`include/` Git tree is byte-identical to the already accepted 4,206-file
corpus in `f3c00371e`. That immutable corpus is reused for complete-corpus
queries; no additional full Source archive is created. This packet separately
retains 338 complete current Source files from 34 relevant roots and their
quoted includes, plus the extra inspected `profile_write.cpp` implementation.
Whole prior worker/Root records and their artifact pins remain available with
their original epochs.

Fresh Native analysis is restricted to **`007FF100..007FF187`: 136 bytes and
57 instructions**. Five typed queries capture the prototype, callers, xrefs,
callees and complete body bytes. They use the verifying `bsp.py` client against
project `bsp`, `/battlestationspacific.exe`, x86 and image base `00400000`, with
autostart disabled. `C:/Users/sqz269/bsp.gpr` is the configured project path;
the API does not independently expose that full filesystem identity.

All 136 bytes match the retained complete 12,223,752-byte PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
No caller, callee, vtable data, `007FEFE0`, `008D7A50`, callback or exception
body was freshly opened. Their Source/docs remain explicitly retained evidence
or unresolved effects.

## Actual request contract

The original ABI consumes the profile in ECX, a `NativeString` header pointer
at entry-ESP `+4`, and a callback at entry-ESP `+8`; both exits use `RET 8`.
The live stored signature remains `undefined(void)`. The saved pseudocode's
`unaff_retaddr` is the second argument, not a return address. This packet makes
no Ghidra repair.

| Sites | Proven local operation |
| --- | --- |
| `007FF101..F110` | Capture `[0109CECC]` in EBX before any call; save incoming profile in EBP; select embedded profile name header `+34h`. |
| `007FF113..F138` | Exact header self-copy skips work. Otherwise call resize with source length and `1`, re-read source length, then conditionally copy source data into destination data using the destination's current length. |
| `007FF13B..F145` | Load the current table and `+1Ch` word from the captured manager; call it with `(profile+34h, 1)`. |
| `007FF147..F151` | When AL is nonzero, reload the callback argument and store it into global `00F87458`. |
| `007FF157..F164` | Request the read on the same captured manager, then pass **only immediate `007FEFE0` in ECX** to `006ADB50`. No profile pointer is passed as a continuation argument. |
| `007FF170..F17F` | On a false query, reset the captured incoming profile, then reload and conditionally call the callback argument. |

The manager must remain live across name allocation, virtual query and request;
this function does not reload its global after any delegate. It does not pin a
vtable before name assignment: the query target is fetched afterward from the
captured object's current table. Those allocator, string, virtual-query and
request effects are not newly audited.

The body has **one explicit memory-store instruction**, the accepted-request
write to `00F87458`; it also has thirteen pushes, seven calls, four conditional
branches and two returns. Five calls are direct; the storage query and rejected
argument callback are indirect. Delegated name/reset operations can write
memory. The rejected branch has no local `00F87458` store; that does not exclude
reset or callback side effects on shared state.

The ordinary stack model covers every local branch. Four saved-register pushes
make `[ESP+14h]` the first argument and `[ESP+18h]` the second; both exits restore
entry ESP before removing eight argument bytes. Callee cleanup and nonvolatile
register preservation are explicit assumptions, not fresh provider ABI proof.

Typed xrefs report three direct call sites:

| Call site | Reported owner |
| --- | --- |
| `0067CE4C` | `0067CC60`, press-start sign-in. |
| `004E26EF` | `004E2200`, game console-command queue. |
| `004E3469` | `004E27E0`, still named `FUN_004e27e0`. |

Every invocation reaches the same global callback store. A menu-only private
slot would therefore need an explicit shared-domain contract with other
participating callers. These xrefs do not rule out indirect callers.

## Production owner and deferred capture

The existing `apply_sign_in_0067cc60` builds its save name in a local 128-byte
buffer, queries storage and calls the request hook. The existing request helper
copies that name into the profile and queries storage again; its backend request
copies the name into backend-owned storage. A binding must preserve both queries
and the immediate copying, rather than retain a view of the caller's buffer or
reuse the outer query result.

`MenuPressStartHost` borrows `GameMenuHost::Impl&`. Its four construction sites
are all automatic local variables in `src/game_hosts_menu.cpp`:

| Line | Scope |
| --- | --- |
| 1554 | `GameMenuHost::Impl::pump` |
| 1564 | `GameMenuHost::Impl::advance_path` |
| 1785 | `GameMenuHost::run_title_init_004c9a70` |
| 1848 | The front-end branch of `GameMenuHost::frame` |

Capturing the adapter would dangle after those scopes return. The implementation
owner survives individual frames, but `GameMenuHost` has ordinary destruction
and startup teardown explicitly deletes the menu before the front-end host.
Merely capturing `owner_` also does not prove survival through deferred delivery,
reentrant callbacks or shutdown. No cancellation/drain policy is recovered here.

The full corpus contains no production `ProfileResetState`, `ProfileIoState`,
`StorageOperationState` or `PcProfileIoHost` binding in the `game_*` sources.
`PcProfileIoHost` mentions remain confined to its own implementation/header.
The current menu hook discards the name and logs unimplemented; its storage
availability/query predicates return false. The callback `0067CA40` is described
by prior evidence, but no production callback routing is supplied by this hook.

The existing `drive_profile_storage_task` closure captures
`profile`, `settings`, `io`, `read_host` and `write_host` **by reference**.
`restore_profile_settings_008d7a50` separately captures `settings`, `state`
and `host` by reference. The copied enum/task value does not extend any owner
lifetime. `PcProfileIoHost`, its writer, backend, operation, Lua owner, archive
services, output, input, conversion flag and UI/render services must all remain
valid whenever those closures can run, including synchronous delivery.

Retained `GAME_PROFILE_RESET` evidence says `007FEFE0` obtains the global game
profile at completion. The fresh request corroborates only that no profile
argument is forwarded to it. The semantic captured reference and that current
global observation are separate contracts. A future binding must establish a
stable canonical identity across the interval or use an accepted current-profile
resolver. This review neither assumes stability nor implements such a resolver.

## Shared slots and reentrant ordering

| Slot | Owner/ordering contract |
| --- | --- |
| `00F87458` | Profile-I/O completion. The fresh request replaces it after successful query. The rejected branch uses its argument callback. Existing profile completion takes/clears the current slot and transfers it into settings restore on the successful read path. |
| `00E198F8` | Storage-driver continuation. Existing driver may update storage before publishing the argument; interactive prompts retain it. Completion takes/clears the current slot, calls it, then dismisses the prompt, even if the callback reentered. |
| `00F88958` | Settings completion. Accepted `008D79A0` evidence reloads it after reader/cleanup work, clears it and invokes the saved current value. |

The driver is not merely an asynchronous registrar: completion may happen
before `007FF100` returns. A nested call can replace a shared slot between
delegates. A binding must preserve those current-slot observations and ordering,
not introduce a per-request queue, callback snapshot, extra clear, lockout,
forced completion or cancellation. All participating calls must share the same
durable operation/slot domain. The captured manager in this request and current
manager resolution in other retained entry contracts must remain distinct.

Existing `StorageOperationHost` also requires real progress, prompt and render
services. Prompt callback `006AD3C0` must resume the same state/host domain.
Prior `0067CA40` evidence describes testing current storage state before clearing
menus and requesting the title skip. Neither callback can safely retain the
temporary menu adapter. Their Native bodies are outside this fresh scope.

## Reader lifetime already supported

There is no blanket use-after-close claim. The existing reader deliberately
represents its global root with an untracked Lua pseudo-index. Balanced reads
release child tables/cursors before closing Lua; releasing the remaining
global root afterward does not access the closed state. The profile and settings
readers leave their child sections, and `PcProfileIoHost` keeps separate stacks
of reader frames so nested reads need not replace an outer frame.

These positive Source contracts must be retained. Arbitrary outstanding tracked
children/cursors, callbacks that retire their own owners, interrupted reads or
exceptional unwinding are not covered by the normal sequence. This packet does
not reorder destruction or invent blanket reference invalidation.

## Readiness and next bounded frontier

No bounded production Source packet is ready. The missing prerequisites are:

1. A durable profile/storage/three-slot domain shared across participating
   request callers, with a proved retirement and callback lifetime.
2. An explicit current-global versus captured-profile selection contract.
3. Real menu-completion and prompt-response bindings that survive reentry.
4. The previously identified reader on the actual raw settings owner, followed
   by the correctly timed optional mission-Lua publication adapter.

The smallest next Native owner body is **`007FEFE0`**, under a separate lease:
it is the exact continuation passed by this request without a profile argument.
Its current game/profile/storage selection and callback-transfer boundaries
should be established before approving an owner adapter. `0067CA40` and
`006AD3C0` are additional callback frontiers; `008D7A50` remains retained evidence
unless separately leased.

The existing request/completion/driver helpers in `profile_reset.cpp`,
`profile_persistence.cpp`, `pc_profile_io.cpp`, `profile_commit.cpp` and
`storage_operation.cpp` should be reused. The production ownership seams are
`game_hosts_menu.hpp/.cpp` and `game_hosts.hpp/.cpp`; naming these files is not
an approved implementation packet. No raw-reader or mission resolver is assigned
here. Writing the copied settings read view or inserting a call into raw startup
would not establish the required contract.

## Replay and limits

The report pins 34 Source excerpts, six prior-document excerpts, eight complete
corpus searches, all four adapter construction sites, whole prior worker/Root
records, and the complete fresh Native body. The new Source closure has 627
quoted edges; the extra implementation adds one resolved edge. System/SDK
dependencies remain explicit and are not a compiler-input closure.

`python local/profile_request_owner_verify.py` checks retained artifacts only,
including the reused prior evidence directory, and recomputes the finite Native
and Source assertions. Its guard rejects external reads, writes, subprocesses,
network access and dynamic-library loading. The separate
`--original-worktree` mode verifies actual Git objects, selected working bytes
and the exact two-file change scope.

No C++, build, test, Native execution, SDK/OS/game action or Ghidra mutation was
performed. No complete delegated-effects, register ABI, exception/fault,
startup timing, selector or gameplay proof is claimed.
