# Text-context update and retained geometry ownership

Addresses: `00aba8d0`, `00ab8530`, `00ab8400`, `00ab8250`, `00ab80c0`, `00abb1d0`, `00ab6ab0`, `00abaed0`, `00abb000`, `00ab8ce0`, `00ab8c30`, `00ab98f0`, `00b73bb0`, `00b73b70`, `00b73c60`, `00b86550`, `00b865a0`, `00b19210`, `00b6dfa0`, `00aa9730`

`00aba8d0` is a one-argument UTF-16 update routine, not a layout routine with
an extra caller flag. It ensures the main/shadow drawables, compares the
transformed text with the cached string, and rebuilds only when different.
Changed empty text clears two draw ranges while retaining their resources.
Changed nonempty text requests vertex/index objects, runs the existing
single-line or wrapped builder, and shares the resulting buffers with the
shadow geometry. This is the next useful ownership boundary above the scalar
layout components; it is not a recovered complete GUI object or draw method.

This read-only packet used `Client` from `tools/ghidra_export.py` with
`config/target.json`, verifying project `bsp`, program
`/battlestationspacific.exe`, x86 LE32 and base `00400000` before each live
batch. Main code stayed in `00ab6000..00ad89ff`. The primary explicitly
authorized eight exact renderer/base helpers below. No Ghidra mutation,
C++, shared metadata, build or test was performed. The only tracked additions
are this document and [font_context_ownership_audit.json](../reports/font_context_ownership_audit.json).
Ignored evidence is under `exports/bsp/parallel_font_context/`.

## Exact update boundary and caller flags

`00aba8d0..00abaec6` takes ECX context, one stack pointer to a native UTF-16
string wrapper, and returns with `RET 4`. No semantic return value was
established. The first input is read at `00aba8f1`; by `00aba9eb`, the same
consumed stack slot holds the AL result from `00ab8ce0`. Its later byte test
at `00abac0b` is therefore **shader-selection-attempted state**, not another
argument and not a force-update flag. Selection returning a null shader still
reports AL 1; an already cached shader reports zero.

The observed sequence is:

1. Call `00ab8530` before inspecting the text. Copy the supplied UTF-16
   wrapper, and apply `00a9ec30` when font `+48` is nonzero. That external
   transformation remains the explicit already-transformed input boundary of
   the scalar components; no host locale substitute was recovered here.
2. At `00aba921..00aba94f`, two empty strings compare equal. Two nonempty
   wrappers are compared by the existing CRT `__wcsicmp` at `00c03a39`.
   There is no stored-length equality check in this comparison. Equal text
   jumps directly to temporary-string cleanup. A case-only change can skip
   geometry and preserve the previous cached spelling.
3. For changed text, reset width `context+114` to float zero, clear optional
   child objects through `00ab80c0`, obtain geometry 0 and section 0 from
   `context+4C`, and assign the transformed string to `context+EC/+F0`.
4. If the new stored length is zero, clear vertex count `section+10` and
   primitive count `section+18` in both main and shadow geometry 0/section 0.
   Return without buffer creation, shader selection, scalar layout, sharing,
   shadow constants or the later virtual update. Existing vertex/index,
   material and layout references remain. The line count `+110`, wrapped
   extent `+178` and first origin `+18C` are not reset by this empty branch.
5. Otherwise request buffers using the stored UTF-16 length, initialize
   primitive type 4, base/minimum vertex 0 and start index 0, and clear both
   section counts. After material setup and section-layout preparation,
   `context+FC == 0` selects `00ab9fd0`; any nonzero byte selects `00aba270`.
   Both receive the copied text wrapper and the main section.

An equal empty update still runs `00ab8530` first; it is not a universal
zero-work operation. Neither the equality check nor the empty branch checks
whether width, scale, alignment, shader override or font identity changed.
`00ab8c30` changes the font lookup result and releases/clears `+1EC`, but does
not itself invoke the geometry update.

The direct callers explain the separate flags:

| Routine | Actual ABI | Observed dispatch |
|---|---|---|
| `00ab6ab0` | ECX context; UTF-16 wrapper; `RET4` | Calls `00aba8d0`, then context virtual `+50` with `context+50`, even if text compared equal or was cleared. |
| `00abaed0` | ECX context; narrow-string wrapper, byte flag; `RET8` | Compares/caches narrow input at `+F4/+F8`. Zero flag calls narrow-to-UTF-16 helper `004c5e60`; nonzero calls singleton helper `00a9fad0` with the input and output wrapper. Either result goes to `00aba8d0`, followed by virtual `+50`. The external resolver's full lookup semantics are not established here. |
| `00abb000` | ECX context; narrow wrapper, float width, byte flag; `RET0C` | Same conversion choice, then `00ab8f00` ellipsis before update. Ordered equality with **-1.0f** selects `context+20` as width; zero remains an explicit zero target and NaN does not select the fallback. Live bytes at `00d7a260` are `00 00 80 BF`, correcting this audit's earlier zero-constant interpretation. See [GUI_TEXT_ELLIPSIS.md](GUI_TEXT_ELLIPSIS.md). Equal cached narrow input skips the body, including reconsideration of width/flag. |
| `00abb1d0` | ECX context; no stack args; `RET` | Copies current UTF-16 text, assigns empty text to `+EC/+F0`, then calls `00aba8d0` with the saved copy. This forces a rebuild for previously nonempty text; an already empty string still compares equal. |

