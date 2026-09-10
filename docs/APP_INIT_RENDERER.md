# Application initialize, renderer device and renderer subsystems

Addresses: 00b32410, 00a8fe30, 0073bf80, 00b14a10, 00b3c4c0, 00ad9f90, and the
join points 00b1ff50, 00b21ec0, 00b20a80, 00b21960, 00b318b0, 00b319b0,
00b2c8e0, 00b2aeb0, 00becee0.

Packet `app_init_renderer_device`, packet 5 of `APP_INITIALIZE_MAP.md`. Read-only
audit against project `bsp`, program `/battlestationspacific.exe`. No Ghidra
rename, comment, prototype or save was performed by this packet; ledger names are
recorded for the integrator to apply.

Every descriptive name below is a hypothesis, not a recovered symbol.

## Scope correction to the packet table

The packet brief places `00b14a10`, `00b3c4c0` and `00ad9f90` in phase 7.
`APP_INITIALIZE_MAP.md` places them correctly in **phase 9**, steps 50 and 51, at
`0073df01`, `0073df2c` and `0073df57`. Phase 7 ends at `0073dd2f`. The ordered
sequence below uses the map's phase numbering.

## Ordered sequence

Sites are in `0073d410 BSP_Application_Initialize`. Every renderer object below
is allocated with `operator new` (`00bf681b`), constructed only when the
allocation is non-null, and its pointer is then **discarded**: each constructor
publishes itself into a global, so nothing in the initializer holds a reference.

| # | Site | Allocation | Callee | Gate | Publishes |
| --- | --- | ---: | --- | --- | --- |
| 4a | 0073d9d3 | - | 00737c40 file-access log | `00e1ae77 != 0 && 0109cee8 == 0` | - |
| 4b | 0073da25 | 1Ch | 00be0a30 FileBlock `Skel_Init` | allocation non-null | - |
| 4c | 0073da88 | **1D94h** | `00b32410` renderer construct | allocation non-null | `00f8d394` |
| 6 | 0073dc25 | - | platform virtual +4h -> `00becee0` | - | creates the HWND, then the device |
| 7a | 0073dcbf | 4003F0h | 00a3d060 (XLive array) | `00f8abdc == 0`, then allocation non-null | not this packet |
| 7b | 0073dd04 | **2Ch** | `00a8fe30` shadow depth target | allocation non-null | `00f8bbf0` |
| 7c | 0073dd12 | - | `0073bf80` shader preload | - | fills application+8h |
| 7d | 0073dd2f | - | renderer virtual +F0h -> `00b21960` | - | gamma |
| 9a | 0073df01 | **6ACh** | `00b14a10` render resources | allocation non-null | `00f8d39c` |
| 9b | 0073df2c | **4h** | `00b3c4c0` | allocation non-null | `0108fed8` |
| 9c | 0073df57 | **14h** | `00ad9f90` | allocation non-null | `00f8c218` |
| 9d | 0073df82 | 30h | 00af1450 particle shaders | allocation non-null | not this packet |
| 9e | 0073df99 | - | `00f8c218` virtual +0Ch | - | resolves to `00ad9cb0`, a bare RET |

Phase 4 constructs the renderer but creates **no device**. Device creation happens
in phase 6, inside the window routine `00becee0`, which calls renderer primary
virtual +4h (`00b2aeb0`). That prefix is already reconstructed; see
`D3D9_STARTUP.md`. Phases 7 and 9 therefore run against a live device and a
filled capability record, and both depend on phase 5 having loaded the settings
block at `00f88980` (`008d8190` at 0073daa5).

## 00b32410 BSP_D3D9Renderer_Construct

`__thiscall(this)`, plain `RET`, returns `this` in EAX. Object size 1D94h. SEH
scope handler `00cbdef5`. Body 00b32410-00b328ec.

Order:

1. `00b32434` base `00b283f0`. It forms the `this+0Ch` subobject and, through
   `00b25f40`, stores the primary pointer in `00f8d394` (`REGISTERED_ROOT_RESOURCE.md`).
2. `00b3243b` `*this = 00d5f0a8` primary vtable, `00b32441` `*(this+0Ch) = 00d5f0a4`.
3. `00b3246c` `00b29430` on `this+34h`, the binding/state cache. It sets the
   cached gamma at `this+196Ch` to positive float zero (`GAMMA_CAPABILITIES.md`).
4. `00b32473` `this+1970h = 0`, then `00b32479` `Direct3DCreate9(20h)` into
   `this+1990h`; `this+1994h = 0`.
