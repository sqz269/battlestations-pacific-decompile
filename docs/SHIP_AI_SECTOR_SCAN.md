# The obstacle sector scan 009EB660 and the neighbour list it walks

Addresses: 009EB660 009D84E0 009D80C0 009D8160 009DD010 009DD540 009DC2E0 009E0270 009E52E0
009F0D20 009F0EA0 009EF230 009F1420 009F50E0 004158E0 00415970 00811A30 009D8CE0 009D8C60
009D8010 009F0100 009E4330 009DFCB0 009DE2F0 00424C40 00419010 00438AA0 00438B10 00414C60

Packet `cc_ai_sector_scan`, worker `agent/cc-ai-sector-scan`, 2026-09-12 UTC. Ghidra was read-only
for this packet: no renames, comments, prototypes, function creation or saves through the bridge;
the reviewed-name records were added to the sharded ledger, which the integrator applies. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; every live query verified both.
Descriptive names are hypotheses, not recovered symbols.

`docs/SHIP_AI_OBSTACLE_TABLES.md` read `009EB660` partially and named what it could: the entry,
the two hit arms and the bearing tail. Everything between `009EB6B7` and `009EBECC` was unread,
and so was the producer of the neighbour list at `blk+604h` / `blk+608h` that the read half walks.
This packet reads both, plus the producer of the four sector shape fields `009EF230` does not
write.

## Answer

**One sector is one probe, and a probe is either a ray or a turning circle.** The sign of
`sector+4h` decides which (`009EB69F` COMISS against `+0.0f`, `009EB6AE` JBE). A negative radius,
written as `-1.0f`, is the straight probe; a positive one is an arc of that radius whose centre
sits abeam. Both are swept out to `sector+8h`, the braking distance `009EF230` stores, plus a
hysteresis margin the sector only gets while it is already blocked.

**Each probe is tested against two obstacle sources, never one.** First the avoid-zone segment
tree at `blk+0A3Ch` (`004158E0` for the ray, `00415970` for the arc), then every entry of the
neighbour list at `blk+608h`. The neighbours are oriented boxes, not points, and the test is a
clip: the ray's range or the arc's swept bearing is narrowed to the nearest hit, and the index of
whoever narrowed it last is what the sector reports.

**A blocked sector does not record the obstacle, it records a way past it.** `009D84E0` bears the
four corners of the blocking box from the hull, takes the widest corner on each side, adds 25
degrees to whichever side the pair has already agreed on, and returns the corner of the smaller
turn. That corner is `sector+18h`/`+1Ch`, its bearing is `sector+20h`, and which side won is
`sector+28h`. The tail then biases the bearing a further eight degrees away from the obstacle,
scaled down to nothing as the hit approaches ten metres (`009EC20B..009EC237`).

**The list is filled two levels up and aged one slot before the scan reads it.** In the controller
step `009F50E0`: `009F5156` rebuilds the twelve sector shapes (`009E0270`), `009F516C` runs the
throttled pre-pass `009F1420` whose `009F1A25` appends neighbours, `009F51E4` ages and compacts
the list (`009F0EA0`), and `009F51FA` refreshes three sectors (`009EF230`), each of which runs
`009EB660`. The consumer, `009F3F80`, runs from the tail of `009F4DA0` at `009F5248`.

## 1. The entry and the branch, `009EB660..009EB6AE`

`void __thiscall(sector)(blk)`, `RET 4` at `009EC264` and `009EC275`. `ECX` is one `2Ch`-byte
sector (`009EB66C MOV EDI,ECX`) and the stack argument is the control block (`009EB693 MOV
ESI,[EBP+8]`).

```
margin = sector+14h ? GameSettings()+1D8h : 0.0f          ; 009EB66E..009EB696
sector+14h = 0 ; sector+24h = 0                           ; 009EB6A3, 009EB6A7
if (0.0f <= sector+4h) goto arc                           ; 009EB69F COMISS, 009EB6AE JBE
```

The margin is the hysteresis on the blocked latch: a sector that was blocked last frame looks one
settings field further before it clears. It is added to `sector+8h` in both arms (`009EB78C`,
`009EB98F`) and again in the avoid-zone arm at `009EC073`.

## 2. The straight probe, `009EB6B4..009EB929`

```
L = sector+10h ; D = sector+0Ch
if (sector+0h) { origin = pos + L*port ; dir = +forward }  ; 009EB6DF..009EB71F
else           { origin = pos - L*port ; dir = -forward }  ; 009EB725..009EB774
range = sector+8h + margin                                 ; 009EB783..009EB790
probe = origin + D*dir                                     ; 009EB871..009EB89B
```

