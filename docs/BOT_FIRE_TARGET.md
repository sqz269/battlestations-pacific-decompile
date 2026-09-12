# How an AI unit picks and engages a target

Addresses: `009F5DA0` `009F6A20` `009F69C0` `009F65E0` `009F65B0` `009F6520` `009F5D30`
`009F5B70` `009F59F0` `009F52F0` `009F5610` `009F6170` `00D21B40` `00D21B48` `008FFA20`
`008FFF20` `00902920` `009030C0` `006DF520` `00959C20` `008FDAF0` `008FEF40` `008438B0`
`00744A90` `004643A0`

Two separate mechanisms decide what an AI-controlled unit shoots at. The **weapon
director's automatic target selector** chooses the entity, once a second, and hands it to
`00835860` `BSP_WeaponDirector_SetFireTarget`. The **gun-side bot ticks** turn whatever
target is current into a pair of gun angles and offer them to `0085ABA0`
`BSP_TurningGun_SetTargetAngles`, which `docs/GUN_AIMING.md` establishes accepts a pair
only when a platform firing window contains it. The six callers that packet left unread
are read here.

Every descriptive name below is a hypothesis, not a recovered symbol.

## 1. The selector is the director's `+38h` subobject

`include/bsp/weapon_director.hpp` already records `director+38h` as a 40h-byte subobject
built by `009F6A20` at `0083676A`. That subobject is the automatic target selector.

`009F6A20` is `__thiscall(this, void* director)`, body `009F6A20..009F6ABD`:

| write | evidence | meaning |
| --- | --- | --- |
| `this+4h` = director | `009F6A42` | the owner |
| `[this]` = `00D21B48` | `009F6A45` | the derived vtable |
| `this+8h` = `[director+24Ch]` | `009F6A4B`, `009F6A51` | the unit entity |
| `009F69C0(this+10h, [director+24Ch])` | `009F6A5B` | build the search state |
| `this+34h` = `1.0f` | `00D7A24C` | the think interval |
| `this+38h` = `-rand(0, 1.0f)` | `00BD2F10` at `009F6A67` | a random negative phase, so the first tick always runs and the 1 Hz thinks of different units fall on different frames |
| `this+0Ch` = director | the second store of `param_2` | the director again |
| `this+3Ch` = `FLT_MAX` | `00D7A248` | the retained score |

**The vtable.** `009F5DA0` has no caller in the call graph, so it is reached through a
slot. Nothing in `.text` holds a 4-aligned pointer to `009F5DA0` except `00D21B4C`, and
nothing at all points at `00D21B4C`, so `009F5DA0` is not the head of a table. The
destructor `009F6170` settles the pair: it writes `00D21B48` over `[this]`, frees the
vector at `this+14h`, clears `this+14h/18h/1Ch` and then writes `00D21B40`. So `00D21B40`
is the base vtable (`009F5290`, then `__purecall` `00BF698E`) and `00D21B48` the derived
one (`009F6170` the scalar deleting destructor, `009F5DA0` the tick at slot `+4h`). The
interface has two slots: destroy, and `tick(float dt)`.

This is **not** the AI group of `docs/LUA_BINDING_AI.md` and not the unit controller. It
is a per-director object, so every unit that owns a weapon director owns one.

### Layout, `director+38h`, 40h bytes

| offset | meaning | evidence |
| --- | --- | --- |
| `+00h` | vtable `00D21B48` | `009F6A45` |
| `+04h` | the owning director | `009F6A42` |
| `+08h` | the unit entity, `director+24Ch` | `009F6A51` |
| `+0Ch` | the director again, the ECX of every director call in the tick | `009F5DC1`, `009F5F1B` |
| `+10h` | the search state base, the ECX of `009F5D30` | `009F5E7C` |
| `+14h`/`+18h`/`+1Ch` | `std::vector` of 8-byte priority entries | freed and cleared by `009F6170` |
| `+20h` | the max engagement range | `009F6611`-`009F661D` writes `state+10h` |
| `+24h` | the owner entity the distance is measured from | `009F69F5` writes `state+14h` |
| `+28h` | the best candidate of the last scan | `009F5CF1` |
| `+2Ch` | that candidate's score | `009F5CF4` |
| `+30h` | issue-an-attackmove byte, from `009F5860` | `009F5D02`, read at `009F5ED9` |
| `+34h` | think interval, `1.0f` | `009F5DB9` |
| `+38h` | think countdown | `009F5DA6` |
| `+3Ch` | the retained score of the target in use | `009F5E77`, `009F5EB6` |

## 2. The selection rule

`009F5DA0` is `__thiscall(this, float dt)`, `RET 4`, body `009F5DA0..009F5F45`. In order:

| # | gate or step | evidence | constant |
| --- | --- | --- | --- |
| 1 | `dt < countdown` -> `countdown -= dt`, return | `009F5DB5`, `009F5DF7` | - |
| 2 | else `countdown += interval - dt` | `009F5DB9` | interval `1.0f` |
| 3 | `007788B0(unit)` says the controller belongs to another: if `[director+54h]` is null return; if it is the `follow` command singleton return; else `0077C980(unit, 0)` and return | `009F5DC4`-`009F5DEB` | `00E08F60` = `follow`, index 14 of the `00E08EF8 + 8*(k-1)` table in `docs/SCENE_COMMAND_TYPES.md` |
| 4 | `[unit+184h]` set -> return | `009F5E06` | - |
| 5 | `009F5610` false -> if `[director+30h] == 2` tail-call `0071D9E0(director, 2)`; return | `009F5E15`, `009F5F2F` | state `2` |
| 6 | `EBX = director->vtable[2Ch]()`, the director's current target | `009F5E2C` | - |
| 7 | if that target exists **and** `[director+23Ch]` is set, build a command target with `00465080(target, 0.0f)` and ask `0071D6D0(attackmove, ct)`; if it accepts, keep the current target and jump to step 12 | `009F5E40`-`009F5E64` | `00E08F78` = `attackmove`, index 17 |
| 8 | if `[director+30h] != 2`, reset the retained score to `FLT_MAX` | `009F5E6F` | `00D7A248` |
| 9 | `009F5D30(this+10h)` scans the party list and writes `+28h`/`+2Ch`/`+30h` | `009F5E7F` | - |
| 10 | no candidate -> return | `009F5E89` | - |
| 11 | `009F52F0(newScore, retainedScore)` must hold, then `retained = new` | `009F5EA7`, `009F5EB6` | `new < retained * 0.8` |
| 12 | if the chosen entity equals `00521EA0(director+18Ch)`, the director's current command object, return | `009F5EC4`, `009F5ECB` | - |
| 13 | `0071DF70(director)` must allow a new target | `009F5ED0` | - |
| 14 | if `+30h` is set, `0071D980(attackmove, 00465080(chosen, 0.0f))` | `009F5EDF`-`009F5EF8` | - |
| 15 | `00835860(chosen, force = 0)` unless the director both reports a current target and has `[director+23Ch]` set | `009F5EFD`-`009F5F21` | - |

`009F5610`, `__fastcall(this) -> bool`: `[director+3Dh]` must be set (the `allowMove`
byte of `docs/WEAPON_DIRECTOR.md`, not `allowFire`), and when `[director+54h]` is
non-null its `vtable[0Ch]()` must not return `1` or `2`.

### The party scan, `009F5D30`

`__fastcall(searchState)`, body `009F5D30..009F5D9D`. It calls `008053C0`
`BSP_Recon_EnsureSlot` with `ECX = [[state+14h]+54h]`, the owner's party index, clears
`state+18h`/`+1Ch`, and walks the intrusive list at `slot+0DE8h` (`next` at `+4h`,
payload at `+8h`, the entity at `payload+4h`). A list element is scored when
`entity->vtable[140h]()` returns null **or** the object it returns has a clear byte at
`+1D4h`; otherwise it is skipped.

### The per-candidate score, `009F5B70`

`__thiscall(searchState, entity* candidate)`, `RET 4`, body `009F5B70..009F5D2x`.

| # | gate | evidence |
| --- | --- | --- |
| 1 | `candidate->IsKindOf(6)` must hold | `009F5B81` |
| 2 | refresh the candidate's world pose when `[cand+0C8h]` is clear | `009F5B8F` |
| 3 | skip the whole candidate when `0071C4F0(cand+0FCh)` **and** `00811F50(cand, 0)` both hold | `009F5BB4`-`009F5BC8` |
| 4 | walk the priority vector; when `[cand+5Dh]` is set every entry fails and the candidate is dropped | `009F5BFA` |
| 5 | the first entry whose `entry+4h` kind `candidate->IsKindOf` accepts decides the tier; the 1-based counter at the reused parameter slot is the tier index | `009F5C16`, `009F5C1C` |
| 6 | distance = `0042B2F0(ownerPos - candPos)`; reject when `state+10h <= distance` | `009F5C9B` |
| 7 | `score = (count - index) * 10000.0 - distance`, with `count` the entry count and `index` the 1-based counter; `FSUBRP` at `009F5CC4` fixes the sign | `009F5CB2`-`009F5CC4`, `00CE4BD8` = `10000.0` |
| 8 | reject when `score <= state+1Ch`; the running best starts at `0.0f`, so a negative score never wins and the bottom tier is unreachable | `009F5CD1`, `009F5D49` |
| 9 | `009F59F0(state, owner, cand)` must accept | `009F5CDE` |
| 10 | accept: `state+18h` = candidate, `state+1Ch` = score, `state+20h` = the byte `009F5860` returns | `009F5CF1`-`009F5D02` |

