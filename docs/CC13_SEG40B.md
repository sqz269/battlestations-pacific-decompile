# CC13 seg40b: segments 40 and 58 leftovers (packet `cc13-seg40b`)

Segment 40 is `00728A90..00735EC0` (guns, gun-bot base, shot commands, mission narrative).
Segment 58 is `008E3DE0..008EE5F0`. Code: `include/bsp/cc13_seg40b.hpp` and
`src/cc13_seg40b.cpp` (marker `cc13_seg40b`). Nothing is bound into `bsp_game`. Segment 58 needed
no new code, so there is no `cc13_seg58b` file.

## Census (index of 2026-10-19, main 32b1ed132)

| Segment | Functions | With a record | Without | Library / compiler | Trivial | Game code |
|---|---|---|---|---|---|---|
| 40 | 285 | 159 | 126 | 112 (24 STL instantiations, 41 STL shapes, 3 xlen throw sites, 16 adjustor thunks, 26 deleting destructors, 2 `Catch_All`) | 11 (this packet) | 3, leased to cc15-g24c |
| 58 | 228 | 96 | 132 | 132 (31 STL instantiations, 85 STL shapes, 8 xlen throw sites, 8 `Catch_All`) | 0 | 0 |

The three segment-40 game rows `00734870`, `007349C0` and `00734E50` are held by cc15-g24c's lease
and were left alone. The library rows stay bound.

## Hosted gameplay

Segment 40 has 29 rows carried by an existing implementation, mostly in `game_hosts_gunnery.cpp`
and `game_hosts_script_orders.cpp`. Every one already has a verification record:
- `verified_against_listing` / `verified_ledger_only`: 14 rows;
- labelled substitutions verified against the listing: 3 rows;
- labelled substitutions (`00734140`, `00734FA0`): 2 rows;
- `spot_checked_no_defect_seen` / `spot-checked`: 7 rows;
- `minor_deviation`, with the deviation documented at the row: 3 rows (`0072CDD0`, `00731E20`,
  `00735100`).
No new adoption note.

One re-check sits next to the level-bomb path. The host's `kDiveBombThrowMulRows` /
`kLevelBombThrowMulRows` (`game_hosts_gunnery.cpp` ~236) match this installation's
`scripts/datatables/robots.lua` PilotBot rows in 00901610's order:
- Stun: dive 1.0 (:1274), level 1.0 (:1284);
- SPNormal: 1.0 (:583), 1.0 (:594);
- SPVeteran: 0.0 (:722), 1.0 (:732);
- MPNormal: 1.0 (:860), 1.0 (:870);
- MPVeteran: 1.0 (:998), 1.0 (:1008);
- Elite: 0.0 (:1136), 1.0 (:1146).

## Routines

Gun-bot base vtable `00CFDFA0` (constructor `0072BBD0`; slot table in docs/GUN_BOT_TICKS.md
section 2). The defaults below sit at these slots. Derived vtables keep some of them at the same
slots; the absolute scan of `.rdata` found the copies at `00CFE000`, `00D0D360`, `00D180A0`,
`00D18140` and `00D18238` (each start = its slot +0Ch tick entry less 0Ch).

| Address | Slot | C++ | Body |
|---|---|---|---|
| `0072BC70` | +14h | `gun_bot_default_name_0072bc70` | `MOV EAX,00CE3A0C / RET`: the empty literal. |
| `0072BC90` | +1Ch | `gun_bot_default_false_0072bc90` | `XOR AL,AL / RET 4`. |
| `0072BCA0` | +20h | `gun_bot_default_one_0072bca0` | `FLD1 / RET`. |
| `0072BCB0` | +24h | `gun_bot_default_one_0072bcb0` | `FLD1 / RET`. |
| `0072BCC0` | +28h | `gun_bot_default_noop2_0072bcc0` | `RET 8`. |
| `0072BCD0` | +2Ch | `gun_bot_default_zero_0072bcd0` | `XOR EAX,EAX / RET`. |
| `0072BD00` | +3Ch | `gun_bot_default_noop1_0072bd00` | `RET 4`. |

The base-destructor bodies, each `MOV [ECX],imm32 / RET`, are reached only by `JMP` from
exception-unwind funclets (rel32 scan):

| Address | Vtable stored | Funclets |
|---|---|---|
| `00729090` | `00CE4974` (BSP_SessionMessageRoot) | `00C856D0` (ECX = `[ebp-30h]` object) |
| `007290A0` | `00CE4974` | `00C856B0` (ECX = `[ebp-30h]` object) |
| `007293B0` | `00CE4974` | `00C8587B` (ECX = `[ebp-4Ch]` object) |
| `00730B50` | `00CFE2B8` (the shot-command base, src/cc13_seg40.cpp 00731220) | `00C85890`, `00C858B0`, `00C858D0` (ECX = `[ebp-10h]`) |

`0072BC90`'s absolute hit at `004CA918` is a false positive: it falls inside the rel32 of
`CALL 00BF65AC` at `004CA917`.

## Uncertainty

- The slot roles of +14h..+2Ch and +3Ch are not named. Nothing found calls them through the base,
  so the C++ names describe only the bodies.
- `00CE3A0C` is four zero bytes, used as the shared empty string (docs/APP_INIT_PLATFORM.md). The
  C++ returns `""`, not that address.
