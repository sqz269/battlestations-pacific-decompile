# Torpedo-bomber goaway entry draws, the PilotBotConfig load, and a rack's CanFire (packet cc13-goaway)

Two packets for the cc9 gameplay lane, 2026-10-19. A builds three OFF switches and a loader; B is an
audit with a ledger-only verdict. Nothing is wired ON and no game was run.

Files: `include/bsp/cc13_torp_goaway.hpp`, `src/cc13_torp_goaway.cpp` (new, `bsp_core`), and the
two host sites in `src/game_hosts_units.cpp`.

## A. The goaway enter's three draws

### A.1 The binary

`009D0D90` `BSP_BotStateTorpedoGoAway_Enter`, goaway vtable `00D212E0` slot `+4h`,
`__fastcall(ECX = state)`, plain `RET`. It draws on stream 1 three times (`MOV ECX,1`,
`CALL 00BD2F10` `BSP_Random_UniformFloatRange`, both bounds as float stack arguments, first = low):

| site | low, high | result goes to | condition |
| --- | --- | --- | --- |
| `009D0E2C` | `FLD1` 1.0, `[00D20CE4]` 1.15f (`3F933333`) | `FMUL [ESP+8]` (SafeDist, or the target extent) then `FSTP [ESI+24h]` | always |
| `009D0E71` | `[00CEB4D4]` 50.0f, `[00CE3D08]` 100.0f | `FADD [ESP+8]` (`+78h + +74h`, stored as a float at `009D0E5A`) then `FSTP [ESI+1Ch]` at `009D0E88` | `[approach+132h] != 0` (`009D0E42`/`009D0E4A JZ`) |
| `009D0EA3` | `[row+10h]`, `[row+14h]` (`009D0E8B`-`009D0EA0`, row = `[approach+14h]`) | `FSTP [ESI+28h]` at `009D0EAB` | always |

`009D0E3A` calls `009D0C10` between the first and the other two. Its direct callees
(`007F0280`, `009FD570`, `00438AA0`, `00438B10`, the unit's `vtable[50h]`) do not include
`00BD2F10`, so the order on the stream is `009D0E2C`, `009D0E71` (when taken), `009D0EA3`.

`row` is `&PilotBotConfig.levels[[[unit+DF4h]+34h]]` (`009F9CFF`-`009F9D22`,
docs/TORPEDO_RUN_PROFILE.md). `row+10h`/`+14h` are config `+1Ch`/`+20h`, which the PilotBot reader
`009973B0` fills from `TorpFlikFlakTime[1]` and `[2]`: the lookups at `00997C68` and `00997CC5`, and
the store `FSTP [ESI+1Ch]` at `00997C96` (checked in the listing this packet).

The reader of `+28h` is the tick `009D0F10`: `009D0F5E` counts it down while the range is inside
the break-off ring less 100 m, and the manoeuvre window only re-seeds once it is below zero and the
aircraft is above `+20h` (`009D0FB2`-`009D1025`, `src/torpedo_goaway_tick.cpp`). So `+28h` is the
delay before the first "flik-flak" weave of the escape; robots.lua's own comment on the key says
so ("a ketto kozotti idonkent beszur egy flik-flak manovert menekules kozben", at intervals between
the two it inserts a flik-flak manoeuvre while escaping).

### A.2 The host before this packet (unchanged OFF)

`src/game_hosts_units.cpp`, the goaway enter inside the torpedo state transition:

- `gin.distance_jitter = torpedo_goaway::kDistanceJitterLo` (the low end, no draw);
- `gt.climb_jitter = torpedo_goaway_tick::kClimbJitterLo` (the low end, no draw);
- `gt.window_delay_draw = 0.0f` with `unimplemented("PilotBotParameters::TorpFlikFlakTime", 00997c68)`.

No cc9 switch routes around this: the enter has one host caller, the transition into
`TorpedoState::kGoAway`, and no `if constexpr` wraps the three values.

### A.3 PilotBotConfig verdict: the loader exists, the host never ran it; it now can, behind a switch

- The image fills `[00F8A30C]` in `00901610` `load_robot_config_00901610` (src/robot_config.cpp),
  called once per process by `004DC6A0` `BSP_Game_ConstructGlobalSubsystems`. It opens a mask-41h
  Lua owner, runs `Scripts\global\luaMW_init.lua` then `Scripts\datatables\Robots.lua`, builds nine
  descriptors from the `Robots` global and looks the aliases up.
- `bsp_game` records `004DC6A0` (src/game_hosts_mission_frame.cpp, the `global_subsystems` step:
  "needs a GlobalSubsystemContext this process cannot build"), so the loader never ran and no
  host code holds a PilotBot row. Every PilotBot value the host uses today is a labelled copy of
  robots.lua (`kPilotDiveBombRows`, the strafe rows, the TorpRelease distances).
- What blocked it was the context, not the loader: 00901610 needs a `LuaStateOwnerEnvironment`
  and a `LuaScriptRuntime` over a `LuaScriptFiles`. The units host has the mission Lua host, whose
  `open_script`/`script_size`/`read_script`/`close_script` (00885110) and `script_variant_names`
  (00BDEF90) are exactly the VFS boundary those need. `load_plane_globals_007e2a20` already runs a
  private state from the same reader.
- New: `cc13_torp_goaway::MissionScriptFiles` adapts that reader to `LuaScriptFiles`, and
  `load_pilot_bot_config_00901610(files)` reads `Scripts\fundamentals.lua` for the owner bootstrap
  and then hands everything to the existing loader in src/robot_config.cpp, unchanged (a host
  adapter, not an image function). It keeps the six PilotBot rows
  process-wide and releases the other descriptors (`clear_robot_registry_00900bb0`).
- The call is in `GameUnitsHost::load_gameplay_settings_0083b5e0`, right after
  `load_plane_globals_007e2a20`, under `kPilotBotConfigLoadBound`. That is the existing
  settings-load path, and it runs before `create_units`, so before any goaway enter can read a row.

This installation's `scripts/datatables/robots.lua` (124185 bytes, mtime 2025-06-01 16:03 local,
SHA-256 `CA3E16E8...D73737D2`), `Robots["PilotBot"]`, `TorpFlikFlakTime` (seconds), by the
reader's level index:

