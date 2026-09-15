# Constructed renderer, native device startup and focused Reset

Addresses: `00B2AEB0`, `00B2ABD0`. Supporting focus/window reads:
`00B20C50`, `00BEC230`.

The complete reconstructed device startup now runs after the complete
`B32410` renderer constructor and before installed cold texture/effect loading.
It uses the constructor's actual D3D9 factory, renderer and singleton children.
The raw manager, string pool, allocator list, canonical resource owners and
constructor-produced pending/lost cells remain shared throughout.

`B2AEB0` creates the real HAL device, default surface owners, default states,
16 MiB dynamic vertex buffer and 1 MiB dynamic index buffer. Its final pending
request enters complete `B2ABD0`. The real Win32 focus query accepts the actual
probe window; three real 100 ms sleeps surround release, COM Reset and restore.
No focus, IAT or COM hook is installed. This successful source path took 375 ms.

The Reset-only success marker at renderer `+1D90` is 2. Both readiness bytes are
1; renderer loss and the shared pending/lost/retry state are clear. Independent
device queries verify the actual restored default-surface identities and
64x64 dimensions, D24S8 depth, and dynamic-buffer sizes, usage, pool and format.
The surface assertion reads the established `NativeSurfaceOwnerStorage::surface_2c`.

The initialized graphics pools are reused from main commit `955ebfe9`, imported
unchanged as `2d61cc1f`. Six graphics globals retain distinct storage and share
the existing allocator-list domain. Actual surface/cube/derived-hardware/tree
startup and real CRT callback registration are also used. The volume pool uses
the existing shared `B3EC60` lifecycle over its own storage. The import's strict
Win32 build and both existing CTests pass; its earlier original-byte pool fixture
is prior evidence, not a newly replayed result.

After Reset, the same device passes the installed cold-effect checks: actual
error/white DDS loads and retained bytes, all eight exact installed HAL shader
bytecodes, four primary programs, one secondary program, 28 canonical owners,
and the established reference counts/cache aliases. The shader-cache cursor
ends at 164; 14 reads return 2,301,268 bytes. Installed inputs remain unchanged.
The actual dormant control worker is destroyed and its OS thread exits with
code 0. Probe focus is restored and the native timer request is balanced.

## Evidence and boundaries

Fresh live Ghidra bytes match the installed PE for both complete parents and
the two window/focus leaves: 1,458 bytes. Another 976 bytes cover newly borrowed
profile/literal views. All 23 direct calls are checked; 21 indirect instructions
are retained. Twenty-four explicit source units are compiled with consistent
extended material contexts; 23 are selected by the final link. All production
startup/reset providers come from the rebuilt `bsp_core.lib`.

The first fixture compilation failed on reference qualification, and the first
runtime capture failed because its assertion used a texture's COM offset for
a surface owner. The corrected assertion follows the existing surface layout;
no production algorithm was changed. Failed source/executable/map/log artifacts
and the subsequent passing capture are retained with the report's immutable
source/build/runtime evidence. No permanent tests were added.

This is reconstructed-source startup and a nominal Reset, not original
startup machine-code, FH3/SEH, native caller ABI or gameplay proof. The fixture
still supplies raw renderer/CameraPlaneSet/scratch inputs, a small STATIC window,
platform HWND/activity bytes, material policy, PC/USA and a loose-file mount.
Logical and hardware owner collections are empty during this initial Reset;
cold textures/effects load afterward. Failed Reset/recreation, nonzero gamma,
fullscreen/multisampling, frame execution, drawing and complete shutdown remain
unverified. The native gamma-disabled/equal-zero gate bypasses unavailable pow
providers; concrete recreation and online adapters are bound but unreached.

## Follow-up packet

Replace the fixture's raw platform bytes and STATIC window with the native
`184h` platform constructor `BECDA0` and window startup `BECEE0`. The current
`platform_window.hpp` still describes typed projections, and `win32_window.hpp`
contains only the creation fragment. Recover their actual producer/consumer
contracts before activating the platform/control frame loop. Preserve this
complete constructor/device/Reset/effect composition as the downstream check.

Detailed evidence: `reports/native_device_effect_startup.json`.

Immutable capture: `local/checkpoints/2d61cc1f/native-device-effect-startup/validation.json`, SHA-256
`651464203c518e1b9062958b3a1a242c7a1b8bfed6e613a7086fe3ea923b37e8`. It preserves 4907 artifacts,
85 physical Win32 modules and 304 selected root source providers.
