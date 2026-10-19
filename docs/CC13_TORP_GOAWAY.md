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
  consumer in the run shifts. Expect unrelated movement in an ON pair from this.

What the switches restore is the VALUE RANGES, not a sequence. The image's stream 1 is per-thread
and seeded from the clock (docs/CC13_RNG_STREAM_CENSUS.md), so no run of the image reproduces a
particular sequence. The gap closed here is that the host pinned each draw to its low end (or 0)
where the image draws over a range. The draws still sit on their own sub-switches in the image's
order, so the host's stream-1 consumption per event matches the image's count and order.

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
| BSM01 | `cc9-physics1\local\p24_off_b1_def.log` | **56** (Jap #3.1 ... Jap #80.1 squadrons; `Jap #13.1|.-III` alone 44). Corrected 2026-10-19: the first count's `\S+` name pattern skipped names with spaces |
| JM06 | `cc9-physics1\local\p24_off_j6_def.log` | 0 |
| IJN08 | `cc9-lua52\local\l70r_w_i08_main_ref.log` | 0 |
| USN15 | `cc9-lua52\local\l70r_u15j_main_ref.log` | 0 |
| USNOS | `cc9-physics1\local\p23h_off_os.log` | 0 |
| LOMP06 | `cc9-lua52\local\l70r_w_l6_main_ref.log` | 0 |

So ON moves JM08 (9 enters, 18-27 draws), USN01 (3 enters, 6-9 draws) and BSM01 (56 enters,
112-168 draws) directly, and every run with a torpedo goaway indirectly through the shared stream.
The same eleven logs are the one current-main set used for every count in this document (A.8, C.4).

### A.8 The tick's two draws (scope addition)

`009D0F10` `BSP_BotStateTorpedoGoAway_Tick`, `RET 4`. The re-seed branch `009D0FB2`-`009D1025`
(entered when `+28h` < 0, the `JBE` at `009D0FB6`, and the aircraft is above `+20h`, `BL` at
`009D0FB8`) draws twice on stream 1:

| site | low, high | result |
| --- | --- | --- |
| `009D0FD7` (`ECX = 1` at `009D0FC9`) | `[00CE3854]` 3.0f, `[00CE6630]` 6.0f | `FSTP [EDI+34h]` at `009D0FDE`, the window length |
| `009D1000` (`ECX = 1` at `009D0FF1`) | `[row+10h]`, `[row+14h]`, row = `[[ESI]+14h]` (`009D0FDC`-`009D0FFA`) | plus `+34h` (`009D1016` `FADDP`), `FSTP [EDI+28h]` at `009D101A` |

Before this packet the host handed the tick fixed values every tick (`kWindowJitterLo` = 3 and 0),
so it took no draw. A value at the call cannot fix that, because the image draws only on a re-seed.
So `TorpedoGoAwayTickInputs` gains `reseed_draws`: when set, `torpedo_goaway_tick_009d0f10` asks
the hook for `window_jitter_009d0fd7()` and then `window_delay_009d1000()` inside the re-seed branch, in the
image's order; null keeps the old inputs. The host passes a binding only when a tick sub-switch is
on. Each method draws when its own sub-switch is on and otherwise returns the OFF value.

Switches (all false), under the enter's parent `kTorpGoawayEnterDrawsBound`: `kTorpGoawayTickJitterDrawBound`
(`009D0FD7`), `kTorpGoawayTickDelayDrawBound` (`009D1000`, which also needs
`kPilotBotConfigLoadBound`). The delay reads the same row as the enter's `009D0EA3`.

Two caveats from rev4:
- **A failed load.** The delay method needs the PilotBot rows loaded. When
  `load_pilot_bot_config_00901610` failed, or the row index is outside 0..5, it returns 0 without
  drawing. The stream is then one draw short on every re-seed, against the image's two. The ON log
  line of the load (A.4) says whether the rows loaded.
- **Row index timing. UNCERTAIN, as for the enter (A.4).** The image reads the row as
  `[[state+4]+14h]`, the pointer it captured when it built the approach (`009F9D22`). The host reads
  `pilot_skill_index` at the tick. The two differ only if a writer of bot+34h (`009565A0`,
  `007B8AE0`) runs in between.

x87: `009D1014`-`009D101A` adds the float delay draw to the float `+34h` and stores a float. That is
one float add, the same at 24 and 53 bits, and `delay + state.window_length_34` matches it.
`009D101D`-`009D1025` (`+30h` = -(`+34h` x 0.5)) is unchanged code.

ON effect: each weave lasts [3, 6) s instead of 3 s. The gap to the next weave is TorpFlikFlakTime
on top of that ([12, 22) s SPNormal) instead of 0, so weaves come every 15 to 28 s instead of back
to back.

Reach and movers. The host does not log re-seeds; its per-slot goaway tick summary
(`goaway 009D0F10: ticks=N arms[win_low win_high ...]`) gives the exposure. In the current-main OFF
logs, JM08 (`p24_off_j8_def`) has 380 goaway ticks, of which 204 are in an open window. USN01
(`p24_off_u1i`) has 348 ticks, 295 of them in a window (KatTBD alone: 151 of 167). BSM01
(`p24_off_b1_def`) has 871 ticks, 531 in a window (missed in the first count, see A.7). The other
eight WINS rows have no torpedo goaway (A.7). ON, the open share should fall to about 3-6 s in every 15-28
s of countdown inside the ring. That moves the goaway arm mix (`win_*` towards `post_*`), the
commanded altitude and roll, and therefore escape paths and AA exposure in JM08, USN01 and BSM01. Every
re-seed also takes two stream-1 draws.

ON-only lifetime: the binding object lives on the stack for one tick call. It reads the same
process-wide rows as the enter (A.6), `owner_.gunnery`, and the slot's `pilot_skill_index`.

Load timing: `009013D0` (register) has one caller, `00901610`, which calls it nine times (once per
descriptor). `00901610`'s one caller is `004DC6A0`, once per process. The host runs the same loader
once per process, at the first mission's settings load, before any unit exists. It is therefore
loaded before the first goaway enter or tick, as in the image.

## B. The level-bomber rack's CanFire (006E5816): FAITHFUL in effect, ledger only

`006E5816` calls the rack's `vtable[1D0h]`. For both rack vtables (`00CF96A8` and `00CF9918`; their `+1D0h` slot entries are at `00CF9878` and
`00CF9AE8`) that slot
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
| `gun+450h <= 0` (checkReload) | inert | writers: `0072E8DB` (setup, `XORPS` zero), `00730A08` (`BSP_Gun_Fire`), `00728B59` (an undefined 3-instruction setter with no reference: no rel32, no absolute dword). `00730160` sits only in slot entries of non-rack vtables (the entries at `00CFBEF8`...`00CFE720`); the rack's `vtable[1D8h]` is `006E4D50`, the drop |
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

