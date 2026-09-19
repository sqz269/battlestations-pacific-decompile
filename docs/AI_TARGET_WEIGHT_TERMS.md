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

### The constructors, read

The hop above is now read rather than inferred. Each constant is a `MOV dword ptr [ESI+8], imm` in
the named constructor, and each vtable install matches `ORDNANCE_KIND_IDENTITY.md`'s census column,
which is what ties the constructor to its authored `Type` string. `BSP_TorpedoClass_Construct` was
the control: `006EA500` installs `00CFA56C` and `006EA50C` stores `0Ah`, the value
`00A093E1 CMP EAX,0Ah` independently tests for.

| Constructor | Selector | Byte-table arm | Accuracy row |
| --- | --- | --- | --- |
| `006EA1C0 BSP_ArtilleryBulletClass_Construct` | `4` | **reject** | none |
| `006EA260 BSP_BombClass_Construct` | `9` | `009FE384` | Bomb |
| `006EA4F0 BSP_TorpedoClass_Construct` | `0Ah` | `009FE3F5` | Torpedo |
| `006EA3A0 BSP_DepthChargeClass_Construct` | `0Bh` | `009FE44A` | DepthCharge |
| `006EA6B0 BSP_DummyTargetClass_Construct` | `0Ch` | reject | none |
| `006EA7F0 BSP_DummyKamikazePlaneClass_Construct` | `0Dh` | `009FE64A` | Kamikaze |
| `006EA870 BSP_DummySubmarineClass_Construct` | `0Eh` | reject | none |
| `006EA720 BSP_ParatrooperClass_Construct` | `0Fh` | `009FE465` | Paratroopers |
| `006EA470 BSP_FlakBulletClass_Construct` | `10h` | `009FE480` | Flak |
| `006EA200 BSP_KamikazePlaneClass_Construct` | `11h` | `009FE64A` | Kamikaze |
| `006EA330 BSP_RocketClass_Construct` | `12h` | `009FE4F1` | SmallRocket / BigRocket |
| `006EA5E0 BSP_WaterMineClass_Construct` | `13h` | out of range at `009FE28E` | none |
| `006E8320 BSP_BulletClass_Construct` | **none** | - | - |

Three of the four byte-table rejects are now explained by name: `0Ch` DummyTarget, `0Eh`
DummySubmarine and, outside the range check rather than the table, `13h` WaterMine. Those are
targets and mines, and having no hit chance is right.

**The fourth reject is not explained, and it is an anomaly worth stating rather than smoothing
over.** `BSP_ArtilleryBulletClass_Construct` stores `4`, and selector `4` takes the reject arm,
while the Artillery accuracy row at `120h`-`12Ch` is reached only by selectors `5`, `6` and `7`. So
on the reading that `+8h` is fixed at construction, an artillery shell would get no accuracy at all
and every artillery barrel would be skipped at `00A094F5` — which the authored row
`{0.00, 0.50, 0.70, 0.55}` contradicts.

**The open question, sharpened.** Selectors `2`, `3` (MachineGun) and `5`, `6`, `7` (Artillery) have
no constructor among the thirteen, and `006E8320` sets `+8h` for none of them: it writes `+4h` and
then zeroes `+10h` through `+3Ch`, stepping over `+8h`. Five unattributed selectors plus the
Artillery anomaly point the same way: **`+8h` is very likely refined after construction**, by a
writer that splits artillery into three and machine-gun into two from authored data, with the
constructor constant only a default. That writer is not found yet. Until it is, no selector can be
published per barrel without inventing it, which is why this packet stops here rather than turning
the model on with a guessed mapping.

### Resolved: the refiner is `006E9890`, and the anomaly is not one

The prediction in the paragraph above is confirmed and the "anomaly" is closed, so **the Artillery
reject needs no explaining away: selector `4` is a pre-refinement value that no live projectile
keeps**. The writer is `006E9890 BSP_ProjectileClass_DeriveEngagementRange`, which the gunnery host
**already models** for ranges (`summary mission gunnery ... bullet_ranges_derived`). It rewrites
`[EDI+8]` on exactly the two classes that were unattributed, and leaves every other class on its
constructor constant (`006E99E5 JNE` jumps past the store to the `006E9A40` tail).

**The generic `Bullet` class, selector `1` (`006E9968 CMP ESI,1`).** It resolves the bullet's name
(`[EDI+14h]`, defaulting to the literal at `00E199AC`) and runs `00BF9440` against the literal at
`00CFA420`, whose bytes `41 41 00` are **`"AA"`**. Then `006E99D0 SETNE AL` / `006E99D5 ADD EAX,2`
/ `006E99D8 MOV [EDI+8],EAX`:

| Bullet name contains `"AA"` | Selector | Row |
| --- | --- | --- |
| no | `2` | MachineGun |
| yes | `3` | MachineGun |

Both land on arm `009FE2A2`, which is why the byte table's first two entries are both `00`, and the
authored Lua row is commented `geppityu es AA talalati esely` - **"machine gun *and AA* hit
chance"**. The authored comment and the byte table independently agree with the name test.

**The `Artillery` class, selector `4` (`006E99E2 CMP ESI,4`).** `006E99E7` loads `[EDI+0ACh]` and
bands it against two thresholds, each test also requiring the same threshold to exceed `[EDI+0B4h]`:

| Band | Selector | Row |
| --- | --- | --- |
| `[00CF0B50]` = `75.0f` beats both | `5` | Artillery |
| else `[00CE3808]` = `150.0f` beats both | `6` | Artillery |
| else | `7` | Artillery |

Three calibre bands under 75, under 150 and above, which reads as millimetres and matches the three
selectors arm `009FE313` serves. All three take the same Artillery accuracy row, so the banding
does not change the accuracy; it is the engagement-range derivation that needs it, and the accuracy
lookup simply tolerates all three.

**So the selector space is closed.** Every live value `2`-`12h` is now attributed, the two
in-range rejects are accounted for (`4` and `1` are pre-refinement values, `0Ch` and `0Eh` are the
two dummies), and `13h` WaterMine falls outside the range check. **Publishing a selector per barrel
is no longer blocked by an unknown**: the gunnery host already calls the routine that derives it,
and the remaining work is to carry `[EDI+8]` out of that derivation onto the published row, load the
`BulletTypeAccuracy` keys into `AiTuningBlock`, and resolve the target class at query time in
`AiWeightModelBinding::barrel_accuracy`, which already receives the target it currently discards.
That implementation and its IJN01 run are not in this packet.

**Attribution, and a process note against this packet.** The refinement rule above was **already
recorded** by the earlier packet `cc7_bullet_engagement_range_kinds`, in `006E9890`'s ledger
evidence and in `docs/BULLET_ENGAGEMENT_RANGE.md`: the `"AA"` test producing `2` and `3`, the
`75.0f`/`150.0f` calibre bands producing `5`, `6` and `7`, and the naming of `006E9890` as the
producer of those five sub-types. This packet re-derived it from the listing without checking, and
only found the prior record when appending evidence to the address. The re-derivation is therefore
**a confirmation, not a discovery**, and the credit is `cc7`'s. What is new here is the other half
of the join: that those sub-types are what `009FE270` switches on, which row of
`BulletTypeAccuracy` each reaches, and that `cc7`'s `[EDI+0ACh]` is `DamageMin` rather than a
calibre, which is what its own record already says.

The cost was avoidable. `python tools/bsp.py lookup 006e9890` answers this in one call, and this
packet reached the routine by a byte scan and read it cold instead. Look the address up before
reading it, even when a scan hands you the function name.

## The accuracy is published (packet `cc8_ai_bullet_type_accuracy`, 2026-09-18)

Three pieces, landed together because none of them turns the model on alone.

### 1. The selector reaches the row

`006E9890`'s reconstruction already computed the refined `+8h` and the gunnery host already called
it for the engagement range; it simply dropped the other half of the answer.
`src/game_hosts_gunnery.cpp` now keeps `finalised.sub_type` on `GameGunRow::bullet_sub_type` and
publishes it onto `GameAiWeaponFacts::Barrel::bullet_sub_type`. **`Barrel::accuracy` is deleted**,
because an accuracy is not a property a barrel has: it is a property of a (barrel, target) pair.

### 2. The tuning block carries the table

`bsp::AiTuningBlock` spans the whole `23Ch` record but `ai_tuning_keys()` loaded only 33 keys.
`ai_tuning_load_00a335d0` now also calls `ai_tuning_load_bullet_type_accuracy_00a335d0`, which
fills `110h`-`18Ch` from the authored table, and `ai_tuning_load_attacker_vs_target_00a335d0`,
which fills `05Ch` and `060h`.

That second one is not incidental. `00A08460` divides `DamageCalcTime` by each barrel's reload to
get its time factor, and `DamageCalcTime` was **not** among the 33 keys, so it was reading the
unloaded `0.0f`. Turning the model on without it would have multiplied every barrel's damage by
zero and answered a zero weight for every pair, collapsing candidate admission — a result that
would have looked like a finding and been an artefact. Both values are uniform across the seven
mode tables (`DamageCalcTime` 60, `MaxTargetKillRatio` 150.0, diffed across all seven).

