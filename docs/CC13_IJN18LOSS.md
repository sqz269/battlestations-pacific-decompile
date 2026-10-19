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
  - have the harness release the role after select. That is a labelled harness input: section 6 shows the image
    takes the role without input.

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

## 6. 2Eh auto-take (packet `cc13-2ebind`)

**The image takes the gunner role on a plain select, with no player input. The host matches it, so there is no
sub-switch.**

Screen 2Eh is in the single-player INTF_SCENE3D level-1 set for the controlled unit. The bad log lists
`screens 29h 49h 44h 27h 4Dh 45h 46h 26h 2Eh ...`.

The screen's per-frame body is 005484F0:

1. **Gates (00548512-0054854E).** The screen's mover `+40h` must be non-zero, and the entries must be present
   (`[ESI+24h]`, `[ESI+20h]`, `[[ESI+20h]+14h]`). The controlled unit `[00E188D8]` must be set. Last,
   `00927F30(first entry, role 0)` must be true: the local player holds role 0, the select role.
2. **Auto-cycle (0054867A-00548692).** `MOV EDX,[ESI+44h]; CALL 00545B30` tests whether the current group is
   available. If it is not, or if the group is 0 (`CMP [ESI+44h],EBP`), 005484F0 calls 00548410 at 00548692, the forward cycle.
3. **The take.** 00548410 steps to the next available group. It then calls 00545BD0 with take=1, which goes through
   00545410 -> 0077C470 with that group's mask.

What this means for a select:
- A fresh screen starts at group 0, so the first update after the player receives role 0 takes the roles of the first
  available group.
- Group 1 is never available: it falls to 009542B0's switch default. For Zao the first available group is 3
  (mask 10h, role 4).
- Input only changes the group afterwards. Keys 9Ch-9Fh go through 005484B0, and the 9Bh axis through 00548410.

`src/hud_weapon_group_screen.cpp` `weapon_group_screen_update_005484f0` has the same gates and the same cycle step.
`kHudGunnerRoleTakeBound` routes 0077C470 to `role_request_0077c470`.

In both IJN18 runs the take follows `select Zao` (role 0 through 00645600 -> 20h). `takehelm` sends only
`0077C470(unit, 2, 1)`, which is mask 2. The `HudWeaponGroupScreen::role_request` call counts are 15 (bad) and 19
(good).

Only kRoleOwnerDeviceWalkBound (0095AC1D -> 00954000) turns that role into player-held guns, and the image does the
same. The IJN18 loss is therefore a plan/harness gap.

**Reach.** In cc9-lua52's `config\win_orders`, 43 of 55 order files `select` or `takehelm` a ship, and none issues
a fire order. In each one, the selected ship's first available weapon group is player-held and silent while the walk
is ON. cc9's USN01 note shows the same effect: Northampton's turret shots drop from 113 to 95.

| mission | files that select a ship and never fire |
|---|---|
| bsm01 | 3 |
| ijn08 | 4 |
| ijn18 | 2 |
| jm06 | 7 |
| jm08 | 6 |
| usn01 | 11 |
| usn02 | 2 |
| usn12 | 4 |
| usn15 | 3 |
| usnos | 1 |
| **total** | **43 of 55** |

There are two options for cc9:
- add group fire orders to these plans. Player-seat shells must aim correctly first, because cc9 notes "0085ABA0
  refuses";
- have the harness release the gunner role after the select. That is a labelled harness input, not image behaviour.

## 7. Releasing the gun seat: the map toggle (packet `cc13-releasegun`)

**The player gives up the gun seat by opening the map, and the role stays released while the map is up.** The plan
order `<frame> releasegun <unit>` drives that input. It sits behind `kHarnessReleaseGunMapBound`, which is OFF.

### The image path

1. **Input.** 0068C1F0 tests C1h at 0068C64C. C1h is `IC_MAP_TOGGLE = 193` in this installation's `Inputs.lua`.
   - The test sits inside the arm for interfaces other than 29h and 2Bh-2Dh.
   - Its gates: no left Alt (`GetKeyState(A4h)`), the screen 3Bh bytes +81h/+82h clear, and the screen 45h bytes
     +156h/+157h clear.
   - 0068B470 is then reached at 0068C6B1.
