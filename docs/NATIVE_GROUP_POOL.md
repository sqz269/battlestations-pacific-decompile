# Native group pool

`NativeGroupPool` reconstructs eleven routines for the raw `cGroup` storage pool
at `010902F4`. It owns actual slabs and slots through caller-supplied `0x38` pool
storage and the existing `AllocatorListDomain` over `00E188B4`. The companion
keeps no second slab table, free stack, allocator head, or cached slot state.
Group construction, parenting, children, and destruction of group contents are
separate owner responsibilities.

Evidence and original ABI are recorded in
[the audit](../reports/native_group_pool.json). Names are descriptive hypotheses.
The checked target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, with
bytes compared against the unchanged installed game executable.

## Storage and binding

The pool has allocator-list words at `+00/+04/+08`, a real Win32 critical section
at `+0C`, signed recursion at `+24`, slab-table pointer at `+28`, slab count at
`+2C`, table capacity at `+30`, and first available slab at `+34`. The sentinel is
`FFFFFFFF`. Profile `00D634D4` selects trim virtual zero `00B8F270`.

Each slab occupies `0x31C4` bytes:

| Offset | Meaning |
| --- | --- |
| `0000..317F` | 32 raw slots, stride `0x18C` |
| Each slot `+188` | DWORD slab-table index belonging to the pool |
| `3180..31BF` | 32 WORD free-slot indices |
| `31C0` | WORD free count |
| `31C2..31C3` | Unwritten padding |

Slab initialization writes indices `31..0` and all 32 slab IDs; group payload
`[0,188)` and the last padding WORD retain their original bytes. Initial
allocation therefore visits slots `0..31`; subsequent returns form a LIFO stack.
Every group constructor/destructor must preserve its slot's `+188` DWORD.

The caller binds one stable pool using `bind_static_native_group_pool_010902f4`.
`initialize_static_native_group_pool_00cd8460` initializes it and registers the
matching destructor through `std::atexit` or a supplied registration function.
An unbound adapter fails explicitly. Storage, domain, and companion must outlive
the registered callback. Initialization runs once. The C++ companion destructor
does not perform native shutdown or unregister the storage.

## Recovered behavior

| Native address | Action and original ABI |
| --- | --- |
| `00B8E7D0` | ECX slab, stack index; EAX slab, RET 4; initialize slot IDs/free stack |
| `00B8E8B0` | ECX table header; RET; free nonnull first word without clearing |
| `00B8EC80` | ECX pool; RET; free slabs/table, destroy section, unlink allocator |
| `00B8ED40` | ECX pool, stack slot; RET 4; return raw slot |
| `00B8EEB0` | ECX slot; push it, select static pool, call return; RET |
| `00B8F190` | ECX pool; EAX pool, RET; initialize and reserve 32 table entries |
| `00B8F270` | ECX pool; RET; trim empty slabs without taking a lock |
| `00B8F310` | ECX pool; EAX slot, RET; acquire raw slot |
| `00B8F450` | Select static pool and tail-call acquire; EAX slot |
| `00CD8460` | Construct static pool, register `00CE0F00`; EAX atexit result, RET |
| `00CE0F00` | Select static pool and tail-call destroy |

Construction prepends the actual allocator element before initializing the
section and reserving its pointer table. Allocation enters the section and
increments actual `+24`. When all slabs are full, it publishes the next slab
index before allocation, initializes the slab, grows a full table by
`capacity * 2 + 2`, then appends the slab. It pops the free stack and scans forward
for another available slab when the chosen one fills. Return reads the live
embedded ID, computes signed low-32-bit pointer difference divided by `0x18C`,
pushes the WORD slot index, increments count, and lowers the first-free index.
It does not validate null, foreign, duplicate, or already-returned slots.

Trim frees a completely empty slab and replaces its entry with the last slab.
It rewrites all 32 IDs of a moved slab, retries that table position, and finally
recomputes the first available slab. Capacity and table allocation are retained.
Destruction frees every slab regardless of live slots, frees the table, drains
positive recursion with actual `LeaveCriticalSection` calls, deletes the section,
and unlinks the allocator element using base profile `00D7A0C0`. It preserves
the native stale table/count/capacity/link fields and calls no group destructor.

The implementation uses the existing real `singleton_lifetime_allocate/free`
boundary. Native `00BF55BE` and `00BF6989` forward to the same allocator/free
implementations. There is no diagnostic allocation substitute in product code.

## Flow and exception evidence

Ghidra's old free-call overrides hide returning continuations at `00B8E8BC`,
`00B8EC9D`, `00B8ECB5`, `00B8F249`, `00B8F296`, and `00B8F3A6`. Disk and saved
bytes establish their complete behavior. In particular, `00B8F310` pseudocode's
return after table free is incorrect, and the hidden `00B8F296..00B8F2CF` block
performs slab compaction and ID rewriting. Workers made no Ghidra mutations;
the audit records exact repair sites for the integrator.

`00CD8460` had no Ghidra function entry during the audit. Its last instruction is
RET at `00CD8475`, length 1, with exclusive end `00CD8476`. The other tail adapters
end after five-byte JMP instructions, not at their final instruction addresses.

Constructor handler `00CC2ADE` selects FuncInfo `00DFC194`; its three-entry unwind
map at `00DFC17C` runs table cleanup through `00CC2AD3`, section cleanup through
`00CC2AC8`, then base unlink through `00CC2AC0`. MSVC `__finally` preserves this
order. Acquisition has no local unwind region: allocation failure preserves
published fields and held recursion, and failure after slab creation but before
table installation does not automatically free that unappended slab. Static
atexit registration failure also leaves the constructed pool intact.

## Validation and limits

One ignored focused fixture executes relocated original machine code and the
new C++ implementation against the same observed allocation/free and atexit
boundaries. Win32 section operations are real. It compares 2,089 storage
checkpoints per path, 76 ordered boundary events, and 622,973,260 slab bytes.
The sequence reaches 1,025 slots and grows the table from 32 to 66 entries,
compacts successive empty slabs, checks all moved IDs, reuses returned slots
without clearing payload, retains capacity through trim, and destroys two live
slabs with nested recursion. A constructor allocation callback traverses the
actual allocator list; registration deliberately returns 17 and the registered
callback is used for teardown. Unwritten slab padding is checked separately.

The focused fixture passed MSVC Win32 `/W4 /WX /fp:strict` compilation. The full
repository build and existing CTest results are recorded in the audit. No tracked
tests were added. Native exception dispatch, allocation-failure injection,
concurrent stress, process-exit callback scheduling, full owner integration and
gameplay remain unvalidated. These are new C++ interfaces, not drop-in ABI
replacements; successful allocator evidence is not game validation.