`pos` is `blk+184h`/`blk+188h` and `forward` is `blk+1ACh`/`blk+1B0h`. The negation is spelled
`-0.0f - v` against the constant at `00D7A208`, so `sector+0h` clear means the astern fan.

The avoid-zone test runs only when `blk+0A3Ch` is non-null (`009EB778`) and the byte `blk+0A24h`
is set (`009EB7AA`). It asks `004158E0` for the last crossing of the segment from `probe` to
`origin + range*dir` with the tree at `blk+0A3Ch`, and on a hit sets the zone flag at `[ESP+0Eh]`
and shortens `range` to the distance from `origin` to the crossing (`009EB827..009EB84D`).

Then the list walk, `009EB85B..009EB91E`:

```
for (i = 0; i < blk+604h; ++i) {
    n = blk+608h[i]
    if (009D8160(n)(probe))      { blocking = i ; break }   ; 009EB8C0, 009EB925
    if (009D80C0(n)(pos))        continue                   ; 009EB8CA
    r = range
    if (009DD540(n)(origin, dir, &r) && r > D) {            ; 009EB8E2, 009EB8F6
        range = r ; blocking = i                            ; 009EB8F8
    }
}
```

`009EB8F6 JBE` restores the previous range when the hit is nearer than `D`, so a hit inside the
sector's own clearance is discarded rather than shortening the probe.

## 3. The arc probe, `009EB92E..009EBEB6`

```
H = [[blk+3FCh]]+50h()                                     ; 009EB939
right = H - pi/2 ; left = H + pi/2                         ; 009EB953, 009EB970 (00CE3C64)
R = sector+4h
arc = min(2R, sector+8h + margin)                          ; 009EB980..009EB9B6
```

Four arms follow, on `sector+0h` (`009EB9DB`) and the sign of `L` against `+0.0f` (`009EB9E1`,
`009EBB9B`, the constant `00D7A218`):

| `sector+0h` | `L` | beam vector | reference bearing | sweep | half angle |
| --- | --- | --- | --- | --- | --- |
| ahead | `>= 0` | `blk+19Ch`/`+1A0h` | `left` | `Sub` | `-D/R` |
| ahead | `< 0` | `blk+1A4h`/`+1A8h` | `right` | `Add` | `+D/R` |
| astern | `>= 0` | `blk+1A4h`/`+1A8h` | `right` | `Sub` | `-D/R` |
| astern | `< 0` | `blk+19Ch`/`+1A0h` | `left` | `Add` | `+D/R` |

All four then compute the same three things, in the same slots:

```
S      = pos + |L| * beam                                  ; 009EBA11, 009EBACE, 009EBBCB, 009EBC84
centre = pos + (|L| + R) * beam                            ; 009EBA5A, 009EBB13, 009EBC10, 009EBCC9
swept  = sweep(reference, arc / R)                         ; 009EBAA1, 009EBB58, 009EBC57, 009EBD0E
probe  = S +/- D * forward                                 ; 009EBB61 adds, 009EBD17 subtracts
```

The reference bearing is the bearing from the centre back to the hull, which is what forces
`blk+19Ch`/`+1A0h` to be the beam vector at `heading - pi/2`: the arm that offsets along it passes
`heading + pi/2` (`009EB9FD`, `009EBA9A`).

The avoid-zone test is the arc version of the same pair of gates: `00415970` with
`this = &blk+0A3Ch`, the centre, `R`, the start bearing `AddWrappedAngle(reference, +/- D/R)`
(`009EBD6B`) and `&swept` (`009EBDAA`). It narrows `swept` and sets the same zone flag.

The list walk, `009EBDBD..009EBEAB`, differs from the straight one in its guard and its distance
test:

```
for (i = 0; i < blk+604h; ++i) {
    n = blk+608h[i]
    if (!n+14h || [n+14h]+5Eh)   continue                   ; 009EBDE2, 009EBDFB
    if (009D8160(n)(probe))      { blocking = i ; break }    ; 009EBE0E, 009EBEB3
    e = swept
    if (009DD010(n)(centre, R, reference, &e)) {             ; 009EBE36
        if (|Sub(e, reference)| * R > D) {                   ; 009EBE51..009EBE7D
            swept = e ; blocking = i
        }
    }
}
```

The bearing change is turned into an arc length by multiplying by `R` before the comparison with
`D`, so both arms use the same clearance in the same units.

## 4. The four geometry callees

