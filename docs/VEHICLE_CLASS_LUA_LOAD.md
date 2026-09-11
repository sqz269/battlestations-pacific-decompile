# Vehicle class Lua load: the parent-most base and the eight ship leaf readers

Addresses: 0087C640 00877FA0 00837DE0 006E00F0 006EB4A0 006FB550 006FE6A0 0074C630 00759590
00854230 00857F40; read as contracts 00960230 00831840 0087CA80 00424C40 00424A10 0083B5E0
00424D00 00870CD0 00964790 004B1400 0047B3C0 00425850 00B646E0 00B64640 00413920 004134F0

## What this packet found, and what it did not have to

The packet was scoped as "decode `00960230` and `00831840` in full". Both were already decoded and
committed: `00960230` is `docs/VEHICLE_CLASS_FIELDS.md` (68 key sites, plus the shared pre-reader
`0087CA80`) and `00831840` is `docs/SHIP_CLASS_FIELDS.md` (67 keys, with the nested container
layouts corrected). Nothing of either is repeated here and no constant either header declares is
redeclared. The budget went instead to the three addresses of the packet that were genuinely
unread, `0087C640`, `00837DE0` and `006FE6A0`, and then to the seven sibling leaf overrides that
`006FE6A0` turned out to be one of.

The headline correction is that **`006FE6A0` reads no Lua key at all**. The descriptor doc's
follow-up table said the `MDestroyer` override "writes `+560h`..`+570h`"; it does, but the values
come from a global gameplay-settings block, not from the row. The same is true of all eight leaf
overrides: each chains to `00831840`, reads between zero and thirteen keys of its own, and ends by
copying one pair of dwords out of a per-class slot of that settings block.

## The eight leaf classes are recovered, not inferred

Each override is slot `+8h` of exactly one vtable, and each leaf vtable is followed by its
class-name literal at `+30h` (the convention `docs/VEHICLE_CLASS_DESCRIPTORS.md` established).
Reading the literal gives the class name as data rather than as a hypothesis.

| Override | vtable | name literal | class | row `Type` | shipped rows |
| --- | --- | --- | --- | --- | --- |
| `006E00F0` | `00D1ADF8` | `00D1AE28` | `MBattleship` | `BattleShip` | 40 |
| `006EB4A0` | `00D1ADBC` | `00D1ADEC` | `MCargo` | `Cargo` | 13 |
| `006FB550` | `00D1AD38` | `00D1AD68` | `MCruiser` | `Cruiser` | 41 |
| `006FE6A0` | `00D1ACF8` | `00D1AD28` | `MDestroyer` | `Destroyer` | 31 |
| `0074C630` | `00D1AD78` | `00D1ADA8` | `MLandingShip` | `LandingShip` | 6 |
| `00759590` | `00D1AEBC` | `00D1AEEC` | `MMothership` | `MotherShip` | 18 |
| `00854230` | `00D1AE38` | `00D1AE68` | `MSubmarine` | `Submarine` | 9 |
| `00857F40` | `00D1AE78` | `00D1AEA8` | `MTorpedoBoat` | `TorpedoBoat` | 3 |

The `Type` column is the one inference here: it is the row type whose keys match the override's
key list, and the eight names line up one-to-one with the eight `Type` values
`docs/SHIP_CLASS_FIELDS.md` already used as its ship-row rule. The factory's own `Type` dispatch
was not read, so a row could in principle reach a different leaf than its `Type` suggests.

Every override is `__thiscall(descriptor, LuaObject* row)` with `RET 4`, takes the descriptor in
`ECX` into `ESI` and the row from the stack into `EDI`, and forwards the row to `00831840` before
its own first key. `006FE6A0` is the one with no SEH frame, because it opens no `LuaObject`.

## The key table

`offset` is into the descriptor. The ship base `00963380` is `0804h` bytes, so every offset below
is the leaf's own storage and **the same offset means different things in different leaves**.
`site` is the `BSP_LuaObject_GetByName` call and `store` the instruction that writes the slot.

### `006E00F0` MBattleship

