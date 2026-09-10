# Application initialize map (0073d410)

Addresses: 0073d410, 00419cc0, 0041dd40, 0041e870, 00438e40, 00439040, 004c1400, 004c14c0, 004c9c90,
004ddb90, 004e5540, 004fc150, 005547d0, 006a7be0, 006ad0d0, 007364a0, 007366b0, 007367f0, 00736a90,
00736b60, 00736c30, 00736dd0, 00736ea0, 007372a0, 00737c40, 00737e20, 00738360, 0073bae0, 0073bf80,
0073c3b0, 0073c960, 0073cb10, 0073ce20, 00740840, 00757fe0, 0078c400, 008d4870, 008d5b50, 008d8190,
008d88e0, 00a3d060, 00a40df0, 00a4d356, 00a79230, 00a88770, 00a8fe30, 00a900f0, 00a982d0, 00aa06d0,
00aa09d0, 00aa0d30, 00ad71c0, 00ad9ac0, 00ad9f90, 00af0060, 00af0b10, 00af1450, 00b14a10, 00b32410,
00b3c4c0, 00b80a50, 00bb40b0, 00bbcb40, 00bd1510, 00bd1780, 00bd17a0, 00bd9230, 00bd9f90, 00be0660,
00be0a30, 00be1890, 00be2900, 00be7ab0, 00be80b0, 00beb2c0, 00becda0, 00beda60, 00bedfb0, 00bf55be,
00bf681b, 00bf6989, 00bf7680, 00bf79f0, 00bf9440, 00bf97cb

Packet `app_initialize_map`, owner `agent/app-init-map`. Analysis only: no C++, no build, no Ghidra
writes. Ghidra reports the body as 0073d410-0073e52c, 1160 instructions, 162 basic blocks,
208 call sites, cyclomatic complexity 81, prologue and epilogue intact.

## Class identity

Three memory checkpoints inside the function pass literal labels in ECX to `TRIV_body_004c9c90`
(tagged trivial body, a memory-tracking marker):

| Site | Label |
| --- | --- |
| 0073db30 / 0073db3c | `After Archives and RenderWindow` |
| 0073e141 / 0073e146 | `After InitGui` |
| 0073e501 / 0073e50d | `End of cSkeletonAppMidway::Init` |

The last label names the original method: `cSkeletonAppMidway::Init`. The label is a recovered
string; the C++ class layout is not recovered. Two further labels are pushed as scope names:
`persistent_data` at 0073d74d and `filestore` at 0073d7f1, each naming one of the three system
mounts, and `Skel_Init` at 0073d9fe, which becomes a `+FileBlock %s` profiling scope through
`BSP_FileBlock_Construct`.

## Ordered call sequence

Recovered from the linear disassembly of 0073d410, so this is call-site order, not execution order
across branches. Library helpers (`_memcpy`, `_memset`, `_strstr`, `_strcat_s`, `_free`, the
`LIBCRT_unmatched_00bf681b` operator-new wrapper) and the string/pool pair
`BSP_NativeString_Assign` + `BSP_SizedStoragePool_GetSingleton_Provisional` /
`BSP_SizedStoragePool_ReturnBlock_Provisional` are elided except where they carry meaning.

### Phase 0, allocator and process identity (0073d43f - 0073d4bd)

1. `00bd1780`, `00bd17a0` - two allocator hook installers (`FUN_00bd1680` plus
   Correction (docs/APP_INIT_ALLOC_STRINGS.md): `00bd1780`/`00bd17a0` are GameAlloc and GameFree, not hook installers; the site at `0073d431` allocates sixteen bytes and frees them to warm the lazy game-pool singleton.
   `FUN_00bd11f0` / `FUN_00bd1620`). Run before anything else.
2. `this+0x18 = 1`, `this+0x19 = 0`.
3. `00439040` `BSP_Application_CaptureModulePath` - `GetModuleFileNameA`, `__strlwr`, `_splitpath`
   into `DAT_00e176c8`.
4. `00be2900` - small object with vtable `PTR_FUN_00d685f4` and the constant `0x40000000`,
   allocated then constructed. Gate: allocation non-null.
5. `00bedfb0` `BSP_TimerService_Construct` - gate `DAT_01090ab0 == 0`, delegates to
   `BSP_FrameClock_Initialize`.
