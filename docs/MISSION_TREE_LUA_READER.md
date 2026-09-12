# The mission-tree table readers (packet `mission_tree_lua_reader`)

Addresses: 005CAAF0 005C9F70 005C6A70 005C5DA0 005C5860 005C4AD0 005CAA40 005C9E30 005CA300
005C9B60 005C9830 005C9370 005C92F0 005C6850 005C49C0

`docs/MISSION_TREE_BRIEFING_SCREENS.md` recovered the shape of the mission tree's records — a
34h group entry, a 434h mission record, two 154h side blocks — and left most of their bytes
unnamed. This packet reads the routines that fill them and names those bytes from the Lua keys.
Every offset below is a write site in one of the readers, and every key is a literal at the
address the listing pushes.

The reader the mission tree drives is the same `00CE44FC` visitor
`docs/GUI_LUA_READER.md` recovers: same six virtuals, same `(tag, word)` argument pairs, same
field-type switch. Nothing here reimplements that; the value semantics of every read below are
that document's `00BD63B0` / `00BD61C0`.

## The load, `005CAAF0`

`__fastcall(this)`, `RET`, an SEH frame and 994h of stack. After
`BSP_FrontEndScreen_Register` (`004F71D0`) it is the whole of the mission tree's data model:

| Step | Site | What runs |
| --- | --- | --- |
| 1 | `005CAB21` | `00B66BD0` constructs the interpreter owner on the frame |
| 2 | `005CAB38` | `00B6A020(4)` creates the state with library mask **4** |
| 3 | `005CAB4D`..`005CAB68` | a NativeString is resized to 22h and the 34-byte literal at `00CF176C` copied in |
| 4 | `005CAB85` | `00B69D40(path, 0)` runs the file and every VFS override of it |
| 5 | `005CABBF` | `00B67980` makes the globals LuaObject |
| 6 | `005CABDB` | `00B67800(globals, "MissionTree")`, the literal at `00CF1760` |
| 7 | `005CAC11` | `004425C0` constructs the reader over that object |
| 8 | `005CAC6F` | the `missionGroups` walk |
| 9 | `005CAD95` | the `multiMissionInfos` walk |
| 10 | `005CAE84`..`005CAF0C` | `00586150` supplies an id, `005C3470` resolves it, a negative index clamps to (0, 0) |
| 11 | `005CAF1C`.. | the reader and the LuaObjects are destroyed and the state closed |

Two facts are worth stating plainly because they differ from the GUI's use of the same machinery.

**The library mask is 4, not 65h.** `005CAB28` pushes 4 into `00B6A020`, so this interpreter opens
the `table` library alone. The GUI's page interpreter opens base, `table`, `string` and `math`
(`docs/GUI_LUA_READER.md`). Anything the shipped `missiontree.lua` needs from the base library —
`DoFile`, `pairs`, `type` — must therefore come from `00B6A020`'s own `luaL_loadstring` +
`lua_pcall` bootstrap, which that document records as followed no further. **Unresolved**: which
functions that bootstrap installs. It is the only reason the shipped file's `DoFile` at line 1 can
work at all.

**The script path is `Scripts/datatables/MissionTree.lua`** (`00CF176C`, 34 characters, resized to
22h before the copy) and the global it reads is `MissionTree` (`00CF1760`). The installed file is
`scripts/datatables/missiontree.lua`; the VFS is case-insensitive on this path.

### The two top-level keys, settled

`005CAC68` enters `missionGroups` (`00CF1750`) and `005CAD8E` enters `multiMissionInfos`
(`00CF1734`). The first loop's `push_back` is `005CAA40`, whose stride is 34h and whose
destination is `ESI = this + 14h` (`005CACC8`); the second's is `005C9E30`, stride 434h,
destination `this + 24h` (`005CADCF`).

So: **`missionGroups` fills the 34h group vector at `+14h` and `multiMissionInfos` fills the flat
434h record vector at `+24h`.** `docs/MISSION_TREE_BRIEFING_SCREENS.md` inferred exactly this from
the role split and marked it as inference; it is now read off the two `LEA`s.

Both loops have the same shape, and it is not the obvious one:

```
Enter(key);
for (i = 1; HasKey(kind=Index, i); ++i) {
    push_back(vector, &default_constructed_temp);   // 005CAA40 / 005C9E30
    ~temp();                                        // 005C9B60 / 005C8AE0
    element = vector._Mylast - stride;              // three iterator-debug checks
    Enter(kind=Index, i);
    ReadEntry(element, reader);                     // 005C9F70 / 005C6A70
    Leave();
    ...
}
Leave();
```

The element is appended empty and filled **in place**, so a record the script leaves half-written
keeps the default-constructed remainder rather than a copy of a temporary. The group temporary at
`ESP+38h` is zeroed at exactly `+00h`, `+04h`, `+08h`, `+0Ch`, `+10h`, `+14h`, `+28h`, `+2Ch`,
`+30h` (`005CACA0`..`005CACC0`) — the three string pairs and the nested vector's three pointers,
and neither the `gratDate` triple nor the `_Myproxy` word. That is an independent confirmation of
the group layout below.

`BSP_LoadingScreen_ReportProgress` (`0057BEC0`) runs once per **group**, never per mission and
never in the second loop. `005CAD49`..`005CAD65` computes it on the x87 stack:

```
progress = (float)(i * 0.02 + 0.45)      // i is the 1-based group index
```

with `0.02` at `00D7A2F8` and `0.45` at `00CF1748`, both doubles. Five shipped groups therefore
take the loading bar from 0.47 to 0.55.

## The 34h group entry, `005C9F70`

`__thiscall(MissionGroup *this, LuaReader *reader)`, `RET 4`, 168 instructions, no gaps. It is
called with the reader positioned on one element of `missionGroups`.

| Offset | Lua key | Read | Type | Notes |
| --- | --- | --- | --- | --- |
| +00h | `groupName` | `+10h` | String | |
| +08h | `helpLine` | `+10h` | String | note the capital L; the mission record's key is `helpline` |
| +10h | `gratMsg` | `+10h` | String | |
| +18h, +1Ch, +20h | `gratDate` | `+10h` | Vec3 | three floats into a frame temporary, then `CVTTSS2SI` into three ints |
| +24h..+30h | `missions` | enter | — | the nested 434h vector, cleared by `005C92F0` first |

`gratDate` is `{ 1940, 09, 08 }` in the shipped file: a **year, month, day** triple, stored as
integers. This is the same float-array-then-truncate idiom the mission record uses for `date`.

The nested walk is the loop above with `005C6A70` as its entry reader.

## The 434h mission record, `005C6A70`

`__thiscall(MissionRecord *this, LuaReader *reader)`, `RET 4`, 1144 instructions, no gaps. It is
the largest routine in the packet and the one that names most of the record. Reads marked
*default* use the reader's `+0Ch` slot and take the listed fallback when the key is nil; reads
marked *plain* use `+10h`, where a nil reaches the type switch and a String nil resizes the
destination to zero.

| Offset | Lua key | Read | Type | Default |
| --- | --- | --- | --- | --- |
| +000h | `id` (`00CF16BC`) | plain | String | — |
| +008h | `name` (`00CE49B8`) | plain | String | — |
| +010h | `contentID` (`00CF16B0`) | default | String | `""` (`00CE3A0C`) |
| +018h | `helpline` (`00CF16A4`) | default | String | `""` |
| +020h | `sceneFile` | plain | String | — |
| +028h | — | — | — | **never written**: an 8-byte hole |
| +030h | `background` | default | String | `""` |
| +038h | `debriefingText` | default | String | `""` |
| +040h | `backgroundMovie` | default | String | `""` |
| +048h | `backgroundVoice` | default | String | `""` |
| +050h | `debriefingVoice` | default | String | `""` |
| +058h | `MovieName` (`00CF1698`) | default | String | `""` |
| +060h, +064h, +068h | `date` (`00CF1690`) | plain | Vec3 | — |
| +06Ch..+074h | `Pos` (`00CF168C`) | guarded | Vec3 | three `XORPS` zeroes when absent |
| +078h..+084h | `prerequisites` (`00CF167C`) | `005C5860` | string vector | untouched when absent |
| +088h..+094h | `navalacademy` (`00CF166C`) | `005C4AD0` | int vector | untouched when absent |
| +098h | `description` (`00CEBAA4`) | plain | String | — |
| +0A0h | `picture` (`00CE4994`) | plain → `00AA2660` | texture | see below |
| +0A4h..+0B3h | — | — | — | the atlas rectangle `00AA2660` writes |
| +0B4h | `forcedDifficultyLevel` (`00CF1654`) | default | Int | **3** |
| +0B8h..+20Bh | `allied` (`00CE5618`) | `005C5DA0` | side block 0 | |
| +20Ch..+35Fh | `japanese` (`00CE560C`) | `005C5DA0` | side block 1 | |
| +360h | `sideMission` (`00CF1648`) | guarded | Bool | byte cleared when absent |
| +364h..+423h | `MultiPlayMapSizes` (`00CE8B10`) | nested | 16 × Vec3 | see below |
| +424h | `UniqueMultiSettings` presence | `+14h` | Bool | the `HasKey` result itself is stored |
| +428h | — | — | — | **never written** |
| +42Ch, +430h | `UniqueMultiSettings` | nested | ordered map | head pointer and size |

