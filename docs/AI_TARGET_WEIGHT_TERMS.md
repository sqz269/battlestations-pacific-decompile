# The four terms `00A0F810` stood in for

Addresses: `00923BE0`, `00923BE4`, `00923BEA`, `00923BF1`, `00923C01`, `00923C09`, `00923C16`,
`00923C27`, `00923C34`, `00923C41`, `00923C4B`, `00876260`, `0087BCC0`, `00A0F810`, `00A0F84C`,
`00A0F859`, `00A0F864`, `00A0F8CE`, `00A04560`, `00A08460`.

Packet `cc8_ai_target_weight_terms`, read-only Ghidra analysis. Every descriptive name is a
hypothesis, not a recovered symbol. Reconstruction: `include/bsp/ai_close_attack_tick.hpp`,
`src/ai_close_attack_tick.cpp`. Host: `src/game_hosts_ai.cpp`. Report:
`reports/ai_target_weight_terms.json`. Predecessor: `docs/AI_TARGET_WEIGHT.md`, which bound
`00A0F810`'s four multipliers and labelled four terms inside them as substitutions.

This packet takes those four terms to their instructions, in the order the lead set: the health
term, the inner weight, the two record factors with the target scale, and the attacker's
command-building zeroing. **Runs are blocked** while the remote-desktop session the agents run in
is disconnected, so nothing here is measured; each term says what it would move.

## Term 1 — the health term, `00923BE0`

`BSP_UnitInstance_GetHealth`, `__thiscall(unit) -> float in ST0`, `RET 0`, body
`00923BE0`-`00923C4B`, **read in full**.

```
00923be4  CMP byte [unit+5Dh],0 / JZ 00923bef   ; the torn-down byte
00923bea  FLDZ / RET                            ; torn down -> 0.0f, the class getter is skipped
00923bef  EAX = [unit]
00923bf1  EDX = [EAX+110h] / CALL EDX           ; the class's health FRACTION
00923bf9  FSTP [ESP+4] / FLD [ESP+4]
00923c01  FLDZ / FCOMIP ST0,ST1 / FSTP ST0
00923c07  JBE 00923c21                          ; 0 <= value takes the high clamp
00923c09  XORPS XMM0,XMM0                       ; value < 0 -> 0.0f
00923c16  MOVSS [unit+164h],XMM0 / RET          ; cache and return
00923c21  MOVSS XMM0,[ESP+4]
00923c27  MOVSS XMM1,[00D7A24C]                 ; 1.0f
00923c2f  COMISS XMM0,XMM1 / JBE 00923c37
00923c34  MOVAPS XMM0,XMM1                      ; value > 1 -> 1.0f
00923c41  MOVSS [unit+164h],XMM0 / RET          ; cache and return
```

Note the comparison order at `00923C01`: `FLD` pushes the value, `FLDZ` pushes zero on top, so
`FCOMIP ST0,ST1` compares **zero against the value** and `JBE` takes the high-clamp arm when the
value is non-negative. Read the other way round the two clamps swap.

**It is a fraction, not an absolute.** The class getter is `vtable[+110h]`; for the destroyer family
that slot (`00CFC3D0 + 110h` = `00CFC4E0`) holds `00876260 BSP_UnitInstance_GetHealthFraction`,
body `00876260`-`00876274`, which is `unit+370h / unit+36Ch`, current over maximum. So the clamp
into `[0, 1]` is a safety clamp on a ratio.

Both arms cache the clamped result at `unit+164h`. A store census of that offset
(`tools/store_census.py 0x164`) finds three writers in this family: `0087BD09` in
`BSP_UnitInstance_InitHealthAndParts`, which seeds it, and `00923BE0`'s own two stores. Nothing
else writes it, so `+164h` is this routine's cache and not a field others maintain.

### Correction to the ledger's earlier reading of `00923BE0`