The update call at `00ab9e61` belongs to the optional per-glyph child route
inside `00ab98f0`, not ordinary quad output. After writing the quad, that
route requires nonnull context `+1B0` and `+1AC`, zero `+DC`, nonempty
`+1A4/+1A8` and membership of the current code unit in that string
(`00ab9c5e..00ab9cc4`). It changes UVs, constructs a child, and recursively
updates that child's text. The existing quad fragment excludes this route.
A bounded owner must keep those optional children disabled rather than claim
that ordinary geometry covers them.

## Resource creation and replacement

`00ab8530` takes ECX context and no stack arguments. When `+188` is null it
requests a drawable of size `184h`, constructs it with the literal `Shadow`,
clears drawable `+138` bits 0/1, creates a geometry, and assigns it to slot 0.
It obtains a draw section and `guidefault.mshd` material, appends the section
and assigns that material. It releases the temporary section, material and
geometry references after assigning them. Existing main drawable `+4C`
gets a section/material only when geometry 0 has no section. This routine
does not create the main drawable when `+4C` is missing.

Shadow byte `context+15C` controls attachment to the main drawable through
`00b6e680`; false detaches it and calls `00b6d890(0)`. This setup happens
before text equality and is repeated near the end of nonempty updates.
Native allocation/null failures are unchecked in these bodies.

`00ab8400` has **two stack arguments, glyph capacity and geometry; RET8**.
ECX is not consumed as a receiver. It requests `simplecolor.mvfm` through
renderer virtual `+38`, calls renderer `+5C` with `(4*N, 1, declaration)`,
and calls renderer `+60` with `(6*N, 1, 0x65)`. The final value is the
16-bit index format used by the existing renderer. It assigns stream 0 and
the index object to the supplied geometry, then releases the temporary
vertex, declaration and index references. There is no capacity reuse check
in this helper or the changed-nonempty caller. These are repeated factory
requests; physical allocator/cache behavior behind the factories is separate.

Fresh setter evidence establishes the retained ownership:

| Helper | Original ABI | Retained relationship |
|---|---|---|
| `00b73bb0` | ECX geometry; stream index, stream; `RET8` | Geometry `+64+4*i` retains the new stream, then releases the old stream when different. `+7C` is the stream high-water count; text uses index 0. |
| `00b73b70` | ECX geometry; index object; `RET4` | Geometry `+60` retains new and releases old on pointer change. |
| `00b73c60` | ECX geometry; section; `RET4` via API tail jump | Appends the section in order and increments its intrusive reference. Vector expansion is external. |
| `00b864c0` | ECX section; material; `RET4` | Existing cached reconstruction retains new/releases old at section `+20`; not newly live-exported in this packet. |
| `00b19210` | ECX material; shader; `RET4` | Retains new/releases old at material `+7C`. Also marks a nonnull shader `+B4 = 1` and clears the material's registered parameter entries/count, including when the shader pointer is unchanged. |

Thus releasing the local constructor/factory reference is not releasing the
assigned resource. Materials retain texture objects through the already
audited `00b189f0` slots. Glyph texture pointers remain aliases of font-owned
resources, as documented in [FONT_MATERIAL_BINDINGS.md](FONT_MATERIAL_BINDINGS.md).

## Section layout cache, locks and shadow sharing

`00b865a0` is **ECX section, one geometry stack argument, RET4**. Its old
fastcall/no-argument pseudocode loses that argument. When section `+50`
contains a renderer layout object, it calls `00b86550`, releases that layout
reference and clears `+50`. `00b86550` releases and nulls each stream at
section `+3C`, then resets count `+4C` to zero. If count is zero,
`00b865a0` gets geometry stream 0 and retains it. It gathers each retained
stream's virtual `+24` result and requests renderer virtual `+40`, storing
the returned layout at `+50`. The per-stream descriptor meaning agrees with
the existing declaration/layout interface; the complete factory remains an
external contract.

There is an exact qualification: `00b865a0` clears old streams only when an
old layout exists. A nonzero stream count with a null layout is reused as-is.
A host fragment can require the successful coherent-cache domain; it must
not claim that unconditional clearing emulates native failed creation.

