# D3D9 startup recovery

The application initialization at `0073da88` constructs the concrete renderer via
`00b32410`. It installs primary vtable `00d5f0a8`, secondary vtable `00d5f0a4`,
and calls `Direct3DCreate9(32)`, storing the API pointer at object offset `1990h`.
The renderer singleton `00f8d394` reaches primary virtual slot `+4h`, now recovered
as `BSP_D3D9Renderer_InitializeDeviceAndResources` at `00b2aeb0`.

This is a `thiscall` function with ten stack arguments (`RET 28h`). Its original
object layout and resource ownership are not reproduced. The C++ entry point
ports only the device-creation prefix, through the store at `00b2b014`.
It is recorded separately from whole reconstructed routines.

## Device creation

The 56-byte `D3DPRESENT_PARAMETERS` begins at object offset `1a28h`.
The Windows platform caller `00becee0` passes, in order:

| Argument | Observed value |
| --- | --- |
| Window | Platform HWND at `+30h` |
| Fullscreen | Platform mode argument |
| Width, height | Platform fields `+24h`, `+28h` |
| Backbuffer format | 21, `D3DFMT_A8R8G8B8` |
| Backbuffer count | 1 |
| Multisample type | Forwarded platform argument |
| Depth format | 75, `D3DFMT_D24S8` |
| Presentation sync | Boolean derived from nonzero requested bit depth |
| Fullscreen refresh rate | 0 |

The prefix clears the structure, copies these fields, chooses swap effect DISCARD,
zero multisample quality, enables depth when the depth format is nonzero, and sets
`D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL`. Presentation interval is exactly
`~(sync << 31) & 0x80000000`, so only the low bit affects it.

`GetDeviceCaps(0, HAL, &caps)` precedes `CreateDevice`. The assembly tests DevCaps
bit `10000h` and the low word of VertexShaderVersion against `0101h`. Both passing
selects hardware vertex processing (`40h`); otherwise software (`20h`). It adds
MULTITHREADED (`4h`). SDK structure offsets are compile-time checked against the
assembly's stack offsets. No mixed or pure-device fallback appears here.

**CreateDevice receives Windowed=TRUE even for a fullscreen request.** Only after
the COM call does `00b2b014` store `!fullscreen` into the presentation structure.
The port preserves this ordering and the API's other parameter mutations. The
later reset processor `00b2abd0` uses the stored parameters when the platform window
is active and focused; see `D3D9_STATES.md`. Its dependencies still need reconstruction.
Fullscreen behavior has not been exercised.

The native prefix ignores HRESULTs. The new interface reports failures and guards
against overwriting an owned output pointer; these are documented interface
differences. It does not invent a renderer object with unresolved globals.

## Validation and remaining work

`bsp_d3d9_probe` supplies an ordinary hidden diagnostic HWND, calls the recovered
prefix against the installed D3D9 runtime, and queries the resulting swap chain.
It observed success, flags `44h`, 640x480, windowed, color format 21, depth 75 and
interval 0. It releases all COM references and destroys the diagnostic window.
This verifies device creation on this machine; it does not establish rendered
imagery, original-code differential agreement, fullscreen behavior or gameplay.
No additional CTest cases were added. Existing tests remain 2/2 passing.

After the ported prefix, the native routine records a thread ID, calls
`timeBeginPeriod(1)`, initializes render state through `00b24460`, `00b238d0` and
`00b26170`, and creates dynamic default-pool buffers: a 16 MiB vertex buffer and
a 1 MiB 16-bit index buffer, both usage `208h` (DYNAMIC | WRITEONLY). It attaches
them to unresolved wrappers and continues engine setup. These operations are
not part of the device prefix. The default-state helper `00b26170` and its cached
setters have since been reconstructed separately; see `D3D9_STATES.md`.

Also recovered `00bec3b0`, the original stdcall window-procedure thunk: it loads
singleton `0109cf04` and forwards object, HWND, message, WPARAM and LPARAM to
platform vtable slot `+28h`, then returns with `RET 10h`. It now has a name and
prototype in Ghidra. The full window handler still depends on input/audio/UI;
the diagnostic HWND does not implement or replace it.
