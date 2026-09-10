# Camera frame command and clipping state

The renderer virtual calls in camera command `00B71360` resolve through vtable
`00D5F0A8`: slot `+A0` is camera preparation `00B285A0`, `+A4` is viewport binding
`00B26770`, and `+08` is Clear `00B21430`. These three targets were absent from
Ghidra's function table. Complete installed-PE/live-Ghidra byte matches and
offline instruction decoding establish their extents through `00B287BF`,
`00B26891`, and `00B214DA`. The audit retains the missing-function preimages;
the worker made no saved-analysis mutations.

`include/bsp/camera_frame_state.hpp` and `src/camera_frame_state.cpp` expose
typed camera and renderer companions. They execute the recovered operations
against the existing `CameraState` and `D3D9StateCache`, including the actual
device, optional guard, cached render states, borrowed viewport identity and
clip counts. One stable companion must accompany each underlying object. These
are new C++ storage and lifetime interfaces, not native constructors, layouts,
intrusive wrappers or drop-in ABI replacements. Descriptive names remain
reconstruction names.

## Recovered entrypoints

| Native address | Original ABI | Established behavior |
| --- | --- | --- |
| `00B71360` | ECX camera; RET | If byte `+17C` is nonzero, prepare camera, bind borrowed `+180` viewport, then Clear with count0/null rectangles and fields `+188..194`. |
| `00B285A0` | ECX renderer; stack camera; RET4 | Disable clip mask, refresh inverse VP and frustum caches, copy plane records/count, reset active/pending counts, optionally transform selected user planes, restore pending mask, then optional ambient state. No outer guard. |
| `00B26770` | ECX renderer; stack viewport; RET4 | Optional guard; SetViewport, increment attempted-call counter, publish borrowed wrapper, cached scissor state, re-read scissor byte and optionally SetScissorRect. |
| `00B21430` | ECX renderer; six stack arguments; RET18h | Zero flags skip guard, color dereference, COM call and counter. Otherwise guarded Clear with count/rectangles/flags, dereferenced color, x87-copied depth and stencil; increment attempted-call counter. |
| `00B23E50` | ECX renderer; stack index/float4; RET8 | Optional guard; cache four raw words at `+190C + index*16` before SetClipPlane, including on API failure. |
| `00B25040` | ECX renderer; stack float4; RET4 | Set plane at active count, increment active count, set low-bit clip mask. Pending count is unchanged. |
| `00B25080` | ECX renderer; RET | Set mask from pending count, then copy pending count to active count. |
| `00B70510` | ECX camera; EAX matrix pointer; RET | On missing projection flag20h, invert current VP, copy through x87 into `+260`, set flag20h. |
| `00B70710` | ECX camera; EAX plane-set pointer; RET | On missing flag4, set it before nested VP refresh; extract/normalize six planes and assign their records with flags7. The plane count is unchanged. |
| `00B632D0` | ECX destination, EDX source; EAX destination; RET | General 4x4 Gauss-Jordan inverse with pivot selection, row exchange, native float stores and x87/SSE arithmetic; no singularity check or fallback. |
| `00B653F0` | ECX six-plane destination; stack matrix; RET4 | Extract six matrix-column combinations, negate each D, normalize all four coefficients. |
| `00B65BA0` | ECX source plane; stack destination/matrix; RET8 | Snapshot float4 bits, call vector transform `00B62D10`, copy result bits through temporary storage. Aliasing is supported. |
| `004FB850` | ECX color destination; stack source float4; RET4 | Multiply RGBA channels by verified double255.0, perform runtime-selected truncation, clamp signed results to0..255, pack ARGB. |

The audit also records complete ranges, hashes and prior annotations for the
small getters, inline plane copy `00B250B0`, six-plane assignment `00B658E0`,
normalization `00B650B0`, vector transform `00B62D10`, vector length `00419440`,
and CRT conversion helpers `00BF7420`/`00BF7456`.

## Camera and renderer state

The camera companion holds enabled byte `+17C`, borrowed viewport `+180`,
optional ambient-light object's float4 at its own `+08`, Clear fields
`+188/+18C/+190/+194`, and material render mode `+198`. Render mode is separate
from a command batch index. The inverse VP and frustum use the existing camera
projection validity word at native `+2F0`; no separate validity flags were
introduced.

Each plane record is sixteen coefficient bytes followed by flags. Sixteen
inline records occupy `140h` bytes and a separate count follows. Frustum
refresh writes the first six records with x87 copies and flags7, preserving
later records and count. Zero initialization is not evidence of the native
constructor: callers must supply the established count and any additional
planes. The renderer copies all `140h` record bytes and then the count.