| address | ABI | what it does |
| --- | --- | --- |
| `009D80C0` | `char __thiscall(node)(const float* p)`, `RET 4` | `p` inside the box at centre `+20h`/`+24h`, axes `+28h`/`+2Ch` and `+30h`/`+34h`, half extents `+38h`/`+3Ch`. False when `+68h`. |
| `009D8160` | `char __thiscall(node)(const float* p)`, `RET 4` | the same test against centre `+44h`/`+48h` and half extents `+5Ch`/`+60h`, with the **same** axes. False when `+68h` or `+69h`. |
| `009DD540` | `char __thiscall(node)(const float* o, const float* d, float* io)`, `RET 0Ch` | four support-point rejections through `009D8860`, then the four edges of the `+44h` box clipped against the ray by `009D8210`, narrowing `*io`. |
| `009DD010` | `char __thiscall(node)(const float* c, float r, float from, float* io)`, `RET 10h` | the same four corners clipped against the arc, narrowing `*io`. |

That the two containment tests reuse `+28h..+34h` for a box whose corner builders use
`+4Ch..+58h` is what the listing says, not a simplification made here.

## 5. `009D84E0`, the passing corner

`float* __thiscall(node)(float* out, const float* observer, float reference, int other_side,
unsigned char* out_flag)`, `RET 14h` at `009D8858`, body `009D84E0-009D885A`, read whole. It
returns `out` in `EAX` (`009D8843`), which is why `009EBF6C` reads through the return value.

```
if (node+68h) { out = (node+44h, node+48h) ; return }       ; 009D84E9..009D84FB
corner[0..3] = (node+44h,+48h) +/- node+5Ch * (node+4Ch,+50h)
                              +/- node+60h * (node+54h,+58h) ; 009D8509..009D871B
widest_left = -pi (00CE684C) ; widest_right = +pi (00D7A264)
for (i = 0; i < 4; ++i) {                                    ; 009D8720..009D87B8
    b   = pi/2 - atan2(corner[i].z - observer.z,
                       corner[i].x - observer.x), folded      ; 009D873E..009D876D
    rel = SubtractWrappedAngle(b, reference)                  ; 009D877B
    if (rel > widest_left)  { widest_left = rel ; li = i }
    if (rel < widest_right) { widest_right = rel ; ri = i }
}
turn_left  = |widest_left|  + (other_side == 1 ? 25deg : 0)   ; 009D87FA (00D1F6C0)
turn_right = |widest_right| + (other_side == 2 ? 25deg : 0)   ; 009D880F
if (turn_right < turn_left) { *out_flag = 0 ; out = corner[ri] }  ; 009D8838
else                        { *out_flag = 1 ; out = corner[li] }  ; 009D882B
```

`009EBF83` turns the flag into `sector+28h`: `flag ? 2 : 1` (`009EBF7D SETNZ`, `009EBF80 ADD 1`).
The 25-degree bias is a penalty on the side named by `other_side`, so a node carrying `1` pushes
this ship toward `1` as well, which is what makes `009F3F80`'s disagreement test at `009F475B`
meaningful.

## 6. The two arms and the tail, `009EBEBB..009EC275`

```
if (blocking >= 0) {                                         ; 009EBEBB
    sector+14h = 1 ; sector+24h = blk+608h[blocking]          ; 009EBECC, 009EBEDB
    raise n+78h to GameSettings()+194h + 1.0                  ; 009EBEDE..009EBF0A (00D7A210)
    H = [[blk+3FCh]]+50h() ; if (!sector+0h) H += pi          ; 009EBF1A, 009EBF39 (00D7A264)
    corner = 009D84E0(n)(&out, &pos, H, n+88h, &flag)         ; 009EBF67
    sector+18h/+1Ch = corner ; sector+28h = flag ? 2 : 1      ; 009EBF6E, 009EBF7A, 009EBF83
    sector+20h = bearing(corner - pos)                        ; 009EBF86..009EBFDF
} else if (zone_flag) {                                       ; 009EBFE4
    sector+14h = 1                                            ; 009EBFEF
    H = [[blk+3FCh]]+50h() ; if (!sector+0h) H += pi          ; 009EBFFE, 009EC01D
    q = { pos, direction(H), 5.0f, 5.0f, sector+8h + margin,
          _, blk+0A38h }                                      ; 009EC077..009EC0BB (00CE3850)
    if (009DC2E0(&blk+0A24h)(&q)) {                           ; 009EC0C1
        sector+20h = q.bearing                                ; 009EC0DE
        sector+18h/+1Ch = pos + q.range * direction(q.bearing) ; 009EC11B..009EC158
    } else {
        sector+20h = H                                        ; 009EC1A3
        sector+18h/+1Ch = pos + q.range * direction(H)         ; 009EC15D..009EC1A0
    }
} else return;                                                ; 009EBFE9
; 009EC1A8
if (!sector+14h) return
d    = |sector+18h/+1Ch - pos|                                ; 009EC1B2..009EC201
bias = InterpolateClamped(10.0f, 0.0f, 150.0f, 8deg, d)       ; 009EC235
sector+20h = flag ? Add(sector+20h, bias) : Sub(sector+20h, bias)  ; 009EC256, 009EC267
```

