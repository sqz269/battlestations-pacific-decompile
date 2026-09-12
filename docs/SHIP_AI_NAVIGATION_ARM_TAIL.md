# The tail of the ship AI's navigation arm: how a Navigate ship gets a throttle

Addresses: 009EEAAB 009EECDB 009EED62 009EEF00 009EF007 009EF051 009EF0A8 009EF206 009ED6B0 009D5240 004192E0 009DBBC0 009E4330 009DA590

`009ED6B0 BSP_ShipAi_ControlsStep` is `__thiscall(blk)(float seconds)`, `RET 4`, body
`009ED6B0-009EF228`, called at `009F5209`. Its navigation arm ends in the range this packet
read, `009EEAAB..009EF226`. The output block `009EE671..009EEAA2` has already written the
heading target `blk+324h`, the distance to the path point `blk+32Ch`, the remaining path
length `blk+330h`, the applied turn lead `blk+328h` and the look-ahead radius `blk+340h`
(`docs/SHIP_AI_NAVIGATION_ARM.md`). The tail turns those into the two values `009F3F80` then
consumes: the throttle ceiling `blk+344h` and the ahead/astern latch `blk+35Ch`. It also
publishes the two arrival bytes `blk+2FDh` and `blk+2FEh`.

It does **not** write `blk+1D0h` or `blk+1D4h`. There is no store to either displacement
anywhere in the range. The desired throttle and rudder stay whatever the setters `009DBF90`
and `009DFFB0` left; `009F3F80` builds the ring command from `blk+35Ch`, `blk+344h` and the
heading target instead (`docs/SHIP_AI_THROTTLE_TO_RING.md`).

Every name below is a hypothesis, not a recovered symbol.

## Coverage

| routine | range | coverage |
| --- | --- | --- |
| `ship_ai_navigation_arm_tail_009eeaab` | `009EEAAB-009EF226` | complete |
| `ship_ai_traffic_clearance_009eec15` | `009EEBE0-009EEC3D` | complete |
| `ship_ai_traffic_setback_step_009eed62` | `009EED62-009EEEDC` | complete |
| `ship_ai_approach_ceiling_009eecdb` | `009EECDB-009EED5D` | complete |
| `ship_ai_astern_choice_009ef0a8` | `009EF112-009EF19A` | complete |
| `ship_ai_set_direction_latch_009d5240` | `009D5240-009D5293` | complete |

Nothing in the packet range is left unread. What stays outside it: the bodies behind the
three host boundaries (`009DE5B0`, `vtable[50h]`, `vtable[5Ch]`), the producer `FUN_009E4330`
of four of the inputs, and `009EDA28..009EE57B`, which is now the only unread range of
`009ED6B0`.

## 1. The traffic setback, `009EEAAB..009EEEEC`

The range opens with a local set to zero on every path in (`009EEAA8 XORPS XMM0,XMM0` on the
fall-through, and `XMM0` is already zero at `009EE91F` on the `009EEA0E JBE` path; no branch
enters between `009EE91F` and `009EE964` from outside, so the local is zero either way). Call
it the **setback**: how far short of the goal the ship has to stop because something is
parked in the way.

Four things gate the walk, and all four must hold (`009EEAB7`, `009EEAC7`, `009EEAD5`,
`009EEAF8`):

```
[[blk+3FCh]+73Ch]+21h != 0        ; the navigatorParams "keep clear" byte
record+20h != 0                   ; the path continues past this point
blk+604h > 0                      ; the block has neighbours
blk+32Ch < 2000.0                 ; 00CF0DD8, the path point is close enough to care
```

Then:

```
allowance = blk+32Ch * |blk+328h|                       ; 009EEAFE..009EEB26
point     = (blk+1DCh, blk+1E0h)                        ; the goal
dir       = normalize(point - (blk+184h, blk+188h))     ; 009EEB63, 004192E0
restart:                                                ; 009EEB80
  n = [[00E188A8]+19CCh]+60h                            ; 009EEB8B, re-read every pass
  for i in 0..n-1:
      e = 009DBBC0(list, i)                             ; 009EEBB0
      if (!e || !e->vtable[5Ch](6) || e == [blk+3FCh]) continue
      R = max(60.0f, ([e+9C8h] + [[blk+3FCh]+9C8h]) * 0.5)
          + max(0.0f, allowance - setback)              ; 009EEBE0..009EEC37
      p = 00427EB0(e)                                   ; 009EEC41, x and z
      if ((point - p).length2 < R*R) goto blocked       ; 009EEC8D..009EECBF
  goto done                                             ; 009EECD8
blocked:                                                ; 009EED62
  t     = (p - point) . dir
  setback -= t
  proj   = point + t*dir
  perp   = length2d(proj - p)                           ; 009EEE00, 00414C60
  if (perp < R) {                                       ; 009EEE13 JBE
      back = sqrt(R*R - perp*perp)                      ; 009EEE25, 00BF7030
      setback += back
      proj    -= back*dir
  }
  point   = proj - 20.0*dir                             ; 00CE3D88
  setback += 20.0
  if (blk+330h <= setback) goto done                    ; 009EEEE6 JC taken to restart
  goto restart
done:                                                   ; 009EECDB
```

`allowance` is the arc the applied turn lead sweeps at the current distance: it widens the
clearance circle of the first neighbours and tapers away as the walk moves back, because
`max(0, allowance - setback)` falls to zero. The walk runs **backwards** from the goal toward
the ship along `dir`, so the setback is a distance measured from the goal, which is exactly
how `009EEF2E` and `009EEF68` use it.

The walk has no iteration bound of its own. It leaves on a clear pass or when the setback has
eaten the whole remaining path. The reconstruction adds a 1024-step guard that is **not** in
the image and reports the step count in `ShipAiTrafficSetback::steps`.

## 2. The approach throttle ceiling, `009EECDB..009EED5D`

```
if ([ESP+43h]) {                                        ; 009EECDB
    v     = blk+3C4h
    stop  = float((v / [[[blk+3FCh]+538h]+508h]) * v * 0.5)   ; 009EED1A..009EED3F
    ratio = float(blk+330h / stop)                      ; 009EED43
    blk+344h = clamp(ratio, 0.25f, 1.0f)                ; 009EED4B, 00CE3868 / 00D7A24C
} else {
    blk+344h = 1.0f                                     ; 009EEF00, 00D7A24C
}
```

`[ESP+43h]` is set at `009EE6E5` / `009EE6EC`, just before this packet's range:
`record+20h != 0 && blk+1E4h != 0`, so the taper only applies when the goal really is a
destination. The denominator is the stopping distance from the block's reference speed, the
same `(v/decel) * v * 0.5` shape `009ED8EC` builds for `blk+32Ch` without its hull term. The
store at `009EEF0A` happens on both paths.

`blk+344h` is a **ceiling**, not a throttle: `009F4439` and `009F4462` limit the AI's throttle
against it (`docs/SHIP_AI_THROTTLE_TO_RING.md`). The 0.25 floor is why a ship still creeps in
rather than stopping dead a ship-length out.

## 3. Start, stop and arrive, `009EEF00..009EF04C`

```
release = false
if (blk+35Ch == 0) {                                            ; 009EEF04
    if (record+21h && blk+3D8h + setback < blk+330h) {           ; 009EEF14..009EEF38
        release = true ; result = 1                              ; 009EEF3E, 009EF002
    }
} else if (!blk+3A5h) {                                          ; 009EEF45
    stop = !record+21h                                           ; 009EEF52
        || blk+330h < blk+3D4h + setback                         ; 009EEF5C..009EEF70
        || (blk+2FDh && record+20h                               ; 009EEF72, 009EEF7F
            && blk+340h < min(blk+3CCh*0.25, blk+1F0h*0.1))      ; 009EEF8D..009EEFD1
    if (stop) {                                                  ; 009EEFD3
        009D5240(blk, 0, false) ; result = 1                      ; 009EEFDC..009EF002
    }
}
if (blk+35Ch == 0 && !release) {                                 ; 009EF00D, 009EF011
    if (record+21h && !blk+2FCh) {                               ; 009EF015, 009EF022
        blk+2FEh = 1                                             ; 009EF034
        if ([blk+2F4h]+1Ch > 3) blk+2FDh = 1                     ; 009EF03B, 009EF045
    }
    goto 009EF206
}
```

