# The bullet class engagement range: `006E9890` and the kind it switches on

Addresses: `006E9890`, `00855A90`, `0070C0B0`, `006EAFE0`, `006E97F0`, `006E8770`, `006E8320`,
`006EA1C0`, `006EA200`, `006EA260`, `006EA330`, `006EA3A0`, `006EA470`, `006EA4F0`, `006EA5E0`,
`006EA6B0`, `006EA720`, `006EA7F0`, `006EA870`, `006EA910`. Data `00CFA424`, `00CFA428`,
`00CF9058`, `00CEFF98`, `00D7A218`, `00D7A248`, `00CF0B50`, `00CE3808`, `00CFA420`, `00E199AC`,
`00CE8ED0`, `00CE3A0C`, and the thirteen descriptor vtables `00CFA138`, `00CFA440`, `00CFA478`,
`00CFA4B0`, `00CFA4DC`, `00CFA508`, `00CFA53C`, `00CFA56C`, `00CFA59C`, `00CFA5CC`, `00CFA604`,
`00CFA63C`, `00CFA678`.

Report: `reports/cc7_bullet_engagement_range_kinds.json`. Header
`include/bsp/bullet_engagement_range.hpp`, source `src/bullet_engagement_range.cpp`.

`docs/TORPEDO_CATEGORY_ADMISSION.md` established that the gun's range getter `00731020` answers
with `descriptor+60h` and that `+60h` is not the authored Lua `Range`. This document recovers the
routine that produces `+60h`, the value it switches on, that value's producer, and what the rule
gives for every bullet class in this installation.

The installation is **modded** (BSPRM/AlterBSP). `scripts/datatables/classtables/arcade/
bulletclasses.lua` has mtime 2026-05-09 23:04 and is 75048 bytes with 119 rows;
`deviceclasses.lua` has mtime 2026-05-09 22:37. Nothing in the installation was modified. Every
count below is **this installation's**, not retail's.

## 1. `006E9890` in full

`__fastcall(ProjectileClassDesc* this)`, `RET 0`, `bool` in `AL`, body `006E9890`-`006E9A47`,
coverage **complete**. It is slot `+10h` of the class-descriptor vtable and is reached only
through that slot or through the two overrides that call it.

```
006E9896  if ([this+64h] != 0) goto 006E9A41        ; the once-only latch
006E98A1  kind = [this+8h]; [this+64h] = 1
```

Then exactly one of four arms, on `kind`:

| arm | site | kinds | what it writes |
| --- | --- | --- | --- |
| artillery | `006E98BC` | `4`, `5`, `6`, `7` | `+60h = Range`; `+50h = sqrt(Range * 9.81)`; `+5Ch = V0 / that` |
| gun | `006E9941` | `1`, `2`, `3`, `10h` | if `Range > 0`: `+54h = Range / V0`. Then `+60h = +54h * +50h` |
| depth charge | `006E9923` | `0Bh` | `+60h = 240.0f` (`00CFA428`) |
| default | `006E9932` | everything else | `+60h = 3000.0f` (`00CFA424`) |

The artillery arm is the one the decompiler renders least well; from the listing:

```
006E98BC  MOVSS XMM0,[EDI+68h]        ; Range
006E98C7  FLD float [ESP+8]           ; the same value
006E98CB  FMUL double [00CF9058]      ; * 9.8100004196167 = (double)9.81f
006E98D1  FSTP float [ESP+0Ch]        ; rounded to a float BEFORE the sqrt
006E98D9  CALL 00BF7030               ; sqrt on the x87 stack
006E98DE  FSTP float [ESP+0Ch]        ; s = (float)sqrt(...)
006E98F0  MOVSS [EDI+60h],XMM0        ; +60h = Range, unchanged
006E98F5  FLD float [EDI+50h]         ; the authored V0
006E98F8  FLD float [ESP+0Ch]         ; s
006E98FC  FLD ST0
006E98FE  FDIVP ST2,ST0               ; V0 / s
006E9900  FXCH
006E9902  FSTP float [EDI+5Ch]        ; +5Ch = V0 / s
006E9905  FSTP float [EDI+50h]        ; +50h = s
```

`sqrt(g * R)` is the launch speed that reaches range `R` on a flat 45-degree arc (`R = v^2 / g`),
so the artillery arm **replaces the authored `V0` with the speed implied by the authored `Range`**
and keeps the ratio of the two in `+5Ch`, the field the constructor initialises to `1.0f` at
`006E83E7`. `docs/TORPEDO_CATEGORY_ADMISSION.md` listed only the `+50h` store; `+5Ch` is the
correction.

The gun arm's guard is `COMISS XMM0,[00D7A218]` / `JBE`, and `00D7A218` is `0.0f`, so the division
happens only on a strictly positive, ordered `Range`. Because the same `V0` is then multiplied
back in, **a gun class that authors a `Range` round-trips to it** - that is the only reason the
host's `bc.Range` model has worked for the artillery categories.

