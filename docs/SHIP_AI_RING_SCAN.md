# 009E76D0, the bearing-ring scan that picks an attackmove ship's heading

Addresses: 009E76D0, 009E6640, 009E6870, 009E6400, 009E5DA0, 009E5E90, 009E6A90

Packet `cc_ai_ring_scan`, worker `agent/cc-ai-ring-scan`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was READ-ONLY. Every
descriptive name below is a hypothesis, not a recovered symbol. Reconstruction in
`include/bsp/ship_ai_ring_scan.hpp` and `src/ship_ai_ring_scan.cpp`; call-site rows in
`reports/ship_ai_ring_scan.json`.

Vocabulary is the one `docs/SHIP_AI_APPROACH_UPDATE.md` fixes: `nested` is the ring object at
`sub+8h`, `slot i` is the `4Ch`-byte record at `nested + 4h + i*4Ch`, `tune` is `[brain+0AB0h]`,
`unit` is `[brain+0AA8h]`. That document covers the six routines around this one; this document
covers the scan itself and the four scorers it left as "contract unread".

## Answer to the packet question

**The ring.** `009E5530` builds sixty slots one sixtieth of a turn apart, so the bearings are six
degrees apart (`docs/SHIP_AI_ATTACKMOVE_SUBSTATES.md`). Each slot carries its bearing at `+8h` and
the matching unit XZ direction at `+0Ch`/`+14h`. The scan ranks the sixty by five floats at
`+2Ch..+3Ch` and hands the winner's bearing to `009E5E90`.

**The five weights and who fills them.**

| field | producer | what it measures | maximum |
| --- | --- | --- | --- |
| `+2Ch` | `009E7FC0` at `009E8292` (mode 0) or `009E6400` at `009E6464`/`009E6473` (mode 4) | mode 0: the ship class's own rating of the bearing, normalised across the frame. mode 4: a beam preference, zero head-on and astern | `tune+0h`; `1.0f` on the mode-4 arm |
| `+30h` | `009E6870` at `009E6975` | distance from the nearer edge of the standoff arc `009E6E80` chose | `tune+4h` |
| `+34h` | `009E74D0` at `009E764B` | how little the ship has to turn from its present heading | `tune+10h` |
| `+38h` | `009E74D0` at `009E76B7` | agreement with the evade bearing | `tune+14h`, times five on the evade path |
| `+3Ch` | `009E9190` at `009E96B9` | agreement with the nearby-traffic avoidance heading | `tune+8h` |

`009E6640` is the fifth scorer and does **not** write any of the five. It produces the accept/reject
score `009E76D0` ranks separately, and it owns the blocked byte at `+40h` and the two probe fields
at `+44h`/`+48h`.

**How the winner is chosen.** Every slot is probed by `009E6640`; the frame maximum is taken; when
that maximum is below `1.0` every score is divided by it, so the best slot always ends at exactly
`1.0`. A slot scoring strictly above `0.85` is accepted and only has its blocked byte cleared; a
slot at or below it keeps the byte and has `+2Ch`, `+38h` and `+3Ch` zeroed and
`+30h = -0.0f - tune+4h`, which is a rejection large enough to sink it in the sum. The winner is
the slot with the largest `+3Ch + +2Ch + +30h + +34h + +38h`, first maximum winning.

**What it passes to `009E5E90`.** The winner's bearing, unless `nested+1209h` is set and both
override bytes are clear, in which case the bearing is replaced outright by the heading toward the
attackmove goal, reversed by `pi` when the ship still has speed to burn. Either way the bearing and
the frame time go to `009E5E90`, which writes the commanded heading `nested+120Ch`; `009E6A90` then
turns that heading into the commanded throttle `nested+1210h`.

## `009E76D0`, the scan

`__thiscall(nested)(float seconds)`, `RET 4` at `009E7EDA`, body `009E76D0-009E7EDC`, **complete**.

