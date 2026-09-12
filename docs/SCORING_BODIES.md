# Scoring bodies (packet `cc2_scoring_bodies`)

Addresses: 008bb770, 007fc9f0, 007fcbc0, 007f8b40, 007fc940, 008bc540, 00915760, 008d2d60,
007fd510, 008bbe00, 0090bda0, 008bc0c0, 0090be30, 008bc9b0, 008d0140, 0091bda0, 0077ce60,
00803510, 004b4750, 00593570, 00593ca0, 0091c560

Follow-up of `docs/SCORING_BINDING_TABLE.md`, which left five bindings `contract: unread`,
the kill trees unopened and the `GlobalConfig+2Ch` multiplier vector unknown. The 284h record
layout, the party and unit-class dictionaries and the persisted container shapes come from
`docs/MISSION_PROGRESS_ARCHIVE.md` and `include/bsp/mission_progress.hpp`; they are cited, not
re-derived. Ghidra was read-only for this packet.

## 1. The commit-slot accessor, `004B4750`

`int __fastcall(Game*)`, body `004B4750..004B476x`, `RET`. It returns
`[game+21A0h] + [[game+21A0h]+1424h]*284h + 4`, the record of the commit slot. Every routine
below that writes "the record" without a slot argument goes through it: `008BBE00`,
`008BC0C0`, `008D0140`, `007FCD90`, `00626C50`, `006B5960`, `006B7710`. It ignores any slot
the script passed.

## 2. `Scoring_GrantBonus` (008BB770) writes the player profile, not the scoring record

Both callees take `ECX = [00e188a8] + 650h`. Evidence, `008BBB07` `MOV ESI,[0x00e188a8]`,
`008BBB17` `ADD ESI,0x650`, `008BBB32` `MOV ECX,ESI`; the other branch is `008BB964`/`008BB974`.
That object is the player profile: `BSP_LuaBinding_ScoringIsUnlocked` (008BBC40) and
`BSP_Profile_IsUnlockSatisfied` (007FC4C0) reach the same `+650h` subobject, and the debrief
screen `0062C2C0` reads it at `0062C557`, `0062C5A1`, `0062C5C4`, `0062C69A`, `0062C852`,
`0062C87F` and `0062CE41`. **No part of `Scoring_GrantBonus` touches `[game+21A0h]`.**

The binding tests whether Lua argument 3 is a string and calls one of two callees. Both are
`__thiscall(profile, NativeString* a, NativeString* b, NativeString* c, <int|NativeString*> d)`
and both end `RET 10h` (`007FCBBA`, `007FCD8A`), so four stack arguments (rule 7). Argument
order is the Lua order: native argument 1 is Lua argument 0.

| Callee | Selected when | Record built |
| --- | --- | --- |
| `007FC9F0` | argument 3 is **not** a string | `{Lua1, Lua2, "", integer(Lua3)}` |
| `007FCBC0` | argument 3 **is** a string | `{Lua1, Lua2, Lua3, 0}` |

Both do the same three steps.

1. Walk the `8h`-stride vector at `profile+ACh` (`007FCA18` `LEA EDI,[ECX+0xac]`, `_Myfirst`
   `+B0h`, `_Mylast` `+B4h`, step `007FCAA6` `ADD ESI,0x8`) comparing each `NativeString`
   with argument 1: equal lengths, then `__stricmp`. On a hit the next iteration returns and
   nothing is written (`007FCAAB` `CMP byte ptr [ESP+0x13],0x0`, `JNZ` to the epilogue).