## C. The dive-bomb goaway: three draws and the target term (follow-up packet)

Source: docs/CC13_FRONTIER_MERGE.md section 6.5 (cc13-frontier, `f932f7d11`). The dive goaway
state is `task+704h` (vtable `00D20CA0`); the host's turn lives in `src/dive_bomb_goaway_turn.cpp`.
The `+68h` desired direction (`009C4E27`-`009C4E58`) belongs to cc13-steergate and is not touched.

### C.1 The draws (re-read from the bytes)

| call | stream | bounds | result |
| --- | --- | --- | --- |
| `009C49FC` (enter `009C4950`) | `ECX = 1` at `009C49F2` | `1.0` (`FLD1` `009C49F7`), `1.25` (`[00CF29A8]` `009C49E5`) | `009C4A01 FMUL [ESI+20h]`, then `FSTP`/`FLD`/`FST [ESI+20h]` |
| `009C4D13` (tick, re-roll) | `ECX = EBX` at `009C4D08` | `3.0` (`[00CE3854]`), `6.0` (`[00CE6630]`) | `FSTP [ESI+2Ch]` at `009C4D18` |
| `009C4D33` (tick, re-roll) | `ECX = EBX` at `009C4D24` | `12.0` (`[00CEB4B8]`), `15.0` (`[00CE5380]`) | `FADD [ESI+2Ch]`, `FSTP [ESI+24h]` at `009C4D3E` |

