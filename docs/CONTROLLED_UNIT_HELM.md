# The controlled ship's helm with no input (packet cc9_controlled_unit_helm)

2026-09-23. Read-only. Names are hypotheses; offsets are unit-relative unless stated.

## Answer

**The image also leaves the un-driven controlled Lexington at throttle 0.** The host's stationary
Lexington (100.51 m moved in 449.96 s on E2 9000 and USN04 4500) is faithful. The order-ring
throttle the start-speed seed writes does not survive in the image either. No switch is added.

## Why the host's throttle is 0

The zeroing write in the host is the drive's order-ring store,
`ShipAiRing::set_write_slot_throttle` (`src/game_hosts_ship_ai.cpp:3466`, 0080E170 called from
009F3F80). It publishes the AI block's desired throttle `blk+1D0h`. For the controlled unit that
value is never set: `GameCommandsHost::cruise_step` records cruise arm 2
(`CruiseState::player_controlled_arm`, 009E11E8) and returns. So the block keeps its constructed
0.0. The ring tick 00813020 then slews the seeded live throttle unit+980h (0.8769) to 0 at
`slew_a` = 8.0/s, which takes about 0.11 s. The seeded axial speed of 15 m/s (0092D770) coasts
down to rest, which is the 100.51 m.

## What the image does

1. **Seed.** 0082356C sets the ring's live throttle and every pending slot to StartSpeed / reference
   (0080D9B0), and the controller's axial speed to StartSpeed (0092D770). It does not touch the
   confirmed pair at unit+994h/+998h (ring+15Ch/+160h). That pair stays 0 from the unit's bulk
   clear.
2. **Controlled means role 0 is human.** unit+184h is not written by 004C0890 (SetControlledUnit
   stores only the global 00E188D8 and publishes a listener). A byte-store scan for `[reg+184h]`
   finds its unit writers:
   - the entity constructor, 00928701;
   - the role setter 009281C0, sole caller 0077F480, which clears it at 009281D6 when role 0 is
     given slot 8, the AI;
   - the session entity-message arms at 00780214 (set to 1 when a player slot takes role 0 and
     0059BBD0 accepts) and 00780235 / 00780439 (clear).

   usn_19_coralus.lua (lines 458-459) gives every Lexington role to PLAYER_AI and opens
   `EROLF_AA_FLAK + EROLF_AA_MACHINEGUN + EROLE_CAPTAIN` to PLAYER_ANY. `luaWeHere` (line 3352)
   calls `SetSelectedUnit(Mission.Lex)`. The player therefore sits in the captain's seat, and
   unit+184h is set.
3. **Cruise forcing.** 009F3DF3 forces a unit with unit+184h into `cruise` whatever its director
   holds, so the script's `NavigatorMoveOnPath` orders for the Lexington do not drive it. The
   log's commands churn between the script's moveonpath and the AI's moveto every few seconds.
4. **Cruise arm 2, 009E11CE..009E1262.**
   - Arm 1 (009E11A5) is skipped, because the helm slot `[p+1B0h]` is 8 (AI).
   - Arm 2 is taken because unit+184h is set. It latches state+8 = unit+994h and state+0Ch =
     unit+998h, but only while state+8 is below -10.0f (00CE6848, COMISS at 009E11F9).
   - The brain constructor seeds both at 009F3A22/009F3A27 with -99.0f (00CF5BFC). Arm 1 resets
     them to that value (009E1407/009E140C), and the AI arm never writes them.
   - So the first arm-2 run latches the confirmed pair, and every later run re-applies it through
     009DFFB0 (rudder) and 009DBF90 (throttle).
5. **The confirmed pair only moves on authoritative ring slots** (00813144, `!predicted`).
   - The ring constructor 00812D40 marks every slot predicted.
   - 0080D9B0 and 0080E170 write values without clearing the flag.
   - Only the backfill 00812FA0 writes authoritative slots. It is reached from the helmsman
     message 8Eh (008141A0, lag-compensated, dropped unless unit+1B0h is the sender's slot) and
     from ship sync 00816C80.
6. **With no input no 8Eh is sent.** 0064B870, the HUD's integrated controls, issues through
   00816A40 every frame, but only while `00927F30(unit, 1)` holds, i.e. the local player holds
   role 1. The transfer that gives it role 1 (0077C470(unit, 2, 1) at 0064B9A6) needs:
   - a thrust axis above 0.1 (00D7A3A0), or a held-control flag;
   - unit+1130h == 0;
   - role 0 held by the player and role 1 not yet held.

   Only that transfer seeds the HUD lever from unit+980h/+984h (0064B9AE..0064B9BD). With no key
   press it never happens.

So in the image the first cruise re-plan latches throttle 0 and rudder 0. The drive writes 0 into
the ring every tick, and the Lexington coasts to rest from its 15 m/s start speed within a few
hundred metres. Its heading stays at the spawn heading, and it stays there until the script
releases the seat (line 866 hands every role back to PLAYER_AI in the last stage). **Expected E2
track: about 100 m, then stationary at the spawn point for the whole mission.** That is what the
host produces.

## What a game-validated reference would need

- A run of this installation with the player idle, recording the Lexington's position and whether
  the player holds its captain role after `luaWeHere`. The open link in the chain above is the
  0059BBD0 acceptance test at 00780208; it was not read.
- A harness that plays the player's part, if the reference should show a moving carrier. For
  example, the harness's order option could raise throttle through the HUD path (role-1 transfer
  plus an 8Eh helm order). That is not a host change; it is a different scenario.

## Host notes

- `GameUnitsHost::unit_player_controlled_0184` answers "is the controlled unit". The image's byte
  is "role 0 is human-held". The two agree here because the script puts the player in the
  Lexington's captain seat. They would differ for a selected unit whose captain role is AI-only.
- The E2 loss at 225.81 s follows from that idle player. It is not an AI or host defect.

## Runs

None. Runs were still blocked (probe failures at 12:30 and 12:36). The owed runs keep their order:
the station-keeping USN04 pair, then the torpedo-boat switch pair. This packet adds no pair.

## Correction, 2026-09-24 (packets cc9_scripted_helm and cc9_player_role_bookkeeping)

**The Answer above is wrong for USN04.** The image does not stop the idle player's Lexington.

- **unit+184h means the player holds role 1, not role 0.** 00780214 sets it on a role-1 take
  (`[msg+24h] & 2`) that passes `0059BBD0(unit, 1, slot)`. 0059BBD0 checks the role-1 permission
  word unit+18Ch: it must be 9 or the slot. A store census over every MOV form finds no other
  setter. Step 2 above misread the mask bit as role 0, and 009281C0's Ghidra comment repeats the
  error.
- **The Lexington's role 1 is AI-only on USN04.** usn_19_coralus.lua lines 458-459 open only the
  captain, machine-gun and flak roles to PLAYER_ANY, and EROLF_PILOT (role 1) stays PLAYER_AI (8).
  So +184h is never set. 009F3DF3 does not force the carrier into cruise, and its director keeps
  the script's `NavigatorMoveOnPath(CarrierPath1, PATH_FM_CIRCLE)` (line 515), which the AI drives.
- **What the player does get.** HUD page 27h (0067BB50) takes the captain role, role 0, on the
  controlled unit. That closes the ship's own torpedo request through cruise arm 3's
  00521E70(unit, 0) store, not through arm 2.
- **The host now does this** behind `kPlayerRoleBookkeepingBound` (docs/SCRIPTED_HELM.md
  section 6). The stationary carrier in the idle references before it (100.51 m in 449.96 s) was a
  host artefact of reading "the controlled unit" as +184h.
