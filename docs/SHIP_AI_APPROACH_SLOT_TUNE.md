# The tune block the ship AI's ring scorer reads

Addresses: 009F1160, 009F11AA, 0081ED40, 0081F200, 0081F28C, 009E6EC5, 009E7489, 009E784B,
009E81FA, 009E964E, 009E75F2, 009E5D8C, 00CE38B8, 00CE3D34, 00CE3854, 00CE3804, 00D7A24C,
00CE3868, 00D05AAC, 00D7A260

Packet `cc8_ship_ai_approach_slot_tune`, worker `agent/cc8-ship-approach-curves`. Project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; Ghidra was **read-only**. Every
descriptive name below is a hypothesis, not a recovered symbol.
`reports/ship_ai_approach_slot_tune.json` carries the rows. It continues
`docs/SHIP_AI_FIREPOWER_INPUTS.md`, whose one named follow-up it answers, though not with the
answer that follow-up predicted.

## `brain+0AB0h` is a pointer, and the block belongs to the unit

Every reader takes two steps. `009E6EBD MOV ECX,[ESI]` loads the brain from `nested+0h`,
`009E6EBF MOV EAX,[ECX+0AB0h]` loads the block pointer, and `009E6EC5 MOVSS XMM0,[EAX+1Ch]` reads
the field. `009E5D8C` in the approach constructor does the same through `[EDX+0AB0h]`.

There is exactly **one** store to `brain+0AB0h` in the image. A wildcard scan for
`89 ?? b0 0a 00 00` over `.text` returns a single hit, `009F11AA` inside
`BSP_ShipAi_BrainRecordConstruct`; the immediate form `c7 ?? b0 0a 00 00` returns none. The plain
displacement `b0 0a 00 00` occurs in dozens of other functions, which is the positive control that
makes the single hit meaningful rather than a scan that simply found nothing.

```
009F1198  MOV EAX,[EDI+538h]      the unit sub-object
009F119E  MOV [ESI+0AB0h-4],EAX   brain+0AACh
009F11A4  MOV EAX,[EDI+73Ch]
009F11AA  MOV [ESI+0AB0h],EAX     brain+0AB0h
```

So the block is `[unit+73Ch]`. It belongs to the unit, not the brain, and it outlives any brain
record.

## What fills it: ten compiled-in constants

`[unit+73Ch]` is written once, by `BSP_UnitVehicleBase_Construct`. The allocator at `0081F200`
returns the block, `0081F214..0081F27D` writes ten immediates into it from `.rdata`, and `0081F28C`
stores the pointer. The failed-allocation arm at `0081F283` sets `EAX` to zero and leaves the
pointer null, which every reader would fault on, so the image treats the block as always present.

| offset | value | constant | store | first reader |
| --- | --- | --- | --- | --- |
| `+00h` | `10.0` | `00CE38B8` | `0081F21C` | `009E81FA`, the slot score's scale |
| `+04h` | `4.0` | `00CE3D34` | `0081F228` | `009E7489`, the scorer's weight |
| `+08h` | `3.0` | `00CE3854` | `0081F235` | `009E964E`, the avoidance strength |
| `+0Ch` | `1000.0` | `00CE3804` | `0081F25C` | the avoidance span beside it |
| `+10h` | `1.0` | `00D7A24C` | `0081F269` | `009E75F2`, the evade bearing |
| `+14h` | `0.25` | `00CE3868` | `0081F242` | the evade weight |
| `+18h` | `1.0471976` | `00D05AAC` | `0081F24F` | the evade span, `pi/3` |
| `+1Ch` | `-1.0` | `00D7A260` | `0081F26E` | `009E6EC5`, the standoff range override |

`+24h` and `+28h` take the same `-1.0`, and a byte `1` goes to `+21h`.

**There is no Lua key and no string.** The values are immediates in the constructor, not rows from
`ShipGlobals` or any other table, which is why no loader on main produces them and why every unit
in every mission gets the same block. Each name in the table above is therefore a hypothesis taken
from the **reader**, not recovered from the producer.

`+1Ch` is worth singling out. `009E6ECA` compares it against the `0.0f` at `00D7A218` and, when it
is at or above zero, takes it verbatim as the standoff range and skips every other arm including
the 119-step curve scan. The installed `-1.0f` is what keeps that scan alive. The host had already
been returning `-1.0f` there as a guess; it is now sourced.

## Host methods

`src/game_hosts_ship_ai.cpp`. One shared `bsp::ShipAiApproachTune` on the host holds the block,
because the image writes the same ten constants into every unit's copy.

