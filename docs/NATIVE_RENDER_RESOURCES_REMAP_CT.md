# Native render-resource remap textures (CT)

CT reconstructs the four remap-texture setters used by scene loading and
front-end previews, their reset routine, and its raw material accessor. It uses
the existing actual texture cache, canonical owner lifetimes and complete raw
material texture assignment. These functions are source implementations; the
application scene host still requires native renderer/service composition.

## Native bodies and ordering

| Address | Bytes | Behavior |
| --- | ---: | --- |
| `00B0FD70` | 77 | Replace service+66C |
| `00B0FDC0` | 77 | Replace service+670 |
| `00B0FE10` | 77 | Replace service+674 |
| `00B0FE60` | 77 | Replace service+678 |
| `00B0FEB0` | 255 | Clear four material slot2 bindings, then four remap references |
| `00B4CBA0` | 4 | Return current raw owner+14 |

The setters take native ECX=service and one stacked actual8h name header, return
the loader result in EAX, and use RET4. Each captures its old field first. If
nonnull, it reads the current CE2220 target, decrements actual+4, calls the
current canonical terminal only at zero, and clears the field after returned
release. It then captures current F8D394 and renderer slot64, invokes the
existing complete B319B0(name,0), and publishes its exact return without adding
a reference. Equal old/new identities still release and load again.

Reset takes ECX=service, no stacked arguments and RET; no semantic EAX is
established. It rereads owners in order65C,658,654,650. Each nonnull owner yields
its material through B4CBA0; complete unchecked B189F0 receives that actual
material, slot2 and null. It does not clear slot3 or release those owners.
After the material calls, reset captures service+66C **before** one CE2220
target read. That single target serves releases66C,670,674,678. Each later
field is read after the previous callback; each nonnull field clears only after
returned release. There is no rollback, resource substitution or safe-null
material fallback. Noise+67C is untouched.

B4CBA0 has no retain or stack-argument consumption. Its original instructions
are MOV EAX,[ECX+14] and RET. The descriptive material name reflects the reviewed
post-effect callers and is a hypothesis, not a recovered symbol.

All six bodies (567 bytes), 206 instruction owners, eight direct calls and
20 indirect calls were audited against the existing saved Ghidra program and
installed PE. The PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
No listing repair or binary change was needed. The indirect source dispatch is
reviewed separately from the mechanical direct-call checker.

## Source contracts

The context borrows the same cache/actual owner domain, current CE2220 cell,
renderer publication and actual D5F0A8 profile. Existing texture companions
dispatch current2D/cube/volume terminal profiles; the remap layer does not force
2D or reinterpret a numerical game address as executable host code.

Each setter retains a fresh cache operation and diagnostic phase/old/result
identities. A failed call is not replayable. Cache children survive until their
existing native obligations are explicitly resolved. Admission checks select
the current renderer source at the native call boundary, after old release;
they are host diagnostics rather than native exception behavior.

The reset's nonzero post-effect fields require actual live owner/material
storage. The current implementation does not invent their constructors or
claim full service initialization and destruction. Original machine ABI,
unrestricted FH3/SEH and canonical noexcept behavior remain distinct.

## Validation and integration status

The new source is compiled with MSVC Win32 `/O2 /MD /fp:strict /W4 /WX` and linked
with the current three libraries into an executable with `/MANIFEST:EMBED`.
The tracked base build and both existing CTests pass. At this checkpoint,
`cmake/startup.cmake` is leased by another orchestrator: registration of the new
module in bsp_core is pending, and its strict standalone compile is the evidence
for this source. No shared registry lease was bypassed or CMake override added.

One extension to the CS constructor/mesh probe uses four actual native D3D9
cache entries. It executes all setters, equal-identity replacement, and an
unsupported-renderer admission after a real old-reference release. The failure
leaves the field zero and does not enter the cache; no old-reference restoration
is asserted. This is not a native loader-exception test.

After dropping the four independent creator references, reset drives their
actual counts to zero and dispatches the real cache/texture destruction paths.
Instrumented IAT wrappers delegate to real InterlockedDecrement: the first reset
decrement changes the current IAT cell, and all four releases still use the
already captured target. All four remap fields clear, all four texture companions
retire, and the existing constructor/three-parser fixture finishes with all64
canonical mesh/texture companions retired and its native managers drained.

The four nonnull post-effect/material reset arms are reviewed and compiled but
unexecuted. No fake post-effect factory or successful material callback is used.
Cube/volume execution, cold texture/archive loading, shader compilation, original
machine-code differential execution, native application wiring, full parent
destruction and gameplay remain outstanding.

Evidence is recorded in `reports/native_render_resources_remap_ct.json`, its
annotation/integration reports and immutable local receipts. Earlier source
contracts remain in the CS report and the original material/cache modules.