The enter's draw is unconditional; it comes after the target term (`009C49E5` is the join of both
`JE`s). `EBX = 1` at `009C4A5C`. From there to `009C4D08`, the only other `EBX` instruction is
`009C4CB3 POP EBX`, on the `RET 4` path, which never reaches the re-roll. Both re-roll draws are
inside `009C4CF1`'s `0 > +24h` guard.

### C.2 The host and the switches (include/bsp/cc13_torp_goaway.hpp, all false)

- `kDiveGoawayDrawsCc13Bound` is the parent, with one sub-switch per draw in image order:
  `kDiveGoawayStandoffDrawBound` (`009C49FC`), `kDiveGoawayWindowDrawBound` (`009C4D13`) and
  `kDiveGoawayCountdownDrawBound` (`009C4D33`).
- When a sub-switch is on, `run_dive_bomb_goaway_enter_009c4950` or the re-roll block draws through
  `owner_.gunnery->death_mode_draw_00bd2f10(1, index_of_slot(unit_), lo, hi)` in place of the low end.
- The re-roll draws are taken only when `0.0f > gt.countdown_24`, which is `009C4CF1`'s own test on
  the same state just before `dive_bomb_goaway_reroll_009c4cf1` runs it. A tick without a re-roll
  therefore takes no draw, as in the image. No new state.
- OFF keeps 1.0, 3.0 and 12.0 exactly as before. `if constexpr` removes the draw code, so the OFF
  run is unchanged.
- Stream 1 is per-thread and clock-seeded in the image (see A), so these switches restore the value
  ranges, not the image's sequences.

x87: the enter's `FMUL` of the draw by `+20h` is rounded to float by `009C4A08 FSTP`. The re-roll's
`FADD` is rounded by `FSTP [ESI+24h]`. At the 24-bit control word (the main thread), both match the
host's float arithmetic exactly. At 53 bits, the draw's `ST0` would carry more than float precision
into the `FMUL`/`FADD`. The host's draw returns a float, so it can differ by 1 float ulp there. Modelled
at 24 bits; 53 recorded.

### C.3 The target term, one OFF switch per enter (dive, torpedo, level; rocket unhosted)

The four enters that call `007B5BE0`, re-read from the bytes. Each tests the target for non-null
and `vtable[5Ch](5)` first. The bases and multipliers differ:

| enter | target | base the term competes with | compare | after | switch |
| --- | --- | --- | --- | --- | --- |
| dive `009C4950` | `[approach+48h]` (`009C499C`) | `2 * approach+B4h` (`009C4990 FLD`, `FADD ST0,ST0`) | `009C49D0 JBE`: equal takes the term | `* U(1, 1.25)`, `* 1.5` when ctl+369h and `[00E17BF2]` | `kDiveGoawayTargetExtentCc13Bound` |
| torpedo `009D0D90` | `[approach+CCh]` (`009D0DC9`) | SafeDist `tuning+438h` (`009D0DD1`), 700 in planeglobals.lua:612 | `009D0E07 JA`: base kept only if strictly larger | `* U(1, 1.15)` | `kTorpGoawayTargetExtentCc13Bound` |
| level `009B5680` | `[approach+48h]` (`009B56B9`) | SafeDist `tuning+450h` (`009B56BE`), 1000 in planeglobals.lua:535 | `009B56F4 JA` | `min(.., 3500)` (`009B5702`-`009B571B`: qword `00D046A0`, float `00D04698`), then `* U(1.05 [00CE780C], 1.2 [00CE3814])` (`009B5746`, ECX = 1) and `* approach+24h` | `kLevelGoawayTargetExtentCc13Bound` |
| rocket `007B5F10` | `[approach+70h]` (`007B5F6A`) | `tuning+664h` Strike/AttackDist (1800, planeglobals.lua:603) `* 0.8` (qword `00CE3D40`) = 1440 | `007B5F9F JA` | `* U(1, 1.1)` (`00CE6448`) | none: the host runs no rocket task (`RocketTaskHost` has no implementer), so there is nothing to bind |

The level row's draw is the host's existing `lb_draw("ga18", kGoAwayJitterLow, kGoAwayJitterHigh)`,
unchanged by this packet.

