# CC12 pending registration registry getter readiness

Baseline: `bab59dd4aa832fbf9a5c26b4397ac198cdec5f90`. This read-only packet owns
`00875280` and the present document/report. No Source implementation, native ABI
replacement, build, test, probe, ledger change or Ghidra mutation is admitted.
Machine evidence: [cc12_pending_registry_getter_readiness.json](../reports/cc12_pending_registry_getter_readiness.json).

## Result and limits

The complete ordinary getter is established: `00875280..0087533C`, **189 bytes,
51 instructions, seven physical CALLs** (five direct and two IAT). It passes the
300-byte size gate. Its actual mutable publication is **`00F878CC`**. The full
instruction trace and pinned service contracts establish **no explicit native
input arguments**; this conclusion does not depend on the saved empty prototype.

The caller `00875890` reaches `008758BA` with `ECX=payload` and `EAX=rawgroup`.
Neither is consumed by the getter. Entry EAX is immediately replaced with the old
`FS:[0]`; entry ECX/EDX are not used as ordinary data inputs. The first `00415350`
call is itself no-input by the pinned current Source contract. Other calls use
locally supplied arguments. The getter reads no caller stack argument and ends
with plain `RET`. Fast return EAX is the first captured publication; slow return
EAX is a later publication reload. Slow-path ESI is saved/restored; the body does
not modify EBX/EDI/EBP. Nested provider ABI guarantees remain inherited.

**Production Source binding remains held.** `00874BC0` must establish the raw8
constructor's returned identity, owner profile and section publication. The
getter's own handler/cleanup paths and the owner's retirement/lifetime contract
also remain open. No constructor, destructor, EH, full group bootstrap or caller
body was expanded in this packet.

## Complete ordinary ordering

| Instructions | Observed operation |
| --- | --- |
| `875280..87528E` | Save old `FS:[0]`, push state `FFFFFFFF` and handler `C96433`, install the frame. This also occurs on the fast path. |
| `875295..87529F` | Read `F878CC` into EAX. A nonzero capture goes directly to the common epilogue and returns that value. No manager lookup, section access, allocation or registration occurs. |
| `8752A6..8752B8` | Call `415350`; unconditionally dereference manager `+10h` into ESI. Store local guard profile `CE37FC` and that captured section. |
| `8752BC..8752C5` | If ESI is nonnull, call `EnterCriticalSection(ESI)`, then add one to DWORD `[ESI+18h]`, modulo 2^32. |
| `8752C9..8752D8` | Set full EH state zero and reread `F878CC`. If now nonnull, skip allocation, constructor, publication and registration. |
| `8752DA..8752E8` | Push eight, call `BF681B operator_new`, pop its argument, capture EAX allocation in local storage and test it. Allocation runs while state is zero. |
| `8752EA..8752FA` | Set the low state byte to one even on the null branch. For nonnull allocation, set ECX to it and call `874BC0`; retain returned EAX. Otherwise explicitly produce EAX zero. |
| `8752FC..875301` | Restore low state byte to zero **before** publishing EAX to `F878CC`. The getter does not compare the constructor result with the allocation or the old publication. |
| `875306..875314` | Call `415350` a second time, **then** reread `F878CC`, push that current object, set ECX to the second manager and call `BD0C30`. |
| `875319..875322` | If the original captured ESI is nonnull, subtract one from `[ESI+18h]` modulo 2^32, then call `LeaveCriticalSection(ESI)`. No manager/section re-fetch. |
| `875328..87533C` | On the slow path reload `F878CC` after release and restore ESI. Common epilogue restores the old exception chain and stack, then returns. |

The allocation is eight raw bytes. It is distinct from the 52-byte pending
registration node, the two pending sentinels and the fixed-step group headers.
The published value is **constructor return EAX**, not an independently retained
allocation pointer. Equality of those addresses requires the constructor body.

## Owner, manager and profile boundaries

The first manager's `+10h` section is the construction guard. It is distinct from
the registry owner's `+04` section subsequently consumed by `00875890`. This
getter never reads or writes owner `+00/+04`, allocates the owner's section, or
reads its method table. **`CE37FC` is the local guard profile; it is not evidence
of the registry owner's profile.** Owner initialization is delegated to `874BC0`
and completes before its returned EAX is published at `875301` on normal return.
The profile and internal section-store order remain unproved here.