```
nested+1214h = wrap_00605070(seconds * 0.2 + nested+1214h)      009E76DA..009E76F4 (00CE3D10)
if nested+1234h == 3:                                           009E76F9
    every slot+40h = 0, nothing is scored                       009E79B4..009E79C8
else:
    for each slot:                                              009E7722..009E778B
        score[i] = 009E6640(slot, seconds, nested+11DCh,
                            nested+11D4h, nested+11D5h, nested+11F0h)
        best = max(best, score[i])                               009E776C FCOMIP, JBE
    if best < 1.0:  score[i] /= best for all sixty               009E7795, 009E779E..009E7808
    for each slot:                                               009E7822..009E79AA
        if score[i] > 0.85:  slot+40h = 0                        009E7828 JA, 009E7852
        else: slot+2Ch = slot+38h = slot+3Ch = 0
              slot+30h = -0.0f - tune+4h                         009E782D..009E784B
winner = argmax(slot+3Ch + slot+2Ch + slot+30h + slot+34h + slot+38h)
                                                                 009E79CA..009E7C17
nested+11F8h = ring[winner].angle_08                             009E7C28
(the wobble at 009E7C3E..009E7C78 is dead, see below)
if nested+1209h and not nested+11D4h and not nested+11D5h:       009E7C72..009E7C92
    refresh the unit's world matrix when [unit+0C8h] is clear     009E7CA1..009E7D71
    h = pi/2 - atan2(goal.z - unit.z, goal.x - unit.x), +2*pi if negative
                                                                 009E7D7A..009E7DDD
    if (double)[unit+490h] - interp(0.2, 400, 1.2, 50,
            |wrap(nested+11ECh - h)|) > (double)nested+11E0h:
        h = wrap(h + pi)                                         009E7E68..009E7E9C
    nested+11F8h = h                                             009E7EA6
009E5E90(bearing, seconds)                                       009E7ECB
```

The accept threshold is the double `0.8500000238418579` at `00CF0B58` (the float `0.85f` widened);
reading only the first four bytes of that address gives `2.0f` and is wrong. The reject base is the
float `-0.0f` at `00D7A208`, so the reject value is `-tune+4h` with a preserved sign of zero when
`tune+4h` is zero.

**The unrolled bodies, now read step by step.** The accept/reject loop starts at `009E7822`, not at
`009E78CC` as the earlier packet's note said (`009E78CC` is its fourth step). Six steps, then
`ADD EDX,6` and `ADD ECX,1C8h` at `009E799E`; `ECX` starts at `nested+40h`, which is slot 0's `+3Ch`,
and each step's offsets are that base plus a multiple of `4Ch`. The winner scan is `009E79CA` for
slot 0, an eight-way body `009E79E5-009E7BC9` (`ADD EDX,8`, `ADD ECX,260h`, `CMP EDX,0x35`) covering
slots 1 to 56, and a one-at-a-time remainder loop `009E7BD4-009E7C17` covering 57 to 59. Every
comparison is `FCOMIP` then `JBE`, so the first slot holding the maximum wins and a NaN total never
displaces the incumbent.

**The wobble is dead twice over.** `009E7C3E..009E7C78` takes `sin(nested+1214h)`, multiplies it by
the double at `00D7A258` (which is `0.0`), wraps the sum onto the bearing through `00438AA0` and
pops the result with `FSTP ST0` at `009E7C78` without storing it. The phase advance at `009E76DA` is
reproduced because it is stored; the wobble itself is not projected.

## `009E6640`, the obstacle probe

`__thiscall(slot)(float seconds, float bearing, char override_a, char override_b, float range)`,
`RET 14h` at `009E684F` and `009E6866`, body `009E6640-009E6868`, **complete**, `ST0` result.
`009E76D0` is its only caller.

```
allowed = true
if override_a or override_b:                                     009E6648..009E6658
    d = wrap(slot.angle_08 - bearing)                            009E666B
    if (override_a and d < 0) or (override_b and d > 0):         009E6681, 009E668D
        allowed = false ; slot+44h = 0.0f                        009E6692, 009E6697
slot+48h -= seconds                                              009E669C..009E66AB
if slot+48h < 0:                                                 009E66B4 JBE
    slot+44h = range ; slot+40h = 0 ; slot+48h = 2.0f            009E66C3..009E66D4 (00CE3958)
    space = unit->vtable[218h]()                                 009E66EB
    if [unit+0C8h] == 0: 00414DB0(unit)                          009E66F8, 009E6705
    start = 00417B10(space, (unit.x, unit.z), 3.0f, 1)           009E673E (00CE3854)
    len = range * interp(20 deg, 0, 120 deg, 1,
                         |wrap(bearing - slot.angle_08)|)        009E67AA, 009E67AF
    end = start + (slot.dir_x_0c, slot.dir_z_14) * len           009E67D6..009E6804
    if 0041B4E0(space, start, end, &hit):                        009E6808
        slot+40h = 1 ; slot+44h = |start - hit|                  009E681F, 009E6833
return (allowed and slot+40h == 0) ? 1.0f : slot+44h / range     009E683B..009E6866
```

