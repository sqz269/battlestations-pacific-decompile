# Native D3D9 texture2D pool

`D3D9Texture2DPool` reconstructs eleven functions for the raw storage owner at
`0108DB38`. It uses the existing `AllocatorListDomain` over the same actual
`00E188B4` head as the other allocator pools. It allocates storage for texture
owners; texture construction, COM ownership, caches, retained sources and
renderer registration belong to separate packets.

[`d3d9_texture2d_pool_audit.json`](../reports/d3d9_texture2d_pool_audit.json)
records exact extents, original ABI, byte preimages, proposed names, source
hashes and validation. The verified analysis target is
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Descriptive names are
reconstruction hypotheses, not recovered source symbols.

## Actual storage and integration

The primary supplies one canonical `D3D9Texture2DPoolStorage`, one stable
`D3D9Texture2DPool` companion and the shared `AllocatorListDomain`. The companion
binds profile `00D61940`, virtual zero `00B3E510`, before initialization publishes
the element. It adds no second list, slab table or slot state. The C++ companion
destructor does not perform the explicit native lifecycle.

| Pool offset | Native field |
| --- | --- |
| `00`, `04`, `08` | Allocator profile, previous element, next element |
| `0C..23` | Actual initialized Win32 `CRITICAL_SECTION` |
| `24` | Signed explicit recursion count |
| `28` | Actual slab pointer table |
| `2C`, `30`, `34` | Slab count, table capacity, first free slab index |

Compile-time assertions require MSVC Win32, a 24-byte critical section and the
complete `0x38` owner layout. Caller storage has no field initializers.

Each `0xAC4` slab has 32 raw `0x54` slots, 32 free-index words at `+A80`, a free
count word at `+AC0`, and two unwritten bytes at `+AC2`. The texture owner occupies
`[0, 0x50)`; the DWORD at slot `+50` belongs to this pool and stores the current
slab index. Every texture constructor/destructor must preserve it. Any enlarged
host companion must live outside the raw slot.

`00B3D1E0` writes count 32, indices 31 down to 0, and all 32 trailing slot IDs.
It preserves every texture byte and the final two slab bytes. Allocation pops
the last free index, yielding slot zero first on a fresh slab.

## Native functions and behavior

| Address | Original ABI and action |
| --- | --- |
| `00B3EE80` | ECX pool, EAX pool, RET; constructor |
| `00B3EF60` | ECX pool, EAX raw slot, RET; allocator |
| `00B3D1E0` | ECX raw slab, stack slab ID, EAX slab, RET 4; slab initialization |
| `00B3D8D0` | ECX pool, stack raw slot, RET 4; return slot |
| `00B3E510` | ECX pool, RET; trim empty slabs without an internal lock |
| `00B3E430` | ECX pool, RET; destroy raw pool |
| `00B3D3B0` | ECX address of table pointer, RET; free nonnull table, preserve header |
| `00B3F2B0` | Select `0108DB38`, tail-call allocation; EAX slot |
| `00B3DCD0` | Incoming ECX slot; push slot, select `0108DB38`, call return, RET |
| `00CD7B60` | Construct canonical pool, register `00CE0CA0`; EAX atexit result, RET |
| `00CE0CA0` | Select `0108DB38`, tail-call destruction |

Construction prepends the actual allocator element, installs the pool profile,
initializes the section, clears the explicit fields, and sets first-free to
`FFFFFFFF`. It publishes capacity 32 before allocating the 128-byte table.
The table pointer is published after the returning old-table free continuation.

Allocation enters the section and increments explicit recursion. When first-free
is `FFFFFFFF`, it publishes the current slab count as first-free, allocates the
slab and initializes it with the live index after the allocation returns. If
count equals capacity, it publishes `capacity * 2 + 2` before allocating the new
table, copies live entries, frees the old table, publishes its replacement and
appends the slab. The implementation preserves native reloads across these
allocation calls, which may invoke the CRT new handler.

Native slab allocation/free use `00BF681B`/`00BF65AC`; table allocation/free use
`00BF55BE`/`00BF6989`, which forward to the same implementations. Source code
uses the existing real CRT `singleton_lifetime_allocate/free` seam. Raw payload,
unused table entries and padding are not cleared.