The existing ledger evidence (packet `cc2_unit_timed_subupdates`) records the routine as "the
virtual at vtable `+110h` **floored at zero**, with the zero also written back to `+164h`". The
floor is right and the ceiling is missing: `00923C27` compares the value against `1.0f` at
`00D7A24C` and `00923C34` clamps anything above it down, and **both** arms cache, not only the
zero one. A consumer that relied on the documented shape could pass a fraction above one through,
which the native never does. The ledger record is extended rather than replaced.

### What it does to the weight

`00A0F810` subtracts this from the double `2.0` at `00D7A308`, so slot D runs over **`[1.0, 2.0]`**:
a full-health target contributes `1.0`, a destroyed or torn-down one `2.0`. That is the model's
preference for a damaged target, worth at most a factor of two.

### Correction to `docs/AI_TARGET_WEIGHT.md`

That packet left `target_term` at `0.0f`, which made slot D the constant **`2.0`** — the value a
**destroyed** target yields. It should be the full-health `1.0`. `ai_unit_health_00923be0` now
supplies it, and the host passes the torn-down byte, which it does reach through the scene node
flags (`SceneNodeFlags::torn_down`, `+5Dh`), and `fraction_available = false`, which returns the
full-health `1.0f`.

The ordering of candidates does not change, because the factor was uniform either way and
`ai_close_attack_candidate_admitted` only tests for a positive weight. What changes is the
magnitude: every candidate weight halves. Any future rule that compares a weight against a
threshold rather than against another weight would have read the old value as twice its true size.

**The torn-down arm cannot be reached from the candidate loop.** `00A13B60` drops a candidate that
fails `close_candidate_alive` before scoring it, so every candidate that reaches the weight is
live. The arm is modelled because the native has it, and the census counts any hit so that a
non-zero value would flag the assumption breaking.

### Still stood in for

The fraction itself. `unit+370h` and `unit+36Ch` are the current and maximum health, and the values
this process holds for them live on the gunnery host's per-unit row (`health`, `max_health`), which
the AI coordinator does not hold and which this packet may only read. Term 2 carries the same
problem for the inner weight and settles the route for both.

## Term 2 — the inner weight, `00A08460`

`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` and `include/bsp/ai_target_weights.hpp` already reconstruct
the model as `ai_target_weight_00a08460`, driven by `AiTargetWeightModelHost`'s fourteen methods.
This packet does not re-read it. What it adds is the route from the AI coordinator to the data it
needs, and the switch that turns it on.

### The route, and why it is a process-wide table

The model reads the target's hit points and capture state (`target+48h` and `+4Ch`, at `00A08593`
and `00A085A8`) and walks the attacker's subsystems and their barrels (`+94h`/`+98h` at `00A095E3`,
the `48h`-stride entries at `+74h`/`+78h`) for a reload, an accuracy and a shot count per barrel.
In this process those values live on the **gunnery host's** per-unit row, and the AI coordinator is
constructed with the log and the units host only. `GameUnitRow` carries no weapon or health field,
so there is no route through the units host either.

`GameAiWeaponFacts` and `game_ai_weapon_facts()` are therefore a process-wide table, for the same
reason `game_objective_sets()` is one: the producer and the reader sit in different hosts and
neither owns the other. **This avoids the wiring line in `src/game_hosts.cpp` the lead ruled out.**
`AiWeightModelBinding` implements all fourteen methods over it.

### The switch

`close_target_weight` runs the real model when the table carries a row for **both** the attacker
and the target, and otherwise keeps the `009FDF30` class weight exactly as before. The census
reports both counts and the number of published rows, so the arm taken is never in doubt.

**The table is empty today.** Filling it is one publish call from the gunnery host, which this
packet may only read, so `model_runs` will be zero and `class_stand_ins` will equal the query count
until that call lands. The adapter and the switch are written now so that the call is the only
thing left.

Six of the fourteen methods are labelled stand-ins even once rows exist, each at its site: the memo
pair `00A03B90`/`00A079B0` (a cache, so skipping it changes no answer), the forced-rule override
`00A31DB0` (its rules come from the globals loader's `ForcedTargetWeightValues` tail, which this
process does not run), the two entity type queries at `vtable[+18h]` and `+1Ch`, the distance
falloff `009FE200` and the capture scale `00424C40+3B0h`. The `AiModeTuning` record is projected
from the block `00A335D0` filled, carrying `MaxTargetKillRatio` at `+05Ch` and `DamageCalcTime` at
`+060h`, which are the two fields the barrel arithmetic uses.

