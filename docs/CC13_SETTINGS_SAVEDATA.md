# CC13 settings and mission save data (segment 29)

Addresses: 0069e300, 0069e9e0, 006a33f0, 006a5640, 006a9850, 006ad470, 006ad8d0, 006addc0, 006ae120, 006ae280, 006ae400, 006ae730,
006ae870, 006aea00, 006aebf0, 006aedc0, 006aef70, 006af120, 006af2b0, 006af420, 006af870,
006af8b0, 006afb00, 006afe80, 006afec0, 006aff20, 006aff90, 006b0020, 006b0650, 006b0be0,
006b1190, 006b1490, 006b17b0, 006b1870, 006b1e40, 006b23e0, 006b2d30, 006b2e90, 006b2fc0,
006b3130, 006b3dd0, 006b3fd0, 006b4cb0, 006b5960, 006b5bf0, 006b68c0, 006b7710

Packet: partition segment 29 (`00696470..006BEB10`), lineage cc13, worker cc13-settings1.
Every name below is a descriptive hypothesis, not a recovered symbol.

## Scope and what was skipped

The segment holds four clusters. Two were already covered or belong elsewhere:

- `00696470..006ACxxx`, input settings: keyboard archive writer/reader, device maps and the
  `std::_Tree` instantiations are reconstructed already (`docs/KEYBOARD_SETTINGS_ARCHIVE.md`,
  `docs/INPUT_SETTINGS_TREE_CLUSTER.md`, `src/keyboard_archive.cpp`, `src/keyboard_restore.cpp`).
  This packet adds only the control-preset writer `006A5640`.
- `006B7E30..006B8B10`, the mission `LuaMachine` helpers: cited by recent cc12 documents
  (`CC12_MISSION_LUA_VARIANT_VECTOR_RESERVE_READINESS.md`, `CC12_MISSION_LUA_PAYLOAD_VECTOR_READINESS.md`)
  and by `src/game_hosts_lua.cpp`. Skipped.
- `006BA620..006BEB10`, air operations and landing messages: cc9 gameplay lanes
  (`src/game_hosts_units.cpp`, `docs/SQUADRON_LAND_TASK.md`). Skipped.
- `006AD600` (clears `scoring`, `thisTable`, `Mission`, `MissionGlobals`, `MessageSender`) is
  already reconstructed in `src/mission_load_path.cpp`; not repeated.

The new work is the mission save-data Lua pickler, `006ADDC0..006B7710`, which no earlier
document or source covered, plus the control-preset writer.

## Status

| Item | Exported | Reconstructed | Build-tested | Fixture/game validated |
| --- | --- | --- | --- | --- |
| `006A5640` control preset writer | yes | `src/control_preset_archive.cpp` | yes | no |
| `0069E300`, `0069E9E0`, `006A33F0`, `006A9850` binding display text | yes | `src/input_binding_text.cpp` | yes | `0069E300` probed with the real `GetKeyNameTextA` |
| Lua pickler encode/decode (`006ADDC0..006B68C0`, 26 routines) | yes | `src/mission_savedata_lua.cpp` | yes | host round-trip probe only (below) |
| `006B7710` save tail (register, encode, emit, restore) | yes | wrapper only | yes | no |
| `006B7710` archive header, `006B5960` load tail, `006AF8B0`, `006AFB00` | yes / disk | documented only | n/a | no |
| `006AD8D0` root emitter | yes | documented, host callback | n/a | no |

Nothing here is ABI compatible: the host code takes `lua_State*` and stack indices where the
native code passes 14h-byte `LuaObject` wrappers and 8-byte `NativeString` values by value.

## Reachability

No absolute dword, RVA or rel32 reference to `006B7710`, `006B5960`, `006AFB00`, or to the
unnamed load body at `006AF8B0` exists anywhere in the on-disk image (scripted scan of every
section for the 32-bit value, the RVA, and every `E8`/`E9` target in `.text`). The same holds
for the unnamed archive adapters at `006AD530` (begin section, `006AD470`, end; `RET 0Ch`) and
`006AD560` (begin section, `006AD4D0` = the `00951940` writer twin of `006AD470`, end), which
have no Ghidra function.
Every other routine below is reached only from those four. The mission save/load path therefore
looks like retained but unreferenced code in this build; nothing at run time has been observed
to execute it, and the only save files in this installation are `options.txt`, `save/0/player`
and `save/0/quick` (no `game` kind file). Treat every behavioural statement as static analysis.

