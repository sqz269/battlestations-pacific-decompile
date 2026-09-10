# Vehicle class descriptors

Addresses: 00964790, 00749050, 00963380, 00437F50, 00437BD0, 0095BA60, 0095BAB0, 0095B9C0,
0095B9E0, 0095BA30, 00963C20, 00963CB0, 00963D30, 00963DB0, 00963E40, 00963EC0, 00951AA0,
00951AF0, 00951B40, 00951C20, 00951D00, 00951DE0, 009536E0, 00953770, 0095FEE0, 0095FF50,
0095FFC0, 00749180, 00951E30, 00960050, 004E27E0, 00506550, 00592640, 00964020, 00425850

Proposed by `docs/SCENE_UNIT_CREATORS.md`. Every descriptive C++ name below is a hypothesis and
not a recovered symbol, with one exception that is now much larger than the predecessor doc
recorded: **all 22 class-name literals are recovered from the image**, not just `MDestroyer`.
They are returned by the descriptor's own vtable slot `+0Ch`, which is pure virtual in the base
(`00CFF7D8` = `00BF698E`, `__purecall`), and `00963B60` is `mov eax, 0D1AD28h ; ret` where
`00D1AD28` is the ASCII `MDestroyer`. The same slot on each of the other 21 vtables is the same
one-instruction constant load.

## Corrections to the predecessor doc

| Claim in `SCENE_UNIT_CREATORS.md` | What the image says |
| --- | --- |
| `singleton+10h` is one 4096-entry `Type`-to-class-index table | It is **two 2048-entry arrays**: `+10h` forward (`Type` -> class index) and `+2010h` inverse (class index -> `Type`). `2010h - 10h` and `4010h - 2010h` are each `2000h` bytes = 800h ints |
| the class name `MDestroyer` is the only genuine symbol | 22 class-name literals are recovered; see the table below |
| Destroyer's vtable is `00D1ACF8`, "second vptr at `+6Ch` = `00D1ACF4`" | Correct, and the pattern is general: every leaf writes `*this = V` and `*(this+6Ch) = V-4`. `00963380` itself writes `00D1ACC4`/`00D1ACC0`, which the inlined `MDestroyer` body then overwrites |

## The registry singleton, `00437F50`

`00437F50` is a `__fastcall` with no arguments returning the singleton in `EAX`, built once behind
a null test on `00F8A0B8`. Layout, from the accesses in `00964790`, `0095BA60`, `0095BAB0`,
`0095B9C0` and `00592640`:

| Offset | Field |
| --- | --- |
| `+4h` | descriptor cache, data pointer |
| `+8h` | descriptor cache, size |
| `+0Ch` | descriptor cache, capacity |
| `+10h` | forward index map, `800h` ints, keyed by the `Type` enum value |
| `+2010h` | inverse index map, `800h` ints, keyed by class index |
| `+4010h` | party-requires-class bitmap, data pointer, stride 3 |
| `+4014h` | bitmap size, grown by `004359E0` |

`00437BD0` is `__thiscall(int* vector /* = singleton+4 */, int size)`, `RET 4`: it grows through
`00436530` when the capacity is short, null-fills new slots, and on shrink releases each dropped
descriptor with `InterlockedDecrement(desc+4)` followed by `vtable[0]` at zero.

### The index map is the identity

`00592640` and `00506550` both contain the same loop, and it is the only initialisation either
array gets:

```
00592652: puVar3 = (uint *)(singleton + 0x2010);
          i = 0;
          do { puVar3[-0x800] = i;   /* singleton + 10h  + i*4 */
               *puVar3       = i;    /* singleton + 2010h + i*4 */
               ++i; ++puVar3; } while (i < 0x800);
00592667: *(int *)(singleton + 0x10   + a * 4) = b;
0059266E: *(int *)(singleton + 0x2010 + b * 4) = a;
```

So both arrays are reset to the identity and then **exactly one pair is rewritten**. The two
arrays are one bijection kept in both directions. `008CC4B0` asserts `inverse[i] != i` on a
specific index, which only holds for the one remapped pair.

This settles a contradiction the predecessor doc left open. `00964790` passes an already-resolved
class index to `0095BA60`, which maps it *again* through `+10h`; `0095BAB0` indexes the party
bitmap with the class index directly. Both are correct precisely because the map is the identity
for anything not remapped.

