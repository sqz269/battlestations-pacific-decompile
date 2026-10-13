# CC13 VER 14: USN21 strike losses, the Japanese CAP, and the AA machine gun (cc13-ver14)

Packet cc13-ver14 was a read-only verification for cc9, done on 2026-10-13.

- **Method.** Image bytes were read with `bsp.py disasm-raw` and `local/tools_scene2/sc2_x87.py`, and constants
  from the PE on disk. Mission files come from this installation. The evidence is existing logs only.
- **What was not done.** No game runs, no Ghidra writes, no C++ changes.
- **Adoption.** `src/game_hosts_gunnery.cpp` is gunnery40's file, so every host change below is an adoption note.
- **Status.** Nothing here is reconstructed, build-tested or game-validated.

## 0. The premise: killer_cat 1 is the ship AA machine gun

- **What `killer_cat` records.** It is the gun category of the last damaging hit: `dt.last_category =
  gun.category` (`src/game_hosts_gunnery.cpp:10308`), printed by the `death row` note (`:11737`).
  - Category 1 is `AAMACHINEGUN` (`src/gunnery_tables.cpp:17`).
  - Plane guns are category 0.
- **The run.** `J:\PROG\battlestations-pacific-decompile-cc9-ships44\local\s46_e_u21d.log` has 197 death rows:
  56 cat 1, 9 cat 5, 127 cat 6, 2 cat 7 and 3 with no killer.
  - Every cat-1 row names a Japanese ship as the killer.
  - `hits c0` is 0 on all 197 rows.
  - No Japanese squadron appears in the log.
- **The image has no Japanese fighters here either.**
  - `scripts/missions/usn/usn_sibuyan.lua` (2024-10-29) and its `.scn` hold these Japanese planes:
    - `JakeTmpl`, one floatplane (`:725`);
    - `Kamikaze1`-`15`, which `luaMoveToPh4` generates only in phase 4, after the strikes (`:884-899`).
  - The scene has no Japanese MotherShipGen and no airfield.
  - So fighter CAP, dogfight, escort and tail gunner cannot move the USN21 strike losses.

## 1. Japanese CAP that the script spawns and the host loses (question a)

**Spawn paths, all live in the host:**

- scene `PlaneSquadronGen` entities, which the logs list per mission;
- `GenerateObject` of Hidden squadrons: USN21's `JakeTmpl` spawns and dies to AA;
- `LaunchSquadron` `0089E3C0` and `IsReadyToSendPlanes` `00895D20`, both concrete.

A one-pass census of the 1347 logs over 5 MB in the worktrees found 38 missions. For each mission, the newest
log was taken and its `scene type PlaneSquadronGen ... party=Japanese` lines were read. Japanese fighters are
placed in IJN01, JM05, JM08, JM16, IJN14, PRCP08, USN06 and USNRM01. USN04 launches them from script.

**USN04 Coral Sea (`usn_19_coralus.lua`, 2024-08-26).** Japanese carriers launch Zeros with
`LaunchSquadron`:

- Shoho at `:2300` and `:2368`;
- Zuikaku and Shokaku at `:1393`-`:1591`, in phase 3.

The host spawns them: `LaunchSquadron 0089e3c0: class=150 count=4 -> slot 0 (started)`. Then they die without
being hit.

| log (all identical) | aircraft launched by Zuiho-class01 | water contacts, no death row, no damage |
| --- | --- | --- |
| `cc13-strafe2\local\st2_u4base.log`, `cc9-ships44\local\s44_gpon_u4.log` / `s44_gpoff_u4.log`, `cc9-gunnery36\local\g37_a9allon_u4p1.log` / `g37_a9alloff_u4p1.log` | 28: the Zero CAP sqn07, then Val/Kate sqn10, 12, 14, 16, 17, 18 | 28 |

The pattern, taking the sqn07 leader:

- lift-off at 264.16 s at the deck edge, y 13.3, |v| 40.3;
- the first surface probe at 290.55 s: alt 4.6, vy -14.0, spd 19.2, throttle 1.0;
- then `plane water contact ... dead=1`.

