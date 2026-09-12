# The two corridor widths a path plan is latched to

Addresses: 009ED3E0 00778890 0070D400 0070D5D0 009D9DE0 0070D290 0070D080 0070D070 0070D030 0070D0C0 009EE61E

Packet `cc_ai_corridor`. Every descriptive name below is a hypothesis, not a recovered symbol.
Ghidra was read-only for this packet: no renames, comments, prototypes or function creation, and
no saves. The reconstruction is `include/bsp/ship_ai_path_corridor.hpp` and
`src/ship_ai_path_corridor.cpp`; the report is `reports/ship_ai_path_corridor.json`.

`009ED3E0 BSP_ShipAi_RefreshPathPlan` is one pass of the ship AI's plan refresh, and it is three
pieces with three owners:

| range | what it does | owner |
| --- | --- | --- |
| `009ED3E0-009ED498` | build the two corridor widths from the unit's group | this packet |
| `009ED49A-009ED4E3` | hand both widths to `009D9DE0` on each plan block and OR both answers into `nav+2FCh` | `cc_exe_2q`, `include/bsp/ship_ai_path_refresh.hpp` |
| `009ED4E4-009ED69E` | seed, revalidate, search and swap the two plans | `cc_exe_2q`, same file |

The brief for this packet gives the head as `009ED3E0..009ED4E2`. The store that ends it,
`009ED4DE MOV byte ptr [ESI+2FCh],BL`, is six bytes and ends at `009ED4E3`; `009ED4E4` is the
`JNZ` that opens the arm. The range is therefore `009ED3E0..009ED4E3`, and the part of it this
packet had to project is `009ED3E0..009ED498`, because `cc_exe_2q`'s arm function already
projects the two `009D9DE0` calls and the flag fold.

## 1. The two widths, end to end

```
width_a = width_b = 20.0f                                  ; 009ED3E3, 009ED3F5, 009ED3FB, 00CE3930
if (00778890([nav+3FCh])) {                                ; 009ED401, 009ED408 JZ 009ED490
    ctl = [[nav+3FCh]+284h]
    width_a = min(float(0070D400(ctl) + 20.0), 600.0f)     ; 009ED41A, 009ED41F, 009ED425, 009ED429..009ED449
    ctl = [[nav+3FCh]+284h]                                ; reloaded, 009ED449 / 009ED44F
    width_b = min(float(0070D5D0(ctl) + 20.0), 600.0f)     ; 009ED45B, 009ED460, 009ED466, 009ED46A..009ED48A
}
BL = [nav+2FCh]                                            ; 009ED49A
if (009D9DE0([nav+2F4h])(width_a, width_b)) BL = 1         ; 009ED4AE, 009ED4B7
if (009D9DE0([nav+2F8h])(width_a, width_b)) BL = 1         ; 009ED4D1, 009ED4DA
[nav+2FCh] = BL                                            ; 009ED4DE
```

The `20.0` added to each extent is the double at `00CE3D88`; the sum is rounded to float32 by the
`FSTP` at `009ED425` before anything reads it. The cap is the double `600.0` at `00D20198` with
the float `600.0f` at `00CE4BC4` as the replacement, and it is unreachable: `0070D400` already
clamps its own answer to `400`, so a width cannot pass `420`.

`009ED455` stores the `0070D400` width in the stack slot `009ED4A7` pushes **first** and
`009ED48A` stores the `0070D5D0` width in the slot `009ED4A3` pushes **second**. `009D9DE0` writes
its first argument to `plan+4h` and its second to `plan+8h` (`009D9E32`, `009D9E3D`), so the
`0070D400` side is `plan+4h` and the `0070D5D0` side is `plan+8h`.

The pair is **not** swapped when the ship's latch is astern. `009DEEF8` in
`docs/SHIP_AI_ARM_FINAL_STEP.md` swaps the same two extents into the free-bearing query on
`blk+35Ch == 2`; this head has no test on `blk+35Ch` at all.

### Where the widths are stored

On the plan block, not on the navigator, and on **both** blocks every pass: `009ED4AE` latches
`nav+2F4h`, the plan in use, and `009ED4D1` latches `nav+2F8h`, the plan being computed.
`docs/SHIP_AI_PATH_PLANNER.md` has the block at `nav+224h` and `nav+28Ch` with `+4h` and `+8h`
constructed to the same `20.0f` at `00CE3930` (`009D9CFB`, `009D9D00`).

