# Options and hardware settings, partition segment 56 (cc13-options1)

Segment 56 spans `0088B410..008DCD40` (775 functions). Names below are hypotheses, not recovered
symbols. Host interfaces in `include/bsp/options_hardware_cc13.hpp` are projections and cannot
replace the native functions. Status words: *exported* (Ghidra pseudocode only), *reconstructed*
(C++ written from the listing), *build-tested* (compiles in `./scripts/build.ps1`),
*game-validated* (observed in a run). Nothing in this packet is game-validated.

## Triage

Method: `local/bsp_index.sqlite` survey of every non-funclet function in the band (ledger name,
reconstruction record, string references, docs), then a check of every function address
against `src/` and `include/`, and `tools/callsite_census.py` (rel32 plus absolute dwords) for
anything Ghidra reports without callers.

| Cluster | Range | Owner / decision | Reason |
| --- | --- | --- | --- |
| Lua object helpers | `0088B410..0088BBC0` | cc9 Lua lane, skip | `BSP_LuaObject_IsVector3Table`, `BSP_LuaObject_SetVector3Fields` and small unnamed helpers called only by the natives. |
| `luaMW_*` natives | `0088BC00..008D2D60` | cc9, skip | The 560-row table `00E0B7B8` is fully listed in `src/mission_lua_host.cpp` `kBindings`. Every native in the band but three is referenced in `src/`. The HintSystem wrappers `008D1F50..008D2BB0` are cc13-hints1's. |
| Settings-touching natives | e.g. `GetDifficulty` `008AE030`, `SetSkillLevel` `00895250`, `Set/GetDamagedGFXLevel` `0088F710`/`0088FCC0`, `GetConfiguredInput` `008CC030`, `Save/LoadCheckpoint` `008ACB70`/`008ACA30`, `SoundFade` `008D16B0` | cc9, skip | All are already registered and referenced by cc9's mission Lua host. Reconstructing them here would duplicate that work. |
| Unregistered natives | `0088BF20` `Dump` | **this packet**, named | Debug stub that is in no table. See below. |
| Unregistered natives | `0089F360` `PlaneSetMovieCollision`, `008CCEB0` `SortByDistance` | cc9 list, not touched | Gameplay natives. Neither is in `00E0B7B8` and neither has a rel32 or absolute-dword reference (census `total 0`). They are dead code, listed for cc9. |
| Small holders | `008D2ED0..008D2F20` | **this packet** (`008D2ED0`), STL left alone | `008D2ED0` has no reference. |
| Lobby registry | `008D2F50`, `008D3E00` | `008D2F50` is already in `docs/MISSION_LOBBY_SETTINGS.md`; **this packet** names `008D3E00` | Registry destructor. |
| Settings core | `008D41C0..008D8190` | mostly done earlier; **this packet** adds `008D48E0`, `008D4A00..008D4AF0`, `008D6B70`, `008D6C00` | Load/apply/write/archive/language/Xbox-compat/hardware-reported were reconstructed by earlier packets: `src/game_settings.cpp`, `src/settings_text.cpp`, `src/settings_archive_reader.cpp`, `src/game_native_settings_application.cpp`, `src/profile_commit.cpp`, and the docs `OPTIONS_MENU_SCREENS.md`, `OPTIONS_SETTINGS_COMMIT.md`, `APP_INIT_LOCALE.md`, `SETTINGS_CAPABILITIES.md`. |
| Scene tokenizer | `008D8900..008D9F20` | cc12 scene-property lanes, skip | Covered by `NATIVE_SCENE_TOKENIZER_R158.md`, `scene_file.cpp`, `scene_property_bag.cpp`. `008D8F10` is still open but belongs to the scene-property-bag lane. |
| Unit/table defaults | `008D9150`, `008D97A0` | done earlier, skip | `CC12_GAME_APPLICATION_*`, `NATIVE_GAME_TABLES_R114.md`. |
| HUD helpers, STL | `008DA100..008DB7B0` | HUD lanes, skip | `HUD_CENTRAL_UPDATES.md`, `HUD_MARKERS_RUNTIME.md`. These are not settings code. |
| Global-config effect state, objectives, session messages 55..59 | `008DBCD0..008DCD40` | cc12 / cc9 / done, skip | `CC12_GLOBAL_CONFIG_*`, `OBJECTIVE_UNIT_LIST.md`, `NATIVE_SESSION_MESSAGES_56_TO_62_R191.md`. |

