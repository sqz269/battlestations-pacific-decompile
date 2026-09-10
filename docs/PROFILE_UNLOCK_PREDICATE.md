# The profile unlock predicate (packet `profile_unlock_predicate`)

Addresses: 007fc4c0, 007fc820, 0090c560, 004c7f10, 004d0640, 005097d0, 007f8890, 007fb990,
007fdf00, 008bbc40, 008cb920, 005c5990

What makes a mission, a unit or an award available is one string test against the player-profile
block at `game+650h`. `007FC820` is the all-of over a mission's requirement vector and `007FC4C0`
is the any-of over one requirement's tokens. The game's own name for `007FC4C0` is
`Scoring_IsUnlocked`: the Lua binding `008BBC40` carries the literal
`luaMW_Scoring_IsUnlocked failed:` and returns nothing but this routine's result.

## The requirement model

A requirement is **not** a structured record. It is a plain native string.

| Level | Native shape | Combination |
| --- | --- | --- |
| Requirement container, `record+78h` | `std::vector<NativeString>`: proxy `+78h`, first `+7Ch`, last `+80h`, end `+84h`, element stride 8 | **all-of**, `007FC820` |
| One requirement | `NativeString` `{ dword length; char* data; }` | — |
| Token | the `" ,"` split of `data` | **any-of**, `007FC4C0` |
| Token match | one of five profile containers | first source that answers wins |

`007FC820` returns `AL = 1` before reading anything when `first == last` (007FC844), so an empty
container means available. `007FC4C0` returns 0 when the split yields no token, so an **empty
requirement string locks its subject** — which is why `00584750` and `0050A400` compare `UnlockID`
against the empty string at `00CE3A0C` and skip the call when it matches.

### Where the data comes from

- **Missions.** `Scripts/datatables/missiontree.lua`, key `prerequisites`, a Lua array of strings
  that becomes the vector at `record+78h`. All 188 shipped entries: 131 are `{}` and 57 hold
  exactly one token, always a bare mission id (`{ "IJN04" }` 24 times, `{ "IJN17" }` 6, `{ "CHG09" }`
  4, `{ "USN02" }` 2, then `BSM01..BSM10`, `CHG01..CHG08`, `IJN03`, `JM08`, `USN01`, `USN20` once
  each). No shipped `prerequisites` entry has more than one element and none contains a separator,
  so **the multi-token any-of and the multi-element all-of are both unexercised by shipped data**.
- **Units.** `Scripts/datatables/autoload/vehicleclasses.lua`, keys `Unlock` (boolean) and
  `UnlockID` (string). `00584750` reads `globals.VehicleClass[index]`, and only when `Unlock` is
  `true` and `UnlockID` is a non-empty string does it call the predicate. Every shipped `UnlockID`
  is of the form `<MISSIONID>_GOLD` (`IJN12_GOLD`, `YAMA_GOLD`, `USNSY_GOLD`, …).
- **Scripts.** Mission Lua calls `IsUnlocked("JM2_GOLD")`, `IsUnlocked("JM6_SILVER")`,
  `IsUnlocked("JM6_BRONZE")` directly, and `Scoring_GrantUnlock` writes those same ids.

So the distinct requirement kinds are not a tagged enum in the data; the kind is decided at
evaluation time by **which profile container the token is found in**. Shipped tokens fall into two
shapes: a bare mission id (mission completion) and a `<MISSIONID>_<MEDAL>` id (a granted unlock).

## `007FC4C0`, one requirement against the profile

`__thiscall char BSP_Profile_IsUnlockSatisfied(this = game+650h, const NativeString* requirement)`,
`RET 4`, SEH frame `00C8FC28`, body `007FC4C0..007FC810`.

1. `007FC4EE` reads `requirement->data`; a null pointer is replaced by the empty string at
   `00F8745C`.
2. `007FC504` copies it into `char[256]` at `ESP+64h` with an unbounded
   `do { *dst++ = *src++; } while (c);` loop. **A requirement longer than 255 bytes smashes the
   native frame.** The reconstruction does not reproduce that.
3. `007FC520` calls `_strtok(buffer, " ,")`; the separator literal at `00D08D18` is a space and a
   comma.
