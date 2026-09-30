# Gameplay gap ranking

Addresses: 007F16D0, 006C0840, 009C3647, 009C3100, 0071E430, 006C0750, 00758270, 009F2124, 00A2B8F0,
009FFEB0, 009C359F, 009C23B0, 009229F0, 007AC9D0, 007325A0, 00730762, 008AD4CD, 0071C1E0, 009C207C,
006D2510, 006D40F0, 00846320

Packet `cc9_gameplay_gap_ranking_2`, worker cc9-lua19, 2026-09-29 (stamped 20:06 UTC). It refreshes
the first ranking (packet `cc9_gameplay_gap_ranking`, cc9-ships14, 11:29 UTC), whose rows are now
the closed list at the end. Read-only: no switch, no Ghidra write.

## Sources

- **One binary.** A no-flip `tools/pair_export.py --commit 31de7f88a` export of main,
  `local\l19_main\build\win32\Release\bsp_game.exe` in the cc9-lua19 tree, SHA-256 prefix
  `CAB704E1D6E3`.
- **Launch form.** Reference p's: `--frames F --press-start-frame 30 --menu-select M --mission-frames N
  --mission-frame-seconds 0.05`, with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`
  (`local\l19_runs.ps1`). The logs are `local\l19_main_<row>.log`.
- **Rows that ran.** USN04, USN13, JM05, IJN01 and LOMP10 at 3200/3000, and JM05 at 9200/9000.
- **Rows that did not run.** USN01, JM08, E2 (USN04 9000), USN13 9000, LOMP10 9000 and IJN01 9000.
  - They died at renderer init. `CreateDevice` returned null with hr `0x8876086A`, and the log shows
    `logonui=1/1`.
  - This is the environment, not the code: the 300-frame USN01 smoke fails the same way.
  - The first batch lost its device mid-run. USN13, IJN01 and JM05 9000 then logged `present failed
    hr=0x88760868` (D3DERR_DEVICELOST). Each one still simulated every mission frame (`ran=N
    simulated=N`), so its gameplay counters are used here, but it is not a reference row.
- **Census tools.**
  - `local\l19_gap.py` is ships14's `s14_gap.py`. It lists the UNIMPLEMENTED host-method rows.
  - `local\l19_nz.py` lists every nonzero refusal-type counter of a `summary` line
    (refus/unbound/stand-in/missing/unresolved/fallback/unread/skipped), and every line that labels
    a substitution or stand-in.
  - `local\l19_site.py` prints the source lines before each record site.
- **The totals.** 598 UNIMPLEMENTED rows over the six logs, and 36 nonzero refusal-type counters.
- **Method.** Each candidate was checked against its record site's comment and the row's outcome
  counters. An UNIMPLEMENTED count was taken as a gap only where the recorded answer differs from
  what the image would do on a live path.

**The reruns, 2026-09-29 (after about 20:40 UTC).** The six missing rows were run on the same
binary. USN01, JM08, E2 and LOMP10 9000 ran clean. USN13 9000 and IJN01 9000 lost the device again
but simulated every frame. What they add to the table below:
- **#2, moveto arrivals:** E2 14 and JM08 9.
- **#3, carrier decks refused:** E2 2, USN13 9000 9, USN01 1, IJN01 9000 1.
- **#4, retarget-reachable frames:** IJN01 9000 6242, LOMP10 9000 1801, USN01 1745.
- **Land tasks:** LOMP10 9000 installs 10 and IJN01 9000 installs 4. Both rows have a single site,
  so #1 does not apply.

## Top 15 new gaps

Reach is from the runs above (calls or units per row). "Mine" means the units / landing / air-ops
lane (cc9-lua19).

