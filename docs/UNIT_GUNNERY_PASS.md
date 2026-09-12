# The unit-side gunnery pass

Addresses: `00864FE0`, `00864580`, `00864380`, `00864BD0`, `008636A0`, `00863A80`,
`008624C0`, `00861CD0`, `00861D20`, `00861D70`, `00861DC0`, `00863990`, `008633D0`,
`00862820`, `00864D90`, `00862C30`, `00864CA0`, `00862440`, `00729BC0`, `00729B90`,
`005459E0`, `0071EBF0`, `00727BD0`, `00863640`, `008627A0`, `00852820`, `008527A0`.
Vtables `00D0D360` (primary), `00D0D348` (observer base), `00D0D314`, `00D0D324`,
`00D0D31C`, `00D0D32C`. Data `00E092C8`, `00E19BF8`, `00E0A4F8`, `00E0A510`,
`00E0A520`, `00E0A528`.

`docs/GUN_BOT_TICKS.md` section 8 established that no gun-side tick reads the weapon
director, that the fire target reaches each bot as a command record at `bot+38h` through
`00727F10`, and that `00727F10`'s only caller is `00864FE0`, left **contract: partially
read**. This document reads `00864FE0` in full: what drives it, how it enumerates the
unit's guns, what it takes from the director, and what it decides per gun.

`docs/GAME_EXECUTABLE.md` milestone 2n stops at this boundary. This is the routine
`bsp_game.exe`'s AI needs before a unit can fire.

## 1. What the object is, and what drives it

`00864580` is the constructor. Its first act is `CALL 0072BBD0` with argument `0`
(`008645A5`) - the **same base constructor the five gun bots use**
(`docs/GUN_BOT_TICKS.md` section 2). The unit-side gunnery pass is therefore a member of
the gun-bot class family, and it is a tick sub-node exactly as the gun bots are.

| Fact | Evidence |
| --- | --- |
| primary vtable `00D0D360` at `this+0h` | `008645B6 MOV dword ptr [ESI],0xd0d360` |
| observer base vtable `00D0D348` at `this+1Ch` | `008645BC` |
| instance size `558h` | `00810E1E PUSH 0x558` before `operator_new`, and the ctor's last field ends at `0CCh + 12*61h = 558h` |
| the tick is vtable slot `+0Ch` | `00D0D36C` holds `00864FE0`; the base's slot `+0Ch` is `0071C490`, a bare `RET 4` |
| `__thiscall(this)(float dt)` | `00865014` `FLD float ptr [EBP+8]`, `0086589E RET 4` |
| body | `00864FE0-008658B8` |

The class shape lines up slot for slot with the gun-bot primary base `00CFDFA0`:

| Slot | Base `00CFDFA0` | This class `00D0D360` |
| --- | --- | --- |
| `+00h` | `0072BE20` | `00864660` scalar deleting destructor |
| `+04h` | `008FBC80` | `00864BD0` attach to a tick element |
| `+08h` | `0071C480` | `0071C480` (inherited) |
| `+0Ch` | `0071C490` (`RET 4`) | **`00864FE0` the pass** |
| `+10h` | `0071C4A0` | `0071C4A0` (inherited) |
| `+14h`..`+2Ch` | `0072BC70`..`0072BCD0` | identical |
| `+30h`..`+4Ch` | - | `00864540`, `00864530`, `00864520`, `0072BD00`, `0072BD10`, `0072BD30`, `00864510`, `00865CB0` |

### Creation and attachment

Ghidra reports no caller for `00864580`; a scan of `.text` for `E8` displacements landing
on it finds **seven** call sites, every one with an identical shape:

```
operator_new(558h) ; 00864580(obj) ; [unit+6DCh] = obj ; obj->vtable[+4h](unit+310h)
```

| Call site | Enclosing function Ghidra reports |
| --- | --- |
| `006D3E6D` | `FUN_006D3C10` (`RunwayLength`, `RunwayWidth` literals) |
| `007483C4` | `FUN_007482B0` (`DamageLevel` literal) |
| `0074D866` | none; `BSP_LiveEffectReferences_Append` `0074D780` is only the nearest earlier start |
| `007D6692` | `FUN_007D5AC0` |
| `00810E3D` | `FUN_00810DD0` |
| `00849F22` | `FUN_008499A0` |
| `0095E85F` | `FUN_0095E5B0` (`LaunchClassID`, `_vehicle` literals) |

`00810DD0` is the clearest of the seven and is quoted here in full because it also fixes
two neighbouring fields:

```
00810DE9  if ([unit+740h] == 0) [unit+740h] = 009F3F20()
00810DFD  if ([unit+744h] == 0) [unit+744h] = [[unit+738h] + 38h]
00810E15  if ([unit+6DCh] == 0) {
00810E1E      p = operator_new(558h); 00864580(p); [unit+6DCh] = p
00810E4C      p->vtable[+4h](unit + 310h)
          }
```

So the gunnery pass lives at **`unit+6DCh`** and hangs off **`unit+310h`**, the unit's
tick element - the same field number the gun bots use on a gun (`gun+310h`,
`docs/GUN_AIMING.md`). `docs/FIXED_STEP_JOB_WAVES.md` wave 2 has `00875B90` call
`008759B0(element, 0.05f)`, which walks `element+1Ch` and calls `sub->vtable[+0Ch](0.05f)`
on every enabled sub-node.

**Cadence: the pass is invoked on every fixed step with `dt = 0.05f`, and it throttles
itself.** It accumulates `dt` into `this+6Ch` and does nothing until that reaches
`[GlobalConfig+88h]`, then zeroes it (`00865014`, `0086502D`, `0086506A`). `00864BD0`
primes `this+6Ch` to `[GlobalConfig+88h]` at attach (`00864C1D`..`00864C28`), so the
first tick after attachment runs immediately. The producer of `GlobalConfig+88h` is
**contract: unread**; it is not written anywhere in `00432650`-`00434500`.

