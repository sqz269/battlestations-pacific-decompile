# cc13 adversarial audit 1

Addresses: 00B76CE0 00B75720 00B75520 00B9AE00 00A8BD20 00B6FC30 0062C2C0 0062CA7B 00511F40
005B4772 005B4E3C 005B58DF 00B156C0 00795EE0 00AF8500 0068F030 005FD650 00B9D7B0 00A93690
00B75450

Reviewer: cc13-audit1 (Claude, 2026-10-11/12). Read-only review of reconstructions other cc13
workers landed. Nothing in `src/` or `include/` was changed by this packet. All the reconstructed
code reviewed here is build-tested only; this audit adds listing evidence, not run-time evidence.

Method: each routine was compared with the bytes on disk (`bsp.py disasm-raw`, raw opcode bytes
where the Capstone mnemonic for a DE-prefix x87 pop form is ambiguous), x87 blocks were replayed
with `tools/x87_sym.py` (note: its `--fpu` list is bottom-first, top is the last element),
constants were read from the PE at the load's own width (`local/au1_dw.py`, a dword/float/double
reader), jump tables were read from the image, and register values were established by filtering
the whole function listing for the register. Huge bodies were sampled by range, as listed.

Verdict classes: CONFIRMED-CORRECT, DEFECT (with fix), DOC-CLAIM-WRONG, UNSURE.

## Verdicts

| Routine | Source | Coverage of this review | Verdict |
|---|---|---|---|
| 00B76CE0 sample track | src/scene_graph_cc13.cpp:189 | complete: both out-of-range switches (tables 00B77170 / 00B77188 match the enum 0..5), wrap paths, linear extrapolation both ends, equality early-outs, s, interpolation table 00B771A0 (0-2 Hermite, 3 linear, 4 hold, 5 Bezier) | CONFIRMED-CORRECT (precision note 1) |
| 00B75720 Bezier segment | src/scene_graph_cc13.cpp:175 | complete, stack slots traced | CONFIRMED-CORRECT |
| 00B75520 Hermite basis | src/scene_graph_cc13.cpp:65 | complete (EAX = p1, ECX = 1 - p1, EDX = s^3 - s^2, stack = t0) | CONFIRMED-CORRECT |
| 00B75450 wrap time | src/scene_graph_cc13.cpp:49 | computing path; callee 00BF85B0 proven floor (note 3) | CONFIRMED-CORRECT |
| 00B9AE00 ocean projector | src/water_render.cpp:196 | sampled: steps 1 (fov * (double)1.2f [00CEC160]), 7 (lift both branches), 8 (nudge), constants 00CE3D78/00CE380C/00CEC400/00D7A358/00CED5D8/00D7A308/00D7A208/00CE6448/00CE3DC0 | CONFIRMED on sampled steps; DEFECT-minor D3 (NaN) |
| 00A8BD20 shadow cascade fit | src/shadow_cascades_cc13.cpp:330 | sampled: prologue (00B6FC30 = aspect * fov, 0.5 [00D7A280]), slot map of C (+228h), W (+128h, overwritten by the translated slice), P (+288h), T*P (+318h), light point through unshifted P (00A8C8A4), PPS loop reads C with T*P | CONFIRMED on sampled blocks |
| 0062C2C0 rewards enter, opening | src/ingame_gui.cpp:167 | ranges 0062C3A0..0062C560, 0062C548..0062C90B, 0062C8CB..0062C90B | CONFIRMED-CORRECT |
| 0062CA7B rewards page builders | src/ingame_gui.cpp:1030 | range 0062CA7B..0062CBE0 (page mode NEG/SBB/AND/ADD = 0 or 2) | CONFIRMED on sampled range |
| 00511F40 tactical library enter | src/menu_screens_2.cpp:1765 | 00511FAE..00512041, jump table 00514BD8 (kinds 0..8), 00512B60..00512C8D and 00512EAD (naval academy marking) | CONFIRMED on sampled ranges |
| 005B4772 map zoom step | src/map_screen_update_cc13.cpp:28 | complete (XMM0 = 0 from XORPS at 005B4722) | CONFIRMED-CORRECT |
| 005B4E3C map zoom trigger | src/map_screen_update_cc13.cpp:68 | trigger conditions 005B4E3C..005B4EB8 | CONFIRMED on sampled range |
| 005B58DF map zoom shader values | src/map_screen_update_cc13.cpp:120 | complete | DEFECT-minor D2 (NaN clamp) |
| 00B156C0 scene render | src/scene_renderer_cc13.cpp:410 | 00B15900..00B15D2E (kernel offsets, lens constants, clamps), constants 00CEC9E0 (-0.5), 00CE3CB8 (20000.0) | CONFIRMED on sampled range |
| 00795EE0 game camera evaluate | src/cc13_camera_controllers.cpp:502 | head, dt gate, tail (JBE skips the +0Ch store), theta scan 00795F25..00795FF6 | CONFIRMED on sampled range; U1 resolved (note 2) |
| 00AF8500 compose object rotation | include/bsp/particle_emitters_cc13.hpp:213 | complete | quirk claim CONFIRMED |
| 0068F030 hint definitions loader | src/gui_screens_3.cpp:639 | cooldowntime and duration branches, row loop bound | **DEFECT D1 + DOC-CLAIM-WRONG** |
| 00B9D7B0 grid tile refresh | docs/CC13_WATER_RENDER.md section 10.2 | the "native defect" claim | quirk claim CONFIRMED |
| 00A93690 remove matching binding | docs/CC13_SOUND_STREAMING.md | complete | quirk claim CONFIRMED, plus an unrecorded empty-array case |

