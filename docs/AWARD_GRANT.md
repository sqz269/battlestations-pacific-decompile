# The award grant path (packet `award_grant_path`)

Addresses: 00a41030, 00a40d60, 0090c5d0, 00a3e520, 004b44f0, 006b8da0, 00a3e510, 00a3ead0

## Headline

**`00a41030` and `00a40d60` are not grant entry points.** They are the compiler-emitted
`std::vector<int>::push_back` and `std::vector<int>::insert` that `00a410a0`
(`BSP_OnlineAwards_GrantIfSessionActive`) uses to append an achievement id to the queue at
`manager+360h`. The real grant entry is `00a410a0`, already named by `game_frontend_shell_entry`,
and the queue is drained by `00a3fa70`, which is the only caller of `XUserWriteAchievements`
(`00a4d596`) in the image. Nothing on this path writes a local profile; the local half is the
separate `BSP_AwardTracker_RecordAtLeast` call at `004e43d5`.

**The name-to-id map holds no names of its own.** `00E19900` is a pointer to a 20h byte object
`BSP_Game_OnInit` builds at `004e3d7f`..`004e3daf` with the constructor `006b9450`, and that
constructor parses `Scripts\datatables\Achievements.lua`. The award names and the ids both come
from that shipped data file, not from the executable: the only award name in `.text` is `GA_HM`
at `00CE824C`.

## The registry at `00E19900`

`004e3d7f` allocates 20h bytes, `004e3d98` runs `006b9450` on them and `004e3daf` stores the
pointer. The object is a `std::vector` at `+0h`..`+0Fh` (cleared by the erase at the top of the
constructor) and a `std::map` at `+10h`: head node `+14h`, size counter `+1Ch`, which the
constructor increments once per parsed row (`006b9c56`).

`006b9450` string references: `Scripts\datatables\Achievements.lua`, `Achievements`, and the
column names `Name`, `ID`, `Description`, `GUITexture`, `Sound`, `Multi`, `Params`, `Unlock`,
`Score`, `XLastAchievementID`, `Index`. Each row key becomes the map key and the row becomes the
map value, whose fields are:

| Value offset | Column | Reader |
| --- | --- | --- |
| `+00h` | `Name` (localisation key) | string, `00b662b0` |
| `+08h` | `ID` (GUI icon index) | number, `00b66290` |
| `+0Ch` | `Description` | string |
| `+14h` | `GUITexture` | string |
| `+1Ch` | `Sound` | string |
| `+24h` | `Multi` | byte, `00b66250` |
| `+28h` | `Unlock` | string |
| `+34h` | `Index` | list of integers |
| `+3Ch` | `Score` | `BSP_LuaReference_GetIntegerOrDefault(0)` |
| `+40h` | `XLastAchievementID` | `BSP_LuaReference_GetIntegerOrDefault(-1)` |

`0050fc30` is the map's `operator[]`; it returns `node+14h`, which fixes the value base. Its
comparator is `00443d00` `BSP_NativeString_LessCaseInsensitive`, so the map is ordered and looked
up **case-insensitively**. The key is a `NativeString`, eight bytes (`{length, pointer}`), which is
why `node+0Ch` (`pair::first`) plus 8 gives the value at `node+14h`.

### `006b8da0` `BSP_AwardRegistry_GetAchievementId`

`__thiscall(this, const NativeString* name)`, RET 4, ECX = `*(00E19900)`. It calls the map find
`006b8ce0`, checks the returned checked iterator's `_Mycont` against `this+10h` and its `_Ptr`
against the head node at `this+14h`, and on a hit returns the dword at `node+54h`. `54h - 14h =
40h`, so the returned value is the `XLastAchievementID` column. A miss returns 0.

### The 1..99 range check

`004e437a` is `LEA EAX,[ESI-1]; CMP EAX,0x62; JA`, and `0090ef86` repeats it as an unsigned
`(id - 1) < 99`. The check exists because the registry answers for every row: rows without the
column answer the `-1` default and the `RANK` row answers `0`. Both fall outside 1..99, so the
range check is what separates the 52 uploadable achievements from the 22 local-only badges
(`BW_`, `BU_`, `BO_` prefixes) and the counter row.

### The recovered table

Read from the retail `scripts/datatables/achievements.lua` of the Steam install, resolving each
`["XLastAchievementID"] = ACHIEVEMENT_x` through the constant block at the head of the same file.
52 rows land in 1..99, densely covering 27..78 with no duplicates. `GA_HM` is 76. The table is
reproduced in `include/bsp/award_grant.hpp`; it mirrors shipped data, not recovered code, and a
different build of the data file would change it.

