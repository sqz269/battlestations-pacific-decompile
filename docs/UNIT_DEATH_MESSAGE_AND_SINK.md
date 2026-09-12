# The death message receiver, the wreck handler and the sink (packet `cc2_death_sink`)

Addresses: 00814560 0092BD30 00814520 00824B60 00926390 00761310 00780090 008110F0, and
read-only context 00821E80 00827A90 00935C70 00926D90 0080F9F0 00818970 007F0030 00959450.

Worker `agent/cc2-death-sink`, 2026-09-11 UTC. Ghidra was read-only for this packet: no renames,
comments, prototypes, function creation or saves. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Every descriptive name is a hypothesis, not a recovered symbol, with
the single exception of `sinkTime`, which the executable prints itself (see below). No run-time
evidence: the `bsp_game.exe` harness does not load a mission, so nothing here was observed on a
frame (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6).

## Headline

Three findings change what `docs/UNIT_DAMAGE_AND_DEATH.md` concluded.

1. **The death message carries nothing.** `00814560` is `__thiscall(unit)` with `RET 0`. The arm at
   `00821FD6` is entered from the jump table with `ECX` still holding the original `this`, and the
   body never touches the stack. The class field `+B0h` the damage doc expected it to read is read
   by the **sender**, not the receiver.
2. **A damage death does run the on-killed hook, but only for kamikaze classes.** `00814560` calls
   `BSP_MissionEntity_Kill 00926D90` only when the class descriptor's `KamikazeDamage +510h` or
   `KamikazeBlastDamage +514h` is above zero. For every other class the receiver detaches the live
   parts and returns, so the kill list stays empty and slot `80h` never runs. The damage doc's
   "a unit that is shot to death never runs the on-killed hook" is right for an ordinary ship and
   wrong for a class with either kamikaze field set.
3. **There is no sink timer and no native settle loop.** The wreck's motion is three physics writes
   made once, in the ship's `vtable[7Ch]` override `00824B60`: the hull body's inertia is doubled and
   the angular and linear damping are set to `2.5` and `0.5`. `+828h` (`sinkTime`) is only ever
   zeroed; nothing in the image advances or reads it.

A fourth: the breakup selector is `[desc+B0h] < 100.0`, not `< 0.0f`. `00827AD0` loads the **double
`100.0` at `00D7A220`** and `00827AD6 FCOMIP ST0,ST1` compares it against `[desc+B0h]`, so the
`JBE` at `00827ADA` takes the breakup branch when `100.0 <= [desc+B0h]`.

## The route from the message kind to the receiver

`00821E80 BSP_UnitInstance_HandleMessage` indexes the byte table at `00822400` with
`msg[10h] - 4Bh` and jumps through the 27 dwords at `00822394`.

| kind | table byte | index | target | arm |
| --- | --- | --- | --- | --- |
| `70h` | `00822425` = `05` | 5 | `008223A8` -> `0082217D` | `00819A20`, the unit's own death explosion |
| `98h` breakup | `0082244D` = `12h` | 18 | `008223DC` -> `00821FBC` | `00814520` |
| `99h` detach | `0082244E` = `13h` | 19 | `008223E0` -> `00821FF0` | `0080E440` (already in docs/SESSION_MESSAGE_DISPATCH.md) |
| `9Ah` death | `0082244F` = `14h` | 20 | `008223E4` -> `00821FD6` | `00814560` |

Register provenance for the two receivers (rule 8): filtering the whole listing for `ECX` shows the
last write before the jump is `00821EA8 MOV EDI,ECX`, which does not change `ECX`; every later `ECX`
write is inside an arm, after its own `CALL`. So an arm entered from the table still holds the unit
in `ECX`, and `00814560`/`00814520` are `__thiscall(unit)` with no stack argument, confirmed by
`RET` with no immediate in both bodies.

The `70h` arm is a different message and is listed only to close the explosion doc's question: it
resolves `msg[1Ch]` through `00521E30` and passes that entity plus `&msg[20h]` to `00819A20`.

