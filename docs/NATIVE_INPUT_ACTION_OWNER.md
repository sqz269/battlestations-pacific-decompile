# Native lazy input-action owner

Packet `orch4_native_input_action_owner_ac` reconstructs `004BEC00`, `00A93DA0`,
`00A93DD0` and `00A93E50` in `native_input_action_owner.hpp/.cpp`. Names are
hypotheses, not recovered symbols. The existing `BSP_InputManager_GetSingleton`
name is retained. Analysis used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, read-only. This packet is not an application input
binding and does not publish an `InputTickState` or `InputFocusBackendState`.

## Coverage and original ABI

| Entry | Original ABI and inclusive last instruction | Coverage |
|---|---|---|
| `004BEC00` | No incoming arguments; EAX pointer; plain RET at `004BECBC` | Complete owner schedule in the stated provider/C++ exception domain |
| `00A93DA0` | ECX raw24h allocation; EAX same pointer; RET at `00A93DC7` | Complete stores; padding untouched |
| `00A93DD0` | ECX raw24h owner; no stack arguments/result; RET at `00A93E4F` | Complete owner destruction and supported C++ unwind schedule; required nonempty providers remain external |
| `00A93E50` | ECX owner, one flags DWORD; EAX captured owner; RET4 at `00A93E6B` (ends `00A93E6D`) | Complete scalar destruction/free order |

The explicit C++ context is not the original callable ABI. FH3/SEH metadata,
hardware-fault cleanup, native register/throw identity and mutable exception
spill aliases are outside this source interface. Native profiles are identity
DWORDs, not callable C++ vtables. CRT allocation/free uses the existing concrete
`singleton_lifetime_allocate/free` services, with both sizes exactly24h.

## Canonical storage and producer

`A93DA0` is the producer. `MOV EAX,ECX` at `A93DA0` establishes the return despite
the old void pseudocode. It writes:

| Offset | Actual storage |
|---|---|
| `00` | `D5B630`, whose slot0 points to scalar deleter `A93E50` |
| `04/08/0C` | 30h action-record base / signed count / signed capacity, zero |
| `10/14/18` | DWORD-vector base / signed count / signed capacity, zero |
| `1C` | One byte, value1 |
| `1D..1F` | Untouched |
| `20` | DWORD1 |

The declared raw24h storage owns no hidden containers, strings, allocation or
implicit destructor. The two vector headers are passed directly to providers;
their base/count/capacity are never mirrored in projected owning vectors.
`F8BBF8` is distinct from hardware backend `F8BBF4` and settings `E198E8`.
Construction does not load settings or install actions.

## Getter and shared lifetime

`004BEC15` captures the initial publication. A nonnull fast return uses that
capture without accessing the manager. The slow path gets `00415350`, captures
that manager's section pointer at+10, enters it and increments its+18 recursion
word, then rechecks `F8BBF8`. It allocates24h through `BF681B`, constructs into
that allocation and publishes the returned pointer at `004BEC81`. State1 covers
only the returned allocation through construction; it ends before publication.
The current constructor has no throwing C++ calls. The existing allocator
throws on exhausted allocation; the native nullable branch remains expressed.

`004BEC86` gets the manager again. Only afterward does `004BEC8B` reload the
publication used by `BD0C30`. `CapturedSoundLifetimeSection` keeps the first
section and releases it even if publication callbacks replace the manager.
The slow return reloads `F8BBF8` after leaving. There is no private lifetime
domain or automatic unregister/rollback on registration failure.

The shared `SoundLifetimeAccess` supports the real raw01090AA0 publication and
the existing semantic fixture domain. Application composition must use its
canonical shared manager and admit `D5B630` in its actual finite deletion map.
This worker does not edit that dispatcher or `game_hosts*`.

## Destruction and required providers

| Caller/site | Native dependency | Exact required contract |
|---|---|---|
| `A93DD0/A93E03` | `0086A430` | ECX actual header+10, signed count0, RET4; resize DWORD vector |
| `A93DD0/A93E1F`; unwind helper `A93D80/A93D85` | `00A93C10` | ECX actual header+4, signed count0, RET4; destroy removed 30h records in reverse order |
| `A93DD0/A93E0B,A93E27`; `A93D80/A93D8D` | `_free` at `BF6989` | Capture header base AFTER resize returns; cdecl one pointer, ADD ESP4; thunk to existing `BF65AC` |
| `A93E50/A93E60` | `_free` at `BF65AC` | Same captured allocation as destructor receiver; cdecl one pointer, ADD ESP4 |

Both resize bodies were read, including signed comparisons and RET4. The
source interface deliberately retains address-named required methods
`call_0086a430` and `call_00a93c10`. Their implementations must act on the raw
headers, accept native signed counts, and preserve record callbacks/cleanup.
No empty-vector default is supplied. Primary owns the nonempty record/provider
closure, including `A93940/A939C0/A93A80/A93B30/A93C10` and `86A430/86A220`.

The destructor stamps `D5B630`, resizes/frees header+10, then resizes/frees
header+4. Base pointers and capacities remain in the dying bytes after free.
Only then does it unconditionally clear the current `F8BBF8` cell and stamp
`CE3818` into the original owner. It neither compares publication identity nor
unregisters a manager entry. Raw manager deletion has already popped the entry;
explicit early deletion requires a separate caller-owned unregister schedule.
The scalar wrapper tests flags bit0 after successful destruction and returns
the captured address bits even when it freed that allocation.

## Exception evidence

