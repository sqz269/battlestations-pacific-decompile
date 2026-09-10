# Mesh object lifetime

Addresses: 00b93b40, 00b74280, 00b73a10, 00b93910, 00b73e60,
00b72da0, 00b727f0, 00b73c10, 00427110, 00b73840, 00419cc0,
00bd1510, 00bd12a0.

The native Mesh resource wrapper owns one reference to a separate mesh. That
mesh owns its index buffer, its vertex streams, draw sections, and copied
weight-map-name storage. Each mesh allocation occupies a C0h-byte pool slot:
BCh bytes of object storage followed by a DWORD pool-chunk index. The slot
returns to the pool only when the deleting destructor receives flag bit zero
set. It is not individually passed to CRT free.

This is a native ownership and allocation audit. It supplies contracts for
the host resource runtime; it does not implement a native class layout,
allocator, graphics object, or binary-compatible replacement.

## Evidence and limits

`reports/mesh_object_lifetime_audit.json` records 13 owned function spans,
three rechecked constructor/parser spans, two vtable entries, original names,
ABI observations, and the four stale call-flow overrides found in this packet.
Every complete span was read through the verified `bsp.py ghidra bytes` route
and matched the installed PE. The binary SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The configured target is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; each live CLI query verifies that target.

Independent field packets establish the semantic names attached to member
offsets: [vertex/index payloads](MESH_VERTEX_INDEX_PAYLOADS.md) and
[subset/LOD fields](MESH_SUBSET_LOD_FIELDS.md). The releases and allocation
layout below are directly established by this packet. Draw-section material
retention and buffer construction remain in those packets. The existing
[parser boundary](RESOURCE_MESH_PARSER_BOUNDARY.md) supplies creation-time
retention and the partial constructor defaults, whose spans were rechecked.

## Wrapper and reference transition

Table `00d63738` contains `00bd30e0` at slot zero and deleting destructor
`00b93b40` at slot one. The table for the underlying mesh, `00d62d60`, has the
same slot-zero reference-destruction dispatcher and `00b74280` at slot one.

The 10h-byte wrapper constructed by `00b947a0` begins with its vtable and
reference count, retains the mesh pointer at `+8`, and stores the serialized
prefix DWORD at `+C`. The newly constructed mesh starts at reference count
one. Wrapper construction increments it to two; releasing the local mesh
handle returns it to one. The prefix is a value, not a retained object.

Wrapper body destructor `00b93910` sets the wrapper table, unconditionally
decrements `mesh->reference_count` at `mesh+4` through InterlockedDecrement,
and invokes mesh vtable slot zero with ECX=mesh exactly when the result is
zero. It then calls resource-item base destructor `00b86890`. This path
assumes the wrapper's mesh pointer is nonnull; there is no null check.

`00b93b40` calls that body destructor, optionally calls CRT `_free` on the
wrapper when the low bit of its stack flag is set, and always returns the
original wrapper pointer in EAX with `RET 4`. The returned address can already
be freed. No reference increment, prefix cleanup, or source-node retain is
performed by either wrapper destructor.

## Mesh owners and destruction order

`00b73e60` takes ECX=mesh and ends with a plain RET. Its normal path performs
these operations in order:

1. Set table `00d62d60`. If `+60` is nonnull, atomically release that reference,
   invoke its slot zero at count zero, and clear `+60`.
2. Visit pointer slots beginning at inline offset `+64`, with DWORD count at
   `+7C`. Atomically release each nonnull pointer and invoke its slot zero at
   count zero. No count reset or pointer clearing is performed here.
3. Call `00b73c10`, which releases every pointer in the heap array `+54` with
   signed count `+58`, then calls `00b73840` on that vector with desired count
   zero. The per-element release assumes nonnull pointers.
4. Resize the 8-byte-element vector at `+B0/+B4/+B8` to zero with `00427110`,
   destroying its elements in reverse order, then free the `+B0` backing array.
5. Resize the pointer vector at `+54/+58/+5C` to zero again, then free its
   `+54` backing array. This second resize releases no references.
6. Set intermediate table `00d5c104`, call base destructor `00bd30f0`, restore
   the exception frame, and return at `00b73f4d`.

| Member | Ownership established here | Semantic link from field packets |
| --- | --- | --- |
| `+60` | Optional intrusive reference | Index buffer |
| `+64`, count `+7C` | Counted inline intrusive references | Vertex streams |
| `+54`, count `+58`, capacity `+5C` | Heap pointer array and each referenced object | Draw sections; each section separately retains its material and attached index buffer |
| `+B0`, count `+B4`, capacity `+B8` | Heap array and each nonnull element storage pointer | Copied weight-map names |