| Key (string) | Getter | Offset | Type | Default | Site / store |
| --- | --- | --- | --- | --- | --- |
| `Battlecruiser` (`00CF92F0`) | `00B662F0` GetBooleanOrDefault | `+808h` | byte | `false` (`PUSH 0`) | `006E0122` / `006E013C` |

### `006EB4A0` MCargo

The tuning copy runs first, at `006EB4C6`..`006EB508`; both bytes are then cleared at `006EB50E`
and `006EB515`, so a `SubType` that is not a string leaves both false.

| Key (string) | Getter | Offset | Type | Default | Site / store |
| --- | --- | --- | --- | --- | --- |
| `SubType` (`00CFA9D0`) == `Junk` (`00CFA9C8`) | `00B660A0` IsString, `00B662B0` GetString, `00425850` | `+808h` | byte | `false` | `006EB51C` / `006EB560` |
| `SubType` == `TroopTransport` (`00CFA9B8`) | the same string, second compare | `+809h` | byte | `false` | `006EB51C` / `006EB571` |

`00425850` is `BSP_NativeString_EqualsCStringInsensitive`, so both compares are case-tolerant.
The string is copied into a stack `NativeString` by `0041E870` and released through
`00419CC0` / `00BD1510` at `006EB584`.

### `006FB550` MCruiser

| Key (string) | Getter | Offset | Type | Default | Site / store |
| --- | --- | --- | --- | --- | --- |
| `HeavyCruiser` (`00CEB14C`) | `00B662F0` | `+808h` | byte | `false` (`PUSH 0`) | `006FB582` / `006FB59C` |

The byte is re-read at `006FB5AF` to pick the tuning pair, which is the only case where a leaf's
own key selects its tuning source.

### `006FE6A0` MDestroyer

No keys. The whole body after the `00831840` chain is the tuning copy, `006FE6AD`..`006FE6E3`.

### `0074C630` MLandingShip

| Key (string) | Getter | Offset | Type | Default | Site / store |
| --- | --- | --- | --- | --- | --- |
| `BigLandingShip` (`00CFFD08`) | `00B662F0` | `+808h` | byte | `false` | `0074C66A` / `0074C687` |
| `LandedDamage` (`00CFFCF8`) | `00B66380` GetIntegerOrDefault | `+80Ch` | int32 | `0` | `0074C6A5` / `0074C6C1` |
| `LandedCapturePower` (`00CFFCE4`) | `00B66380` | `+810h` | int32 | `0` | `0074C6DF` / `0074C6FB` |
| `LandingTroopType` (`00CFFCD0`) | `00B66380` | `+814h` | int32 | `1` (`PUSH 1` at `0074C71E`) | `0074C719` / `0074C736` |
| `Rocketer` (`00CFFCC4`) | `00B662F0` | `+809h` | byte | `false` | `0074C754` / `0074C770` |
| `SubType` == `LCVP` (`00CFFCBC`) | `00B660A0`, `00B662B0`, `00425850` | `+818h` | byte | `false` | `0074CB02` / `0074CB81` |
| `SubType` == `LSM` (`00CFFCB8`) | second compare | `+81Ah` | byte | `false` | `0074CB02` / `0074CBA1` |
| `SubType` neither, or not a string | fallthrough | `+819h` | byte | `true` | `0074CBAA`, `0074CBCE` |

The three `SubType` bytes are cleared together at `0074CAED`..`0074CAFB` and exactly one is set.

Between `Rocketer` and the tuning copy, `0074C796`..`0074CA53` walks
`globals.LandingTroopClasses[LandingTroopType].Classes` — read from the globals object `00B67980`
returns, **not** from the row. Each element's `Type` (`00CE4780`) is compared with `Soldier`
(`00CE693C`) and `Vehicle` (`00CE6928`) through `__stricmp` `00BF7FBF`; a soldier resolves
`ClassName` (`00CE6930`) through `004B1400` and a vehicle resolves `ClassId` (`00CE6920`) through
`00964790` `BSP_VehicleClass_GetOrCreate` then `0047B3C0`. The iteration is
`00B67080` / `00B67190` with `00B66420` IsUnbound as the end test. **No descriptor slot for the
result was decoded**; `004B1400` and `0047B3C0` were read only for their role.

