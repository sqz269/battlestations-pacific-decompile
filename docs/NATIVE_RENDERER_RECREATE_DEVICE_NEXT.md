# Native device recreation dependencies

`B29670..B29B17` is the complete 1,191-byte device recreation routine. It is
still unported as an actual renderer-storage operation. Existing typed state,
resource, and vertex-layout projections do not close that requirement.

This discovery verified the original `bsp.gpr` and `/battlestationspacific.exe`
through guarded queries. Twenty fresh live/PE spans total 2,222 bytes, including
the full recreation body, its FH3 action and metadata, selected original
renderer profile cells, the ready leaves, and the separately missing gamma
setter. Exact spans, instruction decodes, dependency lookups, SDK declarations,
and source hashes are in `reports/native_renderer_recreate_device_next.json`.
No executable analysis or original game file was changed.

The native interface takes the actual renderer in ECX, no stack arguments, and
returns with `RET`. The body contains 22 distinct immediate call targets and
13 indirect call sites, including the two real critical-section imports.

## Lifetime and observable ordering

The current `108D6DC` mode controls the optional guard. After guard setup,
native state0 is armed before entering the tracked critical section reached
through renderer `+199C`; its depth at `lock+18` increments after Enter.
The one-entry FH3 map `DF5AC8` points to `CBD2F0`, which destroys only the
optional guard at `EBP-148` through the complete `B21110` provider. It does not
release the tracked critical section during unwind. An inner exception can
therefore leave the tracked lock and depth held.

Normal cleanup independently reloads the current renderer `+199C`, decrements
that lock's depth, and calls Leave. It then clears the EH state and consults the
current mode again for optional guard cleanup. A generic scope lock around the
whole reconstruction would change this behavior.

The main sequence is:

1. Clear byte `+1D8A`, call primary virtual `+98(0)`, unbind current device
   resources, release dynamic buffers, and release reset resources.
2. Walk the actual checked hardware-layout tree at `108D530`, then four current
   pointer arrays at `+1AAC`, `+1AB8`, `+1AC4`, and `+1AD0`. Their current table
   and count are reread around the calls. The `2C`-byte records at `+1A78` call
   each current owner at record `+28` through virtual `+28`.
3. Conditionally call XLive device destruction. A captured device at `+1A10`
   receives an AddRef/Release pair when nonnull; the current device field is
   then released unconditionally and cleared. There is no added null repair.
4. Call current `IDirect3D9::GetDeviceCaps` and `CreateDevice` for adapter0/HAL.
   HRESULTs do not gate the following operations. `DevCaps` bit `10000` and
   the low WORD of `VertexShaderVersion >= 0101` select hardware processing;
   otherwise software processing is used. Multithreaded creation is always
   added. The current window, presentation structure, and device field are
   passed directly.
5. Apply sRGB-write state and default states, then pass cached gamma `+196C`
   through the original x87 load/store into primary virtual `+F0`. Restore
   resources, optional auxiliary owners, XLive, the checked layout tree, the
   four arrays, and the current record virtual `+2C` callbacks.

The auxiliary owner route at `+1974/+1978` publishes byte `+1D8C=1` before its
two virtual `+20` calls. Its concrete ownership/type closure remains open.

## Existing source does not imply native dependency closure

The actual-storage iterator `B20DC0` and optional guard providers `B33AD0`,
`B33B00`, and FH3 cleanup `B21110` are complete. `B23D80`, reached by the
unbind routine, also has a complete actual-storage surface binding.

The following boundaries remain material:

| Route | Current boundary |
| --- | --- |
| `B24BF0` unbinding | Full 453-byte body unported; indirect renderer bind methods and its own guard states must be preserved. |
| `B237D0`, `B24460`, `B26170` | Typed state/cache projections exist; full native renderer field access and callback composition remain separate work. |
| `B262C0`, `B23B10` | Resource release/restore fragments exist; complete registry, owner, and virtual-call routes remain open. |
| `B49D00/B49F80`, `B49DC0/B4A040` | Buffer save/restore bodies remain unported, including additional native release helpers and physical owner virtual methods. |
| `B60A10` | Existing CreateIfMissing fragment does not close the full 666-byte current hardware-layout owner operation. |
| CRT/XLive | Returning invalid-parameter behavior and actual XLive thunk/provider integration must be explicit. No empty replacement calls are allowed. |

The original constructor stores primary profile `D5F0A8` at `B3243B`. Selected
profile entries resolve these indirect routes:

| Renderer slot | Original target | Current boundary |
| --- | --- | --- |
| `+98` | `B24E70` frame targets | Typed fragment |
| `+E0` | `B23F20` vertex layout | Typed semantic binding |
| `+F0` | `B21960` gamma setter | Missing saved function; exact raw body and open math dependency documented |
| `+130` | `B24710` texture binding | Typed semantic binding |
| `+134` | `B24840` vertex stream | Typed semantic binding |
| `+138` | `B24B00` index stream | Typed semantic binding |

The `+F0` target is the independent 474-byte body `B21960..B21B3A`, whose fresh
hash matches `GAMMA_RAMP.md`. Its old power helper and exact floating-point
behavior remain unresolved. It must not be treated as the enclosing saved
pixel-constant function or replaced by an assumed modern `pow` contract.

## Ready bounded work

`native_shader_device_reset5` owns `B1FEF0`, `B5E750`, `B5E810`, `B5E890`, and
`B5E8E0`: 353 bytes total. The seven-byte getter returns actual renderer `+1A10`
without retention. Pixel/vertex shader save uses captured versus current COM
pointers, actual bytecode allocation, and both `GetFunction` calls. The saved
size DWORD starts with the incoming owner pointer bits because the prologue
uses `PUSH ECX`; it is not initialized to zero. Restore loads the actual renderer
global only after its gates, calls the correct SDK shader creation slot, frees
the current bytecode, then clears owner `+C`. The saved analysis omits the
ten-byte free-return/clear tails at `B5E8C7` and `B5E917`.

`native_hardware_layout_device_release1` owns only `B600B0..B600E4`, 52 bytes.
It preserves the initial `+40` pointer for AddRef/Release, rereads current `+40`
for the final conditional Release, and clears the field only after that call.
It does not remove tree nodes, destroy the owner, or reset descriptor fields.

Both packets can operate on actual borrowed storage with genuine existing
allocation and COM services. The complete recreation packet remains blocked
on the broader dependencies above. This report establishes static boundaries;
it provides no original-body fixture, ABI replacement, GPU-reset, or gameplay
validation for the full recreation routine.