## Saved file shape

`006B7710` (`PUSH EBP; MOV EBP,ESP`, no arguments, `RET`) returns at once when the storage
object `[0109CECC]+8 == 1`. Otherwise it builds an `ArchiveTextWriter` on the stack (vtable
`00CE4104`) and writes, in order:

| Key | Value | Evidence |
| --- | --- | --- |
| section `_savedata` | | `006B7762` |
| `playerSlot` | string `[game+650h+38h]` | `006B7781..006B77AA` |
| `missionID` | string `[game+2198h+4]` | `006B77C2..006B77EB` |
| `actSceneFile` | string `[[game+5FCh]+90Ch+4]` | `006B77F6..006B7832` |
| `gameTime` | float `[00F876A4]`, variant type 2 | `006B7837..006B7869` |
| `actUnitID` | object pointer `[00E188D8]`, variant type 4 | `006B786E..006B789A` |
| section `scoring` | `BSP_MissionScoreRecord_WriteArchive` (`00908A30`) on the commit record | `006B78A4..006B78D6` |
| section `_game` | `004B4A30` on the game object | `006B78E2..006B790B` |
| section `_entities` | one integer-keyed section per entity, body from entity vtable `+A8h` | `006B791C..006B7A1D` |
| section `entIDcont` | `cont0` / `cont1` through `00951940` on `00F89A08` / `00F89A5C` | `006B7A22..006B7A9A` |

Variant type 4 is the object-handle kind: the resolvers installed by `006AD0D0`
(`docs/APP_INIT_BOOTSTRAP.md`) turn a pointer into its 16-bit handle at `+174h` and back.

The `_entities` walk takes the list at `[[game+19CCh]+28h]`, skips entities with byte `+5Eh` set
and those whose `+C4h` kind is `44h`, `36h`, `47h`, `4Ah` or `41h`, and keys each by its
handle `+174h`. A second walk over `[[[game+19CCh]+4]]` (next link `+38h`) keeps only kinds
`3Dh`, `29h`, `32h` and keys them `2000h`, `2001h`, ... in walk order.

Then, at `006B7AA2..006B7BBE`, the Lua state is pickled and printed:
`006B3DD0`, `006B68C0`, `006AD8D0` for `Mission`, `MissionGlobals`, `MessageSender` (the
pointer table `00E08DF0..00E08DFC`) and `thisTable`, `006B23E0`, then the function registry is
cleared. `collectgarbage("collect")` runs when `[[game+1A08h]+4] != 0`, and
`BSP_StorageOperation_RunContinuation` finishes with `ECX = 00BD53C0`.

`006AD8D0` (`ECX = NativeString*` global name, `RET`) appends to the save storage through
`BSP_SaveStorage_AppendArchiveBytes` (`00BD49B0`): for a table, `name + " =\n"` (`00CF8288`)
followed by the table dump `0043C530(ECX=&value, EDX=0, 10h, 1)`; otherwise
`name + " = " + 0043C1C0(value) + "\n"`. The save is therefore Lua source. `0043C530` and
`0043C1C0` (segment 2) are not reconstructed here; the host takes the emitter as a callback.

### Load side

