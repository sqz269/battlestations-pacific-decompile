# Application ordinary retirement: 00BEA8B0

The complete normal body at `00BEA8B0` is established: **153 bytes, 40
instructions**. It retires the **current Application publication**, which may
differ from the receiver. It removes that current pointer using a second manager
lookup, clears the publication, releases the lock captured from the first
manager, and finally changes the original receiver to the singleton-base table.

The existing raw manager getter and removal Source are reusable at the observed
call edges. This audit supplies no Application Source wrapper, actual owner,
callable table, native exceptional cleanup, or new reconstruction credit.

The [report](../reports/cc12_application_ordinary_retirement_readiness.json)
retains the complete bytes and listing, current PE comparisons, Source and prior
audit pins, frozen compiler-input identity checks, and remaining boundaries.
The starting main was `bab59dd4aa832fbf9a5c26b4397ac198cdec5f90`.

## Scope and target

Live queries verified the existing `bsp` project and
`/battlestationspacific.exe`, configured in `C:/Users/sqz269/bsp.gpr`. Function
count remained **64729**, matching the snapshot. `BEA8B0..BEA948` passed the
requested 300-byte gate before its complete body was inspected.

This was the only fresh function queried. Native service descendants, other
callers, FH3/funclets and Application tables were not expanded. The two indirect
call operands were resolved through existing PE import metadata, adding no
fresh table-byte capture. The saved placeholder prototype omits `ECX`; physical
instructions establish the actual receiver input and normal `RET0`.

## Actual normal schedule

Let `A` be the incoming actual receiver, `G` the mutable `E1AE90` publication,
`M1` and `M2` the two independently returned managers, and `S` the section
captured from `M1+10h`.

| Site | Operation |
| --- | --- |
| `BEA8B0..BEA8CC` | Install SEH frame, save registers, capture actual `A=ECX` in EDI and a local spill. |
| `BEA8D0`, `BEA8D6` | Write `D68BC4` to actual `A+0`, then set local state DWORD to zero. |
| `BEA8DE`, `BEA8E3` | Get `M1`; capture `S=[M1+10h]` in ESI. There is no manager-null check before this load. |
| `BEA8E8..BEA8F0` | Materialize the actual local guard words `CE37FC` and captured `S`. |
| `BEA8F4..BEA8FD` | If `S` is nonnull, enter it, then increment its DWORD `+18h` after Enter returns. |
| `BEA901` | Set the low state byte to one, including the null-section path. |
| `BEA906`, `BEA90B` | Get `M2`; **after that getter returns**, load current `G` into ECX. |
| `BEA911..BEA914` | Push that current pointer `P`, set `ECX=M2`, and call `BCFCA0(M2,P)`. |
| `BEA919` | After removal returns, unconditionally store literal zero to `G`. |
| `BEA923..BEA92C` | If captured `S` is nonnull, decrement its current DWORD `+18h`, then leave that same section. |
| `BEA932..BEA948` | After Leave returns, or on the null-section path, write `CE3818` to actual `A+0`, restore SEH/registers/stack and execute `RET0`. |

The import operands are `CE2218 = KERNEL32!EnterCriticalSection` and
`CE2210 = KERNEL32!LeaveCriticalSection`. The tracked-word adjustments are
ordinary DWORD ADDs; no clamping, stronger synchronization or replacement lock
is introduced. No semantic EAX result is assigned by this body.

## Receiver, current values, and lifetime

The body never compares `A` with `P`. If they differ, it removes `P`, clears `G`,
and changes `A`'s table. Replacing the removal operand with the destructed
receiver would change observed behavior.

The two manager results likewise have no identity check. The lock remains the
section captured from `M1`, even when the second getter returns a different
manager. Removal uses `M2`. Neither a cached manager nor a newly fetched lock
preserves this schedule.

There is no early exit when `G` is null. Both manager gets, optional locking,
the removal call, literal-zero store and final receiver-table write still
occur. The existing raw removal Source separately handles a null object by
touching no manager fields; this does not remove the parent's unguarded
`[M1+10h]` access.

The publication clear has no identity condition or reload after removal.
If removal changes `G` and returns, the next literal-zero store overwrites its
then-current value. After the clear, the body decrements and leaves captured
`S` before changing `A` to `CE3818`.