Three entries in the previous packet's table are now named rather than described:

- `+00h`, "the mission key `005C3470` matches on", is the Lua **`id`** — `"CHG01"`, `"IJN17"`,
  `"am1"`. `+08h`, called `title`, is the Lua **`name`**, the display string. The saved-mission
  round trip at `game+2198h` therefore carries an id, not a title.
- `+60h`/`+64h`/`+68h`, "three dwords copied verbatim into briefing `+0CCh`..`+0D4h`", are the
  **`date`** triple: year, month, day.
- `+0B4h`, "difficulty; 3 means use the player's choice", takes that 3 from the reader's own
  default pair at `005C7641`, so a mission without `forcedDifficultyLevel` is the ordinary case.

**`picture`.** `005C6D8E` reads the string into a frame temporary. `005C75A2` tests its **size**:
a non-empty string reaches `00AA2660(gui, &name, this + 0A4h, &out, 1.0f)` through
`BSP_GuiManager_GetOrCreate` (`004C12B0`) and the returned texture replaces `+0A0h` after an
`InterlockedDecrement` release of the old one; an empty string releases and leaves `+0A0h` null.
So `+0A0h` is a refcounted texture and `+0A4h`..`+0B3h` its atlas rectangle.

**`MultiPlayMapSizes`.** Sixteen `Vec3` corners at a 0Ch stride from `+364h`, two per mode, keyed
`<Mode>_nw` and `<Mode>_se`. The record-offset order is IslandCapture1v1, IslandCapture2v2,
IslandCapture3v3, IslandCapture4v4, Siege, Competitive, Duel, Escort; the reader's **call** order
is Competitive, Duel, Escort, the four Island Capture sizes, Siege. The two arms of `005C6DBE`
disagree, and that is the only interesting default in the packet:

- key absent: `005C6DC6`'s else arm writes `{-15000, 0, +15000}` into every `_nw` and
  `{+15000, 0, -15000}` into every `_se`, from `DAT_00CF1458` and `DAT_00CF1464`.
- key present: the table is entered once and each corner is a `+0Ch` read whose default is the
  **origin**, so a mode the table omits is `{0, 0, 0}`.

**`UniqueMultiSettings`.** `005C78E5` first clears the container at `+42Ch` — a red-black head
whose left, parent and right are set to itself with `+430h` zeroed, i.e. an `std::map`. The
presence byte at `+424h` is the `HasKey` result. When present, `005C7944`..`005C7ADF` is a double
key enumeration through the reader's `+18h` slot: the outer keys are multiplayer modes, the inner
keys parameter names, and the only value read inside a leaf is `MenuDIS` (`00CF15E0`) as a Bool.
`005C7A7D` is a `SETZ` on the byte the read produced, so **the stored value is the negation of
`MenuDIS`**. The insert pair is `005C6850` (find-or-create by the two key strings) and `005C49C0`
(the mapped byte); neither was read, so which of the two strings is the map's first key component
is **provisional**. Both enumerations use the 250-entry key array of `docs/GUI_LUA_READER.md`,
cleared to FFFFFFFFh first, and neither is bounds checked.

