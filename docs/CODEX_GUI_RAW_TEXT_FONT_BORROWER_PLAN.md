# Raw Text Font borrower linkage

Read-only proposal pinned to accepted `f2fb3c1bdfd5c2a3f2cd15898b770f24a7c53ea4`.
Only this document changes. No source/CMake/config/ledger, build, game, probe or
Ghidra change; no new Font/native ownership or nonempty retirement acceptance.

## Current source boundary

GameFontHost::native_fonts (game_hosts_fonts.cpp127) exports once and enters
sticky export_pending BEFORE producer export. Its sole current completion is
bind_text_factory145: admitted first bind becomes bind_entered before identity
checks, requires SAME resources and empty actual factory, and latches failure.
native_operation_interrupted189 retains unresolved export/bind/retirement.

GameNativeFontApplication::native_fonts221 marks fonts_exported. Its binder226
requires exact resources AND typed/native registry authority, and stores the
actual GuiTextRuntimeFactory. retire_fonts235 refuses exported-but-unbound
resources or surviving factory allocations BEFORE native Font cleanup. Keep
that legacy route unchanged; an unrelated empty legacy factory cannot authorize
the independent raw Text graph.

The accepted raw DEFAULT owner has NO Font borrow. It can be healthy and bound
while requires_process_retention()==false (owner.cpp269). That predicate reports
interruption, not borrower retirement. custody_settled253 requires completed
Text scalar/physical return/disposal and retired/physically returned internal188
Model; it covers neither future primary4C nor glyph/cursor/content operations.
retire_before_native_drain339 admits only the private DEFAULT graph; its terminal
retired state plus !retain and custody_settled is the present pre-parent proof.
after_native_renderer_drain368 checks that proof, verifies the renderer FIRST,
closes effect/sampler facades, then marks closed. Keep the entire graph until
then, even after Text companion disposal.

AB7700 main-geometry preparation is Font-independent (coordinated with its
worker). Do not export/bind Fonts merely to implement that operation or DEFAULT.
The existing Font field body takes LEGACY NativeGuiTextReference
(native_gui_text_field_storage.hpp); no raw-Model overload is presently admitted.
AB8C30 writes actual Font image to Text108 without a Font retain (cpp235-249),
then reads actual image+1C alpha. AB8CE0 also needs that real image. AB8250 does
not release/clear108: null bits or a post-destruction read are not quiescence.

## Proposed concrete metadata link

Add a separate strongly typed raw-owner arm; no std::function, user callback,
erased pointer/bool predicate, fake legacy owner or native credit. Proposed API:

```cpp
// GameFontHost and GameNativeFontApplication, alongside existing legacy bind:
void bind_raw_text_owner(GameNativeRawDefaultTextOwner&);
void detach_raw_text_owner_after_parent_drain(GameNativeRawDefaultTextOwner&);

// Private producer-facing methods on the future Font-capable raw owner:
void bind_font_borrower(GameNativeFontApplication&, NativeFontResourceOwners&,
    void* actual_registry_image, GameNativeRendererApplication&, GameVfsHost&);
void require_font_retirement(const GameNativeFontApplication&,
    const NativeFontResourceOwners&, const void* captured_registry_image) const;
void detach_font_borrower_after_parent_drain(const GameNativeFontApplication&,
    const NativeFontResourceOwners&);
```

Font application stores one stable raw-owner pointer and the admitted registry
image as metadata, mutually exclusive with text_factory. FontHost distinguishes
bound_legacy / bound_raw / detached_raw while retaining export_pending and
bind_entered interruption states. The raw owner stores exact producer/resources/
registry-image identities and a one-shot bound/detached link state. Its ordinary
failure latch retains any entered/rejected link; no reset/rebind is offered.

Install the nonmoving raw owner in the host BEFORE Font export. After complete
image Font production and ready renderer metadata, export with native_fonts(),
then call FontHost.bind_raw_text_owner(owner) immediately. The host marks
bind_entered first; the producer checks ready/exported, no legacy/raw binding,
nonnull ready native registry image, and SAME renderer/VFS/resource collection.
It calls the owner's metadata binder, then stores the raw pointer nonthrowingly;
only complete success closes the host export gap. Any failed identity/admission
attempt latches host native_failed and preserves all providers, including an
exported sampler facade. No native Font lookup/getter, Text allocation, retain,
field store or callback occurs in binding.

The owner requires its own prepared/quiescent, pre-Font-entry state and compares
the producer-supplied renderer/VFS against its retained actual services. Bind
the exact ready NativeFontResourceOwners/native registry image, not a copied
registry or matching descriptor names. Later raw Font operations must require
this completed link, use those SAME objects and admit actual selected Font
images against the same actual-owner registry. No borrowed native Text pointer
or Font ownership credit escapes this private owner.

## Retirement and explicit unlink

Before any Font retirement effect, the new producer arm calls the exact stored
owner.require_font_retirement(...). This is a direct method on the retained
object, not a replaceable bool callback. Before this call the producer verifies
its STILL-LIVE current native registry image equals the admitted image; no lazy
lookup, retired-image read or equal-name substitution. The owner must require:

* matching link/producer/resources/captured image identities;
* terminal retired owner phase, no failure/active/entered/poisoned provider;
* settled actual Text scalar, late flags, physical-slot and disposal records;
* settled ALL Font-using main/internal Models, materials, glyph/cursor/content
  calls and surviving native credits, including flags0 physical custody.

The last item needs the wider raw retirement packet's real diagnostics. Current
DEFAULT custody_settled is insufficient once that graph expands. No count0,
retired companion, absence of failure, unrelated factory, cleared108 or guessed
second release substitutes. Guard failures retain the whole graph. Pending Font
export/bind, failed loads or retirement also keep the existing sticky FontHost
interruption policy; nothing cancels the export gap or retries native cleanup.

Preserve existing host ordering: game_hosts.cpp2025 requires frontend borrowers
already ended, invokes retire_native_gui_text_before_fonts2034, then fonts.retire_native_fonts.
The latter must perform the new raw-owner guard before producer retirement.
Producer retirement frees actual Font images/registry; retain their C++ owners,
descriptors, raw borrower record and all renderer/VFS/string/input/data services
through shared parent drain. Do not revalidate a freed registry/image afterward.

close_native_gui_text_after_drain (GUI application.cpp801) currently closes and
immediately resets raw owner. Insert FontHost.detach_raw_text_owner_after_parent_drain
AFTER owner.after_native_renderer_drain() and BEFORE that reset. Its producer
arm requires successful Font retirement plus exact link and closed/quiescent
owner metadata; owner detach verifies closed parent-drain completion and clears
the link, then producer/host clear their raw-owner pointer. No old Font/Text
payload read is permitted. This makes the borrowed callable's entire lifetime
explicit; FontHost survives until game_hosts.cpp2197. A detach failure retains
all metadata and exits through the existing host closure catch.

Current DEFAULT and AB7700-only paths remain unexported and need no raw Font
authorization. Prerequisites for USING this link are genuine raw AB8C30/AB8CE0
transport/admission, actual lookup/image domain, widened all-operation custody
tracking and nonempty raw Text retirement. Workers retain those disjoint source
surfaces. This plan supplies no glyph/content/Page or repeated-lifecycle proof.
