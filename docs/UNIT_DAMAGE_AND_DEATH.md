# Unit damage, invincibility and death

Addresses: 0088E000 008AC5C0 00891B20 00897A50 00897CB0 0088E1B0 0095DA00 0087D730 00879070
00877B90 00827A90 00958A30 0042ED80 008110F0 00926C80 00926D90 0077D1A0 007ED380 00742210
00876260 00923BE0 00935C70 0080E490 009273A0

Packet `cc2_unit_damage`, read-only in Ghidra. Every descriptive name below is a hypothesis, not a
recovered symbol. The six mission Lua binding handlers of the table `00E0B7B8` share the prologue of
docs/LUA_BINDING_CORE.md and take their entity argument through `00888AA0`
(docs/LUA_BINDING_ENTITY.md); only what each one does after the handle is resolved is new here.

## What the six bindings do

| Lua name | handler | body | native effect |
| --- | --- | --- | --- |
| `AddDamage(entity, amount)` | `0088E000` | `0088E000-0088E1A3` | `entity->vtable[1ACh](float amount)` at `0088E15B` |
| `Kill(entity [, hard])` | `008AC5C0` | `008AC5C0-008AC7A5` | one of three kills, chosen by two class-id tests |
| `Sink(entity, corner)` | `00891B20` | `00891B20-00891D4B` | `008110F0(entity, pivot)` at `00891CFD` |
| `SetInvincible(entity, value)` | `00897A50` | `00897A50-00897CAE` | `entity->vtable[F4h](float)` at `00897C63` |
| `IsInvincible(entity)` | `00897CB0` | `00897CB0-00897E30` | pushes `[entity+150h] > 0.0f`, read at `00897DC6` |
| `ExplodeToParts(entity)` | `0088E1B0` | `0088E1B0-0088E31A` | `00935C70([entity+1018h])` at `0088E2CF` |

`AddDamage` reads exactly two Lua arguments: the entity table and one number. There is no damage
type, no source and no part index anywhere on this path, and `0095DA00` is `__thiscall(this, float)`
with `RET 4`, so the game has no armour or per-part damage model reachable from this binding.

`SetInvincible` accepts either a boolean or a number (`00897A50` branches on
`BSP_LuaObject_IsBoolean`): `true` becomes `1.0f` (`00D7A24C`), `false` becomes `0.0f`, and a number
is passed through unchanged. The value is a *fraction of maximum health*, not a flag.

`Kill` reads an optional second boolean and turns it into the cause code `1` (absent or false) or
`2` (true). It then calls `entity->vtable[5Ch]` with `18h` and, failing that, with `1Ah`
(`008AC729`, `008AC740`) to pick between `007ED380` (kill a fixed member array), `00742210` (kill a
member vector) and `00926D90` (kill this entity).

## Health and the fields the damage path touches

The offsets below are on the entity object whose vtable is the root `00CE6290`; for a ship that is
the `0x1188` unit instance of docs/UNIT_INSTANCE_LAYOUT.md with vtable `00CFC3D0`. The flag bytes
`+5Ch`..`+60h` already have constants in `include/bsp/lua_binding_entity_lookup.hpp`; this packet
adds their writers.

