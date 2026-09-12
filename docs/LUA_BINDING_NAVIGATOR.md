# The mission Lua bindings that order the mission's ships

Addresses: `008A30D0`, `008A2F20`, `008A2BC0`, `008A2D70`, `00899D10`, `00895250`, `008AD330`, `008AB850`, `0088A810`, `0077C8D0`, `00905300`

Packet `cc_lua_navigator`, worktree `agent/cc-lua-navigator`. Ghidra was read-only. Every name
here is a hypothesis, not a recovered symbol. The reconstruction is
`include/bsp/lua_binding_navigator.hpp` and `src/lua_binding_navigator.cpp`; the run evidence is
`reports/lua_binding_navigator.json`.

`docs/GAME_EXECUTABLE.md` milestone 2l runs `usn_2_java`'s own order function and watches it
address 21 of 32 created instances through ten bindings that are all host records, so **no order
reaches a ship**. This packet reads eight of those ten rows, the descriptor reader they share,
and the formation helper one of them calls.

## The shape every one of the eight has

`__fastcall(lua_State* in ECX)`, result count in `EAX`. Around the binding's own work sits the
machine prologue and epilogue of `docs/MISSION_LUA_MACHINE.md`: `00B66C00`
`BSP_LuaStateOwner_ConstructBorrowed`, `00B679B0` `BSP_LuaObject_OpenCallFrame`, indexed argument
reads through `00B677E0`, then `00B66400` `BSP_LuaObject_ResultCount` and `00B669A0`
`BSP_LuaStateOwner_Close`. None of the eight pushes a value above its arguments, so all eight
return 0. Lua argument index 0 is the first positional argument; the shipped scripts call these
as free functions (`SetSkillLevel(unit, SKILL_STUN)`), not as methods, so "self" is simply
argument 0.

### The dead error-prefix block

Each body opens by sizing a `std::string` through `0041DD40` `BSP_NativeString_Resize`,
`memcpy`-ing a `luaMW_<name> failed:` literal into it through `00BF7680`, and immediately
returning the block to the pool (`00419CC0` + `00BD1510`). The length is exactly `strlen` of that
literal, which is how each body is identified: `0x21` for `luaMW_NavigatorAttackMove failed:`,
`0x22` for `MoveToRange`, `0x28` for `DirectMoveToRange`, `0x20` for `MoveToPos`, `0x1B` for
`JoinFormation` and `SetSkillLevel`, `0x1E` for `SetRoleAvailable`, `0x1A` for `RepairEnable`.
The string never escapes. It is the residue of an inlined error-report helper whose reporting
arm the optimiser removed, and nothing in the reconstruction models it.

## `0088A810`: the Lua value to `SceneCommandTarget` reader

`__fastcall(ECX = SceneCommandTarget* out, EDX = const LuaObject*)`, body `0088A810-0088A8F3`,
`RET 0`, returns the out pointer in `EAX`. 17 callers, every mission Lua binding that takes a
target. The record is `bsp::SceneCommandTarget` from `include/bsp/scene_deferred_refs.hpp`; this
packet restates no layout, and the field order the copy loop in `008A30D0` walks
(`008A3219-008A326D`: word, word, dword, four floats) matches that header field for field.

The routine fetches the field named by the literal at `00CE59B4`, whose bytes are `49 44 00`,
**"ID"**, through `00B67910` at `0088A83B`, and tests it with `00B65FB0` at `0088A84A`.

| `value.ID` | what the descriptor becomes |
| --- | --- |
| present | `00888AA0` at `0088A855` resolves `value.Ptr` to an entity. Position is the zero vector `00F87574`, `object` is the pointer. When the pointer is non-null, `kind` = 1 and `object_id` = the uint16 at `entity+174h` (`0088A88B-0088A895`). |
| present, resolve null | falls to the shared tail: `kind` 0, `object_id` 0, `position_valid` 0, position at the origin. Not an error. |
| absent | `00888760` at `0088A8A1` reads the value as a `Vector3` into the position; `position_valid` = 1 at `0088A8A9`, `object` = null at `0088A8BE`. |

`0088A8C1` clears `object_id` and `kind` for both surviving paths and `0088A8C7` clears the `+14h`
float with `XORPS`. `position_valid` is the only field that separates the two branches, which
supports `scene_deferred_refs.hpp`'s provisional reading of `+1h`: 1 means the position fields
were authored, 0 means they are filler.

