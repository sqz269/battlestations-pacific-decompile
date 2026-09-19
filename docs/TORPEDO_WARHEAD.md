# The torpedo warhead: a torpedo's damage is its Blast, not its DamageMin/DamageMax

Addresses: 0084BC60 0084BAD0 00470350 00470510 004705C0 008777D0 00826F10 006E2820 006E2850
006E7C60 00856050 00BD2F10

Packet `cc8_torpedo_warhead`. Every descriptive name below is a hypothesis, not a recovered
symbol. This document answers the question docs/TORPEDO_AFTER_THE_DROP.md section 15.6 raised and
explicitly did not answer: "Nothing above is a reading of the image's torpedo warhead."

## 1. The headline

A torpedo that strikes a ship delivers its warhead through the **radial burst** `0084BC60` spawns
on impact, not through the direct segment hit. The direct hit carries the class row's
`DamageMin`/`DamageMax`; the burst carries `Blast.BlastDamageMin`/`BlastDamageMax`. For the Kate's
torpedo in this installation those are **50** and **1200**.

The Lexington's `Armour` is **50**. `00470510` is `(base * ownerMod - armour) * scale`, so the
contact damage of a torpedo hit on that carrier is exactly

```
(50 * 1.0 - 50) * 1.0 = 0.0
```

and `008777D0`'s `0.0 < damage` test then skips the `AddDamage` call. Section 15.7's measurement -
six torpedo hits, `taken 0`, both carriers at `health 8000` - is not an approximation of zero. It
is the arithmetic landing exactly on zero, and it is what the image's contact damage would also do.
What the host was missing is the other 1200.

## 2. The burst, read from the listing at `0084BE25`..`0084BEE3`

```
0084be25: MOV EAX,dword ptr [ESI + 0x8]     ; the projectile's weapon class descriptor
0084be28: CMP byte ptr [EAX + 0x6c],0x0     ; the "has blast" flag
0084be2c: JZ  0x0084bee8                    ; no Blast table -> no burst, function ends
0084be32: FLD  float ptr [EAX + 0xb8]       ; BlastDamageMax
0084be38: SUB ESP,0x8
0084be3b: FSTP float ptr [ESP + 0x4]
0084be3f: MOV ECX,0x1
0084be44: FLD  float ptr [EAX + 0xb4]       ; BlastDamageMin
0084be4a: FSTP float ptr [ESP]
0084be4d: CALL 0x00bd2f10                   ; a uniform draw between them
0084be52: FSTP float ptr [ESP + 0x84]       ; -> the &damage slot
...
0084be9c: LEA  EDX,[EAX + 0x70]             ; -> &radius
0084bea2: LEA  ECX,[ESP + 0x34]             ; -> &centre
0084bee3: CALL 0x0084bad0
```

Against the argument contract of `0084BAD0` in docs/EXPLOSION_RADIAL_DAMAGE.md
(`ECX = centre`, `EDX = &radius`, `+4h = &damage`, `+8h = ignoreFalloff`, `+Ch = sourceEntity`,
`+10h = shot`), the four stack arguments resolve as:

| slot | value | evidence |
| --- | --- | --- |
| `+4h` | `&damage`, the draw above | `LEA ECX,[ESP+90h]` at `0084BE90` is `ESP_entry+84h` after the three pushes at `0084BE6E`, `0084BE8E`, `0084BE8F` - the same slot `0084BE52` stored to |
| `+8h` | `classDesc[+8h] == 10h` | `CMP dword ptr [EAX+8],0x10` at `0084BE5F`, `SETZ DL` at `0084BE74` |
| `+Ch` | `[ESI+4h]`, the owner | `MOV ECX,[ESI+4]` at `0084BE71`, `PUSH ECX` at `0084BE8E` |
| `+10h` | `[ESI+20h]`, the shot | `MOV ECX,[ESI+20h]` at `0084BE69`, `PUSH ECX` at `0084BE6E` |

