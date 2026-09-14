# Gunnery recon detection: rule (c) of the contact list

Addresses: `008048A0`, `008048D1`, `008048E5`, `0080491C`, `0080492F`, `00804947`, `0080497F`,
`00804987`, `00922DC0`, `00922DD0`, `00922C80`, `008049A3`, `008049B9`, `00804A1E`, `00804A5F`,
`00804A67`, `00804A7B`, `00804A82`, `00804A86`, `00804A98`, `00804AB0`, `00804ACF`, `00804AD7`,
`00804AE4`, `00804AFA`, `00804B0B`, `00804B14`, `00804B29`, `00804B2F`, `00804B35`, `00804B40`,
`00804B45`, `00804B69`, `00804B85`, `00804BB4`, `00804BC7`, `00804BD1`, `00806840`, `0080686A`,
`00806883`, `008068C2`, `008068E7`, `008068F4`, `008068FA`, `0080693D`, `00806957`, `0080695C`,
`00806981`, `00806984`, `0080699A`, `00807529`, `00807556`, `00807574`, `00807615`, `00807634`,
`00807644`, `008076A5`, `008077A0`, `008077C2`, `00808A53`, `00860000`-range reader `00865201`,
`00865206`, `0096239A`, `009623AE`, `009623CF`, `009623D9`, `00D1AAF4`.

Reused as contracts, not re-derived here: `00804947`/`008049A3`/`00804AB0`/`00804A1E`/`00805AF0`/
`00805B3D`/`0080695C`/`00807647`/`008085AA` (`docs/RECON_SLOT_LISTS.md`, `docs/SENSOR_TABLES.md`),
`00438B10` (`docs/UNIT_RUDDER.md`), `00BF701A` `_CIatan2` (CRT, never ported).

The `bsp_game.exe` milestone builds each unit's contact list from rules (a) and (b) of
`docs/RECON_SLOT_LISTS.md` alone, so every enemy inside class scope is published. This packet is
rule **(c)**: the per-observer detection value and the predicate that decides publication.

---

## 1. ABIs, from the stack cleanup

| address | ABI | cleanup evidence |
| --- | --- | --- |
| `008048A0` | `__fastcall bool(ECX=observer, EDX=target, int mask, float dt, float* outGain)` | `00804BD1 RET 0Ch` — three stack arguments. `00804A6F`/entry `MOV EDI,ECX` and `MOV EBX,EDX` take the two register arguments. Body `008048A0`-`00804BD3`. |
| `00806840` | `__thiscall void(ReconSlot* ECX, UnitList* targets, UnitList* observers)` | `0080699A RET 8` — two stack arguments; `0080684E MOV EBP,ECX`. Body `00806840`-`0080699C`. |
| `00922DC0` | `__thiscall bool(ECX=entity)` — a `MOV DL,1` thunk into `00922C80` | `00922DC2 JMP 0x922C80`; the sibling thunk `00922DD0` is `XOR DL,DL`. Tail call, so `00922C80`'s own cleanup applies. |

The stack-argument order at `008048A0` is fixed by its only live call site. `008068D3`..`008068E0`
pushes `&localGain`, then `dt`, then `0FFh`, so the callee sees `mask` at `esp+3Ch`, `dt` at
`esp+40h` and `outGain` at `esp+44h` relative to its own post-prologue frame — exactly the slots
`00804ACB` (`MOV ECX,[ESP+40h]`, mask), `00804B35` (`FMUL [ESP+44h]`, dt) and `00804BB4`
(`MOV EAX,[ESP+48h]`, outGain) read once `PUSH EBP` at `00804A22` has shifted the frame by 4.

`00806840`'s argument order is fixed the same way: `00807552 PUSH EBX` (observers) then
`00807553 PUSH EDI` (targets), so `targets` is the first stack argument.

---

## 2. The detection value, instruction by instruction

All of this is `008048A0`. It is x87 throughout, with every intermediate stored back to a `float`
slot before the next multiply, so the reconstruction rounds at the same points.

### 2.1 The range scale

