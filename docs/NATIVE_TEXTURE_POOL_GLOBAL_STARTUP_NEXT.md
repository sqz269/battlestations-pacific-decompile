# Native texture-pool startup: remaining cube family

The next unported family is the actual cube-texture pool `0108DB70`, profile
`D61944`, with 34h-byte slots. The requested texture2D and render-target surface
pools already have complete exact-address implementations; they are routing
exclusions, not new reconstruction packets.

| Existing canonical family | Complete lifetime / trim / static entries | Existing source and exact ledger pointers |
| --- | --- | --- |
| texture2D, `0108DB38`, profile `D61940` | `B3EE80`, `B3E430`, `B3D3B0`, `B3E510`, `CD7B60`, `CE0CA0` | `src/d3d9_texture2d_pool.cpp`; function records for these addresses in `config/reconstruction/00b30000.jsonl`, `00cd0000.jsonl`, `00ce0000.jsonl` |
| render-target surface, `0108DB00`, profile `D6193C` | `B3EC60`, `B3E2B0`, `B3D370`, `B3E390`, `CD7B40`, `CE0C90` | `src/d3d9_surface_pool.cpp`; exact-address function records in those same shards |

Each entry above was individually queried through `tools/bsp.py lookup`.
Texture2D has actual-storage/native-differential checked status; surface has
typed/native-host-fixture checked status. Their allocation/return providers are
also present. `B2A7C0` render-target creation reaches the canonical surface
allocator `B3F2A0`. Current source `NativeTexture2DOwnerContext` borrows the real
texture2D pool. There is no `native_texture_pool_allocate.*` or
`native_render_texture_owner.*` file on the reviewed `3c1d701` base. No existing
pool body was reconstructed or recounted by this discovery.

## Cube identification and actual storage

The local Windows SDK `10.0.26100.0/shared/d3d9.h` declares
`IDirect3DDevice9::CreateCubeTexture` at slot25 / offset64h, returning an
`IDirect3DCubeTexture9*`. The slot is computed from declaration order including
IUnknown, not inferred from a neighboring pool's shape. Fresh native route
bytes show `B2A473` loading current renderer+1A10, `B2A49D` and `B2A4D0` reading
device slot64h, `B2A4EB` calling canonical allocator `B3F2C0`, then `B2A50B`
calling `B3D650` on the returned owner. That constructor uses SDK-confirmed
cube GetLevelDesc+44h and GetLevelCount+34h. Its profile is `D61870`, and its
unnamed logical base has profile `D5F280`. This establishes the cube family;
future descriptive C++ names remain hypotheses.

`0108DB70` is actual 38h pool storage: allocator node at +00/+04/+08, real
18h-byte Win32 critical section at +0C, signed recursion at +24, table pointer
at +28, unsigned count at +2C, capacity at +30 and earliest-free index at +34.
Profile `D61944` has one slot pointing to `B3E690`. The adjacent profiles
`D6193C` and `D61940` point to the already-complete surface and texture2D trims;
they are not interchangeable bindings.

Each 6C4h-byte cube slab contains 32 slots of 34h bytes. Logical owner bytes
occupy [0,30h); the pool owns a DWORD index at slot+30. A 32-WORD free-index
stack begins at slab+680h, its WORD count is at +6C0h, and +6C2h is untouched
padding. Valid operations require actual initialized pool/section storage and
valid reached table/slab extents. No private pool or global-head reset is needed.

## Complete candidate bodies and independent packets

