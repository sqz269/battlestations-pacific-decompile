# The gunnery tables: who fills them

Addresses: `00727BD0`, `00956C20`, `00955EB0`, `007327B0`, `0087D7B0`, `00729F10`,
`00731020`, `008AC140`, `008AC2B0`, `008AC420`.

Reconstruction: `include/bsp/gunnery_tables.hpp`, `src/gunnery_tables.cpp`. Report:
`reports/gunnery_tables.json`.

`docs/UNIT_GUNNERY_PASS.md` reads the gunnery pass `00864FE0` as a set of rules over five
inputs it could not find producers for. This document is those five producers. Every
descriptive name below is a hypothesis, not a recovered symbol, with two exceptions that
are literal strings in the image: the twelve weapon `Function` spellings and the three Lua
binding names in section 5.

## Summary

| Input | Producer | What it is |
| --- | --- | --- |
| `00E092C8`, the twelve preference lists | **nothing at run time** | file-initialized `.data` in the image |
| `unit+394h`, the per-category gun records | `00956C20` | rebuilt from the device children |
| `unit+430h`, the per-category ranges | `00956C20` | max weapon range over that category's guns |
| `GlobalConfig+88h` | `0087D7B0` | `Globals.WeaponSystems.WeaponDirectorThinkTime`, `2` |
| `entity+1D4h` | `008AC140`/`008AC2B0` | the Lua `AddUntouchableUnit` flag |

The category index `0..0Bh` is not a mapping from anything: it **is** the weapon `Function`
of the gun's class descriptor, `[gun+3F4h][+80h]`, which `007327B0` writes from the
device-class Lua row.

## 1. `00E092C8` is in the image, not loaded

`docs/UNIT_GUNNERY_PASS.md` records `00E092C8` as "zero in the file on disk ... filled at
run time" and its producer as `contract: unread`. Both halves are wrong.

`.data` starts at VA `00E08000` and its raw size in the PE header is `10000h`, so
`00E08000`-`00E18000` is initialized storage written to disk. `00E092C8`-`00E0A4F8` sits
inside it and carries twelve authored rows of `61h` dwords on a `184h` stride, each row
non-zero only in its leading entries.

There is no run-time producer and no Lua source. Scanning the whole image for the
little-endian dword `00E092C8` returns exactly one hit, the `MOV ESI, 0E092C8h` immediate at
`00727BDB` inside the reader itself. Nothing writes the block.

The values the earlier document quotes as observed "in a live process" are simply these.

| Category | `Function` | Row VA | Class ids, best first |
| --- | --- | --- | --- |
| `0` | PLANEGUN | `00E092C8` | `61h` |
| `1` | AAMACHINEGUN | `00E0944C` | `17h 11h 12h 15h 16h 10h 13h 0Eh 0Ch 0Bh 08h 41h` |
| `2` | LIGHTARTILLERY | `00E095D0` | `07h 0Ch 0Bh 0Eh 0Ah 08h 09h 0Dh 1Ch 1Bh 45h 46h 19h 41h` |
| `3` | MEDIUMARTILLERY | `00E09754` | `07h 0Ah 08h 09h 0Ch 0Bh 0Eh 0Dh 1Ch 1Bh 45h 46h 19h 41h` |
| `4` | HEAVYARTILLERY | `00E098D8` | `0Dh 0Ah 07h 09h 0Ch 0Bh 08h 0Eh 1Ch 1Bh 45h 46h 19h 41h` |
| `5` | FLAK | `00E09A5C` | `17h 11h 12h 15h 16h 14h 10h 13h 0Eh` |
| `6` | LIGHTARTILLERYFLAK | `00E09BE0` | `17h 11h 12h 16h 15h 14h 10h 13h 0Eh 07h 08h 0Bh 0Ch 09h 0Ah 0Dh 1Ch 1Bh 45h 46h 19h 41h` |
| `7` | TORPEDO | `00E09D64` | `09h 0Dh 0Ah 07h 0Ch 0Bh 08h 41h` |
| `8` | DEPTHCHARGE | `00E09EE8` | `08h 41h` |
| `9` | DEPTHCHARGELAUNCHER | `00E0A06C` | `08h 41h` |
| `0Ah` | BOMBPLATFORM | `00E0A1F0` | empty |
| `0Bh` | CATAPULT | `00E0A374` | empty |

