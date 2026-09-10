# The mission-tree and briefing screens (packet `mission_tree_briefing_screens`)

Addresses: 005CA880 005CA8E0 005CAA20 005CAAF0 005C27F0 005C2800 005C4040 005C3470 005C3870
005C57D0 005C5600 00439020 0051E4D0 0051E250 0051E270 0051E280 0051E5F0 0051CDE0 0051C7D0
0051B1B0 0051B450 0051B7B0 0051B890 0051AA70 0051DCE0 004E2770 0057D060 007FC820

The two screens the main-menu manager at `00E198AC` owns at `+5Ch` and `+64h`, and the path they
form from the campaign list into a running mission. `docs/MAIN_MENU_SCREENS.md` already records the
class table; this packet recovers the data model behind it, the selection rules, and the exact
request sequence a mission start emits.

## The two classes

| | Mission tree | Briefing |
| --- | --- | --- |
| Manager field | `00E198AC+5Ch` | `00E198AC+64h` |
| Constructor | `005CA880` | `0051E4D0` |
| `operator new` size | 34h | 138h |
| Primary vtable | `00CF170C` | `00CEC948` |
| Secondary vtables | `00CF16F8` at `+08h` | `00CEC934` at `+08h`, `00CEC914` at `+0Ch` |
| Screen id / interface id | 2, `INTF_MISSIONTREE` | 3, `INTF_BRIEFING` |
| GUI layout | none; it is a data table | `FE_briefing_listbox`, `FE_briefing_grid` |

Both constructors are `__fastcall(this)` returning `this` under an SEH frame, `RET`, and both chain
`BSP_FrontEndScreen_Construct` (`004F7180`) before overwriting `+00h`.

### Primary vtable slots

Read back from the ten dwords at each table. Slot roles are the `004F7180` hierarchy roles of
`docs/GAME_FRONTEND_STATES.md`.

| Slot | `00CF170C` (mission tree) | `00CEC948` (briefing) |
| --- | --- | --- |
| +00h id leaf | `005CA8E0` `mov eax,2; ret` | `0051E250` `mov eax,3; ret` |
| +04h | `004F7570` base, `xor al,al; ret` | `004F7570` base |
| +08h | `004F7580` base, `xor al,al; ret` | `004F7580` base |
| +0Ch destroy | `005CAA20` | `0051E5F0` |
| +10h register | `005CAAF0` | `0051E280` |
| +14h bind layout | `004F7590` base | `004F7590` base |
| +18h enter | `005C27F0`, a bare `RET` | `0051CDE0` |
| +1Ch exit | `005C2800`, a bare `RET` | `0051B1B0` |
| +20h update | `005C4040`, `RET 4` | `0051C7D0`, `RET 4` |
| +24h fill list | `004F75D0` base, `RET 4` | `0051E270`, `RET 4` |

`0051E270` is `slot24(void* sink) { 004D6790(ECX = sink, this + 10h); }` — it hands the
`FE_briefing_listbox` page handle to the list the manager walks. The mission tree keeps the base
no-op, consistent with it owning no page.

The briefing's `+08h` base vtable `00CEC934` is `{0051E260, 0051B8C0, 0051B250, 004F8F20, 004F8F30}`;
`0051E260` is the adjustor thunk `sub ecx,8; jmp 0051E5F0`. The `+0Ch` base `00CEC914` is a longer
table whose only override is slot 0, `0051AA70`.

## The mission-tree data model

`005CAAF0`, `__fastcall(this)`, `RET`, an SEH frame and 9C0h of stack. It chains
`BSP_FrontEndScreen_Register` (`004F71D0`) and then reads
`Scripts/datatables/MissionTree.lua` through the keys `multiMissionInfos` and `missionGroups`,
calling `BSP_LoadingScreen_ReportProgress` once per entry of the first loop.

The class holds two iterator-debugging vectors — MSVC `_Container_base12` shape, a `_Myproxy` dword
then first/last/end — which is what fills a 34h object:

| Offset | Field | Evidence |
| --- | --- | --- |
| +00h | primary vtable | `005CA88C` |
| +08h | secondary vtable `00CF16F8` | `005CA894` |
| +0Ch | selected mission index | `005CAE9A`, written from `005C3470[0]` |
| +10h | selected group index | `005CAEA0`, written from `005C3470[1]` |
| +14h..+20h | `vector<MissionGroup>`, stride 34h | `005CAB9x` bound checks, `+0x34` stride |
| +24h..+30h | `vector<MissionRecord>`, stride 434h | second loop, `+0x434` stride |

The constructor zeroes exactly `+18h`, `+1Ch`, `+20h`, `+28h`, `+2Ch`, `+30h`, i.e. both
first/last/end triples, which is the independent check on the two-vector reading.

**Group entry, 34h.** Only one field is recovered: a nested `vector<MissionRecord>` at `+24h`
(proxy) / `+28h` (first) / `+2Ch` (last) / `+30h` (end), stride 434h. `005C3870` and `005C3470`
both index it that way. The first 24h bytes are unrecovered; by analogy with the record they
start with the group's name string.

**Mission record, 434h.** Every offset below is a site in `005C5600` or `0051DCE0`. Strings are the
engine's `{size_t size; char* data;}` pair, not `std::string`.

| Offset | Field | Evidence |
| --- | --- | --- |
| +00h | `name`, the mission key | `005C56C9` copies it to game `+2198h`, `005C5705` to game `+6B8h`; `005C3470` matches on it with `__stricmp` |
| +08h | `title` | `005C5600` passes it to `0057D060`; `0051DCE0` copies it to briefing `+0C4h` |
| +20h | `scene`, the `.scn`/scene name | `005C5600` reads the data pointer at `+24h` and calls `004E2770(scene, 0)`; a null pointer becomes `DAT_00E1952F`, the empty string |
| +60h, +64h, +68h | three dwords copied verbatim into briefing `+0CCh`, `+0D0h`, `+0D4h` | `0051DCE0` |
| +78h..+84h | `vector` of unlock requirements | `005C57D0` passes `record+78h` to `007FC820` with ECX = game `+650h` |
| +0B4h | difficulty; 3 means "use the player's choice" | `005C5769..005C5793` |
| +0B8h | side block 0 | `005C572A` reads its first byte |
| +20Ch | side block 1 | `psVar2[0x83]`, `005C5600` |

**Side block, 154h.** Two of them, at `+0B8h` and `+20Ch`. `0051DCE0` computes
`record + side*154h + 0B8h` and reads from there.

| Block offset | Field | Evidence |
| --- | --- | --- |
| +00h | enabled/kind dword | its low byte is read at `005C572A` and `005C5735` |
| +04h | `briefing_key` string | `0051DCE0` copies it to briefing `+0D8h`; its **size** is the branch at `005C573A` |
| +14h | string vector | `0051DCE0` `BSP_StringVector_Assign` |
| +24h | string vector | `0051DCE0` `BSP_StringVector_Assign` |
| +64h | string vector, the loading-screen text list | `005C5600` passes it to `0057D060` |

The side index is `side = (record[0B8h] == 0) ? 1 : 0` (`XOR EDX,EDX; CMP byte [ESI+0B8h],BL;
SETZ DL`), computed twice in `005C5600` and once more for `00626930`. Block 0 enabled selects block
0; otherwise block 1. The two blocks are most plausibly the two playable sides of the same mission,
but nothing in this packet names them, so that is a hypothesis.

### Selection

`005C3870`, `__fastcall(this)` returning `MissionRecord*`, is the selected-mission accessor and has
two arms on `game+1FE4h`:

- **Zero (the campaign arm).** `group = groups[this+10h]`, `record = group.missions[this+0Ch]`,
  both bound-checked against the vector extents; `LIBCRT_unmatched_00bf6713` is the iterator-debug
  trap on a bad index.
- **Non-zero.** It ignores the indices and linearly scans the `+24h` vector of 434h records,
  comparing each record's `+00h` string against the string at `game+2198h`. `game+2198h` is the
  field `005C5600` writes on launch, so this arm re-finds the current mission by name. `game+1FE4h`
  is the same flag the briefing's back action and its help line test, i.e. the non-campaign session
  flag.

