# The last step of every arm of the ship AI controls step, `009DE5B0`

Addresses: 009DE5B0 009EF213 009DA1D0 009DC2E0 0070D400 0070D5D0 0070E450 00778890 007788B0 009ECA20

`009DE5B0` is the routine `BSP_ShipAi_ControlsStep` `009ED6B0` calls at `009EF213`, after the
navigation arm tail `009EEAAB..009EF226` has finished and before the step returns. It is the last
thing that touches the control block on a navigation tick, so everything it writes is an input of
the hop `009F3F80` makes next.

It writes exactly three things: `blk+324h`, the heading target, under four rules applied in order;
`blk+354h`, the clearance hold; and `unit+102Ch`, the turn-assist load latch. It writes nothing
else. In particular it never writes `blk+1D0h` (desired throttle), `blk+1D4h` (desired rudder),
`blk+1D8h` (desired heading), `blk+35Ch` (the ahead/astern latch) or `blk+30Ch` (the avoid-zone
layer key it reads).

**`__thiscall(blk)(float unused)`, `RET 4` at `009DF115`**, body `009DE5B0-009DF117`, `0B67h`
bytes, Ghidra function `FUN_009de5b0`, one call site. The call site pushes a float
(`009EF206 FLD [ESP+0D0h]`, `009EF20D PUSH ECX`, `009EF210 FSTP [ESP]`) that the body never reads;
the only use of the incoming frame is `ECX` at `009DE5B4 MOV ESI,ECX`. The `RET 4` still balances
it, so the argument is part of the ABI even though it is dead. Ghidra's stored prototype
`void __fastcall FUN_009de5b0(int param_1)` omits it.

## Coverage

| routine | range | coverage |
| --- | --- | --- |
| `009DE5B0` | `009DE5B0-009DF115` | complete: every instruction of the body read from the stored listing and projected |
| `0092D730` body-axis speed | `0092D730-0092D76E` | complete, contract only (host method) |
| `009DA1D0` clearance gate | `009DA1D0-009DA244` | complete, contract only; the routine belongs to `ship_ai_clearance_profile` |
| `00778890` leads-its-controller | `00778890-007788A7` | complete, contract only |
| `007788B0` follows-another | `007788B0-007788C7` | complete, contract only |
| `0070E450` controller area key | `0070E450-0070E4B2` | complete, contract only |
| `0070D400` controller extent | `0070D400-0070D5C3` | partial: head and one walk step read; the four-way unrolled tail `0070D46C-0070D5C3` and the `iVar3 < iVar4` remainder loop are not projected |
| `0070D5D0` controller extent | `0070D5D0-0070D7A4` | partial: same shape, same unread tail `0070D640-0070D7A4`; the only difference read is the `-0.0f - x` at `0070D5E9` |
| `009DC2E0` free-bearing query | `009DC2E0-009DCEA2` | **not read**: contract taken from `docs/SHIP_AI_SECTOR_SCAN.md` section 14 plus the two call paths here |
| `009ECA20` the producer of `blk+14Ch`/`+150h`/`+160h`/`+30Ch` | `009ECA20-009ED3D7` | partial: only `009ED040..009ED3D1` read, for those four fields; the rest of the routine belongs to another packet |

The reconstruction in `src/ship_ai_arm_final_step.cpp` is build-tested under `/W4 /WX` Win32 and
is not fixture-tested, not ABI-compatible and not game-validated.

## 1. The early out, `009DE5B6..009DE5E5`

```
speed = 0092D730([blk+3FCh]+1018h)         ; 009DE5C2, the controller at unit+1018h
if (|speed| <= 1.0f) return                ; 009DE5DE against the float at 00D7A24C
```

The sign bit is cleared with `AND 7FFFFFFFh` on the stored float (`009DE5CF`), not with an FPU
instruction. `0092D730` dots the controller's velocity with its body forward axis, so the value is
signed and its sign is the direction the hull is **actually** travelling, not the direction the AI
asked for. A ship that is stopped, or nearly so, leaves this routine having changed nothing: the
heading target the arm published stands unaltered.

