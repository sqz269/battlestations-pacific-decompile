# Ship AI: blk+32Ch = 0 and the 364h ahead/astern flip (packets cc13-drive32c, cc13-escortblock)

Read-only trace, 2026-10-19. There are no source edits. Host files are cited by function. Image
addresses are from the listing of /battlestationspacific.exe.

## 1. The question

In mode 3 (attack-move or NavigateAstern), with `kShipAiNavigateAsternSkipBound` ON, the prologue
at 009ED918..009ED93F stores blk+3E0h into 32Ch and 3E0h + 2000 (00CF0DD8) into 330h. The host's
3E0h is 0, so 32Ch is 0.

The skip ON run is cc9-lua52 `l70r_n_w_l10_main_ref`:
- Every one of Ashigara's 3113 `ship ai step` samples is `mode=navigate_astern`, with
  `d32c 0.00 d330 2000.00`.
- 1122 of the samples read throttle -0.625.
- From step 6990 to step 7370 the phases are regular: 20 steps at -0.625, then 20 steps at 1.000,
  a 40-step cycle. The heading error is 0.002-0.036 rad.

The skip OFF comparison is `l70r_as_l10_askip_ref`. There 32Ch is the real distance (12207.90 at
step 10), only 15 of 2216 samples read -0.625, and the mission completes.

The questions:
- Which comparison goes degenerate at 32Ch = 0, and does a 32Ch seed of 1000 stop the flip?
- Why does a leader in the image not stay boxed in by its own escorts (section 8)?

**Answer.**
- The one 32Ch comparison that goes degenerate (section 3) does not reach Ashigara's own-group
  nodes.
- Neither the host nor the image filters escorts out of the sector scan or out of the 009F475B
  back-off. So the back-off from an escort is image behaviour, and the host counts no escort that
  the image would not.
- What 32Ch does change is the steering: the free-bearing range and the target heading. The skip
  OFF run shows the flip almost vanishing when 32Ch is large. No OFF switch is proposed.

## 2. The readers of 32Ch / 330h

The drive 009F3F80 does not read 32Ch itself. The readers are:

| reader | image | effect at 32Ch = 0 |
| --- | --- | --- |
| arm-final free-bearing range | 009DE633..009DE662: range = min(318h*k, 32Ch) | range 0, so the JA at 009DC313 (range below kFreeBearingMinRange, 00CE38B8) exits with false; the log has 263 answers from 189040 arm calls |
| arm-final, other uses | 009DE77A | not on the throttle path |
| leader corridor | 009DEEBF..009DEEDD: hull + 32Ch | the corridor collapses to the hull |
| 009F0100 neighbour pass | 009F0022, 009F06D4..009F06EC | see section 3 |
| publish | 009F4D4D / 009F4D5B: slot+40h | read only by other ships' 009D8CE0 crossing negotiation, lim_o = min(range, slot+40h) = 0; the log shows negotiated=0 |

009EF350 and 009EF910, which set 370h, do not read 32Ch (`ship_ai_clearance_profile.cpp`).
009EB660, which sets sector.blocked, does not read it either: blocked comes from neighbour boxes
and avoid zones over the braking distance (`ship_ai_sector_scan.cpp`).

## 3. The degenerate comparison, and what [EDI+48h] is

The listing at 009F06D4..009F06EC:

```
009F06D4  FLD  [ESI+32Ch]
009F06DA  FADD [EDI+48h]
009F06DF  FCOMIP ...        ; along against 32Ch + [EDI+48h]
009F06E3  JBE  009F06FF
009F06EC  MOV  [EAX+88h], 0 ; node cleared
```

The host is `ship_ai_neighbour_pass_009f0100` (`ship_ai_neighbour_clips.cpp`, `along >
in.remaining_32c + in.corner_reach_add_1c8`).

**[EDI+48h] is settings+1C8h, not a neighbour slot's +48h.** Review rev2 asked; the trace of EDI:

- 009F02BF calls 00424C40 (`BSP_GameSettings_GetSingleton`). 009F02CF does MOV EDI, EAX,
  009F02E6 does ADD EDI, 180h, and 009F02FD saves EDI to [ESP+40h].
- No instruction from 009F0301 to 009F06D9 writes EDI. The whole spilled listing was searched.
- Inside the loop only the behind arm writes EDI: 009F07C6 MOV EDI, [EBP], the node. Every path
  then reaches 009F09DC MOV EDI, [ESP+40h] before the back edge (009F09F9 JL 009F0640).
- So at 009F06DA, EDI is settings + 180h, and [EDI+48h] is settings+1C8h. That field is
  `ShipAvoidance.NearbyShip_NextCornerReachDistAddOn`, default 100 (0083BB9B). Its partner at
  009F072F, [EDI+4Ch], is settings+1CCh, the line-check width.