| offset | type | meaning | written by | read by |
| --- | --- | --- | --- | --- |
| `+150h` | float | invincibility fraction of max health | `0042ED80` at `0042ED8F` | `00879070` `008790D0`, `008110F0` `008110F9`, `00897CB0` `00897DC6` |
| `+2E8h` | int | set to `-1` when health comes back within `1.0` of max | `00877B90` at `00877C20` | no reader found |
| `+36Ch` | float | maximum health | no writer found (see open questions) | `00877B90` `00877BD9`/`00877C0C`, `00879070` `008790F7`, `00876260` `00876267` |
| `+370h` | float | current health; `<= 0` is dead | `00877B90` at `00877C04`, nowhere else | `00879070`, `00827A90` `00827AB0`, `00958A30` `00958A58`/`00958DAA`, `00876260` |
| `+374h` | int | last replicated health byte | `00877B90` at `00877C84` | `00877B90` `00877C77` |
| `+5Ch` | byte | active (`kEntityFlagActive`), cleared in the drain | `009273A0` at `009274DA` | docs/LUA_BINDING_ENTITY.md |
| `+5Dh` | byte | released (`kEntityFlagReleased`) | `009273A0` at `009274CE` | `008110F0` `008110F3`, `00923BE0` `00923BE4` |
| `+5Eh` | byte | release requested (`kEntityFlagReleaseRequested`) | `009273A0` at `009274D2` | `00879070` `008790A1` |
| `+5Fh` | byte | killed flag, makes `Kill` idempotent | `00926D90`, `009273A0` at `009274D6` | `00926D90` |
| `+60h` | byte | destroyed flag (`kEntityFlagUnreadGate`), makes `Destroy` idempotent | `00926C80` | `00926C80`, `00926D90` |
| `+70h` | int | death cause, inherited from the parent when it is already dying | `00926C80`, `00926D90` | `0077D1A0` `0077D1DB` |
| `+538h` | ptr | vehicle class descriptor: `+A0h`/`+A4h` sink half-extents, `+B0h` death-message selector | docs/UNIT_INSTANCE_LAYOUT.md | `00891B20` `00891CAF`, `00827A90` `00827AC4` |
| `+71Ch` | ptr | owning unit; non-null only on a damageable subobject | not found | `0095DA00` `0095DA03`, `00958A30` `00958A4B` |
| `+728h` | float | subobject damage lockout timer | `00958A30` | `0095DA00` `0095DA0C` |
| `+1018h` | ptr | breakable-parts object, per-part health vector at its `+310h`/`+314h` | not found | `0080E490` |

`00876260` (`00CFC3D0+110h`, no Ghidra function, read from the raw bytes) is the whole health
fraction: `FLD [ecx+370h] / FDIV [ecx+36Ch] / RET`. `BSP_UnitInstance_GetHealth` `00923BE0` returns
`0.0` when `+5Dh` is set and otherwise dispatches that slot.

## The damage rule

`0095DA00` (`__thiscall(this, float)`, `RET 4`) refuses the call outright when
`[this+71Ch] != 0 && [this+728h] > 0.0f`, that is, on a subobject whose lockout is still running.
Two difficulty multipliers are then applied, both only outside multiplayer
(`[00E188A8+1FE4h] == 0`): `0095DA00` multiplies by `config->[50h][level]` when
`BSP_UnitInstance_IsLocalPlayerRole(0)` is true, and `0087D730` multiplies by `config->[20h][level]`
when the entity is the current player's unit (`[this+54h]` equals `[currentPlayer+28h]`). The level
is `[00E188A8+6ACh]`, or the constant `2` outside a campaign (`kNonCampaignReportedDifficulty`).

`00879070` holds the rule itself (`008790A1`-`0087914B`, x87, read from the listing):

1. Return when the session mode is `2` (a client does not apply damage locally) or when the
   entity's `+5Eh` byte is set.
2. If `amount <= 0` skip straight to step 5. A negative amount is a repair and is subject to neither
   the dead test nor the invincibility floor.
3. Return when `[this+370h] <= 0.0f`: a dead unit takes no further damage.
4. When `[this+150h] > 0.0f`, compute `floor = (1.0f - (1.0f - inv)) * [this+36Ch]` exactly as
   written (`008790E3`-`008790FD`: `FLD1 / FLD ST0 / FSUBRP ST2,ST0 / FXCH / FSTP / FSUB / FMUL`, so
   the intermediate `1.0f - inv` is rounded through a float store). Return when `health <= floor`;
   otherwise clamp `amount` to `health - floor`.
5. Call `00877B90(this, health - amount)`.

So `SetInvincible(unit, 1.0)` makes the floor equal maximum health and every damage call returns at
step 4, and `SetInvincible(unit, 0.25)` lets the unit be worn down to a quarter of its maximum and
no further. `IsInvincible` reports `inv > 0`, so a fractional value reads back as invincible.

When the telemetry flag `00F8A0C4` is non-zero and the health actually changed, `00879070` records
a `"damage"` or `"repair"` event with the old and new fractions through `00986B00` (a contract).

