# Native hardware-layout construction prerequisites

Discovery packet `native_hardware_layout_construction_next`, based on checkout
`8a5ad85` and live `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`.
This packet adds evidence and proposed work boundaries only. It does not add
native implementations, change Ghidra, or upgrade reconstruction counts.
Descriptive names below are hypotheses, not recovered symbols.

The concrete route is renderer profile `00d5f0a8` slot `+40h` -> **`00b2f710`**
cache factory -> `00b606f0` / `00b605b0` hardware-pool allocation ->
**`00b60cb0`** derived constructor -> `00b48c00` base initialization and
`00b60790` stream conversion/COM creation. The resulting owner installs
`00d62af4`, whose three entries are `00bd30e0`, `00b60770`, and `00b5ff00`.
Thus its deleting slot reaches the existing actual-storage owner teardown.
The adjacent renderer slot `+44h` is `00b2f4c0`, the existing tree-removal route.
These immutable table bytes establish the supported profile, not the profile
of a running renderer.

`00b2f710` and record initializer `00b47640` are currently absent from the
function inventory. The factory occupies `[00b2f710,00b2f7f7)`; it is not part of
the preceding `00b2f700` five-byte thunk. Its direct call at `00b2f7a0` explains
why the old snapshot reports no callers for `00b60cb0`. Likewise, `00b606f0`
is an allocation wrapper despite its `CG_static_dtor_stub` tag, and
`00b48c00` is a concrete base constructor despite its array-helper tag.
Function creation/annotation belongs to the implementing owner under a lease
and Ghidra write lock, followed by export/index refresh.

## Complete constructor and factory behavior

| Entry | Native ABI and complete body | Behavior and cleanup |
| --- | --- | --- |
| `00b47640` | ECX record; EAX same; RET; 17h bytes | Write the 0Ch record `{0,1,0}` in field order. |
| `00b48c00` | ECX owner; EAX same; RET; 71h bytes | Write base profile `CEB130`, reference count 1, then `D61D10`; construct four records at `+08h` through `BF7CD1` with `B47640/B483F0`; write count `+38h=0`, then stride `+3Ch=0`. Preserve `+40h` and trailing pool index `+44h`. |
| `00b60cb0` | ECX owner, stack stream-list pointer; EAX same; RET4; 51h bytes | Complete `B48C00`; arm base cleanup; clear `+40h`; install `D62AF4`; call `B60790`. An exception runs **base `B48960`**, not derived `B60700`. |
| `00b60790` | ECX owner, stack stream-list pointer; RET4; 271h bytes | Append/retain each stream, convert its actual raw declaration records, perform both diagnostic string lifetimes and actual support access, read the current renderer/device, call COM creation, and recompute stride even after a failed HRESULT. |
| `00b2f710` | Renderer ECX unused; stack key/stream-list pointer; EAX owner; RET4; E7h bytes | Search actual tree `108D530`. A hit increments the found owner's intrusive count and returns it. A miss allocates, constructs, makes two raw key/value copies, inserts, and returns the newly constructed pointer. |

Factory hit validation checks the iterator's owner against actual `108D530`,
and verifies that the selected node is not the current head before dereferencing
value `node+20h`. The miss passes `ECX=44h` to `B606F0`; that wrapper ignores the
size and replaces ECX with `108FE9C` before tail-calling `B605B0`.
The raw slot is guarded only after allocation returns and until construction
returns. On constructor failure the factory cleanup calls `B60260` -> existing
`B60110` to return the raw slot. The guard is disarmed **before** key copies and
insertion: there is no factory owner/slot cleanup if those later operations
throw. The insertion result and its duplicate flag are ignored. Do not add a
cache-owned AddRef or a post-construction failure cleanup absent from the bytes.
If raw allocation returns null, construction is skipped but a null value still
goes through pair creation and insertion; the code does not return early.

