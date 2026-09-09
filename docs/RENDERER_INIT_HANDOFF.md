# Renderer initialization handoff

Implementation update: DEFAULT_SURFACES records the depth-binding reconstruction
and default-capture COM ownership projection, including current validation.
The original audit below describes the native sequence and remaining boundaries.

The window initializer `00becee0` calls renderer primary vtable slot `+4h`,
`00b2aeb0`. The current C++ device prefix stops at the presentation-parameter
store `00b2b014`; the sequence below is still an integration boundary, despite
several individual device/state/resource helpers already existing.

## Verified sequence after device creation

Native `00b2aeb0` uses ECX for the renderer, ten stack arguments, and `RET 28h`.
At the continuation, ESI is renderer, EBX is one, and EBP points to renderer
`+1a10h` (the device pointer).

| Order and address | Observed operation | Current reconstruction boundary |
|---|---|---|
| `00b2b01a..00b2b026` | `GetCurrentThreadId`, store global `0108d4c8`, then `timeBeginPeriod(1)` | Thread ownership and timer-period lifetime must belong to the integrated renderer owner. The HRESULT is ignored natively. |
| `00b2b02c..00b2b042` | Set render state `161` to `(multisample argument != 0)` through `00b24460` | Existing cached setter; `161` is `D3DRS_MULTISAMPLEANTIALIAS`. |
| `00b2b047..00b2b050` | Capture default surfaces `00b238d0`, then initialize defaults `00b26170` | Default-state leaf exists; full capture is missing. |
| `00b2b055..00b2b065` | For streams 0,1,2,3 call `00b24a40(stream,1)` | Existing stream-frequency setter; earlier docs describing it as unported are stale. |
| `00b2b067..00b2b0e5` | Allocate 44-byte vertex wrapper, construct `00b4bbb0`, store at `+1974h`; create 16 MiB dynamic/writeonly DEFAULT vertex buffer; attach via wrapper `+14h` with flags `1000h` and capacity; release temporary COM reference | Existing `create_dynamic_buffers_00b2aeb0` extracts device allocation only. Existing `VertexBufferBinding` has typed ownership but does not reproduce the native constructor/diagnostic registry. |
| `00b2b0e7..00b2b164` | Allocate/construct analogous index wrapper `00b4bb60`, store at `+1978h`; create 1 MiB INDEX16 buffer, same usage/pool; attach and release temporary COM reference | Same owner/registry boundary. Native completes vertex attachment before starting index allocation; allocating a pair first changes ordering. |
| `00b2b166..00b2b1ba` | Set renderer byte `+1d8ch` to one; compute unsigned width/height aspect comparison and store boolean at `+1a14h` | Small arithmetic fragment, not evidence that the renderer is fully ready. Meaning of `+1d8ch` beyond the write is not established here. |
| `00b2b1c0..00b2b1c8` | Call primary virtual `+f0h` with float positive zero | Resolves to gamma-ramp setter `00b21960`, discussed below. |
| `00b2b1cc..00b2b1d2` | Set pending-reset global `0108d4b8` to one, invoke `00b2abd0` | Full native reset/resource callback lifecycle remains missing. |

The aspect computation uses x87 signed integer loads with `+4294967296.0f`
correction for unsigned inputs, an unspilled division, and comparison against
binary64 `1.3333333730697632` at `00cf5750`. This is the widened float32
approximation of 4/3, not exact mathematical 4/3. `FCOMIP` followed by `JBE`
selects false for unordered as well as less/equal. A C++ float division can
introduce a rounding boundary that the native sequence does not have.

Native initialization assumes the allocation/device operations succeeded;
there are no HRESULT branches before the wrapper or COM dereferences. An
integrated typed owner should expose explicit failure/cleanup behavior rather
than simulate those unchecked native faults, and document that interface change.

## Default-surface capture is the first substantive missing unit

`00b238d0` is ECX-renderer, no stack arguments, ordinary `RET`. Its assembly
has a complete body through `00b23b01`; misleading decompiler stack variables
are unnecessary for determining the ownership order:

1. Get render target 0 (`IDirect3DDevice9` slot `+98h`) and call surface
   `GetDesc` (`+30h`). The local description is not consumed by the remaining
   capture body; the wrapper's initializer queries its own description.
2. Allocate 52 bytes through `00b3f2a0`, construct via
   `00b3f630(surface, 0, 0)`, replace renderer `+197ch` with normal intrusive
   retain-before-release assignment, drop the constructor's temporary wrapper
   reference, and release the getter's COM reference.
3. Get the depth-stencil surface (`device +a0h`), construct another wrapper
   with exactly the same two zero arguments, replace renderer `+198ch`, drop
   the temporary wrapper reference and getter COM reference.
4. Call `00b21690` on the installed depth wrapper.

