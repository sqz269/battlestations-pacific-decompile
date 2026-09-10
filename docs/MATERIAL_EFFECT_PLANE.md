# Material effect clip plane

`construct_and_append_effect_plane_00b448a3` reconstructs the selected plane
branch of `00B44750`, from `00B448A3` through `00B44A53`. The existing dispatcher
owns the effect-change, extra-plane-byte and positive-distance gates. This helper
performs the remaining owner reads, plane construction and append on the renderer
captured at `00B4485A`. It does not read the live renderer slot again. The next
native reload, for vertex layout, is `00B44A62`.

The companion method `restore_material_effect_planes_00b25080` resolves the same
captured renderer's `D3D9CameraFrameAccess` and invokes its recovered restore.
Both helpers can directly implement the corresponding required
`MaterialEntryOperations` methods. They need `D3D9CameraFrameAccess::cache()` for
an explicit owner-identity check; the primary integrator owns that shared accessor
and source registration. This packet changes four new files only.

## Actual owner reads

| Native point | Owner and operation |
| --- | --- |
| `00B4487E` | Capture entry `+10` camera in EBX, before the plane gate and singleton operation. |
| `00B448A3` | Call actual queue singleton getter `004C11F0`. Its shared owner and any lazy acquisition remain real service dependencies. |
| `00B448AA` | `00B1BFA0` is exactly `MOV EAX,[ECX+30]; RET`: the queue's current `RenderCommandContext`. No retain occurs. |
| `00B448AF` | Capture context `+0C`, the retained scene owner, in ESI. Its transform supplies the direction and position. Context `+08` camera is not read here. |
| `00B448B2` | Test scene-transform world-valid bit 2 and conditionally call recovered `00B6DB70`. |
| `00B448F2` | Test the same scene flag again, saving the condition through the following math. |
| `00B448F6..00B448FC` | Reload captured entry camera `+198` mode, then the actual effect's `+140+mode*4` distance. The gate-time distance argument is not reused. |
| `00B4495D..00B44961` | Refresh the same scene transform if the saved second test was false. |
| `00B449F8`, `00B44A13` | View and projection getters operate on the captured entry camera, independently of the scene transform. |
| `00B44A4A..00B44A4F` | Reload the saved renderer from stack `+20`, then append through `00B25040`. |

`MaterialEffectPlaneBindings` provides identity mappings from the existing
`RenderCommandReference`, `CameraTransform` and `D3D9StateCache` owners. Those
mappings are side-effect-free and return existing objects; they are new host
interfaces, not inferred game virtuals. Only `queue_singleton_004c11f0` is a
required operation. The concrete queue getter uses existing
`RenderCommandQueue.current_context` and `RenderCommandContext.scene` storage.
Every captured owner remains alive across subsequent calls. A missing binding
fails explicitly and never constructs a substitute queue, camera or renderer.

## Arithmetic and copy order

The basis is `(0,0,1)`, from raw positive-zero stores and `00D7A24C=3F800000`.
`0042D0D0` receives ECX=output vector, EDX=input vector, stack=scene world matrix
and a zero normalization byte, and returns EAX=output with `RET 8`. It multiplies
the complete 3-by-3 basis using the original x87 schedule and float32 spill
points. Taking the third matrix row directly would lose `0*NaN` and `0*Infinity`
behavior. Only its `normalize=false` branch is reconstructed here; the nonzero
branch calls `00419510` and remains outside this packet.

For transformed direction `v`, the native operations are:

1. Reload distance through x87 `FLD/FSTP`. Form each scaled component from `v`
   and that distance with the observed x87 products and float32 spills.
2. Form each normal component with SSE `-0.0f - v[i]`.
   `00D7A208=80000000`; replacing it with positive zero changes signed-zero cases.
3. After the second conditional world refresh, load/store world translation
   `+120/+124/+128` through x87, then add the scaled direction to that position.
4. Accumulate `point.y*normal.y`, then `point.x*normal.x`, then
   `point.z*normal.z` through x87. Spill the sum to float32, reload it, apply
   `FCHS`, and store plane D. The normal is not normalized.
