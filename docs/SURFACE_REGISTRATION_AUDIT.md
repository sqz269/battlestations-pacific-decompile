# Surface registration correction audit

`00b3e730` does not register surface wrappers for renderer reset. The earlier
inference that calling this getter inside `00b3f630` implied resource-registry
participation was incorrect. A real renderer-owned surface callback list exists,
but registration occurs explicitly in renderer factories, separately from the
singleton getter and surface allocation pool.

## What the singleton actually establishes

`00b3e730` checks global `0108fedc`, obtains a shared lifetime-manager lock via
`00415350`, checks the global again under that lock, allocates **eight bytes**,
and calls `00b61d50`. It stores the result in `0108fedc`, passes that singleton
to `00bd0c30`, releases the lock, and returns the singleton pointer. The generic
manager's lifecycle is audited separately by the parent; that call receives
the singleton, not an individual surface.

Constructor `00b61d50` is nine bytes: `MOV EAX,ECX; MOV [EAX],00d62b64; RET`.
It initializes only the vtable word. The second allocated DWORD is untouched by
this constructor. The proven vtable entry at `00d62b64` points to deleting
destructor `00b61d60`, which clears `0108fedc`, installs base vtable `00ce3818`,
and conditionally frees the singleton when its stack flag has bit zero set.
Words following the first entry must not be assumed to be methods of this
singleton without independent vtable-boundary evidence.

The surface constructor calls this getter at `00b3f73d`, passes no surface,
and does not use its returned EAX. The surface destructor calls it at
`00b3f517` and likewise does not use its return value. The getter's direct
xref list contains 18 calls across surface, buffer, texture/layout-related
routines; broad use does not itself establish resource registration. Global
`0108fedc` xrefs are getter reads/writes and initialization/destruction writes,
not a per-surface list.

This establishes lazy singleton construction and lifetime ownership, but not
its original class name or intended diagnostic role. Calling it a surface
registry or requiring a fake registration operation here would invent behavior.

## Surface constructor and allocation are separate mechanisms

The complete `00b3f630` body establishes intrusive count one, wrapper vtable,
metadata, borrowed-input COM balancing, retained COM surface through `00b3cc80`,
temporary string operations, the singleton getter call, and a tracking-counter
increment when flags contain `10h` or `100h`. It contains no append to the
renderer's surface callback array.

`00b3f2a0` is an **allocator thunk, not the surface destructor**:
`MOV ECX,0108db00; JMP 00b3ed40`. It routes allocation through a pool object.
The allocator locks the pool, selects/creates a `744h`-byte block, and returns
one slot using `38h` stride and a free-index array; a 52-byte surface object
fits that 56-byte slot. This is storage bookkeeping. Direct xrefs to pool
global `0108db00` cover allocation/deallocation and global lifetime sites;
they do not connect that pool to reset iteration.

The actual surface destructor is `00b3f4e0`; deleting wrapper `00b3f5b0` calls
it, then returns the storage to the pool through `00b3d860` when requested.
The allocator's pseudocode has a misleading early return after `_free` in
its capacity-growth branch, so this audit does not certify a full allocator
reconstruction from pseudocode. The pool does not need to be recast as a
reset registry to explain the observed allocation path.

## Proven reset registration and removal

Renderer fields `+1b0ch`, `+1b10h`, `+1b14h` form the surface pointer array,
count, and capacity. The following explicit path establishes participation:

- Offscreen render-target factory `00b2a7c0` calls the surface constructor with
  flags `10h` and recreation kind zero (`00b2a884..00b2a88b`). It then selects
  renderer `+1b0ch`, grows capacity through `00b22850` if necessary, writes the
  wrapper pointer at `00b2a8cc`, and increments count at `00b2a8d2`. No intrusive
  increment occurs in that append sequence.
- Reset release `00b262c0` separately handles the renderer's default depth
  owner and four render-target owner slots, then enumerates this array and
  invokes each surface's virtual `+3ch` release callback. Array traversal is
  visible at `00b263ff..00b26423`.