## The navigator bindings

`008A30D0` `NavigatorAttackMove` and `008A2F20` `NavigatorMoveToRange` are the same routine with
one constant changed. Each resolves Lua argument 0 as the acting entity (`00888AA0` at `008A31CF`
/ `008A301F`), reads Lua argument 1 into a `SceneCommandTarget` through `0088A810` (`008A3204` /
`008A3054`), and calls `0077D600` `BSP_Entity_IssueCommand` (`008A3273` / `008A3077`) with a fixed
command object, that descriptor, and the constant flags 1.

| binding | handler | command object | registry row | category |
| --- | --- | --- | --- | --- |
| `NavigatorAttackMove` | `008A30D0` | `00E08F78` `attackmove` | 17 | 2 |
| `NavigatorMoveToRange` | `008A2F20` | `00E08F68` `moveto` | 15 | 3 |
| `NavigatorMoveToPos` | `008A2BC0` | `00E08F68` `moveto` | 15 | 3 |
| `NavigatorDirectMoveToRange` | `008A2D70` | `00E08F68` `moveto` | 15 | 3 |

Command objects and ordinals are `docs/SCENE_COMMAND_TYPES.md`'s table of 26.
`00E08F78` already exists as `bsp::kCommandObjectAttackMove` in
`include/bsp/unit_commanded_speed.hpp`; this packet adds `kCommandObjectMoveTo` for `00E08F68`
under the same rule. What the ship's consumer does with the issued command is
`docs/ENTITY_ORDER_MESSAGE.md`'s and `docs/CRUISE_COMMAND.md`'s, not this packet's: `0077D600`
copies the descriptor, resolves the id, may notify the AI group, and routes an `MT_COMMAND`
message. This packet did not re-read it and models it as one host method.

## `00899D10` `JoinFormation`, and which ship becomes the follower

`00899D10` resolves Lua argument 0 (`00888AA0` at `00899E10`, into `ESI`) and Lua argument 1
(`00888AA0` at `00899E41`, into `EDI`), then calls `0077C8D0` at `00899E5B` with `ECX` = argument
0 and the single stack argument = argument 1.

`0077C8D0` `__thiscall(entity, void* other)`, body `0077C8D0-0077C97B`, `RET 4`, four callers
(`00816E30` `BSP_UnitInstance_ApplyEntityCommand`, `00899D10`, `009483D0`, `00A10DC0`):

1. `entity->vtable[16Ch]("follow", other)` at `0077C8F8`. The literal `00CFB52C` is the bytes
   `66 6f 6c 6c 6f 77 00`, `"follow"`, which sit immediately after the `follow` command class's
   vtable `00CFB518`. A false answer ends the routine with no effect at all (`0077C902`).
2. `entity+1ACh` is read (`0077C904`) and, when it is `0..7` unsigned (`CMP 7 / JA`), passed to
   `00905300` with `ECX` = `[00E188A8]+21A0h`.
3. A session message of type `76h` is built by `0075B430` at `0077C926` and given vtable
   `00D02D30` at `+0h`, 1 at `+4h`, zeros at `+18h`/`+1Ah`/`+1Ch`, and the **other** entity's
   uint16 id at `+20h`. `+0h`/`+4h`/`+18h` match `include/bsp/entity_orders.hpp`'s message base;
   `+20h` is this type's own field.
4. `0077C2A0` `BSP_Session_RouteMessage` routes it at `0077C964` with `this` = the acting entity,
   route-flags override 7 and out 0.

**The direction:** the question asked is "may *this* entity follow *other*", and the message
carries *other*'s id. So **Lua argument 0 is the follower and Lua argument 1 is the leader.**

`0077C980` (type `77h`, vtable `00D02D44`) and `0077CA60` are the group-scoped siblings, keyed on
`entity+284h`; they were not read beyond confirming that they are distinct message types.

`00905300`, body `00905300-00905319` read in full, `__thiscall(base, int slot)`, `RET 4`: adds 1
to the dword at `base + slot*284h + 170h` and returns its address. Only the arithmetic is
established. The `0..7` bound matches the player-slot range elsewhere in the corpus, but what the
counter records was not read and the reading is provisional.

