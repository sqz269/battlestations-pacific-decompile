# Actual Group pool and default constructor, CC10

Addresses: `AC6F50 AC6FF0 AC70D0 AC71A0 AC7260 AC73D0 AC7410 AC74F0 AC7590
AC76D0 CD7440 CE0AE0`; boundary-only `CB8B0E`.

This packet supplies the actual type2 default constructor and the distinct
F8BFA0 pool. It reuses the existing raw material/page pool engine in
`src/native_material_pools.cpp`: that entire preexisting file remains an exact
text prefix. Only Group-specific instantiations, assertions and a table cleanup
entry are appended. No common template algorithm is changed or duplicated.
Names describe behavior and are not recovered symbols.

| Entry / normal coverage | Provider / exact contract |
|---|---|
| AC6F50..AC6F63,20B | actual AA9390(type2), stamp D5CB80, same pointer; ECX destination, RET |
| AC6FF0..AC7032,67B | `initialize_slab<Group>`: WORD count32, descending free indices, each hidden slab ID; ECX slab, stack index, EAX slab, RET4 |
| AC70D0..AC70DD,14B | actual pool+28 header; capture nonnull table pointer, matching CRT free, no clearing; ECX header, RET |
| AC71A0..AC7229,138B | `destroy_pool<Group>`: stamp pool table, free every slab/table, drain held recursion, delete CS, unlink base element; ECX pool, RET |
| AC7260..AC72CB,108B | `return_slot<Group>`: real CS, current hidden ID, signed F0 division, append free index, increment WORD count, minimum first-free; ECX pool, stack slot, RET4 |
| AC73D0..AC73DB,12B | fixed F8BFA0 failed-slot return selector, ECX slot, RET |
| AC7410..AC74E2,211B | `initialize_pool<Group>`: real shared allocator list, CS and pointer table; ECX storage/EAX same, RET |
| AC74F0..AC758F,160B | `trim_empty_slabs<Group>`: no internal lock, free empty slabs, move last table entry, rewrite all moved IDs, rescan first-free; ECX pool, RET |
| AC7590..AC76CB,316B | `allocate_slot<Group>`: current CS/count/table, metadata-only new slab, capacity-before-allocation, decrement free count and return raw slot; ECX pool/EAX slot, RET |
| AC76D0..AC76D9,10B | selects F8BFA0, ignores incoming requested size, tail AC7590 |
| CD7440..CD7455,22B | AC7410(F8BFA0), then CRT atexit(CE0AE0), preserve registration result; RET |
| CE0AE0..CE0AE9,10B | selects same F8BFA0, tail AC71A0 |

The 38h actual storage is `NativeMaterialPoolStorage`: allocator links/vtable
at0/4/8, real 24-byte critical section+C, recursion+24, slab table+28,
count+2C, capacity+30, first-free+34. Group vtable D5CBFC has virtual0 AC74F0.
The process owner uses the existing `GameNativePhysicalPoolProcess` E188B4
allocator-list domain. Its companion binding is outside the actual storage;
no extra allocator fields or replacement list are introduced.

Each slab is1E44h: 32 F0h slots, EC-byte payload and hidden DWORD ID at+EC,
32 reversed WORD free indices at1E00, WORD free count at1E40, and two untouched
tail bytes. Slab construction writes only those metadata fields. Group default
construction uses the real raw base and its allocated self-linked sentinel;
it leaves base padding and the hidden ID untouched. Pool return, trim and
destruction never run a Group destructor or release payload resources. The
caller must finish payload teardown before releasing its slot/pool.

Per-body equivalence is established by the raw listings retained in the local
archive. Initialization publishes base vtable/previous/next/old-head link/head,
stamps D5CBFC, initializes the CS and recursion, zeros table/count/capacity and
writes first-free FFFFFFFF. Capacity32 is stored before the80h table allocation.
Allocation publishes first-free before slab allocation, and doubled capacity+2
before replacement table allocation. Copy/free/publication follows the native
schedule. Neither normal allocator adds failure rollback nor automatic unlock.
The actual CRT/new-handler and Win32 CS services remain the existing providers.

