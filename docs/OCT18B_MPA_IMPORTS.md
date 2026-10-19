# OCT18B mpA import-thunk classification

Only the 46 assigned mpA rows below are classified. The constructor at `00A47EE0` belongs to the integrator; no mpB row is included. This packet adds zero reconstructed game bodies and no C++ wrappers. All rows use `status=library_instantiation_out_of_scope`.

## Identity and evidence

- Original disk image: `I:\SteamLibrary\steamapps\common\Battlestations Pacific\battlestationspacific.exe`.
- SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`; image base `00400000`.
- Parsed the original PE import descriptors with `pefile`. For every listed VA, read exactly six bytes, required opcode `FF 25`, decoded the little-endian absolute IAT operand, and required that exact address to occur in the PE import table. All 46 passed (276 bytes).
- Before the live query batches, `bsp.py ghidra count` verified the configured project `bsp` (`C:/Users/sqz269/bsp.gpr`), program `/battlestationspacific.exe`, language and image base through the existing client; live/snapshot function counts were both 76774. `bsp.py ghidra proto --brief` queried all 46 rows in three bounded batches; every stored body range is exactly VA through VA+5.
- DLL totals: 39 xlive.dll, 2 fmodex.dll, 2 DSOUND.dll, 1 d3d9.dll, 1 d3dx9_40.dll, 1 binkw32.dll. These are executable import stubs; whether the DLL internally implements or forwards an export is outside this evidence.
- Every row was grepped individually in `include/` and `src/` by address and existing name before this document was written. A combined follow-up searched snake_case names and addresses without the leading `00`. References below identify existing interfaces or explicit adapters. Abstract methods may project arguments/results and are not machine-ABI proof.
- Correct existing Ghidra library labels were preserved. Names on ordinal-only rows are inherited Ghidra/API correlations, not names recovered from the executable import table; ordinal identity is the proven boundary. Most Ghidra signatures are `undefined name(void)` placeholders and were not treated as real prototypes. No Ghidra name, comment, listing, or saved analysis was mutated.

## Native boundary legend

- **J**: Win32 x86 indirect tail-JMP through the named IAT slot; no prologue, stack adjustment, argument adaptation, or return instruction. Incoming registers, stack arguments and caller return address pass directly to the DLL. Full callee prototype is not established by the six-byte thunk.
- **F20**: J; FMOD C++ decorated stdcall export with explicit System* plus four float* arguments (20 stack bytes), FMOD_RESULT return; current Ghidra prototype and existing abstract interface agree.
- **F8**: J; FMOD C++ decorated stdcall export with explicit System* and int* arguments (8 stack bytes), FMOD_RESULT return; current Ghidra prototype and existing abstract interface agree.
- **B8**: J; _BinkSetSoundSystem@8 decorates an eight-byte stdcall argument area. Existing host supplies the callback and one DWORD; the wrapper itself recovers no argument types.
- **D12**: J; existing NativeD3dx9Float16Import uses float* WINAPI(float*, const uint16_t*, UINT), three Win32 stack slots. The local loader is a separate typed forwarding adapter, not a reconstructed DLL body.
- **P28**: J; existing NativeReadProfileSettings is seven DWORD-sized stdcall slots (RET1Ch in prior SDK evidence), uint32_t raw status. Existing NativeProfileSettingsSdkRuntime resolves xlive.dll ordinal 5331. Existing fragment evidence remains unchanged.

## Per-row evidence

Every row is a terminal `JMP [IAT]`; the `FF 25` operand bytes below independently reproduce the IAT VA. The export column uses exact PE names or `#ordinal`.