`00864BD0` is not a Ghidra function; read from disk bytes it is
`00864BD0-00864C9C`, `RET 4`:

```
00864BD0  008FBC80(this)(node)                 ; the base attach, sets this+50h
00864BDD  c = operator_new(24h)
          c[0] = [this+50h] ; c[4] = c[8] = c[0Ch] = 0 ; c[20h] = 0
          c[10h] = [00D7A260]  (-1.0f)
00864C0F  this+68h = c                         ; the visibility cache
00864C18  008636A0(this)([this+50h])           ; install the two policy objects
00864C1D  this+6Ch = [GlobalConfig+88h]        ; the first tick runs at once
00864C2B  this+80h .. this+0ACh = 3            ; all twelve masks, twelve stores
00864C78  save [this+77h] and [this+78h]
00864C83  this+70h, +74h, +78h = 01010101h     ; all twelve enable bytes
00864C8C  restore [this+77h] and [this+78h]
00864C94  00863A80(this)                       ; build the director bridge
```

Steps 5 and 6 are the same pair `008624C0` step 3a performs, which is why the
bridge is created only after them.

## 2. The instance

From `00864580` and every read site in `00864FE0`.

| Offset | Field | Evidence |
| --- | --- | --- |
| `+00h` | primary vtable | `008645B6` |
| `+08h`,`+0Ch`,`+10h`,`+11h` | the base sub-node links, expired and enabled bytes | `0072BBD0` |
| `+1Ch` | observer vtable | `008645BC` |
| `+50h` | the owning unit | `008FBC80` from the tick node's payload |
| `+58h` | enabled; `1` in the ctor, latched off when the unit dies | `008645C3`, `008658A1` |
| `+59h` | a suppression latch; requires `[unit+61h]` | `0086503B`..`00865052`, read at `008651E7` |
| `+5Ch` | the fire-target provider (a policy object) | `008636F4`, `00863716` |
| `+60h` | the category gate (a policy object) | `00863743`, `00863761`, `0086376A` |
| `+64h` | the director bridge, a 14h-byte adapter | `00863A80`, read at `00865062` |
| `+68h` | the visibility cache, a 24h-byte vector | `00864C0F`, aged at `0086505D` |
| `+6Ch` | the throttle accumulator | `0086501A`, `0086506A` |
| `+70h`..`+7Bh` | twelve per-category enable bytes, `1` in the ctor | `0086462D`, read at `00865191` |
| `+7Ch` | the category-7 enable override | `008651BD`; written by the bridge from `director+3Dh` at `008624D0` |
| `+7Dh` | lets category 7 fire at the director's own fire target; `1` in the ctor | `008645D6`, read at `0086581C` |
| `+80h + i*4` | twelve per-category target-class masks, `3` in the ctor | `00864632`, read at `008633F0` and `00863409` |
| `+B0h`,`+B4h`,`+B8h` | a `std::list` of per-target engagement records; the sentinel comes from `00863570` | `008645AA`..`008645F7`, walked at `00865078` |
| `+BCh` | a scratch container of `28h`-byte elements | `00864610`, used as the `vtable[0FCh]` out parameter |
| `+CCh + i*61h + classId` | a twelve-by-ninety-seven per-instance allow byte table, memset to `1` | `00864623 PUSH 0x61 / PUSH 0x1 / CALL 00BF79F0`, read at `00862855` |
| size | `558h` | `00810E1E` |

`0CCh` is a byte table of `12 * 61h`; `61h` is 97, and `docs/ENTITY_CLASS_IDS.md` proves
the class-id space is exactly `0`..`60h`. So the table is **one byte per (weapon category,
entity class id)** and the constructor allows every pair.

## 3. The twelve weapon categories, and how the director selects them

`008624C0` is the **director-to-gunnery bridge**, and it is what carries the weapon
director's stance into this object. `00863A80` builds its adapter:

```
00863A85  unit = this+50h
00863A93  require unit->vtable[5Ch](2)                  ; a class test
00863A9F  holder = unit->vtable[140h]()                 ; 0047F320 = "mov eax,ecx; ret"
00863AAD  director = holder->vtable[114h]()             ; 0080E150 = [unit+738h]
00863ABB  a = operator_new(14h)
          a[0] = this ; a[4] = director ; a[8] = 0 ; a[0Ch..10h] = 0
00863AE0  008624C0(a)(force = 1)                        ; then this+64h = a
```

`unit->vtable[140h]` is `0047F320`, literally `mov eax,ecx; ret`. It is the "this object
as a unit" cast: it returns `this` on a unit and (elsewhere) null. `docs/WEAPON_DIRECTOR.md`
already proves `unit->vtable[114h]` = `0080E150` = `[unit+738h]`. So `adapter+4h` **is the
weapon director**, and the identical two-call chain in the tick at `00865466`..`00865498`
reaches the same object.

`008624C0`, `__thiscall(adapter)(char force)`, `RET 4`:

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `008624CB` | `owner+7Ch = [director+3Dh]` - the category-7 override, unconditionally |
| 2 | `008624D3` | `--adapter+8h`; when `force` or the counter has run out, set `force = 1` and reload the counter with `0Ah` |
| 3 | `008624F4` | `allow = [director+3Ch]` (`allowFire`). Skip the rest of this step unless `force` or `allow` differs from the cached `adapter+0Ch` |
| 3a | `00862508` | `allow != 0`: write `01010101h` over `owner+70h`, `+74h` and `+78h` - all twelve enable bytes - then restore the previous `owner+77h` and `owner+78h` |
| 3b | `0086252A` | `allow == 0`: the same three stores with `0`, the same two restores, and then, if `[owner+50h]->vtable[5Ch](0Fh)` (the unit is a plane), force `owner+70h = owner+71h = 1` |
| 4 | `00862561` | `[director+222h]` changed or forced: `00861D70(owner)(v)` |
| 5 | `00862586` | `[director+220h]` changed or forced: `00861CD0(owner)(v)` |
| 6 | `008625AB` | `[director+221h]` changed or forced: `00861D20(owner)(v)` |
| 7 | `008625D0` | `[director+223h]` changed: `00861DC0(owner)(v)` |

