# Native D3D9 surface pool

`D3D9SurfacePool` reconstructs the raw storage owner at native `0108DB00`.
It uses the same `AllocatorListDomain` and actual shared head `00E188B4`
as the directional-light and camera pools. It implements eleven functions;
surface object construction, COM ownership and reset traversal remain separate.
Exact extents, byte preimages, source hashes and validation are recorded in
[`d3d9_surface_pool_audit.json`](../reports/d3d9_surface_pool_audit.json).
The saved target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
Descriptive names are reconstruction hypotheses, not recovered source symbols.

## Storage and integration

The primary supplies one canonical `D3D9SurfacePoolStorage`, one stable companion
and the shared `AllocatorListDomain`. The companion binds concrete profile
`00D6193C`, virtual zero `00B3E390`, before native initialization publishes the
element. The domain stores dispatch associations only; links and the shared head
remain in their actual caller-supplied storage.

| Pool offset | Native field |
| --- | --- |
| `00`, `04`, `08` | Allocator profile, previous element, next element |
| `0C..23` | Actual Win32 `CRITICAL_SECTION` |
| `24` | Signed explicit recursion count |
| `28` | Actual raw slab pointer table |
| `2C`, `30`, `34` | Slab count, table capacity, first free slab index |

Compile-time assertions require MSVC Win32 and the complete `0x38` owner layout.
No field initializer zeroes caller preimages. The C++ companion destructor does
not perform native destruction, remove list links, or release a held section.
The explicit native lifecycle methods own those actions.

Each `0x744` slab contains 32 raw `0x38` slots, a 32-word free stack at `+700`,
a free count at `+740`, and two untouched bytes at `+742`. A native surface
occupies only the first `0x34` bytes of a slot. Its trailing DWORD at `+34` is the
authoritative current slab ID. Every surface constructor/destructor must preserve
that DWORD, and any enlarged C++ companion must live outside the raw slot.

`00B3D120` writes free count 32, stack indices 31 down to 0, and all 32 slot IDs.
It leaves every surface byte and the final two slab bytes unchanged. Allocation
pops the stack from its end, so a fresh slab yields slot zero first.

## Native behavior and ABI

| Address | Native action and ABI |
| --- | --- |
| `00B3EC60` | Construct: ECX pool, EAX pool, RET |
| `00B3ED40` | Allocate raw slot: ECX pool, EAX slot, RET |
| `00B3D120` | Initialize slab: ECX slab, stack slab ID, EAX slab, RET 4 |
| `00B3D860` | Return raw slot: ECX pool, stack slot, RET 4 |
| `00B3E390` | Trim empty slabs: ECX pool, RET; no internal lock |
| `00B3E2B0` | Destroy raw pool: ECX pool, RET |
| `00B3D370` | EH table cleanup: ECX address of table pointer, RET |
| `00B3F2A0` | No arguments; set ECX to `0108DB00`, tail-call allocation |
| `00B3DCC0` | Incoming ECX slot; push slot, select `0108DB00`, call return, RET |
| `00CD7B40` | Construct canonical pool, register `00CE0C90` with CRT atexit |
| `00CE0C90` | No arguments; select `0108DB00`, tail-call destruction |

Construction prepends the same allocator element, installs the pool profile,
initializes the actual section, clears the explicit fields, sets first-free to
`FFFFFFFF`, and publishes capacity 32 before allocating the 128-byte table.
The table pointer is published only after its returning old-table free tail.

Allocation enters the section and increments explicit recursion. If first-free
is `FFFFFFFF`, it publishes the current slab count as that index, allocates a
slab, and initializes it with the live index after the allocator returns. When
count equals capacity, it publishes `capacity * 2 + 2` before allocating the new
table, copies live entries, frees the old table, publishes the replacement and
appends the slab. Both the constructor and growth path preserve native reloads
after allocation, which can invoke the real CRT new handler.

