# Native Traceline geometry rendering

Addresses: 00AF26A0, 00AF1C20, 00B71530, 00B651E0, 00B748E0, 00B72F80,
007C1180, 007BB620, 00B6E8C0, 00415550, 00B6D810, 00B6DCB0, 006EF890,
004142E0, 00B51A20, 00BEE050.

The module reconstructs the complete normal bodies for Traceline vertex filling,
model rendering, mesh entry creation and their listed numerical dependencies.
It does **not** reconstruct actual `B1DFF0` collection, GPU draw submission, other
derived virtual implementations, stream construction or resource ownership.
Those remain required application services. No second queue, cache, scene,
model, stream or clock is created.

The 96 original-byte differential cases comprise **40 AF26A0 cases, 32 B748E0
cases and 24 B72F80 cases**. They use the real existing logical map/unmap
implementation with no physical buffer, and a recording fixture at `B1DFF0`.
They establish numerical/control-flow/mutation agreement for those fixture
paths, not complete render-domain composition or game validation.

## Bodies and ABI

All addresses and descriptive names are reconstruction hypotheses. The report
records inclusive bounds, last instruction address/length, original byte hashes
and every native call site. `EDX access` below is a new source binding; native
stack arguments retain their order. Its saved local slot is removed before
the original cleanup. No raw camera is overlaid with a semantic camera object.

| Native body | Original arguments and cleanup | Coverage |
|---|---|---|
| AF26A0..AF3168 | ECX node; context, float LOD, float visibility, flags; AF3166 `RET10`, length3 | complete normal body, EDX access; B1DFF0 dependency remains external |
| AF1C20..AF1C6E | ECX destination; source; AF1C6C `RET4`, length3; EAX destination | complete; unused EDX |
| B71530..B71582 | ECX camera; sphere; B71580 `RET4`, length3 | complete cache/classifier, EDX access |
| B651E0..B652CB | ECX plane set; sphere, initial mask; B652AF/B652C9 `RET8`, length3 | complete no-call classifier, EDX access |
| B748E0..B74B5A | ECX model; context, LOD, visibility, flags; B74B58 `RET10`, length3 | complete normal body, EDX access; other-derived virtuals external |
| B72F80..B7325A | ECX mesh; context, model, LOD, visibility, flags; B7302F/B7314C/B73258 `RET14`, length3 | complete normal body, EDX access; B1DFF0 external |
| B51A20..B51AAA | ECX entry; leading, section, geometry, model, camera, visibility, depth override, flags; B51A7A/B51AA8 `RET20`, length3 | complete actual28h entry writes/depth, EDX access |
| B6E8C0..B6E922 | ECX model; B6E922 `RET`, length1; EAX actual+13C | complete raw world-sphere cache, EDX access |
| 7C1180..7C11D7 | ECX sphere; destination, matrix; 7C11D5 `RET8`, length3; EAX destination | complete raw sphere transform, EDX access |
| 7BB620..7BB6D6 | ECX matrix; 7BB6A4/7BB6C4/7BB6D6 `RET`, length1; ST0 result | complete raw basis-length selection |
| 4142E0..414363 | ECX source; destination, matrix; 414361 `RET8`, length3; EAX destination | complete raw point transform, unused EDX |
| 415550..41558D | ECX/EDX float pointers; 41557B/41558D `RET`, length1; ST0 result | complete ordered maximum leaf |
| B6D810..B6D816 | ECX node; B6D816 `RET`, length1; EAX actual+A0 | complete notification-context pointer getter |
| B6DCB0..B6DCB3 | ECX node; B6DCB3 `RET`, length1; ST0 actual+50 | complete bounds-scalar getter |
| 6EF890..6EF894 | ECX owner; camera,float; 6EF892 `RET8`, length3; only AL=1 | complete five-byte leaf; Ghidra definition pending root |
| BEE050..BEE053 | ECX original clock; BEE053 `RET`, length1; EAX original+20 | complete existing FrameClock current-pair projection; new typed owner ABI |

`B7304A..B7304F` is the only exported instruction gap: installed/live bytes
`8D9B00000000`, a six-byte `LEA EBX,[EBX]` alignment instruction skipped by the
preceding unconditional jump. It contributes no reachable logic and no source
instruction. All other source bodies cover every reachable exported instruction.
`6EF890` was not a Ghidra function at worker inspection; the exact five live/disk
bytes are `B001C20800`. BEE050 is the defined `BSP_FrameClock_GetCurrent` leaf,
`8D4120C3`. Worker performed no Ghidra mutation.

## Native storage and behavior

The node prefix is the same producer-established `NativeNodeStorage`; its
`notification_context_a0` is a pointer, not a dedicated LOD-mode field. A null
pointer makes B748E0 compute LOD from the context+C camera; a nonnull pointer
keeps the supplied LOD. `bounds_scalar_50`, `scalar_ac`, flags5C, hierarchy
34/3C, worldF0 and world-sphere13C are read in place. The existing model tail
provides the actual geometry180 accessor.

`AF3440` and the Traceline lifetime/update packet establish payload80h at184,
the producer-allocated20-byte ring at188, head18C, count190 and frame resources.
The render body first reads context+8 camera+198. Only zero samples the freshly
loaded clock's **virtual14 current pair**, divides its signed64 ticks/frequency
with x87, and stores float32 at node198. It requires at least two ring records
and a non-AAA current model sphere classification before mapping.

The actual stream0 is loaded through B74640/B73260. The native map receives
`(2*count + optional10, 0, 0)` and returns writable bytes. Each ring record
produces two48-byte vertices: position3, direction3, raw colorDWORD, UV2, UV2,
and one scalar. The two scalar signs, ring wrap, head-based texture coordinate,
quadratic leading fade, six-record tail fade and truncate-mode FISTP alpha
remain instruction ordered. Count/request arithmetic has no new clamp.

