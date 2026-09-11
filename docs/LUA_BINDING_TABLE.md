# The 560-row mission Lua binding table, surveyed

Addresses: 00e0b7b8 00e0c938 006b8610 00884be0 0088bec0 00891680 008918d0 0094c480 00946380 00946390 00944680 008ee410 008ea610 008eb350 00a37310 00a37400 00a37650 00a37790 00a38a50 00a378c0 00a37a00 00a37b30 00a38010 00a37d50 00a38960 00a37eb0 00a38200 00a38430

Packet `cc_mission_natives`, worktree `agent/cc-mission-natives`. Ghidra was read-only. Every
name here is a hypothesis, not a recovered symbol.

`docs/MISSION_LUA_HOST.md` recovered the table and `docs/MISSION_LUA_MACHINE.md` confirmed its
size against the installed scripts. This document exports it and measures which rows the
installed scripts actually reach. The self table and the entity return convention are
`docs/MISSION_LUA_SELF_TABLE.md` and `docs/LUA_BINDING_ENTITY.md`.

## The export

`tools/lua_bindings_export.py` reads the rows out of the installed
`battlestationspacific.exe` through `pefile`, so `config/lua_bindings.json` can be regenerated
without the Ghidra bridge. `--check` fails when the file no longer matches the image.

```
python tools/lua_bindings_export.py           # rewrite config/lua_bindings.json
python tools/lua_bindings_export.py --check   # verify it against the image
```

Each row is eight bytes, a name pointer then a handler pointer, and the array runs from
`00E0B7B8` to the `{"", NULL}` sentinel at `00E0C938`. The generator stops at the sentinel
rather than at a fixed count, and gets 560, which is the figure
`docs/MISSION_LUA_MACHINE.md` derived from the span. Handler annotations (Ghidra function,
ledger name, reconstruction) come from `local/bsp_index.sqlite`.

| Measure | Value |
| --- | --- |
| rows before the sentinel | 560 |
| distinct handler addresses | 557 |
| handlers with a Ghidra function | 536 |
| handlers with no Ghidra function | 24 |
| handlers carrying a ledger name | 31 after this packet, 12 before |
| handlers carrying a reconstruction record | 0 |

The nineteen this packet added are exactly the entity-returning set of
`docs/LUA_BINDING_ENTITY.md`. Two of them, `FindEntity` and `CreateScript`, were read in full;
the other seventeen carry only the return convention, which their evidence says.

Three handlers are shared by two names each, which is a real aliasing in the table rather than
an export artefact:

| Handler | Names |
| --- | --- |
| `00896A90` | `AddAirBaseStock`, `AddAirBasePlanes` |
| `008B0C10` | `MissionNarrative`, `MissionNarrativeEnqueue` |
| `0088E560` | `SetMotionBlurParams`, `SetMBP` |

## What the installed scripts reach

`bsp_mission_script_probe --sweep` runs every `Scripts/missions/**/*.lua` through the chunk rule
and the four entry points, then calls every function `CreateScript` registered. With
`--recon-tables` (see the `recon` uncertainty in `docs/MISSION_LUA_SELF_TABLE.md`):

| Outcome | Count |
| --- | --- |
| installed mission scripts | 299 |
| chunk loads and all defined entry points run with no error | 299 |
| load failures | 0 |

Without `--recon-tables` the figure is 297; both copies of
`COTP-USN/usn_02_battle_of_cape_esperance` fail at `commandhelpers.lua:12343` on the global
`recon`, which `004E0305` nils on every mission load.

`00885FB0` runs every content variant after the base file (`append_lua_script_overrides_00bdef90`
in `include/bsp/vfs_lua_scripts.hpp`). Installed-file-checked: `Scripts/missions` holds 299 files
and every one of them ends in `.lua`, so the variant pass adds nothing on this copy and the
per-mission count is the base file alone.

### Bindings reached at load time

**140 of the 560** rows are reached by at least one script during the chunk and the four entry
points. Twenty of those are reached by exactly one script. A row no script reaches at load time
is not dead: the frame loop and the `CreateScript` bodies reach many more, and this count
deliberately excludes both.

| Scripts | Binding | Scripts | Binding |
| --- | --- | --- | --- |
| 299 | `CreateScript` **[entity]** | 194 | `LoadMessageMap` |
| 258 | `FindEntity` **[entity]** | 192 | `Scoring_SetFinalScoringFunctionName` |
| 258 | `PrepareClass` | 191 | `SetThink` |
| 237 | `Music_Control_SetLevel` | 170 | `EnableMessages` |
| 206 | `GetDifficulty` | 160 | `debugtrap` |
| 203 | `SETLOG` | 155 | `MissionNarrative` |
| 202 | `SetParty` | 143 | `Blackout` |
| 196 | `Scoring_RealPlayTimeRunning` | 124 | `SetSkillLevel` |

`debugtrap`'s 160 are the error-handler pushes, not script calls: the probe fetches it before
every named call, and it is reached only when one fails.

