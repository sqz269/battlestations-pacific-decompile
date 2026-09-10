# Mesh resource parser boundary

Addresses: 00b72b40, 00b73b60, 00b73d70, 00b944e0, 00b94710, 00b947a0, 00be99f0.

The registered Mesh parser `00b947a0` returns a 10h-byte resource item that
retains a separate mesh object and stores one serialized DWORD beside it.
The mesh record begins with that DWORD, followed by named child records.
Nine child tags are recognized. Only the prefix and `LODValue` scalar format
are decoded directly within this packet; the geometry field helpers remain
the concrete next dependencies. Skipping a Mesh record does not reconstruct
or load that mesh.

Seven complete code spans (1,065 bytes) and eight data spans (187 bytes)
freshly match the installed executable. Every live query verified project
`bsp`, `C:/Users/sqz269/bsp.gpr`, and `/battlestationspacific.exe`. The binary
SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Exact bytes, ABIs, original names/comments, and proposed annotations are in
[resource_mesh_parser_boundary_audit.json](../reports/resource_mesh_parser_boundary_audit.json).
Names such as mesh object and handle describe the observed role; they are
not recovered source symbols or a demonstrated rendering interface.

## Parser result and normal ownership

The verified parser table `00d63098` contains `00b947a0` at virtual `+8`.
Its ABI is an unused incoming ECX parser object, one node-wrapper pointer on
the stack, EAX item result, RET 4.

The entry first calls `00b94710` with ECX pointing to an eight-byte local
pair and the input node wrapper on the stack. It then allocates 10h bytes,
calls the previously audited item-base constructor `00b868b0`, and replaces
the vtable with `00d63738`. That base constructor establishes reference count
one; see [resource_note_parser_audit.json](../reports/resource_note_parser_audit.json).

| Item offset | Value |
| --- | --- |
| `+00h` | Vtable `00d63738` |
| `+04h` | Initial reference count one from base construction |
| `+08h` | Underlying mesh object from local pair `+0` |
| `+0Ch` | Serialized prefix DWORD from local pair `+4`; meaning unknown |

The wrapper increments the underlying object's `+4` reference count, then
releases its temporary reference before returning. With successful ordinary
construction, the underlying object's count therefore goes from one to two
and back to one; the returned item retains it. If wrapper allocation returns
null, the temporary underlying reference is still released and EAX is null.
The source record has already been processed by then.

This establishes creation-time retention, not teardown. Item table `+4`
points to `00b93b40`; underlying object table `00d62d60 +4` points to
`00b74280`. Those destructors and their buffer/material releases are deferred.
No input-node or source-stream retain is visible in the inspected wrappers;
individual field helpers may have further ownership contracts.

## Underlying object construction

`00b94710` takes ECX output pair, a node-wrapper stack argument, and returns
the underlying object pointer in EAX, RET 4. It sets ECX to BCh before calling
`00b73b60`, but that ten-byte entry immediately replaces ECX with `0108fff8`
and tail-jumps to `00b73a10`. The incoming BCh value is not forwarded. The
target is used as an allocation/context entry, and its existing
`CG_static_dtor_stub` name is incorrect; the allocation mechanism, actual
block size, and failure guarantees remain unexamined.

On nonnull return, constructor `00b73d70` initializes an object whose last
written DWORD is at `+B8h`, requiring at least BCh bytes of storage. It takes
ECX storage, returns the same address in EAX, and ends with plain RET. It
temporarily writes base vtable `00ceb130`, establishes reference count one,
then writes final table `00d62d60`.

The remaining exact writes are partial defaults, not proof of a complete
object layout:

| Offsets | Initialization |
| --- | --- |
| `+0Ch` | Float32 `1.0` from `00d7a24c` |
| `+10h/+20h/+30h/+40h` | Float32 `-1e10` |
| `+14h/+24h/+34h/+44h` | Float32 `+1e10` |
| `+08h/+50h/+54h/+58h/+5Ch/+60h/+7Ch/+B0h/+B4h/+B8h` | Zero DWORDs |
| `+84h/+88h/+8Ch/+90h` | Zero float32 words |
| `+80h/+94h` | Zero bytes only |