6. `006ad0d0` `BSP_Application_InstallCrashHandler`.

### Phase 1, command-line switches (0073d4ce - 0073d610)

7. Three rounds of `BSP_NativeString_Assign` + `_strstr` over the command line set the debug flags
   `DAT_0108d6f0` (`genshaders`), `DAT_0108d6f1` (`devshaders`), `DAT_0108d4ba` (`hiresmode`),
   `DAT_0108d4bb` (`reloadresources`); `devrr` forces `devshaders` and `reloadresources` on.
8. `0073c3b0` `BSP_SystemOptions_ProbeHardware` - `CPUSpeed`, `MemSize`, `GPUDeviceID`,
   `SoundDevice`, `options.txt`. Runs inside the first-time gate below.

### Phase 2, VFS, mounts and packages (0073d637 - 0073d894), gated on `DAT_0109ceec == 0`

9. `00beda60` `BSP_VFS_ProviderManager_Construct` into `DAT_0109ceec`; installs
   `BSP_Application_IgnoreMountFailure` at `+0x90` and `&DAT_00735b30` at `+0x8c`.
10. `004fc150` (leased to `app_shutdown`), then `BSP_VFS_RegisterProviderFactory`.
11. `00736a90` - provider factory singleton A, then `BSP_VFS_RegisterProviderFactory` again.
12. `GetCurrentDirectoryA(0xfa, ...)` + `_strcat_s(..., "\\")`, then three
    `BSP_VFS_MountSystemPath` calls at priorities 0, 99 (`persistent_data`) and 300 (`filestore`).
13. `0073cb10` `BSP_Application_MountPackages` twice (reconstructed fragment).
14. `00738360` `BSP_Application_RegisterResourceSearchPaths` (reconstructed fragment), outside the
    gate.

### Phase 3, platform, window and save storage (0073d8c0 - 0073d988)

15. `00737e20` - gate `DAT_0109cf00 == 0`; builds an object over `FUN_00736100` (leased to
    `app_shutdown`).
16. `00bbcb40` `BSP_WaterRenderer_RegisterTextureSources`.
17. `00becda0` `BSP_Win32Platform_Construct` (window platform base + text queue sentinel).
18. `00beb2c0` `BSP_SaveStorage_Initialize` - `\Battlestations-Pacific`, `\save`.
19. `DAT_00e1ae7c = param_2` (the `cachedload` argument from WinMain);
    `DAT_00e1ae78 = BSP_NativeString_Duplicate(...)` at `00438e40`.
20. `0073ce20` `BSP_Application_ParseCommandLine` - the full switch table.
21. `00736b60` - provider factory singleton B, then `BSP_VFS_RegisterProviderFactory`.
22. `00736c30` - singleton over `FUN_00bb4fb0` (0x1c bytes).
23. `00bd9230` (`this+0x88 = arg`), `00bd9f90` (`this+0x78 = byte`), `00bb40b0`
    (`DAT_010904e0 = CriticalSection`).

### Phase 4, logging scope and renderer (0073d9cc - 0073da88)

24. `00737c40` `BSP_Application_CreateFileAccessLog` - gate
    `DAT_00e1ae77 != 0 && DAT_0109cee8 == 0`, subject `files.txt`.
25. `00be0a30` `BSP_FileBlock_Construct` with the scope name `Skel_Init`.
26. `00b32410` `BSP_D3D9Renderer_Construct` - `Direct3DCreate9`, render worker, shader state
    definitions, system-constant registry. This is the device-creation point.

### Phase 5, input, settings and audio (0073da94 - 0073db2b)

27. `005547d0` `BSP_InputSettings_GetSingleton` (0x540 bytes via `FUN_006ab6b0`).
28. `006a7be0` `BSP_InputSettings_LoadDataTables` - the four control Lua tables.
29. `008d8190` `BSP_GameSettings_LoadFromRegistry` - `SOFTWARE\Eidos\Battlestations Pacific`.
30. Indirect `CALL EAX` at 0073dac0 through `DAT_01090ab0` vtable +0x24, gate `DAT_00e1ae81 != 0`.
31. `DAT_00f8d398 = DAT_00f88a0c`.
32. `00a88770` `BSP_SoundSystem_Initialize(DAT_00f889a4 == 0)`.
33. `00a79230` `BSP_DialogStreamTable_Load` - `sound/streamed_dialogs.def`.