## Term 3 — the two record factors and the target scale, `00A04560`

`FUN_00A04560`, `__fastcall(ECX = out, EDX = entity)`, `RET 4`, body `00A04560`-`00A046B5`. It
builds the per-entity record whose `+10h`, `+14h`, `+18h` and `+1Ch` `00A0F810` reads.

What the body does, in order:

| Site | What |
| --- | --- |
| `00A04568` | `CMP [entity+54h],2` / `SETGE BL`: a flag for the entity's side being 2 or more |
| `00A04570` | when `[entity+C8h]` is clear, `00A0457D` calls `00414DB0 BSP_EntityPose_RefreshWorld` |
| `00A04582`, `00A0459D` | saves `[entity+FCh]` and `[entity+104h]`, the pose's X and Z |
| `00A0458A`-`00A045A7` | `EAX = 0CF6474A9h`, `MUL ESI`, `SHR EDX,6`, `IMUL EDX,4Fh`, `EAX = ESI - EDX`: the entity **pointer modulo 79**, converted to float at `00A045B0`. A per-entity spread, cheap and stable for the entity's lifetime |
| `00A045B9`, `00A045D1` | two `00A371A0` tuning reads, `[record+54h]` and `[record+50h]` |
| `00A045E5` | `00419010` over those two and the constant at `00D21D04` |
| `00A04652` | `IsKindOf(5)` -> the class descriptor is `[entity+538h]` |
| `00A0466E` | else `IsKindOf(18h)` -> it is `[entity+35Ch]`, the plane squadron's plane class |
| `00A04680` | else null |
| `00A046A5` | `00A00020(out, ...)` builds the record |

The plane-squadron arm ties back to `docs/PLANE_SQUADRON.md`: `+35Ch` is the field `007F477E`
fills with `007B8A80`'s class for the squadron's wings, so a squadron is weighed through its
planes' class and not through a class of its own.

`FUN_00A00020`, body `00A00020`-`00A0009B`, stores by offset:

| Store | Offset | From |
| --- | --- | --- |
| `00A00038` | `+0h` | the first stack argument |
| `00A00040`, `00A0004A` | `+4h`, `+8h` | the two floats of the pointer argument, the pose X and Z |
| `00A0004D` | `+14h` | the third stack argument, a float |
| `00A0005B` | `+0Ch` | a byte argument |
| `00A0005E` | `+10h` | the second stack argument |
| `00A00061` | `+1Ch` | a later stack argument |
| `00A00058` | — | `COMISS` against another float argument gates a second `00A371A0` interpolation whose result is not yet traced to `+18h` |

### The mapping, settled

`tools/frame_slot_census.py` normalises both frames once the virtual calls'
cleanups are supplied. `00A00020` needed none; `00A04560` needed four, one per
`RET 4` class test:

```
python tools/frame_slot_census.py 00a00020
python tools/frame_slot_census.py 00a04560 --pop 00a04615=4 --pop 00a0462c=4 \
                                            --pop 00a0465b=4 --pop 00a04672=4
```

`00A00020`'s seven reads then fall on seven consecutive slots, `K=-4` through `K=-28`, which is
arguments one to seven in order. Matching them against the caller's seven pushes, taken backwards
from `00A046A5` because the last pushed is the first argument:

| Argument | Pushed at | Value | Lands at |
| --- | --- | --- | --- |
| 1 | `00A046A2` `PUSH ESI` | the class descriptor, `entity+538h` or `entity+35Ch` | `record+0h` |
| 2 | `00A046A1` `PUSH EDI` | **zero**, from `00A04650 XOR EDI,EDI` | `record+10h` |
| 3 | `00A0469D` then `00A0469E FSTP` | the float `00A0460F` produced | `record+14h` |
| 4 | `00A0469C` `PUSH EDX` | the `SETNZ` of `00A04568 CMP [entity+54h],2` | `record+1Ch` |
| 5 | `00A04691` then `00A04695 FSTP` | the interpolation `00A045EA` stored | the `00A00058` test, then `record+18h` |
| 6 | `00A0468F` `PUSH 1` | the immediate `1` | `record+0Ch`, as a byte |
| 7 | `00A0468C` `PUSH ECX` | `&` the pose pair | `record+4h` and `+8h` |

Two slot coincidences confirm it: `00A04597`, which stores the pose X, and `00A04688`, the `LEA`
that takes argument seven's address, normalise to the same `K=8`; and `00A045EA`, which stores the
interpolation, and `00A04682`, which reloads it for argument five, both land on `K=20`.

### What each field is

* **`record+0h`** is the **class descriptor**, not the entity: `00A04652 IsKindOf(5)` takes
  `entity+538h` and `00A0466E IsKindOf(18h)` takes `entity+35Ch`, the plane squadron's own plane
  class that `007F477E` fills. So a squadron is weighed through its planes' class, and the pointer
  `00A0F810` hands `00A08460` as the "entity" is a class.
* **`record+10h` is always zero.** `00A0F83C` reads it as the `attacker_class` key field, so that
  field arrives at `00A08460` as zero from this path whatever the attacker is.
* **`record+14h`** is `00A0460F`'s float: `009FDF30`'s class weight for the entity's `+C4h` class
  id (`00A045EE`, `00A045F4`), multiplied by `00A04240(entity)` at `00A04604`. **This is where the
  class weight lives natively**, as the target scale `00A0F89B` multiplies by, not as the inner
  weight.
* **`record+18h`** is argument five, stored plain at `00A00091`, or re-rolled through
  `00BD2F10` at `00A00083` when `00A00058`'s `COMISS` finds it negative. Argument five itself is
  `00A045EA`, the `00419010` interpolation over `00A371A0`'s `[record+50h]` and `[record+54h]`.
* **`record+1Ch`** is the **side being two or more**. `00A0F839` passes it to `00A08460` as
  `target_is_neutral`, so "neutral" means a third-party side, and `00A0F84C` tests the attacker's
  copy of the same bit.

### What the host now does with them

The class weight **moves out of the base term into `target_scale`**, where the native keeps it.
The base term becomes `1.0f`, the identity of the product it feeds, until `00A08460` runs for real.
The product is unchanged in value, so no ordering moves; what changes is that the class weight is
now in the right factor and will not be double-counted the day the model turns on. `00A04240` is
unread, so its half of `record+14h` stays at the identity.

The two `+18h` factors keep `1.0f`, because the two tuning offsets `[record+50h]` and `[record+54h]`
that feed the interpolation are unread.

## Term 4 — the attacker's command-building zeroing

`00A0F84C` tests `[attacker record +1Ch]` and, only when it is non-zero, `00A0F859` asks the
attacker `vtable[+18h](1Ch)`; both true zero the weight at `00A0F864`.

**The first input is settled and is now supplied.** `record+1Ch` is the `SETNZ` of
`00A04568 CMP [entity+54h],2`, the entity's side being two or more, and this process has that:
`units.unit_side_0054(attacker) >= 2`.

**The second is not.** `vtable[+18h]` is not the `+5Ch` class test the other three tests in
`00A0F810` use, so the `1Ch` it is asked with is a type-group code rather than the
`MCommandBuilding` class id it resembles. `AiTargetWeightModelHost::entity_is_type` is the same
slot's second consumer, so settling one settles both. It stays false, and the arm therefore never
fires.

That is safe in the one direction that matters: the arm can only ever **remove** weight, so leaving
it off can admit a candidate the native would have scored zero, never reject one it would have
kept.



## The one input left, `009FE270`

`BSP_Ai_BarrelAccuracyForKind`, `__fastcall(ECX, EDX, one stack argument) -> float in ST0`,
`RET 4`, body `009FE270`-`009FE6C3`. The accuracy `00A08460` multiplies into its barrel damage at
`00A094E6`, and the only model input this process has no producer for.