The update prepares the main section before layout at `00abab54`. The
single-line path prepares it again after unlocking at `00aba255`; the
wrapped path does not make that second call. Both lock stream 0 with
`(4*N,0,0)` and the index object with `(6*N,0,0)`. They unlock index first,
then vertex. Single-line section counts are `4*N` vertices and `2*N`
triangles. Wrapped counts are based on emitted glyphs, with the native
low-16 narrowing already documented in [FONT_WRAPPED_LAYOUT.md](FONT_WRAPPED_LAYOUT.md).
Neither routine issues a device draw.

After layout, `00aba8d0` calls context virtual `+50` with `context+50`, then
copies the main vertex stream and index object into shadow geometry 0 using
the retaining setters (`00ababa0..00ababc1`). It copies section `+8/+C/+14/
+10/+18` into the shadow section and prepares that section's layout. The
main and shadow therefore share vertex/index identity; shadow does not run
another glyph/layout pass.

Main texture slots 0/1 are assigned from the first emitted glyph by the
existing builders. The shadow material receives only **slot 0**, copied from
the main material when its signed texture count is positive, else null
(`00ababec..00abac06`). This code does not copy main texture slot 1.

## Shader and parameter lifetime

The context owns the renderer result cached at `+1EC`: `00ab8ce0` stores it
without a second retain, and name change/destruction decrements it. On a new
selection attempt, `00aba8d0` assigns this shader to the main material and
registers pointers to context-held parameter values:

| Registration | Context source |
|---|---|
| `cOverbrightAlphatex` | Adjacent `+1DC/+1E0`, populated by x87 float stores from `+94/+1D4` |
| `cLowColor` | `+A4` |
| `cHighColor` | `+B4` |
| `cBlendFactor` | `+C4` |

The exact string-based parameter helper bodies are an existing external
contract; do not infer a deep copy from these pointer arguments. The native
context keeps the addressed fields alive. Additional inherited setup goes
through `00aa9f10`.

The same saved selection-attempt byte gates shadow shader/parameter setup.
Shadow receives the main material's shader. Its registration sequence includes
`cBlendFactor` with global `00f8be54`, then the pair/color/blend context pointers
above. The repeated blend name is preserved as an observed call sequence;
replacement/duplicate policy belongs to the parameter helper contract.

Every nonempty update also writes context `+168..+174` to the shadow material
parameter returned by `00b179f0(0)`, computes a shadow translation, and
updates attachment. The translation uses signed font height divided by
double 720, multiplied by `context+164`, and spilled to float before placing
that value in both x/y components (`00abade8..00abae44`). The z component is
selected by `context+160`. These operations are separate from wrapped
alignment and must not be folded into raw glyph positions. This packet does
not port the inherited scene transform/material-constant system.

## Teardown and uncertainty boundary

`00ab8250` takes ECX context and returns normally at `00ab83CF`. It installs
the text-context table, releases/clears cached shader `+1EC`, passes shadow
`+188` to `00b6dfa0` and clears that field, then clears optional children.
It releases owned narrow/UTF-16 strings and frees the child vector storage,
and finally invokes base teardown `00aa9730`. The cached decompiler falsely
returns after `_free` at `00ab8345`; raw assembly continues through string
cleanup, base teardown and the actual return. No local font release occurs
in this destructor or `00ab8c30`'s direct font-pointer store; registry lifetime
remains the font lookup contract.

Base teardown detaches the main drawable from its parent/container, calls
`00b6dfa0` for `context+4C`, and clears that pointer (`00aa97A5..00aa97F3`).
`00b6dfa0` unlinks the drawable from its parent list or registration, then
tail-jumps to drawable virtual `+18`. Its target/destructor and complete
renderer object destruction are not established in this packet. The observed
call is an ownership handoff to native virtual cleanup, not sufficient proof
of a concrete COM destruction sequence.

Ghidra's saved body for `00aa9730` ends at `00aa9950` after another false
no-return `_free`. Authorized raw continuation demonstrates list cleanup,
base cleanup `00bd30f0`, SEH restoration and the real `RET` at `00aa99B8`.
The original body and continuation are recorded separately. A read envelope
extended to the next known inventory entry at `00aa9A20`; bytes after the
real return are not used as this function's evidence or proposed annotation.
No global no-return annotations were changed.

## Next implementable host API

The next code packet can replace the probe's manual layout-to-buffer glue
with an owning **font geometry update** component. It can use existing
`LogicalVertexStream`, `LogicalIndexStream`, `D3D9VertexLayout`,
`MaterialTextureSlots`, scalar layouts and the quad writer. It must keep the
native scene-node constructors, virtual `+50/+18` callbacks and optional
children outside its advertised behavior. No no-op versions of these calls
are needed for a useful geometry owner.