## Defects

### D1 (real behaviour) 0068F030: a missing `cooldowntime` is 0, not the previous entry's value

The reconstruction and docs/CC13_GUI_SCREENS_3.md say 005FD650 does not clear +120h and that the
loader writes +120h only when the key is present, so a missing key inherits the previous entry's
value. The first half is right (005FD650 stores +0..+4Ch, +118h, +11Ch and the strings, not
+120h). The second half is wrong: the loader writes +120h on both branches.

```
0068FFEB CALL 00B65FB0          ; is_nil("cooldowntime")   (key string 00CF7B38)
0068FFF0 TEST AL,AL
0068FFF2 JZ   0068FFFD
0068FFF4 MOV  [ESP+1B8h],EDI    ; nil: +120h := EDI
0068FFFB JMP  00690030
...
00690024 CALL 00B66290          ; integer
00690029 MOV  [ESP+1B8h],EAX    ; present: +120h := value
```

The definition is constructed at ESP+98h (0068F1DF `LEA ECX,[ESP+98h]` / `CALL 005FD650`), so
ESP+1B8h is +120h. EDI is 0 on both paths into 0068FFF4: the last EDI writes before it are
`XOR EDI,EDI` at 0068FF7B (fall-through from the `storingrule` assign) and at 0068FD78 (the path
through 0068FF27, which stores the same EDI into a string's pointer and length, 0068FF19/0068FF20);
nothing between writes EDI (whole-listing filter, 0068F030..00690AF6). The neighbouring `duration`
branch has the same shape with an immediate (0069008D `MOV [ESP+1BCh],0Ch`), which the
reconstruction already models.

Proposed fix (owner of src/gui_screens_3.cpp; the file was unleased at review time):

```diff
--- a/src/gui_screens_3.cpp
+++ b/src/gui_screens_3.cpp
@@ void hint_tracker_load_definitions_0068f030(
-// cooldowntime is read only when present and 005FD650 does not clear +120h, so a missing key
-// keeps the previous entry's value (an uninitialised stack word for the first entry); `carry`
-// reproduces that across calls of this function only.
+// cooldowntime: 0068FFF4 stores EDI (0 on every path) when the key is nil, 00690029 the
+// integer otherwise, so a missing key gives 0 (005FD650 itself does not clear +120h).
@@
-        def.cooldown_time = carry.cooldown_time;                        // stale +120h
+        def.cooldown_time = 0;                                          // 0068FFF4, EDI = 0
@@
-        if (!e->is_nil("cooldowntime")) def.cooldown_time = e->integer("cooldowntime");  // 0068FFCE
+        if (!e->is_nil("cooldowntime")) def.cooldown_time = e->integer("cooldowntime");  // 00690029
```

The `carry` parameter (and the header note at include/bsp/gui_screens_3.hpp:416) then has no
native counterpart and can be removed with its callers; docs/CC13_GUI_SCREENS_3.md line 303ff
should drop the inheritance quirk and keep the TitleDescList one (that one holds: the row loop
006904C4..00690909 has no compare against a count, rows are 18h bytes from +54h, and the clear
loop 00690966..00690982 covers exactly eight).

### D2 (NaN only) 005B58DF: the shadow-fade clamp sends NaN to 0, the native keeps NaN

```
005B59B5 FLDZ ; FCOMIP ST1 ; FSTP ST0
005B59BB JBE  005B5A66        ; 0 <= a, or unordered -> upper-clamp test
005B59C1 a := 0               ; only when 0 > a (ordered)
005B5A74 COMISS a, 1.0
005B5A77 JBE  005B59C4        ; a <= 1, or unordered -> keep a
```

`if (!(a >= 0.0f)) a = 0.0f;` (src/map_screen_update_cc13.cpp:132) maps NaN to 0; the bytes keep
NaN (both tests route the unordered case to "keep"). Fix: `if (0.0f > a) a = 0.0f; else if (a >
1.0f) a = 1.0f;`. Its comment "FLDZ / FCOMIP / JBE" is the right site; only the condition
direction for the unordered case is wrong. Same fix shape as `clamp_unit` in
src/scene_renderer_cc13.cpp, which is right.

### D3 (NaN only) 00B9AE00 step 8: NaN forward.y is nudged in C++, not in the image

```
00B9B7D4 FLD |fy| ; FLD cos89 ; FCOMIP ST1 ; FSTP ST0
00B9B7E0 JB  00B9B93C        ; cos < |fy|, or unordered -> skip the nudge
```

src/water_render.cpp:289 `if (!(cos_limit < forward_up))` nudges a NaN. Fix:
`if (cos_limit >= forward_up)` (ordered test; unordered skips, as JB does). The sibling test at
00B9B79E (JBE, NaN negated) is modelled correctly.

## Precision and documentation notes (no behaviour change for finite inputs)

1. src/scene_graph_cc13.cpp:28 computes `time - key.time` in float (SSE) before the double divide;
   00B7705A forms it on the x87 stack (FSUBP from 80-bit operands). The rounding of that one
   difference can differ from the image in the last bit. The file header already says it
   approximates extended intermediates.
2. U1 (resolved 2026-10-12): 00795EE0 is `(elapsed, pose*, offset)`, RET 0Ch. The caller 00795C10
   pushes the offset, then a pointer to its local [ESP+14h] (00795C9C), then elapsed. The body
   reads arg 1 and arg 3 (t = elapsed + offset, 00795EE3/00795EEA), and Ghidra finds no reference
   to arg 2. 00791C90 has the same layout and was already documented that way. The C++ parameters
   were already the right two; only the header note was added.
3. Resolved 2026-10-12: 00BF85B0 is the CRT `floor`. Its x87 fallback 00C0F394 sets the control
   word [00E16460] = 173Fh (RC = round down) through `_ctrlfp` 00C138A7 and rounds with `__frnd`
   00C28548 (FRNDINT). Errors are reported to the handler with opcode 0Bh, which is OP_FLOOR
   in the MSVC trans.h numbering.
4. 00A93690: the documented quirk (a miss copies the last binding into the slot past the end, then
   destroys the last one and decrements the count at +14h) is in the bytes (00A936D0..00A93745).
   Not documented: with a count of 0 the loop is skipped, ESI = begin is compared with
   begin - 34h, the record at begin - 34h is copied over begin, 00A93070 destroys begin - 34h and
   the count becomes -1. No C++ exists for this routine.
5. 00B9D7B0: the "native defect" claim holds. The frame is `SUB ESP,48h` + four pushes; the level-0
   loop (00B9D7F0..00B9DA07) reads [ESP+4Ch..54h] at 00B9D9BF/00B9D9D3/00B9D9E5 and no instruction
   before it writes those slots (no store, no LEA of them, the only calls in the loop are
   00BF6713); the first stores to +44h..+50h are at 00B9DA1B..00B9DA53, after the loop.
6. 00AF8500: the in-place transpose (six swaps 1/4, 2/8, 3/12, 6/9, 7/13, 11/14 at
   00AF8512..00AF8585), Rz from 00B64780 (ECX = out, EDX = &angle, no stack arguments), then
   destination = (M' * Rz) * M' through two 00413920 calls whose second takes the (destination, M')
   pushed before 00B64780: confirmed as documented.

## Checklist notes

- Rule 8 (register provenance) was applied to EDI in 0068F030, XMM0 before 005B4772, XMM0/1/2 before
  00B15958 and 00B15CB3.
- `tools/verify_report_calls.py` takes a report JSON; this packet has none (no host tables), so it
  was not run.
- No Ghidra, ledger or source changes were made.

## Second pass (2026-10-12)

D1-D3 are fixed on this branch: 9eb9d7c10, 41789fbe9 and a55a497db. The open notes 2 and 3 are
resolved by d5c458063 and aa0d0496c, and the 00A93690 empty-array case is now documented
(583a3c5f5).

| Routine / range | Source | Verdict |
|---|---|---|
| 00511F40 0051384C..005145DF carousel | src/menu_screens_2.cpp:1924 | CONFIRMED. The spacing is 0.46 [stored at +43Ch], the side panels sit at centre -/+ 0.46, and the frames and icons are looked up in native order. |
| 00511F40 005145E2..0051469F selection | src/menu_screens_2.cpp:1972 | CONFIRMED. Jump table 00514BFC maps kinds 0..8 to +3D4h, +3E4h, +3F4h, +3C4h, +404h, +3F4h, +3F4h, none, +414h. |
| 00511F40 005146A4..00514BAF keret frames | src/menu_screens_2.cpp:1991 | CONFIRMED. The position is (x + 0, 0 + y, z + 1.0 [00D7A210]), replayed with x87_sym; Capstone's `fadd st(1), st(0)` at 00514849 is the ST0 form. |
| 0062C2C0 rank page 006359F6..00635F9C | src/ingame_gui.cpp:863 | CONFIRMED. The rank clamp is MOVZX/SUB 1/JNS/XOR, then 13h. The "next" threshold is skipped at rank byte 14h and "previous" at byte <= 1. Before = 0 on a signed JL. The bar inset is 0.00625 [00CF2920]. |
| 0062C2C0 award layout 00636258..006363A7 | src/ingame_gui.cpp:917 | CONFIRMED, including the FCOMI quirk: the icon height (qword) is compared with the float-rounded text height, and the unrounded text height is recomputed on the other branch. Gap 1/144 [00CF4570], padding 1/72 [00CEBDD0]. |
| 00B156C0 post objects, composite, final copy | src/scene_renderer_cc13.cpp:612..701 | CONFIRMED on 00B16998..00B16B84 and 00B16BBD..00B16C19. The double +21Ah/+21Bh test is native, 00B4FFE0 is the +23Ch getter, the composite order is 74h/80h/70h, the film gate is `film && (amount > 0 or film >= 2)` with strong = `amount > 0 or film == 3`, and +228h = extent.y / (1 - 2 * amount). |
| 0078FCF0 ease / `modifier_ease` | src/cc13_camera_controllers.cpp:151 | CONFIRMED, including NaN, which reaches the 1 - 2(1-f)^2 branch on both sides. |
| 00B9AE00 step 9 (B9BA2A..B9BD41) | src/water_render.cpp:318 | CONFIRMED for finite input. The XOR of the two side flags selects the reflection, and the blend matches. NaN-only difference: both side flags are formed as "JB -> 0", so an unordered dot gives flag 0, while the C++ `!(0 < d)` gives 1. This was not changed because it only affects NaN camera vectors. |
| 00A8BD20 00A8D808..00A8DA20 near/far, mirror, final multiply | src/shadow_cascades_cc13.cpp:523 | CONFIRMED. 00D7A318 is (double)0.001f and 00CEFF98 is (double)0.6f. On the w < 0 branch, near = -0.0 - max(0.001, lo.z) and far = lo.z, as the C++ has it. The ortho result is copied to ESP+D4h, which is the left operand of the final 00413920 at 00A8DA09; mirror[0] (ESP+88h) is -1 unless the w flag is set. |

## Third pass (2026-10-12)

b0d15112c fixes the step-9 NaN side flags: they are now the ordered `d <= 0`, matching the
FCOMI/JB at B9BA48/B9BA74.

| Routine / range | Source | Verdict |
|---|---|---|
| 0062C2C0 scoring layout 0063018D..0063089E | src/ingame_gui.cpp:561 | CONFIRMED. The mode is 2 when +1FE4h is set, otherwise +6ACh. The done-to width is total.x - done_to.x - 1/48 [00CF0000]. The row step 0.0305556 [00CF53B8] is used at 00631665 and the group gap 0.0027778 [00CF53B0] at 006316F2. |
| 0062C2C0 player damage table 00630C13..0063181C (sampled) | src/ingame_gui.cpp:677 | CONFIRMED on the points calls. 00911CE0 is called at 00630EB7 with (by, [ESI] target category, side, [ESI+4] damage, mode, +24Dh). The kills are CVTSI2SS (0063104B) and passed as float to 00911C00 at 0063107F. |
| 0062C2C0 map usage time 00634851..00634900 | src/ingame_gui.cpp:727 | CONFIRMED. CVTTSS2SI; "%d:%02d:%02d" [00CF52C4] from 3600 (E10h), "%d:%02d" [00CF52BC] from 60, otherwise "%d" of at least 1. |
| 0062C2C0 debriefing page 0062CC42..0062D2F6 (sampled) | src/ingame_gui.cpp:326 | CONFIRMED. The scroller range is the text height (float) - +D0h (0062CDC1..0062CDDA). The voice gate is NEG/SBB/TEST EAX,00E18B5Ch on +10h (0062D111). |
| 00B156C0 DRSH and SCNE call order 00B15DD4..00B1625B | src/scene_renderer_cc13.cpp:498..538 | CONFIRMED. Clear colour FFFFFFFFh comes from BL = FFh (00B1573D, the only EBX write). The three targets are 38h/3Ch/40h. The batch lists take the job-pool path only when the guard byte [0108D6DC] is 0 and both counts exceed 32h; the list +8 word is EBP = 0 (00B15E2D) and 1. The restore order after DRSH matches. |
| 00B9AE00 steps 10..12 B9C24B..B9C6F3 | src/water_render.cpp:362..406 | Min/max CONFIRMED: unrolled FCOMI/JBE, so updates are strict and NaN updates nothing. The output-matrix chain (two 00413920, 00B632D0 twice, 00B63B30, then a 00413920 pair whose second product takes the 00B63B30 result) has the same shape as the C++. I did not trace which stack slot each operand comes from. |

Still not reviewed: the scoreboard page 0062D304..0062E9A4, the objectives page, the party
damage table and the remaining command-table groups (all are host-call sequences built from
helpers the passes above already confirmed), and the exact operand slots of the 00B9AE00
output matrices.

## Handoff

Routing for the lead: D1 to the owner of src/gui_screens_3.cpp (cc13-gui lineage); D2 to the owner
of src/map_screen_update_cc13.cpp (cc13-warnings1); D3 to the owner of src/water_render.cpp
(cc13-water2). All three are now fixed on this branch; see "Second pass" for what was reviewed
after that and what is still unreviewed.