### The `+64h` latch and a second call

`006E9896` tests `[this+64h]`; a non-zero latch jumps straight to `006E9A41`, which loads `AL = 1`
and returns. A second call therefore **writes nothing at all** - not `+60h`, not `+50h`, not the
`+8h` rewrite below - and still reports success. The constructor clears the latch at `006E83CD`.

There is a second, outer latch: `006EAFE0` (section 4) only dispatches the pair when
`[desc+0C8h] == 0`, and slot `+14h` sets `+0C8h` at `006E9878`.

### The tail: `006E9890` rewrites the kind

`006E9968`-`006E9A40` runs after the range arm and rewrites `+8h` for two kinds.

`kind == 1`: `[this+14h]` is taken as a `char*`, defaulting to the empty literal `00E199AC` when
null; `0041E870` builds a temporary native string from it; `00BF9440` is `strstr` against the
literal `"AA"` at `00CFA420`. `006E99A6 CMP EAX,-1` / `006E99AE SETNZ BL` / `006E99D5 ADD EAX,2`
/ `006E99D8 MOV [EDI+8],EAX`, so **`+8h` becomes `3` when the name contains `AA` and `2` when it
does not**. The match is case-sensitive and matches anywhere in the string.

`kind == 4`: two `COMISS` pairs with the threshold on the left and `JBE` falling through only on a
strict greater-than:

```
006E99E7  [00CF0B50] = 75.0f  > [this+0ACh] AND > [this+0B4h]  -> +8h = 5
006E9A14  [00CE3808] = 150.0f > [this+0ACh] AND > [this+0B4h]  -> +8h = 6
006E9A39  otherwise                                            -> +8h = 7
```

`+0ACh` is `DamageMin` and `+0B4h` is `Blast.BlastDamageMin` (`006E8770`'s key table,
`docs/WEAPON_CLASS_DESCRIPTOR.md`), so the artillery tier is a damage tier.

**The rewrite does not change `+60h`.** `1`, `2` and `3` share one arm and `4`..`7` share another,
and the latch stops the hook from ever seeing a rewritten value. This matters for the host: only
the **constructor** sub-type is needed to compute the engagement range.

`docs/WEAPON_CLASS_DESCRIPTOR.md` records that "`2`, `3`, `5`, `6` and `7` have no producer: no
constructor writes them". That is true of the constructors and wrong about the binary:
**`006E9890` is their producer.** Any consumer that switches on `+8h` sees the rewritten value,
because the gun only ever reaches a finalised descriptor.

## 2. What the kind is, and where a bullet class gets it

The kind is the **projectile class sub-type at descriptor `+8h`**, the constant each class's
constructor stores. Every one was read from its own instruction:

| Lua `Type` | native class | constructor | store | `+8h` |
| --- | --- | --- | --- | --- |
| Bullet | MBullet | `006E8320` | `006E83D0 MOV [EAX+8],EDX` (`EDX = 1` from `006E8343`) | `1` |
| Artillery | MArtilleryBullet | `006EA1C0` | `006EA1CE` | `4` |
| Bomb | MBomb | `006EA260` | `006EA278` | `9` |
| Torpedo | MTorpedo | `006EA4F0` | `006EA50C` | `0Ah` |
| DepthCharge | MDepthCharge | `006EA3A0` | `006EA3BC` | `0Bh` |
| DummyTarget | MDummyTarget | `006EA6B0` | `006EA6C8` | `0Ch` |
| DummyKamikazePlane | MDummyKamikazePlane | `006EA7F0` | `006EA808` | `0Dh` |
| DummySubmarine | MDummySubmarine | `006EA870` | `006EA888` | `0Eh` |
| Paratrooper | MParatrooper | `006EA720` | `006EA742` | `0Fh` |
| Flak | MFlakBullet | `006EA470` | `006EA47E` | `10h` |
| Kamikaze | MKamikazePlane | `006EA200` | `006EA20E` | `11h` |
| Rocket | MRocket | `006EA330` | `006EA348` | `12h` |
| WaterMine | MWaterMine | `006EA5E0` | `006EA5FC` | `13h` |

**A host determines the kind from the row's `Type` string, and from nothing else.** `006EA910`
picks the constructor with thirteen case-insensitive name comparisons at `006EAAE6`-`006EACE0`
(`docs/WEAPON_CLASS_DESCRIPTOR.md`); no match stores a null class pointer and the gun cannot fire.
`include/bsp/projectile_kinds.hpp` already carries that chain as
`projectile_class_for_lua_type`, and `weapon_class_sub_type_for_lua_type` in this packet's header
wraps it. So `GameBulletClassRow::type` **is** the key to the kind, one-to-one, with no second
table.