2. **0068B470.**
   - The level-3 collapse (0068B4A8..0068B4C6) runs only with 00E18D1C non-empty.
   - 0051E8E0 runs at 0068B4CE.
   - When map slot 4Ch is not up (`[this+54h]+4h` clear), the routine continues into 0068AC30.
3. **0068AC30.**
   - 00E08880 is EnableMap, which is 1 in single player.
   - 0068AB30 runs at 0068AC50. It pushes 20h only on interface 31h or 32h.
   - The selection is copied at 0068AC63 and 0068AC73.
   - 004F85D0 at 0068AC79 publishes level-2 screens `{4Ch 39h 44h 29h 49h}`.
   - 004D8AE0 at 0068AC8E publishes level-2 contexts `{0Eh 14h 0Ch 11h}`. Its tail at 004D8B46 is `PUSH 2; CALL 004D6410`.
4. **Occlusion.** 4Ch's vtable +8h is 005A8EF0, `MOV AL,1; RET`. 004F7620 walks levels 5 down to 1 and raises the
   floor at the first opaque screen, so no level-1 screen is requested.
   - Of the 25h set, 27h, 4Dh, 45h, 46h, 26h, 2Eh, 35h and 50h exit.
   - 44h, 29h and 49h stay up, because they are in level 2.
5. **46h's exit, 0064A490** (vtable 00CF7978 +1Ch).
   - It calls 005470C0, which runs `0077C470(unit, group mask, 0)` for each entry.
   - When +1Ch (the unit 0064DA40 stored) is set, it also runs `0077C470(+1Ch, 1FEh, 0)`.
   - 0077C470 builds session message 4Bh: +24h the mask, +28h the slot game+18ECh, +2Ch take 0.
   - The 4Bh arm sets each held role back to 8, and the device walk 0095AC1D hands the guns back to the bot.
6. **No re-take while the map is up.**
   - 005484F0 is reached from 46h's update 0064D610, and 004F8906 updates only screens that are requested and applied.
   - 27h's 0067BB50, the captain-role take, is also out of the set.
7. **Closing the map** (C1h again) runs 0068B470's other arm:
   - 004F85D0(0) at 0068B4E9, 004D8AE0 at 0068B4F7, and 0068AA40 at 0068B507;
   - 46h and 2Eh enter again, and 00549260 re-takes the group.

Mask 1FEh is roles 1..8, so the helm (mask 2) is released as well. While the map is up, the ship is driven by the AI.
A plan cannot keep `takehelm` and drop only the guns this way.

The other candidates do not do this:
- The group keys 9Ch-9Fh and the 9Bh cycle cannot choose "no group". 00548410 steps only between available groups.
- The free camera (5Ah, 29h with set `{2Bh}`) exits 46h too, but it is a camera mode.
- The binoculars 0067C0A0 and the periscope 006506E0 belong to the torpedo boat and the submarine.

### The harness order

`<frame> releasegun <unit>` is in `game_hosts_mission_frame.cpp`.
- It refuses any unit but the controlled one, `[00E188D8]`.
- It arms the HUD, then holds C1h for one frame through the `input` line's action record.

Only an armed HUD runs the following host wiring. A plan without the line runs as OFF.

| Image | Host |
|---|---|
| left-Alt test | `HudUpdateBinding::modifier_key_allows_back_out` answers "not held" (LABELLED: no keyboard) |
| 0068B470 open arm and 0068AC30 | `GameHudHost::Impl::back_out_one_overlay_level_0068b470` |
| 004F85D0, 004D8AE0 | `GameMenuHost::publish_level2_screen_set_004f85d0`, `publish_level2_input_contexts_004d8ae0` |
| 005A8EF0 | `MenuSetBindings::screen_occludes_lower_levels` answers true for 4Ch |
| 0064A490 | `GameHudHost::exit_ship_view_screen_0064a490`, from the pump's exit for slot 46h |

Records only:
- the close arm, 0068B4E9..0068B507;
- 0051E8E0, 0068AB30, and the selection copy;
- the screen 4Ch and 39h enter, update and exit virtuals.

In the image, a later `select` re-applies 25h. Its 0064DA40 re-binds 2Eh, which re-takes the group, but 46h stays
occluded while 4Ch is up. The host does not change that path.

Status: reconstructed and build-tested, OFF. The switch was also built ON. No game run.