The score is a **priority tier worth 10000 metres, minus the distance**: the higher tier
always wins, and inside a tier the nearer candidate wins.

`009F59F0` returns true unless the candidate satisfies `IsKindOf(8)` and `00852820` is
false; in that case the owner's `+48h` child list (next at `+44h`) must contain a node
with `IsKindOf(20h)` whose class at `[node+3F4h]+80h` is `8` or `9`. `n(20h)` is the
weapon-device test of `docs/UNIT_WEAPON_DEVICES.md`, so this is "you need the right
weapon to engage that kind of target".

### The priority list, `009F65E0`

`009F65E0` asks the owner entity `IsKindOf` in a fixed order and the first hit picks both
the range and the list. Entries are pushed front to back, so entry 0 is the top tier.
Kinds are the raw ids; what each id names is **not** established by this packet.

| owner `IsKindOf` | range | priority list, top tier first | flags clear on |
| --- | --- | --- | --- |
| `0Eh` (`009F65F4`) | `2500.0` (`00D20278`) | `8`, `0Eh`, `7`, `0Bh`, `0Ah`, `9`, `0Dh` | none |
| `7` (`009F66CD`) | `3500.0` (`00D04698`) | `9`, `8`, `0Dh`, `7`, `0Ah`, `0Bh`, `0Eh` | none |
| `0Ah` (`009F6762`) | `3000.0` (`00CFA424`) | `7`, `0Ah`, `9`, `8`, `0Dh`, `0Bh`, `0Eh` | none |
| `0Dh` (`009F6837`) | `3500.0` | `0Dh`, `9`, `0Ah`, `7`, `8`, `0Bh`, `0Eh` | `8` (`009F68C1`), `0Eh` (`009F68F2`) |
| `8` (`009F690E`) | `3000.0` | `9`, `0Bh`, `0Dh`, `0Ah`, `7`, `8`, `0Eh` | `8` (`009F6961`), `0Eh` (`009F696B`) |
| `0Bh`, `9` or `0Ch` (`009F6982`) | `0.0` | empty | - |
| anything else | not written, `0.0` from `009F69EB` | empty | - |

An owner with an empty list or a zero range can never auto-select: gate 6 of the scorer
rejects every distance. The flag byte at `entry+0h` is written by every push and is read
by nothing this packet found.

## 3. The per-gun aim-point rule

Every AI gun-side tick computes a pair of angles in the gun's local frame and offers it to
`0085ABA0`; the boolean `0085ABA0` returns is what gates the trigger. The shared converter
is `008FDAF0`: transform the world direction by the gun's derived affine inverse
(`00414E10`), take the length, let `00521370` turn the local direction into a pair, then
negate the horizontal half against `-0.0f` (`00D7A208`). That is the same convention
`0085B980` `BSP_TurningGun_AimAtWorldDirection` uses.