Other bytes are not initialized by this body. Do not reinterpret the repeated
float pairs as complete boxes/spheres or initialize all omitted members from
these writes alone.

`00b94710` stores the object in pair `+0`, calls `00b944e0` to process the
node, then polls `00b72b40`, which simply returns object `+58h`. The bounded
loop increments a local index and rereads this count; it performs no visible
per-element action. The semantic role of `+58h` is not established by that
getter. Null underlying allocation is not handled gracefully: parsing and
the final getter still run against the resulting pair/object.

## Prefix and ordered field dispatch

`00b944e0` takes ECX output pair and one input node-wrapper stack argument,
RET 4. At `00b94500` it calls `00be99f0`, then writes EAX to pair `+4` at
`00b94507`. Thus both words later copied into the resource item are supplied
by this path.

`00be99f0` takes ECX node wrapper, forwards node `+8` and the address of its
remaining field `+20h` to `00bf02a0`, and returns EAX unchanged by plain RET.
The existing reader audit establishes that adapter's virtual `+38h` DWORD
read and actual-count debit. A complete prefix consumes four little-endian
bytes. Its numeric meaning is unknown, and it is not validated here.

The parser then repeatedly obtains a child through `00bea680` while
`00715bf0` reports nonzero remaining bytes. These literal tags were freshly
byte-verified:

| Tag | Dispatch target | What this packet establishes |
| --- | --- | --- |
| `LODValue` | Scalar `00be99d0`, then setter `00b72710` | Read one float32; x87 float stores/reloads before passing it on the stack with ECX mesh object. Setter effect unexamined. |
| `LODPhases` | `00b93710` | Delegate with output pair, mesh object, child wrapper. |
| `Subset` | `00b941d0` | Same observed argument boundary; record layout/material binding unexamined. |
| `Indices` | `00b93aa0` | Same boundary; index width, count, decoding and buffer ownership unexamined. |
| `VertexStream` | `00b93e60` | Same boundary; vertex layout, stride, counts and allocation unexamined. |
| `CompressedVertexFormatData` | `00b93800` | Same boundary; compression/format schema unexamined. |
| `BoundingSphere` | `00b93590` | Mesh-specific handler; do not assume the hierarchy reader's four-float wire contract without this wrapper. |
| `BoundingBox` | `00b935c0` | Mesh-specific handler; destination/layout changes unexamined. |
| `WeightMapNames` | `00b93f90` | Same argument boundary; names, weighting and ownership unexamined. |

For the eight delegated branches, ECX is the output-pair address, and the
two stack arguments are the mesh object and child-wrapper pointer. Tag
comparisons use CRT `stricmp` directly for the first four tags and the already
audited native-string comparator `00425850` for the remaining five. Recognized
nonnull tag spelling is case-insensitive. Input order is preserved; duplicate
semantics depend on each handler and cannot be inferred as generic overwrite
or append behavior.

Unknown children call explicit skip `00be9c40`. Every normally completed
child then releases its handle through `00be9ed0`; this release does not add
an implicit seek. The parser performs no additional source-stream read before
the prefix and no global endianness, decompression, or buffer conversion in
this outer body. It also exposes no status return or field-level failure
branch.

## Next dependencies toward a drawable mesh

`00b93800`, `00b93e60`, and `00b93aa0` are the direct format/vertex/index
entry points. `00b941d0` supplies the subset dependency; `00b93710` and
`00b72710` supply LOD behavior. Their resource creation, retained data, and
material/shader association must be established before a loaded geometry
object or draw submission can be claimed. `00b93f90` and the two mesh bounds
handlers remain additional serialized-field contracts.

The allocation context `00b73a10`, item destruction `00b93b40`, and mesh
destruction `00b74280` are the ownership frontier. No D3D buffer creation,
vertex declaration, draw call, texture/material binding, or working renderer
is reconstructed by this packet. The present host loader must continue
reporting Mesh as unimplemented wherever it only skips this record.

This packet changes only documentation and the audit. No C++, tests, shared
ledgers, saved Ghidra annotations, or build/runtime validation were performed.
