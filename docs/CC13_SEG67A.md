# CC13_SEG67A: the plane `closetoship` bot task (segment 67)

Packet cc13-seg67a, 2026-10-17. Grant: the cc9 gameplay lead opened segment 67
(00996120-009F6060) for standalone reconstruction of functions without a
reconstruction record, excluding the live cc9 sub-ranges (009AA000-009AAFFF,
009AE000-009AFFFF, 009B4000-009BAFFF, 009C2000-009C3FFF, 009C8000-009C8FFF,
009D2000-009D4FFF, 009F5000-009F6060). Nothing here touches those ranges; the
moveto (009C2AC0) and follow (009C2980) state constructors inside 009C2000-009C3FFF
are only called through host hooks.

Code: `include/bsp/cc13_seg67a.hpp`, `src/cc13_seg67a.cpp`. New C++ interfaces
over raw native records, every native call through a named `Cc13Seg67aHost`
method at the same point with the same arguments. Not ABI-compatible, not wired,
not game-validated; build-tested only. Names are hypotheses.

## Why this family

The candidate list (unrecorded functions in segment 67 outside the struck
ranges, ranked by reachability from the pilot-bot tick 0099ACD0 and by
reconstructed callers) put the close-to-ship task first among the families with
no implementation: 009A2F40 BSP_BotTask_MakeCloseToShip is reached from the
command producer (src/attack_commands.cpp references it), and `src/bot_tasks.cpp`
holds only its table row (`kCloseToShip`, 009A2F40/009A2730/009A2500, 554h). The
task's vtable 00D1F4E8 has eight slots with no Ghidra function
(009A2800, 009A2890, 009A28C0, 009A2B00, 009A2B90, 009A2C00, 009A2CF0, and the
approach vtable's 009A1C30, which Ghidra labels `Unwind@009A1C30`); they were
read with `disasm-raw` from the disk bytes. 009A2B00 is the task's per-tick slot
(+64h).

## Layout

`task+N` is the 554h-byte task; `approach+N` is relative to task+3F8h.

| offset | meaning | evidence |
|---|---|---|
| task+310h | current state | 009A29B0, 009A27BE |
| task+30Ch | slot passed to the registry load/save 00411170 / 00411450 | 009A2E9B, 009A2D0B |
| task+440h / +444h | moveto altitude (ctl+398h each tick) / beginDist | 009A2B1C, 009A2B32 |
| task+550h | dropStarted, -1.0 at construction | 009A27A1, "dropStarted" 00D1F550 |
| approach+4 / +8 / +Ch / +18h | unit / descriptor / control block / pilot plan | 009A217B, 009A2229, 009A218E, 009A222C |
| approach+2Ch | observer element (vtable 00CF5C20, then 00CF5C94); +14h = approach+40h the ship | 009A1D8F, 009A1D94 |
| approach+44h | dropOrdnance byte | 009A1DA6, "dropOrdnance" 00D1F484 |
| approach+48h | stay-close altitude = ctl+398h | 009A1DB6 |
| approach+4Ch | beginDist = approach+24h * 1500.0 | 009A1D9D, "beginDist" 00D1F478 |
| approach+50h | state registry | 009A253C |
| approach+64h / +A0h / +138h | moveto / follow / stayclose states | 009A1B70 strings |
| stayclose+18h / +1Ch | release delay / release latch | 009A1F59, 009A1FE1 |

## Functions

Every function below is `reconstructed_build_tested` in `src/cc13_seg67a.cpp`
except 009A2D60 (existing implementation, ledger only).

| address | function | ABI | notes |
|---|---|---|---|
| 009A1B70 | close_to_ship_register_states_009a1b70 | thiscall(approach), RET | Unreferenced duplicate of 009A2500's three 00411E70 adds. |
| 009A1BB0 | close_to_ship_follow_state_for_role_009a1bb0 | thiscall(task), EAX | 007B8AD0(unit) true -> task+45Ch, else task+498h. |
| 009A1C30 | close_to_ship_target_point_009a1c30 | thiscall(approach, out), RET 4, EAX=out | Approach vtable[0]. Ship, or the unit when no ship; 00414DB0 when +C8h is zero. |
| 009A1CA0 | point_observer_element_construct_009a1ca0 | thiscall(element, observed), RET 4 | +4/+8/+Ch = 0, vtable 00CF5C20, +14h = observed, +10h = 1, then 00694A60(observed, element) when non-null. |
| 009A1D60 | close_to_ship_approach_construct_009a1d60 | thiscall(approach, owner, ship), RET 8 | 0042E740 tuning +42Ch into 009F9CE0; vtable 00D1F470; element at +2Ch, vtable 00CF5C94; +44h = 0; +4Ch; +48h. |
| 009A1DC0 | close_to_ship_approach_construct_from_stream_009a1dc0 | thiscall(approach, owner, archive), RET 8 | Element built inline with no observed object; reads shipName (type 4 into a stack slot, then the 006952A0/00694A60 rebind), dropOrdnance (3), beginDist (2) through archive slot +10h; +48h from ctl. |
| 009A1F00 | close_to_ship_stay_close_construct_009a1f00 | thiscall(state, approach), RET 4 | +4, +Ch, +10h, +14h, +8, vtable 00D1F4A0, +1Ch = 0, +18h = 00BD2F10(1, 1.0, 2.0). |
| 009A1FC0 | close_to_ship_try_drop_ordnance_009a1fc0 | thiscall(state, distance, dt), RET 8, AL | See below. |
| 009A2170 | close_to_ship_stay_close_tick_009a2170 | thiscall(state, dt), RET 4 | See below. |
| 009A2440 | close_to_ship_approach_destruct_009a2440 | thiscall(approach) | 007B45F0(+138h), 006CDD70(+B8h), 007B45F0(+A0h), 007B65E0(+64h), 004116D0(+50h or null), vtable 00D1F470, 0064A610(+2Ch). |
| 009A2500 | close_to_ship_approach_block_construct_009a2500 | thiscall(approach, owner, ship), RET 8 | 009A1D60, 00411E20, vtables 00D1F4D0/00D1F4CC, 009C2AC0(state, approach, ship, alt, alt, beginDist), 009C2980(state, approach, 100.0), 009A1F00, three 00411E70. |
| 009A2610 | close_to_ship_approach_block_construct_from_stream_009a2610 | thiscall(approach, owner, archive), RET 8 | As 009A2500 over 009A1DC0; the moveto constructor gets approach+40h. |
| 009A2730 | close_to_ship_task_construct_009a2730 | thiscall(task, bot, ship), RET 8 | 0099C6F0(task, bot, 0); 009A2500(task+3F8h, bot+50h, ship); vtables 00D1F4E8/00D1F4DC/00D1F4D8; +550h = -1.0; role state into +310h and its vtable[+4]; 009F9980(approach, task). |
| 009A2800 | close_to_ship_robots_row_009a2800 | thiscall, RET | Slot +5Ch: MOV EAX,00F8A320. |
| 009A2890 | close_to_ship_slot10_009a2890 | thiscall, RET | Slot +10h: 3. |
| 009A28B0 | close_to_ship_command_class_009a28b0 | thiscall, RET | Slot +3Ch: 00E08FA8. |
| 009A28C0 | close_to_ship_slot30_009a28c0 | thiscall, RET | Slot +30h: AL = 1. |
| 009A2920 | close_to_ship_command_still_valid_009a2920 | thiscall(task), EAX | Slot +40h. 2 without ctl or with ctl slot +174h = 0; 0 when +174h (called again) is not 00E08FA8 or 00521EA0(ctl slot +178h(&scratch)) is not task+438h; else 1. |
| 009A29B0 | close_to_ship_set_state_009a29b0 | thiscall(task, state), RET 4 | Exit the old state (vtable[+8]) when non-null, store, enter (vtable[+4]); no null check on the new one. |
| 009A29F0 | close_to_ship_select_state_009a29f0 | thiscall(task, dt), RET 4 | No ship: role state (inline SetState). Ship and in moveto/follow: stayclose when 1500.0f > 2-D distance (FCOMIP/JBE, NaN stays). |
| 009A2B00 | close_to_ship_task_tick_009a2b00 | thiscall(task, dt), RET 4 | Slot +64h. ctl null: nothing. Else +440h = ctl+398h; 009BDE80(moveto, +440h, +440h, +444h); 009A2810(dt); 009A29F0(dt); state vtable[+Ch](dt). |
| 009A2B90 | close_to_ship_target_record_009a2b90 | thiscall(task, out), RET 4, EAX=out | Slot +0Ch: approach vtable[0](scratch) then out = {p0, p1, p2}, out+0Ch = 1 (written between p0 and p1). |
| 009A2C00 | close_to_ship_wait_value_009a2c00 | thiscall(task), ST0 | Slot +4Ch: in follow, tail-jump 009BE3E0(follow); else tail-jump 0099B720 (-1.0). |
| 009A2C50 | close_to_ship_approach_save_009a2c50 | thiscall(approach, archive), RET 4 | shipName (4, the pointer), dropOrdnance (3, byte), beginDist (2, float) through archive slot +0Ch. |
| 009A2CF0 | close_to_ship_task_save_009a2cf0 | thiscall(task, archive), RET 4 | Slot +60h. 0099D060, 009A2C50, 00411450(task+448h, task+30Ch, archive), dropStarted (2). |
| 009A2DF0 | close_to_ship_task_destruct_009a2df0 | thiscall(task) | 009A2440(task+3F8h or null), 007B4030(task). |
| 009A2E50 | close_to_ship_task_construct_from_stream_009a2e50 | thiscall(task, bot, archive), RET 8 | 0099C8B0(task, bot, archive, 0); 009A2610; vtables; 00411170; dropStarted read; 009F9980. |
| 009A2F40 | close_to_ship_make_009a2f40 | fastcall(ECX=bot, EDX=ship), EAX | 00BF681B(554h), then 009A2730 when non-null. |
| 009A2FB0 | close_to_ship_factory_create_009a2fb0 | thiscall(factory, bot, archive), RET 8 | 00BF681B(554h), then 009A2E50. Factory pointer unread. |
| 009A2D60 | (existing) bot_task_update_cruise_profile with kCloseToShipCruiseProfile, src/bot_tasks.cpp | thiscall(task), RET | Ledger only, see below. |

### 009A1FC0, the release decision

From the listing (the decompiler agrees):

- latched (+1Ch set): 007B9140(unit, 0); true returns 1, false falls to the reset.
- not latched, delay > 0 or NaN (COMISS 0,[+18h]; JC): delay -= dt, return 1.
- delay <= 0: if 450.0 (qword 00D1F4C0) > distance (FCOMIP/JA) release at once.
  Otherwise (NaN included) the cone: d = 00438B10(00414EB0(ship - unit),
  unit vtable[+50h]) (unit heading fetched first), |d| by the image's `x > 0 ? x
  : -0.0 - x`, release only when 1.0471976f (00D05AAC) > |d| (COMISS/JBE: NaN
  fails), else return 0 with the delay untouched.
- release: 007BBBA0(unit), latch = 1, delay = 00BD2F10(stream 1, 1.0, 2.0), return 1.
- reset: approach+44h = 0, 007C1D80(unit), latch = 0, return 0.

### 009A2170, the stay-close tick

1. Nothing when the unit or the control block is null.
2. Pitch: gain = 00419010(20, 0.2, 120, 1.0, |unit.y - approach+48h|) and
   009FB800(ECX = state+4, approach+48h, gain).
3. Speed: plan+2B4h = float(007C47F0(descriptor) * 1.2000000476837158),
   plan+2B0h = 0, plan+2D8h = 1.
4. No ship: plan+2CCh = 1, plan+2C4h = 0.0.
5. Ship: v = ship - unit (x, z); distance = 00414C60(v); bearing = 00414EB0(v);
   off = |00438B10(unit heading, bearing)| (AND 7FFFFFFFh); narrow =
   0.8726646304130554 > off (qword compare, NaN wide). If 009A1FC0(distance, dt)
   is false and distance >= (narrow ? 350 : 500) (or NaN), the tick swaps v's
   components through the image's sum trick, normalises with 004192E0 and
   steers with 009F9D90(ECX = state+4, ship + 50 * n). Otherwise step 4's writes.

The steer point is the ship's position plus 50 along the normalised vector
with its x and z exchanged (s = a + b, b' = s - b, a' = s - b', each rounded
at its float store); recorded as the listing computes it, not interpreted.

### Precision

Every chain is one x87 operation per float store. The three float-by-qword
products use qword constants that are float-representable (1500, 1.2f, 50), so
the exact product fits in 53 bits and is rounded once at the store: the result
is the same under the 24-bit and the 53-bit control word. Float sums are
computed in double and rounded at the store, which is correctly rounded in
either mode. No precision assumption is therefore needed for this family.

### RNG

Two 00BD2F10 draws, both stream 1 (ECX = 1), U(1.0, 2.0): 009A1F54 in the
stayclose constructor and 009A20FA after each release. Both are on the shared
process-wide generator.

## Existing implementation recorded (ledger only)

009A2D60 (task slot +54h) was read end to end and matches
`bot_task_update_cruise_profile` with `kCloseToShipCruiseProfile` in
`src/bot_tasks.cpp`: the empty base 0099B660 first, then under 007B8AD0 two
altitude channels, Pilot/CloseToShip/CruisingAlt (+424h) into ctl+394h behind
lock ctl+38Dh, current ctl+380h < 0 and one-shot ctl+3A9h, and DropAlt (+428h)
into ctl+398h behind ctl+38Ch, ctl+37Ch and ctl+3AAh; each write raises
ctl+3ADh; each one-shot is cleared whenever its lock is clear. The channel
offsets are supplied by that function's caller (the PilotAltitudeChannel
arguments); a host must pass (38Dh, 380h, 3A9h) for the cruise channel and
(38Ch, 37Ch, 3AAh) for the second. The image calls 0042E740 once per channel;
the existing code reads tuning through `game_tuning_float`.

## Skipped

- 009A2910 (approach vtable[+4]): an adjustor thunk (`SUB ECX,3F8h; JMP 009A2F20`)
  into the compiler's scalar deleting destructor; compiler-generated.
- 009A28D0 (moveto-or-follow predicate, no vtable slot found in 00D1F4E8): not
  located on a reachable path.
- The CG_* deleting destructors and adjustor thunks of the window: compiler
  generated, bound.

## Adoption notes

None. No cc9-hosted copy of any of these functions was found (git grep by
address, name and the shipName/dropOrdnance/beginDist/stayclose strings).

## Uncertainty

- EH frame states of the constructors are not modelled.
- The archive record's byte payload (type 3 write) carries stack residue in its
  upper three bytes in the image; zero here.
- 00521EA0, 006CDD70, 0064A610 and the archive slots are called, not read.