Return reads the live slot+EC slab ID under the lock, subtracts the current
slab pointer with DWORD wrap and performs signed division toward zero byF0.
Native IMUL88888889/SAR7/SHR31 agrees with the template's signed division for
the supported actual-slot domain. Count and index stores are WORD-sized.
Trim copies the last table entry even when removing that last entry, then
decrements count, rewrites all32 moved hidden IDs when applicable and retries
the current index. It resets/rescans first-free after compaction. Destruction
reloads the current count/table during its full free loop, leaves stale native
fields after release, drains positive recursion before DeleteCriticalSection,
then stamps/unlinks the base allocator element. No payload initialization or
destruction is hidden in these common operations.

Explicit process startup follows the existing page-pool ownership pattern.
`initialize_once_00cd7440` performs native initialization before real
`std::atexit` registration of CE0AE0. A nonzero registration result retains
the initialized pool without rollback. The host's once/mutex/failure state is
separate bookkeeping, not a guessed native field. Allocation access requires
completed explicit startup. Neither this packet nor the factory installs that
startup into the application. Native .CRT data CE34A0 references CD7440.

Ghidra currently lacks two functions: CD7440 ends CD7455 inclusive, last RET1;
CB8B0E ends CB8B17 inclusive, last JMP atCB8B13 length5. CB8B0E loads DEFB70
and tails BF6B43. FuncInfo magic19930522/maxstate3/mapDEFB58 has states
`0 -> (-1,CB8AF0 -> 403970)`, `1 -> (0,CB8AF8 -> 402F70 at pool+C)`,
`2 -> (1,CB8B03 -> AC70D0 at pool+28)`. The existing common template supplies
its host structured cleanup projection. This packet does not implement native
FH3 or claim native C++/SEH exception identity; injected allocation failures
are not independently tested here.

The saved analysis also omits these real returning-free continuations. Source
follows verified disk/live bytes, not the truncated decompilation. The worker
made no Ghidra mutation; each repair is deferred to the integrator.

| Owning entry | Required missing range (exclusive end) | Exact instructions |
|---|---|---|
| AC70D0 | `[AC70DC,AC70DD)` | POP ECX |
| AC71A0 | `[AC71BD,AC71C8)` | ADD ESI,1; ADD ESP,4; CMP ESI,[EDI+2C]; JB AC71B1 |
| AC71A0 | `[AC71D5,AC71D8)` | ADD ESP,4 |
| AC7410 | `[AC74C9,AC74CC)` | ADD ESP,4 |
| AC74F0 | `[AC7516,AC7550)` | current-table last-entry move, decrement/count comparison, all32 moved hidden-ID stores, SUB EDI,1 |
| AC7590 | `[AC7626,AC7629)` | ADD ESP,4 |

Validation: strict Win32 Release build and all three CTests pass. All13 live/PE
spans match (1,098 bytes including the10-byte EH boundary). The report records
every direct/import/register call and required repair instruction. One ignored
real-pool probe compares copied original bodies with source: 1,025 allocations
are the minimum to exceed the initialized32-slab table and exercise capacity66.
It checks actual 33-slab metadata, slot return, moving the last live slab and
repairing all IDs, complete destructor frees and draining a real held recursive
CS. Separate phases check null/nonnull table-header cleanup without clearing,
compare all1E44 slab bytes including payload/tail preimage
and the actual Group default payload with its genuine base sentinel (only that
allocated pointer is normalized). Real source process startup, idempotence,
selector/return, shared virtual0 dispatch and CRT-exit unlink also pass.

Only direct callee/global/import relocation is applied to original probe code;
its pool control flow, free continuations and arithmetic are unchanged. CRT and
Win32 calls use the same genuine existing providers on both sides. The probe
does not validate all invalid-pointer/reentrant/new-handler cases, native EH,
binary ABI or game behavior. AA12F0 optional-copy wrapper, AC6FA0 copy, the full
AA6560 factory and raw Group payload destruction remain outside this packet.
All prior archives remain unchanged.