```
008048D1  PUSH 5 ; CALL [target->vtable+5Ch]      IsKindOf(05h), the unit base
008048E5  JE  8048FD                              not a unit base -> 1.0f from 00D7A24C
008048E7  MOV ECX,[target+538h]
008048ED  MOVSS XMM1,[ECX+0B8h]                   the class's squared ReconModifier
00804903  CMP byte [00E0C978],0 / [[00F88C30]+118h]
0080491C  PUSH 0Ch ; PUSH observer ; CALL 8E6430  the environment modifier, else 1.0f
0080492F  MOV EDX,[00E188A8] ; MOV EAX,[EDX+21C4h] ; FLD [EAX+74h]
```

`00804947`..`0080497B` then evaluates, in this order and with a `float` store after each step:

```
t = env * env            (00804953 FLD ST(1) / 00804955 FMULP ST(2) / 00804959 FSTP)
t = t * reconModifierSq  (0080495D FLD / 00804961 FMUL [esp+14h] / 00804965 FSTP)
u = m74 * m74            (0080496D FLD ST(1) / 0080496F FMULP ST(2) / 00804973 FSTP)
rangeScale = t * u       (00804977 FMUL / 0080497B FSTP)
```

The submerged penalty is two tests, not one:

```
0080497F  CALL [target->vtable+5Ch] with PUSH 8   IsKindOf(08h), MSubmarine
00804981  TEST AL,AL ; JE 8049B9                  not a submarine -> no penalty
00804987  CALL 00922DC0                           the surface test
0080498E  TEST AL,AL ; JNE 8049B9                 on the surface -> no penalty
008049A3  FLD [esp+18h] ; FMUL ST(0),ST(0) ; FSTP ; FLD ; FMUL rangeScale ; FSTP
```

so `rangeScale *= m78 * m78` only for a submarine that `00922DC0` says is not a surface target.
`00922DC0` is the `DL=1` thunk into the shared helper `00922C80`, which rejects on
`[entity+5Dh]`, on `IsKindOf(0Fh)` and `IsKindOf(18h)`, and for an `IsKindOf(06h)` entity compares
a negated depth scalar against `[+49Ch]` (`00922CE8`..`00922CFA`). Its body is read only far enough
to confirm that it is a surface/submerged discriminator and not a party or visibility test; the name
is the pre-existing ledger hypothesis, reused unchanged.

`m74` = `[[[00E188A8]+21C4h]+74h]`, `m78` = `[[[00E188A8]+21C4h]+78h]` — two levels of indirection,
`00E188A8` holds the game pointer and `+21C4h` holds a pointer to the tuning block.

### 2.2 The horizontal distance

`008049B9` and `008049C9` each test a `[unit+0C8h]` dirty byte and call `00414DB0` to refresh the
world frame before it is read. Only `+0FCh` (x) and `+104h` (z) are loaded; `+100h`, the height, is
never touched, so the range test is purely horizontal.

```
00804A1E  dx = target.x - observer.x        (FLD [esp+28h] / FSUB [esp+24h])
00804A2B  dz = target.z - observer.z        (FLD [esp+30h] / FSUB [esp+28h])
00804A3F  dx*dx ; 00804A4B dz*dz ; 00804A53 FADD ; 00804A57 FSTP    distSq
00804A5F  FDIV [esp+14h]                    normalized = distSq / rangeScale
```

The divide is **not** guarded. With `rangeScale == 0` and `distSq > 0` the quotient is `+inf`, which
fails every entry's range test. With both zero it is a NaN, which **passes** it — see §4.

### 2.3 The row

```
00804A67  CALL [[target+1E4h]+4h]()      -> EBP, the target category (the column)
00804A76  CALL [[observer+1E4h]+4h]()    -> EAX, the observer category (the row)
00804A7B  LEA EAX,[EBP+EAX*8]            index = observerCategory * 8 + targetCategory
00804A7F  LEA EAX,[EAX+EAX*2]            * 3
00804A82  LEA EBP,[ESI+EAX*4+8]          table + 8 + index * 0Ch
00804A86  EAX = [EBP+4]  (count)   00804A89  ESI = [EBP]  (begin)
00804A8C  LEA ECX,[EAX*8] ; SUB ECX,EAX ; LEA EDX,[ESI+ECX*4]   end = begin + count * 1Ch
```