Read through `docs/ENTITY_CLASS_IDS.md`, category 1 is
`MPlaneKamikaze, MPlaneTorpedoBomber, MPlaneDiveBomber, MSmallReconPlane, MLargeReconPlane,
MPlaneBomber, MPlaneFighter, MTorpedoBoat, MLandingShip, MCargo, MSubmarine, NavPoint`, and
category 4 puts `MBattleship` first where categories 2 and 3 put `MDestroyer`. Category 6 is
the only dual-purpose row: the whole air list, then the whole surface list. Categories 8 and
9 hold `MSubmarine` and nothing else.

### Category 0 and the `61h` entry

`61h` is one past the class-id space `0..60h`, so category 0's row can never match a class
id. That row exists because the category index is a weapon `Function` and `PLANEGUN` is a
`Function` like any other; a plane's guns are not driven by this unit-side pass.

The earlier document notes without explanation that `61h` "lands in category 1's slot 0".
It does, and it is harmless. `00727BFB` writes at `[00E19BF8 + (id + category*61h)*4]`
without a bound, so category 0's rank `1` goes to `00E19BF8 + 184h`, the first dword of row
1. The next iteration begins with the `REP STOSD` at `00727BEC`, which zeroes all `61h`
dwords of row 1 before filling it. The stray write never survives.

## 2. `unit+394h` and `unit+430h`: `00956C20`

`00956C20` (`__fastcall(Unit*)`, `RET 0`, body `00956C20`-`00956ED0`) is the producer of
both. Its callers are `007280A0`, `00729FA0`, `0072B5A0` and `0095E5B0`, so the index is
rebuilt at unit setup and again whenever a gun's state changes; it is not built once at
construction.

| Step | Site | What happens |
| --- | --- | --- |
| 1 | `00956C31` | `unit+6E0h = 0` |
| 2 | `00956C38` | `00955EB0(unit+424h)` empties the all-guns list |
| 3 | `00956C48`-`00956C55` | `00955EB0` on each of the twelve records at `unit+394h + i*0Ch` |
| 4 | `00956C57` | walk the unit's **direct** device children: head `unit+48h`, next `device+44h` |
| 5 | `00956C62`, `00956C76` | keep a device with `[device+5Dh] == 0` that answers `vtable[5Ch](20h)` |
| 6 | `00956C86`-`00956C8F` | `unit+6E0h = 1` when `[device+3F4h][+80h] == 7` (TORPEDO), **before** the operational test |
| 7 | `00956C98` | `00729F10(device)` must hold |
| 8 | `00956CB6`-`00956CF4` | append to the record of category `[device+3F4h][+80h]` |
| 9 | `00956CF9`-`00956D30` | append to the all-guns list at `unit+424h` |
| 10 | `00956D3E`-`00956D51` | seed `unit+490h` and `unit+494h` with the float at `00CE38B8`, `10.0` |
| 11 | `00956D60`-`00956EC0` | per category: seed, then walk that category's list |

`00729F10(gun)` is `[[gun+3F0h]+720h] == 0 && [gun+3B8h] == 0 && [gun+5Dh] == 0`.

### The record and the node

Step 8 settles the layout the consumer side could only guess at. The record is a doubly
linked list header:

| Offset | Field |
| --- | --- |
| `unit+394h + cat*0Ch` | **count** |
| `unit+398h + cat*0Ch` | head |
| `unit+39Ch + cat*0Ch` | **tail** |

and the node is `operator new(0Ch)`: `prev` at `+0h`, `next` at `+4h`, the gun at `+8h`.
The same three words sit at `unit+424h`/`+428h`/`+42Ch` for the all-guns list.

The gunnery pass's "gate" at `unit+394h` is this count: `0086516D` computes
`unit + cat*0Ch` through `LEA EAX,[EAX+EAX*2]` and compares the dword with zero, taking the
`JE 00865884` skip when the category has no guns.

`00955EB0(record)` empties a list. Its pseudocode shows one erase and a return, which is the
decompiler's usual miss after a `_free`-class call: the listing has the back edge
`JNE 00955EB8` at `00955EF2` after the `_free` at `00955EE7`, so the body loops until the
count reaches zero.

### The engagement ranges

Step 11, per category `c`:

```
unit[430h + c*4] = 10.0f            ; 00956D63, the float at 00CE38B8
unit[460h + c*4] = 0.0f             ; 00956D6C
for gun in list(unit[398h + c*0Ch]):
    r = 00731020(gun+3F4h)          ; 00956D9F
    alt = (kind == 6 && c == 6) ? [[[gun+3F4h]+74h]+7Ch]+60h : 0.0f
    unit[430h + c*4] = max(unit[430h + c*4], r, alt)
    if kind in {2,3,4,6}: unit[490h] = max(unit[490h], r)     ; 00956E43
    unit[494h] = max(unit[494h], r)                           ; 00956E59
    if [gun+3F4h][+78h] > 0:
        d = [gun+3F4h][+74h] + ((kind == 6 && c == 6) ? 48h : 0)
        unit[460h + c*4] += ([[d+34h]+0B0h] + [[d+34h]+0ACh]) * 0.5
```

`00731020(desc)` returns `1.0f` when `[desc+78h] == 0`, otherwise `[[[desc+74h]+34h]+60h]`,
the ammunition record's range. So **`unit+430h + cat*4` is the longest weapon range among
that category's live guns, floored at `10.0`**, which is what `00863A34` gates a candidate's
distance against. `unit+490h` is the same maximum restricted to `Function` `2`, `3`, `4` and
`6`, and `unit+494h` is the maximum over every category. The `6` in those tests is `ECX`,
written once by `MOV ECX, 6` at `00956DB1`; `ECX` has no other write between the loop head
and the comparisons.

The scale `0.5` is the double at `00D7A280`.

## 3. The category index is the weapon `Function`

`[gun+3F4h][+80h]` is written only by `007327B0`, the gun class descriptor's Lua reader,
from the `Function` key of the device class row (`Scripts/datatables/classtables/<mode>/
deviceclasses.lua`). The chain at `007327E4`-`00732991` compares the key against twelve
string literals:

| Value | `Function` | Store |
| --- | --- | --- |
| `0` | `PLANEGUN` | `0073283A` (`EBP`, `XOR EBP,EBP` at `007327EC`) |
| `1` | `AAMACHINEGUN` | `00732857` |
| `2` | `LIGHTARTILLERY` | `00732878` (`EBX`, `MOV EBX,2` at `00732806`) |
| `3` | `MEDIUMARTILLERY` | `00732895` |
| `4` | `HEAVYARTILLERY` | `007328B6` |
| `5` | `FLAK` | `007328D7` |
| `6` | `LIGHTARTILLERYFLAK` | `007328F8` |
| `7` | `TORPEDO` | `00732919` |
| `8` | `DEPTHCHARGE` | `00732937` |
| `9` | `DEPTHCHARGELAUNCHER` | `00732955` |
| `0Ah` | `BOMBPLATFORM` | `00732973` |
| `0Bh` | `CATAPULT` | `00732991` |

`EBP` and `EBX` each have exactly one write across `007327B0`-`007329A4`, which is how the
two register-valued stores are read.

So the map from a gun to its gunnery category is the identity on this enum. There is no
sub-type table, and `docs/WEAPON_CLASS_DESCRIPTOR.md`'s reading of `+80h` as the weapon type
is the same field.

## 4. `GlobalConfig+88h`: `0087D7B0`

`0087D7B0` (body `0087D7B0`-`0087F96F`) runs `Scripts/datatables/Globals.lua` (the path
literal at `00D0E564`, pushed at `0087D81F`) and fetches the global table `Globals`
(`00CE490C` at `0087D882`), then fills the `2E8h` object `00432650` hands out. The gunnery
band is eight consecutive floats:

| Offset | Key in `Globals` | Site | Installed |
| --- | --- | --- | --- |
| `+88h` | `WeaponSystems.WeaponDirectorThinkTime` | `0087E16B` | `2` |
| `+8Ch` | `WeaponSystems.SafeToFireCacheTimeOut` | `0087E1A3` | `2` |
| `+90h` | `LineOfSight.TargetHeightAdd` | `0087E28A` | `5.0` |
| `+94h` | `LineOfSight.ViewerHeightAdd` | `0087E2CC` | `5.0` |
| `+98h` | `LineOfSight.TargetHeightMul` | `0087E30A` | `0.0` |
| `+9Ch` | `LineOfSight.ViewerHeightMul` | `0087E348` | `0.0`, see below |
| `+0A0h` | `LineOfSight.VisibleTimeOut` | `0087E210` | `5` |
| `+0A4h` | `LineOfSight.InvisibleTimeOut` | `0087E248` | `4` |