Proposed interface, to review before C++ work:

```cpp
enum class FontGeometryUpdate { unchanged, cleared, rebuilt, unsupported };

// A move-only host owner. It retains font resources, logical vertex/index
// objects, declaration/layout and material texture projections. Main/shadow
// draw ranges refer to the same buffers. No raw glyph pointer is retained.
class FontGeometryOwner;

FontGeometryUpdate update_font_geometry_00aba8d0_fragment(
    FontGeometryOwner&, D3D9StateCache&,
    std::u16string_view transformed_text,
    const FontGeometryUpdateParameters&, std::string& error);

FontGeometryUpdate rebuild_font_geometry_00abb1d0_fragment(
    FontGeometryOwner&, D3D9StateCache&,
    const FontGeometryUpdateParameters&, std::string& error);
```

`FontGeometryUpdateParameters` should expose the actual existing single-line
or wrapped parameters, the checked declaration/quad layout and fixed origin,
and an explicit multiline byte. Prepared shader/material bindings and font
resources must have real retained owners supplied at owner creation. A draw
view should expose primitive type, base vertex, vertex count, start index and
primitive count, with its validity tied to the owner. Shader compilation,
camera, render target and actual submission remain the caller's work.

Independent integration review adds two concrete API requirements. Owner creation
must receive an explicit borrowed `IDirect3DDevice9&` or resource factory:
`D3D9StateCache` keeps its device private, while buffer recreation and layout
creation require device access. Keep borrowed shader wrappers and parameter
addresses in stable heap storage across owner moves. Equal-COM shader rebinding
can preserve the previous wrapper address, so rebinding cannot repair relocation.
Define the cache lifetime/unbinding rule explicitly and release its dependencies
before the owner's retained font resources.

The implementation contract should be concrete:

- Retain `FontResources` as well as `LogicalTexture` projections: current
  projections borrow their COM texture and cannot alone keep font images
  alive. Retain logical buffer/declaration/layout identities while the state
  cache can reference them, and follow existing unbind/reset and registry rules.
- Accept already-transformed, coherent UTF-16 with no embedded NUL, a loaded
  nonnull font, valid geometry layout and no optional children. Stored native
  string length drives allocation and single-line counts, while its loops
  stop at NUL; this gate prevents a false equivalence for mismatched lengths.
- Preserve the nonempty case-insensitive same-text shortcut. Expose rebuild
  separately to model `00abb1d0`; do not invent a caller flag in `00aba8d0`.
  A rebuild of empty cached text still produces the native equal-empty result.
- For changed empty input, publish cached empty text and zero main/shadow
  counts without discarding buffers/materials or silently refreshing stale
  native line-height metrics. Width becomes zero.
- For changed nonempty input, enforce existing finite/scalar domains and the
  16,384-glyph/16-bit-index capacity before resource mutation. Create and fill
  the real logical buffers, unlock, and publish shared main/shadow draw ranges.
  Do not derive separate shadow vertices or multiply normalized offsets twice.
- Bound failure behavior explicitly. Transactional host allocation/upload
  failure is useful but differs from native's early string/width assignment
  and unchecked allocations. A returned `unsupported` must not be presented
  as an emulated native error. The shader cache/parameter-registration domain
  should be fixed at owner creation until its full owner is implemented.

This proposal replaces concrete geometry preparation and retains its products.
It does not claim to reconstruct `00aba8d0` in full, automatic font/shader
reload, localization, optional child UI, shadow scene registration, device-reset
ordering, original renderer ABI or original-game rendering.

## Byte identity and annotations

The 21,504-byte main envelope `00ab6000..00abb3ff` matched the installed PE,
SHA-256 `e193afaf1a696909b70f2ea28d8359e4e534ad0f288d1e4982792b086ce77817`.
Function listings were decoded separately from established entries because
that envelope starts inside an earlier routine. All eight approved helper
spans also matched disk. The report records complete per-function hashes,
target checks, original names, ABI corrections and proposed append-only
comments. Original executable hash remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Proposed descriptive names include `BSP_TextContext_UpdateUtf16Geometry`
(`00aba8d0`), `BSP_TextContext_EnsureDrawSections` (`00ab8530`),
`BSP_TextContext_CreateGlyphBuffers` (`00ab8400`),
`BSP_TextContext_RebuildCurrentText` (`00abb1d0`),
`BSP_DrawSection_ClearVertexStreams` (`00b86550`) and
`BSP_DrawSection_RebuildVertexLayout` (`00b865a0`). These are reconstruction
names, not recovered symbols. Retain existing CRT, compiler and descriptive
library identities. The primary owns any name/comment/ledger application.