The only reader of either slot found for this packet is `009EE61E`, in the path pick that runs
right after `009ED3E0`: it takes `max(30.0f, [nav+2F4h]+8h)` through `00415550` (the float `30.0f`
is at `00E0E304`, already `kShipAiPathPublishLowFloor` in `include/bsp/ship_ai_goal_vector.hpp`).
No reader of `plan+4h` turned up in anything read here.

## 2. Where an extent comes from

`entity+284h` is a **unit group**: `0070D060 BSP_UnitGroup_MemberAt` returns
`[group + 34h*index + 18h]`, so the group holds `34h`-byte member records at `+18h` with the count
at `+4F8h`, the leader entity at `+14h` (`0070D0C0` writes it) and a column index at `+500h`.

`0070D400` and `0070D5D0` are one reduction with one difference:

```
m = 1.0f                                                   ; 00D7A24C, 0070D403 / 0070D5D3
for (i = 0; i < [ctl+4F8h]; ++i) {
    v = [ctl + 18h + i*34h + 10h + [ctl+500h]*4]           ; 0070D450 / 0070D546, 0070D724
    if (0070D5D0) v = -0.0f - v                            ; 00D7A208, 0070D5DB, 0070D625 / 0070D727
    v = (v < 0) ? 0.0f : min(v, 1200.0f)                   ; 00CFD714, 0070D455 / 0070D45F
    if (m < v) m = v                                       ; 0070D475
}
return (m < 5.0f) ? 5.0f : (m > 400.0 ? 400.0f : m)        ; 00CE3850, 00CE3D90, 00CFD710
```

Both bodies are an unrolled four-slot loop (`0070D450-0070D52E` and `0070D622-0070D70C`) followed
by a remainder loop (`0070D546-0070D57F` and `0070D724-0070D760`) over the same slots in the same
order, so one loop is the whole reduction. Neither has a call: they are pure over the group.

`00778890` is three tests: `[entity+284h]` must be non-null and `[[entity+284h]+14h]` must be the
entity itself. `007788B0-007788C7` is its exact complement and belongs to
`docs/SHIP_AI_ARM_FINAL_STEP.md`.

Consequences worth stating as numbers:

| quantity | value |
| --- | --- |
| width with no group, or in a group this unit does not lead | `20.0f` |
| extent range, always | `[5, 400]` |
| width range while leading a group | `[25, 420]` |
| width difference that invalidates a plan | `> 25.0` (`00CE3880`) |

### What the column is

`0070D290` is the one routine read here that touches both float columns of a member record: for a
member that is not the leader it takes `[record + 10h + [ctl+500h]*4]` (`0070D2BD`), multiplies it
by one scale and rotates it into the leader's frame through the sine/cosine pair `00811150`
returns, and takes `[record + 20h + [ctl+500h]*4]` (`0070D2CC`) as the offset it multiplies by the
other scale and lays along the leader's axis. So the column `0070D400` and `0070D5D0` reduce is
the member's **across-axis formation offset** in the selected formation, and the second column is
its along-axis offset: `0070D400` answers the largest offset on the positive side and `0070D5D0`
the largest on the negative side as a positive number, which is why they are the formation's two
lateral extents.

`0070D290` is a consumer. Nothing read for this packet **writes** either column or the column
index at `+500h`, so by the producer rule the two meanings stay provisional, and which sign is
port is still open. `docs/SHIP_AI_ARM_FINAL_STEP.md` already carries that as the
`ship_ai_controller_formation_slots` follow-up; this packet narrows it but does not close it.

## 3. The only thing that can invalidate a plan mid-search

`009D9DE0 BSP_ShipAiPathPlan_CorridorWidthChanged`, already reconstructed by `cc_exe_2q` in
`include/bsp/ship_ai_path_refresh.hpp` and not redone here. It is a latch: a plan whose
`search_state` at `+1Ch` is still `0` or below skips the comparison (`009D9DE3`), otherwise it
answers "either `|plan+4h - width_a|` or `|plan+8h - width_b|` is above the double `25.0` at
`00CE3880`", and it stores both widths on the plan whatever it answered (`009D9E32`, `009D9E3D`).