### Checkpoint `After Archives and RenderWindow` (0073db3c)

### Phase 6, resource-manager parsers and window title (0073db41 - 0073dc7c)

34. `004c1400` `BSP_ResourceManager_GetSingleton` + `00736dd0`
    `BSP_AnimationChannelsParser_GetSingleton` + `00b80a50`
    `BSP_ResourceManager_RegisterTypeParser`.
35. The same triple with `00736ea0` `BSP_BoneParser_GetSingleton`.
36. `DAT_00f8bbcc+0x218/+0x21c` receive `DAT_00f889a8` / `DAT_00f889b0`.
37. Window title `"Battlestations Pacific"` pushed through `DAT_0109cf04` vtable slot 4 with nine
    arguments (resolution, fullscreen, and the settings globals read in step 29).
38. `00a40df0` `BSP_XLiveSystem_Initialize`; then `*(DAT_00f8abe8+0x18) = FUN_00737d60`.

### Phase 7, renderer subsystems and shaders (0073dcbf - 0073dd12)

39. `CG_array_ctor_helper_00a3d060`, gate `DAT_00f8abdc == 0`.
40. `00a8fe30` - queries the renderer (`DAT_00f8d394` vtable +0x104 and +0xf8) for FourCC
    `0x36314644` (`DF16`) with a 0x200 size threshold; a depth or shadow format probe.
41. `0073bf80` `BSP_ShaderCache_PreloadFromScript` - `shaderfx/shaderpreload.lua`.
42. Indirect `DAT_00f8d394` vtable +0xf0.

### Phase 8, input devices (0073dd5b - 0073ddd5)

43. `CG_array_ctor_helper_0078c400`, gate `DAT_00f871b4 == 0`; `TRIV_body_008d88e0`.
44. `00a982d0` `BSP_DirectInput_CreateDevices` - `DirectInput8Create`.
45. `*(DAT_00f8bbf4+0xd8) = &DAT_004b4630`; `00a900f0` `BSP_InputDeviceTable_ResetSlots`.
46. `007364a0` (leased to `app_shutdown`) constructs a 7-slot object with vtable
    `PTR_CG_scalar_deleting_dtor_00737f10_00cfeae8`.

### Phase 9, world content (0073de42 - 0073df82)

47. `00af0060` `BSP_TextureAtlas_Load("interface/textures/common.ats")`.
48. `00740840` `BSP_DecalSystem_LoadDefinitions`.
49. `00ad9ac0` `BSP_FoliageSystem_LoadTypes`; `00ad71c0` iterates the range
    `this+0x58 .. this+0x5c` and publishes `DAT_00f8c20c`.
50. `00b14a10` `BSP_RenderResources_LoadDefaultTextures` - `noise.dds`, `black.tga`.
51. `00b3c4c0`, `00ad9f90` - two small vtable-only constructors.
52. `00af1450` `BSP_ParticleSystem_LoadShaders`.
53. Indirect `DAT_00f8c218` vtable +0xc.

### Phase 10, movie, network and GUI (0073dfbc - 0073e14b)

54. `007366b0` (leased to `app_shutdown`) then `_BinkSetSoundSystem@8(&DAT_00a4d35c)` - Bink video
    bound to the game sound system.
55. `007367f0` (leased to `app_shutdown`).
56. `00af0b10` - object with a critical section, seeded from `DAT_00d7a24c`.
57. `0073c960` - a 0x4040-byte object with a 0x4000 zeroed table, over `FUN_00736540`.
58. `008d4870` `BSP_GameSettings_GetLanguageName`, feeding
    `00aa09d0` `BSP_Localization_LoadTable` (`lockit/`).
59. `CG_adjustor_thunk_00aa0d30("globals")`, then `00aa06d0` (second `lockit/` consumer).
60. `0073bae0` `BSP_FontSystem_LoadDefinitions` - `Fonts/Fonts.lua`.

### Checkpoint `After InitGui` (0073e146)

61. `004c14c0` - 4-byte singleton with vtable `PTR_LAB_00ce752c`.

### Phase 11, game object and datatable preload (0073e155 - 0073e45b)