| VA | Existing label | PE DLL / export | IAT VA | Original bytes | ABI | Existing interface or adapter reference |
|---|---|---|---|---|---|---|
| `00A4C290` | `getCPUUsage` | `fmodex.dll` / `?getCPUUsage@System@FMOD@@QAG?AW4FMOD_RESULT@@PAM000@Z` | `00CE2510` | `FF 25 10 25 CE 00` | F20 | `include/bsp/cc13_large61.hpp:263` |
| `00A4C296` | `getChannelsPlaying` | `fmodex.dll` / `?getChannelsPlaying@System@FMOD@@QAG?AW4FMOD_RESULT@@PAH@Z` | `00CE2568` | `FF 25 68 25 CE 00` | F8 | `include/bsp/cc13_large61.hpp:262`; `src/fmod_configuration_library.cpp:289 (separate C export adapter)` |
| `00A4C29C` | `DirectSoundCreate` | `DSOUND.dll` / `#1` | `00CE2058` | `FF 25 58 20 CE 00` | J | `include/bsp/engine_seg11_35_66_cc13.hpp:877` |
| `00A4C2A2` | `DirectSoundEnumerateA` | `DSOUND.dll` / `#2` | `00CE205C` | `FF 25 5C 20 CE 00` | J | `include/bsp/engine_seg11_35_66_cc13.hpp:972` |
| `00A4C2A8` | `Direct3DCreate9` | `d3d9.dll` / `Direct3DCreate9` | `00CE23DC` | `FF 25 DC 23 CE 00` | J | `include/bsp/engine_seg11_35_66_cc13.hpp:925` |
| `00A4C2AE` | `D3DXFloat16To32Array` | `d3dx9_40.dll` / `D3DXFloat16To32Array` | `00CE23E4` | `FF 25 E4 23 CE 00` | D12 | `include/bsp/native_vertex_position_read.hpp:22`; `src/native_vertex_position_read.cpp:253` |
| `00A4D356` | `_BinkSetSoundSystem@8` | `binkw32.dll` / `_BinkSetSoundSystem@8` | `00CE23D0` | `FF 25 D0 23 CE 00` | B8 | `include/bsp/movie_factory_followup.hpp:22` |
| `00A4D3F2` | `XUserSetContext` | `xlive.dll` / `#5277` | `00CE2698` | `FF 25 98 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:703` |
| `00A4D3F8` | `XEnumerate` | `xlive.dll` / `#5256` | `00CE2694` | `FF 25 94 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:1101` |
| `00A4D3FE` | `XFriendsCreateEnumerator` | `xlive.dll` / `#5312` | `00CE2690` | `FF 25 90 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:1099` |
| `00A4D404` | `XUserReadStats` | `xlive.dll` / `#5281` | `00CE268C` | `FF 25 8C 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:1110` |
| `00A4D40A` | `XUserCreateStatsEnumeratorByXuid` | `xlive.dll` / `#5286` | `00CE2688` | `FF 25 88 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:1117` |
| `00A4D410` | `XUserCreateStatsEnumeratorByRank` | `xlive.dll` / `#5284` | `00CE2684` | `FF 25 84 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:1113` |
| `00A4D416` | `XCancelOverlapped` | `xlive.dll` / `#5254` | `00CE2680` | `FF 25 80 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:1104` |
| `00A4D41C` | `XUserSetContextEx` | `xlive.dll` / `#5292` | `00CE267C` | `FF 25 7C 26 CE 00` | J | `include/bsp/engine_seg4_6_9_cc13.hpp:704` |
| `00A4D422` | `XGetOverlappedExtendedError` | `xlive.dll` / `#1082` | `00CE2678` | `FF 25 78 26 CE 00` | J | `include/bsp/xlive_sdk_adapter.hpp:37`; `src/xlive_sdk_adapter.cpp:70` |
| `00A4D428` | `XSessionWriteStats` | `xlive.dll` / `#5317` | `00CE2674` | `FF 25 74 26 CE 00` | J | `include/bsp/multiplayer_session_cc13.hpp:1383` |
| `00A4D42E` | `XGetOverlappedResult` | `xlive.dll` / `#1083` | `00CE2670` | `FF 25 70 26 CE 00` | J | `include/bsp/xlive_sdk_adapter.hpp:35`; `src/xlive_sdk_adapter.cpp:64` |
| `00A4D434` | `XNetGetXnAddrPlatform` | `xlive.dll` / `#82` | `00CE266C` | `FF 25 6C 26 CE 00` | J | `include/bsp/multiplayer_session_cc13.hpp:986` |
| `00A4D43A` | `XShowKeyboardUI` | `xlive.dll` / `#5216` | `00CE2668` | `FF 25 68 26 CE 00` | J | `include/bsp/frontend_seg16_24_28_cc13.hpp:508` |
| `00A4D440` | `XShowGamerCardUI` | `xlive.dll` / `#5252` | `00CE2664` | `FF 25 64 26 CE 00` | J | `include/bsp/frontend_menus.hpp:957` |
| `00A4D446` | `XShowMarketplaceUI` | `xlive.dll` / `#5365` | `00CE2660` | `FF 25 60 26 CE 00` | J | `include/bsp/cc13_sweepB2.hpp:234` |
| `00A4D44C` | `XShowGuideUI` | `xlive.dll` / `#5215` | `00CE265C` | `FF 25 5C 26 CE 00` | J | `include/bsp/cc13_sweepB2.hpp:232` |
| `00A4D452` | `XNetGetOpt` | `xlive.dll` / `#78` | `00CE2658` | `FF 25 58 26 CE 00` | J | `include/bsp/frontend_menus.hpp:687` |
| `00A4D458` | `XNetQosServiceLookup` | `xlive.dll` / `#71` | `00CE2654` | `FF 25 54 26 CE 00` | J | `include/bsp/frontend_menus.hpp:686` |
| `00A4D45E` | `XNetQosRelease` | `xlive.dll` / `#72` | `00CE2650` | `FF 25 50 26 CE 00` | J | `include/bsp/frontend_menus.hpp:685` |
| `00A4D464` | `XStringVerify` | `xlive.dll` / `#5303` | `00CE264C` | `FF 25 4C 26 CE 00` | J | `include/bsp/seg23_cc13.hpp:288` |
| `00A4D46A` | `XLiveOnResetDevice` | `xlive.dll` / `#5007` | `00CE2648` | `FF 25 48 26 CE 00` | J | `include/bsp/cc13_front2.hpp:140` |
| `00A4D470` | `XShowPlayerReviewUI` | `xlive.dll` / `#5214` | `00CE2644` | `FF 25 44 26 CE 00` | J | `include/bsp/ingame_gui.hpp:170` |
| `00A4D476` | `XLiveUninitialize` | `xlive.dll` / `#5003` | `00CE2640` | `FF 25 40 26 CE 00` | J | `include/bsp/app_shutdown.hpp:223` |
| `00A4D47C` | `XUserAreUsersFriends` | `xlive.dll` / `#5264` | `00CE263C` | `FF 25 3C 26 CE 00` | J | `include/bsp/multiplayer_session_cc13.hpp:323` |
| `00A4D488` | `XNetUnregisterKey` | `xlive.dll` / `#56` | `00CE2634` | `FF 25 34 26 CE 00` | J | `include/bsp/engine_seg43_45_cc13.hpp:1517` |
| `00A4D48E` | `XNetXnAddrToInAddr` | `xlive.dll` / `#57` | `00CE2630` | `FF 25 30 26 CE 00` | J | `include/bsp/multiplayer_session_cc13.hpp:695` |
| `00A4D494` | `XSocketNTOHS` | `xlive.dll` / `#38` | `00CE262C` | `FF 25 2C 26 CE 00` | J | `include/bsp/audio_online_startup.hpp:293` |
| `00A4D49A` | `XNetInAddrToXnAddr` | `xlive.dll` / `#60` | `00CE2628` | `FF 25 28 26 CE 00` | J | `include/bsp/online_seg71_cc13.hpp:1040` |
| `00A4D4A0` | `XUserReadProfileSettings` | `xlive.dll` / `#5331` | `00CE2624` | `FF 25 24 26 CE 00` | P28 | `include/bsp/native_profile_settings.hpp:8`; `src/native_profile_settings.cpp:21` |
| `00A4D4A6` | `XCloseHandle` | `xlive.dll` / `#5251` | `00CE2620` | `FF 25 20 26 CE 00` | J | `include/bsp/online_cc13.hpp:569` |
| `00A4D4AC` | `XLiveContentCreateEnumerator` | `xlive.dll` / `#5360` | `00CE261C` | `FF 25 1C 26 CE 00` | J | `include/bsp/engine_seg11_35_66_cc13.hpp:1222` |
| `00A4D4B2` | `XLiveContentGetDisplayName` | `xlive.dll` / `#5356` | `00CE2618` | `FF 25 18 26 CE 00` | J | `include/bsp/engine_seg11_35_66_cc13.hpp:1254` |
| `00A4D4B8` | `XLiveContentGetPath` | `xlive.dll` / `#5355` | `00CE26B4` | `FF 25 B4 26 CE 00` | J | `include/bsp/engine_seg11_35_66_cc13.hpp:1609` |
| `00A4D4BE` | `XLiveContentVerifyInstalledPackage` | `xlive.dll` / `#5354` | `00CE2610` | `FF 25 10 26 CE 00` | J | `include/bsp/engine_seg11_35_66_cc13.hpp:1608` |
| `00A4D4CA` | `XNetGetEthernetLinkStatus` | `xlive.dll` / `#75` | `00CE2608` | `FF 25 08 26 CE 00` | J | `include/bsp/online_seg71_cc13.hpp:23` |
| `00A4D4D0` | `XNetGetTitleXnAddr` | `xlive.dll` / `#73` | `00CE2604` | `FF 25 04 26 CE 00` | J | `include/bsp/online_seg71_cc13.hpp:714` |
| `00A4D4D6` | `XSocketSetSockOpt` | `xlive.dll` / `#7` | `00CE2600` | `FF 25 00 26 CE 00` | J | `include/bsp/online_seg71_cc13.hpp:712` |
| `00A4D4DC` | `XSocketIOCTLSocket` | `xlive.dll` / `#6` | `00CE25FC` | `FF 25 FC 25 CE 00` | J | `include/bsp/online_seg71_cc13.hpp:710` |
| `00A4D4E2` | `XSocketBind` | `xlive.dll` / `#11` | `00CE25F8` | `FF 25 F8 25 CE 00` | J | `include/bsp/online_seg71_cc13.hpp:709` |

## Verification limits

Classification used original-image bytes/import metadata and read-only live Ghidra body spans. Existing `raw_profile_defaults_00a4d4a0_r109` fragment and the existing reviewed `XUserReadProfileSettings` name/evidence were retained. The new function records coexist with that fragment. No tests were added and no game was run; this packet changes only this document and the reconstruction ledger shard. No SDK, FMOD, DirectSound, Direct3D, D3DX or Bink implementation is credited as reconstructed game logic.
