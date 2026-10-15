# Scoped native Text effect consumers

`GameNativeGuiEffectApplication::bind_text_effect_consumer` supplies the existing
AB8530 and AB8CE0 Source callers from the **same private effect application and
cache**. It authenticates and binds one exact `GuiTextBufferServices` object.
Binding creates no Text, material, effect, shader cache, renderer or native
string. It changes no compiler mode and installs no global callback or numeric
vtable entry. These are new MSVC Win32 C++ interfaces, not Original ABI entries.

The implementation preserves the legacy caller signatures. A services object
without this optional binding retains its existing callable-host renderer
behavior. Original numeric renderer addresses still are not callable hosts.

## Admission and retained state

Binding requires an idle application and a null services consumer slot. The
renderer publication cell, raw VFS strings, canonical geometry/actual-owner
registry and callbacks, material and parameter process pools, widget/model
owner domain and authenticated D5E520 profile must agree with the existing
application. The exact destruction-access object is retained by reference;
its canonical pool/string/owner references are rechecked on consumer entry.
Shader calls also require the original configuration publication cell, compared
through the renderer's existing `NativePlatformLoadPublication::identity()`.
The texture loader's event-provider cell is not that publication. The caller
must supply a valid actual receiver for +28; a focus-only projection does not
gain whole-platform admission from this identity borrow.

Each whole AB8530 call receives a fresh required-null slot in its existing
`GuiTextSectionOperation`. Each AB8CE0 selection receives a fresh required-null
slot in its canonical `GuiTextLifetime`. The application publishes its stable
heap frame before native effects. Each AB8530 material step has its own fresh
child slot and stable name-header storage before constructing the name.
Completed slots remain history; active or failed slots are never cleared for
retry. A failed shader call cannot be hidden by its already-published +1EC
effect on a later call.

The application's exact retained caller identity authorizes only that caller's
nested material factory or effect acquisition. During whole-caller activity,
unrelated public material/effect calls, new binding and shared-drain closure
reject entry. There is no temporarily disabled guard. The existing application
failure state also covers failures after a nested factory returned, including
later caller work. `requires_process_retention()` includes active callers.

`GuiTextLifetime` asks the same scoped provider about active/failed callers
before retirement, in addition to checking its own retained shader history and
existing section/buffer operations. The provider's successful completion checks
use the live caller; checks after renderer drain inspect retained metadata only
and do not dereference retired Texts, materials or effect results.

## AB8530 material order

The original whole section body remains in `gui_text_sections.cpp`. Shadow
creation, model/mesh setup, section creation/append, native name construction,
material assignment and creator releases keep their existing order. The scoped
change is at the two 535320 sites, AB8654 and AB8727:

1. Retain one child frame and its empty header storage before the existing
   `guidefault.mshd` constructor. This adds no native name copy or retain.
2. Construct that name once through the same string provider.
3. Invoke the existing private-cache material API, publishing the exact
   `GameNativeGuiMaterialFactoryOperation` before its native effects.
4. Preserve native name cleanup before section material publication, then
   release the section, material and applicable mesh creators as before.

FH3 states 4/5 still consume only the armed effect name. The inherited Shadow
name and tracked model construction retain their existing provider boundary.
Completed creators are not rolled back on failure. Moving an effect header's
Source storage into its retained frame does not change the number or order of
native string constructions, allocations, references or destructions.

## AB8CE0 selection and name order

The original whole selection body remains in `gui_text_material.cpp`. A valid
already-cached effect still returns false without selecting or acquiring a new
effect. Otherwise the explicit override wins; the existing ordered signed
DWORD-to-float 720 comparison and font-scale comparison select GuiFont versus
GuiFontBilinear exactly as before.

Only the selected default name is constructed through the native string
provider. An override uses a retained eight-byte borrowed header over the
existing semantic string bytes; it adds no native allocation, copy, retain or
cleanup. The effect application publishes the exact acquisition operation in
a required-null slot before native cache/loader/compiler effects. The existing
legacy acquire signature delegates to that overload with unchanged native
ordering.

The returned caller reference is published into the same +1EC slot before
default-name cleanup, including a null result. True still means selection was
attempted, not that loading succeeded. On unwind, only an armed default name
is consumed. Override borrows end without consuming their owner. There is no
extra effect retain, rollback, cleanup replay or replacement shader cache.

The saved assembly at AB8D5B, AB8DCD and AB8E18 confirms the three renderer
calls. AB8DD5/AB8E1A publish before the default-name return at AB8E42; the
override branch does not construct or consume a replacement native name.
The earlier semantic empty-name comparison remains the established Source
projection; this packet does not claim native string/vector or FH3 ABI parity.

## Storage lifetime and integration prerequisite

Header storage survives with the application's frame after return or unwind.
That does **not** keep consumed name contents readable. Explicit name phases
distinguish construction, live contents, consumption and an ended override
borrow. Nested acquisition frames also end their input-name borrow before
caller cleanup. No later operation rereads a consumed name.

The exact services objects, destruction access, pools, string provider, actual
owner registry, geometry, mapped profile, VFS, Lua owner and renderer must
survive every created resource, retained failure and shared drain. They cannot
be ephemeral stack providers. Failed Text/caller frames must remain alive.
Successful Texts may retire normally; this application must still outlive the
Text objects whose diagnostic histories point at its frames. Its destructor
removes its own services binding only after the existing closure policy passes.
Borrowing and admission checks do not manufacture C++ ownership of providers.

This packet does not open or replay the canonical binary shader-cache bracket.
Cold `guidefault.mshd` may be required by AB9650/AB8530 **before** font-effect
cursor consumption. That ordering and the persistent application/service owner
composition remain prerequisites for activation. A prior GuiFont preload does
not prove that the new consumer ran or that another cold cache entry is ready.

## Validation boundary

The accompanying `reports/cc12_native_gui_text_effect_consumer.json` records
the actual frozen Source, normal MSVC Win32 build, existing checks and artifact
hashes. No new mirrored tests, GPU/window/focus execution, Text constructor,
draw or frontend activation are added. This is Source consumer capability and
build/check evidence, not a Native invocation, Original ABI, visual, startup
or gameplay equivalence claim.