`ai_load_globals_00a335d0` in `ai_target_weights.cpp` **still has no caller**, and this packet did
not give it one. It is the fuller reconstruction and wants an `AiGlobalsLoaderHost` backed by a
live Lua state; the only generic Lua reader in the process lives in `src/game_hosts_lua.cpp`, which
is leased to another worker. The path actually in use is the authored-reader one extended here.
Naming that rather than duplicating it.

### 3. The lookup resolves the target at query time

`AiWeightModelBinding::barrel_accuracy` received a target argument and discarded it. It now takes
the tuning block and a target-group callback, and answers
`ai_bullet_type_accuracy_offset_009fe270(sub_type, group)` out of the block. The dispatch is the
byte table and the arms, written as a pure function so it can be read against the listing.

The group is `Impl::accuracy_target_group`, which is arm `009FE2A2`'s own shape: `PUSH 0Fh` the
plane base, else `PUSH 6` the ship base split by `00827F70`, else the fall-through. Two labelled
substitutions, both narrow:

* **The query family.** The native asks the vehicle class descriptor's `vtable[+18h]`; this asks
  the instance's `vtable[+5Ch]` through `unit_is_kind_of`. The **codes** are the native's, because
  `VehicleClassKind` and the entity class ids are one id space selected from the same
  `VehicleClass.Type`. The vtables are still two different vtables.
* **The small-ship split.** `00827F70`'s two codes are exact, read from `00827F78 PUSH 0Eh`
  TorpedoBoat and `00827F89 PUSH 0Ch` LandingShip. Its third condition, the `BigLandingShip` byte
  at class`+808h`, has no producer here, so **a big landing ship is classed small** where the
  native would class it big.

### What stays incomplete, and why that is a flag rather than a zero

`Barrel::accuracy_resolved` is false for exactly one sub-type, Rocket (`12h`), whose SmallRocket and
BigRocket blocks `009FE4F1` chooses between through target-state predicates `006E3260`, `007B80A0`
and `007B80C0` that are not read. A unit carrying any rocket barrel keeps its row **incomplete** and
therefore keeps the class stand-in, rather than scoring that barrel at a fabricated zero. Six of the
120 authored bullet classes in this installation are `Rocket`, so the shortfall is small; the census
line now prints `complete_rows` beside `weapon_rows` so the cost is visible rather than assumed.

A resolved sub-type that answers offset `0` is a different thing and is passed through as a real
`0.0f`: that is the reject arm, and `00A094F5 FCOMIP / JNC` skipping the barrel is the native's own
behaviour. MachineGun against a big ship or a landfort, Artillery and Bomb against a plane, and a
torpedo against a torpedo boat all legitimately answer zero.

### Measured, and the model is left switched OFF

Three IJN01 runs, one build each, `--frames 3200 --mission-frames 3000`.

| Run | Log | `served` | `attackmove` | `settarget` | `fallback` | `scored` | `model_runs` | `complete_rows` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| before, model off | `local/acc_ijn01_before.log` | 2450 | 2250 | 141 | 59 | 465500 | 0 | - |
| after, model on | `local/acc_ijn01_after.log` | 2450 | **0** | **0** | **2450** | **4900** | 465500 | 321 |
| after, gated on health | `local/acc_ijn01_after2.log` | 2450 | 0 | 0 | 2450 | 4900 | 465500 | 321 |

The before run reproduces `local/tw_ijn01.log` from the census packet exactly, on a tree with main
merged, so the pair is the clean A/B the earlier census could not be.

**Turning the model on is a regression, and it is not shipped.** `model_runs` reaches 465500 and
`class_stand_ins` falls to 0, so the plumbing works and every row is complete. But the model answers
a zero weight for **460600** candidates, which is exactly IJN01's `fort_targets`, so `scored`
collapses from 465500 to 4900 and every one of the 2450 served members takes the fallback moveto:
**the AI issues no attack orders at all.** `inputs_complete` is therefore held at `false` in
`src/game_hosts_gunnery.cpp`, one line, with the three pieces left in place for the packet that can
flip it.

**A first reading of that collapse blamed the target hit points and was wrong.** The argument was
that this installation's `vehicleclasses.lua` has no `Landfort` `Type` row, so the 239 LandFort
entities — registered from a `LandFortClasses` table that is not among the loose scripts — would
resolve no `hp`, and `ai_target_weight_result` answers 0 whenever `target_hit_points` is 0. The
second run tested it by requiring `hit_points > 0` for completeness and **refuted it**:
`complete_rows` stayed at 321 and not one other number moved, so every row carries real health. The
condition was removed again rather than left in as a harmless-looking extra.

