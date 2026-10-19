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

Segment 40 has 32 rows carried by an existing implementation, mostly in `game_hosts_gunnery.cpp`
and `game_hosts_script_orders.cpp`. 29 of them already have a verification record:
- `verified_against_listing` / `verified_ledger_only`: 14 rows;
- labelled substitutions verified against the listing: 3 rows;
- labelled substitutions (`00734140`, `00734FA0`): 2 rows;
- `spot_checked_no_defect_seen` / `spot-checked`: 7 rows;
- `minor_deviation`, with the deviation documented at the row: 3 rows (`0072CDD0`, `00731E20`,
  `00735100`).

Three hosted rows are still `host_reference_unverified`, and are **open**. A line-by-line
check of any of them was not cheap, so they were not verified or replaced here:
- `0072E6D0` BSP_Gun_SetupFromDescriptor (`game_hosts_gunnery.cpp`; `00720000.jsonl` line 200).
  This is a large body. cc13-seg48V checked only these facets from the bytes:
  - `0043F7B0`;
  - `00729560` on gun+424h;
  - gun+448h = `0072AB80(class)` (cc9's barrel_num = max(1, n));
  - gun+44Ch = 0.

  cc9 hosts these facets only, and the rest is not compared.
- `00730B80` BSP_RapidFixedSlaveGun_Construct (`game_hosts_gunnery.cpp`; `00730000.jsonl`
  line 4). This is the MRFSGun (class 21h) constructor. The existing adoption note in
  `docs/CC13_SEG48U.md` ("00730B80 and 'a bomb platform is a Gun subclass'") still stands: cc9
  cites it as the proof that a bomb platform is a Gun subclass. The citation is wrong. The
  conclusion may still hold through class 25h's parent 20h, but the bomb-platform constructors
  `006E08F0` and `006E0970` are unread.

  It is a citation error, not a behaviour mismatch. It is not torpedo-relevant.
- `00735450` BSP_LocalizedMessage_Format (`game_hosts_init_tail.cpp`; `00730000.jsonl` line 58).
  The image calls `_vswprintf_p_l(buffer, EDX count, format, null locale, varargs)`. cc9's
  `localized_message` is an unimplemented stub, because the `00D1E918` table is not
  reconstructed. Its only caller is the hardware probe `0073C3B0`, with five calls and EDX = 1000h. That probe
  does not run on this installation, because the BSM_HWD key is absent.

No new adoption note beyond the existing `00730B80` one.

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

## Functions cc16 found undefined

cc16 (main 2b63fdf04, `local\cc16_undef_candidates.json` in its progress tree) listed eight
undefined-code candidates inside this lease. Each was defined with
`tools/ghidra_define_function.py <start> <end>` (write lock; record in
`reports/defined_functions.json`). Ghidra's bytes match the disk image over every body. A rel32 plus
absolute-dword scan found each one's references.

| Address | Range | Reference | C++ | Body |
|---|---|---|---|---|
| `00728DE0` | ..`00728E0C` | `00CFDE40` (GUN_SET_FIRE_MODE vtable `00CFDE3C` +4h) | `gun_set_fire_mode_write_00728de0` | `0075B480` header; `004290B0` bool +1Ch; `00429070` dword +20h, 2 bits. |
| `00728E10` | ..`00728E3E` | `00CFDE44` (+8h) | `gun_set_fire_mode_read_00728e10` | `0075B4C0` header; on stream+4h: `00428D70` bool +1Ch, `00428D10` dword +20h, 2 bits. |
| `00728EC0` | ..`00728EEB` | `00CFDE54` (R_T_GUN_SET_CONTFIRING vtable `00CFDE50` +4h) | `r_t_gun_set_contfiring_write_00728ec0` | header; bool +1Ch; bool +20h. |
| `00728EF0` | ..`00728F1C` | `00CFDE58` (+8h) | `r_t_gun_set_contfiring_read_00728ef0` | header; on stream+4h: bool +1Ch, bool +20h. |
| `0072A090` | ..`0072A0CD` | nine vtable slot entries at (`00CF9840`, `00CF9AB0`, `00CFAC50`, `00CFBEB8`, `00CFC0F0`, `00CFC328`, `00CFE240`, `00CFE4A0`, `00CFE6E0`) | `destroyed_property_get_0072a090` | `__thiscall(key, arg, out)`, RET 0Ch: key+4h non-null and `_stricmp(name, "destroyed")` == 0 gives `*out = object+3B8h`, true; else false. |
| `007312E0` | ..`0073137A` | `00CFE748` (`00CFE740` +8h) | `torpedo_shot_command_write_007312e0` | The writer half of BSP_TorpedoShotCommand: `type` = "torpedo", `heading` = float +4h, `swimdepth` = float +8h, through writer slot +0Ch. Mirrors the reader `00731270` (slot +4h, src/cc13_seg40.cpp). |
| `007358C0` | ..`007358E9` | `00CFEA08` (slot 0) | `singleton_01090298_scalar_deleting_destructor_007358c0` | `[01090298] = 0`; vtable `00CE3818` (SingletonBase); free on flags bit 0; returns the object. |
| `00735B40` | ..`00735B4B` | `00CFEAC4` | `game_member_1a0c_00735b40` | `MOV EAX,[00E188A8] / ADD EAX,1A0Ch / RET`. |

The torpedo writer is serialization only (the property publisher). The gameplay torpedo launch path
does not read it, so there is no URGENT note.

## Uncertainty

- The slot roles of +14h..+2Ch and +3Ch are not named. Nothing found calls them through the base,
  so the C++ names describe only the bodies.
- `00CE3A0C` is four zero bytes, used as the shared empty string (docs/APP_INIT_PLATFORM.md). The
  C++ returns `""`, not that address.
- The segment's vector helpers look like an engine-native vector, not MSVC STL:
  - `0072A3B0` is the reserve (`max(n, 1)` 14h elements).
  - `0072B6D0` is the resize (reserve, then fill).
  - `0072A6D0` is the push-back (grows to `cap * 2 + 2`).

  MSVC's vector grows by half its capacity, so this growth rule does not match it. Their ledger names already say "NativeVector",
  and they are not reconstructed here.