| # | gap (address) | reach | gameplay consequence | lane | suggested packet |
| --- | --- | --- | --- | --- | --- |
| 1 | **The squadron return-to-base site key is never computed.** The record path `record_return_to_base_007f16d0` builds 006C0840's candidates with `key_known = false` (`game_hosts_units.cpp` near line 9357). So with two or more sites past the filters, `nearest_landing_site_006c0840` flags `site-key-unread`, and `cc9_land_task_reach` refuses every land task ("the resolution carries an unread input"). The plane-side caller `plane_landing_site_006c0840` (near line 9855) already computes the key: 006C09FE-006C0A9B, through `landing_deck_006c0750`, `006BC530` and `006BCC90`. | JM05: every squadron with a placement, 16 at 3000 frames (`refused=1680`) and 25 at 9000 (`5493`); placements 148-441 per squadron, installs 0. | Squadrons ordered home never get a land task, so they never land, rearm or relaunch on JM05. | units (mine) | `cc9_rtb_site_key`: compute the key in the record path with the plane-side helpers. Bind behind a switch; pair on JM05 3000 and 9000. |
| 2 | **Reaching a moveto point never ends the command.** Once `approach+5Ch` is set at `009C3636`, `009C3647` calls approach `vtable[8]` = `009C3100` every tick. That routine tests task `vtable[40h]` (`009C31B0`, the squadron's current command through `+404h`), then ends the moveto (`00E08F68`) through the control block's `vtable[114h]` and `BSP_WeaponDirector_EndCommand 0071E430`. The host records the call (`BotApproachMoveTo::arrived_vtable8`) and the flight circles at the point. | USN04: 7 escorts arrive (A6M Zero #1.2-#4.2, 1616 calls). JM05 9000: 18 US planes (Lexington sqn14/16/17, Yorktown sqn12 and others, 27723 calls). | What the flight does after arriving. PILOT_MOVETO_TASK records that E2's `IJNFightersLex` escorts orbit inside the carrier group's AA and all die by 240 s, which ends phase 1. With the command ended, the squadron goes back to its planner or script. | units (mine) | **Closed**: `kMoveToArrivalEndCommandBound` ON, stage-only; the circling is the image's own (docs/PILOT_MOVETO_TASK.md, `d05adce62`) |
| 3 | **Carrier decks refuse landings.** `landing_deck_006c0750` returns null for a mother-ship holder ("refreshed from the moving ship; that refresh is unread"). Every carrier's sequencer row reads `refused=1`. The refresh lives in the carrier motion remainder `00758270`, the first ranking's #10, which still records its override remainder. | USN04: 2 carriers. USN13: 8 (Essex, Intrepid, Yorktown, Cabot, Cowpens, Monterey, Hill, Wood). JM05: 4. `00758270` records 3000-12000 calls per row. | No aircraft can land on a carrier. Once #1 opens, JM05's carrier squadrons meet this next. | units (mine) | `cc9_carrier_deck_refresh`: read the `+88h..+94h` / deck-pose refresh in `00758270` and bind the carrier holder |
| 4 | **The approach retarget arm** `009F2124-009F272D` (`kShipAiApproachRetargetRingBound` OFF by verdict, SHIP_AI_OPEN_ITEMS 27). The goal is copied every frame instead. | IJN01 now reaches it: `retarget_reachable=6071` of 6100 approach frames, 718 entries. JM05 9000: 3845; LOMP10: 601. | A ship attacking a coastal target steers at the building rather than a point off the coast. | ship AI | re-pair section 27 on IJN01, a new reach |
| 5 | **The AI group reference release** `00A2B8F0` from `00A2E784` (SHIP_AI_OPEN_ITEMS rank 3). | 6 of 6 rows, 58583 calls | group lifetime: groups are not released, which touches regrouping and orders | ship AI / AI | **Closed**: `kAiGroupScoreListReleaseBound` ON, gameplay-identical (SHIP_AI_OPEN_ITEMS 61, `5f8a0e6c6`) |
| 6 | **The carrier arm of the squadron exclusion** `009FFEB0`. The host answers `007EDA90`'s false. | 6 of 6 rows, 38351 calls | whether a carrier's squadrons in an AI group take group orders | AI, bordering planes | read the carrier arm |
| 7 | **The squadron's `+348h` command block** (`009C359F`, `approach+6Ch`). 0 stands in, so the moveto circle radius is TurnCircleRadius; a mission-authored radius is lost (PILOT_MOVETO_TASK "Substitutions"). | USN04 12064, JM05 9000 30222 | the moveto orbit radius and the arrival ring | units (mine) | model the `+348h` block's `+6Ch` from the moveto command, with #2 |
| 8 | **The moveto target-speed override** `009C23B0`: within TurnCircleRadius + 50 m and faster than `007C47F0`, the image lowers the desired speed; the host records it. | USN04 6284, JM05 9000 10074 | the speed on the final approach to a moveto point, and so the arrival time (#2) | units (mine) | read `009C23B0`; bind with #2 |
| 9 | **The command acceptance extra tests.** `WeaponDirector::command_allowed_extra_test` records `009229F0` (torpedo) and `007AC9D0` (moveonpath), whose bodies are unread; the host accepts. | 5 rows, 1744 calls (JM05 572, JM05 9000 981, USN04 114) | torpedo and path orders the image may refuse are carried out | commands (gunnery14) | read both bodies; bind the refusals |
| 10 | **The gun barrel count fallback.** `cc9_gun_barrel_count` takes the muzzle count from the device model (`007325A0` / `0072AB80`). When the model's fire points did not load, the Lua `barrels` value stays. | USN04: 315 of 726 guns. JM05 9000: 397 of 1312. | Fire volume per mount. Uncertain: the fallback guns may be aircraft devices whose count is exact anyway. | gunnery | list the fallback devices by class; decide whether any is a ship mount |
| 11 | **The muzzle offset fallback** (`cc9_muzzle_offsets`, `00730762` / `00859550`). A shot without a loaded offset fires from the mount origin. | JM05: 98 of the shots; JM05 9000: 1311 of 6822. USN04: 0. | shot origin: a few metres on hit geometry and line of fire | gunnery | **Closed**: `no_mount` was the land classes' missing slot mounts; `kLandPlatformAttachmentBound` ON (docs/GUN_BARREL_COUNT.md 9, `2e508be11`) |
| 12 | **`RepairEnable` on a ship is dropped.** Every ship answers `IsKindOf(6)`, so `008AD330` routes the 9Fh message (`008AD4CD`, `Session::route_repair_enable_message`). The host writes `row.repair`, which nothing reads. Damage control keeps `hull_repair_enabled = true` (`game_hosts_gunnery.cpp` near line 3424). | USN13 52, USN04 18, LOMP10 10 calls. usn_13_truk.lua (this installation) sets true on difficulty 0/1 and false on 2. | None at difficulty 0/1, where the host's default matches. At difficulty 2 the player's ships repair when the script says they must not. | gunnery (damage control) / lua | route the flag into the damage-control task's `+45h` |
| 13 | **The navigator evasion setters** `NavigatorSetTorpedoEvasion` / `NavigatorSetAvoidLandCollision` reach `0071C1E0` (`Navigator::avoidance_receiver_torpedo` / `_land`). The host counts them. | USN13 52 + 52, USN04 18 + 18, JM05 2 + 2 | Probably none. The consumer `blk+3ECh` and its setters `009DABB0` / `009DABD0` have no caller found (`ship_ai_avoidance_request.hpp` marks them Unused). | ship AI | a caller census of `blk+3ECh`, then close or bind |
| 14 | **The follow trail arm** `009C207C-009C211C` (`+85h`, `unit+844h/+840h`). `+85h` is raised by `009BFD70` when a follower is in position (BOMBER_AFTER_TASK). | USN04 5780, JM05 9000 20148 calls | formation keeping of in-position wingmen | units (mine) | read the arm; bind behind a switch |
| 15 | **The airfield per-step slot** `006D2510`. The host runs its air-ops block elsewhere (AIROPS_LAUNCH_TICK, "A deviation of position"), but not its destruction slot `006D40F0`. When no hangar at `+830h` has hp `[+370h] > 0`, that slot kills the airfield (`vtable[70h](0)`), or plays `"InferiorFailure"` for a child airfield. | 0 on these rows: no hangar died on JM05 9000, which has five hangar forts. | an airfield whose hangars are all destroyed stays alive | units (mine) | park until a row destroys a hangar |

## Still open from the first ranking

- **AA lethality at 300-800 m (first #1).** Gunnery: AA_LETHALITY_AUDIT 8.x;
  `kPlaneHitTaskNotifyBound` is ON, and `kDiveHitClockBound` stays OFF.
- **The approach sub-throttle and sub-heading producers (first #15).** Parked. The IJN01 calls
  (6055) are the approach reading the fields on every frame, not a submarine approach
  (SHIP_AI_OPEN_ITEMS, "Not ranked").
- **The carrier motion remainder (first #10)** is now part of #3.

## Closed (first ranking)

- **#3**, the kamikaze boat attack step: ON.
- **#4**, touchdown at 3000 frames: expected. The land-task `unread` rows are #1 above.
- **#5**, the wingman heading law: no gap.
- **#6**, the approach lead: ON.
- **#7**, the player gun seat: groups 1-3 ON; groups 4 and 5 have no reach.
- **#8**, the planner spawn: no authored stock on these rows. The shipyard tick `00846320` spawns
  only from the same `+790h` build orders, so it is inert here too.
- **#9**, the scripted-order natives: all bound. The ship fire-stance arm is ON (GUNNERY 61).
  `Scoring_SetMissionCompleted` is still open.
- **#11**, the periscope: ON.
- **#12**, the kill handlers: presentation, plus the firing-list removal routed to lua16 (GUNNERY 64).
- **#13**, `GetCapturePercentage`: faithful 0.
- **#14**, `Kill` of a script entity: ON.

## Checked and not gaps

- **`ai follow refused=`** (every row) is the image's own answer. AI_CAUTIOUS_ROUTE shows `00779D50`
  refusing members of the same group and ships of the other party.
- **`ai world sets ... Objectives_Add is unimplemented`**: the summary wording was stale. `008CD440`
  is bound, and the sets are empty because the scripts pass no targets (AI_WORLD_SETS, the
  correction to section 2).
- **`FindEntity` UNIMPLEMENTED on JM05** is the first-call status only.
- **`DamageControl::pending_exceeds_health_0090e6c0`**: `0090E6C0` is tagged `stl_probable`.
- **USN02's failure at 29.75 s** is the image's own for an idle player (GAME_EXECUTABLE, reference i).
- **`AutoTarget::director_command_state`** (320430 calls): exact while no
  `WeaponDirector::queue_command` runs (GUNNERY_OPEN_ITEMS 17).
- **`ShipMotion::rigid_body_substep_schedule`**: its log text still says "the box inertia is zero".
  The inertia is ON through `kHullInertiaFromShapesBound`.
- **`PilotBot::plan_controls`, `BotTaskGun::tick`, `UnitInstance::smooth_intensity`** are
  routine-level labels on transcribed or visual code (UNIMPLEMENTED_RANKING_3).
- **The AI coordinator's `fallback=`** (USN04 152) counts `00A13B60`'s own moveto fallbacks.
- **The `gunnery aim` / `targeted` / `unit fire` / `aa line` refusals** are the image's gates,
  counted.
- **`GetLastCatapulted` answers nil on JM05.** The launch is the player's: jm05.lua polls the
  player cruiser's last catapulted plane, and an idle player launches none.
- **`dead plane bot ticks_skipped`** and **`ship ai navigator skipped_steps`** are bound
  behaviour (`cc9_dead_plane_bot_think`, and `NavigatorEnable` holding JM05's Event2Pt).
- **Presentation and load plumbing:** the `MissionFrame::*`, `Hud*`, `Gui*`, `FrontEnd*`,
  `MissionLoad(*)`, `Title*`, `MissionDetail*`, `MainMenu*`, `UnitPickScreen*` and
  `InGameInterfaceUpdate::*` rows.
