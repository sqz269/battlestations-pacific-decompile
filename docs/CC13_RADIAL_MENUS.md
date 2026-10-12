# CC13 support-manager radial menus (cc13-radial1)

The seven support-manager radial builders left for cc9 in docs/CC13_BAND_0060_0066.md, plus the
helpers they need. Packet approved by cc9 as reconstruct-only. Code: `include/bsp/radial_menu_cc13.hpp`,
`src/radial_menu_cc13.cpp` (namespace `bsp::radial_cc13`). Call rows:
`reports/cc13_radial_menus_calls.json` (360 direct rows, `tools/verify_report_calls.py`: 0 failed;
102 indirect rows with their vtable slot and receiver).

Status: reconstructed and build-tested. Not ABI-compatible (a C++ interface over a model of the
screen, `RadialScreen`, and a host for the state reads, `RadialHost`), not fixture-tested, not
game-validated. Names are hypotheses, not recovered symbols.

## Orders verdict

**None of the seven builders issues an order or command.** No call reaches the command or order
issue path, no message is sent, and no unit or game state is written. For cc9's harness order
emulation this means the builders can be skipped or emulated as pure presentation; whatever order a
radial selection leads to is issued by the caller (00675C40 / 0066D020), not here.

Evidence, for every function in the packet:

- Direct calls (report `host_steps`): only GUI widget helpers (00AA7E00 find child, 00AAB4C0 clone,
  00AA6750 / 00AA8240 resolved position, 00AB64C0 icon state add), the string temporaries for the
  child names (0041E870, 0041DD40, 00BF7680, 00419CC0, 00BD1510), the checked-iterator failure
  00BF6713, the list helpers of the screen's own loadout-icon list (00665150, 00659BE0, 00662FB0,
  00659070, 00BF65AC), the sibling builders and 0065E870 / 0065ABB0, and three reads/effects listed
  below.
- Indirect calls (report `indirect_calls`): widget vslots +4h (scalar deleting destructor, on the
  screen's own widgets), +34h (visible), +44h (rotation; docs/SHIP_SCREEN_UPDATE.md,
  include/bsp/hud_minimap.hpp), +4Ch (alpha), +88h (icon state); the class record's vslot +18h (flag
  query, include/bsp/ingame_gui.hpp); and the unit's vslot +5Ch (IsKindOf), in 00668D40 only.
- Memory writes (census of every non-stack destination in the listings): only screen fields
  (`+5Ch`, `+B8h`, `+C8h`, `+E8h`, the shifted arrays `+ACh/+BCh/+DCh`, `+2B8h`, `+2BCh`, `+2C0h`,
  `+2C1h`, `+2C4h`, `+2D4h..+2E0h`, `+324h`, `+438h..+444h`) and the nodes of the screen's list
  +15Ch.

Effects outside the menu objects, none of them an order:

| Call | Where | What it does |
|---|---|---|
| `0065F170` `BSP_SupportManager_PlayUnavailableSound` | 00662020, 00662210 | Plays the sample at screen+10h (environment/environment_unavailable.fsb) when the loadout list is empty. Audio only. |
| `00443490` `BSP_DeviceClass_ResolveAndActivate` | 00665F30, 00666DC0 | Resolves the loadout's device class (DL = 0) for its small icon; per its ledger record it calls the class's vslots +10h/+14h when the class is not yet active and writes no flag. Resource activation, no unit state. |
| `009552E0` | 00665F30, 00666DC0 | Reads the class record's loadout entry (null past the count). |
| `00951F10` | 00662020, 00662210, 00665F30, 00666DC0 | Reads record+128h, the loadout count. |

Cursor note for the harness: 006605C0 writes the screen's selection state (+2C4h selection in list
+3E0h, +324h slot, +5Ch page set to 1 when list +3E0h is empty) when a large slot's entity
disappears. That changes what a later confirm in 00675C40 would act on, so an emulation that tracks
the cursor must apply the same adjustment (table below).