The cached copies are `adapter+0Ch` (allowFire), `+0Dh`, `+0Eh`, `+0Fh` and `+10h`. Step 3
costs the full re-enable at most once every ten calls, which is why `owner+77h` and
`owner+78h` are exempted: those two bytes are owned by steps 4 and 7.

The four setters walk four static category lists and edit `owner+80h + category*4`:

| Setter | Director field | Category list | Terminator | Effect on `owner+80h+cat*4` | Also writes |
| --- | --- | --- | --- | --- | --- |
| `00861CD0` | `+220h` | `00E0A4F8` = `{1,2,3,4,6}` | `0Ch` at `00E0A50C` | set / clear **bit 1** | - |
| `00861D20` | `+221h` | `00E0A510` = `{1,5,6}` | `0Ch` at `00E0A51C` | set / clear **bit 0** | - |
| `00861D70` | `+222h` | `00E0A520` = `{7}` | `0Ch` at `00E0A524` | store `3` or `0` | `owner+77h = v` |
| `00861DC0` | `+223h` | `00E0A528` = `{8,9}` | `0Ch` at `00E0A530` | store `3` or `0` | `owner+78h = v` |

Each loop runs while the entry is `< 0Ch`, so category indices are `0`..`0Bh` and the
first entry `>= 0Ch` ends the list.

### What the four groups are

`docs/GUN_BOT_TICKS.md` open question 5 asked what the bits of `[gun+3F0h]+634h` mean.
They are a per-unit fire-inhibit mask whose bit names are string literals in the image,
used by the mission-script actions `aSetGunFireEnabled` (`00465290`) and
`aSetGunFireDisabled` (`00465350`) against the parameter descriptor
`"All|AA_Flak|Artillery|Torpedo|DC"` at `00CE5214`:

| `unit+634h` bit | Literal | Site |
| --- | --- | --- |
| `0` | `AA_Flak` (`00CE5460`) | `004652E2` clears, `004653A5` sets |
| `1` | `Artillery` (`00CE5454`) | `004652FE` clears, `004653C1` sets |
| `2` | `Torpedo` (`00CE544C`) | `0046531A` clears, `004653DD` sets |
| `3` | `DC` (`00CE5448`) | `00465336` clears, `004653F9` sets |

That order is the same order as the four director flags `+220h`..`+223h` read in reverse,
and it matches the four category groups above one for one. So the twelve categories
group as:

| Group | Director flag | `unit+634h` bit | Categories |
| --- | --- | --- | --- |
| Artillery | `+220h` | 1 | `1, 2, 3, 4, 6` |
| AA flak | `+221h` | 0 | `1, 5, 6` |
| Torpedo | `+222h` | 2 | `7` |
| Depth charge | `+223h` | 3 | `8, 9` |

Categories `0`, `0Ah` and `0Bh` belong to no group, so the director never touches their
enable state. Category `1` and category `6` are in two groups each - dual-purpose mounts.

An independent cross-check on the mapping: `00861D20` (the AA flag) toggles **bit 0** of
the mask, and bit 0 of the mask is the bit `008633D0` tests before allowing a target that
answers `IsKindOf(0Fh)` - the plane base. Bit 1, driven by the artillery flag, gates every
non-plane target.

## 4. The target rank table

`00864FE0` ranks candidates with `DAT_00E19BF8[classId + category*61h]`, a signed int.
`00727BD0`, called once from the game singleton's constructor `004DDB90`, builds it:

```
00727BD6  row  = 00E19BF8 ; src = 00E092C8 ; base = 0
          for each category:
00727BEC      rep stosd 61h zeros over the row
00727BF3      rank = 1
              for 61h entries of src:
                  id = *src
00727BFB          if (id != 0) { [00E19BF8 + (id + base)*4] = rank ; ++rank }
00727C0D      row += 184h ; base += 61h
00727C16  until src reaches 00E0A4F8
```

`00E092C8 + 12 * 61h * 4 = 00E0A4F8`, which is both the loop's stop address and the start
of the category group lists - one contiguous authored block. So:

- `00E092C8` is **twelve ordered preference lists**, `61h` class ids each, `0` meaning
  "absent".
- `00E19BF8` is the inverted form: **the one-based position of a class id in its
  category's list, and `0` for a class that is not in the list at all**.
- **Lower rank is more preferred; rank `0` means never engage.**

Both tables are zero in the file on disk (they live in `.data` and are filled at run time
by `00727BD0` from `00E092C8`, which is itself zero in the image). The producer of
`00E092C8` is **contract: unread**. In a live process, category `1`'s list begins
`{17h, 11h, 12h, 15h, 16h, 10h, 13h, 0Eh, 0Ch, 0Bh, 08h, 41h, ...}` and category `0`'s
holds the single value `61h`, which is one past the class-id space and lands in category
`1`'s slot `0`; that is what the data says, and it is recorded here without explanation.

## 5. `00864FE0` as a rule table

`__thiscall(this)(float dt)`, `RET 4`, body `00864FE0-008658B8`, 631 instructions.

### The prologue

