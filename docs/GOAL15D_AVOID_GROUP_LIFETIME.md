# GOAL15D avoidance group lifetime

Descriptive group names are hypotheses. Existing reviewed names and the earlier
typed implementations in `cc13_avoid1.cpp` remain intact. This packet adds
concrete entries over the existing raw Native storage and genuine current-CRT
free service, including the recovered destructor cleanup operation.

| Routine | Inclusive Native body | Original ABI | Coverage |
| --- | --- | --- | --- |
| `0041D0C0` | `0041D0C0-0041D143`, 132 bytes | ECX group; no stack arguments; RET | complete direct record/array/zone clear schedule |
| `0041DE70` | `0041DE70-0041DEC6`, 87 bytes | ECX group; no stack arguments; RET; Native FH3 registration | complete normal operation and recovered array-free unwind operation through current Source C++ EH |
| `0041E940` | `0041E940-0041E95D`, 30 bytes | ECX group; stacked flags word; RET 4; EAX original pointer | complete scalar-delete instruction schedule |

All live queries used the target-verifying `bsp.py` bridge with
`C:/Users/sqz269/bsp.gpr`, project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, base `00400000`. Whole listings, bounds,
incoming references and relevant CRT/EH dependencies were read. The three
listings have no byte gaps. The flow query does not expose typed flow/no-return
properties; no repair, Ghidra mutation, shared registry edit or CMake edit was
performed by this worker.

## Storage and clear order

`Cc13AvoidZoneNativeGroup` is **14h bytes, 20 decimal**, not 20h. The existing
schema supplies vtable +00h, zone-pointer array +04h, DWORD count +08h,
capacity +0Ch and layer key +10h. Existing `AvoidZoneNativeStorage` supplies
the record-pointer array +00h and count +04h inside its complete 24h-byte
storage. No duplicate ownership/projection type is introduced.

The clear kernel captures a current slot cursor, with initial endpoint from
group +04h/+08h. It skips null zone slots. For a nonnull zone it starts index 0,
loads the current record array before each free, and rereads the DWORD record
count after that free. The comparison is unsigned. After the loop it captures
the current record-array pointer, tests it, zeroes record count, conditionally
frees the captured array and nulls the field only after return. It then frees
the captured zone and nulls the current slot only after return.

At each outer advance it reloads group count and array before calculating the
new endpoint, retaining the original cursor. It writes group count 0 only at
normal completion. The group slot array, capacity, layer key and vtable remain
untouched by this entry. There is no extra validation, deduplication, rollback,
owner filtering or destructor callback. Invalid/cyclic/overlapping allocation
graphs retain their original precondition and fault limitations. These reads
remain explicit instructions even though the concrete current free service is
not an injectable callback.

The caller with a null owner in `0041D150` forwards ECX unchanged; the other
direct caller is the owned group destructor. Neither supplies a stack argument
to clear. No gameplay reachability is inferred.

## Destructor and actual Source virtual dispatch

The destructor first replaces vtable +00h with the relocated address of
`kNativeAvoidGroupLifetimeSourceVtable`. That actual const Source table's
slot 0 points to `scalar_delete_native_avoid_group_0041e940`; there is no dummy
success function or hardcoded unmapped Native address. No contract for
additional virtual slots is provided. Source objects entering manager dispatch
must already contain this matching Source table. Native table identity,
production object construction and original heap identity are not installed
by this packet.

The scalar-delete Source entry is
`__fastcall(group, unused_source_edx, uint32_t flags)`. ECX and stacked flags
match the consumed original locations; EDX is deliberately ignored. Thus the
manager's actual load of vtable[0] into EDX followed by CALL EDX reaches the
genuine entry. It calls the destructor, tests **bit 0 of the low byte of the
still-stacked flags after return**, conditionally frees the original object,
returns that original pointer even when freed, and executes RET 4. Other flag
bits do not add a path. With flag 0, the object survives with count 0, a null
zone array, preserved capacity/layer key, and the actual Source table.

Both original free entries have real providers: `00BF6989` is a direct jump
to `00BF65AC`, whose complete body was read. The new entries call existing
`singleton_lifetime_free`, which uses actual `std::free`. This matches current
Source allocations made by `singleton_lifetime_allocate`; it is not a claim
that current CRT heap internals, error state, synchronization, fault behavior,
or return/frame identities reproduce the original private CRT. No allocation
or exception-constructor substitute is introduced into the destruction chain.