## The 154h side block, `005C5DA0`

`__thiscall(MissionSideBlock *this, LuaReader *reader)`, `RET 4`, 485 instructions, no gaps.
Decompiles with its arguments intact, so the table is read straight off the pseudocode.

`005C5DA5` sets `*this = 1` **before any key is read**. The reader is only called when the side
key exists, so "side block 0's first dword is non-zero" and "the mission has an `allied` table"
are the same fact — which is what makes `side = (record[0B8h] == 0) ? 1 : 0` at `005C572A` a
side-presence test rather than a flag the script sets.

| Offset | Lua key | Kind |
| --- | --- | --- |
| +000h | — | the presence dword, written as 1 |
| +004h | `briefingGuiLayer` | String, plain read |
| +00Ch | — | **never written**: an 8-byte hole |
| +014h | `primaryObjectives` | string vector |
| +024h | `secondaryObjectives` | string vector |
| +034h | `hiddenObjectives` | string vector |
| +044h | `hiddenHints` | string vector |
| +054h | `loadingBackgrounds` | string vector |
| +064h | `hints` | string vector |
| +074h | `allunitsid` | string vector |
| +084h | `allunitsnum` | string vector |
| +094h | `changeables` | string vector |
| +0A4h | `allunitslockid` | string vector |
| +0B4h | `allunitslocknum` | string vector |
| +0C4h | `allunitslockhint` | string vector |
| +0D4h | `multiunitsidDuel` | string vector |
| +0E4h | `multiunitsidSiege` | string vector |
| +0F4h | `multiunitsidEscort` | string vector |
| +104h | `multiunitsidCompetitive` | string vector |
| +114h | `multiunitsidIC1v1` | string vector |
| +124h | `multiunitsidIC2v2` | string vector |
| +134h | `multiunitsidIC3v3` | string vector |
| +144h | `multiunitsidIC4v4` | string vector |

Twenty string vectors of 10h bytes each from `+14h` fill the block to 154h exactly. The
`multiunitsid<Mode>` order is **not** the `MultiPlayMapSizes` order; the reconstruction keeps two
separate mode tables for that reason.

Three of these were previously described by role. `+14h` and `+24h`, the two vectors `0051DCE0`
assigns into the briefing, are `primaryObjectives` and `secondaryObjectives`. `+64h`, "the
loading-screen text list" `005C5600` hands to `0057D060`, is `hints` — the shipped values are
`trn.hint_loadinghint1` and friends, confirming that reading.