### `00759590` MMothership

| Key (string) | Getter | Offset | Type | Default | Site / store |
| --- | --- | --- | --- | --- | --- |
| `RunwayWidth` (`00CF8AE8`) | `00B66270` GetNumber, no guard | `+820h` | float | none | `007595CA` / `007595E1` |
| `RunwayLength` (`00CF8AD8`) | `00B66270`, no guard | `+824h` | float | none | `00759606` / `0075961D` |
| `CarrierEscort` (`00D01918`) | `00B662F0` | `+82Ch` | byte | `false` (`PUSH 0` at `00759644`) | `0075963F` / `0075965C` |
| `MaxLandingPlanes` (`00D01900`) | `00B67690` Assign, `00B65FB0` IsNil, `00B66290` | `+828h` | int32 | `0` (else arm at `007596D8`) | `0075968E` / `007596D0` |
| `DeckCamera.Position` (`00CE68B4`) | `00B67A80` | `+860h`..`+868h` | three floats | none | `00759706` / `00759811`, `0075981F`, `0075982D` |
| `DeckCamera.HorzAngle` (`00D018E8`) | `00B66270` | — | float, negated | none | `0075973D` |
| `DeckCamera.VertAngle` (`00D018DC`) | `00B66270` | — | float, negated | none | `00759773` |

The `DeckCamera` sub-table is opened at `007596EA` and the three keys are read from it, not from
the row. The angle conversion is the one piece of x87 here and the assembly was opened for it:
`FLD` then `FCHS` at `0075979B`/`0075979F` for the horizontal angle and `007597BF`/`007597C3` for
the vertical, so **both angles are negated**. `00B646E0` builds one axis rotation from the negated
horizontal angle and `00B64640` the other from the negated vertical; `00413920`
`BSP_Matrix_Multiply4x4` multiplies them in that order and `004134F0` `BSP_Matrix_Copy4x4X87`
copies the product into `descriptor+830h` at `00759806`, a `30h`-byte basis that ends exactly
where the position vector at `+860h` begins.

### `00854230` MSubmarine

Every float key uses `00B66330` GetFloatOrDefault with a literal default.

| Key (string) | Offset | Type | Default (address) | Site / store |
| --- | --- | --- | --- | --- |
| `PeriscopeWave` (`00D0C2F8`) | `+80Ch` | effect handle | none, slot keeps its memset zero | `00854263` / `008542A5` |
| `PeriscopeDepth` (`00D0C2E8`) | `+810h` | float | `-1.0f` (`00D7A260`) | `00854304` / `0085431F` |
| `SwimDepth1` (`00D0C2DC`) | `+810h` | float | `-1.0f` (`00D7A260`) | `0085434A` / `00854365` |
| `SwimDepth2` (`00D0C2D0`) | `+814h` | float | `-1.0f` | `00854384` / `0085439F` |
| `SwimDepth3` (`00D0C2C4`) | `+818h` | float | `-1.0f` | `008543BE` / `008543D9` |
| `PeriscopeMoveRange` (`00D0C2B0`) | `+81Ch` | float | `10.0f` (`00CE38B8`) | `008543F8` / `00854413` |
| `UpDownStopTime` (`00D0C2A0`) | `+82Ch` | float | `5.0f` (`00CE3850`) | `00854432` / `0085444D` |
| `UpSpeed` (`00D0C298`) | `+830h` | float | `1.2f` (`00CE3814`) | `0085446C` / `00854487` |
| `DownSpeed` (`00D0C28C`) | `+834h` | float | `1.2f` (`00CE3814`) | `008544A6` / `008544C1` |
| `UpDownAccel` (`00D0C280`) | `+824h` | float | `0.25f` (`00CE3868`) | `008544E0` / `008544FB` |
| `UpDownRotation` (`00D0C270`) | `+828h` | float | `0.034906585f` (`00D0C26C`, two degrees) | `0085451A` / `00854535` |
| `AirRunOutTime` (`00D0C25C`) | `+838h` | float | `120.0f` (`00D05804`) | `00854554` / `0085456F` |
| `AirReloadTime` (`00D0C24C`) | `+83Ch` | float | `5.0f` (`00CE3850`) | `0085458E` / `008545A9` |