The kind's two rewrite inputs have producers too. `+14h` is **not** a separate field: `006E8770`
reads the `Name` key (`00CE8ED0`, default `""` at `00CE3A0C`) into the native string at
descriptor `+10h`, and a native string is `{+0h: size, +4h: char* data}` - the copy at
`006E87E7`-`006E87F3` pushes `[EBP]`, `[EBX+4]` and `[EBP+4]` into `memcpy` with `LEA EBP,[ESI+10h]`
as the destination. So `+14h` is the `Name` string's character data, and the constructor zeroes it
at `006E834E`, which is the null the default at `006E9974` covers.

## 3. The eleven tables

Census method: an image-wide byte scan for `90 98 6e 00` and `ghidra xrefs 006E9890`, which
returned **13 rows uncapped** (11 `DATA`, 2 `UNCONDITIONAL_CALL`). `ghidra xrefs` caps at 25 rows;
13 is under the cap, and the byte scan agrees, so the census is complete.

| vtable | slot `+10h` | class | ctor `+8h` | hook |
| --- | --- | --- | --- | --- |
| `00CFA138` | `00CFA148` | MBullet | `1` | `006E9890` |
| `00CFA440` | `00CFA450` | MArtilleryBullet | `4` | `006E9890` |
| `00CFA478` | `00CFA488` | MKamikazePlane | `11h` | `006E9890` |
| `00CFA4B0` | `00CFA4C0` | MBomb | `9` | `006E9890` |
| `00CFA4DC` | `00CFA4EC` | MRocket | `12h` | `006E9890` |
| `00CFA508` | `00CFA518` | MDepthCharge | `0Bh` | `006E9890` |
| `00CFA59C` | `00CFA5AC` | MWaterMine | `13h` | `006E9890` |
| `00CFA5CC` | `00CFA5DC` | MDummyTarget | `0Ch` | `006E9890` |
| `00CFA604` | `00CFA614` | MParatrooper | `0Fh` | `006E9890` |
| `00CFA63C` | `00CFA64C` | MDummyKamikazePlane | `0Dh` | `006E9890` |
| `00CFA678` | `00CFA688` | MDummySubmarine | `0Eh` | `006E9890` |
| `00CFA53C` | `00CFA54C` | MFlakBullet | `10h` | **`0070C0B0`** |
| `00CFA56C` | `00CFA57C` | MTorpedo | `0Ah` | **`00855A90`** |

`0070C0B0`, body `0070C0B0`-`0070C0C9`, coverage complete: it calls the base, checks `AL == 1` and
derives `+0D8h = [+58h] / [+50h]` (`MinRange / V0`). **It does not touch `+60h`**, so a flak class
keeps the gun arm's `FlyTime * V0`.

`00855A90`, read as a contract (`cc7_torpedo_category_admission` owns it):

```
00855A95  base = 006E9890(this)         ; puts 3000.0f in +60h, torpedoes taking the default arm
00855A9A  FLD float [ESI+0E4h]          ; WaterTravelSpeed
00855AA0  FMUL float [ESI+54h]          ; * FlyTime, kept at extended precision
00855AA5  FMUL double [00CEFF98]        ; * 0.6000000238418579 = (double)0.6f
00855AAB  FSTP float [ESI+60h]          ; the only rounding
00855AAE  +0ECh = sqrt(2 * MaxFall * 9.81); return base && +0ECh > 0
```

Because `006E9890` writes `3000.0f` first and `00855A90` overwrites it, a torpedo's range never
depends on the default constant - but a reader of the base alone would wrongly conclude it does.

## 4. Where the hook is called

`006EAFE0`, `__fastcall(int* outHolder, int bulletId)`, SEH frame, body `006EAFE0`-`006EB05D`,
coverage complete. This settles the "invocation site: unread" clause in
`docs/TORPEDO_CATEGORY_ADMISSION.md`.

```
006EB00F  CALL 006EA910                 ; build or fetch the cached descriptor into *out
006EB014  ECX = [ESI]                   ; the descriptor
006EB016  NEG/SBB/TEST                  ; skip everything when it is null
006EB033  CMP byte [ECX+0C8h],0 ; JNZ   ; the outer once-only latch
006EB03C  EDX=[ECX]; EAX=[EDX+14h]; CALL EAX   ; slot +14h, the load pass
006EB043  ECX=[ESI]; EDX=[ECX]; EAX=[EDX+10h]; CALL EAX   ; slot +10h, this hook
006EB050  return ESI
```

Neither dispatch uses a `CALL dword ptr [reg+10h]` memory operand - that encoding does not occur
anywhere in the image (`FF 50 10`, `FF 51 10`, `FF 52 10`, `FF 53 10`, `FF 55 10`, `FF 56 10`,
`FF 57 10` all return zero `.text` matches). Every virtual dispatch is `MOV reg,[vtbl+off]` then
`CALL reg`, which is why a scan for the direct form finds nothing.

