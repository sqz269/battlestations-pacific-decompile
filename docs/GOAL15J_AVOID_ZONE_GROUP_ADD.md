# Raw avoidance-group entity addition

Address `0041D1E0`, descriptive Native name `BSP_AvoidZoneGroup_AddSceneEntity`
(hypothesis, not recovered symbol). Complete physical body `0041D1E0..0041D310`,
305 bytes and 103 instructions, no gaps.

Current packet status: complete caller Source, Native evidence, strict
compilation, emitted mapping and a pre-integration composition fixture passed.
The constructor was initially a named-incomplete gate. Its genuine complete
Source now exists and is tested in commit
`2f6ebe1d78d0ca7c46a749889d43db0d6e1f16ae`; primary acceptance, constructor
integration and the registered archive rerun remain explicit gates. No
replacement constructor, local success stub or optional provider bypasses it.

| Routine | Coverage | Source |
| --- | --- | --- |
| `0041D1E0` | complete raw caller and current cleanup operation; pre-integration proof, primary/registered gates pending | `src/native_avoid_zone_group_add.cpp` |

## Interface and actual storage

Original is void thiscall: ECX raw group, one stacked entity word, `RET 4`, no
semantic EAX result. The Source entry is
`add_native_avoid_zone_group_entity_0041d1e0(group,actual_world_pointer_cell,entity)`.
Its EDX parameter borrows the ADDRESS of the actual world-pointer cell for the
genuine constructor. This Source context is not an Original EDX argument or a
replacement world/global. The cell value is not read by this caller.

The 14h-byte `Cc13AvoidZoneNativeGroup` schema is reused: vtable `+0`, zone-array
pointer `+4`, unsigned count `+8`, capacity `+C`, layer word `+10`. Zones reuse
24h-byte `AvoidZoneNativeStorage`. No duplicate ownership record or host vector
is substituted.

The public naked Source thunk borrows the actual incoming entity-cell address.
The private persistent working set contains only that address, the borrowed
world-cell address and current Source phase. Its address lives in the enclosing
C++ frame while the naked kernel executes and unwinds. The actual incoming
entity word is neither copied away nor replaced with a private cleanup cell.

## Complete selection, construction and append schedule

The entity is captured in EDI. The real raw path-interface getter runs at
`0041D201` BEFORE testing the captured entity's current `+54 == 2` at `0041D206`.
Its returned path is captured in EBX after that comparison. Thus actual predicate
effects on entity kind are visible. There is no null entity/path repair.

The caller allocates 24h bytes first, then overwrites its ACTUAL incoming entity
word with that allocation at `0041D21C`. Phase0 is armed at `0041D222`. Only then,
on the nonnull branch, does it read CURRENT group `+10` for the layer, push layer
then the captured path, and call the real constructor. The retained allocation
null branch zeroes EBP and proceeds to a dereference at `EBP+4`; no successful
null fallback is added. The actual current allocator throws under its normal
positive-size failure policy, but the physical branch remains present.

Stable constructor contract agreed with its owner:
`void* __fastcall construct_native_avoid_zone_0041ccd0(void* zone,
void* const volatile* actual_world_pointer_cell,void* path,uint32_t layer_word)`
in `bsp/native_avoid_zone_construct.hpp`. Original constructor uses ECX zone and
two stacked path/layer words, EAX owner and `RET 8`; Source EDX is only the
borrowed actual cell address. No dummy EDX is added to the Original prototype.

After the constructor returns, its zone pointer is captured in EBP. The caller
reads the returned zone's current count and tests it BEFORE disarming phase0 at
`0041D243`. Any nonzero DWORD count takes the nonempty route.

Nonempty insertion preserves all inlined growth behavior. It grows only when
current group count equals capacity, and only if wrapped `2*capacity+2` is greater
than old capacity unsigned. Capacity is published before unsigned MUL by four;
overflow saturates the byte request to `FFFFFFFF`. Allocation receives that raw
request. Copying reloads CURRENT array and count each iteration. CURRENT old
array is freed before replacement publication. Appending reloads current count
and array, stores the captured constructed-zone pointer, then adds one to the
CURRENT count. Vtable and layer remain untouched. No extra length check, overflow
repair or rollback is added.

