# Native camera foundations integration

This batch integrates the native viewport owner, shadow texture access, camera
pool and borrowed camera state/frame interfaces. The installed-asset probe
passes after integration. Full native camera and shadow construction remain
separate work; the probe continues using an explicitly diagnostic camera.

## Established components

| Component | Native scope and integration |
| --- | --- |
| Viewport owner | `00B1F850/00B1F8F0`, origin/size and depth setters; real `34h` owner/refcount, allocation preimages and current renderer reads. `00B1FF60` is named as an evidence-only renderer parameter getter. |
| Shadow texture access | Eight bounded getters/metadata/toggle routines; same raw global/fallback slots, concrete target/texture profiles and mutable unsigned reported dimensions. No texture allocation or COM resize is added. |
| Camera pool | Eleven functions over canonical `0108FFB0`; actual allocator-list membership and Win32 section, `45Ch` slots with slab IDs at `458h`, and `8BC4h` slabs. Raw pool operations do not construct/destroy cameras. |
| Camera state | Projection and state now borrow native fields. The getter reads separate `1D8/1D4/1C8/1C4` addresses in native x87 order; `1CC/1D0` remain unrelated. |
| Camera frame | Scalars, matrices, planes and axes borrow actual backing. Viewport/fog slots retain raw owner identity and preserve consumer capture order. See `CAMERA_FRAME_OWNER_BACKING.md`. |

The root integrator reviewed the implementations, checked integrated source
hashes against worker artifacts, rechecked the recorded native byte ranges
against the original installed executable, and reran the focused fixtures.
The original game executable remains unchanged. Build source registration is
now part of `bsp_core`; no new permanent test suite was added.

## Verification

The viewport sequence matches thirteen complete owner states, including
renderer replacement between constructor dimension reads, overlapping setter
sources, preserved padding, reference-count transitions and final free. The
shadow texture sequence matches live global/fallback selection, unsigned
metadata, alias-driven reload after the first dimensions write and flag gates.

The camera projection sequence matches twelve complete `2F4h` prefixes across
four x87 rounding modes: 9,072 bytes, plus status/control words and returned
cache identity. Unrelated signaling-NaN gap words are not loaded. The camera
pool sequence matches 136 states, eight actual allocation/free observations and
8,336,740 raw slab bytes, including every moved slab ID, return/reuse and actual
critical-section recursion cleanup. Native CRT startup and EH are not executed
by that pool fixture; its allocator observations call real malloc/free.

The new host frame check covers actual field/slot identity and the raw-owner
fog conversion/capture hazard. The final strict Win32 build passes both CTests.
The installed probe passes camera preparation, light/fog/particle cleanup and
all 77 VS/PS system vectors after two material uploads. The mesh draw reports
2,499 visible pixels, 54 colors and restored device state. The render remains
the same dark diagnostic image; this is not a gameplay or visual-parity claim.

## Corrected evidence and remaining work

The pool fixture exposed a truncated camera return-wrapper preimage.
`00B71350..00B7135C` is twelve bytes: incoming ECX slot is pushed, ECX is changed
to the canonical pool, `00B711E0` consumes the stack argument, and plain RET
returns. The corrected discovery report preserves the old ten-byte preimage
and identifies why it was insufficient. `00B71930` is now identified as the
camera-pool allocation entry, replacing its earlier provisional scene-node
allocation label.

The native camera still needs look-at construction, plane-set initialization,
type initialization, actual viewport publication/retention, complete staged
constructor EH and destructor composition. The renderer's real parameter
storage at `1A20/1A24` is a required boundary distinct from present parameters.
The shadow chain additionally needs real surface ownership and its pool,
render-target groups and full shadow construction. Group discovery confirms
that surface-wrapper refs and cached COM refs are independent: setters do not
refresh cached COM fields, and destruction reloads them after wrapper release.

The sharded ledgers and saved Ghidra annotations identify each bounded native
entry with original ABI, prior names/comments and uncertainty. Refreshed
exports and exact validation/source hashes are recorded in
`reports/native_viewport_shadow_integration_audit.json`. The component reports
retain the distinction between exported, reconstructed, build-tested,
fixture-tested, ABI-compatible and game-validated.