`[ESP+0Fh]`, the side flag, is cleared at `009EBEC1`, so every avoid-zone hit takes the
subtracting branch. The bias constants are `00CE38B8` (10.0f), `00CE3808` (150.0f) and `00D20A18`
(0.13962634f, eight degrees).

The bearing convention appears six times and is one rule: a bearing is
`fold(pi/2 - atan2(dz, dx))` and a direction is `(cos(fold(pi/2 - b)), sin(fold(pi/2 - b)))`, with
the doubles `00CE3830` (pi/2) and `00CE3828` (2 pi) and a single conditional add, not a loop.

## 7. `009E0270`, the twelve sector shapes

`void __thiscall(blk)(float)`, `RET 4`, body `009E0270-009E04D9`. Called from the controller step
at `009F5156` and from the brain construct `009E4330` at `009E46A9`. `009E02FA LEA EAX,[ESI+818h]`
is `sector[0]+10h`, the outer loop steps `0x108` (six strides) and runs twice (`009E048B`,
`009E0490`), and the six inner writes are the six rudder buckets `009F3F80` indexes with
`group * 6 + bucket`.

With `r = 00811A30(unit)(0.5f)` (`009E02D7`, the float at `00CE3800`), `w = unit+9CCh` the hull
half width and `E = blk+3E4h = unit+9C8h * 0.45` (`009E44DB`, the double at `00CF1748`):

| bucket | `+0h` | `+4h` radius | `+0Ch` clearance | `+10h` lateral |
| --- | --- | --- | --- | --- |
| 0 | `group == 0` | `r` | `E / 10` | `w / 2.2` |
| 1 | `group == 0` | `2.5 r` | `E / 5` | `w / 4` |
| 2 | `group == 0` | `-1.0f` | `E / 2` | `w / 2.2` |
| 3 | `group == 0` | `-1.0f` | `E / 2` | `-w / 2.2` |
| 4 | `group == 0` | `2.5 r` | `E / 5` | `-w / 4` |
| 5 | `group == 0` | `r` | `E / 10` | `-w / 2.2` |

The doubles are `00CE3DE0` (2.5), `00D05AC8` (2.2), `00CE3DC0` (10.0), `00D7A348` (0.25),
`00D7A370` (5.0), `00D7A280` (0.5) and the float `00D7A260` (-1.0f). Group 0 is the ahead fan and
group 1 the astern fan (`009E0334 TEST`, `009E0338 SETZ`). Buckets 2 and 3 are the two straight
probes, one off each beam; the outer four are turning circles of two radii. Because the arm in
`009F3F80` tests `sector[index]` and `sector[index+1]`, bucket 5 exists only as bucket 4's partner.

## 8. `009EF230`, which three sectors refresh and how far they look

`void __thiscall(blk)(void)`, `RET`, body `009EF230-009EF347`.

```
v     = max(0092D730([unit+1018h]), class+500h * 0.1)        ; 009EF242..009EF283 (00D7A3A0)
u     = v + 3.0                                              ; 009EF28F (00D7A2B0)
brake = (u / class+508h) * u * 0.55 + unit+9C8h * 0.6        ; 009EF29F..009EF2D6 (00CEC8F0, 00CEFF98)
c      = blk+0A18h
group  = c >= 2 ; parity = c % 2 != 0                        ; 009EF2C9..009EF2E7
blk+0A18h = c < 3 ? c + 1 : 0                                ; 009EF2E9..009EF2FD
start  = parity + group * 6                                  ; 009EF308..009EF30B
for (k = 0; k < 3; ++k) {                                    ; 009EF312..009EF33F
    s = &blk[808h + (start + 2k) * 2Ch]
    s+8h = brake ; 009EB660(s)(blk)                           ; 009EF32F, 009EF334
}
```

so counter 0 refreshes sectors 0, 2 and 4, counter 1 refreshes 1, 3 and 5, counter 2 refreshes 6,
8 and 10 and counter 3 refreshes 7, 9 and 11: the twelve over four frames, three a frame.
`docs/SHIP_AI_OBSTACLE_TABLES.md` documented this; it is now projected.

