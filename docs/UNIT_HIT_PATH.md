# Unit hit path: impact record to damage, and the repair tick

Addresses: 008777D0 007BBCF0 00470510 004705C0 004704E0 00879810 0093C770 0093C860 0093CA20
0087BCC0 00934150 0080E410 0080E440 00878340 007470B0 0074CD30 0087BD01 0087BCF4 0087BD11

Packet `cc2_unit_hit_path`, read-only in Ghidra. Every descriptive name below is a hypothesis, not a
recovered symbol. This packet continues docs/UNIT_DAMAGE_AND_DEATH.md, which established
`00879070` `BSP_UnitInstance_ApplyDamage`, `00877B90` `BSP_UnitInstance_SetHealth` and the fields
`+150h` / `+36Ch` / `+370h`. Nothing in that doc is restated here except where a claim corrects it.

## Correction: 00879810 is not on the hit path

`00879810` (`00879810..0087982B`, `RET 4`, six instructions, read from the listing) is

```
00879810  MOVSS  XMM0, dword ptr [0x00D7A208]   ; -0.0f  (bytes 00 00 00 80)
00879818  SUBSS  XMM0, dword ptr [ESP + 0x4]    ; -0.0f - amount  ==  -amount
0087981E  PUSH   ECX                            ; allocate one stack slot
0087981F  MOVSS  dword ptr [ESP], XMM0          ; overwrite it with the negated float
00879824  CALL   0x00879070                     ; ECX is untouched, so `this` passes through
00879829  RET    0x4
```

so it is `BSP_UnitInstance_ApplyHealthDelta`: it negates its float and calls `ApplyDamage`. Because
`00879070` treats a non-positive amount as a repair, a **positive** argument to `00879810` raises
health and a negative one lowers it. Its only two callers, `0093C770` and `0093C860`, are the hull
and subobject steps of the damage-control repair tick, not an impact path. The hit path reaches
`00879070` through the other caller, `0087D730`.

Evidence for the constant: `python tools/bsp.py ghidra bytes 00D7A200 --length 24` gives
`00D7A208 = 00 00 00 80`, the float `-0.0f`. `00D7A210` is the double `1.0` the damage doc already
cites, so the two are neighbours in the same literal pool and must not be confused.

## The impact-to-damage chain

`008777D0` is the unit instance's **hit-record handler**. It is reached virtually, so the table
below names the containing function of each site; `python tools/bsp.py ghidra proto <site> --brief`
was run for every one.

| level | site | callee | this | arguments | what it contributes |
| --- | --- | --- | --- | --- | --- |
| 0 | (vtable dispatch, producer unread) | `007BBCF0` | unit instance | `hit` record ptr | entity-side hit response: clears `+520h`, sets `+DFCh` when the shooter passes `IsKindOf(5)`, refreshes the pose for the local player's unit, forwards to `[+9D4h]->vtable[ECh]` and to `00999AA0`, copies `[[hit+38h]+4h]` into `unit+800h` when `hit+34h != -1` |
| 1 | `007BBDB1` | `008777D0` | unit instance | `hit` record ptr | splits the record into one hull damage and one part damage |
| 2a | `008778A4` | `00470510` | `hit` record | `float armour * mod2` | hull damage value |
| 2b | `008779D3` (loop) | `004705C0` | `hit` record | `float armour * mod2`, `int partHitIndex` | per-part-hit damage value |
| 3 | `008778D2`, `00877A37` | `vtable[1ACh]` | unit instance | `float damage` | `AddDamage`; base `0095DA00`, overrides `007470B0` and `0074CD30` |
| 4 | `0095DA8D` | `0087D730` | unit instance | `float damage` | difficulty scaling, docs/UNIT_DAMAGE_AND_DEATH.md |
| 5 | `0087D7A3` | `00879070` | unit instance | `float damage` | the damage rule |
| 6 | `0087914B` | `00877B90` | unit instance | `float health - amount` | the only writer of `+370h` |

