# Ship torpedo evasion: what the image does, and why the Lexington does not turn away

Packet `cc9_torpedo_evasion` (2026-09-23). This is a read-only packet plus one small switch.
Offsets are relative to blk = brain+8h unless written as brain+N, and names are hypotheses.

## Answer

The image has a torpedo response for AI-driven ships. It is **off for the player-controlled
ship**, and the Lexington is the controlled unit in the E2 and USN04 runs. So the image would
not turn the Lexington away either, and its loss at 225.81 s in the E2 9000 control is not a
missing host term. The Lexington sits still because it is the controlled unit and the harness
holds its helm at throttle 0 and rudder 0 (`order=throttle 0.000 rudder 0.000`). It moved
100.51 m in 449.96 s against the 6749.45 m its 15.0 m/s StartSpeed would give. Every aim row
reads `target_speed=0.00`, and every torpedo census row reads `target_moved=0.0 m`.

## The image's torpedo response

1. **Detection, brain pre-pass 009F158A** (docs/SHIP_AI_BRAIN_PREPASS_SCHEDULE.md). When the
   torpedo timer (settings+1F0h `TorpedoAvoidance.CollectTimer[2]`, 2.0 s) is due, the pre-pass
   walks the world torpedo list (world+21Ch count, +220h head; `MTorpedo` registers there
   through 00856360). It admits a torpedo when either condition holds:
   - it is inside 0.6 x hull length;
   - it can reach the hull within the horizon TorpedoPredict + TorpedoObservation +
     (period + 3.0) at the relative planar speed, and it is closing.

   Admission calls 009F0AD0 (body 009F0AD0-009F0D1C), which refreshes an existing contact
   track's lifetime or allocates a 68h track. That path makes stream-1 random draws and
   constructs the track with 009EACA0 (body 009EACA0-009EADDF). The track is appended to
   blk+404h, counted by blk+400h.
2. **Gate, 009DA1D0** (body 009DA1D0-009DA244, whole listing read). It returns false for any of:
   - a torpedo boat (`[blk+3FCh]->vtable[5Ch](0Eh)`);
   - a unit deeper than -15.0 (the double at 00CE3D58, against unit+100h);
   - blk+3ECh = 0;
   - the weapon director's `torpedoAvoidance` byte +240h = 0.

   The director constructor sets +240h to 1 (00836724), and Lua `NavigatorSetTorpedoEvasion`
   (008A3CD0, via 00835940) changes it.
3. **Throttle, 009E04E0** (chain slot 11, 009F51F3). It walks the tracks behind that gate
   (009E061B), refreshes them with 009DC060 (body 009DC060-009DC2DD), and builds the 65-bin
   throttle cost profile at blk+4h. It also accumulates the avoidance vector at
   blk+34Ch/+350h, with the hold at blk+354h.
4. **Heading, 009DE5B0 at 009DE8F8** (docs/SHIP_AI_ARM_FINAL_STEP.md). Behind the same gate,
   once the hold has run out and the vector is longer than 1e-2, the heading target blk+324h is
   replaced with the vector's heading (plus pi when astern). This is the turn-away.

## Why it is off for the controlled ship

- 009F3DF3 forces the controlled unit (unit+184h set) into `cruise` whatever its director holds
  (docs/GAME_EXECUTABLE.md, the state census).
- Cruise, 009E1170, then takes **arm 2**. The check at 009E11C2 is `CMP byte [unit+184h],0`, and
  009E11D6 does `MOV byte [EAX+3F4h],BL` with BL = 0. That clears brain+3F4h, which is blk+3ECh.
  009E11DE sets brain+3F8h to -1 and 009E11EA clears brain+3FCh.
- So 009DA1D0 answers false for the controlled ship: no track is consumed, no throttle profile
  forms, and no heading override happens.
- Arm 1 (a Party slot that is neither 8 nor AI-held) also stores 0 there. Only arm 3 and the
  states that leave the pre-pass value (1) let a ship evade.

