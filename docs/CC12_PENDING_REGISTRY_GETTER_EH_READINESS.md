# CC12 pending registry getter unwind readiness

Baseline: `2e6a351a0225d06a67d6a8099e7a70d8f2861852`, synchronized through
published Main `86337b8ae9008cb28d2dd2f56ddcacc14de6ee3b`. This read-only packet
independently captures the getter's handler, record, map and two direct actions.
Evidence: [cc12_pending_registry_getter_EH_readiness.json](../reports/cc12_pending_registry_getter_EH_readiness.json).

## Result and scope

The getter's encoded next-state chain is **allocation cleanup at state one,
guard cleanup at state zero, then state `-1`**. The allocation action reads the
current saved-allocation slot; the guard action passes the address of the local
guard, whose current section is read by the concrete cleanup service. Neither
action substitutes the registry's current publication for these frame operands.

This establishes encoded actions and the qualified ordinary cleanup schedule,
not the shared runtime's recognition, frame setup, state-update timing, nested
failure or hardware-fault behavior. No Source getter/exception wrapper or new
credit is admitted. All requested size limits passed.

## Independent handler, record and map

At `C96433`, ten code bytes execute `MOV EAX,DC853C`; `JMP BF6B43`. Ghidra has
no function starting there, and no listing repair was performed. The handler
replaces EAX but does not set ECX/EDX/EBP/ESP or push a return address. The shared
`BF6B43` body and its ambient runtime inputs remain outside this packet.

The initial bounded 16-byte window also contains three `CC` padding bytes at
`C9643D..3F` and three adjacent opaque bytes at `C96440..42`. Those final three
bytes were not decoded or used as another function's evidence. The handler
capture remains below the 32-byte limit.

The independent 36-byte absolute x86 FuncInfo at **`DC853C`** contains magic
`19930522`, zero upper BBT bits, maxState `2`, map pointer **`DC852C`**, zero
try/IP/type-list fields and encoded EHFlags `1`. The current pinned MSVC
`ehdata.h` corroborates the layout; its version is not evidence of original
compiler/runtime or fault-policy compatibility.

| State index | Encoded next state | Action | Complete ordinary action |
| --- | --- | --- | --- |
| 0 | -1 | `C96420` | `LEA ECX,[EBP-14h]`; tail `JMP 411EE0` |
| 1 | 0 | `C96428` | `MOV EAX,[EBP-18h]`; `PUSH EAX`; `CALL BF65AC`; `POP ECX`; `RET` |

The map is 16 bytes/two entries, below the four-entry limit. Its state/action
pairs are fresh getter evidence; constructor EH records were not imported as
proof of these fields or actions.

The guard action is eight bytes/two instructions. It passes a guard **address**,
not a section value, and leaves the incoming stack unchanged at its tail jump.
The allocation action's complete returning path is eleven bytes/five
instructions. Its saved Ghidra body ends after the CALL at `C96430`, covering
only nine bytes. Fresh GPR bytes and the PE independently contain `59 C3` at
`C96431..32`: `POP ECX; RET`. The full 11-byte span was decoded without changing
function membership or any no-return setting.

The allocation action rereads its frame slot into EAX and pushes that value as
the free service's cdecl argument. If the child returns, `POP ECX` reclaims the
argument word before `RET`. EAX afterward is whatever the child leaves; no
semantic action result is established. Both actions require the runtime's
EBP/stack contract. The free body and guard body were not queried natively.

## Parent frame and state timing

Let `S` be ESP on entry to the pinned complete 189-byte `875280` getter.
Following three frame pushes, `SUB ESP,0Ch` and slow-path `PUSH ESI`, ESP is
`S-1Ch`. Its stored fields are:

| Slot | Address | Parent evidence |
| --- | --- | --- |
| State | `S-04h` | Slow-path `[ESP+18h]` |
| Handler | `S-08h` | Pushed `C96433` |
| Previous exception chain | `S-0Ch` | Pushed old `FS:[0]` |
| Guard section | `S-10h` | Captured first manager's section at `[ESP+0Ch]` |
| Guard profile | `S-14h` | `CE37FC` at `[ESP+08h]` |
| Allocation spill | `S-18h` | Returned allocator EAX stored at `[ESP+04h]` |
| Saved caller ESI | `S-1Ch` | Slow-path push |