Callers of `006EAFE0`: `006E195F`, `00731A9C`, `008222E8` (`BSP_UnitInstance_HandleMessage`),
`004E9FB1` (`BSP_SceneWaterMine_Instantiate`). `00731A9C` is on the gun path, so **every
descriptor a gun reaches has been finalised before `00731020` reads `+60h`**.

Slot `+14h` is `006E97F0` for ten of the thirteen classes, `00700150` for MDummyTarget and
`006FE730` for MDummyKamikazePlane. `006E97F0` is **contract: unread**; this packet read only far
enough to see it store `[ESI+0C8h] = 1` at `006E9878`.

## 5. The constants

| address | bytes | value | use |
| --- | --- | --- | --- |
| `00CFA424` | `00 80 3b 45` | `3000.0f` | the default arm |
| `00CFA428` | `00 00 70 43` | `240.0f` | sub-type `0Bh` |
| `00CF9058` | `00 00 00 60 b8 9e 23 40` | `9.8100004196167` | `(double)9.81f`, the artillery arm |
| `00CEFF98` | `00 00 00 40 33 33 e3 3f` | `0.6000000238418579` | `(double)0.6f`, the torpedo |
| `00D7A218` | `00 00 00 00` | `0.0f` | the `Range > 0` threshold |
| `00D7A248` | `ff ff 7f 7f` | `FLT_MAX` | the constructor's `FlyTime` default (`006E83D3`) |
| `00CF0B50` | `00 00 96 42` | `75.0f` | artillery tier -> `5` |
| `00CE3808` | `00 00 16 43` | `150.0f` | artillery tier -> `6` |
| `00CFA420` | `41 41 00` | `"AA"` | the `Name` substring test |
| `00E199AC` | `00` | `""` | the `Name` fallback when `+14h` is null |

## 6. The rule applied to this installation's 119 bullet classes

`ArcadeTable` has 119 rows. **Exactly the 32 `Artillery` rows author a `Range`; not one of the
other 87 does.** That is the whole of the brief's "87 of 119 carry no `Range`", and it is not a
coincidence: the artillery arm is the only arm that consumes `Range`, and the authors only wrote
it where it is consumed.

Under the rule, **all 87 classes that today receive `max_range = 0` receive a strictly positive
range**, and none produces zero, infinity or NaN.

The six cases the packet asked for:

| case | bullet class | `Type` | today | rule | derivation |
| --- | --- | --- | --- | --- | --- |
| fighter machine gun | `92` "30mm type 5" | Bullet | `0` | **`960.00006`** | `FlyTime 1.2 * V0 800` |
| AA machine gun | `36` "25mm/60 AA" | Bullet | `0` | **`960.00006`** | `FlyTime 1.2 * V0 800` |
| flak gun | `39` "5inch/38 destroyer AA Flak bullet" | Flak | `0` | **`2400`** | `FlyTime 3.0 * V0 800` |
| depth charge | `12` "Depth Charge ship US" | Depthcharge | `0` | **`240`** | the constant; this class authors no `FlyTime` at all |
| rocket | `8` "5'' HVAR" | Rocket | `0` | **`3000`** | the default constant |
| artillery control | `1` "14/45 KGV primary" | Artillery | `3000` | **`3000`** | the authored `Range`, unchanged |

The fighter case is concrete: `VehicleClass[322]` (`ln Reppu`, the A7M) has five gun platforms,
four of them `Gun[1] = 102`, which is `deviceclasses.lua` `ArcadeTable[102]`, `Function` =
`PLANEGUN`, `Bullet` = `92`. Those are the four category-0 guns with a `range` column of `0`.
The fifth platform is device `89`, a `BOMBPLATFORM` firing bullet class `79`.

**The `V0 = 0` rockets are a non-issue.** All five rocket classes (`6`, `8`, `11`, `51`, `116`)
author `V0 = 0` and a finite `FlyTime`, but sub-type `12h` never reaches the `FlyTime * V0` arm -
it takes the `3000.0f` default at `006E9932`. The packet brief's worry about a zero range from
that arm does not arise, and neither does the brief's "kinds 4-7 -> Range, bombs and rockets":
bombs are `9` and rockets are `12h`, and both take the default.

The artillery control round-trips exactly: `+60h` comes back as `3000`, while `+50h` becomes
`sqrt(3000 * 9.81) = 171.55` and `+5Ch` becomes `300 / 171.55 = 1.7487`.

### Are the five silent categories silent for one reason?

Grouping `deviceclasses.lua` by `["Function"]` and resolving each row's `["Bullet"]` ids:

| function | devices | bullet class ids | authored `Range`? | rule gives |
| --- | --- | --- | --- | --- |
| PLANEGUN | 12 | 82, 84, 86, 87, 88, 89, 90, 91, 92, 95 | none | 750 - 960 |
| AAMACHINEGUN | 78 | 36, 40, 41, 42, 43, 46, 84, 89, 91, 95, 107, 108 | none | 800 - 1600 |
| FLAK | 12 | 5, 14, 19, 39, 44, 49 | only `19`, which is `Artillery` | 1000 - 2400 |
| DEPTHCHARGE | 8 | 12, 32, 54, 57, 144, 154 | none | 240, and 3000 for `144`/`154` |
| TORPEDO | 29 | 61, 62, 65, 66, 67, 70 | none | 1851.98 - 6135.98 |
| HEAVYARTILLERY | 49 | 1, 16, 23, 25, 38, 48, 50, 100, 101, 102, 110-113, 166, 997 | every `Artillery` id | the authored `Range` |

So the five silent categories **do** share one cause: none of their bullet classes authors a
`Range`, every gun gets `max_range = 0`, and `00956D63` leaves the category at its `10.0f` seed.
The working control, HEAVYARTILLERY, is the one group whose classes all author `Range`.

This is a **sufficient** cause for all five, established statically. It is **not** proven to be
the only cause for each: that requires an IJN01 or USN02 run after the host is wired, which this
packet cannot do because it does not own `src/game_hosts_gunnery.cpp`. Two things to watch in that
run: `144` (DummySubmarine, the kaiten) and `154` (WaterMine) are reached through DEPTHCHARGE
devices and take the `3000.0f` default rather than the depth-charge constant, and FLAK's class
`19` is `Artillery`, not `Flak`.

### The whole table

`kind` is `constructor +8h -> +8h after the 006E9968 rewrite`. `range` is the `+60h` the rule
produces. Blank means the key is absent from the row.

