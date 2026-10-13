# CC13 pass over segment 56 (packet cc13-sys5)

Packet cc13-sys5, lineage cc13. Range: segment 56 (`0088B410..008DCD40`) without
`00880000..0088FFFF`, which cc9 holds for shallow water. Lease: `00890000-008DCD40`.
Every name is a descriptive hypothesis, not a recovered symbol. Code:
`include/bsp/engine_seg56_cc13.hpp`, `src/engine_seg56_cc13.cpp` (registered in
`cmake/startup.cmake`).

## Scope

The index listed 490 functions in `00890000..008DCD40` with a reviewed ledger name and neither a
reconstruction record nor a tag. 414 of them are mission Lua binding handlers of table `00E0B7B8`
(`config/lua_bindings.json`, named by `docs/CC13_BAND_0080_008F.md` and `docs/CC13_GP_0085_008F.md`).

| Group | Count | Action |
| --- | --- | --- |
| Presentation, tooling, input, GUI, localisation and environment bindings | 82 | reconstructed, build-tested, recorded |
| Non-binding helpers (colours, objective entry reads, dotted GUI path, language list, perf registry) | 13 | reconstructed or modelled, recorded |
| Existing reconstructions without records (`lua_binding_core.cpp`, `game_settings.cpp`) | 12 | backfilled |
| Scene tokenizer bodies modelled by `scene_file.cpp` / `scene_property_bag.cpp` | 5 | recorded as `partial_projection` |
| `STL_*` / template bodies | 23 | tagged (`stl_shape`, `tmpl_shape`) |
| Gameplay bindings and the rest | 355 | left to owners (below) |

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| 82 bindings in `engine_seg56_cc13.cpp` | yes | yes (between the machine prologue and epilogue) | yes | no |
| Perf-timing registry `008D1A10/008D1AB0/008D1B80` | yes | object semantics only (`PerfTimingRegistry`) | yes | no |
| Helpers `008DA920/970/A60`, `008DACD0/AD30/ADC0/B990`, `008C8C20/9F80`, `008D76C0` | yes | yes | yes | no |
| Backfilled `lua_binding_core.cpp` (9) and `game_settings.cpp` (3) | yes | earlier packets | yes | no |

Nothing is ABI compatible or game-validated. No game runs were made.

## Method

Every binding is `__fastcall(lua_State* in ECX)`, returns its result count in EAX and runs the
machine prologue and epilogue of `include/bsp/lua_binding_core.hpp` (`00B66C00`, `00B679B0`,
`00B677E0`, `00B66400`, `00B669A0`), plus the one-time `luakod` log-category init. The C++ models
only the body between them, over the existing `LuaBindingArgumentReader`/`LuaBindingResultWriter`
interfaces, extended by `LuaArgs`/`LuaResults` for the accessors these bindings use
(`00B66000`, `00888760`, `00B662D0`, `00B662F0`, `00B66330`, `00B66480`, `00B66710`/`00B672B0`,
`00B67930` + `0088BA30`). Host methods are named by the callee they stand for; the object each
method runs on (`ECX`) is recorded beside it in the header.

Evidence per binding came from three views, one at a time: a compacted export (the plumbing
stripped by a script), a call-site listing (the four instructions before each non-plumbing call
plus every x87 and absolute-address instruction, from the disk bytes through Capstone), and
`disasm-raw` where argument order or a branch mattered. Each record's evidence names the
instructions it rests on.

Points the listing settled that the pseudocode did not:

- Two-argument float bindings (`SetDOF` 00897210, `MovCam_SetSpeedLimits` 008B6D20,
  `HideScoreDisplay` 008C24B0) read argument 1 first (ESI) and argument 0 second (EDI), then pass
  `(arg0, arg1)`. The read order is kept.
- `FreeCameraSetPos` 008923B0 passes `(arg4 rad, arg3 rad, 0)` to `00467050`: after `SUB ESP,0Ch`
  the pushes read `[esp+1Ch]` (arg4) as the first parameter and `[esp+44h]` (arg3) as the second.
- `MapHideNodeByName` 008C03B0 calls `005A4020(name arg0, 00B662F0(arg1, default 0))`.
- `AddListener` 008C6760 reads arg1 before arg0 and builds the LuaObject for arg2 directly in the
  outgoing argument area; the call is `00980C10` on `[00F8A0C4]` with `(arg0, arg1, arg2)`.
