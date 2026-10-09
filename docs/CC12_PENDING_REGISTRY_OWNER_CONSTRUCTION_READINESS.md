# CC12 pending registry owner construction readiness

Baseline: `98fec2587fdac0a3c4d9fe90e5a30bc05c03a6f3`. This read-only packet owns
`00874BC0` and the present document/report. It adds no Source implementation,
ledger/CMake edit, Ghidra mutation, build, test, probe or Original credit.
Machine evidence: [cc12_pending_registry_owner_construction_readiness.json](../reports/cc12_pending_registry_owner_construction_readiness.json).

## Result and Source readiness

The complete ordinary constructor is established: `00874BC0..00874C04`, **69
bytes, 19 instructions, one direct CALL** to `00BD1860`. It passes the 300-byte
gate. Native **ECX is the actual raw8 receiver**, EAX returns that same captured
address, and the body ends with plain `RET`. The saved empty prototype is not
the physical input contract; assembly supplies the receiver and return evidence.

The body writes owner profile **`00D0DEA0`** before calling the actual tracked
section service. It writes that service's returned EAX to **owner `+04` only
after normal return**. It never pre-clears that field. This closes the prior
getter audit's allocation-versus-constructor-return uncertainty on ordinary
return: `00875280` passes its raw8 allocation as ECX and receives the same owner.
Its publication/reload ordering remains the separate pinned getter contract.

The ordinary store schedule and existing service edge are recoverable for a
future scoped Source implementation. **Production Source binding remains held**
on constructor/getter cleanup evidence, coherent owner retirement and section
lifetime, callable table delivery and actual `F878CC` publication ownership.
No Source API or owner alias is admitted by this audit.

## Physical receiver and exact ordering

| Instructions | Observed operation |
| --- | --- |
| `874BC0..874BCE` | Push initial state `FFFFFFFF` and handler `C963E8`; save and install the old `FS:[0]` exception chain. Entry EAX is overwritten. |
| `874BD5..874BD9` | Push ECX as a local spill, save ESI, copy the ECX receiver to ESI, then explicitly save it at local `[ESP+04]`. |
| `874BDD` | Set full EH state zero at `[ESP+10h]`, **before** the owner profile store and service call. |
| `874BE5` | Store DWORD `D0DEA0` to owner `+00`. No base-profile store or owner-field read precedes it. |
| `874BEB` | Call `BD1860` with no explicit arguments. The constructor has not written owner `+04`. |
| `874BF0` | Load the saved old exception-chain pointer into ECX; the constructor's frame is still installed. |
| `874BF4` | Store service return EAX to owner `+04`, unconditionally, including a zero result. |
| `874BF7..874C04` | Set EAX to captured ESI receiver, restore caller ESI, restore `FS:[0]`, remove 16 local bytes and return. |

The two owner stores cover exactly `+00..+07` on normal return. The pinned getter
allocates exactly eight bytes, so raw8 is supported by the allocation contract
as well as this accessed range. No EDX or caller stack argument is consumed.
ESI is saved/restored; the body does not modify EBX/EDI/EBP. The reused section
provider preserves ESI in its pinned current Source body.

There is no receiver validation, old-section read/release or live-owner reset.
Valid writable raw8 storage is required. Owner `+04` retains its preimage from
this constructor's perspective until the section call returns; it is not
temporarily initialized to null. External callbacks and exception cleanup are
separate effects. Reconstructing this as a reset of an existing lock owner would
invent a release policy.

## Actual section-service identity

The native direct edge is `874BEB -> BD1860`. Existing current Source already
provides `create_native_tracked_critical_section_00bd1860` in
`native_renderer_worker_lifetime.hpp/.cpp`. The prior service audit records its
no-argument, EAX-result, plain-RET contract. This packet pins that declaration,
definition, service audit and relevant allocator/layout files; it does not
query or expand the native descendant body.

The pinned Source provider performs this established service sequence:

1. Request exactly `1Ch` raw bytes through its fixed `allocate_worker_lock`
   helper and `singleton_lifetime_allocate({critical_section, bytes, bytes})`.
   Both native and host sizes are `1Ch`.
2. Use the existing current CRT malloc/new-handler/retry/`bad_alloc` boundary.
   It is not the old random-thread `nothrow` creation wrapper or a larger
   semantic critical-section projection.
3. For a nonnull result, call real `InitializeCriticalSection`. Only after its
   normal return, write DWORD depth `+18h = 0` and return the raw section. The
   explicit null-allocation branch returns zero.

The current `TrackedCriticalSection` assertions establish size `1Ch` and depth
offset `18h`. The provider adds no allocation cleanup if OS initialization
raises nonlocally. This reuses the actual named service and its current Source
contract; layout compatibility alone is not the basis for ownership. Native
CRT objects, original heap ABI, provider exception identities and hardware-fault
equivalence remain outside that Source boundary.

The constructor's own instructions do not enter the section or update its depth.
Its section at owner `+04` is distinct from the getter's first manager `+10h` guard.
A normal null service result is stored as null, and the constructor still
returns the receiver; no assertion or alternative path is present.

## Failure, table and publication boundaries

State zero is armed before the `D0DEA0` profile write, and there is no later
state update in this ordinary body. When `BD1860` is called, the profile has
already been written while owner `+04` has not. If the service exits by an
exception, the ordinary `+04` store is not reached. Subsequent profile, section
or publication effects depend on handler `00C963E8` and its cleanup paths, which
were not inspected. Null/invalid receiver faults, mutable EH-spill aliases and
original FH3/hardware-fault dispatch likewise remain unproved.

Do not infer an unwind that clears `F878CC`, restamps a base profile, frees a
section/allocation or registers/unregisters the owner. Cleanup from a same-layout
observer or pending-entity lock cannot establish this constructor's behavior.

The ordinary constructor neither reads nor writes `F878CC`. That publication
belongs to the getter after ordinary construction returns. No private/default
publication, substitute lock owner or private lifetime manager is introduced.
`D0DEA0` is an encoded native profile identity. No table contents or target body
was queried, so callable Source table delivery and scalar retirement routing
remain explicit dependencies.

The next bounded evidence is constructor handler `C963E8` and its specifically
owned cleanup boundary, with a size gate before expansion. Getter handler
`C96433`, owner retirement and canonical publication binding remain separate.
The prior common pending storage, callable registration-node profile,
producer/consumer phase and independent group bootstrap holds remain in force.

## Verification and scope

Fresh target-verified queries used existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32 image base `00400000`. Live and saved total
function counts both remained 64,729. **All 69 fresh file-backed body bytes**
match the installed PE. Body SHA-256 is
`dbd4459a91dddfa42be5d17021ea441644245ea4f24c04bbf5f2167bcbdfbba6`;
the complete PE hash was rechecked as
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Independent Capstone decoding covers all 69 bytes and confirms 19 instructions
and one call. Nine current Source/prior file pins and sixteen inherited getter
pins pass. The getter's two existing byte windows were compared with the PE
without fresh getter/caller queries. Targeted `.hpp/.cpp/.inc` searches found no
current `874bc0`, `f878cc` or `d0dea0` implementation in `include/` or `src/`.

JSON parsing and owned-file diff checks complete this audit. No descendant,
FH3, retirement, getter/caller or group-bootstrap sweep occurred. No build,
fixture test, executable probe, runtime run or new Original credit is claimed.