If the shared helper supplies **EBP = S**, both action offsets match these
slots. The ordinary getter never sets EBP; the matching offsets corroborate
that expected relation but do not prove the unexpanded helper ABI. The actions
use current mutable frame storage, not immutable copies or a fresh manager or
publication lookup.

The ordinary state schedule is precise:

1. Initial state `-1` remains through guard initialization, optional
   `EnterCriticalSection` and depth increment. The fast path never arms state zero.
2. `8752C9` arms full state zero after those ordinary operations, including the
   null-section path, and before double-check/allocator execution.
3. Allocation runs in state zero. Its returned pointer is stored at `8752E4`
   before the low state byte becomes one at `8752EA`.
4. State one covers the nonnull constructor call; it is also briefly written on
   the null-allocation branch. `8752FC` restores state zero **before** owner
   publication, the second manager lookup and current-publication registration.
5. Normal depth decrement/Leave occurs while state zero remains armed. There is
   no earlier state `-1` store before release or the final publication reload.

Do not add an early disarm or infer a particular repeated cleanup/fault outcome
from that final state. The shared runtime's state publication versus action
execution order remains a qualification.

## Existing concrete Source and qualified ordering

Current `destroy_native_singleton_guard_00411ee0` in
`native_diagnostic_sink_lifetime.hpp/.cpp` captures guard `+04` **before**
stamping guard profile `CE37FC`. For a nonnull captured section it decrements
the unsigned depth at `+18h` once, then calls real `LeaveCriticalSection`.
It preserves guard `+04` and does not destroy or free the section. This is the
actual raw8 guard/raw Win32 section service, not a semantic projection.

Current `singleton_lifetime_free` in `singleton_lifetime.hpp/.cpp` supplies
`std::free` for the existing Source CRT allocation domain. The pinned renderer
lifetime evidence identifies the `BF65AC -> BF9DC8` free boundary. Native free
internals, original heap/CRT identities and fault behavior remain unexpanded.

Under the qualified dispatcher/frame contract and normally returning actions,
state one frees the **current allocation spill first**, then continues to the
state-zero guard cleanup. State zero alone does not select allocation free.
In particular, allocation failure occurs before state one is armed; publication
and registration happen after state zero is restored. These getter actions do
not restore an earlier publication or free whichever owner currently occupies
`F878CC`.

For a constructor failure, separately pinned constructor/base-cleanup evidence
supplies the conditional inner sequence: clear `F878CC`, stamp receiver base
profile, then outer saved-allocation free followed by guard cleanup. This
composition assumes the shared unwinder follows the encoded frame/state chain
and each cleanup returns. It does not guarantee dispatch or successful cleanup;
if free or another action fails nonlocally, later actions are not proved to run.

## Refreshed base-cleanup Source admission

The explicit-cell `8748F0` Source leaf is now published and admitted for its
ordinary Source contract. The current pinned
[primary review](../reports/cc12_native_pending_registry_base_cleanup_primary_review.json)
records the entire 25-byte/nine-instruction leaf, entire 14-byte/six-instruction
concrete child, two positive unique Core members and the successful MSVC Win32
build with three existing checks. Its seven current input pins and updated
document hash were replayed here.

The older worker report still records its historical pending status; this
primary report supersedes that admission status. The leaf still does not supply
the Native ECX/FH3 entry, genuine cell/owner producer, callable profile or runtime
integration. This getter EH audit adds no credit to the primary's recorded
admission and does not implement constructor/getter composition or retirement.

## Verification

Fresh queries verified existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32 image base `00400000`; live and saved total
function counts both remained 64,729. **All 87 captured file-backed bytes** match
the installed PE: 81 semantic code/data bytes, three padding bytes and three
excluded adjacent opaque bytes. The 29 semantic code bytes decode to nine
instructions: one CALL, two tail JMPs and one RET.

The full PE hash remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Eighteen current Source/prior file pins, sixteen inherited getter pins, seven
primary input pins, eight prior PE windows and the SDK pin pass. The prior
windows were replayed from accepted evidence without fresh live queries.

JSON and owned-file diff checks complete this audit. No Source, CMake, ledger,
Ghidra listing/annotation mutation, native descendant/shared-runtime expansion,
other handler/caller/profile/bootstrap sweep, worker build, test, probe, runtime
run or new Original credit occurred.