## 9. The neighbour list at `blk+604h` / `blk+608h`

**It is inline, not a pointer.** `blk+604h` is the count and `blk+608h` an array of pointers with
capacity `0x80` (`009F0D39 CMP against 0x80`), so it occupies `blk+608h..blk+807h` and ends
exactly where the twelve sectors begin. The cursor is `LEA EAX,[ESI+608h]` stepped by four
(`009EB8A5`, `009EB8AF`, `009EB90C`; `009EBDD2`, `009EBDDC`, `009EBE99`).

**The node is `0x90` bytes**, `operator new(0x90)` at `009F0E2A`, constructed by `009E52E0`
(`009E52E0-009E53A5`) or `009E53B0`. The fields this packet established:

| offset | what | evidence |
| --- | --- | --- |
| `+14h` | the observed unit; `[+14h]+5Eh` is its gone flag | `009E52FA` writes, `009EBDE2`, `009D8B96` |
| `+18h` | the observing control block | `009E5308` |
| `+20h`/`+24h` | the near box centre | `009D80D3`, `009D80DC` |
| `+28h`/`+2Ch`, `+30h`/`+34h` | the beam and forward axes both boxes are tested along | `009D80E3`, `009D811D`, `009D818D`, `009D81C7` |
| `+38h`/`+3Ch` | the near box half extents | `009D8111`, `009D813A` |
| `+44h`/`+48h` | the avoid box centre, also the position `009D8B90` measures from | `009D817D`, `009D84ED`, `009D8BDE` |
| `+4Ch`/`+50h`, `+54h`/`+58h` | the axes the corner builders use | `009D8537`, `009D8519` |
| `+5Ch`/`+60h` | the avoid box half extents | `009D81BB`, `009D81E4`, `009D8514` |
| `+68h`, `+69h` | no pose, no arc | `009D80C3`, `009D816D`, `009DD043`, `009DD04C` |
| `+78h` | the lifetime, in seconds | `009E540C` seeds, `009F1009` ages, `009EBEF7` and `009F0DFF` raise |
| `+88h` | the agreed passing side | `009EBF51` reads, `009D912F` is the only non-zero writer |

**`009F0D20` appends.** `void __thiscall(blk)(unit* candidate)`, `RET 4`, body
`009F0D20-009F0E83`, sole call site `009F1A25`. After three admission predicates it walks the list
for a node already holding the candidate; a match has its lifetime raised to
`GameSettings()+194h + 1.2` (`009F0DCD`, the double at `00CEC160`) and nothing is appended.
Otherwise a new node is constructed and appended and the count is incremented (`009F0E62`,
`009F0E69`).

**The candidates are the world's unit list.** `BSP_ShipAi_BrainPrePass` (`009F1420`) walks the
linked list at `[[00E188A8]+19CCh]`, count `+60h`, head `+64h`, payload `+8h`, next `+4h`
(`009F1877..009F18B8`, `009F1A33`), skipping itself (`009F18BB`). Two filters decide who reaches
`009F0D20`:

```
if (|other+100h - self+100h| >= 15.0) skip                   ; 009F18ED..009F1933 (00CF3F20)
radius = (self+9C8h + other+9C8h) * 0.5
       + max((self_class+500h + other_class+500h) * settings+19Ch,
             settings+198h)                                  ; 009F1987..009F19F1
if (|self_xz - other_xz|^2 <= radius^2) 009F0D20(blk)(other) ; 009F19FF..009F1A25
```

so it is an altitude band of fifteen metres and a closing-distance radius, not a spatial index
query and not one of the recon slot triples of `docs/RECON_SLOT_LISTS.md`. The pre-pass itself is
throttled: `009F515B` runs it only when the accumulator `ai+0B18h` reaches `ai+0B14h`.

**`009F0EA0` ages and compacts.** `void __thiscall(blk)(float dt)`, `RET 4`, body
`009F0EA0-009F115D`, sole call site `009F51E4`, one slot before the sector refresh.

```
for (i = 0, kept = 0, removed = 0; i < blk+604h; ++i) {
    n = blk+608h[i]
    n+78h -= dt                                              ; 009F1009..009F1018
    if (n+78h <= 0) { 0064A610(n) ; free(n) ; ++removed ; continue }  ; 009F1121, 009F1127
    if (!n+14h || [n+14h]+5Eh) n+68h = 1                     ; 009F1029..009F110A
    else { 009EAE20(n)(&dir) ; 009EAFC0(n)(...) }            ; 009F104D, 009F10FF
    if (removed) blk+608h[i - removed] = n                   ; 009F1112..009F1116
}
blk+604h -= removed                                          ; 009F114E
```