Both level-3 sites store the value they are about to pass into `hit+48h` first
(`008778BF` and `00877A24`), so the record carries the applied damage back to its producer.

`008777D0` returns `1`. Its own body is `008777D0..00877A43`; argument counts come from the stack:
`00470510` and `004705C0` are `__thiscall` with the record in `ECX`, and `008777D0` itself takes
one stack argument (`MOV EBX, [ESP+0x1C]` at `008777D4` after `SUB ESP,0x14 / PUSH EBX`).

### What 008777D0 does

1. **Hull pass**, skipped entirely when `hit+34h == -1` (`008777DD`, `CMP [EBX+0x34],EDI` with
   `EDI = -1` set by `OR EDI,0xFFFFFFFF` at `008777DA`; register provenance taken by filtering the
   whole listing for `EDI`, whose only writes in the body are `008777DA`, `008778E4` (`XOR EDI,EDI`,
   the part-loop counter) and `008779FA` (`ADD EDI,1`)).
   * armour source: when `this->vtable[5Ch](6)` is true **and** `hit+0Ch < 0.0f`
     (`COMISS XMM0(0), [EBX+0Ch] / JBE`), armour is `[this+354h]->vtable[24h]()`; otherwise it is
     the plain field `this+368h`.
   * armour modifier: `BSP_GameplayModifiers_ProductForUnit(2, x)` with `x = this+3F0h` when
     `this->vtable[5Ch](20h)` is true and `x = this` otherwise, and only while
     `00E0C978 != 0 && [00F88C30+A0h] != 0`; else `1.0f` (`00D7A24C`).
   * `damage = 00470510(hit, armour * modifier)` at `008778A4`.
   * when `damage > 0`: `hit+48h = damage` and `this->vtable[1ACh](damage)`.
2. **Part pass** (`008778E4..00877A37`), over `hit+40h` entries of `10h` bytes at `hit+3Ch`, skipping entries whose
   `+4h` is `-1`. The armour source here is `[this+354h]->vtable[24h]()` when the entry's `+0h`
   equals `4` and `this+368h` otherwise; the modifier is the same `ProductForUnit(2, ...)` value.
   The loop keeps the **largest** `004705C0` result and its index, starting from `00D7A244`
   (`FF 7F FF FF`, `-FLT_MAX`), and afterwards, when an index was kept and the value exceeds
   `00D7A218` (`0.0f`), does `hit+48h = value` and one more `this->vtable[1ACh](value)`.

So a single impact can raise `AddDamage` twice: once for the hull segment and once for the worst
part hit. The winning part index is computed but not passed on from here; whatever consumes it
does so through the record.

### The two damage formulas

`00470510` (hull) and `004705C0` (part) are `__thiscall` on the hit record and return in `ST(0)`.
Both read a weapon multiplier and an owner-scoped gameplay modifier from the record's `+4h`
pointer, which is the shot/weapon descriptor:

| step | `00470510` (hull) | `004705C0` (part) |
| --- | --- | --- |
| weapon scale `s` | `[hit+4h]->vtable[58h]()`, else `1.0f` | same |
| base `b` | `hit+14h` | `hit+28h` |
| owner modifier | `ProductForUnit(1, [[hit+4h]->vtable[108h]()+ACh])` when that owner is non-null and `00E0C978 != 0 && [00F88C30+94h] != 0`; else `1.0f` | same |
| falloff `f` | none | `1 - entry[i].+0Ch / hit+24h`, replaced by `1.0f` when `hit+2Ch != 0` and `f > 0` |
| result | `(b * ownerMod - armourScaled) * s` | `max(0, f * b * ownerMod - armourScaled) * s` |

The hull result is **not** floored at zero, so an armour value above the weapon's hull damage makes
`00470510` return a negative number; `008777D0` then skips the `AddDamage` call because of its
`0.0 < damage` test, and no repair happens. The part result is floored at zero inside `004705C0`.