`004E27E0` writes the same pair, from a different caller. `0095BA30` resets both the cache and the
bitmap to size zero.

## `00964790`, the factory

`__fastcall(int typeId /* ECX */, bool readRace /* DL */)`, `RET 0`, returning the cache slot in
`EAX`. Frame: `AND ESP,0FFFFFFF8h` after the `EBP` push, SEH record with handler `00CAA1BF`, and
`EBX/EBP/ESI/EDI` saved, so every stack slot in the body is `+10h` from where the prologue writes
it. The body:

1. `classIndex = singleton[10h + typeId*4]`, read **twice** through two separate `00437F50` calls
   (`009647BB` and `009647C8`) into two stack slots.
2. Grow the cache to `classIndex + 1` when `classIndex >= singleton[8]`.
3. Cache probe at `009647ED`. The test compiles as `NEG EAX ; SBB EAX,EAX ; TEST EAX,0F8A0BDh ;
   JNZ`, which is `cached != 0` with a dead immediate. A hit jumps straight to `00965226`, which
   re-grows the cache and returns `cache[classIndex]`.
4. Miss path. `00B67980` on `(*00E188A8)+1A0Ch` gives the globals; `00B67800(.., "VehicleClass")`
   at `00CE5880`; `00B67720(table, classIndex)` indexes it; `00B67800(row, "Type")` at `00CE4780`,
   then `00B662B0` for the string, copied into a stack `NativeString` by `0041E870`.
5. `LandingShip` (`00CEB7BC`) through `00B66380` with default 0. Non-zero **recurses** into
   `00964790` with `DL = 1` (`009648E3`), so the companion class exists first.
6. **Write-back into the Lua row**, which the predecessor doc did not have:
   `00B67460(row, "ID", classIndex)` (`00CE59B4`) and `00B673A0(row, "Got", 1)` (`00CE452C`).
   Neither key is authored in the shipped file; both are runtime marks.
7. The 22-way comparison chain, `00425850` (`__thiscall(NativeString*, const char*)`, `RET 4`,
   delegating to `_stricmp`) against the literals in the table order below. `Destroyer` and
   `Cruiser` inline `operator new(size)` + `memset` + `00963380` + their own vptr pair; the other
   20 call `00470B80` (`BSP_Memory_AllocZeroed`) and a named constructor. No match returns 0 and
   caches nothing (`00964EA6`).
8. With `readRace` set: `Race` (`00CE8EE0`) through `00B66380` default 0, then
   `party = (race == 1 || race == 4) ? 1 : 0` (`00964F31`), then `0095BA60(classIndex, party)`.
9. `descriptor+70h = classIndex`; `descriptor+74h` receives the `Type` string by the
   resize-then-`memcpy` pair at `00964F66`/`00964F7B`.
10. `NumEngines` (`00D0651C`): nil (`00B65FB0`) gives 0, otherwise `00B66290` into `descriptor+7Ch`.
11. `"Class_"` (`00D1A95C`) into a stack string; when `vtable[18h](0Fh)` holds (the plane family),
    `0095BAB0(classIndex)` selects `"enemy_"` (`00D1A954`) or `"own_"` (`00D1A94C`) and appends it.
    `004CACA0(name, classIndex)` consumes it. Nothing else is appended.
12. Guarded by `00F8A098 == 0` and `!vtable[18h](19h)` and `!vtable[18h](1Bh)` (not a land vehicle,
    not a land fort): `operator new(1Ch)` and `00BE0A30(obj, &name, 00E0CFC0)`. This block is still
    not decoded; `00BE0A30` is `BSP_FileBlock_Construct`, so it is plausibly a per-class file
    handle, but that is not established.
13. `++00F8A098`; `vtable[8h](&outString)`, the Lua load; re-grow; release the old cache slot and
    store; `vtable[10h](0)`; `vtable[14h]()`; `--00F8A098`.
14. `vtable[18h](6)` (a ship) and `descriptor+0C0h != -1`: clear `00E0CFC0`, recurse into
    `00964790` with `DL = 1` for that class, restore `00E0CFC0`. Because the store in step 13
    already happened, a self-referencing row terminates.

