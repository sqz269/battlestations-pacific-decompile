# Mission Lua teardown (`004DC5C0`, the host destructor, and the per-load reset)

Addresses: 004dc5c0 004dc729 008844f0 00888300 00888380 008882d0 008882b0 00887d90 008876b0 006b87c0 00b65e80 00b65e20 00884be0 004de07c 004de093 004de09c 004dfb70 004e0203 004e0225 004e029b 004e0321 00d0e7a8 00cf8394

Packet `cc2_lobby_settings`, worktree `agent/cc2-lobby-settings`. Ghidra was read-only; every
descriptive name is a hypothesis. This is the follow-up the correction in
`docs/MISSION_LUA_MACHINE.md` left open. The reconstruction is in
`include/bsp/mission_lobby_settings.hpp` and `src/mission_lobby_settings.cpp`.

## Correction: `004DC729` is not a teardown

`docs/MISSION_LUA_HOST.md` says "`004DC729` loads it and calls `00886900` on shutdown", and
`docs/MISSION_LUA_MACHINE.md` records "The teardown at `004DC729` is a different address and
remains unread". Both are wrong about the address.

`004DC729` is `MOV ECX,[EBP+1A08h]` immediately followed by `CALL 00886900` at `004DC72F`, and it
lies inside `004DC6A0` `BSP_Game_ConstructGlobalSubsystems` (Ghidra body `004DC6A0..004DC93F`,
rule 2). `00886900` is `BSP_MissionLua_RunGlobalScriptFolders`, the bring-up step already
reconstructed in `src/global_script_folders.cpp`. **There is no teardown at `004DC729`**: it is
the load path's script-folder call, reached once per scene load. The real teardown is
`004DC5C0` `BSP_Game_OnDestroy`, and the real per-load reset is inside `004DFB70`.

Previous value recorded before this correction: "The teardown at `004DC729` is a different address
and remains unread."

## The two objects and their layout

| Address | Object | Layout |
| --- | --- | --- |
| `game+1A08h` | pointer to the mission Lua host, `14h` bytes | `+0h` vtable `00D0E7A8`, `+4h` `LuaMachine*`, `+8h..+10h` the deferred-call list (`+0Ch` head node, `+10h` size) |
| `game+1A0Ch` | inline `LuaStateOwner` | `+0h` owns byte, `+4h` borrowed `lua_State*` |
| `host+4h` | `LuaMachine`, `0Ch` bytes | `+0h` vtable `00CF8394`, `+4h` `lua_State*`, `+8h` owns byte, set to 1 at `006B87B0` |

Construction, all in the game constructor `004DDB90`: `004DE07C` `operator new(14h)`, `004DE093`
`FUN_008882D0` (writes the vtable, allocates the list head node through `00884830`, zeroes the
size and the machine pointer), `004DE09C` `MOV [ESI+1A08h],EAX`. The host's vtable has a single
slot, the scalar deleting destructor `00888380`; the bytes at `00D0E7AC` are the string
`"TerminateExecuti…"`, so there is no second virtual.

`game+1A0Ch` is built later, at `004DD641` (`00B65E20`) from `[[game+1A08h]+4h]+4h`, that is from
the machine's own state. Its owns byte is therefore the borrower's, which is what makes the order
below safe.

## Teardown order, `004DC5C0` `BSP_Game_OnDestroy`

Coverage: **complete** for the Lua part, `004DC5C0..004DC634`. The five object deletes after it
(`game+21A0h`, `game+21D8h`, `00E18678`, `00E1867C`) and `004A9AC0` are listed for order only and
were not read.