## `00814560`, the death-message receiver

`__thiscall(unit)`, `RET 0`, body `00814560-008145A7`. Coverage: complete.

| site | what happens |
| --- | --- |
| `00814563` | `ECX = [unit+1018h]`, the parts object (`0080E490` is its getter) |
| `00814569` | `00935C70 BSP_UnitParts_DetachAllLiveParts(parts)`: every part with health above zero is set to `-10000.0f` and detached with a zero impulse |
| `0081456E` | `EAX = [unit+538h]`, the vehicle class descriptor |
| `00814574`-`0081458F` | `COMISS` `[desc+510h]` then `[desc+514h]` against zero; both `<= 0` returns at `008145A6` |
| `00814591` | `00926D90 BSP_MissionEntity_Kill(unit, 1)` |
| `0081459A`-`008145A1` | `ECX = [unit+1018h]`, tail `JMP 0092BD30` |

`+510h` and `+514h` are `KamikazeDamage` and `KamikazeBlastDamage` (`docs/SHIP_CLASS_FIELDS.md`,
written by the class loader at `00831A05` and `00831A4A`, both defaulting to `0.0f`). The same pair
gates the collision damage in `docs/UNIT_CONTROLLER_UPDATE.md`, and `+514h` with `+518h`
(`KamikazeBlastRange`) is the damage/radius pair the unit death explosion uses at `00819C14`; that
closes the `contract: unread` left in `docs/EXPLOSION_RADIAL_DAMAGE.md` for those two fields.

