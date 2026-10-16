# Complete raw avoidance-zone clipping

Address: `0041A540`. Descriptive Native name retained:
`BSP_AvoidZone_ClipWorldBoundsAndRebuildCorners` (a hypothesis, not a recovered symbol).

| Entry | Physical body | Coverage | Source |
| --- | --- | --- | --- |
| `0041A540` | `0041A540..0041AC53`, 1812 bytes, 535 listed instructions | complete four-plane algorithm, every inline growth/rebuild branch, two cleanup operations | `src/native_avoid_zone_clip.cpp` |

The new entry is `clip_native_avoid_zone_world_bounds_0041a540` in
`bsp/native_avoid_zone_clip.hpp`. The earlier typed
`avoid_zone_clip_world_bounds_0041a540` and its historical records remain intact.
This entry operates on the actual raw storage using concrete current heap and
cleanup services. There are no injected allocation, clipping or success callbacks.

## Physical interface and storage

Original input is ECX = zone, followed by two stacked minimum/maximum XYZ
pointers. The epilog is `RET 8`; no semantic EAX result was recovered. Incoming
EDX is overwritten by the maximum-pointer load before use. The Source fastcall
declaration has an unused EDX placeholder, which is not an Original parameter.
Only X at `+0` and Z at `+8` of each bounds pointer are read; Y is unused.

The public naked thunk passes the addresses of the actual two incoming stack
cells to the private Source implementation. It does not substitute private C++
parameter copies. The two original loads occur in their original order: maximum
at `0041A55B`, minimum at `0041A57E`.

`AvoidZoneNativeStorage` is reused: 24h bytes, corner-array pointer/count/capacity
at `+0/+4/+8`, layer/entity at `+C/+10`, bounds at `+14..+20`.
`ShipAiPathLateralRecord` is reused: 24h bytes, X/Z at `+0/+4`, six derived floats
at `+8..+1C`, clearance at `+20`. No duplicate owner type or C++ record constructor
replaces the actual raw initialization sequence. Layer, entity and cached bounds
are never rewritten by this entry.

Caller storage must satisfy the raw pointer/count contract for every reached
access. The Native count and growth arithmetic are unsigned DWORD operations.
An overlapping input with zero corners still reads the previous point before its
count test. The Source retains that ordering rather than adding an empty guard.

## Floating-point operations

The four rejection comparisons are, in order, zone.minX versus maximumX,
minimumX versus zone.maxX, zone.minZ versus maximumZ, and minimumZ versus
zone.maxZ. Their x87 `FCOMI(P)`/`JNC` edges reject ordered `>=`; unordered values
do not take those edges. The rejection path performs the original x87 stack pops
before clearing the actual records.

Planes retain the original order and operations:

| Plane | X coefficient | Z coefficient | Constant |
| --- | --- | --- | --- |
| 0 | 1 | 0 | 100 - minimumX |
| 1 | -1 | 0 | maximumX + 100 |
| 2 | 0 | 1 | 100 - minimumZ |
| 3 | 0 | -1 | maximumZ + 100 |

Four private Source literals bind the exact Native bits: float +0 `00000000`,
float +1 `3F800000`, float -1 `BF800000`, double 100 `4059000000000000`.
The x87 bytes `DC EB` at `0041A5D3` update ST3 as ST3 - ST0. Register forms are
emitted literally to preserve this destination and the distinct subtraction and
division forms later in the body.

Each plane fetches the previous point before checking input count, resets the
current output count, and loads Z coefficient, X coefficient, then constant.
Current distance evaluates `(currentZ*nZ + currentX*nX) + d`; previous distance
evaluates `d + (previousX*nX + previousZ*nZ)`. The products and additions stay
extended until their separate binary32 stores at `0041A7C9` and `0041A7E7`.
Current inside means ordered `currentDistance > 0`. Entry interpolation requires
previous distance < 0; exit interpolation requires previous distance > 0.
Zero and NaN do not become positive. No epsilon, finite guard or alternate
classification is introduced.