- `SoundFade` 008D16B0: the clamp is `FCOMPI` of the double 0.01 (`00D7A358`) against the duration
  with `JBE` keeping it, so a duration below 0.01f becomes the float 0.01f (`00D7A238`); the rate
  is `|delta / duration|` by clearing the sign bit (`AND 7FFFFFFFh`).
- `SetRainDropNumPercent` 008C7F50 stores the divided value in both places: the `FSTP` at 008C801B
  overwrites the raw value's slot before the second load.
- `EnableInput` 008AFBD0: level `(-(b != 0) & FFFFFFFBh) + 5`, so 0 when enabled and 5 when
  disabled, on input context 1Eh. `GameOver` writes the same level 5.
- `color_blend_from_black_by_index` 008DAA60 stores the 0.5 scale (`00CE3800`) in every branch,
  after the index test; the results are (0,0,0.5,1), (0,0.5,0,1), (0.5,0,0,1) or black.

Constants were read from the PE with `tools/pe_const_read.py`: `00CE3D28` (double pi of float),
`00CE3D20` 180.0, `00CE4B48` 255.0, `00CE47A0` 1000.0, `00CEDAE8` 1/1024, `00D7A220` 100.0,
`00D7A24C` 1.0f, `00D7A218` 0.0f, `00CE3800` 0.5f, `00CF8FA4` "pos", `00D14188` "rot",
`00CE3A70` ".".

## Second batch

Added after the first commit, same method:

- Camera: `GetCameraRelPosW` 008BF230, `GetCameraRelPosL` 008BF460, `GetCameraState` 008BF6A0
  (Position is camera+30h, not the +120h world position GetCameraPosRot uses; each angle is
  `(float)(a + 42 pi as qword 00D13268)` fmod `2 pi` (qword 00CE3828), then degrees; Zoom is
  float 00F889B4 over camera+1C4h), `GetDistFromCrosshair` 008C0560 (00794750 with the camera
  direction first and the entity offset second, `sqrt(a*a + b*b)` above 1e-10, over the FOV).
- The thirteen `MovCam_*` mode setters `008B2CC0..008B6AD0`. They share an optional tail
  (boolean default true, number default 0, string default "", then every further argument as
  a Lua variant vector built by 00887370 + 006EDF00), modelled once as `read_movcam_tail`.
  `005CD160` is `__fastcall(movcam)` with no stack argument: values the decompiler showed passed
  to it are pushes for the setter that follows. `MovCam_SetZoom` was corrected accordingly.
  `MovCam_AddInterpolator` reads arguments 4, 3, 2, 1 in that order (EDI, EBP, EBX,
  [ESP+24h]) and passes them as `(name, arg1, arg2, arg3, arg4)` to 007900B0.
- Glue: `UnloadMessageMap` 008C6490 (its session message passes 1 where LoadMessageMap passes
  0), `GetConfiguredInput` 008CC030 (two string tables), `ForceEnableInput` 008CBAD0,
  `UnloadClass` 008CC630.
- Small: `NextTokenIsInt` 008D8F10 (the "%d" acceptance rule), `008DAD70`, `008DADA0`,
  `008DAD80`, `008C8980`, `008C9350`; `objective_kind_008dbf40` in cc9's
  `src/game_hosts_lua.cpp` backfilled after checking the six strings at 00E0C948.

## Third batch

- Clouds: `SetOldCloudAlpha` 008C8090 (three-level node walk; kind test on the first two
  levels only), `SetCloudVisibility` 008AE480 (closed box test of the node position).
- `GameCameraLookAtTarget` 00891FB0: the two controller calls take `(entity | vector, a1, a2,
  a3, a4)`; the slots were mapped from the `SUB ESP` sequences at 008922EA and 00892313
  against the frame offsets of the four locals (defaults 2.0f, 99999.0f, 0, 0).
- `SetGuiName` 008A8F90, `EnableEngineSound` 008911E0, `StartVehicleAnim` 008C2E00,
  `StopVehicleAnim` 008C3060, `GetVisibilityTerrainNode` 008C3410, `InsertFishSchool` 008C2970,
  `TempAddFloatsam` 008C5B80, `TempAddAnim` 008C5CD0, `RemoveTempAddedStuff` 008C5E20.