The two override bytes are a turn-side lock: `nested+11D4h` refuses every slot on the negative side
of the reference bearing and `nested+11D5h` every slot on the positive side, and a refused slot has
its clear distance forced to zero so it scores zero.

A verdict lasts two seconds of game time; the timer at `+48h` is the only thing that makes the probe
re-cast. `009E5530` and `009F30F0` seed it with a draw in `[0, 2)` so the sixty probes are spread
across the window instead of all firing on the same frame.

The probe length is the counterintuitive part and is reproduced as written: the interpolation is
`x0 = 20 deg` with `y0 = 0` and `x1 = 120 deg` with `y1 = 1`, so a slot pointing at the reference
bearing gets a zero-length probe and is never blocked, and only slots more than 120 degrees off it
get the full range. `bsp_game.exe` does not reach this path (see Coverage), so no run log can
confirm or contradict the reading.

## `009E6870`, the standoff-arc score

`__thiscall(slot)(const float block[4])`, `RET 4` at `009E697D`, body `009E6870-009E697F`,
**complete**. `009E6E80`'s tail is its only caller (`009E74B7`), sixty times, with the block
`{side, span, nested+11DCh, tune+4h}` laid out at `009E7483..009E74A0`.

```
edge_add = wrap(block[2] + block[0])                            009E688A
edge_sub = wrap(block[2] - block[0])                            009E68A2
g = min(|wrap(slot.angle_08 - edge_add)|,
        |wrap(slot.angle_08 - edge_sub)|)                       009E68BC, 009E68E3, 009E6902
cost = (block[1] >= g) ? (g / block[1]) * (0.2 * block[1])      009E6931 FCOMIP, JC
                       : g - (block[1] - 0.2 * block[1])
slot+30h = interp(0, block[3], pi, 0, cost)                     009E6970, 009E6975
```

Both arms agree at `g == block[1]`: inside the span the cost rises at a fifth of `g`, outside it at
one for one. So the score is `tune+4h` on either edge of the arc and decays slowly inside the span
and fast outside it, reaching zero at `pi` of cost. `0.2` is the pooled double at `00CE3D10`, the
same eight bytes `009E76DA` uses for the phase rate; the image float-stores `0.2 * span` at
`009E692D` before the comparison, which is why the multiply is written out rather than folded.

## `009E6400`, the beam score of the mode-4 arm

`__thiscall(slot)(float bearing)`, `RET 4` at `009E646A` and `009E6479`, body `009E6400-009E647B`,
**complete**. `009E7FC0`'s mode-4 arm is its only caller (`009E8080`), and only while
`nested+11E0h - nested+11E4h < 300.0`.

```
a = |wrap(bearing - slot.angle_08)|                             009E6414, 009E6421
a = (pi/2 <= a) ? (pi - a) / 1.2 : a / 1.2                      009E6434 FCOMIP, JBE
slot+2Ch = (a > 1.0f) ? 1.0f : a                                009E6456, 009E6464/6473
```

`pi/2` is the double at `00CE3830`, `pi` the double at `00CE3D28`, `1.2` the double at `00CEC160`.
The result is a triangular ridge on `|a|`: zero at head-on and astern, clipped at `1.0` for every
bearing between `1.2` and `pi - 1.2` radians off the reference. Mode 4 is the ordinary standoff
approach, so the arm prefers a beam aspect on the reference bearing.

## `009E5DA0`, the ship-class rating

`__thiscall(slot)(a 44h-byte block by value)`, `RET 44h` at `009E5DF9`, body `009E5DA0-009E5DFB`,
**complete**. `009E7FC0`'s mode-0 arm is its only caller (`009E81A7`); the callee's `RET 44h` is what
fixes the argument count at seventeen dwords (checklist rule 7), and the caller supplies them with a
`REP MOVSD` of `0x11` dwords straight out of `nested+127Ch` (`009E8192..009E81A2`).

