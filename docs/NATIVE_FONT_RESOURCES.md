# Native font resource identities and lifetime

Parent integration follow-up: this source and the existing scalar/semantic font
users pass the normal Win32 build and both existing tests. The native destructor
has now been restored through `00AD5586`, with zero listing gaps. Its full prior
documentation was archived and all existing labels, local variable metadata,
signature and comments were verified preserved before the new evidence comment.
See `docs/ORCH5_GLYPH_FONT_BATCH.md`. The raw-tail limitation noted below records
the worker's earlier inspection state. Native font/resource execution remains
unvalidated; the C++ destructor's stated partial ABI boundary is unchanged.

Addresses: `00AD4C30`, `00AD53A0`.

`NativeFontResources` owns the existing decoded `FontData` allocation and the
two actual image references returned by the native renderer path. Adoption
moves its `unique_ptr`, preserving the `FontData`, map-node and glyph addresses.
It does not copy metrics or introduce a second glyph map. The narrow
`FontGlyphData` extension adds raw resource slots at `+18/+1C`; Win32 assertions
verify these offsets and the resulting `20h` payload size. Padding `+16` is not
given an invented semantic value.

The scalar decoder still produces null resource slots. The semantic
`FontResources` loader still owns `shared_ptr<D3D9RetainedTexture2D>` images and
leaves these new raw slots null. Its wrapper pointers and COM interfaces cannot
be passed as native logical-texture identities. The new owner is the explicit
actual-resource path; it takes an already decoded sole `FontData` allocation
and already owned raw texture results. It does not upgrade or cast the semantic
texture wrappers.

| Routine | ABI and bounds | Coverage |
| --- | --- | --- |
| `00AD4C30` | ECX font; five native-string pointer arguments; final `RET14h` at `00AD517E`, length 3, inclusive end `00AD5180` | Partial initial-load resource publication: glyph `00AD5004/5007`, owning space pair `00AD50CB/50D1`, resource half of fallback copy `00AD5109`, CR `00AD5142/5148`. Decoder, renderer64, VFS, strings, tree and unwind remain separate. |
| `00AD53A0` | ECX font, no stack arguments; native final `RET` at `00AD5586` | Partial: typed payload destruction and actual captured GFX/alpha release/clear through `00AD5476`. Native string/tree/sentinel tail `00AD5477..00AD5586`, payload allocator ABI and SEH are excluded. |

The AD4C30 entry/name/texture prefix `00AD4C30..00AD4F3F`, scalar scaling,
record decoding, allocation and tree insertion `00AD4F40..00AD5078`, stream
release `00AD5079..00AD508E`, special scalar setup and final native-string
cleanup are not reimplemented here. Existing scalar decoding supplies the
complete successful scalar state. Adoption publishes the missing resource
fields after that decoding, so it claims the initial-load final resource
identities and subsequent lifetime, not native per-record callback timing or
native failure/unwind behavior. Native reload `00AD51D0` first destroys old
payloads/images before calling AD4C30; it cannot be replaced by this initial
adoption operation.

The producer and ownership evidence is direct:

| Native site | Meaning established by the write or release |
| --- | --- |
| `00AD4CD0`, then `00AD4CD2` | Current renderer `+64` returns the GFX image saved in the consumed stack slot. |
| `00AD4E29/4E2B` or `00AD4E6E/4E76` | Alpha load; the empty authored name selects the literal `white.tga`. |
| `00AD4F3A..4F4A` | Sign-extend source height, scale with SSE, truncate, store only low word at font `+14`. |
| `00AD5004/5007` | Every glyph receives borrowed GFX/alpha at payload `+18/+1C`; no retain. |
| `00AD50CB/50D1` | Embedded space glyph owns alpha/GFX at font `+68/+64`. |
| `00AD5109` | Eight DWORD copy from the required `0091h` glyph; includes both resource pointers. |
| `00AD5142/5148` | Embedded CR glyph receives the same borrowed pair. |
| `00AD540F` | Free each allocated glyph payload without releasing either image. |
| `00AD5431..5456` | Capture current space-GFX, decrement actual `+04`, call its current virtual0 at zero, then clear font `+64`. |
| `00AD5457..5476` | Reload current space-alpha after the GFX callback, perform the same release and clear font `+68`. |

The adoption inputs are two distinct caller slots, each transferring an owned
reference. They may hold the same raw image if they represent two references;
the implementation rejects an aliased nonnull image with total count below two.
Null native results are preserved. The operation validates each nonnull image
against the caller's existing `NativeRenderActualOwners` and requires its
companion to borrow that exact live raw `+04` count. Current original-token
profiles must be `D61948`, `D61870` or `D618B0`, the existing native 2D/cube/volume
texture domain. These tokens are never invoked as callable pointers. All validation and owner
allocation precede transfer, so rejection leaves the inputs owned by the caller.
No new texture wrapper, reference count, COM `AddRef` or per-glyph retain is added.

The owner uses `space_lf_glyph.gfx_texture_18/alpha_texture_1c` as its only
owning slots. Other glyphs and the missing/CR records alias them at initial
publication. Later mutations of an individual glyph resource field remain
visible through that same record; they do not silently change ownership or
retain another image. Teardown follows the native live-slot reads: clearing a
slot occurs after any terminal callback, and alpha is reloaded after GFX
destruction. The actual resource domain must survive the font and any longer
lived material references. Its registered texture companions must perform the
real current texture destruction/pool-return operations when the count reaches
zero; the existing native texture owner/profile implementations are the source
providers. This packet does not fabricate such registrations.

`NativeFontResourceOwners` supplies the C++ lifetime association from exact
`FontDescriptor*` identity to the sole resource owner. It owns published
resources, rejects duplicate descriptor identities and rejects missing or
retired owners. It performs no name fallback and holds no metric cache. The
stable `FontRegistryOwnedFont::descriptor` is suitable; pointers into the
compatibility `FontRegistry.fonts` vector require that vector to remain fixed.
Descriptors, the association collection and resources must outlive Text
borrowers; no mutation/removal is allowed during active calls.

`signed_height_14()` reads the current `FontData.scaled_height` lowword on
each call and sign-extends it. `glyph()` delegates to the existing selector
and returns the same record, preserving space/LF, CR and missing-glyph rules.
At the late Text height site, the caller must reload its current `text.font`
after callbacks, use native zero for null, and only then resolve a nonnull
descriptor. Capturing a different FontData earlier would lose that behavior.

The full renderer `+64` target is `00B319B0` (vtable `D5F0A8+64`, bytes
`B0 19 B3 00`). Its body copies/lowercases the name, applies the optional
guard and calls `00B30B40` with name/flags/0/1. That cache/file loader and the
actual texture creation/registration route remain required before this owner
can load installed fonts itself. Original game addresses are never cast to
callable C++ pointers. No default resolver or pretend successful loader is added.

Ghidra's AD53A0 body still stops at the falsely nonreturning `_free` call ending
`00AD5568`; disk and live bytes establish the continuation `00AD5569..5586`.
No separate function is invented for that tail. All implemented release sites
are inside the current saved body. Worker Ghidra use was read-only.

Validation: strict MSVC Win32 `/W4 /WX /O2 /fp:strict` compilation passed for
the new owner and existing scalar/resource loaders affected by the header.
No tests were added. No native resource load, teardown callback, differential
fixture or game execution is claimed. Typed `std::map` destruction releases
nodes with payloads, earlier than native node destruction; pool, tree, native
font layout and SEH equivalence remain outside this projection.
