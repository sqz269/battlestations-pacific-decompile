# Settings profile-restore owner binding review

The existing profile completion reconstruction cannot yet supply a production
archive callback on the same raw settings receiver. `PcProfileIoHost` owns a
semantic `GameSettingsBlock` reference and has no production construction.
The production press-start adapter's profile-read request is still
unimplemented. The raw process settings owner exists, but no raw archive-reader
binding connects it to that profile path. A compatibility adapter alone would
leave these owners and the call schedule unassigned. No C++ implementation is
authorized or added by this review.

## Evidence epochs

Source is frozen at accepted main `ea3c9fc17`, with all **4,206 tracked files**
under `src/` and `include/` retained as complete Git and working archives.
All 4,206 match the compatibility worker's `48d432db3` Source bytes after LF
normalization. That is a current Source comparison, not adoption of its Native
findings: compatibility commit `9d371a997` was pending Root review at assignment
and remains labeled that way in this freeze.

The accepted reader worker `83d73fe97` retains its own `7af8a0ac6` Source epoch.
Its whole report, Root acceptance report and their retained artifacts are
preserved, including Root's explicit later Source overlay. The separate
compatibility report, artifacts and whole Source corpus are also preserved.
No earlier epoch or report was rewritten.

Only `008D79A0` was freshly queried and decoded. Five typed, read-only queries
capture its prototype, xrefs, callers, callees and **175 bytes / 48 instructions**,
covering `008D79A0..008D7A4E`. Every byte agrees with the retained complete
12,223,752-byte PE, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The typed client verifies project `bsp`, `/battlestationspacific.exe`, x86 and
image base `00400000`; `C:/Users/sqz269/bsp.gpr` is the configured project file,
not an independently exposed live filesystem identity. Autostart was disabled.

## Native completion owner contract

The body consumes no argument and ends with `RET`. It does not consume an
incoming settings receiver; the receiver is selected inside the body.

| Sites | Actual local behavior |
| --- | --- |
| `008D79B9..79C4` | Capture `[0109CECC]` in ESI and test captured manager `+08h`. Only exactly zero enters the reader branch. The manager is not null-guarded. |
| `008D79CB..79DE` | Retain captured manager `+38h` in EDI, obtain globals through `00B67980`, then construct the local reader with `004425C0`. |
| `008D79E3..79F5` | Push the local reader pointer, load **literal `00F88980` into ECX**, and call `008D6DC0`. |
| `008D79FA..7A0A` | Reload captured manager `+30h`; when nonnull call `_free`, then zero that captured manager field. |
| `008D7A11..7A24` | Close the captured `+38h` Lua owner, then destroy the local reader. |
| `008D7A29..7A3C` | Reload the current callback at `00F88958`, clear the slot, and call the saved pointer if nonnull. |

Both the state-failure branch and normal reader branch reach the callback
reload. A callback published by a delegate before that reload can therefore
be the one delivered. A new callback published by the callback itself survives
because the old slot has already been cleared. There is no retry, retain or
callback queue in this body. The storage manager and its embedded Lua owner
must stay live across all delegated calls; replacing the global does not change
the captured ESI/EDI objects during this invocation.

Under the ordinary balanced-call model, the constructor's reader address and
the pointer passed to `008D6DC0` are both entry-ESP minus `20h`. This reuses the
accepted constructor's `RET 14h` and reader's `RET 4`; the other provider
cleanup/nonvolatile contracts remain explicit assumptions. The local stack
model covers every branch and reaches `RET` at entry ESP. It does not prove
callee side effects, arbitrary alias writes, exception unwinding or callback ABI.

All seven explicit memory-write instructions are accounted for: three stack
stores, two FS exception-chain stores, captured manager `+30h`, and `00F88958`.
The body has six direct calls and one optional indirect callback, eight explicit
pushes and three conditional branches. There is no local settings-field store
or block copy. Its delegated reader can write settings; this absence of a local
store is not a preservation claim for settings `+24h` or any other field.

The fresh incoming references are `008D7B6F` (direct call) and `008D7B36`
(callback-address data reference), both owned by `008D7A50`. That caller's
accepted Source/docs retain the storage/name gates and queued versus immediate
completion contract. Its Native body and all six direct callee bodies remain
outside this packet's fresh analysis scope.

## Existing Source ownership and missing composition

`complete_profile_settings_restore_008d79a0` already represents the state-zero
reader branch, optional buffer cleanup, owner close, reader destruction and
clear-before-call sequence. `restore_profile_settings_008d7a50` captures
`settings`, `state` and `host` by reference for deferred completion. The header
explicitly requires all three to outlive delivery. These are existing host
projections; this review adds no duplicate completion helper.

