# Orch5 command owner, menu layout and map runtime

Addresses: 00549570 00549580 00549620 00549FC0 0054A8D0 0054A960 0054A970 0054A990 0054B500 0054C050 00541B80 00543710 005861B0 00AB1EF0 00AB2820

This batch uses `agent/orch5-20260911` as the integrator worktree and three
independent worker worktrees. Main remains the shared integration checkout.

| Packet | Worker commit | Result |
| --- | --- | --- |
| Command owner | `3726bc45` | Nine bounded routines for screen 58h, its two pages, 22 widget bindings, enter/help variant, collector and lifetime; constructor analyzed separately. |
| Menu layout | `593319fd` | Complete normal-path binder and two embedded helpers: five pages, 55 direct-child lookups, both scrollers and list/text setup. |
| Map runtime | `0376bb23`, `febd2f77` | Actual retained-owner adapter for existing map geometry, plus concrete Icon size58 and scale48 dispatch. |

The binder identifies `+338h` as `selector_Icon`, preserves the two different
values assigned to `+568h`, and calls the actual Icon scale runtime. The map
adapter borrows the live mission selection, point lists and widget pointer
slots. It resolves the same GUI owners that own those widgets and rejects
missing data. It does not produce the mission-detail point lists itself.

The primary inspected the implementations and native help-variant, text-style,
list-position arithmetic, Icon size and filter/scale call sequences. The normal
Win32 build (`scripts/build.ps1`, MSVC `/W4 /WX /fp:strict`) passed with all
three new source modules registered in `cmake/startup.cmake`. Both existing
tests passed. No persistent tests were added. See
`local/orch5_owner_layout_build.log`.

The report verifier accepted 159 numeric call rows with zero failures. Three
of those rows are indirect calls whose target requires separate vtable
evidence; 156 direct CALL sites were verified mechanically. Another 25
symbolic indirect rows remain explicitly outside that mechanical check.
See `local/orch5_owner_layout_call_verification.log`.

The map worker's focused probe passed using actual retained GUI owners:
matrix publication and identity, live slot/selection/list changes, missing
point rejection and the actual selected-widget null gate. The Icon
size/scale/filter/rebuild paths were compiled and inspected but not executed.
This is not menu, renderer, gameplay or native ABI validation.

Five previously undefined Ghidra leaves were created after disk/live byte
checks: `00549570`, `00549580`, `0054A960`, `0054A970`, `00AB2820`.
Definition events and prior annotations are preserved in
`reports/orch5_menu_owner_definitions.json`,
`reports/orch5_icon_scale_definition.json`,
`reports/orch5_menu_owner_annotations.json` and
`reports/orch5_menu_layout_map_annotations.json`. The existing project/program
was verified, mutations used the write lock, the project was saved, and all
15 affected exports were refreshed. Correct existing library names were kept.

Actual command/Text/Movie/Listbox owners, cloning/lifetime services and game
host composition remain required. In particular the canonical GUI factory
does not yet construct Text. The next workers are reconstructing ellipsis,
content assignment/glyph clearing and Text style against existing state and
resource contracts. These prerequisites prevent claiming a working menu.

Standard-build results are recorded in `reports/orch5_owner_layout_batch.json`.
The coordinated helper reconciled current main, rebuilt successfully, passed
both existing tests, and integrated the batch into main at `a4fbaed0`.
See `local/orch5_owner_layout_integration.log`. Git history and remote ancestry
establish publication.
