# Installed mesh resource decoding

Addresses: 00b944e0, 00b93aa0, 00b93e60, 00b93800, 00b941d0,
00b93710, 00b72710, 00b93590, 00b935c0, 00b93f90, 00b2dbd0,
00b8e580, 00b8eb50, 00be9a20, 00bf02f0, 00b73d70.

The Win32 diagnostic now decodes every Resource payload in installed
`models/misc/repulogepdarabok_004.mmod`: Mesh, Note and GroupParams. Mesh field
readers use the real encoded declaration grammar and the retained VFS stream.
These are owning C++ wire values. They do not yet instantiate the native
resource manager, retained materials, GPU mesh bindings or a running game.

`MeshResourcePayload` preserves the prefix DWORD and child order. Streams,
subsets, LOD phases and weight names append; indices and LOD value replace;
compressed-format data attaches to the last preceding stream. Native LOD
defaults to 1.0. Mesh-specific sphere/box wrappers consume and discard floats.
Recognized child release does not seek unread tails, while unknown children
and the rope-stream exception explicitly skip. GroupParams reads one float
and explicitly skips its remaining payload. The installed GroupParams record
has no trailing bytes; nonempty-tail behavior is supported by assembly.

Subset decoding preserves the native primitive remap, four range DWORDs,
effect-name alias and ordered stream/texture/lighting requests. Repeated
lighting records overwrite the same native material values when realized;
the serialized slot is ignored. The host retains those ordered requests for
later actual material resolution. It does not fabricate effects or textures,
or inject a stream-zero event in place of native runtime fallback.

Indices retain exact 16- or 32-bit payload bytes. Vertex streams retain exact
`count * declaration.stride` bytes. Each compressed-format metadata element is
32 bytes; decoding that record is distinct from reading the vertex payload.
The shader-constant consumer and actual material/buffer binding are the next
integration step. No CPU decompression algorithm is inferred from the field name.

## Validation

`./scripts/build.ps1` passes MSVC Win32 compilation and both existing CTests.
The existing reader diagnostic and full D3D9 diagnostic both pass. There are
no new test targets. Logs and exact source/artifact hashes are linked from
`reports/mesh_resource_validation.json`.

The installed 1303-byte model is read through mounted VFS. The probe verifies:

- One stream: 12 vertices, `pssn4nubn4ussn2.mvfm`, stride 16, three elements.
- All 192 vertex bytes, 96 metadata bytes and 48 index bytes against independent
  installed-file offsets. Indices have count 24 and format 65h.
- One subset: primitive 5 maps to 4; range words 0,12,0,8; `textured.mshd`;
  stream 0, `repulodestroyed.tga` in slot 0, then all 17 lighting float words.
- One LOD phase; absent LODValue retains 1.0. GroupParams matches float bits
  `3f7c28f6`. All seven Mesh child tags retain their encounter order.
- Existing Note and hierarchy values, including negative-zero matrix bits,
  remain exact. Final cursor is 1303 and no Resource payload is skipped.

The full diagnostic also verifies existing font rendering and state restoration.
It does not render this mesh. ABI compatibility, native allocation/refcounts,
complete game startup and gameplay remain unverified.

## Saved analysis

Matched audits establish the mesh wrapper, BC-byte mesh object, C0-byte pool
slot, retained stream/index/subset ownership and sized string cleanup. Four
bounded function definitions had stale CRT-free return-flow artifacts; verified
continuations were restored before correcting flow. The identity declaration
resolver at `00b2c280` was recovered as a missing 80-byte function. Prior names
and comments are preserved in annotation records, the existing `bsp` project is
saved, and affected exports are refreshed. Audit, repair and annotation evidence
is linked by the validation report; no original installation file was changed.