62. Allocate and zero a 0x71a0-byte object, construct it with `CG_array_ctor_helper_004ddb90`,
    store at `this+0x14`, copy `this+0x1a` into `+0x719d`.
63. Five identical preload blocks, in this order: `scripts/datatables/inputs.lua`,
    `keyboardsetup.lua`, `controllerinputnames.lua`, `controlpresets.lua`, `scoring.lua`. Each is
    `BSP_NativeString_Resize` + `_memcpy` + `004fc150` + `BSP_FileStoreFactory_GetOrCreate` +
    `BSP_FileStore_CacheFromVFS`. Already covered by docs/STARTUP_SCRIPT_PRELOAD.md.

### Phase 12, game entry and tail (0073e45c - 0073e52c)

64. `if (DAT_00e1ae78) { _free(DAT_00e1ae78); DAT_00e1ae78 = NULL; }` - see the flow break below.
65. `004e5540` `BSP_Game_OnInit` - `GGame::OnInit`, `GGame::OnInitOnce`, `GGame::OnInitTitle`,
    `Scripts\global\luaMW_init.lua`.
66. `this+4 = 1`.
67. `008d5b50` `BSP_PostEffectSystem_Initialize` with `ECX = 0x00f88980`.
68. `METRICS_wrapper_00757fe0` over a 0x23c-byte object.
69. `local_134` virtual slot 4 if non-null; `007372a0` registers a small object with the singleton
    lifetime manager.
70. Checkpoint `End of cSkeletonAppMidway::Init`, then `RET 0x8`.

## Callee inventory

Status values: `reconstructed` = a `[BSP reconstruction]` fragment exists; `tagged` = library or
codegen tag, not application code; `named-only` = has a ledger name but no reconstruction;
`untouched` = still `FUN_`. Thirty callees were still `FUN_` before this packet and now carry a
provisional reviewed name (marked *new*); the `Status` column records the state they were in when
the packet started.