Its answer can only **raise** `nav+2FCh`: `BL` is loaded from the byte at `009ED49A` and both arms
are `JZ` past a `MOV BL,1`. Raising the byte is what throws a running plan away, because the arm
at `009ED4E4` then takes the "a plan is being computed" branch rather than following the plan in
use. Nothing in the head can clear the byte; only `009ED5B6`, after a swap, does that.

So the executable-facing rule is:

- A fresh plan gets `20.0f` on both sides unless the ship leads a unit group, in which case it
  gets the group's two lateral extents, each clamped to `[5, 400]` and raised by `20`.
- A running plan is thrown away when a width moves by more than `25`, that is when the extent on
  one side moves by more than `25` units, and only once the plan's search has started.
- The three ways that can happen, all of them reachable in one pass: `00778890` flips, because the
  ship gains or loses its group or stops being its leader; the member count at `ctl+4F8h` changes,
  because the group gains or loses a member; or the column index at `ctl+500h` changes, because
  the formation changes shape. A leader flip alone only invalidates a plan when the extent on that
  side is above `25`, since the width moves from `20` to `extent + 20`.
- Nothing else in `009ED3E0-009ED498` can invalidate a plan. There is no class test, no damage
  test and no speed test in the head; the only entity field it reads is `+284h`.

`ship_ai_group_corridor_extent_change_invalidates` in the header states that rule over extents so
a caller does not have to re-derive the `+20` step. It is derived from two image rules, not a
routine in the image, and is labelled as such.

## 4. Host methods, in call order

| # | method | call site | native | containing function |
| --- | --- | --- | --- | --- |
| 1 | `unit_leads_group_00778890` | `009ED401` | `00778890` | `009ED3E0` |
| 2 | `group_extent_positive_0070d400` | `009ED41A` | `0070D400` | `009ED3E0` |
| 3 | `group_extent_negative_0070d5d0` | `009ED45B` | `0070D5D0` | `009ED3E0` |

`ECX` at step 1 is `[nav+3FCh]`, loaded at `009ED3EF`; at steps 2 and 3 it is `[[nav+3FCh]+284h]`,
reloaded separately at `009ED40E`/`009ED414` and `009ED449`/`009ED44F`. All three callees are
`__fastcall` with `RET 0` and no stack arguments, and both extents return in `ST0`.

A host that already models the group can answer steps 2 and 3 with the pure rules
`ship_ai_unit_group_extent_positive_0070d400` and `..._negative_0070d5d0` over a
`ShipAiUnitGroupView`; the three methods exist because the executable reaches the natives, not
because the rules are unknown.

`ship_ai_path_refresh_plan_009ed3e0` composes the head with `cc_exe_2q`'s
`ship_ai_path_refresh_arm_009ed4e4`, which is the whole of `009ED3E0`.

## Coverage