Five of the nineteen entity-returning bindings are reached at load time: `CreateScript` (299),
`FindEntity` (258), `GenerateObject` (14), `GetSelectedUnit` (7) and `GetSquadronPlane` (7).
The full ranking is the tail of the `--sweep` output.

### One name outside the table

`DoFile` is called by scripts and is not a row. It is installed from the `LuaStateOwner` layer
at `00B6A303` with callback `00B69E00`, which `docs/MISSION_LUA_MACHINE.md` established and
which `00B66C00` repeats every time a binding builds an owner over the mission state
(`00B66C2F MOV EDX,0xB69E00` into `00A67B20 lua_pushcclosure`). It is the only name the sweep
saw that the table does not carry.

## Corrections

None to a prior document's claim about the table itself. The three corrections this packet
makes are to the follow-up rows of `docs/MISSION_LUA_MACHINE.md` and are recorded in
`docs/MISSION_LUA_SELF_TABLE.md`: `00CE7494` is a string and not a vtable, `00884240` is a
`vsprintf_s` helper and not a self-table populator, and `Effect` does not return an entity.

## Uncertainties

- The 140 figure counts load-time reach only. How many rows the frame loop reaches was not
  measured, and the `CreateScript` bodies the probe calls afterwards are excluded from it.
- Handler annotation comes from the index, so the 12 ledger-named and 0 reconstructed figures
  are a snapshot of `local/bsp_index.sqlite` at export time, not of the ledger shards directly.
- The three aliased handlers were not read, so whether the two names differ in behaviour by an
  argument or are genuinely identical entry points is open.
- Whether a name pointer can be null before the sentinel was not observed; the generator stops
  on a null name, an empty name or a null handler, and on this image only the sentinel row
  triggers it.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `lua_binding_ai_block` | 00a372a2 through 00a38d9f | docs/LUA_BINDING_AI.md | The fourteen `AI*` handlers in the one gap that has no Ghidra function at all |
| `lua_binding_spawn_block` | 00944680 0094c480 00946380 00946390 | docs/LUA_BINDING_SPAWN.md | The four spawn handlers in undefined gaps |
| `lua_binding_alias_pairs` | 00896a90 008b0c10 0088e560 | docs/LUA_BINDING_ALIASES.md | Whether the three doubly-named handlers differ by argument |
| `lua_binding_frame_reach` | 00e0b7b8 | docs/LUA_BINDING_TABLE.md | Which rows the frame loop reaches, which the load-time sweep cannot see |

## no_ghidra_function

Twenty-four handlers named by the table have no Ghidra function. Each sits in a gap between two
defined functions; the evidence for every boundary is the preceding function's body end and the
following function's start, both from `python tools/bsp.py ghidra proto <addr> --brief`, plus
the absence of any function entry at the handler address itself in `local/bsp_index.sqlite`.

| Gap start | Gap end (inclusive) | Preceding function | Following function | Handlers inside |
| --- | --- | --- | --- | --- |
| `0088BEBF` | `0088BF1F` | `0088BD20` ends `0088BEBE` | `0088BF20` | `Log` `0088BEC0` |
| `00891678` | `00891B1F` | `008914F0` ends `00891677` | `00891B20` | `GetWaterLoad` `00891680`, `GetLeaks` `008918D0` |
| `00944674` | `00944FCF` | `00944540` ends `00944673` | `00944FD0` | `Spawn` `00944680` |
| `00946378` | `009463AF` | `009462B0` ends `00946377` | `009463B0` | `SpawnNewIDIsRequested` `00946380`, `SpawnNewIDRemove` `00946390` |
| `0094C472` | `0094C48F` | `0094BFF0` ends `0094C471` | `0094C490` | `SpawnNew` `0094C480` |
| `008EA607` | `008EA74F` | `008EA4E0` ends `008EA606` | `008EA750` | `PreparePowerup` `008EA610` |
| `008EB342` | `008EB49F` | `008EB110` ends `008EB341` | `008EB4A0` | `GetAvailablePowerups` `008EB350` |
| `008EE3F3` | `008EE5EF` | `008EE020` ends `008EE3F2` | `008EE5F0` | `AddPowerup` `008EE410` |
| `00A372A2` | `00A38D9F` | `00A37250` ends `00A372A1` | `00A38DA0` | fourteen `AI*` handlers, `00A37310` to `00A38430` |

The fourteen in the last row are `AICreate` `00A37310`, `AIEnable` `00A37400`,
`AIEnableGrouping` `00A37650`, `AIMergeGroups` `00A37790`, `AIGetGroupInfo` `00A378C0`,
`AISetCommand` `00A37A00`, `AISetHintWeight` `00A37B30`, `AISetDefendResourcePercent`
`00A37D50`, `AISetSpawnSceneUnitsWeightMul` `00A37EB0`, `AISetQuickSpawnTargetPos` `00A38010`,
`AIGetTargetWeight` `00A38200`, `AISetTargetWeight` `00A38430`, `AICreateGroup` `00A38A50` and
`AIReloadGlobals` `00A38960`. A 6.4 KB stretch with one defined function at its head is the
largest undefined region any binding handler points into, and none of these fourteen was read
by this packet.