What remains is `00A08460`'s own coverage rather than anything this packet publishes. Its
attacker-is-type-`0Fh` branch `00A0861F..00A09222` is unprojected, and
`AiWeightModelBinding::entity_is_type` and `entity_kind` both answer a constant, so the model takes
its no-bonus arms for every attacker. Whether the residue is that, or authored-zero accuracy for the
attackers' actual barrel types against the fall-through group, is **not settled here** and is the
next packet's question. The counters that would settle it are a per-arm barrel census inside
`barrel_accuracy`, which no run so far carries.

#### What was checked and did not explain it

Three candidates were eliminated rather than left hanging, so the next packet does not re-walk them.

* **The published selector is the refined one.** `weapon_class_derive_engagement_range` assigns
  `out.sub_type = weapon_class_refined_sub_type(in)` at `src/bullet_engagement_range.cpp:96`, so an
  artillery barrel publishes `5`, `6` or `7` and not the constructor's `4`, which the reject arm
  would have turned into a silent zero. The latch early-return above it cannot fire here: the
  gunnery host builds a fresh input per gun.
* **Target hit points.** Refuted by the third run, as above.
* **`ForcedTargetWeightValues`.** `AiWeightModelBinding::forced_rule_weight` stubs `00A31DB0`, and
  that table is real and unrun: `scripts/datatables/highlvlaiglobals.lua:164` and the six repeats,
  rows of `{ attacker class, target class, target-is-neutral, weight }` that override one pair's
  weight outright. It is a genuine missing producer and worth its own packet. It does **not**
  explain this collapse: the shipped rows are mostly overrides **to zero**
  (`TORPEDOBOAT`/`COMMANDBUILDING`, `TORPEDOBOAT`/`SHIP`, `SUBMARINE`/`COMMANDBUILDING`), the two
  positive ones raise `TORPEDOBOMBER` and `DIVEBOMBER` against `SHIP`, and no row covers an
  AA-armed ship against a landfort. Running it would zero more pairs here, not rescue any.

The arithmetic that remains consistent with every measurement: `damage = time_factor * accuracy *
shots`, `time_factor` is now non-zero and `shots` is at least 1, so a zero total means **every
barrel's accuracy is zero**, and the split is exact — all 188 in-range trio candidates per member
pass score zero while both non-trio candidates score positive. Two authored rows have exactly that
shape, `MachineGun` (`0.10, 0.15, 0, 0.0`) and `Flak` (`0.50, 0.20, 0.00, 0.00`): positive against a
plane or a small ship, zero against a big ship or a landfort. That fits an AA-armed party-0 force
scoring its two nearby aircraft and nothing else, which would make the accuracy **correct** and the
missing attack orders a consequence of the unprojected plane-attacker branch instead. It is a
hypothesis with no counter behind it yet, which is why the flag stays off rather than the reading
being written up as settled.

## The zero-weight question (packet `cc8_ai_target_weight_zero`, 2026-09-18)

### The gate on `00A0861F`, read from the bytes

The branch is gated on a byte, and the byte is the **attacker**'s class, which settles that the
label "attacker-is-type-`0Fh`" is right and that the target tests inside it are subordinate to it:

```
00a085ad  push 0Fh            ; PlaneBase
00a085af  mov ecx,ebp         ; EBP, the attacker vehicle class, EDX at entry
00a085bd  call edx            ; vtable[+18h]
00a085bf  mov [esp+37h],al
...
00a0860a  cmp byte ptr [esp+37h],bl
00a08619  je  00a09228        ; NOT a plane -> the subsystem and barrel walk
00a0861f  ...                 ; a plane -> the unprojected region
```

So `00A09228` onward, the subsystem walk at `00A09379` and the accuracy at `00A094E6`, is the
**non-plane** path, and it is the one this process projects. A plane attacker never reaches it in
the native. Inside the region the first two queries are on `EDI`, the target
(`00A08624 PUSH 0Fh`, `00A08633 PUSH 8`), which is why a quick reading can mistake the whole region
for a target branch; `EDI` is the first stack argument, the target, and `[EDI+4Ch]` at `00A085A8` is
the capture state the projected path already reads.

`coverage: partial` and deliberately so. The region is about 3 KB and is a **different damage
model**, not a variation on the barrel walk: it reads target-class fields `+28h`, `+30h`, `+34h`,
`+3Ch`, `+58h`, `+5Ch`, `+64h`, `+68h`, `+70h`, `+74h`, `+7Ch` and `+80h` in ratio pairs, asks class
queries `25h`, `6`, `10h` and `1Ch`, and calls `00443490`, `00731040`, `009552E0` and `009FF3A0`.
Projecting it is a packet of its own and this one did not attempt it.

