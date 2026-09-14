# The attack-capability inputs: the `vt[5Ch]` queries and the two classifiers

Addresses: `007EE8F0`, `00922B10`, `00922B70`, `00922B90`, `00922C80`, `00427EB0`, `00424C40`,
`008DDF90`, `00827F70`, `0042B8F0`, `004351E0`, `00435360`, `00480930`, `00929F79`, `004E7F31`,
`004E800E`, `00780E16`. Constants `00D7A348`, `00CE3938`.

## The answer in one line

`vt[5Ch](n)` is **answerable from a static table**: it is exactly "is `n` the entity's class id or
one of its ancestors", and the ancestor chains compiled into the binary form one consistent tree.
Unlike the projectile descriptor chains of `docs/ORDNANCE_KIND_IDENTITY.md`, the entity chains do
**not** diverge from the parent graph anywhere. Eight of the eleven inputs are therefore answerable
from authored data alone. Three are not, and they are named below.

Everything here is **exported / read**: the Ghidra listing, `.rdata` bytes, and a signature scan
over `.text` of the shipped PE. `00922B10` and `00922C80` are given as pure rules but **no C++ was
written, compiled, or run** for this packet.

## Part 1 — is `vt[5Ch]` table-answerable?

The test's shape is in `docs/ENTITY_CLASS_IDS.md` (`00480930`): a compiled chain of
`CMP EAX, <ancestor id>` / `JZ accept`, then `CMP EAX, dword ptr [ECX+0C4h]` against the instance's
own stamped id, then `XOR EAX,EAX / RET 4`. Three checks, run over the shipped PE on disk.

### Check A — the census is complete

Signature scan of `.text` for `3B 81 C4 00 00 00` (`CMP EAX,[ECX+0C4h]`), walking each hit back to
the preceding `CC` padding:

| measure | value |
| --- | --- |
| occurrences in `.text` | 88 |
| distinct enclosing function starts | 88 |
| `class_test` addresses in `reports/entity_class_ids.json` | 87 |
| in the report but not found by the scan | 0 |
| found by the scan but not in the report | 1 — `00435360` |

`00435360` is a **duplicate body**, not a missing class. It and `004351E0` (class `4Eh`'s recorded
test) compile the identical chain `4Eh, 4Ch, 0` plus the dynamic compare; the ledger already calls
them `BSP_ClassId4E_IsKindOf` and `BSP_ClassId4E_IsKindOfSecondBody`. Class `4Eh`'s vtable
`00CE3FD0` slot `5Ch` points at the second copy. No semantic difference.

### Check B — every compiled chain matches the parent column

All 87 test bodies decoded from the PE (`MOV EAX,[ESP+4]`, `CMP EAX,imm8`, `CMP EAX,imm32`,
`TEST EAX,EAX` for the root `0`, `CMP EAX,[ECX+0C4h]`, `SETZ`, `RET 4`; the decoder fails loudly on
any opcode it does not know and none appeared). Comparing each decoded id set against the chain
derived from the report's `parent` column alone:

```
classes in report: 87   decoded: 87   chain == parent-chain: 87
divergences: 0          undecoded: 0
classes missing the dynamic +C4h compare: none
```

### Check C — the chains are a consistent tree, with no circularity

Check B could be circular if the report's `parent` column had itself been read off these same
bodies. This check uses **only** the decoded sets: for each class `C`, is there exactly one class
`P` in `chain(C) \ {C}` whose own decoded chain equals `chain(C) \ {C}`?

```
87 classes, 1 root ['00'], 0 inconsistent
```

So the parent relation is recoverable from the compiled chains themselves. A host holding an
id-to-parent table and walking it reproduces `vt[5Ch]` exactly — for every class that has a test.

### The dynamic `+C4h` compare, and the one thing it is for

Every one of the 88 bodies ends with `CMP EAX,[ECX+0C4h]`, so a class that does **not** override
slot `5Ch` still answers its own id. Reading slot `5Ch` out of all 94 vtables the report lists:

```
94 vtables checked, 1 mismatch: id 4e, vtable 00CE3FD0, own test 004351E0, slot 5Ch 00435360
```

and that one is the byte-duplicate above. **For every class with a test, the dynamic compare
changes no answer in the shipped binary.**

### The gap: three class ids that have no test