5. A long field-clear run over `this+1994h..1B55h`, including
   `InitializeCriticalSection(this+19F4h)`, `this+1A18h = 3`, and the three
   embedded subobject vtables `00d5f060` at `+1A60h`, `00d5f088` at `+1A74h` and
   `00d5f074` at `+1A98h`. `+1A98h` is the effect registry of
   `MATERIAL_DISPATCH.md`. `this+1B51h/1B53h/1B55h` are cleared here.
6. `00b32709` `00b0cce0` on `this+1B78h`; `00b32764` array helper `00b5e270` on
   `this+1D2Ch`; `this+1D80h = 1`, `this+1D8Ah = 1`.
7. `0108d4b8` and `0108d4b9` are cleared.
8. Three heap singletons, each `operator new` then constructor, each with the
   pointer discarded: 4CCh -> `00b1bb90`, 2Ch -> `00b585a0`
   `BSP_ShaderStateDefinitions_Initialize`, 10h -> `00b5bf70`
   `BSP_SystemConstants_ConstructRegistry`.
9. `00b3285c` `00b27d80` on `this`.
10. `00b32874` `IDirect3D9::GetAdapterIdentifier(0, 0, &identifier)` through
    IDirect3D9 vtable +14h. The identifier buffer is at `ESP+14h`.
11. `00b32883` `strstr(identifier + 200h, "NVIDIA")`. `200h` is
    `D3DADAPTER_IDENTIFIER9.Description`, so the vendor test reads the
    **Description**, not the Driver string. The boolean lands in `this+1D88h`.
    The decompiler splits the stack buffer at 1F0h and makes this look like an
    offset-496 read; the listing at `00b32876` is `lea ecx,[esp+214h]` against
    the `lea edx,[esp+14h]` passed to the query, so the difference is exactly 200h.
12. `00b32898` `this+14h = 0`; `00b3289b` `00b2c8e0` on `this`, the capability
    gather (below).
13. `00b328a2` 24h -> `BSP_RenderWorker_Construct` `00b33da0`, or 0, into `this+1970h`.
14. `00b328d3` `BSP_CriticalSection_Create` into `this+199ch`.

## Capability record at renderer+1B18h

Renderer primary virtual **+104h** is `00b1ff50`, two instructions:
`lea eax,[ecx+1B18h]; ret`. It is a plain accessor for a capability record that
`00b2c8e0` fills from `IDirect3D9::GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps)` at
`00b2c91a`. The `D3DCAPS9` buffer baseline after that call is `ESP+1F4h`
(`GAMMA_CAPABILITIES.md` anchors it through the Caps2 read at `ESP+200h`).

| Record | Renderer | Written at | Source |
| --- | --- | --- | --- |
| +00h | +1B18h | 00b2c95a | `D3DCAPS9.MaxTextureWidth` (+58h) |
| +04h | +1B1Ch | 00b2c948 | `D3DCAPS9.MaxTextureHeight` (+5Ch) |
| +08h | +1B20h | 00b2c98b | `D3DCAPS9+60h`, provisional |
| **+28h** | +1B40h | 00b2c985 | **`LOWORD(D3DCAPS9.PixelShaderVersion)`** (+CCh) |
| +2Ch | +1B44h | 00b2c977 | `LOWORD(D3DCAPS9.VertexShaderVersion)` (+C4h) |
| +30h | +1B48h | 00b2caa0 | `(pixel_shader_version < 200h) ? 1 : 2` |
| +3Bh | +1B53h | 00b2c96a | `D3DCAPS2` bit 17, `D3DCAPS2_FULLSCREENGAMMA` |
| +3Ch | +1B54h | 00b2c936 | `D3DCAPS2` bit 20, `D3DCAPS2_CANCALIBRATEGAMMA` |
| +5Ch | +1B74h | 00b2cb3c | set when `VertexShaderVersion < 0101h`; `+1B44h` is then forced to `0101h` at `00b2cb4b` |

**Correction and refinement to `APP_INIT_BOOTSTRAP.md`.** That document records
the settings tail as "the renderer is queried through `(*00f8d394)->vtbl+0x104`;
when `caps+0x28 < 0x200` the shadow pair `+0x84`/`+0x85` and `+0x90` are forced
to zero", leaving `caps+0x28` unidentified. It is
`LOWORD(D3DCAPS9.PixelShaderVersion)`, so that gate and the identical gate in
`00a8fe30` both mean **pixel shader model below 2.0 disables shadows**. Nothing
in the bootstrap description is wrong; the field now has a source.

## 00a8fe30, shadow depth target and the DF16 probe