4. Per token it builds a temporary `NativeString` (`0041DD40` + `_memcpy`, released through the
   sized storage pool) and tries five sources in order. The accumulated byte at `ESP+13h` is tested
   at the head of the next iteration (`007FC554`) and returned, so **the first satisfied token ends
   the walk**.

| Order | Site | Container | Test | Meaning |
| --- | --- | --- | --- | --- |
| 1 | `007FC5A1` `MOV ECX,[EDI+64h]`, `0090C560` | the 24h-byte object at `profile+64h` | key present **and** `node->value != 0` | the mission of that id is completed |
| 2 | `007FC602` `LEA ESI,[EDI+70h]`, `004C7F10` | `set<NativeString>` at `profile+70h`, saved as `"Unlocks"` | found | the unlock was granted and persisted |
| 3 | `007FC65B` `ADD ESI,7Ch`, `004C7F10` | `set<NativeString>` at `profile+7Ch` | found | granted this session, not yet folded into `Unlocks` |
| 4 | `007FC6C2` `005097D0` | `map<NativeString,int>` at `profile+A0h` | `TEST EAX,EAX; JG`, so **strictly > 0** | a named counter: `RANK` and the award names |
| 5 | `007FC6DD` `007F8890` | `list<NativeString>` at `profile+D0h` | found | the id is an owned downloadable-content name |

`ECX` for sources 4 and 5 is reloaded from `[ESP+18h]`, which holds the incoming `this`
(`007FC4F5`), so both are profile methods. All five compare case-insensitively:
`004C7F10`, `004C8B80` (inside `005097D0`) and `0090C100` (inside `0090C560`) all order through
`BSP_NativeString_LessCaseInsensitive` `00443D00`, and `007F8890` compares length then `__stricmp`.

Note the asymmetry between source 1 and source 4: the mission table accepts any non-zero value, the
counter map requires a strictly positive one.

## `007FC820`, a whole requirement vector

`__thiscall char BSP_Profile_TestUnlockRequirements(this = game+650h, vector<NativeString>* v)`,
`RET 4`, body `007FC820..007FC87E`. `EBP` holds `this` across the loop and each element is passed
on the stack with `ECX = EBP` (`007FC852`). `007FC85C` returns `AL = 0` on the first element that
fails; `007FC876` returns `AL = 1` when `first == last`.

## The five profile containers

The block at `game+650h` is reset by `BSP_PlayerProfile_ResetToDefaults` `007FDB20`
(docs/GAME_TITLE_INIT.md) and serialized by `BSP_PlayerProfile_Serialize` `007FDF00`.

| Offset | Absolute | Shape | Written by | Saved as |
| --- | --- | --- | --- | --- |
| `+64h` | `game+6B4h` | pointer to a `24h`-byte object holding `map<NativeString,int>`; `007FDB20` rebuilds it with `new(24h)` + `00920E10` | mission completion; `005C4080` queries it with `record+00h` for every `434h` record | rebuilt by `007FDF00` |
| `+70h` | `game+6C0h` | `set<NativeString>`, case-insensitive | `007FB990` folds the pending set in | `"Unlocks"` (`00D08C6C` at `007FE29C`) |
| `+7Ch` | `game+6CCh` | `set<NativeString>`, case-insensitive | `008CB920` `Scoring_GrantUnlock`, `ADD ECX,6CCh` | not saved |
| `+88h` | `game+6D8h` | `set<NativeString>`, case-insensitive | `007FDF00` itself | `"SeenUnlocks"` (`00D08C60` at `007FE9E2` + `LEA ECX,[EDI+88h]`) — **not read by the predicate** |
| `+A0h` | `game+6F0h` | `map<NativeString,int>` | `007FDB20` seeds `map["RANK"] = 1` at `007FDDE3`; `BSP_AwardTracker_RecordAtLeast` `007FBE20` raises award entries | inside the profile sections |
| `+D0h` | `game+720h` | `list<NativeString>` node `{next, prev, length, data}` | not written by `007FDF00`; ids like `DL_Content_0000062` (`008D57A0`, `00706760`) | not saved |