## Per-builder table

Screen = the support-manager screen (ECX). Kinds and flags are as the native tests them.

| Address | Name | Builds | Availability rule (state read) | Orders issued |
|---|---|---|---|---|
| 006605C0 | `BSP_HudSupportManager_PruneLargeSlots` | Removes large slots and clones a fresh slot 3 from +138h: `icon_L_Icon` state 3, `circle_small_01/02_Group` hidden, `circle_HL_Icon` hidden, rotations `5*step -/+ base`, alpha 0; hides +FCh | Runs only when list +344h is nonempty (+34Ch). A slot id (+438h..+444h) > 0 is stale when no node of list +3ACh has an object with u16 id +174h equal to it; one removal per pass (+2DCh); a stale id met while a removal is pending is dropped without touching the widgets | No. Writes the cursor: see below |
| 00661B50 | `BSP_HudSupportManager_StepSlotRingScroll` | Per-frame scroll of the large slot ring: +2BCh -= 4*dt; leaving pair +148h/+14Ch, groups +ACh rotated by `base + off`, icons +BCh by `-base - off`; fades by 0065E870 | The entry under the cursor (list +3E0h node at +2C4h) that is not a live unit (byte +14h clear, i.e. a class record) has its `icon_L_Icon` dimmed to 0.6 once `1 - t > 0.6` | No |
| 00662020 | `BSP_HudSupportManager_HighlightLoadout` | Shows `circle_HL_Icon` on the loadout icon at +334h, hides the rest | Empty loadout list -> unavailable sound. Catalog record +3A4h flag 10h: icon u is loadout u+1 (match `+334h - 1`, stop at record+128h); else icon u is entry u (0 = default) | No (sound only) |
| 00662210 | `BSP_HudSupportManager_HighlightHangarLoadout` | Same as 00662020 | Hangar record +2A0h; loop 0..record+128h inclusive, returns at once when the count is negative | No (sound only) |
| 00665F30 | `BSP_HudSupportManager_BuildLoadoutRing` | Clears the loadout icons, then one small icon per loadout (state 1 with the loadout's small texture/UV), plus a default entry (icon state 0, no loadout) when flag 10h is clear; first entry from template +108h[q], the rest from +118h[q] placed from +108h[q]'s resolved position; alpha 0 on the three children; highlight at +334h | Catalog record +3A4h (page 2 catalog). Empty unless the record has flag 0Fh **and** record+128h != 0. Flag 10h removes the default entry. Quadrant q = +314h; other values skip the placement | No |
| 00666DC0 | `BSP_HudSupportManager_BuildHangarLoadoutRing` | As 00665F30 | Hangar record +2A0h (00668D40 picks it when the page-2 unit is kind 9 or 45h) | No |
| 00668D40 | `BSP_HudSupportManager_StepCatalogRingScroll` | Per-frame scroll of the catalog ring: +2B8h -= 4*dt; leaving pair +13Ch/+140h, groups +7Ch rotated by `base - off`, +8Ch/+108h/+118h by `off - base`; at 0 rebuilds the loadout ring | On page 2 (+5Ch == 2) at the end of the scroll: 00666DC0 when list +1ECh is nonempty (+1F4h) and the unit at *(+304h)+8 answers IsKindOf(9) or IsKindOf(45h); otherwise 00665F30 | No |

Helpers: `0065E870` `BSP_HudRadialIcon_SetLargeIconAlpha` (already named by cc13-sweep60;
reconstructed here) sets one alpha on `icon_L_Icon`, `circle_HL_Icon` and `circle_L_Icon`;
`0065ABB0` `BSP_HudSupportManager_ClearLoadoutIcons` destroys every icon of list +15Ch and clears it.

### Cursor adjustment in 006605C0 (page 0 only)

After removing slot s (n = size of list +3E0h, read at +3E8h):

- n == 0: +5Ch = 1, +2C4h = 0, +324h = 0.
- s == +324h: if +324h != 0 then +324h -= 1 and +2C4h steps back (wrapping to n-1); else +2C4h
  steps forward (wrapping to 0). The highlight widget at +DCh[+324h] is then shown at alpha 0.
- s != +324h: +324h -= 1 when s < +324h (signed); +2C4h steps back (wrapping to n-1).

## Call interface (callers are cc9's, not reconstructed)

- 00668D40 and 00661B50: `__thiscall(screen, float dt)`, RET 4. 00675C40 calls 00668D40 at
  0067607A and 00661B50 at 00676088 with its own first argument `[EBP+8]` (the frame dt), and again
  00661B50 at 00676213 after clearing +154h, followed by 006605C0 at 00676228.
- 006605C0, 00662020, 00662210, 00665F30, 00666DC0: `__thiscall(screen)`, RET, no arguments.
  Call sites: 00662020 at 0066D358, 00677E51, 0067A045; 00662210 at 0066D17F, 0067815A, 00679235,
  00679FB0; 00665F30 at 0066C43B (0066B6C0), 0066D31F, 00677E4A, 00678ECF, 00669159; 00666DC0 at
  0066D178, 00678153, 0067922E, 0066914D.

## Evidence notes

- 00665F30 and 00666DC0 were compared with a diff of the listings with addresses, stack offsets and
  registers masked: the differences are the record field (+3A4h vs +2A0h), the frame size, register
  allocation, one redundant flag-10h branch in 00666DC0 that compares the same index two ways, and
  one repeated `circle_HL_Icon` visibility call in 00665F30 (00666683..006666CB). Both are modelled
  by one builder with a `repeat_highlight` switch.
- In the loop of 00665F30 the loadout id is the loop index i (stack slot read at 00666764 and
  0066679A), while the line offset uses j - 1 with j = i - 1 under flag 10h.
- x87, checked against capstone: Ghidra prints `FMUL ST1` for both `D8 C9` and `DC C9`. At
  00668D6C the instruction is `fmul st(1), st(0)`, so the step is `t - dt*4.0` and the value left on
  the stack is the constant 4.0, which the reverse arm consumes (`(1-t)*step + 4*step`), not 4*dt.
  At 006669F4 the instruction is `fmul st(1)` = `ST0 *= ST1`. The pseudocode of 00668D40 and 00661B50
  is wrong on these arms; the reconstruction follows the listings.
- Constants (from the PE): 00D7A328 = 4.0, 00D7A370 = 5.0, 00D7A210 = 1.0 (doubles), 00CEFF98 =
  0.6000000238418579 (double), 00CE3D30 = 0.6f, 00D7A24C = 1.0f, 00D7A218 = 0.0f, 00CF69A0 =
  0.04500000178813934, 00CED088 = 0.01899999938905239 (doubles).
- Widget vslot +44h is the rotation setter (docs/SHIP_SCREEN_UPDATE.md, include/bsp/hud_minimap.hpp):
  each slot group rotates around the ring and its icon child counter-rotates, which is why these are
  radial menus.
- 00662210 walks the list without a size bound; past the end the native reaches the checked-iterator
  failure 00BF6713. The reconstruction stops there instead.
- Ghidra: the eight names applied with `tools/ghidra_annotate.py --apply` under the lock; prior values
  in the tree's `local/ghidra-annotations-20261012T134905Z.json` (all `FUN_`). No function was
  created; `_free` 00BF9DC8 keeps `void __cdecl _free(void * _Memory)` before and after.

## Open questions

- What lists +344h, +3ACh, +3E0h and +1ECh hold beyond what docs/CC13_INGAME_GUI.md records (page 0
  list +3E0h, page 1 list +344h, page 2 unit list +1ECh); the names of the rings ("slot" for
  +ACh, "catalog" for +7Ch) follow which builder they feed and are provisional.
- The meaning of +2DDh and +2E0h (set with +2DCh by 006605C0; read elsewhere in 00675C40).
