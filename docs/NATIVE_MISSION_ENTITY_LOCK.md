# Native mission/entity lock

Addresses: `004BD150`, `004B7ED0`, `004C1570`, `004C4860`, `004C4890`.

`native_mission_entity_lock.cpp` reconstructs the actual eight-byte owner published
at `00F878FC`. It reuses `NativeObserverLockOwner`: profile DWORD at `+00`, actual
malloc-backed `TrackedCriticalSection*` at `+04`. The section is `1Ch` bytes,
including the Win32 `CRITICAL_SECTION` and tracked depth at `+18`.

| Native routine | Original ABI | Coverage |
|---|---|---|
| `004BD150..004BD194` | ECX raw8; EAX same owner; RET | Complete source behavior, 69 bytes |
| `004B7ED0..004B7EE0` | ECX owner; RET | Complete, 17 bytes |
| `004C1570..004C162C` | No inputs; EAX owner; RET | Complete source behavior, 189 bytes |
| `004C4860..004C4882` | ECX owner; RET | Complete, 35 bytes |
| `004C4890..004C48C6` | ECX owner; stack DWORD flags; EAX captured owner; RET4 | Complete 55-byte PE sequence; stored membership excludes `004C48BE..004C48C0` |

These are new C++ interfaces. Original FH3/SEH dispatch, hardware-fault cleanup,
mutable native stack-slot aliases, library exception identities and arbitrary
callable native vtables are outside their ABI. Native names remain descriptive
hypotheses. Ghidra access in this worker is strictly read-only; root owns annotations.

## Owner production and deletion

The constructor stamps `CE7548`, calls canonical `BD1860`, and stores its returned
section at `+04` only after successful return. `CE7548` contains exactly one virtual
entry, `004C4890`. The next DWORD at `CE754C` begins a distinct profile stamped by
`004BF9A0` at `004BF9C3`; that constructor also writes byte `+44` and calls `006F7670`.
It is not another mission-lock slot. Both producers and the profile bytes are retained.

The nondeleting destructor stamps `CE7548`, calls canonical `0041CC80` on the actual
`+04` slot, unconditionally clears `F878FC`, then stamps base `CE3818`. The section
release captures its slot and section, drains positive recursive depth, deletes
the real section, frees it through the existing CRT provider and clears that slot.
The scalar entry performs the same sequence and frees the captured raw8 owner only
when `flags & 1`; it returns the captured address even after free. Neither destructor
unregisters, checks current publication identity, or acts on a replaced publication's owner.

The base cleanup `004B7ED0` only clears publication and stamps `CE3818`. It preserves
the section-slot preimage. Its constructor-unwind use must not release an uninitialized
or unrelated section pointer.

## Allocating getter and cleanup

The fast path captures the first `F878FC` read and returns it. The slow path calls
`00415350`, captures that manager's actual section `+10`, enters and increments it,
then rechecks `F878FC`. If still absent it allocates eight bytes, conditionally
constructs the owner, and publishes the result. It calls `00415350` again before
reloading `F878FC` for `BD0C30` registration. It then decrements and leaves the first
captured section and reloads publication for the return. Manager replacement does
not change which section is released. Publication replacement during section entry
can skip construction; replacement during leave changes the slow return.

| Call site | Target and established contract |
|---|---|
| `004BD17B` | `BD1860`: allocate raw1Ch, initialize the actual OS section, zero depth |
| `004C1596`, `004C15F6` | `00415350`: actual raw01090AA0 manager publication; complete existing allocating provider |
| `004C15AF` | IAT `CE2218`: EnterCriticalSection, one pointer argument |
| `004C15CC` | `BF681B`: raw8 allocation; `004C15D1 ADD ESP,4` |
| `004C15E3` | `004BD150`: construct the captured allocation |
| `004C1604` | `BD0C30`: ECX second manager, current owner stack argument, RET4 |
| `004C1612` | IAT `CE2210`: LeaveCriticalSection on the captured first section |
| `004C486C`, `004C489C` | `0041CC80`: actual section-slot destruction |
| `004C48B9` | `BF65AC`: captured owner free; `004C48BE ADD ESP,4` |
| `00C64D23` | Tail `004B7ED0`: constructor base cleanup |
| `00C64F83` | Tail `00411EE0`: captured guard cleanup |
| `00C64F8C` | `BF65AC`: captured raw8 free; `C64F91 POP ECX; C64F92 RET` |
| `00C64D2D`, `00C64F98` | Tail `BF6B43`: original FH3 selector, required external runtime |

