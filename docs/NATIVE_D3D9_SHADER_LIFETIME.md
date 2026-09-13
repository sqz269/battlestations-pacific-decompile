# Actual D3D9 shader wrapper lifetime

Addresses: 00b5f410, 00b5f490, 00b5f6e0, 00b5f700, 00b268a0, 00b268c0

These six complete normal bodies consume the same actual 10h shader wrapper
constructed by `native_d3d9_shader_construction`. They preserve its raw +04
reference count, actual +08 COM interface and opaque +0C word. The C++
interfaces and host companions are not original binary/FH3 replacements.
Descriptive names are hypotheses, not recovered symbols.

| Routine | Original ABI | Inclusive body |
| --- | --- | --- |
| B5F410 pixel destruction | ECX owner; RET | B5F410..B5F480, 113 bytes |
| B5F490 vertex destruction | ECX owner; RET | B5F490..B5F500, 113 bytes |
| B5F6E0 pixel scalar deletion | ECX owner; stack flags; EAX original pointer; RET4 | B5F6E0..B5F6FD, 30 bytes |
| B5F700 vertex scalar deletion | Same | B5F700..B5F71D, 30 bytes |
| B268A0 vertex unregister | ECX renderer; stack wrapper; AL found; RET4 | B268A0..B268B2, 19 bytes |
| B268C0 pixel unregister | Same | B268C0..B268D2, 19 bytes |

The 324-byte normal bodies contain 94 instructions. All six complete listings
were reviewed in the saved `C:/Users/sqz269/bsp.gpr` program
`/battlestationspacific.exe`. The original scalar wrappers' post-free gaps
were repaired and re-exported before this implementation. Twelve direct
CALL sites and two current COM-table calls establish the dependency order.

B268A0/B268C0 take the address of their stacked wrapper value and forward it
to the existing actual B253E0/B25450 first-match swap-removal helpers at
renderer+1AC4/+1AD0. They neither retain nor release registry elements. The
B32410 renderer producer and B289A0/B289F0 registration bodies establish
these actual 0Ch headers; this module creates no renderer or registry.

Each destructor publishes its D62A60/D62A70 profile first. Pixel B5F410 then
calls the actual resource-support singleton B3E730, reads the current F8D394
renderer and unregisters. Vertex B5F490 reads/unregisters through the current
renderer before B3E730. The context borrows the same support publication,
application lifetime domain and renderer publication used during construction.
The order is retained across callbacks that change the current renderer or
owner fields.

Only afterward does either destructor read current owner+08. A nonnull COM
identity is captured, its current table+08 Release entry is called as stdcall
with that COM identity on the stack, and owner+08 is cleared after return.
There is no extra AddRef or release of a replacement stored during Release.
The destructor does not touch +04 or +0C. Normal completion calls the existing
seven-byte BD30F0 function to publish CEB130.

The one-state FH3 maps independently establish base cleanup on an exception:
CC1168 selects DF9E88, whose state0 entry DF9E80 selects CC1160; the latter
loads the saved owner and jumps through B5E720 to BD30F0. The vertex chain is
CC1188/DF9EB4/DF9EAC/CC1180/B5E7E0/BD30F0. Source dependency exceptions run
that established base-cleanup effect and preserve unfinished COM/registration
work in a one-shot diagnostic frame. No original FH3 encoding is supplied,
and no interrupted COM release is retried. Explicit diagnostic acknowledgement
retires metadata only after the caller resolves the failed native state.

The scalar wrappers call full destruction before inspecting flags bit0. That
bit selects BF65AC through the existing shared actual allocator boundary.
They return the original address even after it has been freed; it is then
identity-only data. Failed destruction does not execute the scalar free.

`NativeD3d9ShaderReference` supplies the concrete terminal provider required
by the actual pass-slot setters. Its base `RenderCommandReference` borrows
the same raw+04 atomic, without count initialization or an extra retain. The
caller binds exactly one companion in the same `NativeRenderActualOwners`
domain. Only after the existing release helper decrements +04 to zero does
lookup dispatch the terminal method.

The terminal reads the current native profile and requires its virtual0 to
be BD30E0. It then uses the existing BD30E0 helper, which captures the current
profile again and supplies flags1 to current virtual4. The two verified
scalar entries B5F6E0/B5F700 dispatch the concrete bodies above. Unsupported
profiles/code words have no successful fallback. After native destruction
and free, the caller's nonthrowing retirement callback removes the canonical
binding and may dispose the companion. No owner/companion access follows it.
The original allocator, COM providers, support domain and current table views
must remain valid; a terminal callback must not throw.

The default registered MSVC Win32 build and both existing CTests passed.
Focused original/source and composed lifetime validation is recorded in
`reports/native_d3d9_shader_lifetime.json`; its exact fixture scope remains
separate from construction-only and ABI-boundary worker fixtures. Full compiler
continuation, shader draw/readback, original FH3 and gameplay are not established
by this source implementation or by compilation.

## Focused lifetime composition evidence

Eight original/source destructor/scalar comparisons passed on actual
B5F9B0/B5FAF0-produced wrappers with real HAL shader COM objects, actual
support/string domains and the original renderer-header producer fragment.
They cover flags0/1/100, returned identity after free, preserved04/opaque0C
without free, swap-last unregister and current renderer/COM replacement.

Both actual pass-slot setters then exercised concrete canonical
`NativeD3d9ShaderReference` destruction, free and companion retirement. The
same raw+04 atomic and prior new-slot publication were observed; actual
B5F720 pass/state cleanup completed afterward. A source-only actual support
registration exception establishes the pinned state0 BD30F0 cleanup effect,
preserved COM/native writes, pixel/vertex unregister timing, no scalar free,
replay rejection, explicit diagnostic cleanup and failed-frame guard exit77.

All wrapper COM acquisitions, canonical companions, registry allocations and
pool/support domains were cleaned. Four process-baseline HAL COM holds and
the device remain until exit. The evidence pins23 disk/live spans,65 transitive
source/header inputs and21 linked core members;12 numeric calls pass the live
verifier. Two COM calls remain explicit indirect SDK boundaries. This extends
the earlier construction-only arena fixture; it does not retroactively turn
that earlier fixture into a terminal-lifetime test.