## 2. The query block and the two entry values, `009DE5EB..009DE678`

The routine builds a `20h`-byte block at `[ESP+3Ch]` and hands its address to `009DC2E0` at the
very end. Its layout is the one `docs/SHIP_AI_SECTOR_SCAN.md` section 14 already established from
`009EC089..009EC0C1`, field for field:

| query offset | stack slot | what | evidence |
| --- | --- | --- | --- |
| `+00h`/`+04h` | `[ESP+3Ch]`/`[ESP+40h]` | the origin, `blk+184h`/`blk+188h` | `009DE617`, `009DE637` |
| `+08h`/`+0Ch` | `[ESP+44h]`/`[ESP+48h]` | the direction, written last | `009DEFC2`, `009DF0DD` |
| `+10h` | `[ESP+4Ch]` | the first corridor half-width, seeded `0.0f` | `009DE672` |
| `+14h` | `[ESP+50h]` | the second, seeded `0.0f` | `009DE66C` |
| `+18h` | `[ESP+54h]` | the range | `009DE662` |
| `+1Ch` | `[ESP+58h]` | the answer, read back into `blk+324h` | `009DF103` |
| `+20h` | `[ESP+5Ch]` | the layer key, seeded from `blk+30Ch` | `009DE62F`, `009DEFE5` |

The seed range is `min(blk+318h * 0.75, blk+32Ch)`: the block's look-ahead, three quarters of it,
capped by the distance to the path point (`009DE5EB`, the double `0.75` at `00CEC9D8`;
`009DE633..009DE662`). `blk+318h` has a floor of `250.0f` under it from the block constructor
(`009E462B`, `include/bsp/ship_ai_nav_block_ctor.hpp`).

Two values are kept from before any write: `blk+324h` as it stood on entry (`009DE5F1`), used only
by the turn-side bound in section 5, and `blk+30Ch` (`009DE5FF`), used as the query's `+20h` word
and as the comparand in section 6.

`[ESP+0Fh]` is set to 1 at `009DE678`. It is the only gate on section 6 and section 3 is the only
thing that clears it.

## 3. The direction-of-travel disagreement and the astern heading, `009DE67D..009DE6D7`

```
v = 0092D730(controller)                                 ; 009DE67D, re-read, not cached
reverse = (v < 0) ? (blk+35Ch == 1) : (blk+35Ch == 2)     ; 009DE68B..009DE6AB, SETNZ inverted
H = [[blk+3FCh]]+50h()                                    ; 009DE6B0, the unit's own heading
if (blk+35Ch == 2) H = wrapAdd(H, pi)                     ; 009DE6D2, the float pi at 00D7A264
```

`reverse` is true exactly when the latched ahead/astern direction disagrees with the sign of the
speed the hull actually has: latched ahead while making sternway, or latched astern while still
carrying way. Both angular corrections below are negated in that case, spelled `-0.0f - x` against
the float at `00D7A208`. `H` is the frame every angle in sections 3 and 5 is measured from, so an
astern ship steers by where its stern points.

## 4. The avoid-zone escape blend, `009DE6DB..009DE8ED`

Gated on the byte `blk+160h`. Its producer is `009ECA20` (the routine `BSP_ShipAi_ControllerStep`
`009F50E0` calls): `009ED296` sets it to 1 when `004178F0` answers that the hull pose `blk+184h` is
inside an avoid zone on the layer the walk reached, and `009ED060` clears it. In the same block
`009ECA20` writes the two fields the blend then reads:

- `blk+14Ch`, how far the ship must travel to leave the zone. `009ED2E9` stores the length of
  `(exit point from 00417B10) - blk+184h`; `009ED2FF` substitutes `1.0f` when that length is at
  most `1.0f`; `009ED356`/`009ED368` cap it at `blk+3C8h`.
- `blk+150h`/`blk+154h`, that same delta normalised (`009ED33E`, `009ED348`), or `(0, 0)` for the
  short case (`009ED30A`, `009ED312`).