| id | Name | Type | kind | arm | Range | V0 | FlyTime | `+60h` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | 14/45 battleship gun bullet - KGV prim | Artillery | 0x04 -> 0x07 | authored range | 3000 | 300 | - | **3000** |
| 2 | Hedgehog Projector Depth Charge | Depthcharge | 0x0B -> 0x0B | constant 240 | - | 50 | 20 | **240** |
| 3 | battleship sec. gun bullet - KGV secon | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 4 | Fido homing airplane torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 0 | 60 | **1111.19043** |
| 5 | 4.7'' AA rocket | Flak | 0x10 -> 0x10 | fly time times v0 | - | 200 | 5 | **1000** |
| 6 | Tiny Tim | Rocket | 0x12 -> 0x12 | constant 3000 | - | 0 | 10 | **3000** |
| 7 | 5-Inch bombardment rocket | Artillery | 0x04 -> 0x07 | authored range | 4000 | 300 | - | **4000** |
| 8 | 5'' HVAR | Rocket | 0x12 -> 0x12 | constant 3000 | - | 0 | 6 | **3000** |
| 9 | 8/55 heavy cruiser gun bullet - Northa | Artillery | 0x04 -> 0x07 | authored range | 2300 | 300 | - | **2300** |
| 10 | Japan 800 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 11 | R4M AA | Rocket | 0x12 -> 0x12 | constant 3000 | - | 0 | 4 | **3000** |
| 12 | Depth Charge ship US | Depthcharge | 0x0B -> 0x0B | constant 240 | - | 20 | - | **240** |
| 13 | 6/47 cruiser gun bullet - De Ruyter | Artillery | 0x04 -> 0x06 | authored range | 1900 | 300 | - | **1900** |
| 14 | Akizuki Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 800 | 2.5 | **2000** |
| 15 | 5inch/38 destroyer gun bullet - Fletch | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 16 | 15 battleship gun bullet - Repulse pri | Artillery | 0x04 -> 0x07 | authored range | 3000 | 300 | - | **3000** |
| 17 | 4inch destroyer gun bullet - Clemson | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 18 | submarine bullet | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 19 | 3.9/65 combined gun bullet - semmi | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 20 | Akizuki Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 800 | 2.5 | **2000** |
| 21 | kamikaze plane impact | Kamikaze | 0x11 -> 0x11 | constant 3000 | - | - | - | **3000** |
| 22 | Paratrooper | Paratrooper | 0x0F -> 0x0F | constant 3000 | - | - | - | **3000** |
| 23 | 18/45 battleship gun bullet - Yamato p | Artillery | 0x04 -> 0x07 | authored range | 3300 | 300 | - | **3300** |
| 24 | 6/60 cruiser gun bullet - Yamato secon | Artillery | 0x04 -> 0x07 | authored range | 1800 | 300 | - | **1800** |
| 25 | 14/45 battleship gun bullet | Artillery | 0x04 -> 0x07 | authored range | 3000 | 300 | - | **3000** |
| 26 | Huge Battery DLC Shell | Artillery | 0x04 -> 0x07 | authored range | 2900 | 300 | - | **2900** |
| 27 | 17.7 Type 91 Mod3 airplane torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 0 | 60 | **1851.98413** |
| 28 | 8/50 heavy cruiser gun bullet | Artillery | 0x04 -> 0x07 | authored range | 2200 | 300 | - | **2200** |
| 29 | 22.4 Mark 13 airplane torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 0 | 60 | **1851.98413** |
| 30 | 5.5 cruiser gun bullet | Artillery | 0x04 -> 0x06 | authored range | 1700 | 300 | - | **1700** |
| 31 | fubuki bullet nemdebar | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 32 | Depth Charge ship US | Depthcharge | 0x0B -> 0x0B | constant 240 | - | 10 | - | **240** |
| 33 | submarine bullet | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 34 | deck gun bullet | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 35 | kamikaze US10 impact | Kamikaze | 0x11 -> 0x11 | constant 3000 | - | - | - | **3000** |
| 36 | 25mm/60 AA | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 37 | Agano 6'' (DR clone) | Artillery | 0x04 -> 0x07 | authored range | 1900 | 300 | - | **1900** |
| 38 | Huge Battery Long Range Shell | Artillery | 0x04 -> 0x07 | authored range | 10000 | 300 | - | **10000** |
| 39 | 5inch/38 destroyer AA Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 800 | 3 | **2400** |
| 40 | 20mm/70 Oerlikon AA | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 41 | 28mm quad AA | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 42 | 40mm Bofors AA | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 2 | **1600** |
| 43 | .5 cal twin AA mg | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 44 | 5inch/38 destroyer AA Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 800 | 2.5 | **2000** |
| 45 | 13mm/76 AA | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 46 | 25mm/60 AA | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 2 | **1600** |
| 47 | US 725 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 48 | 20/45 battleship gun bullet - Super Ya | Artillery | 0x04 -> 0x07 | authored range | 3300 | 300 | - | **3300** |
| 49 | Jap Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 800 | 2.5 | **2000** |
| 50 | 12 heavy cruiser gun bullet - Alaska D | Artillery | 0x04 -> 0x07 | authored range | 2300 | 300 | - | **2300** |
| 51 | 5'' HVAR | Rocket | 0x12 -> 0x12 | constant 3000 | - | 0 | 6 | **3000** |
| 54 | Depth Charge ship US | Depthcharge | 0x0B -> 0x0B | constant 240 | - | 10 | - | **240** |
| 55 | Depth Charge aerial US | Depthcharge | 0x0B -> 0x0B | constant 240 | - | 0 | - | **240** |
| 57 | Depth Charge ship Jap | Depthcharge | 0x0B -> 0x0B | constant 240 | - | 5 | - | **240** |
| 58 | Depth Charge aerial Jap | Depthcharge | 0x0B -> 0x0B | constant 240 | - | 0 | - | **240** |
| 61 | 21. Mark 14 submarine torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 23 | 60 | **1851.98413** |
| 62 | 21. Mark 15 ship torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 13 | 60 | **1851.98413** |
| 63 | 22.4 Mark 13 airplane torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 0 | 60 | **1851.98413** |
| 64 | 19. Mark 24 airplane torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 0 | 60 | **1851.98413** |
| 65 | 21. Mark 14 submarine torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 23 | 60 | **1851.98413** |
| 66 | 21. Type 95 Mod1 submarine torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 23 | 60 | **1851.98413** |
| 67 | 24. Type 93 Mod1 Long Lance ship torpe | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 13 | 60 | **6135.98438** |
| 69 | 17.7 Type 91 Mod3 airplane torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 0 | 60 | **1851.98413** |
| 70 | 21. Type 95 Mod1 submarine torpedo | Torpedo | 0x0A -> 0x0A | torpedo override 00855A90 | - | 23 | 60 | **1851.98413** |
| 71 | US 500 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 72 | US 250 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 73 | US 100 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 74 | US 50 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 75 | US incendiary bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 77 | Japan 500 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 78 | Japan 250 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 79 | Japan 100 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 80 | Japan 50 kg bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 81 | Jap incendiary bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 82 | 20mm type 99m1 | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 750 | 1 | **750** |
| 84 | .30 cal | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 85 | .50 cal | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 86 | 20mm M2 | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 87 | .30 cal | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 88 | .50 cal | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 89 | 7.7mm type 97 | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 90 | 13.2 mm Type 2 | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 91 | 20mm type 99m1 | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 92 | 30mm type 5 | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 95 | .50 cal | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 98 | kamikaze plane impact | Kamikaze | 0x11 -> 0x11 | constant 3000 | - | - | - | **3000** |
| 99 | normal plane impact | Kamikaze | 0x11 -> 0x11 | constant 3000 | - | - | - | **3000** |
| 100 | 16/?? battleship gun bullet - Iowa pri | Artillery | 0x04 -> 0x07 | authored range | 3300 | 300 | - | **3300** |
| 101 | 16/?? battleship gun bullet - Iowa pri | Artillery | 0x04 -> 0x07 | authored range | 3000 | 300 | - | **3000** |
| 102 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 900 | 3.5 | **3150** |
| 107 | .5 cal twin AA mg | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 1.2 | **960.000061** |
| 108 | 40mm Bofors AA | Bullet | 0x01 -> 0x03 | fly time times v0 | - | 800 | 2 | **1600** |
| 109 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 900 | 3.5 | **3150** |
| 110 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 900 | 3.5 | **3150** |
| 111 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 900 | 3.5 | **3150** |
| 112 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 900 | 3.5 | **3150** |
| 113 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 900 | 3.5 | **3150** |
| 114 | Akizuki Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 800 | 2.5 | **2000** |
| 115 | .30 cal | Bullet | 0x01 -> 0x02 | fly time times v0 | - | 800 | 1 | **800** |
| 116 | R4M AA | Rocket | 0x12 -> 0x12 | constant 3000 | - | 0 | 4 | **3000** |
| 117 | 3.9/65 combined gun bullet - semmi | Artillery | 0x04 -> 0x06 | authored range | 1500 | 300 | - | **1500** |
| 118 | 12 heavy cruiser gun bullet - Alaska D | Artillery | 0x04 -> 0x07 | authored range | 2300 | 300 | - | **2300** |
| 119 | Nuclear Bomb | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 142 | globals.unitclass_dummytargetvehicle | DummyTarget | 0x0C -> 0x0C | constant 3000 | - | - | - | **3000** |
| 143 | globals.unitclass_kamikaze | DummyKamikazePlane | 0x0D -> 0x0D | constant 3000 | - | - | - | **3000** |
| 144 | globals.unitclass_kaiten | DummySubmarine | 0x0E -> 0x0E | constant 3000 | - | - | - | **3000** |
| 154 | Watermine Test ship US | WaterMine | 0x13 -> 0x13 | constant 3000 | - | 10 | - | **3000** |
| 160 | Paratrooper | Paratrooper | 0x0F -> 0x0F | constant 3000 | - | - | - | **3000** |
| 161 | 5inch/38 AUSTIN AA Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 800 | 2.5 | **2000** |
| 162 | Watermine Test ship US | WaterMine | 0x13 -> 0x13 | constant 3000 | - | 10 | - | **3000** |
| 163 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 900 | 10 | **9000** |
| 164 | 20/45 battleship gun bullet - Super Ya | Artillery | 0x04 -> 0x07 | authored range | 90000 | 900 | - | **90000** |
| 165 | 8/50 heavy cruiser gun bullet | Artillery | 0x04 -> 0x07 | authored range | 2500 | 410 | - | **2500** |
| 166 | 18/45 battleship gun bullet - Yamato p | Artillery | 0x04 -> 0x07 | authored range | 3300 | 350 | - | **3300** |
| 167 | Kurama 260 | Artillery | 0x04 -> 0x07 | authored range | 2300 | 372 | - | **2300** |
| 168 | Yamato 1945 Flak bullet | Flak | 0x10 -> 0x10 | fly time times v0 | - | 1200 | 3.5 | **4200** |
| 169 | IJN 800KG HE | Bomb | 0x09 -> 0x09 | constant 3000 | - | - | - | **3000** |
| 170 | globals.unitclass_kamikaze | DummyKamikazePlane | 0x0D -> 0x0D | constant 3000 | - | - | - | **3000** |
| 997 | 14/45 battleship gun bullet - KGV prim | Artillery | 0x04 -> 0x07 | authored range | 4500 | 300 | - | **4500** |

