# The ship-class field at class+524h that scales every AI turn

Addresses: 00828F20, 0082E850, 00951F20, 00964790, 009650E6, 0096515D, 00963380, 00831840, 009DA250, 009DA268, 00D1ACC4, 00D7A280, 00758140, 00852200

Packet `cc_ai_class_field`. Ghidra was read-only for this packet; the two names it adds to the
ledger are hypotheses, not recovered symbols. The follow-up that produced it is
`ship_ai_class_field_0524` in docs/SHIP_AI_THROTTLE_TO_RING.md.

## The answer

`class+524h` has no Lua key. It is **derived**, once per class, out of two keys the ship rows do
author:

```
class+524h = 0.5 * MaxRotAngle (class+4F8h) / MaxRotAngleChangeRatio (class+4FCh)
class+520h = MaxSpeed (class+500h) / MaxRotAngle (class+4F8h)
```

Both stores are in `00828F20`, the ship-class descriptor's virtual slot `+14h`. That is why the
key search in packet `cc_ai_throttle_ring` came up empty: `00831840`
`BSP_ShipClass_ReadLuaFields` reads Lua keys and writes `+4F8h..+51Ch` and then `+538h` onward,
and nothing in the Lua row corresponds to `+520h` or `+524h` at all.

For `VehicleClass[20]`, the DeRuyter 1935, the field is **0.1 rad exactly**
(`0.099999994f`), so the AI applies full rudder beyond `0.1 * 1.2 = 0.12 rad` (6.875 deg) of
heading error. The probe's stand-in was `MaxRotAngle` itself, `0.122173`, which is 1.22173 times
too large.

`MaxRotAngleChangeRatio` has no other consumer anywhere in the image. `00828F23` is the only
instruction that loads `class+4FCh` through a descriptor, so the entire effect of that key on the
game is this one product.

## 00828F20, the derive pass

`bool __thiscall(descriptor)(void)`, `RET` at `00828F79`, body `00828F20-00828F79`, coverage
complete.

It is virtual slot `+14h` of the ship-class descriptor vtable at `00D1ACC4`:
`00963380` `BSP_ShipClass_ConstructBase` stores that vtable pointer at `009633C0`
(`MOV dword ptr [ESI],0xD1ACC4`), and `00D1ACC4 + 14h = 00D1ACD8` is one of the seven data
references Ghidra reports for `00828F20`. `00758140` and `00852200`, which the bridge lists as
unconditional callers, are one-instruction `JMP 00828F20` thunks in ranges with no Ghidra
function, so this body is the only implementation in the image.

It runs once per class from `0096515D` in `BSP_VehicleClass_GetOrCreate` (body
`00964790-009652A8`), and it runs **after** the Lua row has been read into the descriptor. The
same function dispatches the descriptor's slot `+8h` at `009650E6` (`EDX` from
`009650DA MOV EAX,[ESI]` then `009650DC MOV EDX,[EAX+8]`, `ECX = ESI`, one pushed argument, the
row at `ESP+38h`), which for a ship is `00831840`: the ship vtable `00D1ACC4` holds `00831840` at
`+8h` and `00828F20` at `+14h`. Between them are the slot `+10h` call at `00965143` with the
argument `0` and, after `0096515D`, the slot `+18h` query at `0096516F` with the argument `6`.
The derive pass's return value is discarded at that site: `0096515F` immediately decrements the
counter at `00F8A098` that `009650D3` raised around the load.

### The two gates

```
00828F23  MOVSS  XMM0,dword ptr [ECX + 0x4fc]   ; MaxRotAngleChangeRatio
00828F2B  XORPS  XMM1,XMM1
00828F2E  COMISS XMM1,XMM0
00828F37  JNC    0x00828f74                     ; -> XOR AL,AL ; RET
00828F39  MOVSS  XMM0,dword ptr [ECX + 0x4f8]   ; MaxRotAngle
00828F41  COMISS XMM1,XMM0
00828F49  JNC    0x00828f74
```