`blk+3D4h` and `blk+3D8h` are the hysteresis pair, and `FUN_009E4330` produces both:
`blk+3D4h = max(1.5f * class MaxSpeed, 0.4f * [unit+9C8h])` (`009E4537`) and
`blk+3D8h = 1.5f * blk+3D4h` (`009E453F`). So a ship stops inside the smaller radius and does
not start again until the goal is half again as far away. The setback adds to both, which is
what makes a ship hold station behind traffic rather than nose into it.

The third stop reason is the interesting one: a ship that is already parked
(`blk+2FDh`) and still has path ahead of it stops again when its look-ahead radius has
collapsed under the smaller of a tenth of the longest path seen (`blk+1F0h`) and a quarter of
the class turn distance (`blk+3CCh`, from `0082E960(class, 0.9f)` at `009E4568`).

`blk+2FEh` matters to the executable more than anything else here. `009DA590
BSP_ShipAi_GoalReachedTest` returns false at once while it is clear, and `009E5821` in the
`movetopos` step calls it through `state->vtable[2Ch]` to decide whether the order is finished
(`docs/SHIP_AI_PATH_PLANNER.md`). `009ED779` clears it at the top of every tick, so
`009EF034` is the only thing that can ever set it for a Navigate ship, and it only fires with
the ship stopped, not releasing the stop, a valid path point and `blk+2FCh` clear.

## 4. The ahead/astern latch, `009EF051..009EF1FE`

```
if (blk+1CCh != 0) {                                             ; 009EF051
    want = (blk+1CCh == 1) ? 1 : 2                               ; 009EF05B..009EF066
    009D5240(blk, want, false)                                   ; 009EF068..009EF0A3
    goto 009EF206                                                 ; no result byte on this arm
}
if (blk+3A5h && !blk+3A6h) goto 009EF206                         ; 009EF0A8..009EF0B8
if (!(0 > blk+360h))       goto 009EF206                         ; 009EF0BE
err  = |wrap(blk+324h - unit->vtable[50h]())|                    ; 009EF0D6..009EF109
thr  = max([[blk+3FCh]+9C8h] * 3.0, 2 * 0082E850(class))         ; 009EF112..009EF13B
if (blk+35Ch == 2) thr += 80.0                                   ; 009EF14A..009EF159, 00CF1440
want = (thr > blk+330h && err > rad(blk+35Ch == 2 ? 120 : 130))  ; 009EF167..009EF19A
       ? 2 : 1
if (want != blk+35Ch) { result = 1 ; 009D5240(blk, want, false) } ; 009EF19C..009EF1FE
009DE5B0(blk)(seconds)                                            ; 009EF213
return result                                                     ; 009EF218
```

So a ship going ahead reverses only when the goal is inside `max(3 * hull radius, 2 * turn
radius)` and it is pointing more than 130 degrees away from it. Once astern, both halves of
that test get easier: the distance threshold gains 80 units and the angle drops to 120
degrees, so it takes a clearly better situation to go ahead again than it took to back down.
That double hysteresis is why an AI ship reverses into a berth rather than oscillating at the
threshold.

The return value is `AL` at `009EF218`, and `009F520E` zeroes `brain+0B14h` when it is set. It
means a direction change was **wanted**, not applied: `009EF1AB` and `009EF1CD` write the byte
before the `blk+360h` test that can still refuse the write. The `blk+1CCh` arm never writes
the byte at all.

## 5. `009D5240`, the routine all four latch writes inline

`__thiscall(blk)(int direction, char force)`, `RET 8`, body `009D5240-009D5293`, complete,
read from the disk bytes. No Ghidra function starts there and no decoded call reaches it.

