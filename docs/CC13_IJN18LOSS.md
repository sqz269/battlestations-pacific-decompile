# IJN18's 525 s loss with the astern skip OFF (packet `cc13-ijn18loss`)

Read-only, no game runs. Logs, both in `J:\PROG\battlestations-pacific-decompile-cc9-lua52\local\` (reference env,
same orders `config\win_orders\ijn18_l67_i18_d.txt`):
- **Bad:** `l70r_as_i18_askip_ref.log`. Exe `local\l70_askip`, `kShipAiNavigateAsternSkipBound` OFF. The log is
  closed: `.done` 06:36Z.
- **Good:** `l69r_w_i18_main_ref.log`. Tree build of l69 main. The countdown expires at 873.16 s, phase 3 runs, and
  EndMission comes at 1691.52 s.

**Verdict.**
- The loss is `ijn_18_solomons.lua`'s phase-2 "every JapShip dead" check.
- The divergence that starts it is the **player-held Zao's main battery going silent**. kRoleOwnerDeviceWalkBound
  (cc9-gunnery53, f886af750) is in the window: it moves the guns of the gunner role, which the HUD takes for the
  controlled ship, into the player seat. The harness never fires that seat.
- Ship steering (mode 3) is on the loss path in both runs, but it is not what differs.

## 1. The loss condition

`scripts\missions\ijn\ijn_18_solomons.lua` (this installation):
- `:274-281` sets `Mission.JapShips = {Zao, Kita1, Kita2, Asa1, Asa2, Asa3, Asa4}`.
- `:575-579`: in `MissionPhase == 2`, `table.getn(luaRemoveDeadsFromTable(Mission.JapShips)) == 0` calls
  `luaMissionFailed()`. The same check for phase 1 is at `:555`.
- `:790` sets the phase-2 countdown: `Countdown(..., 1, 540, "luaPh2FadeOut")`. Both runs start it at 333.14 s.

Bad log:
- line 93108: `mission end: MissionStatus at 525.00 s status=failed text="Game Over"`.
- The last JapShip, Kita2, dies at 523.15 s (`killed_by Balt2`).
- The `luaTimetable` error at `:585` (`luaGetDistance's first param is a non-table parameter`) is a consequence:
  `GetSelectedUnit()` is nil once every ship is dead.

| JapShip | bad (`l70r_as`) sunk_at, killer | good (`l69r_w_main`) |
|---|---|---|
| Zao (player helm) | 339.34 Gear3 | **alive** (health 8750, taken 292) |
| Kita1 | 232.51 Gear2 | 218.91 Gear2 |
| Kita2 | 523.15 Balt2 | 509.55 SD4 |
| Asa1 | 428.42 Oma4 | **alive** (2700) |
| Asa2 | 139.85 Oma5 | 139.85 Oma5 |
| Asa3 | 145.00 Oma3 | 145.00 Oma3 |
| Asa4 | 211.11 Oma6 | **alive** (2700) |

## 2. The first divergence: Zao stops firing

The death rows of both logs agree through index 24. At index 25, the good run has
`death row: victim=Recon1 t=72.00 ... killer=Zao killer_gun=3 killer_cat=6 killer_blast=1 killer_range=2030`. That is
Zao's platform-1 turret, category 6, a flak burst. The bad run has no Recon1 death there.

Every ship-AI step agrees until step 2730 (Asa4), after Recon1. (Normalised: the newer log prints `desired_1d8` and
`hull`.)

Zao's guns:

| | good | bad |
|---|---|---|
| cat 6 (10 rows, the three main turrets and their peers) | assigns 220, **shots 119** | assigns 713, **shots 15**, refusals ~5839 per row |
| `player seat ... held_ticks` | 0 | **76899** |
| `player seat artillery ... turns / refusals` | 316942 / 306500 | 0 / 0 |
| `role owner device walk` | (not in this build) | `takes=27 releases=115` |
| `HudWeaponGroupScreen::role_request` (0077C470) | calls=19 | calls=15 |

The chain:
1. The orders give Zao to the player: `890 select Zao`, then `900 takehelm Zao`.
2. Screen 2Eh's weapon-group bind takes the selected group's gunner role for the controlled unit.
   kHudGunnerRoleTakeBound is in both builds: 00545410 -> 0077C470, where group 3 is mask 10h, role 4.