`00F8A098` is a plain nesting counter, not a lock: only the outermost resolve builds the `1Ch`
object of step 12.

## The descriptor

### Base, `00749050`, `0138h` bytes

`__fastcall(void* this)`, returns `this`, SEH handler `00C874BB`. It calls `0087C640` (the
parent-most base, not read here), writes the vptr pair `00CFF7CC` / `00CFF7C8`, and zeroes
dwords `1Dh, 1Eh, 20h, 25h..27h, 2Dh, 37h..47h, 4Ah..4Ch` plus the byte at `48h`. The highest
index it touches is `4Ch`, i.e. `+130h`, which is why `0138h` is the base size and why the
smallest shipped descriptor (`Shipyard`) is exactly `0138h`.

Recovered fields:

| Offset | Field | Evidence |
| --- | --- | --- |
| `+0h` | primary vptr | every constructor |
| `+4h` | reference count | `InterlockedDecrement(desc+4)` in `00437BD0` and `0096511B` |
| `+54h` | a `char*` name | `00438020` reads `*(char**)(desc+54h)` and falls back to `"globals.unknown"` |
| `+6Ch` | secondary vptr, always `primary - 4` | every constructor |
| `+70h` | class index | `00964F5C` |
| `+74h`/`+78h` | `NativeString` of the row's `Type` | `00964F53`..`00964F80` |
| `+7Ch` | `NumEngines`, 0 when nil | `00964FAE`/`00964FBC` |
| `+0C0h` | linked class index, tested against -1 | `00965182`. No constructor writes it; it is zero out of the `memset` until the `vtable[8h]` load fills it |

### Vtable

Eleven slots in the base (`00CFF7CC`..`00CFF7F4`), twelve on the ship family. The base's
class-name string does not follow its vtable, but every leaf's does, which is how the 22 names
below were read.

| Slot | Base | Role | Evidence |
| --- | --- | --- | --- |
| `+0h` | `00BD30E0` | deleting-destructor trampoline for the refcounted base | called at refcount zero in `00437BD0` and `0096512E` |
| `+4h` | `00749160` | scalar deleting destructor (`00749163` then conditional `_free`) | body |
| `+8h` | `00960230` | load from the Lua row; takes a string out-parameter | `009650DA`; the base body is the large field reader |
| `+0Ch` | `00BF698E` | `GetClassName() -> const char*`, **pure** | `__purecall` in the base, `mov eax, <literal>; ret` in every leaf |
| `+10h` | `009598D0` | activate; `__thiscall(this, arg)`, walks the pointer vector at `+94h`/`+98h` when the byte at `+44h` is clear | `0096513F` calls it with 0 |
| `+14h` | `00951F20` | finalize; the base returns a global bool | `0096515D` |
| `+18h` | `00749010` | `IsKindOf(int kind)`, `RET 4`; the base accepts 4 and 5 | body |
| `+1Ch` | `00749140` | `GetKind() -> int`, `RET 0`; the base returns 5 | body |
| `+20h` | `0095F500` | not read | |
| `+24h` | `004407A0` | a float getter; Destroyer's `009635D0` is `fld [ecx+6B4h]; ret` | body |
| `+28h` | `00749150` | **allocate instance**, `__thiscall(this, int flag)`, `RET 4`. The base is `xor eax, eax; ret 4` | body; `docs/SCENE_UNIT_CREATORS.md` for the call site |
| `+2Ch` | absent | added by the ship base; `00827FB0` on every ship leaf | `00D1AD24` |

The secondary vtable at `descriptor+6Ch` has a single slot: `004E63F0` in the base, `00963B50`
(`mov eax, 1; ret`) on Destroyer. Its role is not established.

### `00963380`, the ship base, `0804h` bytes

`__fastcall(void* this)`, SEH handler `00CA9D9D`. Calls `00749050`, writes `00D1ACC4`/`00D1ACC0`,
then runs `_eh_vector_constructor_iterator_(this + 138h, 30h, 14h, 0095B920, 009560F0)` — twenty
`30h`-byte sub-objects starting **exactly where the base ends** — and zeroes 79 further dwords up
to index `200h` (`+800h`). Ghidra labels it `CG_array_ctor_helper_00963380`; that label is wrong,
it is an ordinary constructor.