| method | binding | site | value |
| --- | --- | --- | --- |
| `tune_scale_00` | ScoreReset | `009E81FA` | `10.0` |
| `tune_slot_04` | Standoff | `009E7489` | `4.0` |
| `tune_reject_penalty_04` | RingScan and Select | `009E784B` | `4.0` |
| `tune_avoid_strength_08` / `tune_avoid_span_0c` | Avoid | `009E964E` | `3.0` / `1000.0` |
| `tune_bearing_10` / `tune_evade_14` / `tune_evade_span_18` | Evade | `009E75F2` | `1.0` / `0.25` / `pi/3` |
| `tune_range_override_1c` | Standoff | `009E6EC5` | `-1.0` |

## Validation

Build: `scripts/build.ps1`, Win32 Release, `/W4 /WX`, clean. Tests: 2 of 2 passing.

USN02, 3000 mission frames at 0.05 s, against the firepower packet's after-run:

| | before | after |
| --- | --- | --- |
| winning slot, first and last | 0 | 0 |
| standoff, Haguro / Jintsu / destroyers | 1450 / 1550 / 200 | unchanged |
| gunnery | `shots=734 hull=180 deaths=2 total_damage=18525.6` | identical |
| aim | `angle_sets=1169733 refusals=185667 shots=734` | identical |

The new `heading_changes` counter is new in this build, so it has no before value. To attribute
it, a control build with all eight tune values forced to zero, run on the same mission and settings, produces a byte-identical census: the same heading_changes of 78, 67, 94 and 83 for Haguro, Jintsu, Yudachi and Samidare, the same standoff ranges, the same slot 0 winner and the same shots=734 hull=180 deaths=2 total_damage=18525.6. The heading does move during a run, but not because of anything in this block.

**The tune block reaches the scorers and changes nothing a census can see**, and the packet can say
why for each half.

The standoff range is unchanged because `tune+1Ch` was already `-1.0f` in the host before this
packet. That is the one tune field the standoff choice reads, and its installed value is precisely
the one that does nothing: it fails the `>= 0.0f` test at `009E6ECA`, so the override arm was never
taken before either.

The winning slot is unchanged because the scores the tune block scales are themselves not produced.
`009E7FC0` writes `slot+2Ch = slot+18h / running_max * tune+0h`, and `slot+18h` comes from the four
per-slot scorers whose bodies this process still answers with neutral values. Multiplying a tie by
`10.0` leaves a tie, and `009E76D0` keeps slot 0.

## Corrections

Appended to `docs/SHIP_AI_APPROACH_UPDATE.md` (the tune vocabulary: `brain+0AB0h` is a pointer to a
block that belongs to the unit) and to `docs/SHIP_AI_FIREPOWER_INPUTS.md` (its follow-up predicted
that binding the tune block would move the headings; it does not).

## no_ghidra_function

none. `009F1160` and `0081ED40` are both existing Ghidra functions, and the fill at `0081F200` is
inside the second.

## Corrections from packet cc8_ship_ai_approach_slot_scorers

Appended, not a rewrite.

| was | is | evidence |
| --- | --- | --- |
| Follow-up `ship_ai_approach_slot_scorers`: "the real blocker ... until `slot+18h` has a producer, every slot ties and slot 0 wins". | `slot+18h` has had a producer since `cc8_ship_ai_firepower_inputs`; the adapter that would fill it is never called. `009E7FC0` returns at `009E80B0`, its first mode-0 gate, before it scores a single slot. The four scorers were already whole on main and needed no work. | `firepower=0` against `ring_scans=8400` in every run of this chain. The gate census in `docs/SHIP_AI_APPROACH_SLOT_SCORERS.md` reports `flag_0b28=0` with `flag_stops=600` of 600 for all fourteen ships. |

## Follow-up packets

- `ship_ai_approach_slot_scorers`: **the real blocker.** `009E5DA0`, `009E6400`, `009E6870` and
  `009E6640`, the four per-slot routines that fill `slot+18h`, `slot+2Ch`, `slot+3Ch` and the
  blocked byte at `slot+40h`. Until `slot+18h` has a producer, every slot ties and slot 0 wins
  whatever the tune block says. This is the third input in a row that turned out to sit behind the
  one before it, and it is the one the whole chain now waits on.
- `ship_ai_approach_tune_overrides`: whether anything writes a field through `[unit+73Ch]` after
  the constructor, and what the block's extent past `+28h` is. This packet found the one producer
  of the pointer and the one producer of its contents; it did not enumerate writers through the
  pointer.
- Land units take a different path to `unit+73Ch`: `BSP_LandConvoy_AttachAndBuildRoster`,
  `BSP_LandFort_Construct` and two unnamed routines also store there, and none was read.