`ESI` at that point is `[[observer+538h]+0B4h]`, loaded at `00804A0E`. The index is the same one the
loader computes at `008085AA`, so the reader and the writer address the same list. There is no
bounds test: the native relies on no category reaching 7.

---

## 3. The `1Ch` sensor row, from its producer

The layout below is taken from the **writer** `008085C0`..`00808A6A` (`docs/SENSOR_TABLES.md` §2),
which fills seven dwords on the stack and `REP MOVSD`s them into the list at `00808A5B`; the stride
is confirmed independently at the writer (`00808A53 LEA ECX,[EAX*8] ; SUB ECX,EAX`) and at the
reader (`00804A8C`, the identical idiom, and `00804BA6 ADD ESI,1Ch`).

| offset | type | Lua key at the writer | written | read by rule (c) |
| --- | --- | --- | --- | --- |
| `+0h` | float | `Dist` | `008085E5` | never |
| `+4h` | float | `Dist * Dist` | `008085F1 FMUL ST0` | `00804AB7`, against the normalised distance |
| `+8h` | float | `Gain * 0.5` (the double at `00D7A280`) | `00808626` | `00804B35`, multiplied by `dt` |
| `+0Ch` | float | from `MaxLevel`: 0 -> `0.0f`, 1 -> `0.25f`, 2 -> `1.0f` | `0080866B`..`00808685`, `00808975`, `0080896B` | `00804B69`, the accumulator cap |
| `+10h` | int | `RawType` | `008089F4`, stored `00808A23` | `00804ACF`, `1 << it` against the mask |
| `+14h` | float | `Angle`, radians | `00808992`, stored `008089C6`/`008089E5` | `00804B29`, the bearing half-angle |
| `+18h` | byte | `Angle` present (`IsNil` at `008089BA`) | `008089CF`/`008089EC` | `00804AD7`, gates the bearing test |

Two consequences of the producer that a consumer-only reading would miss:

* `+8h` is **half** the authored `Gain`. The gain per second the rule applies is `Gain / 2`.
* `+0Ch` is not a free clamp. It is one of three image floats keyed off `MaxLevel`, and `0.25f` is
  exactly `kReconDetectionBlipThreshold`. Because `00805B3D` takes `>= 0.25` to blip and `>= 0.5` to
  identified, a `MaxLevel 1` entry caps the value at the blip threshold and can never identify.

---

## 4. The admit/reject predicate

Per entry, in this order, each failure jumping to `00804B97` (next entry):

1. **Range.** `00804AB0 FLD [ESI+4h]` / `00804AB3 FLD normalized` / `00804AB7 FCOMPI ST(1)` /
   `00804ABB JA`. Rejected when `normalized > entry[+4h]`. Units: both sides are squared distances
   divided by `rangeScale`, i.e. `Dist` is in world units and the comparison is scale-free.
   `JA` is false on unordered flags, so a **NaN normalised distance passes**. The reused
   `recon_sensor_entry_gain_00804ab0` writes the test as `if (normalized > max) reject`, which has
   the same NaN behaviour; no divergence.
2. **Raw type.** `00804AC1`..`00804AD1`: rejected when `(mask & (1 << entry[+10h])) == 0`. The live
   caller passes `0FFh` (`008068E0`), so this never rejects in the rebuild.
3. **Bearing**, only when `entry[+18h] != 0` (`00804AD7`):
   `heading = observer->vtable[50h]()` (`00804AE4`), `bearing = _CIatan2(dx, dz)` — `00804AF2`
   loads `dx` first and `dz` second and `_CIatan2` takes the first as `y`, so the angle is measured
   from `+z` toward `+x`. `00804B0B CALL 00438B10(bearing, heading)` wraps the difference into
   `(-pi, pi]`; `00438B10` is `RET 8` (both float arguments cleaned), which the frame arithmetic at
   `00804B35`'s `[ESP+44h]` confirms. `00804B1B AND ECX,7FFFFFFFh` takes the magnitude bitwise, and
   `00804B29 FCOMPI` / `00804B2D JA` rejects when `|delta| > entry[+14h]`. So `Angle` is a
   **half**-angle: the admitted arc is `2 * Angle` wide, centred on the observer's heading.

