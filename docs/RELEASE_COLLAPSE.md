# Where the E2 bomb releases went (25 to 10)

Packet `cc9_release_collapse`. No host code changed and Ghidra was not read or written. Every
count below comes from two existing E2 9000 logs, both with `BSP_GUNNERY_RNG_STREAMS=1`:
- **K0**, `local\K0_9000.log`: main before the plane death modes, from the
  `cc9_planner_kate_targeting` pair.
- **L0**, `local\L0_9000.log`: main 4ab52ffe0, the control of the `cc9_fighter_gun_lead` pair.

The table comes from `local\val_table.py`.

## 0. Units

`summary mission gunnery bomb_drops` counts **bombs**, not aircraft. A Val carries two bombs, and
K0 has one single-bomb release. So the headline is 25 to 10 bombs, which is 13 to 5 releasing
aircraft. The "25 to 5" in the packet brief compares bombs with aircraft.

## 1. Answer

| class | aircraft | bombs | what it is |
| --- | --- | --- | --- |
| (a) released after its own death in K0 | -11 | -21 | Val #1.1 x4, #5.1 x4, movieval x3 |
| (b) killed before release by changed AA | 0 | 0 | none |
| (c) live, reaches the dive, no longer releases | 0 | 0 | none |
| (d) new live releases in L0 | +3 | +6 | #3.1\|.-2, #3.1\|.-3, #7.1\|.-3 |
| **net** | **13 to 5** | **25 to 10** | |

**All of the loss is class (a).**
- Each of the 11 released in K0 between 8.4 and 25.5 s after its logged death. For example, Val
  #1.1 died at 150.50 s and released at 172.65 s, and movieval died at 174.51 s and released at
  188.95 s.
- In K0 a dead aircraft kept flying its dive arm and released. In L0 the death modes remove the
  aircraft, or the dead-release gate refuses it:
  - #1.1|.-4, whose census logged 2 bombs refused;
  - #5.1, with 1 + 1 refused.
- That is the faithful loss that `cc9_plane_death_modes` (fd70f01dd) established at
  007CEA1C/007CEA29.

**Class (b) is zero, and not by coincidence of totals.** Every class-(a) Val dies at the same
time in both logs, within 1.1 s:
- #1.1 at 150.50 s, #1.1|.-3 at 159.66 s, #5.1 at 227.51 s;
- movieval at 174.51 s, 178.71 s and 182.31 s.

So the AA that kills the first two waves did not move. The two Vals that released alive in K0,
#3.1 and #3.1|.-4, release in L0 too.

**Class (d) is positional, not RNG-coupled.**
- The three gains are Vals whose first dive pulled out at the 175-195 m floor in both runs, with
  the aim point 1.2-1.4 km away. In L0 a later dive reached aimglide and released.
- Their command target is Yorktown-class01. Its ship-AI trace is identical in both logs until
  step 3030 (about 151 s) and then diverges; at step 4000 the target bearing is 4.6629 against
  4.6971.
- In K0 dead Kates' torpedoes still reached Yorktown, and `cc9_plane_death_modes` removed Yorktown's
  4599 damage. So the carrier's track differs and the second dives meet a different geometry.
- Both logs use per-consumer random streams, so this is a position change, not a draw change.

**The faithful count is about 5 releasing aircraft (10 bombs) on this tree, and the reference
stands.**

## 2. The leaderless wingman (the L1b case)

In `local\L1b_9000.log` (the fighter lead on), fighters shot down flight lead Val #3.1 at
100.30 s. Its wingman #3.1|.-2 then flew without releasing. The leader's death is not why:
- **The wingman continued the attack alone.** Its dive arm ran the same chain as in L0: attackrun,
  fly-above, goaway, fly-above, turndown, aimdive. It entered aimdive twice.
- **Both dives ended in the pull-out exit.** They pulled out at 175 m with the aim point 1410 m
  and 975 m away (`db aim exit`, `pullout_18=2`). Neither reached aimglide.
- **The same pull-out ends most of wave #7.1's dives in both logs.** In L0, #7.1, #7.1|.-2 and
  #7.1|.-4 each dive twice and release neither time.
