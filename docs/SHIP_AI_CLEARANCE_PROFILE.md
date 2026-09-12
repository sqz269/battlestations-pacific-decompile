# The turn clearance and the throttle cost profile

Addresses: 009EF910, 009E04E0, 009D56F0, 009D67F0, 009E4330, 009DE2F0, 009EF350, 009F4D10, 009F3F80

Packet `cc_ai_clearance_profile`. Ghidra was read-only for the code reading; the two names added
to the ledger are hypotheses, not recovered symbols. Files:
`include/bsp/ship_ai_clearance_profile.hpp`, `src/ship_ai_clearance_profile.cpp`,
`reports/ship_ai_clearance_profile.json`.

This packet answers the two follow-ups milestone 2p left open
(`docs/GAME_EXECUTABLE.md`, `ship_ai_clearance_37c` and `ship_ai_throttle_profile_producer`) and
the last two uncertainties of `docs/SHIP_AI_OBSTACLE_TABLES.md`.

## Where the two routines sit in one controller step

`009F50E0 BSP_ShipAi_ControllerStep` calls, in this order:

| slot | call site | callee | what it does here |
| --- | --- | --- | --- |
| 11 | `009F51F3` | `009E04E0` | fills the 65 bins at `blk+4h`, clears the bypass byte `blk+45h` |
| 13 | `009F5209` | `009ED6B0` | - |
| 14 | `009F5227` | `009F4D10` | `blk+33Ch = -1.0f`, then `009EF350` and `009EF910` |
| 16 | `009F5248` | `009F4DA0` | `009F3F80` reads all three products |

`009F3F80` reads the clearance at `009F4170`, the `blk+33Ch` gate at `009F4517` and the profile at
`009F44A7` and `009F4878`. Both products are therefore made and consumed inside the same step, in
that order; there is no one-step lag.

## 1. `009EF910`, the turn clearance `blk+37Ch`

`void __thiscall(blk)(float seconds)`, `RET 4`, body `009EF910-009F00F3`. The only caller is
`009F4D10` at `009F4D87`.

The routine measures the room on the inside of the turn the hull is about to make. It runs on a
timer (`blk+374h`, reset to settings `+1D4h` at `009EF948`), and every refresh begins by writing the
**sentinel 9999.0f** (`00CE3D64`) into `blk+37Ch` at `009EF96F`. Nothing else in the routine raises
that value; five later stores can only lower it.

### The geometry

The pivot is not the hull centre. `009DE2F0` rebuilds two shoulder points every step
(`009DE4C2..009DE524`): the hull position `blk+184h/+188h` displaced sideways by `blk+1B8h` along the
port beam normal `blk+19Ch/+1A0h`, giving `blk+18Ch/+190h` (centre + offset) and `blk+194h/+198h`
(centre - offset). `009EF910` picks one of them by the sign of the heading error and by the steering
mode:

```
ahead  (blk+35Ch != 2), 009EFAAF: error <= 0 -> blk+18Ch, adding arm
                                  error >  0 -> blk+194h, subtracting arm
astern (blk+35Ch == 2), 009EFBDD: error >= 0 -> blk+18Ch, subtracting arm
                                  error <  0 -> blk+194h, adding arm
```

The two per-ship constants come from the control-block constructor `009E4330`:

```
blk+3CCh = 0082E960(unit->538h, 0.9f)                      ; 009E4568, the hull radius
blk+3E4h = unit+9C8h * 0.45                                ; 009E44DB
blk+1B4h = blk+3E4h / (blk+3CCh * 1.5)                     ; 009E45C1..009E45D1, a half-angle
blk+1B8h = sqrt(blk+3CCh^2 - unit+9C8h^2 * 0.25)           ; 009E45D7..009E4606, the shoulder offset
```

### The sweep

Both arms build the same pair of bearings (`009EFB03..009EFC7F`):

```
from = wrapped(hull_heading  +/- (pi/2 - blk+1B4h))        ; 009EFB14 / 009EFC40
to   = wrapped(blk+324h      +/- pi/2)                     ; 009EFB33 / 009EFC5F
d    = wrapped_subtract(to, from)                          ; 009EFB4E / 009EFC7A
```