- The [EDI+44h]/[EDI+48h] reads at 009F0854/009F085F sit in the behind arm, where EDI is the
  node. That pair is node data, not settings.

The threshold is therefore 32Ch + 100. It is not 32Ch plus the neighbour's 330h:

- **At 0:** every node more than 100 m along the course is cleared.
- **At 1000:** the window is 1100 m. A node inside it reaches the lateral test (009F06FF..009F073C)
  and posts side 1 or 2 (009F079E JB; in the behind arm, 009F0AB7..009F0ABF).

The comparison only matters for a node that gets past the gate. The loop calls 009D8010 first
(009F0650, host `if (!gate(i, 0.0f)) continue`). When node+68h is clear, the unit is the occupant
owner (00778890), and the neighbour shares its +284h group, 009D8010 does the following:

- It clamps node+7Ch to at least 0.5 (00CE3800 via 00415550).
- It sets node+88h = 0 (009D808B).
- It returns true only when +7Ch < 0, so it returns false every tick.

So for a group leader, own-group nodes never reach 009F06D4. Their pass side stays 0 whatever 32Ch
holds. For Ashigara, pass_posts=0 is explained by this gate for its escorts, and by the 100 m
window for any other ship.

## 4. The flip chain (image, then host `ship_ai_obstacle_tables.cpp`)

1. **Sector.** 009EB660 marks the sector blocked. 009D84E0 picks sector.pass_side = take_max ? 2 : 1,
   which is never 0.
2. **Back-off.** The guards at 009F4725..009F4755 are: hull_with_latch, committed_with_latch (364h
   set), a non-null blocker, its owner non-null, and owner+5Eh clear. Nothing tests the group. At
   009F475B, `CMP [EDI+88h], EBP` with SETNE reads 0 != 1/2, which is always true for a
   leader's escort, so back_off fires. The closing-speed arm (009F4768..009F4799, closing >
   0.75 * 3C4h, 00CEC9D8) is not reached.
3. **On back_off:**
   - 009F47A7 sets 380h = 1.0 s (00D7A24C).
   - 009F47AF..009F47EE adds 2.0 - |v_blocker| (00CE3958) to 384h when the blocker moves slower
     than 2.0.
   - 009F47FC/009F4805 sets the profile window to [-1, 0], astern only.
4. **Escape requests.** The gate is at 009F488D/009F489E (380h < 0 or 384h > 10). The requests are
   at 009F4977..009F4A57:
   - 3: stall >= 10 and stopped.
   - 4: 378h == 0 and (370h in {1, 3} or 384h > 10), at 009F4A44.

   36Ch is reset every tick (009ED788), so at the 009F4A59 gate, run = request != 0.
5. **Flip to astern.** 009F4AA0..009F4B26: with 364h set, the desired throttle is clamped to at
   most 0. Once body speed is below the rudder floor and 368h < 0, 009F4AEE clears 364h, zeroes
   384h, and re-arms 368h = 1.0 s.
6. **Ceiling.** With 364h clear, 009EC7C0 negates the ceiling and floors it at -0.625 (00D21A78,
   009EC9CD..009EC9F9). Those are the log's -0.625 samples.
7. **Flip back.** committed_with_latch is now false, so step 2 cannot fire. Request 2
   (!committed_with_latch && !36Ch && 370h == 0 && hold && !side) runs the other arm. After the
   1 s 368h dwell, 009F4B53 sets 364h again, and the cycle returns to step 2.

Each phase lasts one 368h dwell: 1 s, which is 20 steps at dt = 0.05. Two phases make the 40-step
cycle seen at steps 6990..7370. The heading error is small, so the ThrustMin_Slow arm (heading
error over 75 degrees) is not involved.

## 5. Ashigara is a group leader

From `l70r_n_w_l10_main_ref`:

- `formation group 0: leader=Ashigara type=6 column=0
  members=[Ashigara,Oyodo,Kiyoshimo,Asashimo,Sugi,Kashi,Kaya,Kasumi]`. Kaya leaves at t = 429.52.
- `neighbour Ashigara ... sector_blocks=13552 ... pass_runs=50 pass_posts=0 ... delivered=0
  side_changes=0 negotiated=0 traffic_writes=0`.
- `formation Ashigara role=leader steps=36000 limit344_min=0.1281 below1=36000`.

No log carries `BSP_SHIP_ESCAPE_TRACE=Ashigara` output (every `local\*.log` from 2026-10-17 on was
searched), so the blocker is not named.

## 6. Prediction with 32Ch = 1000 (330h = 3000)

- **Flip path, if the blocker is an escort.** Sections 3 and 4 are unchanged: the gate still
  holds the escort's +88h at 0, so if the sector is still blocked by an escort, 009F475B still
  fires.
