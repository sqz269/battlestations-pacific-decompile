# CC13 profile checkpoints and Lua text dumpers (segment 2)

Addresses: 0043c1c0, 0043c530, 0043cc20, 00435d30, 00436730, 00437250, 00437c60, 00437c70,
004379c0, 00438020, 00438a00, 00b68460

Packet: partition segment 2 (`0042A920..0043E840`), lineage cc13, worker cc13-settings1.
Names are descriptive hypotheses, not recovered symbols. This continues
`docs/CC13_SETTINGS_SAVEDATA.md` (its open item 1, the dumpers behind `006AD8D0`).

## Scope

Most of segment 2 is camera, matrix and HUD-gauge code; this packet took only the profile and
text-serialisation cluster `00435D30..0043CF9B`. Already covered elsewhere and not repeated: the
checkpoint read path `004374F0` and the cross-state copy `00436C50`
(`docs/PROFILE_MANAGER_REFRESH.md`, `src/profile_manager.cpp`), the unlock predicate
(`docs/PROFILE_UNLOCK_PREDICATE.md`), and the `0043BDF0` float formatter (`"%f"`).

## Status

| Item | Exported | Reconstructed | Build-tested | Probed |
| --- | --- | --- | --- | --- |
| `0043C1C0` value text, `0043C530` table dump, `006AD8D0` emitter | yes | `src/lua_text_dump.cpp` | yes | local probe |
| `00435D30` Lua slot -> archive writer | yes | `src/profile_checkpoint.cpp` | yes | local probe through `ArchiveTextWriter` |
| `00436730` checkpoint copy, `004379C0` load | yes | `src/profile_checkpoint.cpp` | yes | local probe |
| `00438020` save (Lua and archive parts) | yes | `src/profile_checkpoint.cpp` | yes | local probe |
| `00437C70` clear (Lua part), `00437250` rewrite | yes | `src/profile_checkpoint.cpp` | yes | no |
| `0043CC20` play-time text | yes | `src/profile_checkpoint.cpp` | yes | no |
| `00438A00` request, `00437C60` continuation | yes / disk | documented | n/a | no |

Nothing is ABI compatible or game-validated. The script bindings `MW_SaveCheckpoint`
(`008ACB70`) and `MW_LoadCheckpoint` (`008ACA30`) remain unbound in `src/mission_lua_host.cpp`
(cc9 lane); these functions are what they would call.

## Checkpoint model

The mission Lua state (`game+1A0Ch`) holds a global `BSP_Chk_Save` (`00CE4124`) keyed by mission id
(`game+2198h`, a `NativeString` used with C-string semantics). Each entry is a table with
`__difficulty`, `__unlockFrom`, `__unlockTo`, `__unlockName`, `__playTime` (`00CE4160`, `00CE4150`,
`00CE4144`, `00CE4134`, `00CE4170`) and the slots of the table the script saved. Its text image is
written to the game storage file (kind 2) named at `game+684h`.

### Save: `00438A00` and `00438020`

`00438A00` (`__thiscall`, `ECX` = profile manager, `[ESP+4]` = data `LuaObject*`, `RET 4`) copies
the table into manager `+4`, sets manager `+18h` to 1, and, when `00A3E540` (byte `+120h` of the
object at `[00F8ABE8]`) is set, requests a storage write (`00BD3E10(game+684h, 2)`) and runs the
driver with continuation `004389F0`; otherwise it calls `00438020` on the profile manager singleton
(`00425C20`).

`00438020` (`ECX` = manager, `RET`):

1. Record (`00436690` defaults 0, -1, -1, ""): `difficulty = [game+6ACh]`; the first non-zero dword
   of the vehicle-class registry array (`00437F50`, `+10h`, count `+8`) gives `unlockFrom` = its
   index and `unlockTo` = its value; `unlockName` = the localized `+54h` name of the class from
   `BSP_VehicleClass_GetOrCreate` (`00964790`), or `globals.unknown` (`00CE4180`), through
   `00A9FAD0` and the UTF-16 narrowing `00436630`. The host takes these as inputs.