`JNC` leaves through the failure arm when `CF = 0`. An ordered `COMISS XMM1,XMM0` sets `CF = 1`
only for `0 < value`, but an **unordered** pair sets `ZF = PF = CF = 1`, so a NaN value passes the
gate rather than failing it. The reconstruction reproduces that (`(0.0f < v) || std::isnan(v)`).
On the failure arm nothing is stored, so both fields keep whatever they held; `00963380` clears
`+51Ch`, `+52Ch`, `+530h` and `+534h` explicitly (`009633D2`..`009633E4`, `EDI = 0` from
`009633B9 XOR EDI,EDI`) but never `+520h` or `+524h`.

### The arithmetic

```
00828F44  MOVSS  dword ptr [ESP],XMM0           ; spill MaxRotAngle
00828F31  MOVSS  dword ptr [ESP + 0x4],XMM0     ; spill MaxRotAngleChangeRatio (earlier)
00828F4B  FLD    float ptr [ESP]                ; ST0 = MaxRotAngle
00828F4E  FLD    ST0                            ; ST0 = ST1 = MaxRotAngle
00828F50  FDIV   float ptr [ESP + 0x4]          ; ST0 = MaxRotAngle / MaxRotAngleChangeRatio
00828F54  FMUL   double ptr [0x00d7a280]        ; * 0.5
00828F5A  FSTP   float ptr [ECX + 0x524]        ; store and pop; ST0 = MaxRotAngle again
00828F60  FDIVR  float ptr [ECX + 0x500]        ; ST0 = MaxSpeed / MaxRotAngle
00828F66  FSTP   float ptr [ECX + 0x520]
00828F6F  JMP    0x00951f20
```

`00D7A280` holds `00 00 00 00 00 00 E0 3F`, the double `0.5`. `FDIVR` divides the **memory**
operand by `ST0`, not the other way round, which is what makes `+520h` a radius and not a rate.
`FLD ST0` at `00828F4E` is the reason the second quotient can reuse `MaxRotAngle` without
reloading it. Both quotients are computed at the x87 working precision and only the two `FSTP`s
round to float32; the reconstruction uses `double` intermediates and one `static_cast<float>` per
store for that reason.

The tail jump at `00828F6F` goes to `00951F20` (body `00951F20-00951F2A`), a two-line
`return 00876180() != 0`. So the boolean this routine returns is that call's result and not a
report on the derivation. No caller in the image looks at it.

### Units

`class+520h` is metres: `MaxSpeed` is m/s and `MaxRotAngle` is rad/s, so the quotient is the
hull's turn radius at full speed and full yaw rate. `0082E850`
`BSP_ShipClass_GetTurnRadius` is its only reader (`0082E853 MOVSS XMM0,[ECX+520h]`); it calls the
descriptor's own slot `+18h` with the argument `0Eh` and, when that returns zero, multiplies the
radius by the float at `+438h` of the `00424C40` tuning singleton before returning it on the x87
stack. For the DeRuyter the unmultiplied value is `134.745`.

`class+524h` is `0.5 * (rad/s) / (rad/s/s)`, that is half the time `MaxRotAngleChangeRatio`
would need to wind the yaw rate up to `MaxRotAngle`. `009DA250` then uses it as if it were an
angle: `009DA268` loads it, `009DA276` scales it by the double `1.2` at `00CEC160`, and
`009DA280` divides the heading error in radians by the product. The image is not dimensionally
consistent here and no conversion instruction reconciles the two; the number is simply the
heading error at which the AI reaches full rudder. Marked provisional: the intent behind the
`0.5` is a hypothesis, the arithmetic is not.

## Why no Lua key writes the field

Two independent searches, both negative:

* `00831840` `BSP_ShipClass_ReadLuaFields` (docs/SHIP_CLASS_FIELDS.md, body
  `00831840-0083468A`) writes `+4F8h` through `+51Ch` and then `+538h` onward. Its key table
  contains no entry for `+520h`, `+524h`, `+528h`, `+52Ch`, `+530h` or `+534h`.
