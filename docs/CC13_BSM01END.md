# CC13 bsm01end: why BSM01 stopped ending

Packet `cc13-bsm01end` (read-only, cc13-seg67n, 2026-10-19).

**The question.** BSM01's end test is `bsm_01_stationed_at_pearl.lua` line 859 (the lead's 857): the
mission ends when `DonaldShotDown >= 10`, when `RunningJapGang` is empty, or when Donald is more than
17 km from Akagi. Which of these three legs stopped holding?

**The logs.** Both are in `cc9-lua52\local`:
- old: `l69r_b1d_main_ref.log`, head `ec0df436c`;
- new: `l70r_n_b1d_main_ref.log`.

**Answer.** None of the three legs fails, because the test is never evaluated: the new run never leaves
phase 3. Phase 4 needs a hit by HenryPT on the MiniSub, and in the new run that hit never lands.
- In the old run the hit was one machine-gun round at 679.86 s.
- In the new run the sub runs aground in the shallows. The host's seabed clamp holds its throttle at
  zero both ways and keeps it at the surface. The image's bytes do the same.
- No host defect was found on this path, so no switch is proposed.

The scripts used are in this packet tree's `local\` (`b1_*.py`).

## 1. The gate that fails: MiniSub, then WEGOTIT, then `luaMoveToPh4`

**The end test is inside phase 4.** It sits under `MissionPhase == 4`.
- New run: the summary at line 1213489 reads `MissionPhase=3 EndMission=nil`.
- Old run: `dialog callback luaMoveToPh4 t=684.16`, then `EndMission at 809.93 s`.

**Phase 4 starts only from the hit listener.** The listener and its callback are in the lua file:
- `luaAddMiniSubListener` (line 2006) adds a `hit` listener on target `{Mission.MiniSub}`, attacker
  `{Mission.Henry}`. Line 377 sets `Mission.Henry = FindEntity("HenryPT")`.
- `luaMiniSubHit` (line 2032) starts the `WEGOTIT` dialog. That dialog's last entry is the callback
  `luaMoveToPh4`.

**The three legs only exist after `luaMoveToPh4`.**
- `luaMoveToPh4` (line 922) sets `AttackRunning = false` and creates Donald (line 937).
- It also spawns the 20 squadrons of lines 1010 and 1012.
- `luaJapSpawned` puts their Val and Kate wings into `RunningJapGang` (line 1493).
- So in the new run the gang is never filled, Donald never exists, and the end test is never reached.

**The exitzone removal works in both runs.** Jap #11.1, #14.1, #13.1 and #16.1 leave through 00982120
and die with `KillReason=exitzone` (00929800). Before phase 4 these planes are not gang members.

## 2. Why the hit lands in the old run and not in the new one

**Old run.**
- Line 189596: `hit listener 00988510: target "MiniSub" type "Bullet" damage 0.0`.
- The death row has killer=HenryPT, killer_gun=0, category 1, range 286.
- HenryPT was parked at (-446, -2984). The sub's depth trace rose from y -6.96 to -0.6.

**New run: the listener counts 17899 hit events and fires 0 times.**
- The 120 blasts on MiniSub come from bullet classes 17 and 18. These are `Type=Artillery`, first used by
  category 2. HenryPT has no such gun: its guns are `1:3 7:4 8:1`, and it drops its depth charges as
  weapon group 5. The attacker filter rejects these blasts correctly.
- The listener does fire on HenryPT's depth charges whenever they reach the sub. cc9-gunnery35's
  `g35_b1v0`, `b1v1`, `b1v3` and `b1v4` each log `type "Depthcharge" damage 0.0 -> luaMiniSubHit()`.
  So the 00988510 path works.

**HenryPT's track diverges at frames 5660..5670 (283.5 s).**
- The order is the same takehelm toward (2234, -2203), but the rudder reads -0.125 in the old run and
  -0.175 in the new one.
- The first route arrival comes after 180 helm issues in the old run and 186 in the new.
- HenryPT reaches the line-9 park point (-430, -2990) at about 542 s in the old run and about 724 s in
  the new.
- From there it drops 233 charges up to 886 s without a hit.

**The sub runs aground from about depth-trace step 6800 to the end of the run.**
- It reads `y=-0.5 clear=+1.43 front=5..6 rear=12..14` throughout.
- Its target is Rescue (`sub_attack`, `fire=Rescue`), and the Rescue deaths differ between the runs.
- The log has no line that gives the sub's x/z position, so its distance from HenryPT is inferred, not
  measured (section 4).

**The divergence starts at 133.5 s, earlier than the 186 s AA kill.**
- The earliest differing death rows are the land cranes:
  - Crane 01 14 and Crane 02 08 take their first damage at the same moment, 133.5 s, but die at
    different times;
  - Crane 01 12, 01 09 and 01 17 die only in the new run;
  - Crane 01 13 is killed by HenryPT in the old run and by Sacramento in the new.
- The air kills follow. Jap #6.1|.-III takes first damage at 141.30 s in the old run and 165.61 s in the
  new; Jap #9.1|.-II dies at 186.21 s (Nevada) against 188.01 s (HenryPT).
- So the run is chaotic. The old run completed because of one hit.