`B60790` reads four pointer positions followed by the signed stream count at
input `+10h`; its actual loops do not enforce a four-stream limit. For each
stream it first calls `B48A00`, then reads declaration data/count at `+0Ch/+10h`.
Five-DWORD, 14h-byte records become eight-byte D3D elements. Stream/offset use
low WORDs; type/method/usage use low BYTEs. Fourteen usage counters are shared
across all streams and indexed by the full usage DWORD. The END is DWORDs
`000000FF,00000011`. The packed elements begin at frame `ESP+68h`; the live
element-count word is at `ESP+E8h`. No native bounds checks justify replacing
this scratch behavior with an unbounded vector. Any valid-input restriction
for a host interface must be explicit.

Each iteration constructs the real 8-byte temporary string for the 13-byte
`VertexFormat\0` literal at `D61BD0`, captures the current owner COM `+40h`
into a borrowed 0Ch diagnostic record, copies the string into that record,
calls actual `B3E730`, then frees the diagnostic name and temporary string in
that order. These calls remain observable even though the diagnostic support
getter receives no record argument. At `B6099C` the code reloads current global
renderer `F8D394`; `B609C6` reads its current device at `+1A10h`, then the current
COM table's `+158h` entry. It passes the actual output address `owner+40h`.
There is no separate device-getter function and no cached device parameter.

## Exception maps and existing dependencies

| Owner | FuncInfo / unwind map | States and action |
| --- | --- | --- |
| Base constructor `B48C00` | `DF8194 / DF818C` | State 0 -> -1, `CBF770`: captured owner -> `BD30F0`. The fixed four-record CRT construction iterator owns its completed prefix. |
| Stream append `B48A00` | `DF80F8 / DF80F0` | State 0 -> -1, `CBF6F0`: stack record at `EBP-18h` -> actual `B483F0`. |
| Stream creation `B60790` | `DFA0B8 / DFA0A8` | State 0 -> -1, `CC1310`: temporary string `EBP-DCh` -> `41DD20`; state 1 -> 0, `CC131B`: diagnostic `EBP-D4h` -> `B3F4C0`. |
| Derived constructor `B60CB0` | `DFA118 / DFA110` | State 0 -> -1, `CC1350`: captured owner -> `B48960`. |
| Cache factory `B2F710` | `DF60A8 / DF60A0` | State 0 -> -1, `CBD800`: raw allocation saved at `EBP+4` -> `B60260`. |

Current native dependencies already exist: owner/base destruction and raw slot
return in `native_hardware_layout_owner`, actual CPU declaration deletion in
`native_vertex_declaration_owner`, actual tree erasure/rotations in
`native_hardware_layout_tree`, actual header string resize/destruction in
`native_string`, diagnostic record destruction in `native_physical_buffer_owner`,
and actual support access in `native_resource_support`. Their exposed host
allocator/string/exception service boundaries remain explicit; the reconstructed
process is not the original game CRT or exception ABI.

The ledger already calls `B48A00` and `B47D60` reconstructed functions, but their
current bodies in `src/d3d9_vertex_layout.cpp` use the host `shared_ptr` layout.
They are **missing native-storage dependencies**, not reusable native bodies.
`B60790` is only named; `B60A10` is a conversion fragment; the latter cannot
replace full creation with its strings, support calls, and current device reads.

Native append first retains a stack temporary, then increments the current
owner count before accessing the selected record. If replacement differs, it
publishes the new pointer, retains it, and releases the captured old pointer
through its current virtual profile. Metadata becomes `{frequency=1,extra=0}`
only after that release returns; it disarms the temporary guard before the final
temporary release. Identity skips still write metadata. Native stride clears
`+3Ch` and sums actual `declaration+CCh` DWORDs with a signed, current-count loop.
Neither function adds the host interface's aborts or shared ownership.

## Pool and tree prerequisites

`B605B0` borrows the initialized 38h pool at `108FE9C`, its Win32 critical section
at `+0Ch`, recursion count `+24h`, slab pointer table/count/capacity at
`+28h/+2Ch/+30h`, and earliest free slab at `+34h`. If no slab is available it
publishes the next slab index, allocates 944h bytes through `BF681B`, and calls
`B5FF50`. That initializer sets WORD free count `+940h=32`, writes free indices
`31..0` at `+900h`, and writes each slot's trailing DWORD slab index at
`slot+44h`. It leaves all 44h-byte owner payloads untouched.

