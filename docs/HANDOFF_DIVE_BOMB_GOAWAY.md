# Handoff: the goaway climbs now, and the state's other half is an evasive turn

Packet `cc8_dive_goaway`, branch `agent/cc8-dive-goaway`, commit `b7be4aca1`. Successor of
`cc8-dive-flyover`. Read `docs/DIVE_BOMB_GOAWAY.md` whole first - it is this packet's evidence and
it is written to be read in order. This file is only what that document does not say.

## (a) Settled, and not to be re-derived

* **`009C4A40` is the `kGoAway` tick**, named `BSP_BotStateDiveBombGoAway_Tick` in Ghidra and the
  ledger, proved by dispatch: the vtable install at `009C74EE` is `[ESI+30Ch]` and `ESI = task+3F8h`
  on three independent agreements (`kGoAway 704h`, `kAimDive 734h`, `kAimGlide 754h`). Body
  `009C4A40`-`009C4E65`.
* **The climb is two curves and a max**, `009C4B32`-`009C4BD8`, both now confirmed by measurement to
  three significant figures, not just read from the listing. Curve A is the climb-back-to-cruise arm
  over the altitude **deficit**; curve B is the ground-avoidance arm over the raw altitude. The old
  comment calling curve A untraced and substituting B for it is retracted in the header, the ledger
  and the doc.
* **`009C7F00`'s ceiling and `009C4A40`'s ceiling are the same expression**,
  `min(ctl+398h, approach+ACh + approach+50h)`, built at `009C4ACF`-`009C4B05` and `009C7F23`.
* **The goaway does not complete on USN04 at 4800 frames, and the reason is the clock, not the
  model.** `#3.1` reaches 866.8 m against a 900 m gate with the command still positive. Do not
  "fix" this with a constant. At 9000 frames all six complete at 899.7-900.0 m and fly a **second
  attack run**: `transitions` 6 -> 10-15, a second `turndown` and `aimdive`, `releases` 1 -> **2**,
  `rounds_left` 1 -> **0**, ending in `done`. The chain closes with no constant touched.
* **`local\goaway_long.log`'s mission-level numbers are not comparable to the pair** and must not be
  quoted: `bomb_impacts=0`, `total_damage=3596.8`, `deaths=5`, `Lexington-class01` unsunk, and the
  arm starting ~2100 frames later. Two unchecked candidates: `--frames` changing the pre-mission
  budget, and my not passing `--instance-tag`/`--affinity-core` while another worker's runs
  overlapped. Only the state/tick/release counts from it are used.
* **The -5 degree nose-down flag** (`009C4A43`-`009C4A68`, `00CF885C`) is recovered and carried on
  `DiveBombGoAwayCommand::wrote_bank_heading`, deliberately **unconsumed**. See (c).

## (b) The one thing to do next in this state

**Bind the evasive turn: the `+24h`/`+28h`/`+2Ch` timers and the two arms at `009C4D9D`-`009C4E63`.**
This is the other half of the goaway and it is currently absent.

* `009C4A6D`-`009C4ACD`: `+24h` counts down by `dt` while `(+20h - 100) > approach+BCh`; `+28h`
  accumulates `dt`; when `approach+C4h < 1.0` and `+28h > +2Ch + 6.0` the tick forces `+24h = -1.0`.
* `009C4CF1`-`009C4D9A`: a negative `+24h` re-rolls `+2Ch` and `+24h` through two
  `BSP_Random_UniformFloatRange 00BD2F10` calls (`00CE3854`/`00CE6630` and `00CEB4B8`/`00CE5380`) and
  an `00419010` curve over the altitude, then sets `+28h = -(+2Ch * 00D7A280)`.
* `009C4DAD` splits: `+2Ch > +28h` takes the **bank** arm `009C4DAF`-`009C4DF6`
  (`cmd+2C4h = clamp(2 * +28h * +18h, 00CE3814, 00D05EA4)`, `cmd+2CCh = 1`); otherwise the
  **heading** arm `009C4E05`-`009C4E1D` (`CALL 009C47D0` fills `+1Ch`, then `cmd+2C0h = +1Ch`,
  `cmd+2CCh = 2`, then `0042E740`/`0099B630`/`009FABE0` for the speed side).