| index | level | line | TorpFlikFlakTime |
| --- | --- | --- | --- |
| 0 | Stun | 1252 | 20, 30 |
| 1 | SPNormal | 561 | 12, 22 |
| 2 | SPVeteran | 700 | 5, 8 |
| 3 | MPNormal | 838 | 5, 8 |
| 4 | MPVeteran | 976 | 5, 8 |
| 5 | Elite | 1114 | 5, 8 |

These are recorded for reference only; the code reads the loaded rows and hard-codes none of them.

### A.4 The switches (include/bsp/cc13_torp_goaway.hpp, all false)

| switch | site | ON |
| --- | --- | --- |
| `kTorpGoawayEnterDrawsBound` | parent | gates the three below |
| `kTorpGoawayDistanceJitterDrawBound` | `009D0E2C` | `gin.distance_jitter` = stream-1 draw in [1.0, 1.15) |
| `kTorpGoawayClimbJitterDrawBound` | `009D0E71` | when `has_ordnance_132`, `gt.climb_jitter` = stream-1 draw in [50, 100) |
| `kTorpGoawayWindowDelayDrawBound` | `009D0EA3` | `gt.window_delay_draw` = stream-1 draw in [row+10h, row+14h) |
| `kPilotBotConfigLoadBound` | `00901610` | runs the load; required by the window-delay draw |

The draws go through `GameGunneryHost::death_mode_draw_00bd2f10(1, unit, low, high)`, the host's
shared stream-1 generator, in the image's order. Each switch falls back to its OFF value when the
gunnery host is absent; the window delay also falls back when the load failed or the level index is
outside 0..5 (`pilot_bot_row` answers null: there is no image row there).

The row index is the slot's `pilot_skill_index` (unit+390h / bot+34h) when `kSkillLevelBound`, else
1, the same rule as `dive_bomb_row_index`. UNCERTAIN: the image captures the row when the approach
is built (009F9D22), and the host reads the index at the enter; they differ only if a writer of
bot+34h (009565A0, 007B8AE0) runs in between.

OFF identity: with every switch false the enter computes the same values with the same log calls
as before, and `load_gameplay_settings_0083b5e0` makes no script read.

x87: the enter runs on the main thread (24-bit PC). The two arithmetic steps after the draws are
single float operations on float operands (`FMUL` by a float, `FADD` of a float), so 24 and 53
bits round them identically; the existing `torpedo_goaway_enter_009d0d90` and
`torpedo_goaway_enter_tail_009d0e3a` are unchanged. No new float constant is introduced (the
bounds reuse `kDistanceJitterLo/Hi` and `kClimbJitterLo/Hi`).