The keywords for the segment (`options`, `hardwarereported`, `english`, `xboxcompatibilitymode`,
`vsync`, `texturedetail`, `targetindicator`) come from `008D8190`, `008D6170` and `008D44C0`,
which were reconstructed before this packet. The options/hardware code that was still open was
small: the save to the profile store and the equality function.

## Settings save to the profile store: `008D6C00`, `008D6B70`

Reconstructed and build-tested. See `save_profile_settings_008d6c00` and
`complete_profile_settings_save_008d6b70`. This is the write counterpart of
`restore_profile_settings_008d7a50` (`docs/PROFILE_COMMIT.md`) and uses the same pending-callback
slot `00F88958`, so it takes `ProfileSettingsRestoreState`.

`008D6C00` is `__thiscall`: ECX = settings, but ECX is never read. It takes one callback on the
stack and returns with `RET 4`. `callsite_census` finds seven calls (Ghidra reported three):
`0055F9BD`, `0055F9FC`, `0055FA28`, `005EFE62`, `005F01E6`, `005F0CB5` and `005F6E58`. Every site
loads `ECX = 00F88980`. Most of them push `00BD53C0` (`BSP_StorageOperation_EmptyContinuation`,
a lone `RET`). `005F6E58` (the options commit `005F65C0`) pushes `005F0090` when `[ESP+1Fh]` is
set and `00BD53C0` otherwise.

1. `008D6C19..008D6C37`: requires storage manager `[0109CECC]+21h` and `00A3E510` on `[00F8ABE8]`
   (the selected Xenon user). Unlike the restore, there is no `00BD3450` reset and no game check.
2. `008D6C61`: when `007F8CA0` (ECX = game+650h) reports a profile save name, the name is a copy of
   game+684h. Otherwise it comes from `004C5E00` (`BSP_StorageManager_GetNewestSlotName`), which
   calls storage-manager vtable `+10h` with an out-string (`004C5E00..004C5E1A`). On the PC
   vtable `00D68C10` that slot is `00BEB630`; see the next section.
3. `008D6CC4/008D6CCE`: compared against the empty string from `00436710` through `00449AF0`.
   Only nonemptiness matters.
4. When the name is nonempty, `008D6D06` stores the callback in `00F88958`. `008D6D0C` calls
   `00BD3E10(name, 0)`, a single-file write of kind 0 (`docs/PC_STORAGE_BACKEND.md`, operation 4). Kind 0 is
   the leaf `quick`, the first of the three names at `00E15324` (`include/bsp/storage_backend.hpp`).
   `008D6D11` passes the literal `008D6B70` to `006ADB50`.
5. Otherwise, `008D6D7B..008D6D83` calls the argument when it is nonnull and leaves the slot as it
   was.

`008D6B70` (no arguments, `RET`) had no Ghidra function. The only reference to it is the immediate
at `008D6D11`. When the storage state `[0109CECC]+08h == 2`, it builds a stack writer
`{00CE4104, 0}`, runs `008D64A0` (`BSP_Settings_WriteToStore`) with ECX = `00F88980`, then reads
`00F88958`, clears it and passes the saved value to `006ADB50` without a null test
(`008D6BB8..008D6BC8`). In any other state it reads the slot, clears it and calls the value when
nonnull (`008D6BDC..008D6BEF`). Two differences from the profile writes in `src/profile_write.cpp`:
no state-1 branch clears the profile name, and no `00CE3784` retiring vtable is stored on the
normal path. The host still releases its writer, because the host needs that for lifetime.