## 3. The grounding, read from the bytes

Each subsection gives the image's behaviour first, then the host's.

### 3.1 The scan

**The image** builds the floor estimate in two places:
- **`00855420`** (`BSP_SubmarineUnit_UpdateMotion`) scans one footprint point per step:
  - a provider height of 0 is skipped (008557F8);
  - otherwise the candidate is `height + class Height + 3.0` (0085580C..0085583C);
  - the accumulator keeps the maximum (00855828);
  - the climb rate `(candidate - hull_y) / time` goes to a front or a rear bucket (0085585C);
  - this applies only while the scan state is below 10 (00855842) and the candidate is above the hull
    (00855851).
- **`00855440`..`0085547A`** publishes when the state is 0:
  - +F38h goes to +F2Ch, the clearance;
  - the buckets +F3Ch and +F40h go to +F30h and +F34h, and the buckets are zeroed;
  - the accumulator is reseeded from +EFCh, the deepest band.
- **The offsets.** `00936DC0` reads +F2Ch, +F30h and +F34h as +123Ch, +1240h and +1244h. Its base is
  310h higher.

**The host** has the same publish and sample code: `submarine_scan_publish_00855440` and
`submarine_scan_sample_00855420` in `src/submarine_model.cpp`.

### 3.2 The depth target

**The image's `00936DC0`:**
- The target starts as `bands[effective]` (00936EAB).
- It is clamped only when both conditions hold:
  - the owner's role-1 slot (`[[+740h]+50h]+1B0h`) is 8, or `00927F10` answers true (00936EC5..00936ED4);
  - the clearance +123Ch is above the band (00936EE2, `JBE`).
- When it is clamped:
  - the target becomes `00415510`'s maximum of the two (00936EF5);
  - the gain becomes `00CE380C` = 1.5 (bytes `00 00 C0 3F`).

**The host:** `submarine_depth_target_00936dc0` does the same.
- **SUBSTITUTION, labelled:** the slot test reads the boat's own role-1 slot, and it treats a slot of
  0..7 as player-held.

### 3.3 The throttle bounds

**The image:**
- 00936E00..00936E2B seed the order ring's slot (`+97Ch` cursor, +844h and +848h) with 2.0
  (`00CE3958`) and -1.0 (`00D7A260`).
- When the gain is above 1.0 (0093706F), 0093707C..00937107 overwrite them:
  - +848h = `-00419010(0.5, 1.0, 3.0, 0.0, +1244h)`, from the rear rate;
  - +844h = `00419010(0.5, 1.0, 3.0, 0.0, +1240h)`, from the front rate.
  - The constants are 0.5 at `00CE3800`, 3.0 at `00CE3854`, FLD1 and FLDZ.
- `00419010` (`BSP_Math_InterpolateClamped`) maps a rate of 3.0 or more to 0.0.

**The host:** `submarine_throttle_bounds_00936dc0` uses the same constants and the same front/rear
assignment.

### 3.4 Applied to MiniSub

- The clearance of +1.43 is above every band (0, -7, -40, -80), so the clamp holds the sub at the surface
  whatever level the script orders.
- The front rate (5..6) and the rear rate (12..14) are both 3.0 or more, so both throttle bounds are 0.
  The boat can neither go ahead nor go astern.
- **Verdict:** faithful as far as the bytes go. Fed the same terrain, the image's boat stops in the same
  place.

## 4. A position line for the sub

The host has no log line with the MiniSub's x/z position.
- **Cheapest source:** `trace_submarine_depth` in `src/game_hosts_units.cpp`, which writes the
  `submarine depth trace` line every 200 force steps. It could also print `slot_.motion.position[0]` and
  `[2]`.
- That change is to log output only and moves no gameplay. It is not made in this doc-only packet.

## 5. Switches in the window `ec0df436c..241db740b`

This list was taken from `git log` over the window, not from rev2's list of 21.

**Feed the divergence; no reach on this gate:**
- `e96489cd3` `kLineOfFireLandFromMuzzleBound`: the likely first cause of the 133.5 s crane
  differences, because AA fire past land is gated. docs/CC13_LOFLAND.md finds it faithful.
- `fb31ab760` (dogfight fire envelope and per-task gun block), `f886af750` `kRoleOwnerDeviceWalkBound`,
  `917c0b57c` (PerRack, follow-release seams, goaway stores) and `b473e5fcf`
  `kPlaneDeviceWalkSharedDrawBound`. Each changes plane fire or bomb release, and each was flipped by
  mechanism.

**No reach:**
- `5288f4100` / `5eecdcfd6` `kPlayerGunSeatTargetInterceptBound`. Only the group 1/2 arm reads the seat
  target (00959C91, 00521E30). b1d fires group 5 only, and the 005488FE gate never targets ships or
  submarines (0042BB40).
- `843efc336` `kNavigatorForceTorpedoBound`: no torpedo is involved on this path.

**The submarine model:** no commit in the window changes the dive or seabed model.

## 6. Status

- Read-only; no code change.
- The gate, the hit-listener filter, the exitzone removal and the seabed clamp all match the image.
- If the b1d run is meant to complete reliably, its orders file must bring HenryPT to the sub. That is a
  harness change, not a host switch.