## The three small bindings

**`00895250` `SetSkillLevel`**, body `00895250-008953EF`. Reads Lua argument **1** as an integer
(`00B66290` at `00895351`) *before* resolving Lua argument 0 as the entity (`00888AA0` at
`00895381`), then calls `entity->vtable[128h](level)` at `008953A2`. No clamp, no range test, no
message: the script's value reaches the virtual unchanged.

**`008AD330` `RepairEnable`**, body `008AD330-008AD534`. Resolves argument 0 as the entity
(`008AD42E`), asks `entity->vtable[5Ch](6)` (vtable read `008AD448`, `PUSH 6` at `008AD44B`, call
`008AD44F`), reads argument 1 as a boolean (`00B66250` at `008AD46D`), then takes one of two
**exclusive** arms at `008AD487`:

- false: writes the byte at `entity+378h` directly (`008AD4E8`).
- true: builds a type-`9Fh` message (`0075B430` at `008AD494`, vtable `00D03360`, the boolean at
  `msg+1Ch`) and routes it through `0077C2A0` at `008AD4CD` with flags 7 and out 0.

What class id 6 means was not read; all this body shows is that the answer selects local write
versus routed message.

**`008AB850` `SetRoleAvailable`**, body `008AB850-008ABABF`, three Lua arguments. Argument 0 is
read with `00888D20` `BSP_LuaTable_GetPtrField` at `008AB953`, **not** with `00888AA0`: `00888D20`
fetches the table's `Ptr` without the entity-table validation, so any table carrying a `Ptr`
answers. Arguments 1 and 2 are integers (`00B66290` at `008AB984` and `008AB9B4`). Three arms:

| condition | effect |
| --- | --- |
| session mode `[00E188A8+1FE4h]` == 0 (read `008AB9D1`) | `owner->vtable[148h](role, value)` at `008ABA51` |
| otherwise, `004BCA50` effective mode < 4 (`008AB9DE`) | type-`4Ch` message, vtable `00D02CA4`, role at `+1Ch`, value at `+20h`, routed at `008ABA2F` with flags 5 |
| otherwise | `owner->vtable[148h]` at `008ABA73`, plus `BSP_Game_AssignPartyPlayerSlots(0)` at `008ABA60` when value < 8 |

## Host methods the executable must implement, in call order

`include/bsp/lua_binding_navigator.hpp` declares one pure virtual per native call site.
`reports/lua_binding_navigator.json`'s `host_methods` array is the machine-readable form with the
`address` (call site) and `native` (callee) of each; `python tools/verify_report_calls.py
reports/lua_binding_navigator.json` checks all 27 direct rows against the live function bodies
and reports 0 failures. The nine remaining rows are field reads and virtual dispatches, which
that tool marks `indirect` and does not check.

Per binding, in the order the native performs them:

- `NavigatorAttackMove` / `NavigatorMoveToRange`: `argument_entity` (`008A31CF` / `008A301F`) ->
  `argument_id_field_is_nil` (`0088A83B`, `0088A84A`) -> `argument_entity` or `argument_vector3`
  (`0088A855` / `0088A8A1`) -> `entity_object_id` (`0088A88E`) -> `entity_issue_command`
  (`008A3273` / `008A3077`).
- `JoinFormation`: `argument_entity` x2 (`00899E10`, `00899E41`) -> `entity_command_is_available`
  (`0077C8FE`) -> `entity_route_slot` (`0077C904`) -> `slot_counter_increment` (`0077C91B`) ->
  `entity_object_id` (`0077C92B`) -> `session_route_formation_message` (`0077C926`, `0077C964`).
- `SetSkillLevel`: `argument_integer` (`00895351`) -> `argument_entity` (`00895381`) ->
  `entity_set_skill_level` (`008953A2`).
- `RepairEnable`: `argument_entity` (`008AD42E`) -> `entity_is_kind_of` (`008AD44F`) ->
  `argument_boolean` (`008AD46D`) -> `entity_set_repair_enabled_field` (`008AD4E8`) or
  `session_route_repair_enable_message` (`008AD494`, `008AD4CD`).