| Step | Site | Rule |
| --- | --- | --- |
| 0 | `00864FF0`..`0086500E` | if `!this+58h`, or `this+50h == 0`, or `[unit+5Dh] != 0` (the unit is dead), write `this+58h = 0` and return. The pass latches itself off permanently |
| 1 | `00865014`..`00865035` | `this+6Ch += dt`; `elapsed = this+6Ch`; return unless `elapsed >= [GlobalConfig+88h]` |
| 2 | `0086503B`..`00865052` | if `this+59h` is set and the unit is gone or `[unit+61h] == 0`, clear `this+59h` |
| 3 | `00865057` | `00862C30(this+68h)(elapsed)` - age the visibility cache |
| 4 | `0086506A` | `this+6Ch = 0` |
| 5 | `0086506F` | if `this+64h != 0`, `008624C0(this+64h)(0)` - pull the director stance (section 3) |
| 6 | `00865078`..`0086513C` | walk the `this+B0h` list: for each node, `rec = [node+10h]`; if `[rec+1Ch] == 0` or `[rec+18h] == 0`, call `rec->vtable[0](1)` and erase the node, otherwise `00862CD0(rec)` |
| 7 | `0086513C` | `vector constructor iterator(&candidates, 8, 50h, 00861920)` - an eighty-entry array of `{Entity*, float distance}` on the stack, with a parallel eighty-one-entry order array |

### Per category, `i = 0` to `0Bh`

The loop counter is `[ESP+14h]`, compared `CMP EAX,0xC / JL` at `0086588B`.

| Step | Site | Rule |
| --- | --- | --- |
| 8.1 | `0086516D` | skip the category unless `[unit + 394h + i*0Ch] != 0` |
| 8.2 | `00865191` | `enabled = this[70h + i] != 0 && this[80h + i*4] != 0` |
| 8.3 | `008651B4` | if `enabled` and `i == 7`, replace `enabled` with `this+7Ch` |
| 8.4 | `008651C0` | skip the whole gather unless `[this+60h]->vtable[+4h](i)` |
| 8.5 | `008651D7`..`0086543C` | the recon sweep, only when `enabled && !this+59h && i != 7` |
| 8.6 | `00865442`..`008654AA` | resolve the two director targets |
| 8.7 | `008654AC`..`0086576B` | expand both director targets into sub-entities |
| 8.8 | `00865773`..`0086587E` | assign or clear every gun in the category |

Note that the candidate count is reset to zero at `008651AA`, inside the per-category
body, so each category starts from an empty list.

#### 8.5 The recon sweep

```
008651FE  list = [ BSP_Recon_EnsureSlot([unit+54h]) + DE8h ]
00865220  for (node = list; node; node = [node+4]) {
              cand = [[node+8] + 4]
00865237      if (!00863990(this)(i, cand, &dist))   continue
00865248      if ( 00862440(cand)())                 continue
0086525D      if (!00864D90(this+68h)(cand))         continue
00865272      rank = DAT_00E19BF8[[cand+0C4h] + i*61h]
              insert (cand, dist) into the ordered list
          }
```

`entity+54h` is the side object `docs/GUN_BOT_TICKS.md` already uses in
`00803510([gun+54h],[target+54h])`, and `[recon + DE8h]` is that side's contact list.

The insertion at `00865284`..`0086542B` is an ordered insert with a four-way unrolled
scan. The predicate that stops the scan at position `k` is

```
rank > order[k].rank   OR   (rank == order[k].rank AND dist > order[k].dist)
```

so the array is sorted by **descending** rank with **descending** distance inside a rank.
Step 8.8 walks it from the last entry backwards, which therefore visits the **lowest rank
first and, within a rank, the nearest first**. Since `00727BD0` makes rank 1 the first
authored preference, that is "best class first, nearest first".

`00863990(this)(int category, Entity* target, float* outDist)`, `RET 0Ch`:

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `008639A2` | `008633D0(this)(category, target)` must hold |
| 2 | `008639B6` | `unit = this+50h` must be non-null and answer `IsKindOf(5)` |
| 3 | `008639CC` | refresh both poses through `00414DB0` when `+0C8h` is clear |
| 4 | `00863A21` | `dist = |unit.pos - target.pos|` via `0042B2F0`; `*outDist = dist` |
| 5 | `00863A34` | `dist < [unit + 430h + category*4]` or return false - **the per-category engagement range lives on the unit at `+430h`** |
| 6 | `00863A4F` | if `target->IsKindOf(0Fh)` and `007B8AD0(target)` (`[target+9D8h] == 0`, no follow target), add `[00D7A220]` = `100.0` to `*outDist` - a plane that is not following anything is pushed back a hundred units in the tie-break |

`008633D0(this)(int category, Entity* target)`, `RET 8`:

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `008633DF` | `00862820(this)(category, target)` must hold |
| 2 | `008633F0` | if bit `0` of `this[80h + category*4]` is clear, reject a target that answers `IsKindOf(0Fh)` |
| 3 | `00863409` | if bit `1` is clear, reject a target that does **not** answer `IsKindOf(0Fh)` |

`00862820(this)(int category, Entity* target)`, `RET 8`:

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `00862825` | require `[target+5Ch] != 0`, `[target+5Dh] == 0`, `[target+60h] == 0`, `[target+5Eh] == 0` |
| 2 | `00862855` | require `this[0CCh + category*61h + [target+0C4h]] != 0` |
| 3 | `0086285F` | require `DAT_00E19BF8[[target+0C4h] + category*61h] != 0` |
| 4 | `00862869` | category `7`: require `target->IsKindOf(6)` and **not** `target->IsKindOf(0Eh)` |
| 5 | `0086288C` | if `target->IsKindOf(8)`: categories `8` and `9` additionally require `00852820(target)`; every other category except `7` requires it too |

