# The goal vector's "target visible" gate, `brain+0B28h`

Addresses: `009F1420` (the narrowing at `009F14D2`..`009F1522`), `008053C0`, `009DFBE0`,
`00922DC0`, `00922C80`, `00922B70`, `00922B90`, `00424C40`, `00427EB0`, `008DDF90`.
Constants `00D7A348` (0.25), `00CE3938` (50.0), settings `+49Ch`.

## The answer in one line

`brain+0B28h` was never true for a reason that is not in `009F1420` at all: both of the narrowing
tests the routine calls were **recorded refusals** in the host, and the flag opens only if one of
them answers true. Both are answerable. With them answered the flag is true on **600 of 600**
scans for all fourteen ships, and the ring scan four packets had been feeding runs for the first
time.

The recon arm alone carries it. `surface=0` in every run: `00922DC0` is never consulted because
`009DFBE0` has already opened the gate.

## The gate, instruction by instruction

The listing at `009F14D0`..`009F1529`:

```
009F14D2  MOV byte ptr [EDI+0B28h],1          ; open
009F14D9  JZ 009F1529                         ; no filtered target: stays open
009F14DB  MOV EAX,[EAX+54h]                   ; target side
009F14DE  MOV ECX,[EDI+0AA8h]                 ; the owner unit
009F14E4  CMP EAX,[ECX+54h]                   ; same side?
009F14E7  JZ 009F150A                         ; yes: stays open
009F14E9  MOV ECX,[ECX+54h]                   ; own side, in ECX
009F14EC  CALL 008053C0                       ; the side's recon slot
009F14F1  MOV ECX,[EDI+0B24h]                 ; the FILTERED target
009F14F7  PUSH ECX
009F14F8  MOV ECX,EAX
009F14FA  CALL 009DFBE0
009F14FF  TEST EAX,EAX / SETNZ DL / MOV [EDI+0B28h],DL
009F150A  CMP byte ptr [EDI+0B28h],0
009F1511  JNZ 009F1564
009F1513  MOV ECX,[EDI+0B20h]                 ; the RAW target, not the filtered one
009F1519  CALL 00922DC0
009F151E  TEST AL,AL / JZ 009F1529
009F1522  MOV byte ptr [EDI+0B28h],1          ; reopened
```

Two details the reconstruction in `src/ship_ai_goal_vector.cpp` already had right and that the
listing confirms: the recon test takes the **filtered** target `+0B24h` and the surface test takes
the **raw** target `+0B20h`, and the `+54h` the side comes from is an integer index, because
`008053C0` uses it as an array subscript.

## `008053C0 BSP_Recon_EnsureSlot`

`__fastcall(int side)` in ECX, `RET 0`, body `008053C0`-`00805423`, SEH frame `00C904BB`.

The slot table is the pointer array at `00F874BC`, subscripted by the side
(`008053D9 MOV EAX,[ESI*4 + 0F874BCh]`). A null entry is filled in place: `operator new` of
`12A0h` bytes (`008053E4`/`008053E9`), constructed by `008050E0(side)` at `00805404`, stored back
at `0080540D`. The same side always gets the same slot and the call never fails.

## `009DFBE0 BSP_Recon_FindTargetInUnionList`

`__fastcall(ReconSlot* slot)` in ECX with one stack argument, `RET 4` at **both** exits
(`009DFC01`, `009DFC06`), so the argument is callee-popped. Body `009DFBE0`-`009DFC08`, thirteen
instructions, no Ghidra name before this packet.

```
head = slot->+0E0Ch                 ; 009DFBE0
if (!head) return 0                 ; 009DFBE8 -> 009DFBFF
for (node = head; node; node = node->+4h)   ; 009DFBF8
    record = node->+8h              ; 009DFBF0
    if (record->+4h == target)      ; 009DFBF3
        return record               ; 009DFC04
return 0
```

`slot+0E0Ch` is the head of **triple 4**: `docs/RECON_SLOT_LISTS.md` puts the triple at `+E08h`
and calls it "the union of 0..3, **not published to Lua**", built by `00807933`..`00807966` from
the own, enemy, neutral and unknown triples. So "this side knows that target" means the target
survives into the union, and `docs/RECON_SLOT_LISTS.md` already settles what that takes:

* **rule (a)**, the class scan `00806480`: twenty-two class ids and only those, tested against the
  unit's own stamped `+C4h`, because the scan walks the world registry's per-class list for each.
* **rule (b)**, the four gate bytes `008074D5` reads: `+5Ch` set, `+5Dh` / `+5Eh` / `+60h` clear,
  and `IsKindOf(2)`.
* **rule (c)/(d)**, the sensor pass `00806840` / `008048A0` and the drain `00807647`: an own-side
  entry is never drained, an enemy or neutral entry below level 1 is dropped and one at level 1 is
  moved into the unknown triple. The union holds 0, 1, 2 and 3, so the surviving rule is
  **own-side, or detection level at least a blip**.

The goal vector only reaches this call when the sides differ, so in practice it asks the second
half: is this enemy at least a blip.

## `00922DC0` / `00922C80 BSP_Entity_IsSurfaceTarget`

`00922DC0` is two instructions, `MOV DL,1` then `JMP 00922C80` (body `00922DC0`-`00922DC6`): its
only act is to pass `allow_far = true`, which `00922C87` saves to `BL` and `00922D5D` is the only
place that reads.

`00922C80` is `__fastcall(Entity* in ECX, bool allow_far in DL) -> bool in AL`, `RET 0`, body
`00922C80`-`00922DB6`. **It was already reconstructed on main**, by packet `cc7` in
`include/bsp/attack_target_classify.hpp`, from the same listing. This packet re-read the body
instruction by instruction before binding it and found the existing transcription correct on every
arm, both x87 comparisons and both constants, so no second copy was written. For the rule itself
see `docs/ATTACK_CAPABILITY_INPUTS.md` Part 2; the two helpers it calls are named here:

* `00922B70 BSP_Entity_CastToSubmarine`, `__fastcall(Entity*) -> Entity*` or null, `RET 0`, body
  `00922B70`-`00922B8B`: a safe downcast on `vtable[5Ch](8)` at `00922B7C`.
* `00922B90 BSP_Entity_CastToLandFort`, the same shape with class `1Bh` at `00922B9C`, body
  `00922B90`-`00922BAB`.

Answers, for the four cases the packet asked about: a **ship** is a surface target outright
(`00922CD7`); a **submarine** only while `pos.y > -(SubmarinePeriscopeLevel * 0.25)`, which is
`-5.0` with the shipped default of 20.0, so surfaced yes and submerged no; an **aircraft** never,
at `00922C9E`, and neither does a squadron at `00922CAF`; a **structure** depends on which one —
`MAirfield`, `MShipyard` and `MCommandBuilding` are true outright, an `MLandFort` is true only
when its `+538h[+178h]` faked type is `1Bh` and only when `allow_far` is set, which from this
thunk it is.

## Host methods

Both bindings are in `src/game_hosts_ship_ai.cpp`, class `GoalVectorBinding`.

| method | native | was | is |
| --- | --- | --- | --- |
| `ShipAiGoal::recon_knows_target` | `009DFBE0` | recorded `false` | the union-triple membership rule over the host's unit facts |
| `ShipAiGoal::target_is_surface` | `00922DC0` | recorded `false` | `bsp::entity_is_surface_target_00922c80` over filled `EntityTargetFacts` |
| `ShipAiGoal::target_is_surface_set_branch` | `008DDF90` | - | recorded; the walk continues into the answerable tail |

The recon binding does **not** build a second recon. It answers rules (a) and (b) from the same
unit facts the gunnery host's contact sweep uses (`unit_class_id`, the four scene-node gate bytes,
registry liveness) and runs them through the new pure rule.

New source: `include/bsp/ship_ai_goal_vector_visibility.hpp` and
`src/ship_ai_goal_vector_visibility.cpp`, which hold `009DFBE0`'s literal walk
(`recon_union_find_target_009dfbe0`), rule (a)'s class test
(`recon_scan_visits_class_00806480`) and the membership rule
(`recon_union_contains_009dfbe0`). The surface half is reused from
`include/bsp/attack_target_classify.hpp` and not restated.

