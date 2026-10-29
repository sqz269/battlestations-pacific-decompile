# One retained raw DEFAULT Text owner

**Unimplemented Source proposal.** Pinned dependencies: Text `e75a17321`, Model
tree/zero `e7690366e`, Model node/scene/type `2049a6f19`, DEFAULT AA1380 helper
`ba2a9890c7beed8c4363154cc9bba197102bd21a`. Current A2 composition was inspected
as committed `51e6199d`, not mutable shared files. No build, probe, game, source
activation or Ghidra mutation was performed. No Original ABI/body credit follows.

## Minimal new owner API

Proposed NEW files `include/bsp/game_native_raw_default_text_owner.hpp` and
`src/game_native_raw_default_text_owner.cpp`, with a private nonmoving Impl:

```cpp
GameNativeRawDefaultTextOwner(log, renderer, vfs, singletons, data, input);
prepare_metadata_in_ready_renderer(model_call_capacity);
construct_default_in_open_shader_cache();
retire_before_native_drain();
requires_process_retention() const noexcept;
after_native_renderer_drain();
```

The caller installs this owner BEFORE preparation or entry. Constructor holds
real service references only. Preparation installs service/attempt objects before
exporting sampler bindings or building further metadata; any failure latches the
whole graph. Sampler destruction already rejects exported-but-unclosed metadata
(`game_native_gui_sampler_application.cpp160-162`), even before native loading.
No failed partial preparation is silently erased to retry.

Own dedicated existing `GameNativeGuiSamplerApplication` and
`GameNativeGuiEffectApplication` METADATA, using the real input/platform, VFS,
singleton, renderer/type/string/owner domains. Their constructors borrow the
existing cache/compiler/registry services; they create no native shader cache,
procedural factory setup, material manager or effect. This avoids a new public
getter for A2's private facade and gives the owner actual control of facade
lifetime. Existing sampler knowledge/evidence rules remain intact; no B-16/MSG
residue or completion is fabricated.

## Pure borrows and the one attempt

Borrow renderer scene domain, camera base runtime, node constants, actual-owner
registry, grid.graphics.streams.geometry, material destruction profile and Lua
binding. Borrow canonical VFS raw/typed strings and type storage/counter, process
Text/Model/material pools and current data/GUI state cells. Material/parameter
access objects are persistent views of those real process pools and strings.
`effects.native_effect_cache()` supplies the real cache/compiler context.
No Font, locale, layout, glyph, property/content or standalone GuiWidgetOwner
is required for DEFAULT; do not construct the large legacy Text application.

One heap-stable Attempt contains these independent objects BEFORE AA139E:

| Storage | Required identity/custody |
| --- | --- |
| ba2 factory context/acquisition | Canonical Text process; exact raw93 record/publication/frame; nonmoving acquisition |
| e75 raw93/85 acquisition | Its ONE nested ModelRecord; no copied lifetime or replacement owner |
| Raw DWORD arrays and frames |85 locals/arguments, parenting frame, geometry frame and raw93 frame; real initialized lifetimes |
| e769 records/provider | Exact nested Model actual/reference/construction/lifetime/release aliases |
|2049 records/provider | Same e769 record; caller-budgeted prepared call array, never recycled |
| Private node metadata | Base-runtime point_lights; shared owners/imports/strings/world/recursive frames; real type gate/scene/parenting providers |
| Raw122 context/frame/storage | Exact constructor/retirement graph; separate aligned companion buffer and persistent scalar frame |
| Disposal context | A SMALL separate `{owner, attempt}` object with its actual extent, not sizeof the enclosing Attempt |
| Owner entry/retirement records | Factory/bind/scalar entered/returned/failed markers and independent volatile flags cell |

Assemble private gates/scenes/trees/node,481 storage/identity, geometry/parenting,
e769/2049 bindings, then e75 sections/default/retirement/raw122 contexts. Bind
both Model dispatchers to the SAME record. All contexts, type facades, call slots
and opaque provider backing remain stable. Capacity is a qualified budget, not
an invented Original constant or universal callback bound; exhaustion retains
the already-entered graph.

