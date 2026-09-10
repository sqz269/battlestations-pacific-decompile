# Mission Lua host (`game+1A08h`)

Addresses: 00884be0 006b8740 006b8610 006b8ad0 006b89f0 00885110 00885fb0 008860b0 00887750 00887b30 00887e50 0045f440 0045f520 00887220 00885da0 006b7e70 006b7e80 006b7e90 006b7ea0 006b8120 006b8460 006b8470 006b8530 006b80c0 006b80e0 006b8130 006b84b0 004b6b00 004b6b20 004b6b30 008890f0 008c8390

Packet `mission_lua_host`, worktree `agent/mission-lua-host`. Ghidra was read-only for this
packet; every name below is a hypothesis, not a recovered symbol. The embedded interpreter is
Lua 5.1.1 (00a60870-00a7772f plus loadlib at 00c2f3e0); nothing here rebuilds any part of it.

## The two objects

`game+1A08h` is a **pointer** to the mission Lua host. `004DD620` loads it in `ECX` and calls
`00884BE0`; `004DC729` loads it and calls `00886900` on shutdown. The host's own layout that this
packet establishes:

| Offset | Contents | Evidence |
| --- | --- | --- |
| +0h | vtable | `00884BE0` never writes it, so the allocation happens in the caller |
| +4h | `LuaMachine*` | `00884C34` stores the `00BF681B(0Ch)` block; every wrapper reads `[this+4]` |
| +8h, +Ch | queue used by `00887E50`'s deferred path | `0088814A` reads `[ESI+0Ch]`, `00888150` uses `ESI+8` |

`game+1A0Ch` is an **inline** `LuaStateOwner`, constructed at `004DD641` from
`[[game+1A08h]+4h]+4h`, that is, from the same `lua_State`. `0045F440` and `0045F520` take
`LEA ECX,[ESI+1A0Ch]` to reach the globals. `game+1A1Ch` receives the constant `004C6880` at
`004DD64F` and is out of scope here.

The `LuaMachine` at `host+4h` is twelve bytes:

| Offset | Contents | Evidence |
| --- | --- | --- |
| +0h | vtable `00CF8394` | `006B8744` |
| +4h | `lua_State*` | `006B8756`, and every wrapper's `MOV ECX,[ECX+4]` |
| +8h | owns-state byte, set to 1 | `006B87B0` |

## State creation, `006B8740`

`__fastcall(LuaMachine* this)`, RET, returns `this` in `EAX`.

1. `luaL_newstate()` (`00A6A260`) into `+4h`.
2. `lua_atpanic(L, 006B8720)` (`00A67390`, panic function in `EDX`).
3. `lua_gc(L, 6, 100)` (`00A68280`): `LUA_GCSETPAUSE` with a pause of 100, so the collector
   restarts immediately after each cycle.
4. Walk the pair table at `00CF8350`, `{const char* name, lua_CFunction open}`, terminated by a
   null function at `00CF8388`. For each: `lua_pushcclosure(L, open, 0)`, `lua_pushstring(L, name)`,
   `lua_call(L, 1, 0)`.

| Lua name | `luaopen_*` |
| --- | --- |
| `""` (00CE3A0C is an empty string) | `00A67210` `luaopen_base` |
| `table` | `00A66010` |
| `io` | `00A652B0` |
| `os` | `00A64190` |
| `string` | `00A63910` |
| `math` | `00A61C50` |
| `debug` | `00A61620` |

`luaopen_package` (`00C2FF90`) is **not** in the table, so the mission state has no `require`, no
`package` and no `loadlib`. `luaL_register` is never called from game code; the nine callers of
`00A6AA00` are all `luaopen_*` bodies inside the library.

## Bring-up, `00884BE0`

`__fastcall(MissionLuaHost* this)`, RET, no stack arguments. Called once from
`BSP_Game_OnInitOnce` (`004DD5B0`).

