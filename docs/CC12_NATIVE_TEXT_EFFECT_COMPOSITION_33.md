# Native Text effect composition 33

`GameNativeTextApplication` now has a constructor that takes the existing
persistent `GameNativeGuiEffectApplication`. It binds that exact graph to the
Text application's private `GuiTextBufferServices` before any Text allocation.
This connects the existing whole `AB8530` section-material and `AB8CE0`
font-effect consumers without exposing mutable service internals or creating a
second effect application. The earlier three-argument constructor remains
available with its existing behavior.

The Source change is in `include/bsp/game_native_text_application.hpp` and
`src/game_hosts_text.cpp`. The machine-readable receipt is
[`cc12_native_text_effect_composition_33.json`](../reports/cc12_native_text_effect_composition_33.json).
The underlying caller contracts remain those in
[`GAME_NATIVE_GUI_TEXT_EFFECT_CONSUMER.md`](GAME_NATIVE_GUI_TEXT_EFFECT_CONSUMER.md).
No new Original body or original register/private-frame/string/FH3 ABI is
reconstructed by this composition.

## Admission and publication

The new constructor first completes the existing Text service/factory metadata
and domain validation. It then compares the identity of the supplied
configuration publication cell with the renderer's borrowed publication.
Mismatch throws before either side publishes a service binding.

The existing effect application's binding validates the canonical renderer,
geometry, actual owners, strings, material/parameter pools, destruction access,
mapped profile and widget/node domains. Only after those checks return does it
publish its borrowed service pointers and install its consumer in the Text
services. The constructor then records that exact consumer identity. There is
no throwing work after publication.

Binding does not construct a Text, acquire a material/effect, open a shader
cache bracket, move/reset its cursor, replace a provider, initialize platform
padding or manufacture stack/output evidence. The existing consumer still
authorizes only the exact retained `AB8530`/`AB8CE0` caller and nested name/
acquisition frames. All native factory, selection, reference and failure
schedules remain unchanged.

## Metadata lifetime

The effect application's destructor checks and clears its consumer slot inside
the borrowed services. Consequently the Text application must remain alive
until that destructor finishes, even when no Text allocations remain. The new
retention guard enforces this outstanding borrow. Destroying Text metadata
early reaches the existing terminate-on-unreconciled-lifetime policy.

Text entries compare the current consumer slot with the recorded identity.
After effect metadata clears the slot, further entries reject the closed
binding instead of falling through to the legacy effect path. This check only
compares the saved identity; it does not dereference a retired consumer. The
new member is appended to the private Source `Impl`.

The host composition must use this order:

1. Construct the real renderer/providers and one persistent effect application.
2. Construct Text with the new overload, and bind the native font producer to
   its same factory before producing borrowers.
3. Perform the recovered Text, material, font and frame work with those owners.
4. Retire all Texts and native fonts while their providers remain alive.
5. Perform the existing shared native singleton/renderer drain.
6. Close effect metadata with `after_native_renderer_drain`, then destroy that
   effect application while Text's services still exist.
7. Destroy the now-unborrowed Text metadata. Preserve both graphs on failure.

No destructor adds a second native drain, caller-credit release, rollback or
retry. An already entered failure remains nonreplayable under the existing
Text and effect application policies.

## Verification and activation boundary

The strict MSVC Win32 build and all three existing suites pass. The current
`game_hosts_text.obj` contains both constructor overloads. Its new constructor
compares cell identities before the real binding call; the successful tail
after that call contains only the consumer capture/stores and return plumbing.
The compiled retention method also checks the outstanding consumer borrow.
These are Source/build/COFF observations, not native execution of Text.

Normal host activation is still absent. Default `AB8CE0` selection needs a
valid actual signed platform height at `+28`; publication identity alone does
not establish this. The host's current 44-byte HWND/focus projection supports
`+30/+41` and cannot supply that height from opaque bytes. The complete
184-byte platform construction/deletion Source path still needs host adoption.
Persistent native fonts and widget parents, the recovered cold `guidefault`
versus font-effect cache order, buffers, passes, drawing and retirement must
then execute in one actual host composition. No native Text constructor,
consumer invocation, draw, visual parity or gameplay pass is claimed here.