They meet at `009EFC81 FCOMIP ST0,ST1`, whose carry flag clears `BL`. The adding arm arrives with
`ST0 = d` and `ST1 = 0` (`009EFB53 FLDZ; FXCH`), the subtracting arm with `ST0 = 0` and `ST1 = d`
(`009EFC7F FLDZ`), so `BL` is `d >= 0` on one arm and `d <= 0` on the other. **`BL` clear means the
sweep is still ahead of the hull**, and `009EFCA0 JNZ` and `009EFD6C JNZ` both skip the whole
measurement when `BL` is set. The Ghidra pseudocode renders this test with the operands swapped
(`fVar8 > fVar9`); the listing is what the header and the source follow.

### What lowers the clearance

With the sweep pending, three things can write `blk+37Ch`:

1. `009EFD00 009D57E0(&blk+0A24h)(pivot, blk+3CCh, from, &to)`, `RET 10h`. Its body tests the byte
   at `blk+0A24h` (the constructor sets it to 1 at `009E4417`) and forwards to `00415970` with
   `this = blk+0A3Ch` (`009D57FE ADD ECX,0x18`), which walks the static avoid-zone segment tree.
   A hit writes **0.0f** at `009EFD10`.
2. `009EFD4A 00415D70(&blk+0A3Ch)(pivot, blk+3CCh, dir_a, dir_b, from, &to)`, `RET 18h`. Only the
   first four stack arguments are used by the body. It clips each segment of the same tree to the
   wedge whose inward normals are the hull forward axis `blk+1ACh/+1B0h` (negated astern) and the
   negated commanded-heading direction, and returns the smallest `distance - radius`, or
   `FLT_MAX` (`00D7A248`) when nothing survives.
3. `009EFE03..009EFF54`, the neighbour loop. For every entry of the list at `blk+608h`
   (count `blk+604h`), `009D8860` returns the footprint rectangle's extreme corner along each of the
   two wedge normals; a neighbour survives only when both dot products are positive.
   `009D8A30` then returns the point of that rectangle closest to the pivot, and
   `sqrt(d2) - blk+3CCh` replaces `blk+37Ch` when it is smaller (`009EFF30`).

A separate earlier loop (`009EFD7E..009EFDD6`) asks `009DD010` whether any neighbour's footprint
actually meets the swept circle. It filters on the owner at `neighbour+14h`: skipped when null, when
`owner+5Eh` is set, or when the owner's category `owner+54h` is not one of the three `009EC770`
enabled. If it finds one, `blk+37Ch` is forced to **0.0f** at `009EFF60` and the min-distance loop
does not run.

### The four outcomes in `blk+370h`

| value | where | condition |
| --- | --- | --- |
| 0 | `009EF969` | the refresh default |
| 1 | `009F00BF` | the faded heading error exceeds the settings gate |
| 2 | `009EFFA5` | a neighbour blocks the sweep and its speed exceeds 2.0 (`00CE3958`) |
| 3 | `009EFFB8` | a neighbour blocks the sweep and is slower, or a static zone blocks |

`ship_ai_obstacle_tables.hpp` already carries this field as `escape_mode_370` after `009F3F80`'s
writers `009F4999` and `009F4A02`. `009EF910` is the other writer, and these are the values it can
leave. When the outcome is not 0 the hold `blk+354h` is raised to 3.0f (`00CE3854`, `009F00E3`),
never lowered.

The heading-error gate is faded near the end of the path (`009F0022..009F0072`):
`ratio = blk+330h / 00811A30(unit, 1.0f)`, then
`InterpolateClamped(1.0, 1.0, 2.0, 0.0, ratio)` scales the error. The gate itself is
settings `+214h` when the latch and the committed direction agree and `+218h` otherwise.

### Why an open-sea ship reads 1.0 danger in the executable, and what the real routine answers