There is no local virtual dispatch, payload destructor, refcount decrement,
outer free or duplicate suppression. `D68BC4` remains the actual receiver's
profile during the manager/removal/lock operations; the final store uses
`CE3818`. These native identity words do not provide callable Source vtables.

## Existing Source dependencies and qualification

| Observed edge | Concrete existing Source | Established reuse boundary |
| --- | --- | --- |
| `BEA8DE` and `BEA906` to `415350` | `get_native_singleton_manager_00415350` | Borrow the actual mutable manager publication; preserve separate current reads and the existing canonical allocation/constructor failure behavior. |
| `BEA914` to `BCFCA0` | `unregister_native_singleton_object_00bcfca0` | Raw manager in ECX and current pointer on the stack. Clear only the first matching slot, preserving length, capacity and later duplicates; no destruction or locking. |
| `BEA8F7` and `BEA92C` through imports | Direct SDK Enter/Leave calls | Existing forwarding Source calls the real APIs. The caller owns the extra `+18h` adjustments and captured-section lifetime. |

The current getter/removal Source and headers canonically match eight retained
source/compiler-input copies across two verified build manifests. The removal
CPP's physical difference is only CRLF versus LF; its normalized content is
identical. This checks reuse identity, without rerunning those historical
builds, archive proofs or tests.

The actual manager publication already has one stable Source owner:
`GameNativeStringProcess::manager_01090aa0()`. `GameSingletonHost` borrows that
same cell. Existing `BD0960` Source publishes a real tracked section at raw
manager `+10h`. A detached manager cell, typed projection, or new private lock
would not establish these operands.

Service descendants remain external contracts. The getter uses a Source
reference argument and Source allocation/exception domain; removal retains its
actual SDK CRT validation behavior and native compatibility limits. Existing
generic game-singleton deletion code has extra publication and scalar-deletion
policies, so only its concrete service calls are applicable here.

## Exceptional and owner boundaries

The native body installs a handler pointer `CC7260`, records state zero, forms
the local `CE37FC/S` guard, and records state one after optional lock entry.
These are physical observations. No handler or funclet was queried. Native
unwinding, fault behavior, guard cleanup and partial retirement remain held.
The normal path does not authorize an invented exception unlock, publication
rollback, final-table write, automatic free or replay.

The ordinary normal path can now be specified from complete body evidence and
concrete service edges. A later Source implementation still needs explicit
actual bindings and a qualified failure-retention policy. It cannot claim the
native FH3 contract from this audit.

Current Source has no address-labelled `BEA8B0` or `735F30` implementation and
no `CFEAB0`, `D68BC4` or `D68BC8` profile match in the bounded source search.
`GameStartupHost` still constructs projected frame bookkeeping and clears a
flag on destruction; its locale integration explicitly records the absence of
an actual `E1AE90` owner. The raw dispatcher already handles `CE3818`, which
does not admit the missing Application profiles or owner by itself.

The prior [scalar-table audit](CC12_APPLICATION_SCALAR_TABLE_ENTRY_READINESS.md)
remains qualified: `CFEAB0` reaches ordinary `7379A0`; `D68BC4` reaches this
ordinary body; `D68BC8` reaches a scalar that writes `D68BC8` and calls this body
directly. Each scalar reads the deletion bit only after its child returns.
This packet adds no table read or scalar proof, and does not turn the observed
WinMain stack owner into a heap owner.

## Remaining prerequisite and validation

The single next bounded prerequisite is **`00735F30`**, called by the already
captured outer `7379A0` destructor on actual receiver+8 with argument zero,
before vector-data free and base retirement. Its `7379CD` call bytes were reused
and checked against the installed image. No target body, size, behavior or
descendant is inferred; a separate packet should start with lookup, current
Source and its agreed size gate.

All 153 freshly captured bytes decode completely to 40 instructions and match
the installed PE. The reused five-byte outer call also matches. Thirteen Source
pins, two target guards, prior audit pins and bounded excerpts were verified.
Every canonical input pin from the three reused Application audit reports was
replayed, and the two frozen service manifests/eight artifacts were checked.
Only this document and its JSON report changed; no Source, CMake, ledger or GPR
mutation, build, test, probe, native execution or new credit occurred.