`009EAFC0` is given the clamped hull velocity (`009F0F73`, the floor `00D7A23C` = 0.001f, the cap
`class+500h * settings+1B8h`), `blk+1BCh`/`blk+1C0h`, `unit+9C8h * 0.55` and `unit+9CCh * 0.75`
(`009F0FCB..009F0FEF`, the doubles `00CEC8F0` and `00CEC9D8`). It and `009EAE20` are the two box
refreshes; neither was read.

## Host methods the executable must implement, in call order

| # | method | native call site | callee |
| --- | --- | --- | --- |
| 1 | `settings_blocked_margin_1d8` | `009EB681` | `00424C40`, then `+1D8h` |
| 2 | `unit_heading_vtable50` | `009EB939` | indirect, `[[blk+3FCh]]+50h` |
| 3 | `avoid_zone_segment_crossing_004158e0` | `009EB819` | `004158E0` |
| 4 | `point_in_avoid_box_009d8160` | `009EB8C0` | `009D8160` |
| 5 | `point_in_near_box_009d80c0` | `009EB8CA` | `009D80C0` |
| 6 | `clip_ray_against_node_009dd540` | `009EB8E2` | `009DD540` |
| 7 | `clip_arc_against_avoid_zones_00415970` | `009EBDAA` | `00415970` |
| 8 | `point_in_avoid_box_009d8160` | `009EBE0E` | `009D8160` |
| 9 | `clip_arc_against_node_009dd010` | `009EBE36` | `009DD010` |
| 10 | `settings_neighbour_memory_194` | `009EBEDE` | `00424C40`, then `+194h` |
| 11 | `raise_node_lifetime_78` | `009EBEF7` | inlined, no CALL |
| 12 | `unit_heading_vtable50` | `009EBF1A` | indirect |
| 13 | `unit_heading_vtable50` | `009EBFFE` | indirect |
| 14 | `avoid_zone_free_bearing_009dc2e0` | `009EC0C1` | `009DC2E0` |

`00414C60`, `00419010`, `00438AA0` and `00438B10` are already reconstructed and are called
directly (`009EB848`, `009EC235`, `009EC256`, `009EC267`), not through the host. `009D84E0` is
projected rather than hosted, so it is not in the table.

`tools/verify_report_calls.py` checked all 55 call rows of `reports/ship_ai_sector_scan.json`
against the live function bodies and the call graph: 0 failed. The four indirect rows are reported
as indirect and not checked.

## Coverage

| routine | coverage |
| --- | --- |
| `009EB660` | complete: `009EB660..009EC275` projected operation for operation by `ship_ai_scan_obstacle_sector_009eb660` and `ship_ai_sector_probe_009eb660`. The five geometry callees stay behind host methods. |
| `009D84E0` | complete |
| `009EF230` | complete |
| `009E0270` | partial: `009E0328..009E0493`, the twelve-shape loop. `009E0270..009E0327` (`blk+168h`, `blk+3C4h`, the `009DE2F0` call) and `009E0499..009E04D7` are not projected |
| `009F0D20` | partial: `009F0D39..009F0D43` and `009F0DC8..009F0E69`. The three admission predicates `009F0D49..009F0DBF`, which call the entity vtable slot `+5Ch` with the literal 8 and `00827F70`, are not projected |
| `009F0EA0` | partial: `009F0FD3..009F1152`. The head `009F0EA9..009F0FCF` and the two box refreshes are not projected |
| `009F1420` | read in one slice only, `009F1861..009F1A3E`, the candidate loop. Nothing was named or projected; `docs/SHIP_AI_GOAL_VECTOR.md` owns the routine |
| `009D8160`, `009D80C0`, `009DD540`, `009DD010` | read, not projected: they are host methods, and section 4 is their contract |
| `009E52E0` | read, not projected: it is the evidence for the node layout |
| `009DC2E0` | not read past its call site. Only the query block and the bearing read back are established |
| `004158E0`, `00415970` | read, not projected; not named either, because three other packets call them |
| `009F0100`, `009D8C60`, `009D8010`, `009D8CE0` | read only far enough to settle who writes `node+88h` (the fourth correction). Not named, not projected |

`src/ship_ai_sector_scan.cpp` builds clean under `/W4 /WX` for Win32 and the existing
`reconstructed_math` test still passes. No new test case was added: every routine here is a
projection whose evidence is the listing, and the four math helpers it calls already have coverage.

## Run-time evidence