### The hit record

Read from the two consumers (`008777D0`, `00470510`, `004705C0`) and from `007BBCF0`. The producer
is unread, so rule 4 of docs/WORKER_VERIFICATION_CHECKLIST.md is **not** satisfied for this table:
every meaning below is a consumer's use, and the layout is `contract: producer unread`.

| offset | type | read at | meaning as used |
| --- | --- | --- | --- |
| `+04h` | ptr | `00470510` body, `004705C0` body | shot/weapon descriptor; `vtable[58h]()` is a float scale, `vtable[108h]()` an object whose `+ACh` is the owning unit |
| `+0Ch` | float | `008777F8` | negative selects the `[unit+354h]->vtable[24h]()` armour source |
| `+14h` | float | `00470510` body | hull damage base |
| `+24h` | float | `004705C0` body | part falloff range (divisor) |
| `+28h` | float | `004705C0` body | part damage base |
| `+2Ch` | byte | `004705C0` body | "ignore falloff" flag |
| `+34h` | int | `008777DD`, `007BBCF0` body | hull segment index; `-1` means no hull hit |
| `+38h` | ptr | `007BBCF0` body | record whose `+4h` is copied into `unit+800h` |
| `+3Ch` | ptr | `008777D0` part loop, `004705C0` body | array of `10h`-byte part-hit entries: `+0h` kind (`4` selects the alternate armour), `+4h` part index (`-1` skips), `+0Ch` float distance |
| `+40h` | int | `008778E6`, `00877A00` | part-hit count |
| `+48h` | float | `008778BF`, `00877A24` | written back: the damage the handler applied |

## The part model

`0087BCC0` (`0087BCC0..0087BF73`, `__thiscall`, SEH frame) is the unit's **part and health
initialiser**, and it is where the maximum health comes from.

| site | store | source |
| --- | --- | --- |
| `0087BCF4` | `this+368h` = armour | `[this+354h]+4Ch` |
| `0087BD01` | `this+36Ch` = maximum health | `[this+354h]+48h` |
| `0087BD09` | `this+164h` = `1.0f` | `00D7A24C` |
| `0087BD11` | `this+370h` = current health | `[this+354h]+48h`, the same `XMM0` |
| `0087BD19` | `this+378h` = `1` | immediate |

`this+354h` is the vehicle-class descriptor (proven in docs/UNIT_INSTANCE_UPDATE.md, written by
`006FE590`), and docs/VEHICLE_CLASS_FIELDS.md attributes `+48h` to the Lua key `HP` (default
`100.0f`, written at `0087CC98`) and `+4Ch` to `Armour` (default `0.0f`, `0087CCB4`). So:

* **maximum health `+36Ch` is the class row's `HP`**, applied once at construction and never
  written again by any store this packet found (see the search below);
* the unit starts at full health;
* `+368h`, the armour `008777D0` subtracts from every weapon's damage, is the class row's `Armour`.

The open question of docs/UNIT_DAMAGE_AND_DEATH.md ("no writer of `+36Ch` was found") is answered:
the writer is `0087BD01`.

Search method: `mcp__ghidra__search_byte_patterns` over the whole program for every `MOVSS
[reg+36Ch], XMM0` encoding (`F3 0F 11 8x 6C 03 00 00`) and the `FSTP [reg+36Ch]` forms
(`D9 9x 6C 03 00 00`). The MOVSS hits are `006F2486`, `007C6D35`, `007C6DFD`, `007F2D16`
(`BSP_PlaneSquadronTickableEntity_Construct`) and `0087BD01`; the FSTP hits are `005C7F02`,
`005C865C`, `006F3951`, `008C1A71`, `0092AE25`. Only `0087BD01` is inside a routine that also
writes `+368h`, `+370h` and `+378h` in the same breath on a `+1188h` unit instance; the other nine
are `contract: unread` and could belong to other classes at the same offset.

### The part list

After the health block, `0087BCC0` builds the unit's part table:

| site | what |
| --- | --- |
| `0087BD20..0087BD43` | `n = ([desc+20h] - [desc+1Ch]) / 30h` (`IMUL 0x2AAAAAAB / SAR EDX,3`, a divide by `48`), or `0` when `[desc+1Ch]` is null |
| `0087BD4F` | `0087B460(this+344h, n, 0)` resizes the vector at `this+344h` to `n` null pointers (4-byte elements: `SUB EAX,ECX / SAR EAX,2` at `0087BD62`) |
| `0087BD58..0087BDC7` | for `i` in `[0, n)`: `this+344h[i] = [desc+1Ch] + i*30h`, with a bounds check on both vectors that falls into `00BF6713` (the CRT range throw) |
| `0087BDC9..0087BDF3` | walk the hierarchy parent chain `+3Ch` calling `vtable[B0h]` until one returns non-zero, then call it once more |
| `0087BDF5..0087BEA4` | when `[desc+50h]` is non-null, `operator new(1ACh)` and `007135C0(obj, this, [desc+50h]->vtable[8](lod))`, where `lod = 0.95f` (`00CED9E0`) if `this->vtable[5Ch](1Bh)` and `1.0f` otherwise, and `lod` is passed through `this->vtable[190h](lod)` first |
| `0087BEA4` | `this+360h = obj` (or `0` when the allocation returned null) |

| offset | type | meaning | evidence |
| --- | --- | --- | --- |
| `desc+18h` | vector header | part descriptors: begin `+1Ch`, end `+20h`, stride `30h` | `0087BD20`, `0087BD71`, `0087BDC2` |
| `desc+48h` | float | `HP`, the unit's maximum health | `0087BCFC` |
| `desc+4Ch` | float | `Armour`, subtracted per hit | `0087BCE7` |
| `desc+50h` | ptr | source of the `1ACh` part instance; `vtable[8h](lod)` selects by detail | `0087BDFB`, `0087BE3A` |
| `unit+344h` | vector | `const PartDesc*` per part, pointing into `desc+18h` | `0087BD4F`, `0087BDBC` |
| `unit+360h` | ptr | the `1ACh` part instance `007135C0` constructs | `0087BEA4` |
| `unit+368h` | float | armour | `0087BCF4` |
| `unit+378h` | byte | set to `1` here | `0087BD19` |

The 48-byte part descriptor's own fields are `contract: unread`; nothing in this packet dereferences
one. `unit+360h` and the breakable-parts object at `unit+1018h` (docs/UNIT_DAMAGE_AND_DEATH.md) are
**not** shown to be the same object, and no evidence here says they are.

`0087BCC0` is called from `0043F7B0` and `00955420`; both are `contract: unread`.

### Detaching a part

`00934150` (`00934150..0093553A`, `__thiscall`, `RET 8`) takes `(partsObject, int index,
const float impulse[3])`. The ABI is established from `0080E440`, which pushes a stack `vec3`, then
`&vec3` and the index, calls, and cleans only `0Ch` (`ADD ESP,0xC` at `0080E46C`), so the callee
popped the two pushed arguments. Its two callers are:

| site | caller | arguments |
| --- | --- | --- |
| `0080E467` | `0080E440` `BSP_UnitInstance_DetachPart` | `[unit+1018h]`, the caller's index, `{0,0,0}` |
| inside `00935C70` | `00935C70` `BSP_UnitParts_DetachAllLiveParts` | the parts object, each live index, `{0,0,0}` |

`0080E440` itself is called only from `00821E80`. The body of `00934150` is `contract: unread`:
5,098 bytes of debris spawning, effects and physics, outside this packet's budget.

Neighbours read while establishing the above:

* `00878340` is `SetHealth(this, [this+36Ch])`, a full heal (`FLD [ECX+36Ch] / PUSH ECX / FSTP
  [ESP] / CALL 00877B90 / RET`, five instructions).
