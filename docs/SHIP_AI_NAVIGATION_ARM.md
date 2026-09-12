# The ship AI's navigation arm, 009ED6B0's second half

Addresses: 009ED6B0 009ED761 009EE671 009EE6AC 009EE756 009EE76B 009EE78B 009EE7EB 009EE813
009EE83A 009EE848 009EE87B 009EE895 009EE8B1 009EE8C7 009EE907 009EE916 009EE964 009EE9C7
009EEA00 009EEA08 009EEA1A 009EEA9D 00415510 00415550 00415620

Packet `cc_ai_order_hop`, worker `agent/cc-ai-order-hop`, 2026-09-12 UTC. Ghidra was read-only
for this packet. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every
live query verified both. Descriptive names are hypotheses, not recovered symbols.

## What this adds

`docs/SHIP_AI_STATES.md` reconstructed `009ED6B0`'s direct-control arm, `009ED6B0-009EDA25`, the
half that serves the three desired-value setters, and left `009EDA26-009EF228` in pseudocode
only. This packet reads and projects the arm's **output block**, `009EE671..009EEAA2`: the part
that turns a path point and the unit's pose into the three fields `009F4D10` publishes. The
collision and formation work before it is still unread.

## The mode gate

Two tests stand between the routine's entry and the output block.

* The arm as a whole is entered only when the steering mode is `Navigate` (2) or
  `NavigateAstern` (3): the decompiled `if ((blk+1C4h != 2) && !astern) goto LAB_009ef206`.
* `009EE756 CMP byte [ESP+4Fh],0; JNZ 009EF206` then skips the output block. That byte is
  `009ED7E9 SETZ BL` on `blk+1C4h == 3`, stored at `009ED7F3` and not written again.

So the block projected below runs for mode `Navigate` only. `NavigateAstern` reaches the path
pick and the goal publish before it and then leaves; modes 0 and 1 never get that far. The
reconstruction does not re-test the mode: its caller owns the gate, and `src/ship_motion_probe.cpp`
sets the mode explicitly because no site in the image was read writing 2 or 3
(`include/bsp/ship_ai_states.hpp`).

## The block, operation for operation

`blk` is `brain+8h`, the same base `ShipAiControlBlock` uses. `pose` is `[blk+184h]` and
`[blk+188h]`; `wp` is the path point `009E3C00` hands back at `009EE550`.

| address | operation |
| --- | --- |
| `009EE67A..009EE69D` | `blk+0A90h = 1`, `blk+0A94h = wp.x`, `blk+0A98h = 0.0f`, `blk+0A9Ch = wp.z`. The middle component is always stored as a literal zero (`009EE689 XORPS`) |
| `009EE6AC` | `pathLength = 009D9E50([blk+2F4h])(&pose)` |
| `009EE6B5..009EE6CB` | `blk+1F0h = max(blk+1F0h, pathLength)`, through `FCOMPI`/`JBE`, so an unordered compare does not raise it |
| `009EE6F1..009EE712` | `d = (wp.x - pose.x, wp.z - pose.z)`, two float stores |
| `009EE765` | `blk+304h = side`, the code `009E3C00` returned with the point |
| `009EE76B/770` | `blk+32Ch = 00414C60(&d)`, the planar distance |
| `009EE776..009EE783` | `blk+338h = (morePath == 0)` |
| `009EE78B..009EE7BF` | when `morePath == 0`: `e = (next.x - wp.x, next.z - wp.z)`, `len2 = e.x*e.x + e.z*e.z` with one float store |
| `009EE7C3..009EE7E4` | `if ((double)len2 > 100.0) blk+334h = 00414EB0(&e); else blk+338h = 0` (`00D7A220`) |
| `009EE7EB..009EE809` | the bearing gate: `(double)blk+32Ch > 0.1` (`00D7A3A0`) **and** the byte at `[ESP+99h]` |
| `009EE813/818` | `blk+324h = 00414EB0(&d)`, the bearing from the unit to the path point |
| `009EE83A/842` | the turn lead runs only while `(double)blk+32Ch < 1000.0` (`00CE47A0`) |
| `009EEA00` | `blk+330h = pathLength` |
| `009EEA08..009EEAA2` | the look-ahead radius, below |