Table allocation `00BF55BE` forwards to `00BF681B`; table free `00BF6989`
forwards to `00BF65AC`. The implementation reuses
`singleton_lifetime_allocate/free`, including its actual CRT allocation and
new-handler loop. Slots, unused table entries and padding are not zeroed.

Returning a slot reads its actual `+34` ID and computes its index using the
native signed pointer-difference division by `0x38`. It pushes that index,
increments the free count and lowers the first-free index when appropriate.
It neither destroys the surface nor immediately trims the slab.

Trim frees an empty slab, copies the final table entry into its position,
decrements count, and rewrites all 32 IDs when a slab moved. It revisits that
position because the replacement may also be empty. It then recomputes the
lowest available slab. No internal critical-section entry is added.

Destruction reinstalls the pool profile, frees every raw slab, frees the table,
drains positive explicit recursion through actual `LeaveCriticalSection`,
deletes the section and unlinks the same allocator element with the base
profile `00D7A0C0`. It performs no surface virtual destruction and preserves
stale table, count, capacity and link fields after their allocations are gone.

The complete post-free continuations matter: constructor `00B3ED19`, allocator
`00B3EDD6`, trim `00B3E3B6..00B3E3ED`, and destruction `00B3E2CD..00B3E2D8`
and `00B3E2E5`. Older no-return inference at CRT free omits these real paths.

## Exception handling and static lifetime

Constructor handler `00CBEE1E` selects FuncInfo `00DF7664`, whose unwind map at
`00DF764C` contains states 0 through 2. State 0 invokes allocator unlink through
`00CBEE00`; state 1 invokes critical-section destruction through `00CBEE08`;
state 2 invokes table cleanup through `00CBEE13` and continues through 1 and 0.
The implementation preserves that cleanup order with MSVC `__finally`.
`00B3D370` frees only a nonnull table pointer and does not clear it.

Allocation has no such cleanup region. If allocation throws, the published
first-free index, grown capacity and held recursion survive. A slab allocated
before a failed table growth has not yet been appended and is not automatically
freed. No C++ scope guard changes those native failure semantics.

`bind_static_d3d9_surface_pool_0108db00` binds the actual canonical owner.
`initialize_static_d3d9_surface_pool_00cd7b40` constructs it and registers
`destroy_static_d3d9_surface_pool_00ce0c90` with `std::atexit` by default, or
an explicitly supplied CRT registration function. Registration's return value
is preserved and failure does not roll back construction. Unbound adapters fail
explicitly. Startup runs once; storage, domain and companion must outlive every
registered callback. The public C++ signatures are new interfaces.

## Validation and remaining boundaries

One isolated native-versus-host fixture passed 2,089 matching states and 74
matching allocation/free observations per path, comparing 90,952,140 raw slab
bytes. Its 1,025 initial slot allocations create 33 slabs and grow the table from
32 to 66. It returns two separated empty slabs, checks consecutive removal at
the same table index and every moved ID, returns/reuses a moved slot with its
payload intact, returns the remaining slots, and trims all slabs. It then
refills two slabs and destroys the nonempty pool with actual/explicit recursion
two, checking slab-free and table-free observations before cleanup drains it.

The fixture sparsely loads exact installed executable spans, retains guard traps
outside provided bytes, and uses four actual Win32 critical-section APIs. Its
observed allocation/free boundaries use real malloc/free with deterministic raw
preimages for comparison. It does not substitute unresolved game behavior.
The separate `/W4 /WX /fp:strict` MSVC Win32 fixture build and the repository's
existing build/tests passed. Build registration is a primary integration step.

All 19 code/hook spans and eight data spans used by the audit matched live
Ghidra and the installed PE. EH leaf and FuncInfo bytes are verified evidence;
native exception dispatch, injected allocation failure and real CRT startup/exit
sequencing were not executed by this fixture. No surface constructor/destructor,
COM call, texture cache, reset traversal or render-target group was implemented
here. This is reconstructed, build-tested and fixture-tested behavior through
a new interface, not a drop-in ABI replacement or game validation.