## 7. The host prescription

`src/game_hosts_gunnery.cpp:438` sets `gun.max_range` from the flattened `range`, which the Lua
chunk near line 296 fills with `num(bc.Range, 1000) or 0`. That is `descriptor+68h`, not `+60h`.
The torpedo-only block at lines 450-467 already implements `00855A90`; it should be folded into
the general rule rather than kept beside it.

Replace both with:

1. Keep the flattened `range` where it is - `GameBulletClassRow::range` is the **authored** value
   and stays that.
2. Add `GameMissionLuaHost::read_bullet_class_string(int index, const char* key)` beside
   `read_bullet_class_number` (`src/game_hosts_lua.cpp:959`): the same `Bullets[index][key]` walk
   with `LUA_TSTRING` and `lua_tolstring`. `read_bullet_class_number` only accepts `LUA_TNUMBER`,
   and `Type` is a string.
3. At line 438, when `gun.bullet_class >= 0`:

```cpp
bsp::WeaponClassFinaliseInput in;
in.sub_type = bsp::weapon_class_sub_type_for_lua_type(
    lua.read_bullet_class_string(gun.bullet_class, "Type"));
in.range        = lua.read_bullet_class_number(gun.bullet_class, "Range", 0.0f);
in.muzzle_speed = lua.read_bullet_class_number(gun.bullet_class, "V0", 0.0f);
in.fly_time     = lua.read_bullet_class_number(gun.bullet_class, "FlyTime",
                                               bsp::kWeaponClassFlyTimeDefault);
in.water_travel_speed = lua.read_bullet_class_number(gun.bullet_class, "WaterTravelSpeed", 0.0f);
in.max_fall           = lua.read_bullet_class_number(gun.bullet_class, "MaxFall", 0.0f);
in.flak_min_range     = lua.read_bullet_class_number(gun.bullet_class, "MinRange", 0.0f);
const bsp::WeaponClassFinaliseResult r = bsp::weapon_class_derive_engagement_range(in);
gun.max_range         = r.engagement_range;   // 00731020's answer
gun.water_travel_speed = lua.read_bullet_class_number(gun.bullet_class, "WaterTravelSpeed", 0.0f);
gun.swim_speed        = r.swim_speed;
```