`00877B90` (`__thiscall(this, float)`, `RET 4`) is the only writer of `+370h`. It returns when the
value is unchanged, clamps to `[0, [this+36Ch]]`, stores, sets `+2E8h` to `-1` when the stored value
is above `max - 1.0` (the operand at `00877C12` is the **double** `1.0` at `00D7A210`), then:

- unless the session mode is `2`, calls `this->vtable[1B0h]()` at `00877C40`;
- when the session mode is `1` (host), computes `clamp((int)(fraction * 256.0), 0, 255)` from
  `BSP_UnitInstance_GetHealth` and the double `256.0` at `00D0DEE0`, and when that byte differs from
  `+374h` stores it and sends a message built by `00876D30` on channel `4`.

## The death sequence

| order | site | what happens |
| --- | --- | --- |
| 1 | `0087914B` | `00879070` calls the setter with `health - amount` |
| 2 | `00877C04` | `00877B90` clamps and stores `+370h` |
| 3 | `00877C40` | `this->vtable[1B0h]()`, unless the session mode is `2`; base `0042B140`, ship `00827A90` |
| 4 | `00827AAB` | `00827A90` calls `00958A30` **before** testing the health, then returns if `[this+370h] > 0` |
| 5 | `00958DBE` | on the `[this+71Ch] == 0` branch with `health <= 0`, `00958A30` calls `this->vtable[70h](1)` |
| 6 | `0077D243` | the ship's `vtable[70h]` override `0077D1A0` broadcasts a destroy message (id `4Eh`) carrying `+70h`, then calls the base |
| 7 | `00926D4E` | `00926C80` sets `+60h`, fills `+70h` from the parent or with `1`, recurses over the children and queues the entity on the `std::list` at `00F899A8` |
| 8 | `00827B77` | back in `00827A90`, the death session message goes out on channel `7`: id `9Ah` when `[[this+538h]+B0h] < 0.0f`, otherwise a breakup message from `00761310` behind a random roll against `[00424C40()+65Ch]` |
| 9 | `0092747F` | on a later frame `BSP_EntityEventQueues_FlushPending` `009273A0` dispatches `vtable[74h]` for every entity on the destroy list |
| 10 | `0092751F` | the same flush sets `+5Dh`/`+5Eh`/`+5Fh` to 1 and `+5Ch` to 0 and dispatches `vtable[80h]` for every entity on the kill list `00F899B4` |

Step 10 is the answer to "who dispatches slot 32": `00927515`-`0092751F` is
`MOV EDX,[ESI] / MOV EAX,[EDX+80h] / MOV ECX,ESI / CALL EAX`, inside the Ghidra body
`009273A0-009275D1` (docs/FIXED_STEP_FANOUT.md), walking the container copied from `00F899B4` at
`009273E4`. It is the only dispatch of that slot found in this packet, and it takes no stack
argument. Base `00928C80`, unit adjustor thunk `00951FB0`, player-unit override `00779AF0`.

The two queues are distinct and the damage path only fills the first one. `00926C80` (Destroy)
queues on `00F899A8` and is what a unit killed by damage reaches; `00926D90` (Kill) queues on
`00F899B4` **and** calls `vtable[70h]` on the way, so the Lua `Kill` binding reaches both lists
while damage reaches only the destroy list. On this reading a unit that is shot to death never
runs the on-killed hook, and the Lua self table's `Ptr` is only released for an explicit `Kill`.
This is stated as a finding, not a certainty: a second dispatch of `vtable[80h]` elsewhere in the
executable would change it, and no program-wide scan for `CALL [reg+80h]` was run.

`00926D90` (`__thiscall(this, int cause)`, `RET 4`) returns when `+5Fh` is already set; otherwise it
sets `+5Fh`, calls `vtable[70h](1)` with `+70h = cause` when `+60h` is still clear, remaps the cause
`7` to `2`, recurses over the child list (`+48h` head, `+44h` next) and queues the entity.
`00926C80` (`__thiscall(this, int recurse)`, `RET 4`) returns when `+60h` is already set; otherwise
it sets `+60h`, takes `+70h` from the hierarchy parent at `+3Ch` when that parent is itself
destroyed and `1` otherwise, and, when `recurse` is true, walks the children calling `vtable[78h]`
as a predicate and `vtable[70h](1)` on each child that passes.