Pool table growth publishes capacity `2*capacity+2` before `BF55BE`, copies
existing pointers, frees the captured old table, then publishes the new table.
The decompiler's return after `BF6989` is false: bytes at `B60646` are
`add esp,4`, followed by table publication. Allocation decrements the slab WORD
count before reading its selected free index, returns `slab+index*48h`, and scans
later slabs when necessary. It releases the lock only on its normal paths;
there is no pool-allocation EH map or exception cleanup to justify an added
unlock guard. The existing common allocator service may be reused, with its
documented host CRT boundary.

Full pool startup is separately located at undefined `CD7CA0` -> `B604D0`, with
shutdown registration `CE0D40` -> `B60270`. `B604D0` links the real allocator
list `E188B4`, installs pool profile `D62AF0`, initializes the critical section,
and allocates an initial 32-entry slab table. The proposed allocation packet
borrows that already initialized object; it must not fabricate zero globals or
silently claim full startup/shutdown reconstruction.

The actual 28h tree node layout is now established by `B29CD0/B28370`: links at
`+0/+4/+8`, four raw declaration pointers at `+0Ch..+18h`, signed key count at
`+1Ch`, raw owner value at `+20h`, color at `+24h`, sentinel at `+25h`; the last
two padding bytes are untouched. Keys order count **descending** as signed
DWORDs, then pointer values **ascending** as unsigned DWORDs. This is pointer
identity ordering, not a declaration-content comparator.

The missing key/find/copy entries are `B20BF0`, `B23020`, `B28220`, `B282B0`,
and `B25EF0`. Forward copies reload the source count on each iteration and
leave unused key words unchanged. Missing insertion uses `B20D30`, `B28370`,
`B29CD0`, `B2F1B0`, and `B2F540`. `B2F1B0` is a complete node link/rebalance
routine with a length-error path, not merely its `STL_xlen_throw` tag. At
unsigned tree count >= `0AAAAAA9h`, it constructs the existing native legacy
logic-error base then installs `D69260` and throws with descriptor `D83F98`.
Its exception transport must be completed as an owner; an arbitrary host
`length_error` or throw callback is not evidence of the same native state.

## Disjoint implementation packets

| Packet | Exact addresses | Readiness and files |
| --- | --- | --- |
| `native_hardware_layout_fields` | `B47640 B48C00 B48A00 B47D60` | Ready; dispatched to the fields worker during discovery. `include/bsp/native_hardware_layout_fields.hpp`, `src/native_hardware_layout_fields.cpp`, matching uppercase doc and `_audit.json`. |
| `native_hardware_layout_pool_allocate` | `B5FF50 B605B0 B606F0 B60260` | Ready against initialized actual pool and existing `B60110`; separate `native_hardware_layout_pool_allocate` header/source/doc/audit. |
| `native_hardware_layout_tree_key` | `B20BF0 B23020 B28220 B282B0 B25EF0` | Ready independently; integrator preparing it. Separate `native_hardware_layout_tree_key` header/source/doc/audit. |
| `native_hardware_layout_construct` | `B60790 B60CB0` | Ready after fields; reuse existing owner/string/diagnostic/support services and a current renderer/device view. Separate `native_hardware_layout_construct` files. |
| `native_hardware_layout_tree_insert` | `B20D30 B28370 B29CD0 B2F1B0 B2F540` | After tree-key packet; retain existing rotations and implement the owning length-error transport in this packet's files. |
| `native_hardware_layout_factory` | `B2F710` | After constructor, pool allocation, tree-key, and tree-insertion packets. Separate `native_hardware_layout_factory` files. |

Exact proposed paths, contracts, dependencies, body hashes, and pending startup
work are machine-readable in `reports/native_hardware_layout_construction_next.json`.
The primary agent owns shared CMake/ledger/tag integration. Claims must follow
the current leases and completed implementations, not this discovery snapshot.

Validation: 30 complete code ranges (including unwind actions/handlers) and
10 immutable data/EH ranges matched
live Ghidra bytes to the installed binary. Complete assembly was inspected for
constructor/factory/pool paths, including raw gaps and unwind actions. No C++
changed, so no build or new tests were required. This is static discovery and
byte parity; it does not establish runtime construction, ABI compatibility,
the running renderer's profile, or game behavior.