The `FlyTime` fallback must be `bsp::kWeaponClassFlyTimeDefault` (`FLT_MAX`, `00D7A248`), not
`0.0f` as the existing torpedo block passes. No class in this installation exercises the
difference, but a class that authors `V0` and no `FlyTime` would get `0` instead of a huge range.

`DamageMin`, `Blast.BlastDamageMin` and `Name` are only needed if the host also wants the
**refined** sub-type; they do not change `max_range`.

The behaviour of the already-working categories is unchanged: for `Artillery` the rule returns the
authored `Range` unchanged, and for the gun kinds a class that authored a `Range` round-trips.

## 8. What is proven and what is assumed

Proven:

- `006E9890`'s body, every branch, from the listing; and the two overrides' bodies.
- The eleven vtables and the two overriding ones, by an image-wide byte scan **and** an uncapped
  13-row xref list that agree.
- All thirteen constructor sub-types, each from its own `MOV [ESI+8],imm`.
- `+14h` is the `Name` string's character data, from the reader's own copy at `006E87E7`.
- `+0ACh` = `DamageMin`, `+0B4h` = `Blast.BlastDamageMin`, from the reader key table.
- Every constant, decoded from its `.rdata` bytes.
- The invocation site `006EAFE0` and its four callers.
- The 119-row table: `src/bullet_engagement_range.cpp` reproduces an independent Python model of
  the rule on all 119 rows, and the refined sub-type on all 119, with zero mismatches.

Assumed or partial:

- The reconstruction selects the override by sub-type instead of by vtable. Exact only because
  constructor sub-type and vtable are one-to-one **before** the hook runs.
- `006E97F0` (slot `+14h`) is **contract: unread** past its `+0C8h` store. It runs *before* the
  finalise hook, so if it wrote `+50h`, `+54h` or `+68h` it would change the inputs read here.
- The x87 model. The native keeps intermediate products at extended precision and rounds at each
  `FSTP float`; the reconstruction computes in `double` and casts at the same points. Faithful,
  not bit-exact.
- "The missing range is why five categories are silent" is proven **sufficient**, not
  **exclusive**. Needs a run.
- `006E8770` was read here only for the `Name` block `006E878F`-`006E87FB`; the rest of its key
  table is `docs/WEAPON_CLASS_DESCRIPTOR.md`'s and was used as a contract.

## 9. Follow-up packets

- Wire the rule into `src/game_hosts_gunnery.cpp` and re-run IJN01 and USN02. Compare the
  per-category assign and shot counts against the zeroes this packet started from. That is the
  only way to settle whether any of the five categories is silent for a **second** reason.
- The refined sub-types `2`, `3`, `5`, `6`, `7` now have a producer. Re-check every consumer that
  switches on `+8h` - `006E85B4` launch bias, `0072FE0x` world mode 2, `0073039D` ammo provider,
  `0095561B` last-fired stamp - against the **post-finalise** value, not the constructor value.
  `docs/PROJECTILE_KINDS.md`'s consumer table is written against the constructor value.
- `+5Ch`, the artillery arm's `V0 / sqrt(g * R)` ratio: find its consumers.
- `006E97F0`, slot `+14h`: read the load pass.
- Sub-type `5` is **dead** for this installation: no `Artillery` class has both `DamageMin` and
  `Blast.BlastDamageMin` under `75`. 21 classes reach `7` and 11 reach `6`. Worth checking the
  realistic table and a retail installation.
