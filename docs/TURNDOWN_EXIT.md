# The turndown exit and its speed hold are the image's own

Addresses: 009C7EA0, 009C44F0, 009C450D, 009C4512, 009C4518, 009C4524, 007C47F0, 00D1F98C, 00D7A260,
00D20E80, 00929800, 008BD900.

Packet `cc9_turndown_exit`. Every name is a hypothesis, not a recovered symbol. Background:
`docs/AIMDIVE_ENTRY.md` section 6 named this term.

## 1. The exit, 009C7EA0 BSP_BotStateDiveBombTurnDown_IsComplete

`__fastcall(state)`. ECX+4 is the approach, and its +4 is the unit.
* **The bank fold.** The bank is unit+C68h, folded to |bank| (009C7EAE COMISS against 0 [00D7A218],
  and -0.0 [00D7A208] minus it otherwise).
* **The rule.** The turndown is complete when **pitch < -1.3** ([00D1F98C], float, 009C7ED8 COMISS
  with `JA` taken on -1.3 > pitch). It is also complete when **pitch < -1.0 and |bank| > 135°**
  (-1.0 [00D7A260] at 009C7EE5; 2.3562 [00D20E80] at 009C7EEA, `JA`).
* **What the second arm means.** The image lets the aircraft leave the turndown upside down once it
  is 57° nose-down. Nothing in 009C7EA0 or the turndown tick rolls it upright first.
* **What the aimdive does with it.** It handles the inverted entry through 009C4F80. At pitch -40°
  or steeper that returns the bearing of the body's up axis, which is heading + π while inverted,
  and the aimdive's own roll law then rolls the aircraft (`docs/AIMDIVE_ENTRY.md` section 2).
* **The host.** `dive_bomb_turndown_complete_009c7ea0` is the same rule with the same constants.

**Measured exits.** The traces are `local\M0T_9000.log` (main configuration, frozen throttle) and
`local\V1_9000.log` (throttle fix on). The row is the first aimdive tick, after the transition.

| run | aircraft | t | pitch | bank | speed |
| --- | --- | --- | --- | --- | --- |
| M0T | D3A Val #1.1 | 1260 | -1.078 | 3.120 | 114.6 |
| M0T | D3A Val #3.1 | 1086 | -1.045 | 3.122 | 112.2 |
| M0T | D3A Val #1.1\|.-3 | 1267 | -1.051 | -3.118 | 114.5 |
| V1 | D3A Val #1.1 | 1339 | -1.076 | 3.120 | 53.6 |
| V1 | D3A Val #3.1 | 1326 | -1.056 | -3.104 | 47.6 |
| V1 | D3A Val #1.1\|.-3 | 1358 | -1.051 | -3.118 | 52.1 |

Every exit takes the inverted arm, at pitch -1.05 to -1.08 with |bank| of about 3.11-3.12. That
holds in both regimes. The only difference is the speed.

## 2. The speed hold, 009C44F0 BSP_BotStateDiveBombTurnDown_Tick

* 009C450D calls 007C47F0 on approach+8h, the class. **007C47F0 is `tuning+24Ch × class+184h`**:
  007C47F9 FLD, 007C47FF FMUL.
* 009C4512 stores that to cmd+2B4h. 009C4518 sets cmd+2B0h = 0 (byte). 009C4524 sets cmd+2D8h =
  EBX = 1 (dword), which is speed mode.
* **The authored values.** tuning+24Ch is `Dynamics/SpdMultipliers/LevelFlight` = 1.8
  (`planeglobals.lua` line 280, "above stall speed % the lift is maximal"). class+184h is StallSpd
  = 19.166666 for both D3A Val rows (VehicleClass[158] and the kamikaze [46]) in this installation's
  `vehicleclasses.lua`. The product is
  **34.5 m/s**.
* The turndown writes no throttle or air-brake slot, and nothing else in 009C44F0-009C46C6 stores
  to +278h/+2A8h. The throttle is therefore the speed hold's (0099D300 speed mode):
  * the increment is interp(-6.94, -2, 6.94, 2, error) × 0.6 when positive, × dt;
  * that is clamped to ±1, and a negative demand becomes the air brake.
* **The host.** `bot_desired_speed_007c47f0` is the same product from the same two sources, with
  speed mode 1. With the throttle fix on (V1), the Val enters the turndown at about 63 m/s against
  a 34.5 m/s command. Its throttle is already cut to 0.001 by the fly-over's hold and stays there through the turndown
  (D3A Val #1.1's `val_trace`, `docs/PILOT_THROTTLE_SLOT.md` run V1), and it leaves the turndown at
  48-54 m/s. That is the image's own law acting on the image's own number.

## 3. Decision: no divergence

* The inverted exit is 009C7EA0's second arm, reached in both regimes at the same attitude. The
  image does not roll the aircraft upright before the aimdive.
* The 34.5 m/s hold is LevelFlight × StallSpd, authored, with the throttle driven by the bound
  speed hold.
* The frozen-throttle host left the turndown at 112-115 m/s only because its speed hold never
  moved the throttle (`docs/PILOT_THROTTLE_SLOT.md`).
* **Nothing is bound, no switch lands, and no pair was run.** No code changed.
* The regime the throttle fix produces (a slow, inverted entry, then the entry swing, the
  shallow dive and Modes A and B) is so far the image's own behaviour at every term read:
  * the turndown exit (this doc);
  * the aim error (`docs/AIMDIVE_ENTRY.md`);
  * the roll law (`docs/DIVE_MODES.md` 2);
  * the fly-over (`docs/DIVE_MODES.md` 1).
* The remaining unread inputs to that regime are the flight model's response at 50 m/s: drag, lift
  and the control-rate gains at low speed. Also unread is whether USN04's Vals really dive at that
  speed in the image. That needs a game-validated reference, which this project does not have.

## 4. The control run's mission failure at 228.06 s

* **What fails.** Primary objective 1 of USN04 (`objective primary:1 Active=true Success=false` in
  the summary of `local\DC_9000.log`). The end row is `EndMission=true at 228.06 s status=failed
  text="Game Over" entity="Lexington-class01"`.
* **The cause is the carrier's death.**
  * At 225.81 s torpedo 13 (bullet 69, a B5N Kate torpedo; `exit=entity_impact
    hit=Lexington-class01`) delivers a 1200-base blast that takes the last 203.7 health.
  * Earlier impact blasts (bullet 15, 15.0 each; not the Vals' bomb, which is bullet 77) had brought
    the ship down to that.
  * `entity dead: unit=0 "Lexington-class01" died=225.81 s ... KillReason=harm (00929800)`.
* **Where the decision is made.** The mission script, running in the Lua VM. Within 2.25 s of the
  death publication it marks objective 1 failed and calls five natives, all logged UNIMPLEMENTED
  with neutral returns: `Objectives_Failed` (008BD900, argc 5), `CountdownCancel`,
  `MissionNarrativeClear`, `Scoring_SetMissionCompleted` and `BannSupportmanager`. It then sets
  `Mission.EndMission`, which the host stamps (`docs/MISSION_END.md` section 3, `kMissionEndBound`).
* **Is that path bound?** The decision is the script's own logic reading the bound death
  publication (00929800), and the damage that caused it comes from the gunnery host's bound impact
  and blast law. The five natives are UNIMPLEMENTED, but they are presentation and scoring calls
  made after the decision. None of them decides it.
* Which script condition turns the death into the objective failure could not be read. The USN04
  script is packed, and no loose `.lua` in `scripts\` names it.
