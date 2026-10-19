# CC13_SEG67N: the segment-67 rows oct18b-nC left open

Packet cc13-seg67n (cc13 worker, 2026-10-18). Source: the 84 `open_rows` of
`reports/oct18b_nc_summary.json` (segment 67, 00996120-009F6060), which cc16's Codex lane oct18b-nC
skipped because the segment is cc13's. Grant: cc9 gameplay lead 2026-10-18 addendum (segment 67 on
the gameplay-segment terms): standalone code, nothing wired, no game runs.

Files: `include/bsp/cc13_seg67n.hpp`, `src/cc13_seg67n.cpp`. Every name is a hypothesis, not a
recovered symbol. Status words: **reconstructed** (new C++ in this packet, build-tested), **twin**
(byte-identical to an existing reconstructed body; ledger-only), **hosted** (an existing host or
source models it; verified against the listing, ledger-only), **compiler** (adjustor thunk or
scalar deleting destructor; bind-only), **skipped** (leased elsewhere or under review).

Evidence: the live Ghidra listing (`bsp.py ghidra disasm`) and disk bytes, each body read whole.
Class identities come from docs/BOT_TASKS.md ("The thirteen classes") and from the string that
follows each vtable (`immelmann` at 00D20468, `takeoff` at 00D21290). Slot numbers are byte offsets
into the 26-slot task interface.

## Skipped

- `009A7A40`: inside cc13-undef's leased range 009A76E0-009A7E7F.

The four 009E rows first skipped here (`009E49F0`, `009E4CA0`, `009E50D0`, `009E6060`) were not
leased by cc13-prop (`lease list`, 2026-10-18) and are reconstructed in the follow-up below.

## Follow-up: the 009E ship-AI command slots (reconstructed)

`ship_ai_state_command_009e49f0` / `_009e4ca0` / `_009e50d0` / `_009e6060`
(`BSP_ShipAiState218F0_Command` .. `BSP_ShipAiState21994_Command`): each is slot +24h of the
ship-AI state vtables 00D218F0, 00D21920, 00D2195C, 00D21994, body `MOV EAX,00E08F78; RET`
(the attackmove command object; same body as 009DB350/009DB5A0). Original ABI `__thiscall`,
ECX unused, result in EAX. Uncertainty: which state each vtable is was not traced.

## Batch A: plane bot-task slots (reconstructed)

ABI of every row: `__thiscall`, ECX = the object, no register inputs beyond ECX. No x87 arithmetic
in this batch: float work is FLD/FSTP copies of floats, `XORPS`/`MOVSS` zero stores, and `FLDZ`.
No RNG.