1. `malloc(0Ch)` then `006B8740` on it; the result goes to `this+4h`. A failed allocation stores
   null and every later wrapper would fault, so the native does not handle it.
2. `006B8AD0("PC=true", 0, 0, 2)`: run the platform chunk. Literal at `00D0E714`.
3. Walk the registration table at `00E0B7B8` and install every row with `006B8610`.
4. `006B89F0([00884770()+4h], [00884770()+8h], "Scripts\\fundamentals.lua", 0, 0, 2)`. The bytes
   come from the singleton at `00884770`, not from the virtual file system, so the third argument
   is a chunk label. `Scripts/fundamentals.lua` also exists on disk in the install.

## The registration table, `00E0B7B8`

`{const char* name, lua_CFunction fn}` pairs, terminated by a null function pointer at `00E0C938`.
**560 entries, all names distinct, 557 distinct addresses.** The three aliases are
`SetMotionBlurParams`/`SetMBP` (`0088E560`), `AddAirBaseStock`/`AddAirBasePlanes` (`00896A90`) and
`MissionNarrative`/`MissionNarrativeEnqueue` (`008B0C10`).

`006B8610` is `__thiscall(LuaMachine* this, const char* name, lua_CFunction fn)`, RET 8:

```
mov ecx, [ecx+4]            ; L
mov edx, [esp+8]            ; fn        (loaded before the frame shifts)
push 0                      ; nup
call lua_pushcclosure        ; 00A67B20
mov edx, 0FFFFD8EEh          ; LUA_GLOBALSINDEX (-10002)
push name
call lua_setfield            ; 00A67E40
```

So **every binding is a plain global**; there is no namespace table and no metatable. The complete
table is reproduced as data in `src/mission_lua_host.cpp`, name and address per row, in table
order. Groups, by the prefix the table itself uses:

| Group | Example names | Rough range |
| --- | --- | --- |
| logging and debug | `Log`, `LogToFile`, `Assert`, `SETLOG`, `dprintf`, `debugtrap`, `TerminateExecution` | `0088BC00`-`008C8560` |
| hints and support manager | `ShowHint`, `AddStoredHint`, `IsHintCritical`, `PermitSupportmanager` | `008D1F50`-`008D2BB0` |
| damage, repair, flooding | `AddDamage`, `SetFireDamage`, `GetLeaks`, `SetPump`, `SetRepairLevel` | `0088E000`-`008ADC40` |
| air bases and squadrons | `SetAirBaseSlot`, `LaunchSquadron`, `SquadronSetTravelAlt`, `PlaneSetRollCtrl` | `00895BA0`-`008A28B0` |
| navigation and pilots | `NavigatorMoveToPos`, `PilotBomb`, `PilotTorpedo`, `UnitSetFireStance` | `008A2A50`-`008A7940` |
| spawning and placement | `Spawn`, `SpawnNew`, `PutTo`, `PutRelTo`, `GenerateObject` | `00944680`-`008AAF30` |
| objectives, scoring, narrative | `Objectives_Add`, `Scoring_AddMissionScore`, `MissionNarrative`, `Countdown` | `008B0AC0`-`008CD440` |
| camera | `MovCam_RefPos`, `MovCam_PathWalk`, `Camera_Shake`, `GetCameraPosRot` | `008B2CC0`-`008BFA70` |
| world and weather | `SetDayTime`, `SetWeather`, `SetSky`, `SetShallowWater`, `SetCloudVisibility` | `008B1CC0`-`008C4B50` |
| triggers and messages | `AddProximityTrigger`, `AddListener`, `DisplayMessage`, `LoadMessageMap` | `008C61C0`-`008D0EA0` |
| strategic AI | `AICreate`, `AISetCommand`, `AISetTargetWeight` | `00A37310`-`00A38A50` |
| loading and profiling | `Loading_Start`, `Loading_Progress`, `Loading_Finish`, `PerfTimingStart`, `MemoryStatus` | `008C76C0`-`008D1DD0` |