`006B5960` (no arguments, `RET`) wraps the global `_savedata` in a `LuaReader` (`004425C0`), reads
`entIDcont` through `006AD470` (`cont0`/`cont1` via `00951820` into `00F89A08`/`00F89A5C`),
`_game` through `004B4A20`, `scoring` through `BSP_MissionScoreRecord_ReadArchive` (`00916A00`),
copies commit-record `+8` to `[game+650h+5Ch]`, resets the mission clock with `0.0`, reads
`actUnitID` as type 4 and passes a non-null result to `BSP_InGameHudRoot_SetSpectatedUnit`.
It then calls `006B3DD0` and `006B23E0`, clears the registry, sets `_savedata = nil` and runs
the same conditional `collectgarbage`. The unnamed `006AF8B0` (disk bytes; no Ghidra function
contains it, the destructor `006AF870` returns at `006AF8AA`) is the earlier load step: it reads
`_savedata` header fields through `BSP_LuaReader_ReadField` (`00BD6830`), calls
`BSP_Game_SetPendingScene` (`004E2770`) and then `006AF420`. That routine clears a
case-insensitive `map<string,int>` at `this+4` and, for each `thisTable` member whose `Dead`
(`00CF829C`) is false and whose `Name` (`00CE8ED0`) is a string, stores `atol(member key)` under
the name, or `-1` when the name is already present (`006AF6BE`), so duplicated names are
ambiguous. The owner object has vtable `00CF82BC` (scalar deleting destructor `006AFEC0`,
destructor `006AF870`) and is constructed by `006AFE80` from `BSP_Game_OnInit` (`004E3AA0`). `006AFB00` collects
the integer keys of `_savedata._entities` and then calls `BSP_SceneEntity_RestoreFromSaveData`
(`006AF780`) in a loop; its pseudocode has unresolved register inputs and the argument flow was
not read from the assembly.

## The Lua pickler

Reconstructed in `include/bsp/mission_savedata_lua.hpp` / `src/mission_savedata_lua.cpp`.

### Markers

| Literal | Address | Meaning |
| --- | --- | --- |
| `__` | `00CF82F0` | path of a table, set during encode, cleared before printing |
| `_t_` + path | `00CF8298` | second reference to an already named table |
| `_f_` + name | `00CF8294` | a function, by its global name |
| `_p_` + handle, `_p_x` | `00CF8290`, `00CEB488` | a light userdata entity, by its `+174h` handle; `x` for null |
| `_R_` | `00CF82EC` | visit mark during restore |
| `|`, `#` | `00CF100C`, `00CF828C` | path separator; marks a number key segment |

A path is `prefix + "|" + segment`, where the segment is the key text for string keys and
`"#"` + `"%d"` (integer key, `004263B0`) or `"#"` + `"%f"` of the float32 key (`006AF2B0` via
`0043BDF0`) otherwise. Roots: `globals.__ = ""`, other table globals `"|name"`, thisTable
`"|thisTable"`, its members `"|thisTable|member"`.

### Key classes

Every walker iterates with `lua_next` (`00B67080`/`00B67190`) and sorts slot keys into vectors by
exact `lua_type`: string keys (`00B660A0`, type 4) as `NativeString`, then (encode and naming
only) integer keys (`00B66A60`: float32 equal to its `CVTTSS2SI` truncation), then other
numbers as **float32** (`00B66270`). Keys of other types are dropped. Number keys are later
looked up with `CVTTSS2SI` truncation (`006B1E65`, `006B4CDD`, `006B13B8`, ...), so a
non-integral key such as `1.5` is read and written through index `1`, and integer keys beyond
float32 precision are rounded. The encode/naming routines come in three key flavours that differ
only in the lookup, the segment formatting and the setter (call-sequence diff of the bodies):

| Role | string key | integer key | float key | C-string key |
| --- | --- | --- | --- | --- |
| naming | `006B1870` | `006B0650` | `006B1E40` | |
| encode table | `006B3FD0` | `006B3130` | `006B4CB0` | `006B5BF0` |
| encode entity | `006AEBF0` | `006AEDC0` | `006AEF70` | |
| encode function | `006B2D30` | `006B2E90` | `006B2FC0` | |
| restore walk | `006B0020` | | `006B0BE0` | |
| restore slot | `006AEA00` | | `006AF120` | |
| resolve `_t_` | `006ADDC0` | | `006AE400` | |
| resolve `_f_` | `006AE120` | | `006AE730` | |
| resolve `_p_` | `006AE280` | | `006AE870` | |

Register ABIs (from the entry sequences and `RET` sizes): naming string `ECX=prefix*, DL=flag`,
stack parent LuaObject + key, `RET 1Ch`; naming integer `ECX=prefix*, EDX=key`, stack parent +
flag, `RET 18h`; naming float `ECX=prefix*, DL=flag`, stack parent + float, `RET 18h`; encode
string `ECX, DL`, stack parent + key + force byte (`[ESP+148h]` in the `128h` frame), `RET 20h`;
encode C-string `ECX=prefix*, EDX=char* key`, stack parent + flag + force, `RET 1Ch`; entity and
function encoders and `006AEA00` take parent + key by value, `RET 1Ch` (`RET 18h` for float);
`006B1190` and `006B1490` take one LuaObject, `RET 14h`. The flag selects `""` versus `"#"`
before the segment; every caller passes 1 for string keys and 0 otherwise.

