# CC13 VER 7: byte verification of the running ship-AI navigator

Packet `cc13-ver7` (cc13 lineage, Claude), 2026-10-13. This is a verification pass over cc9's running
ship-AI navigator: how a ship's order becomes a helm and throttle command, the obstacle sectors, the
clearance, the navigation arm, the formation ceiling, and the out-of-map countdown. I made no game
runs. Existing logs in the cc9 worktrees were used as evidence.

Status words follow AGENTS.md. Everything below is **exported** and **reconstructed**. The fixes are
**build-tested** (`scripts/build.ps1`, ctest 2/2) and committed OFF. Nothing here is game-validated.

## 1. Scope and method

The range the packet named, `0081BEB0`-`00826D70`, is mostly segment 51's unit-instance code. Its
two ends are the ship's exit-zone pair (`0081BEB0` BSP_UnitInstance_ExitMissionZone and `00826D70`
BSP_ShipUnit_UpdateOutOfMapCountdown). The navigator proper is `009D5000`-`009F6000`, hosted by
`src/game_hosts_ship_ai.cpp` and `src/ship_ai_*.cpp`. I checked both.

Method (the same as `docs/CC13_VER_3.md`):
- I compared the code instruction by instruction against the Ghidra listing export, x87 included.
- `tools/x87_dc_form_scan.py --range 009D5000-009F6000 --forms dc,de` lists the DC-form sites. Every
  reconstruction that contains one was already audited (`docs/CC13_X87_DC_FORM_AUDIT.md`). The only
  DC site in `009F3F80` is `009F41D3` `DC C1` (`FADD ST1,ST0`, current + 0.05). `009F4200` is `D8`
  (2*dt), and so is `009EE8BF`.
- Constants were read from the PE on disk (`local/ver7_rd.py`).
- Field writers were found by a capstone sweep for stores to `[reg+disp]` (`local/ver7_stores.py`).
  Field readers were found by a scan of the listing exports for `+ 0xNNN]`.
- Settings values come from this installation's `scripts/datatables/shipglobals.lua`.
- `reports/cc13_ver7_calls.json`: `tools/verify_report_calls.py` checked 22 call rows against live
  Ghidra, and 0 failed.

Precision. Gameplay runs at 24 bits. A host double chain that the image rounds at each x87 operation
is counted as "last ulp", not as a divergence.

## 2. Results

| Address | Host | Verdict |
|---|---|---|
| `009F3F80` middle `009F40CA`-`009F4B98` | `ship_ai_drive_order_ring_middle_009f40ca` (src/ship_ai_obstacle_tables.cpp:371) and its flags, danger, bucket and index helpers | **match** (its blocker input: D2) |
| `009F3F80` head `009F3F89`-`009F3FE3` | `ship_ai_backoff_countdown_009f3f89`, behind cc9's `kShipAiObstacleBackoffCountdownBound` (OFF) | known, cc9's switch |
| `009F3F80` hop `009F4B99`-`009F4D04` | `ship_ai_order_ring_hop_009f4b99` (src/ship_ai_throttle_ring.cpp:205) | **match** |
| `009EC7C0` | `ship_ai_throttle_ceiling_009ec7c0` | **match** |
| `009DA250` | `ship_ai_rudder_from_heading_error_009da250` (src/ship_ai_throttle_ring.cpp:89) | **match**, last ulp (1.2 against 00CEC160) |
| `009D8B90` | `ship_ai_neighbour_closing_speed_009d8b90` | **match** |
| `009EB660` unit arm `009EBEB3`-`009EBFDF` | `ship_ai_scan_obstacle_sector_009eb660` and the binding at game_hosts_ship_ai.cpp `ObstacleSectorRefresh` | **D2** |
| `009ED6B0` per-tick span `009ED6E3`-`009ED7D2` | `navigate_009ed6b0` binding and `ship_ai_direct_control_arm_009ed6b0` | **D3** |
| `009EE671`-`009EEAA8` (output block, turn lead) | `ship_ai_navigation_arm_009ee671`, `ship_ai_turn_lead_009ee848` (src/ship_ai_navigation.cpp) | **match**, last ulp in the lead chain |
| `009EEAB3` gate byte `[[blk+3FCh]+73Ch]+21h` | hard-coded true (game_hosts_ship_ai.cpp, `tuning.keep_clear_of_traffic_21`) | **D4**, no reach found |
| `009EF910` timer `009EF91D`-`009EF956` | `ship_ai_refresh_turn_clearance_009ef910` | **D5** |
| `009F3F80` stall `blk+384h` | `ShipAiObstacleState::stall_time_384` against `ShipAiControlBlock::direction_counter_384` | **D6** |
| `009F4DA0` | `formation_throttle_ceiling_009f4da0` (game_hosts_ship_ai.cpp) | **match** |
| `009F4D10` | `ship_ai_publish_order_009f4d10` (src/unit_autopilot_pair.cpp) | **match**; the host writes the slot after the three tails, and none of them reads it |
| `00826D70` | not hosted | **finding 1**; model `ship_out_of_map_countdown_00826d70` (src/ship_nav_ver_cc13.cpp) |