The constructor zeroes the counts and nullable owners used above. It does
not initialize every byte of the object, or all unused inline slots. A host
implementation should model the established owners explicitly; blindly
copying C0h bytes or inventing absent material defaults is not justified.

Scalar deleting destructor `00b74280` calls the complete mesh destructor,
then, when flag bit zero is set, calls `00b72da0` with ECX=`0108fff8` and the
original mesh pointer on the stack. It returns that original pointer in EAX,
`RET 4`. Calling the body destructor without the deletion flag does not
return the storage slot to the pool.

## Pool layout and invariants

Entry `00b73b60` discards the incoming ECX value BCh, loads global pool
`0108fff8`, and tail-jumps to `00b73a10`. The allocation target takes ECX=pool,
no explicit stack arguments, and returns the slot pointer in EAX with RET.

| Pool member | Directly observed role |
| --- | --- |
| `+0C` | Critical section entered around allocation and return |
| `+24` | Counter incremented on entry and decremented on exit; not a live-object count |
| `+28` | Heap array of chunk pointers |
| `+2C` | Number of chunk pointers |
| `+30` | Pointer-array capacity; growth is `2 * old_capacity + 2` |
| `+34` | Index of earliest available chunk, or `FFFFFFFFh` sentinel |

A new chunk requests exactly `1844h` bytes. Its constructor `00b727f0`
takes ECX=chunk and one stack chunk-index DWORD, returns the same chunk in
EAX, and ends with `RET 4`. It writes:

- 32 slots at stride C0h, with each slot's trailing `+BC` DWORD set to the
  chunk index. The BC-byte payload is not initialized by this helper.
- A 32-entry uint16 free-slot stack at chunk `+1800`, initialized to
  `31, 30, ..., 0`.
- A uint16 available-slot count at `+1840`, initialized to 32. The final
  two bytes of the allocation have no established role.

Allocation decrements the available count and returns
`chunk + free_slots[available_count] * C0h`. Fresh slots are therefore issued
in ascending index order. Exhausting a chunk sets the current index sentinel
and searches later chunks for a nonzero available count. Growing the chunk
pointer array copies existing pointers, frees the old pointer array, and
continues to install the new array; the call to free is not an early return.

`00b72da0` takes ECX=pool and one slot pointer, `RET 4`. It reads the chunk
index from `slot+BC`, computes `(slot - chunk_base) / C0h`, stores that uint16
index at the free stack's current count, and increments the count. It lowers
the current chunk index using an unsigned comparison, which also replaces
the `FFFFFFFFh` sentinel. It neither destroys the object nor frees its chunk.
No alignment, bounds, double-return, or full-free-stack checks occur in this
path; valid pool membership is a caller invariant.

The mesh constructor `00b73d70` writes through `+B8` and leaves the `+BC`
pool word intact. Thus BCh is the evidenced object extent within a C0h slot.
Allocation failure guarantees remain unproved: the allocation path tests
some allocation results but subsequently assumes valid chunk/array storage.
No safe null-return contract follows from those tests alone.

## Weight-map-name storage cleanup

The no-grow cleanup path of `00427110` operates on `{data,count,capacity}`
with 8-byte elements `{length,storage}`. Shrinking decrements count first;
for each nonnull storage pointer it prepares stack arguments
`(storage,length+1,1)`, calls no-argument singleton getter `00419cc0`, then
uses EAX as ECX for `00bd1510`. The latter consumes all three arguments with
`RET 0Ch`; its third argument is not read. The decompiler's inferred call
arguments incorrectly attach them to the singleton getter.

`00bd1510` sends lengths at least `96h` to CRT `_free`. Below that threshold,
when global `01090aa4` is zero, it locks the singleton's critical section
and calls `00bd12a0` to return the pointer to its size-bin storage pool. That
leaf stores the returned pointer in the pool's shared circular pointer
storage, advances masked cursors, and increments available/high-water
counters. When the global is nonzero, the small-storage path does nothing.
The meaning of that global and the complete pool allocation/lifecycle are
outside this packet. Neither path is an intrusive string reference release.

## Ghidra repair handoff and validation

Four calls had stale `CALL_RETURN` overrides despite matched native
continuation bytes: `00b93b50`, `00b73aa1`, `00b73f03`, and `00bd1525`.
The mesh destructor's second free at `00b73f1f` was not an instruction in the
saved analysis yet, because its body stopped at `00b73f07`. Native code
continues through `00b73f4d`; `00bd1510` also has a hidden `RET 0Ch` at
`00bd152f`. The audit preserves the before state and required spans for the
primary integrator to restore, annotate, and export. This worker did not
change Ghidra, shared ledgers, C++, tests, or build configuration.

Validation performed here is complete-span PE/live-byte agreement and
focused native instruction inspection. No host fixture, native ABI, D3D,
rendering, or gameplay validation is claimed by this packet.