`005C3470`, `__thiscall(this, pair<uint,uint>* out, const string* name)` returning `out`, is the
reverse lookup: it walks every group and every mission, compares `record+00h` to `name` with
`__stricmp`, and writes `{missionIndex, groupIndex}`. The default is `{FFFFFFFFh, 0}`. `005CAAF0`
calls it with a name obtained from `00586150` and then clamps a negative mission index to `(0, 0)`,
so an unknown or absent saved mission falls back to the first mission of the first group.

### Which items are selectable

`005C57D0`, `__fastcall(this)`, `RET`:

```
record = 005C3870(this);
if (007FC820(ECX = 00E188A8 + 650h, record + 78h))
    005C5600(this);
```

`007FC820` is `__thiscall(profile, container*)` returning `char`; it walks the container's
first/last pair and calls `007FC4C0(profile, element)` per element. `game+650h` is the profile block
`docs/GAME_AWARD_TRACKERS.md` and `docs/MAIN_MENU_SCREENS.md` refer to. So a mission is activatable
exactly when the profile satisfies the record's `+78h` requirement list; a locked mission simply
does nothing on accept. `007FC4C0` itself is a 176-line routine with a 256-byte character buffer and
was not read: how one requirement is evaluated is **not** recovered here.

Completion state is not touched by anything in this packet. The mission-list icons are drawn by the
main-menu screen (`00588C70` walks `00E198AC+5Ch`, per `docs/MAIN_MENU_SCREEN_UPDATE.md`), so
the completion marks live on that side of the boundary.

### The mission-tree update

`005C4040`, `__thiscall(this, float)`, `RET 4`, 34h bytes, no Ghidra function (it falls inside the
stored body of `FUN_005C3DE0`):

```
if (004D92B0(ECX = 00E188A8, 4Bh))            // back, edge-or-repeat
    004CC460(ECX = [00E198AC], 1, 0);         // request INTF_MAINMENU
else
    00427190(ECX = 004C1E90(...), 3);         // backdrop scene 3
```

`00427190` is the `SCRIPTS/datatables/MPakScenes.lua` backdrop selector the main-menu update calls
in its own epilogue, so the mission-tree interface keeps backdrop scene index 3 alive while it is
current. Enter and exit are bare `RET`s: this screen has no page to raise.

## The briefing screen

`0051E4D0` zeroes twenty-eight scattered dwords up to `+11Ch`, then allocates a node through
`0051ACB0`, stores it at `+130h`, sets its byte `+19h` to 1 and makes its `+00h`, `+04h` and `+08h`
point back at itself — a circular list head — and clears `+134h`.

`0051E280` (register), `__fastcall(this)`, `RET`, chains `004F71D0` and binds:

| Field | Bound object |
| --- | --- |
| +10h | GUI page `FE_briefing_listbox` |
| +14h | GUI page `FE_briefing_grid`, then its virtual `+34h(0)` hides it |
| +18h | widget `Main_Listbox` |
| +1Ch | widget `MainListbox_Text` |

Other fields established by the virtuals:

| Field | Meaning | Evidence |
| --- | --- | --- |
| +28h | the current briefing key, taken from `+0D8h` | `0051CDE0` via `0051CC90` |
| +54h | cleared on enter | `0051CDE0` |
| +60h | the list-box object `00A9C7C0` selects into | `0051AA70` |
| +6Ch, +98h, +124h | widgets the play action shows/hides | `0051B7B0` |
| +9Ch, +0A0h, +0A4h | the three difficulty widgets | `0051B7B0` |
| +0ACh..+0B0h | vector of item widget handles | `0051AA70` |
| +0C4h/+0C8h | mission title string | `0051DCE0` |
| +0CCh, +0D0h, +0D4h | record `+60h`, `+64h`, `+68h` | `0051DCE0` |
| +0D8h/+0DCh | briefing key string | `0051DCE0` |
| +0FCh, +100h | the history prev / next widgets | `0051AA70` |
| +104h | non-zero when a history page exists | `0051B450` |
| +114h | the history page index, stepped by ±1 | `0051AA70` |
| +130h | circular list head allocated in the constructor | `0051E4D0` |

