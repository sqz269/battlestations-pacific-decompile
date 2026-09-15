# The yaw arm, explained by the people who wrote it

Packet `cc7_yaw_authored`. No new disassembly: this is a cross-check of the law recovered in
`docs/PILOT_BOT_PLAN_CONTROLS.md` and `docs/PILOT_COMMAND_BAND_REPAIR.md` against the authored
comments in `scripts/datatables/planeglobals.lua`.

The law was read out of `0099D300`'s instruction stream by two packets that never looked at the
data file. The data file's authors left a comment beside every constant. They agree, term for term,
and the agreement settles the two things a listing cannot: the **units** and the **intent**.

| key | value | authored comment (Hungarian) | what the listing does with it |
|---|---|---|---|
| `SoftHdgZone` | `0.01` | *"ennel kisebb elteres eseten ugy teszunk, mintha nem is lenne elteres"* — below this deviation we act as if there were none | `0099DEC5`-`0099DEFC`: subtracted as a deadband |
| `SoftHdgLimit` | `0.5` | *"'huzott forduloval ennyi sec alatt megtett szog'-nel kisebb elteresnel mar tompitjuk az elterest"* — below the angle a pulled turn covers in this many seconds, we damp the deviation | `0099DF0F`: the limit is `(TurnRollSpd + PitchSpd) * 0.5` — a turn **rate** times a **time**, which is an angle |
| `SoftHdgMul` | `0.35` | *"lehetoleg 0.1 es 1.0 kozott legyen. ennyivel finomodik a heading allitas"* — preferably 0.1..1.0; this is how much the heading adjustment is softened | `0099DF24`: the multiplier inside the limit |
| `YawTurnRollRange` | `{DEG(30), DEG(60)}` | *"ekkora roll tartomany eseten yaw-al is kanyarodik"* — over this roll range it turns with yaw as well | `0099DFFB` and `0099E94A`: the two interpolations, with swapped endpoints, that blend the heading term out and the turn term in |
| `YawCtrlSetTimeMul` | `0.8` | *"manoverezes kozben ugy allitja be a yaw kontrolt, hogy elvileg ennyi sec kell ahhoz, hogy a kivant pozicioba keruljunk"* — it sets the yaw control so that in theory this many seconds are needed to reach the wanted position | `0099E884`: the divisor `YawSpd * cos(bank) * 0.8` |

The reconstruction these confirm is `plan_yaw_0099e81a`, `yaw_base_numerator_0099de8a` and
`yaw_base_gain_0099dffb` in `include/bsp/plane_ai_control.hpp` and `src/plane_ai_control.cpp`
(packets `cc7_pilot_bot_axis_arms` and `_2), which decompose the arm the way the native does. This
doc adds no code.

## What the cross-check buys

**Units.** `(TurnRollSpd + PitchSpd) * SoftHdgLimit` is dimensionally a rate times a time. The
comment says in as many words that it is "the angle covered in this many seconds by a pulled turn",
so `TurnRollSpd + PitchSpd` is the plane's combined turn rate and the product is an angle in the
same units as the heading error. Nothing in the listing says that; it would have been an assumption.

**The divisor's meaning.** `error / (YawSpd * cos(bank) * YawCtrlSetTimeMul)` reads, from the
comment, as "the fraction of full yaw control that covers this error in 0.8 seconds". That is a
*time-to-target* controller, not a proportional gain, and a reimplementation that treated the
divisor as a tuning constant rather than as a rate-times-time would get the right numbers today and
the wrong ones the moment `YawSpd` changed.

**The blend.** *"Over this roll range it turns with yaw as well"* is the whole design in one line: a
wings-level plane holds heading with its rudder, a banked plane turns with its elevator, and between
30 and 60 degrees of bank it does both. The two interpolations with swapped endpoints
(`1 -> 0` for the heading term, `0 -> 3` for the turn term) are that sentence.

## What it does not settle

The comments say nothing about `plan+2CCh`'s gate value of `2`, about `unit+340h`, or about the
turn arm's numerator, all of which remain as `docs/PILOT_BOT_PLAN_CONTROLS.md` and
`docs/PILOT_COMMAND_BAND_REPAIR.md` leave them. Agreement on five constants is not a licence to
assume the sixth.

It is also worth being explicit that this is an installation's data file, not provably retail:
`planeglobals.lua` is one of two files in `scripts/datatables` not carrying the install date
(`docs/PLANE_CONTROL_RATE_LAW.md` has the full caveat). The *comments* are far more likely to be
original than the values, since a modder retunes numbers and rarely rewrites Hungarian prose - but
that is a judgement, and the values above should be read as this installation's.
