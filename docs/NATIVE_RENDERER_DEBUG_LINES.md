# Actual renderer debug-line records (`00B28D00`)

The new `draw_native_renderer_debug_lines_00b28d00` provider reconstructs the
complete 966-byte body through `00B290C5`. It operates on the supplied actual
renderer and calls the substantive raw state, shader, vertex/index binding and
record-reserve providers. Root owns its later `native_renderer_end_frame`
integration; this packet does not close the other EndFrame providers.

The original takes its renderer in ECX, has no stack arguments, and ends in a
plain RET. The source `__fastcall` interface adds a borrowed context in EDX:

```cpp
void __fastcall draw_native_renderer_debug_lines_00b28d00(
    void* actual_renderer, const NativeRendererDebugLinesContext*);
```

The context borrows the existing vertex and index binding contexts, the actual
`0108D6DC` synchronization storage, the original `D5F0A8` profile through slot
`+138`, and the live float cell `D7A24C`. All services must use the same actual
renderer, synchronization cells and owner associations. Those objects and every
raw location reached by the operation must remain valid through its callbacks.
No context, owner, device, profile, pool, or global is created by this function.

An initial zero DWORD at renderer `+1D04` returns before any context access. This
path accepts a null context and leaves even a negative capacity untouched. A
nonzero count requires the complete borrowed context. Counts are native signed
DWORDs; a negative initial count still enters the state sequence.

The native schedule is preserved:

1. Nine render-state calls, vertex shader unbind, pixel shader unbind, current
   device `SetFVF(4042h)`, and two texture-stage calls.
2. Three transforms, for states `100h`, `2`, and `3`, use the same 64-byte local
   matrix. Each reads the current `D7A24C` bits once, then captures the current
   device before writing all sixteen matrix words. Each diagonal receives those
   captured bits and every other word receives positive zero. The current table
   is read from that captured device after the stores. The constant, device and
   table are not reused across calls.
3. Current renderer `+134(0,null)` and `+138(null,0)` dispatch to full raw
   `B24840` and `B24B00`. The current renderer token and borrowed profile slot
   are checked separately at each call. Unknown profiles raise a source binding
   error at the reached call; numeric original code tokens are never invoked as
   host function pointers.
4. Only the draw is gated by DWORD `+1D90` and, if that is zero, byte `+1D8A`.
   When both are zero, capture current data `+1D00`, count `+1D04`, device
   `+1A10`, and device table in native order. The primitive count is signed
   `count / 2`, rounded toward zero, passed as DWORD bits. The call uses type
   `2`, stride `14h`, and current device slot `+14C`.
5. Twelve render-state calls, six texture-stage calls, and two final
   render-state calls execute even when the draw was skipped. Repeated cache
   operations remain present. None of the COM HRESULTs suppresses later work.
6. Capture the header at renderer `+1D00`. Negative signed capacity invokes full
   `B22940(header,0)`. Decrement the current positive signed count until it is
   nonpositive, then write zero. Reserve retains its original allocation, copy,
   free and publication order; no extra record destruction or free is added.

The two shader bodies form a separate packet. This provider uses the
shader worker's shared APIs, each taking `(actual_renderer, actual_logical,
NativeRendererSynchronizationGlobals&)`. It does not use `D3D9StateCache` or
duplicate the shader cleanup schedule. The existing raw `B22940` provider uses
stride `14h`; its name's `records20` means twenty bytes, not `20h`.

`B28D00` has no local FH3 frame and no outer guard. A source exception leaves
prior cache, COM and ownership effects intact and prevents all unvisited calls
and record cleanup. Called providers retain their own guard cleanup and
second-exception boundaries. No rollback, whole-frame restoration or malformed
header repair is introduced. In particular, negative-capacity reserve requires
valid native copy extents; the source does not make overflowing native cases
safe.

The native evidence is pinned under
`local/output/debug_lines_inputs/manifest.json`: all 966 body bytes match live
Ghidra and the installed PE, all 275 instruction addresses agree with the
listing, all six explicit branch targets are internal instruction starts, and
the final instruction is RET. The original `D5F0A8` profile and `D7A24C` cell
also match live and disk bytes. `reports/native_renderer_debug_lines.json`
contains 34 numeric direct-call rows; seven virtual COM/renderer call sites are
explicitly marked indirect and are not proved by the mechanical call gate.

Shader dependency `5ccb5885` is integrated through private ancestry merge
`878fafdb`. The strict MSVC Win32 build passed with all eight native seeds
verified before configuration and both existing CTests passing. The numeric
call gate passed all 34 direct sites, and the source literal arguments agree
with the native pushed DWORDs at every direct call.

One focused fixture passed an original/source empty-return comparison and
three nonempty source cases using two real HAL devices. Those cases cover
fresh device/constant reads and shared matrix identity despite an injected
failed HRESULT, a draw-gated real reserve, and a second-transform source
exception that preserves prior state and leaves records undrained. The three
cases observed 30, 29, and 16 COM calls. Fourteen temporarily observed slots
across the two original device vtables were restored and checked before device
release. The fixture retained the real device vptrs. Its null logical-buffer
bindings isolate this provider; it does not repeat the existing nonempty owner
teardown fixtures. Only the initial count-zero path executed copied original
instructions; the nonempty cases executed the reconstructed source.

The final manifested `/MD` probe links its fixture source solely against the
complete rebuilt core/Lua/zlib libraries and platform import libraries. It does
not compile reconstructed modules into the probe directly. The immutable
artifact manifest in `local/output/debug_lines_validated_artifacts/manifest.json`
separates compiler/source/library/test artifacts from the modules actually
loaded by the final Win32 fixture; the report records its hash and scope.

Root's subsequent dependency audit identified a separate integration limit:
resource-support getter `B3E730` and existing physical/surface/layout/shader
contexts still use projected `SingletonLifetimeDomain` access. Migration to the
same actual `01090AA0` manager remains necessary for full parent destruction
and nonempty terminal composition. This provider neither creates a projected
manager nor claims that migration. The focused fixture's logical-buffer
bindings are null, so it never invokes that resource-support/lifetime path.

This is a new source interface. Original caller ABI, aliases into other native
private stack slots, original hardware-fault/FH3 execution, active original
renderer execution, visual parity and game validation remain unproved.

## Integrated validation at 425b3b48

Original/source empty return matched. Three nonempty source scenarios used two real D3D9 devices, checked current device/constant reloads and ignored HRESULT, performed negative-capacity reserve and retained partial failure. Fourteen original COM slots were restored and checked. Original nonempty execution and actual-AA0 terminal composition remain unproved. The combined strict Win32 build, eight seed checks and both CTests passed.
The four final-library probes,118 direct/tail rows, seven saved/read-back
annotations and38 live/PE spans are retained in `local/checkpoints/425b3b48/native-renderer-constructor-wave/validation.json`
(SHA256 `64c19259048ada5f8868750e99c793c0b95674ea7f391b124b8ed2e613ae8fe0`). Full parent execution and application/gameplay
validation remain open.