**Correction to docs/GAME_TITLE_INIT.md.** That doc records `+74h`, `+80h`, `+8Ch` as "three
`std::list`s cleared through `004CEC60`". Those three dwords are the `_Myhead` fields of three
`std::set<NativeString>` red-black trees whose objects start at `+70h`, `+7Ch` and `+88h`; the
`004D0640` insert descends on `_Isnil` at `node+15h` and the key sits at `node+0Ch`. The same doc's
"a keyed map, `map["RANK"] = 1` through `005070C0`" is the map at `+A0h`: `ESI` at `007FDDDB` is the
container `0058B520` had just cleared at `007FDD89`, which `docs/GAME_TITLE_INIT.md` itself ties to
`+A4h`.

The lifecycle of an unlock id is therefore: mission Lua calls `Scoring_GrantUnlock("JM2_GOLD")`
→ `+7Ch`; the debriefing screen `0062C2C0` lists `+7Ch` under `FE.scoring_unlockedunit` and calls
`007FB990`, which moves every entry into `+70h` and empties `+7Ch`; `007FDF00` saves `+70h` as
`"Unlocks"`. The predicate accepts an id in either set, so a grant counts immediately.

## Callers, and what a false result costs

| Caller | Subject | On false |
| --- | --- | --- |
| `007FC820` | one element of a mission's `prerequisites` | the whole vector fails |
| `00597870` (main menu) | `007FC820(record+78h)` per mission | the entry is labelled with the string `confidental` (`PTR_s_confidental_00E08870`) |
| `005C57D0` `BSP_MissionTreeScreen_ActivateSelectedMission` | `007FC820(record+78h)` | activating the mission does nothing (docs/MISSION_TREE_BRIEFING_SCREENS.md) |
| `005C5990` | every requirement of a list of missions | the failing requirement string is inserted into a local set: the unmet-prerequisite list |
| `00584750`, `0050A400` | `globals.VehicleClass[i].UnlockID` | the vehicle-class entry reports unavailable; `UnlockID` nil or empty short-circuits to available |
| `004FAC50` (reached from `00511F40`, `FE.taclib_unlocks_help`) | the `NativeString` at `+30h` of each `6Ch`-byte row in the vector at `this+3C8h` | writes `1` to the row's byte at `+45h`, the locked flag, and continues |
| `0050EF80` | a token built from a caption vector | takes the locked arm: copies a replacement caption out of the vector at `this+180h` instead of the live one at `this+170h` |
| `008BBC40` | the Lua argument of `Scoring_IsUnlocked` | returned to the script unchanged |

## Reconstruction

`include/bsp/profile_unlock.hpp` and `src/profile_unlock.cpp`.

- `ProfileUnlockState` is the projection of the block: the two counter maps, the two unlock sets,
  and the content-id list, all keyed with `NativeStringCaseInsensitiveLess`.
- `parse_unlock_expression_007fc4c0` is the `strtok` split, including `strtok`'s collapsing of
  separator runs, so `"IJN04,,JM8_GOLD"` and `"IJN04 JM8_GOLD"` parse alike.
- `classify_unlock_token_007fc4c0` returns the `UnlockSource` that answered, in native order.
- `is_unlock_expression_satisfied_007fc4c0` and `are_unlock_requirements_met_007fc820` are the
  any-of and the all-of. The latter takes `const std::vector<std::string>&`, which is exactly
  `MissionRecord::unlock_requirements` from `include/bsp/mission_tree_screens.hpp`.

`kUnlockExpressionBufferSize` records the native's 256-byte stack buffer; the reconstruction does
not reproduce the overflow. These are new C++ interfaces, not drop-in binary replacements.

## Uncertainties

- The mission-completion object at `profile+64h` is `24h` bytes and the map is only its first
  member as far as this packet read it. `0090C560` uses `object+4h` as the end node, so the map is
  at offset 0; what the remaining `1Ch` bytes hold is unread.
- Whether the two unlock containers are `std::set<NativeString>` or `std::map<NativeString, X>` is
  settled only for the insert path: `004D0640` links a node whose value is the bare 8-byte string
  and `004C7F10` compares against `node+0Ch`, with no mapped field touched anywhere. A map with a
  zero-size mapped type would be indistinguishable.
