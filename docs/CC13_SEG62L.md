# CC13 seg62L: seg 62/65 large bodies, lobby layout, large61 audit

Packet cc13-seg62L (worker cc13-sweepB11, tree cc13-seg62L). New files
`include/bsp/cc13_seg62L.hpp` and `src/cc13_seg62L.cpp`, with an audit fix in
`src/cc13_large61.cpp`. Names are hypotheses, not recovered symbols. Nothing is
wired into a host. Status: Win32 build-tested only. There is no fixture and no
original-ABI or game validation.

## Triage (rule 1)

| Address | State | Where |
| --- | --- | --- |
| 0097D650, 00980380, 00982540, 0095DE00, 0097B5E0 | already reconstructed | `src/cc13_large62.cpp` |
| 00421CE0 | already reconstructed (grid contour tracer) | `src/cc13_x87c.cpp` |
| 00957740, 00957BD0 | existing `skipped_cc9_lane` records, left as they are | docs/CC13_SWEEPC.md |
| 0097BF20 | skipped: `listing_fragment_not_function_start` | see below |
| 005D8B80, 005D8FC0 | **reconstructed here** | `src/cc13_seg62L.cpp` |

0097BF20 is skipped. Ghidra treats the address as a fragment of a neighbouring
body, not a function start. A Ghidra boundary repair is needed first, which is
more than a trivial annotate/lock edit. No record was changed.

## Reconstructions

Both bodies are called as hooks from `cc13_sweepB8` (`lobby_005d8b80`,
`lobby_005d8fc0`). The host is `Cc13Seg62LHost : Cc13SweepB11Host`. It adds one
hook, `game_set_command_line_text_004c8950` (004C8950, `__thiscall(game, text)`,
RET 4). Neither body uses x87.

### Lobby layout record

| Location | Contents |
| --- | --- |
| screen +2E4h / +2E8h | first/last of a vector of 24h-byte rows |
| row +0h | mission id |
| row +4h | NativeString vector (first +8h, last +0Ch) |
| row +14h | NativeString vector (first +18h, last +1Ch) |
| screen +2F0h / +2F4h | selection (row / entry) |
| screen +2F8h / +2FCh | first selection |
| 00E195BC / 00E195C4 | the two selected NativeString globals |

Rows are counted as a signed (last - first) / 24h (IMUL 38E38E39h, SAR 3,
sign fix) and compared unsigned. Every checked access calls 00BF6713 where the
image does.

### 005D8B80 BSP_LobbyLayout_RefreshSelection

`__thiscall(ECX screen)`, RET, returns EAX = 00E195BC.

**With a selection** (+2F0h and +2F4h both not -1):
1. 00E195BC = the 005D5160 record's +14h vector [+2F4h].
2. If [00E188A8]+1FE4h == 2, call 004C8950 with the record's +4h vector [+2F4h]
   text, or 00E19605 when that is null.

**Otherwise:**
1. Empty both globals: 0041DD40(.., 0, 0), then a memcpy from 00CE3A0C.
2. Set the selection to -1/-1.
3. Go through the rows whose id equals [[00E188A8]+5FCh]+1098h. In each, find
   the first +4h entry that equals the game's +600h string: equal length, then
   the inline empty-string rules, then `_stricmp` 00BF7FBF.
4. On a match, copy both strings and select (row, entry), then go on to the next
   row. The last matching row wins.

### 005D8FC0 BSP_LobbyLayout_SelectDefault

`__thiscall(ECX screen)`, RET. It returns at once when [00E188A8]+5FCh is null.

For every row whose id matches and whose +14h vector is non-empty:
1. Copy entry 0 of each vector into 00E195BC / 00E195C4.
2. Select (row, 0).
3. Fill +2F8h/+2FCh if both are -1.
4. Copy the +4h entry 0 text (00E19605 when null) into game+600h:
   0041DD40(strlen, 0), then a memcpy of the new length, with no terminator.

There is no break, so every matching row runs.

## Blind audit of src/cc13_large61.cpp

Each body was read against the full Ghidra listing. Frame offsets, argument
order, branch senses (including NaN through JBE/JA after FCOMIP/COMISS), the
literal string lengths and the call order were checked.

| Address | Grade | Finding |
| --- | --- | --- |
| 00947BC0 | FAITHFUL | Zeroed members, a0..a6 offsets and the three vector copies match. The +44h copy really is gated on a4's size (00947D8E reloads [ESP+34h]). |
| 00945450 | FAITHFUL | The second-name index trick (SETL, SUB 1, AND 3) and the matrix product order Rz*Ry*Rx*T match. |
| 0094BFF0 | FAITHFUL | Nine arguments (RET 24h), the 00949530 argument order, the forward-row x87 math (each step stored to float) and the destructor order match. |
| 0093D2D0 | FAITHFUL | The three ocean-shader comparisons, the flag split, the guard bits, ground +8 * 3.0 per pass, both ocean_size copies, and the operand swap 711C+7128 (commutative) match. |
| 0094B600 | MINOR, fixed | The two arc quotients (int at entity+33Ch over 114.6 and -114.6; the doubles are exactly 114.6f) were computed in double and then rounded to float. At the main thread's 24-bit precision each is a single rounding, so they are now float divides. The stale 00946D70 tag passed as 0 stays MINOR and documented. |
| 0093FF10 | REAL, fixed | The label percents `(int)(level*100)` and `(int)(100*volume)` were modelled in double. At 24-bit precision FMUL rounds to single before `_ftol`, so level 0.29f prints 29 where the double model printed 28. Now float multiplies. The 00940495 stack sum (dy²+dx²)+dz² and the 00940991 volume quantisation (v/0.05 + 0.5, steps*0.05) are now single precision too. |

No body was a placeholder. The EH unwind states are still not modelled in any of
the six bodies; the file header already says so. The 53-bit-precision
alternative stays an uncertainty: the overlay and spawn handlers run on the main
thread, where x87 precision is 24-bit.

## Uncertainty

- 004C8950 is named from its existing ledger record (`BSP_Game_SetCommandLineText`).
  Its use here (session kind 2 shows the selected layout text) is inferred.
- The meaning of a row's mission id, and why the last matching row wins in
  005D8B80, have not been checked against the lobby UI.
