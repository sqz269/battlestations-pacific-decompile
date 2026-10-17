# CC13_FIX2: fighter gunfire-avoidance timer scaling

Packet cc13-fix2 (2026-10-17), finding R2 of docs/CC13_VERIFY15.md, under cc9's grant to edit
`src/game_hosts_units.cpp` behind an OFF switch.

## Switch

`kPilotFighterGunfireScaleBound`, `false`, declared next to `kPilotGunfireAvoidanceBound` in
`src/game_hosts_units.cpp`. It acts only inside the gunfire-avoidance arm (0099EC40,
`BSP_PilotBot_AvoidGunfire`), which itself runs only while `kPilotGunfireAvoidanceBound` is ON (it is).
With the switch OFF the code is exactly as before.

## Behaviour

When the timer re-arm fires (`ga_timer_3e8 <= 0` and not zero, 0099EF2D) and the two 00BD2F10 draws have
stored the evade time in `ga_timer_3e8` (+3E8h) and the negated rest time in `ga_rest_3ec` (+3ECh), a unit
whose class IsKindOf 13h (MPlaneFighter, `src/entity_class_ids.cpp:31`) scales both:

    f = float(1 - level x 0.3);  if (0.4 > f) f = 0.4f;
    +3E8h = +3E8h x f;  +3ECh = +3ECh / f

`level` is `[[unit+DF4h]+34h]`, the pilot bot's robot level index. The host's copy is
`GameUnitSlot::pilot_skill_index` (the same index that 009F9D08-009F9D22 multiply by 248h to point
approach+14h at the PilotBot row, which the host captures as `db_skill_row_14` from `pilot_skill_index`).
The image reads it live here, not through the approach, so the host reads `pilot_skill_index`, not
`db_skill_row_14` (which only a dive-bomb approach sets). With `kSkillLevelBound` off the level is 1
(SPNormal), matching the host's other skill sites. The image applies no 0..5 clamp here (no row lookup),
so neither does the host.

| level | f | evade time | rest time |
|---|---|---|---|
| 0 Stun | 1.0 | unchanged | unchanged |
| 1 SPNormal | 0.7f | x 0.7 | x 1/0.7 = 1.43 |
| 2 and up | 0.4f (clamp) | x 0.4 | x 2.5 |

## Evidence (disk bytes, `bsp.py disasm-raw 0099EF60`)

- 0099EFA0 the second 00BD2F10 draw; 0099EFAB FCHS; 0099EFAD stores +3ECh.
- 0099EFA5 ECX = [EDI+2F4h] (the unit); 0099EFB3-0099EFBE `call [vtable+5Ch](13h)`, `JE 0099F02F` when
  false. The unit's IsKindOf is the host's `bsp::unit_is_kind_of(u.class_id, 0x13)`.
- 0099EFC8 f = float [00D7A24C] = 1.0f (bytes `00 00 80 3F`).
- 0099EFC0-0099EFE0: when [EDI+2F4h] is null or [unit+DF4h] is null, f stays 1.0f. The host has no
  null pilot-bot state for a flying plane, so that arm is not represented.
- 0099EFE2 FILD dword [DF4h-object+34h]; 0099EFE5 FMUL qword [00CE3DC8] = 0x3FD3333340000000 =
  0.30000001192092896 (0.3f widened); 0099EFEB FLD1; 0099EFED FSUBRP (1 - product); 0099EFEF FSTP dword f.
- 0099EFF3-0099EFFF: FLD f; FLD qword [00CE65D0] = 0x3FD99999A0000000 = 0.4000000059604645 (0.4f
  widened); FCOMPI ST(1) compares 0.4 with f; JBE 0099F015 (0.4 <= f, or unordered) keeps f. So a NaN f
  keeps f; the host writes `if (0.4f > f) f = 0.4f;`, false for NaN.
- 0099F001 f = float [00CE7804] = 0x3ECCCCCD = 0.4f.
- 0099F015-0099F01D: +3E8h = float(+3E8h x f) (FMUL ST(1), FSTP dword).
- 0099F023-0099F029: FDIVR dword [+3ECh] gives +3ECh / f; FSTP dword +3ECh.
- RNG: no extra 00BD2F10 draws; only the stored values change.

## Precision

0099EC40 is a unit job (called from 009A17D0), which may run at 24- or 53-bit precision control
(docs/CC13_FPU_PRECISION.md). The host models 24-bit order: the product rounded to float, then the
subtraction rounded to float. For every reachable level (0..5) both precisions give the same f:
1 - 0.3f is exactly 0.7f, and levels 2 and up clamp to 0.4f. The multiply and divide of two floats
rounded once to float are identical at either precision (the double result rounds correctly to float).

## What a pair run should show

Only with `kPilotFighterGunfireScaleBound` ON, on a mission with fighter dogfights (any fighter that a
firing plane holds in its gun cone within 800 m): at SPNormal each fighter evasion lasts 0.7 x as long
and the rest between evasions is 1.43 x longer: each re-arm stores 0.7 x the drawn `ga_timer_3e8` and
1.43 x the drawn `ga_rest_3ec` for fighters. Non-fighter evaders (USN04's Vals) are unchanged, as is
every RNG draw count. No game run was made in this packet.

## Uncertainty

- The null unit / null +DF4h arm (f = 1.0f) is not modelled; the host always has the level.
- The `pilot_skill_index` setters (`src/game_hosts_units.cpp:17904`, `:17912`) take the level unclamped; the image does not clamp here either.
