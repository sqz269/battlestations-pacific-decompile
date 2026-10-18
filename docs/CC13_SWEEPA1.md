# cc13-sweepA1: the last segment 0..14 leftovers of docs/CC13_SWEEPA.md

Packet cc13-sweepA1 (2026-10-18), worktree cc13-sweepA1b (the branch name agent/cc13-sweepA1 belongs to an
earlier, merged packet). Code: `include/bsp/cc13_sweepA1b.hpp`, `src/cc13_sweepA1b.cpp`, namespace
`bsp::cc13_sweepA1b`. `SweepA1bHost` extends `cc13_sweepA4::SweepA4Host`. Nothing is wired. Names are
hypotheses. Status: build-tested only (no fixture, not ABI-compatible, not game-validated).

## Triage (rule 1)

66 addresses were listed (004A0D20 excluded by the packet). 57 already have a reconstruction from cc13-sweepA2,
A3 or A4 (or older work) and a ledger record; they are not touched. The other nine:

| Address | Outcome |
|---|---|
| 004D80D0 | reconstructed here |
| 0046B730 | reconstructed here (cc9's scene-contents host models only part of it, through a hook) |
| 004F3DA0 | reconstructed here (replaces the `unreferenced_recorded` record) |
| 004F4880 | reconstructed here (replaces the `unreferenced_recorded` record) |
| 004C74C0 | cc9 lane, hosted inside `border_zone_at_point_00489c40` (src/plane_retreat_task.cpp); compared and recorded only |
| 004C3A80 | cc9 lane (controlled-unit selection, docs/CC13_SEGS_0_9_11.md); not hosted, so no record; skipped |
| 004C7AA0 | cc9 lane (closest-border-zone geometry, docs/CC13_SEGS_0_9_11.md); not hosted, so no record; skipped |
| 00460750 | STL vector insert-n instantiation; vendor body, bind only (existing `out_of_scope_vendor_library` record) |
| 004E5540 | existing partial implementation `game_on_init` (src/game_entry.cpp), already recorded; not rewritten |

Ledger fix: the names ledger already says 004D0860 is `std::operator+` (one record, cc13-sweepA2 correction), but
Ghidra still showed BSP_Game_SplitCommandToken. It is now renamed in Ghidra through
`tools/ghidra_annotate.py --apply`. 0046B730 is named BSP_SceneDatabase_KillPass (names ledger and Ghidra).

## 004D80D0 BSP_Game_UpdateMultiplayerInterface

`game_update_multiplayer_interface_004d80d0`, `__fastcall(ECX = game)`, plain RET, SEH 00C66498. Read from the listing
(359 instructions). The pseudocode drops the error-code selection.

- The network session [00F8A2FC] gets vcall +0Ch(float game+21ECh). 006890B0 runs on the multi menu [00E198B4]
  when it is set, then 0076A750(game+1EF0h).
- When game+1FE4h is 0, [00F1B038] is set to 0 and the function returns before the tail.
- Interface state +624h is 1 or 2 and byte +610h is clear:
  - Mode +5D4h is not 0Ch/0Dh: the leave arm runs. 00688C70; when [00F1B038] != 2, 00688AD0(1Bh) and
    005D2E70([[00E198B4]+50h], 7); [00F1B038] = 0; 004BDF50.
  - [00F1B038] == 1: nothing.
  - Other non-zero [00F1B038]: 004254B0 ("... -> SCENE_AUTO_TERM") and 004D3ED0(game+5D8h, &10h).
  - [00F1B038] == 0 and +1FE4h == 2: the prompt with "FE.multi_terminated_client" (state 1) or
    "ingame.multi_gameclosed_xbox", then +608h = 1, 004CD0F0(1, 0, 1) and [00E19698]+4 = 0 when that pointer is set.
  - [00F1B038] == 0 in mode 0Dh otherwise: the menu command screen's +288h = 1 and the "FE.multi_terminated" prompt.
  - Every arm except the two that do nothing sets +624h = 3.
- Each prompt is 00531B00(2, &text, 2, 004D7B10, 0, &"", 0.0f, 0, "" by value, 1) on 00425D10(). The strings are
  built with 0041E870 and the text is released (0041DD20) before the empty string.
- State 0, +1FE4h == 2, 004B61C0(game+1EF0h) true and +610h clear: kicked = [[game+207Ch]+9Ch] == 3.
  - In modes 0Ch/0Dh: 004D7BA0(kicked) when [00F1B038] is 0, else the "SCENE_TERM" request unless it is 1.
  - Otherwise the leave arm runs with code 3 (kicked), 6 ([00F8ABE8]+128h set), 2 (no session, or session vcall +19Ch
    true, or vcall +40h true) or 1.
- Mode 0Dh tail: 004C3A80(game) runs when ([00E188A8]+61Ch or +1FE4h is set) and the phase +614h is 4..7. Otherwise
  it runs when 006529E0([[00E198C4]+BCh]) is false. Then the page [game+18CCh+[+18ECh]*4], when its short +10h > 0 and
  byte +18h is clear, clears [00E19698]+4 (no null check) and sets +18h.
- Modes 0Ch, 0Dh and 12h: 0076C500(game+1EF0h).

Uncertain: 004D7BA0 receives the SETZ byte; the image pushes the whole dword, whose upper bytes are stale.

## 0046B730 BSP_SceneDatabase_KillPass

`scene_database_kill_pass_0046b730`, `__fastcall(ECX = database)`, plain RET, SEH 00C61CEB. Its sole caller is
BSP_EngineMovie_Stop. Read from the listing (272 instructions). The pseudocode loses ESI in the record loop and the
ECX/EDX inputs of the create call.

1. 004881F0([[00E188A8]+21D0h]).
2. World entities from [[world+8h]], following +44h. An entity whose +C4h key is not in the database map +158h
   (00468DB0 find, iterator checked) goes to a local vector through 004323D0. For a registered entity, each
   unregistered child goes to the vector instead (+50h count, +48h first, +44h next, index-walked twice as in the
   image), through the inlined push_back or 004322D0. Each collected entity gets 00926D90(entity, 2).
3. The settle sequence: 00874D00(CL 0), 00903670(world), 00874D00, then three times world vcall +0Ch(0.0f) and
   00874D00.
4. 004BC890([00E188A8]; 8, 0).
5. For each record of the vector +24h: the class record is the node +8h of 00468FB0(database+34h, record +0Ch name).
   When [class] is not in +158h, the record is recreated:
   - entity = 00925A90(world, record +58h or 00E18560), only when the record's +54h/+58h string differs from "" (the
     inlined compare with _stricmp).
   - The object is [class+4](ECX [class], EDX [record+8h]; entity, record+14h, [record+4h], 0).
   - The object's +C0h = 00922E20(operator new(0Ch), [record+4h]), or 0 when the allocation fails.