Scanning `.text` for `MOV dword ptr [reg+0C4h], imm32` (`C7 8x C4 00 00 00 imm32`, all registers
but ESP): **92 genuine stamp sites, 80 distinct ids**, all `<= 60h`. Of those 80:

* 77 are among the 87 tested classes;
* 10 tested classes are never stamped, so they are abstract (`02`, `4E`..`56`);
* **three are stamped but have no class test of their own** —

| id | stamp site | enclosing function |
| --- | --- | --- |
| `58h` | `00929F79` | `BSP_TickableGameEntity_Construct` |
| `5Bh` | `004E7F31` | `FUN_004E7EF0` |
| `5Ch` | `004E800E` | `FUN_004E7F90` |

(`60h` is stamped at `00780E16` in `BSP_MultiScore_Construct` and *does* have a test, `007810F0`.)

An instance of `58h`, `5Bh` or `5Ch` answers its own id only through the dynamic compare, and its
vtable's slot `5Ch` holds some ancestor's chain that the id-to-parent table cannot name. A host must
treat these three as **unknown**, not as "answers false to everything". None is reachable from any
`VehicleClass.Type` (Part 3), so **no authored unit is one**; the gap is confined to non-unit
entities.

One of the three is resolved. `5Bh`'s constructor `004E7EF0` writes the primary vtable `00CE8BD0`
at `004E7EFF`, and `docs/ENTITY_CLASS_IDS.md` lists `00CE8BD0` as a vtable of class `01`; slot `5Ch`
of it (`00CE8C2C`) holds `0047F190`, class `01`'s test. So a `5Bh` instance answers exactly
`{5Bh, 01, 00}` and is false for every queried id. `58h` (`00929E50`, which writes a subobject
vtable `00CE89DC` at `+1A4h` but no primary vtable of its own — its base constructor `00925CE0` was
not read) and `5Ch` (`004E7F90`) were **not** resolved: their chains are undetermined here.

Two further byte matches, `00644F9E` and `0068BFE2`, are **not** class stamps: both are
`66 C7 86 C4 00 00 00 imm16`, 16-bit writes to `+C4h` on a `BSP_InGameHudRootScreen` object
(immediates `1` and `0FFFFh`). The scan matched from the `C7` inside the operand-size prefix.

**Discrepancy not resolved:** `docs/ENTITY_CLASS_IDS.md` records "92 times in `.text`, with 74
distinct immediates". The 92 agrees; the distinct count does not — this scan finds 80. Whichever is
right, the three untested ids above stand on their own stamp sites.

### The pure rule

```
bool Entity_IsKindOf(class_id own, uint8 queried)   // vt[5Ch](queried)
{
    for (id in ancestors_including_self(own))       // the chain ends at the root 0
        if (queried == id) return true;
    return false;                                   // 0 is an ancestor of every class
}
```
Valid for every class in the 87-row table. Undefined for `58h`, `5Bh`, `5Ch`.

## Part 2 — the two classifiers, reconstructed

Both are ordinary `__fastcall` functions and both are directly reconstructible. `00922C80` is
already named `BSP_Entity_IsSurfaceTarget` in the ledger (via `docs/GAMEPLAY_SETTINGS.md`).

### `00922B10` — airborne

`__thiscall(entity)`, `RET 0`, no stack arguments.

```
if (!entity)            return false;
if (entity->+5Dh != 0)  return false;                 // "no longer engageable"
return IsKindOf(0Fh) || IsKindOf(18h);                // any plane, or PlaneSquadronGen
```

### `00922C80` — attackable surface or ground target

`__fastcall(entity /* ECX */, bool allow_far /* DL, saved to BL at 00922C87 */)`, `RET 0`.