### Save-name fallback: `00BEB630`, `00BEB0C0`

The selection rule is reconstructed and build-tested (`newest_valid_storage_slot_00beb630`). The
Win32 walk is not projected, and `PcStorageBackend` (`src/storage_backend.cpp`) does not
implement this slot yet. `00BEB630` (ECX = manager, out NativeString*, `RET 4`) starts from the
empty string (`00436710` at `00BEB669`) and a zero time. It enumerates `FindFirstFileA` /
`FindNextFileA` over manager `+53Ch`, which the initializer `00BEB2C0` builds as root + `"\*"`
(`00D68C58`). Each entry that passes vtable `+20h` (`00BEB1C0`, `BSP_PcStorage_HasValidMarker`)
is timed by `00BEB0C0` at `00BEB724`. That helper `sprintf`s `root(+534h)\slot\valid`, opens the
file with GENERIC_READ, and returns the last-write FILETIME, or 0 on any failure. An entry
replaces the choice only when its time is strictly greater (high dword compared first). Ties
therefore keep the earlier entry, and a time of zero never wins. `FindClose` runs even when
`FindFirstFileA` returned INVALID_HANDLE_VALUE.

Effect: on PC, saving settings with no profile save name writes the settings archive to leaf `quick`
of the slot whose `valid` marker is newest. With no valid slot the name is empty, the save is rejected and
the callback is called directly. The other two callers of `004C5E00` are the console-command
paths `004E26AA` and `004E3418` (census). This packet does not describe them.

## Settings equality: `008D4AF0`, `008D48E0`

Reconstructed and build-tested. `callsite_census` finds no rel32 call to `008D4AF0` and no
absolute dword equal to its address. Its only callee-side reference is the call to `008D48E0` at
`008D4D1B`. It is recovered because it fixes which fields count as the user's settings, not
because anything calls it.

`008D4AF0` is `__thiscall(ECX = this, [ESP+4] = other)`, returns EAX 1 or 0, and ends with `RET 4`.
In native order it compares:

- 39 scalars (`kSettingsEqualityHead`) at `008D4AF3..008D4D0D`;
- the `+98h` vector through `008D48E0` (ECX = this+98h);
- 8 more scalars (`kSettingsEqualityMiddle`), in which `+90h` is compared a second time;
- the `+B4h` NativeString through `00435C40` (length equality first, then `__stricmp`);
- `+B1h`, `+1Dh` and `+1Ch`.

Float fields (`+20h`, `+28h`, `+2Ch`, `+30h`, `+34h`, `+64h`) use `FUCOMIP; FSTP ST0; LAHF;
TEST AH,44h; JP` (six FUCOMIP in the body). That is IEEE `==`: NaN is unequal to itself and
`+0 == -0`. The body never reads `+0Ch` (hints), `+80h` (marker alpha), `+95h`
(HardwareReported) or anything above `+B8h` other than the string. Offsets `+38h`, `+3Ch`,
`+49h`, `+4Bh`, `+4Ch` and `+50h` are compared but have no name in `GameSettingsBlock`. The
comparator reads them from `NativeGameSettingsStorage` bytes.

`008D48E0` is `__thiscall(ECX = vector, [ESP+4] = other)`, returns AL, and ends with `RET 4`.
Entries are 8 bytes, a NativeString (`+0` length, `+4` text). The counts must be equal. A
zero-length entry equals only another zero-length entry. Otherwise `__stricmp` (`00BF7FBF`)
decides, with no length comparison first.

## Named only