`007D1E50` is the equivalent plane base (`060Ch` for all eight plane leaves) and `00749180` the
land-fort base; `00951E30` (`CommandBuilding`) calls `00749180`, so it derives from `LandFort`.

## The 22 kinds

Descriptor size is the allocation argument in `00964790`; instance size is the `operator new`
argument inside the slot `+28h` routine; kind is the slot `+1Ch` constant; rows is the count in
the installed `Scripts/datatables/autoload/vehicleclasses.lua`. Order is the comparison order.

| `VehicleClass.Type` | class name (recovered) | desc | instance | kind | constructor | vtable | slot `+28h` | rows | SEH |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `Destroyer` | `MDestroyer` | 808h | 1188h | 7 | inline + `00963380` | `00D1ACF8` | `006FE590` | 31 | 0Ah |
| `Cruiser` | `MCruiser` | 80Ch | 1188h | 0Ah | inline + `00963380` | `00D1AD38` | `006FB430` | 41 | 0Bh |
| `LandingShip` | `MLandingShip` | 81Ch | 122Ch | 0Ch | `00963C20` | `00D1AD78` | `0074BE00` | 6 | 0Ch |
| `Cargo` | `MCargo` | 80Ch | 118Ch | 0Bh | `00963CB0` | `00D1ADBC` | `006EB290` | 13 | 0Dh |
| `BattleShip` | `MBattleship` | 80Ch | 118Ch | 0Dh | `00963D30` | `00D1ADF8` | `006DFEF0` | 39 | 0Eh |
| `Submarine` | `MSubmarine` | 840h | 1288h | 8 | `00963DB0` | `00D1AE38` | `008531A0` | 9 | 0Fh |
| `TorpedoBoat` | `MTorpedoBoat` | 814h | 1190h | 0Eh | `00963E40` | `00D1AE78` | `00857E20` | 3 | 10h |
| `MotherShip` | `MMothership` | 870h | 12C8h | 9 | `00963EC0` | `00D1AEBC` | `00758D30` | 18 | 11h |
| `ReconPlane` | `MReconPlane` | 60Ch | E94h | 14h | `00951AA0` | `00D19BF4` | `008091D0` | 0 | 12h |
| `SmallReconPlane` | `MSmallReconPlane` | 60Ch | E94h | 15h | `009536E0` | `00D1A5A8` | `0084CA50` | 5 | 13h |
| `LargeReconPlane` | `MLargeReconPlane` | 60Ch | E94h | 16h | `00953770` | `00D1A5EC` | `0074E540` | 4 | 14h |
| `Fighter` | `MPlaneFighter` | 60Ch | E94h | 13h | `00951AF0` | `00D19C30` | `007DDAE0` | 28 | 15h |
| `DiveBomber` | `MPlaneDiveBomber` | 60Ch | E94h | 12h | `00951B40` | `00D19C70` | `00956390` | 9 | 16h |
| `TorpedoBomber` | `MPlaneTorpedoBomber` | 60Ch | E94h | 11h | `00951C20` | `00D19F4C` | `009564E0` | 9 | 17h |
| `Kamikaze` | `MPlaneKamikaze` | 60Ch | E94h | 17h | `00951D00` | `00D1A224` | `00956240` | 6 | 18h |
| `LevelBomber` | `MPlaneBomber` | 60Ch | E94h | 10h | `00951DE0` | `00D1A4F8` | `007D7850` | 11 | 19h |
| `AirField` | `MAirfield` | 140h | 8E4h | 45h | `0095FEE0` | `00D1A9A0` | `006D3110` | 2 | 1Ah |
| `Shipyard` | `MShipyard` | 138h | 7A4h | 46h | `0095FF50` | `00D1A9DC` | `00848380` | 2 | 1Bh |
| `LandVehicle` | `MLandVehicle` | 150h | 740h | 19h | `0095FFC0` | `00D1AA18` | `0074DF10` | 21 | 1Ch |
| `LandFort` | `MLandFort` | 180h | 758h | 1Bh | `00749180` | `00CFF790` | `00747000` | 368 | 1Dh |
| `CommandBuilding` | `MCommandBuilding` | 1ACh | 7E8h | 1Ch | `00951E30` | `00D1A538` | `006F5C10` | 7 | 1Eh |
| `DummyTargetVehicle` | `MDummyTarget` | 144h | none | 35h | `00960050` | `00D1AA58` | `00749150` | 1 | 1Fh |