`00852820(entity)` refreshes the pose and returns `entity.y > ([entity+1204h] + [entity+1200h]) / [00D7A2B0]` - a depth test, which is why it gates the depth-charge categories against a class-8 target.

`00862440(entity)`, `RET 0`: `entity->vtable[140h]() != 0 && [that + 1D4h] != 0`. A
candidate whose unit-side AI sets `+1D4h` is skipped by the sweep. What `+1D4h` means is
**contract: unread**.

`00864D90(cache)(Entity* target)`, `RET 4`, is the visibility test with a cache:

- `cache+4h` is a `0Ch`-byte-stride array, `cache+8h` its size, `cache+0Ch` its capacity;
  each entry is `{Entity* key, byte result, float ttl}`.
- `00864D9C` linear-searches for `key == target` and returns the cached `+4h` byte.
- On a miss it computes an aim point from the target's position plus a height taken from
  `[target+538h]` (`+0A8h`, or a `GlobalConfig+98h`-scaled term through `00862C00` when
  `[[target+538h]+50h]` allows), calls `00864680(cache)(point)` for the actual test,
  picks `GlobalConfig+0A0h` or `+0A4h` as the TTL depending on the answer, jitters it
  through `00BD2F10`, grows the array if needed and appends the entry.
- `00862C30(cache)(float dt)` subtracts `dt` from `cache+10h` and from every entry's
  `+8h`, and compacts out every entry whose value has reached zero.

#### 8.6 The two director targets

```
00865442  fireTarget    = [this+5Ch]->vtable[+4h]()
00865456  commandTarget = 0
          if (unit && unit->vtable[140h]()) {
00865476      d = unit->vtable[140h]()->vtable[114h]()
              if (d) {
0086549C          commandTarget = 00521EA0( 0071EBF0( unit->vtable[140h]()->vtable[114h]() ) )
              }
          }
```

These are **two different targets**, and `docs/GUN_BOT_TICKS.md` section 8 named only the
second.

`[this+5Ch]` is one of two policy objects installed by `008636A0`. When the unit answers
`IsKindOf(2)`, has a director, and the director answers `vtable[48h](2)`, the object is
eight bytes with vtable `00D0D324` holding the director; otherwise it is four bytes with
vtable `00D0D314`. Their `vtable[+4h]` slots are:

- `00861B90` (default): `xor eax,eax; ret` - no fire target.
- `00863640` (director-backed): `director->vtable[2Ch]()`, which `docs/WEAPON_DIRECTOR.md`
  proves is `008364E0`, the getter for `director+238h`. On a null answer it follows
  `[director+34h]` through `007788D0`, and if that entity answers `IsKindOf(6)` it reads
  that ship's own `[+738h]` director's `vtable[2Ch]` instead - a fall back to the leader's
  fire target.

`0071EBF0(director)` is different: it scans the ten command slots at `director+54h`,
stride `1Ch`, for the first empty one and takes the **last occupied** slot's command, then
returns `cmd->vtable[0Ch]()`. `00521EA0` resolves that to an entity. It only runs the scan
when `[director+30h] == 1`; the other arm at `0071ECD8` is **contract: unread**.

So `fireTarget` is the director's standing fire target and `commandTarget` is the newest
queued command's target.

#### 8.7 Sub-entity expansion

Both targets are expanded the same way, `commandTarget` at `008654AC` and `fireTarget` at
`00865609`:

```
     if (t && t != fireTarget && 00863990(this)(i, t, &d)) {
008654E5    origin = 00427EB0(unit)              ; commandTarget uses the refreshed pose
00865647    origin = unit+0FCh..+104h            ; fireTarget reads the pose fields directly
00865516    0063BCD0(this+0BCh)                  ; clear the scratch list
00865521    t->vtable[0FCh](this+0BCh)           ; ask the target for its sub-entities
0086552A    iterate the scratch list
0086557A       s = *it
00865584       if (00864D90(this+68h)(s)) {
                   candidates[n] = { s, |origin - s.pos| } ; order[n] = n ; ++n
               }
     }
```

These entries are appended **unsorted** - `order[n] = n` at `008655E1` and `0086574B`,
not an ordered insert. They therefore sit at the end of the order array and, because
step 8.8 walks the array backwards, **the director's targets are tried before anything
the recon sweep found**. The `fireTarget` block runs last, so its sub-entities are tried
first of all.

`00427EB0` is `BSP_EntityPose_GetWorldPositionRefreshed`; the `fireTarget` arm inlines the
same thing with an explicit `00414DB0` when `[unit+0C8h]` is clear.

#### 8.8 Assignment

```
00865773  for (node = [unit + 398h + i*0Ch]; node; node = [node+4]) {
              gun = [node+8]
0086579A      for (k = n - 1; k >= 0; --k) {
                  c = &candidates[order[k]]
008657C0          if (005459E0(gun) && c->entity->vtable[5Ch](0Fh)
                      && 00729B90(gun) > c->dist)      continue
008657F7          if (!00729BC0(gun)(c->entity))       continue
00865804          assigned = (c->entity == fireTarget)
00865809          if (i == 7 && assigned && !this+7Dh) continue
00865833          00727F10(gun)(c->entity, assigned)
00865838          k2 = [[gun+3F4h] + 80h]
                  if (k2 == 2 || k2 == 3 || k2 == 4 || k2 == 6)
00865860              00864CA0(this)(c->entity, gun)
                  goto next_gun
              }
0086586C      00728000(gun)()
          }
```

**The unit's gun list is `unit + 394h + i*0Ch`.** The stride is `0Ch` and there are twelve
records: `+394h` is the gate step 8.1 reads and `+398h` is the head of a singly linked
list whose nodes carry `next` at `+4h` and the gun at `+8h`. The third word at `+39Ch` is
not read here. This is neither the `+48h` device list nor the `+3D0h`/`+3CCh` root array
the packet asked about; it is a third, category-bucketed index.

