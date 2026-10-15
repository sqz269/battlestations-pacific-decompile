# Native Text draw readiness, CC12

Read-only assessment at Source `b0d57d9f64d31f34f9e5a564534aca360d2b5641`.
The receipt is `reports/cc12_native_text_draw_readiness.json`. This packet changed
no C++, CMake, Ghidra analysis, cache, compiler mode, or installed asset, and ran
no build, fixture, game, or graphics operation.

The first application gap is that no caller constructs `GameNativeTextApplication`
or binds its factory to the native font producer. Inside the already-composed
default Text path, the first **demonstrated effect-provider gap** is earlier than
font shader selection: `AB9650 -> AB8530 -> 535320("guidefault.mshd")` still enters
the legacy callable renderer+48 route. This is a supported-domain source audit,
not proof that every prerequisite GUI/model owner is already bound in the app.

## Current evidence boundary

`cc12_native_gui_font_effect_startup_integration_14.json` describes GUI07 at
Source `d4edc832430c5e501a5f29c7115e8fc1d2d42da3`: six installed fonts, 1060
glyph records, three actual image identities, twelve owning image slots retired,
genuine GuiFont/GuiFontBilinear effects, 119 presents, and shared native drain.
That historical runtime was not repeated here. Its fonts retire before the frame
loop; it explicitly leaves Native Text buffers/rendering unverified and lists
43 unimplemented host methods. No inference about present device availability
is made; the integrator separately reported a later CreateDevice failure
`0x88760868` before texture reload.

Current Source confirms the boundary:

- `game_hosts.cpp:2337-2376` constructs `GameFontHost`/`GameFrontendHost`, then
  optionally runs the native font/effect probes. `game_native_font_probe.cpp:208`
  calls `retire_fonts()` without exporting native fonts or creating a Text.
- `game_hosts_frontend.cpp:199-201,698-699,765-810,987-1011` uses `GameTextHost`,
  copies its glyph quads into `BridgeQuad`, and submits fixed-function
  `DrawPrimitiveUP`. This is not an AB8400/B24010 Text draw.
- `GameNativeTextApplication` is actually implemented in `game_hosts_text.cpp`
  starting at line 476. The public header and those definitions are its only
  production mentions found by the bounded caller inventory. Native font
  production now exists, so older comments saying that producer is absent
  must not be read as current absence of the underlying bodies.

## Original construction and concrete Source

| Original entry | Established contract and current implementation |
| --- | --- |
| `AA1380` | ECX is source; allocate a 1F4-byte slot through `AB79E0`; null source calls `AB9650`, nonnull calls `ABB2C0`; EAX result, plain RET. Live bytes and saved listing confirm the branch and three return sites. `GuiTextRuntimeFactory` uses the same raw slot plus one typed owner/lifetime; its copy operation is conditional on separate clone bindings. |
| `AB9650` | ECX Text, returns same Text, RET at `AB98EE`; base `AA9390(type3)`, numeric `D5C6C8`, derived defaults, then `AB8530`. Source publishes the raw prefix/association and retains its construction frame before completing sections. |
| `AB8530` | ECX Text, no stack args. Even with shadow disabled, missing +188 constructs the `Shadow` model, mesh, section, and `guidefault.mshd` material. It later adds a missing section to existing main geometry; it does not create the main model. Source has a persistent `GuiTextSectionOperation`. |
| `AB7700`, `ABB630`, `AB6AA0` | Existing current74 main-geometry, derived-properties18, and loaded78 implementations. The loader must still create/parent the real primary model and preserve 74, base properties, derived18, 78 ordering. Pool allocation alone does not run the loader sequence. |
| `AD55C0`, `AD4C30`, `AD53A0` | Font constructor is ECX font plus four words, RET10h; no font vtable. Initial load takes five strings, RET14h; direct destruction has plain RET. `GameNativeFontApplication` supplies normal initial production/adoption using genuine images and decoded DAT. Per-field stream/pool/string/tree/FH3 ABI and destructive `AD51D0` reload remain outside this typed composition. |