### A.5 Expected ON effect

- Distance jitter: the break-off distance `+24h` becomes SafeDist times [1.0, 1.15) instead of
  exactly SafeDist, up to 15 % further out.
- Climb jitter: only an aircraft that enters goaway still carrying its torpedo (`+132h` set, an
  aborted run) draws; a spent bomber takes the squadron cruising altitude and draws nothing. When
  drawn, the climb target rises from band + 50 to band + [50, 100).
- Window delay: the first weave waits [12, 22) s on SPNormal ([20, 30) Stun, [5, 8) the rest)
  of countdown inside the break-off ring instead of opening on the first tick above `+20h`.
- Every enter now consumes two draws (three with ordnance) from stream 1, so every later stream-1
  consumer in the run shifts. That is the image's behaviour, and the expected source of unrelated
  movement in an ON pair.

### A.6 ON-only lookups and their data lifetime

- `loaded_pilot_bot_rows()` / `pilot_bot_row(i)`: a function-local static in
  src/cc13_torp_goaway.cpp. Filled by `load_pilot_bot_config_00901610`, called from
  `load_gameplay_settings_0083b5e0` at the units host's construction, which the mission frame
  calls before `create_units` (src/game_hosts_mission_frame.cpp, right after
  `run_load_scene_contents_004d4df0`).
  Nothing clears or moves it: it is process-wide, set once (a second call returns
  `already_loaded`), as `[00F8A30C]` is in the image, which only `00900BB0` clears at shutdown.
- `owner_.gunnery`: the gunnery host the other stream-1 draws in this file already use; null only
  before it is bound, and the draw then falls back to the OFF value.
- `slot_.pilot_skill_index`: a slot field; see A.4 for its timing.

### A.7 Reach (current-main OFF logs, 2026-10-19)

Counted from the units host's per-slot summary `torpedo <name> goaway 009D0D90/009D3150: enters=N`
(`local/goaway_reach2.py`):

| mission | log | goaway enters |
| --- | --- | --- |
| JM08 | `cc9-physics1\local\p24_off_j8_def.log` | 9 (Hosho_sqn07/08/11/13 aircraft, 1 each) |
| USN01 | `cc9-physics1\local\p24_off_u1i.log` | 3 (KatTBD x3) |
| USN04 | `cc9-physics1\local\p24_off_u04.log` | 0 |
| USN21 | `cc9-physics1\local\p24_off_u21f_def.log` | 0 |
| BSM09 | `cc9-physics1\local\p24_off_b9_def.log` | 0 |
| BSM01 | `cc9-physics1\local\p24_off_b1_def.log` | 0 |
| JM06 | `cc9-physics1\local\p24_off_j6_def.log` | 0 |
| IJN08 | `cc9-lua52\local\l70r_w_i08_main_ref.log` | 0 |
| USN15 | `cc9-lua52\local\l70r_u15j_main_ref.log` | 0 |
| USNOS | `cc9-physics1\local\p23h_off_os.log` | 0 |
| LOMP06 | `cc9-lua52\local\l70r_w_l6_main_ref.log` | 0 |

So ON moves JM08 (9 enters, 18-27 draws) and USN01 (3 enters, 6-9 draws) directly, and every run
with a torpedo goaway indirectly through the shared stream.

### A.8 Not in this packet

The tick has the same two kinds of hole: `009D0FD7` `UniformFloatRange(3, 6)` and `009D1000`
`UniformFloatRange(row+10h, row+14h)`, taken only when the window re-seeds. The host hands the tick
precomputed low ends every tick (`in.window_jitter_draw`, `in.window_delay_draw`), so drawing them
needs the draw moved inside the re-seed branch of `torpedo_goaway_tick_009d0f10` (a callback), not
a value at the call. With the rows now loadable it is a small follow-up.

## B. The level-bomber rack's CanFire (006E5816): FAITHFUL in effect, ledger only

