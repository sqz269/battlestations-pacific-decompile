# Mission completion flags and return to the mission list

Addresses: 005c2f70, 005c27e0, 00599340, 0090c560, 005806a0, 00597870.

Packet `orch5_mission_map_flag`, branch `agent/orch5-mission-flag`.
The verified read-only Ghidra wrapper used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Descriptive names are hypotheses, not recovered
symbols. The integrator owns Ghidra annotations and source registration.

## Result

`005c2f70` asks whether the mission is completed. It does not inspect either
side block, any campaign selector, or the mission's unlock requirements.
The flag icon therefore represents the mission-progress completion field.
`00599340` returns from mission-detail page 9 to the selected mission's
campaign list, choosing between base/DLC and US/Japan lists.

`include/bsp/mission_map_flag_policy.hpp` and `src/mission_map_flag_policy.cpp`
reconstruct these two routines. They reuse `MissionRecordData`,
`MissionProgress`, `MainMenuPage`, `MainMenuScreenState`, and the existing
`mission_side_index_005c27e0` function. No existing implementation is changed.

## Completion adapter: 005c2f70

The entire native body is six instructions, `005c2f70..005c2f84` inclusive:

| Site | Operation |
| --- | --- |
| 005c2f70 | Copy incoming ECX, the mission-record pointer, to EAX. |
| 005c2f72 | Load game pointer from `00e188a8` into ECX. |
| 005c2f78 | Load the progress object from `game+6b4h` into ECX. |
| 005c2f7e | Push the original record pointer; its offset zero is the mission-id string. |
| 005c2f7f | Call `0090c560`, which consumes that one stack argument. |
| 005c2f84 | Bare RET, preserving AL from the callee. |

Native ABI: ECX-only record input, no stack arguments, boolean result in AL;
compatible with `__fastcall bool(record*)` or a parameterless member call.
The original source-level convention cannot be distinguished from these bytes.
The decompiler guesses a void return, but both actual consumers store AL:
`0058c301 -> 0058c310` in the detail builder, and `00597e07 -> 00597e14` in the
list builder. This is a return-type inference problem, not a missing body.

The existing `0090c560` contract is corroborated by its complete listing:
`0090c574` performs its case-insensitive lookup, `0090c591..0090c59d` returns
false for the end iterator, and `0090c5b3..0090c5c1` returns whether the first
record int at `node+14h` is nonzero. A negative completion int is true too.
`game+6b4h` is the same pointer as `profile+64h` with profile at `game+650h`.
See `MISSION_PROGRESS_ARCHIVE.md` and `PROFILE_UNLOCK_PREDICATE.md` for the
existing container/comparator contract; no library container algorithm is ported.

The reconstruction composes the record-to-id adapter with that known contract
over `MissionProgress::mission_scores_00`. Only `mission_completed_00` counts;
ranking, difficulty counters, `count_284`, and the two other progress maps do
not enter the predicate. It does not insert a missing key. Normal C++ container
ownership and the existing ASCII case-insensitive comparator replace native
checked iterators and string storage.

## Selected-mission list refresh: 00599340

The complete stored body is `00599340..0059939c` inclusive, 34 instructions.
Native ECX is the original main-menu screen, with no `this` adjustment, no
stack arguments, and no meaningful return. It saves/restores ESI and uses a
bare RET. It has no x87, indirect calls, or undisassembled fall-through gap.

1. `00599340..0059934a`: return without calls unless `00e08874 == 9`.
2. `0059934c`: test byte `screen+565h`. This occurs before the record lookup.
3. `00599355` or `00599379`: call existing `005806a0` to get selected record.
4. `0059935a/0059937e`: pass that record in ECX to existing `005c27e0`.
5. Restore original screen in ECX and call `00597870` with exactly one page:

| screen+565h | Side index | Page argument | Call site |
| --- | --- | --- | --- |
| nonzero | 0 | 7, `CampaignUsDlc` | 00599369 |
| nonzero | 1 | 6, `CampaignJapanDlc` | 00599372 |
| zero | 0 | 5, `CampaignUs` | 0059938d |
| zero | 1 | 4, `CampaignJapan` | 00599396 |

`005c27e0..005c27eb` is exactly `XOR EAX,EAX; CMP byte [ECX+b8h],AL;
SETZ AL; RET`. It returns full EAX 0/1, selecting side 1 when the first side
block's enabled low byte is zero. Its existing correct name,
`BSP_MissionRecord_SideIndex`, and implementation are retained. Only that low
byte participates even though the reconstructed field is stored as a dword.

`MissionListRefreshHost` exposes the selected-record lookup and list-builder
calls. The proven side getter is called directly between them. The wrapper
snapshots the DLC byte before invoking the host and makes no direct page,
selection, or widget writes; those belong to the existing builder contract.
A selected record must be valid when page 9 is active, as native lookup traps
on invalid indices. There is no invented null-record fallback.

## Correction to the starting follow-up

The `mission_record_side_flag` follow-up in `MAIN_MENU_MISSION_DETAIL.md`
should be understood as mission completion, independently of side selection.
That document also calls `00599340` another route into `0058c010` and assigns
the calls at `005997c1` and `005998d0` to it. Those addresses lie outside
`00599340..0059939c`; the bounded routine here calls the mission-list builder
`00597870`, not the detail builder. The enclosing event-handler attribution
of those later sites is left for integration to verify. Existing docs are
unchanged by this packet.

## Verification and limits

All three owned routines have complete Ghidra functions and were exported
after inspecting their listings. `no_ghidra_function` is empty; no Ghidra
mutation or flow repair is needed for this packet. The code is reconstructed
with existing callee contracts, pending the primary integrator's combined
Win32 compilation and existing checks. No test was added for these bounded
rules, and no runtime/UI or native differential validation is claimed.
The C++ interfaces are not ABI-compatible replacement entry points.