The argument convention of each body is not reconstructed by this packet, only the registration
contract they all share: `__fastcall(lua_State* L in ECX)` returning the Lua result count. The one
body this packet does read is the error handler below.

## Chunk execution, `006B89F0`

`__thiscall(LuaMachine* this, const void* buffer, int size, const char* chunk_name,
ResultSink* results, NativeString* error_out, int result_mode)`, RET 18h, returns 0 or 1 in `EAX`.

```
top = lua_gettop(L)                                 ; 006B7E70 -> 00A673D0
status = luaL_loadbuffer(L, buffer, size, name)     ; 00A6A160, L in ECX, buffer in EDX
if (status == 0) status = lua_pcall(L, 0, -1, 0)    ; 00A680E0, nargs in EDX
failed = (status != 0)
if (lua_gettop(L) > top) {
    if (failed) {
        msg = lua_tolstring(L, -1, NULL)            ; 00A67810, idx in EDX
        if (error_out) error_out->assign(msg)       ; 0041E350
    } else if (results) {
        006B89C0(results, this); 00887220(results, top_delta, result_mode)
    }
    lua_settop(L, top)                              ; 006B7E80 -> 00A673E0
}
return failed
```

Two facts matter for the rebuild. **`errfunc` is 0 here**, so a file load gets no traceback; the
`debugtrap` handler is used only by the named-call path. And `lua_tolstring` runs on the failure
path even when no sink was supplied (`006B8A80`), which coerces the error value on the stack
before `lua_settop` discards it.

`006B8AD0` is `__thiscall(const char* source, ResultSink*, NativeString* error_out, int mode)`,
RET 10h: an inline `strlen` then `006B89F0(source, len, source, ...)`, so a string chunk names
itself.

## File load

### `00885110`, one file

`__thiscall(MissionLuaHost* this, const char* path)`, RET 4.

1. Copy `path` into a local native string and call `[[0109CEEC]] virtual +4h (name, 2)`. `0109CEEC`
   is the virtual file system singleton; the returned object is a stream.
2. Release the local string back into the sized storage pool.
3. `stream->virtual +18h` returns a byte; when it is zero the function returns and **does not
   release the stream**.
4. `size = stream->virtual +30h ()`; `buffer = malloc(size)`; `stream->virtual +24h (buffer, size, 0)`.
5. `006B89F0(host->LuaMachine, buffer, stream->virtual +30h (), path, 0, &local_error, 2)`.
   The result sink is null and the error sink is a stack local.
6. `free(buffer)`; `InterlockedDecrement(&stream[1])` through `[00CE2220]` and, at zero,
   `stream->virtual +0h ()`; release the error string back to the pool.

**What an erroring script leaves behind: nothing.** The message is written into a stack native
string and released at `0088531B` without ever being read, and `006B89F0`'s return value is
discarded at `008852E7`. The Lua stack is restored to its entry top. A script that fails to open
is an equally silent no-op.

`ghidra flow 00885110` reports two gaps; the one after `CALL 00BF6989` at `008852ED`
(`008852F2..00885331`, 63 bytes) is the stream release and string cleanup that Ghidra dropped as
unreachable because `_free` is annotated no-return. The listing, not the pseudocode, is the source
for that tail. The other gap (`00885134..00885234`, after the `JMP` at `0088512F`) is the
exception funclet region and was left alone.

### `00885fb0`, the file and its content variants

`__thiscall(MissionLuaHost* this, const char* path, char run_variants)`, RET 8.

1. `00885110(path)` unconditionally.
2. When `run_variants` is zero, return.
3. Copy `path` into a native string and call `00BDEF90` on the virtual file system singleton
   (`MOV ECX,[0109CEEC]` at `00886007`), which fills a vector of eight-byte native strings.
   `00BDEF90` is the content-variant expander shared with `BSP_Localization_ParseTableFile`
   (`00AA0020`): it splits the name at the first `.` using `_strcspn` and builds the
   content-specific names, with a `Content file name for %s` diagnostic.