One cross-link worth having: `00A08870`, `00A0888B` and `00A088A0` call **`006E3260`, `007B80A0`
and `007B80C0`** — the same three target-state predicates that `009FE270`'s rocket arm `009FE4F1`
branches on. Reading those three once resolves the rocket split and part of this branch together,
which makes them the highest-value next read in this area.

**Its shape, which matters for what this packet published.** The region's tail is not a separate
accuracy model: it calls **`009FE270` itself**, at `00A08E60`, `00A08F42`, `00A0909A` and
`00A091F1`, with `00A08E9C` fetching the same `00A371A0` tuning record. Each of those four sites
sits in a repeating group with `009FE200`, the distance falloff, `00A001D0`, a per-slot query, and
`00415550`, a max — the same accuracy-times-falloff-then-max shape the barrel walk has, iterated
over a plane's ordnance slots through `00A001D0` and `00A07A60` instead of over subsystem barrels.
Class queries seen across the region: `0Fh` and `8` on the target at the head, then `25h`, `6`,
`10h`, `1Ch`, `17h`, `20h` and `14h`.

So **the BulletTypeAccuracy table this packet loaded and the dispatch it implemented serve both
paths**. Whatever the census says about IJN01, the tuning load and
`ai_bullet_type_accuracy_offset_009fe270` are not wasted on a plane-attacker projection: that
projection would call straight into them.

### The census, and why it is an observation pass

`inputs_complete` stays `false`, so the model still does not run and the stand-in still scores. The
census therefore could not live inside `barrel_accuracy`, which is only reached when the model runs
— measuring it there would have required switching on the regression it is meant to diagnose. It is
instead a pure observation pass in `close_target_weight`: for every candidate it reads what
`009FE270` **would** answer for each of the attacker's barrels, and the time factor and barrel
contribution `00A08460` would build from it, and stores nothing back. Behaviour is unchanged by
construction.

Two lines come out of it. `summary mission ai target weight path` splits every query by whether the
attacker is a plane, which says whether IJN01's close-attack members take the projected barrel walk
at all or the unprojected region. `summary mission ai target weight accuracy` gives one row per
(bullet sub-type, target group) pair with the looked-up accuracy, the zero count and the summed
contribution, which says whether the zeros are authored or a coverage gap.

### The census, measured: `local/zero_ijn01.log`

**Both of the candidates the last packet named are refuted.** The run is on the old command-target
rule, i.e. **before** `0daec4b56`, the same side as every other column in this document.

`path`: `plane_attacker=38000 other_attacker=427500`. So **92% of queries take the projected
non-plane barrel walk**, and the unprojected region `00A0861F..00A09222` is not where IJN01's
weights come from.

`accuracy`, the rows with a non-zero lookup count:

| sub-type | group | lookups | accuracy | zero | contribution |
| --- | --- | --- | --- | --- | --- |
| `00h` unresolved class | other | 37600 | 0.0000 | 37600 | 0.000 |
| `02h` machinegun | other | 84600 | 0.0000 | 84600 | 0.000 |
| `03h` machinegun AA | other | 2566200 | 0.0000 | 2566200 | 0.000 |
| `06h` artillery medium | other | 921200 | **0.5500** | 0 | **12370968.9** |
| `07h` artillery heavy | other | 423000 | **0.5500** | 0 | **800611.4** |
| `09h` bomb | other | 65800 | **0.2000** | 0 | **13160.0** |
| `0Ah` torpedo | other | 338400 | 0.0000 | 338400 | 0.000 |
| `0Bh` depthcharge | other | 94000 | **0.5000** | 0 | **1410000.0** |
| `10h` flak | other | 564000 | 0.0000 | 564000 | 0.000 |