| address | slot / vtable | C++ | body |
| --- | --- | --- | --- |
| `0099A810` | pilot bot 00D1F348 +44h | `pilot_bot_head_task_slot08_0099a810` | count `bot+5Ch` != 0: tail `JMP` to the head task's vtable[+8h] (ECX = `*[bot+58h]`); else 0. RET. |
| `0099C2A0` | direct (from 007BBB55) | `bot_task_current_state_query18_0099c2a0` | state = `[task+310h]`; non-null and `state->vtable[+18h]()` -> EAX 1, else 0. RET. |
| `009A54B0` | depthcharge +48h | `bot_task_depth_charge_target_009a54b0` | EAX = `[task+48Ch]`. |
| `009A9960` | dogfight +2Ch | `bot_task_dogfight_hit_notice_009a9960` | float `task+4E0h` = 0; AL = 1; RET 4 (argument unread). |
| `009AA3B0` | dogfight +0Ch | `bot_task_dogfight_target_record_009aa3b0` | target `[task+4ACh]`, else `[task+4C4h]`; none -> byte `out+0Ch` = 0. Else `00414DB0` when byte `+C8h` is 0, copy `+FCh..+104h`, `out+0Ch` = 1. RET 4, EAX = out. |
| `009ABBC0` | slot 0 of 00D1FB00, 00D1FC60, 00D1FC68 | `bot_point_at_4c_009abbc0` | out = `ECX+4Ch..+54h`; RET 4, EAX = out. |
| `009AE260` | dropkamikaze +4Ch | `bot_task_drop_kamikaze_wait_value_009ae260` | state `+50Ch` or `+5C0h` -> `JMP 009BE3E0` on it; else `JMP 0099B720`. ST0. |
| `009AF2F0` | kamikaze +4Ch | `bot_task_kamikaze_wait_value_009af2f0` | state `+524h` -> `009BE3E0`; else `0099B720`. |
| `009C3550` | moveto / attackmove +4Ch | `bot_task_moveto_wait_value_009c3550` | state `+494h` -> `009BE3E0`; else `0099B720`. |
| `009CFA40` | takeoff +4Ch | `bot_task_takeoff_wait_value_009cfa40` | control `[task+404h]` non-null and `control+3D0h != [task+3FCh]` -> `FLDZ`; else `JMP 0099B720`. |
| `009B36C0` | land +24h | `bot_task_land_in_landing_states_009b36c0` | AL = state is `+5D8h` (begin), `+5F8h` (final) or `+620h` (park). |
| `009B9590` | stop +40h | `bot_task_stop_still_valid_009b9590` | `[task+2FCh]` null -> 2; else `0071BE40(obj->vtable[+114h]()) == 00E08F88` -> 1/0. |
| `009BA9E0` | immelmann +60h | `bot_task_immelmann_save_009ba9e0` | `0099D060(task, archive)`, then write `{0,"stage"}`, `{1, [task+3F8h]}`. |
| `009BAB70` | gain altitude +60h | `bot_task_gain_altitude_save_009bab70` | `0099D060`, write `{0,"target"}`, `{4, [task+3F8h]}`, write `{0,"alt"}`, `{2, float [task+3FCh]}`. |
| `009BAC30` | stop +60h | `bot_task_stop_save_009bac30` | `JMP 0099D060` (a Ghidra thunk; its name stays the target's). |
| `009BE140` | slot 0 of 00D209C4 | `move_to_target_entity_009be140` | EAX = `[ECX+10h]`, the entity of the move-to target `009BE0F0` builds. |

Constant slots (whole body `MOV EAX,imm; RET` or `XOR AL,AL` / `MOV AL,1; RET`):

| address | class, slot | value |
| --- | --- | --- |
| `009A54F0` | depthcharge +10h | 4 |
| `009A99F0` | dogfight +10h | 7 |
| `009ADDD0` | dropkamikaze +10h | 4 |
| `009AF070` | kamikaze +10h | 8 |
| `009B33B0` | land +10h | 2 |
| `009B7BC0` | levelbomb +10h | 4 |
| `009BA910` | immelmann +10h | 0Ah |
| `009BAA80` | gain altitude +10h | 0Ah |
| `009BAC50` | stop +10h | 0Bh |
| `009C3160` | moveto / attackmove +10h | 3 |
| `009CC3E0` | strafe +10h | 6 |
| `009CF9F0` | takeoff +10h | 1 |
| `009D3290` | torpedo +10h | 5 |
| `009BA8F0` | immelmann +5Ch | factory record `00F8A4E8` (static init 00CD4FA0) |
| `009BAA70` | gain altitude +5Ch | factory record `00F8A4D8` (static init 00CD4F70) |
| `009BAC20` | stop +5Ch | factory record `00F8A4F8` (static init 00CD4FD0) |
| `009C3140` | moveto / attackmove +5Ch | factory record `00F8A584` (static init Unwind@00CD5130) |
| `009BAC40` | stop +30h | false |
| `009CFA30` | takeoff +24h | true |

The slot +10h value is not the kind id at `task+300h` (stop is kind Fh but answers 0Bh); its
meaning is unread.

### Uncertainties (batch A)

- `0099A810`: the 149 slot-17 call candidates are slot-only matches; no caller of the pilot-bot
  slot +44h was traced.
- `0099C2A0`: the state slot +18h contract is unread (base `007B45E0` answers false). Its only
  caller, `007BBB30`, is undefined code with no known caller, and it can pass ECX = 0 (no task),
  which the body does not guard; nor does the reconstruction.
- `009B9590`: `00E08F88` is the command object the ship-AI `stop` state's slot +24h returns
  (`009DAC80`); reading it as "the stop command" is a hypothesis.
- `009CFA40`: `control+3D0h` read as the squadron leader by analogy with `0099B740`.
- `009ABBC0`: the owners of 00D1FB00 and 00D1FC60 were not traced.

## Batch B: twins, hosted rows and compiler bodies (ledger-only)

Twins of `009A2B90` (`close_to_ship_target_record_009a2b90`, src/cc13_seg67a.cpp), slot +0Ch,
`__thiscall(ECX = task, out)`, RET 4, EAX = out. Each body is the same 55-byte sequence (`RET 4` at +34h; 009A2B90-009A2BC6)
`MOV EAX,[ECX+3F8h]; MOV EAX,[EAX]; ADD ECX,3F8h; SUB ESP,0Ch; LEA EDX,[ESP]; PUSH EDX; CALL EAX`,
then FLD/FSTP copies of the returned point into out+0/+4/+8 with byte out+0Ch = 1:
`009A5F10` (depthcharge), `009AE990` (dropkamikaze), `009AF5D0` (kamikaze), `009B36F0` (land),
`009B8C50` (levelbomb), `009CCE90` (strafe), `009D4A30` (torpedo).

Hosted, verified against the listing:

| address | slot | host | verdict |
| --- | --- | --- | --- |
| `009B3730` | land +30h | game_hosts_units.cpp ground re-takeoff head check | state `+620h` (park) or `+5F8h` (final) -> false: matches |
| `009C7900` | divebomb +2Ch | `plane_hit_task_notify_00999aa0` | `task+4BCh` = 0, true: matches |
| `009D3270` | torpedo +2Ch | `plane_hit_task_notify_00999aa0` | `task+52Ch` = 0, true: matches |
| `009CF9D0` | takeoff +30h | ground re-takeoff head check | false: matches |
| `009C32A0` | moveto / attackmove +58h | `kMoveToLegAltitudeBound` drop | partial, see the adoption note |

Compiler multiple-inheritance adjustors (`SUB ECX,n; JMP <CG_scalar_deleting_dtor>`), bind-only,
`library_instantiation_out_of_scope`: `009A2600` (-50h, 009A2710), `009A2910` (-3F8h, 009A2F20),
`009A4FF0` (-FCh, 009A5220), `009AD910` (-C4h, 009AF870), `009ADDF0` (-4BCh, 009AF920), `009AEEC0`
(-DCh, 009AEED0), `009AF090` (-4D4h, 009AF700), `009B7C80` (-4D4h, 009B9010), `009C9E80` (-464h,
009CA290), `009CC200` (-D4h, 009CC210), `009CC5A0` (-4CCh, 009CD2E0), `009CF840` (-3Ch, 009D0030),
`009F3CB0` (-58h, 009F3F00). The tertiary-view offsets match docs/BOT_TASKS.md (dropkamikaze
+4BCh, kamikaze and levelbomb +4D4h, retreat +464h, strafe +4CCh).

### Adoption note: the moveto release (009C32A0)

Slot +58h of moveto (00D20B68) and attackmove (00D20BE0): `009C32B8` calls
`009FD050(ECX = task+3F8h, -1.0f [00D7A260], 00F87574)`, then, with a current state at
`task+310h`, tail-jumps to that state's exit (`vtable+8h`). The host
(`kMoveToLegAltitudeBound`, game_hosts_units.cpp) drops the leg record on the next think after
`moveto_task_installed` clears, so the `009FD050(-1)` half is covered (LABELLED timing). The
current state's exit at release is not modelled there. Not wired; LIVE in a normal mission
whenever a plane's moveto or attackmove task is released. Shared RNG: not involved.

The caller is `00999EE0` (drains the bot+64h retired list filled by `0099A660`;
`00999EF7: mov eax,[edx+58h]; call eax`). Attackmove's constructor `009C3B10` calls the moveto
constructor `009C3000`, so both tasks own the same three states, and only these can be current at
release:

| state (task offset) | vtable | exit (`+8h`) | what the exit does |
| --- | --- | --- | --- |
| moveto (+47Ch) | 00D20A80 | `007B3DC0` | bare `RET` |
| circle (+52Ch) | 00D20A9C | `007B3DC0` | bare `RET` |
| follow (+494h) | 00D20AB8 | `009BDE40` | `007B8A90(unit)` clears the turbo byte unit+9F9h; stores 0.0 to `[[state+4]+1Ch]+4Ch`, the gun hold timer |

No exit issues an order or draws from `00BD2F10`. Only the follow exit has an effect.

Where the exit's gun store lands. The follow state's +4 is the moveto approach: task+3F8h, with the
follow state at approach+9Ch (`009C2E73`). Approach +4h is the unit (`009F9CEA`). Approach +1Ch is
the retiring task's own gun block, task+314h: `009F9980`, called from `009C30CF`, stores it at
`009F999D`. Right after slot +58h, `00999EE0` deletes the task (`00999F0B`, vtable+0 with flag 1).
The exit's +4Ch = 0.0 therefore lands on a block that is freed a moment later.

Every replacement task builds its own gun block: both base constructors call `009FAAD0` on
task+314h (`0099C790`, `0099C950`), and +4Ch = 0.0 (`009FAB19` XORPS, `009FAB4B`).

Order in the image:

1. `0099A4C0` (bot tick, `0099AE7E`) moves the head to the retired list +64h.
2. In the same tick, the new head's +54h start runs, then its update `009998A0` (gun tick
   `00999979`) runs on the new block.