| Address | Name | Evidence |
| --- | --- | --- |
| `008D4A00` | `BSP_IntPairVector_PushBack` | Grows through `008D4750` to `max(2*cap,1)`, copies two dwords, increments the count. No reference. |
| `008D4A40` | `BSP_IntPairVector_Clear` | `008D4750(0)` on negative capacity, count to zero. No reference. |
| `008D4A80`, `008D4AB0` | `BSP_IntrusivePtr_Release_<addr>` | Identical bodies: InterlockedDecrement pointee `+4`, vtable slot 0 at zero, clear holder. No reference. |
| `008D3E00` | `BSP_MultiLobbySettings_Destroy` | Vtable `00CEF164`. Tears down the maps at `+34h`, `+28h`, `+1Ch`, `+10h`, `+04h`, clears singleton `00F8894C` and leaves base vtable `00CE3818`. Only caller: `00576AF0`. |
| `008D2ED0` | `BSP_OwnedPointer_FreeAndClear` | `_free(*holder)`, then `*holder = 0`. No reference. |
| `0088BF20` | `BSP_LuaBinding_Dump` | Builds `"luaMW_Dump failed:"` (`00D112E8`) as a NativeString, frees it and returns 0 results. Not in `00E0B7B8` and has no reference. |

None of these got C++. The containers are generic, and the destructor's members are `std::map`s
in every host projection, so a C++ version would only restate the standard destructor.

## Verification

- `./scripts/build.ps1` in this worktree: exit 0, `options_hardware_cc13.cpp` compiled, 2/2 CTest
  suites passed (run after each source change).
- `python tools/verify_report_calls.py reports/cc13_options1_calls.json`: 15 call rows, 0 failed.
- Ghidra: function defined at `008D6B70..008D6BFF` (`reports/cc13_options1_defined_functions.json`)
  and the 14 ledger names applied through `tools/ghidra_annotate.py --apply`. The prior values
  are in the worktree's `local/ghidra-annotations-*.json`.
- No test was added. The comparator is a table over the listing, and the save mirrors an existing
  fixture-tested restore. Neither body has a concrete regression risk that a test would catch
  better than the listing citations.

## Segment 3, `0043E8F0..004486A0`: weather state

### Triage

| Cluster | Range | Owner / decision | Reason |
| --- | --- | --- | --- |
| Unit-instance helpers | `0043E8F0..0043FE70` | cc9/units, skip | Called from unit fire, hit and airops paths (`UNIT_*`, `AIROPS_*`). |
| Device classes | `0043FEA0..00443490` | open, not taken | Lua `DeviceClass`/`platform`/`_device` resolution. `NATIVE_DEVICE_CLASS_*_ORCH4.md` records the factory `00443090`/`00443490` as evidence only, with no C++ source; that doc calls the missing source "a real object family". It builds per-unit devices, so a reconstruction belongs to a units packet, not to this one. |
| Weather config | `00443D60..00445BF0` | **this packet** for what was left | `src/weather_config.cpp` already had the constructor, the Globals.lua loader and the map helpers. Added below: `00445790`, `00445A30`, `00445BF0`, plus four names. |
| Game dynamics list | `00446080..00448490` | skip | `GAME_DYNAMICS_LIST.md`; moving-object bookkeeping (gameplay). |

### Reconstructed (build-tested): `00445790`, `00445A30`, `00445BF0`

`00445A30` (`BSP_WeatherConfig_SetCurrentWeather`) is `__thiscall`: ECX = the WeatherConfig
at game+21C4h, a NativeString argument passed by value and destroyed by the callee, `RET 8`.

1. It copies the name into `current_weather_2c`. The copy is guarded against self-copy, resizes
   with preserve, then copies `length` bytes (`00445A5A..00445A81`).
2. It sets `current_weather_modifier_34 = weather_recon_modifiers_38[name]` through `00444BE0`
   (`00445A8C/00445A93`). A missing name is inserted as 0.0f. At load, `00444D20` resets this
   field to 1.0f for `"default"`.
3. It calls `00445790` on the game's current camera mover `[game+1ED4h]` (`00445AA4`).

