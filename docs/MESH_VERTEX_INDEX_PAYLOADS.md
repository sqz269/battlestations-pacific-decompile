# Mesh vertex and index payloads

`src/mesh_buffers.cpp` reconstructs the complete-transfer payload portions of
`00b93aa0`, `00b93e60`, and `00b93800`. It preserves the serialized bytes in host
vectors. It requires a declaration resolver for each native format name; it
does not infer stride from node size. Format resolution is integrated separately.
The evidence record is `reports/mesh_vertex_index_payloads_audit.json`.

The three native handlers ignore their incoming ECX, take mesh object and node
wrapper as two stack arguments, and return with `RET 8`. They expose no stable
result or error flag. Their host interfaces are distinct C++ APIs. Mesh dispatch,
subsets, bounds and ownership are documented in `RESOURCE_MESH_PARSER_BOUNDARY.md`,
`MESH_SUBSET_LOD_FIELDS.md` and `MESH_OBJECT_LIFETIME.md` respectively.

## Serialized fields

| Child | Payload consumed |
|---|---|
| `Indices` | DWORD index count; DWORD format; count times format width raw bytes. Format `65h` has width 2, `66h` width 4. |
| `VertexStream` | DWORD vertex count; counted string format name; count times resolved declaration stride raw bytes. |
| `CompressedVertexFormatData` | Last vertex stream's declaration element count times `20h` raw bytes. There is no count or other prefix in this child. |

All integers and index words retain the existing little-endian reader contract.
The compressed-format bytes are opaque records in this implementation. The native
handler neither decompresses them nor establishes their numeric components.

`Indices` reads its two DWORDs at `00b93aaa` and `00b93ab3`, then calls renderer
virtual `+60h(count,1,format)`. The exact width selection is at `00b93acb..ae5`;
unknown formats yield zero width natively. Virtual `+0ch(count,0,0)` returns the
destination, and `00b93af5..afc` multiplies width by count and transfers bytes.
Virtual `+10h` and `+2ch` then run before `00b73b70` retains the index stream at
mesh `+60h`. The handler releases its temporary intrusive reference afterward.
Graphics-object failure behavior and those virtual callbacks are not implemented
by the host payload decoder.

`VertexStream` resolves its counted format name through renderer `+38h`, calls
renderer `+5ch(count,1,declaration)`, then releases the temporary declaration
reference. Assembly distinguishes the declaration pointer in EDI from the local
string handle; the decompiler incorrectly merges both into one apparent string.
The stride load at `00b93edc` is declaration `+cch`, and multiplication at
`00b93ee5` supplies the raw transfer at `00b93ef5`. A case-insensitive C-string
comparison with `rope.mvfm` at `00b93f08` explicitly skips and detaches the tail.
Other format names have no implicit tail skip. Virtual `+14h` follows, and
`00b72b20` supplies the previous stream count as the append index passed to
`00b73bb0`. The mesh retains that stream and the handler releases its reference.

`CompressedVertexFormatData` gets stream count from mesh `+7ch`, subtracts one,
and indexes mesh `+64h + 4*index` using `00b73260`. The native code has no empty
stream check. Its virtual `+24h` returns the declaration, and `00b47900` reads
the flat declaration element count at `+10h`. Allocation uses unsigned count
times `20h`, saturating the allocation request to `FFFFFFFFh` on overflow, while
the raw-read length is the wrapped left shift by five. `00b61d90` simply stores
the allocation at stream `+50h`; it neither releases a previous pointer nor
decodes the records. Host replacement releases the prior vector through normal
C++ ownership and rejects multiplication overflow.

## Concrete stream and raw-read dependencies

Renderer factory `00b287c0`, reached by virtual `+5ch`, passes count, declaration,
and flags into `00b4bc00`. The constructor installs table `00d61d6c` at
`00b4bc2c`, retains its declaration at stream `+68h` through `00b4be91..bec4`, and
the table DWORD at `00d61d90` (`+24h`) is `00b48ce0`. That existing named getter
is exactly `MOV EAX,[ECX+68h]; RET`. This establishes the compressed field's
declaration dependency independently of its node size. The constructor's full
graphics allocation, registry, and device-retry behavior remains outside this
payload implementation.

`00be9a20` has ECX node-wrapper pointer, destination and count stack arguments,
and `RET 8`. It unwraps the node, supplies node `+8h` as reader ECX and node
`+20h` as remaining-budget pointer to `00bf02f0`. The latter has three stack
arguments `(destination, requested_count, remaining_pointer)` and `RET Ch`.
At `00bf02f7` it takes the address of its requested-count argument slot and
passes that nonnull pointer to stream virtual `+24h`. After the read it subtracts
the overwritten actual count from the remaining budget at `00bf0310`, ignoring
stream status. The argument initially contains the requested count if the stream
fails to write the actual-count output. There is no zero-fill or native extent
guard. The pseudocode's `unaff_retaddr` and two-argument signature are incorrect;
assembly establishes the three-argument call and accounting.

The host `StructuredNode::read_bytes` enforces the same full-transfer domain as
the existing typed reader. The payload API rejects unknown index formats,
overflow, insufficient remaining bytes and allocation failures. Failure does
not roll the input cursor back; output vectors commit only on success. Normal
success leaves node closing to the caller, except the explicit rope skip.

## Installed evidence and validation boundary

The installed `models/misc/repulogepdarabok_004.mmod` is 1303 bytes, SHA-256
`5fab0cfe63220f84693cae4140fc98689670316585993dae71bd4f503bc7a7e6`.
Its Mesh prefix is zero and its buffer fields appear in the order below.

| Field | File data offset | Count / extent | SHA-256 of raw data |
|---|---:|---|---|
| Vertex stream `pssn4nubn4ussn2.mvfm` | 177 | 12 vertices, 192 bytes | `9d528ec79acbd2f1725fb3b04da65d4c6eecc241b8e94157f47a1f62ec953cdf` |
| Compressed format data | 403 | 96 bytes | `70ecefba68560b19a7db57d45615005e13f5aca00c3b59838b4edea4bc673751` |
| Indices, format `65h` | 522 | 24 indices, 48 bytes | `3df317aa2c08859b1cb6c5a8501d54d86a2f5b5ae47972a2a84aeb338de3b109` |

The separately recovered format tokens establish three elements with sizes
8, 4, 4, giving stride 16; therefore 12*16 and 3*32 exactly explain the first
two extents. Byte inspection independently finds these 24 index values:
`0,1,2,0,2,3,4,5,6,4,6,7,8,9,1,8,1,0,7,6,10,7,10,11`.
This fixture does not exercise 32-bit indices, rope tails, repeated compressed
fields, short reads, or invalid formats.

Thirteen captured ranges, comprising twelve full function spans and the getter
table DWORD, match the current installed PE. The constructor is byte-matched and
its declaration-retention fragment is inspected; its whole graphics behavior is
not thereby reconstructed. All 38 call sites in the three handlers and two raw
adapters had flow override `NONE`; no saved Ghidra edits were made by this worker.
Build and integrated C++ installed-resource execution are owned by the primary
batch and recorded separately. No new standalone tests were added. These payload
decoders do not establish native ABI compatibility, D3D mesh rendering or game
validation.