3. At the next tick's drain (`0099ADA3`/`0099ADC2`), `00999EE0` runs `009C32A0`, which:
   1. drops the leg via `009FD050(-1.0)`;
   2. JMPs to the state exit;
   3. returns to `00999EFC`, which deletes the task.

Nothing frees or resets the state or approach+1Ch before the exit.

Host (game_hosts_units.cpp): one `df_gun` per unit lives for the slot's life (1155), and no install
re-runs `009FAAD0`. `follow_exit_009bde40` (6596) is reached only from the in-task state switch and
the exit_follow override. The moveto/attackmove release sites (9655, 14247, 14957, 17720, 17869,
17904) clear the state without it. All six are `0099A4C0` retires, so in the image each reaches
`00999EE0` -> `009C32A0`.

Visible difference after any task change:

- `df_gun.hold_4c` keeps the follow sight's 2.0 (9487, `009C232A`). The image's new task starts
  at 0.0.
- In the gun tick, a hold above 0 forces the envelope: `009FCC33` JA -> `009FCC9E` BL = 1, which
  skips the range and cone tests. `009FCCB4` (units.cpp:24330) skips only the fine-aim
  correction, and the path then fires at `009FCDB4` (host dogfight_task.cpp:286-296).
- So a stale hold makes the next task's guns FIRE without aim correction whenever a target is in
  the gun range. This lasts up to 2.0 s, decayed by dt (`009FC7D0`; dogfight_task.cpp:272).
