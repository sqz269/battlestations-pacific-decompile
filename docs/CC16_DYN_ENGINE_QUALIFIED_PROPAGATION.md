# Qualified Dyn engine construction propagation

Frozen base: `02a78bd1fc85cd7e2ff06450d1aa7edeae15ae94`. This adds the named
library interfaces `dyn_engine_construct_qualified_00c55ea0` and
`dyn_engine_ensure_qualified_00c55f50`. They compose the real qualified Root
and admitted qualified task constructor with the recovered enclosing cleanup.
Old APIs, callers, context/storage layouts and normal code remain unchanged.
This is valid-input C++ source propagation, with zero native-body, binary ABI,
fault-transport or activation credit. No Game/application context is rebound.

## Genuine inputs and suppliers

Both entries require the existing `DynEngineRuntimeContext` and
`DynProfileRootPartialNodeDelete`; the constructor also borrows actual fresh
14h engine storage. The descriptor is a `const volatile uint32_t*`, with its
first DWORD readable only on the nonnull task path. Its address/backing must
remain valid and disjoint through that operation. Null-task and hot-ensure
paths do not read it. No descriptor snapshot occurs before allocation.

The raw provider must genuinely match the same allocator used for 48h Root,
9Ch profile, 358h task and 14h engine blocks and return normally. The existing
`GameNativeDynProcess::profile_partial_node_delete_00bf65ac()` recognizes its
actual default `(context,allocate,release)` tuple after completed startup and
returns real unsized scalar `::operator delete`; that tuple allocates these
raw blocks through `::operator new`. The aliases are type-compatible. This
packet does not invoke that getter or manufacture another owner/default/thunk.
Nonnull validation is not authentication of arbitrary custom/manual custody.

Vector BF6989 cleanup remains the genuine `context.memory.release` service.
The Root name's existing SBO malloc/new-handler/free domain stays separate.
Canonical engine/profile publication cells and the name remain the supplied
context's actual stable authorities; no copied cell or PE table call is added.
The existing private A900 current-data operation is reused for engine409B10:
the native fourteen-byte helpers differ only in the relative operand of their
same BF6989 call. Both capture current data once, release iff nonnull, and
leave fields untouched. All providers/backing must outlive nested cleanup.

## Native scopes and source composition

Read-only BSP checks used existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Original PE SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Complete native listings were read; six fresh live C55/helper/map spans agree
with disk. All eleven readiness spans are independently disk/hash checked.
The admitted task source is unchanged from 237; its existing qualifications
remain in force. Readiness 21448's 18 source pins are checked against its own
frozen 64cd snapshot, rather than incorrectly treating that older snapshot as
this packet's current source baseline.

Native C55EA0 is 175 bytes/57 instruction starts, stack(engine,descriptor),
EAX=engine, RET8. FuncInfo E042CC/map E042B4 has three states:

| State | Predecessor/action | Extent |
| --- | --- | --- |
| 0 | -1 / CC83E5→409B10 | After initial +0/+4/+8 zeros, before 9Ch allocation through return: current engine-vector cleanup |
| 1 | 0 / CC83ED→BF65AC | Original captured 9Ch only around Root; demote before engine+C store |
| 2 | 0 / CC83F8→BF65AC | Original captured 358h through late descriptor read, task child, +10 store and return |

The qualified constructor rejects a missing raw provider before caller data
access, performs only the three original initial zeros, then starts its outer
current-vector catch before 9Ch allocation. The captured profile catch composes
the genuine three-argument qualified Root and ends lexically before engine+C
publication. No completed profile/Root is freed when the later task allocation
fails. After that allocation, a separate captured task catch covers one live
volatile descriptor DWORD read, the real qualified C37740, +10 and return.
The null task branch skips both descriptor access and child construction.
Each raw catch calls the provider unconditionally on its original allocation,
then rethrows. The outer catch captures current engine vector data once,
conditionally calls its actual release service and rethrows. No old API fallback.