`blk+324h`, `blk+32Ch` and `blk+330h` are exactly what `009F4D10` publishes into the unit's order
slot at `009F4D48`, `009F4D55` and `009F4D62` (`docs/UNIT_AI_ORDER_SLOT_READER.md`). The triple
the navigation arm produces is therefore **the bearing to the path point, the distance to it and
the length of path still to run**, which is what settles the slot's meaning.

### The turn lead, `009EE848..009EE9D8`

```
scaledPath = float(blk+1F0h * 1.5)                        ; 009EE856, 00CE3D78 a double
denom      = min(1000.0f, scaledPath)                     ; 009EE87B, 00CE3804 / 00415510
ratio      = float(double(blk+32Ch) / denom)              ; 009EE877 stores the distance as a
                                                          ; double, 009EE880 FDIVR, 009EE891
taper      = float(1.0 - clamp(ratio, 0.0f, 1.0f))        ; 009EE895 / 00415620, 009EE89A
lead       = float((1.57079637051 - blk+3D0h) * taper * taper)  ; 009EE8B1..009EE8C3, one store
                                                          ; 00CE3830, an 80-bit chain
h          = unit->vtable[50h]()                          ; 009EE8C7 on [blk+3FCh]
if (blk+35Ch == 2) h = 00438AA0(h, pi)                    ; 009EE8C9, 009EE8EA, 00D7A264
delta      = 00438B10(h, blk+324h)                        ; 009EE907
if (blk+304h == 2 && delta < 0) delta = 0                 ; 009EE916..009EE936
if (blk+304h == 1 && delta > 0) delta = 0
if (|delta| > blk+3D0h) {                                 ; 009EE95E..009EE964
    corr = (delta > 0) ? min(delta - blk+3D0h,  lead)     ; 009EE96F, 009EE97F
                       : max(blk+3D0h + delta, -lead)     ; 009EE986, 009EE9AA
    blk+324h = 00438AA0(blk+324h, corr)                   ; 009EE9C7, 009EE9D2
    blk+328h = corr                                       ; 009EE9D8
}
```

The correction carries the sign of the heading error and is bounded by `lead`, which is
`pi/2 - blk+3D0h` at zero distance and taper zero at `min(1000, 1.5 * blk+1F0h)` and beyond. It
is added to the bearing, and `delta` is `heading - bearing`, so the effect is to pull the
published heading target back toward the heading the unit is actually on. It is a rate limit on
how far the AI is willing to swing its target in one frame, tightest far out and loosest close in.

`00415510` and `00415550` are the image's `min` and `max`:
`__fastcall(const float* a, const float* b) -> float`, bodies `00415510-0041554E` and
`00415550-0041558E`. Each loads both through the stack and decides with one `FCOMIP`/`JBE`, so an
unordered compare returns `b` in both. `00415620` is `clamp(a, low, high)`,
`__fastcall(const float* a, const float* b, const float* c) -> float`, `RET 4`, body
`00415620-0041565C`: `FCOMI b,a; JA` returns `low`, then `FCOMI a,c; JBE` returns `a`, otherwise
`high`.

### The look-ahead radius, `009EEA08..009EEAA2`

```
if (blk+32Ch < blk+340h + blk+340h) {                      ; 009EEA08 FADD ST0,ST0, 009EEA0A/0E
    axis  = 00414EB0(blk+19Ch)                             ; 009EEA1A
    off   = 00438B10(blk+324h, axis)                       ; 009EEA37
    c     = float(fcos(off))                               ; 009EEA44, one float store
    m     = max(0.001f, |c|)                               ; 009EEA64 00D7A23C, 009EEA84
    r     = float(double(blk+32Ch) / (m + m))              ; 009EEA5C stores the distance as a
                                                           ; double, 009EEA89, 009EEA95
    blk+340h = min(blk+3C8h, r)                            ; 009EEA8F, 009EEA9D, 009EEAA2
}
```

`blk+340h` is seeded from `blk+3C8h` every frame at the top of `009ED6B0` (`009ED761`,
`009ED769`), so `blk+3C8h` is the ceiling and `blk+340h` the per-frame value. `r` is the radius of
the circle through the unit that is tangent to its own axis and passes the path point, which is
why the cosine is floored rather than guarded: at exactly a right angle the radius would be
infinite.

## x87 precision

