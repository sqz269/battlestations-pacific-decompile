# Airbase and carrier air operations

Addresses: `006BCD20`, `006CA770`, `006BF100`, `006BF230`, `006BF330`, `006BD3F0`, `006BC6F0`,
`006CD350`, `006CDC70`, `006C65B0`, `006CADD0`, `004E96D0`, `006EBD30`, `006EC070`, `006EC140`,
`006EC8E0`, `006ECB50`, `00953920`, `009539A0`, `00953A60`, `00960230`, `007C1D80`, `007ED650`,
`007F1B70`, `007F3970`, `0089F080`, `008A20E0`, `00891D50`, `00892860`, `00892AA0`, `00892C30`,
`00892DE0`, `00895ED0`, `00896750`, `00896A90`.
Vtables `00CFAAB8` (MCatapult), `00CFE0A8` (the base gun).

Packet `cc2_air_operations`, worktree `agent/cc2-air-operations`. Ghidra was read-only. Every
descriptive name is a hypothesis, not a recovered symbol. The exceptions, which are recovered
strings from the image, are the four `Catapult` Lua keys, the eleven property-bag keys the catapult
launch writes, and the four catapult field names the debug dump prints.

There are **two unrelated launch mechanisms** and conflating them is the trap this doc exists to
prevent:

| | Airbase / carrier | Ship catapult |
| --- | --- | --- |
| Holder | an air-operations block on the entity | the gun object plus ten slots on the unit |
| Stock | `std::list` of `{class, count}` on the block | one integer at unit+`638h` |
| Launch unit | a slot that launches a whole squadron | one plane per shot, through gun `Fire` |
| Script surface | `AddAirBaseStock`, `SetAirBaseSlot`, `LaunchAirBaseSlot` | `SetCatapultStock`, `ShipUseCatapult` |

## 1. Where the air-operations block lives

`006BCD20` is the accessor every airbase binding starts with. It is a class test and a fixed
offset, nothing more:

| Class test | Class | Block |
| --- | --- | --- |
| `vtable[5Ch](9)` at `006BCD2C` | `MMothership` (carrier) | `owner+1188h` |
| `vtable[5Ch](45h)` at `006BCD3B` | `MAirfield` | `owner+72Ch` |
| neither | - | null |

Independent corroboration: `007F1B70` walks the scene's carrier list and airfield list and reaches
the same two offsets after the same two class tests (`007F1BA7 LEA ECX,[ESI+1188h]`,
`007F1BE7 LEA ECX,[ESI+72Ch]`). The class ids come from `docs/ENTITY_CLASS_IDS.md`, which pairs
`09h` with `MMothership` and `45h` with `MAirfield`.

`docs/UNIT_INSTANCE_SUBOBJECTS.md` documents a different `+72Ch`, the owned-ref slot of the ship
layout. `MAirfield` is not a ship. The two share an offset and nothing else.

`006BCD20`'s second parameter is a filter, not a mode: when it is set the block is refused unless
the owner at `block+7Ch` exists and its byte at `+5Dh` is clear. That byte is **contract: unread**.