(the `submarine` rows mirror these at about a ninetieth of the volume, with `06h`/`07h` at `0.7000`
and `0Ah` torpedo at `0.4500`, which is the torpedo arm's submarine entry firing correctly.)

**So the zeros are not authored.** IJN01's attackers carry artillery, bombs and depth charges whose
authored accuracy against the fall-through group is `0.55`, `0.20` and `0.50`, and their summed
contribution is over 14 million. The "AA-armed force" reading in the previous section is **wrong**
and is retracted: machine-gun and flak barrels do answer zero there, but they are not the only
barrels these units carry.

Two things the census does **not** establish, said plainly. Only the `submarine` and `other` groups
were ever exercised — no candidate in the 3000-unit collect radius classified as plane, small ship
or big ship — so the `Plane`, `SmallShip` and `BigShip` rows of the dispatch are **untested by this
run**, and the labelled `00827F70` substitution with them. And `00h`, an unresolved bullet class,
accounts for 37600 lookups, which is the `gun.bullet_class < 0` guard in the gunnery host rather
than anything in this dispatch.

### The actual cause: a null subsystem handle

Neither candidate. `src/ai_target_weights.cpp` walked the barrels with

```cpp
const void* subsystem = nullptr;   // resolved natively at 00A09379
```

and `AiWeightModelBinding` keys every barrel accessor on that pointer through
`index_of(entity) = (std::size_t)entity - 1`. For a null handle that is `SIZE_MAX`,
`GameAiWeaponFacts::row` bounds-checks it away, and **`barrel_count` answers 0**. The inner loop
therefore never executed once: `best` stayed `0`, `capture_accumulator` stayed `0`, `total` stayed
`0`, and `ai_target_weight_result` returns `0` for a non-positive total. Every candidate, every
mission, whatever the accuracy table said.

That also explains the one number that never fitted the authored-zero reading: the 4900 that still
scored. They were not admitted on their weight at all. `00A146D9`'s second arm admits a candidate
whose weight is not positive when it is in the target group, which is exactly
`ai_close_attack_candidate_admitted(0.0f, true)`.

`subsystem_count` returning `1` and the binding's own comment that "the attacker has a single
subsystem carrying every barrel" are the contract; the fix is to hand the walk the attacker handle,
which is what resolves back to that flattened subsystem. The native's `00A09379` resolving a real
subsystem object stays the labelled substitution it already was.

### A second annihilating stub, of the same family

Fixing the handle was not enough, and the second fault is worth stating because it is the same
mistake twice: **a stand-in whose contract does not match its caller's.**

`AiWeightModelBinding::distance_falloff` answered `return a`, on the reading that `a` was the value
being scaled and returning it kept the value undiminished. The one call site,
`src/ai_target_weights.cpp:379`, **multiplies by the answer** and passes `(0, 0, 0, 0)` — four
distance arguments this projection has not recovered. So the stub answered `0.0f`, `weighted` was
zero for every barrel, `best` could never leave `0`, and the barrel loop still contributed nothing
to `total`. A falloff is a multiplier, so its neutral value is `1.0f`. Labelled: the real `009FE200`
diminishes with range, so `1.0f` over-states a distant barrel rather than annihilating it.

With both fixed the model's shape is finally the listing's:
`total = best + min(sum_damage, DamageCalcTime)`, clamped by hit points and `MaxTargetKillRatio`,
where `best` is the largest single barrel contribution.

### The prediction, written before the run reported

Registered in advance so the check is a test and not a fit. From the census, `06h` artillery medium
against the `other` group contributed `12370968.9` over `921200` lookups, so one such barrel's
damage averages **13.43**. A unit with several of them saturates the capture term, which clamps at
`DamageCalcTime` = 60. So

```
model ~= (13.43 + 60) / hit_points        a fort, artillery at 0.55
model ~= (17.1  + 60) / hit_points        a submarine, artillery at 0.70
```

and `00A0F810` then multiplies by the class weight in `target_scale` and by `0.01` for a
non-command member of the trio. The authored class weights are `Landfort` **1.0** and `Submarine`
**4.0**, so

```
fort      ~= (73.4 / hp_fort) * 1.0 * 0.01   ~= 0.0004  at hp 2000
submarine ~= (77.1 / hp_sub)  * 4.0          ~= 0.2      at hp 1500
```

**Predicted: the real model prefers the submarines and ships by roughly three orders of magnitude
over the forts**, which is the `0.01` static-installation arm doing exactly what section 1 says it
does, amplified by the class weight. Predicted coordinator row: `served` unchanged at 2450,
`attackmove` and `settarget` back above zero, `fallback` small, and `scored` back near 465500
because every weight is now positive so admission stops depending on the target-group arm.

If the run instead shows attacks still off, or shows forts preferred, the flip comes back off.

### Measured: `local/zero_ijn01_fixed.log`, and the prediction held

All three columns are on the **old** command-target rule, before `0daec4b56`.

| Run | `served` | `attackmove` | `settarget` | `fallback` | `scored` | `model_runs` | `complete_rows` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| before, model off | 2450 | 2250 | 141 | 59 | 465500 | 0 | - |
| model on, both bugs present | 2450 | **0** | **0** | **2450** | **4900** | 465500 | 321 |
| model on, both fixed | 2450 | **2050** | **141** | **259** | **427900** | 465500 | 321 |

| Prediction | Measured | Verdict |
| --- | --- | --- |
| `served` unchanged at 2450 | 2450 | **held** |
| `attackmove` and `settarget` back above zero | 2050 and 141 | **held** |
| `fallback` small | 259, against 2450 broken | **held** |
| `scored` back near 465500 | 427900, short by 37600 | **held with a named residue** |

**The AI attacks again**, and `settarget` returns to exactly its before value of 141. The residue is
exact rather than approximate: `465500 - 427900 = 37600`, which is precisely the census's `00h`
lookup count against the `other` group. Sub-type `00h` is an **unresolved bullet class**, the
`gun.bullet_class < 0` guard in the gunnery host, so those attackers publish a zero-accuracy barrel,
answer a zero total and fail admission. That is a known, already-named gap and not a new fault. One
caveat on the identification: the census counts barrel lookups and `scored` counts candidates, so
the two coincide exactly only if those attackers carry one barrel each. Suggestive, not proof.

Second-order movement, before to after: gunnery `entity_impacts` 104 to **1891** with `hull` steady
at 104 and `water` 69 to 1852, `total_damage` 1571.8 to 1579.2, `deaths` 4 either way. So the same
hulls are being hit for the same damage while far more rounds fall in the water, which is what a
changed target preference at unchanged gunnery accuracy looks like.

**What could not be checked, stated plainly.** The sampled-candidate arithmetic was **not** verified
against the run. The prediction's `0.0004` for a fort and `0.2` for a submarine used assumed hit
points of 2000 and 1500, and no line in the run records a per-candidate weight, its hit points or
the chosen target — the same instrumentation gap this document flagged after the first census. So
the claim that the model now prefers submarines and ships over forts by about three orders of
magnitude is a **calculation from the census and the authored class weights, not a measurement**.
What the run does show is consistent with it and does not test it: `attackmove` fell 2250 to 2050
and `fallback` rose 59 to 259, so 200 member-ticks that used to find a target no longer do, which is
what devaluing 188 of every 190 in-range candidates by the `0.01` arm would do.

The flip therefore stays on: every measurable part of the prediction held, and the one unmeasurable
part is labelled as unmeasured rather than counted as confirmation.

## The choice observed (packet `cc8_ai_target_choice_observed`, 2026-09-18)

### Which commits every column in this document sits before

Two baseline-moving commits bound these measurements, and both are named rather than assumed.

**`0daec4b56`, the `0071EBF0` command-target rule.** Every column in the sections **above** predates
it; every column in **this** section is after it. See below.

**`67e8ac821`, the `Hidden = B true` hold-back.** **Every column in this document, including all
three runs in this section, predates it.** That commit makes the scene loader hold back every
authored object whose block sets `Hidden`, as `0046D3B5` does, so such objects are registered but
not created until a mission script spawns them and missions load emptier: USN04 creates 19 units
instead of 53 and USN01 62 instead of 77, with IJN01 and USN02 uncounted. Candidate counts, served
counts, the chosen-class table and every gunnery number can all move across it, so **no column here
may be compared with one taken after it**. The binary these three runs used was built at 22:22 from
a tree without it.

### Which side of `0daec4b56` each column is on

Every column in the sections **above** predates `0daec4b56` and is on the old "last current row wins"
command-target rule. Every column in **this** section is on the image's `0071EBF0` rule. The
baseline moves across it: IJN01 model-off went from `settarget` 141 / `fallback` 59 / `scored`
465500 on the old rule to **166 / 34 / 514990** on the new one, so old and new columns are not
comparable and are not compared.

The three runs below come from **one binary**, using the `BSP_AI_WEIGHT_MODEL` switch this packet
added, and ran concurrently in slots 0, 1 and 2 on cores 2, 4 and 6.

### IJN01, new rule, one binary

| Column | `served` | `attackmove` | `settarget` | `fallback` | `scored` | `model_runs` | `complete_rows` |
| --- | --- | --- | --- | --- | --- | --- | --- |
| model off | 2450 | 2250 | 166 | 34 | 514990 | 0 | 321 |
| model on | 2450 | 2206 | 166 | **78** | **475370** | 514990 | 321 |

### What the model actually prefers, measured

This is the line no earlier run carried. Chosen counts, with the runner-up counts beside them:

| Class | off, chosen | off, runner-up | on, chosen | on, runner-up |
| --- | --- | --- | --- | --- |
| `08h` Submarine | 2344 | 539 | **2104** | 573 |
| `11h` TorpedoBomber | 20 | 1877 | **214** | 1799 |
| `13h` Fighter | 15 | 0 | 17 | 0 |
| `1Bh` LandFort | 37 | 0 | **37** | 0 |

Both columns' chosen counts sum to `attackmove + settarget` exactly (2416 = 2250 + 166, and
2372 = 2206 + 166), so the table is complete rather than sampled.