* A byte scan of `.text` for every store encoding at displacement `524h`
  (`D9 ?? 24 05 00 00` x87, `F3 0F 11 ?? 24 05 00 00` MOVSS, `F2 0F 11 ...` MOVSD,
  `89 ?? 24 05 00 00`, `C7 ?? 24 05 00 00`, `88 ?? 24 05 00 00`, plus `8D ?? 24 05 00 00` for a
  helper taking the field's address) returns eleven sites. Ten belong to other objects:
  `007EA5DA` and `006C3F14` are the `0042E740` tuning singleton's own `+524h` (at `006C3F14`
  `EAX` still holds the singleton returned by the call at `006C3ED5`, as the filtered listing of
  `006C3E50` shows: nothing writes `EAX` between `006C3EDA MOV EBX,EAX` and the load);
  `00956638`, `0095CE21`, `0079B696`, `007C49D3`, `0079D13E` and the `MainMenuScreen` and
  `StorageOperation` sites are unit instances and unrelated classes. `00828F5A` is the only store
  to a descriptor, and `009DA268` the only load from one.

## The neighbours, class+51Ch through +537h

| offset | what writes it | what it is |
| --- | --- | --- |
| `+51Ch` | `00831CB7` in `00831840`; cleared at `009633D2` | `ExplosionEfx`, an effect handle (docs/SHIP_CLASS_FIELDS.md) |
| `+520h` | `00828F66` only | `MaxSpeed / MaxRotAngle`, the turn radius in metres; read only by `0082E850` |
| `+524h` | `00828F5A` only | `0.5 * MaxRotAngle / MaxRotAngleChangeRatio`; read only by `009DA268` |
| `+528h` | nothing found | unsettled: no store at that displacement in the image belongs to a descriptor, and `00963380` does not clear it |
| `+52Ch`, `+530h`, `+534h` | `009633D8`, `009633DE`, `009633E4` set them to `0`; `00963A7E`, `00963A84`, `00963A8A` in `CG_vector_deleting_dtor_00963600` tear them down | a begin/end/capacity triple, the usual shape of the game's vector members. No site in the image appends to it, so it stays empty in a shipped build |

`+528h` and the empty vector are labelled unsettled, not absent: the scan covered direct
displacement forms and the `LEA` form, not an access built from a computed base.

## The value for VehicleClass[20]

From the installed `scripts/datatables/autoload/vehicleclasses.lua`, the DeRuyter 1935 block
starting at line 13618:

| key | line | value |
| --- | --- | --- |
| `MaxRotAngle` | 13884 | `0.122173` (7.000 deg/s) |
| `MaxRotAngleChangeRatio` | 13885 | `0.610865` (35.000 deg/s/s) |
| `MaxSpeed` | 13886 | `16.4622` |

| quantity | value |
| --- | --- |
| `class+524h` derived | `0.099999994f` |
| full-rudder heading error, `+524h * 1.2` | `0.12 rad`, 6.875 deg |
| the stand-in's threshold, `MaxRotAngle * 1.2` | `0.14661 rad`, 8.400 deg |
| `class+520h` derived | `134.745` m |

The ratio `MaxRotAngle / MaxRotAngleChangeRatio` is exactly `0.2` for this row (7 deg over
35 deg), which is why the derived field lands on a round `0.1`. That is a property of the
authored numbers, not of the formula.

## Run-time evidence

`bsp_ship_motion_probe.exe --class 20 --moveto 4000,4000 --steps 6000`, the same invocation as
the run-time table in docs/SHIP_AI_THROTTLE_TO_RING.md, before and after the stand-in was
replaced:

| quantity | was, `MaxRotAngle` stand-in | is, `00828F20` derived |
| --- | --- | --- |
| `class+524h` used | `0.122173` | `0.100000` |
| minimum distance | 849.88 | 842.31 |
| first step inside the 2000-unit radius | 4586 | 4580 |
| time to that radius | 229.31 s | 229.01 s |
| final heading error | 0.032551 rad | 0.027144 rad |
| final ring slot rudder | -0.222025 | -0.226200 |
| steps taking the `009F4BC6` deadband | 0 of 6000 | 0 of 6000 |

The run never saturates the rudder, so the smaller authority shows up as a proportionally larger
demand for the same error rather than as a different trajectory: the heading error settles 17
per cent lower and the arrival is 0.3 s earlier. A course that did saturate would diverge much
further, because the saturation threshold moves from 8.4 deg to 6.9 deg.

## Coverage

