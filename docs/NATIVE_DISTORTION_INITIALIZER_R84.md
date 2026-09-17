# Native distortion initialization

Addresses: 00b4f560, 00b724e0

## Result and limits

R84 reconstructs the complete 2,664-byte B4F560 initializer, using the existing
concrete holder, post20, material, scene, camera, frame and viewport providers.
It also exposes B724E0 over caller-owned 24h scene storage so the initializer
can preserve allocation-before-name-construction order. The existing allocating
scene factory delegates to that entry and retains its caller cleanup role.

The strict MSVC Win32 build and three existing CTests pass. Runtime comparisons
cover the 582-byte capability/holder/numeric prefix, 192 x87 cases and six
in-place scene cases. **The complete initializer, post20 shader construction,
parameter registrations, camera/viewport tail, native exceptions and gameplay
were not executed.** These are new C++ interfaces, not binary replacements.
No repository tests were added.

## Entry and capability decisions

B4F560 spans [B4F560,B4FFC8), takes ECX existing26Ch owner and stacked width,
height, returns AL and RET8. The B107F0 caller's allocation/construction/init
sequence is verified at B11E40..B11EA5.

Each capability dispatch reloads F8D394 and the renderer's current +F8 slot,
supported as D5F0A8/B21EC0. It queries format111, then fallback114 only if
needed, with adapter format22, engine flags16 and texture kind3. Success writes
the chosen format at +264. Format112 is required next; success writes +268.
Direct B20160(112) supplies the post-blend flag byte+260 with zero-only HRESULT
success. Width and height then round down to multiples of eight.

Capability rejection returns before resource allocation. Rejecting both first
formats leaves +264 untouched; rejecting112 retains the previously selected
+264 but leaves +268/+260 untouched. **+34/+38/+3C remain constructor preimages
on these false returns.** The original caller releases the owner after false;
R82's failure-path safety concern remains unresolved, and no extra zero stores
are introduced here.

## Holder and numeric prefix

Two full B4E020 18h holders at +10/+14 use the selected format, followed by a
third at +18 using112. All use rounded width/height, multisample0/mode0/null
external surface. The first two publish before disarming state0; the third
disarms state1 before publication.

For each numeric pair, the code captures a height texture from current+10,
reloads current+10 for a separate width texture, invokes width+3C, spills the
width result to float32, then invokes height+40 on the captured first texture.
Unsigned dimensions use FILD signed32 plus conditional CE3978 bias. The first
pair divides D7A280 (0.5) by each dimension and stores +24/+28. The second uses
FLD1/FDIVRP and stores inverse dimensions at +2C/+30. Height also spills before
the final FLD/FSTP stores. The source retains these operations and call order.

## Full resource sequence

The initializer creates three full B4E470(count3,null) post effects and registers
nine borrowed parameter sources through the existing B17E10 path:

| Post field and name | Parameter | Owner source | DWORD count |
| --- | --- | --- | --- |
| +1C displace_damp.mshd | cSampleOffset | +24 | 4 |
| +1C | cSampleOffsets | +40 | 64 |
| +1C | cSampleWeights | +140 | 64 |
| +1C | cBumpFadeFactor | +244 | 1 |
| +20 displace_bump_to_disp.mshd | cSampleOffset | +24 | 2 |
| +20 | cTexelOffset | +2C | 2 |
| +20 | cBumpHeight | +248 | 1 |
| +20 | cRefractionIndex | +24C | 1 |
| +240 dummy_passtrough.mshd | cSceneColorSampleOffset | +24 | 2 |

The first cSampleOffset is a float4 and includes BOTH half and inverse pairs.
The other two uses of +24 borrow only a float2. Arrays+40..23F and scalar
parameters+244..24C are not populated or overwritten by this initializer.
Every parameter resolves the current post and material at its native call site.

After configuring +20, its current frame color0 receives the current +18
holder's primary surface. Then the initializer allocates24h, constructs
DistortScene and publishes its actual storage at +3C; creates40h frame+38;
allocates a camera slot through the existing pool and constructs DistortCamera
at +34. It creates a new34h viewport, sets rounded dimensions, assigns it to
the current canonical camera through B71990, then releases the creator through
the current decrement import and concrete D5E5F8 terminal. Finally it creates
+240 and registers its scene-color offset. No scene-root attachment or frame38
surface assignment is invented: neither occurs in this native initializer.

## Cleanup and host bindings

DF8810 FuncInfo declares28 unwind states with mapDF8834. Only states3/10/17/21/25
chain to2/9/16/20/24; all other next states are-1. Allocation states0/1 free
the current holder raw block, 2/9/24 free the relevant post raw block,16 scene,
19 frame,20 returns the camera pool slot through B71350, and23 frees viewport.
The intervening name states perform the corresponding mask-gated or direct
41DD20 return. Mask bits1/2/4/8 clear before normal returns; final post bit10
remains set after its normal name return, as in the original. Final parameter
state27 uses the distinct last temporary header.

The persistent block prepares three existing post-construction blocks, camera
scene/lifetime binding credits and two viewport records before entry. Published
children survive later failures; no rollback or extra native references are
added. Completed post children are preserved if a later host binding throws.
All begun returns/frees and surviving identities remain recorded for explicit
external disposition. Reset requires native survivors and parameter borrows to
be finished and child blocks reset. Scene zero-release self-deletes its companion;
the block's borrowed diagnostic publication can then be stale and is not
dereferenced by reset.

The in-place scene entry performs the previously reconstructed normal body and
member cleanup, but never frees caller raw storage on construction failure.
The allocating wrapper performs that free. Host companion allocation may throw
before weak-base entry. Native FH3/SEH, asynchronous private-slot aliases and
secondary cleanup failures remain outside the demonstrated runtime scope.

## Verification and follow-up

The collector checks3,740 selected live/PE bytes, including complete initializer
and scene bodies, dimension leaves, all28 unwind actions/maps, literals, profile
slots and the service caller. The report includes113 call rows,104 direct.

Twelve original/source prefix cases compare whole26Ch storage with only holder
pointers normalized. They cover real hardware queries, forced fallback114,
positive success accepted by B21EC0 but rejected by B20160, and three early-false
routes. Successful cases create real72x40 holders from input79x41. Controlled
COM cases restore the real factory before allocation. Existing concrete adapters
provide native callees; execution stops before the first post allocation.
All created holders are explicitly released and tracking returns to baseline;
final device/API counts are zero. The parent destructor is not invoked on
the incomplete owner, so this does not prove safety of the false-return caller.

Copied original numeric fragments match source output, x87 exception/stack bits
and control words for192 combinations of unsigned edge dimensions, three
precision modes and four rounding modes. Six separate original/source scene
cases now call the new in-place entry with explicit raw allocation, comparing
names0/12/513, full normalized24h storage and retained weak-handle invalidation.
That probe reuses R83 native bytes from the same verified PE hash; current R84
scene-constructor bytes also match. Both probes and their artifacts are sealed.

Next, assemble and execute the full post/material runtime and the camera tail,
wire process-static weak-pool lifetime, resolve the early-failure ownership
contract, and bind B107F0 resource startup. Full application/gameplay proof is
still required for the reconstruction goal.