`0051DCE0`, `__thiscall(this, MissionRecord*, int side)`, `RET 8`, is how a mission reaches the
screen: title, the three dwords, the side's briefing key and the two string vectors at block `+14h`
and `+24h`. It is called from exactly one site, `005C5754`.

`0051CDE0` (enter) is 589 decompiled lines. It calls `00518DA0`, shows the `FE_briefing_grid` page,
clears `+54h`, pushes the string `"BRIEFING"` through `005189B0`, and reads `+0D8h`. When
`DAT_00E18D91` is clear it hides the multiplayer mode groups `Duel_Group`, `Escort_Group`,
`Siege_Group`, `Competitive_Group`, `Island_Capture_Group` and `BG_01_Group`; it builds the
objective groups `_pri_objective_Group`, `_sec_objective_Group`, `1_pri_objective_Group`,
`1_sec_objective_Group`, `_pri_Group`, `_sec_Group`, `1_pri_Group`, `1_sec_Group` with the labels
`PRIMARY OBJECTIVE` and `SECONDARY OBJECTIVE`, and reads the mode through
`BSP_Game_GetEffectiveGameMode` (`004BCA50`). It is **analyzed only**: the objective-group loop and
the index arithmetic into the two string vectors were not read line by line.

`0051B450`, `__thiscall(this, bool suppress_all, bool suppress_history)`, `RET 8`, builds the help
line through `0054B530` from four `(control code, locale key)` pairs:

| Code | Key | Condition |
| --- | --- | --- |
| 0A2h | `FE.briefing_play` | only when `suppress_all` is false **and** `game+1FE4h` is zero |
| 0A7h | `FE.briefing_history` | only when `suppress_history` is false and `this+104h` is non-zero |
| 0AFh | `FE_xbox.bhelp_navigate` | always |
| 0A3h | `globals.back` | always |

0A2h/0A3h are the same accept/back control codes the main-menu GUI handler uses on its pages, which
is the independent check on the mapping in `docs/MAIN_MENU_SCREEN_UPDATE.md`.

`0051B7B0`, `__thiscall(this)`, `RET 4`, is the play action. It reads the active widget through
`00A9CA00`, matches it against the three difficulty widgets `+9Ch`/`+0A0h`/`+0A4h` and writes the
matching index 0, 1 or 2 into **both** `game+6ACh` and `game+6B0h`; then it clears `+90h`, calls
virtual `+34h` and `+60h(0)` on `+98h`, `+60h(1)` on `+6Ch` and `+124h`, clears the byte at
`[+124h]+77h`, rebuilds the help line with `0051B450(0, 0)` and pumps the input manager. Its other
arm (`0051B866`) calls `00439020`, the mission-start request pair. Ghidra's stored body stops at
`0051B866`, so the branch that chooses between the two arms is only visible in the listing.

`0051B890` is the back action, listing only, `__thiscall(this)`, `RET`:

```
if (game+1FE4h) tail-call 006881F0 on [00E198B4];
else 004CC460(ECX = [00E198AC], 2, 0);      // request INTF_MISSIONTREE
```

`0051C7D0` (update), `__thiscall(this, float)`, `RET 4`, listing only, is the same thing behind the
`4Bh` edge-or-repeat test:

```
if (004D92B0(ECX = 00E188A8, 4Bh)) {
    if (game+1FE4h) 006881F0(ECX = [00E198B4]);
    else 004CC460(ECX = [00E198AC], 2, 0);
}
```

`0051AA70`, `__thiscall(this, widget)`, `RET 4`, is the widget-activated handler of the `00CEC914`
base: the widget `+0FCh` decrements `+114h`, `+100h` increments it, and otherwise it scans the
handle vector at `+0ACh..+0B0h` and calls `00A9C7C0(ECX = this+60h, index)` on the match.

`0051B1B0` (exit) walks the containers at `+2Ch..+34h` and was read only as far as its prologue.

## Starting a mission