```
allow_query = 0                                          ; 009DE6F5, this is the only clear
err  = |wrapSub(H, blk+324h)|                            ; 009DE701..009DE717
w    = Interp(pi/4, 1.0, 80deg, 0.0, err)                ; 009DE742, 00CEB5A8 and 00CF8858
if (!(w > 0)) goto section 5                             ; 009DE755
if (3.0f > blk+354h) blk+354h = 3.0f                     ; 009DE763, 00CE3854
ratio = float(double(blk+330h) / max(100.0f, unit+9C8h)) ; 009DE79C..009DE7A8
s     = Interp(3.0, 0.0, 5.0, 1.0, ratio) * w            ; 009DE7D3, 00CE3854 and 00CE3850
g     = Interp(0.0, 0.15, blk+3C8h, 1.0, blk+14Ch) * s   ; 009DE814, 00CE7818
if (!(g > 0)) goto section 5                             ; 009DE82B
raise unit+102Ch to 1.5 * g                              ; 009DE853, the inlined 009D4FB0
cap   = g * 30deg                                        ; 009DE85B, the double at 00CEC730
d     = wrapSub(headingAngle(blk+150h), blk+324h)        ; 009DE86B, 009DE888
if (reverse) d = -0.0f - d                               ; 009DE898
blk+324h = wrapAdd(blk+324h, clamp(d, -cap, cap))        ; 009DE8CD, 009DE8E7
```

Three ramps multiply: nothing at all while the ship is already more than 80 degrees off its
heading target, nothing while less than three hull radii of path remain, and the weight rises with
how far the ship still has to go to get out of the zone. The product caps one tick's escape turn at
30 degrees. `009D4FB0`'s shape (raise only when the request is strictly greater) is
`docs/UNIT_COMMAND_PRODUCERS.md`, "The load latches".

The important structural point is `009DE6F5`: being inside an avoid zone suppresses the free
bearing query in section 6 for the whole tick, whatever the three ramps then decide, including when
they all close and no turn is applied at all.

## 5. The avoidance-vector override, `009DE8F1..009DE96C`

```
if (!009DA1D0(blk))                       goto section 6   ; 009DE8F8
if (!(0.0f > blk+354h))                   goto section 6   ; 009DE8FF
if (!(float(x*x + z*z) > 1.0e-4))         goto section 6   ; 009DE930, the double at 00D7A268
blk+324h = headingAngle(blk+34Ch, blk+350h)                ; 009DE932, 009DE945
if (blk+35Ch == 2) blk+324h = wrapAdd(that, pi)            ; 009DE95D
```

This is the only rule in the body that **discards** the heading the arm's rudder law produced
rather than nudging it. `blk+34Ch`/`blk+350h` is the avoidance vector `009E04E0` accumulates every
step and `blk+354h` the hold it counts down (`009E0562`, `009E056A`, `009E0591`, see
`docs/SHIP_AI_CLEARANCE_PROFILE.md`). While the hold is still positive nothing happens; once it has
counted past zero and the vector is longer than `1e-2`, the ship is pointed straight down the
avoidance vector and the arm's answer for this tick is gone. Section 4 raising the hold to `3.0f`
is therefore also what suppresses this override for the next three seconds of clearance stepping.

`009DA1D0` is `__thiscall(blk)`, `RET 0`, body `009DA1D0-009DA244`: false when the unit answers
`vtable[5Ch](14)`; otherwise it refreshes the pose through `00414DB0` when `unit+C8h` is clear,
requires `unit+100h >= -15.0` (the double at `00CE3D58`), requires `blk+3ECh`, and requires
`0080E160(unit)->+240h`.

## 6. The traffic separation turn, `009DE96C..009DEE07`

Runs when `blk+604h > 0`, the neighbour list `docs/SHIP_AI_SECTOR_SCAN.md` section 9 describes
(count at `blk+604h`, an inline array of `0x90`-byte node pointers at `blk+608h`, capacity `80h`).