2. `BSP_Chk_Save[id] = {}` (`0043822D`), re-read, then the five fields: three integers through
   `00B67460`, the name through `00B67630`, `__playTime = [[game+21A0h]+14A0h]` (float) through
   `00B67400`.
3. Each slot of manager `+4` is copied with `00436C50`.
4. `BSP_PlayerProfile_SetCheckpointRecord` (`007FC2C0`) on `game+650h`.
5. When `00A3E540` is set and storage `[0109CECC]+8 != 1`: an `ArchiveTextWriter` writes section
   `BSP_Chk_Save`, section `<id>` with `__difficulty`/`__unlockFrom`/`__unlockTo` (type 1),
   `__unlockName` (type 0), `__playTime` (type 2) and each data slot through `00435D30`, closes it,
   then, if manager `+18h` is set and `BSP_Chk_Save` is bound, writes every other table-valued
   entry whose key text differs case-insensitively from the id, closes `BSP_Chk_Save`, and runs
   the driver with `00BD53C0`.

### Load: `004379C0`

`__thiscall`, `ECX` = manager, `[ESP+4]` = destination script context, `RET 4`. When
`BSP_Chk_Save[id]` is a table: `__playTime` (`00B66330`, default 0.0, numbers only) is stored at
`[[game+21A0h]+14A0h]`, a new table is created on the destination and each slot is copied with
`00436730`. Otherwise the destination gets nil (`00B66430`). The host returns the play time and
leaves the result on the destination stack.

### Clear and rewrite: `00437C70`, `00437C60`, `00437250`

`00437C70` (from `0058BF89`, mission start): `BSP_Chk_Save[id] = nil`, then
`007FBE10` (adds `94h` and jumps to `007FBB20`, a `map::erase(key)`) drops the profile record. If
the save name at `game+684h` is non-empty, storage state is 0 and the storage's virtual `+1Ch`
reports the file (kind 2), it requests a write with continuation `00437C60`. That continuation is
eight bytes with no Ghidra function: `CALL 00425C20; MOV ECX,EAX; JMP 00437250` (referenced only by
the `MOV ECX,437C60h` at `00437D8F`). `00437250` returns when storage state is 1; otherwise it
writes section `BSP_Chk_Save` with every table entry whose key differs from the id (so the
cleared mission is excluded twice over) and runs the driver with `00BD53C0`.

### Copy and archive rules

`00436730` (`__thiscall`, destination table, key, value; `RET 0Ch`) is a twin of `00436C50`: the
call-sequence diff differs only in the C-string setters (`00B67580`, `00B67800`, `00B66790` instead
of the `NativeString` ones) and a float floor helper `004355C0` in one arm. Booleans, numbers,
strings and tables are copied; light userdata and other types are skipped (jump table `00436C20`).
A string key keeps its C-string prefix, any other key goes through `00B66290` (float32, `__ftol`).
A number is stored through the integer setter when `float32 - floor(float32) == 0`, otherwise as
float32. A child table is created and assigned before it is filled.

`00435D30` (`__thiscall`, writer, key, value; `RET 0Ch`) maps a number key to an integer variant
(`00B66290`), a string key to a string variant, and anything else to integer 0. Values: boolean
type 3, whole float32 type 1 (`00BF85B0` floor test, `00435E95..00435EB7`), other numbers type 2,
string type 0, table as a section (virtual `+4`/`+8`) of its slots in `lua_next` order; nil, light
userdata, functions, userdata and threads are skipped (jump table `00435F28`).

## Lua text dumpers