`006E5816` calls the rack's `vtable[1D0h]`. For both rack vtables (`00CF9878`, `00CF9AE8`) that slot
is `006E3460` `BSP_MBombPlatform_CanFire` = `00729A80(checkReload = 1)` and then
`vtable[210h](2Ah, 0)` (docs/CC13_SEG33A.md), not `00729A80` directly. The host's level-bomber rack
tick (`run_level_bomber_rack_tick_006e56f0`) records `Rack::can_fire_00729a80` and fires while the
rack has ammo. When the image's CanFire refuses, `006E58CF`-`006E58E1` clears `dropBombs` only if
`gun+3B8h` is set or the ammo `gun+484h` is not positive; otherwise the rack retries next tick. So a
gate could only delay a drop. Gate by gate:

| gate (`00729A80`) | for a rack | evidence |
| --- | --- | --- |
| `[[gun+3F8h]+34h] != 0` | passes | `+3F8h` is the fire-mode record (`0072D5B0`), `+34h` its bullet class; a rack without one never drops in the image either |
| `gun+3B8h == 0` | passes unless a failure request sets it | writers: `0072B5BC` (`0072B5A0` vtable slot `1C8h`, the destroyed-failure request), `007280C0` (clear), `0072A1BF` (network create). No plane-rack failure producer is modelled in the host |
| `gun+358h <= 0` | passes unless a damage write outside the gun segment raises it (not scanned) | the only write in `00720000`-`0073FFFF` is the constructor `0072E61A` |
| `gun+450h <= 0` (checkReload) | inert | writers: `0072E8DB` (setup, `XORPS` zero), `00730A08` (`BSP_Gun_Fire`), `00728B59` (an undefined 3-instruction setter with no reference: no rel32, no absolute dword). `00730160` sits only in non-rack vtables (`00CFBEF8`...`00CFE720`); the rack's `vtable[1D8h]` is `006E4D50`, the drop |
| `gun+478h <= 0` (checkReload) | inert | written by the constructor (0) and `0072D2C0` `BSP_Gun_SetTriggerHeld` (`U(0, 0.12)` at `0072D3B8`), which the rack overrides: its `vtable[1E8h]` is `006E3CB0`, `RET 4` |
| artillery / kind-7 unit cooldowns | pass | the rack's weapon type is `0Ah` BOMBPLATFORM (docs/WEAPON_CLASS_DESCRIPTOR.md), outside {2, 3, 4, 6} and 7 |
| muzzle y >= 1 or not AA/artillery kind | passes | `0Ah` is neither |
| a barrel with `[gun+414h][i] <= 0` | passes | barrel timers are seeded 0 at setup (`0072EE99` `FLDZ` -> `0072CF00`), only count down in `0072D130`, and are re-seeded only by `BSP_Gun_Fire`/`0072D520` rearm (not reached for a rack) or by a fire-mode switch (`0072D5B0`, FLT_MAX when the mode changes and the unit has no load), whose callers are ship gun bots, Lua bindings and `00954000`; no plane rack path reaches one |

Displacement scans of `006E0000`-`006E5FFF` (no `+414h`/`+450h`/`+478h` access on a gun; the two
`+448h`/`+450h` reads at `006E224E`/`006E226A` are a projectile's) and of `00720000`-`0073FFFF`
(control: `00729A80`'s own six reads all found) back the table. This agrees with
docs/GUNNERY_OPEN_ITEMS.md item 144 and adds the `+478h` producer and its override.

Verdict: no gate can block a level-bomb rack's fire in a normal mission. The host's "fires while
it has ammo" is faithful in effect; the residue is the damage gate `+3B8h` (a destroyed rack keeps
`dropBombs` and never drops), which needs a plane-rack failure producer the host does not have, so a
switch would have nothing to read. Ledger only; no switch.

## Uncertainties

- The PilotBot load is probe-tested, not game-tested. An ad hoc probe (`local/goaway_probe.cpp`,
  not in CMake) ran `load_pilot_bot_config_00901610` over this installation's loose script files
  (fundamentals.lua, luaMW_init.lua, its `DoFile("unlocks.lua")`, Robots.lua all read) and got all
  six rows, matching A.3's table and the host's existing SPNormal copies (TorpReleaseAlt 12,
  TorpReleaseDistNear 450, Far 650). In `bsp_game` the files come through the mission host's VFS
  reader instead; that path has not run (no game runs in this packet). The ON log line reports
  reads, misses and the six pairs, so the first ON run says whether it loaded.
- Region and X360COMP for the owner bootstrap are not reachable from the mission host; PC defaults
  are used and none of the three scripts reads them.
- Row capture timing (A.4).
- `009D0C10`'s RNG-freedom is from its direct callees only.
