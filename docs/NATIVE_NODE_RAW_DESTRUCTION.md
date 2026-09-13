# Native node destruction with a raw string pool

Addresses: B6F440..B6F568,297 bytes. This extends one existing complete normal
body. It adds no new native body, B6F3E0 claim, point-light allocator facade or
camera lifetime implementation. Existing typed overloads remain available.

| Routine | Coverage | Original ABI | New source overload |
| --- | --- | --- | --- |
| B6F440 | complete normal body in existing association/backing domain | ECX actual node prefix; no stack args; RET; no semantic result | runtime,binding,NativeStringRawPoolContext& |

The main change is consumed cleanup state. Native state2 covers attachment,
hierarchy, retained-owner and scene operations. B6F50C sets state1 BEFORE array
shrink/free. B6F51E captures current name data; B6F526 sets state0 BEFORE current
length+1 at B6F52C/B6F531 and getter419CC0 at B6F536. BD1510 at B6F53D consumes
the captured data/size plus unused1 and returns with RET0C. B6F544 sets-1 BEFORE
the D5C104 and CEB130 base phases. A getter failure during normal name return must
therefore run base cleanup only; the array and name have already been consumed.

The shared typed/raw body now keeps that state across normal cleanup. Each
unwind stage decrements the state before invoking its operation. The raw overload
calls actual419CC0 using the same01090AA8 and01090AA0 cells, then actualBD1510
using the current01090AA4 gate. It does not hide the getter behind the existing
NativeStringStorage::release noexcept interface. It leaves the freed array and
failed name header bytes intact. The caller must retain and explicitly resolve
failed name ownership; it cannot retry destruction on the ended prefix. Base
cleanup writes D5C104 through a volatile store and calls the existing rawBD30F0
leaf, which writes CEB130 and preserves the reference count. No physical slot
return or association disposal belongs to this direct destructor.

Original FH3 metadata is FuncInfoDFA900, unwind mapDFA8E8..DFA8FF, handler
CC1A01..CC1A0A. CC19F3 targets owner+164/B6F3E0, CC19E8 targets owner+54/41DD20,
and CC19E0 targets AA6E10 ->BD30F0. Cleanup-only exceptions are projected through
C++ and a second cleanup exception terminates. This does not certify original
FH3 second-exception search ordering, SEH or hardware-fault unwinding.

The point-light array still uses the runtime's actual matching allocation
registry. Requested0 is the simple shrink only for valid nonnegative count and
capacity; native negative capacity can grow and negative count can fill. Those
cases and foreign backing are outside this extension. The array count reaches0
without element/light release; current backing is freed and pointer/capacity
remain. A foreign/repeated free terminates in the existing matching-domain
provider. Root repairs the complete23-byte helper analysis without assigning
this packet another body or changing allocation ownership.

All13 incoming preparations are recorded, including seven ordinary calls and
six EH JMPs. Scalar wrappers B6F8D0/B74B60/B91C00 preserve incoming ECX in ESI and
leave their flag argument for subsequent pool disposal. Camera B71F10, model
B750C0, light B7C5B0 and group B8F680 pass their captured owner after consuming
their derived cleanup state. Their CALLs have no node stack arguments. EH thunks
CC1AD0/CC1B00/CC1B20/CC1C50/CC1ED0/CC2AF0 restore ECX from EBP-28 for the first,
EBP-10 for the others, then JMP. Saved-frame producers and complete containing
listings are retained. Node layout comes from the existing B6F5A0 producer and
exact174h prefix; a derived/pool tail is not cleared or freed here.

The constructor audit found a separate obstacle to a fully raw camera runtime:
NativeNodeDestructionRuntime still requires a public SizedStoragePool& strings.
The new raw destructor does not consume it, but the runtime cannot be created
from raw cells alone. A concrete follow-up should make the borrowed semantic
name domain optional, add a raw/no-semantic-pool constructor, preserve existing
semantic entry points with checked semantic access, and migrate12 direct string
accesses across8 source files listed in the report. Camera construction179 and
finish_node122..128 currently select semantic paths, including unwind tails242
and273 and normal tail277. They need persistent raw forwarding through all
those paths. No dummy pool or false claim of a fully raw camera environment is
introduced here; terminal/noexcept adapters remain explicit boundaries.

Verification: strict Win32 source and probe compilation passed. Baseline822b99f0
build and both existing CTests passed; an initial probe link showed its old
destructor object lacked the new symbol. A subsequent full build invocation
rebuilt changed source/dependents and again passed both CTests. All8 seed spans
matched. Final fixture uses only external probe.cpp with the current three
modified-worktree libraries, not separately compiled provider sources.

One focused fixture has a typed normal control and the raw throwing-getter path.
The control actually allocates/returns names through runtime.strings, so that
required semantic pool is used. The raw phase constructs a live raw pool/manager
and actual node binding, with matching point-light backing. It temporarily clears
the current pool publication while retaining the original registered pool and
names. Only the disposable probe joins an unnamed Win32 job with its own process
commit limit, causing actual419CC0's second8AD4A0 allocation to throw bad_alloc.
The quota is removed after catch. The fixture observes baseCEB130, count1 and
the178h allocation tail unchanged; count0/freed pointer/capacity retained; and
the same name header and raw pool return count. A repeated array free would
terminate. The caller restores publication and explicitly cleans retained name
ownership, then drains the actual raw manager/pool. All teardown succeeds.

The source state transition proves one base invocation; the fixture observes
its final profile rather than instrumenting individual writes. This is actual
provider/source exception evidence, not an original FH3 byte replay. It does not
cover hostile descriptor mutation, foreign allocators, arbitrary virtual targets,
private frame aliases, concurrency, original caller ABI or gameplay. Existing
retained-owner/scene callbacks must remain their real canonical dispatch.

Evidence capture: C:/Users/sqz269/bsp-bd-node-destruction/worker_capture.zip.
It retains exact modified and baseline source/header archives, three libraries,
probe/recipe, original bytes/listings, input hashes and logs. Default
run.ps1 -Repo <integrated root> compiles only external probe.cpp against that
root's current libraries with an embedded manifest. Root owns integration,
analysis repairs and any later raw camera/runtime composition work.
