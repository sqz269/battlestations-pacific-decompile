# Qualified raw DEFAULT Text factory

Status: **UNBUILT new-file Source helper and ownership proposal**. No build,
probe, game, activation, Ghidra mutation or production caller is supplied. No
shared source, CMake, configuration, name/body ledger or existing factory is
edited. Address `00AA1380` is qualified for its DEFAULT normal path only; this
is neither full-body credit nor Original register/FH3/private-stack ABI proof.

Pinned Text dependency: `e75a17321ea79efcb66e615552b5cb067f104247`, over
Model lifetime `48195196e5` and geometry `d403f54118`. The private Model tree/
zero/scalar router is unbuilt `e7690366e326496119516aa82ba9ed836338893e`.
The concrete provider-wiring audit is `e2612187c15388e7656ee9a49f2a11f237c43821`.
Those source versions are dependencies, not acceptance inherited by this helper.
They were inspected as pinned Git blobs because cc9's existing branch is not
their fast-forward ancestor; no old working-tree source was replaced.

## Native schedule and current callers

Cached `exports/bsp/functions/00aa1380/assembly.txt` establishes ECX source,
AB79E0 allocation at AA139E, then for source zero/non-null allocation ECX=EAX
and AB9650 at AA13DD. AA13F2 returns null when allocation is null. The other
branch calls ABB2C0 and is excluded. Native AA1380 has no stack argument and
returns EAX; AB79E0 ignores its incoming1F4 size and selects the SAME F8BDF0.
The new C++ helper has explicit parameters and no copy/source argument.

At e75, current application Text creation still goes through
`GameNativeGuiRenderApplication`'s Text callback (render_application.cpp378),
`GameNativeTextApplication::make_type` (game_hosts_text.cpp788) and legacy
`GuiTextRuntimeFactory::make_type` (gui_text_runtime_factory.cpp140). That
factory requires a semantic GuiWidgetOwner, uses the prefix bridge and legacy
implementation, and its comment explicitly excludes native base-callback
ordering. It cannot be reused by casting or copying an owner/Model/NodeBinding/
SceneResource into the new raw family. AA6560's existing layout-loader Text
entry is likewise not newly wired. The e75 raw93 overload and raw122 binder
have declarations/definitions but no production composition caller.

## New API and ownership boundary

New files only:

- `include/bsp/native_gui_text_default_factory_storage.hpp`
- `src/native_gui_text_default_factory_storage.cpp`

`make_native_gui_text_default_factory_storage_00aa1380(context, acquired)`
borrows the canonical already-initialized `GameNativeGuiTextProcess`, a
persistent `NativeGuiTextRawModelReferenceContext`, its SAME mutable raw93
acquisition, a prepared raw122 scalar frame, aligned caller companion storage
and the real sized companion-disposal record. All graph storage is installed
by the caller before entry; none is created by the factory.

The noncopyable/nonmovable factory acquisition records prepared context,
constructor record, actual pool, entered/returned allocation and constructor
boundaries, raw slot/result, completion and exception. Known-region checks run
before its first mutation. The attempt is then installed before any native
allocation callback. Admission authenticates existing context/pool/type-CELL
identity, extent and fresh records only. It does not initialize IDs, call a
native type getter/predicate early, run startup or create readiness. Reached
getter/callback ordering stays with raw93 and its existing dependencies.

Normal schedule is one genuine AB79E0 call through that SAME canonical pool;
null returns null normally with no constructor. A nonnull slot enters exactly
one e75 raw93 call on the SAME1F4 storage. There is no prefix-fragment replay,
raw payload clearing or constructor bypass. Success requires SAME returned
identity and the genuinely completed mandatory Text+188 Model graph.
Text+188 is a field; its target is separately allocated raw188h Model storage.

Raw122 publication remains a separate existing call, after nonnull completion:

```cpp
void* raw = make_native_gui_text_default_factory_storage_00aa1380(c, attempt);
if (raw) {
    auto& reference = bind_native_gui_text_raw_model_reference(
        c.companion_storage, raw, c.publication, c.reference_frame, c.disposal);
    // Adopt/use the existing initial Text credit explicitly; no extra retain.
}
```

The publication context must already refer to exactly the same raw93 context,
frame and acquisition. Its retirement resources/base and independent188 record
must be the constructor's SAME objects, and its actual pool the canonical Text
pool. The factory neither binds the registry nor constructs a local publication
context whose lifetime would end on return. No unique_ptr/legacy factory return
is used to imply native cleanup or graph ownership.

The caller's retained owner also records the separate bind-entered/bind-failure
state before calling raw122 and preserves it on throw. Factory `complete`
means only its normal AA1380 DEFAULT result, never successful publication or
permission to repeat the existing binder.

