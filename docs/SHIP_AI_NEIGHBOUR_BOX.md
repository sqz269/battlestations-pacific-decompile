# The neighbour node's two boxes: 009EAE20 and 009EAFC0

Addresses: 009EAE20 009EAFC0 009EB4D1; read as evidence 009F0EA0 009E52E0 009F0D20 0098A8E0 0092D730
0080FC30 00811940 00812090 00414DB0 00424C40 00414C60 00419260 00415510 00415550 00415620
00438AA0 006BC0C0 006D1E30 006DFD60 009D80C0 009D8160 009DD010 009DD540 009D84E0 009D8860
009D8A30 009D8B90 009EF350 009DCEB0 009D8CE0.

Packet `cc_ai_box_refresh`, worker `agent/cc-ai-box-refresh`, 2026-09-12 UTC. Ghidra was
read-only for this packet: no renames, comments, prototypes, function creation or saves through
the bridge; the two reviewed names and the two reconstruction records went into the sharded
ledger, which the integrator applies. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; every live query verified both. Descriptive names are hypotheses,
not recovered symbols.

`docs/SHIP_AI_SECTOR_SCAN.md` left these two as "the two box refreshes; neither was read", and
`docs/SHIP_AI_AVOIDANCE_REQUEST.md` read only `009EAFC0`'s first 39 bytes. Both bodies are read
whole here.

## Answer

**A neighbour node carries two oriented boxes, and these two routines are the only writers of
either.** `009EAE20` builds the **near box**: where the other ship is now, swept forward over a
short lookahead. `009EAFC0` builds the **avoid box**: where the other ship will be by the time
this ship gets there, shrunk in proportion to how far ahead the guess reaches. The sector scan
clips its probes against the near box for "am I already inside it" and against the avoid box for
everything else (`009D80C0` takes `+20h..+3Ch`, `009D8160`, `009DD010`, `009DD540`, `009D84E0`,
`009D8860` and `009D8A30` take `+44h..+60h`).

**The near box is the other ship's own hull box, not a point.** Its centre is the other hull's
world position pushed along its own heading by `min(NearbyShip_PosSpeedCorrig * speed,
length * 0.25)`, its half extents are `length * 0.55 + |advance|` across the bow and
`beam * 0.60` abeam. The advance is a **`min`, not a clamp**: a ship making sternway has a
negative body-axis speed, so the centre moves astern and the along-track extent still grows.

**The avoid box is a dead-reckoned guess with three shapes.** When the other ship is close, or
the arithmetic says there is no time to project, or the projection collapses, the avoid box is
literally the near box, field for field (`009EB2C9`). Otherwise the other ship is run forward by
`speed * time * NearbyShip_EstPos_ShipSpdMul`, capped, and the box is placed at the end of that
run: along a straight line when the projected turn is under three degrees, and around a turning
circle of radius `|travel / turn|` otherwise.

**`node+69h` is not about the geometry at all.** `009EAFC0` stores it from its last argument
(`009EB040`), which is the inlined party filter's answer in `009F0EA0` — whether this ship is
allowed to avoid that one. The one place the byte is set for a geometric reason is the destroyed
entity arm at `009EAFD9`, where it is set together with `node+68h`.

**The two boxes do not always share an axis pair.** `009D8160` tests the avoid box along
`+28h..+34h` while `009D84E0` and the three clip helpers build its corners from `+4Ch..+58h`.
`docs/SHIP_AI_SECTOR_SCAN.md` uncertainty 3 asked whether the producer keeps the two pairs
identical. It does on the near-box copy tail (`009EB2D7..009EB2EC`) and on the straight arm
(`009EB4B2..009EB4C7`), and it does **not** on the arc arm: `009EB505..009EB51B` writes the
rotated axis into `+4Ch..+58h` and leaves `+28h..+34h` at the unrotated heading. A turning
neighbour is therefore tested for containment against one orientation and clipped against
another.

## 1. `009EAE20`, the near box

`void __thiscall(node)(const float* self_velocity_xz)`, `RET 4` at `009EAFB2`, body
`009EAE20-009EAFB4`, read whole. Sole call site `009F104D`.

**The argument is never read.** `009F1046 LEA ECX,[ESP+30h]; 009F104A PUSH ECX` hands over the
caller's floored hull velocity pair. The body's frame is `SUB ESP,1Ch; PUSH ESI` (and `PUSH EDI`
at `009EAE82`), so the argument sits at `[ESP+24h]`, later `[ESP+28h]`; the largest displacement
any instruction in the body touches is `+1Ch`. Nothing reads it.

```
unit = node+14h
if (!unit || unit+5Eh) return                                 ; 009EAE2B, 009EAE3B
node+40h = unit->vtable[50h]()                                ; 009EAE46, 009EAE48
speed    = 0092D730(unit+1018h)                               ; 009EAE54, signed
a = pi/2 - node+40h ; if (a < 0) a += 2*pi                    ; 009EAE60..009EAE7A
node+28h = cos(a) ; node+2Ch = sin(a)                         ; 009EAE87/9D, 009EAE97/A8
node+30h = node+2Ch ; node+34h = -0.0f - node+28h             ; 009EAEB8, 009EAEC0
advance = min(GameSettings()+1A8h * speed, unit+9C8h * 0.25)  ; 009EAECA..009EAF07
if (unit+C8h == 0) 00414DB0(unit)                             ; 009EAF07, 009EAF30
node+20h = unit+FCh  + advance * node+28h                     ; 009EAF5A, 009EAF7F
node+24h = unit+104h + advance * node+2Ch                     ; 009EAF73, 009EAF86
node+38h = unit+9C8h * 0.55 + |advance|                       ; 009EAF8C..009EAF9C
node+3Ch = unit+9CCh * 0.60                                   ; 009EAF9F..009EAFAB
```