`0043C1C0` (`__fastcall`, `ECX` = out, `EDX` = `LuaObject*`, `RET`): `nil`, `true`, `false`;
an integral float32 as `%d`; another number as `0043BDF0` `%f`; a string as `"` + text + `"` with
no escaping; a light userdata as `"_ptr_<WORD at +174h>_"` or `"_ptr_x_"`; a function as
`"_function_<lua_topointer as %d>_"`; anything else as `"_other_type:_<name>_"` where the name comes
from `00B68460` (`none`, `nil`, `bool`, `lightuserdata`, `number`, `string`, `table`, `function`,
`userdata`, `thread`, else `unknown`).

`0043C530` (`ECX` = table `LuaObject*`, `EDX` = indent, `[ESP+4]` = depth, `[ESP+8]` = a word only
forwarded, `RET 8`) appends to the save storage (`00BD49B0`): the indent is `EDX` tab characters;
`{\n`; for each slot `\t[<key text>] =`, then either `\n` and the nested table at indent+1,
depth-1, or ` <value text>,\n`; then `}\n` at indent 0 and `},\n` deeper. At negative depth it emits
only indent + `\tnil, -- too deep recursion` with no line break, so the next slot line continues on
the same line. `006AD8D0` starts it at indent 0 and depth `10h`, so a mission save keeps 17 table
levels. Strings containing `"` or a newline make the emitted text unreadable as Lua.

`0043CC20` (`ECX` = out, `[ESP+4]` = float seconds, `RET 4`; caller `0062C2C0`, the scoring screen)
truncates to whole seconds and builds `.H|.:|` (only from 3600 s), `.MM|.:|` (zero-padded only when
hours were printed) and `.SS` (always two digits) in an 80h-byte `BSP_FixedStringBuilder`.

## Host probe

An uncommitted local probe (`local/s1_probe/probe3.cpp`) saved a checkpoint for `JM06` with record
`{2, 5, 7, "Yamato"}`, play time 99.25 and a data table holding a nested array, a string, a boolean
and an integer key, next to an existing `JM02` entry, through the real `ArchiveTextWriter`. The
archive was `BSP_Chk_Save = { ["JM06"] = { ["__difficulty"] = 2, ..., ["__playTime"] = 99.250000,
[3] = "three", ..., ["pos"] = { [1] = 1.500000, [2] = 2, }, }, ["JM02"] = {...}, }`; the load
returned 99.25 and a table; `emit_lua_global_text_006ad8d0` printed the same data in the
`0043C530` layout. This checks the host code against its own reading of the assembly only.

## Divergences and uncertainty

- Native Lua errors (indexing a non-table `BSP_Chk_Save`, walking a nil) become host
  `std::runtime_error`; cycles, which recurse without bound natively, stop at a host depth limit.
- `00A3E540`'s meaning (byte `+120h` of `[00F8ABE8]`) is not established; the host takes the
  archive decision as a nullable writer.
- `CheckpointRecordFields` and the play time are caller inputs; the vehicle-class and localization
  chain that fills them was read but not reconstructed.
- `00436730` and `00436C50` behave the same in the host; the native difference is only which
  key-setter wrappers they call.

## Handoff

Done: dumpers, checkpoint save/load/clear/rewrite, slot archive writer, play-time text; 10 ledger
names applied to Ghidra; report rows in `reports/cc13_profile_save.json`.

Open:

1. `006A9A40` (set a binding slot: copies the 14h descriptor into
   `runtime_bindings[input][slot]`, sets `runtime_reverse` from the sign of the scale, then walks
   the device record's vector at `+5Ch` and calls `BSP_InputSettings_SuppressAlternateBinding`
   `00699BF0` and mirrors axis partners for keys 8/3Ch) and `006AAC50` (`|. (|`/`|.)|` conflict
   text) were surveyed only.
2. Wiring `MW_SaveCheckpoint`/`MW_LoadCheckpoint` in the mission Lua host belongs to the cc9 lanes.
3. The unlock record chain (`00437F50`, `00964790`, `00A9FAD0`).