| Address | Name | Status | Subsystem | Behaviour | Confidence |
| --- | --- | --- | --- | --- | --- |
| 00419cc0 | BSP_SizedStoragePool_GetSingleton_Provisional | named-only | memory | 2857 callers; pool lookup paired with the release below | high |
| 00bd1510 | BSP_SizedStoragePool_ReturnBlock_Provisional | named-only | memory | 2851 callers; returns a block to the sized pool | high |
| 0041dd40 | BSP_NativeString_Resize | named-only | strings | 2280 callers; grows the native string buffer | high |
| 0041e870 | BSP_NativeString_Assign *new* | named-only | strings | resize + memcpy from a literal | high |
| 00438e40 | BSP_NativeString_Duplicate *new* | untouched | strings | strlen, new, memcpy; result freed at 0073e466 | high |
| 00bd1780 | FUN_00bd1780 | untouched | memory | allocator hook install, runs first | low |
| 00bd17a0 | FUN_00bd17a0 | untouched | memory | second allocator hook install | low |
| 00be0a30 | BSP_FileBlock_Construct | named-only | telemetry | `+FileBlock %s` profiling scope, here `Skel_Init` | high |
| 00439040 | BSP_Application_CaptureModulePath *new* | untouched | app bootstrap | GetModuleFileNameA + strlwr + splitpath | high |
| 006ad0d0 | BSP_Application_InstallCrashHandler *new* | untouched | app bootstrap | registers `&LAB_006ad0c0` via FUN_00bd4fc0 | medium |
| 0073ce20 | BSP_Application_ParseCommandLine *new* | untouched | app bootstrap | 13 switch tokens incl. `cachedload`, `filelog`, `nozip` | high |
| 0073c3b0 | BSP_SystemOptions_ProbeHardware *new* | untouched | app bootstrap | CPUSpeed/MemSize/GPUDeviceID/SoundDevice, options.txt | high |
| 008d8190 | BSP_GameSettings_LoadFromRegistry *new* | untouched | settings | `SOFTWARE\Eidos\Battlestations Pacific`, 20 keys | high |
| 008d4870 | BSP_GameSettings_GetLanguageName *new* | untouched | settings | stride-0x20 table lookup with fallback | medium |
| 00737c40 | BSP_Application_CreateFileAccessLog *new* | untouched | telemetry | `files.txt`, gated by the `filelog` switch | medium |
| 00beda60 | BSP_VFS_ProviderManager_Construct *new* | untouched | VFS | builds DAT_0109ceec, registers a provider factory | high |
| 00be0660 | BSP_VFS_RegisterProviderFactory | named-only | VFS | list insert; called three times | high |
| 00736a90 | FUN_00736a90 | untouched | VFS | provider factory singleton A, vtable 00cfe9fc | medium |
| 00736b60 | FUN_00736b60 | untouched | VFS | provider factory singleton B, vtable 00cfea00 | medium |
| 00736c30 | FUN_00736c30 | untouched | unknown | singleton over FUN_00bb4fb0, 0x1c bytes | low |
| 00737e20 | FUN_00737e20 | untouched | unknown | gate DAT_0109cf00, base FUN_00736100 (leased) | low |
| 00be1890 | BSP_VFS_MountSystemPath | reconstructed | VFS | three mounts at priority 0, 99, 300 | high |
| 0073cb10 | BSP_Application_MountPackages | reconstructed | packages | two package scans | high |
| 00738360 | BSP_Application_RegisterResourceSearchPaths | reconstructed | packages | search-path defaults | high |
| 00be80b0 | BSP_FileStoreFactory_GetOrCreate | reconstructed | file store | five preload blocks | high |
| 00be7ab0 | BSP_FileStore_CacheFromVFS | reconstructed | file store | five preload blocks | high |
| 004fc150 | FUN_004fc150 | untouched, leased to app_shutdown | resources | request builder used by every preload block | n/a |
| 004c1400 | BSP_ResourceManager_GetSingleton | named-only | resource manager | lazy singleton, called twice | high |
| 00736dd0 | BSP_AnimationChannelsParser_GetSingleton | reconstructed | resource manager | parser singleton, vtable 00cfea38 | high |
| 00736ea0 | BSP_BoneParser_GetSingleton | reconstructed | resource manager | parser singleton, vtable 00cfea48 | high |
| 00b80a50 | BSP_ResourceManager_RegisterTypeParser | reconstructed | resource manager | registers the two parsers | high |
| 00bbcb40 | BSP_WaterRenderer_RegisterTextureSources *new* | untouched | renderer/water | Caustics + ShoreWave texture sources | medium |
| 00becda0 | BSP_Win32Platform_Construct | named-only | platform/window | window platform base + text queue sentinel | high |
| 00beb2c0 | BSP_SaveStorage_Initialize *new* | untouched | save/profile | `\Battlestations-Pacific\save` | high |
| 00bedfb0 | BSP_TimerService_Construct *new* | untouched | timing | wraps BSP_FrameClock_Initialize | medium |
| 00be2900 | FUN_00be2900 | untouched | unknown | vtable 00d685f4, constant 0x40000000 | low |
| 00bd9230 | FUN_00bd9230 | untouched | memory | `this+0x88 = arg` setter | low |
| 00bd9f90 | FUN_00bd9f90 | untouched | memory | `this+0x78 = byte` setter | low |
| 00bb40b0 | FUN_00bb40b0 | untouched | threading | `DAT_010904e0 = CriticalSection` | medium |
| 00b32410 | BSP_D3D9Renderer_Construct | named-only | renderer/D3D9 | Direct3DCreate9, render worker, shader states | high |
| 00a8fe30 | FUN_00a8fe30 | untouched | renderer/D3D9 | probes FourCC `DF16` depth format on the device | medium |
| 0073bf80 | BSP_ShaderCache_PreloadFromScript *new* | untouched | renderer/shaders | shaderfx/shaderpreload.lua | high |
| 00b14a10 | BSP_RenderResources_LoadDefaultTextures *new* | untouched | renderer/textures | noise.dds, black.tga | medium |
| 00b3c4c0 | FUN_00b3c4c0 | untouched | renderer | vtable-only ctor over FUN_00b61c70 | low |
| 00ad9f90 | FUN_00ad9f90 | untouched | renderer/terrain | vtable-only ctor over FUN_00ad9d00 | low |
| 00af0060 | BSP_TextureAtlas_Load *new* | untouched | renderer/textures | `Loading atlas: %s`, common.ats | high |
| 00af1450 | BSP_ParticleSystem_LoadShaders *new* | untouched | particles | particleaxialsprite shaders, atl_all.dds | high |
| 00740840 | BSP_DecalSystem_LoadDefinitions *new* | untouched | decals | scripts/datatables/decals.lua | high |
| 00ad9ac0 | BSP_FoliageSystem_LoadTypes *new* | untouched | foliage | Effects\foliage\FoliageTypes.lua | high |
| 00ad71c0 | FUN_00ad71c0 | untouched | foliage | walks this+0x58..0x5c, sets DAT_00f8c20c | low |
| 00af0b10 | FUN_00af0b10 | untouched | unknown | object with a critical section, seed DAT_00d7a24c | low |
| 005547d0 | BSP_InputSettings_GetSingleton *new* | untouched | input | 0x540 singleton over FUN_006ab6b0 | high |
| 006a7be0 | BSP_InputSettings_LoadDataTables *new* | untouched | input | four control Lua tables | high |
| 00a982d0 | BSP_DirectInput_CreateDevices *new* | untouched | input | DirectInput8Create | high |
| 00a900f0 | BSP_InputDeviceTable_ResetSlots *new* | untouched | input | 3x8 slots, virtual +0x14 | medium |
| 00a88770 | BSP_SoundSystem_Initialize *new* | untouched | sound | `Out of sounjd memory:` | high |
| 00a79230 | BSP_DialogStreamTable_Load *new* | untouched | sound | sound/streamed_dialogs.def | high |
| 00a4d356 | _BinkSetSoundSystem@8 | tagged | movie | Bink bound to the game sound system | high |
| 00a40df0 | BSP_XLiveSystem_Initialize *new* | untouched | network/XLive | XenonSystemManager log lines | high |
| 00aa09d0 | BSP_Localization_LoadTable *new* | untouched | localization | `lockit/`, `Content file name for %s` | medium |
| 00aa06d0 | FUN_00aa06d0 | untouched | localization | second `lockit/` consumer, takes a char flag | low |
| 00aa0d30 | CG_adjustor_thunk_00aa0d30 | tagged | localization | called with the table name `globals` | high |
| 0073bae0 | BSP_FontSystem_LoadDefinitions *new* | untouched | GUI/fonts | Fonts\, Fonts/Fonts.lua | high |
| 0073c960 | FUN_0073c960 | untouched | GUI | 0x4040-byte object with a 0x4000 zeroed table | low |
| 004c14c0 | FUN_004c14c0 | untouched | GUI | 4-byte singleton reached at the After InitGui mark | low |
| 004e5540 | BSP_Game_OnInit *new* | untouched | game logic | GGame::OnInit/OnInitOnce/OnInitTitle | high |
| 008d5b50 | BSP_PostEffectSystem_Initialize *new* | untouched | renderer/post | PostEffectSysCam/Obj, distort and adaptation shaders | medium |
| 007372a0 | FUN_007372a0 | untouched | unknown | tail singleton registration, vtable 00cfea68 | low |
| 007364a0 | FUN_007364a0 | untouched, leased to app_shutdown | unknown | 7-slot object built at 0073ddd5 | n/a |
| 007366b0 | FUN_007366b0 | untouched, leased to app_shutdown | movie | built immediately before BinkSetSoundSystem | n/a |
| 007367f0 | FUN_007367f0 | untouched, leased to app_shutdown | unknown | built at 0073dffa | n/a |
| 004c9c90 | TRIV_body_004c9c90 | tagged | telemetry | memory checkpoint marker, label in ECX | high |
| 008d88e0 | TRIV_body_008d88e0 | tagged | unknown | trivial body | n/a |
| 00757fe0 | METRICS_wrapper_00757fe0 | tagged | telemetry | block metrics over a 0x23c object | n/a |
| 004ddb90 | CG_array_ctor_helper_004ddb90 | tagged | game logic | constructs the 0x71a0 game object | n/a |
| 0078c400 | CG_array_ctor_helper_0078c400 | tagged | unknown | gated by DAT_00f871b4 | n/a |
| 00a3d060 | CG_array_ctor_helper_00a3d060 | tagged | network/XLive | gated by DAT_00f8abdc | n/a |
| 00bf55be, 00bf681b | LIBCRT_unmatched_00bf681b | tagged | CRT | operator new wrappers | n/a |
| 00bf6989, 00bf7680, 00bf79f0, 00bf9440, 00bf97cb | _free, _memcpy, _memset, _strstr, _strcat_s | tagged | CRT | library | n/a |