The `+8h` argument settles a question the implementation depends on. `classDesc[+8h]` is the
projectile sub-type, and docs/PROJECTILE_KINDS.md's table gives `MTorpedo = 0Ah` against
`MFlakBullet = 10h`. So the byte `0084BE5F`/`0084BE74` computes is **false for every torpedo**, and
`004705C0`'s falloff is **not** waived: a torpedo's burst damage falls off with distance inside the
radius. The same test is the flak shell's decal branch elsewhere in `0084BC60`
(`kProjectileSubTypeDecalVariant`).

`EDX` is `&classDesc[+70h]`, one float read in place: the radius is **not** a draw. The centre is
`hitPos - direction * 0.05` - `[ESI+24h..2Ch]` minus `[ESI+10h..18h]` scaled by the double at
`00D7A270`, differenced at `0084BEBF`..`0084BEDF`.

**What `0.05` scales, corrected.** An earlier draft of this document read it as "one 20 Hz frame of
travel", which assumed the direction was a velocity. It is not: `buffer+10h` is written by
`0084BF00` as `delta * (1 / BSP_Vector3f_Length(delta))`, and docs/PROJECTILE_IMPACT.md's field
table calls it "the segment direction, **normalised**". So the back-off is a flat **5 cm** out of
the struck surface at every speed, not a time step. The difference is invisible on a 30 m/s torpedo
(1.5 m against 0.05 m) and decisive on a 700 m/s shell, where treating it as a velocity puts the
burst 35 m back down the flight path - which is exactly how this host's first ungated run was
caught doing it (section 11).

**Correction to docs/PROJECTILE_IMPACT.md.** Its step-7 row (line 315) reads "radius randomised
from `classDesc[+B4h]`/`[+B8h]`". Those two fields are the blast **damage** min and max and they
feed the `&damage` argument; the radius is the single float at `+70h` and is not randomised.

### A torpedo really does reach `0084BC60`

Step 7 is only the torpedo's rule if a torpedo's impact runs `0084BC60` at all, and it does.
docs/PROJECTILE_KINDS.md's tick table gives `MTorpedo`'s tick advance (`+8h`) as **`006E1300`**,
the same body the bomb, the depth charge and the rocket run; the torpedo overrides only slot `+Ch`.
`006E1300 BSP_BombProjectile_TickAdvance` calls `0084C430 BSP_Projectile_SweepStepSegment`, whose
only callees on this path are `0084BF00 BSP_Projectile_TraceSegmentAndImpact` and, under it,
`0084BC60`. So the swimming round's hit handler is the generic one and step 7 is reached.

The `shot` pointer the hit record stores is the projectile's **primary** subobject, vtable
`00D0C3E8`, not the `+310h` interface: `00826F10` asks `shot->vtable[5Ch](2Bh)` whether the shot is
an `MTorpedo`, and `00D0C3E8+5Ch` is `00856260 BSP_MTorpedo_IsKindOf`. The `+310h` subobject's
vtable starts at `00D0C378` and the next vtable begins at `00D0C3CC`, only `54h` bytes later, so it
has no `+54h` slot to call. Two independent slots therefore anchor the same object.

## 3. The class-descriptor fields this fixes in place

`006E7C60`, the base projectile's `vtable[54h]`, reads `DamageMin`/`DamageMax` at descriptor
`+ACh`/`+B0h` (docs/GAME_EXECUTABLE.md section 7). The blast pair sits directly above it:

| offset | field | read at | producer |
| --- | --- | --- | --- |
| `+6Ch` | byte, gates the burst | `0084BE28` | **inferred**, see below |
| `+70h` | float `BlastRange`, the burst radius | `0084BE9C` | `006E8770`'s key table, docs/KILL_CREDIT.md |
| `+ACh` / `+B0h` | float `DamageMin` / `DamageMax` | `006E7C60`, `006E2820` | `006E8770`'s key table |
| `+B4h` / `+B8h` | float `BlastDamageMin` / `BlastDamageMax` | `0084BE44` / `0084BE32` | `006E8770`'s key table, docs/BULLET_ENGAGEMENT_RANGE.md |