4. `00885110` on every entry, substituting the empty literal at `00F87904` for a null buffer.
5. Release the vector.

### `008860b0`, the mission script

`__thiscall(MissionLuaHost* this, const NativeString* scene_name)`, RET 4. Builds
`"Scripts/missions/"` (`00D0E738`) + name + `".lua"` (`00CFD2C8`) through two
`BSP_NativeString_Concat` calls and hands `result->data` (or the empty literal) to
`00885FB0(path, 1)`. `ECX` is preserved into `ESI` at `008860CD` and restored for the
`00885FB0` call at `0088618E`, which settles the open question in `docs/MISSION_SCENE_LOAD.md`:
`008860B0` **is** `__thiscall` on the host at `game+1A08h`, and Ghidra's `__cdecl` rendering is
wrong.

## Named entry-point dispatch

### `00887750`, the body

`__thiscall(MissionLuaHost* this, const NativeString* self_key, const NativeString* name,
const ArgVector* args, int stack_first, int stack_last, ResultSink* results, char force)`,
RET 1Ch.

1. `if (!force && game+1FE4h == 2) { release the local scope object; return; }` (`00887782`).
   With `force` set the gate is skipped entirely.
2. If `results` is non-null, stash `this->LuaMachine` into `results+4h`, releasing whatever was
   there through `006B88E0`.
3. `saved_top = lua_gettop(L)`.
4. If `stack_first != 0`, normalise both indices against `lua_gettop(L)`: a negative index becomes
   `top + 1 + index`.
5. `lua_getfield(L, LUA_GLOBALSINDEX, "debugtrap")` (`00D0E79C`), then
   `error_handler = lua_gettop(L)`.
6. `[00F87900] += 1` — a process-wide Lua re-entrancy depth, decremented at `008879C4`.
7. Split `name` on `'.'` with `00BD20A0` (separator in `DL`), which drops empty segments.
   `lua_getfield(L, LUA_GLOBALSINDEX, segment[0])`, then for each later segment
   `lua_pushstring(L, seg)`, `lua_gettable(L, -2)`, `lua_remove(L, -2)`.
8. If `self_key` is non-null and non-empty: `lua_getfield(L, LUA_GLOBALSINDEX, "thisTable")`
   (`00CE7494`), `lua_pushstring(L, self_key->data)`, `lua_gettable(L, -2)`, `lua_remove(L, -2)`.
   `nargs` starts at 1.
9. Push every twenty-byte record of `args` through `00885DA0`; `nargs += args->count`.
10. If `stack_first != 0` and `stack_first <= stack_last`: `lua_pushvalue(L, i)` for each index in
    the range; `nargs += 1 + (stack_last - stack_first)`.
11. `marker = lua_gettop(L)`; `game+1A18h += marker`;
    `lua_pcall(L, nargs, LUA_MULTRET, error_handler)`; `game+1A18h -= marker`.
12. `delta = lua_gettop(L) - saved_top`; if `results` and `delta != 0`,
    `00887220(results, delta, 4)`.
13. `lua_settop(L, saved_top)`; `[00F87900] -= 1`; release the split vector.

The `nargs` count at step 9 uses `args->count` verbatim, so a tag-4 record (which pushes nothing)
makes the count disagree with the real stack. This is a native quirk, not a reading error; the
reconstruction models the pushed count separately and notes the divergence.

### `00885DA0`, the argument records

`__thiscall(const ArgRecord* record, lua_State-carrying LuaMachine* machine)`. Records are 20 bytes
and the tag is at `+4h`, the value at `+8h`:

| Tag | Push | Wrapper |
| --- | --- | --- |
| 0 | number | `006B80E0` -> `lua_pushnumber` `00A679D0` |
| 1 | string from the native string at `+8h` | `006B8120` -> `lua_pushstring` `00A67A50` |
| 2 | boolean from the byte at `+8h` | `006B80C0` -> `lua_pushboolean` `00A67BC0` |
| 3 | `thisTable[sprintf("%d", short at +8h)]`, or nil when the short is zero | `00884240` then the `thisTable` walk |
| 4 | nothing | falls straight through |
| 5 | a new table filled from the container at `+8h` | `006B84B0` -> `lua_createtable` `00A67D10` |
| 6 | nil | `006B8130` -> `lua_pushnil` `00A679C0` |

Format literal for tag 3 is `%d` at `00CE3A34`.

### `00887B30` and `00887E50`, the two entry wrappers

`00887B30` is `__thiscall(a1..a5)`, RET 14h, a straight tail into
`00887750(a1, a2, a3, a4, a5, results = 0, force = 1)`. It always runs on the calling thread and
ignores the lifecycle gate.

`00887E50` is `__thiscall(a1..a5)`, RET 14h, with the same five arguments and no result sink.
It differs in three ways:

- `game+1FE4h == 2` returns immediately (`00887E73`), and the test is repeated after the lock is
  taken (`00887EC8`).
- `BSP_FrameJobPool_GetSingleton` (`004C1130`) then `[[pool+4]] virtual +10h`: when it returns
  non-zero the call is **queued** instead of run. `00888101` takes a critical section through
  `[00CE2218]`, builds a seven-dword record and hands it to `00887C30` with `this+8h`/`this+Ch`,
  which is the host's own deferred queue.
- Otherwise it copies `[0109CEEC]+14h`, the virtual file system critical section, into a local
  scope object with vtable `00D0E6EC` and runs the same body as `00887750` inline.

`00887E50` has 40 callers across the game; `00887B30` has one (`0045F520`).

### `0045F440` and `0045F520`, the guarded entry points

Both are `__thiscall(Game* this, const char* name, const ArgVector* args)`, RET 8, and are
structurally identical:

```
BSP_LuaStateOwner_GetGlobals(&globals)      ; 00B67980 on ECX = game+1A0Ch
BSP_LuaObject_GetByName(&entry, name)       ; 00B67800
exists = 00B66200(&entry)                   ; "is defined and not nil"
BSP_LuaObject_Destruct(&entry); BSP_LuaObject_Destruct(&globals)   ; 00B67700 twice
if (exists) {
    NativeString local(name);
    <dispatcher>(0, &local, args, 0, -1)    ; ECX = [game+1A08h]
    release local
}
```

`0045F440` dispatches through `00887E50`; `0045F520` through `00887B30`. Neither reads a return
value, and a missing global costs one globals fetch and two destructor calls.

`00B67980`, `00B67800`, `00B66200` and `00B67700` belong to the GUI Lua reader packet and were
read only.

### The error handler, `008C8390`

Registered as the global `debugtrap`. `__fastcall(lua_State*)`, returns 0, so it contributes no
results. It opens a `LuaStateOwner` over the incoming state, calls
`00887750(0, "debug.traceback", 0, 0, -1, &results, 0)` — the same dispatcher, which is exactly
why the splitter on `'.'` exists — reads the returned string and passes it to the logging helper
`004254B0`. The traceback is logged and swallowed; the handler does not re-raise.

## The `game+644h` re-entry guard

Three inlined helpers survive as standalone bodies, none of them with a Ghidra function and none
with a call reference except the unwind funclet at `00C672E9`:

| Address | End | Body | ABI |
| --- | --- | --- | --- |
| `004B6B00` | `004B6B15` | `this->+644h += (arg != 0) ? 1 : -1` | `__thiscall(Game*, char)`, RET 4 |
| `004B6B20` | `004B6B2F` | `[00E188A8]->+644h += 1`, `EAX = ECX` | RET, scope-guard constructor |
| `004B6B30` | `004B6B3C` | `[00E188A8]->+644h += -1` | RET, scope-guard destructor |
| `008890F0` | `008890FB` | `return this->+644h > 0` | `__thiscall(Game*)`, RET, `SETG AL` |