The routine is an adapter, not a scorer. It replaces word 5 of its own copy of the block with
`wrap(word5 - slot.angle_08)` (`009E5DB4`, `009E5DBB`) and hands a pointer to the block to
`0095EB40` with `ECX = [slot+0h]`, the unit. The returned float is `slot+18h`; words 12, 11, 10 and
13 are copied into `slot+1Ch`, `+20h`, `+24h` and `+28h` (`009E5DCC..009E5DED`) and nothing in this
packet or the last reads those four again. `009E7FC0` then turns `slot+18h` into `slot+2Ch` as
`slot+18h / max(slot+18h) * tune+0h`.

Word 5 is `nested+1290h`, which `009E7FC0` seeds from `nested+11DCh` at `009E8153`; words 6 and 7
are `nested+1294h = 20.0f` and `nested+1298h = 60.0f` (`009E814B`, `009E8161`). The producer of the
other fourteen dwords is unknown, and the copy is a raw `MOVSD`, so the header carries the block as
seventeen dwords rather than seventeen floats (checklist rule 4).

`0095EB40` is therefore the largest single gap left in the ring ranking: it fills the weight with the
largest scale factor. It is in the vehicle-class cluster at `00951F40` and has two other callers,
`0095F080` and `009E6240`.

## `009E5E90` and `009E6A90`, verified whole, not re-reconstructed

Both were read instruction by instruction for this packet and both agree with
`docs/SHIP_AI_APPROACH_UPDATE.md`; `ship_ai_approach_commit_bearing_009e5e90` and
`ship_ai_approach_limit_throttle_009e6a90` already project them, so this packet adds no second
projection. Three details worth pinning:

- The slot index at `009E5E91..009E5ECC` is `(int)((bearing + pi/60 [+2*pi if negative]) * 60 /
  (2*pi))` with the `60` at `00CE3D68` and the `2*pi` at `00CE3828`. It is not range-checked.
- The two ring walks at `009E5F6F` and `009E5F9B` both start **at** the committed slot `nested+11E8h`
  and stop when the index reaches the target, so the committed slot is counted and the target is
  not. `BL` is seeded from the committed slot's own blocked byte at `009E5F6C` and is never cleared
  between the walks, so a blocked slot found in the backward walk still charges 60 in the forward
  one.
- The heading band is `(-0.1, 150 degrees)`: `-0.1` is the double at `00CE3928` and `150 degrees` is
  the float `2.6179938f` at `00D1FED0`. Outside it the commanded heading is `nested+11ECh` stepped
  by 150 degrees toward the chosen side (`009E600F` subtracting, `009E6035` adding).

For `009E6A90`, the sentinel arm is `009E6AC8..009E6B17`: the throttle is replaced whenever it is
above the double `1000.0` at `00CE47A0`, and `009F1BC0` writes `9999.0f` into it at `009F1BF7` on
every frame, so the replacement runs every frame the limiter runs. The replacement is
`interp(pi/6, 1.0, 70 deg, 0.5, |wrap(unit->vtable[50h]() - nested+120Ch)|)` from `00CEC724`,
`00CE3988` and `00CE3800`. The final clamp is at `009E6BC7..009E6E78` and stores the unchanged
command on an unordered comparison; see Corrections for the exact mechanism.

## Every reader and writer of `nested+120Ch` and `nested+1210h`

Found by a byte-pattern search for the four displacements `0C 12 00 00`, `10 12 00 00`,
`14 12 00 00` and `18 12 00 00` over the whole image, then attributing each hit to its containing
Ghidra function (checklist rule 2). `sub = nested - 8`, so `sub+1214h` is `nested+120Ch` and
`sub+1218h` is `nested+1210h`.

| field | site | function | what |
| --- | --- | --- | --- |
| `+120Ch` | `009E5F4C`, `009E602A`, `009E6050` | `009E5E90` | the three stores of the commanded heading |
| `+120Ch` | `009F31E6` | `009F30F0` | `MOVUPS` of zero over `+120Ch..+121Bh`, the ring reset |
| `+120Ch` | `009E6A98` | `009E6A90` | read as the first term of the heading error |
| `+120Ch` | `009F3314` | `009F3240` | read into a stack slot, stored to `brain+1E0h` at `009F335C` |
| `+120Ch` | `009E5E70` | none (see below) | a one-instruction accessor |
| `+1210h` | `009F1BF7` | `009F1BC0` | the `9999.0f` sentinel, written every frame |
| `+1210h` | `009E6AD4`, `009E6B17`, `009E6BD4`, `009E6BF6`, `009E6E42`, `009E6E59`, `009E6E6C` | `009E6A90` | the sentinel test, the seed, the reverse arm and the three clamp stores |
| `+1210h` | `009F31E6` | `009F30F0` | zeroed by the same `MOVUPS` |
| `+1210h` | `009E5613` | `009E5530` | the construction zero |
| `+1210h` | `009F339E` | `009F3240` | read into a stack slot, clamped into `brain+1D8h` at `009F3635` |
| `+1210h` | `009E5E80` | none (see below) | a one-instruction accessor |