| Candidate and complete native span | Original ABI | Packet |
| --- | --- | --- |
| `B3D2A0..B3D2DF`, 64 bytes | ECX actual slab, stack index, EAX same slab, RET4 | allocation5 |
| `B3F170..B3F29F`, 304 bytes | ECX actual pool, EAX actual slot, RET | allocation5 |
| `B3D940..B3D9A4`, 101 bytes | ECX pool, stack slot, RET4 | allocation5 |
| `B3F2C0..B3F2C9`, 10 bytes | no arguments; ECX=`108DB70`; tailcall allocator | allocation5 |
| `B3DCE0..B3DCEB`, 12 bytes | incoming ECX slot; push it, ECX=`108DB70`, call raw return, RET | allocation5 |
| `B3F090..B3F162`, 211 bytes | ECX pool, EAX same pool, RET; FH3 cleanup | lifetime3 |
| `B3E5B0..B3E639`, 138 bytes | ECX pool, RET; no own EH frame | lifetime3 |
| `B3D3F0..B3D3FD`, 14 bytes | ECX actual table header (`pool+28`), RET | lifetime3 |
| `B3E690..B3E72F`, 160 bytes | ECX actual pool, RET; no internal lock | trim1 |
| `CD7B80..CD7B95`, 22 bytes | no arguments, EAX `_atexit` result, RET | static2 |
| `CE0CB0..CE0CB9`, 10 bytes | no arguments; ECX=`108DB70`; tailcall destructor | static2 |

All eleven were unported at discovery. `CD7B80` has no exact Ghidra function;
an enclosing `CD7B60` lookup is not this function. `B3F2C0` is incorrectly tagged
as a static-destructor stub and `CE0CB0` as a static initializer. The other
candidates retain `FUN_<address>` names. The root approved allocation5 for its
worker after byte verification; remaining packets retain disjoint ownership.

Each packet owns only its `.hpp` under `include/bsp/`, matching `.cpp` under
`src/`, uppercase document and matching `reports/*_audit.json`:

1. `native_cube_texture_pool_allocate` owns the five allocation entries above
   in `native_cube_texture_pool_allocate` files. It is source-ready against
   actual valid initialized pool storage and existing CRT/Win32 services.
   Canonical C++ adapters may explicitly borrow actual `0108DB70` storage;
   they must not invent a private global. Initialization and list binding are
   composition preconditions, not stub callees.
2. `native_cube_texture_pool_trim1` owns only `B3E690` in
   `native_cube_texture_pool_trim` files. Frozen APIs are
   `trim_native_cube_texture_pool_00b3e690(void*) noexcept` and
   `bind_native_cube_texture_pool_trim_00d61944(void*, AllocatorListDomain&)`.
   Binding supplies the actual node, `D61944`, `B3E690`, actual pool and real
   trim function. It may allocate host dispatch metadata before publication.
3. `native_cube_texture_pool_lifetime3` owns `B3F090/B3E5B0/B3D3F0` in
   `native_cube_texture_pool_lifetime` files. It borrows actual pool/list state.
   Source work is independent; real initialization composition requires trim1's
   genuine binding before the node becomes visible to actual new-handler work.
4. `native_cube_texture_pool_static2` owns `CD7B80/CE0CB0` in
   `native_cube_texture_pool_static` files. It depends on lifetime3, trim1,
   one canonical actual pool/list binding and real `std::atexit`. It neither
   initializes a private list nor supplies an inert registration collector.

Shared list `403970/4B46B0`, section `402F70`, and CRT services are dependencies,
not new owned functions. Shared CMake, ledger, packet and Ghidra edits stay with
the integrator. All proposed C++ APIs differ from the original binary ABI.

## Allocation and cleanup contracts

`B3D2A0` writes free count32 first, then 32 ordered pairs: WORD `31-i` at +680,
DWORD supplied index at slot+30. It preserves owner payload and +6C2. Raw
allocation enters the actual section and increments explicit recursion. When
earliest is `FFFFFFFF`, it publishes current count as earliest before calling
actual `BF681B(6C4h)`, then initializes a successful slab using current earliest.
If current count equals current capacity, it publishes `capacity*2+2` before
`BF55BE(4*capacity)`, copies current table/count, frees the current old table,
and publishes the captured replacement. It appends the captured slab and
increments count. It pops the free-index stack, computes `slab+index*34h`, and
if that slab is now full scans later current entries for available space.
Normal returns decrement recursion and leave the actual section. There is no
EH frame: allocation failure retains reached lock/depth/earliest/capacity state
and does not free an already-created slab or perform an invented rollback.

