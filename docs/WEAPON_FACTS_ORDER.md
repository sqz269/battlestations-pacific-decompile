# The AI weapon facts at the first think (packet `cc9_weapon_facts_order`, cc9-lua21, 2026-09-30)

Addresses: 00A08460 (read through its inputs), 00A0C330 / 00A0C3C0 (the group-value pair),
00864BD0 (the gunnery attach). The binding edit is in cc9-gunnery15's `src/game_hosts_gunnery.cpp`
and is handed to the lead, not committed here.

## 1. The question (SHIP_AI_OPEN_ITEMS 70.1)

At t=0.05 the first AI think scored group values with the stand-in weight 1.0, because the
weapon-facts table was still empty.

## 2. The image's order

- `00A08460` reads fields of the vehicle CLASS descriptors (`00A04560` record+0h = `[entity+538h]` or
  `[entity+35Ch]`), present from load:
  - the target's hit points at `target+48h` (`00A08593`);
  - the attacker's subsystem barrel lists at `+94h`/`+98h` (`00A095E3`).
- `target+4Ch` is the class `Armour`, not a capture state (GUNNERY_OPEN_ITEMS 80,
  cc9-gunnery18).
- It has no table and no publication step. So the facts exist before the first brain think:
  every unit is constructed before the scene's first tick.

## 3. The host's order

`GameGunneryHost` fills `game_ai_weapon_facts()` in `publish_ai_weapon_facts`, at the END of each
gunnery step (after the per-unit, aim, fire, projectile and damage passes). The AI coordinator's
think in the same frame runs before that. So:
- the t=0.05 think finds no rows;
- `group_value_pair_00a0c3c0` and `close_target_weight` take their stand-in branch;
- a spawn batch registered mid-mission (`register_new_units_00864bd0`) is likewise invisible
  until the next step's publish.

**This is a host timing substitution, not the image.**

## 4. The binding (prepared for the owner)

`kAiWeaponFactsAtAttachBound` in `src/game_hosts_gunnery.cpp` publishes the table once more at
the end of `attach_00864bd0` and of `register_new_units_00864bd0`, after `build_guns` /
`attach_passes`. The rows are the same function's, from the freshly built guns:
- hit points are the class maximum, as every step publishes;
- the barrels are the guns' reload, muzzle count and sub-type.

The prepared edit script is `local\l21_wf_edit.py` in the cc9-lua21 tree (three anchors; the
`--on` flag sets the switch).

## 5. Predictions, written before the ON runs

OFF is the export of `36ad0b232` (`local\l21_p0`). ON is the same tree with the edit applied
`--on` (`local\l21_pa`).

OFF `summary mission ai group target value`:

| row | calls | model pairs | stand-in pairs |
| --- | --- | --- | --- |
| USN13 | 42756 | 233727 | 2790 |
| USN01 | 524 | 6358 | 96 |
| JM05 | 0 | 0 | 0 |
| IJN01 | 312 | 10096 | 0 |
| USN04 | 38 | 3265 | 0 |

`close_target_weight`'s `class_stand_ins` is 0 on all five, so only the group value is exposed.

**Predictions:**
- **Mechanism:** `stand_in_pairs` goes to 0 on USN13 and USN01. `model_pairs` rises by about the
  same count, unless a later spawn batch is what fed some of the stand-ins: USN13 may keep a
  residue, which would then show as 0 here too, since the register path is covered.
- **USN13 and USN01:** the t=0.05 Attack/Capture group choices can change where the model's value
  differs from 1.0.
  - Expected exit 3 on USN13, with a move in the first orders' target groups.
  - USN01 exit 1 or 3.
  - Deaths within ± 3 of OFF.
- **JM05, IJN01 and USN04:** no stand-in pairs OFF, so gameplay-identical (exit 0 or 1).

**Verdict rule:** recommend ON if `stand_in_pairs` is 0 on USN13 and USN01 and the three controls
are gameplay-identical. Recommend OFF, with the reason, if a control moves.

## 6. Measured, and the recommendation: ON

The predictions in section 5 were written to disk before the ON runs launched. They are committed
together with this section.

| row | stand-in pairs OFF -> ON | model pairs | planner calls | pair_diff | per-entity |
| --- | --- | --- | --- | --- | --- |
| USN13 | 2790 -> 0 | 233727 -> 253882 | 42756 -> 60918 | 3 | deaths identical (23); 53 unit rows moved |
| USN01 | 96 -> 0 | 6358 -> 6545 | 524 -> 884 | 3 | deaths identical (5); hits 145 -> 151, damage 3040 -> 3499 |
| JM05 | 0 = 0 | 0 | 0 | 0 | identical |
| IJN01 | 0 = 0 | 10096 = 10096 | 312 = 312 | 1 | gameplay identical |
| USN04 | 0 = 0 | 3265 = 3265 | 38 = 38 | 1 | gameplay identical |

- **The mechanism held.** Every stand-in pair is gone. The t=0.05 think on USN13 and USN01 now
  scores its groups with `00A08460`'s own weight, and the group choices that follow moved.
  USN13's planner call count rose by 42%.
- **The three controls are gameplay-identical.**
- **Recommendation: ON** (`kAiWeaponFactsAtAttachBound = true`), by the rule in section 5.

The edit is the owner's to apply (cc9-gunnery15's file).
