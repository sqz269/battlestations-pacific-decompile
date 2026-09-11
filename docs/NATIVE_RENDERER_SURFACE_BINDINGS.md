# Actual renderer surface binding

`src/native_renderer_surface_bindings.cpp` reconstructs complete color binder
`00B23D80..00B23E4F` and depth binder `00B21690..00B21732` using borrowed actual
renderer bytes, actual `NativeSurfaceOwnerStorage` objects, a real D3D9 device,
and the actual synchronization global view. End addresses are exclusive.
Descriptive names are hypotheses. These are new C++ interfaces, with no recovered
HRESULT return contract or drop-in binary compatibility claim.

| Native entry | Inputs and stack cleanup | Actual storage |
| --- | --- | --- |
| `00B23D80` | ECX renderer; stack DWORD slot, wrapper; RET 8 | device `+1A10`, default wrapper `+197C`, counter `+1BA0` |
| `00B21690` | ECX renderer; stack wrapper; RET 4 | device `+1A10`, counter `+1BCC` |

Both wrappers expose the borrowed COM surface at `+2C`. Neither binder retains
the wrapper or compares/stores an identity cache. A nonnull supplied wrapper
causes a counter increment after the COM call returns, regardless of HRESULT or
whether that wrapper's COM pointer is null. Counter increments read the current
DWORD and wrap modulo 2^32. A thrown call skips the increment.

The color binder reads wrapper `+2C`, then current renderer `+1A10`, then the
captured device's current vtable and slot `+94` (`SetRenderTarget`). A null
wrapper and slot zero first dereferences the default wrapper at `+197C`, without
a null guard, then captures its surface before the device. A null wrapper with
another slot passes a null surface. Neither null-wrapper path increments.

The depth binder instead captures the current device before reading wrapper
`+2C`, then loads the captured device's current vtable and slot `+9C`
(`SetDepthStencilSurface`). Its null-wrapper path passes null and does not
increment. Explicit volatile DWORD reads preserve these differing load orders.

## Optional guard and native exception evidence

Both entries test the current raw mode byte. When enabled, they store the
renderer in the actual eight-byte local guard before calling `00B33AD0`, and
store its returned AL afterward. Entry occurs while native EH state is -1;
an entry exception does not run guard cleanup. A skipped entry leaves the
guard uninitialized. The implementation does not invent a repair when a mode
transition would cause cleanup to read that native uninitialized record.

State zero protects the COM call and counter update. Color unwind `00CBCF10`
and depth unwind `00CBCD50` both pass `[EBP-14h]` to actual `00B21110`.
Their respective handlers at `00CBCF18` and `00CBCD58` select the complete
36-byte FuncInfo records at `00DF5534` and `00DF52C4`. Each has one unwind
entry, mapping state zero to -1 and its guard action, at `00DF552C` and
`00DF52BC`. These bodies, handlers and maps were matched against the installed
PE and the saved Ghidra program.

Normal cleanup tests current mode and disarms the EH state before calling
`00B33B00`. A normal leave exception therefore does not trigger another leave.
The C++ catch protects only the body, with entry before it and normal leave
after it. Native normal cleanup loads the whole DWORD at guard `+0`, including
three uninitialized padding bytes. The new interface passes only the defined
low byte because `00B33B00` ignores that argument entirely. This is an explicit
interface difference; incidental stack padding and native register returns
are not claimed. Exceptional cleanup uses the native byte-sized read.

## Validation boundary

The strict MSVC Win32 build and both existing CTests passed after this change.
One focused primary-library original-caller fixture matched 27 comparisons and
2,898 trace DWORDs. It executes both complete native binders, the original
`B33AD0/B33B00/B21110` synchronization bodies, and the original unwind actions
with their relocated native FuncInfo and maps. Host-image handler bridges pass
that metadata to the real MSVC runtime. The 12 verified spans total 636 bytes;
504 bytes belong to the five executed complete native functions.

Coverage includes null/default paths, failed HRESULTs, counter wrapping, mode
changes, changing the current lock, and fixture exceptions at COM/enter/leave
boundaries. The source functions come from the primary library. Renderer and
wrapper bytes outside the specified writes remain unchanged. Existing actual
synchronization evidence is in
`reports/native_renderer_synchronization_actual_audit.json`; the actual surface
owner layout is established in `include/bsp/native_surface_owner.hpp`.

The real D3D9 attempt reported zero adapters and HAL `CreateDevice` failure
`0x8876086C` on `WinSta0/Default`. Its original log is retained in the fixture
evidence. The passing comparison therefore uses a fixture-only COM-ABI object,
opaque surface tokens and controlled HRESULTs; real initialized Windows
critical sections and real Enter/Leave calls are retained. Fixture C++ throws
test scope ordering, not an assertion that the Windows APIs normally throw.
No production callback interface or mock was added. Driver/surface behavior
and actual rendering were not validated by this comparison.
Full pipeline stop, cache clearing, render worker execution, binary installation
and game/runtime validation remain separate work.