## Gates and early returns

There is no early `return` in the real control flow. Every branch is a skip-this-block guard, so a
failure of any one subsystem leaves the rest of initialization running. The guards are:

| Guard | Skipped work |
| --- | --- |
| allocation result non-null (25 sites) | each `operator new` is followed by `if (p) ctor(p)`; a null allocation silently skips the construction and leaves the owning global null |
| `DAT_01090ab0 == 0` | timer service construction (0073d4a5) |
| `DAT_0109ceec == 0` | the whole VFS block: provider manager, provider factories, the three mounts, both package scans (0073d604 - 0073d88d) |
| `DAT_0109cf00 == 0` | the 00737e20 object (0073d8ac) |
| `DAT_00e1ae77 != 0 && DAT_0109cee8 == 0` | the files.txt access log (0073d9b4) |
| `DAT_00e1ae81 != 0` | the indirect `DAT_01090ab0` vtable +0x24 call (0073dab8) |
| `DAT_00f8abdc == 0` | XLive array construction (0073dcb2) |
| `DAT_00f871b4 == 0` | the 0078c400 array construction (0073dd4e) |
| `DAT_00e1ae78 != 0` | the free of the duplicated string at 0073e463 only |
| command line contains `devrr` | forces `devshaders` and `reloadresources` on |