- The `RANK` key is seeded to 1 by the reset, so a requirement token `"RANK"` would always pass. No
  shipped data uses it.
- No shipped requirement exercises more than one token or more than one element, so the any-of and
  all-of shapes are recovered from the code only.
- `007FDF00` was read by grep over its listing for the two unlock keys, not end to end. The
  section layout, the `Version` handling and where `+A0h` is written are not recovered.
- `005C5990` has a fall-through gap after the `_free` at `005C5B19` (`005C5B1E..005C5B21` decodes as
  `ADD ESP,4` / `MOV ECX,[ESP+28h]`), so the tail that consumes its collected set is outside the
  stored body. Not repaired here; `bsp.py ghidra flow 005c5990 --apply` would.
- `007FC4C0` shows a 5-byte gap at `007FC54B..007FC550`, but it follows a `RET 4` rather than a
  call, and `007FC550` is a live jump target, so nothing is missing from the body.

## State reached

| Address | Name | State |
| --- | --- | --- |
| 007fc4c0 | BSP_Profile_IsUnlockSatisfied | reconstructed, build-tested, installed-file-checked |
| 007fc820 | BSP_Profile_TestUnlockRequirements | reconstructed, build-tested, installed-file-checked |
| 0090c560 | BSP_MissionProgress_IsCompleted | analyzed |
| 004c7f10 | BSP_NativeStringSet_Find | analyzed |
| 004d0640 | BSP_NativeStringSet_Insert | analyzed |
| 005097d0 | BSP_Profile_GetNamedCounter | analyzed |
| 007f8890 | BSP_Profile_OwnsContentId | analyzed |
| 007fb990 | BSP_Profile_FlushPendingUnlocks | analyzed |
| 007fdf00 | BSP_PlayerProfile_Serialize | exported, partially analyzed |
| 008bbc40 | BSP_LuaBinding_ScoringIsUnlocked | analyzed |
| 008cb920 | BSP_LuaBinding_ScoringGrantUnlock | analyzed |
| 005c5990 | BSP_MissionTreeScreen_CollectMissingPrerequisites | analyzed |
| 00584750, 0050a400, 004fac50, 0050ef80, 00597870 | — | analyzed as callers only |

Every function above already has a Ghidra function, so there is no `no_ghidra_function` entry.

## Follow-up packets

- `profile_serialize_sections` — `007fdf00`, `007fd780`, `00920e10`, `0090bf50`, files
  `docs/PROFILE_SERIALIZE.md`, `reports/profile_serialize.json`, `include/bsp/profile_save.hpp`,
  `src/profile_save.cpp`. Contract: the section format `BSP_PlayerProfile_Serialize` reads and
  writes, which of the 21 keys map to which offsets in the `game+650h` block, and what the
  `24h`-byte mission-progress object holds beyond its map.
- `unit_library_availability` — `00584750`, `0050a400`, `004fac50`, `00511f40`, files
  `docs/UNIT_LIBRARY_AVAILABILITY.md`, `reports/unit_library_availability.json`,
  `include/bsp/unit_library.hpp`, `src/unit_library.cpp`. Contract: the `6Ch`-byte unlock row at
  `screen+3C8h`, how `vehicleclasses.lua` fills it, and what the locked flag at `+45h` changes in
  `FE.taclib_unlocks_help` and `fe/lobby/unlock_disabled.tga`.
- `debriefing_unlock_flush` — `0062c2c0`, `0062a970`, `007fb990`, `005c3be0`, files
  `docs/DEBRIEFING_UNLOCKS.md`, `reports/debriefing_unlocks.json`. Contract: how the debriefing
  screen builds the `FE.scoring_unlockedunit` list from `profile+7Ch` and when it flushes.
- `dlc_content_ownership` — `007f8890`, `008d57a0`, `00706760`, `004fd8d0`, files
  `docs/DLC_OWNERSHIP.md`, `reports/dlc_ownership.json`. Contract: who fills the list at
  `profile+D0h`, and what `UnlockDLCMinimum` gates.
