# HUD unit rows, plane bow waves and ship rudder review

Addresses: 00644DB0, 00644CC0, 00644C20, 00648C20, 00647080, 007D1D30,
007CD2F0, 007D1B50, 007D3E60, 007D5890, 0082ECB0, 0082E890, 00811890,
00811940, 00811960, 00811AB0

## Scope and ownership

This batch follows the proposed packets in `HUD_CENTRAL_UPDATES.md`,
`PLANE_CLASS_FIELDS.md` and `UNIT_FORCE_COMMANDS.md`. The second orchestrator
uses `agent/orch2-20260910`, with separate `orch2-` worktrees and address/file
leases for all three workers. The render work on `main` remains owned by the
other orchestrator. Workers read Ghidra; the reviewing orchestrator applies
evidence annotations and analysis repairs under the shared write lock.

The starting checkout was `e0ad746`. Live commands verified project `bsp`,
`C:/Users/sqz269/bsp.gpr`, and program `/battlestationspacific.exe`. All 13 seed
functions existed and had no missing CALL fallthrough in the initial audit.
All eight native math seeds matched the installed executable. The initial
MSVC Win32 Release build passed both existing CTests.

## Plane-model flow repair

Following the bow-wave records into model finalization exposed 17 missing
three-byte instructions in `007D3E60`, each after a call to CRT free helper
`00BF65AC`. The worker temporarily released that address; the orchestrator
claimed it and used `tools/ghidra_flow_repair.py --apply` under the write lock.
Each gap's live bytes matched the installed executable before disassembly.

The repair restored all 17 instructions and saved Ghidra, then refreshed the
shared `007D3E60` export. A second flow audit found zero CALL gaps; the two
gaps following jumps were left alone. For example, `007D4800: ADD ESP,0x4`
now follows the free call at `007D47FB`. The saved function has 1,632 listed
instructions; the decompiler output changed from 820 to 803 lines.

`reports/plane_bow_wave_flow_repair.json` records the exact sites, disk-byte
hashes, mutation responses and post-repair result. This changes saved analysis,
not executable bytes or the installed game.

## Review notes

- The current `00644DB0..0064505B` span is 684 bytes. It handles input selection;
  the prior HUD doc's 1,195-byte description is corrected by an appended section.
- The payload widget call at `00649803` pushes a computed EAX state. The native
  load at `006497F2` and push at `00649802` contradict a constant-3 decompiler
  argument. The worker's reconstruction must preserve the computed value.
- The bow-wave post-model fragment copies the first marker Vec3 to the three
  floats at each descriptor record's `+4h`, `+8h`, `+Ch` (stores at `007D492E`,
  `007D4938`, `007D4946`). Runtime effect construction is a separate boundary.
- The rudder curve `0082ECB0` spills its normalized speed and divided base rate
  to floats before the final x87 multiply chain. Native operation order and
  float spill locations are evidence; a portable C++ translation alone does
  not prove bitwise x87 equivalence.

## Integration and validation

Worker commits, final integration, annotation readback and the combined build
are recorded in `reports/hud_plane_rudder_integration.json` when this batch
finishes. Reconstruction uses explicit C++ interfaces and external host
contracts; it is not a drop-in native ABI replacement or game validation.
