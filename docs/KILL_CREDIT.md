# Same-side kill credits in USN02 (packet cc9_kill_credit)

Addresses: 0077CE60 (0077CE66-0077CEBE), 00929800, 0091BDA0, 0098AEE4, 0070C257, 0084BF00.
Ghidra was read only.

## 1. The image's credit rule

- **The attacker is written per hit.** `BSP_Unit_RecordDamageAttribution` (0077CE60) runs on
  every hit (step 7 of 009239A0). It returns at once unless the victim is live: `+5Ch` set,
  `+5Dh`, `+60h` and `+5Eh` clear (0077CE66-0077CE88).
- **It needs damage.** It takes the hit's damage: `00470510` for a hull segment, or `00470740`
  when `record+34h` is -1. It continues only when that damage is above 0.0 (`0077CEB7` COMISS
  against `[00D7A218]`, `JBE 0077D12E`).
- **The block.** It then writes the attribution: `+2C4h`, the attacker from the shot's owner
  (`[record+4]->vtable[108h]`), and the class, side and ordnance fields.
- **Who gets the credit.** `0091BDA0` credits `+2C4h`. `00929800` publishes `Dead` and the
  `KillReason` from the cause at `+70h`, and names no attacker.
- **So the credit goes to the owner of the last damaging hit, whatever its side.**
- **Hits on friends are possible.** The projectile sweep excludes only the round's own owner
  (`0098AEE4` pointer compare; `projectile+238h`, `0070C257`). The one friendly-fire exemption is
  weapon class 11h against a shooter of the same `+9D4h` parent (`0084BF00`), which applies to
  aircraft guns in a squadron. Ship shells and torpedoes can hit ships of their own side.

## 2. The host against it, and the same-side kills

- **Host.** The gunnery host wrote the attribution and `last_attacker` on every hit, including
  hits whose damage is zero. That is the only difference from 0077CE60's rule. The host decides
  a death in the same call as the hit that causes it, and that hit carries damage. So the
  difference can only matter for a death reached by another path after a later zero-damage hit.
- **The same-side kills, with `BSP_DEATH_TABLE` on the landed tree** (`local/kC_usn02.log`,
  option on):

| victim | time | credited to | side | weapon | range |
| --- | --- | --- | --- | --- | --- |
| Amatsukaze | 127.85 s | Hatsukaze | IJN on IJN | category 7 torpedo blast | 320 m |
| Jintsu | 177.06 s | Tokitsukaze | IJN on IJN | category 7 torpedo blast | 4895 m |
| Jupiter | 237.11 s | Encounter | Allied on Allied | category 7 torpedo blast | 22 m |

- **All three are friendly torpedo hits,** and the killing hit is the credited ship's own
  torpedo. They are not misses of a gun, and not a credit rule naming someone other than the
  killer.
- **The line-of-fire test does not cover torpedoes:** 00729560 installs it for weapon kinds 1, 5
  and 6 only.
- Whether the image's torpedo bot refuses a launch with a friendly in the spread's path was not
  read. It is the open question these kills raise.

## 3. The binding and predictions (written before the pair)

`kKillCreditDamageGateBound` (default true): the attribution is written only when the hit's base
damage (`hull_damage_base + part_damage_base`, the 00470510 or 00470740 value) is above 0. The
victim-live gate is already the host's early return for a dead victim.

Pair: USN02 9000, option on, OFF `local\kO` against ON `local\kT`.
- Every count identical: hits, shots, deaths and damage. The gate changes only who is named, and
  only when a zero-damage hit is the last hit.
- The zero-damage count logged by the ON side is small: 0-30 hits.
- Kill credits: identical in both runs. Every death in this mission comes in the same call as a
  damaging hit.

## 4. The pair, and the decision

`local/kO_usn02.log` against `local/kT_usn02.log`, both with the option and `BSP_DEATH_TABLE` on.
They are identical in every count:
- 0 of 464 gun rows and 0 of 32 unit rows differ.
- Queued hits are 763 on both sides, deaths 20 and damage 75904.7.
- There is no mission end on either side.

The ON side skipped **0** zero-damage attributions, inside the predicted 0-30. Every hit in
USN02 carries base damage, so the gate never fires here. It lands **ON** as the image's rule.

**Answer to the question.** The same-side credits are real friendly torpedo hits, and the credit
rule names the killing hit's owner, as the image does. They are neither line-of-fire refusals the
host missed (the test does not apply to torpedoes) nor a credit-rule artefact. The next read is
whether the image's torpedo bot holds a launch with a friendly ship in the spread's path. That
would be in the torpedo bot tick 008FBB00 and its launch gate, and is not read here.