Preparation first sets D3DRS_CLIPPLANEENABLE(152) to zero and resets renderer
active `+19EC` and pending `+19F0`. If support byte `+1B51` is set, records with
flag4 are skipped and flag2 is required. For each selected record it recomputes
inverse view and inverse projection, multiplies inverse-projection by
inverse-view, transposes the product and transforms the plane. SetClipPlane
uses consecutive pending slots; pending increments after the attempt even if
COM fails. The final mask is `(1u << (count & 31u)) - 1u`, preserving x86 shift
masking. Active count is then copied from pending. Later material operations
append an active plane or restore the pending mask through the same companion.

The recovered clip-cache span `+190C..19EB` provides fourteen float4 slots
before the counts. The typed interface requires plane-set count at most16,
every selected/appended slot below14, and compatible actual device limits.
Out-of-bounds native accesses are not emulated. No speculative extra storage
or device capability inference is used.

The viewport wrapper exposes DWORD X/Y at `+08/+0C`, width/height at
`+10/+14`, exact scissor byte `+20`, and RECT `+24`. Device MinZ/MaxZ are always
0/1. Binding increments `+1BD0` and stores a borrowed identity at `+1904` after
the attempted SetViewport. It sets D3DRS_SCISSORTESTENABLE(174) using the exact
byte, then reads the byte again before SetScissorRect. Clear increments
`+1BD4` after a nonzero-flags device attempt. The native routines ignore
HRESULT; the new APIs expose viewport/scissor/Clear/clip HRESULTs without
changing state-update or continuation order. Cached render-state HRESULTs
remain unavailable through the existing setter.

Each guarded entry uses the actual cache's synchronization and lock. A viewport
bind nests the render-state setter's guard. Like native code, guard destruction
re-reads the current enabled flag, and `leave_00B33B00` ignores its saved enter
result. Live mode changes retain this unusual native behavior; the companion
does not invent a separate guard state. Native SEH registration and replacement
of the global renderer pointer between camera-command calls are outside this
stable-reference interface.

## Numerical evidence and limits

The general inverse, vector transform, plane extraction/normalization and
extended-input x87 integer conversion use private reassembled kernels with
typed constants. Their original register schedules, stores and branch order
are preserved. Arithmetic instruction forms were decoded from bytes because
Ghidra's x87 text sometimes omits the destination operand. No original game
address is executed by production code.

Plane normals use `00419440`: copy XYZ through float stores, store each square,
add stored y-squared plus x-squared in x87, add stored z-squared, store the sum,
take the square root and store the result. The finite nonnegative CRT square
root core is represented by FSQRT, with the existing CRT evidence in
`reports/model_world_bounds_audit.json` covering masked nearest CW007F/027F.
Other rounding/control modes, exceptional CRT diagnostics, user matherr/SEH
behavior and complete status/FIP/FDP parity remain open. Plane normalization
uses a stored reciprocal when length is greater than zero, otherwise positive
zero; the unordered comparison follows the zero branch.

Ambient conversion explicitly receives the actual runtime choice represented
by native global `0109EEA4`. Nonzero selects a double spill followed by
CVTTSD2SI. Zero selects the recovered extended-ST0 `00BF7456` signed64
truncation path and uses its low signed32 word before clamping. The previously
reconstructed binary32-only batch-depth converter is not a substitute for
this extended input. This shared CRT dependency remains owned by the primary
integrator, with its existing library name preserved.

## Validation

- All28 function ranges, totaling5380 bytes, match installed PE bytes against
  the saved `bsp` program; the renderer vtable and three numerical constants
  also match. Every query verified project/program identity through the CLI.
- Scoped MSVC Win32 `/W4 /WX /fp:strict` compilation passed. The focused local
  probe passes38 installed-PE comparisons across CW007F/027F: general inverse
  with aliasing, plane transform with aliasing, six-plane extraction using
  the established finite square-root core, and both ambient conversion modes
  including exceptional channel inputs.
- A focused COM-vtable probe passed nested guard depth, borrowed viewport
  identity, exact viewport fields, scissor-byte re-read, zero-flags Clear skip,
  failed-call counters/cache, enabled-command order, frustum count preservation,
  two eligible user clips and append/restore mask behavior. It uses the actual
  reconstructed cache and an isolated fake device.
- `scripts/build.ps1` passed after seed verification, including both existing
  reconstructed and native math checks. Primary integration owns the CMake
  registration; the separate scoped compile covers this new source.

These are export, byte-identity, compile and isolated numerical/device-call
checks. No live game, graphics device, rendered frame or native object ABI was
validated.
