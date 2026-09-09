# Retained font geometry owner

`FontGeometryOwner` replaces the installed-font probe's manual layout, quad,
buffer and logical shader-wrapper preparation. It consumes the existing
single-line or wrapped scalar component and the existing quad writer, uploads
their actual output, and publishes main/shadow ranges sharing vertex, index
and layout identities. This is a new MSVC Win32 interface, not the original
text-context ABI or a complete native drawable.

Implementation: [font_geometry_owner.hpp](../include/bsp/font_geometry_owner.hpp)
and [font_geometry_owner.cpp](../src/font_geometry_owner.cpp). The existing
[d3d9_font_probe.cpp](../src/d3d9_font_probe.cpp) is the integration consumer.
The reviewed native contract is [FONT_CONTEXT_OWNERSHIP.md](FONT_CONTEXT_OWNERSHIP.md),
including its final device/cache and stable-address API corrections.

## Evidence and original interfaces

This packet reuses the complete assembly and original-PE comparisons in
[font_context_ownership_audit.json](../reports/font_context_ownership_audit.json).
It did not perform another live Ghidra batch or mutate saved analysis.

| Native routine | Original ABI and observed contribution |
|---|---|
| `00aba8d0..00abaec6` | ECX context, one UTF-16 wrapper stack argument, `RET4`. Case-insensitive equality skips the changed-text body. Changed empty resets width and both section counts; nonempty requests buffers and selects scalar layout with byte `+FC`. The apparent second flag is a saved shader-selection result. |
| `00ab8400..00ab852d` | Glyph capacity and geometry on stack, `RET8`; ECX is unused. Requests `simplecolor.mvfm`, `4*N` vertices, `6*N` 16-bit indices, then retains them through geometry setters. No capacity-reuse branch. |
| `00abb1d0` | ECX context, no stack arguments, `RET`. Copies stored text, empties the cached string and updates from the copy. Existing nonempty text rebuilds; empty remains equal. |
| `00ab9fd0` / `00aba270` | ECX context, string/section stack arguments, `RET8`. Existing scalar fragments preserve their arithmetic and accepted domains. Both lock vertex and index capacities and unlock index before vertex. |
| `00ab98f0` | ECX context, ten stack arguments, `RET28h`. Existing ordinary-quad fragment writes four vertices and six indices; later optional child UI remains outside this owner. |
| `00b73bb0` / `00b73b70` | ECX geometry, stream-index/stream (`RET8`) or index object (`RET4`). Retain new object and release old on pointer change. |
| `00b865a0` / `00b86550` | Section layout preparation and release of retained stream references. The owner uses the existing logical declaration/layout API in the successful coherent-cache domain. |

Decisive original spans remain `00aba8d0`, 1,527 bytes, SHA-256
`81b87216443b038af05450c9d23a2348da40c546753edbba95265cdb6cea8ea8`, and
`00ab8400`, 302 bytes, SHA-256
`2a8aac6e63e7d1cb2ca66dee1d00e3edad7291d398e0d957b0e5bfb56e799c9c`.
The audited PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The prior report records target verification and exact additional helper spans.
Cached wrapped assembly at `00aba6ac..00aba6b7` also confirms the first emitted
x origin is stored at context `+18C`.

## Fixed inputs and update behavior

Construction takes an explicit borrowed `IDirect3DDevice9&` and
`D3D9StateCache&` for that same device. Both must outlive the owner and its
move destination. `FontGeometryOwnerResources` retains a shared, decoded
`FontResources`, acquires one COM reference to each supplied shader, and
copies the sampler pass and pixel usage mask. Font, images and shader domain
remain fixed; callers must not externally reset or mutate these resources.
Null resources, missing fallback glyph `0091`, texture references other than
0/1, or more than 16 pixel/four vertex sampler slots throw
`std::invalid_argument` at construction. No device binding occurs there.

The main material projects retained font GFX and alpha images to slots 0/1.
Shadow receives GFX at slot 0 only, matching the observed copy of the main
material's slot 0. Its absent slot 1 resolves to null if a supplied pass uses
that slot. The two materials share the prepared shaders. Full shader selection,
parameter registration and native context addresses are not introduced.

`update_00aba8d0_fragment` accepts already-transformed UTF-16 code units.
The span must have no embedded NUL and at most 16,384 code units. It compares
the cached text with `_wcsicmp`, without an additional length-equality gate.
That uses the host CRT's active case-mapping locale; no equivalence with an
arbitrary original-process locale is claimed. The installed ASCII fixture
does not depend on non-ASCII case mappings.

| Result | Published behavior |
|---|---|
| `unchanged` | Case-insensitive equality retains the original spelling, bytes, metrics and bindings. Scalar parameters are not validated. Equal empty input also takes this result. |
| `cleared` | Changed empty input stores empty text, sets measured width and both ranges' vertex/primitive counts to zero, and retains the last buffers, declaration/layout, CPU bytes, placements, lines and other metrics. Scalar parameters are not validated. |
| `rebuilt` | Changed nonempty input validates the chosen scalar path and all quads, creates fresh logical/physical buffers and layout, uploads real bytes, unbinds old owned bindings and publishes the new generation. |
| `unsupported` | Input/scalar/quad guards or reported device failures preserve the previous text, geometry and metrics. This is host failure handling, not an emulation of native unchecked allocation failures. |

`multiline == 0` selects `single_line`; every nonzero byte selects `wrapped`.
Single-line vertical scale is separately supplied. `origin_x/origin_y` are
explicit host raw-coordinate placement, added with an x87 float spill before
quad normalization. They do not model native scene transforms. Wrapped y
alignment is applied by the existing `00aba860` fragment **after** the quad
writer's coordinate normalization. The existing scalar, quad and offset
fragments retain the caller's x87 control word and their audited spill order;
the owner does not replace their arithmetic with a conventional wrapping pass.