Entry ratio is `-previous/(current-previous)`; exit ratio is
`previous/(previous-current)`. Their original extended subtraction/division,
ratio float store, separate dx/dz float stores, product float stores and final
coordinate float stores remain ordered. Geometry is x87. SSE performs the
original MOVSS, XORPS and COMISS operations; no SSE geometry or C++ double bridge
replaces it. No control-word or MXCSR change is added.

## Allocation, alias and publication schedule

The first scratch header is zeroed, state0 is armed, and its saved zone count is
used for capacity before allocation and count after allocation. The second
header is zeroed, CURRENT zone count is reloaded, and state1 is armed before its
allocation. Allocation byte counts retain wrapped multiplication by eight.
Copying reads CURRENT record-array and count cells each iteration, stores X
before loading Z from the captured record pointer, and only then clears the old
records through the concrete raw cleanup entry.

All three inlined scratch append sites remain complete. They grow only for
`count == capacity`, publish `2*capacity+2` before the wrapped byte allocation,
and copy in the original x87 order. A null destination cursor skips both source
reads and stores while advancing. Each copy iteration reloads the current source
data cell. Current old data is freed before replacement publication. Appending
uses the captured old count and publishes that count plus one, retaining its
effect across aliases. Scratch header identities swap between planes. Any
result with fewer than three points exits after second-then-first scratch frees.

Final rebuilding allocates each 24h-byte record and retains the physical null
branch and exact field stores. Pointer-array insertion grows only at count ==
capacity and only if wrapped `2*capacity+2` is greater than old capacity. Capacity
is published before allocation; unsigned MUL by four saturates overflow to
`FFFFFFFF`. The old array and current count are reloaded throughout copying,
CURRENT old array is freed, then the replacement is published. After insertion,
current count/current array/last record are reloaded before X and Z stores.

A new record is deliberately unguarded before array insertion. An unpublished
replacement buffer is also unguarded. Exceptions retain the original partial
state and possible unpublished-allocation leaks; no rollback or third guard is
added. The allocator is the genuine existing
`singleton_lifetime_allocate({SingletonAllocationKind::object,n,n})`, including
malloc(0), new-handler retry and current bad_alloc policy. Deallocation uses its
matching `singleton_lifetime_free`. Guarded POD8 count allocation is not used.

## Normal direct calls

All sites below are within `0041A540`. Heap calls push one DWORD and use
`ADD ESP,4`; the raw record cleanup consumes ECX with no stacked arguments.

| Site | Native callee | Consumed operation |
| --- | --- | --- |
| `0041A683` | `00BF55BE` | first scratch allocation, saved count |
| `0041A6BB` | `00BF55BE` | second scratch allocation, current count |
| `0041A6F2` | `004167C0` | clear old corner records after scratch copy |
| `0041A879` | `00BF55BE` | entry-intersection scratch growth |
| `0041A8B9` | `00BF6989` | current old scratch data |
| `0041A910` | `00BF55BE` | current-point scratch growth |
| `0041A94B` | `00BF6989` | current old scratch data |
| `0041AA0A` | `00BF55BE` | exit-intersection scratch growth |
| `0041AA49` | `00BF6989` | current old scratch data |
| `0041AAF2` | `00BF681B` | allocate 24h-byte record |
| `0041AB44` | `00BF6989` | collapsed-result second scratch free |
| `0041AB55` | `00BF6989` | shared first scratch free |
| `0041ABA4` | `00BF55BE` | zone pointer-array growth |
| `0041ABD5` | `00BF6989` | current old zone pointer array |
| `0041AC1A` | `00BF6989` | completed-result second scratch free |
| `0041AC37` | `004167C0` | bounds-rejection corner cleanup |

The only live caller is `0041CCD0`, at `0041D053`. Its later derive operation
is a separate caller operation; this clipping body has no derive call or tail.
Concrete raw cleanup dependencies are in `native_avoid_clip_cleanup.hpp`, accepted
commit `588c8d82844d73bd3f02efc4a8e26fa01486f2cf`. Original allocation/free thunks
were checked through their actual jump targets. Their correct library names and
all historical Source records remain unchanged.

## Native cleanup metadata and current Source frames

