# Dyn hull production for avoidance geometry

Addresses: `00C5DF30`, `00C5DEB0`, `00C5DDD0`, `00C5DAE0`, `00C35E80`,
`00C5D900`, `00C5CAE0`, `00C32510`, `00C389C0`, `00C35BF0`, `00C35DE0`,
`004039D0`, `00C40F50`, `00C37450`, `00C32D20`, and the algorithm helpers
listed in `reports/avoid_zone_dyn_hull.json`.

The reconstruction now produces the actual Dyn convex-hull vertex and adjacency
data consumed by the avoidance physics shape. The eight-byte handle owns a
68h internal object; copying the handle creates independent internal storage.
The original support/simplex/extrusion algorithm is translated from this game's
code. No alternative hull or physics library is used.

`00423C50` remains the caller/integration boundary. Its ordered polygon pieces
must first pass through `avoid_zone_draft_extrude_00423d81`. Pass those actual
local XYZ points to `avoid_zone_dyn_hull_construct_00c5df30`, reject fewer than
four hull vertices using `avoid_zone_dyn_hull_vertex_count_00c32d20`, and use
`avoid_zone_dyn_hull_copy_00c40f50` for the manager-owned retained handle.
The shape descriptor must point to that retained handle. This module does not
create Dyn physics bodies or claim collision/gameplay validation.

## Coverage and native ABI

Every descriptive name is provisional; the existing Dyn library attribution is
preserved. C++ interfaces are new and do not preserve the native register ABI.
The public handle, data and vertex structures do preserve their Win32 layouts.

| Address | Native ABI / role | Coverage |
|---|---|---|
| `00C5DF30` | ECX handle; stack count, points; RET8 | Complete constructor sequence in the finite input domain |
| `00C5DEB0` | ECX handle; stack count, points; RET8 | Complete replacement order; allocator failure/SEH ABI unresolved |
| `00C5DDD0` | stack data, count, points; RET0Ch | Complete fixed-descriptor pipeline; ignores generator status as native does |
| `00C5DAE0` | EAX result, stack descriptor; RET4 | Partial: constructor's flags 1, stride 12, epsilon 0.001, vertex limit 4096; reverse winding `00C5DC94..00C5DCC5` and polygon output `00C5DCE8..00C5DD8D` not exposed |
| `00C35E80` | EDX destination; six stack arguments; RET18h | Partial input projection: packed XYZ and nonnull scale; both degeneracy branches recovered; optional null-scale path `00C3606C..00C36070` not exposed |
| `00C5D900` | ECX count, EDX points; stack indices-out, face-count-out, limit; RET0Ch | Complete ordered live-triangle extraction; native work-pool identity unresolved |
| `00C5CAE0` | stack points, count, limit; RET0Ch | Complete finite-domain hull algorithm, including repair extrusions and vertex limit |
| `00C32510` | EAX input count, ESI output-count pointer; four stack arguments; RET10h | Complete first-encounter compaction output |
| `00C389C0` | ECX data; stack vertex-count, XYZ, triangle-count, indices; RET10h | Complete output data; unused temporary edge hash and its pool allocation identity not reproduced |
| `00C35BF0` | EDI direction, stack data; RET4 | Complete finite-domain first-strict-maximum support search |
| `00C35DE0` | ECX adjacency-vector array, EAX vertex, BX neighbor; RET | Complete ordered unique-neighbor effect; C++ work-vector allocation |
| `004039D0` | EAX source, ESI destination; RET | Complete deep-copy stores; native allocator failure ABI unresolved |
| `00C40F50` | ECX destination, stack source handle; RET4 | Complete independent copy and second-word preservation |
| `00C37450` | ECX handle; RET | Complete adjacency, vertices, internal-object free order; handle words intentionally unchanged |
| `00C32D20` | ECX handle; EAX result; RET | Complete first pointer's +4 vertex-count read |

The report also lists the complete finite-domain support, simplex, triangle
normal/normalization, neighbor-link, back-to-back splice and extrusion helpers
at `0040C790`, `0040EF40`, `004060E0`, `00C32860`, `00C36730`, `00C38F90`,
`00C48170`, `00C482B0`, `00C51480`, `00C51740`, `00C517D0`, `00C58920`.
Global triangle slots at `0109EA00/+4/+8` are represented by per-call C++
ownership; this does not establish original concurrency or native pool ABI.

## Producer evidence