```
if (direction == blk+35Ch) return                          ; 009D5244
if (!(0 > blk+360h) && !force) return                      ; 009D524F COMISS/JA, 009D5258
blk+35Ch = direction                                       ; 009D5269
blk+374h = -1.0f                                           ; 009D526F, 00D7A260
blk+384h = 0.0f                                            ; 009D5277
blk+360h = (direction == 0) ? 0.0f : 1.0f                  ; 009D527F..009D5289, 00D7A24C
```

`009EEFE4`, `009EF087`, `009EF1B2` and `009EF1D4` reproduce this exactly, all with
`force = false`. The asymmetric timer reload is the point: a **stop** reloads the cooldown
with zero and can be undone on the very next tick, while a gear change reloads it with 1.0
and locks the latch for a second of block time.

`blk+360h` is counted down at `009ED7B4..009ED7D2`, at the top of every tick and only while it
is still non-negative (`009ED7C5 JC` skips the subtraction once it has gone under), so
`0 > blk+360h` is a cooldown that has expired, not a flag.

## Who else writes `blk+35Ch`

The `.text` section of the executable on disk was scanned for the displacement `5C 03 00 00`
behind a `mod=10` modrm, each hit classified by opcode, and each write-shaped site's
containing function checked. Every `83 BE/BF 5C 03 00 00` hit is `CMP /7`, a read.

On the ship AI control block the writers are `009D5269` inside `009D5240`, the three in the
direct-control arm (`009ED849` ahead, `009ED861` stopped, `009ED89F` astern), the two in the
station-keeping arm (`009EDB49`, `009EDB65`) and the four in this range. Nothing else.

The ten remaining write sites in the image are other structures that happen to share the
displacement: `007F2CE7`, `007F4652`/`007F46E1`/`007F4783`, `008761F7`, `0087B6FA`,
`0087BF16`, `009C7594` (a `00D20CC8` vtable pointer), `00A3EEE6`, `00A3F4C7`, `00A40079`,
`00ACC010`. None of them is a ship AI control block.

## How the executable composes it

`009ED6B0`'s navigation arm, per tick, on `blk` for a ship in Navigate:

| step | native | what |
| --- | --- | --- |
| 1 | `009EE5C2` `009ED3E0` | refresh the path plan from `blk+1DCh` |
| 2 | `009EE5F4` `009E3C00` | the path point record, `docs/SHIP_AI_GOAL_VECTOR.md` |
| 3 | `009EE671..009EEAA2` | the output block: `blk+324h`, `blk+32Ch`, `blk+330h`, `blk+328h`, `blk+340h` |
| 4 | `009EEAAB` | `ship_ai_navigation_arm_tail_009eeaab`, this packet |
| 5 | `009EF213` `009DE5B0` | the unconditional last step of every arm, contract unread |

Inside step 4 the host methods run in this order, one per native call site:

| order | site | native | method |
| --- | --- | --- | --- |
| 1 | `009EEB63` | `004192E0` | `normalize_004192e0_009eeb63` |
| 2 | `009EEB8B` | field read | `neighbour_list_count_009eeb8b` |
| 3 | `009EEBB0` | `009DBBC0` | `list_element_009dbbc0` |
| 4 | `009EEBC8` | `vtable[5Ch](6)` | `entity_is_kind_009eebc8` |
| 5 | `009EEBD2` | field read | `own_unit_009eebd2` |
| 6 | `009EEBE0`, `009EEBEE` | field read | `entity_hull_radius_009eebe0` |
| 7 | `009EEC41` | `00427EB0` | `entity_position_00427eb0_009eec41` |
| 8 | `009EF0D6` | `vtable[50h]` | `unit_heading_vtable_0050_009ef0d6` |
| 9 | `009EF112` | `0082E850` | `ship_class_turn_radius_0082e850_009ef112` |
| 10 | `009EF213` | `009DE5B0` | `after_arm_009de5b0` |

The pure helpers `00415510`, `00415550`, `00415620`, `00414C60`, `00438B10` and `00BF7030` are
already reconstructed and are called directly rather than through the host.
`reports/ship_ai_navigation_arm_tail.json` carries the `address` / `native` row for every one
of those sites.