With no neighbours (`blk+604h == 0`) and no segment tree (`blk+0A3Ch == 0`), the test at `009EF9F9`
jumps straight to the tail and the clearance stays at the **9999.0f** the refresh just wrote.
`009F3F80` then forms `ratio = blk+37Ch / unit+9CCh`, a number far past the 4.0 of
`kShipAiDangerClearanceMax`, and `InterpolateClamped(1.0, 1.0, 4.0, 0.0, ratio)` clamps to **0.0**.
The correct answer on an open sea is therefore no danger at all.

The executable's substitute record leaves `blk+37Ch` at zero, so its ratio is 0, which is below the
1.0 of `kShipAiDangerClearanceMin`, and the same interpolation clamps to `y0 = 1.0`: full danger, on
every ship, on every step. The sentinel is the whole difference. A record that writes 9999.0f and
nothing else already produces the right danger level for a ship with nothing near it.

## 2. `blk+33Ch` is not a clearance, and the rudder gate is not where the brief placed it

`blk+33Ch` is a constant, not a computed quantity. `009F4D10` stores `-1.0f` (`00D7A260`) into it
unconditionally at `009F4D27`, before anything else in that routine, and `009EF910` never touches it.

`009F4514` reads it as `XORPS XMM0,XMM0; COMISS XMM0,[ESI+33Ch]; JBE 009F454C`, so the clamp at
`009F451D` (which limits the desired rudder `blk+1D4h` into `+/- blk+348h` through `00415690`) runs
**only when `blk+33Ch` is negative**. The gate is therefore open by default and is closed by making
the field non-negative.

The one routine that closes it is **`009EF350` at `009EF876`**, which `009F4D10` calls at `009F4D78`,
between the `-1.0f` store and `009EF910`. `009EF876` stores a square root (`009EF869 CALL 00BF7030`)
of the smaller of two squared distances, so a non-negative `blk+33Ch` is a real distance to a
blocking neighbour and means "an obstacle is close, do not limit the rudder".

A block that never writes `blk+33Ch` leaves it at zero, `0.0 <= 0.0` is true, and the clamp is
skipped forever. That is the shape of "the gate never opens" in the executable's run: the field is
zero rather than `-1.0f`. The existing reconstruction of `009F4D10`
(`include/bsp/unit_autopilot_pair.hpp`, `ShipAiPublishResult::blk_33c`) already carries the `-1.0f`;
what the executable is missing is a consumer that stores it on the block.

## 3. `009E04E0` and the 65-bin throttle cost profile

`void __thiscall(blk)(float seconds)`, `RET 4`, body `009E04E0-009E1167`. The only caller is
`009F50E0` at `009F51F3`.

### The producer is `009D56F0`, and it is the only one

`void __thiscall(profile)(float low, float high, char cost)`, `RET 0Ch`, body
`009D56F0-009D575C`, whole:

```
i0 = clamp(ftol((low  + 2.0) * 16.0), 0, 0x40)             ; 009D56F0..009D5723
i1 = clamp(ftol((high + 2.0) * 16.0), 0, 0x40)             ; 009D5723..009D5740
if (i0 <= i1) {
    profile+41h = 0;                                       ; 009D5748
    for (i = i0; i <= i1; ++i) profile[i] += cost;         ; 009D5750 ADD byte ptr, 8-bit wrap
}
```

`009D67F0` (`RET 8`, body `009D67F0-009D680C`) is the cost-1 wrapper: `ADD ECX,0x4` at `009D67FD` is
what turns the control block into the profile, which is why the three call sites inside `009E04E0`
pass `this = blk` while the profile lives at `blk+4h`.

`009D56F0`'s only callers are `009D67F0` and `009E04E0`; `009D67F0`'s only caller is `009E04E0`.
So **`009E04E0` is the sole producer of the bins**, and the only other writer of that memory is the
constructor's `memset` (below).

### Why every bin is zero and the profile is bypassed before `009E04E0` runs

The control-block constructor `009E4330` does, at `009E434C..009E4363`:

```
PUSH 0x41; LEA EAX,[ESI+4]; PUSH 0; PUSH EAX
MOV byte ptr [EAX + 0x41], 1                               ; blk+45h = 1
CALL 0x00BF79F0                                            ; memset(blk+4h, 0, 0x41)
```