### Encode (`006B68C0`)

1. `globals.__ = ""` (`006B6904`), so a reference to `_G` itself encodes as `"_t_"`.
2. Collect the key text of every table-valued global. For each, except `thisTable`, `recon`,
   `_G`, `Mission`, `MissionGlobals`, `MessageSender` (case-insensitive), run the naming pass
   with prefix `""` (`006B6DD4`). Naming sets `__` on every reachable table that has none and
   recurses string keys, then integer keys, then float keys; it never rewrites values.
3. Collect the key text of **every** `thisTable` slot (no type test), set
   `thisTable.__ = "|thisTable"`, then `thisTable[k].__ = "|thisTable|k"` for all of them
   before encoding any (`006B7060..006B71E2`), so members refer to each other by their own paths.
4. Forced encode of each member (`006B7280`, force 1).
5. Encode `Mission`, `MissionGlobals`, `MessageSender` when not nil, prefix `""`, force 0
   (`006B734F`). A root already reached from `thisTable` is replaced by a `"_t_"` string.
6. Clear `__` (`006B1190`) on every table global except `recon` and `_G` (`006B75B2`).

Encode of one slot (`006B3FD0`): read `child = parent[key]`; if `force` or `child.__` is nil, set
`child.__` to the path, sort its slots into tables / light userdata (`00B66160`, type 2) /
functions (`00B66200`, type 6), recurse into tables (force 0), then rewrite light userdata as
`_p_`, functions as `_f_`. Otherwise write `parent[key] = "_t_" + child.__` (`006B4B8C..006B4C09`).
Strings, numbers and booleans stay as they are.

`_f_` uses the singleton `00E198F0` (`006B17B0`, 10h bytes, vtable `00CF82E8`, map at `+4`)
filled by `006B3DD0` with `lua_topointer(function) -> global key text`. The lookup is
`operator[]` (`005FF5D0`), so a function that is not a global encodes as `"_f_"` and restores as
`globals[""]`, normally nil. Because the save path restores in place, **saving would also
replace every non-global function in the saved trees with nil**; this is derived from the code,
not observed.

### Restore (`006B23E0`)

1. `globals.__ = ""` again; nothing removes it afterwards.
2. For every table global except `recon`, `_G`, `_savedata`: walk it (`006B0020`). The walk marks
   each table `_R_ = true` on first visit, recurses tables (string keys, then float keys), then
   resolves string slots (string keys, then float keys).
3. Resolve every string-valued global, with no exclusions (`006B295A`): this restores a root that
   was saved as `"_t_..."`.
4. Clear `_R_` with the same exclusions (`006B1490`, cycle safe because it clears before
   recursing).

A slot is resolved when its string has length 3 or more and its first three characters equal
(case-insensitively) `_t_`, `_f_` or `_p_`. `_t_`: start at the globals table, and while the
path is not empty take segment `[start, end)` with `start = 2` when `path[1] == '#'` else 1 and
`end` the next `|` after position 0 (or the length); a `#` segment indexes by
`_ftol(atof(segment))`, any other by C string. `_f_`: `globals[rest]`. `_p_`: `"x"` gives a null
light userdata; otherwise `(WORD)atol(rest)` goes through the same two-bank table as
`BSP_ObjectHandle_Resolve` (`00F89A10`, `00F89A54`/`00F89AA8`, stride 10h, `+C`) with no range
check, and the result is stored as a light userdata.

## Host probe