Two details the listing settles. `PeriscopeWave` is guarded by `00B66A60` IsInteger at `00854272`
and only then goes through `00B66290` and `00870CD0` `BSP_EffectHandle_Acquire`, with the usual
`00CE221C` / `00CE2220` refcount pair; a non-integer leaves the slot alone. And **`SwimDepth1` is
not a second field**: `00854332` does `XORPS XMM0,XMM0; COMISS XMM0,[ESI+810h]; JBE`, so the
`SwimDepth1` read happens only when `PeriscopeDepth` left `+810h` negative, and it writes the same
slot. It is the legacy alias for `PeriscopeDepth`.

`MSubmarine` is also the only leaf whose four tuning slots differ from one another, which is what
establishes `+560h`..`+56Ch` as an array of four rather than one scalar written four times.

### `00857F40` MTorpedoBoat

| Key (string) | Offset | Type | Default (address) | Site / store |
| --- | --- | --- | --- | --- |
| `TurboTime` (`00D061F0`) | `+808h` | float | `10.0f` (`00CE38B8`) | `00857F72` / `00857F90` |
| `TurboStrength` (`00D061CC`) | `+80Ch` | float | `3.0f` (`00CE3854`) | `00857FB3` / `00857FD1` |
| `TurboRechargingTime` (`00D061DC`) | `+810h` | float | `10.0f` (`00CE38B8`) | `00857FF4` / `00858012` |

## `00837DE0`, the tuning block selector

`__fastcall(settings)` with no stack arguments, `RET 0`, no callees, body `00837DE0`..`00837DFA`.
It loads the game object from `[00E188A8]`, tests the session-mode dword at `game+1FE4h`
(`00837DE5`) and returns `settings+0F0h` when it is non-zero, `settings+080h` otherwise
(`00837DEC`, `00837DF4`). `game+1FE4h` is the same session mode `docs/UNIT_ORDER_RECORD.md` and
`docs/UNIT_COMMAND_PRODUCERS.md` read; the two blocks are `70h` apart.

`settings` is always the singleton `00424C40` returns, re-fetched immediately before each call.
That singleton is `76Ch` bytes, allocated once through `operator_new` at `00424C9A`, constructed by
`00424A10`, stored at `00F8753C` and registered with `BSP_SingletonLifetime_Register`.

Each leaf reads one pair from the selected block. The block is therefore a per-ship-class table of
pairs, and the pairs partition cleanly:

| Block offset -> `+560h`..`+56Ch` | Block offset -> `+570h` | Leaf | Gate |
| --- | --- | --- | --- |
| `+04h` | `+00h` | `MMothership` | always |
| `+0Ch` | `+08h` | `MDestroyer` | always |
| `+14h` | `+10h` | `MTorpedoBoat` | always |
| `+1Ch` | `+18h` | `MLandingShip` | `BigLandingShip` false |
| `+24h` | `+20h` | `MLandingShip` | `BigLandingShip` true |
| `+2Ch` | `+28h` | `MBattleship` | always |
| `+34h` | `+30h` | `MCargo` | always |
| `+3Ch` | `+38h` | `MCruiser` | `HeavyCruiser` false |
| `+44h` | `+40h` | `MCruiser` | `HeavyCruiser` true |
| `+60h`, `+64h`, `+68h`, `+6Ch` | `+5Ch` | `MSubmarine` | always, four distinct slots |

`+48h`..`+58h` of the block is not touched by any of the nine call sites. The ninth caller,
`00424D00`, is not a class reader at all: it is the `AvoidZone` / `AvoidZoneG` parser
`BSP_Game_LoadMissionScene` runs, and it calls `00424C40` then `00837DE0` the same way, which is
independent confirmation that the receiver is the settings singleton and not a descriptor.

