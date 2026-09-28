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

## USN02, 2026-09-27 (packet `cc9_torpedo_evasion`, second pass, worker cc9-ships2)

The question was whether Exeter's loss at 151.30 s (the SetFireTarget pair's ON side, base
`6c936d2d0`) is a missing evasion. It is not. **The image evades, and the host already evades the
same way**, through `kShipTorpedoResponseBound` and the whole 009DE5B0 (both ON). No switch was
added, so no pair was run. Ghidra was read-only.

### The image, and which ships take it

The detection, gate and consumers are the ones listed above and in docs/SHIP_TORPEDO_RESPONSE.md:
- **Detection:** the pre-pass torpedo walk 009F163F..009F1855.
- **Gate:** 009DA1D0.
- **Speed change:** the throttle profile 009E04E0.
- **Heading override:** 009DE5B0 section 5.

Player-side ships that the idle player does not hold take the response. Exeter's gate is open
(`gate_open` 64660 of its steps on main).

Section 5 has a gate of its own that the host reproduces. The whole section-5 body, listed by
address:

```
009DE8F3  CALL 009DA1D0 / TEST AL,AL / JE 009DE96C     ; the torpedo gate
009DE8FC  XORPS XMM0,XMM0
009DE8FF  COMISS XMM0,[ESI+354h] / JBE 009DE96C          ; needs blk+354h < 0
009DE908  |34Ch,350h|^2 against the double at 00D7A268  ; needs a vector
```

The traffic pass 009EF350 re-arms that hold every time it writes a heading:

```
009EF8E6  MOVSS XMM0,[00CE3854]        ; 3.0
009EF8EE  FSTP  [EDI+324h]             ; the traffic heading target
009EF8F4  COMISS XMM0,[EDI+354h] / JBE 009EF905
009EF8FD  MOVSS [EDI+354h],XMM0        ; hold = 3.0
```

**So a ship whose traffic pass is steering it never takes the torpedo turn.** It keeps only the
throttle profile's speed change. That is an image rule, and the host has it: the traffic pass and
section 5 share `ctl.blk.clamp_354` / `throttle_profile.hold_354`.

### What the host did (main `2a1d54495`, `BSP_TORPEDO_TRACE`)

The trace is diagnostic and env-gated. It is added at the end of the whole-009DE5B0 binding, so
no gameplay changes. `BSP_TORPEDO_TRACE=<unit[,unit...]>` prints the following every 0.5 s and on
every override step:
- the pose;
- the heading target before and after 009DE5B0, with the override flag and the gate;
- blk+354h and the avoidance vector;
- the nearest foreign torpedo's distance, time to closest approach and miss distance at the
  current velocities.

The `max_turn` column of the summary line is always 0.000 on the whole-routine path, because only
the partial path updates it. It is not evidence of an untaken turn.

**The premise moved.** On current main, **Exeter is not lost and USN02 does not fail inside 9000
mission frames**. Phase 2 is reached, and `MissionFailedRan=nil` (`local\te_main_usn02.log`). The
landings since `6c936d2d0` (cc9-lua2's and cc9-gunnery3's, among them the fire-cooldown flip)
moved the run.

**Exeter on main** took one torpedo, at about 160.4 s (Hatsukaze round #556, health 2641 after
it), and survived:
- It had 573 overrides, the first at 31.40 s.
- **Heading:** from 148 s the heading target was overridden to 1.4465 or 1.4599 (the vector
  (1.985, 0.248)). The ship turned from 0.587 to 0.893 rad by 160.2 s, about 0.025 rad/s.
- **Speed:** the throttle profile took the speed from 17.38 m/s down to 7.79 m/s at 154.2 s,
  and back to 16.59 m/s.
- The round closed with a predicted miss of 16.4 m and struck.

**Other player-side torpedo victims on main** (`local\te_main2_usn02.log`):

| ship | sunk | override in its last 12 s | blk+354h | note |
| --- | --- | --- | --- | --- |
| John3 | 152.75 (Asagumo) | none | 2.95 from before 141 s to 151.5 s | the traffic pass re-armed the hold each step. The vector was (1.916, 0.574) from 147 s; speed rose from 9.1 to 24.0 m/s |
| Encounter | 153.60 (Tokitsukaze) | none | 2.95 throughout | the same traffic-pass hold |
| Alden | 166.16 (Tokitsukaze) | yes, from 163.90 s | -0.05 | turning about 0.03 rad/s; the miss fell from 34.9 to 32.2 m and it was struck |
| Perth | 203.61 (Ushio, 5189 m) | yes | -0.05 | predicted miss 93 m until a new round at 203.3 s with miss 6 m |

**The SetFireTarget pair's ON side**, where Exeter was lost at 151.30 s, also shows Exeter's
response running: `gate_open` 36679, 204 overrides, the first at 28.75 s. No trace was taken on
that base.

### Verdict

- **The image turns and slows AI-driven ships for torpedoes, and the host does the same.** The
  gates are the image's: 009DA1D0, and the blk+354h hold that the traffic pass re-arms.
- **No manoeuvre is missing, so nothing is bound.**
- **Exeter's 151.30 s loss was the base's own outcome under the image's rules, and it is gone on
  current main.**
- **Open, not claimed:** whether the host's turn rate (about 0.025 to 0.03 rad/s for a cruiser
  near 15 m/s) matches the image's. That belongs to the rudder and hydrodynamics hosts, not to
  this response.
