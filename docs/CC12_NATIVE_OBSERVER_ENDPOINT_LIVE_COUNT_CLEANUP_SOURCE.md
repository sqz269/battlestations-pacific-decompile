# CC12 native observer endpoint live-count cleanup Source candidate

This candidate reconstructs the complete ordered schedule of
`007EE620..007EE665` (70 bytes, 24 instructions) through a new naked MSVC Win32
fastcall interface. It retains the raw receiver and selected entity pointer,
and calls the existing `native_pending_entity_kill_00926d90` with explicit
actual pending-owner and producer-access references.

The candidate is **unregistered and uncompiled**. It adds no Source admission,
Original ABI compatibility, startup or gameplay credit. Baseline:
`bee819237cd14aa4a0ba1931286386d9f439ae9e`.

The four owned files are this document, the
[header](../include/bsp/native_observer_endpoint_live_count_cleanup.hpp),
[Source](../src/native_observer_endpoint_live_count_cleanup.cpp), and
[report](../reports/cc12_native_observer_endpoint_live_count_cleanup_source.json).
No CMake, Ghidra, ledger, consumer, storage domain, tests or probes are added.
`00653390` and the separate EH routes are outside this candidate.

## Evidence and mapping

Fresh owned GPR bytes equal the full file-backed original PE span and the
Root Astra approved whole-body capture:

| Item | Value |
| --- | --- |
| Owned Native body | `007EE620..007EE665`, 70 bytes, 24 instructions |
| Exact body SHA-256 | `982a339db60ea4a42c68d12ee902902bca4d6e8b355132ed8faef1e0f328dd4f` |
| PE section / RVA / file offset | `.text` / `003EE620` / `003EE620` |
| Original image SHA-256 | `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` |
| Source text mapping | 24 mapped Native positions plus 4 inserted instructions |
| Source graph | 8 blocks, 11 edges; all 28 instruction stack positions and merges checked |

The existing project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was verified before the fresh owned queries; live
and snapshot counts remain 64,729. The accepted
[endpoint audit](CC12_OBSERVER_ENDPOINT_CLEANUP_ABI_READINESS.md) supplies the
complete 70-byte contract, separately pinned as prior evidence. The other
88-byte body and subordinate `00926D90` Native body were not reopened.

The mapping compares each Native opcode/operand or branch target with the
corresponding Source assembly statement. The intentional differences are the
actual Source cdecl call, four added argument/cleanup instructions, and RET8.
This is a Source text and symbolic-stack check, with no compiled encoding,
COFF relocation, library selection or execution claim.

## New interface and explicit argument timing

`cleanup_native_observer_endpoint_live_count_007ee620` returns void and takes
actual raw receiver R in ECX, an unused DWORD placement parameter in EDX, and
two stack references: `NativePendingEntityOwners&` and
`NativePendingEntityProducerAccess&`. It introduces no endpoint/entity class,
proxy storage, facade or resolver implementation. The header requires MSVC
Win32, and the Source asserts four-byte pointers and DWORDs.

Let S be Source entry ESP. `[S]` is its return address, `[S+4]` the actual owners
reference word and `[S+8]` the actual access reference word. EBP/ESI and the
conditional EDI save retain the Native schedule; no extra register save is
introduced. The selected entity is passed directly from ECX as `void*`.

Only an eligible iteration reads the added reference slots. After both Native
predicates pass, active ESP is `S-0Ch`:

| Source operation | Scheduled input | Stack after operation |
| --- | --- | --- |
| `PUSH DWORD[ESP+14h]` | Current access word at entry `[S+8]` | `S-10h` |
| `PUSH 1` | Original full DWORD cause one | `S-14h` |
| `PUSH ECX` | Actual selected entity pointer | `S-18h` |
| `PUSH DWORD[ESP+1Ch]` | Current owners word at entry `[S+4]` | `S-1Ch` |
| `CALL native_pending_entity_kill_00926d90` | Cdecl argument tuple described below | Child entry `S-20h` |
| Child return, then `ADD ESP,10h` | Required cdecl zero-argument-cleanup return | `S-0Ch` |

At the child entry, stack offsets +4/+8/+0Ch/+10h contain owners, actual entity,
raw cause one and access. The access reference is captured before the original
cause PUSH; owners is captured after the cause and actual-pointer pushes.
They are not cached in a register or local before the loop. A later eligible
iteration reads the current argument backing again, after any prior child
effects.

The Native call entered at `S-14h` with only cause one and required RET4. The
Source cdecl call has four stack arguments, requires a zero-cleanup return and
adds 10h in the caller. Extra loads and pushes change the cause/return positions,
fault opportunities and possible stack aliases. Actual objects and argument,
save, return and selected-memory backing must remain valid at the documented
points. Universal Native stack-alias equivalence is not claimed.

On its own return, Source RET8 consumes the two added reference words and
leaves ESP=`S+0Ch`. The Native plain RET left `S+4`. The initial nonpositive-count
path reads neither added reference slot, but still executes Source RET8. The
C++ interface nevertheless requires valid borrowed references from its caller.