`0092BD30`, `__thiscall(controller)`, `RET 0`, body `0092BD30-0092BD60`, coverage complete: it
takes the hull body at `[controller+2Ch]`, returns if it is null, walks the shape chain from
`00C31DC0` (the body's first shape at `B+70h`) through `[shape+208h]` and calls
`00C48020(shape, 0)` and `00C48050(shape, 0)` on each. Per `docs/UNIT_CONTROLLER_UPDATE.md` those
two set `shape+30h` and `shape+2Ch` and notify through `[00CE2218]`, so the wreck's shapes end with
both fields cleared. The sibling `0092BD70 BSP_UnitController_SetCollisionGroup` writes the same
fields, which is the reason for reading this as a collision-filter reset; the meaning of the two
fields themselves is `contract: unread`.

## The death sequence, one fixed step

Rows are `docs/FIXED_STEP_FANOUT.md` rows. A message routed by `0077C2A0` in a local session is
serialised into the array at `session+24Ch` stamped with the current tick, and
`docs/SESSION_MESSAGE_DISPATCH.md` establishes that the drain in the same step is due to deliver it,
so the whole sequence below is one fixed step when the damage lands before row 9.

| order | row | site | what happens |
| --- | --- | --- | --- |
| 1 | 6 or 14 | `00926700` | the deferred hit queue applies the damage (contract, `docs/ENTITY_EVENT_QUEUES.md`) |
| 2 | - | `0087914B`, `00877C04` | `00879070` subtracts, `00877B90` clamps and stores `+370h` |
| 3 | - | `00877C40` | `vtable[1B0h]` -> ship `00827A90` |
| 4 | - | `00958DBE` | `00958A30` dispatches `vtable[70h](1)` -> ship `0077D1A0`: destroy message `4Eh`, then base `00926C80` queues the entity on the destroy list `00F899A8` |
| 5 | - | `00827ADA` | selector: `100.0 > [desc+B0h]` sends `9Ah`, otherwise the breakup path |
| 6 | - | `00827B77` | `0077C2A0(unit, msg, 7, 0)` routes it; in a local session it is copied into `session+24Ch` |
| 7 | 9 | `00778450` | `BSP_Session_PumpStep` drains the copy, the ladder falls through to `0077FE80`, the default arm dispatches `vtable[164h]`, and `00821FD6` calls `00814560` |
| 8 | 9 | `00814591` | only for a kamikaze class: `Kill(unit, 1)` sets `+5Fh`, dispatches `vtable[70h](1)` again and queues the entity on the kill list `00F899B4` |
| 9 | 15 | `0092747F` | `009273A0` dispatches `vtable[74h]` for the destroy list: `00926390`, below |
| 10 | 15 | `0092751F` | the same flush dispatches `vtable[80h]` for the kill list, which is non-empty only after step 8 |
| 11 | 16 | `00903610` | the world expiry pass, which skips this entity: it only advances entities whose `+6Ch` is non-zero, and `+6Ch` is set by `00922FD0 BSP_SceneNode_Kill`, not by anything on this path |

Step 9 is the piece the damage doc did not have. `00926390` (already `BSP_MissionEntity_OnDestroyedHook`
in the ledger), `__thiscall(entity)`, body `00926390-009263B1`, coverage complete:

```
00926393  CMP byte ptr [ESI+5Dh],0    ; already released -> RET
0092639B  MOV byte ptr [ESI+5Dh],1
0092639E  MOV byte ptr [ESI+60h],1
009263A1  CALL 00925C90               ; observer-lock helper, contract
009263AE  JMP  [vtable+7Ch]           ; ship: 00824B60
```

So slot `74h` is where the released flag is set, and it tail-dispatches a **third** death slot,
`7Ch`, that no earlier doc names. Evidence for the slot number: in the vtable based at `00CF90B0`,
`+70h` holds `0077D1A0` (`00CF9120`) and `+7Ch` holds `00824B60` (`00CF912C`); slot `80h` at
`00CF9130` is `00951FB0`, the on-killed override the damage doc already found. All eight image-wide
references to `00824B60` (`00CF912C`, `00CFA7F4`, `00CFB7B4`, `00CFC44C`, `00D016AC`, `00D096F4`,
`00D0BFFC`, `00D0C6C4`) sit exactly `0Ch` after a `0077D1A0` entry, so the slot is the same in every
vtable that installs it.

## `00824B60`, the ship's wreck handler (slot `7Ch`)

`__thiscall(ship)`, body `00824B60-008252B0`. Coverage: **partial**. `00824B60-00824FE4` is an
effect and sound teardown read only in outline (it releases effect handles through `004D1100` and
`008674C0`, then `00484620`, under the class gates `[desc+55Ch]` and `[game+19FCh]`); the sink block
`00824FE5-0082523F` below is complete; `00825240-008252B0` is the epilogue and the
`00959450 BSP_Unit_OnDestroyed` call already documented in `docs/UNIT_INSTANCE_UPDATE.md`.

| site | what happens |
| --- | --- |
| `00824FE5` | `0074EC50(&unit+10D4h)`, the leak manager (contract) |
| `00824FF0`-`00825001` | `body = [[unit+1018h]+2Ch]`; `00C37F10(body, &buf)` reads the inertia |
| `00825006`-`00825041` | the written-back triple is `(2*I.x, 2*I.y, 2*I.z)`; the scale is the double `2.0` at `00D7A308` |
| `00825044` | `00C37E70 Dyn_Body_SetInertia(body, scaled)` |
| `0082505C` | `00C37DE0 Dyn_Body_SetAngularDamping(body, 2.5f)`, `00CF87C8` |
| `00825074` | `00C37E00 Dyn_Body_SetLinearDamping(body, 0.5f)`, `00CE3800` |
| `0082507E`, `00825086` | `[unit+828h] = 0.0f` and `[unit+82Ch] = 0.0f` |
| `0082508E` | `00818970` releases the live effect handles at `+BB0h`, `+BC0h` and `+BACh` (outline only) |
| `00825093` | everything below runs only when `[unit+70h] == 1`, the destroy cause `00926C80` stored |
| `008250BD` | `[unit+BC8h] = 00BD2F10([cfg+64Ch], [cfg+650h])` with `cfg = 00424C40()` |
| `008250CE` | the five-point scatter below runs only when `[desc+6A8h]` is non-zero |
| `008250F0`-`008251CD` | five `float3` slots at `unit+B68h`, stride `0Ch` |
| `008251D3`-`0082523F` | if the class has breakup pieces and `[00E188A8]+21D0h` is non-null, `004A5AA0(manager, unit)` |

The x87 in the inertia block was decoded from the bytes, not from the printed listing, because
Ghidra prints `DC C9` and `D8 C9` both as `FMUL ST1`: `D9 40 04` `DD 05 08A3D700` `DC C9` is
`ST1 = I.y * 2.0`, `D9 C9` swaps, `D9 40 08` `D8 C9` is `I.z * 2.0` and `D8 08` is `2.0 * I.x`. The
three results land at `[ESP+4Ch]`, `[ESP+50h]`, `[ESP+54h]`, which is the buffer whose address was
pushed at `0082501D`.

The five-point loop, per slot, with `L = [desc+A0h]` (Length), `W = [desc+A4h]` (Width) and
`H = [desc+A8h]`, the double `2.0` at `00CE3DE0` and the double `3.0` at `00D7A2B0`:

```
slot.x (+0h) = 0 + random(-W/2, +W/2)
slot.y (+4h) = 0 + random( 0.0, +H/3)
slot.z (+8h) = 0 + random(-L/2, +L/2)
```

`EDI` starts at `unit+B70h` and the three stores are `[EDI-8]`, `[EDI-4]` and `[EDI]`, so the slot
base is `unit+B68h`; the loop adds `0Ch` and the final store `FSTP [EDI-10h]` lands on the same
slot's `+4h`. The `A0h`/`A4h` roles match `00891B20`, the Lua `Sink` binding, which builds its pivot
as `x` from `+A4h` and `z` from `+A0h` (`docs/UNIT_DAMAGE_AND_DEATH.md`).

## Where the sink actually happens

A byte-pattern search of the whole image for the displacement `28 08 00 00` returns 45 sites. Four
address a unit:

| site | container | what it does |
| --- | --- | --- |
| `0081111A` | `008110F0 BSP_UnitInstance_Sink` | writes `0.0f` |
| `0082507E` | `00824B60` | writes `0.0f` |
| `0081838C` | `00818340`, the `_ship` diagnostic dump | prints it as **`sinkTime`**, type code `2` (float) |
| `0081FAC5` | `0081F980` | registers `&unit+828h` as a float property for the script/property binding |

So `+828h` is named by the executable itself and is nevertheless dead in native code: no site reads
it, decrements it or accumulates into it. The same search for `+82Ch` finds no unit site other than
the two zeroing stores. The wreck's descent and roll are therefore **not** timer-driven. What the
wreck handler leaves behind is a rigid body with twice the inertia and the two damping rates, and
`004462D0 BSP_GameDynamicsList_ApplyBuoyancyStep` (fixed-step row 2) keeps integrating it; the
settle and the roll are that body's response, and the physics library owns them.

The visible sinking extras are in `BSP_UnitInstance_Update 008255B0`, already documented in
`docs/UNIT_INSTANCE_UPDATE.md`, and both of its gates are what `00824B60` had just written:

- step 2, `0082568F..00825824`, runs only when `+5Dh` is set, which is exactly the flag `00926390`
  set one call earlier. It counts `+BC8h` down by the delta and, at or below zero, reloads it from
  `00BD2F10` and emits the template at `[[unit+538h]+0F0h]`. `00824B60` is the producer of that
  timer's first value. The two argument orders differ: the seed at `008250BD` passes
  `([cfg+64Ch], [cfg+650h])` and the reload passes `([cfg+650h], [cfg+64Ch])`; `00BD2F10`'s body was
  not read, so which one is the low bound is `contract: unread`.
- step 10, `00825C11..00825D4D`, runs only when `[desc+6A8h]` is set, the same gate as the scatter,
  and transforms the five `unit+B68h` points into world space to attach or release five effects.
  So the scattered points are effect anchors on the hull, not motion state.

The three timed sub-updates at `00825D4D` (`008252C0`, `00956600`, `00834E90`) are contracts; this
packet stopped at their call sites.

`008110F0 BSP_UnitInstance_Sink` is the Lua entry to the same state, re-read here from the listing:
`+5Dh` set or `+150h > 0.0f` (`00D7A218` is `0.0f`, so any non-zero invincibility) refuses;
otherwise `vtable[70h](1)`, the same two zero stores, and the object at `+740h` released through its
`vtable[10h]` and nulled. It reaches the wreck handler the same way a damage death does, through the
destroy list, and skips the health write and the session message entirely.

## The breakup alternative

`00827AE5`'s sibling branch builds its message with `00761310`, `__thiscall(msg)`,
body `00761310-00761368`, coverage complete: `msg+10h = 98h`, `msg+4h` set to `3` then `1`, vtable
`00D02C68` then `00D033C4`, `msg+14h` the local slot id from `[[00E188A8]+18ECh]` indexed into
`+18CCh` when that index is `0..7` and `0` otherwise, `msg+18h`/`+1Ah`/`+1Ch` zeroed. It is reached
only when the class selector `[desc+B0h] >= 100.0`, both controller tests `0092BEA0` and `0092BE90`
return non-zero, and `[00424C40()+65Ch] > 00BD2F10(0.0f, 1.0f)`.

Kind `98h` lands on `00814520`, `__thiscall(unit)`, `RET 0`, body `00814520-00814553`. Coverage:
complete for the body, `contract: unread` past its tail call.

| site | what happens |
| --- | --- |
| `00814529` | `0080F9F0([unit+538h])`: true when the descriptor's vector at `[+6E0h,+6E4h)` (stride `14h`) or the one at `[+700h,+704h)` (stride `1Ch`) is non-empty, that is, when the class defines breakup pieces |
| `00814537`-`00814547` | if so and `[00E188A8]+21D0h` is non-null, `004A5AA0(manager, unit)` |
| `0081454C`-`00814553` | `ECX = [unit+1018h]`, tail `JMP 00935D30` |

`00824B60` ends with the identical descriptor test and the identical `004A5AA0` call, so the wreck
handler and the breakup receiver register with the same manager; `004A5AA0` and `00935D30` are
contracts this packet did not open.

## `00780090`, the second caller of the kind dispatch

`__thiscall(entity, message)`, `RET 4`, body `00780090-00780110`. Coverage: complete.

| gate | site | what happens |
| --- | --- | --- |
| `msg[20h] != 0` | `007800AC` | `entity->vtable[144h](msg[28h])`, then the dispatch and return |
| `msg[30h] != 0` | `007800D5` | `entity->vtable[148h](msg[24h], msg[28h])`, fall through |
| `msg[2Ch] == 1` | `007800EF` | `entity->vtable[150h](msg[24h], msg[28h])`, then the dispatch and return |
| otherwise | `00780100`.. | `entity->vtable[150h](msg[24h], 8)`, then the dispatch |

Every arm ends with `0077FE80(entity, msg, 0)`. The third argument is the status pointer the session
doc describes, and here it is **null**, so either the `msg->vtable[10h]` gate at `0077FE88` always
passes on this route or the pointer is checked; that is `contract: unread`.

The path that delivers through it is `007F0030`, `__thiscall(group, message)`, its only caller:
a switch on `msg[10h]` whose `4Bh` arm loops `[group+3CCh]` times over the member pointers at
`group+3D0h` and calls `00780090` once per member, then calls `007EFFB0` when `msg[20h]` is clear
and `msg[2Ch]` is zero. So `00780090` is the per-member form of `00780120` and the route is a group
or squadron fanning a `4Bh` message out to its members. `007F0030`'s own address has **no** absolute
reference anywhere in the image (a byte-pattern search for `30 00 F0 07` returns nothing), so how it
is installed is unread; the death path does not go through it.

## Host table

One row per native call site the reconstruction models as a virtual method in
`include/bsp/unit_death_sink.hpp`. Full argument detail is in `reports/unit_death_sink.json`.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `00814569` | `00935C70` | `detach_all_live_parts` | parts / - / void | none |
| `00814595` | `00926D90` | `kill_entity` | unit / cause `1` / void | either kamikaze field `> 0` |
| `008145A1` | `0092BD30` | `clear_hull_shape_fields` | controller / - / void | same gate, tail call |
| `00814529` | `0080F9F0` | `class_has_breakup_pieces` | class descriptor / - / bool | none |
| `00814547`, `00825239` | `004A5AA0` | `register_wreck` | manager / unit / void | pieces and `[00E188A8]+21D0h` |
| `00814553` | `00935D30` | `breakup_parts` | controller / - / void | none, tail call |
| `009263A1` | `00925C90` | `observer_lock_helper` | entity / - / void | not already released |
| `009263AE` | `[vt+7Ch]` | `dispatch_on_wrecked` | entity / - / void | not already released |
| `00824FEB` | `0074EC50` | `reset_leak_manager` | `&unit+10D4h` / - / void | none |
| `00825001` | `00C37F10` | `read_body_inertia` | body / buffer / buffer | none |
| `00825044` | `00C37E70` | `set_body_inertia` | body / float3 / void | none |
| `0082505C` | `00C37DE0` | `set_angular_damping` | body / `2.5f` / void | none |
| `00825074` | `00C37E00` | `set_linear_damping` | body / `0.5f` / void | none |
| `0082508E` | `00818970` | `release_attached_effects` | unit / - / void | none |
| `008250BD`, `0082513E`, `00825186`, `008251BC` | `00BD2F10` | `random_range` | - / lo, hi / float | destroy cause `1` |
| `0082509D` | `00424C40` | `game_settings` | - / - / settings | destroy cause `1` |
| `00811111`, `00926E05`, `00958DBE` | `[vt+70h]` | (already `dispatch_destroy` in unit_damage.hpp) | entity / `1` / void | see each caller |
| `00811135` | `[[+740h]+10h]` | (already `release_sink_attachment`) | object / - / void | `+740h` non-null |
| `007800AC` | `[vt+144h]` | `member_apply_a` | member / `msg[28h]` / void | `msg[20h]` |
| `007800D5` | `[vt+148h]` | `member_apply_b` | member / `msg[24h]`, `msg[28h]` / void | `msg[30h]` |
| `007800EF`, `00780105` | `[vt+150h]` | `member_apply_c` | member / `msg[24h]`, `msg[28h]` or `8` / void | `msg[2Ch] == 1` |
| `007800B3`, `007800F6`, `0078010B` | `0077FE80` | `dispatch_entity_kind_message` | member / msg, null / bool | none |

Contracts named but not opened: `00935D30`, `004A5AA0`, `00925C90`, `0074EC50`, `00818970` (outline
only), `00BD2F10`, `00C37F10`, `0092BEA0`, `0092BE90`, `008252C0`, `00956600`, `00834E90`, and the
physics library's `00C31DC0`, `00C48020`, `00C48050`.

## Coverage

| routine | coverage |
| --- | --- |
| `00814560`, `0092BD30`, `00814520`, `00926390`, `00761310`, `00780090` | complete |
| `008110F0` | complete (re-read from the listing) |
| `00824B60` | partial: `00824FE5-0082523F` complete, `00824B60-00824FE4` outline, `00825240-008252B0` unread |
| `007F0030` | partial: the `4Bh` arm only |
| `00827A90` | complete for the selector and both message branches, from this packet's re-read |

## Open questions

- Whether `0077FE80` tolerates the null status pointer `00780090` passes.
- What `shape+2Ch` and `shape+30h` mean, which decides whether `0092BD30` disables collision or only
  reclassifies it.
- Why `sinkTime` exists at all: it is named, script-visible and zeroed twice, but no native reader
  was found. A Lua script or a property binding writing it is the remaining candidate.
- The seed and reload argument orders for `+BC8h` differ; `00BD2F10`'s body settles it.
- `00824B60`'s first `480h` bytes, and `00935D30`, which is where the visible breakup lives.