```
probe = blk+184h + clamp(v / [[unit+538h]+500h], -0.5, 0.5) * (blk+174h - blk+184h)
                                                        ; 009DE9AD..009DEA32
for each node n in blk+608h[0 .. blk+604h):
    if (!n+14h)                                continue  ; 009DEA57
    if (!(n+78h > 0.0f))                       continue  ; 009DEA69, the lifetime
    if (![n+14h]+5Ch || [n+14h]+5Dh || [n+14h]+60h || [n+14h]+5Eh) continue
                                                        ; 009DEA73, 7D, 87, 91; +5Eh is "gone"
    r  = ([n+14h]+9C8h + unit+9C8h) / 1.5               ; 009DEAA8, the double at 00CE3D78
    o  = probe - (n+20h, n+24h)                        ; 009DEAB2, 009DEABD
    if (!(|o|^2 > 1.0f))                       continue  ; 009DEAEE
    if (!(r*r > |o|^2))                        continue  ; 009DEAFE
    w  = Interp(0.4, 100.0, 1.0, 0.0, |o| / r) / |o|    ; 009DEB47, 00CE7804 and 00CE3D08
    sep += w * o                                       ; 009DEB72, 009DEB7E
if (!(float(sep.z^2 + sep.x^2) > 1.0e-10))     goto section 7  ; 009DEBDB, 00CE3820
L = sqrt(that)                                         ; 009DEBE1, 00BF7030
if (!(L > 1.0f))                               goto section 7  ; 009DEBFF
if (L > 100.0) sep /= (L / 100.0)                      ; 009DEC13..009DEC2F, 00D7A220
e = wrapSub(headingAngle(sep), H)                      ; 009DEC41..009DEC83
if (e > pi/2)  e = pi - e                              ; 009DEC97
else if (-pi/2 > e) e = -pi - e                        ; 009DECA9
turn = e / 7.0                                         ; 009DECB7, the double at 00CED5D8
turn = clamp(turn, -(blk+3D0h * 1.8), +(blk+3D0h * 1.8))  ; 009DECC7..009DED09, 00D049A8
if (reverse) turn = -0.0f - turn                       ; 009DED21
```

The probe point is where the ship will be a fraction of the way to `blk+174h`, that fraction being
its speed over a per-class reference speed and never more than half. Each neighbour inside the
combined-radius circle pushes the probe away with a weight that falls from `100.0` at four tenths of
the radius to `0.0` at the radius itself. The bearing of the summed push is folded onto the near
side of the beam (`009DEC97`, `009DECA9`), so a crowd astern is answered by turning the short way,
and one seventh of the folded error becomes the turn.

Then the path point's turn side `blk+304h` bounds it against the room left between the heading
target as it stood on entry and the heading target as it stands now:

```
if (blk+304h == 1 && turn > 0.0f)                      ; 009DED31, 009DED36
    room = max(wrapSub(entry_target, blk+324h), 0.0f)  ; 009DED57, 009DED6C
    if (!(room > turn)) turn = room                    ; 009DED89
else if (blk+304h == 2 && 0.0f > turn)                 ; 009DED93, 009DED9B
    room = wrapSub(entry_target, blk+324h)             ; 009DEDB4
    turn = max(turn, min(0.0f, room))                  ; 009DEDCE, 009DEDDF
blk+324h = wrapAdd(blk+324h, turn)                     ; 009DEDFC
```

`ShipAiNavTurnSide::ClampNonPositive` (`1`) and `ClampNonNegative` (`2`) are the names
`include/bsp/ship_ai_navigation.hpp` gives the same field from `009EE916`/`009EE929`. The rule here
is a bound rather than a zeroing: a separation turn may run the heading target up to the value it
had on entry but never past it, on whichever side the path point asked for.

## 7. The avoid-zone free bearing query, `009DEE0B..009DF10F`