* `0080E410` calls `00878340`, then writes `1.0f` to `+9D8h` and `+9DCh` and clears the bytes
  `+9E4h` and `+9E5h`: a full condition reset. Its only caller is `00758370`.

## The repair tick

`0093CA20` is the damage-control update. It is reached from `008160E5`, inside the raw-listing
region whose enclosing Ghidra candidate is `00815F30` (`no_ghidra_function` at the site itself), and
runs five steps in order:

| order | site | callee | what |
| --- | --- | --- | --- |
| 1 | `0093CA2B` | `0093C770` | hull repair |
| 2 | `0093CA3A` | `0093C860` | subobject repair |
| 3 | `0093CA49` | `0093C520` | `contract: unread` |
| 4 | `0093CA58` | `0093C120` | `contract: unread` |
| 5 | `0093CA67` | `0093C210` | `contract: unread` |

then, when the task's `+24h` is `3` or `4`, its `+34h` and `+38h` floats are both zero and the byte
at `+44h` is clear, it logs `"Ship Repaired Fire && Leak %s Hp %6f %f"` through `004254B0`, sets
`+44h` and calls `00914100`. That literal is why the two steps are named repair rather than damage.

`0093C770` (`__thiscall(task, float dt)`, `RET 4`, `0093C770..0093C856`):

```
rate  = task->byte_45h ? (task->i_24h == 0 ? settings->f_3D4h : 1.0f) : 0.0f
rate *= (00F88C30 && 00E0C978 && [00F88C30+ACh]) ? ProductForUnit(3, task->unit) : 1.0f
unit  = task->ptr_00h
amount = task->f_28h * dt * settings->f_3B4h * unit->f_36Ch * rate
00879810(unit, amount)                       ; ApplyDamage(unit, -amount): health rises
if (unit->f_370h > unit->f_36Ch) 00877B90(unit, unit->f_36Ch)
```

`settings` is `00424C40()`, the game-settings singleton; `1.0f` is `00D7A24C`. The repair is a
fraction of **maximum** health per second, so a large ship repairs faster in absolute terms.

`0093C860` (`__thiscall(task, float dt)`, `0093C860..0093CA11`, SEH frame) does the same for the
subobject list: it walks `unit->+48h` with the next pointer at `+44h` (the same list
docs/UNIT_STATE_MESSAGE.md walks), keeps children for which `vtable[5Ch](4)` is true,
`vtable[5Ch](0Fh)` is false and the byte at `+378h` is set, and for each one applies
`settings->f_3B8h * dt * child->f_36Ch * rate` with `rate` built from `settings->f_3D8h` when
`task->i_24h == 2`. When a child comes back to full it calls `00877B90(child, child->f_36Ch)`, and
when the child's `+5Dh` byte is set it builds the string `"destroyed"` and dispatches
`child->vtable[19Ch](&out, 0)`, releasing the returned block through the sized-storage pool. That
last branch is `contract: unread` past the dispatch.

The repair task record, read only from its two consumers:

| offset | type | meaning |
| --- | --- | --- |
| `+00h` | ptr | the unit instance being repaired |
| `+24h` | int | task kind; `0` selects `settings->f_3D4h` in the hull step, `2` selects `settings->f_3D8h` in the subobject step, `3`/`4` allow the completion log |
| `+28h` | float | hull repair rate, a fraction of maximum health per second |
| `+34h`, `+38h` | float | fire and leak amounts the completion test requires to be zero |
| `+44h` | byte | "already reported repaired" flag |
| `+45h` | byte | hull repair enabled; when clear the hull rate is `0.0f` |

## Coverage