`005C5600`, `__cdecl(void)` in Ghidra's rendering but in fact `__thiscall(this = the mission-tree
screen)`, `RET`, SEH frame. In native order:

1. `record = 005C3870(this)`.
2. `side = (record[0B8h] == 0)`.
3. Copy `record+00h` into the string at `game+2198h`.
4. `scene = record[24h]`, or `DAT_00E1952F` when null; `004E2770(ECX = game, scene, 0)`.
   `004E2770` clears `game+5FCh`, empties the string at `game+600h/+604h`, then calls
   `004E1D70(scene, 0, FFFFFFFFh, flags)` and `004C6890(0)` — the pending-scene setter.
5. Build the loading-screen text from the side block's `+64h` string vector, `record+08h` and the
   low byte of `record+20Ch`, and publish it with `0057D060`, which reaches
   `BSP_LoadingScreen_PublishConfig` (`0057CFF0`).
6. Copy `record+00h` into the string at `game+6B8h`.
7. `00626930(ECX = record, EDX = side)`.
8. **Branch on the selected side block's briefing key size** (`CMP dword [EBP+4], EBX` at
   `005C573A`, where EBP is the block base):
   - **Empty key — launch now.** `game+6ACh = (record+0B4h == 3) ? game+6B0h : record+0B4h`, then
     `00439020()`, which is exactly `BSP_Game_RequestState(6); BSP_Game_RequestState(0Ah)`.
   - **Non-empty key — show the briefing.** `0051DCE0(ECX = [00E198AC]+64h, record, side)` fills the
     briefing screen, then `004CC460(ECX = [00E198AC], 1, 0)`.
9. Reset the shared front-end timer (`004BEC00` then `00A92C40` with 0.0f) and run `004D2A80`.

`00439020` is `__cdecl(void)`, `RET`, three callers: `0051B7B0` (the briefing's play action),
`0058BDF0` and `005C5600`. Against the drain table of `docs/GAME_FRAME_CONTROL.md`, request 6 runs
the manager virtual `+0Ch` (`BSP_MainMenu_ApplyPendingInterface`, which publishes the screen set and
the input context) and request 0Ah runs `004DFB70`, one of only two callers of
`BSP_LoadingScreen_Begin` (`0057CB60`). The drain is FIFO and re-reads the count after each handler,
so both are serviced in the same pass: the interface change lands first, then the scene load with
its loading screen. Request 0Dh, the in-mission state of `docs/GAME_SIMULATION_GATE.md`, is not
enqueued here; it is reached later, out of `004DFB70`'s own path.

So the **fields the mission start writes before anything else reads them** are: `game+2198h` (the
mission key, which `005C3870`'s non-campaign arm reads back), `game+600h/+604h` and `game+5FCh`
(cleared by `004E2770`), `game+6ACh` (the effective difficulty), `game+6B8h` (the mission key
again), and the loading-screen configuration published in step 5.

## Uncertainties

- **Nothing in the image requests interface 3.** A byte scan of `.text` for `6A 00 6A 03` finds no
  `004CC460` call site, and the only literal that reaches the manager's pending field on this path
  is the `1` at `005C5760`. `00685820` maps interface ids to screen ids by the identity and
  publishes a one-element screen set, so on the evidence here selecting a mission with a briefing
  key leaves `INTF_MAINMENU` current while the briefing screen object holds the mission data. Either
  the briefing surface is driven by the main-menu screen (whose slot `+14h` override `005861B0`
  names `FE_briefing` and `FE_briefing_grid`), or some site supplies id 3 from a register. This is
  the single largest open question in the packet and it is why no launch path is claimed to raise
  `INTF_BRIEFING`.
- **`multiMissionInfos` versus `missionGroups`.** `005CAAF0` references both keys, and `005C3870`
  proves that `+14h` is the 34h group vector with nested missions and `+24h` the flat 434h record
  vector searched by name. Which Lua key fills which vector is inferred from that role split, not
  read off the call, because the decompiled reader loses its register arguments.
- **The two side blocks** are assumed to be two variants of one mission. Only the enabled byte, the
  briefing key and the three string vectors are recovered; the other 12Ch bytes of each block are not.
- **`007FC4C0`**, the per-requirement unlock predicate, is unread, so "which items are selectable"
  is recovered at the list level, not at the predicate level.
- `0051B7B0`'s selector between the difficulty arm and the `00439020` arm is outside Ghidra's stored
  body and was read only in the raw listing.

## Routines with no Ghidra function

These are named from the listing only; Ghidra has no function at the address, so the orchestrator
must define one before applying a name. End addresses are inclusive.

| Address | End | Role |
| --- | --- | --- |
| 005C27F0 | 005C27F0 | mission-tree enter, a bare `RET` |
| 005C2800 | 005C2800 | mission-tree exit, a bare `RET` |
| 005C4040 | 005C4073 | mission-tree update; falls inside `FUN_005C3DE0`'s stored body |
| 0051C7D0 | 0051C818 | briefing update; falls inside `FUN_0051C7A0`'s stored body |
| 0051B890 | 0051B8B8 | briefing back action |

## State reached

| Address | State |
| --- | --- |
| 005CA880, 0051E4D0 | analyzed (class, vtables, field zeroing) |
| 005CAAF0 | analyzed (the two vectors, the stride, the selection restore) |
| 005C3470, 005C3870, 005C57D0, 005C5600 | analyzed, reconstructed, build-tested |
| 00439020 | analyzed, reconstructed, build-tested |
| 005C4040, 0051C7D0, 0051B890 | analyzed, reconstructed, build-tested |
| 0051E280, 0051DCE0, 0051B450, 0051AA70 | analyzed, reconstructed (data and rules), build-tested |
| 0051CDE0 | analyzed (page, mode groups, objective group names) |
| 0051B7B0 | analyzed (difficulty arm and the start arm) |
| 0051B1B0, 005CAA20, 0051E5F0 | exported/listing only |
| 004E2770, 0057D060, 007FC820 | analyzed as boundary calls |
| 007FC4C0, 00626930, 00518DA0, 0051CC90 | not read |

## Follow-up packets proposed

- `briefing_enter_objectives` — addresses 0051CDE0 0051CC90 00518DA0 005189B0 004BCA50; files
  `docs/BRIEFING_OBJECTIVES.md`, `include/bsp/briefing_objectives.hpp`,
  `src/briefing_objectives.cpp`. Contract: the 589-line enter virtual — how the two string vectors
  at side-block `+14h`/`+24h` become the primary and secondary objective rows, which mode group the
  effective game mode selects, and what `DAT_00E18D91` gates.
- `profile_unlock_predicate` — addresses 007FC4C0 007FC820 and the `game+650h` profile block; files
  `docs/PROFILE_UNLOCKS.md`, `include/bsp/profile_unlocks.hpp`, `src/profile_unlocks.cpp`.
  Contract: how one unlock requirement string is parsed and evaluated against the profile, and
  therefore what makes a mission, a unit or an award available.
- `interface_three_raiser` — addresses 00685820 004CC460 005861B0 00594BF0 and the main-menu
  mission-detail page; files `docs/BRIEFING_RAISE_PATH.md`. Contract: settle how `INTF_BRIEFING`
  becomes the current interface, or prove that the briefing surface is drawn by the main-menu
  screen and that the briefing screen object is a data holder only.
- `mission_tree_lua_reader` — addresses 005CAAF0 005CA300 005C9E30 005C9F70 005C6A70 005C9B60;
  files `docs/MISSION_TREE_TABLE.md`. Contract: the per-entry readers, so the remaining 24h bytes
  of a group and the remaining 12Ch bytes of a side block get names from the Lua keys.

## Corrections from docs/MISSION_TREE_LUA_READER.md

`missionGroups` fills the 34h group vector at screen+14h and `multiMissionInfos` the flat 434h record vector at +24h (settled by destination address). Record +00h is the Lua `id` (the string the selection lookup matches and the launch path saves), +08h the `name`, and the +60h triple is `date` (year, month, day); the difficulty default of 3 is the reader's own default. Side block 0 is `allied`, block 1 `japanese`, and the block's first dword is written 1 before any key is read, so the side selector at `005c572a` is a presence test. The installed table has 5 groups, 143 campaign missions and 34 multiplayer entries; its 188 `prerequisites` lists are commented out (the installed copy is modded), and the commented values are mission ids. The mission-tree interpreter opens only the `table` library (mask 4).