`__thiscall(this, width, height)`, `RET 8`, returns `this`. Object size 2Ch. Base
`00a8a980` stores width at `+4h`, height at `+8h`, clears `+0Eh`, and through
`00a8a820` publishes the object into `00f8bbf0`.

The call site computes a **square** extent from one settings byte:

```
0073dccb  mov dl,[00f8899d] ; neg dl ; sbb edx,edx ; and edx,800h ; add edx,800h
```

so the extent is `2048` or `4096`, and both stack arguments receive it. Byte
`00f8899d` is settings `+1Dh`, the `HiResShadow` token (literal `00d15f04`,
verified at `008d846b`-`008d848a`). This is the first place the setting is shown
to control a concrete size.

The probe body:

1. `00a8fe82` renderer virtual +104h, then `00a8fe96`
   `cmp dword ptr [eax+28h],200h; jc` -> skip everything when the pixel shader
   version is below `ps_2_0`.
2. `00a8fead` renderer virtual +F8h with `(16h, 100h, 3, 36314644h)`. On success
   `+24h = 17h`, `+28h = 'DF16'`, `+0Eh = 0`.
3. Only if that fails, `00a8fef5` the same call with `50h` = `D3DFMT_D16`. On
   success `+24h = 17h`, `+28h = 50h`, `+0Eh = 1`.
4. If both fail the routine returns with `+24h`, `+28h` and `+0Eh` all zero.

`17h` is written identically for both formats, so it is an engine-side texture
kind, not a `D3DFORMAT`.

### Renderer virtual +F8h is a CheckDeviceFormat wrapper

`00b21ec0`, `__thiscall`, `RET 10h`, returns a byte. It calls `00b20a80` to
translate the engine resource flags, then

```
IDirect3D9::CheckDeviceFormat(0, D3DDEVTYPE_HAL, AdapterFormat=a1,
                              Usage=<translated>, RType=a3, CheckFormat=a4)
```

through IDirect3D9 vtable +28h at `00b21efb`, and returns `HRESULT >= 0`.
`00b20a80` writes a `D3DPOOL` derived from `flags & 0Fh` through EDX and a
`D3DUSAGE` mask through ECX; the wrapper forwards the **usage** and ignores the
pool. For the probe's `100h` the mask is `D3DUSAGE_DEPTHSTENCIL` (`00b20af8`),
so the two queries are ordinary depth-texture support tests against an
`X8R8G8B8` adapter format with `D3DRTYPE_TEXTURE`.

## 0073bf80 BSP_ShaderCache_PreloadFromScript

`__thiscall(this)`, plain `RET`, void. **`this` is the application object**, not
the renderer: `0073dd09` loads it from the initializer frame slot `[ESP+2Ch]`
that `0073d437` filled with the incoming ECX.

The routine builds the 1Ah-byte name `shaderfx/shaderpreload.lua`, runs it
through the Lua wrappers (`00b69d40`), opens the table key `FileNames`
(`00b67800`), logs `--- SHADER LOADING BEGIN ---`, and iterates. Each entry name
is passed to **renderer primary virtual +48h** (`00b318b0`, `RET 4`), the effect
loader that rewrites `.mshd` to `.shfx` and resolves through the registry at
renderer+1A98h (`MATERIAL_DISPATCH.md`). The returned handle is appended to the
application's vector at `+8h` (data), `+0Ch` (size) and `+10h` (capacity), grown
by doubling through `00735ec0` with a floor of 1. It then logs
`--- SHADER LOADING END  ---` and closes the table.

So the preload result is a live effect list owned by the application, and the
shader cache is the renderer effect registry rather than a separate object.

## 0073dd17, the gamma step

```
fld dword ptr [00f889e4] ; renderer virtual +F0h
```

Primary virtual +F0h is table word `00d5f198` = `00b21960`, the gamma ramp setter
of `GAMMA_RAMP.md` (`__thiscall`, one stack float, `RET 4`). `00f889e4` is
settings `+64h`. No token in `008d8190` writes `+64h` directly and no direct xref
writes the global, so on a default run it is the zero-initialised image value
`0.0f`; the only other reader is `004de8ad`. The capability gate is inside the
setter, not at this call site.

## 00b14a10 BSP_RenderResources_LoadDefaultTextures

`__thiscall(this)`, plain `RET` (`00b14f44`, and a second `RET` at `00b14f5b` on
the failed-allocation tail), returns `this`. Object size 6ACh. Ghidra's
pseudocode for this routine is truncated, so the following is read from the
listing 00b14a10-00b14f5b.