An entry that passes all three **applies**:

```
00804B2F  EDX = [observer+54h] * 34h        the OBSERVER's party indexes the record
00804B32  FLD [ESI+8h] ; FMUL dt ; FSTP     gain = entry[+8h] * dt
00804B40  MOV byte [esp+13h],1              "applied" is set BEFORE the clamp
00804B45  FLD [target + EDX + 1F4h]         current = detRecord[party].value  (1E8h + 0Ch)
00804B52  sum = current + gain
00804B69  FCOMI ; JBE 804B7B                sum <= cap -> keep gain
00804B6D  FSUBRP ; FSUBP                    else gain = gain - (sum - cap)  == cap - current
00804B85  FCOMPI ; JBE 804B97               strictly greater replaces the running best
00804B8B  bestGain = gain
```

The clamp is exact, not a ramp: the entry may only raise the value to its cap. The clamped gain is
**negative** when `current > cap`, which happens when a wider entry has already pushed the value past
this entry's `MaxLevel`; the `applied` byte is still set, so the entry counts as a detection while
contributing a negative candidate gain that `00804B85` will normally discard.

**There is no hysteresis anywhere in the rule.** The only decay path is `00806957 CALL 00805BE0`,
which snaps both the value and the level to zero in one step. A target that no observer saw this tick
drops from identified to nothing immediately.

Return: `00804BC7 MOV AL,[esp+0Fh]` — the `applied` byte, i.e. *any* entry applied. `00804BB4`
writes the best gain through `outGain` only when the pointer is non-null. The gain and the boolean
are independent: `applied` can be true with a zero or negative `*outGain`.

---

## 5. The fold, and what it publishes

`00806840` walks the target list (`00806849 EDI = [targets+4h]`, advanced `00806984 EDI = [EDI+4h]`)
and, for each target, the observer list (`00806899 ESI = [observers+4h]`, advanced `00806910`).
Both list heads are at `+4h` of a `{count, head, tail}` triple, and each node is
`{prev, next, payload}` with the payload at `+8h`. The payload carries the unit at `+4h` and the
published level at `+0Ch`.

Two short circuits jump straight to the publish at `0080695C`, leaving the detection record untouched:

* `0080686A CMP [[00E188A8]+1FE4h],2 ; JE 80695C` — the non-originating machine never computes
  detection locally. The test is re-read inside the target loop.
* `00806883 CMP byte [slotIndex*34h + unit + 1F8h],0 ; JNE 80695C` — the record's force byte
  (`1E8h + 10h`).

Otherwise, per target:

```
008068C2  PUSH 5 ; CALL [observer->vtable+5Ch]    only a unit-base entry observes
008068E7  CALL 008048A0(observer, target, 0FFh, [slot+30h], &gain)
008068F4  OR BL,AL                                 any observer applied
008068FA  JBE                                      strictly greater gain wins the observer slot
0080693D  CALL 00805AF0(det, bestGain, 1, bestObserver)    when BL
00806957  CALL 00805BE0(det)                              when not
```

`dt` comes from `[slot+30h]`, the slot index from `[slot+28h]` (`kReconSlotIndexOffset`), and the
detection record is `unit + 1E8h + slotIndex * 34h`. `00805AF0`'s fourth native argument is the
winning **observer unit pointer**; the reconstruction reports an index into its observer array
instead, because a pure form has no pointers to hand back.

The publish is the last four instructions of the loop body:

```
00806968  CMP byte [det+10h],0
00806979  EAX = [det+8h]     forced level        00806981  MOV [payload+0Ch],EAX
0080697E  EAX = [det+4h]     accumulated level
```

so the value the rest of the pipeline reads is written to the **contact-list node's payload**, not to
the detection record. The detection record keeps the raw value and level; the payload keeps the
snapshot the drain will filter on.

---

## 6. How rule (c) composes with (a) and (b)

From the call site `008073C0`, not from prose. The slot holds four `97 * 0Ch` bucket arrays —
own at `+34h`, enemy at `+4C0h`, neutral at `+94Ch` — plus four flat `{count, head, tail}` lists:
own at `+0DD8h`, enemy at `+0DE4h`, neutral at `+0DF0h`, unknown at `+0DFCh`
(`0x4C0 - 0x34 = 0x94C - 0x4C0 = 0x48C = 97 * 0Ch`).