The callers are `luaMW_SetWeather` `008B21A0` (registered in cc9's mission host) and
`00445BF0`. Recon detection reads the `+34h` modifier (`docs/GUNNERY_RECON_DETECTION.md`).
That consumer changes unit state and stays with cc9; only the setter is reconstructed here.

`00445790` (`BSP_WeatherConfig_ApplyCameraShake`, ECX = config, `[ESP+4]` = mover, `RET 4`)
does nothing for a null mover. Otherwise it:

1. copies `base_amplitudes_44`;
2. assigns the mover's vtable `+12Ch` view-mode name;
3. looks up `weather_amp_multipliers_68[current_weather_2c]`, then
   `view_mode_amp_multipliers_5c[view]`;
4. forms `m = (float)(view * (double)weather)` (`0044580C` FMUL qword, `0044581E` FSTP dword);
5. multiplies each amplitude by m, rounding each result to float;
6. calls `0042C4C0` (`BSP_CameraMover_SetShake`) with the scaled amplitudes and
   `frequencies_50`.

`0042C4C0` stores those six floats at mover `+1D0h..+1E4h`, the shake block that `004329D0`
advances. Both products are exact before rounding, so double arithmetic reproduces x87. Nothing
writes `+74h`/`+78h` here. The other callers are `BSP_Game_SetCameraMover` `004BC410` and
`BSP_MovCam_EnterMode` `007A5110`.

Correction to `docs/MISSION_CAMERA.md` line 31: `00445790` does not register the mover with
`[game+21C4h]`. It pushes the weather camera-shake parameters into the mover.

`00445BF0` (`BSP_WeatherConfig_ReadState`, ECX = config, `[ESP+4]` = reader, `RET 4`) has no rel32
or absolute-dword reference, so it never runs. It:

1. reads `{0,"dayTime"}` through reader vtable `+10h` as a Float into `+04h`;
2. reads `{0,"actWeather"}` as a String;
3. passes a by-value copy of that string to `00445A30`;
4. wraps `+04h`: adds 24.0 (`00CE4790`, a double) while `0 > t`, then subtracts while
   `t > 24.0f` (`00CE4788`).

So the field `weather_config.hpp` calls `start_day_time_04` is also the live time of day in the
image's own save shape.

### Named only

| Address | Name | Evidence |
| --- | --- | --- |
| `00445940` | `BSP_WeatherConfig_Destroy` | Vtable `00CE4934`. Erases maps `+68h`, `+5Ch`, `+38h`, then frees `+2Ch`, the same order as `release_weather_config_storage`. Only caller: `00445BD0`. |
| `00445900` | `BSP_WeatherFloatMap_Destroy` | Erases the whole tree and frees the head. Referenced by 11 unwind funclets. |
| `00445AE0` | `BSP_WeatherFloatMap_Construct` | Head node from `00443E20`, isnil byte set, size 0. No reference. |
| `0042C4C0` | `BSP_CameraMover_SetShake` | Six floats by value, `RET 18h`. Callers: `00445790`, `007998C0`. |

## Segment 0, `00401010..004138D0`: physics library and bot-state archive plumbing

### Triage

| Cluster | Range | Owner / decision | Reason |
| --- | --- | --- | --- |
| Math, Dyn physics, pool allocators, legacy strings | `00401010..00410400` | skip (library) | RTTI vslots `Dyn_*Shape`, `Dyn_*BroadPhaseManager`, `Mit_*cPoolAllocator`. The named DYN pools and convex-hull helpers are already in the ledger. This is the third-party dynamics engine, not startup or options code. |
| Archive handles and time/freq pairs | `00410630..00410E7F` | **this packet**, named | Plumbing that bot tasks use to save and load. |
| Bot-state registry | `00411170..004114C9` | **this packet**, named; cc9 owns behaviour | `stateIndex` load/save of the AI task state machines. The rest of the registry (`BSP_BotStateRegistry_*`, `BSP_BotStatePairs_*`) is already named and reconstructed in `src/native_land_state_registry*.cpp`. |
| Avoid zones onward | `004120D0..004138D0` | skip | `AVOID_ZONE_OWNER.md` (gameplay). |

This band has no stand-alone "startup state machine". `stateIndex` is the archive key of the
bot-task state machines.

### Named only, with one listing-level finding

Archive values are `{tag, word}` pairs (`GuiLuaVariant`). A reader is called through vtable
`+10h` (`00BD6830` on the Lua reader), a writer through `+0Ch`.

| Address | Name | Evidence |
| --- | --- | --- |
| `00410630` / `004107B0` | `BSP_ArchiveReaderRef_Destroy` / `BSP_ArchiveWriterRef_Destroy` | 12-byte handles `{vtable 00CE3768 / 00CE3794, int* count, visitor*}`, passed by value and destroyed by the callee. Each decrements the count; at zero it frees the count and calls visitor vtable `+8`. Base vtables are `00CE374C` / `00CE3784` (`00CE3784` is the writer base that `profile_write.hpp` names). |
| `00410D80` / `00410E00` | `BSP_TimeFreqPair_Read` / `_Write` | ECX = `{float time, float freq}`, handle by value, `RET 0Ch`. Keys `"time"` (`00CE37B0`) and `"freq"` (`00CE37A8`), Float tag 2. |
| `00411170` | `BSP_BotStateRegistry_ReadCurrent` | Reads `{0,"stateIndex"}` (`00CE37B8`) as an Int and indexes the `{name, state}` vector (begin `+8`, end `+0Ch`). The bounds check is unsigned and calls `00BF6713` when it fails. Then `machine+4 = state` and state vtable `+4`. |
| `00411350` | `BSP_BotStateRegistry_IndexOf` | **Returns -1 for every input.** The match branch `00411391..004113A0` loads the begin pointer, runs the checked-iterator tests and rejoins the loop. The only exit, `004113B6`, is `OR EAX,-1`. |
| `004113C0` / `00411430` | `BSP_BotStateRegistry_FindName` / `_NameOrUndefined` | The name registered for a state pointer, or `"undefined state name"` (`00CE37C4`). ECX passes through `00411430` unchanged. |
| `00411450` | `BSP_BotStateRegistry_WriteCurrent` | Writes `{0,"stateIndex"}` `{1, IndexOf(machine+4)}`, so the value written is always -1. Read back by `00411170`, -1 compares as `0xFFFFFFFF` and reaches `00BF6713`. |
| `004114A0` | `BSP_PointerVector_Tidy` | Frees `+4` and zeroes `+4/+8/+0Ch`. No reference. |

Consequence for cc9: if a checkpoint or save ever serializes a bot task through one of the eight
`WriteCurrent` callers (`009A2C50..009C33D0`), loading it would end in the CRT invalid-parameter
path. Nothing here shows that a save path reaches those task methods, and no run has exercised
one. A host that reconstructs task save and load must decide whether to keep the -1.

## Handoff

Segment 56 has no options or hardware-settings body left open, by the triage above. Open items:

1. `PcStorageBackend` has no vtable `+10h` (`00BEB630`). A host that saves settings with no
   profile save name needs it. Wire `newest_valid_storage_slot_00beb630` to a Win32 walk in
   `src/storage_backend.cpp` (owned by the storage-backend lane, not this packet).
2. `save_profile_settings_008d6c00` is not called by any host yet. Its seven native call sites, in
   (`0055F9B0`, `005EFDC0`, `005F0090`, `005F0B90`, `005F65C0`) are options/front-end screens and
   are still `FUN_` except `005F65C0`.
3. For cc9: `PlaneSetMovieCollision` (`0089F360`) and `SortByDistance` (`008CCEB0`) are
   unregistered, unreferenced gameplay natives.
4. `008D8F10` (scene property bag) is still open, in the cc12 scene lane.
5. Segment 3: the device-class factory family (`0043FEA0..00443490`) still has no source and
   needs a units-lane packet. The `+34h` consumers of the weather setter are cc9's.
6. Segment 0: nobody has checked whether a save reaches the bot-task `WriteCurrent` callers
   (the -1 `stateIndex`). A breakpoint on `00411450` during a checkpoint save would settle it.