5. Obtain the captured entry camera view and invert it with `00B63B30`; obtain
   its projection and invert it with `00B632D0`. Call `00413920` with
   **inverse projection as left operand and inverse view as right operand**.
6. `00B23360` copies sixteen raw DWORDs forward with `REP MOVSD`, then transposes
   six off-diagonal pairs in order `(1,4),(2,8),(3,12),(6,9),(7,13),(11,14)`.
   Lower-to-upper copies use `FLD/FSTP`; upper-to-lower copies use `MOVSS`.
   Signaling NaNs can therefore quiet in only one half of the transpose.
   `std::swap(float)` does not establish this native behavior.
7. `00B65BA0` raw-copies the source plane, calls `00B62D10`, then raw-copies the
   result. The existing recovered implementation supplies this final transform.

No guessed clipping equation, singular-matrix fallback, normalization, alternate
camera domain, or device-capability gate is added. Arithmetic uses the caller's
x87 and SSE control modes. The source is explicitly MSVC Win32.

## Append and ABI boundary

The reused `D3D9CameraFrameAccess` shares the actual cache, device and optional
renderer lock. `00B25040` calls `00B23E50` at current active index, increments
active count, and sets `D3DRS_CLIPPLANEENABLE` (152) to the low-bit mask. Pending
count stays unchanged. `00B23E50` copies coefficients through x87 to renderer
`+190C+index*16` and calls its device `+1A10` virtual `+DC` (`SetClipPlane`) with
the original source pointer. The render-state operation uses device virtual
`+E4`. HRESULT failure does not suppress count advancement or the mask update.
These are the established actual device bindings; the new resolver interfaces
do not stand in for those calls.

Native `00B44750` is thiscall ECX=selected pass, stack=entry/override, `RET 8`.
Its plane interior is not an independently callable native function.
`00B23360` is register-call ECX=destination, EDX=source, EAX=destination, `RET`.
The new public C++ interfaces are semantic projections, not native layouts or
drop-in ABI replacements. Invalid mode, null/inconsistent owner, or an append
beyond the companion's fourteen stored planes is a new host binding error;
earlier cache refreshes persist.

## Verification

`bsp.py ghidra` verified the configured `bsp` project and
`/battlestationspacific.exe` before the analysis/export batches. Nine live native
regions match the read-only installed executable byte-for-byte. Addresses,
lengths, hashes, original annotations and unapplied annotation proposals are in
`reports/material_effect_plane_audit.json`. No Ghidra write or ledger edit is
made by this worker; the primary integrator owns annotation, save and re-export.

The source compiled with MSVC Win32 `/W4 /WX /fp:strict`. `scripts/build.ps1`
passed the baseline project and existing `reconstructed_math` and
`native_math_differential` tests after `verify-seeds`. For that check, the worker
used only an unstaged `cache()` accessor overlay in the shared camera header.
The new source was separately compiled and linked into the focused fixture;
its persistent CMake registration belongs to integration.

The ignored fixture executes copied native `00B448A3..00B44A53`, with original
direction, affine/general inverse, matrix multiplication, transpose, and
plane/vector4-transform bodies. Four selected scenarios compare all four plane
words, x87 status, MXCSR exception flags, singleton count and clip-mask calls.
They exercise a nonuniform scene matrix, a distinct camera view/projection,
late mode/distance changes and entry-camera replacement during queue acquisition,
scene refresh, signed zero, signaling NaN, and failing COM calls that still
advance the mask. Three direct helper comparisons isolate asymmetric transpose
NaN quieting, in-place transpose, and direction arithmetic. All passed.

Queue acquisition, scene refresh, camera getters and append are redirected to
synthetic owner adapters in the native fixture; copied mathematical bodies run
unchanged except for relocated calls/constants. The selected branch's preceding
gates and following draw are not executed by this fixture. This establishes
bounded differential agreement, not a loaded game-owner graph, native singleton
construction, original ABI integration, GPU output, or game validation.