Native handler `00C5E163` selects FuncInfo `00D8424C`, magic `19930522`, maxState2,
unwind map `00D8423C`, zero try blocks, zero IP entries, zero ESType and EHFlags1.
There are exactly two actions:

| State | Next | Action | Actual operation |
| --- | --- | --- | --- |
| 0 | -1 | `00C5E150` | address first current pointer cell (Native EBP-A4), tail to `00412EF0` at `00C5E156` |
| 1 | 0 | `00C5E15B` | address second current pointer cell (Native EBP-7C), tail to `00412EF0` at `00C5E15E` |

State0 is written before the first scratch allocation at `0041A66E`; state1 is
written before the second at `0041A6A5`. Both remain active through rebuilding
and both normal free calls, second then first. Each cleanup reads its CURRENT
pointer cell once, conditionally frees it and does not reset it. No catch,
record-pending guard, replacement-buffer guard or owner rollback exists.

The Source deliberately owns its algorithm cells in the enclosing current C++
frame, which survives unwinding the naked kernel. It does not transplant the
Native FS record or make a guard refer into a dead kernel frame. Algorithm local
references are rebased from Native entryESP-D0 into this working set. Source phase
at +C4 is armed at the two original stores; argument-cell pointer at +C8 and
virtual EBP at +CC are private Source metadata. Original EBP calculations use
flag-preserving spill sequences. The actual EBP remains the working-set base.

Two current C++ guards hold the first/second current data cells at +2C/+54 and
test phase >=0/>=1. On unwind their reverse destruction invokes the real
`free_native_current_pointer_cell_00412ef0`. Both remain active through both
normal frees and are dismissed only after normal kernel completion. A fixed
volatile actual kernel entry retains genuine current MSVC EH regions; it is not
an injectable provider. Emitted current FuncInfo has maxState4: two guard unwind
actions and two std_terminate states for noexcept destructor execution. These
frames, state values, security cookie and termination policy differ from Native.

## Verification and limits

Strict optimized MSVC x86 compilation passed with `/std:c++17 /EHsc /MD /W4 /WX
/permissive- /fp:strict /O2`. All 1812 captured live bytes match the installed PE;
three unreachable padding gaps total 15 bytes and have no listed branch targets.
No listing repair or no-return correction was needed.

The emitted Source kernel is 2096 bytes, the public thunk 17 bytes. The ignored
verifier checks every one of the 535 Native rows: 293 literal, 120 algorithm-cell
rebases, 54 branches, five constant bindings, two phase stores, 16 provider calls,
27 virtual-EBP lowerings, two argument loads, two adapted returns, and 14 removed
Native EH/frame rows. It checks every branch target and exactly 21 kernel
relocations. All 210 Native FP/SSE rows are checked: 209 literal/rebased/bound
rows plus the specially lowered previous-point MOVSS at `0041A722`. This is a
complete emitted mapping, not a claim that the whole Native function bytes match.

One ignored real-provider Source fixture passed: bounds rejection with actual
record free; all four clipping planes with scratch growth and pointer-array
capacity 3->8; bounds pointers aliased into an old record freed mid-call; exact
rebuilt defaults; collapsed empty result; and a genuine impossible Win32
`FFFFFFF8` allocation with the actual CRT new-handler returning zero. The latter
propagated std::bad_alloc through phase0, retained the entire owner/original
record and restored the handler (one invocation). No fake allocator or forced
SEH fault was used. Mid-rebuild failures and double exceptions were not forced.

Artifacts are ignored under `local/goal15i_avoid_clip/`: `compile.log`, complete
`kernel_manifest.json`, `emitted_receipt.json`, `verify_emitted_clip.py`,
`clip_probe.cpp`, `run_clip_probe.cmd` and `probe.log`. The verifier accepts an
optional registered object path; the fixture script accepts `archive` to link
only the final registered archive. Primary owns full registration/build/CTest
and that archive-only rerun. No new test target was added.

This packet proves complete Source operations under the stated current providers,
strict compilation, static emitted mapping and the bounded Source fixture. It
does not prove Native FH3/SEH/local-address/frame/exception/private-CRT identity,
Native differential behavior, ABI drop-in deployment, actual entity factory
binding, gameplay, or a global no-leak property.
