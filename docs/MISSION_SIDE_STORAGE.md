# Canonical mission-side storage

Addresses: `005C5DA0`, `005C6A70`, `005C27E0`.

`MissionRecordData::screen.sides` now owns the screen fields of both native
side blocks. `side_extras` owns their remaining fields at the corresponding
index. The reader fills those two references directly. There are no stored
references, pointer aliases, duplicate side screens or synchronization passes.
Standalone `MissionSideBlockData` remains useful outside a mission record and
its reader overload forwards to the same implementation.

Previously the Lua reader filled `record.sides[i].screen` while menu helpers
read `record.screen.sides[i]`. Every freshly loaded record therefore appeared
to have side0 disabled to those helpers. The executable worked around this
by copying all loaded screens once after loading. Removing the duplicate
storage fixes the producer and allows that entire copy pass to be removed.
The hidden-hint branch in `main_menu_selection_listener.cpp` continues to use
the same selected side index, now against `record.side_extras[side]`.

The same reader audit found four incorrect Lua keys. Semantic mode identifiers
remain `IslandCapture1v1` through `IslandCapture4v4`; a separate key array reads
the actual `multiunitsidIC1v1`, `IC2v2`, `IC3v3`, `IC4v4` fields into slots4..7.
The native destinations are side offsets114h,124h,134h,144h. No map-size or
mode identifier is renamed.

## Native evidence and boundaries

Read-only `bsp.py ghidra` queries verify configured project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base before each batch.
The configured project file is `C:/Users/sqz269/bsp.gpr`. No worker Ghidra
mutation, annotation, export refresh or save was performed.

| Routine | Exact current body | Original ABI | Coverage in this correction |
| --- | --- | --- | --- |
| `005C5DA0` | `005C5DA0..005C6238`,485 instructions | ECX=154h block, stack reader, `RET4` | Complete storage routing and side key coverage; existing abstract Lua semantics retained |
| `005C6A70` | `005C6A70..005C7B23`,1144 instructions | ECX=434h record, stack reader, `RET4` | Partial: side branch `005C767C..005C770F`; surrounding reader remains existing implementation |
| `005C27E0` | `005C27E0..005C27EB`,4 instructions | ECX=record, EAX=zero-extended Boolean, plain `RET` | Complete read-only verification; C++ getter unchanged |

All three live flow audits report zero gaps. All bodies exist. The getter's
machine convention fits a thiscall getter or a unary fastcall function; the
original declaration is unavailable. Descriptive names are hypotheses.

`005C6A9B` captures the record in EDI and `005C6A8E` captures the reader in
ESI. Both side calls push ESI and pass an address within that same record:

| Caller | Call site | Native callee | Verified argument and effect |
| --- | --- | --- | --- |
| `005C6A70` | `005C76B8` | `005C5DA0` | `LEA ECX,[EDI+0B8h]`, entered `allied` table |
| `005C6A70` | `005C7702` | `005C5DA0` | `LEA ECX,[EDI+20Ch]`, entered `japanese` table |
| `005C5DA0` | `005C6066/60A7/60E8/6129` | `005C5860` | Native IC keys and destination offsets114h/124h/134h/144h |
| `00580940` | `005809EA/00580A06` | `005C27E0` | Selected record in ECX, publishes page7-side for DLC or page5-side for campaign |
| `0058BDF0` | `0058BE15` | `005C27E0` | Selected record in ESI/ECX; next instructions form `ESI+0B8h+154h*side` |
| `0058BDF0` | `0058BF18` | `005C27E0` | Same record in ECX; resulting side subsequently passed in EDX |

`005C5DAA` is `MOV byte ptr [EDI],1`, not a dword store at005C5DA5 as an
older source comment said. `005C27E2` compares exactly the byte at record+0B8h
to zero. Side0 therefore wins if present; otherwise the chosen index is1,
including when neither side exists. The screen-side native relative offsets
are00h enabled,04h briefing key,14h/24h primary/secondary objectives,64h hints.
The separate extras contain every other represented field of that block.

The JSON report inventories every native CALL in both reader bodies, plus the
four selection/launch call sites above, with `address`, `native`, and containing
`function`. Non-side record operations are explicitly marked outside this
correction and unread here. Indirect reader calls remain indirect; no new
callee contract is inferred from arguments. `005C5860` is consumed as the
existing string-vector reader, whose current body is `005C5860..005C5981`.

## Verification

`./scripts/build.ps1` passed after the final source change: MSVC 19.51 Win32
Release, including `bsp_game.exe`; existing CTest `reconstructed_math` passed 1/1.
The fresh worktree's Lua download stalled, so the build used independent local
copies of the parent worktree's existing Lua5.1.1, zlib1.2.1 and D3DX header
sources through CMake's `FETCHCONTENT_SOURCE_DIR_*` cache overrides.

`python tools/verify_report_calls.py reports/mission_side_storage.json` passed
35 direct call rows with zero failures; 96 indirect rows are explicitly skipped
by that checker. This verifies call-site boundaries and target identity, not
virtual dispatch resolution.

The local fixture includes the executable's actual `LiveMissionTreeView` and
executes Lua source through `GuiLua51Host`/Lua5.1.1. It checks canonical object
identity from `mission_side_block`, both-side and Japanese-only selection,
missing-side defaults, repeated reads retaining absent list fields, matching
hidden hints, all four native IC keys (with a deliberately wrong expanded key
also present), copy independence, move/source reuse, vector relocation, live
enabled-byte selection, and the retained standalone block overload.

The probe also executes the unmodified installed `MissionTree.lua` and its
actual `MultiGlobals.lua` through Lua5.1.1, with a direct-file `DoFile` callback,
then reads the tables with that same executable adapter and reconstructed
group/mission readers. It closes Lua before checking record ownership. Output:

```text
installed id=IJN01 side=1 briefing=IJN01 objectives=3/2 hints=4 hidden_hints=1
installed id=USN02 side=0 briefing=USN02 objectives=2/1 hints=3 hidden_hints=1
PASS installed MissionTree.lua + MultiGlobals.lua: groups=5 campaign=143 multi=34 nonempty_IC_lists=102; real Lua + exact executable adapter, no screen-copy pass
```

The local source and command are `local/mission_side_storage_probe.cpp` and
`local/mission_side_storage_probe.cmd`; output is in
`local/mission_side_storage_probe.log`. The benign probe links with
`/MANIFEST:EMBED`; it includes `src/game_hosts_mission.cpp` to use the exact
private adapter and links existing game objects except `game_main.obj` and
`game_hosts_mission.obj`. It adds no production diagnostics or test target.

A separate full `bsp_game.exe --frames 90 --press-start-frame 5 --menu-select USN02
--game-root <install>` attempt failed with 0xC0000005 after phase 5 settings loading,
before the mission tree or menu path was reached. Its log reported zero package
entries/mounts, though the installation contains loose assets. This packet does
not establish that crash's cause. The fixture/installed-table success is therefore
distinct from full GameMissionHost startup, rendering or frame validation.

This change remains a semantic C++ interface, not a drop-in154h/434h ABI layout.
Existing reader callback order and coalesced repeated presence checks are not
new claims of arbitrary side-effecting Lua/metatable parity. The picture resource
producer is outside this packet. Installed-table evidence demonstrates the
reconstructed Lua adapter/reader path; it does not validate the original game's
binary ABI or gameplay, and no original game installation files are modified.