| # | Site | Callee | Effect |
| --- | --- | --- | --- |
| 1 | `004DC5C9` | `004254B0` | logs `"GGame::OnDestroy()"` |
| 2 | `004DC5D1` | store | `game+5D4h = 10h`, the lifecycle state |
| 3 | `004DC5DB` | `0046F3E0` | not read |
| 4 | `004DC5E2` | `004DA780` | `BSP_Game_TeardownSessionState` |
| 5 | `004DC5E9` | `004DA650` | `BSP_FrontEnd_DestroyManagers` |
| 6 | `004DC5FF` | `[[00F8A2FC]+8h]` | indirect, then `00992AC0`; gated on `00F8A2FC != 0` |
| 7 | `004DC60C` | `00B65E80` | `BSP_LuaStateOwner_Close` on `game+1A0Ch` |
| 8 | `004DC617` | `008844F0` | releases the `LuaMachine` at `host+4h` |
| 9 | `004DC62C` | `[[host]]` (`00888380`) | deletes the host itself |
| 10 | `004DC634..004DC694` | four vtable slot 0 calls | `game+21A0h`, `game+21D8h`, `00E18678`, `00E1867C`, each null-checked and nulled |
| 11 | `004DC694`-tail | `004A9AC0` | not read |

Step 7 does **not** close the state. `00B65E80` is `if (owner+4h != 0 && owner+0h != 0)
lua_close();` then clears `owner+4h`: the inline owner borrows the state, its owns byte is 0, so
the call only drops the borrowed pointer. The doc comment on `00B65E80` already records that it
does not walk or invalidate the `LuaObject`s it handed out.

Step 8 is where the interpreter dies. `FUN_008844F0(host)` is
`if (host+4h) { (*[[host+4h]])(1); host+4h = 0; }`, and that vtable slot is `006B87C0`: it
restores the vtable, and `CMP byte ptr [ESI+8],0` then `MOV ECX,[ESI+4]` / `CALL 00A68A90`, so
**`lua_close(L)` runs exactly once, with the state in `ECX`**, gated on the machine's owns byte.
It then frees the twelve-byte machine. Note the call at `004DC617` is not null-guarded while the
one at `004DC622` is; `008844F0` would fault on a null host.

Step 9 runs `00888380` -> `00888300`, in this order:

1. `0088831D`: restore the vtable.
2. `00888323..00888340`: delete `host+4h` through its own vtable slot 0 and null it. After step 8
   this is a no-op, so the double release is safe in either order.
3. `0088834A`: `FUN_00887D90` on `host+8h` — the deferred-call list clear: reseat the sentinel's
   two links, set the size at `host+10h` to 0, and for a non-empty list run
   `BSP_MissionLuaDeferredCall_Destruct` (`008876B0`) and free the node chain.
4. `0088835D`: free the sentinel node at `host+0Ch`.
5. `00888380`: free the `14h`-byte host.

So the release order is **borrowed state pointer, then `lua_State`, then the queued named calls,
then the host**. The 560 binding globals, the `DoFile` global installed at `00B6A303`, the
`thisTable` self tables of `docs/MISSION_ENTITY_LUA_ATTACH.md` and the `LobbySettings` table are
all ordinary values inside that one `lua_State`: nothing releases them individually, they go with
`lua_close` at step 8. The queued calls are the only Lua-adjacent allocation that is not in the
state, which is why they get their own clear.

## What survives a scene reload