**None new, and none available yet.** `docs/GAME_EXECUTABLE.md` milestone 2p already records that
`009EF230` runs 15680 times in the validation run and marks nothing, because the neighbour list is
empty in that process. This packet supplies the reason rather than a new run: nothing in
`bsp_game.exe` reaches `009F1A25`, the sole append site, and the avoid-zone object
`blk+0A24h`/`blk+0A3Ch` is the scan's only other input. Wiring either is a milestone in the
executable's own packet. `src/ship_motion_probe.cpp` and the game executable sources belong to
`cc-exe-2q` and were not touched.

## Corrections

| what said it | what is true | evidence |
| --- | --- | --- |
| `docs/SHIP_AI_OBSTACLE_TABLES.md` sector table: `+4h` is the "half width of the probe" | `+4h` is the probe's turn radius, and `-1.0f` selects a straight probe instead of an arc | `009EB69F` COMISS against `+0.0f` with `009EB6AE` JBE picks the two arms; `009EBA84` divides the clearance by it and `009EBA8A` divides the arc length by it; `009E034A` and `009E03B1` write `00D7A260` = `-1.0f` into buckets 2 and 3 |
| the same table: `+0Ch` is the "reach" | `+0Ch` is a small longitudinal clearance, `E/10`, `E/5` or `E/2` of `blk+3E4h` = `unit+9C8h * 0.45`. It places the probe point, starts the avoid-zone arc off the beam bearing, and is the minimum travel a hit must exceed | `009E0345`, `009E037B`, `009E03AC`; `009E44DB` FMUL against the double at `00CF1748`; `009EB7B8`, `009EBB6E`, `009EBD24`; `009EBA7F`, `009EBD59`; `009EB8EB`, `009EBE72` |
| the same table: `+10h` is the "lateral offset", `+0h` is the "kind byte" | confirmed and made precise: `+10h` is `+/- w/2.2` or `+/- w/4` and its sign picks the beam vector; `+0h` non-zero is the ahead fan, zero the astern fan | `009E0336`, `009E036C` and the four FCHS siblings; `009EB9E1` and `009EBB9B` against `00D7A218`; `009E0338` SETZ; `009EB725` negates the forward vector and `009EBD38` subtracts the clearance |
| the same doc: "`[sector+24h]+88h` is the passing side the other ship picked and `sector+28h` is the one this ship picked, **both written by the same 009D84E0 side flag in 009EB660**" | `009EB660` never writes `node+88h`. It reads it at `009EBF51` and hands it to `009D84E0` as the side to bias against. The only non-zero writer in the image is `009D912F` in `BSP_ShipAi_PredictTrackCrossing`, which in one arm copies the other party's `+88h` (`009D90C7`); every other writer clears it | a whole-image scan of the `C7 /0` and `89 /r` forms with the disp32 `0x88`. The clearing writers are `009E5364`, `009E5417`, `009D803A`, `009D8086`, `009D8C80`, `009F019F`, `009F06EC`, `009F073C`, `009F09C7`. `009D8C60`'s non-zero arm builds a message with `009D66B0` and routes it with `BSP_Session_RouteMessage`, so the value is replicated rather than assigned locally |
| the same doc, uncertainty 5: "`009EB660` was read partially. The sector fields `+0h`, `+4h`, `+0Ch` and `+10h` are named from how the unread geometry uses them and are the weakest names in this doc" | closed. Their producer is `009E0270`, section 7 | `009E02FA LEA EAX,[ESI+818h]`, the `0x108` outer step at `009E048B` and the two iterations at `009E0490` |
| `docs/GAME_EXECUTABLE.md` milestone 2p follow-up 6: "the neighbour list `blk+604h` / `blk+608h`" was listed as unknown | the list is inline in the control block, capacity `0x80` pointers spanning `blk+608h..blk+807h`. Producer `009F0D20` from `009F1420` at `009F1A25`, aged by `009F0EA0` at `009F51E4`, candidates from the world unit list at `[[00E188A8]+19CCh]` | `009F0D39`; `009F1877..009F18B8`; `009F1929` against the double 15.0 at `00CF3F20`; `009F1987..009F1A1F` |

## Uncertainties

1. Which of `blk+19Ch`/`+1A0h` and `blk+1A4h`/`+1A8h` is the port beam vector is inferred, not
   read. The arm that offsets the turn centre along `blk+19Ch`/`+1A0h` passes `heading + pi/2` as
   the bearing from the centre back to the hull (`009EB9FD`, `009EBA9A`), which forces
   `blk+19Ch`/`+1A0h` to point at `heading - pi/2`. Their producer, `009DE2F0` at `009DE462` with
   defaults in `009DFCB0`, was not read.