What `+560h`..`+570h` *mean* is not established. `00424A10` does not initialise `settings+80h`
..`+F0h`; the only candidate producer seen is `0083B5E0`, the 6412-instruction Lua-driven settings
loader `00424A10` tails into, which was not read.

## `0087C640`, the parent-most base

`__fastcall(void* this)` returning `this`, SEH handler `00C9686F`, body `0087C640`..`0087C6E9`.
It is what `00749050` `BSP_VehicleClass_ConstructBase` calls, and its vtable `00D0E13C` carries
`0087CA80` — the shared damageable pre-reader — at slot `+8h`. That closes the loop:
`00960230` calls `0087CA80` at `00960267` because it is overriding this class's slot `+8h` and
chaining to the base implementation first.

It writes two vptrs, its grandparent's `00CEB130` at `0087C65A` and its own `00D0E13C` at
`0087C66E`, so the grandparent's constructor is inlined and the refcount `[this+4h] = 1` at
`0087C665` belongs to that grandparent. Both vtables share `00BD30E0` at slot `+0h`.

| Offset | Value | Site |
| --- | --- | --- |
| `+0h` | vptr, `00CEB130` then `00D0E13C` | `0087C65A`, `0087C66E` |
| `+4h` | refcount `= 1` | `0087C665` |
| `+0Ch`, `+10h`, `+14h`, `+1Ch`, `+20h`, `+24h`, `+28h`, `+2Ch`, `+30h`, `+38h`, `+3Ch`, `+50h`, `+58h`, `+5Ch` | zero | `0087C678`..`0087C6A9` |
| `+40h` | `-1` | `0087C699` |
| `+44h` | byte zero | `0087C6A0` |
| `+60h`..`+68h` | an MSVC red-black tree, empty | `0087C6AC`..`0087C6D7` |

The tree is the standard MSVC `_Tree` triple: comparator at `+60h`, `_Myhead` at `+64h`, `_Mysize`
at `+68h`. `00877FA0` is its `_Buynode`: `operator_new(58h)`, zero `+0h`/`+4h`/`+8h`, `_Color`
(`+54h`) `= 1`, `_Isnil` (`+55h`) `= 0`. The caller then sets `_Isnil = 1` at `0087C6C2` and points
`_Left`, `_Parent` and `_Right` at the node itself (`0087C6C9`, `0087C6CF`, `0087C6D4`), the
one-node sentinel ring of an empty `std::map`. The node's value is `48h` bytes at node `+0Ch`; its
element type was not decoded, but `0087CA80`'s `Damage.Sections[]` walk is the only Lua producer in
this base and is the likely owner.

Six offsets in `+0h`..`+6Bh` are not written by this constructor: `+8h`, `+18h`, `+34h`, `+48h`,
`+4Ch` and `+54h`. Four of those six are exactly the scalars `0087CA80` fills — `+34h` `Unique`,
`+48h` `HP`, `+4Ch` `Armour`, `+54h` `Name` — which is a clean cross-check against
`docs/VEHICLE_CLASS_FIELDS.md`, arrived at independently. `+8h` and `+18h` stay unattributed, and
`+6Ch`, the secondary vptr, is written by `00749050`, not here. Anything not listed is zero only
because the factory memsets the allocation.

`0087C640` has two callers beyond `00749050` — `00440770` and `00442B90` — which were not read, so
the class has at least two siblings outside the vehicle hierarchy.

## Host table

One row per native call site these routines make, beyond the `BSP_LuaObject_*` wrappers
`docs/VEHICLE_CLASS_FIELDS.md` already tabulates.