The counter is a signed depth and is never clamped. `BSP_Game_LoadMissionScene` inlines the
increment at `004E0A83` and the decrement at `004E0C52`, bracketing the whole
`luaPrecacheUnits` / `luaStageInitMulti` / `luaStageInit` block.

**What it protects.** Exactly two consumers read it, both Lua bindings:

- `008CC850` `Loading_Start`: `CMP [game+644h], EBP` with `EBP = 0`, `JG` past the body.
- `008C77F0` `Loading_Finish`: `if (game+644h < 1) { 0060D3E0(); BSP_Game_SetCinematicMode(0,0,1); }`.

**The rule.** While the depth is at least 1, a mission script's `Loading_*` calls do nothing. A
script that runs inside an outer load must not open, drive or close the loading screen, because the
loader owns it; the same script run outside a load (from a reload or a runtime trigger) may. The
predicate `008890F0` is the same test written as a function.

## Lua API boundary

Every game-side wrapper is `__thiscall` on the `LuaMachine`, loads `L` from `[this+4h]` and
forwards to the Lua 5.1.1 library, which is compiled with the **register convention**: `lua_State*`
in `ECX`, the first integer argument in `EDX`, the rest on the stack, callee-cleaned.

| Wrapper | ABI | Library function |
| --- | --- | --- |
| `006B7E70` | tail jump, RET | `lua_gettop` `00A673D0` |
| `006B7E80` | RET 4 | `lua_settop` `00A673E0` |
| `006B7E90` | RET 4 | `lua_pushvalue` `00A67570` |
| `006B7EA0` | RET 4 | `lua_remove` `00A67430` |
| `006B80C0` | RET 4 | `lua_pushboolean` `00A67BC0` |
| `006B80E0` | RET 8 | `lua_pushnumber` `00A679D0` |
| `006B8120` | RET 4 | `lua_pushstring` `00A67A50` |
| `006B8130` | tail jump | `lua_pushnil` `00A679C0` |
| `006B8460` | tail jump, `EDX = -10002` | `lua_getfield` `00A67C40` |
| `006B8470` | RET 4 | `lua_gettable` `00A67C20` |
| `006B84B0` | RET 8 | `lua_createtable` `00A67D10` |
| `006B8530` | RET 0Ch | `lua_pcall` `00A680E0` |
| `006B8610` | RET 8 | `lua_pushcclosure` `00A67B20` + `lua_setfield` `00A67E40` |
| `006B8740` | RET | `luaL_newstate` `00A6A260`, `lua_atpanic` `00A67390`, `lua_gc` `00A68280`, `lua_pushcclosure`, `lua_pushstring`, `lua_call` `00A68090` |
| `006B89F0` | RET 18h | `luaL_loadbuffer` `00A6A160`, `lua_pcall`, `lua_tolstring` `00A67810`, `lua_gettop`, `lua_settop` |

That is the complete set the mission host needs: `lua_gettop`, `lua_settop`, `lua_pushvalue`,
`lua_remove`, `lua_pushnil`, `lua_pushboolean`, `lua_pushnumber`, `lua_pushstring`,
`lua_pushcclosure`, `lua_createtable`, `lua_getfield`, `lua_setfield`, `lua_gettable`,
`lua_call`, `lua_pcall`, `lua_tolstring`, `lua_atpanic`, `lua_gc`, `luaL_newstate`,
`luaL_loadbuffer`, and the seven `luaopen_*` entry points.

## Installed scripts

Read-only inspection of `I:/SteamLibrary/steamapps/common/Battlestations Pacific`:

| Measure | Value |
| --- | --- |
| `.lua` files in the install | 621 |
| `.lua` files under `scripts/missions` | 299 |
| mission subfolders | `bsm`, `chg`, `COTP-IJN`, `COTP-USN`, `ijn`, `multi`, `traininggrounds`, `usn` |
| distinct top-level function names across those 299 files | 6051 |

