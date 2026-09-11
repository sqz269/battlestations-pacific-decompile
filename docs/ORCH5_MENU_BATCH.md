# Orch5 menu reconstruction batch

Addresses: 004215D0 005495A0 005495E0 0054A0C0 0054B530 005803E0 00588C70 00599340 005C2F70 005922F0

The independent orchestrator worktree is
`J:/PROG/battlestations-pacific-decompile-orch5-20260911`, branch
`agent/orch5-20260911`. The main checkout remains the other orchestrators'
integration workspace. This batch followed `ORCHESTRATOR_PROMPT.md` and
`COORDINATION.md`, using three workers with separate worktrees and leases.

## Reviewed worker packets

| Packet | Worker commit | Result |
| --- | --- | --- |
| Command bar | `eecb8e4d` | Four routines: five command/label/placement triples, slot selection, two widget-family resets, independent help-line setter. See `MAIN_MENU_COMMAND_BAR.md`. |
| Map geometry | `f0b92abe` | Map-point/flag update and rounded float3 interpolation; vec3 append remains a library contract. See `MAIN_MENU_MAP_POINT_GEOMETRY.md`. |
| Mission flags | `12625142` | Completion adapter and selected-mission list return; existing side-index implementation retained. See `MISSION_MAP_FLAG_POLICY.md`. |

Eight routines have bounded C++ reconstructions; one additional library routine
has an analyzed contract and reviewed name. They expose new C++ interfaces,
not replacement native object layouts or ABIs. Actual GUI owners and host
bindings remain external. No new tests or fake service implementations were added.

The primary reviewed the implementations, independently checked the completion
adapter, list-return branches, GUI event call ownership, zero-command caller,
zoom gate and interpolation listing. The command-bar worker also reviewed the
map callback order and float-store sequence without finding a concrete defect.

## Corrections and coordination tooling

`MissionBriefingPlayHost::clear_help_line` is now `clear_command_bar` because
`0059244C` passes five zero commands to `0054B530`; it does not invoke the
independent help setter. Corrections were appended to the earlier mission-detail,
screen-update and briefing docs. Those corrections also distinguish the bounded
`00599340` list-return handler from `005993A0`'s calls to the detail builder.

`tools/integrate_workers.py` now resolves main through `workspace.main_root()`
when launched from this worktree. Its default trailer file belongs to the
invoked script's checkout; `BSP_COMMIT_TRAILER_FILE` can override it. Existing
main-checkout invocation retains its original default. Read-only startup checks
covered main resolution, local attribution, explicit override and legacy path;
an independent worker review found no missed host implementation or path defect.

## Verification and saved analysis

The baseline and combined batch passed `scripts/build.ps1`: MSVC Win32 Release,
warnings as errors, `reconstructed_math` and `native_math_differential` both pass.
Eight seed ranges matched the installed binary before enabling the native math
checks. The combined log explicitly includes all three new `.cpp` files.
These existing tests do not establish differential or gameplay equivalence for
the new menu routines. No menu fixture or visual/game test was performed.

Nine reviewed names and evidence comments were applied to the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, through the annotation
tool's Ghidra write lock. Prior annotations are preserved in
`reports/orch5_menu_annotations.json`; the project was saved and all nine affected
exports were refreshed. No new Ghidra functions or flow repairs were required.

## Normal registration and main integration

After the shared registry lease cleared, all three sources were registered in
`cmake/startup.cmake`: `main_menu_command_bar.cpp`,
`main_menu_map_point_geometry.cpp` and `mission_map_flag_policy.cpp`.
The temporary `CMAKE_PROJECT_INCLUDE_BEFORE` cache option was removed.
`scripts/build.ps1` then passed with normal registration and both existing tests;
the project file contains all three compiled sources. The initial local-hook
build remains earlier evidence, superseded by this standard build.

The batch is ready for coordinated main integration through
`tools/integrate_workers.py` with `BSP_AGENT=agent/orch5-20260911`,
`BSP_INTEGRATE=orch5-20260911` and the worker branches
`agent/orch5-command-bar`, `agent/orch5-map-points`, `agent/orch5-mission-flag`.
The integration helper reconciles current main and rebuilds before forwarding it.
Worker worktrees are retired only after main integration succeeds. The next
owner/layout/runtime packets use separate worktrees and leases.

## Verification follow-up from the current main checklist

The first reports used routine-specific call maps, so the newly introduced
`tools/verify_report_calls.py` initially discovered zero compatible rows.
They now carry explicit `address`, `native` and containing `function` rows:
16 direct command-bar calls, 8 direct map calls and 9 mission-policy calls.
All 33 direct sites passed current live verification. Eighteen indirect
dispatch rows are retained explicitly; their targets are not established by
the mechanical direct-call check.

The verifier now confirms the project/program before querying, checks the
callee's current function boundary, and checks the exact CALL instruction
even when a caller/callee edge is already present in the index. A focused
live check accepts `005C2F7F -> 0090C560` and rejects `005C2F78 -> 0090C560`
(a MOV in the same caller). The old graph-edge shortcut accepted that wrong
instruction. Per-function listing caching keeps the stricter check bounded.
