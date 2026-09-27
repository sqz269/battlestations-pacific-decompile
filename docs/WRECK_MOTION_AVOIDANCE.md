# How a wreck's motion reaches live ships' avoidance (packet `cc9_sink_pair_prekill_move_read`)

Addresses: 00824B60, 00825044, 0082505C, 00825074, 009EAECA, 009EB1FE, 009EB660, 009D84E0, 009EBF67, 009EBEDE

2026-09-27, worker cc9-units3, on main 2872fab7b. Ghidra was read only. The pointer from
docs/CONSTRUCT_WORLD.md section 25 follows when that file's lease is free.

The sink-descent pair of docs/CONSTRUCT_WORLD.md section 25 (`2e58c646c`, `SD_OFF_USN02` / `SD_ON_USN02` in worktree cc9-units2) moved
Kawakaze's first damage from 104.05 to 102.10 s, before any kill. Its cause was not isolated.

**It is not hull geometry against rounds.** The first divergence is ship AI reacting to how a
wreck moves.

## What differs between the builds

- **OFF, a wreck keeps cruising under its last throttle.** In neither build does the host cut a
  dead ship's engine: the wreck handler's pre-sink part `00824B60..00824FE4` is unbound. At
  52.50 s the controlled DeRuyter (dead at 30.25 s) runs at fwd 16.45 m/s, x = 119.2,
  throttle 1.000.
- **ON, the sink block damps it.** The block sets inertia × 2 (`00825044`), angular damping 2.5
  (`0082505C`) and linear damping 0.5 (`00825074`, `00C37E00`). The same wreck is at 2.86 m/s,
  x = 245.7. (`+828h`, zeroed by the same block, is `sinkTime`, not a throttle.)

## How a wreck's speed reaches live ships

Each live ship sees a neighbour through its obstacle node, and the node's boxes are projected by
the neighbour's own speed:
- the near box's half-length is 0.55 × Length + |advance|, with the lookahead advance from
  `009EAECA` (`src/ship_ai_neighbour_box.cpp`);
- the avoid box is advanced by the observed body-axis speed × the projection time (`009EB1FE`).

A cruising wreck therefore throws a long box ahead of itself. A damped one does not.

## The first differences, in order

1. **Step 1015 (50.75 s), the first host divergence.** The sector scan `009EB660` of Kortenaer
   finds DeRuyter's node blocking. The OFF box is 102.3 m: 0.55 × 171 plus 8.25 m of speed
   lookahead. The scan runs `009D84E0` at `009EBF67` and raises the node's lifetime
   (`009EBEDE`). A diagnostic run of the OFF build logs this block 60 times from step 1015 to
   step 1064, always Kortenaer against DeRuyter. In the logs of record, the natives
   `settings_neighbour_memory_194`, `raise_node_lifetime_78`, `passing_corner_009d84e0` and
   `settings_blocked_margin_1d8` first fire at mission frame 1014 OFF and at frame 7540 ON.
   Kortenaer's printed step lines stay identical through step 1060.
2. **Step 1060 (53.0 s), the first visible gameplay divergence.** Minegumo is slot 1 in a
   formation behind Yamakaze (slot 0): the same heading −0.6283 and the same fire target Alden.
   Yamakaze dies at 51.65 s. ON its sink block damps it; OFF it cruises on at throttle 0.5.
   - Minegumo's heading target first reads 4.4941 OFF against −1.7891 ON, exactly 2π apart.
   - At step 1070 the targets differ by 0.018 rad and the rudders by −0.138 against −0.284.
   - No other ship's step line differs before this (a keyed comparison of every
     `ship ai step` line).
3. From there the engagement drifts. Kawakaze's first damage moves 104.05 → 102.10 s and the
   hull hits 167 → 179.

## Evidence

- **The logs of record:** `SD_OFF_USN02.log` and `SD_ON_USN02.log` in worktree cc9-units2,
  read only.
- **The diagnostic:** `local\PK_OFF_USN02.log` in worktree cc9-units3, one run.
  - The binary is an export of `2e58c646c` (the pair's own OFF commit, `local\pk_off`) with a
    single diagnostic `notef` at the `009EBF67` site. That edit is not committed.
  - It ran 1200 mission frames at 0.05 s.
  - It reproduces the OFF log: the controlled-unit line at frame 1050 matches exactly.
  - The log shows the fit line, the immediate present interval, a module directory in
    worktree cc9-units3 and the final COM release.
- **Tools:** `local\cc9-units3-firstdiff.py` (keyed step-line comparison) and
  `local\cc9-units3-linediff.py` (a masked sequence diff), both in worktree cc9-units3.

## Consequence

The OFF side of any pair where ships die carries wrecks that cruise under their last throttle.
The engine cut belongs to the handler's pre-sink part `00824B60..00824FE4`, which is the next
thing to read. Until it is bound, a wreck's speed on the OFF side of a pair is a host artefact.