## Retained raw schedule

The body saves EBP and ESI, captures ECX into EBP, clears ESI and compares the
current signed DWORD at receiver+3CCh against zero. JLE skips all iteration
work. Only a positive initial count saves EDI and computes the inline entry
address receiver+3D0h.

Each iteration loads the current entry pointer from `[EDI]`. It reads byte
entry+5Ch first; only when zero does it read DWORD entry+900h. The actual kill
provider runs with cause one only when that DWORD equals one. There is no null
guard, container conversion, predicate revalidation, count snapshot or array
length clamp.

After the Source call and its explicit argument cleanup, the original ESI
increment, EDI increment by four and fresh signed count load remain in order.
JL uses that live count, so callback changes can shorten or lengthen the walk
relative to the initial count. The inline entry address advances from its
captured base, while entry values are loaded afresh.

The iteration path restores EDI, then both paths restore ESI. The sole explicit
receiver-field store remains **BYTE `[current EBP+3ECh]=1` after POP ESI and
before POP EBP**. Final RET8 follows EBP restoration. A final-store alias can
change not-yet-consumed EBP/return storage; it cannot retroactively change ESI
or EDI values already restored from their save slots.

No fixed array extent is inferred from the nearby byte at 3ECh. If a count
permits index seven, its inline slot begins there. Receiver and selected-entity
access extents remain the accepted audit's conditional selected coverage,
not a complete class or allocation size.

The child must preserve EBP=receiver, ESI=index, EDI=current slot and EBX, and
return with the specified stack balance. Added argument PUSH instructions
preserve the final predicate flags at child entry. The inserted `ADD ESP,10h`
changes post-child flags, but no branch consumes them before the original
increments and final count CMP overwrite them.

Final arithmetic flags therefore remain the path-specific last CMP flags under
the normal child/backing contract: initial `CMP count,0` for no iterations,
or final `CMP index,current_count` after iterations. POP/MOV/RET8 preserve
them. EAX/EDX have no owned writes; they retain incoming or last Source-child
residuals. Void makes no semantic return promise, and Source child/CRT residuals
are not asserted to equal Original child residuals.

## Existing provider and borrowed domain

The actual provider declaration is
`native_pending_entity_kill_00926d90(owners, actual_entity, cause, access)`.
Its Source accesses live entity fields through `NativePendingProducerEntityView`
references; the resolver must map the same entity identity to actual field
lvalues and actual parent/child/sibling identities. Passing raw selected ECX
avoids inventing a type or copying selected entity storage.

Supply the same initialized shared pending destroy/kill owners and a live access
implementation with the actual `009248D0` lock owner, current virtual providers,
node allocation and count operations. Owners and access must survive the
call, callbacks and direct recursive kill work. This candidate initializes,
allocates or owns none of them.

The admitted Source child captures and locks its section, checks/publishes
entity fields, dispatches its current virtual destruction provider, recurses
with the raw cause, and appends through allocation, growth and link publication.
Its existing Source guard/CRT/lock/service and exception behavior is reused.
The candidate adds no local RAII, catch, noexcept, rollback, exception
translation, Native FH3/SEH reconstruction or guaranteed final byte write.
Faults or nonnormal child returns may bypass the tail.

The existing projectile adapter demonstrates a different Source consumer that
retains owners/access and calls the actual provider. It does not bind the
selected launch task. The `UnitDamageHost` projections and cause-only helper
remain distinct; this candidate does not select them as substitutes.

The accepted parent99 contract passes its late Task34h value as receiver ECX.
Calling this Source interface additionally requires the real pending-owner and
access references. The current selected production route still uses typed
`PlaneSquadronHostRecord` launch fields. Seven exact current excerpts preserve
that boundary; no new call site or raw task/endpoint lifetime binding is added.

## Validation and pending work

The prior Source84 primary report's **67/67 input pins** replay at this baseline.
Its successful Win32 build, three existing checks, 12 captured objects and
14 positive Core roots are historical evidence for that earlier set. The
new Source84 root was absent from the application map. This candidate was not
part of that build and receives no compilation or admission credit from it.

The older endpoint and parent99 inputs differ only at `CMakeLists.txt`: exactly
the accepted `native_tick_subnode_base_cleanup.cpp` registration was added.
The candidate filename has no current CMake registration, and the only exact
function-name matches in Source/header/tests are this declaration and definition.

Current validation covers the complete Native 70-byte span, all 24 mapped
positions, all 28 Source assembly statements and graph/stack paths, scheduled
reference reads, four cdecl argument positions, final-store timing, Source RET8,
14 provider excerpts, seven production-projection excerpts and both complete
candidate files. No build, assembly, tests or probes were run for this bounded
candidate packet.

Primary registration/build review must establish the actual compiled body,
relocations, callee convention, Core selection and final application-map state.
Actual selected owner/access bindings, Original ABI and stack behavior, full
failure/unwind behavior, startup and gameplay remain separate obligations.