- `SetRoleAvailable`: `argument_ptr_field` (`008AB953`) -> `argument_integer` x2 (`008AB984`,
  `008AB9B4`) -> `game_session_mode` (`008AB9D1`) -> `game_effective_game_mode` (`008AB9DE`) ->
  `role_owner_set_role_available` (`008ABA51` / `008ABA73`) or `session_route_role_message`
  (`008AB9EE`, `008ABA2F`), plus `game_assign_party_player_slots` (`008ABA60`).

## The probe run: which ships receive which orders

`src/mission_script_probe.cpp` grew `--navigator-bindings`, which routes the eight rows through
`src/lua_binding_navigator.cpp` against a stub entity state instead of logging, and labels each
minted entity with the first string argument of the row that minted it, which for `FindEntity` is
the ship's authored name.

**The blocker, and the fixture that gets past it.** `usn_2_java`'s `luaInit` reaches its order
function only through `luaPickRnd` (`Scripts/global/commandhelpers.lua:1003`), which compares the
result of `luaRnd()` and so of the native row `random` `0088C160`. Stubbed, that row pushes
nothing, `luaRnd` returns nil, and `luaInit` dies at that comparison before a single ship is
ordered. That is exactly milestone 2l's `status=2 ... attempt to compare number with nil`, and
the probe's default run still reproduces it.

`0088C160` is **not** reconstructed by this packet and is not in its lease. All that was read of
it is the argument-count switch through `00B663F0`: zero arguments draw against the float at
`00D11318`, one against `arg0+1`, two against `arg0+1` and `arg1`, each through `00BD2F10` +
`00BF7420`, pushed as an integer. The generator, its seeding and the exact bound convention were
not read. So `--stand-in-random` is a **deterministic probe fixture** with the native's argument
shape and none of its numerics, present only so the rest of `luaInit` runs. It is off by default.

With `--core-bindings --navigator-bindings --stand-in-random`, `luaInit` completes and 28 entities
receive orders. The full table is in `reports/lua_binding_navigator.json`; in outline:

- Four Allied ships (`DeRuyter`, `Java`, `Kortenaer`, `Electra`) get skill 0, repair off, a
  `SetRoleAvailable(511, 8)` row each, and all four join `DeRuyter`'s formation. `DeRuyter` alone
  receives `moveto -> DRGoTo`.
- Ten more (`Houston` + four, `Exeter` + four) get skill 2, repair on, and join `Houston`'s or
  `Exeter`'s formation.
- Fourteen Japanese ships receive `attackmove`; six carry no skill row and eight get skill 1 with
  repair on.

Three caveats that belong with those rows:

1. **The target column of the fourteen `attackmove` rows depends on the fixture.** Those loops
   pick their target through `luaPickRnd`. The *ship* column does not: those loops are over fixed
   `FindEntity` lists.
2. **`DeRuyter`, `Houston` and `Exeter` follow themselves.** That is the script's own call:
   `usn_2_java.lua:210-228` loops over the whole group including index 1 and calls
   `JoinFormation(unit, group[1])` for every member. Whether the native accepts a self-follow is
   decided by the unread predicate `008162B0`; the probe answers it true for every call.
3. **The call counts are larger than milestone 2l's** (`SetSkillLevel` 22 vs 15, `RepairEnable`
   34 vs 12, `NavigatorAttackMove` 14 vs 6) because that run resolves `FindEntity` only over the
   32 created scene instances while this probe mints an entity for every entity-returning row, so
   more script branches run. `JoinFormation` (14), `NavigatorMoveToRange` (1) and
   `SetRoleAvailable` (4) match exactly.

The host records seven native steps it cannot perform rather than inventing them, including every
`0077D600` issue: the probe is not a game and no ship moves in it.

## Corrections

- **`NavigatorMoveToRange` takes no range.** The packet brief expected a target plus a range.
  `008A2FFD` and `008A3036` push indices 0 and 1 into `00B677E0` and there is no third
  `ArgumentAt` in `008A2F20-008A30C2`; `0077D600`'s three stack arguments at `008A3069`,
  `008A306F` and `008A3070` are flags 1, the descriptor and the command object. Any range belongs
  to the `moveto` command class, not to the call.
- **`MoveToPos`, `DirectMoveToRange` and `MoveToRange` are one native behaviour.** The three
  bodies are identical at `0x1A2` bytes and all three `PUSH 0xe08f68`. They differ only in the
  dead error-prefix length and the one-time `luakod` guard global.