Three of the four are confirmed from the **producer** and not only from this consumer:
docs/BULLET_ENGAGEMENT_RANGE.md reads `+ACh = DamageMin` and `+B4h = Blast.BlastDamageMin` out of
the class reader's own key table at `006E8770`, and docs/KILL_CREDIT.md reads
`classDesc+70h = Blast.BlastRange`. The `+B8h` pairing with `+B4h` follows from the two being the
draw's two arguments at `0084BE32`/`0084BE44`.

`+6Ch` is the exception and is **inferred**: it is a byte tested against zero immediately below
`BlastRange`, and every row that reaches the burst in this installation has a `Blast` sub-table, so
"the Blast table was present" is the natural reading. The key table entry that writes it has not
been read. Nothing in this document depends on the name - only on the fact that the byte gates the
burst, which is in the listing.

## 4. A torpedo's damage base comes from its own descriptor pointer, by a second virtual

The hit record's `+14h` is `shot->vtable[54h]()` (`00470350`, docs/HIT_NARROWPHASE.md). There are
two bodies in that slot across the image, and a torpedo does **not** use the one the earlier
reading assumed:

| body | descriptor | vtables holding it at `+54h` |
| --- | --- | --- |
| `006E7C60` | `[ECX+174h]` | `00CF9DF0` `BSP_ProjectileTickableEntity`, `00CFD5D0` `BSP_FlakProjectile` |
| `006E2820` | `[ECX+314h]` | ten, including `00D0C3E8`, the torpedo projectile's |

The `+54h` offset is checked only for the torpedo's own vtable, where `00D0C43C - 54h` is exactly
`00D0C3E8`; the other nine are inferred from the data references to `006E2820` and `006E2850`
sitting four bytes apart in every one of the ten, which is the `+54h`/`+58h` pair.

`00D0C3E8` is the vtable `BSP_TorpedoProjectile_Construct` (`00856050`) writes at `+0h`; its
`+5Ch` is `00856260` `BSP_MTorpedo_IsKindOf`, which is the slot `00826F10` calls with `2Bh` to ask
"is this shot a torpedo", so the slot numbering is anchored. Read from `00D0C43C` (`bsp.py ghidra
bytes`), `00D0C3E8+54h` is `006E2820` and `+58h` is `006E2850`.

```
006e2820: MOV EAX,dword ptr [ECX + 0x314]   ; the weapon class descriptor
006e2826: FLD float ptr [EAX + 0xb0]        ; DamageMax
006e2838: FLD float ptr [EAX + 0xac]        ; DamageMin
006e2841: CALL 0x00bd2f10                   ; the same uniform draw
006e2850: FLD float ptr [ECX + 0x3e0]       ; vtable[58h], the weapon scale
```

So the two families differ only in where the shot keeps its descriptor pointer (`+174h` against
`+314h`) and where the weapon scale lives. **Both draw between the same `DamageMin`/`DamageMax`
fields**, and the torpedo's warhead is therefore not hiding in a torpedo-only damage field on the
`0FCh`-byte descriptor. Section 15.6 floated that possibility; it is retracted here. The extra
fields `BSP_TorpedoClass_ReadLuaFields` (`008566B0`) adds over the bullet base are
`MaxWaterHitVel`, `MaxFall`, `WaterTravelSpeed`, `HeadingTurn`, `WaterSplashEfx` and the two
homing turn rates - swim behaviour, not damage.

## 5. The data, in this installation