There are no others. The only near miss is `009F2C52` inside `009F1BC0`, which reads
`nested+1218h`, a different field; that is recorded under Corrections.

## Coverage

| routine | body | coverage |
| --- | --- | --- |
| `009E76D0` | `009E76D0-009E7EDC` | complete |
| `009E6640` | `009E6640-009E6868` | complete |
| `009E6870` | `009E6870-009E697F` | complete |
| `009E6400` | `009E6400-009E647B` | complete |
| `009E5DA0` | `009E5DA0-009E5DFB` | complete; the rating itself is `0095EB40`, unread |
| `009E5E90` | `009E5E90-009E605A` | complete, verified only; projected by the previous packet |
| `009E6A90` | `009E6A90-009E6E78` | complete, verified only; projected by the previous packet |

Not projected: the wobble at `009E7C3E..009E7C78`, because the image discards its result.

Callee bodies left unread and carried as host methods: `unit->vtable[218h]` (`009E66EB`), `00417B10`
(`009E673E`), `0041B4E0` (`009E6808`) and `0095EB40` (`009E5DC4`). Each is named by its address in
the header with `contract unread` on it (checklist rule 1).

No run-time evidence: `bsp_game.exe` records `009E76D0`, `009E5E90` and `009E6A90` rather than
running them (`src/game_hosts_ship_ai.cpp`, the `select_slot_009e76d0` and `limit_throttle_009e6a90`
bindings), so the executable does not reach this path and checklist rule 6 does not bind. Wiring it
is the follow-up `ship_ai_ring_scan_runtime`, and it needs a probe host that can answer `0041B4E0`.

Status of the reconstruction: **build-tested**, not fixture-tested against the native routines and
not ABI-compatible. `scripts/build.ps1` (Win32 MSVC `/W4 /WX`) and the `reconstructed_math` test
pass; one new case pins the sentinel path end to end.

## Corrections

1. **`ShipAiApproachSlotScore::penalty_30` had no producer for the normal case.**
   Was: `// +30h, 009E784B: -tune[4] on a rejected slot`.
   Is: `009E6870` writes it at `009E6975` as the standoff-arc score; the reject value at `009E784B`
   is the exception, not the rule. Evidence: `009E6870`'s only store is `slot+30h`, and its call site
   `009E74B7` is inside `009E6E80`'s per-slot tail.

2. **`ShipAiAttackMoveRingSlot::reset_44` and `::jitter_48` were construction values with no reader.**
   Is: `+44h` is the clear distance along the slot's direction and `+48h` is the seconds left before
   the next probe. `009E6640` reads and writes both every frame (`009E669C`, `009E66C3`, `009E66D4`,
   `009E6838`, `009E6852`). The names should follow when the substates header's owner next touches it;
   this packet does not hold a lease on that file.

3. **"Nothing sets `slot+40h` to non-zero; `009E6640` or `009E6870` must."**
   Is: `009E6640` sets it at `009E681F`, on the frame its probe hits something. `009E6870` never
   touches the byte.

4. **`009E76D0` was recorded as partial, and one of the two ranges was wrong.**
   Was: `the 6-way unrolled accept/reject body 009E78CC-009E79AA and the 8-way unrolled winner scan
   009E79E5-009E7B7E were read in their first and last step only`.
   Is: complete. The accept/reject body is `009E7822-009E79AA` (`009E78CC` is its fourth step), and
   the winner scan is `009E79CA-009E7C17`, of which `009E79E5-009E7BC9` is the eight-way body and
   `009E7BD4-009E7C17` the remainder loop. The ledger record for `009E76D0` is updated with
   `--append-evidence`; the old text is preserved there and quoted here.

