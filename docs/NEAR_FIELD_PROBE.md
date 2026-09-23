# The near-field probe 007F0280: the accumulator, and its binding

Addresses: 007F0280, 007F02FE, 007F0555, 007F05F7, 007F06AF, 007F0916, 007F0936, 007F0A1D,
007F0A96, 009C42B8, 009C42BD, 009C6AA3, 009C6B3A, 009C6B75.

Packet `cc9_near_field_probe`. Names are hypotheses, not recovered symbols. The arguments, the
iteration and the eighteen callers are in `docs/BOT_PROBE_007F0280.md`. This document reads what
that one left open (its section 0.6) and binds the probe.

## 1. The accumulator, 007F06AF-007F0AF6, read from the listing

**x87 at loop entry.** `007F0555 FLD1` enters the loop with one value on the stack. Every
iteration leaves exactly one 1.0 there:
* the accepted path: each weight arm ends with an `FLD1`-derived 1.0 in the old `ex0` slot, as
  traced below;
* the three reject paths: `007F0918`/`007F091C` pop the extents, then `007F0923 FSTP ST0` and
  `007F0925 FLD1`.

So the output block starts with `[1.0]`. `tools/x87_sym_walker.py` over `tools/x87trace.py`'s
listing of `007F0941`-`007F0AF6`, with `--init one`, confirms this. Its depth runs exactly one
above x87trace's, which assumed an empty entry, and the path ends balanced at depth 0.

**Frame depth.** Inside the loop the depth is `0x120`, 4 bytes more than the initialisation at
`0x11C`. So loop `[ESP+44h..4Ch]` and `[ESP+50h..58h]` are the init's `[40h..48h]` and
`[4Ch..54h]`: **all six start at the extents** (`007F02FE`-`007F031C`). The out-triples are
`[ESP+12Ch]` (arg2) and `[ESP+130h]` (arg3) at that depth.

**Per candidate.** After the box test, stack `[e.z, e.y, e.x, 1]`:
1. `[ESP+34h..3Ch]` are reset to 0.0 (`007F0580`-`007F058C`, XMM0 = 0).
2. **Weights** (`007F06AF`-`007F076E`): for axis i with `w_i > 0`,
   `f_i = 1 + w_i * |p_i| / e_i`. For example x: `007F06E5 FDIVRP ST3` gives `|x|/e.x`, then
   `007F06EB FMULP ST3` by w, `007F06ED FLD1; FADD ST3,ST0`, and `FSTP [ESP+34h]`. Each arm
   leaves the stack one shorter, and both arms of each axis converge (`007F06F9 FSTP ST2; FLD1;
   FXCH ST2`, `007F0739 FSTP ST1`, `007F076E FSTP ST0`). The result is `[1]`.
3. **Sign rule** (`007F0770`-`007F0853`): if `f_i > |p_i|`, then `p_i := +-f_i`, decided by
   comparing candidate+9D0h (ESI) with self+9D0h (EDI):
   * x and z: `JGE` gives -f, so +f when the candidate's index is lower;
   * y: `JLE` gives -f, so +f when the candidate's index is higher.
   * `-f` is formed as `00D7A208` (-0.0f) minus f. With zero weights f = 0 and nothing changes.
4. **Nearest per side** (`007F0859`-`007F0910`): `p' >= 0` (`COMISS`/`JC` against 0) stores
   `P_i = p'` unless `p' > P_i` (`JA`). Otherwise `N_i = -p'` unless `-p' > N_i`. So P and N are
   the nearest intruder on each side, starting at the extent.

**Output** (`007F0936`-`007F0AED`), only when `[ESP+1Bh]` was set by an accepted candidate:
* `pos_i = e_i > P_i ? 1 - P_i/e_i : 0` (`007F0977`-`007F0981`).
* `neg_i = e_i > N_i ? N_i/e_i - 1 : 0` (`007F0993`-`007F0997`; z uses `FSUBRP` at `007F0A0F`,
  the same value).
* `out_a_i = pos_i + neg_i` (`007F0A28`-`007F0A5C`, stored at `007F0A64`-`007F0A71`).
* `out_b_i = pos_i > -neg_i ? pos_i : -neg_i` (`007F0A74`-`007F0AED`).
* With no accepted candidate, both triples keep their entry zeros.

**Iteration and exclusions.**
* Self is skipped by `007F056F CMP ESI,EDI`, with EDI = arg0.
* Mode != 0 walks `[unit+C50h]+30h`. That is the neighbour object's "everything near" list
  (`007E11D0`: IsKindOf 6 or 0Fh within `(+88h + 3) * 180 = 1080` m, refreshed every 3 s),
  filtered to aircraft by `vtable[5Ch](0Fh)`.
