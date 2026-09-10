# Mesh buffer backing and object registries

Installed mesh vertex and index payloads use **private managed D3D9 buffers**.
`src/mesh_gpu_streams.cpp` implements their flags-1 creation and upload paths over
the existing logical stream APIs. The audit is
`reports/mesh_registered_buffer_backing_audit.json`; it separates the implemented
success path from CPU slab allocation, renderer registries and native retries.

## Actual mesh selection

The renderer table DWORDs at `00d5f104` and `00d5f108` are vertex factory
`00b287c0` and index factory `00b288b0`, respectively. These occupy virtual
`+5ch` and `+60h`. Mesh handlers `00b93e60` and `00b93aa0` pass flags **1**;
the factory ABI is ECX renderer, stack count/flags/declaration-or-index-format,
EAX logical stream, `RET Ch`. Each factory reorders the last two arguments for
its constructor.

| Mesh buffer | Native constructor | Length | D3D format / FVF | Pool / usage |
|---|---|---|---|---|
| Vertex | `00b4bc00` | count times resolved declaration `+CCh` stride | FVF 0 | MANAGED (1), usage 0 |
| Index | `00b4bf30` | count times 2 for `65h`, 4 for `66h` | Exact input `D3DFMT_INDEX16` / `INDEX32` | MANAGED (1), usage 0 |

Both constructors take ECX logical stream and stack count,
declaration-or-format, flags, return this in EAX and end in `RET Ch`. Their
pseudocode misidentifies reused stack locals and indirect COM arguments;
assembly establishes the argument order, output buffer pointer and ownership.

Vertex creation at `00b4bd8e..00b4be84` calls device virtual `+68h`
(`CreateVertexBuffer`), allocates a `2Ch` physical wrapper and installs vtable
`00d61e34`. Index creation at `00b4c05c..00b4c1a2` calls device virtual `+6Ch`
(`CreateIndexBuffer`), then installs wrapper table `00d61e10`. Index creation
always takes this private allocation path, including non-mesh flag values.
The flags-1 host adapter deliberately rejects other flag modes by not exposing
an arbitrary-flags argument.

The existing shared-index recreation method `00b49180` creates INDEX16. It
cannot implement private INDEX32 mesh creation, so the mesh adapter calls
`CreateIndexBuffer` with the decoded wire format. The shared recreation API
retains its established behavior.

The vertex constructor selects renderer `+1974h` only when
`(flags & F000h) == 1000h`, proved by `00b4bd26..00b4bd39`. Getter `00b1feb0`
returns this wrapper; getter `00b1fef0` returns the D3D device at renderer
`+1A10h`. Both use ECX renderer and plain `RET`, with EAX the borrowed pointer.
The dynamic branch increments the wrapper's intrusive reference count,
registers the logical stream under renderer lock `+19F4h`, and initializes its
offset to `FFFFFFFFh`. This branch allocates no private vertex buffer and
reserves no byte range. Existing startup evidence records a 16 MiB dynamic
vertex backing and 1 MiB INDEX16 backing; these are not mesh payload capacities.

## Flags, ownership and cursor

The constructors decode pool from the low nibble 0..3. Pool 0 additionally
implies WRITEONLY. Bit `10h` contributes usage 1; the `F00h` field maps
100/200/300/400/500h to usage 2/4000/40/100/80h. The `F000h` field equal to
1000h contributes DYNAMIC (`200h`); the `F0000h` field equal to 10000h
contributes WRITEONLY (`8h`). The original flags, rather than the locally
augmented flags used to derive usage, are retained in the wrappers.

Attach methods `00b4c370` and `00b4c250` take ECX wrapper and stack COM pointer,
flags, capacity, ending with `RET Ch`. They store flags at `+14h` and byte
capacity at `+18h`; when pointer `+28h` changes they AddRef the new pointer
before releasing the old one. The constructors subsequently release the
temporary returned by CreateBuffer. Each private wrapper therefore owns one
COM reference. Its registry, cursor `+1Ch`, lock depth `+20h` and dynamic lock
count `+24h` start at zero.