For an empty constructed zone, the real raw record cleanup runs first. The
current pointer-array word is captured once; if nonnull, that array is freed and
only the captured zone's pointer word is zeroed after free returns. The zone is
then freed unconditionally. There is no vtable destructor callback, capacity
reset or extra owner normalization.

## Calls and Native EH

All normal sites below are in `0041D1E0`. Heap calls push one DWORD and have
`ADD ESP,4`; the getter/record cleanup consume ECX with no stacked words; the
constructor consumes its two actual stacked words with `RET 8`.

| Site | Native callee | Operation |
| --- | --- | --- |
| `0041D201` | `007AC9D0` | raw path-interface getter before kind check |
| `0041D214` | `00BF681B` | allocate 24h-byte zone |
| `0041D233` | `0041CCD0` | construct with captured path and late layer word |
| `0041D278` | `00BF55BE` | saturated group pointer-array byte allocation |
| `0041D2A8` | `00BF6989` | current old group pointer array |
| `0041D2D7` | `004167C0` | empty-zone corner cleanup |
| `0041D2E4` | `00BF6989` | captured empty-zone pointer array |
| `0041D2F4` | `00BF65AC` | empty zone |

The sole live caller is `00424D00`, at `00424DC5`: entity EDI comes from scene
node `+8`. It is pushed before the group-factory layer word, so the factory's
`RET 4` leaves the entity word for this call; returned group is moved to ECX.
This is Native call evidence, not an automatic Source world-context binding.

Native handler `00C5E1BB` selects FuncInfo `00D84304`, magic `19930522`, maxState1,
unwind map `00D842FC`, zero try blocks/IP entries/ESType, EHFlags1. Exactly two
state values are used, -1 and 0, with ONE action: state0 -> -1 through `00C5E1B0`.
That action loads CURRENT `[EBP+4]`, pushes it, calls `00BF65AC` at `00C5E1B4`,
pops ECX and returns. It frees unconditionally and does not reset the argument.

The guard begins after allocation/entity-cell replacement and remains armed
through construction and the returned count read/test. It disarms before both
nonempty growth and empty cleanup. Allocation failure before arming acquires no
guard. Later group growth failure acquires no zone rollback: prepublished
capacity and the constructed allocation can remain. Constructor-local cleanup
composes before this parent action; no additional zone-record cleanup is added.

The Source uses one current C++ guard conditional on phase==0. Its unwind action
loads the CURRENT actual entity word once and invokes the genuine matched free;
it never caches the originally allocated pointer. Kernel phase writes preserve
the flags needed by the adjacent JZ/JBE. A fixed volatile actual kernel entry
retains genuine emitted current C++ unwind regions; no Native FS record or FH3
frame is transplanted. Source saves its context in one private stack word and
keeps all original algorithm GP registers. Original-depth tracking locates that
word at `ESP+(NativeDepth-12)`, including `+18` before the constructor call.

## Dependency gate and verification

Complete accepted dependencies are the raw path/ancestor access packet
`cf961a900e53aa7a39103444a3850409e92debe1`, raw cleanup packet
`588c8d82844d73bd3f02efc4a8e26fa01486f2cf`, and existing genuine
`singleton_lifetime_allocate({SingletonAllocationKind::object,n,n})` /
`singleton_lifetime_free`. Native BF55BE jumps to BF681B; BF6989 jumps through
BF65AC to BF9DC8. Their correct library names and historical records remain.
Current malloc/new-handler/retry/bad_alloc and matched free replace the private
Native CRT under an explicit current-provider contract, including raw byte
overflow requests. Guarded POD8 allocation is not substituted.