`DummyTargetVehicle` is the only class that never overrides slot `+28h`: it inherits the base's
`xor eax, eax; ret 4` and builds no instance at all. Its secondary vtable slot is also the base's
`004E63F0`.

### Where each constructor diverges from the base

Nineteen of the twenty named constructors are a base call plus the vptr pair and nothing else.
The exceptions, all reading as extra zeroed fields past their base:

| Constructor | Base it calls | Extra work |
| --- | --- | --- |
| `0095FFC0` `LandVehicle` | `00749050` | zeroes `4Eh, 50h..53h` |
| `00960050` `DummyTargetVehicle` | `00749050` | zeroes `50h` |
| `00749180` `LandFort` | `00749050` | zeroes `4Fh, 51h..53h, 55h..58h` |
| `00963DB0` `Submarine` | `00963380` | zeroes `203h` (`+80Ch`) |
| `00951E30` `CommandBuilding` | `00749180` | vptr pair only, but its base is the fort |

`LandingShip`, `Cargo`, `BattleShip`, `TorpedoBoat` and `MotherShip` are allocated larger than
`00963380`'s `0804h` and leave the extra bytes at the `memset`'s zero.

### `MDestroyer` in full

`Destroyer` is not a separate constructor: `00964997`..`009649D6` is `operator new(808h)`
(`00BF55BE`), `memset(p, 0, 808h)` (`00BF79F0`), `00963380(p)`, `*p = 0D1ACF8h`,
`*(p+6Ch) = 0D1ACF4h`. So the whole `MDestroyer` constructor body in the image is two vptr stores;
everything else is `00963380` and `00749050`. Its slot `+28h`, `006FE590`, is `__thiscall(desc,
int flag)`, `RET 4`, and news `1188h` bytes — the value `docs/SCENE_UNIT_CREATORS.md` recorded.