The doubles are `00CE3830` = pi/2, `00CE3828` = 2 pi, `00D7A348` = 0.25, `00CEC8F0` = 0.55,
`00CEFF98` = 0.60; `00D7A208` is `-0.0f`, used as a negation. The bearing fold at
`009EAE60..009EAE99` is `006BC0C0` inlined: the same `pi/2 - h`, the same single conditional
`+2 pi`, the same `FCOS`/`FSIN` pair.

`unit+C8h` is the pose-valid byte `00414DB0` tests the same way (`docs/SHIP_AI_HULL_GEOMETRY.md`,
`docs/SHIP_AI_AVOIDANCE_REQUEST.md`); the translation at `unit+FCh`/`+104h` is read only after
the refresh (`009EAF35`, `009EAF45`), which is why the projection makes it a host method rather
than an input.

The settings row is `ShipAvoidance.NearbyShip_PosSpeedCorrig` (`docs/SHIP_AI_SETTINGS_BLOCK.md`,
authored 2, installed 0.5), so the lookahead is half a second of travel at the installed value,
capped at a quarter of the other ship's length.

## 2. `009EAFC0`, the entry, the cached bounds and the side flag

`void __thiscall(node)(nine stack dwords)`, `RET 24h` at `009EAFE5`, `009EB305`, `009EB4CE`,
`009EB60E` and `009EB64F`; bytes `009EAFC0-009EB651`, read whole. Sole call site `009F10FF`.