5. **`ShipAiApproachState::cleared_1218` was marked "no reader".**
   Is: `009F2C52`, inside `009F1BC0`, loads `nested+1218h` and pushes it into the call at `009F2C5D`.
   `EBP` is `ECX` from `009F1BE5` and is written nowhere else in that body (the whole listing was
   filtered for `EBP`, checklist rule 8), so the base is the nested object and the field is
   `nested+1218h`, not the commanded throttle at `nested+1210h`.

6. **Clarification, not a behaviour change: the `009E6A90` clamp on a NaN.**
   Was: `both are JBE, so a NaN command falls through both`.
   Is: an unordered `FCOMIP` sets `ZF=PF=CF`, so both `JBE`s are taken, `009E6BE8` to `009E6E4D` and
   `009E6E4D` to `009E6E66`, and `009E6E66` stores the copy of the command that `009E6BDA` made
   before either test. The stated outcome, that a NaN command is stored unchanged, is right.

## Follow-up packets

- `ship_ai_ring_scan_runtime` - wire `bsp_game.exe` to run the scan instead of recording it.
  `src/game_hosts_ship_ai.cpp` declines it because "the four unread scorers" fill the weights; three
  of the four are now read and the fourth is a thin adapter. The binding needs a `ShipAiRingScanHost`
  whose `probe_hit_0041b4e0` can answer, which is the next item. Not done here: that file belongs to
  another packet.
- `ship_ai_probe_space` - `unit->vtable[218h]` (`009E66EB`), `00417B10` (`009E673E`) and `0041B4E0`
  (`009E6808`). Without them the probe always reports clear water, every slot scores `1.0` and the
  accept test degenerates.
- `ship_ai_class_bearing_rating` - `0095EB40` (`009E5DC4`) and the producer of the seventeen dwords
  at `nested+127Ch..+12BFh`. It fills the weight with the largest scale factor.
- `ship_ai_approach_accessor_gap` - define the four accessors in `009E5E30-009E5E8F` that Ghidra has
  no functions for: `009E5E50` (`LEA EAX,[ECX+1228h]`, the approach point), `009E5E60`
  (`FLD [ECX+11E0h]`, the goal range) and the two in the table below. The first two are outside this
  packet's lease and are named here only so the gap is contiguous in the record.

## no_ghidra_function

| start | end (inclusive) | body | evidence for each boundary |
| --- | --- | --- | --- |
| `009E5E70` | `009E5E76` | `FLD float ptr [ECX + 0x120c]; RET` | start: `009E5E67-009E5E6F` are `INT3` padding after the `RET` at `009E5E66` that ends the accessor before it, so `009E5E70` is a first instruction. end: the `RET` at `009E5E76` is followed by `INT3` from `009E5E77`. `bsp.py ghidra proto 009e5e70` answers `body ?`. |
| `009E5E80` | `009E5E86` | `FLD float ptr [ECX + 0x1210]; RET` | start: `009E5E77-009E5E7F` are `INT3` padding after the `RET` at `009E5E76`. end: the `RET` at `009E5E86` is followed by `INT3` to `009E5E8F`, and `009E5E90` (`BSP_ShipAi_ApproachCommitBearing`) begins there. `bsp.py ghidra proto 009e5e80` answers `body ?`. |

Both are `__thiscall(nested)()` with no `RET` immediate and an `ST0` result: the commanded-heading
and commanded-throttle accessors. They were read from the disk bytes with `bsp.py disasm-raw`, which
is why the table carries the boundary evidence rather than a Ghidra body range.

## Uncertainties

- The probe-length ramp gives a slot aligned with the reference bearing a zero-length probe. Read as
  written from `009E678B..009E67AA`; no run-time evidence was available to test it.
- `00CE3D10` is one pooled double used as the phase rate at `009E76DA` and the arc slope at
  `009E692B`. The header carries one constant for both; whether the source had one or two is not
  recoverable.
- `009E6870`'s `side` and `span` come from `009E6E80`'s tail, whose two curve arms are projected as
  host calls, so the arc geometry is only as good as those.
- Only five of the seventeen dwords `009E5DA0` forwards are known to be floats (words 5, 10, 11, 12
  and 13). The rest are moved, never typed.
- `nested+11E8h`, the committed slot `009E5E90`'s ring walk starts from, is still written by no
  routine any packet has read; `009E5530` zeroes it at `009E561F`.
