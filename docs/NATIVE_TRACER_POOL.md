# Native SkinedWaterTracer raw pool and lifetime

Addresses: `00BA9B50`, `00BA9C80`, `00BABDD0`, `00BABED0`, `00BABF70`,
`00BAC2B0`, `00BAC3C0`, `00BAC4A0`, `00BAC660`, `00CD88D0`, `00CE0F90`,
compiler dispatcher `00CC3D3E`.

These eleven complete routines operate on the actual 38h static owner at
`0109049C`. The new MSVC Win32 interfaces borrow its storage, real critical
section and the application's same `00E188B4` allocator-list domain. Names are
hypotheses, not recovered symbols. A returned raw slot does not constitute a
successfully constructed `BAD6F0` tracer.

| Inclusive native range | Bytes | Original ABI | Coverage |
| --- | ---: | --- | --- |
| BA9B50..BA9B92 slab producer | 67 | ECX slab, stack slab ID; EAX slab; BA9B90 RET4 | complete |
| BAC4A0..BAC5DB allocate | 316 | ECX owner; EAX raw slot; RET | complete |
| BAC660..BAC669 static allocate | 10 | Ignore incoming ECX, select0109049C; tail BAC4A0 | complete |
| BABF70..BABFD7 return | 104 | ECX owner, stack raw slot; BABFD5 RET4 | complete |
| BAC2B0..BAC2BB static return | 12 | ECX raw slot; select0109049C, call BABF70; RET | complete |
| BAC3C0..BAC492 owner constructor | 211 | ECX fresh38h owner; EAX owner; RET | complete |
| BABDD0..BABE59 owner destructor | 138 | ECX owner; RET | complete |
| BABED0..BABF6F trim | 160 | ECX owner, D63F90 virtual0; RET | complete |
| BA9C80..BA9C8D table cleanup | 14 | ECX0Ch table header; RET | complete |
| CD88D0..CD88E5 startup | 22 | No arguments; EAX actual atexit status; RET | complete |
| CE0F90..CE0F99 exit callback | 10 | Select0109049C; tail BABDD0 | complete |
| CC3D3E..CC3D47 FH3 dispatcher | 10 | EAX=DFD530; tail BF6B43 | analyzed; no C++ entry |

## Physical storage and raw operations

`BAC3C0` produces the owner layout: allocator prefix at00/04/08, native profile
`D63F90`, real Win32 section at0C, explicit depth24, table28, count2C,
capacity30, earliest34. This is separate from the particle-model owner at
`F8D2D0`; only the established allocator-list and actual CRT services are reused.
No additional owner, surrogate pool or duplicate count is introduced.

The slab producer first writes free-count WORD3D90=8. It then writes eight
free-index WORDs7..0 at3D80 and the captured slab ID at7AC in each7B0h slot.
Each3D94h slab contains eight7ACh payloads and a trailing ID per slot; the final
WORD3D92 and every payload remain untouched. The first allocation is slot0.

Allocation enters the real section and increments current depth. When earliest
is FFFFFFFF it publishes current count as earliest, allocates3D94h, and
initializes nonnull slab storage with the then-current earliest. When current
count equals captured capacity, it publishes wrapped `capacity*2+2`, allocates
the wrapped DWORD byte size, copies while reloading count/table, frees the
current old table and publishes the captured replacement. It appends the
captured slab at current table/count and increments count after the store.
The native null-cell guards, unsigned wrap and field reloads are retained.

The selected slab's WORD free count is decremented and reloaded; its free-index
WORD selects `slab+index*7B0`. Exhausting a slab captures the unsigned
later-index/count comparison before publishing FFFFFFFF, then scans a captured
table cursor using current count. Both return paths decrement current depth
and leave the real section.

Raw return captures slot7AC and the current table entry after entering the
section. It interprets the low32-bit slot-minus-slab displacement as signed and
divides by1968 with truncation toward zero. `BABF97..BABFA6` proves signed
IMUL214D0215, SAR8 and sign correction. The result is truncated to WORD and
written at3D80+old_count*2, then the free count is reloaded and incremented;
that reload remains observable when the free-index write aliases the count.
Earliest is lowered by unsigned comparison before depth decrement/leave.

Neither raw operation has an EH frame. Allocation failure retains the entered
section, current depth, prior publications and any unpublished slab. No lock
guard, rollback, payload destructor, pointer check or reclamation is added.
Every reached raw address requires valid backing and an initialized section.

## Lifetime, shared list and static binding

Construction prepends the actual allocator prefix, publishes D63F90, initializes
the section/metadata, then publishes capacity32 before reserving80h of pointer
cells. The current count/table copy and free/publication order is retained even
though ordinary fresh construction begins empty.

The FH3 descriptorDFD530 uses mapDFD518: state0 to-1 invokes
`CC3D20 -> 403970` base unlink; state1 to0 invokes `CC3D28 -> 402F70` section
cleanup; state2 to1 invokes `CC3D33 -> BA9C80` table cleanup. The main body
advances directly0 to2 after initializing section and table metadata. The C++
abnormal-termination cleanup follows table, section, base order.

Trimming has no internal lock. It frees only slabs with free count8, reloads
table/count, moves the current last pointer into the hole, decrements count,
rewrites all eight moved IDs and retries the hole. It then recomputes earliest
using a captured table cursor and current count. Capacity is retained.