| routine | body | aim point | coverage |
| --- | --- | --- | --- |
| `008FFA20` | `008FFA20..008FFF1F` | The command target's pose origin `+0CCh`, transformed; a lead point from `gun->vtable[100h](out, spread, 00F87574, p0..p3)` with the per-index block `00E199A0 + 24h*[bot+34h]`; converted by `008FDAF0`; then a random error on each angle. The pair is **cached** in `+70h`/`+74h` and recomputed only when the countdown `+6Ch` expires, the next countdown being `rand([+88h], [+8Ch])`. | complete for the aim and fire path |
| `009030C0` | `009030C0..0090341D` | A ballistic solution through `00901C20` against the muzzle node `[[gun+3F8h]+34h]+50h`, taking a different branch when `[[gun+3F4h]+95h]` is set and the unit answers `IsKindOf(5)`; converted by `008FDAF0`; a vertical correction loop scales the angle by `00D7A280` while it is below `00D7A320`, offset by `00D7A2F8`, and zeroes it when the gun answers `IsKindOf(0Fh)`. `SetTargetAngles(h + dh, v)`. | partial: the ballistic solver `00901C20` and the `0090341x` jump table are unread |
| `008FFF20` | `008FFF20..0090099B` | Horizontal only: the delta is normalised, transformed and passed through `00521370`, negated, then filtered by `0085AB50(h, 00E0B588)`; `SetTargetAngles(h, 0.0f)`, skipped when the result equals `00D7A278`. | partial: only the `008FFF20..009003B0` aim arm is read |
| `00902920` | `00902920..009030B2` | A lead angle pair built with `BSP_Math_AddWrappedAngle` from a base pair and the per-bot offsets `[bot+68h]`; the vertical half is scaled by `00D7A280` when negative; guarded by `[[bot+5Ch]+3F8h]` and the fire-window floats at `+58h`/`+60h`. | partial: only the `00902B90..009030B0` aim arm is read |
| `006DF520` | `006DF520..006DFC6C` | `00955630` solves for the muzzle node `[[gun+3F8h]+34h]+50h` and returns a pair; the per-bot errors `[bot+5Ch]`/`[bot+60h]` are added with `BSP_Math_AddWrappedAngle`; the fire gate is `006DEE40(gun+480h, h, 00CF9054)`. | partial: only the `006DF9xx` aim arm is read |
| `00959C20` | `00959C20..0095A2xx` | Not a bot: `BSP_Unit_HandleMessage` routes a kind-1/2 command message here; it walks the unit's `+48h` weapon devices (`IsKindOf(20h)`, next at `+44h`), filters with `00954210(device, kind)`, checks `007F60A0 BSP_GunPlatform_AnglesInFireWindow` and then calls `0085ABA0` at `00959E06` and `0095A0F4`, setting the trigger through `device->vtable[1E8h]`. | partial: the two call sites and the device walk are read; the `0095A1C4` arm is unread |

### The trigger, `008FEF40`

`__thiscall(bot, char wantFire, float dt)`. `+59h` holds the request, `+5Ch` a debounce
timer, `+58h` the committed state. A repeated request counts the timer down (only while it
is non-negative) and commits when it reaches zero; a changed request restarts the timer
with `0.1f` (`00D17D3C`) to open fire and `0.3f` (`00CE69C8`) to cease fire. The committed
state is pushed to the gun through `gun->vtable[1E8h]`.

`008FFA20`'s request is a hysteresis on range and angle:

| state | range test | angle test |
| --- | --- | --- |
| not firing | `distance < maxRange - 40.0` (`00D7A378`) | `abs(dh) + abs(dv) < 0.10472` (`00D18390`, six degrees) |
| firing | `distance <= maxRange + 20.0` (`00CE3D88`) | `abs(dh) + abs(dv) <= 0.15708` (`00D18388`, nine degrees) |

## 4. The other `00835860` call sites

| site | caller | shape | who |
| --- | --- | --- | --- |
| `009F5F21` | `009F5DA0` | `SetFireTarget(chosen, force = 0)` | the autonomous selector |
| `008438D5`, `0084390x` | `008438B0` | from the order ordinal: `settarget` (`00E08EF8`) -> the resolved object with `force = 1`; `cleartarget` (`00E08F00`) or `clearorders` (`00E08F08`) -> null with `force = 1` | an order message |
| `00744AD5`, `00744Bxx` | `00744A90` | byte-for-byte the same three-way test | a second order path |
| `004643BD` | `004643A0` | `ECX` is the director `entity->vtable[114h]()` returns; the target is `[this+14h]` and `force = 1` | a holder that pushes its own stored entity |

Only the selector uses `force = 0`, so every scripted or ordered target overrides it; the
selector in turn refuses to overwrite a locked target at step 15.

## Routine table

| routine | coverage |
| --- | --- |
| `009F5DA0`, `009F5610`, `009F52F0`, `009F59F0`, `009F5D30`, `009F5B70`, `009F65E0`, `009F69C0`, `009F6A20`, `009F6170` | complete |
| `008FFA20`, `008FEF40`, `008FDAF0` | complete for the aim and fire path |
| `009030C0`, `008FFF20`, `00902920`, `006DF520`, `00959C20` | partial, ranges named in the table above |
| `009F6520`, `009F65B0`, `009F5860`, `00901C20`, `00955630`, `0085AB50`, `00852820`, `0071C4F0`, `00811F50` | contract: unread |

## Open questions

- What the entity kind ids `6`, `7`..`0Eh`, `20h` name. The `IsKindOf` implementations
  (`006FE530 BSP_UnitInstance_IsKindOf` and its siblings) were not read, so the priority
  lists above are ordered but unlabelled.
- The flag byte at `entry+0h` of the priority vector: written by every push in `009F65E0`,
  read by nothing in `009F5B70`.
- `009F5290`, the base vtable's first slot, has no Ghidra function and was not decoded.
- `008053C0 BSP_Recon_EnsureSlot` is recorded as returning `void`; `009F5D30` uses its
  `EAX` as the list owner, so that prototype needs a correction in its own packet.