The inputs the host has to supply that this range only reads: `blk+3C4h`, `blk+3CCh`,
`blk+3D4h`, `blk+3D8h`, `blk+604h` (all from `FUN_009E4330`), `[[blk+3FCh]+538h]+508h`,
`[blk+3FCh]+9C8h`, `[[blk+3FCh]+73Ch]+21h` and `[blk+2F4h]+1Ch`.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/GAME_EXECUTABLE.md` milestone 2p follow-up 1: the range "can latch `blk+35Ch` and form a desired throttle" | It latches `blk+35Ch` and writes the throttle **ceiling** `blk+344h`. It never writes `blk+1D0h` or `blk+1D4h`; `009F3F80` forms the ring command from the latch, the ceiling and the heading target | no store to `+1D0h` or `+1D4h` exists in `009EEAAB..009EF226`; the only other float store into `blk` is `009EEF0A MOVSS [ESI+344h],XMM1` |
| not previously recorded: who sets `blk+2FEh`, the byte `009DA590` requires before a `movetopos` can finish | `009EF034` is its only writer after the per-tick clear at `009ED779`, and it needs the ship stopped, not releasing the stop, `record+21h` set and `blk+2FCh` clear | the PE scan for `+2FEh` inside `00996120-009F6060` finds writers only at `009DA575`, `009DA5F6`, `009DE0EF`, `009ED779` and `009EF034` |
| the four latch writes look like four pieces of code | all four are inlined copies of `009D5240`, a routine with no Ghidra function and no decoded caller, reproduced instruction for instruction | `009D5240-009D5293` against `009EEFD3..009EF002`, `009EF068..009EF0A3`, `009EF19C..009EF1FE`, `009EF1BE..009EF1FE` |
| `docs/UNIT_COMMANDED_SPEED.md`: navigatorParams `+21h` is "the byte `luaMW_NavigatorForceMoveCloseToTarget` writes" | correct, and the binding writes the **inverse** of its argument, which is why the byte reads as "keep clear of traffic" on the block: clearing it switches the whole setback walk off | `008A359B TEST AL,AL`, `008A359D SETE AL`, `008A35A4 MOV [ESI+21h],AL`; the gate at `009EEAB7` |

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| `009D5240` | `009D5293` | start: `009D5238..009D523F` read `81 68 03 00 00 C2 08 00`, so a `RET 8` sits at `009D523D` and `009D5240` begins `8B 44 24 04`, `MOV EAX,[ESP+4h]`, with no padding between them. end: `009D5291` reads `C2 08 00`, a `RET 8`, and `009D5294..009D5299` are all `CC`. |

## Follow-up packets

| packet | addresses | what |
| --- | --- | --- |
| `ship_ai_arm_tail_host` | `009EEAAB`, `009EF213` | Give `bsp_game.exe` a `ShipAiArmTailHost` over the existing ship AI hosts and drive a `movetopos` ship end to end, so the start radius, the stop radius and the astern flip get run-time evidence instead of a static reading. Nothing here has a run log. |
| `ship_ai_arm_final_step_009de5b0` | `009DE5B0` | The unconditional last step of every arm of `009ED6B0`, `0B67h` bytes, and one of the writers of `blk+3A5h`. Until it is read the tick has an unread tail. |
| `ship_ai_nav_tuning_009e4330` | `009E4330`, `009E4537`, `009E453F`, `009E4568`, `009E4659`, `0082E960` | The per-ship derivation that fills `blk+3C8h`, `blk+3CCh`, `blk+3D4h`, `blk+3D8h`, `blk+3E4h` and `blk+604h`. Four of them are inputs here and only their store sites were read. |
| `ship_ai_neighbour_world_list` | `009EEB8B`, `00E188A8` | The global list at `[[00E188A8]+19CCh]+60h` and what entity kind 6 selects. The walk is O(list) per step and re-reads the count every pass. |
| `ship_ai_station_keeping_arm` | `009EDA28` | `009EDA28..009EE57B`, now the only unread range of `009ED6B0`. It carries its own latch writes at `009EDB49` and `009EDB65`. |