Files defining each engine-invoked entry point:

| Entry point | Files | Wrapper |
| --- | --- | --- |
| `luaStageInit` | 299 of 299 | `0045F440` -> `00887E50` |
| `luaPrecacheUnits` | 295 | `0045F520` -> `00887B30` |
| `luaStageInitMulti` | 282 | `0045F520` -> `00887B30` |
| `luaEngineMovieInit` | 154 | `0045F440` -> `00887E50` |

Those four are the only names the engine calls by name in the mission path; the string references
sit in `004DFB70` and `0045F600` only. Everything else a mission defines (`lua_Think`,
`luaIntroMovie`, `luaMissionComplete`, the per-mission listeners) is reached from Lua itself or
through the `SetThink` / `AddListener` / `CreateScript` bindings, which take the function name as a
string argument. `Scripts/fundamentals.lua` exists on disk and also ships embedded in the
executable; `Scripts/global/luaMW_init.lua` is run separately by `004DD5B0`.

## Callers and callees

`00884BE0` <- `004DD5B0` `BSP_Game_OnInitOnce`. `008860B0` <- `004DFB70`. `00885FB0` <- `008860B0`.
`00885110` <- `00885FB0`, `00886370`. `006B89F0` <- `00885110`, `00884BE0`, `006B8AD0`.
`00887750` <- `00887B30`, `008C8390`. `00887E50` <- 40 sites including `0045F440`, `00452360`,
`00888230`. `0045F440` and `0045F520` <- `004DFB70`, `0045F600`.

## Uncertainties

- The local object `00887750` builds at `00887774` (vtable `00D0E6EC`, plus `[0109CEEC]+14h`) is
  constructed and destroyed but no `EnterCriticalSection` is visible in its body, so whether it
  actually locks, or only carries the section for the destructor, is not settled. `00887E50`
  builds the same object at `00887EB5`.
- `game+1A18h`, adjusted by the stack top across the `pcall`, has no other reader in the addresses
  read here. Its purpose is unknown.
- `00887220`'s second argument (2 for chunks, 4 for calls) selects something inside `00886E00`;
  the meaning of the mode is not established.
- The `ResultSink` at `00887750`'s sixth argument and `006B89F0`'s fourth is modelled only by its
  two entry points (`006B89C0` stores the machine, `00887220` appends values). Its layout is not
  recovered.
- `00885DA0` tag 5 walks a container with debug-iterator checks; the container type is not
  identified, so nested tables are modelled as an ordered element list.
- `00885110` leaks the stream when the file fails to open (`0088528C` jumps past the release).
  Whether the open path already released it on failure was not confirmed from the stream vtable.
- The 560 binding bodies are not analysed. Only the registration contract, `debugtrap` and the two
  `Loading_*` gates were read.

## What remains

- The `00887E50` deferred queue (`00887C30`, host `+8h`/`+Ch`) and when it is drained.
- `00884770`, the singleton that holds the embedded `fundamentals.lua` bytes.
- `00886900`, the host teardown reached from `BSP_Game_OnDestroy`.
- The argument conventions of the 560 bindings.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `mission_lua_queue` | 00887c30 00887e50 00886370 00884770 | docs/MISSION_LUA_QUEUE.md, include/bsp/mission_lua_queue.hpp | The host's deferred call queue at `+8h`/`+Ch`: record layout, who drains it, and the embedded-chunk singleton at `00884770` |
| `mission_lua_values` | 00885da0 00886e00 00887220 00884240 | docs/MISSION_LUA_VALUES.md, include/bsp/mission_lua_values.hpp | The twenty-byte variant record both ways: `00885DA0` push and `00886E00` read, including the tag-5 container and the mode argument |
| `lua_binding_damage` | 0088e000 0088e320 0088e790 0088e9d0 008acd10 008acf00 | docs/LUA_BINDING_DAMAGE.md | Argument and return conventions of the damage and fire bindings, as one worked group of the 560 |
| `mission_loading_gates` | 008cc850 008c76c0 008c77f0 0060d3e0 | docs/MISSION_LOADING_GATES.md | What `Loading_Start`/`Progress`/`Finish` do when `game+644h` is zero, and the cinematic-mode handoff |