6. 00925F20(CL 0) twice, 0046AAB0(database), then the settle sequence with CL 1.
7. Each record gets vcall slot 0 (flag 1) when non-null. The vector is emptied by the inlined erase (memmove_s only
   for a non-empty tail), and the local vector's block is freed (00BF65AC).

Every checked-iterator call 00BF6713 is made at the image's points. Uncertain: whether the create call's callee
cleans its four stack arguments (the image does not adjust ESP after it).

## 004F3DA0 BSP_Triangle2f_ContainsPoint

`triangle2f_contains_point_004f3da0`, `__fastcall(ECX = point, EDX = triangle a, b, c)`, plain RET, AL. It is
unreferenced: its only call is in the undefined wrapper 004F3EE0 (docs/CC13_SWEEPA2.md).

- e1 = b - a, e2 = c - a, d = p - a.
- When e1x == 0, an ordered compare (FUCOMIP / LAHF / TEST AH 44h / JP; NaN takes the other arm): t = dx / e2x,
  u = (dy - t e2y) / e1y.
- Otherwise t = (e1x dy - dx e1y) / (e2y e1x - e1y e2x) and u = (dx - t e2x) / e1x.
- The point is contained when 0 <= t <= 1.0f (00D7A24C), u >= 0.0f (00D7A218) and t + u <= 1. Every NaN rejects.

The listing prints the reverse x87 forms ambiguously, so the operand order comes from the live decompile (rule 4c).
Every step is a float operation (24-bit main-thread precision).

## 004F4880 BSP_Segment2f_ClipAgainstLine

`segment2f_clip_against_line_004f4880`, `__fastcall(ECX = P, EDX = A; float r, const float* B, float* out0,
float* out1)`, RET 10h, EAX = count. It is unreferenced: its only call is in the undefined function 004F4AE0.

1. d = B - A; inv = 00419260(&d), written to the B argument slot; n = d * inv at [ESP+8]; m = (-n.y, n.x);
   Q = P - m.
2. 004F3630(A, &d, &Q, &m, &n.x slot, &inv slot) runs, and its result is ignored. t and s are read from those slots,
   so an output it leaves unwritten keeps the old value, as in the image.
3. s < -r or r < s returns 0 (NaN too).
4. k = 00BF7030(r r - s s) / 00414C60(&d).
5. t0 = t - k and t1 = t + k. Each value in [0, 1.0f] is stored: t0 to out0, t1 to out0, or to out1 when t0 was
   already stored.

## Not done in this packet

- 004C3A80 and 004C7AA0: cc9 gameplay lane, not hosted.
- 004A0D20: excluded by the packet (72 KB).