**The prediction was right about forts and wrong about what the model does.** The calculation in the
previous section argued the model would move preference off the forts by about three orders of
magnitude. Measured: **the land-fort count does not move at all, 37 either way**, because the
stand-in was already picking forts only 37 times in 2416 — the `0.01` arm had nothing left to take
away. The effect the model actually has is one the calculation never considered: **torpedo bombers
go from 20 chosen to 214, a factor of ten**, taken mostly out of the submarines, which drop 2344 to
2104. The real model prefers **aircraft** more than the class-weight stand-in does. That was not
predicted and is recorded as a miss.

### The sampled arithmetic, with real hit points

Model **off**, a sampled choice: `chosen_class=08h(Submarine) weight=4.000000 hp=200.0`. The
stand-in makes `base_weight` `1.0`, `target_scale` the authored class weight — `Submarine` is
**4.0** — and the health and trio slots `1.0`. Predicted `4.0`, measured `4.000000`. The
`00A0F810` product is exact on a real candidate.

Model **on**, the same class at the same `hp=200.0`, weights `0.336`, `0.280`, `0.240`, `1.536`,
`0.504`. Dividing out the class weight gives the model's own answer, and multiplying by the real hit
points gives its `total`:

| weight | `/4.0` = model | `x hp 200` = total | reads as `best + min(sum, 60)` |
| --- | --- | --- | --- |
| 0.336 | 0.084 | 16.8 | one barrel: `best` 8.4, `sum` 8.4 |
| 0.240 | 0.060 | 12.0 | one barrel: `best` 6.0, `sum` 6.0 |
| 1.536 | 0.384 | 76.8 | `best` 16.8 with `sum` at or past the 60 clamp |