`scripts/datatables/autoload/bulletclasses.lua` sets `Bullets = RealisticTable` when `GameMode == 1`
and `Bullets = ArcadeTable` otherwise. `gamemode.lua` in this installation reads `GameMode = 0`, so
the arcade table is the live one. (This installation is modded - the file carries a "Kantai Kessen
does not support BSPRM 1.0s Realistic mode" note.)

| row | Comment | DamageMin/Max | BlastDamageMin/Max | BlastRange | WaterDamage |
| --- | --- | --- | --- | --- | --- |
| `ArcadeTable[69]` | Torpedo Airplane JP | 50 / 50 | **1200 / 1200** | 50 | 10 |
| `RealisticTable[27]` | Torpedo Airplane JP strong | **550 / 650** | 50 / 50 | 10 | 18 |

The two tables put the warhead in opposite places: arcade in the blast, realistic in the contact
damage. Under `GameMode = 0` a torpedo's lethality is the 1200, and the 50 is a rounding error a
carrier's armour cancels outright. Anything measured on USN04 therefore says nothing about the
realistic table.

This also settles section 15.6's stray number. It read a Kate's `damage_dealt` as "about 44" where
a torpedo struck an "unarmoured" squadron mate and inferred a draw of "about 44" from the bullet
row. The draw is not approximate and the squadron mate is not unarmoured. `DamageMin ==
DamageMax == 50`, so the draw is exactly 50 every time, and `VehicleClass` "B5N Kate" has
`Armour = 6`:

| victim | Armour | `(50 - armour)` | measured |
| --- | --- | --- | --- |
| B5N Kate (a squadron mate) | 6 | 44 | `damage_dealt` 44, section 15.1 |
| Lexington 1944, Yorktown 1944 | 50 | 0 | `taken 0`, sections 15.6 and 15.7 |

Both measurements are the same formula landing on its exact value, which is the strongest evidence
in this document that the host's contact-damage path is already faithful and that what is missing
is a second damage source entirely.

## 6. Armour, flooding, and what else a torpedo hit does

* **Armour is subtracted, and there is no underwater exemption on this path.** Both formulas carry
  the same `- armourScaled` term (docs/UNIT_HIT_PATH.md); `00826F10` R2 chooses the source and
  nothing between the water entry and `008777D0` waives it. The blast is not armour-free either -
  it is simply 24 times larger than the armour it must beat.
* **The armour-free `hull_damage(0.0f)` of `0082712E` is only the roll torque.** Section 15.6
  pointed at it as a possible warhead rule. It is not: the value never reaches health, it is
  scaled into a torque message (`93h`) and `src/ship_hit_record.cpp` already routes it. Retracted.
* **Flooding is not torpedo-specific and cannot currently fire.** `00826F10` R7b floods on every
  hit that has a weapon, at `weapon->vtable[10h]()` = `WaterDamage`. But R1 (`00826F62`) sends a
  record with `hit+34h == -1` straight to the part loop, skipping R7a, R7b and R7c together, and
  no read path writes a non-negative `+34h`: the segment shapes write `-1`
  (docs/GAME_EXECUTABLE.md) and the blast gather cannot write one at all
  (docs/EXPLOSION_RADIAL_DAMAGE.md, "Who could write the hull segment index"). `floods=0` is
  consistent with the whole reading, **and that reading has a labelled gap**: the same document
  says its shape enumeration cannot be complete. Do not read `floods=0` as proof that the image
  never floods.
* **The roll torque is the one thing a torpedo does that a shell does not** among the rules read:
  R7a is gated on `vtable[5Ch](2Bh)` `MTorpedo` and `Mass > 500.0`, and the Kate's torpedo row has
  `Mass = 700`.

## 7. The same `-1` gate is why this host cannot simply be made faithful

`008777D0` opens

```
008777da: OR  EDI,0xffffffff
008777dd: CMP dword ptr [EBX + 0x34],EDI
008777e2: JZ  0x008778d8            ; hull pass skipped whole when +34h == -1
```

so in the image a record with `+34h == -1` gets no hull damage, only whatever the part pass finds.
This host sets `kDirectHitHullSegment = -1` on its direct-hit records and applies the hull damage
anyway. That divergence is deliberate and must stay: every hull segment index the reconstruction
has read is `-1`, so importing the gate as read would zero all gunnery damage, not just torpedo
damage. The honest statement is that the producer of a non-negative `+34h` is unread, and both the
image's flooding and the image's hull pass hang on it.

## 8. What this host now does, and which parts are stand-ins

`src/game_hosts_gunnery.cpp`, at the torpedo's `entity_impact`, now calls `apply_torpedo_blast`
after the direct `apply_hit`, mirroring `0084BC60` step 7. Per unit inside the radius it builds a
record with `+28h` = the blast draw, `+24h` = `BlastRange`, and runs `004705C0`, whose result is
`max(0, (1 - distance/range) * base * ownerMod - armour) * scale`. `008777D0`'s part pass, which
this host had never implemented, now applies that result through the same `00879070` / `00877B90`
write the hull pass uses.

| piece | status |
| --- | --- |
| the burst gate, the damage draw, the radius field, the centre back-off | **proved from the listing**, section 2 |
| `BlastDamageMin` reaching the host's bullet row | new, read from the same Lua key the image's `+B4h` holds |
| `004705C0` applied to the burst damage | the image's formula, unchanged |
| the part pass inside `008777D0` | the image's structure, newly implemented here |
| **which entities the burst gathers** | **stand-in.** The image walks a shape tree (`00904470` -> `0098C630`); this host tests its own hull boxes, the same slabs its segment sweep uses |
| **the distance that feeds the falloff** | **stand-in.** The array at `+3Ch` that `004705C0` indexes has no read producer in the image; this host measures the burst centre to the hull box |
| a non-negative `+34h`, and therefore flooding, fire and roll torque on a real hit | **hole**, unread, see section 7 |

### The burst is not torpedo-only, and this host no longer pretends it is

`0084BE28` gates the burst on `classDesc[+6Ch]`, not on the projectile's kind, so in the image
**every** class with a `Blast` table bursts on impact - bombs, rockets and the artillery rounds
that carry one. The first commit of this packet gated the host call on the torpedo category to
protect other packets' baselines; that gate was a host invention and is gone. `apply_impact_blast`
now runs for every shot, and its only gate is the row's own `BlastRange`, which is the image's
`+6Ch` test. Section 11 records the baselines that replaces.

## 9. Measured on USN04, 4500 mission frames, no probe

`local/warhead_before_usn04.log`, current main `e2ada6d71`, no probe of any kind:
`--menu-select USN04 --frames 4700 --mission-frames 4500 --mission-frame-seconds 0.05`.

Twelve drops, `swims_started=8`, five hits. Per round, with the closest approach and crossing
angle taken from the run's own census (centre to centre, horizontal):

| # | swim start | ordered target | closest approach | crossing | outcome |
| --- | --- | --- | --- | --- | --- |
| 1 | never | - | - | - | killed on squadron mate `B5N Kate #4.1\|.-3` at life 0.05 |
| 2 | never | - | - | - | killed on squadron mate `B5N Kate #4.1` at life 0.05 |
| 3 | 1.55 | Yorktown-class01 | 17.8 m @ 7.35 s | 3.040 rad (174.2 deg) | **hit** Yorktown at life 8.25 |
| 4 | 1.60 | Lexington-class01 | 55.4 m @ 9.75 s | 2.857 rad (163.7 deg) | **hit** Lexington at life 9.75 |
| 5 | 1.60 | Lexington-class01 | 50.1 m @ 9.90 s | 2.848 rad (163.2 deg) | **hit** Lexington at life 9.90 |
| 6 | 1.60 | Lexington-class01 | 49.5 m @ 10.00 s | 2.861 rad (163.9 deg) | **hit** Lexington at life 10.00 |
| 7 | never | - | - | - | killed on squadron mate `B5N Kate #8.1\|.-3` at life 0.05 |
| 8 | never | - | - | - | killed on squadron mate `B5N Kate #8.1` at life 0.05 |
| 9 | 1.55 | Yorktown-class01 | 44.9 m @ 6.45 s | 3.066 rad (175.7 deg) | **hit** Yorktown at life 6.45 |
| 10 | 1.60 | Lexington-class01 | 197.4 m @ 5.15 s | 2.857 rad | still swimming at mission end |
| 11 | 1.60 | Lexington-class01 | 212.2 m @ 4.65 s | 2.848 rad | still swimming at mission end |
| 12 | 1.60 | Lexington-class01 | 233.0 m @ 4.05 s | 2.861 rad | still swimming at mission end |

Every round that swam ran at the ordered target and every round that reached one hit it. **Four of
the twelve still die on a squadron mate one tick after the drop**, at life 0.05, which is the
defect docs/TORPEDO_AFTER_THE_DROP.md section 15.4 attributes to the missing formation offset;
main `ab63a11c0` reduced it but has not removed it. That is not this packet's and nothing here
depends on it.

The damage census of the same run:

```
unit                 side  guns  ...  shots  hits   dealt   taken   health   sunk_at
Lexington-class01       0    22        93     7     257    1791     6209     -1.00
Yorktown-class01        0    19       102     2     173       0     8000     -1.00
```

The Yorktown is the clean reading: its only two hits in the whole run are torpedoes 3 and 9, and it
ends at `taken 0`, `health 8000`. The Lexington's `taken 1791` is not torpedo damage - all five
bombs of this run land at `x = -12906..-12923`, on the Lexington, while the Yorktown sits at
positive `x` - and its three torpedo hits contribute exactly the 0 that section 1 computes.

## 10. The same mission with the burst, and the arithmetic of one hit

`local/warhead_final_usn04.log`, same binary but for the change, same arguments. The burst fires
nine times, every one of them a torpedo (`base=1200.0 range=50.0`):

```
torpedo blast on Yorktown-class01   dist=0.2  base=1200.0 range=50.0 armour=50.0 took=1146.2 health=6853.8
torpedo blast on Lexington-class01  dist=0.4  base=1200.0 range=50.0 armour=50.0 took=1139.6 health=6860.4
torpedo blast on Lexington-class01  dist=0.4  base=1200.0 range=50.0 armour=50.0 took=1139.3 health=5721.1
torpedo blast on Lexington-class01  dist=0.4  base=1200.0 range=50.0 armour=50.0 took=1139.7 health=4581.4
torpedo blast on Yorktown-class01   dist=0.1  base=1200.0 range=50.0 armour=50.0 took=1147.2 health=5706.6
torpedo blast on B5N Kate #4.1      dist=0.0  base=1200.0 range=50.0 armour=6.0   took=176.0  health=0.0
  (and three more Kates, the squadron mates the four dud rounds strike)
```

**The arithmetic of one hit**, the first Yorktown row, through `004705C0`:

```
f      = 1 - distance/range = 1 - 0.158/50 = 0.99684      ; the printed 0.2 is rounded
result = max(0, f * base * ownerMod - armour) * scale
       = max(0, 0.99684 * 1200 * 1.0 - 50.0) * 1.0
       = 1196.2 - 50.0 = 1146.2                            ; the logged value
```

Before and after, the two carriers:

| unit | hits | taken | health | |
| --- | --- | --- | --- | --- |
| Yorktown-class01 | 2 -> **4** | 0 -> **2293** | 8000 -> **5707** | its only hits are the two torpedoes |
| Lexington-class01 | 7 -> 10 | 1791 -> 5334 | 6209 -> 2666 | 1791 of the before is bombs |

The Yorktown is the measurement: two torpedo hits, `taken 0` before and `2293` after, which is
`1146.2 + 1147.2` to the digit. The hit count doubles because a burst queues its own record beside
the direct hit, which is what the image does too - `0084BAD0` queues one record per gathered entity
and the direct hit is already queued.

Two consequences worth naming rather than burying. A torpedo that strikes a **squadron mate** now
destroys it: a B5N Kate has 220 HP and `Armour = 6`, so the burst's 1194 is far past it, and USN04's
deaths move 3 -> 5. And the mission's damage total moves 3309.9 -> 9398.6. Neither is a regression;
both are the warhead finally arriving.

### The control, and what it actually controls

`local/warhead_final_usn01.log`, USN01 at 3000 mission frames. Every per-round and per-task fact is
**unchanged**: `drops=5`, `swims_started=5`, all five Mav tasks with `releases=1` and identical
state histograms, `goaway` entered once per aircraft, and torpedo 1 striking `Hangar, Small, 04 01`
at `(4165.8, 0.00, -3294.7)` at life 51.75 with the other four expiring at life 60.05 - byte for
byte the same as the run without the burst.

The mission totals do move, 1285.0 -> 3560.4 and deaths 1 -> 2, and the brief's premise for this
control ("its five torpedoes MISS, so nothing there should move") is **half right**. They miss the
ship they were ordered onto, but one of them has always run on and hit a hangar; before this change
that hit did nothing, because the hangar's `Armour` of 70 exceeds the contact damage of 50. Now it
delivers 1098.9 and the burst reaches three parked wildcats, two more hangars and an oil tank
inside 50 m. So USN01 does not control for "the torpedo chain is untouched" by its totals - it
controls for it by the per-round trace above, which is identical.

## 11. The ungated burst, the defect it exposed, and the new reference baselines

Section 8's torpedo-only gate is removed: `apply_impact_blast` now runs on every entity impact and
the only gate is the row's `BlastRange`, which is the image's `classDesc+6Ch`. Two missions were
re-run to replace the reference numbers that change as a result.

### 11.1 The defect the first ungated run exposed

Turning the burst on for fast ordnance immediately showed something that could not be right. The
burst centre is `hitPos - direction * 0.05`, and this host was handing `apply_impact_blast` the
shot's **velocity** as `direction`. At a torpedo's 30.9 m/s that is a harmless 1.5 m; at a bomb's
127 m/s it is 6.3 m; at an AA shell's speed it is tens of metres. In the first ungated USN01 run
the burst from a bullet-44 round sat **33.2 m** from the aircraft it had actually struck, so the
splash was credited to that aircraft's neighbour instead:

```
  (before)  impact blast bullet=44 on Mav2 dist=33.2 ... took=0.0    <- the round hit Mav2
            impact blast bullet=44 on Mav3 dist=4.2  ... took=20.8   <- Mav3 paid for it
  (after)   impact blast bullet=44 on Mav2 dist=0.0  ... took=25.0
            impact blast bullet=44 on Mav3 dist=28.8 ... took=0.0
```

The image does not do this. `buffer+10h` is written by `0084BF00` as `delta * (1 /
BSP_Vector3f_Length(delta))` and docs/PROJECTILE_IMPACT.md's field table calls it "the segment
direction, **normalised**", so `0.05` is a flat 5 cm nudge out of the struck surface at every
speed. The host now normalises before applying it, and every directly-struck victim reports
`dist=0.0`. This also corrects the torpedo numbers of section 10 upward, because the burst no
longer starts 1.5 m off the hull: a carrier hit is now `(1 - 0/50) * 1200 - 50 = 1150` exactly,
against the 1146.2 measured with the 1.5 m offset.

This was worth the two extra runs: the same error was silently costing the torpedo its last 4
damage per hit, and no torpedo-only measurement could ever have exposed it.

### 11.2 USN01, 3000 mission frames - the new reference

`local/baseline_usn01.log`. **Supersedes docs/TORPEDO_AFTER_THE_DROP.md section 14.5's
`deaths 1, damage 1285.0`**, which was measured when no impact produced a burst.

| | 14.5 reference | torpedo-gated (intermediate) | **ungated, normalised** |
| --- | --- | --- | --- |
| damage | 1285.0 | 3560.4 | **3595.4** |
| deaths | 1 | 2 | **4** |
| blast records | none | 7 | **27** |

The torpedo chain is **byte-identical in all three**: `drops=5`, `swims_started=5`, five tasks with
`releases=1` and identical state histograms, `goaway` entered once per aircraft, torpedo 1 striking
`Hangar, Small, 04 01` at `(4165.8, 0.00, -3294.7)` at life 51.75 and the other four expiring at
life 60.05. Nothing about the run-in, drop, swim, break-off or retire moved; only what an impact
then does.

The 27 blast records, so a later reader can tell a second record beside a direct hit from a new
hit. `dist=0.0` marks the entity the round actually struck; every other row is splash:

| bullet | victim | n | dist | base/range | armour | took each |
| --- | --- | --- | --- | --- | --- | --- |
| 31 | SaltLakeCity | 2 | 0.0 | 65 / 57 | 90 | 0.0 |
| 19 | SaltLakeCity | 8 | 0.0 | 50 / 35 | 90 | 0.0 |
| 44 | Mav2 | 4 | 0.0 | 35 / 35 | 10 | 25.0 |
| 44 | Mav3 | 4 | 28.6-28.8 | 35 / 35 | 10 | 0.0 |
| 69 | Hangar, Small, 04 01 | 1 | 0.0 | 1200 / 50 | 70 | 1129.0 |
| 69 | Hangar, Small, 04 02 | 1 | 20.2 | 1200 / 50 | 70 | 645.5 |
| 69 | Hangar, Small, 04 03 | 1 | 42.3 | 1200 / 50 | 70 | 114.7 |
| 69 | Multi Hangar 1 | 1 | 49.1 | 1200 / 50 | 70 | 0.0 |
| 69 | Static wildcat, closed 02 / 03 | 2 | 37.9, 43.3 | 1200 / 50 | 5 | 150.0 (destroyed) |
| 69 | Static wildcat, closed 04 | 1 | 48.7 | 1200 / 50 | 5 | 25.5 |
| 69 | Oil Tank, Big 01 | 1 | 45.9 | 1200 / 50 | 2 | 96.8 |

Ten of the twelve shell bursts do **nothing** - the SaltLakeCity's `Armour` of 90 is above both
shell blast bases - so the whole of USN01's movement is one torpedo and four bullet-44 rounds. The
three extra deaths over the 14.5 reference are two static wildcats parked beside the hangar the
torpedo hits, plus one more; none is a combatant the mission's outcome turns on.

### 11.3 USN04, 4500 mission frames - the new reference

`local/baseline_usn04.log`. `drops=12`, `swims_started=8`, five hits, unchanged from section 9.

| | before the burst | torpedo-gated | **ungated, normalised** |
| --- | --- | --- | --- |
| mission damage | 3309.9 | 9398.6 | **9459.0** |
| deaths | 3 | 5 | **5** |
| Yorktown-class01 | hits 2, taken 0, health 8000 | hits 4, taken 2293, health 5707 | **hits 4, taken 2300, health 5700** |
| Lexington-class01 | hits 7, taken 1791, health 6209 | hits 10, taken 5334, health 2666 | **hits 14, taken 5350, health 2650** |

The Yorktown is again the clean measure, and on the corrected build it is exact: two torpedo hits,
`2 x (1200 - 50) = 2300`, both bursts logged at `took=1149.9`. The Lexington's four extra hits over
the gated column are its four bomb bursts, now `took=29.9` each (`80 - 50` at `dist=0.0`) where the
6.3 m offset had been giving them 13 to 16.

### 11.4 What was checked for and not found

The two failure shapes worth ruling out, both checked against `local/baseline_usn01.log` and
`local/baseline_usn04.log`:

* **No burst damages its own firer.** `0084BBF9` skips the source entity and this host skips
  `i == shooter`; no blast row in either run names the unit that fired the round.
* **No burst on a water impact.** The call sits inside the entity-impact branch only, so a round
  that reaches the sea makes no burst here. Whether the image bursts on a water or terrain impact
  is **unread** - `0084BC60` is the general impact handler and its step 7 does not test what was
  struck - so this is a boundary of this host, not a reading. It is the obvious next question for
  whoever takes the bomb path.