3. In the bad build, 00780393..007803A8 then hands the 4Bh message to 0095ABE0. Its walk at 0095AC1D, `mask & 10h`,
   runs 00954000, which sets `gun+1ACh` to the player slot on every Function 2/3/4/6 gun
   (`role_owner_device_walk_0095ac1d`).
4. The bot side gate 008FFA99 then gives those guns no target (`slot_ai_held_00927f10` false, `seat_held_ticks`). The
   harness sends no trigger: `trigger_ticks=0`.

In the good build the role is taken the same way, but the guns stay AI-held, so Zao's turrets keep firing. They kill
Recon1 at 72 s and keep Zao alive.

**Image fidelity.**
- cc9 flipped the walk ON by mechanism (`include/bsp/game_hosts_gunnery.hpp`, the USN01 pair: "the AI does not fire a
  player-held mount"). The image reads the same way: 008FFA99 runs a bot only for slot 8 or an AI slot.
- The image therefore silences the turrets for a player who holds role 4 and does not shoot. The harness is that
  player: it has no fire order for Zao's group 3.
- cc9's own note says a host-side firing attempt would also fail ("0 player shells: the HUD camera aims at the sea
  and 0085ABA0 refuses").
- So the loss is faithful for an idle player who holds the gunner role. The plan or harness is what needs to change:
  - fire Zao's group 3 through the seat; or
  - leave the role unheld, if the image's bind can be shown not to take it without input. I did not establish when
    the image's 2Eh bind runs its take.

## 3. Other switches in 4b74dd6b4..241db740b on this path

Excluded as briefed: the seg67 batch and kLineOfFireLandFromMuzzleBound.

| switch | touches IJN18? | note |
|---|---|---|
| **kRoleOwnerDeviceWalkBound** (ON, cc9-gunnery53) | **yes, the trigger** | section 2; faithful by mechanism |
| kGunAimMessageCc15Bound (OFF -> ON) | yes, records only | `gun aim message 79h ... round_trips=9839`, max_delta yaw 2.3e-5 / pitch 2.4e-5 rad: the 79h round trip changes the angles by ulps; the seat's group 3 takes 9839 messages |
| kPlayerGunSeatTargetInterceptBound (ON) | no | `seat target intercept messages=0` |
| kTaskGunBlockReconstructBound / BurstTimerReset (ON) | not reached | `task gun block resets=0` |
| kShipAiCommitmentBandQuarterBound, kShipAiNavGateClosedArmFinalBound, kShipAiRingNanArmsBound, kShipAiWidenedLiterals, kShipAiStationLatchClearBound, kShipAiParkedByteBeforeTailBound, kShipAiClearanceRefreshSentinelBound, kShipAiTorpedoMarginClassLengthBound | possible, after 136 s | the JapShips and the US ships are `state=attackmove mode=navigate_astern` from the first divergent step on. These can reshuffle the later fights, but nothing differs in ship AI before step 2730 |
| kDogfightFire*, the plane device-walk, release and step-tail switches | possible, planes | Recon, Gear and SD sections. None differs before 72 s in the death rows |
| kNavigatorForceTorpedoBound | no | JM06 script only |

## 4. Steering (mode 0/1/3) and the pending 3E0h seed fix

The loss path involves **mode-3** steering:
- From step 2730 every divergent JapShip and US ship row is `state=attackmove mode=navigate_astern`. That is attack
  move, 1C4h = 3, the block that kShipAiNavigateAsternSkipBound skips when ON. Examples:
  - Asa4: `throttle 1.000 -> 0.831, heading -0.4189 -> -1.1519` at step 2730;
  - Kita1 at step 2990;
  - Asa1 at step 3850, where the fire target is `Convoy1` in one run and `Gear4` in the other.
- Zao is on the player helm (`takehelm`) and logs no ship-AI steps after step 10.
- No mode-0 or mode-1 row is on the divergent path.

Any change to mode-3 inputs, the cruise-distance seed at 3E0h included, will reshuffle the post-136 s fights. It does
not touch the trigger above, which is a gun-seat effect at 45-72 s.

## 5. Evidence lines

- Bad log: line 93108 is the mission end; 98689 is the summary end; the seat summaries are under `summary mission
  gunnery player seat`.
- Good log: 136832 `countdown callback luaPh2FadeOut ran at 873.16 s`; 314203 `EndMission at 1691.52 s`.
- First divergent death row: index 25 of the `death row:` sequence (good `Recon1 t=72.00 ... killer=Zao`).