Return reads the actual slot `+50` ID and divides the signed 32-bit pointer
difference by `0x54`. Its assembly uses signed multiply `0x30C30C31`, shift 4
and sign correction. It pushes the resulting WORD index, increments the live
free count, lowers first-free when appropriate, then decrements explicit
recursion and leaves the actual section. It does not destroy a texture.

Trim frees an empty slab, copies the final table entry into its position,
decrements count and rewrites all 32 IDs if a slab moved, including IDs of
occupied slots. It retries the same index because the replacement may also be
empty, then recomputes the lowest available slab. It adds no lock entry.

Destruction reinstalls the pool profile, frees all slabs and then the table,
drains positive explicit recursion through actual `LeaveCriticalSection`,
deletes the section, and unlinks the same allocator element with base profile
`00D7A0C0`. It invokes no texture destructor. The table, count, capacity and
link fields retain native stale values after their allocations are freed.

Full post-free continuations were verified independently of Ghidra's no-return
inference: constructor `00B3EF39`, allocation `00B3EFF6`, trim
`00B3E536..00B3E56D`, destruction `00B3E44D` and `00B3E465`, and the table
cleanup's `POP ECX` at `00B3D3BC`. Function end addresses in the audit are
exclusive; the allocator includes both returns through `00B3F08F`.

## Exception handling and static lifetime

Constructor handler `00CBEE4E` selects FuncInfo `00DF76A0`; its unwind map at
`00DF7688` has three entries. State 2 invokes table cleanup through `00CBEE43`,
then state 1 destroys the actual critical section through `00CBEE38`, then
state 0 unlinks the allocator element through `00CBEE30`. The implementation
preserves that cleanup order using MSVC `__finally`. Table cleanup frees only
a nonnull pointer and does not clear it.

Allocation has no local unwind region. If an allocation throws, the published
first-free index, capacity and held recursion are not rolled back. A slab
allocated before failed table growth has not yet been appended and is not
automatically freed. No scope guard changes those native failure semantics.

`bind_static_d3d9_texture2d_pool_0108db38` binds the one actual owner.
`initialize_static_d3d9_texture2d_pool_00cd7b60` constructs it and registers
`destroy_static_d3d9_texture2d_pool_00ce0ca0`, using `std::atexit` by default or
an explicit CRT registration function. It preserves the registration result;
failure does not undo construction. Unbound adapters fail explicitly. Startup
runs once, and storage, domain and companion must outlive the callback.

The 22-byte initializer at `00CD7B60` is not currently a Ghidra function. Its
complete instructions and exact call to the destructor registration were read
as bytes; creating that function and applying annotations belong to the primary.

## Validation and limits

One isolated original-instruction comparison passed 2,089 matching states and
74 allocation/free events per path, comparing 134,765,644 raw slab bytes.
It allocates 1,025 slots to create 33 slabs and grow capacity from 32 to 66;
returns two separated empty slabs; checks consecutive removal at the same
index and every moved ID; returns/reuses a moved slot with its payload intact;
then returns all remaining slots and trims all slabs. Finally it refills two
slabs and destroys them with actual and explicit recursion two, checking all
slab/table frees occur before lock cleanup and shared-list unlink.

The sparse fixture maps only verified installed executable spans, with trap
bytes and inaccessible pages elsewhere. It uses the actual four Win32
critical-section APIs and real malloc/free observers with deterministic raw
preimages. Every active slab byte compares; the unwritten tail is checked
independently. All 19 code/hook spans and eight data spans matched live Ghidra
and the installed PE, with 29 guarded address-word relocations.

The source passed a separate `/W4 /WX /fp:strict` MSVC Win32 compile and native
fixture. The full repository build and existing `reconstructed_math` test passed
after incorporating primary integration commit `48ebd1a`. Primary integration
owns CMake registration for this new source. No tracked tests were added.

EH leaf/map bytes were verified, but native exception dispatch, allocation
failure injection and original CRT startup/exit registration were not executed.
The fixture uses direct construction and the actual static destruction thunk.
This is reconstructed, build-tested and fixture-tested behavior through new
C++ interfaces, not a drop-in ABI replacement or game validation.