Destruction captures the initial count comparison before publishing the pool
profile. It frees slabs in ascending order using live count/backing, frees the
current table, drains positive signed depth, deletes the section and unlinks
the same list element through the base profile. It neither destroys tracer
payloads nor clears stale metadata nor frees the physical static owner.
Table cleanup likewise leaves its pointer/count/capacity untouched.

Bind the genuine reconstructed trim before shared-list publication. Static
binding changes no owner bytes, list links or CRT callbacks. Startup constructs
the borrowed owner and uses real `std::atexit` for the source exit callback,
returning registration status without rollback. Both raw owner storage and the
same domain must survive through CRT exit; repeated startup is not promised.

## Calls and source boundaries

The report carries every numeric direct CALL/JMP site and containing function,
plus every indirect OS site. Calls were checked against callee bodies and ABI.

| Sites | Native target and contract | Stack cleanup |
| --- | --- | --- |
| BAC442, BAC4FF | BF55BE tails BF681B: actual CRT allocation/new-handler retry | ADD ESP,4 at BAC449/BAC508 |
| BAC4CB | BF681B: actual CRT raw allocation | BAC4D0 ADD ESP,4 |
| BAC4DD | BA9B50: slab metadata producer | BA9B90 RET4 |
| BA9C87; BAC474, BAC531; BABE00 | BF6989 tails returning BF65AC free | BA9C8C POP ECX; respective ADD ESP,4 |
| BABDE8, BABEF1 | BF65AC: actual CRT free | BABDF0/BABF09 ADD ESP,4 |
| BAC412 | IAT CE220C: real InitializeCriticalSection | stdcall callee pops4 |
| BAC4AA, BABF79 | IAT CE2218: real EnterCriticalSection | stdcall callee pops4 |
| BAC5BB, BAC5CF, BABFCC, BABE1D | IAT CE2210: real LeaveCriticalSection | stdcall callee pops4 |
| BABE27 | IAT CE2214: real DeleteCriticalSection | stdcall callee pops4 |
| BAC2B6, BACBA5 | BABF70: raw return | BABFD5 RET4 |
| BAC665 | tail JMP BAC4A0, after selecting0109049C | no stack argument |
| CD88D5; CD88DF | BAC3C0 construct; BF6FF5 actual atexit | no constructor arg; CD88E4 POP ECX |
| CE0F95 | tail JMP BABDD0 | no stack argument |
| 8728C8; C96176 | BAC660 allocation; tail JMP BAC2B0 raw unwind return | no stack argument |

Current direct xrefs were inspected. `BACB90` performs the separate `BAC970`
owner teardown before a flag-bit0 raw return. `C96170` instead returns the
unconstructed raw allocation from the type4 constructor unwind. Those callers,
physical tracer construction and subsequent update/predicate/event behavior
belong to other packets.

## Saved-analysis repairs and verification

The worker kept Ghidra read-only. StartupCD88D0..CD88E5 and compiler
dispatcherCC3D3E..CC3D47 need function starts defined. Their final instructions
are respectively a one-byte RET atCD88E5 and a five-byte JMP atCC3D43.
The saved names `CG_static_dtor_stub_00bac660` and
`CG_static_init_00ce0f90` have the wrong roles; preserve prior comments when
applying the report's proposed names.

Returning-free flow gaps are BA9C8C, BABDED..BABDF7, BABE05..BABE07,
BABEF6..BABF2F, BAC479..BAC47B and BAC536..BAC538. They contain real returning
continuations, including the destructor loop and full compaction/ID rewrite.
The separately skipped alignment LEAs atBABEDD..BABEDF and BAC599..BAC59F
are not returning-call gaps. All1074 bytes, including both missing starts,
match the saved image and installed PE.

Strict standalone MSVC Win32 compilation passed with `/W4 /WX /O2 /fp:strict`.
The repository Win32 build and both existing CTests passed after seed-byte
verification. The primary still owns CMake registration of this separately
compiled source. Two ignored fixtures exercise all eleven original bodies, spanning
1064 bytes checked unchanged outside explicit four-byte bindings/relocations.
The allocation fixture passes265 allocations,32-to66 table growth, all slab
metadata/payload/padding, later full-slab scans, LIFO reuse, ignored incoming
ECX, both return entries and nested real lock/depth. Backed raw inputs compare
signed displacement-1969 and a free-index/count alias.

An actual `malloc(FFFFFFF8h)` failure and real new handler returning zero make
both original and source throw `bad_alloc`. Handler/caller observe depth1,
count/earliest1FFFFFFE, published capacity3FFFFFFE and unchanged old table; a
second thread cannot acquire the section. The fixture records those results
before releasing the lock and restoring bounds for real owner destruction.
Each deliberately unpublished slab survives until fixture-process exit.

The owner fixture composes original constructor, slab, allocation, return,
trim and destructor against the source with actual OS sections and allocator
lists. It passes17 allocations, middle compaction, all eight rewritten IDs,
preserved7ACh payload, return after compaction, empty retries, real virtual0 trim,
depth2 drain and stale table-header preservation. Native and source static
callbacks pass in separate processes through real CRT exit. No permanent tests
were added. Original FH3 dispatch, constructor allocation failure, native ABI
substitution, successful BAD6F0 tracer construction, rendering and gameplay
remain unvalidated.

The live call checker reviewed22 numeric call/tail rows. Nineteen passed;
three are blocked only by the missing startup/dispatcher function definitions
(CD88D5, CD88DF, CC3D43). The report retains those rows for the primary to
recheck after definitions, plus eight explicitly recorded indirect OS sites.