```
if (!entity)            return false;
if (entity->+5Dh != 0)  return false;
if (IsKindOf(0Fh))      return false;                 // any plane
if (IsKindOf(18h))      return false;                 // PlaneSquadronGen
if (IsKindOf(06h)) {                                  // the ship family
    sub = (IsKindOf(08h) ? entity : null);            // 00922B70
    if (!sub) return true;                            // any non-submarine ship
    pos = BSP_EntityPose_GetWorldPositionRefreshed(sub);           // 00427EB0, +4h is y
    s   = BSP_GameSettings_GetSingleton()->+49Ch;                  // SubmarinePeriscopeLevel
    return pos.y > -(s * 0.25);                                    // 00D7A348 = 0.25
}
if (IsKindOf(45h)) return true;                       // MAirfield
if (IsKindOf(46h)) return true;                       // MShipyard
if (IsKindOf(1Ch)) return true;                       // MCommandBuilding
if (BSP_SzurkeNyil_ContainsUnit(set, entity)) return true;         // 008DDF90
if (!allow_far)    return false;
if (IsKindOf(35h))                                    // the dummy-target vehicle
    return BSP_EntityPose_GetWorldPositionRefreshed(entity).y > 50.0;   // 00CE3938
fort = (IsKindOf(1Bh) ? entity : null);               // 00922B90, MLandFort
return fort && fort->+538h->+178h == 1Bh;
```

The two float comparisons are x87 and were read from the listing, not the decompiler:

* `00922CE8`-`00922D08`: `FLD [EAX+4]` (`pos.y`), `FLD [EDI+49Ch]`, `FCHS`, `FMUL qword [00D7A348]`,
  `FSTP`/`FLD` through a float temp, `FXCH`, `FCOMIP ST0,ST1`, `JBE reject`. So accept iff
  `pos.y > -(s * K)`, with the product rounded to float before the compare.
* `00922D77`-`00922D84`: `FLD qword [00CE3938]`, `FLD [EAX+4]`, `FCOMIP ST0,ST1`, `JBE reject`.

Constants read from the image: `00D7A348` = `00 00 00 00 00 00 D0 3F` = **0.25**;
`00CE3938` = `00 00 00 00 00 00 49 40` = **50.0**. `settings+49Ch` is
`Submarine.SubmarinePeriscopeLevel` (`docs/GAMEPLAY_SETTINGS.md:240`, default 20.0), so with the
shipped default a submarine is a surface target while `pos.y > -5.0`.

`00922B70` and `00922B90` are one-line dynamic casts: return `this` if `IsKindOf(8)` / `IsKindOf(1Bh)`,
else null.

## Part 3 — the eleven inputs

`BSP_Unit_AttackCommandApplies` is
`__thiscall(unit, commandClass a1, target a2, bool targetIsSurface a3, int loadout a4)`, `RET 10h`.
Stack arithmetic checked against both frame shapes: the `levelbomb` arm saves `EBP` at `007EE959`
(`PUSH EBP` followed by `PUSH 10h` / `CALL EAX`, where slot `5Ch` is `RET 4`, so the `EBP` push is a
save and not an argument), which is why the same two arguments are read at `[ESP+1Ch]`/`[ESP+20h]`
there and `[ESP+18h]`/`[ESP+1Ch]` everywhere else.

`unit+3CCh` is a count and `unit+3D0h` the array of planes it counts: `007ED830` loops
`[EBX+3CCh]` over `LEA EDI,[EBX+3D0h]`. The "self" queries all run on element 0.

| input | site | predicate | answers true for | host needs |
| --- | --- | --- | --- | --- |
| `unit_has_weapon_controller` | `007EE8FC` | `[unit+3D0h] != 0` | — | **runtime**: the squadron has a plane in slot 0 |
| `target_is_structure` | `007EE90A` | `vt[5Ch](1Ch)` | `1Ch MCommandBuilding` | authored: `Type = CommandBuilding` |
| `self_is_level_bomber` | `007EE95A` | `vt[5Ch](10h)` | `10h MPlaneBomber` | authored: `Type = LevelBomber` |
| `self_is_kamikaze_capable` | `007EEB53` | `vt[5Ch](17h)` | `17h MPlaneKamikaze` | authored: `Type = Kamikaze` |
| `self_is_dogfight_excluded` | `007EEB1A` | `vt[5Ch](16h)` | `16h MLargeReconPlane` | authored: `Type = LargeReconPlane` |
| `target_is_bomb_excluded` | `007EE9B2` | `vt[5Ch](0Eh)` | `0Eh MTorpedoBoat` | authored: `Type = TorpedoBoat` |
| `target_is_submarine` | `007EEABF` | `vt[5Ch](8)` | `8 MSubmarine` | authored: `Type = Submarine` |
| `target_is_kamikaze_ship` | `007EEB64` | `vt[5Ch](6)` | the ship family, 9 classes | authored: any of the 8 ship `Type`s |
| `target_is_strafe_fallback` | `007EEBA7` | `vt[5Ch](41h)` | `41h NavPoint` | **scene**, not a `VehicleClass`: creator `004E99B0`, `docs/SCENE_ENTITY_FACTORY.md` |
| `target_is_air` | `00922B10` | see Part 2 | planes and squadrons | class id **plus runtime** `+5Dh` |
| `target_is_surface` | `00922C80` | see Part 2 | see Part 2 | class id **plus runtime** `+5Dh`, world `y`, a unit set, and `+538h[+178h]` |