Native C55F50 is 111 bytes/34 instructions, ECX=descriptor, EAX=existing/new/null,
plain RET. Its first publication capture at C55F66 fast-returns when nonnull.
E042F8/map E042F0 has one state 0→-1/CC840D: captured 14h free only. Allocation
C55F74 precedes acquisition; construction, current publication including null
and return remain acquired. No post-callback fast-path recheck exists.

The new ensure preserves that order: one actual volatile publication capture,
nonnull return before descriptor/provider validation or allocator access; cold
provider rejection, then 14h allocation outside the captured catch; conditional
qualified construction, actual publication/null publication and return inside.
Failure frees only captured 14h. It does not reload/free/clear current publication
or requalify an existing engine. Caller-side argument evaluation is separate.

These actions preserve native residues: Root failure can leave stale profile
publication; later task allocation/construction failure leaves completed
profile/Root orphaned; reached publication/callback effects remain. Raw engine
vector cleanup is not a full destructor. No child destruction, field/publication
clear, handle/worker closure, retry, equality repair or rollback is invented.
The task constructor's ordinary Win32/BOOL/count0/partial-handle limits and
Root's existing std::memset versus native BF79F0 qualification remain explicit.
Cleanup must return normally; secondary-cleanup transport/continuation is open.

## Emitted qualification and validation

Strict MSVC Win32 baseline/current objects passed
`/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX /c /FAs`.
All 32 old code records and 8 old EH metadata records match exact bytes and
relocation offset/type/targets after only anonymous-namespace hash normalization.
The new normal entries are 210/125 bytes, with two 29-byte handlers and four
catch/cold regions measuring 50/18/24/51 bytes. Cold regions also contain the
provider-error blocks/padding; those are not additional cleanup actions.

The constructor's C++ FuncInfo maxState 6 has profile try 1..1/catch 2, task
try 3..3/catch 4 and outer try 0..4/catch 5. Both raw-catch rethrows reach the outer
current-vector catch. Ensure maxState 2 has try 0..0/catch 1. The raw frees use
original captures, never the child result or publication. Root's actual
qualified symbol is defined in this object; the task's exact qualified symbol
has an admitted real definition. No numeric original routine is invoked.

Physical `/EHsc` ordering differs from native and is deliberately qualified:

- Constructor provider check at +2B precedes payload stores. State 0 at +40 is hoisted
  before initial zeros+47/+4D/+54. The first allocator is+60.
- Profile capture at +65/state 1 at +68 protects actual qualified Root at +75. Engine+C
  stores at +7D while physical state 1 persists; state 0 demotion is at +8A before
  the next allocator at +91. The source profile catch has already ended, and
  there is no valid C++-throwing call in the delayed-demotion interval.
- Task state 3 at +95, capture at +9C and null guard at +9F/+A1 precede the sole descriptor
  value read at +A7 and qualified task at +B1. Null skips to +BB; +10/return remain
  acquired through FS restoration at +C3.
- Ensure publication read at +31/fast branch at +35 precedes provider check at +37,
  allocation at +43, captured 14h at +48/state 0 at +4B, constructor at +5E and publication at +69.
  Its allocator has no captured cleanup; no earlier state 0 write is emitted.

There is no C++-throwing call in the valid hoisted initial prelude. These facts
support the C++ catch contract, not native initial-store/profile-store fault
epochs, descriptor faults, SEH/Win32 extern-C behavior, original FH3/frame/stack
identity or secondary-cleanup equivalence. No `/EHa`, synthetic bridge or original
FH registration was added to force an apparent match.

Seven original direct rows pass the existing call checker. Three unconditional
raw-free calls and three EH tail sites are independently PE checked. Local
source/native/object/EH/call receipts are pinned in the report; rerunnable
`object_audit.json` SHA-256 is
`3cf81ff640912df8f7ce01b36d4ff3b0772ba35cefb8c7abbc2fa1014274b7dc`.
No full build/link, new test, fixture, native/object execution, game, probe,
DLL call, Ghidra mutation, CMake/shared metadata or peer runtime-tree change.
Independent review/common build remain primary obligations. Actual context
selection/authentication, retained failure policy, GameRuntime/physics attachment
and production caller adoption remain later work; old callers still use normal
APIs. Existing unrelated runtime receipts retain their prior scope.