* Mode 0, or a null `+C50h`, walks `ECX+3D0h` (the own squadron at every dive-bomb site:
  ECX = approach+0Ch).
* No liveness test is made in the probe. Dead units leave the lists at the refresh.

**Law in one line** (zero weights): for everything in the box, the nearest intruder on each side
of each axis sets `out_a = (N - P)/e`, a signed push, and `out_b = max(1 - P/e, 1 - N/e)`, the
depth of intrusion.

## 2. The binding (kNearFieldProbeBound = true)

`include/bsp/near_field_probe.hpp` / `src/near_field_probe.cpp`:
* `near_field_probe_007f0280` is a pure rule over an explicit candidate array;
* three site adapters sit beside it.

In the host, `nf_probe_007f0280` builds the candidates from every live aircraft other than self,
in the unit's frame, with `plane+9D0h` from the squadron record's `member_formation_index`.
**SUBSTITUTION, labelled:** the image reads the 3 s-refreshed 1080 m list. A direct scan has no
refresh lag, and the box (140 m at most) is deep inside that radius.

| site | caller state | extents / weights / mode | use | wired |
| --- | --- | --- | --- | --- |
| `009C42B8` | dive-bomb attack run, re-roll arm only | 80/60/120, 0, 1 | lateral offset `-(a.x b.y b.z) * 0.5236` | yes |
| `009C6B3A` | dive-bomb fly-over | 80/70/140 (can-dive clear) or 60/70/90 (set), (30,0,0), 1 | slot entry-108 `-a.z b.x b.y * 1.2 (- 0.2)` when `|a.x| > 0.05` | yes |
| `009C4878` | dive-bomb goaway heading (`009C47D0`, through `torpedo_goaway_heading_009d0c10`) | 80/50/100, 0, 1 | `probe = -(a.x b.y b.z)`, turn nudge x 40 degrees when opposed | yes |
| `009D0CBC` | torpedo goaway | 60/50/90, 0, 0 | `-p0 p1 p2` | **contract**: which out-slots feed p0..p2 is not established |
| `009D1A94` | torpedo aim | computed | sector probe | contract |
| the other 13 | states the host lacks or does not tick | - | - | contract |

* **The fly-over's second output** is not bound. `-a.x * b.y * b.z` is stored at `[ESP+1Ch]`
  (`009C6B7B`-`009C6B9E`), passed through a 0.5 dead band and scaled by 2.0 (`00D7A308`) and
  0.5 (`00D7A280`). Its consumer in the heading arm is unread, so it is a contract.
* **`009FD570`'s obstacle list is not the probe.** The probe does not call it
  (`docs/BOT_PROBE_007F0280.md` section 3). The solver rebuilds its own obstacle cache from
  `GGame+19CCh`'s unit list, which is still a hole.

## 5. Predictions, written before runs P0/P1

P0 is this tree with `kNearFieldProbeBound = false`. P1 has it on. Both have the hull-aim trace
on and the hull offset off. PH is P1 with the hull offset on. Runs are USN04 at 4700/4500
(reference: 23 dive-bomb releases), and P0/P1 again at the E2 9000-frame parameters.

1. **Who triggers.** Only aircraft in close formation: same-flight Vals and Kates, and the
   fighter wings. docs/FLYOVER_EXIT.md step 3 puts same-flight aircraft inside the fly-over box
   for about half of the turndown.
2. **Fly-over trim.** When `|out_a.x| > 0.05`, slot entry-108 becomes
   `-a.z * b.x * b.y * 1.2` (minus 0.2 once the can-dive byte is set), bounded by about 1.4. It
   feeds the fly-over speed arm (`s > 0 ? 40 s : m s`), so the desired speed of a Val flying
   over moves by up to tens of m/s. Expect it to change turndown timing, and with it the
   turndown-entry offset per Val.
3. **Attack-run weave.** It is sampled only on the re-roll arm. The lateral offset is
   `-(a.x * b.y * b.z) * 30 degrees`, and each factor is at most 1. With a partner that is
   ahead, astern or above rather than beside, one factor is small, so expect only a few degrees
   in most samples.
4. **Dive-bomb rows** will move for the Val flights through items 2 and 3. Releases change by a
   few either way; the sign is not predicted.
