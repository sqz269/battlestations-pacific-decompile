# Player-profile archive traversal
Addresses: 007fdf00, 007f9540, 004425c0, 00920000, 0090cb40, 0090bf50, 007fd780, 00920e10, 004c1e90, 00425c20, 004374f0

Packet `orch2_profile_archive`, branch `agent/orch2-profile-archive-20260910b`.
Analysis used existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Every live analysis/export batch went through
`bsp.py ghidra`, which verifies that configured project and program. This worker
made no Ghidra mutations and did not touch the game installation or real saves.

`profile_archive.hpp/.cpp` implement the normal-flow **read** body at `007fdf00`
and its separate **write** counterpart at `007f9540`, reusing `ProfileResetState`,
`ProfileUnlockState`, `GuiLuaReader`, the Lua variant keys, and `SettingsValue`.
There is no second profile model. These are new C++ interfaces, not binary
replacements. Names remain descriptive hypotheses rather than recovered symbols.

## Read versus write and original ABI

Both bodies are `__thiscall(ECX=profile, archive*)`, `RET 4`:

| Entry | Last instruction | Inclusive end | Traversal |
| --- | --- | --- | --- |
| `007fdf00` | `007fee0e: RET 4`, length 3 | `007fee10` | Read fields and replace/rebuild state |
| `007f9540` | `007f9caa: RET 4`, length 3 | `007f9cac` | Write values; upgrade signed version below 2 |

Read completion `007fefe0` constructs `004425c0`, the existing Lua reader with
vtable `00ce44fc`. Read virtuals `+4h/+8h` enter/leave a section, `+10h` reads
to a `(type,destination)` pair, `+0Ch` adds a `(type,default value)` pair,
`+14h` tests presence and `+18h` enumerates keys. Their bodies and conversion
rules are already documented in `docs/GUI_LUA_READER.md`; the implementation
calls that concrete reader directly. For instance `007fdf66..007fdf88` passes
the address of profile+E8h and default integer 0.