## The invincibility gate

`0042ED80` (`00CE6290+F4h` and `00CFC3D0+F4h`, so the unit class does not override it) stores the
float at `+150h` and then walks the child list calling each child's `vtable[F4h]` with the same
value, so invincibility propagates to turrets and parts. It is read in three places, and each gate
sits at a different depth:

| gate | site | effect |
| --- | --- | --- |
| damage | `008790D0` in `00879070` | clamps the amount so health cannot cross `inv * max`; a repair is unaffected |
| sink | `008110F9` in `008110F0` | `COMISS [this+150h], 0.0 / JA` returns immediately, so **any** non-zero value blocks a sink |
| query | `00897DC6` in `00897CB0` | `IsInvincible` is `inv > 0.0f` |

Nothing gates `Kill` on `+150h`: `008AC5C0` goes straight to `00926D90`, so `Kill` beats
`SetInvincible`. `ExplodeToParts` is not gated either.

## Sink

`00891B20` turns the Lua integer `n` into a pivot offset from the vehicle class descriptor's
half-extents (`_DAT_00D7A280` is the double `0.5`):

```
x = [desc+A4h] * ((n & 1) ? +1 : -1) * 0.5
y = 0
z = [desc+A0h] * (n < 2 ? 1 : (n < 4 ? 0 : -1)) * 0.5
```

so `n` selects one of six positions on the hull: bow-left, bow-right, midships-left, midships-right,
stern-left, stern-right. The vector is then passed to `008110F0`, **which never reads it**: the body
is `008110F0-00811144`, `RET 4`, and no instruction touches `[ESP+8]`. What `008110F0` actually does
is refuse when `+5Dh` is set or `+150h > 0`, call `vtable[70h](1)`, zero `+828h` and `+82Ch`, and
release the object at `+740h` through its `vtable[10h]`.

## ExplodeToParts

`0080E490` is `MOV EAX,[ECX+1018h] / RET`. `00935C70` walks the float vector `[obj+310h,obj+314h)`,
and for every entry greater than `0.0f` stores `DAT_00D19620` (`C61C4000h`, that is `-10000.0f`) and
calls `00934150(obj, index, &{0,0,0})`. So every part that still has health is given a large negative
health and detached with a zero impulse. `00934150` is a contract.

## Host table

One row per native call site the reconstruction models as a virtual method of
`UnitDamageHost` in `include/bsp/unit_damage.hpp`. Full argument detail is in
`reports/unit_damage.json`.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `0088E0FF` | `00888AA0` | `entity_from_lua_table` | LuaObject / - / entity | none |
| `0088E15B` | `[vt+1ACh]` | `dispatch_add_damage` | entity / float / void | none |
| `0095DA2D` | `00927F30` | `is_local_player_role` | - / 0 / bool | session mode 0 |
| `0095DA51`, `0087D768` | `00432650` | `global_config` | - / - / config | difficulty scaling |
| `0095DA8D` | `0087D730` | (inlined in the model) | entity / float / void | none |
| `0087914B` | `00877B90` | (inlined in the model) | entity / float / void | none |
| `0087919A` | `0041E870` | `telemetry_event` | string / literal / - | `00F8A0C4` |
| `008791EB` | `00986B00` | `telemetry_event` | record / entity, fractions / void | `00F8A0C4` |
| `00877C53` | `00923BE0` | `health_fraction` | entity / - / float | host only |
| `00877C8A` | `00876D30` | `send_health_message` | message / byte / message | byte changed |
| `00877C9E` | `0077C2A0` | `send_health_message` | entity / message, 4, 0 / void | byte changed |
| `00877C40` | `[vt+1B0h]` | `dispatch_health_changed` | entity / - / void | session mode != 2 |
| `00827AAB` | `00958A30` | (inlined in the model) | entity / - / void | none |
| `00827AE5`, `00827B63`, `00827B77` | `0075B430`, `00761310`, `0077C2A0` | `send_death_message` | message, entity / id, channel 7 / void | health <= 0 |
| `00827B1A`, `00827B29`, `00827B33`, `00827B4D` | `0092BEA0`, `0092BE90`, `00424C40`, `00BD2F10` | `breakup_roll` | - / - / bool | `[desc+B0h] >= 0` |
| `00958DBE`, `00811111`, `00926E05` | `[vt+70h]` | `dispatch_destroy` | entity / int recurse / void | see each caller |
| `0092747F` | `[vt+74h]` | `dispatch_pending_destroy` | entity / - / void | drain |
| `0092751F` | `[vt+80h]` | `dispatch_on_killed` | entity / - / void | drain |
| `00897C63`, `0042EDB4` | `[vt+F4h]` | `dispatch_set_invincible` | entity or child / float / void | none |
| `00811135` | `[[+740h]+10h]` | `release_sink_attachment` | object / - / void | `+740h` non-null |
| `0077D1E4`, `0077D21E`, `0077D236` | `0075B430`, `0077C7B0`, `0077C980` | `send_destroy_message` | message, entity / cause, recurse / void | `00E0AF20` and mode 0 or 1 |
| `0077D253` | `00986480` | `telemetry_event` | - / entity / void | `00F8A0C4` |
| `00935D16` | `00934150` | `detach_part` | parts / index, zero vector / void | part health > 0 |
| `008AC729` | `[vt+5Ch]` | `class_id_test` | entity / int id / bool | none |
| `007ED3A3`, `00742275`, `008AC756`, `00926E19` | `00926D90` | `dispatch_kill` | member or child / int cause / void | see each caller |