## The online gate at `00F8ABE8`

The manager object is the one `docs/GAME_SESSION_POLLS.md` projects as `PlatformManagerFlags`;
these four fields are disjoint from the six recorded there.

| Offset | Meaning | Writer |
| --- | --- | --- |
| `+8Ch` | per-slot sign-in state, dword per slot | `00a40510`, `00a40020` |
| `+119h` | a user slot has been selected | `00a40510` state 4 (`00a408c3`) |
| `+11Ah` | the selected account is LIVE enabled | `00a40510` state 5 (`00a40925`) |
| `+11Ch` | the selected XUser slot index | `00a40510` state 4, copied from `+3B4h` |

`00a40510` state 5 calls `XUserGetSigninInfo(this+11Ch, 1, &info)` and stores `info.dwInfoFlags &
1` into `+11Ah`; the flag is `XUSER_INFO_FLAG_LIVE_ENABLED`. `00a3e6a0` clears `+119h` and `+11Ah`
together and resets `+11Ch` to 1. The per-slot values are fixed by `00a3fbb3`, which selects the
literal `"LIVE"` when `*(this+8Ch+this+11Ch*4) == 2` and `"Local"` otherwise, so 2 is LIVE, 1 is a
local profile and 0 is signed out.

Three accessors read them, all one or two instructions with no bound check on the slot index:

- `00a3e510` `BSP_XenonSystemManager_HasSelectedUser`: `MOV AL,[ECX+119h]; RET`.
- `00a3e520` `BSP_XenonSystemManager_IsLiveEnabledAccount`: `MOV AL,[ECX+11Ah]; RET`.
- `00a3ead0` `BSP_XenonSystemManager_GetSelectedSignInState`: `MOV EAX,[ECX+11Ch];
  MOV EAX,[ECX+EAX*4+8Ch]; RET`.

`004b44f0` `BSP_XenonSystemManager_IsSignedIntoLive`, `__thiscall(this)`, RET, EAX 0 or 1, is
`00a3e510 && 00a3ead0 == 2`.

`00a410a0`'s own gate, inlined at `00a410a0`..`00a410cc`, is weaker: `+119h` set, slot state 1 or
2, `+11Ah` set. It accepts a local profile. The call site at `004e4000` is stricter because it
demands `004b44f0` first, so on that path the inner gate never rejects anything the outer one
passed.

## `0090c5d0` `BSP_LegacySave_MidwayFolderPreexisted`

No arguments, RET. The call site at `004e42ec` loads `ECX` from `*(*(00E188A8)+21A0h)`, a double
dereference (`docs/GAME_FRONTEND_ENTRY.md` records it as one), but the body never reads `ECX`.

It calls `SHGetSpecialFolderPathA(0, buf, 5, 0)` (`CSIDL_PERSONAL`), concatenates
`\Battlestations-Midway\save` onto the result and calls `CreateDirectoryA`. It returns **1 only
when the create fails with `ERROR_ALREADY_EXISTS` (0B7h)**. A run that actually creates the folder
returns 0, and so does a failed `SHGetSpecialFolderPathA`. `GA_HM` is therefore the returning
player award: it is granted on the first front end entry of a machine that already carried a
Battlestations: Midway save directory, and the folder this routine creates as a side effect makes
the next run's probe succeed. Note it is created before the id lookup, so the award is reachable
on the second launch of a machine that never had Midway installed.

## `00a41030` and `00a40d60`, the queue append

`00a41030` `BSP_StlVectorInt_PushBack`, `__thiscall(this, const int* value)`, RET 4. MSVC 8
`std::vector` layout: allocator pad `+0h`, `_Myfirst +4h`, `_Mylast +8h`, `_Myend +0Ch`. When
`size < capacity` it stores `*value` at `_Mylast` and advances it (`00a41059`..`00a4106e`);
otherwise it builds a checked end iterator and tail-calls the insert.

`00a40d60` `BSP_StlVectorInt_Insert`, `__thiscall(this = ECX)` with four stack dwords and **RET
10h**: `[ESP+4]` the returned iterator (sret), `[ESP+8]` `_Where._Mycont`, `[ESP+0Ch]`
`_Where._Ptr`, `[ESP+10h]` the value pointer. It computes the offset, calls `00a40b90` (`_Insert_n`,
which reallocates and can throw through `00a40ad0`) and returns `{_Mycont = this, _Ptr = _Myfirst +
offset}`.

Both are shared instantiations: `00a40d60` also has the caller `00bb7c50`, so neither is award
specific.

## What the queue is for