It is a seventeen-way dispatch. `009FE277` calls the stack argument's `vtable[+1Ch]` first;
`009FE27F` takes the selector from the **second argument's `+8h`**, `009FE288` subtracts `2` and
`009FE28E` rejects anything above `10h`, so the live range is `2`..`12h`. `009FE294` indexes a byte
table and `009FE29B` jumps through a dword table:

```
009fe6ec  00 00 09 01 01 01 09 02 03 04 09 05 09 06 07 05 08
009fe6c4  009FE2A2 009FE313 009FE384 009FE3F5 009FE44A
          009FE64A 009FE465 009FE480 009FE4F1 009FE6BB
```

| Selector | Byte | Handler |
| --- | --- | --- |
| `2`, `3` | `0` | `009FE2A2` |
| `5`, `6`, `7` | `1` | `009FE313` |
| `9` | `2` | `009FE384` |
| `0Ah` | `3` | `009FE3F5` |
| `0Bh` | `4` | `009FE44A` |
| `0Dh`, `11h` | `5` | `009FE64A` |
| `0Fh` | `6` | `009FE465` |
| `10h` | `7` | `009FE480` |
| `12h` | `8` | `009FE4F1` |
| `4`, `8`, `0Ch`, `0Eh` | `9` | `009FE6BB`, the reject arm |

Each live arm reads one float out of the `00A371A0` tuning record, gated by the first argument's
`vtable[+18h]` type queries: `009FE2A2` answers `[00A371A0()+110h]` when `vtable[+18h](0Fh)` holds
and otherwise falls to a `PUSH 6` query.

`coverage: partial` — the dispatch is complete, the nine handler bodies are not read. Reading them
is what would let the publisher mark a row complete and turn the model on, so it is the next
packet's obvious first move.

## Validation

**Blocked, not measured.** The remote-desktop session the agents run in is disconnected, so the
machine has no audio endpoint for session 1 and FMOD cannot initialise; every run dies before the
window. `docs/AI_TARGET_WEIGHT.md` section 3 carries the two failure texts and the evidence that it
is the environment. The build is clean at `/W4 /WX` and both existing ctest cases pass.

When runs work again, IJN01 goes first. This packet adds two census lines,
`summary mission ai target weight health` for the torn-down count and
`summary mission ai target weight base` for the model-versus-stand-in split and the published row
count. Expected, on the three missions as they stand:

| Line | Expected | What a different value means |
| --- | --- | --- |
| `torn_down_targets` | `0` | the liveness filter above the weight is not doing what term 1 claims |
| `model_runs` | `0` | something published weapon rows; the base term is then the real model |
| `class_stand_ins` | equal to `queries` | the complement of the above |
| `weapon_rows` | `0` | the gunnery publish call landed |
| `served`, `attackmove`, `settarget` | unchanged from `docs/AI_TARGET_WEIGHT.md` | term 1 halves every weight uniformly and terms 2 to 4 are inert, so no ordering moves |

The behavioural movement `docs/AI_TARGET_WEIGHT.md` predicts for IJN01 is that packet's, not this
one's: this packet corrects a magnitude and lays the route for the base term without changing which
candidate wins.

## Correction: `009FE270` traced to its producer (packet `cc8_ai_target_weight_census`, 2026-09-18)

**The premise that a gun or barrel record stores an accuracy is wrong, and the section above is
corrected on two points.** There is no accuracy field on any weapon record. `009FE270` is a pure
lookup into the AI mode tuning record, and the value is authored in Lua.

### Two corrections to the section above