Its overrides against the base: `+4h` `009652B0`, `+8h` `006FE6A0` (writes `+560h`..`+570h` from
`00837DE0`'s object), `+0Ch` `00963B60` -> `"MDestroyer"`, `+14h` `00828F20`, `+18h` `00963B70`
(kinds 4, 5, 6, 7), `+1Ch` `00963B40` -> 7, `+20h` `0082FE30`, `+24h` `009635D0`
(`fld [ecx+6B4h]`), `+28h` `006FE590`, `+2Ch` `00827FB0`.

### The kind lattice

Kinds 5, 6 and `0Fh` are never a leaf's own value. `00749010` (base) accepts 4 and 5; `00963B70`
(Destroyer) accepts 4, 5, 6 and 7. With the constructor chains, that gives 4 -> 5 (`00749050`) ->
6 (`00963380`, ships 7..0Eh) and 5 -> `0Fh` (`007D1E50`, planes 10h..17h), plus the flat leaves
19h, 1Bh -> 1Ch, 35h, 45h and 46h. The factory's four is-kind queries are `0Fh` (plane naming),
`19h` and `1Bh` (skip the `1Ch` object) and `6` (the ship recursion).

## The party bitmap

`0095BA60`, `__fastcall(int typeId, int party)`, `RET 0`. Returns immediately unless `party` is 0
or 1; otherwise `classIndex = forward[typeId]`, grows `singleton+4010h` through `004359E0`, and
sets `byte[base + classIndex*3 + party] = 1`.

`0095BAB0`, `__fastcall(int classIndex)`, returns
`byte[base + classIndex*3 + localParty] == 0`, where `localParty` is
`*(*(00E188A8 + 18CCh + *(00E188A8+18ECh)*4) + 28h)` and the whole thing short-circuits to a
zero-extended garbage value unless `*(00E188A8+18ECh)` is in `[0, 8)`. True therefore means "the
local player's party has not registered a need for this class", which is why `00964790` selects
`"enemy_"` on true. The third byte of each row is never written by either routine.

`0095B9C0(descriptor)` returns `inverse[descriptor->classIndex]` and is used by `0095BB10` when it
publishes a unit's script state, so the inverse map is read back per descriptor.

## The installed table

`I:/SteamLibrary/steamapps/common/Battlestations Pacific/Scripts/datatables/autoload/vehicleclasses.lua`,
191122 lines, 3158242 bytes, read-only (`local/vc_scan.py`). It opens with `VehicleClass = {}` and
then `VehicleClass[n] = -- <comment>` per row.

* **633 rows**, indices 1..999, so 366 indices in the span are unused. Both index arrays hold
  `800h` = 2048 entries, comfortably above 999.
* **305 distinct keys** across all rows. `Type`, `Name`, `Width`, `Length`, `Mesh`, `Armour`,
  `Comment`, `HP`, `Height` and `ReconClass` are on all 633.
* Per-`Type` row counts match the table above exactly, and the 21 distinct values present are the
  factory's 22 literals minus `ReconPlane`, which has a branch and no shipped row.
* `Race` on 232 rows, values 0 (9), 1 (124), 2 (93), 3 (7), 4 (2), 5 (1). Only 1 and 4 map to
  party 1, so the two `Race = 4` rows join the 124 `Race = 1` rows on that side.
* `NumEngines` on 72 rows; `LaunchedClass` and `Catapult` on 58; `LandingShip` on exactly **2**
  (line 61063 `= 90`, line 62199 `= 40`).
* `ID` and `Got` appear on **zero** rows. Both are written by `00964790` at run time, confirming
  step 6 above is a write-back rather than a read.

## Callers and callees

`00964790` has **55 callers** in the index, far more than the ten scene unit creators of
`docs/SCENE_UNIT_CREATORS.md`; it is the general entry point for "give me the class object for this
`Type`", not a scene-loading helper. Among them: itself twice, `0046DF00`
(`BSP_SceneDatabase_LoadSceneFile`) and `00438020`, which walks every class index, takes
`*(char**)(desc+54h)` (or `"globals.unknown"` for a null descriptor) and resolves it through the
localisation table. Its own
callees include `00437F50`, `00437BD0`, `00470B80`, `0095BA60`, `0095BAB0`, `004CACA0`, `00425850`
and the 20 named constructors, plus the Lua wrappers `00B67980`, `00B67800`, `00B67720`,
`00B662B0`, `00B66290`, `00B66380`, `00B65FB0`, `00B67460`, `00B673A0` and `00B67700`, which are
read-only here and belong to the mission/GUI Lua packets.

`00964020` iterates the whole `VehicleClass` table with `lua_next` and is the "resolve every class"
pass; it was not read past its head.

## Uncertainties and what remains

* The `1Ch`-byte object of step 12 and its `00BE0A30` construction are undecoded, as is
  `00E0CFC0`, the byte the ship recursion clears and restores.
* Slot `+20h` (`0095F500`) and slot `+24h` (`004407A0`) have no established role; only Destroyer's
  `+24h` override was read.
* The secondary vtable's single slot (`004E63F0` base, `00963B50` Destroyer) is unidentified.
* `00960230`, the base's Lua load at slot `+8h`, is a large field reader that was not decoded, so
  which Lua key fills which offset **beyond `+70h`, `+74h` and `+7Ch`** is unknown. That is the
  main gap: the base's other `0138h - 80h` bytes are opaque.
* `00438020` was read only to its loop head; `004359E0` and `004CACA0` were not read at all.
* Why `00964790` reads the forward map twice through two `00437F50` calls is unexplained; the two
  copies are used interchangeably.
* Which caller of `00592640` / `00506550` / `004E27E0` decides the one remapped pair, and what it
  means, is not established.
* `0087C640`, the parent-most descriptor base, was not read, so `+0h`..`+6Bh` is unattributed
  except for the refcount at `+4h` and the name pointer at `+54h`.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `vehicle_class_lua_load` | 00960230 006fe6a0 00831840 00837de0 0087c640 | `docs/VEHICLE_CLASS_LUA_LOAD.md` | The base's slot `+8h` field reader: which `vehicleclasses.lua` key fills which descriptor offset, and the `MDestroyer` override that writes `+560h`..`+570h` |
| `vehicle_class_index_remap` | 00592640 00506550 004e27e0 008cc4b0 0095b9c0 | `docs/VEHICLE_CLASS_INDEX_REMAP.md` | Who chooses the single remapped `Type`/class pair, what the inverse map is read for, and the third byte of each `+4010h` party row |
| `vehicle_class_enumeration` | 00438020 00964020 004caca0 004359e0 | `docs/VEHICLE_CLASS_ENUMERATION.md` | The two whole-table passes: the localisation-name build and the `lua_next` resolve-all, and what `004CACA0` does with `"Class_own_"` |
| `vehicle_class_kind_lattice` | 00749010 00749140 00963b70 007d1e50 0095f500 004407a0 | `docs/VEHICLE_CLASS_KIND_LATTICE.md` | The is-kind predicate on all 22 vtables, the plane base `007D1E50`, and slots `+20h`/`+24h` |

## State reached

| Address | Name | State |
| --- | --- | --- |
| 00964790 | `BSP_VehicleClass_GetOrCreate` | exported, analyzed, reconstructed, build-tested, installed-file-checked |
| 00749050 | `BSP_VehicleClass_ConstructBase` | exported, analyzed, reconstructed (layout only) |
| 00963380 | `BSP_ShipClass_ConstructBase` | exported, analyzed |
| 00437F50 | `BSP_VehicleClassRegistry_GetSingleton` | analyzed (listing only for the body; layout from its callers) |
| 00437BD0 | `BSP_VehicleClassRegistry_ResizeCache` | exported, analyzed |
| 0095BAB0 | `BSP_VehicleClass_IsEnemyOfLocalParty` | exported, analyzed, reconstructed |
| 0095BA60 | (already named by `scene_unit_creators`) | exported, analyzed, reconstructed |
| 00963C20, 00963CB0, 00963D30, 00963DB0, 00963E40, 00963EC0 | ship leaves | exported, analyzed, reconstructed as data |
| 00951AA0, 00951AF0, 00951B40, 00951C20, 00951D00, 00951DE0, 009536E0, 00953770 | plane leaves | exported, analyzed, reconstructed as data |
| 0095FEE0, 0095FF50, 0095FFC0, 00749180, 00951E30, 00960050 | flat leaves | exported, analyzed, reconstructed as data |
| 0095B9C0, 0095B9E0, 0095BA30 | registry helpers | exported, analyzed |
| 00592640, 00506550, 004E27E0 | index-map writers | exported, analyzed (the loop and the pair write only) |
| 00438020, 00964020, 004CACA0, 004359E0 | enumeration and helpers | exported, not analyzed |
| 00425850 | existing reconstruction record | read only |

Every vtable slot function named above (`00749010`, `00749140`, `00749150`, `00749160`,
`00963B40`, `00963B50`, `00963B60`, `00963B70`, `009635D0`, `00951C10`, `00951CF0`, `00951DD0`,
`00960090`) is **listing only, with no Ghidra function**; end addresses are in
`reports/vehicle_class_descriptors.json` under `no_ghidra_function` so the orchestrator can define
them before applying names. They were read with `disasm-raw` and none was claimed.

## Reconstruction

`include/bsp/vehicle_class.hpp` and `src/vehicle_class.cpp`:

* `kVehicleClassKindTable`, the 22 kinds as data, in the factory's comparison order, carrying
  every column of the table above.
* `VehicleClassIndexMap`, the two `800h`-entry arrays with the identity reset and the single-pair
  remap as methods.
* `vehicle_class_party_from_race_00964f31`, `vehicle_class_party_slot_0095ba60` and
  `vehicle_class_debug_name_00964fbf` as pure functions.
* `VehicleClassHost`, one virtual per native call site of `00964790`, in body order, with no
  defaults, and `resolve_vehicle_class_00964790` over it. The Lua half repeats `GuiLuaHost`'s
  operations rather than inheriting them because the factory also writes into the row.
* The descriptor's `Type` string reuses `bsp::NativeString` from `include/bsp/native_string.hpp`,
  which is the same eight-byte `{length, buffer}` object the factory copies into `+74h`.

The one behavioural divergence, marked in the source: on a null allocation the native carries the
null descriptor into the tail and dereferences it, while the reconstruction stops and reports
`AllocationFailed`.