so a fresh block has 65 zero bins **and the bypass byte set**. `009D6B40` in that state only clamps
(`ship_ai_obstacle_tables.hpp`, `009D6B5D`). The first `009D56F0` call of the run is what clears the
bypass. A block whose `009E04E0` never runs is not merely "all bins zero"; it is switched off.

The same constructor sets up a second, 256-entry array the same way at `009E4368..009E4379`
(`memset(blk+46h, 0, 0x100)` with `blk+146h = 1`). Nothing in this packet reads it.

### What the routine computes

Once per step it clears the avoidance vector `blk+34Ch` / `blk+350h` (`009E0562`, `009E056A`) and
counts the hold `blk+354h` down by `seconds` while it is non-negative (`009E0591`). Then it walks the
contact-track list at `blk+404h` (count `blk+400h`), retiring tracks whose lifetime `+14h` has run
out, whose `+64h` byte is set, or that have neither observer pair (`009E05DF..009E05F9`); the removal
moves the last entry into the hole (`009E1057`) and steps the index back.

For a surviving track, `009DC060` (`009E0631`) refreshes the tracked entity's position `+2Ch/+30h`,
its normalised course `+24h/+28h`, its speed `+18h` and its heading `+20h`. The routine then works
out, in the hull's frame:

```
error   = wrapped_subtract(hull_heading, track+20h)        ; 009E064E
side    = rel . (-track+28h, track+24h)                    ; 009E06B0..009E06C2
gap     = (1 - |acos(clamp(error))|) * 00811A30(unit,0.75) ; 009E06E8..009E074B
width   = unit+9CCh + 20.0                                 ; 009E074F
lateral = rel . (blk+19Ch, blk+1A0h)                       ; 009E0785
horizon = max(1.0, ((|lateral| - unit+9CCh/1.5)/|sin| - track+4h) / track+18h)
                                                           ; 009E07D1..009E087F
along   = |lateral/sin| * cos + rel . (blk+1ACh, blk+1B0h) ; 009E0888..009E08E5
margin  = unit->538h+A0h * 0.6 + |track+0h / sin| + 10.0    ; 009E08EF..009E0913
```

and turns the crossing interval `[along - margin, along + margin]` into a **window of own-ship
throttles**: each end is divided by `horizon`, corrected by a constant-acceleration quadratic
(`009E0953..009E09E4` and `009E0A30..009E0AF0`, with `unit->538h+504h` as the acceleration), and then
divided by the reference speed `blk+3C4h` (`009E0B37`). That division is why the axis is a throttle
axis: bin `i` is `i * 0.0625 - 2.0`, so the array covers `-2.0 .. +2.0` times `blk+3C4h`.

The window then decides what is written (`009E0B37..009E0C4D`):

| window | action |
| --- | --- |
| `high <= -2.0` or `low >= 2.0` | nothing; the track is off the axis |
| `high <= 1.0`, `low >= -0.5` | `009D67F0(low, high)`, cost 1, track finished |
| `high <= 1.0`, `low < -0.5` | `009D67F0(-2.0, high)`; if `high <= 0.85` the track is finished, otherwise it also steers |
| `high > 1.0`, `low >= -0.5` | `009D67F0(low, 2.0)`, cost 1, track finished |
| `high > 1.0`, `low < -0.5` | no throttle avoids it: `(low + 0.5) * -2 <= high - 1` counts a stand-on, otherwise a give-way, and the track steers |

A track that steers adds a unit step to `blk+34Ch` / `blk+350h` (`009E0C7C..009E0FBD`): the contact's
left normal `(-dir_z, dir_x)` when the lateral gap lands inside
`[stand_on ? -(30 + width) : 0, width + 30)`, and twice the contact's own course otherwise, with the
sign of that course step depending on whether the relative bearing is inside a quarter turn.

### The tail: the commitment band

`009E10A9..009E114B`. If neither counter fired the routine returns with only the cost-1 bands in
place. Otherwise:

```
ratio = own_speed / blk+3C4h                               ; 009E10B8
if (banded != 0) ratio = 009D6B40(&ratio, -1.0f, 1.0f)     ; 009E10C6..009E10E2
                 side  = (ratio >= 0.0f)                   ; 009E10F2
else             side  = (4 * (give_way - stand_on) <= ratio) ; 009E111F..009E112E
009D56F0(blk+4h, side ? (-2.0, 0.98) : (-0.48, 2.0), 10)   ; 009E10FD / 009E1138 / 009E114B
```

Cost 10 against the cost-1 contact bands is a strong bias, not a ban: `009D6B40` walks outward to the
cheapest reachable bin, so the ship is pushed to the far end of the half of the axis it is already
on rather than allowed to cross zero mid-manoeuvre.

## Coverage

| routine | ABI | coverage |
| --- | --- | --- |
| `009EF910` | `void __thiscall(blk)(float)`, `RET 4`, `009EF910-009F00F3` | complete; all 508 listed instructions read |
| `009D56F0` | `void __thiscall(profile)(float, float, char)`, `RET 0Ch`, `009D56F0-009D575C` | complete |
| `009D67F0` | `void __thiscall(blk)(float, float)`, `RET 8`, `009D67F0-009D680C` | complete |
| `009E04E0` | `void __thiscall(blk)(float)`, `RET 4`, `009E04E0-009E1167` | partial: the throttle-band pipeline, the list maintenance and the tail are projected from the listing. `009E0C7C..009E0FBD` (the avoidance-vector arms) is projected from the Ghidra pseudocode with the listing spot-checked at `009E0C7C`, `009E0C86`, `009E0CA2`, `009E0D07`, `009E0D18`, `009E0D30..009E0D46` and `009E0D71` only; the remaining comparison order inside `009E0DB4..009E0FBD` was not re-checked. `009E0FBD..009E104C` (the track teardown: the two `00695870` destructor pairs and the `_free`) is modelled as one host call, not projected |
| `009DE2F0` | not reconstructed | read only far enough to name the shoulder points and the axes; `009DE2F0-009DE49C` was read, the rest was not |
| `009E4330` | not reconstructed | read only for the `memset`, the bypass byte and the four constants above |

`009E04E0` has one Ghidra flow gap, reported by `python tools/bsp.py ghidra flow 009e04e0`:
`009E1040..009E1046` after the `_free` at `009E103B`. The bytes there are `0F 57 C0 83 C4 04`
(`XORPS XMM0,XMM0; ADD ESP,0x4`), read from the stored listing; the decompiler drops the block that
follows, which is the list compaction at `009E104C`. The second reported gap,
`009E05B9..009E05C0`, is `8D A4 24 00 00 00 00`, a seven-byte alignment `LEA ESP,[ESP]`. Neither is
repaired here: this packet is read-only against Ghidra. `009EF910` has no gaps.

## Corrections