Getter handler `C64D53` loads FuncInfo `D8D2AC`, whose two-entry unwind map is
`D8D29C`: state0 -> -1 through `C64D40` (captured guard `411EE0`); state1 ->0
through `C64D48` (free saved allocation). Registration runs under state0, so
failure releases the captured guard but retains publication/allocation.

Destructor handler `CB6833` loads FuncInfo `DECB7C`, map `DECB6C`: state0 ->-1
through `CB6820` (`A92180`: clear publication, stamp base); state1 ->0 through
`CB6828` (`A93D80`: resize/free action header+4). State1 covers the first vector
resize/free. State0 is installed BEFORE the second resize. Consequently:

* First-provider failure destroys/frees the action table and then runs base
  cleanup. It does not add a retry/free of the failing DWORD vector.
* Second-provider failure runs base cleanup only. It does not retry action
  record destruction or free that table's base after the failed provider.
* Another exception during unwind cleanup terminates in the source C++ domain.

These schedules are represented with explicit local cleanup guards; they add
no allocation or owner state. Direct and unwind CALL/JMP rows are in the JSON
report, with their actual containing functions.

## Metadata corrections and verification boundary

The primary repaired `A93DD0` before this packet: its old export stopped after
`A93E0B`; the current body contains37 instructions through `A93E4F` and no gaps.
Read-only discovery additionally found the scalar's `A93E65..67` listing gap:
live/disk bytes `83 C4 04` are `ADD ESP,4`, within its existing body.

Unowned `A93D80` currently ends at `A93D91`, omitting `ADD ESP,4` at92,
`POP ESI` at95 and `RET` at96; true end exclusive is `A93D97`. Unowned allocation
cleanup `C64D48` ends at `C64D50`, omitting `POP ECX` at51 and `RET` at52; true
end exclusive is `C64D53`. Neither excluded tail contains a CALL.

The two unowned handler entries were absent in Ghidra when inspected:
`C64D53` ends with JMP at `C64D58` length5 (inclusive `C64D5C`, exclusive
`C64D5D`); `CB6833` ends with JMP at `CB6838` length5 (inclusive `CB683C`,
exclusive `CB683D`). Both target `BF6B43`. They are metadata evidence only,
not new reconstructed/leased functions. No worker Ghidra mutations were made.

All387 bytes of the four routines matched the installed PE and live analysis.
Eight native seeds passed verification. `scripts/build.ps1` completed for
MSVC Win32 Release with `/W4 /WX /fp:strict`; both existing CTests passed.

One ignored manifested probe adapts the actual raw-manager plumbing from
`local/sound_raw_fixture_z.cpp`. It executes the original40-byte constructor
against four preimages and compares every24h byte, including padding. It also
checks raw publication/registration identity, repeated fast return, and a fast
return without creating a manager. The existing CRT validation callback repairs
an intentionally invalid manager header, first returning after replacing the
action publication and then throwing in a separate run. These establish the
already-evaluated registration pointer, final slow-return reload, retained
allocation/publication on registration failure, and captured section depth0.
The fixture unregisters/frees its own allocations explicitly and drains the
real empty manager. Its action providers throw if reached: they do not stand
in for missing record cleanup. No production fault hook was introduced.

Probe output: `PASS native ctor4 preimages/all24h bytes; raw getter identity/fast
path; registration publication replacement+failure; captured section depth0;
manager allocations reclaimed`. The log is ignored
`local/input_action_owner_probe.log`. Exact hashes and call-audit results are in
`reports/native_input_action_owner.json`. This probe does not establish
nonempty destruction, finite action dispatcher integration, device enumeration
or application input behavior.

Follow-up in the SAME ignored fixture exercised the primary's concrete
`native_input_action_records.cpp` and `native_input_binding_storage.cpp`
providers. The runner used primary headers and `bsp_core.lib`/Lua archive, plus
this worker's explicitly compiled owner source pending merge. Thus this is a
split-build provider integration fixture, not yet a primary-archive-only run.
The earlier paragraph/log boundary describes its getter-only phase.

The added phase compares original `A93940` against four30h preimages, populates
two action records with actual34h bindings and nested14h modifier arrays, checks
padding and the entire fifth modifier DWORD, independently deep-copies and
reserves the records, and observes the borrowed listener reference count return
from3 to1 on teardown. Both scalar flags0 and flags1 retain the original return
identity. The real raw manager is explicitly unregistered before early owner
deletion, then drained. Normal listener slot0 is a strict unreachable boundary;
no recovered concrete listener destructor is claimed.

One deliberately injected listener-slot exception reaches the real reverse
record destruction and nested binding/modifier/word cleanup. The failing
record retains its listener slot, both nested counts become zero, and the
owner's second-provider failure clears publication/stamps `CE3818` without
freeing the outer action allocation or owner. The fixture then reclaims those
retained allocations explicitly. This validates the supported failure schedule;
the injected listener is not a native listener implementation.

`local/input_action_owner_nonempty_probe.log` records PASS for the complete
fixture. Primary provider review found and corrected one extra volatile read:
`A93500` loads `D7A24C` only for growth (`A93531` skips `A93533` on shrink/no-op).
The corrected primary library passed its strict build and both CTests before
the split fixture was linked. Application record producers, canonical frame
consumers, finite action-owner deletion dispatch and device/game validation
remain outside this packet.

## Correction from docs/NATIVE_SINGLETON_INPUT_ONLINE.md

The shared raw manager now admits the documented online/input profiles. Primary
archive-only fixtures exercised actual BD0400 drain, including nonempty backend
and action storage. See that document and reports/native_singleton_input_online.json
for the executed profiles, artifact hashes and provider/application boundaries.
Original packet validation above describes its earlier standalone state.