## Reconstruction state

| Address | Name | State |
| --- | --- | --- |
| 00884be0 | `BSP_MissionLuaHost_Initialize` | reconstructed, build-tested |
| 006b8740 | `BSP_LuaMachine_Construct` | reconstructed, build-tested |
| 006b8610 | `BSP_LuaMachine_RegisterGlobalFunction` | reconstructed, build-tested |
| 006b89f0 | `BSP_LuaMachine_RunChunk` | reconstructed, build-tested |
| 006b8ad0 | `BSP_LuaMachine_RunString` | analysed |
| 00885110 | `BSP_MissionLuaHost_RunScriptFile` | reconstructed, build-tested |
| 00885fb0 | `BSP_MissionLuaHost_RunScriptWithVariants` | reconstructed, build-tested |
| 008860b0 | `BSP_MissionScript_RunFile` | analysed (convention settled; already named) |
| 00887750 | `BSP_MissionLuaHost_CallNamed` | reconstructed, build-tested |
| 00887b30 | `BSP_MissionLuaHost_CallNamedForced` | reconstructed, build-tested |
| 00887e50 | `BSP_MissionLuaHost_CallNamedThreadSafe` | reconstructed, build-tested |
| 0045f440 | `BSP_Game_CallLuaEntryPointThreadSafe` | reconstructed, build-tested |
| 0045f520 | `BSP_Game_CallLuaEntryPointForced` | reconstructed, build-tested |
| 00885da0 | `BSP_MissionLua_PushArgumentRecord` | analysed |
| 00887220 | `BSP_MissionLua_CollectStackResults` | analysed |
| 008c8390 | `BSP_LuaBinding_DebugTrap` | analysed |
| 004b6b00 | `BSP_Game_AdjustScriptLoadDepth` | reconstructed, build-tested, fixture-tested |
| 004b6b20 | `BSP_Game_EnterScriptLoadScope` | reconstructed, build-tested, fixture-tested |
| 004b6b30 | `BSP_Game_LeaveScriptLoadScope` | reconstructed, build-tested, fixture-tested |
| 008890f0 | `BSP_Game_IsScriptLoadActive` | reconstructed, build-tested, fixture-tested |
| 006b7e70 | `BSP_LuaMachine_GetTop` | analysed |
| 006b7e80 | `BSP_LuaMachine_SetTop` | analysed |
| 006b7e90 | `BSP_LuaMachine_PushValue` | analysed |
| 006b7ea0 | `BSP_LuaMachine_Remove` | analysed |
| 006b80c0 | `BSP_LuaMachine_PushBoolean` | analysed |
| 006b80e0 | `BSP_LuaMachine_PushNumber` | analysed |
| 006b8120 | `BSP_LuaMachine_PushString` | analysed |
| 006b8130 | `BSP_LuaMachine_PushNil` | analysed |
| 006b8460 | `BSP_LuaMachine_GetGlobal` | analysed |
| 006b8470 | `BSP_LuaMachine_GetTable` | analysed |
| 006b84b0 | `BSP_LuaMachine_CreateTable` | analysed |
| 006b8530 | `BSP_LuaMachine_ProtectedCall` | analysed |

Installed-file-checked: the 299 mission scripts and the four entry-point names above.

## No Ghidra function

`004B6B00` (end `004B6B13`), `004B6B20` (end `004B6B2F`) and `008890F0` (end `008890FB`) have no
Ghidra function; `008890F0` is swallowed by the enclosing candidate `008890B0`. `004B6B30` does
start a Ghidra function. The orchestrator must define the three bodies before applying names.