| Site | Callee | Name | this / args | returns | Gate |
| --- | --- | --- | --- | --- | --- |
| `0087C6B6` | `00877FA0` | `std::_Tree::_Buynode` | `ECX` = tree at `+60h`, no args | the `58h` head node | always |
| `006E014F`, `006E0176`, `006EB4C6`, `006EB4ED`, `006FB5B8`, `006FB5DF`, `006FB60C`, `006FB62E`, `006FE6AD`, `006FE6D4`, `0074CA67`, `0074CA8E`, `0074CAA5`, `0074CACC`, `00759835`, `0075985C`, `008545BC`, `008545D1`, `008545E6`, `008545FB`, `00854610`, `00858029`, `00858050` | `00424C40` | gameplay settings singleton | none | the `76Ch` settings object | always |
| the instruction after each of the above | `00837DE0` | tuning block selector | `ECX` = settings, no args | `settings + 80h` or `+F0h` | always |
| `0085428D` | `00870CD0` | `BSP_EffectHandle_Acquire` | `PeriscopeWave` id | handle into `+80Ch` | `00B66A60` IsInteger at `00854272` |
| `006EB552`, `006EB566`, `0074CB70`, `0074CB90` | `00425850` | `BSP_NativeString_EqualsCStringInsensitive` | `ECX` = the `SubType` string, arg = a literal | bool into one byte | the value is a string |
| `0074C8F7`, `0074C9B3` | `00BF7FBF` | `__stricmp` | element `Type`, a literal | match | inside the `Classes` walk |
| `0074C97F` | `004B1400` | soldier-type resolve (contract, body unread) | `ClassName` string | not decoded | element `Type` is `Soldier` |
| `0074C9ED` | `00964790` | `BSP_VehicleClass_GetOrCreate` | `ClassId` integer | descriptor | element `Type` is `Vehicle` |
| `0074C9F4` | `0047B3C0` | landing-troop registration (contract, body unread) | the descriptor | not decoded | as above |
| `007597B0` | `00B646E0` | axis rotation from the negated `HorzAngle` (contract) | `ECX` = out, `EDX` = &angle | matrix | `DeckCamera` present |
| `007597D4` | `00B64640` | axis rotation from the negated `VertAngle` (contract) | `ECX` = out, `EDX` = &angle | matrix | `DeckCamera` present |
| `007597FA` | `00413920` | `BSP_Matrix_Multiply4x4` | two pushed matrices | product | `DeckCamera` present |
| `007597BA`, `007597E1`, `00759806` | `004134F0` | `BSP_Matrix_Copy4x4X87` | `ECX` = dest, arg = src | copy; the last writes `+830h` | `DeckCamera` present |
| `006EB584`, `0074C99F`, `0074CBC0` | `00419CC0` then `00BD1510` | `BSP_SizedStoragePool_GetSingleton` / `ReturnBlock` | the heap string | — | the `NativeString` grew a buffer |

## Installed-file cross-check