2. `00811A30` returns a class curve `0082E960` evaluated at the rudder fraction `0.5f`, divided by
   the gameplay modifier product for category 5 (`008E6430`). That the sector uses the result as
   the radius of a turning circle is what the geometry needs; `0082E960` was not read, so "turn
   radius" is a hypothesis about the curve.
3. The node's two boxes share the axis pair `+28h..+34h` in the containment tests while the corner
   builders use `+4Ch..+58h` for the second box. Whether `009EAE20` and `009EAFC0` keep the two
   pairs identical was not checked; neither was read.
4. What the passing side codes 1 and 2 name in absolute terms is not settled. `009D84E0` penalises
   the side matching its argument by 25 degrees, so a node carrying 1 pushes this ship toward 1,
   and `009F3F80` backs off when the node's `+88h` differs from the sector's choice.
5. `009DC2E0`'s query block is described by what `009EB660` writes into it and reads back. The two
   `5.0f` at block `+10h`/`+14h` and the context pointer at block `+20h` (`blk+0A38h`) are passed
   through unread.
6. `blk+0A24h` is treated as an inline avoid-zone object whose first byte is an enable and whose
   `+18h` (`blk+0A3Ch`) is the head of a segment tree. That is from its three uses here; the object
   itself was not read.

## Follow-up packets

| packet | addresses and files | what is left |
| --- | --- | --- |
| `ship_ai_neighbour_box_refresh` | `009EAE20`, `009EAFC0`, `009F0EA0` `009F0EA9..009F0FCF`, node `+20h..+60h` | How the two oriented boxes are built each frame from the hull pose, the clamped velocity, `unit+9C8h * 0.55` and `unit+9CCh * 0.75`. This packet established the layout from the readers; the writers are the producer. |
| `ship_ai_avoid_zone_object` | `blk+0A24h`, `blk+0A38h`, `blk+0A3Ch`, `009DC2E0`, `004158E0`, `00415970`, `009D57E0`, `00423190`, `009DE5B0` | The avoid-zone segment tree: who fills it, what the node `+10h`/`+18h`/`+1Ch` links mean, and what `009DC2E0` searches for. Three packets outside this one already call the two helpers. |
| `ship_ai_passing_side_negotiation` | `009D8CE0`, `009F3E30`, `009D8C60`, `009D66B0`, `009D8010`, `009F0100`, `009F4D10`, node `+75h`, `+8Ch` | Who decides the passing side, how the session message replicates it, and what codes 1 and 2 name. Only far enough was read here to correct who writes `node+88h`. |
| `ship_ai_hull_frame_vectors` | `009DE2F0`, `009DFCB0`, `blk+184h..blk+1C0h`, `blk+3E4h`, `blk+3C4h` | The pose block every ship AI routine reads: the three direction pairs, which of the two beam vectors is port, and the two length scales `009E0270` divides. |
| `ship_ai_turn_radius_curve` | `00811A30`, `0082E960`, `008E6430`, `class+500h`, `class+508h` | What the class curve at `0082E960` measures and how the gameplay modifier category 5 scales it. Four other ship AI routines call `00811A30`. |

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| none | | Every routine this packet read or named has a Ghidra function whose body range the bridge reports: `009EB660-009EC277`, `009D84E0-009D885A`, `009D80C0-009D8156`, `009D8160-009D8200`, `009DD010-009DD530`, `009DD540-009DD9BA`, `009DC2E0-009DCEA2`, `009E0270-009E04D9`, `009E52E0-009E53A5`, `009F0D20-009F0E83`, `009F0EA0-009F115D`, `009F1420-009F1BBA`, `009EF230-009EF347`, `004158E0-00415963`, `00415970-00415D6E`, `00811A30-00811AAA`. `tools/verify_report_calls.py` checked all 55 call rows of `reports/ship_ai_sector_scan.json`: 0 failed. |

One **flow gap** does need the integrator, and it is not a missing function. `python tools/bsp.py
ghidra flow 009f0ea0` reports ten undisassembled bytes at `009F112C..009F1136`, after the
`CALL 00BF65AC` at `009F1127` that Ghidra's non-returning discovery marked `CALL_RETURN`. The disk
bytes are `83 c4 04 83 c5 01 89 6c 24 1c`: `ADD ESP,4; ADD EBP,1; MOV [ESP+1Ch],EBP`, the increment
of the removal count that the compaction at `009F1112` and the count fixup at `009F114E` both
depend on. The projection uses them; the stored listing does not have them.
`python tools/ghidra_flow_repair.py 009f0ea0 --apply` is the fix.
