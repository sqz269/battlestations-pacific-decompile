# Native pending entity lock

Addresses: `00924180`, `009248D0`, `00923660`, `009256A0`, `009256D0`,
`00CA6AE0`, `00CA6B60`, `00CA6B68`.

The pending destroy/kill list consumers share the actual eight-byte owner
published through `00F899E8`. The owner reuses `NativeObserverLockOwner`:
profile DWORD at `+00`, pointer to an actual malloc-backed 1Ch
`TrackedCriticalSection` at `+04`. Construction writes profile `00D190C4`.
That profile contains **one** entry, `009256D0`; the next DWORD, `00D190C8`,
starts the distinct `00F899EC` owner's profile with `00925740`.

`src/native_pending_entity_lock.cpp` borrows the application's raw `01090AA0`
manager publication through `SoundLifetimeAccess` in its actual-storage mode.
It uses the existing raw manager, registration, CRT allocator/free, and
canonical `create_native_tracked_critical_section_00bd1860` /
`release_native_tracked_critical_section_0041cc80` providers. It introduces no
semantic lifetime domain, allocator port, lock callback or private manager.

| Routine | Native ABI | Coverage |
| --- | --- | --- |
| `00924180[69]` construct | ECX raw8; EAX captured owner; RET | complete source behavior |
| `009248D0[189]` getter | no inputs; EAX owner; RET | complete source behavior |
| `00923660[17]` base unwind | ECX owner; RET | complete |
| `009256A0[35]` nondeleting destruction | ECX owner; RET | complete |
| `009256D0[55]` scalar destruction | ECX owner; flags DWORD on stack; EAX captured owner; RET4 | complete full PE span; saved body has a gap |
| `00CA6AE0[8]` constructor cleanup | original EBP frame; tail JMP `923660` | effects in constructor catch; original funclet ABI excluded |
| `00CA6B60[8]` getter guard cleanup | original EBP frame; tail JMP `411EE0` | section effects through borrowed guard; original funclet ABI excluded |
| `00CA6B68[11]` getter allocation cleanup | original EBP frame; free `[EBP-18]`; POP ECX; RET | effects in getter catch; original funclet ABI excluded |

The source functions have explicit publication-reference arguments and a new
C++ ABI. Native profile DWORDs remain identities, not callable C++ vtables.
`process_native_pending_entity_lock_00f899e8()` exposes one zero-initialized
process cell; `get_process_native_pending_entity_lock_009248d0(raw_manager)`
uses that same cell. Its explicit lifecycle requires manager shutdown to pass
the popped owner and flags to the scalar function with this process cell.
There is no automatic destructor or shutdown registration on the accessor.
Owners created against other explicit cells require their own matching
dispatch binding. The process map integration is a separate integrator edit.

## Ordering and exception evidence

The getter's first publication load at `9248E5` is returned directly when
nonnull. On the slow path, `9248F6` resolves the first manager and `9248FB`
captures its section `+10`; entry increments that same raw section's `+18`.
The second publication test is at `924921`. The getter allocates eight bytes
before arming allocation cleanup, constructs, disarms that cleanup, and
publishes at `924951`. It resolves the manager again at `924956`, reloads the
current owner at `92495B`, and registers through `BD0C30`. Normal release
decrements/leaves the first captured section and the return reload is at
`924978`. Registration failure retains the allocation and publication.

Constructor state0 is armed before the `D190C4` store and `BD1860` call.
`DD9CDC` records state0 -> -1 through `CA6AE0`, which reads the captured owner
from `[EBP-10]` and jumps to `923660`. This clears publication before the base
profile `CE3818` store and leaves the section preimage untouched. The getter
map at `DD9D9C` records state0 -> -1 through `CA6B60` and state1 -> 0 through
`CA6B68`. Thus constructor failure performs base cleanup, frees captured raw8,
and finally releases the captured manager section. The source catches preserve
these observable owner/section effects for C++ exceptions. Native FH3/SEH,
hardware faults, mutable spill aliases and the original CRT exception domain
are outside the source contract.

Nondeleting and scalar destruction first stamp `D190C4`, then release the
actual section slot with `41CC80`. The canonical provider drains positive
signed recursion depth, calls actual Win32 deletion and CRT free, then clears
the owner slot. Both destructors unconditionally clear the supplied
publication and stamp `CE3818`; neither unregisters. Scalar deletion tests
only flags bit0, optionally frees the captured owner, and returns that address
even after free. Destruction requires an owning/quiescent thread and valid
storage. Native null-section behavior is preserved.

## Calls and storage provenance

Each provider body was read before assigning its contract; the machine-checkable
call-site rows and original cleanup instructions are in the companion report.

| Containing function | Site | Target / contract |
| --- | --- | --- |
| `924180` | `9241AB` | `BD1860`: raw1Ch allocation, actual InitializeCriticalSection, depth0 |
| `9248D0` | `9248F6`, `924956` | `415350`: first-read lazy raw14h manager publication |
| `9248D0` | `92492C` | `BF681B`: allocation size8; `924931 ADD ESP,4` |
| `9248D0` | `924943` | `924180`: construct captured allocation |
| `9248D0` | `924964` | `BD0C30`: append current nonnull owner, no internal lock; RET4 |
| `9256A0` | `9256AC` | `41CC80`: release actual owner+04 slot |
| `9256D0` | `9256DC`, `9256F9` | `41CC80` release; `BF65AC` free captured owner; `9256FE ADD ESP,4` |
| `CA6AE0` | `CA6AE3` | tail JMP `923660`: base cleanup |
| `CA6B60` | `CA6B63` | tail JMP `411EE0`: captured raw guard release |
| `CA6B68` | `CA6B6C` | `BF65AC` free captured allocation; `CA6B71 POP ECX; CA6B72 RET` |

The getter's complete direct caller set is `925A06` cancel-pending-destroy,
`926C9C` pending destroy, and `926DBE` pending kill. Each consumes the returned
owner's section+04. No caller supplies a native getter argument. The constructor
has only `924943`; scalar deletion is referenced by the single table slot.

## Verification and limits

Thirteen spans totaling 516 bytes, including all five owner functions,
three funclets, two handler stubs and the profile/EH records, match both live
saved-project bytes and the installed executable. The handler stubs are now
defined ten-byte functions. Ghidra still reports no containing function for
`9256FE[3]` and `CA6B71[2]` after the integrator's formal flow repair. The full
PE continuations, rather than an expanded stored function body, support those
instructions. No claim is made that all stored body boundaries were repaired.

Validation uses the required strict MSVC Win32 build and both existing CTests.
The isolated local probe links the actual worker `bsp_core.lib` and executes
the five original PE spans against the same real raw manager, allocation,
registration and critical-section providers. It covers slow/fast lookup,
registration, constructor return, recursive section drain, flags0x100 and3,
replacement-publication clearing, nondeleting destruction, and direct base
cleanup preserving the section preimage. The source side additionally checks
the process wrapper's publication identity. The fixture relocates call and
publication operands, retains native profile identities, and leaves native
exception handlers unbound; it does not inject or validate exception paths.
There is no new persistent test target. Exact inputs, commands, linked objects,
toolchain, hashes, output and any failures are retained in `local/` and
summarized in `reports/native_pending_entity_lock.json`.

The strict Win32 build, both existing CTests, all 15 direct call-site checks,
and both source/native fixture runs passed. This packet has no game-runtime or native exception-dispatch
validation, no mixed-owner shutdown claim, and no producer/drain reconstruction.