## Real construction schedule

Text pool startup is in game_main1142, but Text/Element TYPE startup occurs at
game_hosts2829, after the early A2 bracket. Prepare this owner only after those
types and renderer scene domain are ready, while metadata getters can still
admit an idle renderer. Construction requires a real later open canonical
shader-cache bracket; the existing mission late-cache opening/callback at
game_native_mission_ship.inc177/190 is one concrete integration point.
The owner neither reopens A2 nor assumes GuiDefault is already warm. A standalone
caller needs a separately reviewed real bracket/hook; no empty cache or fake
effect is supplied. Public cache/Lua/effect interfaces already suffice.

Activate the SAME renderer Lua binding around synchronous entry. Install the
factory-entered marker, invoke ba2, and retain its exact failure. A null return
completes the native empty outcome without construction/publication. For a real
completed raw93, install a separate bind-entered marker BEFORE calling existing
raw122 binder. Binding failure is extra Source admission; retain the completed
unbound native graph. No AB76F0/AB8250/slot return or retry is inferred.

## Widget binding boundary requiring explicit implementation

Existing `NativeGuiWidgetLifetimeDispatch::widget` recognizes legacy
`NativeGuiTextReference`, not `NativeGuiTextRawModelReference` (cpp52-71).
Existing AA8320's Text188 helper also uses legacy node bindings. Neither can be
cast into the new transport or presented as raw child support.

For this strictly DEFAULT owner, supply private canonical table/type binding
using the actual base/Text data views and current Element/Text lineage cells.
Use genuine A9E070/AB83D0 predicates and actual A9BD50 list-removal body where
admitted. The real e75 AB8250 releases raw188 first; AA9730 then stamps the base
and uses its genuine builtin predicate. e75 reauthenticates empty/default state
before those calls, including ordinary zero matches. Excluded child-scene,
child/timed destruction and nonempty hierarchy must reject explicitly and retain
failure, never return false/success or route raw188 through GeneratedModelNodeLifetime.
That closed-domain binding is necessary owner code, still subject to root review;
broader AA8320/raw child transport remains a separate genuine missing surface.

## Retirement, disposal and parent drain

This owner retains an unpublished private DEFAULT Text allocation. With callers
quiescent, install scalar-entry state and seed a separate retained volatile flags
cell with literal1, then call canonical raw122 direct current04/AB8EE0. This is
the genuine forced scalar path, not an added Text count gate/atomic decrement.
Late callback changes to that cell remain meaningful; failure authorizes no
cleanup or pool return.

Saved raw122 disposal runs LAST. It may placement-destroy the retired Text
companion and mark its storage ended; it MUST NOT erase the Attempt, ModelRecord,
dispatchers, facade services or frame backing. No reference/slot read follows
companion destruction. Owner bookkeeping subsequently reads independent records
only. Surviving Model credits may retire through the genuine canonical481 path
later; unbind/retired flags alone prove neither cleanup nor physical return.

Before parent drain require quiescent calls, no entered failure, Text scalar
complete, and actual Text/Model physical-return decisions settled. Failed calls
retain the whole graph for process disposition, not guessed resolution. Flags0 or surviving
Model custody preserves the whole graph and blocks guessed teardown; no forced
second release/return is added. Keep the owner through real singleton/renderer
drain. Verify renderer.after_native_drain() FIRST, then close effects/samplers.
Raw cache-context borrows do not populate the facade's operation vectors, whose
empty case may otherwise close early (effect_application.cpp601-606). Sampler
closure additionally requires no scopes/creators and null renderer/clock/registry.
Only then destroy Attempt, effects, samplers and remaining context metadata.

No shared API change is required to implement this dormant owner. Root owns
CMake registration and the actual call/retirement hooks. Opaque full extents,
transitive disjointness, canonical slot/type provenance and native serialization
remain explicit proofs. Runtime GuiDefault/sampler/residue, failure, flags0 and
repeated-lifetime acceptance are pending; no Page/frontend completion is claimed.