| routine | range | coverage |
| --- | --- | --- |
| `009ED3E0` head | `009ED3E0-009ED498` | complete, read from the listing |
| `009ED3E0` latch fold | `009ED49A-009ED4E3` | read complete; projected by `cc_exe_2q`, not re-projected here |
| `009ED3E0` arm | `009ED4E4-009ED69E` | not read for this packet beyond `009ED4E4`, `009ED5A6`, `009ED5B6`, `009ED5BD`, `009ED5C3` and the tail decompilation used for section 3; `cc_exe_2q` owns it |
| `00778890` | `00778890-007788A7` | complete, read from the listing |
| `0070D400` | `0070D400-0070D5C3` | complete: head, unrolled body, remainder loop and both tail clamps read from the listing |
| `0070D5D0` | `0070D5D0-0070D7A4` | complete: same, with the `-0.0f - v` at `0070D625`/`0070D727` |
| `009D9DE0` | `009D9DE0-009D9E4B` | read complete from the listing as a check on `cc_exe_2q`'s projection; not re-projected |
| `0070D290` | `0070D290-0070D3FE` | partial: only `0070D290-0070D2CC`, the member-record path that reads both columns. The leader path from `0070D362` and the tail after `0070D2F1` were not read |
| `0070D080`, `0070D070`, `0070D060`, `0070D030`, `0070D0C0` | each whole | complete, read from the decompiler; used only for the group layout |
| `0070E450` | `0070E450-0070E4B2` | partial: read only far enough to confirm the `+18h`/`+4F8h` member walk |
| `009EE61E` | one call site | partial: only the three instructions `009EE61E-009EE62B` and `00415550`, to name the reader of `plan+8h` |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| The ledger evidence on `009ED3E0` calls `nav+2FCh` "the ready byte" | it is the "a plan is being computed" byte: the head can only raise it (`009ED49A`, `009ED4B7`, `009ED4DA`, `009ED4DE`), `009ED528` and `009ED63B` raise it, and only `009ED5B6` clears it, immediately before the swap | `009ED4DE`, `009ED5B6`, and `docs/GAME_EXECUTABLE.md` milestone 2q, which already names it that way. Evidence appended to the existing name; the name itself is unchanged |
| `include/bsp/ship_ai_path_planner.hpp` calls `plan+4h` `field_04` and comments both `+4h` and `+8h` as the constructed `20.0f` | both are corridor widths, rewritten on every pass of `009ED3E0` by `009D9DE0`; for a ship leading a unit group they are the group's two lateral extents plus `20`, in `[25, 420]` | `009ED4AE`, `009ED4D1`, `009D9E32`, `009D9E3D`. The header belongs to another packet and was not edited |
| `include/bsp/ship_ai_path_refresh.hpp` states its coverage as `009ED4E4..009ED69E` and says the head is not projected | its `ship_ai_path_refresh_arm_009ed4e4` also projects `009ED49A-009ED4E3`, the two `009D9DE0` calls and the flag fold; only `009ED3E0-009ED498` was missing | `src/ship_ai_path_refresh.cpp` lines 57-67 against `009ED49A-009ED4E3`. The header belongs to another packet and was not edited |
| `docs/SHIP_AI_ARM_FINAL_STEP.md` leaves `0070D400`/`0070D5D0` partial, with the four-way unrolled tail and the remainder loop unprojected | both are read whole here; the tails are the same step as the head, and the answer is clamped to `[5, 400]` with a floor at `00CE3850` and a cap at `00CFD710` | `0070D546-0070D57F`, `0070D581-0070D5C3`, `0070D724-0070D7A4` |

Nothing in this packet contradicts `docs/SHIP_AI_ARM_FINAL_STEP.md`'s reading of the two extents;
it adds the tail clamps that doc could not read and identifies the column through `0070D290`.

## Run-time evidence

None, and none is available. `bsp_game.exe` reaches `009ED3E0`'s arm 3426 times in milestone 2q's
run, but it does not project this head: `src/game_hosts_ship_ai.cpp:2812` passes
`kShipAiPathCorridorWidthDefault` for both widths and records `009ED3E0` rather than completing it.
With a constant width `009D9DE0` answers false after the first pass, so the run says nothing about
the group arm. Wiring the head into the executable is the follow-up below; the game sources belong
to another packet and were not touched.

## no_ghidra_function

none. Every address read for this packet is inside a function Ghidra already has, and each call
site was checked with `bsp.py ghidra proto <site> --brief` through
`tools/verify_report_calls.py`.

## Follow-up packets

1. **`ship_ai_unit_group_slots`** - the producer of the member record. Who writes
   `record+10h + k*4` and `record+20h + k*4`, who sets `ctl+500h`, and how many columns exist. The
   record is `34h` bytes so at most four fit in each array, which is this packet's hypothesis from
   the size alone. Until it lands, "across-axis offset" rests on the consumer `0070D290`, and
   which of the two extents is port is undecided. This is
   `docs/SHIP_AI_ARM_FINAL_STEP.md`'s `ship_ai_controller_formation_slots`, narrowed.
2. **`game_ship_ai_corridor_widths`** - wire `ship_ai_path_corridor_widths_009ed3e0` into
   `src/game_hosts_ship_ai.cpp:2812` behind a group host, so the executable can show a plan being
   invalidated mid-search. It needs a unit group in the scene, which the current fixtures do not
   build. The game sources belong to another packet.
3. **`ship_ai_plan_width_consumers`** - who reads `plan+4h`. `009EE61E` reads `plan+8h` and
   publishes `max(30.0f, +8h)`; no reader of `+4h` turned up here, which would make the
   `0070D400` side write-only and is worth settling.
4. **`ship_ai_path_refresh_coverage`** - a one-line coverage correction in
   `include/bsp/ship_ai_path_refresh.hpp` for the range its arm function actually projects, and a
   rename of `ShipAiPathPlanBlock::field_04` to a corridor-width name in
   `include/bsp/ship_ai_path_planner.hpp`. Both headers belong to other packets.
