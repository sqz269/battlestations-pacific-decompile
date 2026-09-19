# `NavigatorMoveOnPath` and the two navigator flags the same script sets

Addresses: `008A3600`, `008A3B10`, `008A3CD0`, `00721A40` (`5Ah` and `5Bh` arms), `00720FA0`,
`0071C1B0`, `00835940`, `00835A40`, `00465080`, `00D02E84` (vtable), `00CFD9C4` (vtable)

Packet `cc8_navigator_path`, worktree `agent/cc8-navigator-path`. Ghidra was read-only. Every
name here is a hypothesis, not a recovered symbol. Read from the **listing**, not the decompile.

`008A3600` is the navigator sibling `cc_lua_navigator` left out of
`docs/LUA_BINDING_NAVIGATOR.md`. It is **not** a fourth copy of that family's shape.

## 1. Two corrections before anything else

**`008A3600` was not unread.** Packet `cc_commanded_speed` already projected its speed store and
recorded, in `reports/commanded_speed.json`, `coverage: partial: only the store at
008A38D5-008A3912 is projected; the 5Bh session message it routes is not read`. That message is
this packet's subject. Nothing here re-does the speed store.

**`008A3600` never calls `0077D600`.** `docs/HANDOFF_USN04_LUA_NATIVES.md` expected the
`NavigatorAttackMove` / `NavigatorMoveToRange` shape with a different command constant. It is not:
there is no `0077D600` call anywhere in `008A3600-008A3960`. The native routes a `5Bh` session
message and the *receiving unit's weapon director* turns that into the command.

## 2. `008A3600`, whole

`__fastcall(lua_State* in ECX)`, body `008A3600-008A3960`, returns 0 (`00B66400` at `008A3917`
with nothing pushed above the arguments). The machine prologue and the dead
`luaMW_NavigatorMoveOnPath failed:` error-prefix block of `docs/MISSION_LUA_MACHINE.md` are
present; the length is `0x21` at `008A3629`, which is `strlen` of that literal.

### Arguments, and the order they are read in

| Lua index | read at | how | default | default set at |
| --- | --- | --- | --- | --- |
| 0 | `008A3707` | `00888AA0` entity | required | — |
| 2 | `008A3764` | `00B66290` integer, the follow mode | **1** | `MOV EDI,0x1` `008A361F`, stored `008A3730` |
| 3 | `008A37A5` | `00B66290` integer | **5** | `LEA EBP,[EBX+0x3]` `008A3734`, EBX = 2 |
| 4 | `008A37E4` | `00B66270` number, `FSTP` `008A37E9`, the speed | class MaxSpeed | `MOVSS XMM0,[EAX+0x500]` `008A3724`, `EAX = [entity+538h]` |
| 1 | `008A381A` | `0088A810` into a `SceneCommandTarget` | required | — |

**Argument 1 is read last**, after all three optional ones. The three optional arguments are gated
by three separate `00B663F0` calls (`008A373D`, `008A3781`, `008A37C0`) comparing `3`, `4` and `5`,
each with a `JL` to the **same** `008A37FD`, so they are strictly cumulative: no argument 3 without
argument 2. The eight shipped call sites all pass three arguments, so arguments 3 and 4 hold their
defaults in every USN04 call and the commanded speed stored is the class MaxSpeed.

### Resolving the path entity, `008A382F-008A3877`

`SceneCommandTarget` is `include/bsp/scene_deferred_refs.hpp`'s, and the three reads match it field
for field: the byte at `+0h` is `kind`, the word at `+2h` is `object_id`, the dword at `+4h` is
`object`.

1. `kind == 0` (`CMP byte ptr [ESP+0x48],0x0` at `008A382F`) -> `XOR EDI,EDI` and jump straight to
   the message build.
2. `object` non-null (`008A383A`) -> that pointer.
3. otherwise resolve `object_id` through a two-range table: below `[00F89A10]`, subtract
   `[00F89A0C]` and index `[00F89A54]`; at or above it, subtract `[00F89A60]` and index
   `[00F89AA8]`. Both are 16-byte records with the pointer at `+0Ch`.

**A latent null dereference in the shipped binary.** Arm 1 leaves `EDI` null and the body still
reads `MOV AX,word ptr [EDI+0x174]` at `008A38A9`. A `NavigatorMoveOnPath` whose argument 1 is not
a resolvable entity reads address `0x174`. The reconstruction stops instead of reproducing it and
says so here rather than pretending the native guards.

### The `5Bh` message, `008A3879-008A38D0`

Built over the `SceneCommandTarget`'s own stack storage at `ESP+0x48`, after that record has been
consumed into `EDI`, so the two never coexist.

| offset | value | site |
| --- | --- | --- |
| `+0h` | vtable `00D02E84` | `008A38A1` |
| `+4h` | 1 | `008A3894` |
| `+18h` | word 0 | `008A3888` |
| `+1Ah` | byte 0 | `008A388F` |
| `+1Ch` | byte 0 | `008A389C` |
| `+20h` | `uint16 [pathEntity+174h]` | `008A38A9`, `008A38B0` |
| `+24h` | the follow mode | `008A38B5` |
| `+28h` | the argument-3 integer | `008A38B9` |