| routine | coverage |
| --- | --- |
| `00879810` | complete |
| `00470510`, `004705C0` | complete |
| `008777D0` | complete |
| `0093C770` | complete |
| `0093C860` | partial: the `"destroyed"` dispatch at `0093C9AE..0093C9F5` is summarised, not modelled |
| `0087BCC0` | partial: `0087BCC0..0087BEA4` read; `0087BEAA..0087BF73` (the `+C0h` property-bag tail into `00876EC0` and `008F2260`) unread |
| `007BBCF0` | analyzed, not reconstructed |
| `0093CA20` | analyzed, not reconstructed |
| `00934150` | `contract: unread` (ABI only) |
| `0080E440`, `0080E410`, `00878340` | complete |
| `007470B0`, `0074CD30` | analyzed, not reconstructed |

## Host table

One row per native call site the reconstruction models as a virtual method of `UnitHitPathHost` in
`include/bsp/unit_hit_path.hpp`. Full argument detail is in `reports/unit_hit_path.json`.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `008777EF`, `00877828` | `[vt+5Ch]` | `unit_is_kind_of` | unit / class id / bool | none |
| `00877809`, `0087791F` | `[[+354h] vt+24h]` | `unit_alternate_armour` | class desc / - / float | kind `6` and `hit+0Ch < 0` |
| `0087784F`, `00877875`, `00877965`, `008779A3` | `008E6430` | `gameplay_modifier_product` | - / kind `2`, unit / float | `00E0C978` and `[00F88C30+A0h]` |
| `008778A4` | `00470510` | `hull_damage_from_record` | hit / armour scaled / float | `hit+34h != -1` |
| `008779D3` | `004705C0` | `part_damage_from_record` | hit / armour scaled, index / float | per part hit |
| `008778D2`, `00877A37` | `[vt+1ACh]` | `unit_add_damage` | unit / float / void | value `> 0` |
| `00879824` | `00879070` | `unit_apply_damage` | unit / negated float / void | none |
| `0093C787`, `0093C7F6` | `00424C40` | `game_settings` | - / - / ptr | none |
| `0093C7C5`, `0093C8C5` | `008E6430` | `gameplay_modifier_product` | - / kind `3`, unit / float | `00F88C30` non-null |
| `0093C820`, `0093C957` | `00879810` | `unit_apply_health_delta` | unit / float / void | none |
| `0093C844`, `0093C99A` | `00877B90` | `unit_set_health` | unit / float / void | health above max |
| `0093C900`, `0093C913` | `[vt+5Ch]` | `unit_is_kind_of` | child / class id / bool | per child |
| `0087BD4F` | `0087B460` | `resize_part_table` | vector / count, `0` / void | none |
| `0087BE1C`, `0087BE5F` | `00BF681B` | `allocate_part_instance` | - / `1ACh` / ptr | `[desc+50h]` non-null |
| `0087BE4B`, `0087BE8E` | `[vt+190h]` | `unit_detail_for_lod` | unit / float / handle | none |
| `0087BE52`, `0087BE95` | `[[desc+50h] vt+8]` | `part_set_for_detail` | part set / handle / ptr | none |
| `0087BE58`, `0087BE9B` | `007135C0` | `construct_part_instance` | new object / unit, part set / void | none |
| `0080E467` | `00934150` | `detach_part` | parts object / index, impulse / void | none |
| `0087834A` | `00877B90` | `unit_set_health` | unit / max health / void | none |

## Open questions

* The producer of the hit record is unread. Who fills `+14h`, `+24h`, `+28h`, `+3Ch` and `+40h`,
  and which weapon or collision system owns the `+4h` descriptor, is the next packet.
* `007BBCF0`'s vtable slot and the routine that dispatches it are unread, so the chain above starts
  one level below the projectile impact.
* `unit+360h` (the `1ACh` part instance) versus `unit+1018h` (the breakable-parts object with the
  per-part health vector at `+310h`): the relationship is unestablished.
* The 48-byte part descriptor in `desc+18h` has no attributed fields.
* The game-settings offsets `+3B4h`, `+3B8h`, `+3D4h`, `+3D8h` are unattributed; they are the four
  scalars the repair tick multiplies by.
* Nine other stores to `+36Ch` exist in the program and were not attributed to a class.