Ordering constraints that actually gate later work, as opposed to being skipped:
`BSP_VFS_ProviderManager_Construct` must precede every mount and both package scans; the mounts must
precede `BSP_Application_RegisterResourceSearchPaths` and the five datatable preloads;
`BSP_D3D9Renderer_Construct` must precede the `DF16` format probe, the shader preload, the default
textures and the particle shaders; `BSP_GameSettings_LoadFromRegistry` must precede the window title
call at 0073dc25 and `BSP_GameSettings_GetLanguageName`; the localization table and fonts must
precede the `After InitGui` checkpoint; everything precedes `BSP_Game_OnInit`.

## Flow breaks and gap markers

Reported, not fixed. Ghidra is read-only for this packet.

1. **0073e46b - 0073e473, nine bytes not disassembled inside 0073d410.** `_free` (00bf6989) behaves
   as a no-return in the listing: disassembly stops after `CALL 0x00bf6989` at 0073e466 and resumes
   at 0073e474. The raw bytes are `83 c4 04` (`ADD ESP,0x4`) then `89 3d 78 ae e1 00`
   (`MOV [0x00e1ae78],EDI`, with EDI zero). The decompiler renders the gap as a spurious
   `return;` immediately before `FUN_004e5540`, which makes it look as though `BSP_Game_OnInit` and
   the entire tail are unreachable when `DAT_00e1ae78` is non-null. They are not: the real code
   frees the string, nulls the global and falls through.
2. **`WARNING: Globals starting with '_' overlap smaller symbols at the same address`** on the
   0073d410 decompilation header.
3. **`WARNING: Removing unreachable block`** in three claimed callees: 00737e20 (00737e68),
   00b14a10 (00b14e69) and 008d5b50 (008d5db2, 008d5cf8, 008d5d34, 008d5e64).
4. **Unaffected register inputs**, a sign the same no-return propagation truncated the recovery:
   `unaff_EBX`, `unaff_ESI`, `unaff_EDI` in 00b14a10 and `unaff_ESI` in 008d5b50. Both bodies need
   the assembly listing, not the pseudocode.
5. **Six indirect call sites** are not resolved by the callee list and are not covered by any
   proposed packet: `CALL dword ptr [0x00ce2264]` at 0073d697 (an import thunk), `CALL EAX` at
   0073dac0 and 0073dc25, `CALL EDX` at 0073dd2f, 0073df99 and 0073e4c7. The 0073dc25 site is the
   window-title call and is the highest-value one to resolve.

## Proposed follow-up packets

Ten packets covering all 59 claimed addresses, ordered so that a runnable startup comes together
earliest. Each is analysis-plus-reconstruction sized for one worker turn.