`kMissionSceneReloadRequest` (0Bh) and the scene request (0Ah) both enter `004DFB70`
(`docs/MISSION_SCENE_LOAD.md`). Nothing in that path deletes the host, the machine or the state:
`008844F0`'s only caller is `004DC617`, and the list clear `00887D90` is reached only from
`00888354` (the destructor), `008882B3` (the destructor's unwind funclet `00C97620`) and
`00887E30`, which has no callers at all. Therefore:

| Thing | Across a reload | Evidence |
| --- | --- | --- |
| the `lua_State`, the 560 bindings, `DoFile` | survive | no `lua_close` on the load path |
| queued named calls | survive | the only live clear is the destructor |
| `thisTable` and every entity self table | survive when the global already exists | the gate below |
| `recon` | cleared to nil every load | `004E0321` |
| `LobbySettings` | rebuilt every multiplayer load, left nil in single player | `005E2F59`, `005E304B` |
| global script folders | re-run every load | `004DC729` -> `00886900` |

The per-load reset sits immediately around the `005E2F00` call in `004DFB70`, all sites inside
Ghidra's body for that function:

| Site | Callee | Effect |
| --- | --- | --- |
| `004E0203` | `00B67980` | globals object |
| `004E021A` | `00B67800` | `t = globals.thisTable` (name at `00CE7494`) |
| `004E0225` | `00B65FB0` | `t:IsNil()` into `BL` |
| `004E0249` | — | `TEST BL,BL` / `JZ 004E02D0`: **a non-nil `thisTable` is kept** |
| `004E029B` | `00B67580` | `globals.thisTable = {}`, only when it was nil |
| `004E02D0` | `005E2F00` | the lobby settings sync |
| `004E0321` | `00B67350` | `globals.recon = nil` (name at `00CE74A0`), unconditional |

### For the owner of `src/mission_scene_load.cpp`

That gate is inverted in the current reconstruction, which this packet does not own and did not
edit. `include/bsp/mission_scene_load.hpp` calls the `00B65FB0` seam `lua_global_exists`, and
`src/mission_scene_load.cpp:222` runs `if (host.lua_global_exists("thisTable"))
host.lua_clear_global("thisTable");` — a rebuild **when the global is there**. The native rebuilds
only when it is **not**. Either the host method has to return `00B65FB0`'s own answer ("is nil")
under a name that says the opposite, or the branch has to be negated; one of the two is wrong
whichever way the host is implemented. The same header calls `00B67350` on `recon`
`lua_declare_global`, but that call sets the global to nil, so "clear" is the verb.

`00B65FB0` returns false for an unbound object and otherwise `lua_type(...) == LUA_TNIL`, so the
"keep" branch is taken for any existing value, table or not. The practical reading: the self-table
root is created once, on the first mission load of the process, and every later mission inherits
whatever the previous mission left in it, while `recon` is deliberately dropped each time.

## Host methods

One row per native call site the reconstruction projects. `this` is the host for the two release
calls and the globals object for the reset.

| Site | Callee | Name | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `004DC60C` | `00B65E80` | `close_borrowed_state` | `game+1A0Ch` | none |
| `004DC617` | `008844F0` | `release_machine` | host / none | none (not null-guarded) |
| `004DC62C` | `00888380` (indirect through `[[host]]`) | `destroy_host` | host, flag 1 | `host != 0` |
| `00888338` | `[[host+4h]]` = `006B87C0` | `delete_machine` | machine, flag 1 | `host+4h != 0` |
| `006B87D2` | `00A68A90` | `lua_close` | state in `ECX` | machine owns byte |
| `00888354` | `00887D90` | `clear_queued_calls` | `host+8h` | none |
| `0088835D` | `00BF65AC` | free the list sentinel | `host+0Ch` | none |
| `004E021A` | `00B67800` | `open_global` | globals, out, `"thisTable"` | none |
| `004E0225` | `00B65FB0` | `is_nil` | object / bool | none |
| `004E029B` | `00B67580` | `set_global_new_table` | globals, `"thisTable"` | the object was nil |
| `004E0321` | `00B67350` | `set_global_nil` | globals, `"recon"` | none |

`00BF65AC` is `free`, `008876B0` and `00884830` are the deferred-call and node-allocation
contracts; they are not host methods.

## Uncertainties

- Steps 3, 10 and 11 of `004DC5C0` were not read; the order is from the listing, the effects are
  not claimed.
- Whether anything re-enters `004DC5C0` between missions was not checked: the claim here is only
  that the load path does not.
- No run-time evidence. `bsp_game.exe` does not reach `BSP_Game_OnDestroy`'s Lua steps yet, and
  the per-load reset sits next to the unimplemented `005E2F00` host, so rule 6 is unsatisfied for
  both parts of this packet.