`00C5DF30` zeros both handle words, then `00C5DEB0` frees an old internal
object, allocates 68h, zeros only its first six words, and calls `00C5DDD0`.
At `00C5DE2C..00C5DE60` the descriptor has flags 1, packed 12-byte points,
epsilon bits 3A83126F (0.001), skin bits 3C23D70A (0.01), and both limits 4096.
Skin and the second limit are not consumed by the reached producer path.
The generator's result lives in EAX, which Ghidra's pseudocode misses:
`LEA EAX,[ESP+14h]` at `00C5DE4A` establishes its output. Consequently the
pseudocode's apparent `00C389C0(0,0,0,0)` is incorrect; loads at
`00C5DE65..00C5DE79` supply the generated vertex/triangle buffers.

The cleanup pass computes an AABB, normalizes coordinates by each extent,
deduplicates within 0.001 on all three axes, and keeps the duplicate farther
from the normalized center. Both native degeneracy branches synthesize an
eight-point box with the observed extent rules. They are recovered native
behavior, not a replacement for failed hull generation. The generator then
restores the scale before running its hull algorithm.

`0040C790` performs the observed stable support search, including the 45-degree
angular sweep, five-degree refinement and persistent allow states 0/1/3.
`00C58920` selects the oriented initial tetrahedron. `00C5CAE0` constructs
its four faces in the observed order, chooses the greatest rise, extrudes
visible faces backward through the triangle array, repairs inverted/too-small
faces, updates support vertices, and respects the vertex budget. `00C5D900`
exports surviving triangles in array order. `00C32510` compacts vertices by
first index encounter, preserving that output ordering.

The native internal record is:

| Offset | Proven producer meaning |
|---|---|
| `00h/04h/08h` | vertex pointer, count, capacity; vertices are 16 bytes each |
| vertex `00h..0Bh` | binary32 XYZ |
| vertex `0Ch` | uint16 offset into adjacency buffer; vertex 0Eh is untouched |
| `0Ch/10h/14h` | uint16 adjacency pointer, count, capacity |
| adjacency payload | for each vertex: neighbor count then unique neighbor IDs, ordered by triangle encounters |
| `18h/24h` | minimum/maximum XYZ |
| `30h..65h` |27 uint16 support seeds over directions{-1,0,1}³; center slot 13 untouched |
| `66h..67h` | untouched padding |

`00C38C68..00C38CB1` inserts the six directed neighbor pairs in order
AB,BA,AC,CA,BC,CB. The buffer size is vertex-count plus three times triangle
count, even when the actual neighbor prefix is shorter. New buffer elements
are zeroed and only that prefix is overwritten. `00C38EE1..00C38EF4` fills
the 26 support seeds and explicitly skips the zero direction. `004039D0`
copies all 16 bytes per vertex and all 27 support slots, including their
unspecified bytes, without assigning an invented value to them.

## Verification

Win32 Release build and both existing CTests passed. One ignored differential
probe links the actual Release `bsp_core.lib`; no test framework or CTest was
added. The 27 reconstructed function byte spans matched live Ghidra and the
installed EXE before relocation. The fixture relocates decoded absolute
operands in the native helper/Dyn spans while leaving relative control flow
unchanged. It executes both real native pool constructors and the original
complete hull pipeline. Host allocator/free, memcpy/memset, finite sqrt and
Win32 critical-section bindings are explicit fixture boundaries.

All 719 cases passed:13 finite geometric cases and all 706 actual Marshall
partition pieces supplied by the polygon reconstruction worker. The latter
come from 21 installed USN01 paths and 2193 native-constructed zone corners.
The scene SHA256 is
`9235d9a6364b08cbd5cf3c6735ba9828aba5a6e7b2569300dc2eecf6d8dac17b`;
the extrusion-input JSON SHA256 is
`5dc602ba766b1c5d866fe4f2367fd0e0e488910964f712553d81c9fd97be212e`.

Triangle order, index compaction, binary32 points, 16-byte vertex records,
full adjacency buffers, AABBs and support seeds matched. Both native and
reconstructed retain copies were verified after destroying their source
handles. The serialized retained data matched SHA256
`069001ff313745494c7d8c6323a3cdda09278e6b3caf68b40bfc4120af50644f`.
The fixture initializes allocations to A5 to verify that native-unspecified
vertex 0Eh, seed 13 and padding remain untouched; A5 is not a recovered value
or a runtime initializer. Five allocations remain owned by the two native
pools and the global triangle-array capacity, with no retained hull handles.

The required math domain is finite coordinates and finite representable
intermediates. Exceptional CRT behavior, malformed triangle graphs, native
allocation-failure/unwind behavior, alternate producer flags, pool ABI,
physics body creation and gameplay are not proven by these fixtures.
