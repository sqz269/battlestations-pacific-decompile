# Native directional light, types and configuration integration

The installed-asset diagnostic now constructs a real pooled directional light,
initializes its runtime type descriptors, applies the recovered lighting
configuration, and reads it through the canonical scene registry. Its ambient
owner comes from the existing concrete scene-resource constructor. This replaces
the diagnostic's manually assembled light-list links and separate color fields.

The directional owner uses one actual `1F0h` pool slot: the shared node prefix,
the `174h` shadow identity, the `178h` retained-scene array, and the live lighting
words. The new raw-shadow accessor reloads that same `174h` field on every call.
Null bypasses resolution; a nonnull owner requires an explicit concrete binding.
No shadow factory, matrices or texture resources are supplied by this probe.

The type-counter owner shares the diagnostic's `SingletonLifetimeDomain` with
the particle clock. Root, node, light and directional descriptors use the
recovered guarded initialization sequence and the same monotonic counter. The
light's directional/light/node destruction phases dispatch the recovered type
predicates. The probe checks that manager shutdown destroys exactly one counter
and clears its published slot, alongside the particle-clock cleanup.

The configuration view borrows explicit diagnostic values: ambient and cube RGB
`.35`, diffuse RGB `.65`, zero specular, unit scales, and elevation/yaw near
`.6154797087` / `.7853981634` radians. The recovered x87 angle helper initializes
direction; these are not an authored map or world record. The nonnull config
path initializes every lighting word consumed by the system-constant builder.
It keeps the native discarded alpha product, float32 spills and live constant
reads. The null path separately preserves effective specular and its scale.

The scene resource retains the ambient owner; its canonical registry holds the
same directional companion used by rendering and destruction. Explicit scene
slot publication and light scene attachment retain that scene. After constant
upload, the light deleting destructor removes its registry entry and retained
scene, ends the native light/node lifetimes, and returns its actual pool slot.
The probe then clears the outer slot, releases the scene and ambient, trims the
empty slab, and verifies the remaining pool count and lock recursion are zero.
The diagnostic also handles the partial attach state if registry allocation
throws after backlink publication and before its retain: it takes the missing
real scene reference before ordinary guard cleanup. This leaves the recovered
attach routine's ordering unchanged. That exception path is source-reviewed;
the installed draw does not inject allocation failure.
The model/geometry diagnostic still has its separate scene and host type inputs;
this change composes the system-lighting scope, not a complete native world.

Validation passed:

- Strict MSVC Win32 repository build and both existing CTests.
- Independent reruns of the existing type, directional-owner and lighting
  fixtures. No tracked tests were added.
- Types: six native/host snapshots totaling 480 bytes and 108 predicate calls
  per path. Native counter creation/registration is outside this fixture;
  real host manager creation and terminal destruction were checked separately.
- Light owner: original constructors/deleting destructors for flags 0 and 1
  match all 496 slot bytes and actual pool return. Nonempty callback replacement
  and cleanup-unwind checks are host execution, not native execution.
- Configuration: four native/host calls match 2,916 bytes and x87 status/control
  across null/default mutation and nonnull input paths. The native numeric
  closure contains no hooks or import replacements.
- Primary review matched 42 native spans to the installed executable and checked
  the component source snapshots. The configuration fixture's older scene source
  differs only by extraction of the association-only erase helper; the current
  helper runs in the installed light teardown.
- The installed draw reports native pool/type/config/registry cleanup and shared
  lifetime checks successful. All 77 VS/PS constant registers survive two material
  uploads, D3D9 state restores, and the draw has 2,499 visible pixels and 54 colors.
  The bitmap is byte-identical to the previous diagnostic output.

The original game executable is unchanged. The diagnostic image remains very
dark; identical diagnostic pixels do not establish game visual parity. Complete
shadow/camera/viewport construction and updates, authored environment ownership,
actual full process startup/exit, native exception ABI and gameplay remain open.
The C++ interfaces are not drop-in binary replacements.

Component evidence is in `LIGHT_TYPE_BOOTSTRAP.md`, `DIRECTIONAL_LIGHT_OWNER.md`
and `LIGHTING_CONFIGURATION_APPLY.md`. Source, binary, fixture, build, probe,
annotation and export evidence is recorded in
`reports/native_light_chain_integration_audit.json`.
