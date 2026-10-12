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
   game+684h. Otherwise it comes from `004C5E00`, which calls storage-manager vtable `+10h` with an
   out-string (`004C5E00..004C5E1A`). What that slot returns is not recovered.
3. `008D6CC4/008D6CCE`: compared against the empty string from `00436710` through `00449AF0`.
   Only nonemptiness matters.
4. When the name is nonempty, `008D6D06` stores the callback in `00F88958`. `008D6D0C` calls
   `00BD3E10(name, 0)`, a single-file write of kind 0 (`docs/PC_STORAGE_BACKEND.md`, operation 4).
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

- `./scripts/build.ps1` in this worktree (result recorded in the handoff below).
- No test was added. The comparator is a table over the listing, and the save mirrors an existing
  fixture-tested restore. Neither body has a concrete regression risk that a test would catch
  better than the listing citations.

## Handoff

See the final section appended at close.
