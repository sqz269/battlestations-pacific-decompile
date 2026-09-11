# Mission-progress archive storage and traversal

Addresses: 00920000, 0090cb40, 00920e10, 0090bf50, 00908a30, 00594a70,
00916a00, 0091ce90, 0091d620, 0085bce0, 00805990, 00625f50, 0090e740,
00915610, 0062c170, 0090c560, 007fd780.

Packet `orch2_mission_progress`, branch `agent/orch2-mission-progress-20260910c`.
Analysis used existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; each `bsp.py ghidra` batch verified that target.
Ghidra was read-only. Names in the repository are descriptive hypotheses;
the primary integrator owns applying annotations and saving the project.

The implementation in `include/bsp/mission_progress.hpp` and
`src/mission_progress.cpp` supplies real owned storage, native-format traversal,
and the separate multiplayer sum. It removes the need to invent a successful
score allocation or substitute the unlock-completion projection for score data.
It models all fields visited by the native persistence routines; unpersisted
runtime fields of the native 284h record remain outside this persistence model.

## Top-level object and the two layouts

`00920e10` constructs three empty 0Ch trees inside a 24h object. `00594a70`
is case-insensitive string-map `operator[]`, returns node+14h, and constructs
new records through `0091d620`/`0091ce90`. A large node holds its key at +0Ch,
record at +14h, count at +298h, and sentinel byte at +29Dh. The two counter
trees use a key at node+0Ch, int at +14h, sentinel byte at +19h.

| Object offset | Archive key | Storage |
| --- | --- | --- |
| +00h | `missionScores` | Case-insensitive mission name -> score record and count |
| +0Ch | `singleBestScores` | Case-insensitive name -> int |
| +18h | `MultiScores` | Case-insensitive name -> int |

At `00920047` the reader tests for `missionScores`. When present, it enters
`singleBestScores` unconditionally and reads enumerated name/int fields; it
then optionally reads `MultiScores`, then enters `missionScores`. When absent,
it enumerates the current section directly as the legacy mission map. Every
mission entry contains a required `sum` record section and required int
`count`. The reader never clears any tree. Duplicate names collide ignoring
ASCII case and update the existing record; the original stored key remains.

`0090cb40` always writes the three current sections in the table's serialization
order `singleBestScores`, `MultiScores`, `missionScores`, even when empty.
Both top-level counter maps include zero values. `0090bf50` accumulates only
the +18h tree, with 32-bit wrap. `0090c560` tests the large record's first int,
so `sync_mission_completion` projects `mission_completed_00`, not `count`, the
difficulty scores, or either top-level counter map.

## Record fields and types

Offsets below are relative to the record at node+14h. Field type tags are
1=int and 2=float. `0091ce90` initializes every represented scalar to zero,
initializes represented containers empty and clears +24Ch; `0091d620` also
zeros the +284h count. The C++ struct has a new layout.

| Archive path inside `sum` | Native offset | Type / shape |
| --- | --- | --- |
| `mission_completed`, `ranking`, `difficulty` | +00h,+04h,+08h | int |
| `play_time`, `completion_time`, `map_usage` | +0Ch,+10h,+14h | float |
| `TotalScores/[0..2]/completed` | +26Ch,+270h,+274h | int, numeric section keys |
| `TotalScores/[0..2]/score` | +278h,+27Ch,+280h | int, numeric section keys |
| `TotalScores/winnerMode` | +194h | int |
| `total/mission,action,ship,plane,command,badge,total` | +1C8h..+1E0h, stride 4 | seven ints |
| `ScoreMaps/mission,action,ship,plane,command` | +18h,+24h,+30h,+3Ch,+48h | name -> int trees |
| `ScoreMaps/missionMedals,actionMedals` | +54h,+60h | name -> int trees |
| `trace/player_damages,party_damages` | +9Ch,+A8h | unit class -> party -> unit class -> int |
| `trace/player_kills,party_kills` | +B4h,+C0h | party -> unit class -> unit class -> int |
| `trace/unit_remaining` | +CCh | party -> unit class -> int |
| `trace/unit_suicide` | +D8h | unit class -> int |
| `trace/unit_usage` | +114h | unit class -> float |
| `objective/allied_party,japanese_party` | +1E4h,+1E8h | int |
| `objective/allied_losses,japanese_losses` | +1ECh,+1F8h | name -> int trees |
| `objective/allied_primary_objectives,allied_secondary_objectives,allied_hidden_objectives` | +204h,+210h,+21Ch | name -> (name,int) trees |
| `objective/japanese_primary_objectives,japanese_secondary_objectives,japanese_hidden_objectives` | +228h,+234h,+240h | name -> (name,int) trees |
| `usedSlot` | +250h | int |
| checkpoint `repairuses,formationuses,shipvsShip,islandCapture` | +168h,+16Ch,+170h,+174h | four ints; quirk below |
| checkpoint pending flag | +24Ch | byte; not read from archive |
| sibling `count` outside `sum` | +284h | int |

The source's `NamedOffset` tables bind literal names to native offsets and
drive the repetitive counter/total/objective/checkpoint traversal. Assembly
destination instructions confirm the table; the report records evidence ranges.

## Reader and writer policies