`00a410a0` pushes onto `manager+360h` (`ADD ECX,0x360` at `00a410d3`). `00a3fa70`, reached from
`BSP_XenonSystemManager_DrainNotifications` `00a40110` and from `00a409f0`, is the pump:

1. Skips unless `+364h`..`+368h` is non-empty.
2. When no write is outstanding (`+3A8h != 3E5h`, `+3A4h == 0`, `+384h != 3E5h`) or the caller
   forces it, it allocates an eight-byte-per-entry array at `+3A0h`, fills each entry with
   `{dwUserIndex = *(this+11Ch), dwAchievementId = queue[i]}`, logs
   `Write to UpLoad Queue Achievement %s %d: %d` with `"LIVE"` or `"Local"`, and calls
   `XUserWriteAchievements(count, array, this+384h)` with the `XOVERLAPPED` at `+384h`.
3. On completion (`XGetOverlappedResult`) it erases every written id from the queue with
   `memmove_s`, zeroes the count and frees the array; `XGetOverlappedExtendedError` logs
   `Achievement Upload ERROR %x`.

`3E5h` is `ERROR_IO_PENDING`, the same constant `session_polls.hpp` already names
`kAsyncStatusPending`.

## The call site at `004e4000`

Disassembled from disk (`004e42e6`..`004e43e2`); Ghidra's stored body for `004e4000` is truncated,
so `disasm-raw` was used.

```
004e42e6  MOV ECX,[00E188A8]; MOV ECX,[ECX+21A0h]; CALL 0090c5d0   ; -> BL
004e432f  PUSH 00CE824C ("GA_HM"); LEA ECX,[ESP+20h]; CALL 0041e870
004e433d  MOV ECX,[00E19900]; LEA EDX,[ESP+1Ch]; PUSH EDX; CALL 006b8da0  ; -> ESI
004e437a  LEA EAX,[ESI-1]; CMP EAX,62h; JA 004e43e7
004e4382  MOV ECX,[00F8ABE8]; CALL 00a3e520; TEST AL,AL; JZ 004e43e7
004e4391  MOV ECX,[00F8ABE8]; CALL 004b44f0; TEST AL,AL; JZ 004e43e7
004e43a0  MOV ECX,[00F8ABE8]; PUSH ESI; CALL 00a410a0
004e43ac  PUSH 00CE824C; LEA ECX,[ESP+28h]; CALL 0041e870
004e43c1  MOV ECX,[00E188A8]; ADD ECX,650h; PUSH 1; CALL 007fbe20
```

So the full predicate is: the Midway save folder pre-existed, `GA_HM` resolves to an id in 1..99,
the account is LIVE enabled, and the selected slot is signed in to LIVE. The local
`BSP_AwardTracker_RecordAtLeast` call is inside the same branch, so this build never records the
award locally without also queueing it online.

The `game+6F0h` map probe at `004c8b80` that `docs/GAME_FRONTEND_ENTRY.md` records as step 1 sits
before `004e42e6` and belongs to `game_award_trackers`; it was not re-read here.

## The other call site, `0090ede0`

`0090ede0` (not leased by this packet, read only) is the general in-mission grant. Its tail repeats
the same three steps — `006b8da0`, the unsigned `(id - 1) < 99` check, `00a410a0`, then
`BSP_AwardTracker_RecordAtLeast` — but with different guards: no folder probe and no explicit
sign-in test, and two name prefix rejections through `00553c80` against `"RANK"` at `00CEF15C`
(only when the name is longer than five characters) and `"Counter"` at `00D18794`. The retail table
does contain `RANK_FR` (67) and `RANK_SR` (68), so those two ids exist but this path never uploads
them. `00D18794` onwards is a dense `.rdata` block of award identifiers (`GA_AE`, `SA_OC`, `SA_IM`,
`GA_AI`, `GA_DN`, `Counter_RUA_DBU`, `BO_*`, `BU_*`, `RUA_TBU`, `GA_PL`, `GA_AO`, `GA_DB`, …) used
by the in-mission code that calls it.

## Reconstruction

`include/bsp/award_grant.hpp` and `src/award_grant.cpp`:

- `kAwardNameIds[52]`, the recovered name-to-id table, plus
  `award_id_for_name_006b8da0`, a case-insensitive lookup returning 0 for an unknown name.
- `is_grantable_award_id`, using `kAwardIdMin` / `kAwardIdMax` from `frontend_entry.hpp` rather
  than redefining them.
- `OnlineSignInState` and the four gate predicates
  `selected_slot_state_00a3ead0`, `live_enabled_account_00a3e520`, `signed_into_live_004b44f0`,
  `may_queue_award_00a410a0`.
- `midway_save_folder_preexisted_0090c5d0(create_succeeded, last_error)`, the Win32 result
  reduction.