5. **Fighters.** Not wired (the dogfight states' callers are not bound). They change only
   through other units.
6. **Hull-on (PH).** If the sideways dive entries come from the missing weave, releases recover
   towards 23. If they do not, the weave is not the cause.

7. **Box occupancy in P0**, from `local/box.py` (J:\PROGattlestations-pacific-decompile\local\cc9ox.py
   pointed at `local/P0_4500.log`'s `hull_trace` lines, using the attack-run box 80/60/120).
   This was added after P0 finished and before any treatment result was read.
   * Same-flight Vals sit inside each other's box for most of the traced states. `D3A Val
     #1.1|.-2` is inside for 81 of 164 pair-ticks in state `734` and 150 of 159 in `79c`. The
     `#3.1` flight is inside for 138-157 of about 150 in `79c`.
   * `movieval|.-2` is never inside (min separation 195 m).
   * So expect probe hits on nearly every Val tick in those states, non-zero fly-over trims,
     and goaway nudges. The per-Val dive-bomb rows will move for every Val flight except the
     movieval wing member that flies alone.

## 6. Runs

Each ran from its own copied binary through `tools/run_game.ps1 -Exe`, and each log ends with the
renderer's final COM release line. The 4500 runs use `--frames 4700 --press-start-frame 30
--menu-select USN04 --mission-frames 4500 --mission-frame-seconds 0.05`. The 9000 runs use the E2
parameters. The hull-aim trace is on in every binary.

| run | probe | hull offset | frames | log | releases | water contacts |
| --- | --- | --- | --- | --- | --- | --- |
| P0 | off | off | 4500 | `local\P0_4500.log` | 18 | 1 |
| P1b | on | off | 4500 | `local\P1b_4500.log` | 17 | 0 |
| P0H | off | on | 4500 | `local\P0H_4500.log` | 11 | 0 |
| PHb | on | on | 4500 | `local\PHb_4500.log` | 10 | 0 |
| P0 | off | off | 9000 | `local\P0_9000.log` | 26 | 8 |
| P1b | on | off | 9000 | `local\P1b_9000.log` | 29 | 8 |

**The control is 18, not the reference 23.** This branch does not contain main's newer commits:
`git merge --ff-only main` refused, because main does not contain `24eac22c7`. The pair is still
a same-tree, same-binary comparison.

P1 and PH, built before the goaway site was wired, were cancelled while queued and never ran.

**P1b against P0 at 4500.**
* **The probe:** 6944 calls, 1864 hits, 908 attack-run weaves (maximum
  `|a.x b.y b.z| = 0.56`, so 16.8 degrees), and 422 fly-over slot writes (slot between -0.303
  and 0). Goaway hits come from the #3.1 flight and `#7.1`. Every Val flight triggers it, as the
  box occupancy predicted, and so do the movieval pair (weaves only).
* **Releases 18 -> 17.** `D3A Val #3.1|.-3` loses its release. Its only probe input is 41
  attack-run weaves: no fly-over trims and no goaway hits.
  * Its final attack heading moves from -2.9931 to -3.0047 rad, and turndown latches at tick
    1072 instead of 1070.
  * Aimdive exits to aimglide at 205.2 m instead of 268.7 m.
  * Aimglide is then blocked on bearing in all 26 calls (before: 5 bearing, 12 lateral, 7 lead
    of 24), so there is no release.
* **Water contacts 1 -> 0.** `D3A Val #1.1|.-4` no longer drowns inside the 4500 frames. Its
  row shows `done=669` against 627. Its probe input is 54 weaves.
* **Other dive-bomb rows** move by one or two ticks in the `#1.1` and `#3.1` flights.
  `#7.1|.-2` (47 slot writes and weaves) and `#7.1|.-4` (weaves only) now leave fly-over into
  turndown rather than goaway.
* **Every moved row is a Val with probe activity.** The `#5.1` flight had comparable activity and
  its rows did not move.
* **Ship-AI and AA rows** move through the changed aircraft positions. That is RNG-coupled
  (`docs/AA_TARGETING.md`), with no target-assignment claim made.

**P1b against P0 at 9000.**
* The probe makes 16873 calls with 3832 hits.
* **Releases 26 -> 29:** `#3.1|.-3` 1 -> 0 (the same weave chain), and `#7.1|.-2` 0 -> 2 and
  `#7.1|.-4` 0 -> 2. Those two are the pair that now reach turndown instead of goaway.
* The same eight Vals drown, at slightly different speeds.
* Gunnery deaths stay at 17. Hits move 186 -> 165, and the AA rows follow the aircraft paths.

**Hull offset on.** P0H has 11 releases and PHb has 10. The probe does **not** recover the hull
offset's losses. The sideways dive entries are not explained by the missing weave.
Prediction 6's alternative holds, and the weave is ruled out as their cause.

## 7. Decision

**The probe lands** (`kNearFieldProbeBound = true`) at the three dive-bomb sites. Every moved
dive-bomb row traces to that Val's own weaves, trims or goaway nudges. The net release change
is -1 at 4500 and +3 at 9000.
* The other fifteen sites stay contracts: their states are not in the host, or (the torpedo
  goaway) their out-slot mapping is not established.
* The fly-over's heading trim (`[ESP+1Ch]`) stays a contract.
* 009FD570's own obstacle cache stays a hole.
* The hull offset switch is untouched.