Method: `local/leafkeys.py` in the worktree parses the shipped
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/Scripts/datatables/autoload/vehicleclasses.lua`
read-only, splitting on `VehicleClass[n] =` and recording each row's `Type` and its indent-level-1
keys. It reports 633 rows and 197 distinct top-level keys, which matches the two earlier packets'
independent parsers (their 198 counts `Type` itself, which this parser records separately).

The eight leaves consume **29 row-level key paths** (`SubType` counted once although two leaves
read it, `DeckCamera` counted once at the top level plus its three sub-keys). Together with the
231 paths `00960230` consumes and the 67 `00831840` consumes, the readers now account for every
key path this reconstruction has traced into the vehicle-class descriptor.

Shipped row counts per leaf key are in `reports/vehicle_class_lua_load.json`. Two results are worth
stating here.

**Seven leaf keys no shipped row provides**, so they always take their literal default:
`MaxLandingPlanes` (0), `SwimDepth1`, `SwimDepth2`, `SwimDepth3` (all `-1.0f`), `UpDownAccel`
(`0.25f`), `UpDownRotation` (`0.034906585f`) and `UpDownStopTime` (`5.0f`). Since no row authors
`SwimDepth1`, the alias branch at `00854332` is dead in the shipped data and `+810h` is always
`PeriscopeDepth` or `-1.0f`; one of the nine `Submarine` rows omits `PeriscopeDepth` too, so that
one keeps `-1.0f`.

**Eight authored keys no leaf reader consumes for their row's type.** Seven `BattleShip` rows
(South Dakota 1945, King George V 1942, Iowa 1944, Montana 1945, North Carolina 1945, Lexington
1945, Louisiana 1945) author a `DeckCamera` block, but `DeckCamera` is read only by `MMothership`;
`MBattleship` reads only `Battlecruiser`. One more `BattleShip` row, `VehicleClass[389+]`
"Black Cat" (comment `HSF Hiei`), authors `Battlecruiser`, `RunwayWidth`, `RunwayLength`,
`TurboTime`, `TurboStrength`, `TurboRechargingTime` and `DeckCamera` at once; only `Battlecruiser`
reaches a reader. This conclusion rests on the `Type`-to-leaf mapping above, so it is as strong as
that mapping and no stronger.

`TurboTime` / `TurboStrength` / `TurboRechargingTime` appear in 26, 26 and 24 rows, but only the
three `TorpedoBoat` rows (and the one `BattleShip` row above) reach `00857F40`; the rest are
`Fighter`, `SmallReconPlane`, `DiveBomber` and `Kamikaze` rows read by the plane override
`007D1F70`, whose own decode is still only the key tabulation in `docs/VEHICLE_CLASS_FIELDS.md`.

## Coverage

| Routine | Coverage |
| --- | --- |
| `0087C640` | complete |
| `00877FA0` | complete |
| `00837DE0` | complete |
| `006E00F0`, `006EB4A0`, `006FB550`, `006FE6A0`, `00759590`, `00854230`, `00857F40` | complete |
| `0074C630` | partial: every row key and both tuning branches are decoded; the `LandingTroopClasses` walk `0074C796`..`0074CA53` is traced but its destination slots are not, because `004B1400` and `0047B3C0` were not read |
| `00960230`, `00831840`, `0087CA80` | unchanged; `docs/VEHICLE_CLASS_FIELDS.md` and `docs/SHIP_CLASS_FIELDS.md` |
| `00424A10` | read for the singleton's size and lifetime only; `+80h`..`+F0h` is not initialised there |
| `0083B5E0` | not read (6412 instructions) |
| `00424D00` | read for its `00837DE0` call site only |

### Not read

The six non-ship overrides of slot `+8h` were **not** read by this packet: `006D0B80` runway,
`00700E40` door, `00749210` structure, `0074D4A0` wreckable, `007D1F70` plane and the base
`00960230` itself. Their key tabulations stand in `docs/VEHICLE_CLASS_FIELDS.md`. Of the ship
family, all eight leaf overrides were read, so none of the "seven other ship overrides" is
outstanding.

## Uncertainties

* What `descriptor+560h`..`+56Ch` and `+570h` hold is not established. The structure is: a
  four-element array plus one scalar, per-class, session-mode dependent, and only the submarine
  varies across the four — which is suggestive given the submarine's four depth states, but no
  reader of those offsets was found.
* `settings+80h`..`+F0h` has no identified producer; `00424A10` does not write it.
* The `Type`-to-leaf mapping is inferred from key co-occurrence. The class names themselves are
  recovered literals; the mapping is not.
* The `48h`-byte value of the tree node at `0087C640+60h` was not decoded, nor was `+8h` or `+18h`
  of the damageable base.
* `004B1400`, `0047B3C0`, `00B646E0`, `00B64640` and `00B67A80` were read for their role at the
  call site only; none of their bodies was opened, so their names here are contracts.
* No run-time evidence: none of these routines was observed executing. `bsp_game.exe` does not
  reach the vehicle-class factory.

## Follow-up

| Packet | Addresses | Contract |
| --- | --- | --- |
| `ship_tuning_block` | 0083b5e0 00424a10 00424c40 | Find what fills `settings+80h`..`+F0h` and what reads `descriptor+560h`..`+570h`; name the four array slots |
| `landing_troop_classes` | 0074c796 004b1400 0047b3c0 | Decode where the `LandingTroopClasses` walk puts its resolved soldier and vehicle classes |
| `plane_class_fields` | 007d1f70 | Still open from `docs/SHIP_CLASS_FIELDS.md`: the 77-key plane reader, which owns most of the `Turbo*` rows |