**Ghidra splits the arc arm out.** `009EB4D1-009EB610` is a separate Ghidra function,
`FUN_009EB4D1`, with **zero callers**, and `009EAFC0`'s own function body excludes it. It is not a
routine: `009EB470 JBE` is the only way in, `009EB5FA..009EB60E` is `009EAFC0`'s own epilogue
(`POP EDI`, `POP EBX`, `POP ESI`, `ADD ESP,20h`, `RET 24h` against `009EAFC0`'s frame), and the
frame arithmetic only balances across the two: `009EB4D1 SUB ESP,8` plus `009EB4E7 PUSH ECX` is
repaid by `00438AA0`'s `RET 8` and `006BC0C0`'s `RET 4`, which is what puts `ESP` back at
`entry - 2Ch` for that epilogue. It matters for attribution: `get_function_by_address` of
`009EB4DE` and `009EB4F6` answers `009EB4D1`, so those two call sites are reported against
`009EB4D1` and not against `009EAFC0`
(`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 2). Merging the two would be a Ghidra edit; this
packet made none.

The nine arguments, read off `009F10A0..009F10FA` (`PUSH EDX` + `SUB ESP,18h` + `SUB ESP,8` is
exactly the `24h` the `RET` cleans):

| arg | slot | built at | value |
| --- | --- | --- | --- |
| 0 | `[ESP+30h]` | `009F10EE` | `[blk+3FCh]+FCh`, this hull's world x |
| 1 | `[ESP+34h]` | `009F10FA` | `[blk+3FCh]+104h`, this hull's world z |
| 2 | `[ESP+38h]` | `009F10C0` | this hull's velocity x, floored |
| 3 | `[ESP+3Ch]` | `009F10D2` | this hull's velocity z, floored |
| 4 | `[ESP+40h]` | `009F10E8` | `[blk+3FCh]+9C8h * 0.55` (`009F0FCB`, `00CEC8F0`) |
| 5 | `[ESP+44h]` | `009F10DD` | `[blk+3FCh]+9CCh * 0.75` (`009F0FE3`, `00CEC9D8`) — **never read** |
| 6 | `[ESP+48h]` | `009F10CA` | `blk+1BCh`, this hull's cached world maximum Y |
| 7 | `[ESP+4Ch]` | `009F10B4` | `blk+1C0h`, this hull's cached world minimum Y |
| 8 | `[ESP+50h]` | `009F10B0` | the party filter's answer, low byte |

Displacements are given in the routine's `PUSH EBX; PUSH EDI` epoch (`ESP = entry - 2Ch`); before
`009EB05D` and after `009EB474` they are 8 lower. Argument 5 has no reader in either epoch: the
only `[ESP+44h]` operands in the body are `009EB04F` (the pre-push epoch, argument 7),
`009EB438` (a `LEA` with `ESP = entry - 30h`, argument 4) and `009EB479` (the post-pop epoch,
argument 7's slot reused as the signed travel).

```
unit = node+14h
if (!unit || unit+5Eh) { node+68h = 1 ; node+69h = 1 ; return }   ; 009EAFCB..009EAFDE
if (unit->vtable[20h]()) {                                        ; 009EAFED, 009EAFF1
    0098A8E0(unit->vtable[20h](), &minimum, &maximum)             ; 009EB005, 009EB009
    node+80h = maximum.y ; node+84h = minimum.y                   ; 009EB014, 009EB022
}
node+69h = (arg8 == 0)                                            ; 009EB02A..009EB040
if (node+84h > arg6 || arg7 > node+80h) {                         ; 009EB03C/43, 009EB053/57
    node+68h = 1                                                  ; 009EB619
    node+44h = node+20h ; node+48h = node+24h                     ; 009EB61D, 009EB623
    node+5Ch = node+60h = 1.0f                                    ; 009EB629, 009EB62E
    node+84h = node+80h = -1000.0f                                ; 009EB63B, 009EB643
    return
}
```

`0098A8E0` copies `[model+13Ch..144h]` into the first buffer and `[model+148h..150h]` into the
second (`docs/SHIP_AI_HULL_GEOMETRY.md` calls them minimum and maximum), so `node+80h` is the
other hull's world **maximum** Y and `node+84h` its **minimum**. `-1000.0f` (`00D7A240`) is the
same sentinel `009E5380`/`009E5388` seed the node with, so the far arm puts the node back to
"bounds unknown". The gate is a plain vertical overlap: a submarine deep under a destroyer is not
a traffic problem.

The vtable is `00CFC3D0`. Slot `+20h` is `006D1E30`, `MOV EAX,[ECX+360h]; RET` — the same model
getter `009DE2F0` calls twice (`docs/SHIP_AI_HULL_GEOMETRY.md`). Slot `+34h` is `00812090`,
which writes the body axis `(unit+94h, +98h, +9Ch)` scaled by `0092D730(unit+1018h)` into the
caller's buffer and returns it: the world velocity. Slot `+50h` is `006DFD60`,
`FLD dword ptr [ECX+1050h]; RET`: the hull heading. Two independent documents already place the
unit at this table (`docs/CRUISE_COMMAND.md` for `+50h`, the hull geometry packet for `+20h`),
which is the evidence for reading the concrete bodies here; a subclass could still override a
slot.

## 3. `009EAFC0`, the projection

Everything below runs with `EDI = GameSettings() + 180h` (`009EB081`, `009EB087`).
`docs/SHIP_AI_SETTINGS_BLOCK.md` section 2 has the key names and the loader sites; this packet
re-checked only the displacements.

```
node+5Ch = node+60h = 1.0f ; node+68h = 0                     ; 009EB064..009EB076  (both floats dead)
gap = |(node+20h,node+24h) - (arg0,arg1)| - node+38h - arg4   ; 009EB07A..009EB0B6
if (gap <= arg4 * s+1A4h) goto near                           ; 009EB0BA..009EB0CD

dir = normalise((node+20h,node+24h) - (arg0,arg1))            ; 009EB0D3..009EB115  (00419260)
dv  = (arg2,arg3) - unit->vtable[34h]()                       ; 009EB119..009EB140
own = |(arg2,arg3)|                                           ; 009EB144, ECX still &arg2
closing = max(1.0f, dv . dir + s+1D0h)                        ; 009EB14D..009EB182
arrive  = max(s+1A0h, (arg4 * s+1A4h) / closing)              ; 009EB188..009EB1A0
excess  = gap / closing - arrive                              ; 009EB1A4..009EB1C6
slack   = gap - closing * arrive                              ; 009EB1CA..009EB1CE
if (excess <= 0 || slack <= 0) goto near                      ; 009EB1D8, 009EB1EF

travel = 0092D730(unit+1018h) * excess * s+1BCh               ; 009EB1FE..009EB212
if (|travel| <= 1.0f) goto near                               ; 009EB21E..009EB238

ref    = 0080FC30(unit) * 0.05                                ; 009EB241..009EB255
shrink = 1 - min(s+1C4h,
                 max(|0092D730(unit+1018h)|, ref) / node+38h * s+1C0h) * excess
                                                              ; 009EB259..009EB2AB
if (shrink <= 0) { node+68h = 1 ; goto near }                 ; 009EB2B7, 009EB2C5

node+5Ch = shrink * node+38h                                  ; 009EB30E, 009EB31C
node+60h = min(1.0f, shrink * 6.0) * node+3Ch                 ; 009EB31F..009EB33D
limit  = min(s+1ACh * slack,
             max(unit+9C8h, s+1B0h) * s+1B4h)                 ; 009EB334..009EB363
if (own > closing) limit *= (own / closing + 1.0) * 0.5       ; 009EB37F..009EB39B
if (|travel| > limit) { excess *= limit / |travel| ;
                        travel = sign(travel) * limit }       ; 009EB3B5..009EB3EA
if (|travel| <= 0.1) goto near                                ; 009EB3F6..009EB404

turn = clamp(-00811940(unit) * excess, -pi/2, +pi/2)          ; 009EB428..009EB449
if (|turn| < 3 degrees) {                                     ; 009EB466, 009EB470
    node+44h = node+20h + travel * node+28h                   ; 009EB475..009EB4A8
    node+48h = node+24h + travel * node+2Ch                   ; 009EB499..009EB4AF
    node+4Ch..+58h = node+28h..+34h                           ; 009EB4B2..009EB4C7
    return                                                     ; node+64h is NOT written
}
node+64h = 00438AA0(node+40h, turn)                           ; 009EB4DE, 009EB4F0
node+4Ch,+50h = 006BC0C0(node+64h)                            ; 009EB4F6..009EB50B
node+54h = node+50h ; node+58h = -0.0f - node+4Ch             ; 009EB516, 009EB51B
R = |travel / turn|                                           ; 009EB540..009EB554
if (travel * turn > 0) { c = near + (node+30h,node+34h) * R ;
                         p = c - (node+54h,node+58h) * R }    ; 009EB57C..009EB5B8
else                   { c = near - (node+30h,node+34h) * R ;
                         p = c + (node+54h,node+58h) * R }    ; 009EB5BA..009EB5F6
node+44h, node+48h = p                                        ; 009EB600, 009EB607

near:                                                          ; 009EB2C9..009EB301
node+44h..+60h = node+20h..+3Ch ; node+64h = node+40h
```

Notes the listing forces:

- **`own` is this hull's speed, not the closing speed.** `009EB11D LEA ECX,[ESP+38h]` points at
  arguments 2 and 3 and is not reloaded before `009EB144 CALL 00414C60`, so the length taken
  there is `|(arg2, arg3)|`. The relative velocity computed just above it at
  `009EB12C..009EB140` goes to two stack slots and is used only for the dot product at
  `009EB14D`.
- **The floor on `closing` is 1.0f** (`FLD1` at `009EB172`), not a settings row, so a converging
  pair never divides by a small number and a diverging one still gets a one-metre-per-second
  arrival rate.
- **`s+1A0h` is a floor on a time and `s+1A4h` scales a distance.** `009EB0BD` multiplies
  argument 4 by `s+1A4h` to make the "close enough, use the near box" threshold, and
  `009EB18F` divides that same product by `closing` to turn it into the arrival time.
- **The shrink is per-axis.** `node+5Ch` takes `shrink` directly, `node+60h` takes
  `min(1, shrink * 6)` (the double 6.0 at `00CE6628`), so the box narrows across the bow long
  before it narrows abeam.
- **Four stores in the routine are dead.** `009EB064`/`009EB06C` write `1.0f` into `node+5Ch`
  and `node+60h`, and `009EB2BB`/`009EB2C0` write it again on the collapse arm; every path out
  of `009EB076` rewrites the pair, either from `node+38h`/`+3Ch` in the near tail or from the
  shrink at `009EB31C`/`009EB33D`. Only the `node+68h = 1` at `009EB2C5` survives the collapse
  arm, and setting it is what makes `009D80C0` and `009D8160` reject the node entirely.
- **The straight arm leaves `node+64h` alone.** It is the only arm that does. Since it is taken
  only when the projected turn is under three degrees, `node+64h` stays within three degrees of
  the last value some other arm wrote, which may be several frames old.

The constants: `00D7A24C` = 1.0f, `00D7A218` = 0.0f, `00D7A240` = -1000.0f, `00D7A210` = 1.0,
`00D7A280` = 0.5, `00D7A270` = 0.05, `00CE6628` = 6.0, `00D7A3A0` = 0.1, `00CE3C64` / `00CE3CCC`
= +/- pi/2, `00D1A8A0` = 0.05235987755982989, three degrees exactly.

## 4. The node's field table

`0x90` bytes (`operator new(0x90)` at `009F0E2A`), constructed by `009E52E0`
(`009E52E0-009E53A5`, `RET 0Ch`, three stack arguments: the observed unit, the observing block
and the initial lifetime). The constructor leaves `+20h..+64h` **uninitialised**; `009E5377`
sets `node+68h`, which is what keeps every consumer off the garbage until `009F0EA0` runs both
refreshes in the same frame it was appended (`009F516C` appends, `009F51E4` refreshes,
`009F51FA` scans).

| offset | what | written by | read by |
| --- | --- | --- | --- |
| `+0h` | vtable `00CF5C94` | `009E5311` | the destructor chain from `0064A610` |
| `+4h`,`+8h`,`+0Ch` | zeroed link fields | `009E52FC`..`009E5302` | not read by anything this packet read |
| `+10h` | byte 1 | `009E530D` | not read by anything this packet read |
| `+14h` | the observed unit | `009E532A` | `009EAFC6`, `009EAE26`, `009EBDE2`, `009F1029`, `009D8B96` |
| `+18h` | zero | `009E5321` | not read by anything this packet read |
| `+1Ch` | the observing control block, the second constructor argument | `009E5338` | not read by anything this packet read |
| `+20h`/`+24h` | near box centre | **`009EAF7F`/`009EAF86`** | `009D80C0`, `009EB07A`, `009EB2C9` |
| `+28h`/`+2Ch` | the observed hull's **forward** axis | **`009EAE9D`/`009EAEA8`** | `009D80C0`, `009D8160`, `009EB475` |
| `+30h`/`+34h` | the **beam** axis, `(+2Ch, -(+28h))` | **`009EAEB8`/`009EAEC0`** | `009D80C0`, `009D8160`, `009EB560` |
| `+38h` | half extent along `+28h`: `length*0.55 + \|advance\|` | **`009EAF9C`** | `009D80C0`, `009EB0A1`, `009EB266` |
| `+3Ch` | half extent along `+30h`: `beam*0.60` | **`009EAFAB`** | `009D80C0`, `009EB32E` |
| `+40h` | the observed hull's heading this frame | **`009EAE48`** | `009EB4D8`, `009EB2FB` |
| `+44h`/`+48h` | avoid box centre | **`009EB4A8`/`009EB4AF`, `009EB600`/`009EB607`, `009EB2C9`, `009EB61D`** | `009D8160`, `009D84E0`, `009D8860`, `009D8A30`, `009DD010`, `009DD540`, `009D8B90` |
| `+4Ch`/`+50h` | the corner builders' first axis | **`009EB4B2`, `009EB505`, `009EB2D7`** | `009D84E0`, `009D8860`, `009D8A30`, `009DD010`, `009DD540` |
| `+54h`/`+58h` | the corner builders' second axis | **`009EB4BE`, `009EB516`/`009EB51B`, `009EB2E3`** | the same five, plus `009D8B90` |
| `+5Ch` | avoid box half extent along `+4Ch` | **`009EB31C`, `009EB2EF`, `009EB629`** | `009D8160`, `009D84E0`, `009DD010`, `009DD540` |
| `+60h` | avoid box half extent along `+54h` | **`009EB33D`, `009EB2F5`, `009EB62E`** | the same four |
| `+64h` | the heading the avoid box is drawn at | **`009EB4F0`, `009EB2FB`** | no reader located |
| `+68h` | "no pose": the node has no usable box | `009E5377`, **`009EAFDB`, `009EB076`, `009EB2C5`, `009EB619`**, `009F110A` | `009D80C0`, `009D8160`, `009D84E0`, `009D8860`, `009D8A30`, `009DD010`, `009DD540`, `009D8B90` |
| `+69h` | "do not avoid": the party filter's answer | `009E5361`, **`009EAFDE`, `009EB040`** | `009D8160`, `009DD010`, `009EF350` |
| `+6Ch` | traffic separation | `009DCEB0` | `009DCEB0` |
| `+70h` | traffic separation bearing | `009DCEB0`, `009EF350` | `009EF350` |
| `+74h`,`+75h` | traffic separation flags | `009E5373`/`009E536A`, `009DCEB0`, `009EF350` | `009EF350` |
| `+78h` | lifetime, seconds | `009E5359`, `009F1009`, `009EBEF7`, `009F0DFF` | `009F1009` |
| `+7Ch` | zero | `009E537B` | not read by anything this packet read |
| `+80h` | the observed hull's cached world maximum Y | `009E5388`, **`009EB014`, `009EB643`** | **`009EB049`** only |
| `+84h` | the observed hull's cached world minimum Y | `009E5380`, **`009EB022`, `009EB63B`** | **`009EB033`** only |
| `+88h` | the agreed passing side | `009E5364`, `009D9100` `009D912F` | `009EBF51`, `009EF350`, `009F3F80` |
| `+8Ch` | the side negotiation's own state | `009E536D`, `009D8CE0` / `009D9100` | `009D8CE0` |

Bold entries are this packet's; the rest are cited from `docs/SHIP_AI_SECTOR_SCAN.md`,
`docs/SHIP_AI_AVOIDANCE_REQUEST.md` and the consumer bodies re-read here. `+70h` through `+75h`
are the traffic separation's, and neither refresh touches them.

**What the executable needs to build and refresh one node for another ship**, in order: allocate
`0x90` bytes and call `009E52E0` with the observed unit, this block and the lifetime; then once a
frame, while the lifetime is positive and the observed unit is alive, run `009EAE20` and then
`009EAFC0` on it in that order. `009EAFC0` reads `node+20h`, `+28h`..`+3Ch` and `+40h`, all of
which `009EAE20` has just written, so the order is not optional.

## Host methods the executable must implement, in call order

| # | routine | method | native call site | callee |
| --- | --- | --- | --- | --- |
| 1 | `009EAE20` | `observed_heading_vtable50` | `009EAE46` | indirect, `[[node+14h]]+50h`; concrete `006DFD60` |
| 2 | `009EAE20` | `observed_body_axis_speed_0092d730` | `009EAE54` | `0092D730` |
| 3 | `009EAE20` | `settings_pos_speed_corrig_1a8` | `009EAEC5` | `00424C40`, then `+1A8h` at `009EAECA` |
| 4 | `009EAE20` | `observed_hull_length_09c8` | `009EAEDB` | field `[unit+9C8h]`, no CALL |
| 5 | `009EAE20` | `observed_pose_valid_00c8` | `009EAF07` | field `[unit+C8h]`, no CALL |
| 6 | `009EAE20` | `refresh_observed_pose_00414db0` | `009EAF30` | `00414DB0` |
| 7 | `009EAE20` | `observed_world_position_xz_00fc` | `009EAF35` | fields `[unit+FCh]`, `[unit+104h]`, no CALL |
| 8 | `009EAE20` | `observed_hull_length_09c8` (again) | `009EAF8C` | field `[unit+9C8h]`, no CALL |
| 9 | `009EAE20` | `observed_hull_beam_09cc` | `009EAF9F` | field `[unit+9CCh]`, no CALL |
| 10 | `009EAFC0` | `observed_model_vtable20` | `009EAFED` | indirect, `[[node+14h]]+20h`; concrete `006D1E30` |
| 11 | `009EAFC0` | `observed_model_vtable20` (again) | `009EB005` | indirect, same slot |
| 12 | `009EAFC0` | `observed_world_bounds_0098a8e0` | `009EB009` | `0098A8E0` |
| 13 | `009EAFC0` | `settings_ship_avoidance_180` | `009EB05F` | `00424C40`, then `ADD EDI,180h` at `009EB087` |
| 14 | `009EAFC0` | `reciprocal_length_00419260` | `009EB0ED` | `00419260` |
| 15 | `009EAFC0` | `observed_velocity_vtable34` | `009EB119` | indirect, `[[node+14h]]+34h`; concrete `00812090` |
| 16 | `009EAFC0` | `observed_body_axis_speed_0092d730` | `009EB1FE` | `0092D730` |
| 17 | `009EAFC0` | `observed_reference_speed_0080fc30` | `009EB241` | `0080FC30` |
| 18 | `009EAFC0` | `observed_body_axis_speed_0092d730` (again) | `009EB259` | `0092D730` |
| 19 | `009EAFC0` | `observed_hull_length_09c8` | `009EB337` | field `[unit+9C8h]`, no CALL |
| 20 | `009EAFC0` | `observed_command_yaw_rate_00811940` | `009EB428` | `00811940` |
| 21 | `009EAFC0` arc arm | `heading_to_direction_006bc0c0` | `009EB4F6` | `006BC0C0`, inside `FUN_009EB4D1` |

`00414C60` and `00438AA0` are already reconstructed and are called directly
(`009EB09C`, `009EB144`, `009EB4DE`), through `bsp/vector_helpers.hpp` and `bsp/unit_rudder.hpp`.
`00415510`, `00415550` and `00415620` are three-instruction comparisons and are projected as
file-static helpers that keep the native's operand order, so an unordered compare picks the same
side it does in the image.

`python tools/verify_report_calls.py reports/ship_ai_neighbour_box.json` checked all 22
`address`/`native` call rows against the live function bodies and the call graph: 0 failed. Ten
further rows are the indirect vtable slots and the plain field reads, which the script reports as
not checkable. The `009EB4DE` and `009EB4F6` rows name `009EB4D1` as their containing function,
which is what caught the split described in section 2.

## Coverage

| routine | coverage |
| --- | --- |
| `009EAE20` | complete: every instruction `009EAE20..009EAFB2` is projected by `ship_ai_neighbour_near_box_refresh_009eae20` |
| `009EAFC0` | complete: every instruction `009EAFC0..009EB64F` is projected by `ship_ai_neighbour_avoid_box_refresh_009eafc0`, all five return arms included |
| `009EB4D1` | complete: `009EB4D1..009EB60E`, the arc branch of the same projection. Ghidra's separate zero-caller function for what is one arm of `009EAFC0` |
| `009E52E0` | partial: `009E52F5..009E53A3`, the field stores, read for the field table. The SEH frame `009E52E0..009E52F4` and the `00694A60` link at `009E532D` are not projected |
| `009F0EA0` | partial: `009F0EA9..009F0FEF` and `009F10A0..009F10FF`, the head that builds the two refreshes' arguments. The ageing loop stays `docs/SHIP_AI_SECTOR_SCAN.md`'s, and this packet did not re-project it |
| `009F0D20` | read only at `009F0E2A..009F0E69`, the three constructor arguments |
| `0098A8E0`, `0092D730`, `0080FC30`, `00811940`, `00812090`, `006BC0C0`, `00438AA0`, `00414C60`, `00419260`, `00415510`, `00415550`, `00415620`, `006D1E30`, `006DFD60` | read whole for their ABI and their result, not re-projected; `0098A8E0`, `0092D730`, `00414C60` and `00438AA0` already have reconstructions |
| `009D80C0`, `009D8160`, `009DD010`, `009DD540`, `009D84E0`, `009D8860`, `009D8A30`, `009D8B90`, `009EF350`, `009DCEB0`, `009D8CE0` | read only for which node fields they consume, to fill the field table's last column |

`src/ship_ai_neighbour_box.cpp` builds clean under `/W4 /WX` for Win32 and the existing
`reconstructed_math` test still passes. **No new test case was added**: both routines are
projections whose evidence is the listing, the four pure rules are exposed as free functions so
the numbers above can be checked without a host, and the three comparison helpers already have
coverage through the packets that call them. This follows the same choice
`docs/SHIP_AI_SECTOR_SCAN.md` made for the same family.

## Run-time evidence

**None, and none available.** `docs/GAME_EXECUTABLE.md` milestone 2p records that the neighbour
list is empty in the validation process: `009EF230` runs 15680 times and marks nothing because
`009F0D20` never appends, which `docs/SHIP_AI_SECTOR_SCAN.md` traced to the world unit list the
pre-pass walks. Both routines here run only from inside that list's ageing loop, so
`bsp_game.exe` cannot reach either one yet. Every claim in this document is static, from the
listing.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/SHIP_AI_SECTOR_SCAN.md` node table: "`+28h`/`+2Ch`, `+30h`/`+34h` the beam and forward axes", and `ShipAiObstacleNode`'s `axis_beam_*` / `axis_forward_*` | `+28h`/`+2Ch` is the **forward** axis and `+30h`/`+34h` the **beam** axis. The names came from the consumers; the producer settles it | `009EAE9D`/`009EAEA8` store `cos`/`sin` of the observed hull's own folded heading into `+28h`/`+2Ch`; `009EAEB8`/`009EAEC0` derive `+30h`/`+34h` from them as `(+2Ch, -(+28h))` |
| `docs/SHIP_AI_SECTOR_SCAN.md` node table: "`+38h`/`+3Ch` the near box half extents", and `ShipAiObstacleNode`'s `near_half_beam` / `near_half_length` | `+38h` is the half extent along `+28h`, so the half **length**; `+3Ch` is the half extent along `+30h`, the half **beam** | `009EAF9C` stores `unit+9C8h * 0.55 + \|advance\|` into `+38h` and `009EAFAB` stores `unit+9CCh * 0.60` into `+3Ch`; `009D80C0` `009D8111`/`009D813A` compares `+38h` against the `+28h` projection |
| `docs/SHIP_AI_SECTOR_SCAN.md` node table: "`+18h` the observing control block, `009E5308`" | `+18h` is **zeroed** at `009E5321`; the observing control block goes to **`+1Ch`** at `009E5338`. `009E5308` is inside `009E5306 MOV dword ptr [ESP+0Ch],ESI`, a stack store, not a node store | the constructor's listing; `009F0E4F PUSH ESI` supplies that argument and `ESI` in `009F0D20` is the block whose `blk+604h`/`blk+608h` the node is appended to |
| `docs/SHIP_AI_SECTOR_SCAN.md`: "`009EAFC0` is given the clamped hull velocity (the floor `00D7A23C` = 0.001f, the cap `class+500h * settings+1B8h`)" | `class+500h * settings+1B8h` is a **floor**, not a cap: `009F0F9F`/`009F0FA1` skip the scaling when it is already below the speed, and `009F0FA3 FDIVRP` then scales the vector **up** to it. The settings key is `ShipAvoidance.NearbyShip_MyMinSpdRatio` | `009F0F97..009F0FBF`; `docs/SHIP_AI_SETTINGS_BLOCK.md` `+1B8h` |
| `docs/SHIP_AI_SECTOR_SCAN.md`: "`009EAFC0` is given ... `unit+9C8h * 0.55` and `unit+9CCh * 0.75`" | It is given both, and reads only the first. Argument 5 (`unit+9CCh * 0.75`) has no reader in the body | the argument slot survey in section 2: no `[ESP+44h]` operand exists in the `PUSH EBX; PUSH EDI` epoch |
| `docs/SHIP_AI_SECTOR_SCAN.md` uncertainty 3: "whether `009EAE20` and `009EAFC0` keep the two axis pairs identical was not checked" | They do on the near-box copy tail and the straight arm; they do **not** on the arc arm, which rotates `+4Ch..+58h` and leaves `+28h..+34h` alone | `009EB2D7..009EB2EC`, `009EB4B2..009EB4C7`, `009EB505..009EB51B` |
| `docs/SHIP_AI_SECTOR_SCAN.md`: "`009EAE20(n)(&dir)`" | The argument is the caller's floored hull velocity pair, and `009EAE20` never reads it | `009F1046`/`009F104A` build it; the body's largest ESP displacement is `+1Ch` against an argument at `+24h` |

The two `ShipAiObstacleNode` field names are **not** changed here: the type belongs to
`include/bsp/ship_ai_sector_scan.hpp` and renaming it is another packet's edit.
`include/bsp/ship_ai_neighbour_box.hpp` documents the mismatch at the top and at every use.

## Uncertainties

1. **`unit+9C8h` and `unit+9CCh` are read as the hull's full length and full beam**, so the box
   half extents are `0.55` and `0.60` of them: a little over half, which is what an avoidance
   box wants. Three uses agree: `009F1987`'s admission radius is `(self+9C8h + other+9C8h) * 0.5`,
   which is the sum of two half lengths; `include/bsp/ship_ai_hull_geometry.hpp` already calls
   `+9CCh` the full beam; and `009EAE20` scales both by very nearly a half. But
   `docs/GAME_EXECUTABLE.md` records that `+9C8h` "has no producer anywhere" and
   `include/bsp/ship_ai_arm_final_step.hpp` calls it a hull **radius**. The producer was not
   located by this packet either. If it turns out to be a radius, every extent above is twice
   what it should be.
2. **`node+64h` has no located reader.** Twelve node consumers were grepped for it
   (`009D80C0`, `009D8160`, `009DD010`, `009DD540`, `009D84E0`, `009D8860`, `009D8A30`,
   `009D8B90`, `009EF350`, `009EB660`, `009F0D20`, `009DCEB0`) plus `009D8CE0`, `009D8C60`,
   `009D8010`, `009F3E30`, `009D9100`, `009F4D10`, `0064A610`, `009E53B0` and `009F3F80`; none
   touches it. Either the field is vestigial or its reader is somewhere none of those packets
   has looked.
3. **`00424C40` is called twice per node per frame**, once from each refresh (`009EAEC5`,
   `009EB05F`). Whether the singleton getter is cheap enough for that to be deliberate is not
   established; the projection simply keeps both calls.
4. **The concrete vtable bodies are read from `00CFC3D0`.** That table is the unit's on two
   independent readings, but nothing here proves the observed unit at `node+14h` is of that exact
   class rather than a subclass that overrides `+20h`, `+34h` or `+50h`.
5. **Whether `node+68h` set by the collapse arm (`009EB2C5`) is meant to persist** is not
   settled. `009F0EA0` clears nothing, and `009EB076` clears it only after the vertical gate
   passes, so a node that collapses stays invisible to the scan until a frame where the
   projection succeeds again.

## Follow-up packets

| packet | addresses and files | what is left |
| --- | --- | --- |
| `ship_ai_hull_scalars_9c8` | `unit+9C0h`, `unit+9C8h`, `unit+9CCh`, `0080FC30`, `009F1987`, the loader that fills them | The producer of the three hull scalars every ship AI geometry routine divides and scales by. Uncertainty 1 above, `docs/GAME_EXECUTABLE.md`'s "no producer anywhere" and the disagreement between `ship_ai_arm_final_step.hpp` ("radius") and `ship_ai_hull_geometry.hpp` ("full beam") all end at the same unread write site. |
| `ship_ai_obstacle_node_field_names` | `include/bsp/ship_ai_sector_scan.hpp` `ShipAiObstacleNode`, `src/ship_ai_sector_scan.cpp`, `docs/SHIP_AI_SECTOR_SCAN.md` | Apply the first three corrections above to the type and its doc: swap `axis_beam_*` with `axis_forward_*`, swap `near_half_beam` with `near_half_length` (and the `avoid_*` pair with them), and move the observing block from `+18h` to `+1Ch`. It is a rename across one header, one source and one doc, and it belongs to whoever owns that header. |
| `ship_ai_node_projected_heading_64` | `node+64h`, `009EB4F0`, `009EB2FB` | Find the reader, or establish that there is none, and decide whether the straight arm's failure to write it is a defect. Uncertainty 2. |
| `ship_ai_avoid_box_arc_arm_merge` | `009EAFC0`, `009EB4D1`, a Ghidra write | `FUN_009EB4D1` is `009EAFC0`'s arc arm, has zero callers and is entered only by the `JBE` at `009EB470`. Removing it and letting `009EAFC0` own `009EB4D1-009EB610` would make `get_function_by_address` agree with the control flow and stop future packets from attributing `009EB4DE` and `009EB4F6` to a routine that does not exist. It is a Ghidra mutation under the write lock, so it belongs to the integrator, not to a read-only worker. |
| `ship_ai_traffic_separation_fields` | `node+6Ch`, `+70h`, `+74h`, `+75h`, `009DCEB0`, `009EF350` | The one block of node fields neither refresh writes. `docs/SHIP_AI_AVOIDANCE_REQUEST.md` already lists `ship_ai_traffic_pass_009ef350`; this packet only confirmed from the writers' side that the two boxes and the separation flags do not overlap. |

## no_ghidra_function

One routine was read from the raw listing because Ghidra has no function at its start.

| start | inclusive end | evidence for each boundary |
| --- | --- | --- |
| `006DFD60` | `006DFD66` | **Start**: it is the pointer stored at `00CFC3D0+50h` (`60 fd 6d 00`), the vtable slot `009EAE46` and `009EB939` call, and `python tools/bsp.py ghidra proto 006DFD60 --brief` reports no function there, only the enclosing candidate `006DFD20 BSP_SensorCategory_SurfaceShip`. **End**: `python tools/bsp.py disasm-raw 006DFD60` reads `006DFD60 FLD dword ptr [ECX+1050h]` (6 bytes) and `006DFD66 RET` (1 byte) from the disk image, followed by `INT3` padding from `006DFD67` to `006DFD6F` and the next accessor at `006DFD70`. |

It is read only to give the indirect host method `observed_heading_vtable50` a contract; nothing
in this packet is projected from it and no name is claimed for it. Defining it is optional. Every
other address this packet read is inside a Ghidra function whose start the tools report, including
`009EAE20` (body `009EAE20-009EAFB4`), `009EAFC0` (body `009EAFC0-009EB651`) and every callee in
the host table, each checked with `python tools/bsp.py ghidra proto <addr> --brief`.

## Correction from docs/SHIP_AI_NEIGHBOUR_BOX_MATH.md

The earlier claim that every unordered helper selected the native operand was
too broad. The original-byte O fixture reproduces failures in the old source:
the shrink gate accepts positive or unordered values, local min selects its
second operand on unordered comparison, and the closing floor and turn clamp
retain NaN. The corrected source preserves these branches, x87 shrink arithmetic
at the caller's precision, and arc-product spills. The value at 00D1A8A0 is
0.052359879016876220703125, the widened binary32 approximation of three degrees;
it is not the previously recorded exact-angle double. Fourteen cases at PC24
and PC53 match 1,188 original-byte words. This bounded fixture does not establish
all-input FP, owner mutation, CRT, ABI or original-game parity. The enclosing
frame and near-box producer are outside this new fixture's proof.