The named-incomplete constructor dependency was retained while its owner
completed all 994 bytes/279 rows, strict compilation, emitted proof and an
actual-provider fixture. The primary then authorized this caller's
pre-integration fixture using that actual object; this does not itself establish
primary acceptance, registration or production readiness. The stable header is
consumed directly, with no local prototype/stub/bridge. Strict optimized MSVC
x86 caller compilation passed with
`/std:c++17 /EHsc /MD /W4 /WX /permissive- /fp:strict /O2` against the genuine stable
header. The 279-byte private kernel and 18-byte public thunk passed complete
emitted mapping: 68 literal rows, ten branches, eight real call relocations,
actual-cell load/replacement, two phase stores, explicit constructor-context
forwarding, two adapted returns and 11 removed Native EH/frame rows. Current
MSVC emitted maxState1 and one actual guard action; this numeric coincidence
does not establish Native frame/EH identity.

One combined ignored fixture now passes with actual group/constructor/clip
objects and registered core. Its only indirect predicate input is an explicitly
local callable Source-vtable witness owned by the fixture. The finite fixture
kind check returns nonzero high EAX bits with AL true/false. This is not a
recovered Native predicate, entity factory, type policy or production binding.
The actual getter dispatch uses query **0x47**; the actual constructor's parent
dispatch uses query **0x44**. All direct callees, SDK and heap operations remain
the real implementations.

The combined fixture verifies getter-before-kind rejection and unchanged actual
entity word; three constructions with actual word replacement, capacity
0->2/spare/2->6 and preserved prior zones; actual constructor/clip/winding and
layer/count; AL44 true/false; and a parent-cell republication observed by the
third real ancestor call. A valid outside-world path covers empty-zone
record/array/body cleanup while retaining group storage.

Two actual allocation failures exercise the cleanup boundary. Real dirty-pose
refresh and two SDK repairs cause a zero-point continuation with a deliberately
large current count. The genuine clip requests wrapped `FFFFFFF8`, invokes the
real current CRT new-handler and throws bad_alloc. That handler republishes the
ACTUAL group entity word to a valid alternate allocation. The constructor's
actual array-only action frees/nulls the original array; the parent's action
frees the alternate CURRENT word. The original zone remains allocated with
count/capacity `3FFFFFFF`, and its first record/body are explicitly reclaimed.
No new pending record was allocated on this zero-point continuation.

A later genuine `FFFFFFFF` group-growth failure occurs after disarming. It
retains prepublished capacity `80000000`, original count/array and the fully
constructed unpublished zone. The fixture explicitly reclaims that zone and
restores its own group metadata before real Source-vtable scalar deletion of
the three retained zones, array and group. Bounded heap observations distinguish
the freed alternate/empty-zone/group blocks from retained failure owners;
they do not establish a global no-leak property. SDK/CRT handlers are restored
by fixture RAII on successful and failed assertions.

The fixture is `local/goal15j_avoid_group_add/group_add_probe.cpp`; its script
`run_group_add_probe.cmd` uses `/MD` and `/MANIFEST:EMBED`, with an `archive`
argument to drop all overlay objects. `verify_emitted.py` accepts an optional
registered object path. Primary owns the final registration/build/CTest,
dependency acceptance and archive-only rerun. No new test target was added.

All 305 captured live body bytes, the 44-byte unwind map/FuncInfo and 21-byte
action/handler match the installed PE. All 103 instruction boundaries, ten normal
branches, eight calls and two RET4 instructions were checked. No gap or
no-return repair was needed. Captures, generator/manifest and verification tools
are ignored under `local/goal15j_avoid_group_add/`.

Current Source context/frame/local addresses, current C++ unwinding/noexcept
termination and CRT identity remain distinct from Native FH3/SEH/frame/exception
identity. No Native virtual drop-in, actual entity factory/class/vtable/world
publication producer, differential execution or gameplay proof is claimed.
Earlier partial/typed avoidance APIs and all historical evidence remain intact.