Vertex stream `+58h` retains its physical wrapper, `+68h` retains its
declaration, `+64h` stores count, `+60h` original flags, `+5Ch` offset 0 and
`+70h` base vertex 0. The existing base-constructor store at `00b61ea6`
initializes tag `+54h` to `40000000h`; the host adapter preserves that value.
Index stream `+8h` owns its physical wrapper; `+Ch` offset and `+20h` base
index start at zero, `+14h` holds count and `+18h` the exact input format.

The existing lock APIs copy the exact stored payload and leave static cursor
zero. Dynamic byte allocation belongs to physical Lock, which advances the
cursor and chooses DISCARD/NOOVERWRITE; physical pointer registration does
neither. The static constructors do not register streams in the physical
wrapper's dynamic registry. Host shared ownership keeps bound stream pointers
stable without inventing a static registration requirement.

Index parser virtual `+2Ch` resolves to `00b49b30` via DWORD `00d61e0c`.
Its entire body is `C3` (`RET`), followed by padding. It has no stack arguments
or established result. The additional data reference at `00d61d5c` means the
proposed no-op name is intentionally generic. This callback requires no host
action. Ghidra had no function at this address before the audit.

## CPU storage and renderer pointer arrays

`00b4b370` is not a static destructor. It executes
`MOV ECX,0108FE18h; JMP 00b4ae80`: no stack arguments, EAX a CPU object slot.
The factory's preceding ECX size value is overwritten. Pool helper `00b4ae80`
locks pool `+Ch`, maintains lock depth `+24h`, and allocates `F44h`-byte slabs.
Slab initializer `00b48eb0` creates 32 slots of `78h` bytes, writes the pool's
slab index at each slot `+74h`, fills 32 WORD free indices at slab `+F00h` in
descending order and writes free count 32 at `+F40h`. Allocation decrements
that count and returns `slab + free_index * 78h`. Exhaustion scans later slabs
for a nonzero free count. Pool fields `+28h/+2Ch/+30h` are slab pointer array,
count and capacity; `+34h` is the current usable slab or `FFFFFFFFh`.
The array grows as `2 * old_capacity + 2`. This is CPU object storage, not GPU
buffer storage. Free-slot return and pool shutdown are outside this packet.

`00b22d10` is a raw four-byte-element array reserve helper: ECX array,
stack requested signed capacity, `RET 4`, no stable return value. It clamps
requests below 1 to 1, grows only when needed, allocates capacity times four,
copies count elements in order, frees the old array, then stores the new
pointer and capacity. Count is unchanged and pointees are not retained.
The caller handles doubling. Renderer vertex factory appends at
`+1AACh/+1AB0h/+1AB4h` and, when virtual `+58h` returns AL exactly 1, also at
`+19B0h/+19B4h/+19B8h`. Index factory appends at
`+1AB8h/+1ABCh/+1AC0h` and optionally `+19C4h/+19C8h/+19CCh` through its
separate reserve helper `00b22d70`, which remains unported here. Neither
factory's append sequence AddRefs the stream; both can append a null pointer
after a failed CPU object allocation.

Two stale `CALL_RETURN` overrides obscure returning `_free` calls: `00b22d5c`
and `00b4af11`. The first hides `ADD ESP,4`, pointer/capacity stores and
`POP EBX` at `00b22d61..00b22d69`; the second hides `ADD ESP,4` at
`00b4af16`. The complete containing-function bytes match the installed PE.
The report preserves the pre-repair state for the primary integration batch.

## Host boundary and verification

The two new APIs validate count times resolved width, exact payload byte
length, vertex stride and declaration element count. They reject zero-sized
buffers, overflow and unsupported index formats, return HRESULTs, clean up
partial owners and publish output only after successful upload. Native code
has weaker failure handling: it retries creation after device recreation
only for a null result pointer and nonzero HRESULT excluding `8876017Ch` and
`8007000Eh`, then assumes a valid COM pointer. Native retries, global renderer
registration, intrusive ABI, CPU slabs, diagnostic strings and complete
constructor/base-destructor behavior are not implemented by this adapter.
Compressed vertex bytes remain unchanged; metadata shader constants are
handled separately.

Every live Ghidra query verified project `bsp`, program
`/battlestationspacific.exe` and the configured target identity. Sixteen byte
ranges, including both complete constructors, allocation helpers, attach
methods, vtable entries and the no-op, match the installed PE. Compilation and
the existing real-device probe are coordinated by the primary integrator.
This worker's audit alone does not establish build, render or game validation.