The usn_19_coralus.lua script (this installation, mtime 2024-08-26) never turns the Lexington's
evasion on. Line 868 turns it off in the last stage, together with `NavigatorStop`. Lines 157
and 197 turn it on for other units, and lines 1761-1772 set it by difficulty for the IJN main
fleet. Those settings matter only for AI-driven ships.

## Host

| stage | host state |
| --- | --- |
| pre-pass torpedo walk 009F158A | recorded (`ShipAi::replan_prepare_threat_scan`) |
| 009F0AD0 / 009EACA0 track creation | not bound; blk+400h stays 0 |
| 009DA1D0 gate | stubbed false (`ShipAiThrottleProfile::avoidance_active_009da1d0`) |
| 009E04E0 track walk | runs over an empty list |
| cruise arm 2's blk+3ECh = 0 | bound (`run_cruise_state_step_009e1170`) |

For the Lexington the host and the image agree: no evasion. For AI-driven ships the host lacks
the whole response. That covers the escorts and the IJN fleets where the script enables it, and
every ship left at the director's default of 1. Binding it is a separate, larger packet. It needs:
- 009F0AD0's allocation and stream-1 draws, which couple into the shared RNG (see
  `BSP_GUNNERY_RNG_STREAMS`);
- 009EACA0's track constructor and 009DC060's refresh, both unread bodies;
- the torpedo-list producer on the world;
- the 009DE8F8 override in 009DE5B0.

It would not change the Lexington's fate in these runs.

## Predictions

No evasion switch is added, so the E2 and USN04 pairs asked for in the packet would compare
identical binaries. None were run.

## Secondary: the torpedo-boat turn radius

`kShipTurnRadiusTorpedoBoatExempt` (on) makes the ship host's 0082E850 return class+520h
unmultiplied for a unit whose kind query answers 0Eh (docs/STATION_KEEPING.md, secondary reads).
Prediction, written before any run: no USN01 or USN04 row moves, because neither mission has a
torpedo boat. It will be measured on the owed USN04 pair.

Three other host bindings of 0082E850 skip the 2.0 multiplier for every ship: the follow step's
`ship_class_turn_radius_0082e850` (two bindings, the station latch and back-off radius at
009E16F0/009E1790) and the path follower's `owner_class_turn_radius_0082e850` (009E3EAE). The
arm tail's 009EF112 binding is recorded and returns 0. The image multiplies at every one of these
sites, so these hosts use half the image's radius. This packet does not change them, because runs
are blocked and the change would move follower latches and path corners unmeasured. Each would
become the same call as the formation host's.

Runs: a 120-frame probe at 12:36 died at the renderer init request (0xC0000005 after
`online_manager_initialize`, session rdp-tcp#0 active), so no pair ran. Both owed measurements
wait for the environment:
- the station-keeping USN04 4700/4500 pair from `build/win32/skC` / `build/win32/skT2`;
- the torpedo-boat switch, which needs a pair built from this tree with it off and on.

## Correction, 2026-09-24 (packet cc9_player_role_bookkeeping)

The Answer's premise is wrong: the Lexington is not "player-controlled" in the +184h sense.

- **The +184h premise.** unit+184h is set only when a player takes role 1 (00780214, through
  0059BBD0 on the role-1 permission). On USN04 that role stays PLAYER_AI (usn_19_coralus.lua lines
  458-459), so +184h is 0. The Lexington is not forced into cruise, and it sails its authored
  `CarrierPath1` under the AI. It does not sit still: `target_speed=0.00` and `target_moved=0.0 m`
  were host artefacts (docs/CONTROLLED_UNIT_HELM.md, correction of 2026-09-24).
- **The torpedo response is still off for it,** by a different arm.
  - Its captain role 0 is held by the human slot 0, which HUD page 27h (0067BB50) takes.
  - So cruise arm 3 stores 00521E70(unit, 0) = 0 into blk+3ECh, and 009DA1D0 stays closed.
  - The "why it is off" list above holds for a ship whose helm the player holds, not for this one.