2. `00450540(profile+ACh, argument1)` appends the name to that vector.
3. `007F8B40` builds a 1Ch record on the stack from three `NativeString`s and an int, and
   `007FC940(profile+BCh, record)` appends it to a `1Ch`-stride vector (`007FCB88`
   `ADD ECX,0xbc`; `007FC940`'s size divisor is `0x1c`). `0061F630` destroys the temporary.

The record's four fields are `+00h/+08h/+10h` `NativeString` and `+18h` int, from `007F8B40`'s
six zeroed words plus the trailing integer. The pushed block is assembled downward at
`007FCAC2`, `007FCAC3`, `007FCAFC` and `007FCB39`, so ascending stack order is
`{argument 2, argument 3, third slot, int}`.

**Nothing in the image reads `profile+BCh`.** Scanning every `81 /r imm32` with `0BCh`
(`ADD reg,0BCh`) returns only `007FCB88` and `007FCD58`, the two writers, plus their unwind
funclets. The list is write-only along that addressing path; see the open questions.

## 3. `Scoring_ClearPlayerScore` (008BC540) and the record clear `00915760`

`00915760` is `__fastcall(MissionScoreRecord*)`, body `00915760..00915F1C`. It zeroes every
scalar of the 284h record and empties every owned container: the seven keyed maps by
`STL_NativeStringIntTree_EraseSubtree` on `_Myhead->_Parent` followed by the three
self-pointers and `_Mysize = 0`, then the trace trees, the objective maps and the loss maps.
The word indices fix the offsets, `param_1[7]` being `_Myhead` of the map at `+18h` and
`param_1[0x72..0x78]` the seven totals at `+1C8h..+1E0h`. Its other caller is
`BSP_MissionPlayerRecords_Reset` (00916980), which runs it over all eight slots.

It does **not** preserve anything. The binding does: `008BC540` reads
`manager + slot*284h + 0Ch` and `+254h` into locals, calls `00915760` at `008BC6B4` with the
record in `ECX`, writes both back, then sets `manager + 14B0h + slot = 1`. In record-relative
terms those are `difficulty_08` and `used_slot_250`.

The slot is Lua argument 0 read as an integer, and the whole body is skipped when argument 0
is absent or not an integer.

## 4. `Scoring_ClearAllMissionsScore` (008D2D60) and the map erase `007FD510`

`007FD510` is `__thiscall(map*, node*)` with the node on the stack, the MSVC `_Tree::_Erase`
shape: recurse on `_Right` (`007FD546`), iterate on `_Left` (`007FD54B`), destroy the value
through `00593570` and the key `NativeString`, then `_free` the node. The `_Isnil` test is
`007FD52B` `CMP byte ptr [EBP+0x29d],0x0`, which fixes the node layout at
`{_Left, _Parent, _Right, NativeString key +0Ch, MissionScoreRecord +14h, _Color +29Ch,
_Isnil +29Dh}` — `0x14 + 0x284 = 0x298`, so this is `MissionScoreMapStorage`'s node.

The binding takes no arguments. `008D2E44` loads `[game+6B4h]`, the `MissionProgress` object,
`008D2E5B` calls `_Erase` on the root of the map at `+0h` with `ECX = ` that object, resets the
head's three self-pointers and `_Mysize`, then calls `BSP_MissionPlayerRecords_Reset`. So it
is `mission_scores_00.clear()` followed by a clear of all eight live records.

## 5. The two mission messages, `0090BDA0` and `0090BE30`

Both are `__thiscall(record, NativeString by value)`: resize the destination and `memcpy`.
`0090BDA0` writes `record+254h` (length) and `record+258h` (text); `0090BE30` writes `+25Ch`
and `+260h`. Neither offset appears in the archive's field table, so neither message is
persisted with the profile.

The `this` is the **commit-slot** record. Evidence, `008BC015..008BC032`:

```
008bc015  MOV ECX,dword ptr [0x00e188a8]
008bc023  CALL 0x004b4750          ; EAX = commit-slot record
008bc028  MOV ECX,EAX
008bc032  CALL 0x0090bda0
```

Both bindings read an explicit slot in multiplayer, require
`[game+18CCh + slot*4] + 50h != 0`, and build a session message (`type 15h` for the condition
message). When `[game+18CCh + slot*4] + 0Bh` is clear they apply it locally through the two
setters above; otherwise they call `BSP_Session_SendMessageToNonlocalPeer`. The remote peer
applies the same message through `0091C650`, whose `case 0x15` calls `0090BDA0` at `0091CB6D`
and whose next case calls `0090BE30` at `0091CB97`. **The slot argument therefore selects the
peer, never the record**: the local write always lands on the commit slot.

**No consumer of `record+254h` or `record+25Ch` exists in the image.** Every `LEA` and `MOV`
with disp32 `254h`, `258h`, `25Ch` and `260h` was enumerated; inside the scoring cluster the
only hits are the two setters, the record copy `00593CA0`, the record destructor `00593570`,
the record clear `00915760` and `0091C560`'s per-slot seeding, whose `+254h` is the manager
expression for `used_slot_250` and not a message. Nothing renders or returns them.

## 6. The kill trees at `record+B4h` and `record+C0h`

### Writer

`0091BDA0`, `__thiscall(ScoringManager*, Unit*)`, body `0091BDA0..0091C553`. Its only caller is
`BSP_Unit_OnDestroyed` (00959450); the call site fixes both arguments:

```
0095950c  MOV ECX,dword ptr [0x00e188a8]
00959512  MOV ECX,dword ptr [ECX + 0x21a0]   ; ECX = the scoring manager
00959518  PUSH EDI                            ; EDI = the destroyed unit
00959519  CALL 0x0091bda0
```

`00959450` reaches it only when the mission clock passes `00D7A24C`, `unit+70h == 1` and
`[game+19CCh]+4ACh` is set, and only on the branch where the owner at `unit+538h` answers
virtual `+18h(17h)` or `+18h(6)` with a set byte; otherwise the kill goes to `009813A0`, the
mission-event reporter, and no tree is touched. That condition is `00959450`'s, already
recorded in its plate comment, and is unchanged here.

The increment is one chained `operator[]` per level:

```
0091bfa3  IMUL ECX,ECX,0x284
0091bfab  LEA EBP,[ECX + EDX*0x1 + 0x4]   ; EDX = the manager, EBP = the record
0091bfb9  JNZ 0x0091bff8                   ; taken when [unit+2D8h] != [unit+2DCh]
0091bff0  LEA ECX,[EBP + 0xb4]             ; player_kills
0091c02d  LEA ECX,[EBP + 0xc0]             ; party_kills
0091c033  CALL 0x0062c170                  ; level 1
0091c038  MOV ECX,EAX
0091c03a  CALL 0x0062bb60                  ; level 2
0091c03f  MOV ECX,EAX
0091c041  CALL 0x00625900                  ; level 3
0091c046  ADD dword ptr [EAX],0x1
```

The record slot is `[unit+2D8h]`, rejected when above 7 (`0091BF86` `CMP EAX,0x7`, `0091BF97`
`JA`, an unsigned test). The tree is `+B4h` when `[unit+2D8h] == [unit+2DCh]` and `+C0h`
otherwise. Both branches push the same three keys in the same order; the first call consumes
the last push.

| Level | Key | Source |
| --- | --- | --- |
| 1 | relative party | `00803510([unit+2D0h], [unit+54h])`, pushed at `0091BFEF` / `0091C02C` |
| 2 | attacker unit class | `[unit+2CCh]`, pushed at `0091BFEA` / `0091C02B` |
| 3 | victim unit class | virtual `+0h` on `[unit+170h]`, pushed at `0091BFE1` / `0091C022` |

`00803510` is `byte __fastcall(subject side, reference side)`: `2` when either side is
neutral and the other is not, `0` when the sides match, `1` otherwise. With the attacker's
side as the subject and the victim's own side as the reference it yields `ENEMY` for an
ordinary enemy kill, which is the value `Scoring_GetPlayerShotDown` filters on.

The whole function does more than this: per-slot string counters over `game+18CCh..18ECh`,
the `Counter_RUA_DU` token and several award paths. Coverage for `0091BDA0` is
**partial: the kill-tree write only**, `0091BDA0..0091BF7F` and `0091C049..0091C553` unread.

### The producer of the attribution fields

`0077CE60`, `__thiscall(Unit* victim, HitRecord*)`, body `0077CE60..0077D19F`, writes the
block the kill writer reads, on every damaging hit above `00D7A218`. It is also the caller of
`00915F20`, the damage-trace recorder for `record+9Ch`/`+A8h`.

| Field | Written from | Meaning |
| --- | --- | --- |
| `+2C4h` | the attacker unit pointer | read back at `0091C049` |
| `+2CCh` | virtual `+0h` on `attacker+170h`, forced to `0Ch` on a kamikaze branch | attacker unit class |
| `+2D0h` | `[[hit+CCh]+54h]` | attacker side |
| `+2D8h` | `attacker+180h`, falling back to `[hit+1Ch]` when it exceeds 7 | credited player slot |
| `+2DCh` | `[hit+1Ch]`, or `attacker[+1ACh + n]` on the `6`/`1Bh` capability branch | originating player slot |
| `+2E4h` | `00F876A4` | the mission clock stamp |

The `+2DCh` fallback chain has a branch whose register `Ghidra` lost (`unaff_EBX` at
`0077CEF0`); that branch is unread. `+2D8h` and `+2CCh` are read end to end.

### Readers

`Scoring_GetPlayerShotDown` (008BC9B0) walks `record+B4h` only. It reads Lua argument 0 as an
integer **defaulting to 0**, not to the local player slot, which differs from every keyed
binding in `docs/SCORING_BINDING_TABLE.md`. It descends only into level-1 key `1`, iterates
every level-2 key, and accumulates every leaf whose level-3 key is `7`, `8`, `9`, `0Ah`, `0Bh`
or `0Ch` — exactly `levelbomber`, `divebomber`, `torpedobomber`, `fighter`, `reconplane`,
`kamikaze`, the six aircraft classes of the archive dictionary. The six compares all reach the
one accumulate at `LAB_008BCCBF`. `record+C0h` has no Lua reader.

`00911E80 BSP_BotSideAi_RebuildUnitDemands` also walks a three-level tree at `+B4h`, but on
its own AI record, not on a `MissionScoreRecord`; `docs/BOT_SCHEDULER_OUTPUT.md` line 95 is
about that object and is unrelated to this one.

## 7. `Scoring_GetUnitTypeShotDown` (008D0140) reads the loss maps, not a kill tree

It builds an empty `NativeString -> int` tree on the stack, copy-assigns one of the commit
record's two loss maps into it, looks up Lua argument 1 with `004C8B80` (find, no insert),
pushes the mapped value or `0`, and frees the copy. Lua argument 0 is a boolean: true selects
`record+1ECh` (allied losses), false `record+1F8h` (Japanese losses); both branches join at
`LAB_008D02F0`. The commit record comes from `004B4750`, called once before the branch, the
same accessor as the message setters. These are the `objective/allied_losses,japanese_losses`
maps of
`docs/MISSION_PROGRESS_ARCHIVE.md`, keyed by name, not by unit class.

## 8. The difficulty multiplier vector at `GlobalConfig+2Ch`

Producer: `BSP_GlobalConfig_LoadFromLuaGlobals` (0087D7B0), the `scripts/datatables/globals.lua`
reader. It fetches `Globals.Difficulty` and then its four sub-tables by name, in this order:
`HPMultipliers`, `ScoreMultipliers` (string at `00D0E50C`), `LockRadiusMultipliers`
(`00D0E4F4`), `PlayerCheatMultipliers` (`00D0E4DC`). One loop then indexes each sub-table with
the same 1-based index and `push_back`s into four `std::vector<float>`:

| Sub-table | Vector | Stored value |
| --- | --- | --- |
| `HPMultipliers` | `config+1Ch` | `1 / x` |
| `ScoreMultipliers` | `config+2Ch` | `x` |
| `LockRadiusMultipliers` | `config+3Ch` | `x` |
| `PlayerCheatMultipliers` | `config+4Ch` | `x` |

Evidence that `+2Ch` is the score table: the second fetch in the loop uses the container
whose `GetByName` destination is the one built at `0087DAAC` with the string `00D0E50C`, and
its `push_back` target is `0087DC08` `LEA EBP,[ESI+0x2c]`.

The installed `scripts/datatables/globals.lua`, line 11, is
`["ScoreMultipliers"] = { 1/4, 1/2, 1, }`. **Length 3, values 0.25, 0.5, 1.0.** The bounds
check at `00910436` therefore accepts difficulty indices 0..2, and the multiplayer value forced
at `00910410` (`MOV EDI,0x2`) is the last entry, a multiplier of exactly 1. `00910500` and
`00910570`, the debrief's two score kernels, index the same vector with the same bounds test.

## Routine table

| Address | Name | Coverage | Status |
| --- | --- | --- | --- |
| 004b4750 | `BSP_MissionScoring_GetCommitRecord` | complete | analyzed, reconstructed |
| 008bb770 | `BSP_LuaBinding_ScoringGrantBonus` | complete | analyzed, reconstructed, build-tested |
| 007fc9f0 | `BSP_PlayerProfile_GrantBonusValue` | complete | analyzed, reconstructed, build-tested |
| 007fcbc0 | `BSP_PlayerProfile_GrantBonusText` | complete | analyzed, reconstructed, build-tested |
| 007f8b40 | `BSP_ProfileBonusRecord_Construct` | partial: only the field order and the 1Ch size | analyzed |
| 007fc940 | `BSP_ProfileBonusVector_PushBack` | complete | analyzed |
| 008bc540 | `BSP_LuaBinding_ScoringClearPlayerScore` | complete | analyzed, reconstructed, build-tested |
| 00915760 | `BSP_MissionScoreRecord_Clear` | complete for the offsets it clears | analyzed |
| 008d2d60 | `BSP_LuaBinding_ScoringClearAllMissionsScore` | complete | analyzed, reconstructed, build-tested |
| 007fd510 | `STL_MissionScoreMapNode_EraseSubtree` | complete | analyzed |
| 008bbe00 | `BSP_LuaBinding_ScoringSetConditionMessage` | partial: the session-message branch unread | analyzed, reconstructed, build-tested |
| 0090bda0 | `BSP_MissionScoreRecord_SetConditionMessage` | complete | analyzed, reconstructed, build-tested |
| 008bc0c0 | `BSP_LuaBinding_ScoringSetVictoryMessage` | partial: the session-message branch unread | analyzed, reconstructed, build-tested |
| 0090be30 | `BSP_MissionScoreRecord_SetVictoryMessage` | complete | analyzed, reconstructed, build-tested |
| 008bc9b0 | `BSP_LuaBinding_ScoringGetPlayerShotDown` | complete | analyzed, reconstructed, build-tested |
| 008d0140 | `BSP_LuaBinding_ScoringGetUnitTypeShotDown` | complete | analyzed, reconstructed, build-tested |
| 0091bda0 | `BSP_MissionScoring_RecordUnitKill` | partial: 0091bda0-0091bf7f and 0091c049-0091c553 | analyzed, reconstructed, build-tested |
| 0077ce60 | `BSP_Unit_RecordDamageAttribution` | partial: the `+2DCh` fallback branch unread | analyzed |
| 00803510 | `BSP_Party_RelativeTo` | complete | analyzed, reconstructed, build-tested |
| 00593570 | `BSP_MissionScoreRecord_Destruct` | partial: identified by its callers and its two message fields | analyzed |
| 00593ca0 | `BSP_MissionScoreRecord_Assign` | partial: same | analyzed |
| 0091c560 | `BSP_MissionScoring_ResetForNewMission` | complete | analyzed |

## Corrections to earlier documents

- `docs/SCORING_BINDING_TABLE.md` follow-ups say `Scoring_GetUnitTypeShotDown` (008D0140)
  "reads the kill trees at `+B4h`/`+C0h`". It reads neither; it reads the `name -> int` loss
  maps at `+1ECh`/`+1F8h` of the commit record.
- The same follow-up groups `Scoring_GrantBonus` with routines that "write outside the keyed
  maps" of the scoring record. It writes no part of the scoring record; its target is the
  player profile at `game+650h`.
- `docs/SCORING_BINDING_TABLE.md` leaves the multiplier table `contract: unread`. It is
  `Globals.Difficulty.ScoreMultipliers`, length 3, `{0.25, 0.5, 1.0}`.

## Open questions

- Nothing reads `profile+BCh`, the bonus record list, along an `ADD reg,0BCh` path, and
  nothing reads `record+254h`/`+25Ch`. Both were scanned by disp32 across the whole image. A
  reader reaching them through an already-offset pointer would not appear in those scans, so
  "write-only" is a scan result, not a proof.
- The per-slot flag array at `manager+14B0h` has two writers (`008BC540` sets, `0091C560`
  clears) and no located reader.
- `0091BDA0`'s other two thirds: the per-slot `Counter_*` string maps and the award tokens.
- `0077CE60`'s `+2DCh` fallback on the `6`/`1Bh` capability branch, where the decompiler lost
  a register.
- Which of `0062C2C0`'s seven `game+650h` sites, if any, renders the bonus list. The screen
  calls `005097D0`, `007FCD90`, `007F93F0`, `007FB990` and `00574050` on the profile; none was
  opened.