## Failures, flags0 and retained188 custody

| Boundary | Retained disposition |
| --- | --- |
| Invalid known caller storage/fresh attempt | Reject before attempt mutation or native effects; no successful factory entry. |
| Installed admission failure | Record failure; no automatic retry or readiness substitution. |
| Allocation entered then throws | Preserve attempt and real pool/provider state even if no slot was returned. Null diagnostic data proves no rollback. |
| Normal null allocation | Complete empty result, no Text constructor or publication. |
| Raw93 admission/provider/constructor throw | Retain slot and the whole independent raw93/85/188 creator/context/frame graph; add no cleanup to whatever raw93 actually consumed. |
| Result admission after raw93 returns | Retain completed native effects; never classify this as a native constructor throw. |
| Later raw122 bind failure | Separate host-admission failure; preserve the completed graph and all reached metadata, with no failed-slot return/retry. |

There is no AB76F0, AB8250, automatic destructor cleanup, inferred native
exception classification or catch-and-return-null fallback. Native AA1380's
failed-constructor slot-return path is not authorized merely by Source
`provider_call`/`host_metadata` or editable completion diagnostics. Any future
disposition API requires separately established genuine unwind/custody proof.

The attempt and independent Model record remain retained beyond Text
companion disposal while any credit, failure or physical slot is outstanding.
Both Text flags0 and Model flags0 may end payload lifetime without returning a
physical slot. `retired` metadata and factory `complete` do not prove cleanup or
permit record reuse. A companion-only disposal callback cannot destroy this
producer graph merely because the Text slot/companion has been retired.

## Real entry requirements and exact remaining dependency

Mandatory188 is never optional for a constructed Text, even for DEFAULT with
empty glyph/content state. Genuine Model B75030/481 reference/B75170 geometry,
mesh, shadow draw section, material and actual GuiDefault effect are required.
Primary4C, parent/children/timed/glyph/cursor/cached1EC branches remain empty;
callback-published unsupported branches are not silently cleared or skipped.

The real existing `GameNativeGuiEffectApplication::native_effect_cache()`
(effect_application.cpp483) is an idle/ready-only borrow with domain validation.
Use its SAME renderer/cache/compiler and the actual sampler/VFS/material/string
providers. Activate the genuine renderer `lua_service_binding()` around
synchronous cache entry, serialize against the facade/device recreation/drain,
and retain actual operation frames through failure. A name/table match, fake
effect, null loader, manufactured completion, cache-hit-only shortcut or a
boolean readiness flag is not that graph. Cold cache paths must also have their
real loader/Lua/event/sampler/VFS services ready if reached.

The frame worker's wiring audit provides these real borrows:

- Existing startup node world supports Model world40 B6DBE0/world3C B6DBC0.
- Geometry is `renderer.game_grid_context().graphics.streams.geometry` with
  the same actual registry and real mesh/section/material terminals.
- Canonical Model01090054 startup is existing CD7F00; constants, strings and
  Model type cells stay in their existing genuine process/counter domains.
- Text base/glyph use `renderer.camera_environment().nodes`. Its point_lights
  is BY VALUE and differs from StartupSceneDomain's separate point-light
  object. Build a private Model tree against that SAME base-runtime point-light
  object, shared owners/imports/recursive frames and e769 dispatcher. Borrow
  shared raw strings/world into the private raw node context; do not copy a
  legacy NodeBinding or borrow the shared node context wholesale.

Before ANY pool effect, e769 must be bound to aliases of this exact nested85
record's actual cell, optional canonical reference, construction, lifetime and
release frames/acquisitions. No second Model credit/lifetime is introduced.
The factory's known point-light/string/record alias checks do not prove opaque
provider code, transitive extents or slab custody; those remain caller proofs.

The smallest remaining provider surface is the approved frame-worker NEW
`native_model_storage_node_dispatch.hpp/.cpp`: real Model type0C/006EF860
(and genuine B6F570 for the still-live base prefix), parenting1C/B6D7B0 and
child50/54/B6ED80/B6EE10, with caller-prepared retained per-call slots and SAME
e769/private node/scene/gate identities. It must admit legitimate D62C88
base-prefix callbacks during B6F440. A table-only or A1 camera-only router does
not implement those callbacks. That helper plus the complete real GuiDefault
provider composition must be integrated/reviewed before production entry.

## Validation boundary

This packet is source/ownership preparation only. Compilation, native DEFAULT
lifecycle/exception/flags0/pool-reuse proof and production wiring remain pending
root's frozen gates and integration. No CMake/ledger change or additional test
case is included. Glyph, cursor, nonempty content, copy construction, full Page,
Original ABI and runtime acceptance are outside this helper's claim.