Raw return loads the slot's current DWORD pool index after entering the
section, captures the indexed slab, and computes signed low32 `(slot-slab)/52`
using the exact `IMUL 4EC4EC4F`, arithmetic shift and sign correction. It stores
the resulting WORD at the current free-stack count, then increments the current
WORD count, lowers earliest when needed, decrements recursion and leaves.
Canonical return `B3DCE0` is also reached by constructor unwind funclets
`CBD388` and `CBD530`; it returns the slot without destroying its logical owner.

The pool constructor prepends the base-profile node into the actual `E188B4`
list, arms state0, installs `D61944`, initializes the real section, writes
depth/table/count/capacity=0 and earliest=`FFFFFFFF`, then arms state2. Capacity32
is published before actual allocation128. Current table/count are reloaded for
copying; current old table is freed before publishing only the captured new
pointer. Real new-handler changes to count/capacity remain observable.

Handler `CBEE7E`, FuncInfo `DF76DC`, map `DF76C4` run states2 -> 1 -> 0 -> -1:
funclet `CBEE73` calls `B3D3F0` on captured pool+28, `CBEE68` calls `402F70` on
pool+0C, and `CBEE60` calls `403970` on the actual pool. Table cleanup frees its
nonnull first word and clears nothing. State0 section-initialization failure
unlinks only the node; no artificial Win32 failure callback is implied.

`B3E5B0` captures the initial nonempty-count test before installing `D61944`,
then frees current slab pointers while reloading current count. It frees the
current table, decrements positive signed depth before each real Leave, then
deletes the section. Inline base unlink captures current previous before
`D7A0C0`, repairs predecessor or actual head, reloads next and repairs its
previous. Own links/table/count/capacity/earliest remain stale. It does not
destroy logical slot owners or free the raw pool storage.

Trim frees only slabs with WORD free count32. Its returning-free tail reloads
current table/count, copies the final pointer into the current index before
decrementing count, rewrites all 32 moved slot IDs at +30 with stride34h, and
rescans that index. Ignored trailing table pointers remain stale. Final scanning
captures the nonempty-count test before writing earliest=`FFFFFFFF`, captures
the table once when needed, and rereads count for each scan test. Trim neither
enters nor leaves a critical section.

## CRT registration, remaining owners and evidence limits

Initializer table word `CE3518` is exactly `CD7B80`, within the native C++
initializer interval `[CE2734,CE36E4)` dispatched by `__cinit` `BFBC47` after
successful C initialization. The complete stub constructs actual `108DB70`,
then registers actual `CE0CB0` through `_atexit` `BF6FF5`. Its return is the
registration result; the CRT dispatcher ignores it, and failure does not roll
back the pool. Shutdown sets that same actual pool and tailcalls `B3E5B0`.
Existing real host CRT allocation/free and `std::atexit` boundaries suffice;
original CRT global heap/callback tables are not reconstructed by these packets.

Do not mark the cube logical owner complete from this pool work. Exact named
base `B34020` has no complete reconstruction record; `B3D650` constructor,
`B3EAD0` destructor and `B3F410` deleting destructor are also incomplete.
`B2A380` creation and loaded-file factory `B2C2D0` are outside this packet map.
Pool allocation/return needs none of those owner operations, and destruction
does not call them. The ready map therefore avoids placeholder owner methods.

Fresh proof covers 29 complete code/data spans / 1,795 bytes, including all
1,046 candidate bytes, plus two route spans / 279 bytes. Every guarded read
verifies `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`; all match
installed PE SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The report includes every raw span/hash, the SDK declaration evidence, exact
ABI, exclusions, named-but-incomplete dependencies, ownership and readiness.
Image zero fill for pool/head proves only loader preimage, not live game state
or permission to reset globals.

Saved flow omits proven returning-free continuations after `B3F144`,
`B3E5C8`, `B3E5E0`, `B3E6B1`, `B3D3F7`, and `B3F201`. The report lists exact
gap ranges, including the 55-byte trim compaction tail. Repair those call-site
flow overrides and decode the gaps under the write lock before refreshing
exports; leave jump-alignment gaps alone. Create exact `CD7B80` before naming it,
preserve prior comments and correct CRT symbols. This discovery made no Ghidra
or C++ mutation and claims no build, differential execution or game validation.
