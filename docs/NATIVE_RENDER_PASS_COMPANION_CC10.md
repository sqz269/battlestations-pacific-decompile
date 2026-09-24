# Canonical render-pass companion

`NativeRenderPassReference` connects a completed service-visible native pass
to the application's existing `NativeRenderActualOwnerRegistry`. This is host
lifetime metadata, not another reconstructed native entry. It borrows the
actual atomic at +4 and binds without changing any owner bytes or count.
Registration uses the same registry as the pass's canonical post-effect
children. Duplicate, invalid-profile and nonlive-count bindings fail before
native effects; registry allocation failure leaves the owner caller-owned.

Final release checks current virtual slot zero for `BD30E0`, reloads the
current profile for its deleting slot, and dispatches one of the existing
complete source destructors below with flags 1. The companion unbinds only
after native destruction/free returns. It performs no second decrement and
does not access native storage after deletion. The caller keeps the companion
and context alive until retirement and external quiescence, then destroys the
companion. Unexpected terminal dispatch or an escaping destruction exception
terminates at the existing nonthrowing `RenderCommandReference` interface.

| Current profile | Deleting slot | Source lifetime |
| --- | --- | --- |
| D5E164 | B10120 | Depth downscale |
| D5E178 | B10140 | Particle blend |
| D5E18C | B10160 | Downscale 4x4 |
| D5E1A0 | B10180 | Downscale 2x2 |
| D5E1B4 | B101A0 | Bright pass |
| D61FE0 | B50FE0 | Luminance |
| D62150 | B54F70 | Bloom |

All seven current profile pairs were checked against live Ghidra and the
installed PE. The existing source lifetimes remain authoritative for native
child release, pool returns and exception cleanup. Binding requires storage
valid for its current admitted profile and native synchronization; it does
not make arbitrary profile replacement or concurrent destruction safe.

One ignored local fixture reused the established R76 actual-domain setup and
created a real D3D holder and a canonical post-effect/frame child beneath a
224h bright-pass fixture. It verified unchanged raw bytes/count at binding,
the exact +4 atomic alias, duplicate rejection, preservation on nonfinal
release, and final canonical deletion. Both companions retired, the registry
emptied, texture/surface tracking returned to its initial values, and final
D3D device/API reference counts were zero. This was a source lifetime check;
it did not execute the original bright wrapper or full pass initializer.
The strict MSVC Win32 build and all three existing CTests also passed.

This companion intentionally applies to service-visible pass identities.
Nested holders, textures and surfaces retain their existing direct lifetime
paths. Registering all of those nested identities would require their direct
deletion paths to retire the corresponding metadata too. No such blanket
registration, completed `B107F0` admission, application activation binding,
native ABI/FH3/SEH equivalence or gameplay claim is made here. Full graph
initialization and remaining service-visible holder/surface registration are
still separate dependencies. Evidence is in
`reports/native_render_pass_companion_cc10.json`.
