# Front-end segments 12 and 17, reconstruction pass (packet cc13-fe3)

Addresses: segment 12 (004F9D80-00519FE0) and segment 17 (00554680-00568930).

Segment 12 holds:
- the tactical library (GIAchievements): entries, the carousel, the DLC list, the loading queue and its
  worker thread;
- the front-end frame;
- in-game screen 23;
- the HUD pilot and bomber screens.

Segment 17 holds:
- the PC control binding screen and the input-settings tree templates;
- the leaderboards screen;
- the HUD limbo screen.

Code: `include/bsp/frontend_seg12_17_cc13.hpp` and `src/frontend_seg12_17_cc13.cpp`, in namespace
`bsp::cc13_fe3`. The source is registered in `cmake/startup.cmake`.

Records: `config/reconstruction/004f0000.jsonl` .. `00560000.jsonl`. Each record's evidence names this
document.

Call-site report: `reports/cc13_fe3_calls.json`.

Generators, kept in the worker tree's `local/` (not committed):
- `fe3_rec.py` and `fe3_extra.py` build the records.
- `fe3_backfill.py` writes the backfilled records.
- `fe3_calls.py` builds the call-site report.
- `fe3_asm.py` prints a compact listing of one function's body.
- `fe3_dec.py` gives a cached compact decompile.
- `fe3_str.py` reads strings from the image.

## Status

| Level | What |
| --- | --- |
| Records added | 264 in total: 260 reconstructed and build-tested, plus 4 backfills |
| Reconstructed, build-tested | Every record whose source is `src/frontend_seg12_17_cc13.cpp`. `./scripts/build.ps1` passes (2/2 tests), also after the final `git merge origin/main` |
| Backfilled | Four existing reconstructions in `src/menu_screens_2.cpp` (packet cc13-menus5) had no record: 00511F40 tactical library Enter, 005162B0 Update, 00516010 StepCarousel and 00568490 HasRequiredContent. Their RET forms were checked against the disk bytes; 00511F40's is out of reach of the 1200-byte sweep |
| Call sites | 566 rows over 259 routines. `tools/verify_report_calls.py` passes them all (0 failed) |
| Fixture-tested / ABI-compatible / game-validated | None. These are new C++ interfaces over raw image storage, with no native adapter. Nothing is wired into `bsp_game`, and nothing was run |
| Ghidra | Not modified (no renames, no comments) |

When the packet started, the index listed 348 functions in the two segments that had a ledger name
but no record and no tag. 85 of them remain (listed in the handoff).

## Method

- Every body was read from the disk listing through `local/fe3_asm.py`. It sweeps `bsp.py disasm-raw`
  up to the next index function or the INT3 padding, and prints each call target with its ledger name.
  Register arguments, stack cleanup and x87 order were taken from the listing, not from the decompile.
- String operands were read from the image: "VehicleClass", "HeavyCruiser", "Type", "Name",
  "UnlockDLCMinimum", "0", "FE.unitlib_nounlock", "GIAchievements::StopLoadingThread()", "DLC_%2d",
  "title_Text", "szarnyas_FrameBox", "GUI_bomber", "GUI_limbo", "FE_controls_PC_listbox",
  "FE_controls_PC" and "invalid vector<T> subscript".
- The model follows cc13-fe1 and cc13-fe2:
  - Objects are `Raw` storage, as defined in `ingame_gui_cc13.hpp`.
  - Vtable stores keep their image addresses.
  - Engine calls go through host interfaces whose methods are named after the image routine or global.
  - The trivial and collect-children slots reuse the `TrivialSlot` and `CollectChildrenSlot` rows of
    cc13-fe2.
- Templates that share one shape across many instances are table-driven. Each table's records use the
  scope `shared_shape_model`. The tables:
  - `kStringVectorDestroy` (19 rows)
  - `kTreeClear`, `kTreeConstruct`, `kTreeDestroySubtree`, `kTreeBuyNode`, `kTreeCopySubtree`
  - `kTreeLowerBound`
  - `kStringTailCopy`, `kStringDwordTailCopy`
  - `kIteratorPreIncrement`
  - `kInputDeviceRecordMembers`
- Checked-index calls to 00BF6713 (`_invalid_parameter_noinfo`) that cannot fire on an in-range index
  are not modelled. The two library carousel steps share the tail of 004FC720.

## Findings

1. **00560FF0 has a dead branch.** This leaderboards listener's second widget test (+124h with +0F8h)
   branches to the same instruction whether it passes or fails, so only the +0A0h/+70h test decides
   the styling.
2. **00564F60 discards a search.** In mode 2 it searches the results for the player's XUID and never
   uses the answer.
3. **004FC9D0 reads an id it never uses.** It reads the selected list item's id and bounds-checks it
   against the +470h records, but always uses the first record.
4. **005635F0 passes a register leftover.** Its last argument to 004BC6D0 is EDX with only the low
   byte set to `mode == 1`. In mode 2 the upper bytes come from the XUID's high dword; otherwise they
   are whatever EDX held.