### The block's fields

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+0Ch` | critical section guarding the list and the slot array | `006CD35F`, `006BD403` |
| `+40h` | `std::list` of stock entries (`+44h` head, `+48h` size) | `006CA78C`, `006BF109` |
| `+4Ch` | slot array base | `006CD3A0`, `00896874` |
| `+50h` | live slot count | `006BD41B`, `006C65B4` |
| `+54h` | slot capacity | `006CAF6A` grows it as `2n+2` |
| `+58h` | plane limit for the whole base | `006CD3D0`, `006CB0BD` writes it from the scene bag |
| `+7Ch` | owning entity, the receiver of every message | `006CA883`, `006CA899` |

## 2. The plane stock

### The record

One list node per vehicle class. The payload is the 12 bytes built on the stack at
`006CA7E8`-`006CA7F3` and copied in by the node allocator `006BFAE0`.

| Offset in node | Offset in payload | Field | Evidence |
| --- | --- | --- | --- |
| `+0h` | - | `next` | list node |
| `+4h` | - | `prev` | list node |
| `+8h` | `+0h` | vehicle class pointer | compared at `006CA7A8`, written at `006CA7EF` |
| `+Ch` | `+4h` | count | added at `006CA847`, read at `006CA86B` |
| `+10h` | `+8h` | the literal `5` | written at `006CA7F3`, never read by any routine read here |

The third word is **contract: unread**. It is stored on insert and no reader in this packet's
coverage loads it.

### `006CA770`, the add

`__thiscall(block, VehicleClass* class, int count)`, `RET 8`.

1. Walk the list for a node whose `+8h` equals the class (`006CA7A0`-`006CA7AF`).
2. Miss: `push_back {class, count, 5}` (`006CA7FB` into `006BFAE0`).
3. Hit: `node->count += count` (`006CA847`).
4. Build a session message from the node's class and count (`006CA87E` into `006BD9E0`) and send
   it to every non-local peer (`006CA891` into `0077C7B0`). The stock is replicated, not recomputed.
5. Notify with the per-category total: `006BF100` sums every entry whose `class+70h` matches this
   class's `class+70h` (`006CA8AF`), and `00984EB0` receives `(owner, class+70h, total)`
   (`006CA8C0`). `00984EB0` holds the string `stock` and reaches the warning channels and the
   mission Lua host. What `class+70h` spells is **contract: unread**; it groups several classes, so
   it is a category key rather than the class id.

### The two producers

- **Script**: `AddAirBaseStock` / `AddAirBasePlanes`, handler `00896A90`, both names one entry
  point (`docs/LUA_BINDING_ALIASES.md`). Entity from argument 0, `006BCD20`, then
  `BSP_VehicleClass_GetOrCreate 00964790` over argument 1, `BSP_VehicleClass_MarkPartyRequired
  0095BA60`, `00964790` again, then `006CA770` at `00896C54` with the class and argument 2.
  The shipped missions call it exactly that way: `AddAirBaseStock(Mission.Enterprise, 101, 40)`
  in `scripts/missions/usn/usn_1_marshall.lua:809`.
- **Scene**: `004E96D0 BSP_SceneUnit_RegisterLaunchStock`, already reconstructed in
  `docs/SCENE_UNIT_CREATORS.md`; it reads `LaunchClassID` and
  `VehicleClass.<name>.Catapult.LaunchedClass` from the scene bag and marks the party.

### Queries over the stock

| Routine | Rule |
| --- | --- |
| `006BF100` | total count for one category key (`class+70h`) |
| `006BF230` | pair `{available, total}`: the class total minus what the slots already hold |
| `006BF330` | `*006BF230(...)`, the available count only; this is what the launch rule uses |
| `006BD3F0` | planes already committed: per slot, the launched squadron's `entity+3CCh` if it has one, else the slot's `+8h` |

## 3. The launch

### 3a. Airbase and carrier: slots

A slot is `58h` bytes; the array is indexed from script as `(n - 1) * 58h`, so **script slot
numbers are 1-based** (`00896874` area, `00896115` area).

| Offset | Field | Producer |
| --- | --- | --- |
| `+4h` | vehicle class, 0 when empty | `006BC6F0` |
| `+8h` | assigned count | `006BC6F0`, cleared by `006C65B0` |
| `+Ch` | requested count, the per-slot cap | read at `006CD40B` |
| `+10h` | copy of `class+134h` | `006BC6F0` |
| `+28h` | launched squadron entity | `006C65C5` clears it, `00896990` reads it |
| `+2Ch` | state | `006CD3A8`, `006C65CF` |
| `+30h` | timer, float seconds | `006C65D5` |
| `+34h` | launch-requested byte | set by `00896925` area, cleared at `006CD41F` |

States seen: `1` cooldown, `2` launching (a slot in this state refuses a new launch), `5` ready.
Other values are **contract: unread**.

The launch decision, `006CD350` at `006CD3A8`-`006CD434`, every constant present:

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `006CD3A8` | only a slot in state `5` launches |
| 2 | `006CD3D7` | `room = block+58h (plane limit) - 006BD3F0(block)` |
| 3 | `006CD3E1` | `room = min(room, 006BF330(slot class))`, the free stock for that class |
| 4 | `006CD40B` | `room = min(room, slot+Ch)`, the slot's own request |
| 5 | `006CD40F` | state goes to `1`; the timer is `0`, or `5.0` (`00CE3850`) when the launch byte was set, which is then cleared |
| 6 | `006CD425` | `room >= 1`: `006BC6F0(slot, class, room)` assigns the count |
| 6' | `006CD414` area | `room < 1`: `slot+4h` and `slot+10h` go to 0 and `slot+8h` is cleared |

`LaunchAirBaseSlot` (`00896750`) does not launch anything itself. It refuses a slot already in
state `2`, routes command message **`83h`** carrying `{slot - 1, bool}` (`00896915` builds it,
`00896964` routes it), sets the launch byte, then runs the air-operations update once
(`00896983` into `006CDC70`) so the squadron exists before the binding returns `slot+28h` to the
script through `thisTable[entity+174h]`. `ShipUseCatapult` (`00891D50`) is the same shape with
message **`7Bh`** and a float (`00891F05`, `00891F48`). Both bindings are therefore *requests*:
the authoritative launch happens in the update.

`006CDC70` is the air-operations update itself; it is a sequence of nine sub-updates behind the
game-state gate `*(00E188A8 + 1FE4h) != 2`. Its members are **contract: unread** except
`006CD810`, `006CD240` and the ones named elsewhere in this doc.

`SetAirBaseSlot(entity, slot, classId, count[, equipment])` (`00895ED0`) fills a slot: class 0 when
the count is 0, `00964790` otherwise, `006CD350` at `00896115`, a state-5 fixup through `006CCDA0`,
then a replication message from `006BD800`. The shipped call
`SetAirBaseSlot(this.JapAirfields[1], 3, 167, 3, 2)`
(`scripts/missions/chg/chg_6_forcez.lua:90`) matches the five-argument form.

### 3b. The ship catapult: `MCatapult`, class id `28h`

`MCatapult` is a gun subclass whose `Fire` spawns a squadron instead of a projectile. Its vtable
`00CFAAB8` differs from the base gun vtable `00CFE0A8` in fourteen slots; the catapult's vtable ends
at slot `1F0h` (the bytes after it are the inline property names `ResourceUsage`, `Behaviour`,
`PlaneParentID`).

| Slot | Catapult | Base gun | Role |
| --- | --- | --- | --- |
| `0h` | `006EC840` | `0072DD00` | destructor |
| `5Ch` | `006EC7B0` | `006E3D50` | class test, id `28h` |
| `A8h` | `006EC140` | `0072ADC0` | debug dump, prints `_catapult` |
| `1D0h` | `006EBD30` | `00729A80` | can-fire gate |
| `1D8h` | `006EC8E0` | `00730160` | `Fire`, the launch |
| `1F0h` | `006ECB50` | - | apply the launch message on a remote client |
| `9Ch`, `A0h`, `A4h`, `DCh`, `164h`, `1BCh`, `1D4h`, `1DCh`, `1E8h`, `1ECh` | | | overridden, **contract: unread** |

Instance fields, the first four named by the debug dump `006EC140` itself:

| Offset | Name | Evidence |
| --- | --- | --- |
| `+3B8h` | disabled byte | `006EBD36` |
| `+3F0h` | owning unit | `006EC8FF` |
| `+3F4h` | weapon descriptor; `+E0h` is the launch interval in seconds | `006EC07F` |
| `+414h` | pointer to the reload timer; `> 0` blocks the launch | `006EBD57` |
| `+480h` | `ammo` | `006EC140`, decremented at `006EC09B` |
| `+484h` | `orgAmmo` | `006EC140` |
| `+488h` | `startLaunch`, a bool | `006EC140` |
| `+48Ch` | launch time from the message | `006ECBC1` |
| `+490h` | `launchTick` | `006EC140`, written at `006EC076` |
| `+494h` | observer pair block, pointer at `+4A8h` | `006ECAA1`, `006ECAC3` |
| `+4A8h` | the squadron this catapult launched | `006ECAF2` |
| `+C8h`/`+CCh` | node matrix valid byte, then the `4x4` world matrix | `006ECA5B`-`006ECA7D` |

The class-side block is read by `00960230` from the `Catapult` sub-table (key `00CE46F8`) and is
already reconstructed as `VehicleClassFieldOffsets` in `include/bsp/vehicle_class_fields.hpp`:

| Lua key | String | Offset | Default | Site |
| --- | --- | --- | --- | --- |
| `LaunchedClass` | `00D1AC88` | `+C0h` | `-1` | `009604C6` |
| `LaunchStock` | `00D1AC7C` | `+C8h` | `0` | `00960503` |
| `MaxLaunchedPlanes` | `00D1AC68` | `+CCh` | `0` | `00960540` |
| `Equipment` | `00CF69AC` | `+C4h` | `0` | `0096057D` |

Missing block: `+C0h = -1`, `+C8h = 0`, `+CCh = 0` (`00960592`). The shipped data table has all
four together, which is the independent check on the grouping:
`["Catapult"] = { ["Equipment"] = 1, ["LaunchedClass"] = 121, ["LaunchStock"] = 10,
["MaxLaunchedPlanes"] = 1 }` in `scripts/datatables/autoload/vehicleclasses.lua:2640`.

Unit-side catapult state:

| Offset | Field | Evidence |
| --- | --- | --- |
| `+530h` | launched-class override, `-1` takes the class default | `006EC905` |
| `+534h` | equipment override, `-1` takes the class default | `006EC91D` |
| `+550h`, stride `18h`, ten entries | launch slots | `00953920` tests `550h`..`628h` |
| `+630h` | index of the last catapulted slot, `-1` when none | `00953A60` |
| `+638h` | catapult stock, clamped to `[0, class+C8h]` | `009539A0` |

`GetCatapultStock` (`00892DE0`) returns `max(entity+638h, 0)` for a class-`5` entity;
`SetCatapultStock` (`00892C30`) calls `009539A0` at `00892D7B`; `GetNumCatapulted` (`00892AA0`)
calls `00953920` at `00892BCB`; `GetLastCatapulted` (`00892860`) calls `00953A60` at `00892993`.

### The launch sequence, `006EC8E0`

Ghidra has no function at `006EC8E0`; the body `006EC8E0`-`006ECB40` (`RET 0Ch`, single epilogue)
was decoded from the raw bytes with capstone.

| # | Site | Step | Constant |
| --- | --- | --- | --- |
| 1 | `006EC8FF` | owner = `this+3F0h` | |
| 2 | `006EC905` | launched class = `owner+530h`, or `ownerClass+C0h` when negative | `LaunchedClass` |
| 3 | `006EC91D` | equipment = `owner+534h`, or `ownerClass+C4h` when negative | `Equipment` |
| 4 | `006EC935` | `007B8A80(launchedClass)` -> the launched class record | |
| 5 | `006EC93C` | name = `record+13Ch`, or the empty string `00E199AE` | |
| 6 | `006EC958` | construct the property bag (`008F41A0`) | |
| 7 | `006EC963`-`006ECA56` | fill it, eleven keys, below | |
| 8 | `006ECA5B` | refresh the node matrix (`00414DB0`) when `this+C8h` is clear | |
| 9 | `006ECA71` | copy the `40h`-byte world matrix from `this+CCh` | |
| 10 | `006ECA96` | `004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen(launchedClass, name, 0, &matrix, &bag, 0)` | |
| 11 | `006ECABA` | store the squadron at `this+4A8h` and bind the observer pair (`00694A60`) | |
| 12 | `006ECACA` | `operator new(0Ch)`, construct from the bag (`00922E20`), store at `squadron+C0h` | |
| 13 | `006ECB0F` | `006EC070(this, *00F876B0)`: `launchTick`, barrel timer, `ammo -= 1` | |

The eleven property-bag keys, all recovered strings:

| Key | Address | Value | Site |
| --- | --- | --- | --- |
| `Type` | `00CE4780` | the launched class | `006EC95D` |
| `WingCount` | `00CF8840` | `1` | `006EC977` |
| `Skill` | `00CF8838` | `owner->vtable[12Ch]()` | `006EC98D` |
| `Race` | `00CE8EE0` | `owner+58h` | `006EC9A6` |
| `Party` | `00CE5804` | `owner+54h` | `006EC9BE` |
| `OwnerPlayer` | `00CF882C` | `owner+188h` | `006EC9D6` |
| `State` | `00CF8818` | `3` | `006EC9F1` |
| `PlaneParentID` | `00CFACC8` | `owner+174h`, u16 | `006ECA01` |
| `Equipment` | `00CF69AC` | the equipment resolved in step 3 | `006ECA20` |
| `Behaviour` | `00CFACBC` | `1` | `006ECA2F` |
| `ResourceUsage` | `00CFACAC` | `(float)(record+124h)` | `006ECA3F` |

So the catapult spawns through the **same scene creator** the mission loader uses for a
`PlaneSquadronGen`; the difference is only that the property bag is synthesised in code and the
placement matrix is the catapult node's own.

The can-fire gate `006EBD30`, in order: `this+3B8h` clear, `this+480h > 0`,
`00953920(owner) < ownerClass+CCh`, `*(this+414h) <= 0`.

The interval, `006EC070`: `launchTick = tick`, then
`SetBarrelReloadTimer(class+E0h - (now - tick) * 00D0DE84)` and `ammo -= 1`. Subtracting the
elapsed time is what makes the remote path (`006ECB50`, vtable slot `1F0h`) land on the same
timer as the local one: that handler resolves the plane entity from the message's u16 at `+20h`,
stores it at `this+4A8h`, copies the float at `+24h` to `this+48Ch` and calls `006EC070` with the
message's tick at `+Ch`.

### Who decides to launch

Established: the mission script, through `LaunchAirBaseSlot` / `ShipUseCatapult`, both of which
route a command message and then let the update act. The AI spawn-hint path of
`docs/LUA_BINDING_AI.md` and any per-tick launcher inside `006CDC70` are **contract: unread**.

## 4. Recovery and rearm

### `008A20E0`, `SquadronLandAndKill`

`__fastcall(lua_State*)`. Argument 0 must answer the class test with `18h` (`PlaneSquadronGen`).

1. `008A220F` -> `007F1B70`: walk the scene's carrier list (`game+19CCh` then `+88h`) and airfield
   list (`+358h`), class-test each with `9` and `45h`, and call `006C65B0(block, squadron)`.
2. `006C65B0` finds the slot whose `+28h` is this squadron, unregisters the observer, clears
   `+28h`, sets `+8h = 0`, `+2Ch = 1`, `+30h = 5.0` and `+34h = 0`. **Landing frees the slot and
   starts its 5-second cooldown; it does not return anything to the stock list.** The stock is
   restored only in the sense that `006BF330` counts what the slots no longer hold.
3. `008A2214`-`008A225B`: walk the planes from the last index down. `008A2232` bounds the index at
   4, so above five entries the plane pointer becomes 0 before the calls.
4. Per plane: `00926D90 BSP_MissionEntity_Kill(plane, 5)` at `008A223F`, then
   `007F3970(squadron, plane, 1)` at `008A2249`.
5. `007F3970` compacts the plane array at `squadron+3D0h`, decrements `squadron+3CCh`, clears the
   plane's back pointer at `plane+9D4h`, sets the dirty byte `squadron+3ECh`, and on the last
   plane kills the squadron and writes `squadron+36Ah` (`0` on this path, because the landing
   call always passes a non-zero flag).

The squadron's plane array is the generic child array already declared as
`kUnitOffDeviceRootCount` `3CCh` / `kUnitOffDeviceRootArray` `3D0h` in
`include/bsp/unit_weapons.hpp`; `kSquadronPlaneOffset` in
`include/bsp/local_player_unit_lists.hpp` names the same `3D0h` for a squadron. Nothing new is
declared for it here.

### `0089F080`, `PlaneReloadBombPlatforms`

The handler is a wrapper: entity from argument 0, then `007ED650` at `0089F198`, and nothing else.

`007ED650(squadron)`: for each of the `squadron+3CCh` planes at `squadron+3D0h`, call
`007C1D80(plane)`; then set `squadron+3ECh = 1`.

`007C1D80(plane)`: walk the plane's device list from `plane+48h` (next at `device+44h`), and for
every device that answers the class test with `25h` call `device->vtable[200h]()`. The class test
walks the ancestor chain, so `25h` selects `MBombPlatform` **and** `MMultipleBombPlatform` `26h`
(`docs/WEAPON_CLASS_DESCRIPTOR.md`). The reload body behind `vtable[200h]` is
**contract: unread**.

So rearm is per plane and per bomb platform, and it is independent of the airbase stock: nothing in
this path touches the stock list or a slot.

### The landing approach

The approach itself is the pilot bots' work. `007F16D0 BSP_Plane_ResolveReturnToBase` uses
`006BCD20` to find the base, which is the contract boundary: this packet establishes what landing
does to the slot and to the squadron, not how a plane is flown onto the deck.
`docs/UNIT_COMMAND_PRODUCERS.md` owns the pilot bots. **contract: unread**.

## Host table

One row per native call site the reconstruction turns into a host method.

| Site | Callee | Host method | this / args | Returns | Gate |
| --- | --- | --- | --- | --- | --- |
| `006CA87E` | `006BD9E0` | `build_stock_message` | stack buffer; class, count | message | always |
| `006CA891` | `0077C7B0` | `send_message_to_peers` | `block+7Ch`; message, 0 | void | always |
| `006CA8AF` | `006BF100` | (pure rule) | block; `class+70h` | total | always |
| `006CA8C0` | `00984EB0` | `notify_stock_changed` | `00F8A0C4`; owner, `class+70h`, total | void | always |
| `00896915` | `0075B430` | `route_command_message` (`83h`) | stack message | void | slot state != 2 |
| `00891F0B` | `0075B430` | `route_command_message` (`7Bh`) | stack message | void | always |
| `00896983` | `006CDC70` | `run_air_ops_update` | block | void | slot state != 2 |
| `006ECA96` | `004F0AD0` | `create_plane_squadron` | launched class; name in EDX, 0, matrix, bag, 0 | squadron | `Fire` |
| `006ECAC3` | `00694A60` | `bind_launched_squadron` | squadron; `this+494h` | void | squadron != 0 |
| `006ECAE9` | `00922E20` | `attach_spawn_record` | new `0Ch` block; bag | record | allocation != 0 |
| `006EC07F` | `BSP_Gun_SetBarrelReloadTimer` | `set_barrel_reload_timer` | catapult; seconds, 0 | void | always |
| `0089F198` | `007ED650` | (loop below) | squadron | void | always |
| `007ED667` | `007C1D80` | `reload_plane_bomb_platforms` | plane | void | per plane |
| `007F1BAD` | `006C65B0` | `release_squadron_slot` | `entity+1188h`; squadron | void | class test `9` |
| `007F1BED` | `006C65B0` | `release_squadron_slot` | `entity+72Ch`; squadron | void | class test `45h` |
| `008A220F` | `007F1B70` | (the two rows above) | squadron | void | class test `18h` |
| `008A223F` | `00926D90` | `kill_plane` | plane; reason 5 | void | per plane |
| `008A2249` | `007F3970` | (pure rule) | squadron; plane, 1 | void | per plane |
| `006CD3D7` | `006BD3F0` | (pure rule) | block | committed | state 5 |
| `006CD3E1` | `006BF330` | (pure rule) | block; class | free stock | state 5 |
| `006CD425` | `006BC6F0` | (pure rule) | slot; class, count | void | count >= 1 |
| `00896C54` | `006CA770` | (the stock add) | block; class, count | void | entity resolved |
| `00896115` | `006CD350` | (the launch rule) | block; slot, class, count, equipment | void | entity resolved |
| `00892D7B` | `009539A0` | (pure rule) | unit; value | void | class test `5` |
| `00892BCB` | `00953920` | (pure rule) | unit | count | class test `6` |
| `00892993` | `00953A60` | (pure rule) | unit | entity | class test `6` |
| `00896872` | `006BCD20` | (the accessor) | entity; flag | block | always |

## Coverage

| Address | Name | Coverage |
| --- | --- | --- |
| `006BCD20` | `BSP_AirOps_GetBlock` | complete |
| `006CA770` | `BSP_AirOps_AddStock` | complete |
| `006BF100` | `BSP_AirOps_StockCountByCategory` | complete |
| `006BD3F0` | `BSP_AirOps_CommittedPlanes` | complete |
| `006BC6F0` | `BSP_AirOps_AssignSlotClass` | complete |
| `006C65B0` | `BSP_AirOps_ReleaseSquadronSlot` | complete |
| `006EBD30` | `BSP_MCatapult_CanFire` | complete |
| `006EC070` | `BSP_MCatapult_AfterLaunch` | complete |
| `006EC140` | `BSP_MCatapult_DebugDump` | complete |
| `006EC8E0` | `BSP_MCatapult_Fire` | complete (no Ghidra function; raw listing) |
| `006ECB50` | `BSP_MCatapult_ApplyLaunchMessage` | complete (no Ghidra function; raw listing) |
| `00953920` | `BSP_Ship_CountCatapultedPlanes` | complete |
| `009539A0` | `BSP_Ship_SetCatapultStock` | complete |
| `00953A60` | `BSP_Ship_GetLastCatapulted` | complete |
| `007ED650` | `BSP_Squadron_ReloadBombPlatforms` | complete |
| `007C1D80` | `BSP_Plane_ReloadBombPlatforms` | complete except `vtable[200h]` |
| `007F1B70` | `BSP_Squadron_ReleaseFromAllAirBases` | complete |
| `007F3970` | `BSP_Squadron_RemovePlane` | complete except `007F2FD0` and `007ED260` |
| `0089F080` | `BSP_LuaBinding_PlaneReloadBombPlatforms` | complete |
| `008A20E0` | `BSP_LuaBinding_SquadronLandAndKill` | complete |
| `006CD350` | `BSP_AirOps_UpdateSlot` | partial: only the state-5 branch (`006CD3A8`-`006CD434`). `006CD43C`-`006CD5xx` (states 1 and 2, and `006CC5C0`) unread |
| `006CDC70` | `BSP_AirOps_Update` | partial: the call list only; every callee unread |
| `006CADD0` | `BSP_AirOps_LoadFromScene` | partial: the slot array construction and `+58h` only |
| `00896750` | `BSP_LuaBinding_LaunchAirBaseSlot` | complete |
| `00895ED0` | `BSP_LuaBinding_SetAirBaseSlot` | partial: `006CCDA0`, `006C0F00`, `006BA4B0`, `006BD800` unread |
| `00891D50` | `BSP_LuaBinding_ShipUseCatapult` | partial: the message payload past the float is unread |

Unread bindings in the same family, listed so the next packet does not re-derive them:
`GetAirBaseStatus` `00895BA0`, `IsReadyToSendPlanes` `00895D20`, `GetAirBaseSlotStatus` `00896220`,
`SetAirBaseSlotCount` `008963E0`, `SetAirBaseSlotReady` `00896590`, `RemoveAirBasePlanes`
`008973C0`, `RemoveAllAirBasePlanes` `008975B0`, `SetAirBasePlaneLimit` `00897720`,
`GetAirBasePlaneLimit` `008978D0`, `LaunchSquadron` `0089E3C0`, `SetAirbaseSmallPlanes` `008A5A20`,
`GetAirbaseOrders` `008AE760`, `SetAirbaseOrders` `008AE900`.

## Uncertainties

- `class+70h`, the stock notification's grouping key, is unread. It is not the class id: `006BF100`
  sums several entries under one key.
- The literal `5` in the third stock-payload word has no reader in this coverage.
- Slot states other than `1`, `2` and `5` are unread, and `006CCDA0(slot, 2, 0, 9, 1)` in
  `SetAirBaseSlot` suggests at least one more transition.
- Whether anything other than a script request starts a launch is unread; `006CDC70`'s callees are
  the place to look.
- `007B8A80`, which turns the launched class id into the record whose `+13Ch` and `+124h` the
  catapult reads, was not opened.
- No run-time evidence was taken. `bsp_game.exe` does not reach a catapult launch on the paths
  `docs/GAME_EXECUTABLE.md` lists, so checklist rule 6 does not bite, but nothing here is
  game-validated either.

## no_ghidra_function

| Start | End (inclusive) | Role |
| --- | --- | --- |
| `006EC8E0` | `006ECB40` | `MCatapult::Fire`, vtable `00CFAAB8` slot `1D8h`; single epilogue `006ECB40 RET 0Ch`. Ghidra's `FUN_006EC860` swallows the range, so it was decoded from the raw bytes with capstone (`local/dis_air.py`) |
| `006ECB50` | `006ECBD4` | the launch-message handler, vtable slot `1F0h`, `RET 4` |

## Correction from docs/LAND_AND_STRUCTURES.md (packet cc2_land_and_structures)

- **Was:** the +72Ch caution names two cases, the ship's owned-ref slot and MAirfield's air-operations block
  **Is:** three cases. MLandFort's +72Ch is an owned-ref slot built by the same 00809270 as the ship's, so the ship and the fort share that subobject and the airfield does not.
  **Evidence:** 00745978 calls 00809270 with ECX = this+72Ch (LEA at 0074596C), against 0081ED7B for the ship; MCommandBuilding re-vptrs the same offset to 00CFAFD8 at 006F568D, which proves it is a base subobject and not a member
