# Native model pool

`NativeModelPool` reconstructs eleven functions for the raw model-storage pool
at `01090054`. The actual `0x38` storage joins the existing `AllocatorListDomain`
over the shared `00E188B4` head. Model construction, resource ownership and model
destruction remain separate responsibilities.

The [audit](../reports/native_model_pool_audit.json) records exact exclusive
extents, original ABI, byte preimages, proposed names, original annotations,
source hashes and validation. The verified analysis target is
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Names are descriptive
reconstruction hypotheses rather than recovered symbols.

## Storage and caller contract

The caller supplies one canonical `NativeModelPoolStorage`, a stable companion
and the shared allocator domain. The companion binds profile `00D62DD0`, virtual
zero `00B74C60`, before initialization publishes the allocator element. It holds
no separate slab state. Its C++ destructor does not perform native destruction.

| Offset | Actual pool field |
| --- | --- |
| `00`, `04`, `08` | Allocator profile, previous link, next link |
| `0C..23` | Actual Win32 `CRITICAL_SECTION` |
| `24` | Signed explicit recursion count |
| `28` | Slab pointer table |
| `2C`, `30`, `34` | Slab count, table capacity, first free slab index |

Compile-time assertions require Win32 pointers, a 24-byte critical section and
the complete `0x38` layout. The storage declaration adds no field initializers.

Each `0x3144` slab contains 32 slots of `0x188` bytes, 32 free-index words at
`+3100`, a free-count word at `+3140`, and two unwritten bytes at `+3142`. A model
occupies `[0, 0x184)`. The DWORD at slot `+184` belongs to the pool and must survive
every model constructor/destructor. Enlarged host companions live outside slots.
The slab initializer writes count 32, free indices 31 down to 0, and all 32 IDs;
it preserves model payload and padding. A new slab returns slot zero first.

## Functions and behavior

| Address | Original ABI and operation |
| --- | --- |
| `00B74450` | ECX slab, stack slab ID, EAX slab, RET 4; initialize slab |
| `00B74530` | ECX table-pointer header, RET; free nonnull table without clearing |
| `00B74690` | ECX pool, RET; free pool allocations, section and list membership |
| `00B74750` | ECX pool, stack slot, RET 4; return raw slot |
| `00B748C0` | Incoming ECX slot; push it, select `01090054`, call return, RET |
| `00B74B80` | ECX pool, EAX pool, RET; construct raw pool |
| `00B74C60` | ECX pool, RET; trim empty slabs without acquiring a lock |
| `00B74D00` | ECX pool, EAX slot, RET; acquire raw slot |
| `00B74EB0` | Select `01090054`, tail-call acquire; EAX slot |
| `00CD7F00` | Construct canonical pool, register `00CE0E50`; EAX atexit result |
| `00CE0E50` | Select `01090054`, tail-call destroy |

Construction prepends the actual allocator element, installs its profile,
initializes the real section, clears the count/table/capacity fields and writes
first-free `FFFFFFFF`. It publishes capacity 32 before allocating the 128-byte
table and publishes the table after the returning old-table free continuation.

Acquisition enters the section and increments explicit recursion. If first-free
is `FFFFFFFF`, it publishes the current slab count as first-free before allocating
a slab. Slab initialization reads that live index after allocation. When the
table is full, capacity becomes `capacity * 2 + 2` before table allocation; live
entries are copied, the old table freed, and the replacement installed before
the new slab is appended. It then pops the actual free-index stack and scans for
another available slab when the selected slab becomes full. No source guard or
cached state changes native publication and reload order around allocation.

Return reads the actual slot `+184` ID, computes the signed low-32-bit pointer
difference divided by `0x188`, pushes its WORD index, increments the live count,
and lowers first-free. The assembly uses signed multiply `0x5397829D`, arithmetic
shift 7 and sign correction. It decrements explicit recursion and leaves the
section. Returning a slot does not destroy its model.

Trim frees a fully empty slab, copies the final table entry into its position,
decrements count and rewrites all 32 IDs when a slab moved, including occupied
slots. It retries the same index because the replacement can also be empty,
then recomputes the first available slab. It does not enter the critical section.