Every sampled total is either twice a single barrel's damage or a barrel plus exactly 60, which is
`total = best + min(sum_damage, DamageCalcTime)` with `DamageCalcTime` = 60. **The listing's
arithmetic and the run agree on real numbers.**

**My earlier estimate of a submarine's hit points was wrong by more than a factor of seven**: I
assumed 1500, the measured value is **200.0**. That is exactly why the prediction's absolute
magnitudes could not be trusted and why this run was needed.

**The fort side was not sampled.** The 24-choice window caught only submarines, so no land-fort
weight or hit points were measured, and the fort-versus-submarine ratio remains a calculation. What
*is* measured is the aggregate that matters more: forts are chosen 37 times out of 2416 with the
stand-in and 37 out of 2372 with the model.

### USN02, the ship control, new rule, model on

`served` 616, `attackmove` 559, `settarget` 0, `fallback` 57, `scored` 4004, `model_runs` 4004,
`complete_rows` 32 of 32. **Identical to every previous USN02 coordinator row in this document**,
across both the old and the new command-target rule and with the model on rather than standing in.
Chosen classes: `07h` Destroyer 471 and `0Ah` Cruiser 88, summing to 559 = `attackmove`. The control
holds exactly.

### The unresolved bullet class, attributed

`unresolved_bullet_class attacker=0Ah(Cruiser) barrel_lookups=21020` and
`attacker=0Dh(BattleShip) barrel_lookups=21020`, identical in both IJN01 columns. **Cruisers and
battleships, not planes.** An earlier reading noticed that sub-type `00h` totalled 38000 lookups
while `plane_attacker` was also 38000 and flagged the coincidence as not evidence; it was right to,
because the attribution is nothing to do with planes.

The leading explanation, with its supporting fact and its problem both stated: of the 416 device
classes in this installation **only the 20 `CATAPULT` rows author no `Bullet` block at all**, so a
catapult built as a gun row takes `gun.bullet_class = -1` and publishes a sub-type `0` barrel; and
catapults are mounted mostly on battleships and cruisers. **The problem with it** is that destroyers
mount them too and show no unresolved lookups at all, so the explanation is incomplete as it stands
and is **not** accepted here. The check that would settle it is whether the gunnery host builds a
gun row for a `CATAPULT` device at all, the same question `docs/ORDNANCE_KIND_IDENTITY.md` already
answered "no" for `BOMBPLATFORM`; that file is leased elsewhere, so this packet did not run it.

One thing the attribution does **not** explain: the admission shortfall. `scored` falls 514990 to
475370, a difference of 39620, against 42040 unresolved barrel lookups. The two are close and are
not equal, and a ship carrying a catapult also carries real guns whose barrels resolve, so those
attackers are not zero-weight and the shortfall cannot simply be them. It is left open rather than
fitted.