The three gun-side predicates:

- `005459E0(gun)`, `RET 0`: `[[gun+3F4h]+80h]` is `5` or `6`. Those are the torpedo and
  sub-kind-6 weapon kinds of `docs/WEAPON_DIRECTOR.md`.
- `00729B90(gun)`, `RET 0`, returns a float: `[[gun+3F4h]+74h + (kind == 6 ? 48h : 0) + 34h]`
  dereferenced, then `+58h`. Used only as a **minimum** distance and only against a plane,
  so a torpedo or depth-charge mount will not take an air target that is closer than that.
- `00729BC0(gun)(Entity* t)`, `RET 4`: rejects a null target, rejects a target that answers
  `IsKindOf(5)` with `[t+54h] == 2`, refreshes both poses, rejects when the distance
  exceeds `[proj+60h]` of the ammunition record, then dispatches on the projectile kind
  `[proj+8h]` to one of the gun's six bot slots and returns `slot->vtable[1Ch](t)`:

  | Projectile kind `[proj+8h]` | Bot slot |
  | --- | --- |
  | `1`, `2`, `3` | `gun+390h` |
  | `10h` | `gun+394h` |
  | `4`, `5`, `6`, `7` | `gun+398h` |
  | `0Ah` (Torpedo) | `gun+39Ch` |
  | `0Bh` (DepthCharge) | `gun+3A4h` when `[[gun+3F4h]+80h] == 9`, else `gun+3A0h` |
  | anything else | none; returns false |

  The ammunition record is `*([gun+3F4h] + 74h + variant*48h + 34h)` with `variant = 1`
  only when the weapon kind is `6` and the target is a plane. Those are the same six slots
  `00727F10` fans a target out to.

`00864CA0(this)(Entity* target, Gun* gun)`, `RET 8`, looks the target up in the `this+B0h`
list through `00863B10`, and on a miss allocates `8Ch` bytes, builds the record with
`00864880(rec)([this+50h], target)` and inserts it with `00864160`; either way it ends with
`00862660(rec)(gun)`. The record is the per-target engagement group step 6 keeps alive and
`00862CD0` updates. `00862CD0` (`00862CD0-008630FC`) and `00864880` are **contract: unread**.

The final `00865865 CMP dword ptr [ESI],0x0 / JNZ` is a redundant re-test of a candidate
already proven non-null by `00729BC0`; the reachable path into `0086586A` is the
candidate loop running out, at `008657A3` when the list is empty and at `00865827`
otherwise.

## 6. The category gate at `this+60h`

`008636A0(this)(Entity* unit)`, `RET 4`, installs both policy objects:

| Condition | `this+5Ch` | `this+60h` |
| --- | --- | --- |
| `unit->IsKindOf(2)` and `unit->vtable[114h]()` and `director->vtable[48h](2)` | 8 bytes, vtable `00D0D324`, `[+4] = director` | - |
| `unit->IsKindOf(8)` | - | 8 bytes, vtable `00D0D32C`, `[+4] = unit` |
| otherwise | 4 bytes, vtable `00D0D314` | 4 bytes, vtable `00D0D31C` |

The gate's `vtable[+4h]`:

- `00861BE0` (default): `mov al,1; ret 4` - every category runs.
- `008627A0` (class 8): category `7` runs only when `008527E0(unit)`; categories `1`, `2`,
  `5` and `6` run only when `008527A0(unit)`; every other category is refused. Both
  `008527E0` and `008527A0` open with the `[entity+0C8h]` pose refresh, and `008527E0` is
  the routine `docs/GUN_BOT_TICKS.md` section 10 lists as **contract: unread**. Class 8
  and the depth test at `00852820` put this on the submarine, but that reading is a
  hypothesis: no string literal was found for it.

## 7. What a director stand-in must provide

Replacing `docs/GUN_BOT_TICKS.md` section 8 item 2 and adding to it. The unit-side pass
reads the director in exactly four ways:

1. `director+3Ch` **allowFire** and `director+3Dh`, through `008624C0` step 1 and step 3.
   `allowFire` clear zeroes all twelve enable bytes, except that a plane keeps categories
   `0` and `1`.
2. `director+220h`..`+223h`, the four sub-kind flags, through `008624C0` steps 4 to 7 and
   the four category lists at `00E0A4F8`..`00E0A528`.
3. `director+238h` **fireTarget**, through `this+5Ch`'s `00863640` -> `vtable[2Ch]` ->
   `008364E0`, with the leader fallback through `[director+34h]`.
4. the newest of the ten command slots at `director+54h`, through `0071EBF0` and
   `00521EA0`, and only while `[director+30h] == 1`.

Everything else the pass needs comes from the unit: `unit+394h` (the twelve category
records), `unit+430h` (the twelve engagement ranges), `unit+5Dh`, `unit+61h`,
`unit+54h` (the side), `unit+634h` (the scripted inhibit mask, read on the gun side) and
`unit+310h` (the tick element).

## 8. Corrections to earlier documents

