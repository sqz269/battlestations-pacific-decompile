# Raw Text AB8CE0 producer/receipt plan

Plan only, based on `276880ca9`; the four proposed code files are identical in
root `5450ea931`. No C++/activation/build/runtime claim. Unlike generic string
helpers, AB8CE0 already retains native renderer/cache/caller-acquisition,
publication and branch-cleanup progress. A typed raw overload can reuse that
real producer. It does NOT by itself admit cached1EC retirement or owned headers.

## Preferred exact API

```cpp
// Keep existing default constructor and legacy entry unchanged.
explicit NativeGuiTextShaderStorageFrame(
    const NativeGuiTextShaderStorageContext&) noexcept;
const NativeGuiTextShaderStorageContext* prepared_raw_context() const noexcept;
// Private Frame member, initialized only by construction; default stays null:
const NativeGuiTextShaderStorageContext* const prepared_raw_context_{};
// Trailing member of NativeGuiTextRawModelReferenceContext, default null:
const NativeGuiTextShaderStorageFrame* const font_shader_frame{};
bool ensure_native_gui_text_font_shader_storage_00ab8ce0(
    NativeGuiTextRawModelReference&, NativeGuiTextShaderStorageContext&,
    NativeGuiTextShaderStorageFrame&);
```

Forward-declare Frame in reference.hpp; include shader_storage.hpp only in
reference.cpp when enumerating its concrete storage. The Frame constructor
binds an immutable typed context address only: no Font export/lookup, current
cell read, name construction, effect/cache call or new native ownership.
Install Frame/context BEFORE initial raw122 bind; pointers cannot be retargeted.
Default-null preserves the existing DEFAULT producer graph and aggregate callers.

Raw bind requires a real prepared context and genuinely fresh Frame/cache/native
progress, using metadata only; never read uninitialized names[]. Its existing
strict DEFAULT payload/93/188/type admission remains. Raw producer preflight
requires the SAME reserved Frame and context, bound canonical raw122, completed
same93/188 graph and actual same cache/owner/string/renderer services. A cached,
entered, failed or replayed Frame cannot be a fresh invocation.

Extend raw122 known-borrowed checks BEFORE bind and later caller-flags admission:
exact Frame extent, prepared ShaderContext, its real Font collection, actual
platform publication CELL, and any reached retained cache key-resolution backing
(when present). Frame contains nested cache/loader acquisition by value. Keep
all transitive providers/resources alive and disjoint under their existing
contracts; no erased region callback or general ownership interface is needed.
Late caller flags may not alias these records. Active/failed producer state
cannot authorize concurrent Text scalar entry or abandonment.

## Minimal body/preflight factoring

Keep separate typed legacy/raw preflights; factor ONLY the existing Native body
and its load/cleanup helpers over a private borrow view of data, effect cache,
raw string context and live one-cell address. Legacy still authenticates its
existing typed domain; raw uses material_strings plus canonical raw THREE-cell
equivalence and the exact e75 owner/cache aliases. No Model cast or fake context.

Preserve AB8CE0 source epochs (`shader_storage.cpp:185-261`): cached1EC returns
false BEFORE name/config/Font/renderer reads; construct/compare/return the empty
header; only then choose override or actual configuration+28. Preserve ordered
CVTSI2SS/UCOMISS/LAHF logic and conditional Font108/+18 read (`104-130`), actual
pooled literals, CURRENT renderer/profile/48 capture and EH arming AFTER capture
(`132-163`). Publish1EC BEFORE selected-name cleanup, retaining point-versus-
bilinear capture order. AL/return means attempted, including native null.
Keep consumed name states, exception cleanup and termination unchanged; no
early import/value sampling, FP conversion, retry or catch-release.

Raw qualification admits the genuine known CURRENT48 B318B0 body only unless a
real alternate provider is separately proved. Reject an unsupported captured
target at its reached epoch with failure retention, never substitute a callback
or reload/force a target. Existing legacy alternate-provider behavior stays.

## Future receipt validation, not ownership admission now

A later cached1EC retirement packet must require SAME prepared Frame/context and
Text destination; phase complete, noncached, attempted, renderer entered/returned,
shader_published, no failure cleanup, and NONNULL result matching CURRENT1EC.
Require known B318B0 plus real nested cache complete/result equality and
caller_acquired (`native_material_effect_cache.cpp:109-135`), with any entered
nested producers quiescent. Registry/count/profile alone is not Text credit.
Credit flow must exclude replacement/consumption by another mutator; a future
raw font setter/shader replacement must record that consumption explicitly.

Cleanup is branch-specific: override requires empty_constructed/compared/
empty_returned and renderer return; it never sets cleanup_returned. Default
GuiFont/GuiFontBilinear additionally needs default_constructed and
cleanup_entered/returned with EH state consumed. Never read consumed name data.
Cached fast return supplies NO new caller credit; attempted-null is normal
Native completion but NO nonnull shader ownership receipt. Entered failure
retains Frame, Text/publication, actual effect credits and all provider graphs.

## Real prerequisites and source dispatch boundary

ShaderContext must borrow the actual Font collection even when a reached native
branch never reads Font108. Height mismatch legitimately skips that read; height
match requires a real same-collection live image, not null/fake metrics or forced
bilinear selection. Nonempty override requires its genuine header producer.
FontHost export remains BLOCKED until the separately planned typed raw borrower
link protects all users through terminal custody; no production accessor/caller,
empty collection, alternate constructor or branch-forcing workaround is added.

Actual renderer/material cache, VFS/raw/typed strings, native imports, data,
platform cell, real Lua activation and caller-owned shader bracket must already
be ready and retained. No early effect callback or second cache/registry/count.
Proposed files: shader_storage.hpp/.cpp, reference.hpp/.cpp and one evidence doc;
Native producer AB8CE0 plus raw122 metadata, no new body credit. Raw118/widget/DEFAULT
owner guards remain strict. Compilation, ABI/FP object inspection and any real
producer/retirement lifecycle are future evidence; this plan ran no tests/games.