Contracts named but not read: `00986B00`, `00986480` (telemetry), `00876D30`, `0075B430`,
`0077C2A0`, `0077C7B0`, `0077C980`, `00761310` (session messages), `00934150` (part detach),
`0092BEA0`, `0092BE90`, `00424C40`, `00BD2F10` (the breakup roll), `00924B10`, `009248D0`
(the entity lock and list node allocation).

## Coverage and open questions

| routine | coverage |
| --- | --- |
| `0088E000`, `008AC5C0`, `00891B20`, `00897A50`, `00897CB0`, `0088E1B0` | complete |
| `0095DA00`, `0087D730`, `00879070`, `00877B90`, `0042ED80`, `008110F0`, `00926C80`, `00926D90`, `007ED380`, `00742210`, `00876260`, `00935C70`, `0080E490` | complete |
| `00827A90` | complete for the branch structure; the two message payloads are contracts |
| `00958A30` | partial: the root-unit branch `00958DA7-00958DCF` is read in full, the subobject branch `00958A58-00958DA6` (`vtable[1B4h]`, `[19Ch]`, `[204h]`, the class ids `45h`/`46h`/`1Bh` and `[desc+18Ch]`/`[desc+190h]`) is summarised only |
| `0077D1A0` | complete, read from the raw bytes with capstone: no Ghidra function starts there, `0077CE60` ends at `0077D19F` and the body runs `0077D1A0-0077D26A` |

- No writer of `+36Ch` (maximum health) was found, so the ceiling's producer is unread and the
  spawn-time initialisation of health is outside this packet.
- The Lua self table's `Dead` field still has no writer. `00928C80` sets `Ptr` and `LastPosition`
  and does not touch `Dead`; nothing on the destroy path does either.
- No run-time evidence supports any claim here: the `bsp_game.exe` harness does not load a mission,
  so the damage path is never reached in a run. Everything is from the live listing.
- `0077D1A0`'s message id `4Eh` and the senders `0077C7B0`/`0077C980` were not followed.

## Correction from docs/ENTITY_EVENT_QUEUES.md (packet cc2_entity_event_queues)

The caveat above, that a second dispatch of `vtable[80h]` elsewhere could still run on-killed
for a unit shot to death, is closed: the only other `vtable[80h]` site is `009263C0`, which
has no reference of any kind in the image. Damage reaches `0077D1A0`, the sole caller of
Destroy, so a damage death lands on the destroy list `00F899A8` only (drained through
`vtable[74h]`); an explicit Kill additionally calls `vtable[70h]`, which for a unit is that same
`0077D1A0`, so a Kill lands on both lists and is the only path that runs on-killed slot 32
(`009273A0`, dispatched over the copied Kill list).