- `plane_turbo_9f9` is transient: the next pilot commit rewrites it from `plan_turbo_2e5`
  (37514), and `0099B572` resets that value every think (37113).

LIVE? yes, after a release in a follow state. For moveto/attackmove this means a wingman's override
or end (17869, 17904, 14957). The other follow-state tasks (depthcharge, dropkamikaze, kamikaze,
land, levelbomb, all of which build `009C2980`) behave the same way. RNG? not directly.

## Batch C: maneuver ticks, the follow debug draw, the ship AI states

| address | slot / vtable | C++ | notes |
| --- | --- | --- | --- |
| `009B9AF0` | immelmann 00D20400 +64h | `bot_task_immelmann_tick_009b9af0` | three-stage pitch-up; FCOS; retires via `0099B690` |
| `009B9CA0` | split s 00D207C0 +64h | `bot_task_split_s_tick_009b9ca0` | calls `009B94E0` at `009B9D05`; `00438B10` at `009B9DCC`; retires via `0099B690` at `009B9D64` |
| `009BE590` | follow-state +14h (six vtables) | `bot_state_follow_debug_draw_009be590` | `0085F990` x3, `00860710`, `00860BE0` x2, `004C7070`, `00419510` |
| `009DAA60` | ship-AI state +18h (15 vtables) | `ship_ai_state_debug_draw_noop_009daa60` | `RET 8` |
| `009DAF60` | ship-AI land 00D21658 +18h | `ship_ai_land_debug_draw_009daf60` | `0085FFF0` at `009DAFEC`, yellow, unit to `state+18h` |
| `009DAC80` | stop 00D215C8 +24h | `ship_ai_stop_command_009dac80` | `00E08F88` |
| `009DADB0` | follow 00D215F8 +24h | `ship_ai_follow_command_009dadb0` | `00E08F60` |
| `009DAF50` | land 00D21658 +24h | `ship_ai_land_command_009daf50` | `00E08FA0` |
| `009DB050` | moveonpath 00D21688 +24h | `ship_ai_moveonpath_command_009db050` | `00E08F80` |
| `009DB350` | kamikaze_attack 00D216B8 +24h | `ship_ai_kamikaze_attack_command_009db350` | `00E08F78` (attackmove) |
| `009DB5A0`, `009DB610`, `009DB690`, `009DB7E0` | sub-states 00D2171C, 00D2174C, 00D2177C, 00D217AC +24h | `ship_ai_attack_substate_command_*` | `00E08F78` |

Hosted (ledger-only, matches): `009DAC70` (stop enter, byte +8 = 1) and `009DB040` (moveonpath
enter, byte +8 = 0) in game_hosts_ship_ai.cpp (`kShipAiStateEnterBytesBound`); `009DB310`
(kamikaze_attack +20h, true) in `active_state_wants_periscope_0020`; `009F3CE0` (controller +48h,
`JMP [[ai+2264h]+2Ch]`) in `GameShipAiHost::goal_reached_009da590`. Compiler: `009DF1C0`, the stop
state's scalar deleting destructor (base vtable 00D056D0 store, `00BF65AC` on flags bit 0).

