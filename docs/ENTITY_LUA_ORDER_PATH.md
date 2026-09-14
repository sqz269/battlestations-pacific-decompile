# From a Lua order to a plane's control axes: the whole path, and where the host stops

Addresses: `00928A00`, `0077E830`, `008A4C90`, `008A4150`, `008A4590`, `0077D600`, `0077C2A0`,
`00780120`, `00816E30`, `0071ECF0`, `00721A40`, `008358D0`, `0071E6C0`, `0071BE40`, `0099A170`,
`0099ACD0`, `00888AA0`, `0088A810`, `00D09EC0` / `00D09F58` / `00CFDA40` / `00D0BD98` (the four
controller vtables).

Worker `agent/cc7-entity-lua-order-path`, 2026-09-14 UTC. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Ghidra was **read-only**: no rename, comment, prototype,
tag, write lock or save. Descriptive names are hypotheses, not recovered symbols.

## First, a correction to my own previous packet

`docs/PILOT_BOT_PLAN_CONTROLS.md` published a 25-row table as "the complete caller set" of
`0077D600 BSP_Entity_IssueCommand`, taken from `python tools/bsp.py ghidra callers`. **It is not
complete. The real count is 62 call sites**, found by scanning `.text` for `E8`/`E9` rel32
branches landing on the target plus every absolute dword reference — the same method I had
already used two packets running for `008358D0`, and did not apply here.

Two conclusions in that document are wrong as a result:

* It said `PilotSetTarget`, the most-used binding in the shipped scripts by an order of
  magnitude, was absent from the caller set. **`008A4C90 PilotSetTarget` calls `0077D600` at
  `008A4EAC`**, and so do `008A4150 PilotMoveTo` (`008A42A7`), `008A4590 PilotMoveToRange`
  (`008A472A`) and six more of the `008A4xxx`/`008A7xxx` binding cluster.
* It said "no planner address appears in `0077D600`'s caller list, so the group-command-to-unit-
  command hand-off is unestablished", and offered that as the next thing to settle.
  **The planners are in the list**: `009FFEB0` (`009FFF09`), `00A02020` (`00A0216A`),
  `00A11FF0` (`00A120EB`), `00A13B60` (`00A14A6E`), `00A14DD0` (`00A14EA7`), and `00A2F6F0`
  (`00A3165D`) in the group-think range. The native AI issues orders through the same single
  entry point the mission scripts use. There was no gap; I manufactured one out of a short list.

`ghidra callers` under-reports whenever the caller is in an undisassembled body or reaches the
target through a vtable. It is a starting point, never a census.

That said, the lead's own finding stands and is not affected: 197 shipped script files calling
`PilotSetTarget` is direct evidence that much of the plane AI is authored in Lua. Both things are
true — the native planners issue orders *and* the scripts do, through one entry point.

## The whole path, end to end

Every hop below is named, and all but two are already reconstructed somewhere in this repository.
The thing that was missing was not any single hop but the **shape**: there are **two session
message hops**, not one, and both must be delivered locally before an order reaches a director
slot.

```
Lua: PilotSetTarget(obj, target) / PilotMoveTo / NavigatorAttackMove / ...
  00888AA0 BSP_ObjectHandle_FromLuaTable      reads `Ptr` off the argument table -> the entity
  0088A810 BSP_LuaObject_ReadCommandTarget    builds the 18h-byte SceneCommandTarget
  007EEC50 BSP_Unit_ChooseAttackCommand       (PilotSetTarget only) picks the attack class
      |
  0077D600 BSP_Entity_IssueCommand            62 call sites; retarget, late-resolve, AI-group
      |                                       forward, then BUILD AND ROUTE a message
      |   007798D0 builds MT_COMMAND, type byte 58h, vtable 00D03630
      |   0077C2A0 BSP_Session_RouteMessage
      v  ---- message hop 1 ----
  00780120 BSP_Session_DispatchEntityMessage  local delivery
      |   at 0078061C, category 58h ->
  00816E30 BSP_UnitInstance_ApplyEntityCommand
      |   a ten-singleton cascade that decides WHICH command the order becomes.
      |   It writes no director slot (docs/ENTITY_COMMAND_ARMS.md's headline).
      v
  0071ECF0 BSP_WeaponDirector_IssueCommand    forwards to the AI group via 00A2BD90, then
      |                                       ROUTES A SECOND MESSAGE, MT_GAMEUNIT_SETCMD
      v  ---- message hop 2 ----
  00780120 BSP_Session_DispatchEntityMessage  local delivery again
      |   at 00780653, category 5Ch ->
  00721A40 BSP_WeaponDirector_ApplyGameUnitMessage
      |   its 5Ch arm is the first thing that writes a slot, through
  008358D0 BSP_WeaponDirector_SetCommand  (vtable +60h)  -> 0071E6C0 PushCommandSlot
      |   slot at controller+54h + 1Ch*i
      v
  0071BE40 BSP_WeaponDirector_CurrentCommand
  0099A170 BSP_Bot_InstallCommandTask         -> one of 13 BotTask_Make* -> bot+58h
  0099ACD0 BSP_PilotBot_Tick                  -> 0099D300 -> slew -> 007B8C90
  007BB6E0 quantiser -> 007B9770 latch -> the flight law
```