- **Steering.** The free-bearing range becomes min(look_ahead, 1000). 009DC313 then stops rejecting
  every query, and the leader corridor widens to hull + 1000. Both change where Ashigara steers,
  and so whether the escort lies in the probed sector at all.
- **The skip OFF run as evidence.** With 32Ch = 12207 and the 009EE765 output running,
  sector_blocks is still 7340, yet only 15 samples read -0.625. Its target heading at step 10 is
  3.8173 rad (-2.466), where the skip ON run has -2.3038. The two runs differ in more than 32Ch,
  so this does not isolate the seed. It does show that a leader with a usable range is not boxed
  in by the same escorts.
- **Prediction.** With the seed, most of the 1122 -0.625 samples go away. The throttle holds ahead
  at its ceiling (capped by 344h; the logged minimum is 0.128), and Ashigara closes on its
  3E0h point. Any flips that remain are escort back-offs at 20-step phases. Those are image
  behaviour, not a host defect.
- **A non-group blocker within 1100 m along** gets a pass side that matches the sector's, so
  009F475B stops firing on it, and back-off then needs closing > 0.75 * 3C4h.
- **The publish.** slot+40h becomes 1000, which lets other ships' 009D8CE0 negotiation engage
  (negotiated is 0 today).

**Pass criterion for the pair:** one run with `BSP_SHIP_ESCAPE_TRACE=Ashigara`. It prints
364/36C/370/378/368/380/384, the blocked bits and the throttle at each flip. Add the blocker's
name if the logger lacks it. It names the blocker of every remaining flip.

## 7. The 364h initial seed (goaway's 009DFCB0 audit)

goaway's audit: the image seeds blk+364h = 1, and the host seeds it false, with the escape latch
as the host's only writer. goaway is verifying the image side. Through section 4 step 6, a fresh
host ship would start astern-committed, with its ceiling floored at -0.625, until its first
escape flip.

What the logs show for Ashigara:

- The first `ship ai step` sample, step 10, reads `dir=ahead throttle= 1.000` in both
  `l70r_n_w_l10_main_ref` and `l70r_as_l10_askip_ref`. Steps 20, 30 and 40 read the same.
- The first -0.625 sample in the skip ON run is step 630, after a ramp from step 600 (-0.559).
- The earlier `ship motion gate on "Ashigara"` line (throttle 0.417) comes before the AI drive and
  is not a 009EC7C0 output.
- So no -0.625 is visible before the first escape flip. The step logger samples every 10 steps,
  though, and request 2 (section 4 step 7) with 368h already below 0 would set 364h on the first
  tick (009F4B53). Steps 1-9 are not logged, so the log neither confirms nor rules out an astern
  first tick. A step-1 sample, or the escape trace above, would settle it.

## 8. Why a leader is not boxed in by its escorts (cc13-escortblock)

1. **Sector scan and its neighbour source.** No exclusion, in either image or host:
   - The walk 009F1856 (`ship_ai_neighbour_candidates.cpp`) skips only self, altitude and range.
   - Admission 009F0D20 (`ship_ai_neighbour_admission.cpp`) skips submarines and enemy
     submarines.
   - The image's sector loop (009EB85B..009EB91E) walks every node in [ESI+604h]/[ESI+608h], with
     only 009D8160 (avoid box hit), 009D80C0 (hull inside the near box, skipped) and 009DD540
     (ray clip). The arc loop (009EBDBD..009EBEAB) adds only the owner/+5Eh test.
   - No test reads +284h, the 009D8010 gate result, or a formation slot. The host's
     `ship_ai_sector_scan.cpp` matches.
2. **009F475B.** It applies to own-group blockers. The guards before it (009F4725..009F4755,
   section 4 step 2) have no group test and no earlier skip.
3. **First throttle sample.** Section 7: 1.000 at step 10 in both runs. -0.625 is first seen at
   step 630.
4. **The escort ring.** The `formation column` joins decompose each member's spawn offset against
   the leader's wake (00811180 with `kWakeDecomposeSegmentProjectionBound` ON).
   - Members spawned ahead of the leader (Oyodo, Kiyoshimo, Asashimo) get `along=0.00`. Oyodo also
     gets `across=-0.00`, which puts its station on the leader.
   - docs/HANDOFF_SHIP_SCREEN.md item 3 already settled that `along=0.00` joins are the image's own
     result ("the wake frame has no ahead"), and that 0070D290 rebuilds stations from them.
   - So the ring is image behaviour, not a host offset error.

**Conclusion.** The host counts no escort that the image would not. Nothing found here is a defect,
so no OFF switch is proposed. What keeps the image's leader from being boxed in is upstream of the
drive: a non-zero 3E0h in mode 3, which gives the free-bearing and corridor a range. That is the
unhosted 3E0h writer already named in `kShipAiNavigateAsternSkipBound` ("whatever steers mode-3
units in the image is unhosted").