- Reset restore `00b23b10` reacquires default render/depth surfaces into their
  existing wrappers, then enumerates the array and invokes virtual `+40h`
  recreation with the device. Traversal is at `00b23beb..00b23c16`.
- Surface destructor `00b3f4e0` obtains renderer singleton `00f8d394` and calls
  `00b27d60(surface)` at `00b3f512` **before** its singleton getter and COM
  release. `00b27d60` selects renderer `+1b0ch` and calls `00b25630` with the
  address of its surface-pointer argument.
- `00b25630` searches for the first matching pointer, replaces it with the
  final array element when necessary, and decrements count. Missing entries
  leave the list unchanged. It does not release the surface; destruction is
  already in progress. Its return boolean reports whether an entry was found.

The default capture routine `00b238d0` constructs and installs wrappers at
`+197ch` and `+198ch`; it does **not** perform that callback-array append.
Default wrappers still participate in reset through their explicit owner
slots, so absence from the offscreen array does not mean absence from reset.
Both are constructed with flags zero and recreation-kind zero; the depth
wrapper must not be assigned a different kind by inference from its format.

An additional literal reference to `+1b0ch` exists at raw `00b2ab28` in a
neighboring factory not defined in the current function snapshot. It was not
needed to prove the path above, and its full behavior is outside this audit.
No claim that `00b2a7c0` is the only factory is made.

## Corrections for implementation handoffs

The singleton call is a real lifecycle side effect and should be represented
or explicitly scoped out; it is not evidence of an unresolved per-surface
registration dependency. Default capture can be reconstructed with its actual
COM/intrusive ownership semantics without inventing a registration call.
Full offscreen-surface creation additionally needs the renderer-owned callback
array and destructor removal. Resource allocation pooling, generic singleton
lifetime ownership, and reset callback membership are distinct mechanisms
with different addresses and owners.

Any older documentation describing `00b3e730` as the resource registry, or
claiming registration occurs inside the surface constructor, should be
corrected. The prior `RENDERER_INIT_HANDOFF.md` sentence tying constructor
completion to unresolved registry behavior was an unsupported inference;
the concrete distinction above supersedes it.

## Evidence boundary

Every Ghidra batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Current complete or
explicitly bounded bodies matched installed PE bytes exactly:

| Address and scope | Bytes | SHA-256 |
|---|---:|---|
| `00b3e730`, complete getter | 189 | `7b9c8865a04b8c98121a241786a0d3703768772cb806f8f557b00270e77c81d9` |
| `00b61d50`, complete singleton constructor | 9 | `6c0ec936e146005bb724c1e96a0a3844a76634ccfea694f9567a58828f37f039` |
| `00b61d60`, complete singleton deleting destructor | 41 | `2ef739c60e79ada0fd96d7f40a8491598626c692a9bfa6f325257b17d3c76854` |
| `00b3f630`, complete surface constructor | 382 | `f59553f64d394a4b3609c84d5d504377bcb3de224a47ef97914c50e39e4a5023` |
| `00b3f4e0`, complete surface destructor | 168 | `94c7941ad588fe299ed80887bbe63e585e649e0098aa291c1d98894fa43dce28` |
| `00b3f2a0`, complete allocator thunk | 10 | `05d840519b2b51aa7a10ce3948c060c5eba2f01fd09ee92dafe2b8bc16b93ff8` |
| `00b27d60`, complete unregister adapter | 19 | `9077346ff1ae7fd162137608cb8fd9977361825dde7a9820c82cbf299c88f692` |
| `00b25630`, complete pointer removal | 103 | `24d2aab156d7889ea7543c821483002b5f93b654671b47704e8253690ef1b8d0` |
| `00b2a869..00b2a8d5`, factory allocation/append fragment | 109 | `6a5c732b8cffbe41a1662478fbef87b95d93c869fa9a55de2b00a77763aef50b` |

This is a read-only correction audit, not runtime ownership/reset validation.
Only this document was edited; exports were refreshed without Ghidra mutation.