## 3. Divergences

### Finding 1: ships never die by "exitzone"

- **Image.** `00826D70` is slot `+1D8h` of every ship class vtable. Its nine data references are
  `00CF9288` ... `00D0C820`, and `-1D8h` from each lands on the vtable stored by
  `BSP_Battleship_Construct`, `BSP_Cargo_Construct`, `BSP_Cruiser_Construct`,
  `BSP_UnitInstance_Construct`, `BSP_LandingShip_Construct` and others. `00953D98` calls this
  slot for every unit, as it does for planes (`docs/PLANE_FLIGHT.md`).
  - Outside the map (`0071C4F0`) and held by a command (`00811F50(1)`: retreat, cruise, a qualifying
    attackmove, moveto or moveonpath, or the local player's unit), the timer `+6F0h` counts down.
  - At `0059C9B0` with margin `Retreat.ExitDist`, or once the timer drops below zero, the state
    becomes 3 and `00811E80` routes `MT_UNITEXITED`. The ship's handler `0081BEB0` kills the mission
    entity with reason 4 (`exitzone`) and sets Lua `ExitZoneParty`.
- **Host.** Nothing calls an equivalent for ships. `note_kill_cause(...,4)` is plane-only
  (src/game_hosts_units.cpp:8213, 8234).
- **Effect.** It applies only when a ship under a retreat or cruise order, or the player's ship,
  actually crosses the map edge. The scripts in this installation that branch on a ship's
  `KillReason == "exitzone"` or on ship exitzone listeners are:
  - LOMP06: an exited CrucialCargo fails the mission (06_crucial_cargo.lua:437-452).
  - JM02: the PoW and Repulse escape branches (995-1293).
  - USN05: `ListenerUSShipExited` and `ListenerFlagShipExited`.
  - USN06: the submarine and convoy exits.
  - USN10: the Iowa and Enterprise exits.
  - escort10, chg_7 and chg_9.

  In the host such a ship sails on, and the fail or victory branch never fires.
- **Status.** Model only. The kill-cause plumbing is in `game_hosts_units.cpp`, which `cc9-lua52`
  leases (adoption note, section 4).

### D2: the obstacle sector's blocker is never stored

- **Image.** `009EB660` stores the blocking node `blk+608h[i]` into `sector+24h` (`blk+82Ch`) at
  `009EBEDB`. `009F3F80` reads it at `009F4662`. Its back-off arm `009F4738`-`009F47EE` then arms the
  astern latch `blk+380h = 1.0` when:
  - the node's pass side `+88h` differs from the sector's `+28h`. The node's side is 0 unless
    `009D912F` predicted a crossing, so this is the common case; or
  - the blocker is player-controlled (`unit+184h`) and closes faster than `0.75 * blk+3C4h`.

  It also adds `2 - |neighbour speed|` to the stall `blk+384h` (`009F47AF`-`009F47EE`).
- **Host.** `ship_ai_scan_obstacle_sector_009eb660` (src/ship_ai_sector_scan.cpp:533-556) omits the
  store. The binding nulls `sector.blocker` after every scan. The neighbour-speed hook returned
  `0.0f`, with the comment "Unreachable: 009EB660's scan finds none".
- **Evidence that it matters.** Neighbour-blocked sectors are common:
  - `sector_blocks > 0` on 43/136 rows of `g39_bb0_os.log`, 41/136 of `s46_b_a5.log`, 39/88 of
    `s46_f_i18aj.log` and 34/48 of `l56_off_j8.log`;
  - SHIP_AI_OPEN_ITEMS 65.5 and 104.1 found no log in which `009F47A7` armed. This is why.
- **Effect ON.** An AI ship whose scanned sector ahead is blocked by another ship backs off. Each
  arming lasts about 1 s in the astern-only throttle window `[-1, 0]` (`009F4805`), and it re-arms
  while the ship stays blocked.
  - Crowded rows move: USNOS, IJN18, JM08, USN15, the USN13 Marus and JM05.
  - Positions move first, then the gunnery and bombing exchanges, and there are fewer rammings.
  - No RNG draw is added; the coupling is through positions only.
- **Fix.** `kShipAiSectorBlockerBound` (OFF), commit `b639d276c`. **Flip it together with
  `kShipAiObstacleBackoffCountdownBound`**: without the `009F3F89` countdown an armed latch never
  expires.

### D3: the turn lead `blk+328h` is not reset each tick

- **Image.** `009ED708` zeroes `blk+328h` in 009ED6B0's unconditional span. `009EE9D8` writes it only
  when the lead applies. The arm tail reads it at `009EEAFE`, where the allowance is `32Ch * |328h|`.
  These are the only three accesses in `009D`-`009F`.
- **Host.** `ShipAiNavState::turn_lead_328` is never reset, so it keeps a stale lead.
- **Effect.** It applies only to units whose setback walk is open. For example, USN04
  Fletcher-class04 has 1269 scans, 1269 steps and a 186.6 m maximum setback in
  `g38_dfgbon_usn04.log`. The setback distance can move. No RNG.
- **Fix.** `kShipAiTurnLeadResetBound` (OFF), `1020e13e1`.

### D4: the navigatorParams byte `+21h` is hard-coded true

- **Image.** `009EEAB3` gates the setback walk on `[[blk+3FCh]+73Ch]+21h`.
  `NavigatorForceMoveCloseToTarget` (`008A3460`) writes the inverse of its argument there.
- **Host.** The byte is always true (game_hosts_ship_ai.cpp, `tuning.keep_clear_of_traffic_21`), and
  the Lua function is not bound.
- **Reach.** Only USN04 calls it, with true on its seven convoy transports. No convoy unit runs the
  setback walk in `g38_dfgbon_usn04.log`, so there is no reach today.
- **Status.** Adoption note only.

### D5: `blk+374h` is one field

- **Image.** `009EF910` counts `blk+374h` down and reloads it with `NearbyShip_WayClearCheckTime`
  (0.25, `009EF956`). Every other access is a `-1.0f` store on a direction change, which forces the
  next refresh:
  - `009D526F`, the latch helper;
  - `009ED867` and `009ED8B5`, the direct latch;
  - `009EDB8B`, station keeping;
  - `009F4B16` and `009F4B7B`, the escape flip.
- **Host.** It keeps two fields: `ShipAiClearanceBlock::refresh_timer_374`, and
  `ShipAiControlBlock::direction_value_374`, which nothing reads.
- **Effect.** After a flip, `blk+37Ch` (the danger input) and `blk+370h` (the escape input) lag by
  up to 0.25 s (5 ticks). Rows with flips move slightly: USN13, JM05 and JM08. No RNG.
- **Fix.** `kShipAiClearanceTimerSharedBound` (OFF), `7d07885ef`.

### D6: `blk+384h` is one field

- **Image.** The direction-change stores at `009D5277`, `009ED86F`, `009ED8AD`, `009EDB73`,
  `009EEFF2`, `009EF095` and `009EF1E6` all write 0.0 into the stall accumulator that `009F3F80`
  reads.
- **Host.** It sends them to `direction_counter_384`, which nothing reads.
- **Effect.** None while D2 is OFF. With D2 and the countdown ON, a flip restarts the stall count
  toward the 10.0 escape threshold.
- **Fix.** `kShipAiStallSharedBound` (OFF), `331779f9b`.

## 4. Adoption notes (cc9 files under lease or out of scope)

- **Finding 1** (`game_hosts_units.cpp`, leased by cc9-lua52).
  - Call `bsp::ship_out_of_map_countdown_00826d70` once per ship tick where `00953D98` runs: unit
    tick, `unit+520h` clear.
  - Host fields per unit: `+6ECh`, `+6F0h`, `+100Ch`.
  - Settings: `GameplayTuningSettings::retreat_exit_dist`, `retreat_exit_time` and
    `retreat_warning_repeat_time`.
  - Hooks:
    - `point_outside_world_map_0071c4f0` and `point_beyond_map_margin_0059c9b0`, both on the unit
      position;
    - the 00811F50(1) predicate, for which the host has the command class and player role;
    - the formation follower and the local player role;
    - a warning record;
    - on exit, `note_kill_cause(unit, 4)` plus the kill.

  The `route_unit_exited_00811e80` hook should do what 0081BEB0 does: the exitzone channel, kill
  reason 4, and Lua `ExitZoneParty = party`.
- **D4.** Bind `NavigatorForceMoveCloseToTarget(unit, flag)` to a per-unit byte, `+21h = !flag`, and
  pass it to `tuning.keep_clear_of_traffic_21`.

## 5. Records

`python tools/bsp.py ledger add-function` (`local/ver7_ledger.py`):
- the model `ship_out_of_map_countdown_00826d70` (`00826D70`);
- check records `cc13_ver7_check_*` at `009F3F80`, `009EB660`, `009EC7C0`, `009DA250`, `009D8B90`,
  `009ED6B0`, `009EE671`, `009EF910` and `009F4DA0`.

## 6. Still open

- `009EB660` outside the unit arm: the probe geometry and the zone arm were not re-read.
- `009EF910` past `009EF9F1`: the sweep, the static zone and the neighbour loops. The host comments
  cite each instruction, and I spot-checked the gates.
- `009D6B40`, the profile snap, was not checked.
- `004224C0` and `009D7050`, the avoid list refill and the cached query.
- `004158E0` (zone crossing) and the path planner and follower.
- `009EE580`-`009EE670`, the path pick, and `009EE6D3`-`009EE756`, the `2FDh` latch. The host's
  arm-tail packet owns the latter.
- The `0080E000` order-slot step.
- The two pre-existing "unreachable" notes that are now stale and have no effect: `game_hosts_ship_ai.cpp` near
  `ThrottleProfileBinding` (tracks are appended when `kShipTorpedoResponseBound` is on) and the
  `+604h` comment near the arm tail call (the neighbour view now sets it).
