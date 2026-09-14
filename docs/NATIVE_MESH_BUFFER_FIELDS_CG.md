# Native mesh index and vertex-stream fields (CG)

CG reconstructs the complete mesh index/vertex field readers and their two raw
read adapters, plus the stream-count and post-upload leaves. The source uses
the existing actual mesh, declaration cache, logical/physical buffer factories,
mapping, string pools and canonical owner companions. It uploads directly into
real D3D9 buffer storage; it does not introduce a second geometry model.

| Address | Bytes | Source routine | Original ABI |
| --- | ---: | --- | --- |
| `00B93AA0` | 154 | `read_native_mesh_indices_00b93aa0` | ECX unused; stacked mesh/handle; RET8 |
| `00B93E60` | 302 | `read_native_mesh_vertex_stream_00b93e60` | ECX unused; stacked mesh/handle; RET8 |
| `00B72B20` | 4 | `native_mesh_vertex_stream_count_00b72b20` | ECX mesh; EAX current +7C; RET |
| `00B49B30` | 1 | `native_logical_stream_post_upload_00b49b30` | Single RET; no semantic input/result |
| `00BE9A20` | 27 | `read_native_resource_node_raw_00be9a20` | ECX handle; stacked destination/count; RET8 |
| `00BF02F0` | 37 | `read_native_resource_raw_00bf02f0` | ECX reader; stacked destination/count/budget; RET0C |

These six ordinary bodies total 525 bytes. The vertex reader also has the
8-byte unwind action `00CC2E30` and 10-byte handler `00CC2E38`. Disk bytes and
saved Ghidra bytes agree across the complete spans, all 205 executable
instruction starts have the expected function owner, and all 16 direct and 15
indirect transfers are recorded. No alignment bytes were excluded. The missing
handler was restored by disassembly/function creation without clearing bytes or
changing no-return flags. The correct `Unwind@00cc2e30` name is retained.

## Read and upload behavior

`BF02F0` captures the reader's current stream and current slot24 before calling
it. Its actual-count pointer aliases the requested-count stack argument, which
starts with the request. After the callback it subtracts the overwritten count
from the **current** budget using DWORD arithmetic. There is no status test,
zero fill, EOF check, or budget clamp. An exception before the callback returns
does not debit the budget. `BE9A20` captures the current node, its reader at +08
and budget address at +20, then calls that raw reader.

The index reader consumes count and format through the existing complete
`BE99F0`/`BF02A0` path. It calls the captured renderer's current +60 factory with
flags1, maps through the captured logical +0C slot, then computes the byte
product. Formats **0x65** and **0x66** use widths2 and4; all other format widths
are zero in the original branch. It performs one raw read, reloads the current
logical +10 unmap and +2C post-upload slots, publishes the stream through the
actual mesh index setter, and finally releases the captured creator reference.
`B49B30` is a verified one-byte RET, so its empty source body is the actual leaf.
The index reader has no resource cleanup on exception.

The vertex reader reads count and a local counted format name. It ignores the
returned header pointer from `BEA010` and passes the local header to the captured
renderer +38 declaration loader. That declaration has one acquired reference.
It independently reloads the renderer for the +5C stream factory, then releases
the declaration temporary **before** reading the same captured declaration's
stride at +CC. The stream owns the declaration reference at this point on the
normal native path. The byte product is computed before mapping through +10.

After one raw read, a case-insensitive `rope.mvfm` comparison invokes the
existing `BE9C40` trailing-data skip/detach operation when equal. The reader then
reloads current +14 to unmap, reads the current mesh stream count at +7C, appends
through `B73BB0`, drops the creator reference, and returns the local format name.
The EH map at `DFC5F4`/`DFC5FC` owns only a completed local name: state0 goes to
-1 through `CC2E30` and the existing `41DD20` destructor. No declaration, stream,
mapping, prior mesh mutation, or factory publication is rolled back by this EH.
Normal name return captures its pointer before disabling cleanup, then reads
the current length and current pool publication at the native release site.