| routine | coverage |
| --- | --- |
| `00828F20` | complete: both gates, both stores, the tail jump |
| `0082E850` | complete for the value it returns; which arm of the `+18h` query is the fallback was not settled |
| `00951F20` | read for its shape only, not reconstructed |
| `009DA250` | unchanged from packet `cc_ai_throttle_ring`, complete; only its `+524h` argument is re-documented here |
| `00963380` | read for the members it clears; not reconstructed |
| `00831840` | not re-read; its key table is taken from docs/SHIP_CLASS_FIELDS.md |

## Corrections

| what said it | what is true | evidence |
| --- | --- | --- |
| docs/SHIP_AI_THROTTLE_TO_RING.md "Uncertainties" item 1 and the packet brief: "`class+524h` has no name, so its Lua key is unknown" | There is no Lua key to find. The field is derived from `MaxRotAngle` and `MaxRotAngleChangeRatio` by `00828F20` after the Lua load, and a different descriptor *reader* does not exist | `00828F5A`, `00828F54`, `0096515D` |
| the same, and the brief: "`00831840` writes `+4F8h..+51Bh` and `+538h` onward, so a reader fills `+51Ch..+537h`" | `00831840` also writes `+51Ch` (`ExplosionEfx`, site `00831CB7`, docs/SHIP_CLASS_FIELDS.md). The gap it leaves is `+520h..+537h`, and only `+520h` and `+524h` in it are ever written | the key table in docs/SHIP_CLASS_FIELDS.md; `009633D2` |
| the brief's candidate `0082B170` for the missing writer | `0082B170` is the native-string append helper the camo `TextureRemaps` container uses (docs/SHIP_CLASS_FIELDS.md, `00833501`/`0082B170`). It touches no motion field | its callees `0041DD40` and `00BF7680`, and its one caller `00831840` |
| `src/ship_motion_probe.cpp` and docs/SHIP_AI_THROTTLE_TO_RING.md: `--yaw-authority` defaults to `MaxRotAngle`, "a stand-in" | the default is now `0.5 * MaxRotAngle / MaxRotAngleChangeRatio` through `ship_class_ai_derived_motion_00828f20`. The switch is kept for overrides | the run-time table above |
| the ledger name `FUN_00828F20` | `BSP_ShipClass_DeriveTurnFields`, a hypothesis | `00828F5A`, `00828F66` |
| the ledger name `FUN_0082E850` | `BSP_ShipClass_GetTurnRadius`, a hypothesis | `0082E853` |

## Follow-up packets

| packet | addresses and files | what is left |
| --- | --- | --- |
| `ship_class_field_0528` | `class+528h`, `class+52Ch..+534h`, `00963380`, `00963600` | Settle the one offset in the gap with no writer and the vector member nothing appends to. Both need a search for computed-base accesses, which the displacement and `LEA` scans here do not cover |
| `ship_class_turn_radius_consumers` | `0082E850`, `00424C40+438h`, the descriptor's slot `+18h` with argument `0Eh` | Who calls `0082E850`, and which arm of the `+18h` query is the fallback. The tuning multiplier at singleton `+438h` has no name yet |
| `ship_class_derive_pass_return` | `00828F20`, `00951F20`, `00876180` | What `00951F20`'s `00876180` actually tests, and whether any other slot-`+14h` override in the descriptor families returns something a caller reads |

## no_ghidra_function

| start | end (inclusive) | evidence |
| --- | --- | --- |
| `00758140` | `00758144` | Five bytes, `E9 DB 0D 0D 00`: `JMP 00828F20` (`00758145 + 000D0DDB`). `00758134..0075813F` and `00758145..0075814F` are `CC` padding, so the instruction stands alone. The bridge lists `00758140` as an unconditional caller of `00828F20` while reporting no function at or containing it; the nearest function that ends before it is `FUN_00758090`, body `00758090-0075812A` |
| `00852200` | `00852204` | Five bytes, `E9 1B 6D FD FF`: `JMP 00828F20` (`00852205 + FFFD6D1B`). `008521FC..008521FF` and `00852205..0085220F` are `CC` padding. The bridge reports no function at `00852200`; the nearest one that ends before it is `FUN_00852140`, body `00852140-00852186`, and the four-instruction stub at `008521F0..008521FB` between them has no function either |

Neither boundary is load-bearing for anything above: both thunks land on `00828F20`, whose body
the bridge does report, and no claim in this document depends on the thunks being functions.