- **The host's Val squadrons are outside the squadron registry.** USN04 registers only the
  fighters. So `unit_is_flight_leader_007b8ad0` answers "leader" for every Val, as its labelled
  substitution says. Each Val runs its own dive task from arm tick 0, and there is no
  follow-to-leader link for a leader's death to break.
- **What the image does on promotion (007ED610) affects formation members still in follow.** A
  member already in attackrun is not one of them, so it does not decide this case.

The term that costs releases in both logs is the aimdive pull-out far from the aim point. The
dive-regime packets closed that term as image law, term by term (`docs/FAITHFUL_DIVE_SET.md`). No
host divergence was found for this packet to bind, so no switch or pair was run.

## 3. Per-aircraft table (A = K0, B = L0)

"release t/alt" is the first release census, in seconds (mission frame x 0.05) and metres. "dead"
is the `entity dead` time, with L0's death mode. K0 has no death modes. "final" is the last
dive-arm state in the hand-over census; the arm stops at death, so for a dead aircraft it is the
state it died in.

| unit | A release t/alt | A refused | A dead / mode | A final | B release t/alt | B refused | B dead / mode | B final |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| D3A Val #1.1 | 172.65 / 288 | 0 | 150.50 - | done | - | 0 | 150.50 explosion_delayed | flyabove |
| D3A Val #1.1|.-2 | 172.15 / 448 | 0 | 162.26 - | turndown | - | 0 | 162.46 explosion | aimdive |
| D3A Val #1.1|.-3 | 173.25 / 312 | 0 | 159.66 - | done | - | 0 | 159.66 powerlost | goaway |
| D3A Val #1.1|.-4 | 178.15 / 212 | 0 | 168.56 - | done | 175.75 / 283 | 1 | 168.26 powerlost | done |
| D3A Val #3.1 | 304.15 / 202 | 0 | 410.52 - | flyabove | 270.05 / 199 | 0 | 281.60 powerlost | flyabove |
| D3A Val #3.1|.-2 | - | 0 | alive | flyabove | 285.65 / 203 | 0 | alive | flyabove |
| D3A Val #3.1|.-3 | - | 0 | alive | flyabove | 311.55 / 227 | 0 | 320.15 explosion_delayed | flyabove |
| D3A Val #3.1|.-4 | 307.05 / 226 | 0 | 399.73 - | flyabove | 306.95 / 227 | 0 | 307.10 powerlost | flyabove |
| D3A Val #5.1 | 253.00 / 293 | 0 | 227.51 - | done | 242.30 / 282 | 2 | 227.51 powerlost | done |
| D3A Val #5.1|.-2 | 255.60 / 288 | 0 | 234.71 - | done | - | 0 | 235.81 explosion_delayed | turndown |
| D3A Val #5.1|.-3 | 253.40 / 244 | 0 | 230.51 - | done | - | 0 | 230.46 explosion_delayed | flyabove |
| D3A Val #5.1|.-4 | 257.20 / 265 | 0 | 243.06 - | done | - | 0 | 243.46 powerlost | goaway |
| D3A Val #7.1 | - | 0 | alive | flyabove | - | 0 | 432.07 explosion_delayed | flyabove |
| D3A Val #7.1|.-2 | - | 0 | 418.57 - | flyabove | - | 0 | 409.77 explosion | flyabove |
| D3A Val #7.1|.-3 | - | 0 | 373.13 - | flyabove | 367.90 / 217 | 0 | alive | flyabove |
| D3A Val #7.1|.-4 | - | 0 | 384.38 - | flyabove | - | 0 | 385.53 explosion_delayed | flyabove |
| movieval | 188.95 / 284 | 0 | 174.51 - | done | - | 0 | 174.51 explosion_delayed | aimdive |
| movieval|.-2 | 190.75 / 298 | 0 | 182.31 - | done | - | 0 | 182.31 explosion_delayed | aimdive |
| movieval|.-3 | 188.45 / 268 | 0 | 178.71 - | done | - | 0 | 178.71 explosion_delayed | aimdive |