`NativeFontResources` transfers one decoded allocation and two returned image
references without extra retain; glyph image fields are borrowed aliases.
The font owns two image slots, not two references per glyph. Stable registry
descriptors and the same `NativeFontResourceOwners` must outlive every Text.
`AB8C30` resolves the current name through the published registry and requires
that exact descriptor/resource/actual-owner association, then invalidates its
cached effect. Material texture assignment separately retains actual images.

## First provider seam and required owner contract

`gui_text_sections.cpp:77-106` calls the canonical geometry owner's legacy
`create_material_for_effect_00535320`. Its implementation at
`gui_native_geometry.cpp:417-440` invokes `native_material_factory.cpp:49-55`,
which casts the renderer's numeric +48 cell to a host function pointer.
Fresh Ghidra bytes confirm `D5F0A8+48 -> B318B0`; that original image token is
not a callable address in this reconstructed executable. AB8CE0 later has the
same unresolved call in `gui_text_material.cpp:25-40`.

The missing implementation is **not** the whole effect/material loader:

- `create_native_material_from_effect_cache_00535320` already executes numeric
  `D5F0A8/B318B0` with a fresh persistent `NativeMaterialFactoryAcquired`.
- `GuiNativeGeometryOwners::register_native_material_creator` already admits
  the completed creator into the same canonical registry without extra retain
  or destructor rollback. Bind failure retains the raw creator and metadata.
- `GameNativeGuiEffectApplication` already owns the private cache/compiler/
  loader graph. Its effect acquisition enters the same `NativeLuaServiceBindings`
  activation, checks the live renderer, retains one-shot frames, and transfers
  the caller reference. It must survive the shared renderer drain.

A future binding should be an operation on that retained application owner,
with the current receiver captured at the recovered call site, a stable actual
name header, and one fresh persistent material/effect acquisition frame. It
must preserve allocation, effect release, canonical registration, failure, and
terminal-callback ordering. The caller's section/Text operation must keep this
frame and all referenced name/provider storage alive after interruption. A bare
cache-context accessor would not supply Lua activation, invocation lifetime,
failure retention, or the existing owner's reentry/drain checks. Installing a
callable replacement numeric vtable is not required by the existing concrete
source dispatch approach.

Retained invocation metadata must also record whether native name cleanup has
already consumed a header. Keeping the frame alive does not undo the recovered
AB8530 cleanup or make an interrupted input replayable.

There are two distinct sites to connect: constructor `guidefault.mshd` material
creation first, then AB8CE0 effect acquisition. AB8CE0's selection is already
implemented: explicit shader name wins; otherwise live configuration+28 equals
720 and font scale equals 1 selects GuiFont; the remaining arm selects
GuiFontBilinear. Do not substitute either shader or change compiler mode.

The integrator/world stream owns common shader-cache/preload sequencing. Any
required cold `guidefault.mshd` acquisition must be reconciled with that single
canonical binary-cache lifetime and forward-cursor order before its one-shot
close. GUI07's two-effect result alone does not establish this constructor
acquisition, a safe new preload order, or permission to recreate the cache.

## Geometry, states, and draw are implemented but not app-bound proof