```
00807531  97x  00804E10(ownList +0DD8h, ownBucket[i])      flatten the own triple
00807556  97x  00806840(slot, enemyBucket[i], ownList)     rule (c) over enemies
00807574  97x  00806840(slot, neutralBucket[i], ownList)   rule (c) over neutrals
00807615  97x  00804E10(enemyList +0DE4h, enemyBucket[i])  flatten the enemy buckets
00807634       drain the enemy list
008077A0  97x  00804E10(neutralList +0DF0h, neutralBucket[i])
008077C2       drain the neutral list
```

So rules (a) and (b) fill the buckets, rule (c) stamps every node's payload `+0Ch` with a published
level, the buckets are flattened, and only then does the drain run — **once over the whole flat list,
not per target**. The reconstruction folds the drain into `gunnery_recon_detect_00806840` for a
caller's convenience; that composition is the reconstruction's, and the native ordering is the one
above.

The drain, `00807644`..`008076F3` for enemies and `008077C2`..`00807871` for neutrals, is the same
predicate on `payload[+0Ch]`:

```
0080764A  CMP EAX,1 ; JL 8076A2      level < 1 -> not copied
0080764F  CMP EAX,2 ; JGE 8076A2     level >= 2 -> not copied
00807654..0080769B                   level == 1 -> allocate a 0Ch node, payload := this payload,
                                     append to the unknown list at +0DFCh
008076A5  CMP [payload+0Ch],2 ; JGE 8076EE
008076AB..008076E7                   level < 2 -> unlink from the relation list and free
```

`recon_triple_filter_00807647(level) = { copy_to_unknown: level == 1, remove_from_relation: level < 2 }`
is therefore exact. The own list has no filter loop anywhere in `00807520`..`00807871`, which is the
whole of the region read, so an own-relation record is never drained.

The consumer closes the loop: `00865201 MOV ECX,[unit+54h] ; CALL 008053C0` resolves the party's
slot and `00865206 MOV EAX,[EAX+0DE8h]` walks the enemy list's head — the list the drain just left
holding only `level >= 2` records. That is why rule (c) is load-bearing for gunnery: without it every
node keeps `payload[+0Ch] == 0` from its allocation, the drain removes everything, and with the drain
absent as well (the current milestone) every class-scoped enemy is published instead.

The producer of `[[target+538h]+0B8h]`, read at `008048ED`, is `00960230`
`BSP_VehicleClass_ReadLuaFields`:

```
0096239A  PUSH 00D1AAF4          the key "ReconModifier"
009623AE  FLD1 ; PUSH ECX ; FSTP [ESP]    default 1.0f
009623BE  CALL 00B66330          read the float field
009623CF  FMUL ST0               square it
009623D9  FSTP [EDI+0B8h]
```

so the field is the **square** of the script's `ReconModifier`, defaulting to `1.0f`, and
`008048A0` does not square it again.

---

## 7. Proven vs. assumed

**Proven from the listing in this pass**

* Both ABIs, from `RET 0Ch` / `RET 8` and the push order at the call sites.
* The range scale: `env^2 * reconModifierSq * m74^2`, then `* m78^2` for a submarine that fails the
  surface test, with a `float` store after every step.
* The horizontal-only distance and the unguarded divide, including the NaN case.
* The row index `observerCategory * 8 + targetCategory`, the `0Ch` pair stride, the `1Ch` entry
  stride, and the empty-row early out.
* The three-part predicate and its order, the `Angle` half-angle semantics, the atan2 argument
  order, and `00438B10`'s `RET 8`.
* The gain `entry[+8h] * dt`, the exact `cap - current` clamp, `applied` being set before the clamp,
  and strictly-greater selection both per entry and per observer.
