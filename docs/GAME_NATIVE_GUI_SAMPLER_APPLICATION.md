# Canonical GUI sampler parent

`GameNativeGuiSamplerApplication` supplies the actual sampler parent for
`GameNativeGuiEffectApplication`. It borrows the existing singleton manager,
renderer, VFS, input/online/XLive services and original numeric factory profiles.
No manager, sampler loader, registry publication or texture/material wrapper is
created to satisfy a missing service.

## Existing owners and publications

The constructor takes the running `GameSingletonHost`, renderer, VFS, input
runtime, original data and caller-qualified platform/MSG bindings. F8D41C is the
host's already-bound resource registry cell. F8D420 is the same actual
`NativeParticleClockStorage` used by the sampler loader, with the renderer's
existing native shutdown domain. AA0 remains the canonical singleton manager.
Domain checks reject a different string, date, pass or retirement graph.

`effect_bindings()` provides the real root/mode domains and retained invocation
factory. F8BBF0 is borrowed from
`GameNativeRendererScalarProcess::shadow_target_publication_00f8bbf0()`. It is the
actual shadow target whose policy byte +0E B45EE0 reads. The earlier descriptive
`material_manager` field name is a hypothesis, not permission to create another
owner. A genuine initialized target remains a parent prerequisite when that
branch is reached; null is not repaired with a policy default.

The sampler cache uses the real resource-registry lookup and VFS date services,
and its entry calls the existing native platform message pump through
`GameInputRuntime::platform_load_context()`. The caller must supply a genuine
actual platform publication and MSG preimage. The component does not fill unknown
MSG bytes with zeros or install a synthetic message service.

## Root and mode caller lifetime

The exact caller assembly, independently revalidated by the specialist, identifies
both root B3C018 and mode B3C024 B3B280 calls as using W=P-30h, where P is steady
ESP at B3BFF9. Their intervening call setup does not overwrite W. The root entry
has no universal statically initialized value. See
`docs/NATIVE_GUI_SAMPLER_ENTRY_RECOVERY.md` and its report for the pinned listing
and executable-byte evidence; this packet does not perform new register recovery.

The native compiler frame now owns one `NativeMaterialDescriptorSamplersCallScope`
before entering root B3B280. Its distinct root/mode contexts alias a single word
and `NativeMaterialDescriptorSamplerStackKnowledge`. The word is intentionally
unwritten and knowledge false on an uncaptured fresh invocation. The compiler
checks that both contexts share the same word and knowledge. Mode receives root's
terminal knowledge unchanged, including an empty root or uncaptured Source1
release. Each independent compiler call receives a new scope.

The established B3B280 helper is responsible for real writes: Source3 establishes
its sampler address, state setters establish their return word, and uncaptured
Source1 release invalidates knowledge. Source0/B5F100 consumes unknown entry data
only by throwing its existing explicit Source boundary. Neither a historical
captured word nor another invocation's final word becomes a default.

An optional evidence callback receives the same actual builder/pass/root/mode
identities and may supply that invocation's captured root entry and separate
root/mode release arrays. The caller retains those arrays through any failure.
The callback is an evidence source, not a shader/texture provider. No callback
means unknown entry and uncaptured releases. Existing qualified legacy compiler
contexts remain available when the new factory is absent.

Both sampler operations are declared after their owned scope in the compiler
frame, so operation teardown precedes scope destruction. Failed compiler frames
keep the scope and its parent alive. Host live-scope bookkeeping guards parent
destruction; it is not a second native reference count.

## Real procedural creator admission

The source sampler cache now has an optional pure metadata admission callback
immediately after its genuine B1B810 creator returns and before native cache
publication or caller retain. The existing returned resource is first recorded
in the cache frame. A registration exception preserves that actual creator and
the failed frame; it never releases or fabricates a replacement.

This parent registers supported BBC6F0/BBC810 procedural allocations through
the established `NativeProceduralResourceReference` and lifetime functions.
Companions borrow the existing raw +04 atomic and are admitted to the renderer's
same `NativeRenderActualOwnerRegistry`. No raw count is initialized or incremented
by admission. Current-profile scalar deletion performs the real original
destruction/free, then unbinds and retires the companion. The parent's creator
list owns companion metadata only; canonical lookup stays in the renderer.

## Integration and closure

Construct the sampler parent before the effect graph and use its
`effect_bindings()` directly. Retain both, the real input/message services,
caller MSG/evidence and original data through all entered failures and the shared
native drain. The application's existing process-retention policy must preserve
the complete failed heap graph; this component never rolls back native work.

After renderer and singleton retirement, close the effect graph and this parent
with their `after_native_renderer_drain()` checks. The parent requires no live
scopes/creator companions and null real F8D420/F8D41C publications. Exported
bindings prevent early parent destruction even before a sampler call is entered.
No extra cache clear, reference release or singleton deletion order is added.

Source base: Main `7a51a2f17`, including the pinned specialist evidence
`03193f1b8`. New parent and changed compiler/cache/effect sources
strict-compile with MSVC Win32 C++17 `/W4 /WX /EHsc /permissive- /O2 /MD /fp:strict`.
The standard registered-source build passes CTest 2/2 (12.00 seconds), including
the permanently registered effect producer and compiler/cache changes. The new
parent TU needs primary-owned CMake registration and combined linkage.

No actual effect or native Text run is claimed by this packet. The next focused
resource probe must supply the real caller MSG and shadow-target policy owner,
then acquire original `GuiFont.mshd` through this effect graph and observe the
actual descriptor/compiler/sampler result or retained failure. A reached unknown
root residue is meaningful evidence requiring caller qualification, not a reason
to insert a universal word. Actual frontend GUI/widget/material attachment,
native Text draw, world rendering, gameplay and original FH3/register ABI remain
unproved. The independent live native-font producer PASS remains resource and
retirement evidence only.