`00916a31` compares global game+738h to 2 with signed `JL`. Versions below 2,
including high-bit values, read no record fields; the enclosing reader still
reads `count`. The new API receives the original global version explicitly.
For version >=2, `play_time` is required. `mission_completed`, `ranking`,
`completion_time`, and `difficulty` use the reader's default-zero virtual.
Every remaining scalar read is conditional on the field being present.
`TotalScores` itself and its numeric children are optional. Existing missing
values remain unchanged. The reader always enters `trace`; its children are
optional. Score maps and objective containers are optional and merge their
enumerated contents. The two objective party fields additionally invoke the
default-zero virtual after their presence checks.

Each objective outer key owns one `(name,int)` pair, not a nested map.
`00919150..00919241` and five repeated blocks assign each enumerated inner key
to that pair's string, then read its int; the final inner key wins in reader
enumeration order. Empty inner sections create no outer record. The nested
trace trees likewise allocate through `operator[]` only when reading a leaf.

The writer always emits `play_time`, three numeric `TotalScores` children,
`winnerMode`, all seven totals, `ScoreMaps`, `trace`, `objective`, `map_usage`,
and `usedSlot`. When completion is zero it suppresses all of
`mission_completed`, `ranking`, and `completion_time`, even if the latter
two are nonzero. Otherwise ranking and completion time are individually
omitted when zero. Difficulty is individually omitted when zero.

Named score/loss trees emit their section iff the stored map is nonempty,
then omit zero values; a nonempty all-zero map therefore produces an empty
section. Trace maps omit empty outer sections but retain zero int leaves;
unit usage omits zero float leaves. Float equality is numeric, including
equal signed zeros; NaN is unequal and therefore emitted. Objective values
always emit their one named int, even when zero.

Read score-map order is mission/action/ship/plane/command/actionMedals/
missionMedals. Writer order swaps plane before ship. Read trace order is
player_kills/party_kills/player_damages/party_damages; writer order is
player_kills/player_damages/party_kills/party_damages. The implementation
preserves these native orders and native numeric tree ordering.

## Enum dictionaries and checkpoint asymmetry

Party names at `00e0b080` are `OWN`, `ENEMY`, `NEUTRAL`, `UNKNOWN`, indexed
0..3. `00805990` compares case-insensitively and returns 4 on a miss. Unit
classes at `00e0b590` are mothership, destroyer, torpedoboat, battleship,
cruiser, cargo, landingship, levelbomber, divebomber, torpedobomber, fighter,
reconplane, kamikaze, submarine, landvehicle, landfort, airfield, shipyard,
path, other, indexed 0..19. `0085bce0` compares only entries 0..18 and returns
19 for all other names. Both dictionaries were read from the configured PE
and cross-checked against the parser/writer assembly references.

`0090aaa5` tests +24Ch. If set, the writer emits the literal `ChackPointData`
at `00d1850c`, writes the four counters, closes that section, and clears the
flag at `0090ab92`. Consequently record and top-level write APIs are mutable.
`00919ef8` instead tests `CheckPointData` at `00d18920`. Assembly proves it
does not enter that section: `00919f14..0091a05e` conditionally reads the four
counter keys directly from the current `sum` section. It never sets +24Ch.
The implementation preserves both the spelling mismatch and navigation,
rather than making these fields roundtrip by changing native behavior.

## ABI, validation, and boundaries

| Addresses | Original ABI / complete normal-flow tail |
| --- | --- |
| 00920000 / 0090cb40 | ECX=24h object, archive pointer; RET4 at 009205D3 / 0090CDCB |
| 00916a00 / 00908a30 | ECX=record, archive pointer; RET4 at 0091A074 / 0090AB9F |
| 00594a70 | ECX=large map, native string pointer; EAX=record pointer; RET4 at 00594B50 |
| 0090bf50 | ECX=24h object; EAX=wrapped int sum; RET at 0090BFBD |
| 00920e10 / 0091ce90 / 0091d620 | ECX=object; EAX=this; RET at 00920EBA / 0091D619 / 0091D635 |
| 0085bce0 | ECX=native string; EAX=unit class; RET at 0085BD45 |
| 00805990 | by-value native string; EAX=party; RET8 at 00805A60 |

Normal-flow C++ reconstruction, not ABI-compatible native node layout,
allocator behavior, debug-iterator assertions, or SEH cleanup. Pseudocode has
incorrectly propagated stack/register identities; assembly was used for
arguments, offsets, enum-map axes, return conventions, and checkpoint state.
All packet bodies have complete assembly tails. No new function creation or
body extension was required. The known `007fd780` Ghidra body issue remains
the primary's metadata responsibility; see `GAME_PROFILE_RESET.md`.

The existing reader caps its native 250-key array. Native persistence assumes
text map keys; this C++ interface rejects numeric/null map keys instead of
dereferencing their payload as a pointer. Unknown party names remain integer
4 after reading; attempting to serialize an out-of-range enum throws because
the native writer would index outside its name table. These cases are not
claims of invalid-input compatibility.

Win32 MSVC `scripts/build.ps1` passed with `mission_progress.cpp` compiled and
no warnings; both existing CTests passed after eight seed-byte checks matched
the installed executable. Worker log: `local/mission-progress-build.log`.
The build used the primary's indexed `ProfileArchiveWriter::begin_section`
overload from its integration branch (commit `a9da34c`); that shared header
change belongs to the primary and is intentionally excluded from this packet.
The primary's ignored real-Lua fixture `local/profile_persistence_probe.cpp`
owns combined archive/profile/mission persistence validation; no duplicate
fixture or checked-in tests were added here. Build/seed checks alone are not
mission binary differential testing or game validation.
