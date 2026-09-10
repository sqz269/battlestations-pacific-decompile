# Native renderer parameters: next reconstruction boundary

Discovery packet `native_renderer_parameters_next`, 2026-09-10, based on
`8a7c825`. The next bounded implementation is the renderer-owned parameter
region, its constructor writes, its startup writes, and the live getter binding.
The current installed mesh probe receives only a device; it cannot recover the
native parameter words from that device at mesh-entry time.

This packet changes no C++, Ghidra annotations, shared ledger, factory, or reset
code. Names below describe inferred behavior. Native addresses and byte evidence
are recorded in `reports/native_renderer_parameters_next.json`.

## Verified storage and dispatch

The installed image is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`,
SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Live queries used the verified `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe` program through `tools/bsp.py ghidra`.

`0073da66` allocates **1D94h** bytes. On non-null allocation, `0073da88` calls
`00b32410` with ECX equal to that storage. The complete current Ghidra extent is
**[00b32410,00b328f8)**, including `RET` at `00b328f7`; the older
`APP_INIT_RENDERER.md` body endpoint `00b328ec` is incomplete.

Construction reaches `00b283f0`, then `00b25f40` with ECX = primary renderer+0Ch.
At `00b25f96..00b25fa0`, `LEA EAX,[EDI-0Ch]` and `MOV [00f8d394],EAX` publish
the primary pointer before singleton registration `00bd0c30`. The lifetime
manager lock, when present, surrounds publication and registration. The base
constructor subsequently installs primary table `00d5e628`; its +30h slot at
`00d5e658` is **00bf698e `__purecall`**. The concrete constructor installs
`00d5f0a8` at `00b3243b`, then initializes its fields. Therefore a non-null
global alone does not establish a constructed concrete parameter binding.

Concrete table `00d5f0a8` contains:

| Slot | Address | Behavior |
| --- | --- | --- |
| +04h | 00b2aeb0 | Device/resource initialization, ECX renderer, RET 28h |
| +2Ch | 00b1fe20 | Frame-active query |
| **+30h** | **00b1ff60** | `LEA EAX,[ECX+1A14h]; RET` |
| +34h | 00b264a0 | Adjacent slot; not the parameter getter |

The getter's complete body is `[00b1ff60,00b1ff67)`, bytes
`8d 81 14 1a 00 00 c3`. It takes ECX, no stack arguments, returns the borrowed
region address in EAX, and neither copies nor retains it.

The region ends immediately before the separate 38h-byte
`D3DPRESENT_PARAMETERS` at renderer+1A28h. A minimum layout-preserving region is
**14h bytes, aligned to four bytes**:

| Region | Renderer | Size | Constructor write | Later confirmed write |
| --- | --- | --- | --- | --- |
| +00h | +1A14h | byte | 0 at 00b32512 | aspect comparison at 00b2b1ba |
| +01h..03h | +1A15h..17h | 3 bytes | unwritten by fragment | not established |
| +04h | +1A18h | DWORD | 3 at 00b32518 | not established |
| +08h | +1A1Ch | byte | 0 at 00b32522 | not established |
| +09h..0Bh | +1A1Dh..1Fh | 3 bytes | unwritten by fragment | not established |
| **+0Ch** | **+1A20h** | **DWORD** | **0 at 00b32528** | startup/mode change |
| **+10h** | **+1A24h** | **DWORD** | **0 at 00b3252e** | startup/mode change |

The five-write fragment is `[00b32512,00b32534)`; ESI is the concrete renderer
and EBX is zero from `00b32439`. Preserve the unwritten bytes if introducing an
exact region owner. Whole-region zeroing is not the native constructor fragment.
Field names beyond width/height remain provisional.

## The initial dimensions come from startup arguments

`00b2aeb0` is `[00b2aeb0,00b2b1f2)`, ECX renderer, ten stack arguments in order:
window, fullscreen byte in a stack word, width, height, backbuffer format,
backbuffer count, multisample type, depth format, presentation sync, refresh
rate. Native epilogue is `RET 28h`.

The relevant order is:

1. Clear only `renderer+1A28h` for 38h bytes and fill presentation parameters.
2. At `00b2af9e`, store the original width argument (EBP) into renderer+1A20h.
3. At `00b2afa4`, store the original height argument (ECX) into renderer+1A24h.
4. At `00b2afb2`, call `IDirect3D9::GetDeviceCaps` through slot +38h.
5. At `00b2aff9`, call `CreateDevice` through +40h, passing the separate
   presentation block and output address renderer+1A10h.
6. At `00b2b014`, change the stored presentation `Windowed` to !fullscreen.

The dimensions are **not reread from the presentation block after CreateDevice**.
The native API may mutate that block; its contents and the viewport parameter
words can therefore diverge. Width/height zero arguments are also stored as zero
at this boundary. Substituting eventual swap-chain dimensions would change it.

The existing `d3d9_create_device_prefix_00b2aeb0` retains presentation parameters
and API failure handling, but has no storage for these two native words. Extend
that prefix to write a supplied actual region owner's fields at the above point,
before either callback. Retain its documented HRESULT adaptation; do not claim
that adaptation reproduces native failure dereferences.

The later startup byte+00h aspect flag is a separate tail after dynamic-buffer
callbacks. It compares the original unsigned width/height using x87 conversion
and division with a double at `00cf5750` (bytes `00 00 00 60 55 55 f5 3f`). The
getter/viewport constructor only requires +0Ch/+10h. Do not move the tail flag
write before callbacks or silently claim that the existing prefix implements it.

## Later mode updates are a separate dependency frontier

`00b29e60` is `[00b29e60,00b2a064)`, ECX renderer, six stack arguments: width,
height, fullscreen byte, multisample type, sync byte, force byte; returns true in
AL, `RET 18h`. Assembly resolves the decompiler's misleading full-width return
and saved-register value in optional-guard cleanup.

It enters the optional renderer guard and tracked lock+199Ch. The no-change
early return performs **no parameter-word refresh**. Otherwise nonzero requested
dimensions replace presentation width/height; zero requests leave those fields
alone. It then updates the presentation configuration and:

- calls `00b29670` synchronously when the Windowed state changes; or
- sets global pending-reset byte `0108d4b8` when Windowed is unchanged.

**After that decision/callback**, `00b29fc2` and `00b29fce` read the current
presentation width/height; `00b29fd7` and `00b29fe0` write the distinct viewport
words. Gamma virtual+F0h follows at `00b29fee`, then a fresh virtual+2Ch frame
query at `00b29ff7`, then conditional device `EndScene` at `00b2a01d`. Lock and
optional-guard cleanup follow. Preserve this observation order and the x87
gamma argument load/spill if this path is reconstructed later.

`00b29670` is `[00b29670,00b29b17)` and includes resource release, device
recreation, default states and restore callbacks. `00b2abd0` is
`[00b2abd0,00b2ae1d)` and processes pending/lost flags, window focus and Reset.
Their existing typed fragments do not establish a complete mode-change owner.
They are not prerequisites for the bounded initial-dimension binding, but must
be closed before supporting later mode updates as native renderer behavior.

Whole `.text` literal scans for displacements `20 1a 00 00` and `24 1a 00 00`
found only the constructor, startup and mode-change renderer accesses after
three false positives in stack exception-state instructions were inspected.
This is a direct-displacement audit, **not proof against every indirect or
alias-based write**.

## Ready packet and installed-probe integration

Recommended packet `native_renderer_parameter_storage`: own the new region
header/source, `include/bsp/d3d9_startup.hpp`, `src/d3d9_startup.cpp`, and exact
getter/constructor/startup fragment ledger records. Coordinate any
`D3D9StateCache` header/source binding edit and CMake change through the primary.
The native address lease need only cover `00b1ff60`, `00b32410`, `00b2aeb0`;
singleton publication and mode/reset routines remain read-only evidence.

Required behavior:

- A real owner holds the region initialized by the five native writes. The
  startup prefix writes **that same region**, and returned getter references
  continue to alias it. A pair copied from `stored`, swap-chain parameters,
  `GetViewport`, or mesh target size is not this owner.
- Associate the region and current concrete dispatch with the exact captured
  `D3D9StateCache` identity. That class is explicitly a new interface, not the
  native renderer memory layout. Do not reinterpret its address as a 1D94h
  renderer. Reject unsupported/unbound dispatch explicitly.
- `NativeViewportRendererAccess::parameters_00b1ff60` resolves the current
  dispatch and region on each call. Existing `00b1f850` code reloads publication
  separately for width and height; preserve those calls and reference reads.
- Carry the startup owner through `src/d3d9_probe.cpp` main ->
  `probe_installed_font` -> `probe_installed_mesh` -> `draw_mesh`, and associate
  it with the local state cache. The present signatures carry only the device.
  The owner must outlive all getter uses and retained camera/viewport work.
- The probe currently renders to 256x256 surfaces while startup requests
  640x480. Observe native constructor dimensions first, then use the existing
  native viewport dimension setter `00b1f940` for the explicit offscreen size.
  Do not change renderer parameter storage to impersonate the offscreen target.

Suggested concrete API for the implementation handoff:

```cpp
struct NativeRendererParametersOwner; // exactly14h; fields at native offsets
void initialize_native_renderer_parameters_00b32410_fragment(
    NativeRendererParametersOwner&) noexcept;