An ad hoc probe (`local/s1_probe/probe.cpp`, not committed, linked against the built
`bsp_lua511.lib`) built a stock Lua 5.1 state with `thisTable = {a, b}` holding cross references,
a shared global table, a global C function, an anonymous function, a light userdata whose WORD
at `+174h` is 77, a number-keyed alias, `Mission.cur = thisTable.a` and
`MissionGlobals = thisTable.b`. After `encode_mission_lua_state_006b68c0` the dump was:
`a = {link="_t_|thisTable|b", s="_t_|Shared", e="_p_77", fn="_f_GlobalFn", anon="_f_"}`,
`b = {[4]={deep=true}, [3]="_t_|thisTable|b|#4", back="_t_|thisTable|a"}`,
`Mission = {cur="_t_|thisTable|a", own={k=2}}`, `MissionGlobals = "_t_|thisTable|b"`.
After `decode_mission_lua_state_006b23e0` every identity was restored, `anon` was nil, no `__`
or `_R_` remained in the trees, and `_G.__ == ""`. This checks the host implementation against
its own reading of the assembly; it is not a native differential run.

## Binding display text `0069E300`, `0069E9E0`, `006A33F0`, `006A9850`

These build the label the control-binding screens show for one input slot. Host:
`include/bsp/input_binding_text.hpp` over the existing `InputSettings` model.

`0069E300` (`__fastcall`, `ECX` = out string, `EDX` = DirectInput code, `RET`) returns
`"^." + GetKeyNameTextA(lParam)` (import slot `00CE2354`), or `""` for an empty name. `lParam` is
`(code << 16) + 1` unless the code is one of the 20 extended keys in `00CF7F48`, whose lParam
comes from `00CF7F98` (for example `C8h` Up -> `01480001h`, `B8h` Right Alt -> `21380001h`).
Code `0Eh` (Backspace) first reads the name of `1Ch` (Return) through itself and returns
`"^.INDIETRO"` when that is `"^.INVIO"` (case-insensitive), the Italian layout. Native passes an
uninitialised 100h-byte buffer; the host zero-fills it. A local probe with the real API on this
machine returned `^.A`, `^.Up`, `^.Backspace`, `^.Enter`, `^.Right Alt`, `^.Num Lock` and `""`
for `FFh`.

`0069E9E0` (`__thiscall`, `ECX` = settings, out string, binding; `RET 8`) appends
`"." + (index + 1) + "#|"` when the device index is non-zero. For class 2 with a device returned
by `004BA6D0(2, index)` it appends the `ControllerInputNames` label (settings+60h, device name
from virtual `+0Ch`, inner map keyed by code) or the device's virtual `+3Ch` label, and returns.
Otherwise it appends the `0069E300` name for class 0 and then the key of every `InputNames`
entry (settings+24h) whose value+0 equals the binding class and value+0Ch equals its code
(`0069EDBB..0069EDCC`). This is the first consumer evidence for the two `InputNameRecord` fields
that `include/bsp/input_settings.hpp` marks provisional: `primary` is a device class and
`secondary` a key code.

`006A9850` (`__thiscall`, `ECX` = settings, out string, device name, input name, slot;
`RET 10h`) returns `(DEVINPUTS)` when settings+50h is set, `FE_pc.cs_na` for a slot whose class
is `-1`, and otherwise the `0069E9E0` text plus, for axes (class 1 code >= 8, class 2 code >=
3Ch), `|.+`/`|.-` or, for class 1 code 0Ah, `|. |^globals.down`/`|. |^globals.up`, chosen by the
slot's bit in the device's runtime reverse vector. An empty result becomes `FE_pc.cs_unknown`.
Native uses map `operator[]` (`0055C110`, `006A45C0`, `006A4CA0`), so it inserts missing entries
before failing the checked index; the host throws `std::out_of_range` instead.

`006A33F0` (`__thiscall`, `ECX` = settings, `[EBP+8]` = action, `[EBP+0Ch]` and `[EBP+10h]` =
two `NativeString` vectors, `RET 0Ch`; caller `008CC030`) reads slots 0, 2, 1, 3 of one action
through `BSP_InputManager_ReadBindingDescriptor` (`00A92790`) on `BSP_InputManager_GetSingleton`
(`004BEC00`), with the descriptor pre-filled `{-1, 0, 0, 0}`, and skips class `-1`. A button label
goes to the first list when the slot scale is not below zero (`COMISS 0.0, scale; JBE`, so NaN
counts as non-negative) and to the second otherwise. An axis label is pushed to both lists, the
first getting `|.+`/down for a non-negative scale and `|.-`/up for a negative one, the second the
opposite. Reading the lists as positive and negative direction labels is a hypothesis.