Yorktown's later script launches crash the same way. Those are sqn19-22, 12 aircraft, at about 17 s after
lift-off with vy -18 to -20. The early Yorktown sqn08 flies normally. No `Kill` native fires at these times.

**Effect.** In the host's USN04, Shoho's CAP and strikes never reach the fight.

**Owner.** This is cc9's plane / air-ops lane: the climb-out after the `cc9_base_launch_liftoff` hand-off. It
needs a run with a per-tick trace on one Zuiho squadron. Diagnosis was not attempted here.

## 2. The AA machine gun, image against host (question b)

Cat-1 kills are made by Japanese 25 mm mounts. Every mount that fired in USN21 is device 47, 48 or 46:

- device 47: triple, 3 barrels, 34088 rounds, 593 hits;
- device 48: turret, 1466 rounds, 13 hits;
- device 46: single, 6458 rounds, 127 hits.

US cat-1 mounts fired 0 rounds, because there were no Japanese aircraft to fire at.

### 2.1 Verified equal (bytes)

| term | image | host |
| --- | --- | --- |
| swing timer | `00902B16`-`00902B6B`: U(3, 8) on stream 1 x InterpolateClamped(0, 1.0, 6.0, 0.2 [`00CE54A0`], skill); timer at `00902AF3` decremented, re-rolled when < 0 | `:7232`-`:7238` |
| error magnitude | `00902CD8`: wrap-sum of the two angle deltas x row+0Ch; `00902D07`: + U(0, row+14h) x 3.1415927f / 180 | `:7241`-`:7266`. The deltas use the aim point, not `vtable[100h]`'s sample; this is swamped by the clamp (below) |
| ship-only divide | `00902D22` IsKindOf(6); AI-held -> /[settings+750h], else /[754h] | `:7268`-`:7271`. Planes are not divided |
| new pair | two `00BD2F90(0, e)` normal draws, rates = (new - old) / timer (`00902D77`-`00902DE0`) | `:7272`-`:7276` |
| integrate, clamp | `00902DEB`-`00902F2C`: h += rh x dt; at \|h\| > 25.0 / d (`00CE3880`, d = the slot `00901C20` writes, see 2.2) clamp, timer = 1.0 (`00D7A24C`), rates x -0.5 (`00CEC9E0`); the same for v | `:7279`-`:7298`, with d = lead distance (`:7181`) |
| negative vertical | `00902F62`-`00902F76` x 0.5 (qword `00D7A280`) | bound earlier |
| fire byte | `00902FE8`-`0090306F`: \|wrap dh\| + \|wrap dv\| < 0.0872664675 rad (`00CF0098`); `[gun+3F0h]+634h` bit 0 inhibits | `:7662`-`:7666` |
| cadence | `0072D130` fixed-step tick: barrel delay `gun+450h` -= dt, one 0ADh per step; `00729A80` blocks on delay > 0 (`00729AC7`) and fires a barrel with timer <= 0 (`00729B6C`); `00730160` sets the timer to the reload (no carry) and the delay to `[class+30h]` (`00730A05`) | same order. Triple 25 mm (reload 0.5, delay 0.16): every 4th 50 ms step, so 5 rounds/s in both. **This closes mgaa1's open item 1** |
| per-hit chain | already verified by `docs/CC13_AUDIT_MG_AA_LETHALITY.md` section 2 | |

USN21 runs every gunner on skill row 1, `{AngleDiffErrorRatio 5, ConstAngleError 4 deg}`: 13597 rolls. The
summary records 15627 clamps and a mean miss of 20.30 m. The error therefore lives at the 25 m clamp. That is why
the sample-point substitution in the angle term cannot move the hit rate.

### 2.2 D1: the fire gate measures to the lead point in the image

**Image.** `00902FC0`-`00902FE2` fires only while `slot < [proj+60h] * 0.9f` (`00D7A390`). The slot is the
`outDist` argument of `00901C20`, that is |lead point - gun|:

- it is pushed as `lea [esp+14h]` at `009029FD` and `00902A4F`;
- `push edi` is at `009029C1` and `pop edi` at `00902F61`, so `[esp+14h]` at `00902E23` and `[esp+10h]` at
  `00902FC0` are the same frame slot.

