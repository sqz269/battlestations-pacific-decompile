# Shared camera axes and system prefix

`system_camera_axes.cpp` reconstructs both camera getters `00B70EA0` and
`00B70FE0`, their x87 vector/CRT square-root routing, and the system-builder
interior `[00B46C50,00B46CB4)`. The new typed interfaces require the actual
`CameraFrameState`, a borrowed actual CRT runtime mode/handler binding, an
initialized output prefix, and the actual timer pointer slot. They do not model
the native camera layout, constructor, service lifetime, or full CRT runtime.

## One shared cache

Both getters test `camera.projection.valid_flags & 0x100`, the same native
`camera+2F0` flags used by existing camera matrix helpers. They update the actual
`CameraFrameState.axis_y` (`+440`) and `axis_x` (`+44C`) together, then set bit100.
Y returns the first cached triple; X returns the second. A hit returns the
existing triple without reading world data or calling math/CRT services.

On a miss, the getters test the existing transform's world-valid bit2 and call
`refresh_camera_world_00b6db70` only if it is clear. Assembly stack arguments
establish this exact sequence, using the refreshed world words8,9,10
(`camera+110,+114,+118`) as `forward`:

1. `first = cross(forward, {0,1,0})`.
2. `second = cross(first, forward)`.
3. Normalize `second`, then copy its components with x87 loads/stores to Y.
4. `third = cross(forward, Y)`; normalize it, then x87-copy to X.
5. Measure the stored Y. If that length is ordered and less than **0.5f**
   (`00CE3800=3F000000`), overwrite Y with `{0,1,0}` and X with `{1,0,0}`.
6. Set the shared bit100 after all prior work.

Forward is not normalized separately. For forward `+Z`, the ordinary X result
is `-X`; the degenerate fallback is `+X`. Unordered length does not take the
fallback. Cache stores, their order, and the final validity update are retained,
so a nonreturning later CRT handler leaves earlier cache writes in place.

## Vector and CRT arithmetic

| Address | Original contract | Recovered behavior |
| --- | --- | --- |
| `004F9B30` | ECX destination, EDX first vector, one stack vector, RET4, EAX destination | Exact x87 cross-product operand loads, products and stores. The x operands are loaded after the first output store; this is not a general alias-safe replacement. |
| `00419440` | ECX vector, RET, ST0 result | Float stores of inputs and each square; retained x87 `ySquared+xSquared`, then stored zSquared; float sum; CRT square root; float result store/reload. |
| `00419510` | ECX destination, EDX vector, RET, EAX destination | Native length call, float length store, FCOMI; positive length gets a float reciprocal, zero/negative/unordered gets positive zero; exact x87 component multiplication schedule. |

The existing `world_ocean.cpp` helpers retain their older scalar C++ interfaces
and implementation. This packet's private kernels implement the stricter
schedule for the camera path; no claim is made that unrelated callers have
acquired these guarantees.

`00BF7030` is not replaced by `std::sqrt`. It stores a double view of ST0 and
calls `00C08418`, whose exponent comparison leaves ZF for `00BF704D`. That helper
saves the x87 control word, classifies the stored bits, and retains the native
routes:

- Ordinary finite nonnegative input uses FSQRT. A non-027F control word first
  becomes `(saved & 0300) | 007F`, retaining precision control and selecting the
  native masked nearest-rounding arithmetic mode. Negative zero bypasses FSQRT.
- Positive infinity returns through the ordinary exit. Negative finite input
  or negative infinity uses the verified negative indefinite 80-bit constant
  `00000000000000C0FFFF` and diagnostic type1.
- NaNs go through `00C083BC`: the stored double quiet bit selects type7; the
  other route adds the verified double1 constant and selects type1. Earlier
  x87 loads/stores may already quiet signaling values, exactly as in native.
- The actual global `0109DD78` is reloaded at each original branch. Nonzero
  bypasses error dispatch through the control-word restorer. Zero routes
  exceptional types through `00C08347` and ordinary results through
  `00C0843B` (`__math_exit`), including its type8 inexact-status dispatch for
  the relevant saved control-word conditions.

`00C08347` (`__startOneArgErrorHandling`) constructs the actual 32-byte Win32
record and calls the supplied `__87except` binding with operation5. Its record
has type at0, `"sqrt"` pointer at4, input double at8, uninitialized unused second
argument at16, and modifiable result double at24. The returned result and saved
control word are reloaded in native order. The string bytes match; the typed
implementation has a new string pointer identity.

`CameraAxesCrtAccess` borrows the actual `0109DD78` mode address and the actual
`void __cdecl __87except(int, CameraAxesCrtException*, uint16_t*)` adapter. It
does not choose a replacement error policy. `00C27489` itself remains outside
this packet: its downstream `__handle_exc`, `__raise_exc`, `__ctrlfp`, matherr,
and errno behavior must come from that binding. The control-word and error
routing before/after the binding are reconstructed locally. Both bindings are
required on a cache miss; invalid bindings are a new typed precondition error.

## Ordered prefix and timer handoff

With native c0 based at `[ESP+14]`, the interior stores X into words124..126
(`c31.xyz`) and Y into words128..130 (`c32.xyz`). It leaves both `.w` padding
words untouched. Output must be nonnull and contain at least131 initialized
float words; invalid capacity is rejected before output or cache side effects.

The writer calls the X getter, performs its three MOVSS stores, then calls the
Y getter. It loads Y.x, directly reads the actual volatile `FrameClock*` slot
at the `00B46C89` point, stores Y.x, then loads/stores Y.y and Y.z. There is no
callback at the capture point. The returned borrowed pointer is consumed by
the next time segment beginning `00B46CB4`; later timer loads must still be
performed independently. The writer preserves earlier stores if a later
operation does not return.

## Verification and limits

- Verified existing `C:/Users/sqz269/bsp.gpr`, project `bsp`, program
  `/battlestationspacific.exe` through the guarded CLI before native batches.
  Fourteen bounded ranges matched the installed executable byte for byte;
  hashes, constant bytes and annotation preimages are in
  `reports/system_camera_axes_audit.json`.
- The new source compiled with MSVC19.51 Win32 `/W4 /WX /fp:strict /O2 /MD`.
  A temporary include overlay used the integrator's actual updated camera/fog
  headers; those hashes are recorded. No shared header was changed here.
- One focused, local differential fixture compared copied verified native
  getters/vector/CRT routing with this implementation: **252 comparisons,
  38 observed CRT dispatches, zero failures**. Inputs included nontrivial finite
  values, zero, parallel forward/up, signed zero/subnormal, qNaN, sNaN, infinity
  and overflow; two masked control words and both live dispatch modes were used.
  It compared cache bytes, shared validity/identity, prefix values/padding,
  captured timer, x87 exception flags/control, handler records and handler result
  replacement. The native world-valid bit was set, so native world refresh was
  not reexecuted by this focused fixture.
- `scripts/build.ps1` passed the worktree's existing build and both existing
  CTest entries. Because CMake integration belongs to the primary agent, that
  build does not itself establish linkage of this newly added source.
- The fixture replaces only the external `__87except` boundary with a recording
  handler. It establishes routing and result propagation, not the real CRT's
  exception/errno/matherr effects. No game process, startup, imports, or gameplay
  was exercised; native ABI compatibility and game validation are not claimed.

The worker made no Ghidra annotation changes. Proposed names preserve existing
vector/library names and add provisional descriptive names only for the two
camera getters; existing comments are retained in the audit for integration.