- **Dive enter.** `009C499C`-`009C49E0`: when `[approach+48h]` (the latched target) is non-null and
  its `vtable[5Ch](5)` answers true, the code calls `007B5BE0` on it and takes
  `+20h = max(+20h, result)`. The comparison is `FCOMIP`/`JBE`, so an equal value takes the
  result.
- **Torpedo enter.** `009D0DC9`-`009D0E0F` has the same arm on `[approach+CCh]` against SafeDist
  (`JA` keeps SafeDist only when it is strictly larger).
- **Level enter.** `009B56B6`-`009B5702`, the same arm on `[approach+48h]` against SafeDist. Its
  host target is the latched `lb_target_plus_one` (the level task's own copy of the command target,
  `009B9030`'s install). `level_bomb_goaway_standoff_base` already had the arm and the cap; the
  switch only feeds it.
- **The kind test.** Kind 5 is "a unit" (docs/UNIT_KIND_QUERY.md: every ship, plane, land vehicle,
  fort, command building, airfield and shipyard), so any unit target passes.
- **`007B5BE0` is not a physical extent.** It computes `m = (+448h > +444h) ? +448h : +444h`
  (`FCOMPI`/`JA`), then `(m > +434h) ? m : +434h` (`JBE`). Those fields are `00956C20`'s
  `unit+430h + cat*4` longest gun ranges for category 6 LIGHTARTILLERYFLAK, 5 FLAK and 1
  AAMACHINEGUN. This is the same triple `009FD66F`-`009FD6F9` uses for the fly-to obstacle radius
  (docs/TORPEDO_AIM_LEAD.md). So the standoff grows to clear the target's longest AA reach. The
  old Ghidra name `BSP_UnitInstance_GetLargestExtent` and earlier "half-extent" readings
  (docs/CC13_VERIFY8.md L4, now corrected) were wrong about what it measures. It was renamed
  `BSP_Unit_LargestAaGunRange` in Ghidra and the ledger on 2026-10-19. The old name and evidence
  are kept in the ledger record, and the prior annotation is in `local/ghidra-annotations-20261019T122232Z.json`.
  The reconstruction keeps its function name `unit_largest_extent_007b5be0` (src/cc13_seg47.cpp,
  another lane's file).
- **Host binding.** For the dive and the torpedo, the target is `command_target_plus_one`, the same
  substitution the dive heading (`goaway_turn_heading_009c47d0`) and the torpedo run-in latch
  already use. For the level, it is `lb_target_plus_one`. Every host slot is a unit, so the kind test
  is true whenever there is a target. The ranges come from the gunnery host's
  `GameGunneryUnitRow::category_ranges[1/5/6]`, the same source `fly_to_obstacles_009fd63c` reads.
  The image's comparison order is kept, so a NaN takes the image's arm.
- **What the ranges are.** Each value is `00956D63`'s longest ammunition-record range in the
  category (`00731020`), including a category 6 flak gun's second ammunition. So the bullet-class
  ranges the gunnery host logs ("bullet class N Type=... range=R (first used by / second ammunition
  of category C)") are exactly the candidate values.
- **The measurement.** `GameUnitsHost::Impl::goaway_target_term` runs at all three enters whether
  or not a switch is on. It touches only per-slot counters, which are new and log-only. The exit
  summary prints one line per slot that entered goaway:
  `goaway target term <dive|torpedo|level> <name>: enters= targets= moved= aa_max= base=lo..hi`.
  `moved` counts the enters where the term exceeded the base. This is the per-mission measurement
  the pairs should read. It is a new line in OFF runs too, but no behaviour moves.

ON-only lookup lifetime: `category_ranges` is filled by `GameGunneryHost::Impl::build_guns` when a
unit's guns are built (`00956C20`'s host binding), and nothing clears it. `unit_rows()` returns a
fresh copy on every call; the enter reads it once, by value, at the enter. The image re-runs
`00956C20` when a unit's weapon set changes; the host does not, so a target that has lost guns keeps
its build-time ranges. The target slot comes from `owner_.slots`, which owns every unit for the
mission. No lookup crosses a mission boundary.

### C.4 Reach (one current-main log set)

All counts use A.7's eleven current-main OFF logs: JM08 `p24_off_j8_def`, USN04 `p24_off_u04`, USN21
`p24_off_u21f_def`, USN01 `p24_off_u1i`, BSM09 `p24_off_b9_def`, BSM01 `p24_off_b1_def` and JM06
`p24_off_j6_def` (all `cc9-physics1\local`); IJN08 `l70r_w_i08_main_ref`, USN15 `l70r_u15j_main_ref`
and LOMP06 `l70r_w_l6_main_ref` (`cc9-lua52\local`); USNOS `p23h_off_os` (`cc9-physics1\local`).

They were summed with `local/goaway_aa_survey.py` from these lines:
- dive: `divebomb <name> goaway turn 009C4DAD: enters= ... rerolls=`;
- torpedo: `torpedo <name> goaway 009D0D90/009D3150: enters=` and `goaway 009D0F10: ticks= arms[win_low= win_high=`;
- level: the `levelbomb <name> ... -> LevelBomb/goaway` transitions.

Squadron names contain spaces ("MainAirFieldEntity 01_sqn05", "Jap #13.1"). The first counts used a
`\S+` name pattern and dropped those planes. That is how JM08's dive and level enters and BSM01's
torpedo enters were missed (rev4 B1).

| mission | dive enters | dive re-rolls | torpedo enters | torpedo ticks / in window | level enters |
| --- | --- | --- | --- | --- | --- |
| JM08 | 9 | 0 | 9 | 380 / 204 | 85 |
| USN04 | 1 | 0 | 0 | 0 | 0 |
| USN21 | 1 | 0 | 0 | 0 | 0 |
| USN01 | 2 | 0 | 3 | 348 / 295 | 0 |
| BSM01 | 0 | 0 | 56 | 871 / 531 | 0 |
| IJN08 | 0 | 0 | 0 | 0 | 144 |
| USNOS | 9 | 3 | 0 | 0 | 82 |
| BSM09, USN15, LOMP06, JM06 | 0 | 0 | 0 | 0 | 0 |

The dive planes:
- JM08: `MainAirFieldEntity 01` sqn01, 05, 06, 09 and 24, with their wingmen.
- USN04: `D3A Val #1.1|.-IV`.
- USN21: `Enterprise_sqn137`.
- USN01: `KatSBD|.-II` and `|.-IV`.
- USNOS: Intrepid sqn09, 20, 22 and 27, and `Essex_sqn14|.-II`.

Movers with the dive draws ON: the dive standoff `+20h` (up to x1.25), which is `009C7F00`'s
completion distance and so how long goaway lasts. This affects JM08, USN04, USN21, USN01 and USNOS.
In USNOS, the only mission with re-rolls, the bank window `+2Ch` and the re-roll countdown `+24h`
also move (both up to +3 s before the altitude scale). Every draw also advances the shared stream 1,
so other stream-1 consumers in those missions shift, AA kill counts among them.

Pairs by mechanism:
- dive draws: JM08, USN04, USN21, USN01, USNOS;
- torpedo draws: JM08, USN01, BSM01;
- dive target term: JM08, USN04, USN21, USN01;
- torpedo target term: JM08, USN01, BSM01;
- level target term: JM08, IJN08, USNOS.

### C.5 The target's AA reach against each enter's standoff (computed from data)

**Method** (`local/goaway_aa_targets.py`):
- **Targets.** The target of each goaway plane comes from the same logs:
  - dive and torpedo: `command target 0071EBF0: unit=<plane or its leader> ... -> <target>`;
  - level: `level-bomb task 009B9030 kind 4 installed <plane> -> <target>`.
- **Classes.** A target's class comes from `GenerateObject ... (type=N`, `hull shapes <name> (type N`
  or `unit hull input unit=<name> type_id=N`.
- **Guns.** For each class, `VehicleClass[id].Platforms[k].Gun[1]` (scripts/datatables/autoload/
  vehicleclasses.lua) leads to `DeviceClass` (classtables/arcade/deviceclasses.lua; this
  installation's gamemode.lua sets `GameMode = 0`, arcade). From that come the gun's `Function` (the
  category) and its first and second `Bullet` classes. This is the walk the gunnery host's flatten
  chunk makes.
- **Ranges.** A bullet class's range is the derived engagement range the gunnery host logs on its
  `bullet class N ... range=R` line, which is the value `00956D63` compares.
- **AA reach.** It is `max(max(c6, c5), c1)` with the seed 10, computed two ways:
  - **host**: first ammunition only, which is what `category_ranges` holds today;
  - **image**: a category 6 flak gun's second ammunition is also offered, as `00956DB1`-`00956DD0`
    does through `[[[gun+3F4h]+74h]+7Ch]+60h`.
- **Bases.**
  - Dive: `2 * approach+B4h`. The logged final `+20h` equals the base when `[00E17BF2]` is clear.
    It is the base times 1.5 when the flag is set, which happens in JM08 and USN04 (`device reload ...
    now=1`); there the base is standoff / 1.5.
  - Torpedo: SafeDist 700.
  - Level: SafeDist 1000, with the 3500 cap never reached.

| mission | enter | enters | target (class) | AA reach host / image | base | term moves the standoff? |
| --- | --- | --- | --- | --- | --- | --- |
| JM08 | dive | 7 | Missouri (15), USTroopTransport 06 (234) | 1600 / 2000 | 1275-1554 | yes |
| JM08 | dive | 2 | USTroopTransport 06 (234); LST 01 (41) and USTroopTransport 01 (234) | 1600 / 2000; LST 1600 / 1600 | 1340-1393 | yes |
| JM08 | torpedo | 9 | Nevada (314) | 1500 / 2000 | 700 | yes |
| JM08 | level | 77 + 8 | Missouri (15); USTroopTransport 04/05/06 (234) | 1600 / 2000 | 1000 | yes |
| USN04 | dive | 1 | Lexington-class01 (1) | 2000 / 2000 | 1848 | yes |
| USN21 | dive | 1 | Suzuya (67) | 2000 / 2000 | 1654 | yes |
| USN01 | dive | 2 | Katori (71) | 2000 / 2000 | 1607-1758 | yes |
| USN01 | torpedo | 3 | Katori (71) | 2000 / 2000 | 700 | yes |
| BSM01 | torpedo | 47 / 5 / 1 / 2 / 1 | Utah (13) / Nevada (314) / Oklahoma (314) / Maryland (259) / California (260) | 2000 / 1500-2000 / 1500-2000 / 2000 / 2000 | 700 | yes |
| IJN08 | level | 136 + 8 | Maru2, Maru1 (224) | 1600 / 2000 | 1000 | yes |
| USNOS | dive | 9 | "Hangar, Small, 02 02" (550), Multi Hangar 1 (594) | 10 / 10 (no guns) | 1634-2045 | no |
| USNOS | level | 82 | TroopTrans2-6 (234) | 1600 / 2000 | 1000 | yes |
| any | rocket | 0 | - | - | 1440 | not hosted |

**Reading the table.**
- **Where the term moves.** It moves every live enter whose target is a ship, which is all of them
  except USNOS's dive bombers, whose targets are hangars. Where it moves, the standoff rises:
  - dive: from 1100-1850 to 1600-2000;
  - torpedo: from 700 to 1500-2000;
  - level: from 1000 to 1600-2000.
- **What changes.** Goaway lasts longer and the plane breaks off further out, so AA exposure and
  re-attack timing change.
- **Residual: host and image disagree.** For the classes whose AA reach comes from a category 6
  flak gun (Missouri, the transports, Nevada and Oklahoma, the Marus), the host's
  `category_ranges[6]` holds only the first ammunition. That gives 1500-1600 where the image's
  rebuild takes the flak shell's 2000. With a target-term switch ON, the host's standoff is
  therefore up to 400 m short of the image's for those targets. The fix belongs to the gunnery
  rebuild binding (`flak_alternate_range = gun.max_range` in `GameGunneryHost::Impl`), not to this
  packet. The same stand-in shrinks `fly_to_obstacles_009fd63c`'s radius.
- **Uncertain:**
  - A plane with two targets over the run is listed against both; the enter used whichever was
    current.
  - In JM08 and USN04, the 1.5 assumes `[00E17BF2]` was set before the enters (the log reports it
    at exit, with one set call).
  - The image column reads `+74h`/`+7Ch` as the device's second bullet entry.

The new per-slot exit line `goaway target term ...` (C.3) measures the host value and the moved
count directly in physics24's pairs.

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