**Host.** `src/game_hosts_gunnery.cpp:7648`-`7652` measures to `unit_aim_point(target)`, the current
Height-raised point, and `:7665` gates on that.

**Effect.** For an inbound aircraft the lead distance is about d / (1 + vr/800), plus the
CalcTargetPosTimeAdd biases. So the image opens fire at a current slant distance of about 1440 x (1 + vr/800):

- on the flyabove leg (vr 40-45 m/s): about 1515 m;
- in aimglide (vr 100-120 m/s): about 1620-1660 m;
- outbound, the image stops earlier.

The host's cat-1 kills stop at 1404 m: 8 of 56 fall in 1300-1440 m, and none beyond. The host is therefore
slightly under-lethal on the approach. A fix lowers releases slightly. There is no RNG change.

**Adoption.** In the cat-1 branch, gate on the lead distance (`:7181`), behind an OFF switch.

### 2.3 D2: when the AAFlakBot rolls its error

This extends `docs/CC13_GPR_40.md` 1.4. A full `.text` rel32 scan finds three callers of `008FDBE0`:

| caller | where | when it rolls |
| --- | --- | --- |
| `008FF055` | `008FF040` AttachBallisticClass | at attach |
| `008FF247` | SetFireTarget `008FF200` | at each SetFireTarget |
| `0090331B` | the flak tick | when gun+474h > bot+64h, so after a shot |

SetFireTarget is reached only through the vtable slot `00D18270`, from `00727F10`. `00727F10` has one caller,
`00865833` in the `00864FE0` assignment body. That body runs every 2.0 s and re-sends the current target. So the
image re-rolls every 2 s per flak gun with an air target.

**Host.** It rolls only after a shot (`:7327`), starting from {0, 0, 0} (`AaFlakError`, `:3004`). So each flak gun's first salvo
of the mission has no error.

**Effect in USN21.**
- 89 flak guns fired: 2737 rounds, of which 723 cat-6 hits at 39.6 each.
- Perfect first salvos add about 40-55 hits, or 5-7 % of flak damage: about +2-4 flak kills in the host. Here the
  host is slightly more lethal.
- The image also draws 7 stream-1 values per roll at every re-send and at attach, which re-orders stream 1.

## 3. Numbers for cc9

The run is USN21 `s46_e_u21d`. The victim is the Helldiver (HP 200, Armour 5; `vehicleclasses.lua`, 2026-05-09).

| | cat 1 (25 mm) | cat 6 | cat 5 |
| --- | --- | --- | --- |
| kills | 56 | 127 | 9 |
| mean first damage -> death | 9.7 s | 5.6 s | 7.3 s |
| kills beyond 1300 m / 1440 m | 8 / 0 | 104 / 98 | 5 / 5 |
| per-hit damage (measured) | 14.6 (bullet 46: 47-55 raw, -5 armour, player-side scale ~1/3.15) | 39.6 | 15.9 |
| per-round hit rate | 1.74 % (733 / 42012) | 33 % (723 / 2188) | 17 % (96 / 549) |

For cat 1 this works out to about 14 hits, or about 790 rounds, per pure-MG kill. That is about 160 mount-seconds
at 5 rounds/s.

The image's per-round terms are equal (2.1). Its range gate differs (D1, image more lethal inbound) and so does its
flak roll timing (D2, host more lethal).

**Verdict.** Host AA is not too lethal overall. The USN21 losses come from exposure (`docs/CC13_VER_9.md`) and
from flak.

## Handoff

- **gunnery40.** Adopt D1 and D2 (sections 2.2 and 2.3), each behind its own OFF switch. Pair on USN21.
- **cc9 plane / air ops.** The USN04 post-launch crashes (section 1). Note that "fighter AI" cannot be judged in
  USN04 until Shoho's planes survive the climb-out.
- **Open.**
  - The image's `vtable[100h]` target sample (`00902BFD`) is unread.
  - Whether `008FF200`'s roll updates the bot+64h stamp, which would suppress the next post-shot roll, is unread.
  - The flak bot's other term, the per-SetFireTarget fire-mode switch for kind 6, is in GPR_40.