`F878CC` must retain its own storage identity. The existing `F899E8`/`009248D0`
pending destroy/kill lock and `E198E0` observer lock cannot replace it merely
because they expose raw8 owners. `E0B6D0/E0B704` remain the pending registration
sentinel pair; `F876C0 + rawgroup*68h` remains the separate group-header selection.
This packet neither initializes nor substitutes those owners.

The first manager return is dereferenced without a null check; a null manager
is invalid at this boundary. A null captured section is allowed and suppresses
enter, depth updates and leave. A null allocation takes an explicit branch that
publishes zero and still performs the second manager lookup/registration. The
existing source allocator normally throws on failure; that does not erase the
observed branch from the native contract.

Reentrancy may change the manager/publication. Registration uses the publication
read **after** the second manager lookup; the returned value is read **after**
release of the first captured section. These values need not equal the newly
constructed result or each other. There is no identity check, CAS or duplicate
suppression in the getter. The first captured section must remain valid through
its normal release.

## Exception boundary

The ordinary instructions install handler `00C96433` and expose states `-1`, `0`
and `1`. State zero is armed after normal acquire/depth increment; allocation
occurs in state zero; its saved result precedes state one; state zero is restored
before publication and registration. This is frame/state evidence only.

No handler, unwind map, landing pad or constructor cleanup body was inspected.
Free-on-constructor-failure, release-on-exception, publication rollback/retention,
hardware-fault handling and original FH3/SEH behavior remain unproved. The nearby
`009248D0` Source implementation's cleanup semantics cannot establish them for
this getter. Automatically adding its RAII cleanup would exceed this evidence.

## Existing Source services and next boundary

The report pins current Source for the reusable services:

- `get_native_singleton_manager_00415350` and `SoundLifetimeAccess` raw mode can
  borrow the application's actual `01090AA0` manager cell. No private manager or
  semantic default is needed.
- `SoundLifetimeManagerView` can preserve second-getter/current-publication
  registration order. `CapturedSoundLifetimeSection` preserves the captured
  section and unsigned depth arithmetic, but its exception cleanup is reusable
  only after this getter's EH contract is recovered.
- `NativeObserverLockOwner` and `TrackedCriticalSection` provide existing raw8
  and raw1Ch projections; constructor evidence must first establish their use.
- `singleton_lifetime_allocate` provides source CRT raw allocation and
  `register_native_singleton_object_00bd0c30` provides the actual manager/object
  registration boundary. Original provider ABI/exception identities remain
  separate from these Source contracts.
- `get_native_pending_entity_lock_009248d0` is an analogous current implementation
  with distinct `F899E8`, constructor `924180`, profile and EH evidence. Its process
  singleton is not an actual `F878CC` binding.

Targeted searches of `include/` and `src/` (`.hpp/.cpp/.inc`) for `875280`,
`874bc0` and `f878cc` found no current implementation or binding at this baseline.
The next bounded evidence packet is a size-gated ordinary `00874BC0` constructor
audit. Getter EH, owner retirement and actual publication ownership remain
explicit dependencies. The shared pending owner, callable registration-node
profile, producer/consumer phase and independent group bootstrap remain held
from the prior audits. No new Source API is admitted here.

## Verification

Fresh target-verified CLI queries used existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32 image base `00400000`; live/snapshot function
counts both remained 64,729. All **193 fresh bytes** match the installed PE initial
image: 189 getter bytes are file-backed; four `F878CC` bytes are virtual zero-fill.
The saved zero cell does not establish a live runtime value. The complete PE hash
was rechecked as `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Independent Capstone decoding covers every body byte and confirms all 51
instructions and seven calls. IAT entries `CE2218` and `CE2210` resolve to real
`EnterCriticalSection` and `LeaveCriticalSection`. Sixteen current Source/prior
file pins, 29 inherited file pins and ten inherited native windows were checked.
Prior producer/splice evidence was reused without querying their bodies or the
group bootstrap again. JSON parsing and owned-file diff checks complete the
readiness audit; no build, fixture test, ABI probe or game run was performed.