```
if (!allow_query) return                               ; 009DEE0B, cleared only by section 4
if (00778890(unit)) {                                  ; 009DEE1E, this entity leads its controller
    a = 0070D400([unit+284h]) ; b = 0070D5D0([unit+284h])   ; 009DEE37, 009DEE4C
    range = min(max(max(a,b) * 2.5, 8.0 * unit+9C8h), unit+9C8h + blk+32Ch)
                                                       ; 009DEE7D..009DEEDD
    key = 0070E450([unit+284h])                        ; 009DEEE9
    if (key != query+20h) {                            ; 009DEEEE
        query+00h/+04h = blk+184h/blk+188h             ; 009DEF92, 009DEFA7
        query+08h/+0Ch = 006BC0C0(blk+324h)            ; 009DEFAD
        query+20h      = 0070E450([unit+284h])         ; 009DEFD3, 009DEFE5, re-read
        searcher       = blk+0A64h                     ; 009DF056
    } else searcher    = blk+0A44h                     ; 009DF0F4
    query+18h = range
    query+10h/+14h = (blk+35Ch == 2)
        ? (0070D5D0 + 50.0, 0070D400 + 50.0)
        : (0070D400 + 50.0, 0070D5D0 + 50.0)           ; 009DEEF8 / 009DEFD8, 00CE3938
} else if (007788B0(unit) && blk+3A5h) {               ; 009DF06B, 009DF074
    query+10h = query+14h = unit+9CCh * 2.5            ; 009DF089, 009DF097
}
if (searcher == blk+0A44h)
    query+08h/+0Ch = 006BC0C0(blk+324h), inlined       ; 009DF09B..009DF0ED
if (009DC2E0(searcher)(&query)) blk+324h = query+1Ch   ; 009DF0FA, 009DF103
```

`00778890` and `007788B0` are exact complements for a non-null controller: both read
`entity+284h`, return false when it is null, and compare `[entity+284h]+14h` against the entity.
`0070D400` and `0070D5D0` walk the controller's `+4F8h` slots of stride `34h` and return the
largest, and the largest negated, of one column of floats at `slot+10h + [ctl+500h]*4`, each
clamped to `[0.0, 1200.0]` and seeded at `1.0`. They are the two lateral extents of the formation
about its own axis, which is why the pair is swapped into the query when the latch is astern: port
and starboard exchange when the hull reverses its facing. `0070E450` returns the largest
`member->vtable[214h]()` over the controller members that answer `member->vtable[5Ch](6)`, starting
from `0`, and `009ECA20` seeds `blk+30Ch` from the same call at `009ECA51`.

The two paths differ in three things only: which searcher is asked, whether the query carries the
block's cached layer key or the controller's current one, and that the "moved" path calls
`006BC0C0` where the other inlines its body. The direction handed to the query is the same value
either way.

`009DEFE5` stores the re-read area key into the query's `+20h` word. It is **not** stored into
`blk+30Ch`: this routine has no store to `blk+30Ch` anywhere in `009DE5B0..009DF115` (the only
mention of that offset is the load at `009DE5FF`), so the cached key stays whatever `009ECA20` last
put there and the comparison at `009DEEEE` can be true on every tick until `009ECA20` runs again.

## Does anything here explain a desired throttle stuck at zero?

No. `009DE5B0` writes `blk+324h`, `blk+354h` and `unit+102Ch` and nothing else; there is no store
to `blk+1D0h`, `blk+1D4h`, `blk+1D8h` or `blk+344h` in the body, and no call that could reach one
(`009DA1D0` is a predicate, `0070D400`/`0070D5D0`/`0070E450` are reductions over a controller's
slots, `00778890`/`007788B0` are field tests, and `009DC2E0` only returns a bearing). The throttle
side of the tick is `blk+344h` from the arm tail and `009F3F80`'s ring build; see
`docs/SHIP_AI_NAVIGATION_ARM_TAIL.md` and `docs/SHIP_AI_THROTTLE_TO_RING.md`.

The override question is answered yes: section 5 replaces the arm's heading target outright, and
section 7 replaces it again with whatever `009DC2E0` answers. Both run after the arm's rudder law
and neither is bounded by it.

## How the executable composes it

`docs/GAME_EXECUTABLE.md` milestone 2q records the call site: `reports/game_executable_milestone_2q.json`
carries `{"method": "ShipAiArmTail::after_arm", "address": "009ef213", "native": "009de5b0",
"function": "009ed6b0", "status": "record", "note": "... 3426 bodies would run here."}`. So the
executable reaches `009EF213` on every navigation tick for 3426 bodies and records the call without
running a body. No host for `009DE5B0` exists on this branch's sources
(`rg -n "arm_final_step" src include` finds nothing at `fcf8b17a`) and the built
`build/win32/Release/bsp_game.exe` here identifies itself as milestone 2l, so no run log from this
worktree can exercise the path; the 2q record above is the run-time evidence for the call site.

