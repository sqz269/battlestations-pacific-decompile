# Native renderer parameter storage and live getter binding

Packet `native_renderer_parameters_owner`, based on `2bc2a97`, implements the
actual 14h parameter region used by viewport/camera construction. The startup
prefix writes that same owner before either Direct3D callback. The installed
probe carries its reference from main through font/mesh loading into the mesh
renderer. This closes the initial-dimension storage and getter dependency;
native camera installation in the draw remains a separate integration.

## Native evidence and ABI

The selected image is `battlestationspacific.exe`, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Current Ghidra project/program and the three instruction spans were rechecked
through `bsp.py ghidra`; the installed image hash was independently rechecked.
Detailed discovery and mode-change evidence remains in
[NATIVE_RENDERER_PARAMETERS_NEXT.md](NATIVE_RENDERER_PARAMETERS_NEXT.md).

| Native range | ABI and reconstructed behavior |
| --- | --- |
| `[00b32512,00b32534)` inside constructor `[00b32410,00b328f8)` | ESI is renderer, EBX=0. Five stores initialize region offsets +00/+04/+08/+0C/+10 to 0/3/0/0/0. This is a fragment, with no standalone native call ABI. |
| `[00b1ff60,00b1ff67)` | ECX renderer; EAX renderer+1A14; RET. Concrete vtable `00d5f0a8` slot +30h contains this getter. The parameter projection returns references without reading their values. |
| `[00b2af9e,00b2afaa)` inside `[00b2aeb0,00b2b1f2)` | Original width in EBP and height in ECX go to renderer+1A20/+1A24. The full native startup receives ECX renderer and ten stack arguments, RET28h. |

The region is renderer+1A14..1A27, alignment 4. `NativeRendererParametersOwner`
has exact offsets and size assertions. Offsets +01..03 and +09..0B are named
preserved byte arrays; neither default member initialization nor the initializer
zeros them. The non-dimension field names remain provisional. The complete
1D94h native renderer, singleton publication, and its construction are not
represented by `D3D9StateCache`.

## Same-owner startup and current dispatch

`d3d9_create_device_prefix_00b2aeb0` now takes an explicit parameter owner after
the startup options. It stores the original width/height arguments before
`GetDeviceCaps` at 00b2afb2 and `CreateDevice` at 00b2aff9. Zero arguments stay
zero at this boundary. The separate `D3DPRESENT_PARAMETERS` block may be changed
by callbacks; its dimensions are never copied back into the parameter owner.
The existing HRESULT adaptation remains: a nonempty device output is rejected,
and a failed caps call returns early. Native failure dereferences are not claimed.

`D3D9StateCache` retains its existing constructor and begins with an unbound
parameter dispatcher. `bind_native_renderer_parameters` explicitly installs a
borrowed `NativeRendererParameterDispatch*`; null unbinds it. This association is
independent of the render-state validity cache and is not changed by invalidate.

`NativeD3D9RendererParameterDispatch(states, owner)` binds an actual region to
that exact state identity. It rejects a different captured state and returns
references to the region's +0C/+10 fields. It owns neither the state nor region.
Both must remain alive for every getter call; a bound dispatcher must remain alive
until replaced/unbound or until the renderer ceases using it.

`D3D9ViewportRendererAccess` resolves the supplied captured state's current
dispatcher on every call and invokes it immediately. An unbound state throws an
explicit binding error. The adapter does not reload renderer publication, cache a
previous dispatch/result, snapshot dimensions, or obtain sizes from the device.
This permits a callback to change dispatch while a caller continues using an
already captured renderer. The viewport constructor retains responsibility for
its own separate publication loads for width and height.

The installed probe owns and initializes the region before startup. It passes
the same reference through `probe_installed_font`, `probe_installed_mesh`, and
`draw_mesh`, which installs a concrete dispatcher on its local `D3D9StateCache`.
The owner lives until main exits after all draw work and COM release. For the
next camera integration, use `D3D9ViewportRendererAccess` with the existing
`current_renderer` publication. Native viewport construction will observe startup
640x480; the existing dimension setter can then explicitly select the 256x256
offscreen target. This packet does not replace the diagnostic camera.

## Validation and limits

- Win32 `./scripts/build.ps1` passed, including existing CTest 1/1. An ignored
  `local/renderer_parameters_worker.cmake` explicitly added the new source to
  `bsp_core`, so this worker build compiled and linked it without editing the
  primary-owned CMake files.
- One ignored MSVC x86 native/host sequence executed the installed constructor
  fragment and getter, then startup `[00b2aeb0,00b2b01a)` with its original
  `[00b2b1d7,00b2b1f2)` epilogue appended. Only the prefix's memset target was
  redirected to host CRT; later timer/resource work was excluded. Two observed
  Direct3D callbacks compared all 14h region bytes, all 38h presentation bytes,
  and behavior flags. Callback mutations produced parameter 803x904 and separate
  presentation 777x555, with original width zero at the first callback. Padding,
  getter identity, live references, current dispatch replacement after publication
  change, and unbound/mismatched binding rejection passed.
- The existing installed-asset `bsp_d3d9_probe` passed with
  `interface/textures/menu_dxt1_2.dds` (exit 0); device startup was 640x480 with
  flags44h. This validates the parameter dataflow's integration/build compatibility;
  the current installed draw still uses its existing diagnostic camera.

No tracked tests were added. The ignored fixture/build/probe logs and exact
commands are listed in `reports/native_renderer_parameters_audit.json`.
The primary integrator owns CMake registration, shared metadata and Ghidra
annotation/export refresh. Mode changes (`00b29e60`), recreation/reset, the later
aspect-flag write at `00b2b1ba`, full renderer lifetime, original binary ABI
compatibility, and game validation remain outside this packet.