The immelmann tick (009B9AF0), the split s tick (009B9CA0), and the batch-A sweeps:

- **24-bit precision.** The x87 work is modelled at 24-bit precision (the alternative is 53-bit).
  The immelmann product and difference each round to float.
- **The +2E4h mask.** Both ticks clear `task+2E4h` (avoidFlags) with `AND dword, 000000FBh`, the
  imm32 form `81 A6 E4 02 00 00 FB 00 00 00`. That clears bits 8-31 as well as bit 2.
- **Unordered compares.** A NaN falls through every COMISS/FCOMIP/JBE test. In the immelmann,
  stage 0 then leaves the early return and sets stage 1. In the split s, `|r|` is `-0.0 - r`
  for a NaN or for 0.

### Finding: the avoidFlags mask in cc13_seg67b (not this packet's rows)

`bot_task_barrel_roll_tick_009b9740` and `bot_task_gain_altitude_tick_009ba2f0`
(src/cc13_seg67b.cpp) write `task+2E4h &= 0xFFFFFFFBu`. The image's bytes at `009B9752` and
`009BA300` are also `81 A6 E4 02 00 00 FB 00 00 00`, so they must be `&= 0xFBu`.

FIXED in the follow-up commit (both ticks now `&= 0xFBu`; ledger evidence carries the
correction). Every image clear of +2E4h uses that imm32 form: 009B9752, 009B9C86, 009B9D2D,
009B9D69, 009B9E29, 009BA1CB, 009BA300. No host file applies a mask to +2E4h; the only mention is
the comment at game_hosts_units.cpp:25720 ("009BA1C8's +2E4h &= ~4 is not modelled").

Does it matter? No: it is fidelity-only, with no gameplay effect.

The proof is the per-tick reset. Every think, `009998A0` stores the whole dword just before the
task arm `CALL [vt+64h]`:

- `009998F1`: `MOV dword [ESI+2E4h], 0`, then `009998FB` CALL.
- `00999950`: `MOV dword [ESI+2E4h], 0FFh`, then `0099995A` CALL.

So bits 8-31 are zero whenever an arm runs, whatever a load or an earlier tick left there.

Census (task classes only, instruction-level `+ 0x2e4]` operands):

- Other dword stores of 0/0FFh: 009A1A08, 009CD1D8, 007B7CA8.
- Byte stores: 0099B3F4, 0099B56C, 009A4F14, 009A513D.
- Copies of approach fields holding 0FFh/1/3: 009A6750, 009AB20A, 009AE92D, 009AF550, 009B3F41,
  009B8BDD, 009C885B, 009CFDBA, 009D4905.
- Readers are byte reads (0099BF18, 009A18BA, 009A1951, 009A1974). The only dword reader is the
  save at 0099D060; the load passes the address at 0099C9B1.
- A different layout, excluded: 009B774C, 009B7764, 009B7906 and 009B791E access +2E4h of the
  level-bomb approach subobject's owner pointer inside 009B75E0, not a task's avoidFlags.

Not censused: REP MOVSD whole-task copies, which xref only a base.

### Uncertainties (batch C)

- **FCOS precision.** FCOS runs at extended precision. `std::cos` followed by the float store can
  differ by one ulp.
- **Field roles.** The unit fields `+B50h`/`+C64h`/`+C68h` are read as speed, pitch and bank from
  their use only.
- **Static colours.** The follow debug draw passes the guarded static colours as constants.
- **Slot +18h contract.** The ship-AI slot +18h is read as "debug draw" only from the land
  override.
- **Sub-state identities.** The sub-state identities of 00D2174C, 00D2177C and 00D217AC were not
  traced.

### Adoption note: the dogfight hit notice (009A9960)

`GameUnitsHost::plane_hit_task_notify_00999aa0` (src/game_hosts_units.cpp) models the image's
walk `00999AA0` over torpedo (`009D3270`), divebomb (`009C7900`) and strafe (`009CC400`) only. The
dogfight task answers true too: `009A9963` zeroes `task+4E0h` (approach+E8h). A dogfight task ahead
of another task in the list therefore takes the notice in the image and stops the walk. No
behaviour reads approach+E8h: the disk-byte scan of 009A6A00-009AB338 finds only the constructor
store (`009A98D3`), the per-tick accumulate (`009AACC3`/`009AACC9`) and the save (`009AA520`). Not
gameplay-relevant; ledger and note only, nothing wired. Shared RNG 00BD2F10: not involved.