1. "Each live arm reads one float out of the `00A371A0` tuning record gated by **the first
   argument's** `vtable[+18h]` type queries" is wrong. `009FE273 MOV ESI,[ESP+10h]` takes the
   **stack argument** after the three pushes, and every vtable call in the routine is on `ESI`:
   `009FE277`/`009FE27C`/`009FE286` is `stack->vtable[+1Ch]()`, and each arm's `009FE2A4`-style
   `MOV EAX,[EDX+18h]` with `MOV ECX,ESI` is `stack->vtable[+18h](classId)`. The first argument
   (`ECX`, kept in `EBP` at `009FE282`) is never dereferenced in the dispatch. The selector from
   the second argument's `+8h` at `009FE27F` was read correctly.
2. The arm count is right but the reading "nine handler bodies" understated one: **arm 8 serves two
   bullet types**, so nine live arms cover ten.

### The producer

`00A094E6` calls it as `009FE270(ECX = EBP, EDX = [ESP+5Ch], stack = EDI)`, where `EDX` is the
weapon record whose `+8h` is the bullet-type selector and `EDI` is the target. The arm then calls
`00A371A0`, the per-mode AI tuning record (stride `23Ch`, selected by
`009FFC80 BSP_Ai_EffectiveGameModeIndex`), and `FLD`s one float out of it. **The only writer of
those floats is `00A335D0`**, the AI globals loader, which reads
`Scripts\datatables\HighLvlAIGlobals.lua`'s `BulletTypeAccuracy` section into record offsets
`110h`-`18Ch`. `src/ai_target_weights.cpp` already carries that field map. So the census of the
field is not a struct-field census: the field is a tuning-table slot with exactly one producer, and
`config/names` needs no new writer.

The authored values, from this installation's `scripts/datatables/highlvlaiglobals.lua:72`, whose
Hungarian comment reads "with a given bullet type, when we shoot a given kind of target, what hit
multiplier to use in the damage calculation, i.e. on average what chance a round has to hit a
target". **The per-row comment `Repulore/Kishajora/Nagyhajora/Landfortra` names the index: 1 plane,
2 small ship, 3 big ship, 4 landfort.** That is what the `vtable[+18h]` queries select, and
`PUSH 0Fh`, the plane base class, is the first test in every four-entry arm.

| Arm | Selector (`record+8h`) | Bullet type | Record offsets | Plane | SmallShip | BigShip | Landfort |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `009FE2A2` | `2`, `3` | MachineGun | `110h`-`11Ch` | 0.10 | 0.15 | 0 | 0.0 |
| `009FE313` | `5`, `6`, `7` | Artillery | `120h`-`12Ch` | 0.00 | 0.50 | 0.70 | 0.55 |
| `009FE384` | `9` | Bomb | `130h`-`13Ch` | 0.00 | 0.50 | 0.70 | 0.20 |
| `009FE3F5` | `0Ah` | Torpedo | `144h` submarine, else `140h` ship | - | 0.75 ship | - | 0.45 sub |
| `009FE44A` | `0Bh` | DepthCharge | `148h` scalar | - | - | - | 0.50 |
| `009FE64A` | `0Dh`, `11h` | Kamikaze | `150h`-`15Ch` | 0.00 | 0.50 | 0.70 | 0.80 |
| `009FE465` | `0Fh` | Paratroopers | `14Ch` scalar | - | - | - | 0.30 |
| `009FE480` | `10h` | Flak | `180h`-`18Ch` | 0.50 | 0.20 | 0.00 | 0.00 |
| `009FE4F1` | `12h` | SmallRocket, then BigRocket | `160h`-`16Ch`, `170h`-`17Ch` | 0.15 / 0.00 | 0.25 / 0.25 | 0.70 / 0.90 | 0.50 / 0.75 |
| `009FE6BB` | `4`, `8`, `0Ch`, `0Eh` | reject, returns `0` | - | - | - | - | - |

Jump table read from the bytes at `009FE6C4` and the selector byte table at `009FE6EC`
(`00 00 09 01 01 01 09 02 03 04 09 05 09 06 07 05 08`), not from the decompiler.

`coverage: partial` on one point. Arm `009FE4F1` runs four `vtable[+18h]` queries at `009FE4F7`,
`009FE524`, `009FE542` and `009FE555` before the SmallRocket block at `009FE564` and reaches the
BigRocket block at `009FE5D5`; **which test splits small from big is not read**, so the two rocket
rows above are the offsets, not a decided mapping. Every other arm is complete.