## Divergences and uncertainty

- Indexing a non-table (a `thisTable` member that is not a table, a missing `_t_` path step,
  a missing `thisTable`) raises a Lua error natively; the host throws `std::runtime_error`
  before touching the state. Native `__index`/`__newindex` metamethods on non-tables are not
  modelled.
- `006B1190` clears `__` with no visit guard, so a cycle among non-root global tables recurses
  without bound natively. The host stops at `kMissionLuaWalkDepthLimit` with
  `std::length_error`. The mission Lua state opens no `package` library
  (`docs/MISSION_LUA_HOST.md`), and `_G` is excluded, so the standard libraries form no cycle.
- Any genuine Lua string that starts with `_t_`, `_f_` or `_p_` (any case) in any global table is
  rewritten by the restore pass; the format has no escape.
- The emitter `006AD8D0` and the dumpers `0043C530`/`0043C1C0` are not reconstructed, so the
  printed text format and float formatting of values are unknown here.
- The archive header names (`playerSlot`, `actSceneFile`, ...) are literals; the meaning of the
  game-object offsets they read is inferred from those names only.
- Reachability (above) is a negative static result; dynamic dispatch through a computed address
  cannot be excluded.

## Control preset writer `006A5640`

Sole caller `BSP_ControlBindingScreen_Update` (`00560430`) at `00560649`, with `ECX` = the
settings singleton from `005547D0` and a writer object built at `005605C4..0056060E` with the
literal `CONTROL_PRESET` (`00CEE75C`). The body opens section `Preset` (`00CEE818`) and closes it
on the device-loop exit (`006A5A85`). Between them the assembly is the `006A51C0` keyboard body
instruction for instruction except register allocation and the boolean omission test: `006A51C0`
compares `slider`/`reverse` with a false variant through `00BD5680` (two calls), `006A5640` tests
the byte inline (`006A583B`, `006A5926`). Both omit false values, so the rows are the same. The
call-target sequences of the two bodies differ only in those two `00BD5680` calls and in indirect
register names; literal sets are identical apart from `Preset`. The host reuses
`write_keyboard_setup_006a51c0`.

## Evidence method

Exports: `python tools/bsp.py ghidra export` for every listed function (project and program
verified by the client). Assembly read through `local/s1_asm.py` (annotates call targets and
`.rdata` strings from the PE on disk). Template identity was checked by diffing call and literal
sequences between flavours. Reachability used scripted dword/RVA/rel32 scans of the on-disk
image. No Ghidra state was changed by the analysis itself; renames and plate comments were
applied afterwards through `tools/ghidra_annotate.py --apply` from the ledger records.

The call sites cited above are listed in `reports/cc13_settings_savedata.json` for
`tools/verify_report_calls.py`.

## Handoff

Done: segment survey, the whole mission Lua pickler (encode, naming, strip, restore, resolve),
control preset writer, binding display text (`0069E300`, `0069E9E0`, `006A33F0`, `006A9850`), 46 ledger
names applied to Ghidra with plate comments, report rows verified by
`tools/verify_report_calls.py` (54 rows, 0 failed).

Open, in priority order:

1. Resolved: `0043C530`, `0043C1C0` and the emitter `006AD8D0` are reconstructed in
   `src/lua_text_dump.cpp`; see `docs/CC13_PROFILE_SAVE.md`.
2. `006AFB00` argument flow (register inputs in pseudocode) and the unnamed load body
   `006AF8B0..006AFAF1` (header fields read through `00BD6830`).
3. The hidden adapters `006AD4D0`, `006AD530`, `006AD560` and the twins `00951820`/`00951940`
   (entity-id registry archive read/write, segment of `00951xxx`).
4. Remaining raw functions in `00696470..006AD3F0` are mostly `std::_Tree`/vector
   instantiations. `006A9A40` (rebinding with `BSP_InputSettings_SuppressAlternateBinding`) and
   `006AAC50` (`|. (|`/`|.)|` conflict text, called from the binding screens) are the next
   string-anchored targets.
5. Whether anything reaches `006B7710`/`006B5960` at run time; a breakpoint on either during a
   mission would settle it.