HRESULT d3d9_create_device_prefix_00b2aeb0(
    IDirect3D9&, const D3D9StartupOptions&, NativeRendererParametersOwner&,
    D3DPRESENT_PARAMETERS&, DWORD& behavior_flags, IDirect3DDevice9*&);
```

The owner has no implicit zero-filling field initializers. Its borrowed getter
result can reuse `NativeViewportRendererParameters` without creating another
width/height store. Keep a live parameter-owner/dispatch binding on, or explicitly
associated with, the exact `D3D9StateCache`; its concrete access implementation
must consult that current binding each time. The access adapter must not copy a
binding that can subsequently change. Root owns the later native camera
installation; parameter implementation owns only the explicit probe dataflow.

This provides a **partial renderer field reconstruction** usable by the native
viewport/camera path. It is not a reconstructed full 1D94h renderer constructor,
singleton lifetime, reset implementation, original binary ABI, or game result.
If acceptance requires that full owner, the binding packet alone cannot satisfy
it; full construction/publication/destruction remains a separate packet.

The parameter owner/getter packet is dependency-ready. The later
`native_renderer_mode_parameter_updates` packet is not ready as a complete
function until the recreate/reset and ownership boundaries are closed. After
parameter integration, existing strict Win32 build/reference checks and the
installed mesh probe are appropriate verification; this discovery adds and
runs no tests and makes no new runtime claim.