| Document | Was | Is | Evidence |
| --- | --- | --- | --- |
| `docs/GUN_BOT_TICKS.md` open question 5 | "Whether bit `1` of `[unit+634h]` really means a human is aiming this gun" | the four bits are a scripted per-category fire inhibit: `0` `AA_Flak`, `1` `Artillery`, `2` `Torpedo`, `3` `DC`. Every writer is the constructor or a mission-script action | the parameter descriptor `"All|AA_Flak|Artillery|Torpedo|DC"` at `00CE5214`, and the writers `004652E2`..`00465336` (`aSetGunFireEnabled`) and `004653A5`..`004653F9` (`aSetGunFireDisabled`) |
| `docs/GUN_BOT_TICKS.md` section 5 step 4 and the routine table | `00927F10` named `BSP_Side_AutoEngageEnabled`, "reads `[[00E188A8]+18CCh+side*4]+9`" | the expression is right; the meaning is not. The record is a **party slot**, and `+9h` is `1` when the slot is AI- or mission-held and `0` when a human player holds it | `004BB220` is the only writer, inside `004BB160`, and `0076C86F` requires the joining player's record to have `+9 == 0` |
| `docs/HUD_CENTRAL_UPDATES.md` line 238 | `00927F10(unit, [unit+1ACh])` | `00927F10` is `__stdcall(int slot)`, one stack argument, `RET 4`, no `ECX` input | body `00927F10-00927F26`; the call site `008FFA98` is `push eax; call 0x927f10` with `ECX` never loaded |
| `docs/GUN_BOT_TICKS.md` section 8 item 2 | "`00864FE0`, the unit-side gunnery pass, which reads the director through `entity->vtable[114h]()`" | `00864FE0` reads the director through `unit->vtable[140h]()` first, which is `0047F320`, an identity cast, and then `vtable[114h]()`. It reads **two** targets from the director, not one | `00865466`..`0086549C`, and `00863640` for the second |
| `docs/GUN_BOT_TICKS.md` section 4 | the skill arrays are described relative to the descriptor pointer because no producer was found | the descriptor header is `0Ch` (vtable, `NoTargetTimeUntilRest`, name), the skill array begins at `descriptor+0Ch` and holds six records, and `load_robot_config_00901610` builds all of them from `Scripts\datatables\Robots.lua` | `00901423` writes the `+4h` float; each allocation equals `0Ch + 6*stride`; the stores at `009019EB`..`00901A53` are the only writes to the five globals |
| `docs/GUN_BOT_TICKS.md` section 4 | "the four lead scalars" at descriptor `+20h`..`+2Ch` | they are `SectionTargetChance`, `EngineRoomWeight`, `MagazineWeight`, `FueltankWeight` - subsystem target weights - fed to `target->vtable[100h]` | the Lua key literals at `00D17E90`, `00D17E7C`, `00D17E6C`, `00D17E5C` and the stores at `008FCA65`..`008FCC0C` |

## 9. The skill descriptors, for reference

Answering the packet's fifth question; the producer is `load_robot_config_00901610`, which
`docs/ROBOT_CONFIG.md` already reconstructs.

| Global | Class name literal | Stride | Allocation |
| --- | --- | --- | --- |
| `00F8A30C` | `PilotBot` | `248h` | `DBCh` |
| `00E199A0` | `TailGunnerBot` | `24h` | `E4h` |
| `00E1999C` | `AAFlakBot` | `20h` | `CCh` |
| `00E19998` | `AAGunnerBot` | `10h` | `6Ch` |
| `00E19994` | `ArtillerySubDirectorBot` | `28h` | `FCh` |
| `00E19990` | `ArtilleryGunnerBot` | `1Ch` | `B4h` |
| `00E1998C` | `TorpedoBot` | `14h` | `84h` |
| `00E19988` | `DepthChargeBot` | `14h` | `84h` |

Every allocation is `0Ch + 6 * stride`. The six skill levels are `Stun` 0, `SPNormal` 1,
`SPVeteran` 2, `MPNormal` 3, `MPVeteran` 4, `Elite` 5, from the key literals at
`00D18398`..`00D183CC` and the indices at `0090142D` and `00901494`. The index reaching a
bot is `unit+390h`, read by `unit->vtable[12Ch]` = `006D1EC0` = `mov eax,[this+390h]; ret`
and defaulted to `1` at `0095CCCC`. Who writes `unit+390h` from the authored scene
property is **not established**.

## 10. Routine table

| Routine | Body | Coverage |
| --- | --- | --- |
| `00864FE0` | `00864FE0-008658B8` | complete |
| `00864580` | `00864580-0086465E` | complete |
| `008624C0` | `008624C0-008625F3` | complete |
| `00861CD0`, `00861D20`, `00861D70`, `00861DC0` | - | complete |
| `00863990` | `00863990-00863A7E` | complete |
| `008633D0` | `008633D0-00863427` | complete |
| `00862820` | `00862820-008628CB` | complete |
| `00863A80` | `00863A80-00863AFE` | complete for the allocation and the field writes; the tail after `00863AE0` read as the `008624C0(force = 1)` call only |
| `008636A0` | `008636A0-0086376F` | complete |
| `00727BD0` | `00727BD0-00727C24` | complete |
| `00729BC0` | `00729BC0-00729D27` | complete |
| `00729B90`, `005459E0`, `00861B90`, `00861BE0`, `00863640` | - | complete |
| `0071EBF0` | `0071EBF0-0071ECEB` | partial: the `[director+30h] == 1` arm only; `0071ECD8` unread |
| `00864D90` | `00864D90-00864FC7` | partial: the cache lookup, the aim point and the append; the `00864680` test itself and the `00862C00` height term unread |
| `00862C30` | `00862C30-00862CCC` | complete |
| `00864CA0` | `00864CA0-00864D83` | complete |
| `00864BD0` | `00864BD0-00864C9C` | complete; no Ghidra function |
| `00864380` | `00864380-0086454F` | partial: the vtable re-install and the four member destructions |
| `008627A0` | `008627A0-008627EF` | complete; no Ghidra function |
| `00862440`, `00852820`, `0047F320`, `007B8AD0` | - | complete |
| `00862CD0`, `00864880`, `00864680`, `00862C00`, `00862660`, `00863B10`, `00864160`, `00864220`, `00863570`, `008527A0`, `008527E0`, `008053C0`, `009F3F20` | - | contract: unread |

## 11. Open questions