* `009C47D0` is unread and is the key: it is what decides which way the aircraft turns away.

Only once those exist should `wrote_bank_heading` be consumed. Gating the wings-level pair off
without them leaves the state with no lateral command at all, which is **further** from the image
than the unconditional wings-level the host writes today. I nearly shipped that; see (e).

## (c) Left deliberately undone, with the reason

* **Item 4 of the packet brief, release accuracy, is not started.** Nothing was measured and nothing
  is claimed about it. The material a successor wants is already in the logs: the `dive entry` census
  line carries `aim error 009C5C9B` and `closest=` per bomb (2-20 m at release against 0.07-3.84 m at
  closest approach, on `pullout_after.log`), and `009C7D71`'s predicted impact is on the `aimdive`
  line as `impact 009C7D71: tf= range= (live )`.
* **Arm B of the aimglide pull-out is still unbound**, but it is two slots from done rather than
  three. See `docs/DIVE_BOMB_GOAWAY.md` section 6: `[ESP+6Ch]` is proved to be the tick's `dt`
  argument, `[ESP+28h]` is the `+1Ch` re-arm timer, and `[ESP+24h]` is `length1 / length2` where
  **length 2 is the planar distance to the approach's `+D8h`/`+DCh`/`+E0h` point**. What remains:
  length 1's point comes from the out-vector of the indirect call at `009C51D3` (`LEA ECX,[ESP+50h]`
  at `009C51CC`) and that virtual is unidentified; and `[ESP+6Ch]` at `009C57F1` holds path-dependent
  scratch, not `dt`.
* **The `+278h`/`+27Ch`/`+2A8h`/`+2ACh` throttle and air-brake slots** the goaway writes on both paths
  are not modelled because this host keeps no such slots. Grep confirms: no `_278`/`_2a8` anywhere in
  `game_hosts_units.cpp`.

## (d) The runs, all in this worktree's `local\`

| log | binary | what it is |
| --- | --- | --- |
| `goaway_before.log` | census only | the strict before; behavioural columns identical to `pullout_after.log` |
| `goaway_after.log` | `b7be4aca1` | curve A bound; the strict after |
| `goaway_long.log` | `b7be4aca1` | 9000 mission frames, the outcome (ii) demonstration - **not** a paired measurement |

`goaway_before.log`'s behavioural columns match the predecessor's
`J:\PROG\battlestations-pacific-decompile-cc8-dive-flyover\local\pullout_after.log` exactly
(`total_damage 9820.7`, `deaths 8`, `bomb_impacts 18`), which is what makes the census provably
print-only and the pair valid.

## (e) Three traps this area set, two of which I walked into

* **The raw listing's `[ESP+N]` shifts inside a `PUSH`+`CALL` window.** In `009C5180` this made two
  slots look as if they had no writer at all and I wrote that down as the open question before the
  decompiler showed both assigned. Any frame walk here must account for the pushes, and the cheap
  cross-check is `ghidra decompile`, which names the locals consistently across the shift.
* **A census counter takes its meaning from where it is incremented.** `complete_ticks` sits in
  `dive_bomb_transition_inputs`, which runs on every tick of the **arm**, so it reads 1109-1283 for
  aircraft whose goaway never completed once. It is left that way so the before/after pair differs
  only by the change under test. Read it as "ticks of the whole arm".
* **The flag-0 side of a two-way split is not the empty side.** `009C4BEF`'s `JZ` looks like "skip
  the command writes", and it is not: it skips *these* writes and reaches different ones 700 bytes
  later. Follow the jump target to the `RET` before calling a branch command-free.

## (f) One number the integrator should carry forward

Binding the climb costs an aircraft. `deaths` 8 -> 9, `entity_impacts` 46 -> 60, `total_damage`
9820.7 -> 10188.4, with every dive-bomb state count and `bomb_impacts` unchanged; the extra death is
`D3A Val #3.1`, one of the six, shot down by `Northampton-class04` after climbing back into the
escorts' AA. The image climbs too, so this is faithfulness rather than regression - but the aircraft
the second attack run was meant for may not survive to fly it, and that is worth knowing before the
`009C86EE` re-attack edge is judged.