5. **004FAF40 is a listener at screen +68h, not the screen.** Its scroller (+64h, which is screen
   +0CCh) and its widget arrays (+170h and +210h, which are screen +1D8h and +278h) match 0050B2D0
   (library Exit) only when ECX is taken as screen +68h. 00509D70 confirms this: it uses the same
   listener and calls the scroller recapture at ECX +64h.
6. **The vehicle class registry is written from the library.** 00506510 and 00506550 rewrite the
   registry's two 800h index maps (+10h and +2010h). See "For cc9".
7. **The loading queue and the atlas singleton share a global.** 00E18D4C is cleared both by
   00509FD0 (loading queue destroy) and by 004FB190 (an atlas deleting destructor).

## For cc9

These change gameplay state or class data, so they were left out (00506510 is only called, through a
host method, from 00514FB0):

- 00506510 BSP_TacticalLibraryScreen_ResetVehicleClassOrder: resets both vehicle class index maps
  (registry +10h and +2010h) to the identity.
- 00506550 BSP_TacticalLibraryScreen_ShowCurrentEntryDetail: does the same and then maps one class.
- 00519020 BSP_HudPilotScreen_PulseAttachedKind25: calls vtable +21Ch(2Ah/2Bh) on each object attached
  to the HUD unit whose kind is 25h.
- 00519B00 BSP_HudBomberView_BindUnit: sends unit role transfers (0077C470) and rebinds the weapon
  group screen.
- 00565D80 BSP_HudLimboScreen_PollRespawnInput: input actions E1h and 8Ch..8Fh force a unit selection
  (006485A0). 00565E00 (limbo Update) reaches it only through the host method
  `hud_limbo_poll_respawn_input_00565d80`.

## Uncertainty

- x87 arithmetic is modelled in double:
  - 004FA230 (the cross product);
  - 004FC8C0 (`float(i) + (double)0.004f`);
  - 00517B00 and 00517BA0 (frame placement with 960.0, 0.003 and 0.006).

  The C++ follows the original's float stores, such as `float(width / 960)`. The last bit can differ
  by double rounding.
- Field names in the comments are hypotheses taken from the call sites. Examples: "cancelled" for job
  +4h, "busy" for worker vtable +10h, and the leaderboard list box roles.
- `TreeDestroySubtree` keeps the value teardown that the listing shows. The N20 node with a nested
  tree (0055AF60) has its own function.

## Handoff

Commits on agent/cc13-fe3:
- a5732d4f3: batch 1.
- 15f3d6a0c: batch 2.
- The batch 3 commit, then the merge of origin/main.

85 targets remain. They are mostly large screen bodies, plus a few templates:

- **Tactical library (segment 12):**
  - pages and lists: 004FF6F0, 004FFD00, 004FFF20, 005001A0..005012F0;
  - preview: 00502480, 00503510 and 0050AA10;
  - entry views: 0050E3C0 and 0050EF80;
  - detail: 00510C20 and 00511BF0;
  - tutorial movie: 00515050;
  - order of battle: 00506660;
  - large helpers: 004FE870, 005072C0 and 0050C760..0050CE70;
  - 0050D900 (ClearList, with a jump table);
  - 00506C80 (string vector assign);
  - 004F9D80, 004FD500, 004FE310 (entry assign), 00501A50, 00503250, 00504130, 00509390, 00509DE0,
    0050A400, 0050B8E0, 0050BF00, 0050C300 and 0050DA40.
- **Frame and HUD (segment 12):** 00517300, 005178F0, 00517ED0, 005198F0, and 00518F30 (cc9's
  game_hosts_hud implements a host method for it).
- **Control binding (segment 17):** 00555DC0, 0055BF00..0055E3A0, 0055F710, 0055FA80, 0055FCA0,
  00560210 and 00560BF0. 0055C3B0, 0055C5A0, 0055CEC0, 0055E3A0 and 00560210 are host methods in
  `frontend_menus.hpp`; `frontend_menus.cpp` calls them.
- **Leaderboards (segment 17):** 00561320, 00561860, 00561BE0, 00561EB0, 00562750, 005630F0
  (InsertN), 005638F0, 00564220, 00565190 and 00565540.
- **Limbo and session browser (segment 17):** 00566080 (limbo Enter), 005661C0, 00568290 and 005684D0.
- **Templates:** 00559E00 (uninitialised copy with SEH), 0055A000 (tree insert), 0055A520,
  0055B520.
- **Skipped:** 00554680 (x87 bounds outline).
- **For cc9:** the five functions in "For cc9".

cc13-fe6 (docs/CC13_FE_6.md) took 00566080 (limbo Enter) and 00561860 (leaderboards
MoveSelection) from this list; 83 remain.

cc13-fe8 (docs/CC13_FE_8.md) reconstructed 40 routines in these two segments, including the leaderboards bodies,
the control-binding selection/conflict/event routines, BindWidgets, the tactical library category
lists, pages, filter and order-of-battle side selection, the frame Register and the in-game camera
binders. The remaining open targets are listed in the cc13-fe8 handoff.