Base `00b0f020` installs vtable `00d5e158`, publishes the object into **`00f8d39c`**
and registers it with the lifetime manager (`00415350` singleton at `01090aa0`,
then `00bd0c30`). `00b14a36` then installs the derived vtable `00d5e480`.

The body is mostly float and pointer field initialisation. The parts that matter:

| Site | Operation | Destination |
| --- | --- | --- |
| 00b14bef | `00b0cd80` on `this+2ACh` | subobject |
| 00b14d72 | `operator new(40h)` -> `00b1fbb0` | `this+1D4h` |
| 00b14dac | renderer virtual +128h (`00b24dc0`, `return this+197Ch`) | fed to `00b1fab0(this+1D4h, 0, value)` |
| 00b14dc9 | renderer virtual +12Ch (`00b20090`, `return this+198Ch`) | fed to `00b1fb00(this+1D4h, value)` |
| 00b14e20 | renderer virtual +64h with `("noise.dds", 0)` | `this+668h` |
| 00b14e95 | renderer virtual +64h with `("black.tga", 0)` | `this+67Ch` |
| 00b14edd | `operator new(0CCh)` -> `00b52550` | `this+34h` |
| 00b14ef3 | 4-byte string `"XXXX"` (`00d5e460`) | `this+684h` |
| 00b14f29 | `operator new(24h)` -> `00b3c800` | `this+0Ch`, else 0 |

Renderer virtual +64h is `00b319b0 BSP_Renderer_LoadTextureByName`, `RET 8`. The
two string literals are `00d5e474` `noise.dds` and `00d5e468` `black.tga`, both
built with a resize of 9. Both are loaded unconditionally, with no
command-line or settings gate.

## 00b3c4c0 and 00ad9f90

Both are `__thiscall(this)`, plain `RET`, returning `this`, and both are pure
singleton constructors.

`00b3c4c0` (4 bytes): base `00b61c70` -> `00b61b10` installs `00d62b50`, publishes
the object into `0108fed8` and registers it with the lifetime manager; then
`00b3c4c8` installs `00d61844`. The whole object is its vtable. Slots +4h, +8h
and +0Ch are `00b3c4f0`, `00b3c500` and `00b3c510`, all bare `RET`. `0108fed8` has
no reader outside this constructor family, so the subsystem identity is **not
established**; only the registration is.

`00ad9f90` (14h bytes): base `00ad9d00` installs `00d5d284`, publishes the object
into **`00f8c218`** and registers it; then `+4h..+10h` are cleared and `00ad9fa6`
installs `00d5d2a0`. Vtable slot +8h is `00ad9fd0`, which assigns the literal
`00d5d2b8` = `"Terrain.mshd"` into a caller-supplied string, so this is the
terrain render object provider (provisional).

**New join.** `00f8c218` is exactly the global the map's step 53 calls through at
`0073df99`. Vtable `00d5d2a0` slot +0Ch is `00ad9cb0`, a bare `RET`, and the call
site pushes no argument and adjusts no stack, which is consistent. Step 53 is
therefore a no-op on the object step 51 has just created. The map lists it as an
unresolved indirect call.

## Globals

| Global | Role | Written | Read by this path |
| --- | --- | --- | --- |
| `00f8d394` | primary renderer | `00b25f9b` from `00b283f0` | 00a8fe82, 00a8fe9f, 0073dd1d, 00b14e0a, 00b14e7f, 00b14d98 |
| `00f8bbf0` | shadow depth target | `00a8a876` | `BSP_Settings_ApplyAll` 008d5ff5, `BSP_Application_Shutdown` 007380bb |
| `00f8d39c` | render resources | `00b0f076` | `BSP_Application_Shutdown` 007380a9 and render diagnostics |
| `0108fed8` | 00b3c4c0 singleton slot | `00b61b66` | none outside the ctor family |
| `00f8c218` | terrain render object | `00ad9d56` | 0073df87 |
| `0108d4b8`, `0108d4b9` | cleared by the renderer ctor | `00b327b7` | not traced |
| `00f8899d` | `HiResShadow` -> shadow extent | `008d8190` | 0073dccb |
| `00f889e4` | gamma -> virtual +F0h | not written directly | 0073dd17, 004de8ad |
| `00f8abdc` | XLive array gate | phase 7 | 0073dcb2 |

## Settings gates, and a correction to two documents

The renderer arguments the window routine forwards all come from the block at
`00f88980`, whose token table is in `APP_INIT_BOOTSTRAP.md`:

| Platform argument | Global | Settings offset | Token |
| --- | --- | --- | --- |
| Fullscreen | 00f8899e | +1Eh | `Fullscreen` |
| 4th argument | 00f889e0 | **+60h** | **`VSync`** |
| Width, height | 00f88994, 00f88998 | +14h, +18h | `Resolution` |
| Multisample | 00f889d8 | +58h | `Antialias` sample count |

**Correction.** `APP_INIT_PLATFORM.md` calls the fourth argument a "colour-depth
selector" and `D3D9_STARTUP.md` derives the presentation-sync value from a
"nonzero requested bit depth". The byte is `00f889e0` = settings `+60h`, which
`008d8190` writes from the `VSync` token (literal `00d15ef4`, byte reader
`008d9a80`, store `mov byte ptr [esi+60h],al` at `008d8487`). It is the vsync
setting, not a bit depth. The observed presentation behaviour in `D3D9_STARTUP.md`
is unaffected; only the name of the input changes. `Antialias` likewise names
the "renderer option, forwarded unchanged" of `APP_INIT_PLATFORM.md` argument 9.

Command-line gates from `APP_INIT_BOOTSTRAP.md` do not reach any renderer step in
these phases. The only gates here are the settings byte at `0073dccb`, the
capability comparison inside `00a8fe30`, and the allocation-non-null test at every
site.

## What later phases depend on this

* Phase 5 `008d8190` reads renderer virtual +104h, so the renderer must already
  exist; it does, from phase 4.
* Phase 6 window creation calls renderer virtual +4h, so device creation depends
  on phase 4 only.
* Phase 7 and phase 9 both depend on phase 6, because the capability record and
  the effect registry are populated during device initialisation.
* Phase 9 step 53 depends on step 51's `00ad9f90`.
* `BSP_Application_Shutdown` reads `00f8bbf0` and `00f8d39c`, so both objects
  outlive initialisation.

## Reconstruction state

| Address | State |
| --- | --- |
| 00b32410 | analysed; sequence and object stores recorded, not ported |
| 00a8fe30 | reconstructed and build-tested as `select_shadow_depth_format_00a8fe30` plus the extent and query helpers |
| 0073bf80 | reconstructed and build-tested as `run_shader_preload_0073bf80` |
| 00b14a10 | analysed from the listing; the two texture loads are sequenced in the port, the field initialisation is not |
| 00b3c4c0 | analysed; sequenced in the port as an allocation and a construction |
| 00ad9f90 | analysed; sequenced in the port as an allocation and a construction |
| 00b1ff50, 00b21ec0, 00b20a80, 00b24dc0, 00b20090 | analysed as contracts |
| 00b2aeb0 | already reconstructed, `D3D9_STARTUP.md`; joined here, not re-derived |

`include/bsp/renderer_startup.hpp` and `src/renderer_startup.cpp` sequence phases
4, 7 and 9 onto `RendererStartupHost`, one method per native call site, and
express the presentation request and the depth-format decision as pure functions.
`renderer_present_request_00becee0` produces the existing `D3D9StartupOptions`, so
it feeds `d3d9_create_device_prefix_00b2aeb0` directly. None of this is
ABI-compatible with the original, and none of it has been game-validated.

## Functions without a Ghidra function record

These are reached only as vtable words and have no function start in the current
program; the integrator must define them before a name can be applied.

| Address | Content |
| --- | --- |
| `00b1ff50` | `lea eax,[ecx+1B18h]; ret` - renderer virtual +104h |
| `00b21ec0` | the CheckDeviceFormat wrapper, `RET 10h` - renderer virtual +F8h |
| `00b24dc0` | `mov eax,[ecx+197Ch]; ret 4` - renderer virtual +128h |
| `00b20090` | `mov eax,[ecx+198Ch]; ret` - renderer virtual +12Ch |

## Uncertainties and what remains

* `17h` at shadow target `+24h` is an engine texture kind of unknown meaning; it
  is written for both accepted formats.
* Record field `+08h` (renderer+1B20h) is read from `D3DCAPS9+60h` on the
  baseline derived here; the surrounding fields anchor the baseline, this one
  does not, so it is provisional.
* `00f889e4` has no located writer. Whether the options screen at `004de610`
  writes it through a pointer was not traced.
* The three heap singletons inside `00b32410` (`00b1bb90`, `00b585a0`, `00b5bf70`)
  were confirmed to discard their pointers, but where each registers itself was
  not traced.
* `00b3c4c0`'s subsystem is unidentified.
* `00b14a10`'s float field initialisation, `00b52550`, `00b3c800` and `00b1fbb0`
  are not analysed.
* The renderer object layout beyond the offsets listed here is not reproduced,
  and no fullscreen path has been exercised.