The host methods the executable must implement, in call order:

| # | method | call site | native |
| --- | --- | --- | --- |
| 1 | `body_axis_speed_0092d730` | `009DE5C2`, `009DE67D`, `009DE994` | `0092D730` |
| 2 | `unit_heading_vtable_0050` | `009DE6B0` | `[[blk+3FCh]]+50h` |
| 3 | `raise_turn_assist_load_009de853` | `009DE853` | inlined `009D4FB0` |
| 4 | `clearance_gate_009da1d0` | `009DE8F3` | `009DA1D0` |
| 5 | `class_reference_speed_500` | `009DE9AD` | `[[blk+538h]+500h]` field read |
| 6 | `neighbour_608` | `009DEA50` | `blk+608h[i]` field read |
| 7 | `unit_leads_controller_00778890` | `009DEE1E` | `00778890` |
| 8 | `controller_extent_0070d400` | `009DEE37`, `009DEF34`, `009DEF54`, `009DF018`, `009DF02B` | `0070D400` |
| 9 | `controller_extent_0070d5d0` | `009DEE4C`, `009DEF19`, `009DEF6F`, `009DEFFD`, `009DF046` | `0070D5D0` |
| 10 | `controller_area_key_0070e450` | `009DEEE9`, `009DEFD3` | `0070E450` |
| 11 | `controller_belongs_to_another_007788b0` | `009DF06B` | `007788B0` |
| 12 | `avoid_zone_free_bearing_009dc2e0` | `009DF0FA` | `009DC2E0` |

`00414EB0`, `00414C60`, `00415510`, `00415550`, `00415620`, `00419010`, `00438AA0`, `00438B10`,
`006BC0C0` and `00BF7030` are already reconstructed as pure functions and are called directly, not
through the host.

## Corrections

| what was written | what the listing says | evidence |
| --- | --- | --- |
| `docs/SHIP_AI_CLEARANCE_PROFILE.md` follow-up `ship_ai_avoidance_vector_consumer`: "`009E04E0` fills the vector every step; nothing read so far reads it back" | `009DE5B0` reads it back. `009DE908` squares `blk+34Ch`/`blk+350h` and `009DE932` takes its bearing into `blk+324h` | `009DE908`, `009DE90E LEA ECX,[ESI+34Ch]`, `009DE932 CALL 00414EB0`, `009DE945 FST [ESI+324h]` |
| `docs/GAME_EXECUTABLE.md` milestone 2q follow-up 7: "the **unconditional** last step of every arm" | unconditionally **reached**, not unconditionally run: `009DE5E5` returns for `\|speed\| <= 1.0f`, and `009DE6DB` splits the body into two disjoint halves for the rest of the tick | `009DE5DE COMISS` against `00D7A24C` with `JBE 009DF111`; `009DE6E2 JZ 009DE8F1` |
| `include/bsp/ship_ai_sector_scan.hpp`: the free-bearing query's `+20h` is `const void* context`, "blk+0A38h at 009EC02A" | both writers this routine has are integer layer keys, not pointers: `blk+30Ch` and `0070E450`'s `int` result. The two readings need reconciling before either header names the field | `009DE62F MOV [ESP+5Ch],ECX` from `009DE5FF MOV ECX,[ESI+30Ch]`; `009DEFE5 MOV [ESP+5Ch],EAX` from `009DEFD3 CALL 0070E450`; `0070E491 CMP`/`JG` is a signed integer reduction |
| `docs/SHIP_AI_SECTOR_SCAN.md` follow-up `ship_ai_avoid_zone_object` names one object, `blk+0A24h` | there are at least three `20h`-byte searchers in the block: `blk+0A24h` (`009EC0C1`), `blk+0A44h` (`009DF0F4`) and `blk+0A64h` (`009DF056`). `blk+0A38h` and `blk+0A3Ch` fall inside the first | `009EC02A`, `009DF056 LEA ECX,[ESI+0A64h]`, `009DF0F4 LEA ECX,[ESI+0A44h]` |
| `docs/SHIP_AI_PATH_PLANNER.md` follow-up `avoid_zone_geometry` asks "whether the layer key `blk+30Ch` is an id or an object pointer" | an `int` id. `009ED3B8` stores `max(blk+164h, an int from 004121B0)` into it, `009ED396` and `009ED3B2` compare it with `JGE`/`JL`, and `0070E450` builds its seed as a signed maximum starting at `0` | `009ED393..009ED3B8`, `0070E452 XOR EAX,EAX`, `0070E491 CMP [ESP+10h],EAX` + `JG` |
| Ghidra's stored decompilation shows one `FUN_009dc2e0(&local_24)` with a single implicit `this`, and no second parameter on `FUN_009de5b0` | one `CALL` instruction reached from two paths with different `ECX`, and a `RET 4` stack argument the body never reads | `009DF056` and `009DF0F4` both fall into `009DF0FA`; `009DF115 RET 4` against `009EF20D PUSH ECX` |
| Ghidra's decompilation ends with `*(undefined4 *)(param_1 + 0x324) = uStack_8;` where `uStack_8` is never assigned | `uStack_8` is the query block's `+1Ch` output word, written by `009DC2E0` | `009DF103 MOVSS XMM0,[ESP+58h]` with the block based at `[ESP+3Ch]` |