`00780120 BSP_Session_DispatchEntityMessage` is therefore **the single host-side requirement for
the whole path**: both hops pass through it, and neither the command cascade nor the slot write
can run until it delivers.

### The command record

The order's payload is the `18h`-byte `SceneCommandTarget` descriptor, built from Lua by
`0088A810` and copied by value at every hop. `0077D600` takes the caller's copy, makes a **local**
copy at `0077D623..0077D66A` and never writes the caller's (`src/entity_orders.cpp`), retargets
the local copy when the resolved object answers the retarget class, and hands the **caller's**
copy — not the retargeted local — to the AI group. `00816E30` re-materialises it from the
message's `+24h..+38h` into a fresh stack descriptor at `00816E52..00816E96`. So the descriptor's
lifetime is per-hop and by value; nothing holds a pointer into a caller's frame.

The "command" itself is not a record but a **singleton**: one of the 26 class descriptors at
`00E08Exx`/`00E08Fxx` (`docs/SCENE_COMMAND_TYPES.md`, `docs/COMMAND_CLASSES.md`), each four
constant getters. The ordinal travels in the message at `+20h` and the flags byte at `+21h`.

### The controller vtable slot

`008358D0 SetCommand` has **no direct call anywhere in the image** — two absolute references only,
`00D09F20` and `00D09FB8`. `00720CD0 IssueTargetCommand` sits at four slots (`00CFDA98`,
`00D09F18`, `00D09FB0`, `00D0BDF0`) and `docs/COMMAND_EXECUTION.md` fixes it at vtable `+58h`, so
the four controller vtables are `00CFDA40`, `00D09EC0`, `00D09F58` and `00D0BD98` and the slot
that stores a command is `+60h`. Worth knowing for a host: **`+60h` is not uniform.** In two of
the four tables it is `008358D0`, which does extra work and then calls `0071E6C0`; in the other
two (`00CFDAA0`, `00D0BDF8`) it is `0071E6C0 PushCommandSlot` directly.

## (1) The entity/Lua attach, `00928A00`

**The native side is already fully recovered and the host already implements the table.** This
packet found no gap in either, so I have written no new code for it.

`docs/MISSION_ENTITY_LUA_ATTACH.md` marks `00928A00`'s body `coverage: complete` and establishes
the shape that matters: it is **virtual slot 39 (`vtable+9Ch`)**, not a call graph — `00CE6290+9Ch`
is `00928A00` itself and `0077E830` is a derived override that calls the base first. Every class
whose slot 39 is one of ten functions gets a self table; 43 vtables carry one.

What the Lua side receives, per entity, in `thisTable[<decimal id>]`:

| field | value | note |
| --- | --- | --- |
| `ID` | the key **text**, not a number | `00928BA5` through `00B67630`'s `lua_pushlstring`; shipped helpers index `thisTable[Obj.ID]`, and a number key is a different key |
| `Dead` | boolean false | |
| `Ptr` | lightuserdata(the entity object) | what `00888AA0` reads back to get the acting entity |
| `Class` | the installed `VehicleClass` row | added by a per-kind setter through `00B675D0` |

`src/game_hosts_lua.cpp:1310 attach_scene_entities_00928a00` already builds all four. **Its
`log_.unimplemented("MissionLua::entity_lua_attach", "00928a00")` at line 1368 is misleading** —
the function does the work; what it cannot supply is a real `Ptr`.

### The one real gap here: two disjoint entity worlds

The host has two kinds of entity and only one of them is reachable from an order:

* `GameScriptEntity` records, whose `Ptr` is the record's own address, which
  `GameScriptOrdersHost::entity_at` resolves through `script_entity(raw)`.
* created scene instances, whose `Ptr` is **the entity's own id cast to a pointer**
  (`game_hosts_lua.cpp:1340`). `script_entity()` cannot resolve that, so `entity_at` falls through
  to `entity_from_argument`.

So `GenerateObject("JudySpawn1")` followed by `PilotSetTarget(obj, …)` names an entity the order
host cannot turn into a unit. **This is the host's own modelling split, not a native behaviour**:
natively there is one entity and `Ptr` is it. Joining the two — giving created scene instances a
resolvable handle — is the concrete fix, and it is smaller than reconstructing anything.

