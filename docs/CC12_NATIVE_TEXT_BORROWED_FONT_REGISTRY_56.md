# Borrowed existing FontRegistry profile, packet 56

`GuiTextFontRegistryAccess` now makes Text's registry access explicit. Its
borrowed-existing profile returns the caller's same live `FontRegistryStartup`
owner, with no publication cell, lifetime manager, allocation or registration.
The legacy profile retains the existing published-cell/real-manager getter.
This is an ordinary Source ownership interface, not Original singleton ABI or
fixed-global evidence. Normal native Text composition remains unactivated.

## Two required profiles

The type has no default constructor. Both factories require live lvalue owners:

```cpp
auto existing = GuiTextFontRegistryAccess::borrowed_existing(actual_registry);
auto legacy = GuiTextFontRegistryAccess::published_singleton(actual_cell, real_manager);
```

Private state admits either an existing registry pointer or the real cell and
manager. The factories only store borrowed references; they never call the
getter. `get()` returns the existing owner directly in borrowed mode, or calls
`get_font_registry_007371d0` with the same cell and manager in legacy mode.
There is no missing-service fallback, mirrored registry, copied descriptor list,
private publication, global owner or replacement lifetime manager.

The legacy getter still reads the live publication and retains its null arm's
double check under the real manager lock, allocation, publication-before-
registration, callback behavior and owner deletion contract. The profile keeps
a pointer return so the font setter's existing explicit null-result check is
preserved. It does not reinterpret the already-owned normal FontHost member
as a manager-owned heap singleton.

`existing_registry() const noexcept` is metadata only. Borrowed mode returns
the same owner; legacy mode reads the current publication cell directly. A null
legacy cell stays null. This operation never invokes the lazy getter, locks,
allocates, publishes or registers. It gives the follow-on factory/producer
admission work an identity to compare without causing native-style effects.
It is a current read, not an immutable legacy-registry promise.

## Propagation and exact setter order

`GuiTextFontNameServices::registry` holds the access value. The new four-argument
constructor accepts names, profile, actual fonts and the live one constant.
A five-argument compatibility constructor retains the previous brace/call shape
with names, publication, real manager, actual fonts and one; it merely creates
the legacy profile and does not invoke the getter early.

`GameNativeTextApplicationBindings::font_registry` replaces its publication and
manager fields. The Text application passes this profile directly into its
font-name services. Borrowed mode therefore requires no unusable legacy fields.
That unused application bindings aggregate has an explicit Source API/layout
change; callers using its old fields or precompiled layout must migrate/rebuild.
The font-name services' old five-argument calls remain accepted, while direct
access to their old fields also needs migration.

On a changed font name the setter still performs this sequence:

1. Store `text.font_name` unless the source aliases that same member.
2. Invoke `services.registry.get()` at the former singleton-getter point.
3. Keep the existing nonnull registry check.
4. Look up the **live stored** `text.font_name` in the returned owner.
5. Validate the same descriptor and actual font/image owner domain, publish the
   borrowed descriptor, read its live alpha scale or the live miss constant,
   then invalidate the cached shader in the existing order.

The post-lookup Source text, including every descriptor/resource identity check,
publication, scalar read and shader invalidation, is unchanged from Root source
`f9dd0d899d2fd6082adfee779f5a843420574847`. The shader-name setter is unchanged.
No font descriptor, FontData/image owner, font producer or factory was replaced.

## Tracked callers and lifetimes

The tracked construction is the Text application's `Impl` initializer in
`game_hosts_text.cpp`. Property services and glyph-child tail services borrow
the resulting font-name services; their setter calls are in
`gui_text_properties.cpp` and `gui_text_glyph_child_runtime.cpp`. The old direct
getter consumer was the font-name setter; access is now centralized in the
profile's legacy `get()` branch. The application bindings are passed through
the private implementation and two public application constructors.

The borrowed registry and its stable descriptors must outlive every Text,
retained callback, pending continuation and actual font/image reference that
uses them. Its real Source owner remains responsible for destruction. Actual
fonts must be retired and associations cleared before descriptor destruction,
as already required by `FontRegistryStartup`. Legacy publication storage and
the real manager retain their existing lifetimes. Profile access, publication
changes, registry mutation and owner retirement must be externally serialized.
Neither metadata inspection nor this profile freezes a mutable publication or
adds synchronization, an owner, exception recovery or a teardown mechanism.

## Verification and limits

One focused manifested MSVC Win32 `/MD /O2 /W4 /WX` case links the rebuilt
production `bsp_core.lib`. It verifies same-owner borrowed access without a cell
or manager, harmless profile copy/destruction, deferred legacy creation,
live publication/fast-path behavior, null-arm lock/allocation/registration,
publication before registration, and actual owner deletion clearing publication.
The inspector returns null/live legacy identities with no lazy effects. Compile
checks cover both font-name service constructor shapes and nondefault/trivial-
destruction profile properties. The fixture's legacy manager uses a real mutex
and owns/deletes its registered registry; no factory or production stub is added.

The final `scripts/build.ps1` exited zero and all three existing suites passed:
`reconstructed_math`, `native_math_differential` and `tool_tests`. This packet
adds no test framework, CMake source, producer/FontHost/runtime-factory edit,
Ghidra mutation, Native ledger record, installation or normal-host activation.
The focused case does not invoke full Text/FontHost composition or prove
Original singleton ABI, fixed `00F8BF44` publication, native SEH or game behavior.
Follow-on factory/producer admission belongs to packet 59.

Receipt: `reports/cc12_native_text_borrowed_font_registry_56.json`. Private probe
and build artifacts remain in `local/cc12_native_text_borrowed_font_registry_56/`.