- The producer of `00E092C8`, the twelve authored preference lists, and of
  `GlobalConfig+88h`, `+90h`, `+98h`, `+0A0h` and `+0A4h`.
- `unit+1D4h`, the flag `00862440` uses to veto a recon candidate.
- What the third word of the category record, `unit + 39Ch + i*0Ch`, holds, and who fills
  the twelve engagement ranges at `unit+430h`.
- `00862CD0` and the `8Ch`-byte per-target engagement record `00864880` builds: the pass
  keeps them alive and hands guns to them, but nothing here reads one back.
- Whether `008627A0`'s class-8 gate really is the submarine; `008527A0` and `008527E0` are
  unread and no literal names them.
- The relationship between the category index `0`..`0Bh` used here and the weapon
  descriptor sub-type `[gun+3F4h]+80h` used by `00729BC0` and `0072C6A0`. They are
  different enumerations: the descriptor sub-type reaches `9`, the category reaches `0Bh`,
  and the mapping between them was not established.

## Correction from docs/GUNNERY_TABLES.md (packet cc2_gunnery_tables)

- **Was:** Both tables are zero in the file on disk (they live in .data and are filled at run time by 00727BD0 from 00E092C8, which is itself zero in the image). The producer of 00E092C8 is contract: unread.
  **Is:** 00E092C8 is non-zero in the image. .data has raw size 10000h from VA 00E08000, so 00E092C8-00E0A4F8 is initialized storage on disk and carries the twelve authored rows. There is no run-time producer at all: the only reference to 00E092C8 anywhere in the image is the MOV ESI immediate at 00727BDB. Only 00E19BF8, which is past the raw extent, is zero at load.
  **Evidence:** PE section header .data va=00E08000 vsize=297EDC raw=00A08000 rsize=010000; a whole-image scan for the little-endian dword 00E092C8 returns one hit at 00727BDC; the rows at 00E092C8, 00E0944C, 00E095D0, 00E09754, 00E098D8, 00E09A5C, 00E09BE0, 00E09D64, 00E09EE8, 00E0A06C read back as the table above, and row 1 reproduces the values the document recorded as a live-process observation.
- **Was:** category 0's list holds the single value 61h, which is one past the class-id space and lands in category 1's slot 0; that is what the data says, and it is recorded here without explanation.
  **Is:** Category 0 is the PLANEGUN weapon Function, which no unit-side gunnery pass drives, so the row is a placeholder whose one entry cannot match a class. The stray rank it writes at 00E19BF8+184h is erased by the REP STOSD at 00727BEC on the next iteration, which zeroes all 61h dwords of row 1 before filling it.
  **Evidence:** 007327B0's Function chain gives category 0 = PLANEGUN; 00727BEC zeroes the row at EBP = 00E19BF8 + category*184h at the top of each iteration, and category 0's write goes to 00E19BF8 + 61h*4 = 00E19BF8 + 184h.
- **Was:** +394h is the gate step 8.1 reads and +398h is the head of a singly linked list whose nodes carry next at +4h and the gun at +8h. The third word at +39Ch is not read here.
  **Is:** The record is a doubly linked list header: +394h is the node count, +398h the head, +39Ch the tail. The node is prev at +0h, next at +4h, gun at +8h. The pass's gate is the count compared with zero.
  **Evidence:** 00956C20's insert at 00956CB6-00956CF4: 00956CD7 node[+8h] = gun, 00956CDD node[+0h] = record[+8h], 00956CEB record[+4h] = node when record[+0h] == 0 else 00956CE6 record[+8h]->next = node, 00956CEE record[+8h] = node, 00956CF1 node[+4h] = 0, 00956CF4 record[+0h] += 1. 0086516D compares the dword at unit + cat*0Ch + 394h with zero and skips the category.
- **Was:** The producer of GlobalConfig+88h is contract: unread; it is not written anywhere in 00432650-00434500.
  **Is:** 0087D7B0 writes it at 0087E16B from Globals.WeaponSystems.WeaponDirectorThinkTime, installed value 2 seconds. The constructor 004324E0 leaves it alone because the whole 2E8h object is filled by that loader, not by the constructor.
  **Evidence:** 0087E114 pushes the key string 00D0E418 "WeaponSystems" and 0087E149 pushes 00D0E400 "WeaponDirectorThinkTime"; 0087E166 calls 00B66270 (lua_tonumber) and 0087E16B is FSTP [ESI+88h]. Scripts/datatables/globals.lua has WeaponSystems.WeaponDirectorThinkTime = 2.
- **Was:** 00862440(entity): entity->vtable[140h]() != 0 && [that + 1D4h] != 0. A candidate whose unit-side AI sets +1D4h is skipped by the sweep. What +1D4h means is contract: unread.
  **Is:** +1D4h is the script-set untouchable flag. The Lua binding AddUntouchableUnit(unit) sets it and RemoveUntouchableUnit(unit) clears it; IsUnitUntouchable(unit) reads it. It is not written by any AI. vtable[140h] is a proxy accessor, the identity on the ship base.
  **Evidence:** Binding records at 00E0C088/00E0C090/00E0C098 pair the three recovered name strings with 008AC420/008AC140/008AC2B0, whose bodies read, set and clear the byte at 008AC552, 008AC263 and 008AC3D6; 0047F320 is MOV EAX,ECX; RET.
- **Was:** the per-category engagement range lives on the unit at +430h (producer not stated).
  **Is:** 00956C20 writes it: unit+430h + cat*4 is the longest weapon range among that category's live guns, floored at the 10.0f constant at 00CE38B8.
  **Evidence:** 00956D63 seeds the row from 00CE38B8; 00956D9F calls 00731020 for each gun on the category list and 00956DF3 stores the running maximum. 00863A34 reads the same slot.