| Component | Established contract and remaining gap |
| --- | --- |
| `PcProfileIoHost` | Borrows `GameSettingsBlock&`, backend, operation and services. Constructor requires `operation.manager_0109cecc == &backend.operation()`. No production construction occurs in the complete corpus. |
| Reader frame | Owns `GuiLua51Host` and `GuiLuaReader`, backed by the backend's nonnull storage Lua state. Frames live in the profile host's vectors. Settings completion pops its frame after owner close, matching the existing normal Source order. It is not a raw `004425C0` object-layout binding. |
| Archive callback | `SettingsArchiveReadHost::publish_xbox_compatibility_008d44c0(bool)` is pure virtual, with no concrete override. `PcProfileIoHost` simply forwards the supplied `services_.archive_read`. Its Boolean alone does not identify the receiver. |
| Raw settings | `GameNativeSettingsProcess` owns the initialized BCh `NativeGameSettingsStorage` for process lifetime. `GameNativeSettingsApplication::load` uses `008D8190`, then exposes a copied semantic read view. That view is not an alias of the raw owner. |
| Production profile entry | `MenuPressStartHost::request_read_007ff100` discards the save name and logs unimplemented. Its storage-availability/query predicates also return false. The real profile route is not reached through this adapter. |
| Mission Lua owner | `GameMissionHost::Impl` owns a separate `GameMissionLuaHost` created during mission loading. Source explicitly distinguishes that timing from the original OnInitOnce schedule. There is no profile-reader callback binding to this owner. |

The three completion slots remain distinct:

| Native slot | Existing Source owner |
| --- | --- |
| `00F87458` | `ProfileIoState::completion` for profile I/O. |
| `00E198F8` | `StorageOperationState::continuation_00e198f8` for storage driving and prompts. |
| `00F88958` | `ProfileSettingsRestoreState::completion`, owned by `PcProfileIoHost::restore_state_`. |

The existing semantic request path is `007FF100` → profile completion
`007FEFE0` → settings restore `008D7A50` → settings completion `008D79A0`.
These arrows are Source/retained-contract evidence, not newly audited Native
bodies. The production entry, raw archive reader, retained callback owner and
mission-machine resolver are still missing as one coherent composition.
Existing Source cleanup is the normal sequence; no new exception-safety or
raw ownership equivalence is claimed.

## Smallest next frontier

No bounded production composition is ready for C++ GO. The next owner packet
should begin at the actual unimplemented `MenuPressStartHost::request_read_007ff100`
seam and, if fresh Native evidence is needed, lease **`007FF100`**. It should use
the existing profile request/completion Source rather than duplicate it, and
settle who owns the profile, storage operation and all three completion slots
through deferred and reentrant delivery. The `007FEFE0` and `008D7A50` bodies
require separate ownership before any fresh Native inspection.

Before a publication adapter can be implemented, a genuine raw archive reader
must accept the same `GameNativeSettingsProcess::settings()` object, and its
callback must resolve the represented optional mission machine after the
compatibility stores. A semantic read-view write, an earlier Boolean presence
snapshot, fundamentals/input Lua, queued replay or a new call in raw startup
does not establish that contract. This packet assigns no early interpreter,
new startup call or synchronization policy.

## Replay and validation limits

The JSON report pins whole Source/Git archives, six prior reports/documents,
4,327 prior artifact records, the complete PE, whole function exports and all
typed outputs. The 37 initial reviewed roots close through 555 project files and four
configured Lua headers; two additional inspected implementations add two
resolved quoted edges, for **557 project files / 1,099 quoted edges**. System,
C++ and SDK dependencies are listed but are not a compiler-input closure.
Eight complete-corpus searches and 23 whole-file-backed excerpts are replayed.

`python local/profile_restore_owner_verify.py` is artifact-only: it checks all
retained pins, Git blob contents, include closure, queries, exact Source
excerpts, PE placement and all 48 instructions, then recomputes the owner and
stack assertions. Its read guard forbids external reads, writes, subprocesses,
network access and dynamic-library loading. A separate
`--original-worktree` mode checks actual Git objects and the two-file change
scope at the original worktree; it is not the portable mode.

No C++, compiler, fixture, Native execution, SDK/OS/game operation or Ghidra
mutation occurred. This review establishes a local owner/call contract and
Source binding gaps. It grants no complete delegated-effects, binary ABI,
startup timing, sound-selector, enable-byte provenance or gameplay credit.