`+0h`, `+4h`, `+18h`, `+1Ah` and `+1Ch` are the same base `JoinFormation`'s `76h` message writes
(`docs/LUA_BINDING_NAVIGATOR.md`), so only the last three are this type's own.

Routed by `0077C2A0 BSP_Session_RouteMessage` at `008A38D0` with `this` = the **acting unit**,
route flags **0** and out 0. JoinFormation's `0077C964` and both `5Ah` sends pass 7; the path order
passes 0. That difference is recorded and not explained.

### The speed store, `008A38D5-008A3912`

Already `docs/UNIT_COMMANDED_SPEED.md`'s. Restated only for the clamp's sense, which needs the x87
stack order: `FLD` the speed, `FLDZ`, so `ST0` = 0.0 and `ST1` = the speed; `FCOMIP ST0,ST1` at
`008A38E8` compares **0.0 against the speed** and the `JBE` at `008A38F4` takes the speed when
`0.0 <= speed`. A negative speed clamps to zero. Then `*(unit+73Ch)+28h` takes `[00F876A4]`, the
mission clock. The store runs **after** the message and unconditionally.

## 3. The receiver: `00721A40`'s `5Bh` arm, `00721A9D-00721AE6`

`00721A40 BSP_WeaponDirector_ApplyGameUnitMessage`, whose arm table is
`docs/DIRECTOR_UPDATE_ARMS.md`'s. The `5Bh` arm, read from the listing:

1. `00720FA0` (body `00720FA0-00720FD6`, read whole) re-resolves the path entity from
   `MOVZX EAX,word ptr [ECX+0x20]`, the message's `+20h`, through **the same two-range table** the
   binding uses at `008A3847-008A3873`. Same globals, same 16-byte stride, same `+0Ch`.
2. `00465080 BSP_CommandTarget_FromEntity` at `00721AC3` builds a descriptor from it.
3. `director->vtable[60h](0x00E08F80, descriptor, 0.0f)` at `00721AD3`. `00E08F80` is `moveonpath`
   (`docs/COMMAND_CLASSES.md`, ordinal 42). The `0.0f` is the slot reserved at `00721AB3` and
   filled by `FSTP float ptr [ESP]` at `00721AB6`, so it is the call's third stack argument.
4. `0071C1B0(director, msg+24h)` at `00721ADB`.

### Correction to `docs/DIRECTOR_UPDATE_ARMS.md` line 199 and `reports/director_update_arms.json`

That row reads `then 0071C1B0(msg+24h)`, which reads as one value being passed. It is a
**pointer**, and `0071C1B0` (body `0071C1B0-0071C1DD`, `RET 4`, read whole) writes **two** dwords:

```
0071C1B2  LEA EDX,[ECX + 0x54]          ; walk the slot array
0071C1B5  CMP dword ptr [EDX],0x0       ; first slot whose dword is zero
0071C1BD  ADD EDX,0x1c                  ; stride 1Ch, bound 10 (0071C1C0 CMP EAX,0xa)
0071C1C5  MOV EAX,[ECX + EAX*0x4 + 0x1a0]  ; that slot's path object
0071C1D2  MOV [EAX + 0x8],EDX           ; = msg+24h, the follow mode
0071C1D8  MOV [EAX + 0xc],ECX           ; = msg+28h, the argument-3 integer
```

So `msg+28h` has a consumer, and the follow mode and the argument-3 integer land as a **pair** on
the slot's path object at `+8h` and `+0Ch`. The slot array base is `director+1A0h`, which is the
same array `docs/CRUISE_COMMAND.md` describes as `director+1A4h+i*4`.

**Open, and not claimed either way:** `0071C1B0` picks the first *empty* slot, but step 3 has just
queued a command. Whether `vtable[60h]` (`008358D0`) has already filled that slot, making this the
*next* one, or defers so that this is the slot the command will land in, was not read. The
arithmetic above is established; the index it selects relative to step 3 is not.

### Correction, packet `cc8_ship_moveonpath`

**Closed.** The store is at `director+1A0h + i*4`, which is `director+1A4h + (i-1)*4`, one element
*below* the base `0071BFF0` indexes; `i` is the first *empty* record, so `i-1` is the last
*occupied* one, which is the command step 3 has just queued. `i` can only be 0 if nothing is queued
at all. So the pair lands on the command just issued, not on the next one.

**And the target is the command/slot object, not "the slot's path object".** `0071BFF0` answers
`slot + 10h` and `007ADC60` reads `+4h`, `+8h`, `+0Ch` and `+10h` off that, so `+8h` and `+0Ch` of
the base object sit below the cursor. The 0Ch-byte thing `007B2250`/`007B22A0` allocate - the one
this doc's term "path object" fits - has no `+0Ch` to write.