## no_ghidra_function

none. Every address this packet read or projected starts a Ghidra function whose body range the
bridge reports: `009DE5B0-009DF117`, `009ED6B0-009EF228`, `009DA1D0-009DA244`,
`009DC2E0-009DCEA2`, `0070D400-0070D5C3`, `0070D5D0-0070D7A4`, `0070E450-0070E4B2`,
`00778890-007788A7`, `007788B0-007788C7`, `009ECA20-009ED3D7`, `0092D730-0092D76E`. `009D4FB0`,
the out-of-line copy of the load latch, has no Ghidra function, but this packet reads only its
inlined copy at `009DE853` and takes the out-of-line bytes from
`docs/UNIT_COMMAND_PRODUCERS.md`.

## Follow-up packets

| packet | addresses | what it would settle |
| --- | --- | --- |
| `ship_ai_avoid_zone_searchers` | `009DC2E0` `009DC2E0-009DCEA2`, `004158E0`, `00415970`, `009D7050`, `blk+0A24h`, `blk+0A44h`, `blk+0A64h` | What the free-bearing search actually does, what the `+20h` word is to it, and what fills the second and third searchers. Nothing read so far writes `blk+0A44h` or `blk+0A64h`, so half of this routine's last step has an unread producer. |
| `ship_ai_controller_formation_slots` | `0070D400` `0070D46C-0070D5C3`, `0070D5D0` `0070D640-0070D7A4`, `0070E450`, `ctl+18h..ctl+500h` | The controller slot record: what the float column at `slot+10h + [ctl+500h]*4` is, who writes it, and which of the two extents is port. Until then the astern swap at `009DEEF8` is a shape without a side. |
| `ship_ai_avoid_zone_layer_key` | `009ECA20` `009ECA20-009ED3D7`, `004120D0`, `004121A0`, `004121B0`, `004178F0`, `00417B10`, `blk+164h`, `blk+308h`, `blk+30Ch` | The producer of every field section 4 and section 7 read. It is also the only thing that can make the `009DEEEE` comparison stop being true. Overlaps `avoid_zone_geometry`, which owns `004179D0` and `00422500`. |
| `unit_turn_assist_load` | `unit+102Ch`, `unit+1034h`, `00825DE0`, `00825EC0` | What the motion side does with the latch this routine raises. `docs/UNIT_COMMAND_PRODUCERS.md` lists it as the open question at the end of "The load latches". |
| `ship_ai_class_speed_constants` | `[unit+538h]+500h`, `[unit+538h]+508h`, `00828F20` | The two per-class constants `009DE9AD` and `009ED8EC` divide by. Neither has a producer in anything read so far. |