## (2) The order bindings

Also largely recovered already. `0077D600` is reconstructed in full in `src/entity_orders.cpp`
(`entity_issue_command_0077d600`) with `docs/ENTITY_ORDER_MESSAGE.md` as its evidence; `0071ECF0`
is reconstructed complete in `src/cruise_command.cpp`; `00816E30`'s cascade is recovered in
`docs/ENTITY_COMMAND_ARMS.md`.

The binding cluster that reaches `0077D600`, with the call site in each:

| binding | address | call site |
| --- | --- | --- |
| `PilotMoveTo` | `008A4150` | `008A42A7` |
| (unnamed) | `008A4300` | `008A4532` |
| `PilotMoveToRange` | `008A4590` | `008A472A` |
| (unnamed) | `008A47B0` | `008A4907` |
| `PilotCloseToShip` | `008A4960` | `008A4AB7` |
| `PilotSetTarget` | `008A4C90` | `008A4EAC` |
| `PilotBomb` | `008A4F00` | `008A507F` |
| `PilotGunFire` | `008A50D0` | `008A52B6` |
| `PilotTorpedo` | `008A5310` | `008A5467` |
| `NavigatorMoveToPos` / `DirectMoveToRange` / `MoveToRange` / `AttackMove` / `Cruise` | `008A2BC0` / `008A2D70` / `008A2F20` / `008A30D0` / `008A75C0` | `008A2D17` / `008A2EC7` / `008A3077` / `008A3273` / `008A7729` |

`PilotSetTarget` is the only one of these that also calls `007EEC50 BSP_Unit_ChooseAttackCommand`
and `00521EA0 BSP_CommandTarget_ResolveObject` — it picks the attack class at issue time, using
the same chooser `0099A170` uses later. `coverage: unread` — I did not read its body.

## The wiring contract

What the host must build, in order. Nothing below asks for a new reconstruction; every hop is
already recovered, and the work is wiring.

1. **Give created scene instances a resolvable `Ptr`.** One entity identity, resolvable by both
   `GameScriptOrdersHost::entity_at` and `argument_ptr_field`. Until this holds, no order can name
   a scene entity and nothing further can be tested.
2. **Deliver session messages locally — `00780120 BSP_Session_DispatchEntityMessage`.** Both hops
   need it. Category `58h` must reach `00816E30`; category `5Ch` must reach `00721A40`.
3. **`00816E30`'s cascade** to choose the singleton, then `0071ECF0` to forward and route hop 2.
4. **`00721A40`'s `5Ch` arm** through vtable `+60h` to write `controller+54h + 1Ch*i`. This is the
   first hop that stores anything a director can be asked for. Its status is
   `partial_projection`, the weakest link in the chain and the one I would reconstruct first.
5. Only then do `0071BE40` → `0099A170` → `0099ACD0` have anything to work with.

`src/game_hosts_script_orders.cpp:326` already logs that the path "stops one hop short of a weapon
director slot". That note is right, and step 4 names the hop.

## Can the host run the shipped scripts?

Partly answered, and I did not get to the end of it. The bindings are registered
(`src/mission_lua_host.cpp` carries `PilotSetTarget` `008A4C90`, `PilotMoveTo` `008A4150`,
`PilotMoveToRange` `008A4590` among them) and `GameScriptOrdersHost` implements the argument
readers, so a script call reaches `entity_issue_command`. What I did **not** establish: whether
the mission Lua state actually loads `scripts/global/commandhelpers.lua`, and whether any shipped
mission's order call has been executed end to end. No order path has been exercised past step 1
above, so the honest answer is that it has not been demonstrated.

## Coverage and status

* **New this packet**: the corrected 62-site census of `0077D600`; the two-message-hop shape of
  the path; the `+60h` slot's non-uniformity across the four controller vtables; the host's
  two-entity-world gap.
* **Carried from existing work, not re-derived**: `00928A00`'s body and field list; `0077D600`'s
  body; `00816E30`'s cascade; `0071ECF0`; the controller slot layout.
* **Reconstructed / build-tested / validated**: **nothing this packet**. Every native hop on this
  path is already reconstructed in the repository, so new C++ from me would have duplicated it;
  the remaining work is host wiring in files I do not own. Nothing needs registering in
  `cmake/startup.cmake`.
* **Unread**: `008A4C90 PilotSetTarget`'s body; `00721A40`'s `5Ch` arm beyond its partial
  projection; whether the two unnamed bindings `008A4300` and `008A47B0` are `Pilot*` rows.
* **Corrected**: `docs/PILOT_BOT_PLAN_CONTROLS.md`'s caller table and its "no planner issues
  commands" conclusion. Both are wrong; this document supersedes them.