**Partial.** Rule (c) does not run in this process: nothing computes a detection level, so every
rule-(a)+(b) member is taken as at least a blip and therefore present in the union. That is the
permissive side of the native rule. A target the native would still be holding at level 0 is known
here.

## Corrections

Appended to `docs/SHIP_AI_APPROACH_SLOT_SCORERS.md` (its follow-up asked why the flag is never
true) and to `docs/ATTACK_CAPABILITY_INPUTS.md` (its Part 2 said no C++ was written, compiled or
run for `00922C80`; there is now, and the transcription is confirmed against an independent
re-read). Neither document's own text is rewritten.

## no_ghidra_function

none. Every routine read here has a Ghidra function with a body range.

## Validation

Build `scripts/build.ps1`, Win32 Release, `/W4 /WX`, clean. Tests 2 of 2 passing. USN02, 3000
mission frames, through `tools/run_game.ps1`.

| number | before (main `5eef10f13`) | after |
| --- | --- | --- |
| `flag_0b28` / `flag_stops` per ship | 0 / 600 of 600 | 1 / 0 of 600 |
| `flag_true` per ship | 0 | 600, from `expiries=75 recon=75 surface=0` |
| `firepower` | 0 | 504000 |
| `heading_changes` (Haguro / Jintsu / Yudachi / Samidare) | 78 / 67 / 94 / 83 | 545 / 572 / 535 / 578 |
| standoff (Haguro / Jintsu / destroyers) | 1450 / 1550 / 200 | 1450 / 1550 / 200 |
| winner slot first / last | 0 / 0 | 0 / 0 (see the caveat below) |
| `shots` | 734 | 853 |
| `hull` | 180 | 119 |
| `deaths` | 2 | 3 |
| `total_damage` | 18525.6 | 12463.2 |

The before row is the scorer packet's measurement on the same commit, not a run made here.

**Attribution.** Every one of those changes traces to the single boolean. The flag opens on the
recon arm, once per timer expiry, and the surface arm is never consulted. `009E7FC0` then stops
returning at `009E80B0`, so the scorers run: `firepower` 0 to 504000, and 504000 calls each to
`009E5DA0`, `009E6870` and `009E6640`. The published heading moves on 545 of 600 scans instead of
78, which changes every ship's course, which changes ranges, which is the whole of the gunnery
delta: more shots at longer and changing ranges, fewer landing, one more ship lost. The standoff
ranges do not move because they are curve outputs, not scan outputs.

**The winner-slot caveat.** `slot_first` and `slot_last` still read 0, and that number carries no
information about the scan. The census reads `ShipAiApproachState::committed_slot_11e8`
(`nested+11E8h`), which `src/ship_ai_approach_update.cpp` reads at lines 1008 and 1016 and **never
writes**. The per-scan winner is the local `best_slot` inside
`ship_ai_approach_select_slot_009e76d0` and is recorded nowhere. Nothing in this packet changes
that; it is the first follow-up below.

USN01, same settings, for the record: clean run, `hull=23 deaths=1 total_damage=220.0`, pilot
attack `ordered=5 closed_mean=4028.3 m`.

## Follow-up packets

- `ship_ai_ring_winner_census`: `009E5E90` writes `nested+120Ch` but never `nested+11E8h`, so the
  committed slot its own turn-cost arm reads back at `009E5FC3` is always 0 and the census cannot
  see the winner. Read `009E5E90`'s tail for the store this reconstruction is missing.
- `recon_sensor_pass_rule_c`: `00806840` and `008048A0`, the sensor pass that sets each entry's
  detection level. Until it runs, every rule-(a)+(b) unit counts as detected and no target is ever
  unknown to a side. `docs/SENSOR_TABLE_DATA.md` has the authored data.
- `ship_ai_approach_point_zone`: `00864BA0`, the no-target arm of the visibility gate, still false.