| claim | correction | evidence |
| --- | --- | --- |
| the packet brief and `docs/GAME_EXECUTABLE.md` follow-up 4: "`blk+37Ch` (the danger level) and `blk+33Ch` (the clearance?)" | Reversed and mis-typed. `blk+37Ch` is the clearance, a distance; the danger level is `blk+0A84h`, which `009F3F80` derives from it. `blk+33Ch` is neither: it is a `-1.0f` sentinel that gates the rudder clamp | `009EF96F` writes 9999.0f into `blk+37Ch`; `009F416E` divides it by `unit+9CCh` to form the danger; `009F4D27` stores `00D7A260 = -1.0f` into `blk+33Ch` |
| the same brief: "the danger level saturates at 1.0 for every ship on every step" | The saturation is a consequence of a missing store, not of the routine. The real routine leaves 9999.0f on an open sea, which ramps the danger to **0.0**, not 1.0 | `009EF9F9 JZ 009EFFC2` with no neighbours and no zone tree; `009F4189..009F41A2 InterpolateClamped(1.0, 1.0, 4.0, 0.0, ratio)` |
| the same brief: "the rudder limit's gate at `009F4514` never opens" | The gate is open when `blk+33Ch < 0` and shut when it is `>= 0`, so `-1.0f` opens it. It never opens because the field is left at zero, not because a producer is missing | `009F4511 XORPS XMM0,XMM0; 009F4514 COMISS XMM0,[ESI+33Ch]; 009F451B JBE 009F454C` |
| `docs/SHIP_AI_OBSTACLE_TABLES.md` uncertainty 2: "`blk+33Ch`, the gate on the rudder limit, is published by `009F4D10` at `009F4D2B`. Its producer was not read" | Resolved, and the store address is `009F4D27` (`009F4D2B` is the middle of that eight-byte `MOVSS`). The value is the constant `-1.0f`; the only routine that replaces it is `009EF350` at `009EF876` | the listing of `009F4D10`; `rg` over the whole image for the displacement `3C 03 00 00` finds exactly `009EF878`, `009F4517` and `009F4D2B` |
| `docs/SHIP_AI_OBSTACLE_TABLES.md` uncertainty 6: "Nothing read here writes the 65 profile bytes at `blk+4h` or the bypass byte at `blk+45h`" | Resolved. `009D56F0` writes both, `009D67F0` is its cost-1 wrapper, and `009E04E0` is the only caller of either. The constructor `009E4330` sets the bypass byte to 1 at `009E435F` | the call graph of `009D56F0`; `009E435F MOV byte ptr [EAX+0x41],1` with `EAX = blk+4h` |
| the Ghidra pseudocode of `009EF910`: `if ((fVar8 > fVar9) && ...)` enters the measurement | The operands are swapped. The listing sets `BL` from one `FCOMIP` reached with the operands on opposite stack slots per arm, and the measurement runs when `BL` is **clear** | `009EFB53 FLDZ; FXCH` versus `009EFC7F FLDZ`, then `009EFC81 FCOMIP ST0,ST1; 009EFC85 JC` and `009EFCA0 JNZ` |
| the writer and the reader of the profile share an index rule | They do not. `009D56F0` truncates `(v + 2.0) * 16.0`; `009D6B40` adds 0.5 first (`009D6B8F`), so the reader rounds to nearest and the writer rounds down | `009D5700..009D570A` has no `00D7A280` term |

## Host methods the executable must implement, in call order

`009EF910`, `ShipAiClearanceHost`:

| # | call site | native | method |
| --- | --- | --- | --- |
| 1 | `009EF97C` | `unit->vtable[50h]` | `hull_heading_vtable50` |
| 2 | `009EFA08` | `009EC770` | `obstacle_category_enabled_009ec770` |
| 3 | `009EFCB8` | `0080E160` | `avoidance_globally_enabled_0080e160_242` |
| 4 | `009EFD00` | `009D57E0` | `static_zone_blocks_009d57e0` |
| 5 | `009EFD4A` | `00415D70` | `static_zone_clearance_00415d70` |
| 6 | `009EFDBD` | `009DD010` | `neighbour_blocks_sweep_009dd010` |
| 7 | `009EFE0F` | `009D8860` | `neighbour_support_point_009d8860` |
| 8 | `009EFEC3` | `009D8A30` | `neighbour_closest_point_009d8a30` |
| 9 | `009EFF7F` | `0092D730` | `neighbour_speed_0092d730` |
| 10 | `009F0000` | `00778890` | `path_fade_applies_00778890` |
| 11 | `009F0038` | `00811A30` | `class_length_unit_00811a30` |

`009E04E0`, `ShipAiThrottleProfileHost`:

| # | call site | native | method |
| --- | --- | --- | --- |
| 1 | `009E0509` | `unit->vtable[50h]` | `hull_heading_vtable50` |
| 2 | `009E061B` | `009DA1D0` | `avoidance_active_009da1d0` |
| 3 | `009E0631` | `009DC060` | `refresh_track_009dc060` |
| 4 | `009E0648` | `unit->vtable[50h]` | `hull_heading_vtable50` |
| 5 | `009E0723` | `00811A30` | `class_length_three_quarters_00811a30` |
| 6 | `009E0FEE` | `006952A0` | `destroy_track` (with `009E0FFD`, `009E1023`, `009E1035`, `009E103B`) |