The allocator and free are existing source-CRT boundaries; original CRT bodies
and FH3 are not ported. `SoundLifetimeAccess` is constructed only from the borrowed
actual raw manager cell. No semantic `SingletonLifetimeDomain` or private manager
is introduced. The underlying manager constructor produces real raw14h storage,
reserves 256 pointer slots and owns its actual section.

| Frame | FuncInfo / unwind map | State transition | Captured cleanup |
|---|---|---|---|
| Constructor | `D8D278` / `D8D270` | 0 → -1 | `C64D20` loads owner `[EBP-10]`, tail `4B7ED0` |
| Getter | `D8D5EC` / `D8D5DC` | 0 → -1 | `C64F80` addresses guard `[EBP-14]`, tail `411EE0` |
| Getter | `D8D5EC` / `D8D5E4` | 1 → 0 | `C64F88` loads allocation `[EBP-18]`, calls `BF65AC` |

Allocation failure before state1 only releases the captured guard. Constructor
failure clears publication/stamps base, frees the captured raw8 allocation, releases
the guard, and rethrows. Publication occurs after allocation state1 has ended.
Second-manager or registration failure retains the constructed publication and only
releases the first captured guard. No implicit rollback or extra static cleanup exists.

## Composition and evidence limits

`process_native_mission_entity_lock_00f878fc()` exposes one process-static pointer cell,
with explicit lifecycle. `get_process_native_mission_entity_lock_004c1570()` receives
the application's same raw01090AA0 manager cell. Explicit-publication overloads support
isolated reconstruction. Production routing must dispatch profile `CE7548` to
`delete_native_mission_entity_lock_004c4890(popped_owner, flags, process_cell)`, including
its unconditional clear when publication differs. Root owns this finite map change;
no host, runtime or shared destruction file is edited here. The native base-killed
provider can borrow the returned owner's existing `section_04` lvalue.

All five complete instruction schedules match the established pending-lock family
after resolving relative calls and mapping only own entries/blocks, profile,
publication and EH handler identities. They are not literal byte-identical clones.
`local/mission_lock_pending_comparison.json` retains every instruction comparison.
`local/mission_lock_spans.json` retains 15 spans, 595 live/PE-matched bytes, including
the complete five entries, funclets, FH3 data, profile, publication and adjacent producer.

The original-byte fixture executes all five entries with actual compiled raw manager,
registration, allocation, section creation/release and removal providers. Import
instrumentation calls real OS Enter/Leave while testing publication and manager
mutations at those boundaries. It checks fast reuse, locked doublecheck, second-manager
registration, captured first-section release, return reload, recursive depth2 destruction,
scalar flags100h/3, replaced-publication clearing and section-preimage base cleanup.
An allocating second manager lookup also changes publication from its actual section
initializer; registration reads that changed owner after the lookup returns.
Source-only allocation fault injection covers raw8, section and registration failures
using the real source allocator/new-handler behavior. Native FH3 records have unbound
handlers in the relocated fixture and no native exception is injected.

Two original continuation ranges currently have no stored containing function:
`004C48BE[3]` and `C64F91[2]`. Complete live/PE bytes establish their instructions.
Initial `C64D28[10]` and `C64F93[10]` snapshots report `no_ghidra_function`. Root
subsequently defined, named and exported these complete selectors. Final readback
confirms `EH_MissionEntityLock_Construct` and `EH_MissionEntityLock_Get`; all 15
direct/tail rows verify, with two unchecked IAT rows. The owned before/after
documentation differs only in two synthesized stack-8 local names/types inferred
after those handler definitions; exact old/new records are retained. Names, comments,
stored hashes and all other fields are unchanged. The worker made no Ghidra writes.
The fixture does not establish allocator-null returns, arbitrary concurrency, original
CRT/FH3 exception identity, process-map integration, binary replacement or gameplay.
Build, CTests, seeds, call verification and retained artifact hashes are recorded in
`reports/native_mission_entity_lock.json`.