**`msg+28h` is the START MODE.** Section 2's table leaves argument 3 as an unknown integer. It is
`PATH_SM_*`: 5 `JOIN`, 6 `BEGIN`, 7 `JOIN_RANDOM_DIR`, 8 `JOIN_BACKWARDS`, and `007B1C50` branches
on exactly those four values with exactly those meanings. The follow mode is `PATH_FM_SIMPLE` 1,
`PATH_FM_PINGPONG` 2, `PATH_FM_CIRCLE` 3. Both tables are in this installation's
`scripts/global/luamw_init.lua` 224-232. See `docs/SHIP_AI_PATH_CURSOR.md`.

## 4. The two companions, `008A3B10` and `008A3CD0`

Neither writes a local flag. Both take `*(entity+738h)` — the weapon director, which is what
`00835A40`'s and `00835940`'s already-recovered names establish — and send a `5Ah` message.

| binding | body | entity | boolean | sender | selector |
| --- | --- | --- | --- | --- | --- |
| `NavigatorSetAvoidLandCollision` `008A3B10` | `008A3B10-008A3CC4` | `008A3C0E` | `008A3C40` | `00835A40` at `008A3C67` | **9** (`00835A8A`) |
| `NavigatorSetTorpedoEvasion` `008A3CD0` | `008A3CD0-008A3E69` | `008A3DCF` | `008A3E06` | `00835940` at `008A3E0E` | **7** (`0083598A`) |

Both senders, read whole, build `0075B430(5Ah)` with vtable `00CFD9C4` at `+0h`, 1 at `+4h`, the
same zeroed `+18h`/`+1Ah`/`+1Ch`, the **selector** at `+20h` and the **boolean** at `+24h`, and
route through `0077C2A0` with flags 7 and `this = *(director+34h)`. So `+20h` is not always an
entity id: in the `5Bh` message it is one, in the `5Ah` message it is a selector.

`008A3B10` alone has an extra arm, and it is on the **disable** side: `TEST BL,BL` / `JNZ` at
`008A3C6C`-`008A3C6E` skips it when the boolean is true, so
`0092BD00(0080E490 BSP_UnitInstance_GetPartsObject(unit))` at `008A3C72`/`008A3C79` runs only when
land-collision avoidance is turned **off**. Neither body was read by this packet.

### Is there a consumer in this host?

**No, and it is a named hole.** The `5Ah` receive path is `00721A93` ->
`director->vtable[38h]` = `00835640` over `0071C1E0`, and neither is projected here. The block the
value would land in does exist: `bsp::ShipAiAvoidanceRequest`'s `enable_3f4` (`blk+3ECh`) in
`include/bsp/ship_ai_avoidance_request.hpp`, whose two setters `009DABB0` (torpedo) and `009DABD0`
(land) that header itself marks `Unused`, i.e. with no caller found. So the send side is complete
and the consumer is `0071C1E0`.

## 5. What the reconstruction does, and the two places it diverges

`src/lua_binding_navigator.cpp` gains `lua_binding_navigator_move_on_path`,
`lua_binding_navigator_set_avoid_land_collision` and `lua_binding_navigator_set_torpedo_evasion`,
on the existing `LuaBindingNavigatorHost` beside the eight siblings rather than on a new host.
`GameScriptOrdersHost` routes the three rows.

Two divergences, both holes, neither a proof:

1. **The entry point.** The executable reaches the director through its own `vtable[60h]`
   (`008358D0`). This host has one command-issue seam, the projected `0077D600` / `00816E30` path,
   and uses it. `00816E30`'s `moveonpath` arm at `00816F7D` exists
   (`docs/ENTITY_COMMAND_ARMS.md`), so the destination slot is the same; the route to it is not.
2. **The follow-mode pair has nowhere to land.** `0071C1B0` writes onto the slot's *path object*,
   and `src/game_hosts_commands.cpp`'s `command_count` already records that this process "does not
   build" one. The pair is carried on the report row and recorded as
   `Navigator::path_object_set_follow_mode` at `0071C1B0`, not stored.

**The ship will not start circling the path, and this packet does not claim it will.**
`src/game_hosts_ship_ai.cpp` states that of the eight AI state steps only `stop` (`009E14C0`),
`movetopos` (`009E5770`) and `attackmove` (`009E8820`) have projected bodies; `moveonpath`
(`009E59C0`) "is still a record". So the order now reaches the director and selects the AI state,
and the motion behind that state is the next packet's, together with the `0071F600` path build that
`src/command_execution.cpp`'s header comment already labels a contract.

## 6. Coverage

| routine | coverage |
| --- | --- |
| `008A3600` | complete: every instruction `008A3600-008A3960` accounted for |
| `008A3B10` | complete as a sequence; `0092BD00` and `0080E490` are called and unread |
| `008A3CD0` | complete |
| `00720FA0` | complete, read whole |
| `0071C1B0` | complete, read whole |
| `00835A40`, `00835940` | complete, read whole |
| `00721A40` `5Bh` arm | complete as an arm; `vtable[60h]` = `008358D0` not re-read |
| `00721A40` `5Ah` arm | not read beyond `docs/DIRECTOR_UPDATE_ARMS.md`'s row |
| `00465080` | not read; taken from its recovered name |