`rebuild_00abb1d0_fragment` explicitly bypasses equality for stored nonempty
text. Empty cached text still returns `unchanged`. It does not expose a
fictional force flag in the native `00aba8d0` argument list.

## Geometry and metrics

Each generation has a fixed 24-byte vertex layout: float3 position at byte 0,
float2 UV at byte 12, and packed white D3DCOLOR at byte 20. The allocation
holds `4*N` vertices and `6*N` indices for stored text length `N`, with fresh
dynamic buffer wrappers using the existing checked `0x1000` lock/recreation
path and logical vertex tag `0x40000001`. This projects the buffer request
through the existing host renderer API; it does not reconstruct the complete
native factory or allocator cache.

Scalar placements retain code units, not surrogate-decoded code points. The
selected ordinary glyph is passed to the existing quad writer once per emitted
placement. Wrapped emission may be shorter than stored text; the CPU snapshot
keeps full allocated capacity, with zero-initialized unused tail bytes. Native
unused buffer contents are not specified. Drawing uses only emitted counts.
All CPU vertex coordinates/UVs must be finite before device allocation begins.

`main` and `shadow` share exactly the same logical vertex, index and layout
objects. Both publish triangle-list type, base/minimum vertex zero, start
index zero, `4*emitted` vertices and `2*emitted` triangles. A shadow scene
translation or extra draw is the consumer's responsibility. The host shares
one coherent layout object; it does not model failed native layout creation.

Metrics map width to context `+114`, line count to `+110`, wrapped height to
`+178` and initial x to `+18C`. Single-line updates set line count to zero and
preserve the previous wrapped-height field. Empty clearing changes only width.
Container width, low-16 font height and normalized height/offset are explicit
host diagnostic values: normalized wrapped height remains stale after a
single-line update, while its active vertical offset becomes zero. Empty
clearing retains all these diagnostic values as well.

## Lifetime, binding and failures

The move-only public object owns a heap implementation. Logical shader wrappers,
font/material owners and the current generation keep stable addresses across
moves. Generations register each logical stream in its physical wrapper and
unregister it before release. The CPU snapshot is borrowed until the next
update/rebuild or destruction; consumers must not mutate or retain its logical
objects for independent cache binding or device reset.

`bind(false)` selects main and `bind(true)` selects shadow. It binds the prepared
shader wrappers, active sampler texture slots, stream 0, indices/base vertex and
layout. It leaves constants, render states, sampler states, target/camera and
submission to the consumer. From the first bind, including a partial failure,
until unbind, this owner has **exclusive ownership of those cache slots**.
Other users must not replace them, invalidate the cache or change matching
device state during that scope.

Equal-COM shader binding in the existing cache preserves its previous logical
wrapper pointer. The owner therefore clears a different wrapper before binding
its own stable wrapper, even when they describe the same COM shader. `unbind`
clears shaders, active texture slots, stream 0, indices and layout; it does not
restore preceding state. Because the existing null-layout cache setter does
not call the device, cleanup also explicitly sets a null vertex declaration.
The installed probe restores its separately captured D3D9 state afterward.

Before replacement/destruction, owned bindings are cleared, the generation's
registries are unregistered, and buffers/layout are released. Shader references
and material projections are released while the font/image owner still lives.
Move assignment first performs that cleanup for the old destination. Moved-from
updates return `unsupported`, bind returns `D3DERR_INVALIDCALL`, and its getters
are empty.

Input and scalar failure does not mutate resources. A candidate allocation or
upload failure discards the candidate and keeps the previous generation.
If cleanup of an already-bound old generation reports failure, publication is
cancelled but its cache bindings have already been cleared; the caller must
rebind before drawing again. Cache methods clear their logical references even
when a device call fails. Destructor cleanup is best effort and nonthrowing.
Explicit unbind returns the first exposed failure. Existing stream/index bind
and unlock APIs discard their COM HRESULTs, so this owner cannot promise to
detect those API failures. Allocation exceptions propagate; the integrated
probe catches standard exceptions before restoring captured device state.

No scene callbacks, optional children, localization, font/shader reload,
device-reset lifetime, intrusive object ABI or original-game rendering are
claimed. The constructor's fixed supplied-resource boundary replaces the
native pre-comparison drawable/shader setup only within this supported domain.

## Integration validation

The existing installed `A` and wrapped `A space A LF A` draws now use this
owner's uploaded CPU snapshot for scalar checks and pixel bounds. Within each
same scenario, the probe checks bound moves, case-only equality with invalid
replacement scalar input, explicit fresh-buffer rebuild, empty clearing with
retained buffers/stale metrics, equal-empty rebuild, changed-nonempty fresh
buffers, shared main/shadow identities and explicit cleanup. It binds both
main and shadow but retains one existing draw per scenario. No test target or
additional standalone fixture was added.

The primary integrator ran the coordinated MSVC Win32 `/W4 /WX` build, both
existing CTests and the full D3D9 probe successfully. All owner lifecycle checks
reported 1 in both scenarios. The installed single `A` retained 74 lit pixels;
wrapped text retained 900, with zero outside expected bounds and successful
state restoration in both draws. Static source review and `git diff --check`
were completed locally. These are build, host lifecycle fixture and installed
asset draw results; they do not establish native differential or original-game
validation.

Independent review confirms the documented ownership domain. The probe checks
move construction, not move assignment or device-failure cleanup. Pixel bounds
come from the uploaded geometry: they check draw consistency, not independent
native geometry parity.