* The two short circuits, the accumulate/reset split, and the publish to `payload[+0Ch]`.
* The full composition in §6, including that the drain runs once over the flattened list.
* The `ReconModifier` producer, keyed on the string at `00D1AAF4`.
* `m74` and `m78` default to `1.0f`, written by `00444D20`, and both are settable by name from
  Lua: the binding rows at `00E0C248`..`00E0C264` pair `008B2320`/`008B24A0` with
  `"SimplifiedReconMultiplier"` (`00D0F8E4`, stored at `008B243C` into `+74h`) and
  `008B25F0`/`008B2770` with `"SimplifiedSonarMultiplier"` (`00D0F8A4`, stored at `008B270C`
  into `+78h`). Taken from `docs/WEATHER_CONFIG.md`, whose own correction note carries those
  instruction addresses; not re-derived from the listing here.
* The `1Ch` row layout, from the writer `008085C0`..`00808A6A`.

**Assumed, or inherited from an existing contract**

* `[slot+28h]` and `[observer+54h]` coincide for own-triple observers. `00804B45` indexes the
  detection record with the observer's party while `00806840` writes the record indexed by the slot
  index. The rebuild only ever passes the slot's own triple as observers, so they should agree, but
  no instruction proves it. `GunneryReconContact::observer_party_mismatch` reports the case.
* `00922DC0` is a surface/submerged test. `00922C80`'s body was read far enough to rule out a party
  or visibility test, not far enough to state the exact predicate.
* `008E6430(0Ch, observer)` returning a gameplay environment modifier. The category constant `0Ch`
  is read directly; the callee's meaning is a contract from the gameplay-modifier packet.
* `00805AF0` / `00805B3D` / `0080695C` / `00807647` / `008085AA` / `00438B10` bodies, reused from
  `docs/RECON_SLOT_LISTS.md`, `docs/SENSOR_TABLES.md` and `docs/UNIT_RUDDER.md`.

**Partial**

* This packet recovers the predicate **and** the row producer. What it does not recover is the
  script side: which vehicles author which `Dist` / `Gain` / `Angle` / `MaxLevel` rows, and the
  `RawType` enumeration beyond `RRT_VISION(0)` / `RRT_RADAR(1)` / `RRT_SONAR(2)`. Without that the
  rule can be executed but not populated from the shipped data.
* `00922C80` is partial, as stated above.
* `00806840`'s reconstruction models **one** target; the native loops over the whole bucket. The
  per-target slice is exact, the loop is not modelled.

---

## 8. Follow-up packets

1. **The sensor table data.** Walk `008085C0`'s Lua side and dump the authored rows per vehicle
   class, so the rule can be driven from shipped data rather than fixtures. Needs the Lua class
   loader, which is `00960230`'s neighbourhood.
2. **`00922C80` in full.** The surface/submerged predicate is the one remaining guess inside the
   range scale, and it decides whether the sonar penalty applies at all.
3. **`008E6430`, category `0Ch`.** The environment modifier is squared into the range scale and is
   currently a host-supplied float. Its producer would close the last input.
4. **The drain's unknown list.** `+0DFCh` is populated by `00807654`..`0080769B` but nothing in this
   packet reads it. Find its consumer to learn what a blip is allowed to do.
5. **`00805AF0`'s observer attribution.** `det+28h` stores the winning observer pointer; no consumer
   was read here.
6. **The milestone wiring.** `src/unit_gunnery_pass.cpp` builds contacts from (a) and (b) only.
   Applying this rule there is a separate packet: it owns that module, this one does not.

## Correction from docs/RECON_SLOT_OBJECT.md (packet cc7_recon_slot_object)

- **Was:** `[slot+28h]` described as storing the winning observer pointer.
  **Is:** `[slot+28h]` is the **party index**. The winning observer pointer is `00805AF0`'s fourth
  argument and lands in the unit's own detection record, not in the slot. Established from the slot
  constructor `008050E0`, read in full, rather than from a consumer.

- **The observer list** is the slot's own `{count, head, tail}` triple at `+0DD8h`, filled at
  `0080753A` before the grouping pass and handed to `00806840` at `00807556`.

- **A bucket is one list, and its index is the vehicle class id** (`ESI + 34h + i*0Ch`); 24 of the
  97 subscripts are occupied in this installation. Where the `61h` bound itself comes from is **not**
  established. There are three slots by party, and slot 2 never classifies anything as enemy.