The two profile writers and the snap are pure rules, not host calls:
`009E0BEC`, `009E0C19`, `009E0C45` and `009E114B` are `009D67F0` / `009D56F0`, and `009E10E2` is
`009D6B40`, which `include/bsp/ship_ai_obstacle_tables.hpp` already projects.

## Uncertainties

1. `00BF9940` at `009E06E8` is identified as `acos` from its guard pattern alone (`0.0` above 1.0,
   `pi` below `-1.0`, the library call between). Its body was not read. The routine feeds it a
   wrapped angle difference rather than a cosine, which may be a defect in the original or may mean
   the value at that point is not what this packet calls it.
2. `blk+19Ch/+1A0h` is called the port beam normal from `009DE465`'s construction
   (`(-forward_z, forward_x)`); which side of the hull that is in world terms was not confirmed
   against a run.
3. `009DD010`'s sweep test was read only as far as the four footprint corners at
   `009DD010-009DD0B0`. The claim that the swept arc runs from `from` to `to` rather than the other
   way round comes from `009EF910`'s argument order, not from that body.
4. The track fields `+0h`, `+4h` and `+0Ch` are named from how `009E04E0` uses them; their producer
   was not found. `+18h`..`+30h` are named from their producer `009DC060`.
5. `blk+3C4h` is called the reference speed after `ship_ai_obstacle_tables.hpp`. Whether a throttle
   of `+2.0` on this axis is reachable at all was not checked against the ship classes.
6. No run-time evidence is quoted. `bsp_game.exe` reaches `009F50E0`'s chain (milestone 2p), so the
   sentinel claim in section 1 is testable by instrumenting the substitute record for `009EF910`;
   this packet did not run it.

## Follow-up packets

| packet | addresses and files | what is left |
| --- | --- | --- |
| `ship_ai_clearance_33c_producer` | `009EF350` `009EF35C..009EF907`, `009DCEB0`, `009D7AF0`, `blk+33Ch`, `blk+74h`/`blk+75h` on the neighbour | The other half of `009F4D10`'s tail: which neighbour closes the rudder gate, and the pass-side bytes `009DCEB0` writes. Read here only far enough to find the `009EF876` store |
| `ship_ai_hull_frame_009de2f0` | `009DE2F0` `009DE2F0-009DE5A0`, `00414DB0`, `004142E0`, `00419260`, `blk+174h..blk+1C0h` | The producer of every geometric input both routines in this packet read. `009DE4A0..009DE524` is settled; the bow/stern points, the normalisation and the tail at `009DE52A` are not |
| `ship_ai_control_block_ctor_009e4330` | `009E4330` `009E4330-009E46C2`, `009DFCB0`, `0082E960`, `008E6430` | The per-ship constants: `blk+3C8h`, `blk+3CCh`, `blk+3D0h`, `blk+3D4h`, `blk+3D8h`, `blk+3E4h`, `blk+340h`, `blk+1B4h`, `blk+1B8h`, and the second 256-entry profile at `blk+46h` that nothing read so far touches |
| `ship_ai_contact_track_source` | `blk+400h`/`blk+404h`, `009DC060`, `006952A0`, `00695870`, the track allocation | Who creates a contact track and sets `+0h`, `+4h`, `+0Ch` and `+10h`. `009E04E0` only consumes and destroys them |
| `ship_ai_avoidance_vector_consumer` | `blk+34Ch`, `blk+350h` | `009E04E0` fills the vector every step; nothing read so far reads it back. `009F4DA0` writes `brain+34Ch`/`brain+350h`, a different pair on a different object (`ship_ai_obstacle_tables.hpp`, line 185), which is worth reconciling before either is named |

## no_ghidra_function

none. `009EF910` (`009EF910-009F00F3`), `009E04E0` (`009E04E0-009E1167`), `009D56F0`
(`009D56F0-009D575C`) and `009D67F0` (`009D67F0-009D680C`) all have Ghidra function bodies, confirmed
with `python tools/bsp.py ghidra proto <addr> --brief`.