Destruction restores the pool profile and frees every slab followed by the
table. It drains positive explicit recursion through real `LeaveCriticalSection`
calls, deletes the section, switches to base profile `00D7A0C0`, and unlinks the
same element. Table/count/capacity/link fields retain native stale values. It
does not invoke any model destructor.

Returning free continuations were verified from raw installed/saved bytes:
constructor `00B74C39`, acquisition `00B74D96`, trim `00B74C86..00B74CBF`,
destruction `00B746AD` and `00B746C5`, and table cleanup's `POP ECX` at
`00B7453C`. The 316-byte acquisition extent includes both returns through
`00B74E3B`; truncated no-return pseudocode does not define the implementation.

## Exceptions and static lifetime

Constructor handler `00CC1C3E` selects FuncInfo `00DFAC1C`, with a three-entry
unwind map at `00DFAC04`. State 2 calls table cleanup via `00CC1C33`, state 1
destroys the critical section via `00CC1C28`, and state 0 unlinks the base element
via `00CC1C20`. Source preserves that order through MSVC `__finally`.

Acquisition has no local unwind region. An allocation exception leaves the
published index/capacity and held recursion in their native state. A newly
allocated slab preceding failed table growth has not yet been appended and is
not automatically freed. These failure paths were inspected statically; the
fixture does not inject allocation failure or execute native exception dispatch.

The implementation uses the existing `singleton_lifetime_allocate/free` CRT
boundary. Native slab new/free are `00BF681B`/`00BF65AC`; table new/free
`00BF55BE`/`00BF6989` forward to those implementations. Binding
`bind_static_native_model_pool_01090054` selects the one actual owner.
`initialize_static_native_model_pool_00cd7f00` constructs it and registers
`destroy_static_native_model_pool_00ce0e50`, using `std::atexit` by default or
an explicit registration function. The registration result propagates without
rolling construction back. Startup runs once; storage, domain and companion
must outlive the callback. An unbound adapter fails explicitly.

The verified 22-byte initializer at `00CD7F00` has no original function entry
in the worker's Ghidra query. Function creation and shared annotation repairs
belong to the primary integrator; this worker made no Ghidra mutations.

## Validation and limits

One isolated original-instruction comparison passed 2,089 matching states,
76 matching events and 616,714,188 compared slab bytes. Each path performs 37
real allocations and 37 frees. It allocates 1,025 slots across 33 slabs and grows
table capacity from 32 to 66; removes separated empty slabs with consecutive
retry; checks all moved IDs; returns/reuses a moved slot without changing its
payload; then returns the remaining slots and trims all slabs. Finally it refills
two slabs and destroys them with actual and explicit recursion two, verifying
that allocation frees precede lock drain and list unlink.

Two events bracket traversal of the real shared allocator list inside the first
table-allocation observer. The canonical element, profile and capacity 32 are
already published; its table remains null and count zero. Both native traversal
and the shared host domain dispatch the actual pool trim at that boundary. This
is a controlled allocation-boundary reentry, not a simulated CRT allocation
failure. Other allocation states are compared without injecting callbacks.

The fixture executes the original static initializer through an observed CRT
atexit boundary, verifies the registered `00CE0E50` callback and deliberately
returns 17; both paths preserve that result and retain the constructed pool.
Teardown invokes the registered callback. Actual process-exit scheduling is not
part of this check. The standalone table-unwind leaf is verified statically;
normal native destruction inlines its free.

The sparse native image maps only verified spans, with trap bytes/inaccessible
pages elsewhere. All 20 code/hook spans and eight data spans matched installed
and live saved bytes; 29 address relocations check their preimages. Four real
Win32 critical-section APIs and malloc/free observers execute. Every active
slab byte compares, every ID is checked before freeing, and unwritten padding
is independently asserted.

The new source passed MSVC Win32 `/W4 /WX /fp:strict` compilation and the focused
native fixture. The full baseline repository build and existing
`reconstructed_math` test passed. The primary owns CMake registration and the
combined build. No tracked tests were added. These are new C++ interfaces;
binary ABI replacement, model behavior, rendering and gameplay are unvalidated.