### What this means for publishing it

The accuracy is **not a per-barrel constant**: it is a function of (bullet type, target class), and
`bsp::AiTargetWeightModelHost::barrel_accuracy(subsystem, barrel, target)` already takes the
target. The flat `GameAiWeaponFacts::Barrel::accuracy` at `include/bsp/game_hosts_ai.hpp` is the
wrong shape and `AiWeightModelBinding::barrel_accuracy` at `src/game_hosts_ai.cpp:184` discards its
target argument. Completing the row needs the **bullet-type selector** published per barrel, not an
accuracy, and the lookup resolved at query time.

Two further blockers, both named by address rather than guessed at:

* `bsp::AiTuningBlock` carries the full `23Ch` stride but `ai_tuning_keys()` loads only 33 keys and
  **none of them is a `BulletTypeAccuracy` entry**, so `tuning.at(0x110)` returns the unloaded
  `0.0f`. `mode_tuning_record()` at `src/game_hosts_ai.cpp:417` copies exactly two fields.
* `ai_load_globals_00a335d0`, the reconstruction that does cover the whole field map, **has no
  caller anywhere in `src/`**, so the `AiModeTuning` records it fills are never populated in this
  process.

Publishing a `1.0f` would invent the value; publishing the table makes it real. Note that many
authored cells are exactly `0.0` (MachineGun against a big ship or a landfort, Artillery and Bomb
against a plane), and `00A094F5 FCOMIP / JNC` skips a barrel whose accuracy is not above zero, so a
correct publication makes the model **skip** those barrels rather than score them.

### Where the selector comes from, and the one hop still open

Traced inside `00A08460`'s barrel loop, which settles what `EDX` is:

```
00a093d0  mov esi,[ebx+74h]      ; the barrel array, +74h/+78h base and count
00a093d3  add esi,[esp+18h]      ; the 48h-stride barrel entry
00a093d7  mov eax,[esi+34h]      ; barrel+34h, the bullet class record
00a093da  mov [esp+5Ch],eax      ; becomes EDX at 00A094E6
00a093de  mov eax,[eax+8]        ; the selector
00a093e1  cmp eax,0Ah            ; 0Ah is the Torpedo arm, which confirms the space
00a093e4  jne 00a093ed
```

So the chain is **barrel `+34h` -> bullet class record -> `+8h` selector -> arm -> tuning offset**,
and the `CMP EAX,0Ah` against the Torpedo selector is independent confirmation of the arm mapping
above, taken from a different site than the jump table.

The bullet class record is what `006EA910 BSP_BulletClass_GetOrCreate` builds
(`docs/ORDNANCE_KIND_IDENTITY.md`): it reads the authored `Bullets` row's `Type` string, matches it
case-insensitively against thirteen literals, and runs that kind's constructor. **`+8h` is
therefore a per-class constant written by each kind's constructor, not authored data**, which is
why no `BulletType` number appears anywhere in `bulletclasses.lua`.

**The open hop, stated as unfinished rather than guessed:** the thirteen constructors were not
read, so **which constant each one stores to `+8h` is not established**. The counts are suggestive
and are not evidence — thirteen `Type` strings against thirteen live selectors, and four rejected
in-range selectors (`4`, `8`, `0Ch`, `0Eh`) against the four `Dummy*`/`WaterMine` types that have no
accuracy row — but the grouping `2,3 -> MachineGun` and `5,6,7 -> Artillery` means at least one
accuracy row serves several classes, so the correspondence is not one-to-one and cannot be assumed.
Reading the constructor bodies named in `ORDNANCE_KIND_IDENTITY.md`'s census table is the next
concrete step, and it is what the gunnery host needs before it can publish a selector per barrel.

Until that lands, `GameGunRow::ordnance` is **not** a substitute: it is the `vtable[8]` entity-class
answer set (`29h`-`34h`), a different id space from this selector, and using it here would be a
guess.