The optional branch writes ten additional vertices, including two degenerate
strip copies, longitudinal extension and the camera-oriented flare. Source
preserves the11 FLD/FSTP words plus raw color copy at+18, all ordered aliases,
the explicit control-word save/truncate/restore sequences, x87 spills and SSE
negative-zero subtraction. Ghidra displayed `AF2E47 DC C9` as `FMUL ST1`;
the byte-directed operation is `FMUL ST1,ST0`, which keeps the half constant
live for the other coordinates. Native fixture comparison caught the reversed
destination before acceptance.

After filling, section0 receives start0 at+C/+14, primitive count at+18 and
vertex count at+10. Stream unmap executes before the four original context/LOD/
visibility/flags arguments are forwarded to B748E0.

B71530 publishes camera flags2F0 bit4 before the real view-projection getter,
frustum extraction and six-plane assignment with flags7. The signed plane
count at camera434 is preserved. B651E0 uses the existing20-byte plane records;
it spills dot product and distance separately, forms negative radius with SSE,
returns AAA only on the original ordered outside branch, or ORs bits0,2,4,...
into the supplied mask. No typed vector is applied to camera storage.

B748E0 preserves alpha gating, mode2 distance rejection, parent bounds culling,
the null-notification-context LOD formula, current virtual58, model geometry,
and child traversal/masks. AAA skips that model's mesh but still visits eligible
children. Child calls receive the original flags and LOD, with visibility
multiplied by the parent scalar. Only distance/alpha early returns skip children.

B72F80 preserves optional transformed mesh-sphere culling; signed inclusive
LOD section-index bounds; range matching; all-section traversal; primitive-count
gates; and the linked section+38 chain. Each entry comes from the same current
0108FE88 cache: increment actual+8 atomically, then use actual+4 backing plus
`(incremented_count-1)*28h`. There is no allocation/capacity repair. B51A20
initializes the first20h bytes, leaves the sort key20/24 intact, and computes
depth through the actual view matrix and world sphere if the override does not
exceed the original zero. Collection receives context+10 and that exact row.

## Required services and reused implementations

| Calls | Concrete contract |
|---|---|
| AF26D0, clock virtual14 | Required callback on the freshly loaded SAME canonical FrameClock and existing SystemTimeTimerVirtuals. The actual D68D50+14 target is BEE050, returning `&clock.current`; no raw clock overlay or double-time substitute. |
| AF26F3, B749E9, B74A34, B51A8F, model virtual48 | Current profile lookup; B6E8C0 dispatches the complete raw getter. Other-derived targets remain `call_virtual48`, contract unread beyond their exact owner/no-stack ABI. |
| AF2769 / AF312B, stream virtual10/14 | Required current logical profile B49980/B49A80, using the SAME NativeLogicalBufferMappingContext and actual stream. Unsupported profile terminates; it cannot return successful empty geometry. |
| B74AC6, virtual58 | Actual Traceline D0C928+58 is 6EF890, a verified AL1/RET8 body. Other targets remain required `call_virtual58(owner,target,camera,lod)`, contract unread. |
| B74B48, child virtual20 | Current target AF26A0/B748E0 dispatches the concrete body with this access. Other targets require `call_virtual20(owner,target,context,lod,visibility,flags)`, contract unread. |
| B730BF/B73121/B731CE/B73230 -> B1DFF0 | Actual command owner ECX=context+10; one28h entry pointer, RET4. Body read: visibility fading, material/batch routing, binding/group retention, allocation and strings. Those actual operations are REQUIRED externally; the older semantic grouping projection is insufficient. |
| Direct existing dependencies | Raw B6DB70 world refresh, B6FCB0 view, B70490 VP, B653F0 extraction, B658E0 assignment, 419440/419510 CRT-backed length/normalize, 4155B0 clamp, B49980/B49A80 logical mapping, and the existing actual geometry/stream/section getters. |

Profile lookup must be a nonmutating lookup of the current native table in the
application's canonical owner domain. Returned numeric slots are compared, not
called as host addresses. All reached storage, current cache backing, referenced
payload/frame resources and nonthrowing services must remain valid. Native
hardware faults, asynchronous changes, arbitrary derived tables and allocation/
exception cleanup across external collection are not validated here.

## Verification

Live Ghidra and installed executable bytes matched for18 complete spans (the16
owned bodies plus existing length/normalize) and six global constants. The PE
relocation directory is absent; the scratch loader relocates explicit absolute
data operands and external calls, retaining original branch bytes and bodies.

Strict MSVC Win32 `/MD /W4 /WX /O2 /fp:strict` compile and the96 differential
cases passed for control words007F,027F,037F,0E7F. Cases include ring wrap/full
capacity, both flare alpha branches, flare suppression, camera/count/outside
gates, mesh culling, all-section and inclusive LOD ranges, empty sections, section
chains, alpha/distance exits, supplied/computed LOD and child forwarding. Raw
node/vertex/section/cache/entry bytes, x87 status/control, MXCSR and recorded
child/collection arguments agree. Actual dependencies shared by both runs are
explicit fixture boundaries; no D3D physical stream or B1DFF0 body was executed.

`scripts/build.ps1` completed and the existing reconstructed_math check passed
1/1. That standard build precedes root CMake registration of this new source;
the strict probe separately compiled this module. Scratch artifacts are under
`C:/Users/sqz269/bsp-at-traceline-render`. The parent replay uses
`build-probe.ps1 -CurrentRoot`, which compiles **probe.cpp only** and links the
current integration libraries. No permanent tests were added. No executable
render-flow wiring, screenshot, actual draw or game validation is claimed.