Twelve of the reads test the same key **twice** in a row before calling `005C5860`
(`005C5E9C`'s shape, and `005C6D03` in the record). The second test cannot fail once the first
passed, so the behaviour is one guard; it is compiled-in redundancy, not a second condition.

## The two array readers

`005C5860`, `__thiscall(LuaReader *reader, GuiLuaKeyKind kind, const char *key,
std::vector<NativeString> *out)`, `RET 0Ch`. It empties `out` (`004954F0`), enters the key, then
reads integer keys 1, 2, 3, ... with the reader's `+10h` slot as String into a frame temporary,
`push_back`s each through `00450540` and returns the temporary's buffer to the sized storage pool,
stopping at the first `+14h` that reports absent. Then it leaves. Both call sites guard it with
`+14h` on the same key, so an absent key never clears the destination.

`005C4AD0` is the same routine for `std::vector<int>`, `RET 0Ch`, reading with field type Int
instead of String and pushing through `00441F30` on a reallocation. `navalacademy` is its only
caller in this packet.

## The container helpers

| Address | Role |
| --- | --- |
| `005CAA40` | `vector<MissionGroup>::push_back`, stride 34h; the in-place arm advances `_Mylast` by 34h, the growing arm is `005CA970` |
| `005C9E30` | the same for `vector<MissionRecord>`, stride 434h; growing arm `005C9C10` |
| `005CA300` | copy-construct a range of 34h group entries under an SEH frame, element constructor `005C9830` |
| `005C9B60` | `~MissionGroup`: `005C9370` for the nested vector, then the three string buffers at `+00h`/`+08h`/`+10h` back to the sized storage pool in reverse order |
| `005C92F0` | clears the group's nested mission vector before the `missions` walk |
| `005C6850`, `005C49C0` | the `UniqueMultiSettings` map find-or-insert pair; **not read** |
| `005C9830`, `005C9370` | the group copy constructor and its nested-vector destructor; **not read** |

The three storage-pool releases in `005C9B60` are the independent check that the group's first 18h
bytes are exactly three engine strings.

## When the readers run

`005CAAF0` has exactly one reference in the whole image: the dword at `00CF171C`, which is slot
`+10h` of the mission-tree vtable `00CF170C`. It is therefore never called directly. Per
`docs/MAIN_MENU_SCREENS.md` and `docs/FRONTEND_MANAGERS.md` the screen is one of the seven
`00686380` (`GVMainMenu::Init`) constructs at `00E198AC+5Ch`, and the register slot is the
front-end hierarchy's publish step. So the whole table is parsed **once, when the main-menu
manager initialises**, not on shell entry and not on mission-tree enter — which is consistent with
the loading-screen progress reports and with the mission tree's own enter and exit virtuals being
bare `RET`s. **Not read here**: the exact instruction in `00686380` that calls slot `+10h`, so
"once per `GVMainMenu::Init`" is carried over from those two documents rather than proved here.

## The installed-file check

`local/parse_missiontree.py` (scratch, not committed) is an independent Lua-subset parser —
comments, string/number/boolean/nil literals, table constructors, `..` concatenation, call
expressions — run read-only against
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/scripts/datatables/missiontree.lua`.
It parses the two top-level tables and counts keys without any reference to the disassembly.

| | `missionGroups` | `multiMissionInfos` |
| --- | --- | --- |
| entries | 5 groups | 34 |
| missions | 143 | — |
| side blocks | 143 (68 `allied`, 75 `japanese`) | 68 (34 + 34) |
| distinct mission keys | 20 | 13 |
| distinct side-block keys | 13 | 13 |
| entries with a non-empty `prerequisites` | 0 | 0 |

Every key the parser found is a key one of the three readers consumes, with two exceptions:

- **`demotips`** appears in a side block and no reader reads it. It is dead data.
- **`contentID`** is read by `005C6A70` and appears **nowhere** in the shipped file, so every
  record's `+10h` is the `""` default.

`MultiPlayMapSizes` and `UniqueMultiSettings` appear on all 34 `multiMissionInfos` entries and on
no campaign mission, so the campaign half of the table always takes the ±15000 default box.
`sideMission` appears on 20 of the 143 campaign missions. Objective rows: 301 primary, 124
secondary, 141 hidden.

**The installed copy is modded.** Every `prerequisites` list with content has been commented out —
`--["prerequisites"] = { "CHG01" },` and 187 more like it — so the installed table locks nothing.
The commented values are mission **`id`** strings, which settles what a mission's unlock
requirement is: `docs/PROFILE_UNLOCK_PREDICATE.md` records `007FC820` walking `record+78h` and
`007FC4C0` testing one plain string against the profile, and the strings the shipped campaign puts
there are the ids of earlier missions. The count of entries with unlock requirements is therefore
0 **as installed** and cannot be read off this copy for the stock game.

## Reconstruction

`include/bsp/mission_tree_data.hpp` and `src/mission_tree_data.cpp`.

The schema is data: `kMissionGroupSchema`, `kMissionRecordSchema` and `kMissionSideBlockSchema`
are arrays of `{key, GuiLuaFieldType, offset, slot, defaults}`, so the tables above are checkable
from the code. `GuiLuaFieldType`, `GuiLuaKeyKind` and `GuiLuaVariant` come from
`bsp/gui_lua_reader.hpp` unchanged.

The records aggregate rather than restate `bsp/mission_tree_screens.hpp`: `MissionSideBlockData`
holds a `MissionSideBlock` plus a `MissionSideBlockExtra` for the 12Ch that struct does not name,
and `MissionRecordData` holds a `MissionRecord` plus a `MissionRecordExtra`. No field has two
homes. `kMissionDifficultyFromPlayer` is reused as the `forcedDifficultyLevel` default rather than
a second constant with the same value.

`read_mission_side_block_005c5da0`, `read_mission_record_005c6a70` and
`read_mission_group_005c9f70` are pure functions over `MissionTreeLuaView`, an abstract view with
one method per reader operation the mission tree uses, stated in values rather than destination
addresses. `load_mission_tree_005caaf0` is the sequence over `MissionTreeScriptHost`, one method
per native call site outside the reader, in the style of `bsp::run_application_frame`.

One test case was added to `tests/math_tests.cpp`: the `MultiPlayMapSizes` asymmetry, because a
single fallback is the natural wrong assumption.

## Uncertainties

- The `00B6A020` bootstrap. With mask 4 the base library is not opened, yet the shipped file calls
  `DoFile`, `pairs` and `type` at load. Either the bootstrap registers them or the shipped file's
  first lines fail; nothing here decides which, and a failure would be fatal because
  `00B66CA0` uses `lua_call`, not `lua_pcall`.
- `005C6850` and `005C49C0` were not read, so the `UniqueMultiSettings` map's key order (mode
  first or parameter first) is provisional, as is the reading of `+42Ch` as an `std::map` rather
  than an `std::set` of pairs.
- `+028h` of the record, `+428h` of the record and `+00Ch` of the side block are written by no
  reader. They may be padding or fields another subsystem owns.
- The `SETZ` at `005C7A7D` makes the stored `MenuDIS` value the negation of the script's. Why the
  sense flips is not recovered; the name suggests "disabled", which would make the stored byte
  "enabled".
- `00586150`, the source of the id `005CAAF0` looks up, was not read.
- The exact call site of vtable slot `+10h` inside `00686380`.
- The `demotips` key is unread by anything in this packet; whether some other subsystem reads the
  same table was not checked.

## Follow-up packets

- `mission_unique_multi_settings` — addresses 005C6850 005C49C0 005C4040 and the container at
  record `+42Ch`; files `docs/MISSION_UNIQUE_MULTI_SETTINGS.md`. Contract: the map's key type and
  ordering, who reads it back, and what the inverted `MenuDIS` byte gates in the lobby.
- `mission_picture_atlas` — addresses 00AA2660 004C12B0 and record `+0A0h`..`+0B3h`; files
  `docs/GUI_TEXTURE_ATLAS_RESOLVE.md`. Contract: the texture/atlas resolve the record inlines, its
  refcount protocol and the meaning of the 1.0f argument.
- `lua_state_owner_bootstrap` — addresses 00B6A020 (tail) 00B66BD0 00B669A0 and the
  `luaL_loadstring` + `lua_pcall` bootstrap; files `docs/MISSION_LUA_HOST.md` (extend). Contract:
  which C functions and Lua helpers a state gets beyond its library mask, which is what makes
  `DoFile` work under mask 4.
- `mission_tree_saved_selection` — addresses 00586150 005C3470 and `game+2198h`; files
  `docs/MISSION_TREE_SELECTION.md`. Contract: where the requested mission id comes from and how it
  survives a session.

## State reached

| State | Routines |
| --- | --- |
| analyzed, reconstructed, build-tested, installed-file-checked | `005CAAF0`, `005C9F70`, `005C6A70`, `005C5DA0` |
| analyzed, reconstructed, build-tested | `005C5860`, `005C4AD0` |
| analyzed | `005CAA40`, `005C9E30`, `005CA300`, `005C9B60` |
| exported only | `005C9830`, `005C9370`, `005C92F0`, `005C6850`, `005C49C0` |

None of it is binary compatible: the records are C++ containers, not the engine's iterator-debug
vectors and `{size, char*}` strings. Every routine named here has a Ghidra function; the packet
has no listing-only routines and `bsp.py ghidra flow` reports no gaps in any of the four large
readers.


## Correction from docs/MISSION_SIDE_STORAGE.md

5C6A70 now writes screen-facing side fields directly into `record.screen.sides`
and the disjoint remainder into `record.side_extras`. Menu consumers and the Lua
reader share these same values; the executable's post-load copy pass is removed.
5C5DA0 writes its enabled BYTE at5C5DAA, not a DWORD at5C5DA5. The four Lua list
keys are multiunitsidIC1v1/IC2v2/IC3v3/IC4v4. Real Lua5.1.1 and installed table
checks passed; full executable startup still failed before tree loading and is
not evidence of gameplay equivalence.
