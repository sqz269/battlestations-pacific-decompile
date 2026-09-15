# Native render-resource surface factories (CU)

CU implements the actual color/depth surface factories and render-resource depth
replacement through the existing concrete surface pool, owners and device
recreation source. It also completes normal build registration of CT's remap
module. Both modules pass the tracked Win32 build and both existing CTests.

## Native evidence

| Body | Bytes | Original contract |
| --- | ---: | --- |
| B0FC10 | 104 | ECX service; width,height,unused DWORD; RET0Ch |
| B21210 | 159 | ECX format; EAX accounting classification; RET |
| B22850 | 95 | ECX three-word header; signed requested capacity; RET4 |
| B2A7C0 | 477 | ECX renderer; width,height,format,multisample; EAX creator; RET10h |
| B2A9A0 | 526 | ECX renderer; width,height,format,multisample,float quality,low-byte discard; EAX creator; RET18h |

Five bodies total1361 bytes. Six compiler support spans add52 bytes; all454
instruction owners and live/installed-PE bytes match. CT's existing six remap
bodies567B/206 instructions were reverified separately. They add no new source
credit in CU. The PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

B2A9A0 had no saved function. The original D5F0A8 slot94 points to it, and its
complete original body was defined under the write lock. The two missing
ten-byte handlers CBD3D0/CBD3F0 were also defined and saved. Existing bodies
required no repair. All prior names/comments are recorded and preserved; no
executable bytes or global no-return policy changed. The source/interface names
remain descriptive hypotheses.

## Recovered behavior

B2A7C0 uses the current device's CreateRenderTarget with quality0 and lockable0.
B2A9A0 uses CreateDepthStencilSurface, converting float quality through original
x87 truncating FISTP64 and taking the low32 bits; discard is the zero-extended
input low byte. Both retry only when the current output is null and the first
HRESULT is nonzero and neither8876017C nor8007000E. A reached retry calls the
existing complete B29670 with its required concrete context, reloads the device
and retries without clearing the original output cell. There is no successful
recreation stand-in or null-COM fallback.

The factory allocates through the already bound actual static surface pool and
calls B3F630 with flags10/kind0 for color or flags100/kind1 for depth. The returned
creator is appended as a borrowed pointer to renderer+1B0C/+1B10/+1B14. Color
appends before releasing the current COM output and then accounts bytes. Depth
accounts first, then appends, then releases the current COM output. Neither
append adds a reference. A failed constructor's raw slot is returned by native
state1; state0 cleans only the optional guard. Completed resources and COM
outputs are not rolled back by added host cleanup.

B22850 preserves the signed minimum/capacity checks, wrapping allocation size,
live count reads during copy, old-array free before pointer/capacity publication,
and unchanged count. It does not zero gaps or retain copied pointers.

Accounting preserves the original format classification and x87 sequence:
integer bits shifted by3 and spilled as float, wrapping width*height, signed
FILD plus original unsigned corrections, current global-counter sign capture
followed by its second read, accumulation and truncating FISTP64 low32 result.
The original control word is restored. Color disarms its native guard and
captures the exit mode before the final conversion/store; depth does this only
after its later append/COM release. MSVC inline x87 avoids replacing this with
SSE arithmetic or an integer byte formula.

B21210's original126-byte switch data is translated without correcting its
values to nominal format sizes. Some inputs return64 and DXT1 returns4; the
factory subsequently shifts the result by3. These are the observed game rules.

B0FC10 consumes three arguments even though it uses only width and height.
Null service+1C8 skips work. Otherwise it clears the captured frame's depth,
captures current renderer slot94, creates format4B/multisample0/quality+0/discard1,
rereads current service+1C8, performs retained depth assignment and drops the
captured creator through current CE2220 and the actual surface terminal.

## Validation

The normal tracked build includes both new modules once. A short registry lease
was claimed for the two-line append and released immediately before building.
No CMake override or foreign lease bypass was used. The earlier CT report/seal
remain historical evidence of its then-pending registration; CU supersedes that
integration status while retaining its execution limits.

The strict Win32 current-library probe creates actual D3D9 32x16 color and depth
surfaces. It verifies flags/kinds/counts, fractional quality0.75 truncating to0,
array growth, and accounting starting at80000010. Native frame-depth replacement
then creates a64x32 surface and retires the old depth. Real surface deletion
removes borrowed registry entries and returns actual pool slots. The fixture
does not create a fake surface, private pool or replacement reference counter.

One focused comparison executes a private copy of the original B21210 machine
code and switch bytes, relocating only its two absolute table operands and
seven internal target words. All98 compact-switch entries,10 FourCC inputs and
five invalid inputs match the source:113 comparisons. Original disk/Ghidra bytes
remain untouched. Factory execution itself is a source composition fixture,
not an original-factory machine-code differential.

CT's actual remap texture tests and CS's complete constructor/three mesh parser
fixture also pass through the current tracked libraries. All64 canonical
mesh/texture companions retire; the separate camera, frame and surface domains
are explicitly retired and native managers drain.

Device-recreation retry, allocator/constructor exceptions, enabled optional
synchronization, nonzero post-effect/material reset, cube/volume and cold texture
loading remain unexecuted. Unmasked FPU exceptions, native caller/FH3/SEH identity,
concurrency and gameplay equivalence are unproven. No CU application run or
native renderer/service startup wiring is claimed. Full parent initialization
and destruction still require their actual producer/consumer ownership closure.

The CU source, flow, annotation and integration reports retain the exact native
spans, previous analysis values, call checks, source manifests and artifact hashes.