| # | Packet | Addresses | Owned files | Contract | Depends on |
| --- | --- | --- | --- | --- | --- |
| 1 | `app_init_alloc_string_helpers` | 0041e870, 00438e40, 0041dd40, 00419cc0, 00bd1510, 00bd1780, 00bd17a0, 00be0a30 | docs/APP_INIT_ALLOC_STRINGS.md, reports/app_init_alloc_strings.json | Recover the native-string and sized-pool helper layer plus the two allocator hook installers and the FileBlock profiling scope, so every later packet has a stable string and allocation contract. | - |
| 2 | `app_init_bootstrap_options` | 00439040, 006ad0d0, 0073ce20, 0073c3b0, 008d8190, 00737c40 | docs/APP_INIT_BOOTSTRAP.md, reports/app_init_bootstrap.json | Recover module-path capture, the crash-handler install, the 13-token command line, the hardware probe and the registry-backed game settings; enumerate every global these set and which later gate reads it. | 1 |
| 3 | `app_init_vfs_and_parsers` | 00beda60, 00be0660, 00736a90, 00736b60, 00736c30, 004c1400, 00736dd0, 00736ea0 | docs/APP_INIT_VFS_SINGLETONS.md, reports/app_init_vfs_singletons.json | Recover the VFS provider manager, the two provider-factory singletons and the registration path, and join them to the resource-manager singleton and the two model parsers it registers. | 2 |
| 4 | `app_init_platform_window` | 00becda0, 00beb2c0, 00bedfb0, 00be2900, 00bd9230, 00bd9f90, 00bb40b0, 00737e20 | docs/APP_INIT_PLATFORM.md, reports/app_init_platform.json | Recover the Win32 platform and window object, the save-directory root, the timer service and the memory-manager and critical-section knobs set around them. | 3 |
| 5 | `app_init_renderer_device` | 00b32410, 00a8fe30, 0073bf80, 00b14a10, 00b3c4c0, 00ad9f90 | docs/APP_INIT_RENDERER.md, reports/app_init_renderer.json | Recover D3D9 device and renderer construction, the DF16 depth-format probe, the shaderpreload.lua cache fill and the fallback textures. Read the assembly for 00b14a10, whose pseudocode is truncated. | 4 |
| 6 | `app_init_input` | 005547d0, 006a7be0, 00a982d0, 00a900f0 | docs/APP_INIT_INPUT.md, reports/app_init_input.json | Recover the input settings singleton and its four Lua control tables, DirectInput device creation and the 3x8 device-slot reset. | 2 |
| 7 | `app_init_audio_online` | 00a88770, 00a79230, 00a40df0 | docs/APP_INIT_AUDIO_ONLINE.md, reports/app_init_audio_online.json | Recover sound-system startup, the streamed-dialog definition table and the XenonSystemManager/XLive bring-up including its notification listener. | 4 |
| 8 | `app_init_locale_gui` | 008d4870, 00aa09d0, 00aa06d0, 0073bae0, 0073c960, 004c14c0 | docs/APP_INIT_LOCALE_GUI.md, reports/app_init_locale_gui.json | Recover language selection, the two lockit table loads and the `globals` table, font definitions from Fonts.lua and the GUI singleton reached at the After InitGui checkpoint. | 3, 5 |
| 9 | `app_init_world_effects` | 00af0060, 00740840, 00ad9ac0, 00ad71c0, 00af1450, 00bbcb40, 00af0b10 | docs/APP_INIT_WORLD_EFFECTS.md, reports/app_init_world_effects.json | Recover the atlas loader, decal and foliage definition tables, particle shader and atlas set, and the water caustics/shore-wave texture sources. | 5 |
| 10 | `app_init_game_entry` | 004e5540, 008d5b50, 007372a0 | docs/APP_INIT_GAME_ENTRY.md, reports/app_init_game_entry.json | Recover GGame::OnInit and its OnInitOnce/OnInitTitle split, the post-effect system bring-up and the final lifetime registration that closes cSkeletonAppMidway::Init. Read the assembly for 008d5b50, whose pseudocode drops four blocks. | 6, 7, 8, 9 |

Excluded from all ten because another worker holds them: 004fc150, 007364a0, 007366b0, 007367f0
(`app_shutdown`). Packet 3 and the five preload blocks in phase 11 both touch 004fc150; whoever
picks up packet 3 should coordinate rather than claim it.