Seven of the eight `vt[5Ch]` queries are single leaves. The one family is `vt[5Ch](6)`, enumerated
from the decoded chains:

```
06 unnamed (abstract); 07 MDestroyer; 08 MSubmarine; 09 MMothership; 0A MCruiser;
0B MCargo;  0C MLandingShip; 0D MBattleship; 0E MTorpedoBoat
```

### From an authored `Type` to a class id

`docs/VEHICLE_CLASS_DESCRIPTORS.md` already binds `VehicleClass.Type` to the entity class id (its
`kind` column, 22 rows). The queried ids resolve to these authored types:

```
LevelBomber -> 10h    Kamikaze -> 17h    LargeReconPlane -> 16h
TorpedoBoat -> 0Eh    Submarine -> 08h   CommandBuilding -> 1Ch
ship family (06h): Destroyer 07h, Submarine 08h, MotherShip 09h, Cruiser 0Ah,
                   Cargo 0Bh, LandingShip 0Ch, BattleShip 0Dh, TorpedoBoat 0Eh
```

This is the same shape as the bullet `Type` mapping and, like it, should be **copied, not derived
from name similarity**: the binding is `docs/VEHICLE_CLASS_DESCRIPTORS.md`'s table, read from the
constructors, not an inference from the strings. `41h NavPoint` is the one queried id with no
`VehicleClass` row at all.

## Part 4 — the wiring contract

**Answerable from authored data alone (8 of 11).** All eight `vt[5Ch]` queries. The host needs one
`uint8 class_id` per entity, resolved at load from `VehicleClass.Type` (or, for `NavPoint`, from the
scene creator token), plus the 87-row id-to-parent table, walked to the root. No live vtable, no
class object.

**Needs runtime state as well (3 of 11).**

* `unit_has_weapon_controller` — whether the squadron currently has a plane in slot 0.
* `target_is_air` — the class id plus the target's live `+5Dh` byte.
* `target_is_surface` — the class id, `+5Dh`, the target's world `y` (for submarines and for class
  `35h`), `Submarine.SubmarinePeriscopeLevel` from game settings, membership of the unit set
  `008DDF90` tests, and `target+538h[+178h]` for the `MLandFort` tail.

A host that holds class ids and the parent table can compute the eight, and should refuse on the
three rather than approximate them. `+5Dh` in particular is a **kill/despawn** byte
(`docs/BOT_TASKS.md:273`, "marks a target no longer engageable") and cannot be guessed from authored
data; treating it as clear would make dead targets attackable.

## Part 5 — what this packet did not establish

* **`0047B850`, `00604A50` and `00828EC0`** are unread. They gate `dogfight`/`strafe`, `kamikaze`
  and `torpedo` respectively and are inputs to `007EEC50` just as much as the eleven above.
  `00827F70 BSP_VehicleClass_IsTorpedoBoatOrSmallLandingShip` is already named and takes the
  authored vehicle class `target+538h`, so it is likely answerable; the other three are unknown.
* **`plane+C24h`**, the byte `dogfight` and `strafe` require. `docs/GAME_AWARD_TRACKERS.md:174` and
  `docs/HUD_ROOT_UNIT_ROWS.md:104` both read it as the machine-gun/forward-gun flag, which fits both
  arms, but **its producer was not read** and this packet does not confirm it is authored.
* **`vehicleClass+178h`**, compared with `1Bh` in `00922C80`'s `MLandFort` tail.
  `docs/VEHICLE_CLASS_DESCRIPTORS.md` does not record `+178h`; with 368 `LandFort` rows this is
  presumably a sub-kind, but it was not read.