Every one is an `FSTP` of `BSP_LuaObject_GetNumber` (`00B66270`), which is `lua_tonumber`
with a float32 spill, so a key the table does not carry installs `0.0f`.

**`GlobalConfig+88h` is the weapon director think time, `2` seconds.** The gunnery pass
accumulates `dt` into `this+6Ch` and runs only when the accumulator reaches it, so at the
fixed step's `0.05f` the sweep runs once every forty steps; `00864C1D` primes the
accumulator to the same value so the first tick after attachment runs at once.

`+0A0h` and `+0A4h` are the visibility-cache TTLs the pass picks between, which is what
`00864D90`'s cache ages.

The shipped `Globals.lua` has no `ViewerHeightMul` key: it spells `ViewerHeightAdd` twice
inside `LineOfSight`, and the first of the pair carries the multiplier's comment while the
second shadows it with `5.0`. The intended `ViewerHeightMul = 0.0` and the missing key both
install `0.0f`, so the authoring slip has no effect; `ViewerHeightAdd` is `5.0`, not `0.0`.

## 5. `entity+1D4h` is the Lua "untouchable" flag

`00862440` reads `[entity->vtable[140h]() + 1D4h]`, a byte, and the gunnery sweep skips any
candidate for which it is set. `vtable[140h]` is a gunnery proxy accessor: `0047F320`
(`MOV EAX,ECX`, the entity itself) for everything on the ship base, `007B97E0`
(`[plane+9D4h]`) for a plane, `006F57A0` (`[fort+738h]`, or the fort itself when null) for a
land fort.

Three Lua bindings own the flag, and their names are recovered strings from the binding
table, whose records are `{name, fn}` pairs on an 8-byte stride:

| Binding record | Name | Function | Effect |
| --- | --- | --- | --- |
| `00E0C088` | `IsUnitUntouchable` | `008AC420` | reads the byte at `008AC552`, pushes the boolean |
| `00E0C090` | `AddUntouchableUnit` | `008AC140` | writes `1` at `008AC263` |
| `00E0C098` | `RemoveUntouchableUnit` | `008AC2B0` | writes `0` at `008AC3D6` |

The pairing is confirmed by behaviour, not only by the table stride: the only one of the
three that pushes a boolean is the one paired with `IsUnitUntouchable`, and the one that
writes `1` is the one paired with `Add`.

`BSP_GameEntity_Construct` initializes the byte at `00928767` with `BL`, and `EBX` is zeroed
once at `00928660` and never rewritten in the constructor, so a unit is touchable until a
script marks it.

So "`+1D4h`'s meaning is unread" resolves to: **a mission script can take a unit out of every
AI gunnery sweep with `AddUntouchableUnit(unit)`** without making it invulnerable to anything
else.

`unit+39Ch` also resolves here: it is the tail pointer of the same list, written by step 8.

## Open

- `00956C20`'s blast accumulator `unit+460h + cat*4` has no reader in this packet's reach;
  `contract: unread` on the consumer side.
- `00956C20` walks only the unit's **direct** children, where `008CF350` recurses with a
  worklist (`docs/UNIT_WEAPON_DEVICES.md`). A gun parented two levels down would be reachable
  by `GetGun` and absent from the gunnery index. Whether any shipped unit nests guns that
  deep was not checked.
- The `Function` chain's fall-through leaves `[desc+80h]` at whatever the descriptor
  constructor set; that default was not read.
- `007B97E0`'s `[plane+9D4h]` and `006F57A0`'s `[fort+738h]` were read as accessors only.
  What owns those pointers is out of this packet.

## Correction from docs/GAMEPLAY_LOOSE_ENDS_1.md (packet cc2_gameplay_loose_ends_1)

- **Was:** 00956C20's blast accumulator unit+460h + cat*4 has no reader in this packet's reach; contract: unread on the consumer side.
  **Is:** It has no live reader at all. The only two functions that read it are unreferenced accessors, so the field is written every pass and never consumed in the shipped build. There are twelve categories.
  **Evidence:** The only indexed accesses to 0x460 in .text are 00956D6C, 00956E94, 00956E9B and 009F9BE4; 009F9BE0 and 009F9BF0 have zero absolute occurrences and zero E8/E9 call sites. 00956EC0 CMP ESI,0xC with JL sets the category count. By contrast the sibling array unit+430h+cat*4 has live indexed readers at 00863A34 and 0095EBC4.
