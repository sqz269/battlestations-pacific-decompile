# `Hidden`, and the host for `GenerateObject` (`00944FD0`)

Packet `cc8_lua_generate_object`. Two commits: the scene pass holds back the entities the script is
supposed to create, and `GenerateObject` creates them.

## 1. `Hidden` is the hold-back, and it sits before the gate

The discriminator was an authored diff, not a class default. Two `MotherShipGen` entities in
`usn_19_coralus.scn`, same class, same templates `(Common, GameUnit, Ship, Command,
MotherShipPlanes)`, same `"MultiType" { }` block:

| entity | the line that differs |
| --- | --- |
| `Lexington-class01` | `--Hidden = B true ;` — commented out |
| `Zuikaku-class01` | `Hidden = B true ;` — active |

`"Hidden"` lives at `00CE5708` and has **five** references in exactly two functions,
`0046CF40 BSP_SceneFile_ReadEntityBlock` (2) and `0046BF70 BSP_SceneEntity_RegisterMultiplayerStock`
(3). The instantiate path is:

```
0046d39d: CMP byte ptr [EDI + 0x4],0x0   ; the pass flag
0046d3b3: JZ  0x0046d3cb                 ; clear -> no Hidden test
0046d3b5: PUSH 0xce5708                  ; "Hidden"
0046d3bc: CALL 0x008f2260                ; bag.find
0046d3c1: CMP byte ptr [EAX + 0xc],0x0
0046d3c5: JNZ 0x0046d5e4                 ; SET -> jumps past the creation
...
0046d426: CALL 0x0046c550                ; never reached for a hidden one
```

So a hidden entity is **registered and not created**. The registration branch still runs `0046BF70`
at `0046D531`, which reads the same string three more times, so the record stays in the scene
database's named-object map and `GenerateObject` instantiates it from there by name. That is what
stops the script's own spawn step producing a second carrier.

**It also settles a thread that went the wrong way twice.** The test is *before* `0046C550`, not
inside it, so the gate never sees a held-back entity. This host's `rejected=0` was a faithful answer
to the wrong question, and the class default for `GenerateInGame` — the previous suspect — needs no
change. `register_multiplayer_stock`'s 106 calls come from the registration branch and are
consistent with that.

### The set, checked against the scripts

| scene | entities | hidden | the names the packet named |
| --- | --- | --- | --- |
| `usn_19_coralus.scn` | 58 | **34** | `Zuikaku-class01`, `Shokaku-class01`, and all seven `luaMoveToPh3` spawns |
| `usn_1_marshall.scn` | 147 | **15** | `ScoutDauntless` |
| `usn_ormoc.scn` | 280 | **29** | `TBM_1` |

USN04's 34 are every Japanese unit plus the movie and dummy props; the Americans are exactly the
ones carrying `--Hidden`.

## 2. The pool

The executable looks a name up in the scene database's named-object map at `sceneDb+18h`
(`0046D96F`). This process has no scene database, so the instantiate pass publishes what it held back
into one process-wide `SceneSpawnPool` keyed by name — the same shape `bsp::air_ops_decks()` already
uses for the decks. An entry survives being spawned, carrying the entity id, for the reason in
section 4.

## 3. The binding

`docs/LUA_BINDING_GENERATE_OBJECT.md` had already recovered the argument shape; this implements it.

| index | type | role |
| --- | --- | --- |
| 0 | string | the authored name, the key `0046D96F` looks up |
| 1 | string | a second name, forwarded; or |
| 1 or 2 | table of three numbers | the world position |
| 2 or 3 | number | the yaw, radians |

* A name the pool does not hold creates nothing and returns nothing, which is `0046D9AF`'s answer.
* A position rewrites only the translation row, as `0046DD48` copies its three floats into the local
  frame's `+30h`/`+34h`/`+38h` and leaves the authored rotation alone.
* The yaw applies only when it is **smaller** than the double `2*pi` at `00CE3828` (`0046DD4C`). The
  `10.0f` default at `00CE38B8` therefore needs no special case at all: `10.0 > 2*pi`, so the
  sentinel fails the same comparison, which is precisely what "keep the authored orientation" means.
  Writing it as a comparison rather than an equality test is the faithful reading.
* Creation runs through the orders host, which owns the units host, on the same `descriptor[1]` path
  `0046DB4B CALL EAX` runs for a load-time entity.
* Of `0046DBE8`'s `00925F20 BSP_SEntity_InitAll`, the part this host can do is the `thisTable` slot
  that every entity reaching virtual slot 39 carries. Without it the script's own variable resolves
  to nothing, which is the whole defect. The binding pushes that slot, which is what the
  entity-returning tail `0089903C` pushes — and the script indexes the result (`Mission.Zuikaku.Dead`),
  so a table is required here, unlike the `squadron` key, which is a number the script converts.

## 4. One labelled deviation

`0046D930` has **no already-created arm**: it instantiates on every call. This process keeps the pool
entry and answers a second call with the same entity. A mission that asks twice would otherwise get
two carriers, and a duplicated capital ship is a worse failure than a repeated handle. It is a
deviation, not a reading, and it is the only one in this packet.

## 5. The sibling bindings, measured

Counted in the scripts rather than assumed:

| script | `GenerateObject` | `SpawnNew` | others |
| --- | --- | --- | --- |
| `usn_1_marshall.lua` (USN01) | 17 | — | none |
| `usn_ormoc.lua` (USN22) | 27 | 2 | none |
| `usn_19_coralus.lua` (USN04) | 34 | 14 | none |

`Spawn`, `SpawnNewIDIsRequested`, `SpawnNewIDRemove` and `SpawnLight` are called by none of the
three. **USN01 needs `GenerateObject` alone**, which makes it the clean first validation.
`SpawnNew` `0094C480` is a different mechanism — it builds a group from class types out of a Lua
table (`party`, `groupMembers` with `Type`, `Name`, `Crew`, `Race`, `WingCount`, `Equipment`), and
USN04 uses it for bomber waves including `Mission.TypeB5N`, the Kate. It is a separate packet and a
second, independent route to a torpedo aircraft.

## Uncertainty

* `0046BF70`'s three `Hidden` reads on the registration branch were not decoded; this packet takes
  only that the record survives, which the `GenerateObject` lookup requires.
* `00925F20 BSP_SEntity_InitAll` is reconstructed only as the `thisTable` slot.
* Whether a held-back entity should also be absent from the world lists and the gun census until it
  is spawned follows from not creating it, but the blast radius is large and is what the before/after
  pair below measures.

## Host methods

| method | address | coverage |
| --- | --- | --- |
| the `Hidden` hold-back | 0046D3C5 | complete for the instantiate pass |
| `GameMissionLuaHost::run_generate_object_00944fd0` | 00944FD0 | arguments, lookup, placement, yaw, table slot; one deviation in section 4 |
| `GameScriptOrdersHost::create_unit_from_scene_record_0046db4b` | 0046DB4B | the creator call, through the units host |

## no_ghidra_function

None.

## Validation

Build clean, both ctest suites pass. The before/after run pair is below; **both columns sit after
`0daec4b56` and `bb9123bc5`**, so neither is comparable to the three USN04 columns in
`docs/USN04_STRIKE_CLASS.md` on the command-target or AI-target-weight axes.