## Divergences and uncertainty

- The models take values and callbacks; nothing reproduces native object layout.
- `GetRawText` 008CA450 and `StopEffect` 008A9CA0 have no null test in the native; the C++ passes
  the possibly-null pointer to the host unchanged.
- `MovCamNew_AddPositions` 008B7850 has no camera null test while `MovCamNew_AddPosition`
  008B79F0 does; both are kept as found.
- `gui_find_widget_by_dotted_path` 008C8C20: the native's `substr(npos + 1)` wraps to 0; the path
  always ends in '.', so the npos case cannot arise, and the C++ ends the walk there instead.
  The child lookup's ECX was taken as the current widget (EBP at the call).
- `PerfTimingRegistry`: the comparator of the `008CFB20` tree was not read; it only orders
  iteration. The case-insensitive comparator of the count map is modelled with `tolower`.
- `language_lockit_id_008d48c0` (backfilled) bounds-checks where the native does not.
- The five scene tokenizer records are `partial_projection`: `scene_file.cpp` reimplements the
  reader behaviourally (`docs/SCENE_FILE_READER.md`), it does not transcribe these bodies.
- Pause 008AFD50 and Loading_Start 008CC850 call `BSP_Game_SetCinematicMode` and were left out:
  whether cinematic mode gates simulation was not established. Loading_Finish 008C77F0 is
  modelled because it only reaches that call after the please-wait screen hides.

## For cc9 (gameplay state; not touched)

All remaining `BSP_LuaBinding_*` handlers in the range that act on units, squadrons, planes,
navigators, pilots, air bases, shipyards, formations, repair/fire, recon, scoring, roles,
borders, triggers, devices and payloads. By family: ship/sub (`SetShipSpeed`, `BreakShip`,
`SetPump`, `AddWaterLoad`, `SetSubmarine*`), air base/shipyard (`SetAirBase*`, `LaunchAirBaseSlot`,
`Add*Stock`), squadron/plane (`Squadron*`, `Plane*`, `SetPlane*Control`, `LaunchSquadron`),
navigator/pilot (`Navigator*`, `Pilot*`), unit orders (`Order`, `PutTo`, `SetParty*`, `SetRace`,
`SetAiControl`, `Unit*`), repair/fire (`AddFire`, `SetRepair*`, `FailureRepairEnable`), recon
(`SetForcedReconLevel`, `ForceRecon`, `SetSimplified*Multiplier`), scoring (`Scoring_*`,
`ScoringGrantUnlock`, `SetAchievements`), triggers (`Add*Trigger`, `RemoveTrigger`), and
`SetHP`/`OverrideHP`, `KillStationaries`, `KillBullets`, `WipeEntityType`, `ReloadTorpedoes`,
`SetDeviceReload*`, `GameOver`, `Pause`, `Countdown*`, `SetWeather`, `SetShallowWater`.
`SetParty` 008A8930 and `SetThink` 00897FB0 got backfilled records only.

## Open (non-gameplay, not done in this pass)

The camera mode family `MovCam_HandMode_*`, `MovCam_RefPos*`, `MovCam_PathWalk*`,
`MovCam_ViktorMode`, `MovCam_TompiMode*`, `MovCam_SetSecondaryTarget`, `MovCam_AddInterpolator`,
`MovCam_PickSnittDeckPosition`, `MovCam_SetPosRotZoom`; camera reads `GetCameraRelPosW/L`,
`GetCameraState`, `GetDistFromCrosshair`, `GameCameraLookAtTarget`; `SetCloudVisibility`,
`SetOldCloudAlpha`; HUD `DisplayUnitHP`, `DisplayScores`; `Message`, `OverrideMessage`,
`MissionNarrative`/`Urgent`/`Override`, `StartDialog`, `SetGuiName`; Lua glue `class_enumeration`,
`UnloadClass`, `DebugTrap`, `UnloadMessageMap`, `GetConfiguredInput`, `ForceEnableInput`;
`HackPressInput` with `HackInput_QueuePress` 008D0060 (its list insert `0088B410` is in the
excluded band); `SceneTokenizer_NextTokenIsInt` 008D8F10 and `ConvertByUnitToken` 008D9750.