Writer virtuals `+4h/+8h` enter/leave and `+0Ch` receives immediate values,
not destinations (`007f95b0..007f95dd` passes profile+E8h's value). The writer
backend remains an explicit three-operation interface. Key tags are 0 for names
and 1 for zero-based indices; value tags are 0 string, 1 int, and 3 bool.

## Field and section routing

Offsets below are relative to `game+650h`. Table order follows the reader;
the writer emits the fields in its own recovered order.

| Save key | Profile field | Read policy | Write policy |
| --- | --- | --- | --- |
| `Name` | +3Ch `player_name_3c` | Not read by this routine | First field, current player name |
| `Version` | +E8h | Default 0 | After `Name`, signed values below 2 become 2 in the profile |
| `Voice` | byte +59h | Direct bool read | Bool |
| `Difficulty` | +5Ch | Direct int read | Int |
| `SelectedDifficulty` | +60h | Default to just-read `Difficulty` | Int |
| `JapanNoseArt` / `AlliedNoseArt` | +E0h / +E4h | Default 1 each | Ints |
| `SelectedMissionID` | +68h string | Direct string read | Written after both drop rates |
| `DropRateTC` / `DropRateDC` | +ECh / +F0h | Preserve each field if its key is absent | Always write both |
| `Bonus` | +ACh string vector | If present, clear then append indices from 0 until first absent | Always emit section, vector order |
| `Unlocks` | +70h string set | Always enter, clear, and insert consecutive indexed strings | Always emit section, set order |
| `AllSeenHints` | +08h string list | Conditional clear, then optional append and limit policy below | Omit empty section, otherwise list order |
| `SavedLobbyFilters` | +14h list and +20h..+2Ch ints | Defaults and signed count policy below | Omit when +2Ch equals 0; otherwise write every stored list element |
| `SeenUnlocks` | +88h string set | Clear before hint processing, then optionally insert indexed strings | Omit empty section, otherwise set order |
| `Achievements` | +A0h string/int map | Always clear; optional current or legacy format below | Always emit; an empty map writes `RANK=1` without modifying the map |
| `Score` | +64h mission-progress pointer | Reallocate and call `00920000` | Call `0090cb40` with current score storage |

Required int reads from nil become 0 through the existing reader; string reads
become empty and bool reads false. Only `Version`, `SelectedDifficulty`, and
the two nose-art fields use the default-value virtual. No read of `Name`, XUID,
display name, pending unlocks or DLC content IDs is invented here.

Reader evidence: scalars `007fdf45..007fe160`, bonus `007fe160..007fe295`,
unlocks `007fe295..007fe399`, hints `007fe3c4..007fe502`, filters
`007fe502..007fe9d9`, seen unlocks `007fe9d9..007fead3`, achievements
`007fead3..007fed4b`, and score/post-load `007fec70..007fedf6`.
Writer evidence: scalars `007f9568..007f971e`, collection and filter traversal
`007f971e..007f9b83`, achievements `007f9b83..007f9c74`, and score/end sections
`007f9c74..007f9ca3`. Raw strings at `00ce8ed0` and `00cef15c` confirm `Name`
and `RANK` respectively. `007f95a2` branches with signed `JGE`; high-bit version
words therefore also upgrade to 2.

## Compatibility and post-load policy

At `007fe3c4..007fe3d6`, get singleton `004c1e90`; if its field+08h is nonzero,
clear the existing hints before testing `AllSeenHints`. If that field is zero,
existing hints remain and the optional section appends. Only when the section
exists, a resulting list size strictly above 1000 clears the whole list
(`007fe4e7..007fe4f9`). This is not a prefix cap or deduplication.

Absent `SavedLobbyFilters` resets its three scalars to 0, declared count to 9,
and the list to five true bytes followed by four false bytes
(`007fe525..007fe5b9`). A present section clears the list and reads its four
named ints, substituting 0 for each absent key. A nonzero declared count starts
the indexed loop; existence is queried **before** the signed `index<count`
comparison (`007fe940..007fe962`). The loop stops at the first missing entry.
The writer gates the whole section only on count!=0 and walks the full stored
list; it does not resize or truncate the list to match the count.

For `Achievements`, `007feb40..007feb71` initializes 250 key tags to -1 and
enumerates the section. Named keys read an integer directly into the map entry.
Each tag-1 key instead reads a string using a separate index initially 0 and
increments that index; it never uses the enumerated key's numeric value
(`007fecc6..007fed17`). The resulting string gets value 1. Other tags are
ignored. Consequently sparse numeric keys retain the original unusual lookup
behavior, and Lua enumeration order can affect a map mixing both formats.

After achievements, the reader destroys and frees a present +64h object,
clears the pointer, allocates 24h bytes, and constructs through `00920e10` when
allocation is non-null. The new pointer is stored regardless. Enter `Score`,
call `00920000`, leave `Score`, leave `PlayerProfile`; only then sum via
`0090bf50`, assign profile+30h, clear the +94h transient records, and invoke
`004374f0` on singleton `00425c20`. `0090bf50` sums the separate mission-progress
+18h counter tree, so the projection does not substitute its existing mission
completion map for this total.

The host contracts cover those exact score-storage calls, the hints-owner
query, and the final manager call. Destroy includes the caller's `00bf65ac`
release of the 24h allocation. The score reader must synchronize the existing
completion projection with its selected storage. No score records, counters,
successful allocations or archive contents are synthesized. Native allocation
failure does not bypass the following score calls; there is no invented
graceful-success branch in this projection. The final manager can repopulate
the transient records just cleared and performs further save/unlock work;
its large body is required separately, not represented as an empty operation.

## Analysis repair and uncertainty

The integrator repaired four fallthroughs hidden by an incorrect no-return
annotation on `free`, under the Ghidra write lock, saved the project, and
refreshed the export. Parent evidence is `reports/profile_archive_flow_repair.json`.
The corrected spans are `007fe55c..007fe567` (12 bytes), `007fe5db..007fe5e4`
(10), `007fec84..007fec89` (6), and `007fedcb..007fedd5` (11). Raw bytes proved
the two list-clear loops, pointer-clear store, and transient-tree loop continue.
The remaining six-byte padding gap `007fea1a..007fea1f` follows an unconditional
jump, not a call. No missing function definitions were created by this worker.

SEH cleanup, secure-SCL checks, native allocator/string representation, the
full mission-score payload and final manager body remain outside this module.
The existing Lua reader caps enumeration at 250 entries whereas native code
can overwrite its frame above that count; equivalence is restricted to the
native array's valid input domain. Container projections inherit the existing
case-insensitive comparator and require representable signed loop counts.
These routines have no drop-in ABI, native-profile differential or game-runtime
validation claim.

## Validation

`python tools/ghidra_export.py verify-seeds` matched all eight configured seeds.
`./scripts/build.ps1` succeeded for MSVC Win32 Release and both existing CTests
passed. Those native differential checks concern the math seeds, not these
profile bodies. One ignored compatibility fixture at
`local/profile_archive_fixture.cpp`, compiled against the built `bsp_core.lib`,
passed using the concrete `GuiLuaReader`. It checks missing-field/default
behavior, conditional collection resets, hint overflow, legacy sparse numeric
achievement routing, score-call ordering, signed-version upgrade, the empty-map
`RANK` output, and full-list writing despite a smaller declared filter count.
Its host score values are explicit fixture data, not recovered or production
save contents. Results and artifact paths are in `reports/game_profile_archive.json`.