## Recovered EH operation

Native `0041DE70` installs handler `00C5E1DB`, saves the group and writes state
0 before entering clear. The state remains 0 through the normal conditional
slot-array free and the following null store. Handler `00C5E1DB` loads
FuncInfo `00D84330` and tail-transfers to the existing runtime handler.
The live metadata is:

| Item | Value |
| --- | --- |
| magic | `19930522` |
| maxState | 1 |
| unwind map | `00D84328` |
| sole map entry | state 0 -> -1; action `00C5E1D0` |
| catch blocks / IP map / exception-spec list | zero |
| EH flags | 1 |

The complete unwind action loads the saved group from `[EBP-10h]`, adds 4,
and tail-jumps to the existing `00412CD0` free-and-clear helper. Its complete
body captures the CURRENT pointer field, returns if null, otherwise frees it
and writes null only after free returns. It does not clear zones, reset group
count, restore old fields, or free the group object on this unwind path.
These consumed metadata/helper addresses are evidence, not newly owned entries.

The Source destructor uses an actual current C++ scope guard over the same
group. It remains active through concrete clear and normal free/null; only
after normal completion is it dismissed. Its destructor performs the same
current-field free/null operation on unwind, leaving partial clear state alone.
There is no catch/replacement exception: current C++ unwinding propagates the
original exception after successful cleanup, with current SDK termination
policy if a noexcept cleanup unexpectedly throws. The concrete free provider
is noexcept and no artificial throwing callback was added.

MSVC initially inferred that the visible naked clear body could not throw and
removed the EH registration. A local volatile function pointer initialized
solely to the actual concrete clear entry prevents that elision. It adds no
provider option, global state or substitute object. The optimized object now
contains real current `__CxxFrameHandler3` registration, state 0 at the indirect
actual-clear call, and an emitted unwind funclet reaching the active guard and
genuine free/null helper. The normal destructor body is 95 bytes. Current EH
registration, cookie, handler, frame, local pointer slot, funclet and exception
identity remain Source-specific. No Native EH/SEH or unwinding compatibility
claim follows from the recovered action or static emission checks.

## Validation and limits

- Strict optimized MSVC Win32 compilation passed with `/std:c++17 /EHsc /MD
  /W4 /WX /fp:strict /O2`; command file
  `local/goal15d_group_lifetime_compile.cmd`.
- `local/goal15d_compare_group_bytes.py` passed for all 132 clear bytes and all
  30 scalar-delete bytes, excluding only their five expected CALL rel32
  operands. The script validates relocation types, sites and actual Source
  targets, then compares every other byte and exact body length. It also
  checks the table's real slot-0 relocation and emitted destructor/guard
  registration/action chain. Optional argv1 selects the registered object.
- The Native 87-byte destructor and Source 95-byte destructor are not subjected
  to an equivalence normalization; the operation and EH schedule were reviewed.
- The direct-call report checker covers all seven Native call sites. The
  recovered handler and unwind tail transfers are recorded separately.
- One ignored actual-heap manager/group/cache composition fixture in peer
  `local/output/avoid_manager_clear_probe.cpp` passed with actual Source table,
  nonempty zone/record/array clear, a null zone slot, slot 19's owned empty
  array, manager count reset/all twenty slots, body queue and hull/cache
  composition. A flag-0 phase checks returned identity, slot-array nulling and
  retained group fields. The same case preallocates the actual queue, then
  uses genuine current `_heapwalk` to observe all eleven tracked group/zone/
  record/hull blocks used before clear and not used immediately afterward;
  body storage, sentinel and queue remain used. The retained flag-0 group
  remains used while its old array becomes not used. This is bounded heap
  evidence, not a whole-process leak claim. It uses the two new Source modules
  plus the real registered archive, `/MD` and `/MANIFEST:EMBED`. Current C++
  exception propagation was not triggered by that case.
- Primary owns final registration/build/CTest, registered-object recheck,
  registered-only fixture rerun, annotation save/export and integration. Those
  pending checks must not be inferred from the worker's strict object or peer
  overlay fixture. No Native execution, ABI drop-in, game or gameplay proof is
  claimed. Earlier typed API/history evidence is preserved without rerun.