| Stage | Concrete Source and remaining application obligation |
| --- | --- |
| Submit/content | `ABAED0` source or `AB6AB0` UTF16 reaches the same `ABA8D0` retained continuation; sections are ensured even for equal-empty input. No substitute scratch Text state. |
| Buffers `AB8400` | `simplecolor.mvfm`, current38 `B317E0`; current5C `B287C0` with capacity*4, flags1; current60 `B288B0` with capacity*6, flags1, format65. Existing concrete dispatch shares declaration cache, physical streams, pools, renderer cell, and actual geometry registry. Mesh setter retains precede creator release; declaration release precedes index creation. |
| Single line `AB9FD0` | Actual vertex/index mappings use length*4/length*6. Section+10 is vertices and +18 is length*2 triangles. First glyph binds the real GFX/alpha identities to material slots0/1. It invokes the x87 quad kernel on actual mapped storage, then unlocks index before vertex and rebuilds layout. |
| Wrapped `ABA270` / glyph `AB98F0` | Existing wrapping, real maps, format conversion, x87 scalar kernel, UV/index stores, and retained optional-child continuation. A pending child is not completion; mappings and parent invocation survive it. Single-glyph proof need not claim wrapping, copy/cursor, ellipsis, or optional-child parity. |
| Layout `B865A0` | `GuiTextNativeLayoutServices` already dispatches real stream+24 `B48CE0` and renderer+40 `B2F710`, using the same hardware pool/tree/owners. Earlier documentation that lists these as absent is superseded by this source. |
| Material/shadow | `B19210` retains selected effect; `B189F0` retains textures. Native parameters borrow Text/global field storage. Shadow uses the actual main streams and current material/section layout. Same raw Text prefix+04 parameter-owner retention exists for reached cursor/copy paths; do not substitute a detached owner token or duplicate count. |
| Queue and command | `native_render_queue_execution` and `B1D950` exist with stable command/system/batch frames. App must supply the same actual camera, targets, scene/system constants, synchronization, jobs, and entries. |
| Pass | `B55550 -> B45360 -> B454D0/B44750 -> B43410` is concrete. Pass frames retain their own constant-builder frame; render/sampler state binders are `B27A80/B27B90`, shaders `B21D10/B21C20`, textures `B24710`, constants `B42350` plus native uploads. Current-profile and reached-branch obligations remain. |
| Device draw | `B24010` uses real `DrawIndexedPrimitive` (device table+148), with native lost/inhibit/count gates; nonindexed `B21B40` also exists. A provider call can return without issuing a draw. Real geometry, device call, selected pass, and output must be observed. |

These files are registered in `CMakeLists.txt` and `cmake/startup.cmake`; Text
implementation is linked through `game_hosts_text.cpp`. Current searches find
no application composition of `NativeRenderCommandExecutionContext` or
`NativeMaterialPassExecutionContext`; their source definitions and queue calls
are present. Registration and historical builds do not establish actual Text
construction, command submission, or draw. Full-scene `B1645B` readiness remains
the integrator/world stream's separate boundary.

## Smallest meaningful installed-font draw proof

This is a proposed acceptance packet, not an execution claim:

1. Keep the current process renderer/VFS/string pools/Lua binding/effect graph,
   actual registry and shared singleton/drain domain. Provide the declared
   canonical GUI model/parenting/material/Text-pool and constant bindings;
   construct one retained `GameNativeTextApplication`. Export the producer's
   native fonts and bind **that same empty factory** before constructing Text.
2. Complete constructor `guidefault` and font effect acquisition through the
   owner/invocation interface above, in the approved common preload order.
   Retain the genuine initial font production across the draw instead of using
   the current probe that retires fonts before frames. No private cache or
   parallel texture/material registry.
3. Create one default Text through the real owner/factory/primary-model order;
   set installed `Arial16`, single-line mode, and one supported visible UTF16
   glyph such as `A` through the existing properties/submission path. Its buffer receipt must
   identify four actual vertices, six indices/two triangles, completed unlocks,
   actual declaration/layout, actual GFX/alpha slots, and the naturally selected
   real GuiFont variant. Only claim the observed single-line supported domain.
4. Submit its actual mesh/section/material through the existing native command/
   pass/draw providers, using retained caller-owned frames and the same real
   camera/viewport/system/target contexts. Observe nonzero B24010 arguments and
   the actual device call; a success return or an unrelated Present is insufficient.
   Capture output that demonstrably comes from this Text path, with the frontend
   bridge excluded from that proof observation. No handwritten sprite quad or
   shader substitution satisfies it.
5. End frame/pass work, retire Text/cursor/children and retained material/stream/
   layout references, require that factory's allocation count reaches zero,
   then `retire_fonts()` while the factory still lives. Keep effect/compiler/
   sampler/context/name/acquisition owners through the original shared renderer
   drain, then call `after_native_renderer_drain`. An interrupted operation
   retains its whole process graph; no replay, forced completion, or second drain.

Record immutable Source/executable/assets, the branch/gate actually reached,
object identities and relevant raw counts, draw arguments, image output, and
terminal teardown. If CreateDevice fails, report that earlier boundary; do not
classify the attempt as a font failure. Even a passing packet would establish
a bounded installed-font Text draw, not Original register/class/private-stack
ABI, FH3 equivalence, full GUI visual parity, faithful whole startup, or gameplay.