* **The identity of `58h` and `5Ch`**, and their ancestor chains — `5Bh`'s was resolved to
  `{5Bh, 01, 00}` above. Whether any of the three can ever be an attack target is also open.
* **`008DDF90 BSP_SzurkeNyil_ContainsUnit`** and the singleton path
  `[00E188A8] -> [+18ECh] -> [+ECX*4+21A4h]` that selects the set it tests.
* The 74-vs-80 distinct-stamp-id discrepancy with `docs/ENTITY_CLASS_IDS.md`.
* Nothing was compiled or run. No C++ was written for this packet.

# `009229F0` is not a liveness test (packet `cc7_target_still_attackable`)

Addresses: `009229F0` (body `009229F0`-`00922A33`, 33 instructions, read in full), `00922990`
(body `00922990`-`009229EE`, read in full), `007AC9D0` `BSP_Entity_PathInterfaceForKind`
(body `007AC9D0`-`007ACA2C`, read in full).

Ghidra read-only. **Exported, reconstructed, build-tested**; `reconstructed_math` still passes, no
new tests. Not fixture-tested, not game-validated. Mod-artefact caveat carried.

## The correction

`src/game_hosts_commands.cpp` calls it `target_still_attackable` and **this document** called it
"the shared target still attackable test". Both are wrong. There is no `+5Dh` read, no timer and
nothing temporal in the body. It is a **class test with a LandFort `FakedType` fallback**:

```
009229F3  if (!entity) return false
009229FC  if (entity->vtable[5Ch](kind)) return true
00922A0D  if (!entity->vtable[5Ch](1Bh)) return false          ; 1Bh = MLandFort
00922A1A  EAX = entity[+538h]                                  ; the vehicle class descriptor
00922A20  ECX = EAX[+178h]                                     ; FakedType
00922A2A  JMP 00922990(fakedType, kind)                        ; tail call
```

`class+178h` is the authored `FakedType` recovered in `docs/ATTACK_GATE_TAILS.md` — key
`"FakedType"` at `00CFF824`, written by `BSP_StructureClass_ReadLuaFields`, default `1Bh`.

## `00922990` is a hand-written table, and it disagrees with the parent chain

```
kind == 06h  ->  fakedType in { 07h, 08h, 0Ah, 0Bh, 0Dh, 0Eh }
kind == 0Fh  ->  fakedType in { 10h, 11h, 12h, 13h, 14h, 15h, 16h, 17h }
otherwise    ->  false
```

The plane arm is complete: all eight plane classes. **The ship arm is not.** Part 3 of this document
lists `vt[5Ch](6)` as nine classes — `06` plus `07 MDestroyer`, `08 MSubmarine`, `09 MMothership`,
`0A MCruiser`, `0B MCargo`, `0C MLandingShip`, `0D MBattleship`, `0E MTorpedoBoat`. The table
**omits `09 MMothership` and `0C MLandingShip`**.

A host answering the fallback with `Entity_IsKindOf(fakedType, 6)` would count a fort faking a
mothership or a landing ship as a ship target, and the game does not. Same hazard as the projectile
descriptors in `docs/ORDNANCE_KIND_IDENTITY.md`: **copy the table, do not derive it.**

## What this unblocks

The torpedo arm of `0099A170` is gated on this routine and on nothing it did not already have:

* the target's class id and the parent table — available through `unit_is_kind_of`;
* `FakedType` on structure rows — authored, producer known;
* **no live state at all.**

`entity_kind_or_faked_009229f0` and `faked_family_00922990` are in
`include/bsp/attack_target_classify.hpp`. **The torpedo arm can be wired.**

## The sibling, also read

`007AC9D0 BSP_Entity_PathInterfaceForKind(entity)` is a sub-object selector, not a predicate:

```
!entity        -> null
IsKindOf(47h)  -> entity + 1E4h        ; Path
IsKindOf(48h)  -> entity + 170h
IsKindOf(49h)  -> entity + 310h
IsKindOf(4Ah)  -> entity + 1E4h        ; 007ACA27 jumps back to the 47h arm
otherwise      -> null
```

`path_interface_offset_007ac9d0` returns the offset or `-1`. Classes `48h`, `49h` and `4Ah` were
**not** looked up in `docs/ENTITY_CLASS_IDS.md`, so only the `47h` arm is named.
