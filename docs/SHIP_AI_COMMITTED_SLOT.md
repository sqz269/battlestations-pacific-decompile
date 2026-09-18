# `nested+11E8h`: not a committed ring winner

Addresses: `009F1BC0` (the store at `009F28B4`..`009F28F1`), `009E5530` (the seed at `009E561F`),
`009E5E90` (the read at `009E5ED1`), `009E76D0`, `00BF7420`. Constants `00D1A8A0` (pi/60),
`00CE3828` (2*pi), `00CE3D68` (60.0).

## The answer in one line

There is no missing commit. `nested+11E8h` is **the ring slot the unit's own heading falls in**,
recomputed from scratch at the tail of every frame-state pass, and `009E5E90` reads it only to ask
whether the bearing it is about to publish lies in that same slot. The field was never a latched
winner, so there is no hysteresis and no reject threshold behind it. What was missing was the
recomputation, and the previous packet's reading of the field as "the winner" was wrong.

## The byte census

Over `.text`, for the displacement `E8 11 00 00`:

| form | pattern | hits |
| --- | --- | --- |
| `MOV [reg+disp32], r32` | `89 ?? E8 11 00 00` | `009E561F`, `009F28F1` |
| the same with a SIB byte | `89 ?? ?? E8 11 00 00` | none |
| `MOV [reg+disp32], imm32` | `C7 ?? E8 11 00 00`, `C7 ?? ?? E8 11 00 00` | none |
| byte / word store | `88 ?? …`, `66 89 ?? …` | none |
| `MOVSS [reg+disp32]` | `F3 0F 11 ?? E8 11 00 00` | `00853814`, `00854AA8`, both `MSubmarine` fields of a different object |
| read | `8B ?? E8 11 00 00` | `009E5ED1`, the positive control |
| `INC` / `ADD` / `CMP` | `FF ??`, `83 ??`, `39 ??`, `3B ??` | none |

So two writers and one reader. A block copy over the whole nested object would not appear in this
census and is the one gap.

## `009E561F`, the seed

`MOV [EDI+11E8h],EBX` in `009E5530 BSP_ShipAi_AttackMoveRingConstruct`, among a run of zeroing
stores. `EBX` is zeroed by `XOR EBX,EBX` at `009E554D` and is not touched again until `009E5747`,
so the constructor seeds the field with **0**.

## `009F28F1`, the per-frame writer

```
009F28B4  FLD    [EBP+11ECh]         ; the heading 009F1C26 set this frame
009F28BA  FADD   qword [00D1A8A0]    ; + pi/60, half a slot
009F28C4  FLD [ESP+14h] / FLDZ / FCOMIP
009F28D2  JBE    009F28E2            ; a non-negative sum skips the wrap
009F28D4  FADD   ST1,ST0             ; + 2*pi from 00CE3828
009F28E4  FMUL   qword [00CE3D68]    ; * 60
009F28EA  FDIVRP                     ; / 2*pi
009F28EC  CALL   00BF7420            ; _ftol2, truncating toward zero
009F28F1  MOV    [EBP+11E8h],EAX
```

This is instruction for instruction the arithmetic `009E5E90` runs at `009E5EA4`..`009E5ECC` on the
bearing it is handed, which the reconstruction already had as
`ship_ai_approach_slot_of_bearing_009e5e90`. Applied here to `nested+11ECh`, it yields the slot of
the unit's own heading.

Three branches at `009F2834`, `009F283D` and `009F2842` jump **to** `009F28B4`, so the block is a
join, not a guarded arm. No branch before the store targets anything past it and no `RET` precedes
it, so the store runs on every call to `009F1BC0`.

`nested+11ECh` is written exactly once per frame, by `FSTP [EBP+11ECh]` at `009F1C26` (byte census
`D9 9? EC 11 00 00`: that is the only store form hit; `009E5EE6`, `009E5F06`, `009E5FDA`,
`009E601C`, `009E6042`, `009E75E2` and `009E7E04` are all `FLD` reads).

## What the commit does not write

Nothing. `nested+1200h` is written only by the evade scorer `009E74D0` (`009E7566`, `009E7571`,
`009E75C4`) and by the constructor at `009E564F`; `nested+11ECh` by `009F1C26`. There is no reject
threshold, no mode test and no hysteresis on the path to `009F28F1`: `tune+4h` at `009E784B`
belongs to the slot reject pass inside `009E76D0` and never reaches this field.

The chain order settles the rest. `009F309B` calls `009F1BC0`, `009F30C7` the evade scorer and
`009F30D6` the slot selection, so the heading slot is refreshed **before** the ring scan that reads
it back.

## Host methods

`src/game_hosts_ship_ai.cpp`, `ApproachBinding`:

| method | native | change |
| --- | --- | --- |
| `ShipAiApproach::frame_state` | `009F1BC0` | the projection now ends with the `009F28F1` store; the census takes the committed slot here |
| `ShipAiApproach::select_slot` | `009E76D0` | takes the winner the routine now returns, and the five score words of slot 0 |

`ship_ai_approach_select_slot_009e76d0` returns `int` instead of `void`. The native keeps its
winner in a register and reads only its bearing at `009E7C1C`; returning it is the only way a
census can see it, because `nested+11E8h` does not hold it.

## Corrections

Appended to `docs/SHIP_AI_APPROACH_UPDATE.md` and `docs/SHIP_AI_RING_SCAN.md`. The previous
packet's claim in `docs/SHIP_AI_GOAL_VECTOR_VISIBILITY.md` that "`009E5E90` writes `nested+120Ch`
but never `nested+11E8h`" is right about `009E5E90` and wrong about the conclusion drawn from it:
the writer is `009F1BC0`, and the field is not a winner.

## no_ghidra_function

none.

## Validation

Build `scripts/build.ps1`, Win32 Release, `/W4 /WX`, clean. Tests 2 of 2. USN02, 3000 mission
frames.

| number | before (this branch, previous packet) | after |
| --- | --- | --- |
| committed slot first / last (Haguro) | 0 / 0, never written | 30 / 28 |
| committed-slot changes over 3000 frames | none possible | 2 to 4 per ship |
| ring winner first / last | not reported | 0 / 0, measured |
| heading_changes (Haguro / Jintsu / Yudachi / Samidare) | 545 / 572 / 535 / 578 | 532 / 572 / 524 / 578 |
| standoff | 1450 / 1550 / 200 | 1450 / 1550 / 200 |
| shots / hull / deaths / damage | 853 / 119 / 3 / 12463.2 | 853 / 119 / 3 / 12463.2 |

**Attribution.** The gunnery census does not move at all, and that is the correct outcome: the
only consumer of `nested+11E8h` is the arm choice inside `009E5E90`, which decides whether the
commanded heading turns clockwise or counter-clockwise to reach the same bearing. That changes
`nested+120Ch`, which is why `heading_changes` shifts by about ten on two of the four ships, and
nothing further, because `ShipAiOrder::slot_to_order_ring` `00825F7C` is still unimplemented and
the commanded heading never reaches the rudder. Ship positions are identical, which is why the
committed slots, the standoffs and every gunnery number are identical.

The committed slots themselves say the ships barely turn: each holds a heading inside two to four
of the sixty slots for the whole mission.

USN01 for the record: clean run, `hull=23 deaths=1 total_damage=220.0`.

Re-measured after merging `main` at `2cee62395` into this branch, which brought in the torpedo,
native-lifetime and frame-slot work of the same day: build clean, tests 2 of 2, and USN02
reproduces every number above exactly, including `committed_first=30 committed_last=28
heading_changes=532` on Haguro and `pen_30=-nan(ind)` with the other five words finite.

## The next input, by address and value

The ring winner is genuinely slot 0 on every scan, and the census now says why:

```
slots Haguro   commits=2 winner_first=0 winner_last=0 total_best=-nan(ind) total_worst=-nan(ind)
words Haguro   raw_18=6700.0000 norm_2c=10.0000 pen_30=-nan(ind) bear_34=0.0733 evade_38=0.2500 avoid_3c=0.0000
```

**`slot+30h` is NaN**, on every ship. The other five words are finite and plausible. `009E7BE0`
sums all five, so every slot's total is NaN, the strict `FCOMIP` / `JBE` at `009E7BFA` is never
taken on an unordered pair, and `best_slot` keeps the seed `0` from `009E79CA`.

The NaN does not come from the reject arm at `009E784B`, whose `tune+4h` is the finite 4.0 that
`src/ship_ai_approach_tune.cpp` reads from `0081F220`. It comes from the arc scorer: slot `+30h`
is written by `ship_ai_ring_scan_arc_score_009e6870` at `src/ship_ai_ring_scan.cpp:152`, native
`009E6870`, one of the three scorers the goal-vector packet switched on. Two candidate producers
inside it, both untested divisions: the `009E7795` normalisation divides every slot score by the
maximum without testing it, and the arc score has its own span divisor.

## Follow-up packets

- `ship_ai_ring_arc_score_nan`: `009E6870`, why slot `+30h` is NaN. It is now the only thing
  between the ring scan and a winner that is not slot 0.
- `ship_ai_order_ring_slot`: `00825F7C` `ShipAiOrder::slot_to_order_ring`, 96000 calls and still
  unimplemented. Until it runs, `nested+120Ch` has no consumer and no ring result can reach the
  rudder.
