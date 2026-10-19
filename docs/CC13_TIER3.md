# cc13-tier3: re-verifying the remaining Tier 3 mechanisms

This is packet cc13-tier3, item 4 of cc9's plan in `docs/CC13_ADOPTION_TRIAGE.md`, Tier 3. It was
done on 2026-10-19 by cc13-steer, from origin/main `4890e900a`.

**Excluded, because another worker holds them:**
* row B (seg67b and `009FC7C0`);
* row C (the land chain);
* `0077BD70` (the JM08 leader handover);
* `007C0D90`;
* the follow seams;
* seg67 ship AI;
* paratroopers;
* Tiers 1 and 2. That takes in row A's Tier 1 rows `006C0DA0`, `006CDC70`, `006C7490` and `006C5950`.

**Method.** For each row:
1. Read the current host code at the cited seam, together with every switch on it, and the export or listing.
2. Measure liveness from the 30 newest `l69r_*_main_ref` logs (cc9-lua52, 2026-10-19). These cover BSM01,
   BSM09, ESMP08, IJN08, IJN18, JM06, JM08, LOMP06, LOMP10, PRCP01, USN01, USN02, USN12 and USN15.
3. Read the 621 `.lua` files of this installation to see which listener channels and filters the shipped
   missions use. `AddListener` counts by channel:
   * `kill` 335, `hit` 235, `recon` 219, `surrender` 76, `input` 50;
   * `entityKilled` 44, `exitzone` 36, `gui` 19, `command` 17, `generate` 17;
   * `hpEvent` 12, `ammoType` 9, `repair` 6, `shipLanded` 5, `target` 2;
   * `failure`, `leak` and `fire` are never used.

**Result.** No row needed a new switch. Every remaining row falls into one of these classes:
* already fixed by a switch that is ON (stale);
* presentation only;
* unreached on the measured rows and by the shipped scripts;
* owned by another lane.

Each verdict is in the row's ledger record, with the evidence appended.

| Row | Address | Verdict | Evidence |
|---|---|---|---|
| D | `008193A0` place at world position | STALE | Follower arms are bound under `kPutToFormationBound` (ON). The refusal record has 0 calls. `set_world_position` has 159 calls in 7 missions. |
| D | `008A9F90` PutTo | STALE | The heading is applied through `set_heading_008196b0` (vtable[11Ch]) under `kPutToFormationBound`. 151 calls. |
| D | `008A0A10` EntityTurnToEntity | STALE | The ship and plane arms are bound (`kScriptedOrderNativesBound`, `kTurnToEntityMemberPlaneArmBound`). The other-kind arm (`008a0eb8`) and the unresolved arm (`008a0d1c`) have 0 calls. |
| E | `00988510` hit listeners | Mostly STALE | Callback arguments, filter fields and the rate limit are ON. `attackerPlayerIndex` and `targetDevice` are still open. Their only shipped user is JM06 `hshit`, whose hospital ship is never hit. |
| E | `009813A0` kill listeners | Labelled cadence | Unchanged. It is cc9-lua's lane, and the six-win pairs were identical when `kKillListenerHarmOnlyBound` was flipped. |
| E | `00982120` exitzone | One more producer, unreached | See section 1. |
| E | `00898E30` FindEntity | FAITHFUL | `0088B1B0` uses `__stricmp`. |
| E | `0088BF80` GetProperty | Open, liveness not shown | See section 2. |
| F | `00892DE0` GetCatapultStock | STALE | `kShipCatapultLaunchBound` (ON) reads the launch-maintained `+638h`. |
| F | `00892860` GetLastCatapulted | Unreached | 0 calls. |
| F | `00893C00` SetUnlimitedAirSupply | No gap | Every shipped call names a submarine. The 8 live calls all stored. |
| F | `008963E0`, `00896A90`, `008975B0` | Presentation, or unreached skips | The preload mark, the GUI and peer messages, and the no-deck or negative-n skips. |
| G | `008198A0` set failure from message | STALE for gameplay | See section 3. |
| G | `0081DE10` shipyard hangar | Ships lane | The `+1130h` sentinel arm is open. It matters only for shipyard-built ships (`kShipyardProductionBound`, GUNNERY_OPEN_ITEMS 136/146, SHIP_AI 197). |
| G | `008227E0` damage smoke | Presentation | Its draws are ON. |
| - | `00968550` warning deadlines | FAITHFUL | The listing matches `update_warning_deadlines`. The sound-object call is presentation. |
| - | `008ACA30` / `008ACB70` checkpoints | OFF by design | `kMissionCheckpointBound` is false. |
| A | `006C5B70`, `006CD240`, `006C5050` | Not touched | These are the slot-notice and queue rows that share Tier 1 #3 to #5's mechanism (`006BF150` notices, frame order). cc13-audit-follow holds that work. |

## 1. `007C6C30`'s `exitzone` warning

`007C6C30` is BSP_Plane_OutOfActionCountdown, vtable +1D8h. When the plane is the squadron's first
plane (`+9D8h == 0`), it calls `00982120(plane, 0)` at `007C6D65` in two cases:
* on its first tick outside the map, when `+6ECh` was 1;
* for the player's squadron, each time `+36Ch` runs out (reseeded from tuning `+350h`).

The host only records this (`game_hosts_units.cpp`, `Plane::out_of_action_notice_00982120`, 8,990
calls in BSM01 and ESMP08). It never queues the event.

The event is `(squadron, exited = false)`. No shipped listener can match it:
* Every `exitzone` listener that names planes asks for `exited = {true}`. These are escort10, USN01
  `AlliedPlanes`, and USN06/PRCP06 `PlanesHidden`.
* The `{false}` listeners name ships: BSM09 `JapCVs`, CHG4 `AlliedCVs` and CHG6 `AlliedFleet`.

So it is recorded with no switch. If a mod adds a plane listener with `{false}`, there is a simple fix.
Queue `note_exit_zone_00982120(squadron, false)` at that seam. Copy the `+9D8h`, `+6ECh` and `+36Ch`
rules from `out_of_action_countdown_007c6c30` (`src/gameplay_seg47_cc13.cpp:233`).

## 2. `0088BF80` and deckless units

Some units own no deck. For these, the host answers `slots`, `numSlots`, `stock` and `planes` with
an empty table, where the image's class reader pushes nothing (nil). The faithful fix is to return 0
values when the unit is not a deck-owning class (mothership `09h` or airfield). Two things stopped it:
* Liveness is not shown. The scripts ask these keys of carrier and airfield variables, and the host
  does not count deckless askers.
* `src/game_hosts_lua.cpp` was under cc15-vcp6's lease.

A counter on `deck == nullptr && !deck_class` would settle whether it is live.

## 3. `008198A0`

The arms are chosen by the message type (`vtable[0Ch]`):
* **Explosion:** `00818110(unit+310h, 008782A0(msg+24h, msg+28h))`. Its step, `0081B630`, runs the
  owner's vtable[1ACh] and then `00982C50` failure("Explosion").
* **SteeringJam:** `+9E4h = 1`.
* **EngineJam:** `+9E5h = 1` and `00867B10` on each `+A14h` effect.

The host already applies both jams and the explosion damage. The census "UNIMPLEMENTED" lines are the
`record()` calls kept beside the jam stores. What remains is presentation:
* the effect stops;
* the Explosion point effect;
* the `failure` channel, which no shipped script listens to.