- **Ghidra's "Removing unreachable block" warnings on these bodies are wrong.** `008A3110` and
  `008A316C`, and the matching pairs in `008A2F20`, `008A2BC0`, `008A2D70` and `00899D10`, are
  reachable. `008A30FB`/`008A30FF` zero the string object before `0041DD40` writes through `ECX`;
  Ghidra's prototype does not model that write, so it folds `CMP EDI,EBP` at `008A310C` to
  always-equal. `008AD330` uses the same idiom with a different stack layout and decompiles with
  the `memcpy` visible, and its length `0x1A` equals `strlen("luaMW_RepairEnable failed:")`.
- **Vtable slot `16Ch` is a predicate, not an apply.** Read from the call site alone, `0077C8D0`
  looks like it applies `follow`. MDestroyer's vtable base is `00CFC3D0` because `[00CFC3D0+160h]`
  is `00816E30`; `[00CFC3D0+16Ch]` is `008162B0`, whose body `008162B0-00816408` returns bool and
  dispatches only `IsKindOf` tests and `00438E10` `BSP_CString_CompareInsensitive`. A false answer
  makes `0077C8D0` return with no effect. What turns an accepted request into an order is the
  delivery of the type-`76h` message, which this packet did not read.
- **`[00E188A8]+1FE4h` is the session mode, not a campaign flag.** This packet's first reading
  called it a non-campaign flag. `include/bsp/unit_damage.hpp`, `include/bsp/unit_order_record.hpp`
  and `include/bsp/unit_state_message.hpp` already declare the same address as the session mode
  (0 local, 2 client); that reading is reused and no constant is restated.
- **`SetInvincible` `00897A50` was left alone.** The brief allowed it only if no reconstruction
  existed. `python tools/bsp.py lookup 00897a50` shows `BSP_LuaBinding_SetInvincible` with
  `docs/UNIT_DAMAGE_AND_DEATH.md` and `reports/unit_damage.json` from packet `cc2_unit_damage`, so
  it was not leased, named or redone. `SetFireTarget` `0089A8B0` is likewise already
  `BSP_LuaBinding_SetFireTarget`.

## What this packet did not do

- `0077D600`'s body, the delivery of the type-`76h`, `9Fh` and `4Ch` messages, and the concrete
  meaning of vtable slots `128h`, `148h` and `5Ch` class id 6. Each is one host method.
- `008162B0`, the `16Ch` predicate: its top-level structure was read (bool return, `IsKindOf` and
  name-comparison dispatch) but not its body, so it was not named and not leased.
- `0088C160` `random`: outside the packet, see the fixture note above.

## Follow-up packets

1. **`random` `0088C160`** and the `00BD2F10` / `00BF7420` generator behind it. It is the single
   row standing between the probe and an unassisted run of `usn_2_java`'s order function, and the
   fixture in `src/mission_script_probe.cpp` should be deleted the moment it lands.
2. **The type-`76h` formation message** and its handler: what a delivered join actually does to
   the follower's navigator, and how it relates to the `follow` command class `00E08F60`.
3. **`008162B0`**, the command-availability predicate at vtable `16Ch`, including whether it
   refuses a self-follow, which decides whether three of the run's rows are real.
4. **The remaining eleven `Navigator*` rows** of `00E0BC10-00E0BC88`: `NavigatorStop`,
   `NavigatorCruise`, `NavigatorMoveOnPath`, `NavigatorForceMoveCloseToTarget`,
   `NavigatorSetAvoidShipCollision`, `NavigatorSetAvoidLandCollision`,
   `NavigatorSetAvoidAllShipCollision`, `NavigatorSetTorpedoEvasion`, `NavigatorAllowMaxDepth`,
   `NavigatorLandAttackerStop`, `NavigatorForceTorpedo`. The two evasion/collision rows are the
   most-called bindings in the milestone-2l run (21 each) and `008A33FF`/`008A35A4` already appear
   in `include/bsp/unit_commanded_speed.hpp`'s navigator-parameter offsets.
5. **`0077C980` and `0077CA60`**, the group-scoped formation siblings (types `77h` and the
   `controller+4F8h` walk).

## no_ghidra_function

none: every address this packet read, named or reconstructed already has a Ghidra function, and
each body range is cited above and in `reports/lua_binding_navigator.json`.