Several native accessors emit balanced AddRef/Release pairs in addition to
those ownership transfers. They do not create an extra retained lifetime.
The surface constructor's `00b3cc80` initializer retains the surface COM
reference and records its description; that leaf is already implemented.
The complete constructor additionally establishes intrusive metadata and calls
`00b3e730`, whose surrounding registry behavior is not a substituteable no-op.

A critical reset detail: **both captured wrappers receive recreation-kind
byte zero**, even the depth wrapper. Assembly at `00b3f689..00b3f699` copies
constructor argument three to wrapper `+30h`, and both capture call sites push
zero. Do not infer `depth_stencil=true` from the surface's format or its owner
slot. These are default device surfaces; reset processing reacquires them.
The generic offscreen recreation helper has a different ownership role.

The next immediate implementable missing leaf is `00b21690`: its new typed
interface can borrow `D3D9SurfaceBinding` and use the existing optional renderer
guard. It unconditionally calls `SetDepthStencilSurface`, passing null for a
null wrapper and wrapper `+2ch` otherwise. It performs no identity-cache test
and retains no wrapper. Counter `+1bcch` increments only for a nonnull wrapper,
even when that wrapper contains a null COM surface, and regardless of HRESULT.
It consumes one stack argument (`RET 4h`). The existing guard helpers already
supply its only non-COM dependency.

After that leaf, integrate capture into an explicit renderer resource owner,
including replacement/release behavior and eventual reset reacquisition.
Recording it as the complete native surface constructor would overstate the
registry/intrusive-lifetime work. The remaining startup sequence should be
composed only as those owner boundaries are resolved.

## Virtual +f0h is gamma, not a frame callback

The renderer vtable word at `00d5f198` contains `00b21960`. This address is absent
from the current function snapshot, so ordinary function export rejected it.
Read-only raw disassembly recovered its complete `00b21960..00b21b39` body;
no Ghidra function creation or annotation was performed in this audit.

It is an optional-guard, ECX-renderer, one-float-argument routine (`RET 4h`).
Byte `+1b53h` gates all gamma work. For enabled operation, it compares the
requested value against cached float `+196ch`; equal ordered values skip the
ramp update. Changed/unordered values store the new setting and generate three
identical arrays of 256 WORDs for `D3DGAMMARAMP`. The ordinary finite path uses
`clamp((setting+10)/20,0,1)`, reciprocal of twice that value as the power
exponent, and the sample coordinate `index/255`; power helper `00bfeb10` and
x87 truncation supply the sample value. The exact exceptional-value/power
helper behavior is outside this handoff audit.

Device virtual `+54h` is `SetGammaRamp(0, flags, &ramp)`. Byte `+1b54h` selects
flags 1 versus 0. Startup requests setting 0, which ordinarily produces the
identity exponent; the capability and cached-value gates may prevent any
actual API call. Do not force gamma changes simply because startup invokes
this virtual function. This unit is independent follow-up work once its
capability initialization and power-helper behavior are grounded.

## Reset handoff must stay explicit

`00b2abd0` first requires the recorded render thread and pending/lost state.
Actual reset additionally requires platform byte `+41h` and focus matching its
HWND. It releases dynamic resources through `00b237d0`, unbinds/notifies through
`00b262c0`, resets using stored presentation parameters, then reacquires default
surfaces and restores resources/defaults. The initial windowed CreateDevice
therefore does not prove the later fullscreen request was applied. A hidden
unfocused diagnostic HWND can leave the pending request deferred; successful
creation and resource probes are not proof of full native startup completion.

## Byte evidence and scope

Every live analysis/export/memory-read batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`, using the configured
`C:/Users/sqz269/bsp.gpr`. The following complete or explicitly bounded byte
ranges matched installed PE bytes exactly on 2026-09-09:

| Range | Bytes | SHA-256 |
|---|---:|---|
| Startup continuation `00b2b01a..00b2b1f1` | 472 | `f402efdd4e12397f2248cebf58af2c5f62f03fdc3c02001b598a2eef16f453d2` |
| Capture `00b238d0..00b23b01` | 562 | `837027c1f54987ea218ea59e550dfeb48b2d410074c9a36d1c5749a30a75b990` |
| Depth bind `00b21690..00b21731` | 162 | `bbbc544b2cef1e63efc7742fc00a83e63c62dd155d919b68452190dc7d2d4881` |
| Raw gamma setter `00b21960..00b21b39` | 474 | `395daf39c6ea3f1871fd11d6aea7b1d6e5d9d2005f6d876b39fd3ffc26ef894a` |

The continuation begins immediately after the already ported store; it includes
the native final `RET 28h` and is not represented as a separate native function.
This is an analysis handoff only: no code, tests, builds, metadata, or Ghidra
annotations were changed. Existing source is authoritative over stale
implementation-status paragraphs in older renderer documents.