`NativeMeshBufferReadAcquired` records progress and outstanding acquired
references/maps without adding references or changing native cleanup. Its
borrowed audit pointers can become stale after a native final release. A call
starts with an empty record and cannot be replayed. The caller must retain the
same canonical owner domain, current renderer/synchronization publications,
physical contexts and actual string-pool domain used by the mesh and cache.
The index profile view must include slot2C. Unrecognized current virtual
profiles fail explicitly at the source boundary.

## Validation

`scripts/build.ps1` compiled CG into the actual Win32 `bsp_core` and passed both
existing CTests. Because `cc7` owns `cmake/startup.cmake`, the build uses the
ignored `local/native_mesh_buffer_fields_cg/extra_sources.cmake` hook to register
the pending CC, CE, CF and CG sources. The tracked registry still omits all four.
This is a worktree build, not a clean-checkout or main-integration claim. Remove
the cached `CMAKE_PROJECT_INCLUDE_BEFORE` override when registering those files.

One controlled-child fixture creates an actual D3D9 HAL device on the RTX5090
and invokes the source readers with real retained-memory streams, native reader
nodes, native mesh/pools, logical/physical factories, actual COM buffers and
canonical reference companions. The original installed image is used only for
verified immutable data mapping; it is not modified or launched.

The fixture passed:

- Index16 and index32 uploads, exact six/twelve-byte GPU readback, trailing
  budget4, mesh ownership count1, and retirement of the replaced index stream.
- Two 24-byte vertex uploads with exact GPU readback. `ordinary.mvfm` leaves
  budget4/depth1; mixed-case `RoPe.MvFm` skips the tail to budget0/depth0. Each
  published stream has count1 and its cached declaration has count2.
- An injected raw-read exception after vertex mapping leaves the creator and
  map outstanding, mesh count2 unchanged, and budget12 undecremented. Explicit
  fixture cleanup occurs **after** these assertions; it is not parser unwind.
- All eight canonical companions retire. The cache's total stride reaches0,
  renderer stream counts reach0, all three used raw slabs return to 32 free
  slots, and device/API final COM releases reach0.

The two declarations are preseeded through actual declaration/record/alias
construction, then loaded via the real cache-hit path. Cold format decoding,
VFS date/loading, and native pool startup are outside this fixture. Raw logical
and declaration pool headers are caller-initialized with real critical sections.
The mesh/section pools use their existing native lifecycle implementation. The
load-time platform callbacks are observed fixture boundaries; game XLive and
focus policy are not validated. The device-recreation provider fails explicitly
if reached; there is no successful replacement implementation.

Two fixture assumptions were corrected before the passing run: D3DFORMAT
identifiers are hexadecimal 0x65/0x66, and slab free counts are 16-bit values.
No CG or dependency source change was needed for either correction. Fixture
correction notes and final build/probe/source hashes are retained with the proof.

## Evidence limits and next work

The new interfaces are source reconstructions, not ABI-compatible replacements.
The fixture executes source parents; it is **not** a copied-original parent
differential oracle. Full native EH/SEH, stack aliases, callback mutation
permutations, short reads, extreme/overflow counts, allocation faults, device
loss/recreation, cold declaration loads, threading, and gameplay remain open.
No full renderer construction or mesh aggregate admission is claimed.

The remaining mesh aggregate dependencies include compressed vertex-format
data at `B93800`, subset/material parsing at `B941D0`, aggregate reader `B944E0`
and the registered parser/construction wrappers beginning at `B94710`.
Confirmed plane-control issues in the separate CD review and unreviewed newer
main commits remain separate integration concerns.

See `reports/native_mesh_buffer_fields_cg.json`, the flow/annotation reports and
`reports/native_mesh_buffer_fields_integration_cg.json` for the address ledger,
saved old comments/prototypes, byte hashes, build configuration and frozen local
artifact manifest. Descriptive names remain reconstruction hypotheses.