Four expressions are 80-bit chains with a single rounding at their store and are reconstructed
that way: the `1.5` scale at `009EE856`, the ratio's division at `009EE880` (whose numerator was
stored as a **double** at `009EE877`), the `lead` product at `009EE8B1..009EE8C3`, and the radius
division at `009EEA95` (numerator stored as a double at `009EEA5C`). Everything else rounds to
float at each named store. `FCOS` at `009EEA44` runs on the float-rounded angle and its result is
rounded once. `docs/X87_CONTROL_WORD.md`.

## Routines and coverage

| routine | state | coverage |
| --- | --- | --- |
| `009ED6B0` | the output block reconstructed as `ship_ai_navigation_arm_009ee671`, build-tested and probe-exercised | partial: only `009EE671..009EEAA2`. `009EDA26..009EE670` (the collision and formation arm, including the `00815F30` publish at `009EE66C` and the four `00415510`/`00415550` calls at `009ED8xx..009EE0xx`) and `009EEAAB..009EF228` are unread. `009ED6B0..009EDA25` stays as `docs/SHIP_AI_STATES.md` left it |
| `009EE848..009EE9AF` | reconstructed on its own as `ship_ai_turn_lead_009ee848` | complete |
| `00415510`, `00415550`, `00415620` | read in full from the listing; projected as local helpers inside `src/ship_ai_navigation.cpp` | complete |
| `00414C60`, `00414EB0`, `00438AA0`, `00438B10` | reused from `docs/VECTOR_HELPERS.md`, `docs/GEOMETRY_HELPERS.md` and `docs/UNIT_RUDDER_CURVE.md` | as reconstructed there |
| `009D9E50`, `009E3C00`, `009ED3E0`, `00815F30`, `00811D80` | not read; host methods or named callees only | none |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/SHIP_AI_STATES.md`: `009EDA26-009EF228`, the navigation arm that computes a bearing to a waypoint, "is read in pseudocode only" | Its output block `009EE671..009EEAA2` is now read from the listing and reconstructed, including the turn lead and the look-ahead radius. The rest of the range is still unread | the table above |
| `include/bsp/ship_ai_states.hpp`: "A false return does NOT mean the routine ended; it means the unprojected navigation arm 009EDA26-009EF228 runs next and may overwrite +324h, +32Ch and +330h" | It does overwrite all three, unconditionally for `+32Ch` and `+330h` and under the `009EE7FB`/`009EE809` gate for `+324h` | `009EE770`, `009EEA00`, `009EE818` |

## no_ghidra_function

none. `009ED6B0` has a Ghidra body (`009ED6B0-009EF228`), and so do `00415510`, `00415550` and
`00415620`. The three unreferenced out-of-line copies this packet found are listed in
`docs/UNIT_AI_ORDER_SLOT_READER.md`.

## Uncertainties

* `blk+3C8h` and `blk+3D0h` have no read producer here. The probe supplies `500.0f` and
  `0.1f` as run inputs and says so; they are not recovered values.
* `blk+19Ch` is read as a two-float direction by `009EEA1A`. Which direction it is - the hull
  axis, the previous leg, the last published heading - is not established; only its shape is.
* The `[ESP+99h]` byte that gates the bearing comes from the unread part of the arm. The
  reconstruction takes it as an input named `steer_enabled` and claims nothing about its producer.
* `blk+304h`'s two values are modelled by what the comparison does, not by what they mean. Port
  and starboard would be an invention.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ship_ai_collision_arm` | `009EDA26`, `009EDB6B`, `009EDD9B`, `009EE66C`, `00815F30`, `0082E850`, `007789D0` | The unread first half of the navigation arm: the two-ship closing test at `009EDBxx..009EDExx` and the sub-record it publishes into the order slot through `00815F30`. That is what the slot's `+08h..+3Fh` half carries |
| `ship_ai_path_source` | `009E3C00`, `009ED3E0`, `009D9E50`, `00811D80` | Where the path point, the side code and the remaining length come from. `009E3C00` already reads another unit's published slot at `009E3DBE`, so it is both a producer and a consumer |
| `ship_ai_nav_block_fields` | `009ED6B0`, `009F39C0`, `blk+19Ch`, `blk+3C8h`, `blk+3D0h`, `blk+1F0h` | The control block's constructor, to give the four navigation fields real initial values instead of the probe's run inputs |