- `should_grant_award_004e4310(AwardGrantInputs)`, the whole call-site predicate as a pure
  function.
- `AwardGrantHost` with one method per native call site of the block, and
  `grant_award_if_earned` / `grant_midway_save_award`, which short-circuit exactly where the
  native branches to `004e43e7`.

The STL instantiations are deliberately not ported. The upload queue offsets are recorded as
constants only.

## State reached

| Address | Name | State |
| --- | --- | --- |
| `006b8da0` | `BSP_AwardRegistry_GetAchievementId` | reconstructed, build-tested |
| `0090c5d0` | `BSP_LegacySave_MidwayFolderPreexisted` | reconstructed, build-tested |
| `00a3e520` | `BSP_XenonSystemManager_IsLiveEnabledAccount` | reconstructed, build-tested |
| `00a3e510` | `BSP_XenonSystemManager_HasSelectedUser` | reconstructed, build-tested |
| `00a3ead0` | `BSP_XenonSystemManager_GetSelectedSignInState` | reconstructed, build-tested |
| `004b44f0` | `BSP_XenonSystemManager_IsSignedIntoLive` | reconstructed, build-tested |
| `00a41030` | `BSP_StlVectorInt_PushBack` | analyzed, not ported (STL) |
| `00a40d60` | `BSP_StlVectorInt_Insert` | analyzed, not ported (STL) |
| `00a410a0` | `BSP_OnlineAwards_GrantIfSessionActive` | analyzed (already named) |
| `006b9450` | registry constructor | analyzed, not named (outside the lease) |
| `00a3fa70` | achievement upload pump | analyzed, not named (outside the lease) |
| `00a40510` | sign-in state machine | read for flag provenance only |
| `0090ede0` | general grant helper | read for the second call site only |

Nothing here is game-validated. Every function named in this packet has a Ghidra function; none
needed defining.

## Uncertainties

- The vector at registry `+0h`..`+0Fh` is cleared by the constructor and never read on this path.
  Its element type is unknown.
- `006b9450` also fills `Params` (indices 1..21) and `Index` sub-tables into the value; the two
  container offsets `+30h` and `+34h` are inferred from `00442190` and `004857f0` call shapes only.
- `0090c5d0`'s `ECX` is loaded but unread. Whether it is a member of the object at `game+21A0h`
  compiled without `this` use, or a free function reached through a stale register, was not
  settled.
- `00a3e6a0`'s `MOV EDX,1; MOV [ECX+11Ch],EDX` resets the slot index to 1 rather than 0. Why 1 is
  the resting index was not established; `session_polls.hpp` already records `kNoLocalSlot = 1`
  for the same manager, which is probably the same convention.
- Slot count: `+8Ch` is indexed without a bound check and only `+8Ch`..`+9Bh` was observed in use,
  so four slots is an inference from the console's pad count, not from a bound in the code.
- The id table is read from one retail install. Patch level was not checked.

## Follow-up packets proposed

- `achievement_upload_pump`: `00a3fa70`, `00a409f0`, `00a40110`, files `docs/AWARD_UPLOAD.md`,
  `include/bsp/award_upload.hpp`. Contract: the `XUSER_ACHIEVEMENT` array at `manager+3A0h`, the
  `XOVERLAPPED` at `+384h`, the `ERROR_IO_PENDING` state machine across `+3A4h`/`+3A8h`, and the
  erase-on-completion loop; who calls the pump each frame and with what force flag.
- `award_registry_construction`: `006b9450`, `0050fc30`, `006b8ce0`, `006b8d50`, `00b67800`,
  `00b662b0`, files `docs/AWARD_REGISTRY.md`, `include/bsp/award_registry.hpp`. Contract: the
  whole `Achievements.lua` row schema including `Params` and `Index`, the vector at `+0h`, and the
  Lua table-walk helpers `00b65f50`/`00b660a0`/`00b66420` this constructor shares with every other
  data table loader.
- `in_mission_award_grant`: `0090ede0`, `00553c80`, `005070c0`, `00648ab0`, files
  `docs/AWARD_IN_MISSION.md`. Contract: the counter that `005070c0` returns per award name, the
  `RANK` and `Counter` prefix rejections, and how a mission event reaches this routine.
- `xenon_signin_state_machine`: `00a40510`, `00a40020`, `00a3e6a0`, `00a3ed10`, files
  `docs/XENON_SIGNIN.md`. Contract: the eight states at `manager+3B0h`, the `XShowSigninUI` /
  `XShowMessageBoxUI` prompts and which one sets `+119h`, `+11Ah` and `+11Ch`.
