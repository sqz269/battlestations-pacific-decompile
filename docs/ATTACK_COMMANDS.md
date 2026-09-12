# The attack commands: two routines hold the behaviour of all seventeen

Addresses: `007EE8F0` `007EEC00` `007EEC50` `007ED7E0` `007ED830` `007ED880` `007ED8D0` `007ED970`
`007EDA10` `007B91C0` `007B9230` `007B9320` `007B93E0` `007B93F0` `007B9480` `007B94F0` `007B9500`
`00922B10` `00922C80` `0099A170` `009F6D80` `009F8160` `009F9770` `009F3D00` `00534870` `00811F50`
`007F16D0` `00816E30` `0071DEE0` `008A4960` `008A4B10` `008A4F00` `008A50D0` `008A5310`
`009A28B0` `009A2920` `009A54D0` `009A5DB0` `009A99E0` `009A9CC0` `009ADDC0` `009AE140` `009AF060`
`009AF560` `009B7BB0` `009B81F0` `009C79A0` `009C8060` `009CC3D0` `009CC850` `009D3280` `009D3EF0`
`007B6CD0` `007B7200` `00D7A23C` (data) `00E08F10`..`00E08FC0` (the singleton block)

Packet `cc2_attack_commands`, the follow-up to `docs/COMMAND_CLASSES.md`'s `contract: unread`
section. Every descriptive name here is a hypothesis, not a recovered symbol. The 26 class
descriptors, their names, categories and `requires_target` bits are `docs/SCENE_COMMAND_TYPES.md`'s
and are not repeated. The controller's ten-slot queue, its mode byte and its stages are
`docs/COMMAND_EXECUTION.md`'s; the controller object, its permission bytes and `00835860` are
`docs/WEAPON_DIRECTOR.md`'s; `cruise` is `docs/CRUISE_COMMAND.md`'s and the commanded-speed stages
(`009E1170`) are `docs/UNIT_COMMANDED_SPEED.md`'s. All of those are cited here as contracts.

## Headline

`docs/COMMAND_CLASSES.md` established that a command class is four constant getters and nothing
else, and expected the behaviour to be in the unit command controller. For the ten attack classes
it is not there either. **Two routines hold it.**

* `007EE8F0` answers, for one class, one target and one ammunition loadout, whether that class may
  be used. It is one `CMP EAX,<singleton>` chain with eleven arms.
* `007EEC50` runs `007EE8F0` over the classes in a fixed preference order and returns the class that
  wins.

Everything downstream consumes the class that came out of those two: the bot task factory
`0099A170`, the per-tick revalidation `009F8160`, the AI target weight lookup `009F9770`, the bot
sub-controller map `009F3D00` and the HUD order icon `00534870`. The controller's own arms for these
classes reduce to two rewrites, described under "What the controller does".

`vtable[5Ch]` is the entity `IsKindOf(int)` test established by `docs/UNIT_TIMED_SUBUPDATES.md` and
`docs/WEAPON_DIRECTOR.md`; every kind id below is a literal pushed immediately before a call to it.

## Method, and a correction to the reference table

`ghidra xrefs` under-reports the singleton references, so the executable was scanned directly for
the four-byte little-endian value of each of the 26 singletons (`local/scan_cmd_singletons.py`).
That found 533 sites and reproduced `docs/COMMAND_CLASSES.md`'s per-class counts exactly, so that
document's count column is the immediate count and needs no correction.

Its **site attribution** does need one. Twenty of the sites this packet used fall outside every
Ghidra function body, so they were read from the raw listing with capstone
(`local/raw_fn.py`); they are marked `no_ghidra_function` in `reports/attack_commands.json` with
inclusive ends. The bisect-to-nearest-function-start attribution that produced the first pass of
the site table was wrong for all twenty: for instance `009A54D1` is not inside `009A5420` (which
ends at `009A5471`) but inside the six-byte unnamed getter at `009A54D0`.

`docs/COMMAND_CLASSES.md` records `00811F50` as "the shape of a name-to-singleton table". **It is
not a table.** `00811F50` is a predicate; see "The suppression predicate" below.

## `007EE8F0`, the feasibility test

`char __thiscall(ECX = unit)(CommandClass* klass, Entity* target, char target_is_surface,
int loadout)`, `RET 10h` at `007EE920` and six other exits, body `007EE8F0`-`007EEBFB`.

The ABI comes from the assembly, not the pseudocode: Ghidra models the first argument as
`unaff_retaddr`, so the decompiled parameter list is shifted by one and unusable. `007EE8F2` takes
the target from `[ESP+10h]` and `007EE934` takes the class from `[ESP+0Ch]`, which with the two
entry pushes fixes the four stack arguments in that order. `007EEC00` supplies them
(`007EEC2C`), and `009F823C` supplies them independently with the same order, which is the
cross-check.

**The side gate**, `007EE8F6`..`007EE932`, runs before any arm. The target must exist and the unit
must have a weapon controller at `+3D0h`. A target that answers `IsKindOf(1Ch)` (a structure) passes
when its `+54h` differs from the unit's `+54h`. Any other target must both differ and not be side
`2`, which `007EE928` compares by immediate. Side `2` is therefore excluded as a target for every
attack class except against structures.

**The arms.** `attackmove` is first and unconditional (`007EE938`, `MOV AL,1`). The remaining ten:

| class | site | what it requires |
| --- | --- | --- |
| `attackmove` `00E08F78` | `007EE938` | nothing |
| `levelbomb` `00E08F28` | `007EE946` | surface target; unit `IsKindOf(10h)`; then either level-bomb ordnance and target `IsKindOf(1Ch)`, or general bomb ordnance and target not `IsKindOf(0Eh)` |
| `divebomb` `00E08F20` | `007EE9C5` | surface target; unit **not** `IsKindOf(10h)`; general bomb ordnance; target not `IsKindOf(0Eh)` |
| `rocket` `00E08F48` | `007EEA20` | surface target; rocket ordnance; target not `IsKindOf(0Eh)` |
| `torpedo` `00E08F18` | `007EEA40` | torpedo ordnance; a surface-running torpedo needs a surface target; `00828EC0(target+538h)` must answer 0 |
| `depthcharge` `00E08F38` | `007EEAB3` | target `IsKindOf(8)`; depth-charge ordnance. The only arm that never reads the surface flag |
| `dogfight` `00E08F58` | `007EEAEC` | target airborne (`00922B10`); controller `+C24h` set; unit **not** `IsKindOf(16h)`; `0047B850` answers 0 |
| `kamikaze` `00E08F50` | `007EEB36` | surface target; unit `IsKindOf(17h)`; if the target answers `IsKindOf(6)`, then `00827F70(target+538h)` and `00604A50(controller)` must not both refuse |
| `strafe` `00E08F40` | `007EEB94` | a surface target, **or** a target answering `IsKindOf(41h)`; controller `+C24h` set; `0047B850` answers 0 |
| `dropkamikaze` `00E08F30` | `007EEBCC` | surface target; drop-kamikaze ordnance |
| anything else | `007EEBF6` | falls through to `XOR AL,AL` |

`IsKindOf(10h)` on the unit's `+3D0h` object is the single discriminator between `levelbomb` and
`divebomb`: the two arms test it with opposite senses, so exactly one of the two can ever apply to
the same unit. The same test at `008A5045` is what `luaMW_PilotBomb` uses.

The torpedo depth band, `007EEA6C`..`007EEA8E`, reads the first weapon slot's descriptor through
`007B9230`, requires its `+8h` to equal `0Ah`, and clears the surface-runner bit unless the float
pair `+F0h`/`+F4h` brackets the constant at `00D7A23C`. That constant is `6F 12 83 3A` = `0.001f`,
read from the image. The x87-free SSE compares (`COMISS`/`JA`/`JC`) were read from the listing.

### The six ordnance queries

Each of `007ED7E0`, `007ED830`, `007ED880`, `007ED8D0`, `007ED970` and `007EDA10` is the same loop
over the unit's `+3CCh` count, and `007B91C0` is the shared body: for each of the weapon
controller's `+994h` slots at `+974h`, take `slot->vtable[220h](loadout)` and ask the descriptor
`descriptor->vtable[8](kind)`. Only the kind differs.

| helper | predicate | kind | class |
| --- | --- | --- | --- |
| `007ED830` | `007B9500` | `31h` | `levelbomb`, primary |
| `007ED7E0` | `007B9320` | `2Ah`, and none of `2Ch`, `31h`, `2Bh`, `33h`, `2Dh` | `divebomb`, `levelbomb` fallback |
| `007ED880` | `007B93E0` | `2Fh` | `dropkamikaze` |
| `007ED8D0` | `007B93F0` | `2Bh` | `torpedo` |
| `007ED970` | `007B9480` | `33h`, and descriptor `+E0h` > 0 | `rocket` |
| `007EDA10` | `007B94F0` | `2Ch` | `depthcharge` |

`007B9320`'s exclusion chain is the reason `divebomb` is the residual class: a slot that carries any
ordnance with its own command class is not counted as a general bomb.

The fourth argument threaded into `vtable[220h]` is a loadout index, not a flag. `007EEC00` computes
it as `(unit+369h && 00E17BF2) ? 1 : 0`; `009F824A` passes `007B58D0`'s value instead. `007B9230`
hard-codes `1`.

### The two target classifiers

`00922B10(entity)` is "airborne": non-null, `+5Dh` clear, and `IsKindOf(0Fh) || IsKindOf(18h)`.
`00922C80(entity, allow_far)` is "attackable surface or ground target": non-null, `+5Dh` clear,
neither aircraft kind, then `IsKindOf(6)`, `IsKindOf(45h)`, `IsKindOf(46h)`, `IsKindOf(1Ch)` or
membership of the unit set `BSP_SzurkeNyil_ContainsUnit` tests, with a further `IsKindOf(35h)` and
height branch under `allow_far`. Its result is the `target_is_surface` byte every arm above reads.

## `007EEC50`, the chooser

`CommandClass* __thiscall(ECX = unit)(Entity* target, char prefer_ordnance, char allow_guns)`.

Two early exits, then two passes.

1. No weapon controller at `+3D0h` (`007EEC5A`): return `moveto` `00E08F68`. This is the only
   non-attack class the routine can return.
2. The side gate fails: return 0.
3. **Ordnance pass**, skipped entirely when `00922B10` says the target is airborne. Tries in this
   order and stops at the first that applies: `levelbomb`, `dropkamikaze`, `divebomb`, `torpedo`,
   `rocket`, `kamikaze`, `depthcharge`.
4. If `prefer_ordnance` is set and the ordnance pass found one, return it.
5. **Gun pass**: `dogfight`, then `strafe`. If `allow_guns` is clear, the result is discarded and 0
   is returned.

The two passes are exactly the two categories. `docs/SCENE_COMMAND_TYPES.md` gives category 2 to
`torpedo`, `divebomb`, `levelbomb`, `dropkamikaze`, `depthcharge`, `rocket` and `kamikaze` - the
seven of the ordnance pass, and the same seven `009F6D80` tests - and category 1 to `artillery`,
`strafe` and `dogfight`, of which the gun pass tries the two that reach it. `artillery` cannot,
because the controller has already replaced it.

## What the controller does

The controller's arms for these classes are two rewrites, both in `00816E30`
(`BSP_UnitInstance_ApplyEntityCommand`), taken before the command reaches a queue slot.

| site | class | what happens |
| --- | --- | --- |
| `00817229`, `00817239` | `artillery` `00E08F10` | replaced by `attackmove` `00E08F78` outright. `artillery` never occupies a slot as itself |
| `00816FC0` | `land` `00E08FA0` | replaced by `attackmove` unless the unit answers `IsKindOf(0Ch)` |
| `00816EFE` | `Leave` `00E08FB0` | resolves the target, keeps it only if it answers `IsKindOf(2)`, then acts at once |
| `00816F43` | `disband` `00E08FB8` | calls `0077CA60` and returns |

`0071DEE0` (`BSP_WeaponDirector_SlotHasActiveOrder`, a peer's, documented in
`docs/COMMAND_EXECUTION.md`) refuses to call `Leave` or `disband` an active order, which is the
other half of the same fact: both run at issue time and never execute from a slot.

That `artillery` and `attackmove` are the same thing to the engine is confirmed independently at
`009F3D57`, where the bot sub-controller map gives them one slot.

### The stance, target, weapon and stage effects

None of the seventeen sets a stance or a fire target directly. `00835860`
(`BSP_WeaponDirector_SetFireTarget`) has eight callers and the only one on this packet's paths is
`00816E30` itself, which sets it from the command's own target record for every class alike; the
per-class arms above do not call it. The director's `+220h` artillery permission and `+222h` torpedo
permission (`docs/WEAPON_DIRECTOR.md`, setters `0071DFD0` and `0071E0D0`) are driven by the
sub-kind message path, not by these classes: no arm read in this packet writes either byte. The
commanded-speed engine `009E1170` (a peer's) is likewise not entered from any of the seventeen arms.

**The weapon selection is the ordnance kind**, and it is a query rather than a command: an attack
class is chosen *because* the unit already carries the matching ordnance, through the table above.
The gun classes select nothing; they gate on the controller's `+C24h` and on `0047B850`.

## The per-class bot tasks

`0099A170` is the factory. It reads the current class off the director, rewrites `land` without a
target into `returntobase`, resolves `attackmove` through `007EEC50(target, 1, 1)` and
`returntobase` through `007F16D0`, and then builds one task.

| class | precondition at the arm | factory |
| --- | --- | --- |
| none / null | - | `009C3C40` |
| `moveto` | - | `009C3BE0` |
| `moveonpath` | - | `009BDBB0` |
| `divebomb` | target, `IsKindOf(2)` | `009C8C70` |
| `levelbomb` | target, `IsKindOf(2)` | `009B9030` |
| `dropkamikaze` | target, `IsKindOf(2)` | `009AEBE0` |
| `torpedo` | target, `009229F0` | `009D4E30` |
| `strafe` | target | `009CD300` |
| `rocket` | target, `IsKindOf(2)`, not `IsKindOf(18h)` | `007B7FD0` |
| `kamikaze` | target, `IsKindOf(2)` | `009AF720` |
| `dogfight` | target, `IsKindOf(2)` | `009AB570` |
| `land` | `006BCD20` and `006C4790` over the director | `009B41C0` |
| `closetoship` | target, `009229F0` | `009A2F40` |
| `depthcharge` | target, `IsKindOf(8)` | `009A6970` |
| `retreat` | - | `009CA2B0` |
| `stop` | - | `009BADB0` |

`0099A020` installs whatever the factory returned.

### The completion rule, ten times over

Each task carries a six-byte constant getter and a still-valid predicate. Neither is a Ghidra
function; both were read from the raw listing.

| class | getter | predicate | latched target |
| --- | --- | --- | --- |
| `torpedo` | `009D3280`-`009D3285` | `009D3EF0`-`009D3F4D` | `+4C4h` |
| `divebomb` | `009C79A0`-`009C79A5` | `009C8060`-`009C80BD` | `+440h` |
| `levelbomb` | `009B7BB0`-`009B7BB5` | `009B81F0`-`009B824D` | `+440h` |
| `dropkamikaze` | `009ADDC0`-`009ADDC5` | `009AE140`-`009AE19D` | `+440h` |
| `depthcharge` | `009A54D0`-`009A54D5` | `009A5DB0`-`009A5E0D` | `+48Ch` |
| `kamikaze` | `009AF060`-`009AF065` | `009AF560`-`009AF5BD` | `+478h` |
| `dogfight` | `009A99E0`-`009A99E5` | `009A9CC0`-`009A9D18` | `+4C4h`, `+4ACh` |
| `rocket` | `007B6CD0`-`007B6CD5` | `007B7200`-`007B725D` | `+468h` |
| `strafe` | `009CC3D0`-`009CC3D5` | `009CC850`-`009CC8BD` | `+44Ch`, `+468h` |
| `closetoship` | `009A28B0`-`009A28B5` | `009A2920`-`009A2995` | `+438h` |

The nine director-driven predicates are byte-identical apart from the class constant and the latched
offset. Reading `009A5DB0` gives the shape:

```
this->+404h              -> the unit; absent means invalid
unit->vtable[114h]()     -> the director; absent means invalid
0071BE40(director)       -> the current class
  must equal my class, or attackmove             009A5DD5, 009A5DE3
00521EA0(0071EB60(director)) -> the resolved target
  when this->+<latched> is set, must equal it    009A5DF8..009A5E04
```

`closetoship`'s `009A2920` differs twice: it reads the class off the entity (`vtable[174h]`,
`vtable[178h]`) rather than the director, and it does not accept `attackmove`, which is consistent
with `007EEC50` never producing `closetoship`. It returns three values (`0`, `1` at `009A297B`, `2`
at `009A298C`) where the others return a bool; the three-value convention is the step result, and
`009A5D80` reads it that way (`1` for the failing branch, `2` or `0` otherwise, keyed on `+46Eh`).
That the third value means "succeeded" is provisional: this packet did not read the consumer.

### Revalidation each tick

`009F8160` re-tests the running command. Its general rule, at `009F8231`, is the class's category:
when `klass->vtable[0Ch]()` is 1 or 2 the command needs a live target and must still pass
`007EE8F0`, and otherwise it goes to `LAB_009F8412`, the abandon path. Two special arms sit before
it: the seven ordnance classes take a faster path to the same test at `009F823C`, and `land`
(`009F81F4`) is abandoned by setting bot `+3Ch` when the target is dead or on the unit's own side.
The `depthcharge` branch at `009F81C6` is not a completion rule: when depthcharge still applies and
the target answers `IsKindOf(2)`, it sets the bot byte `+3Dh`.

`009F8160` also breaks off a `dogfight` when the target's own current command is `retreat`
(`009F831B` and `009F83A0`).

## `returntobase` and `land`

`007F16D0` resolves `returntobase` into something concrete, and is the only routine that produces
`land` outside the Lua bindings and the controller.

* An assigned base at `+35Ch` whose `+198h` is not the sentinel `00D7A218`, plus a group that passes
  `006BCD20`, `006C4790` and `006BED30`: `007F1000(land, unit+404h)`, a `land` command at the unit's
  own base.
* Otherwise, a carrier from the second lookup: `007EF8B0(land, 00465080(carrier+7Ch, 0))`.
* Otherwise the routine **writes a `retreat` record** into its output (`*out = 00E08F90`, with the
  side taken from `unit+54h` and a position built from a float block). A plane with nowhere to land
  retreats.

## `Leave`, `disband`, `tutorial`

`Leave` and `disband` are covered under "What the controller does": both are one-shots that never
occupy an executing slot. Their other sites are menu and lobby paths (`0053AC91`, `00535B79`,
`005F9C72`, `005F9CA3`).

`tutorial` `00E08FC0` has exactly four sites in the whole image, and all four are its constructor
`00CCEB40` and destructor `00CDCCE0`. Nothing reads it. It is a registered class with no producer
and no consumer.

## The Lua producers

`docs/UNIT_COMMAND_PRODUCERS.md`'s PilotBot is a peer's and this packet stops at the call site, but
the mission-script bindings are separate routines and are read here. All four reach
`0077D600 BSP_Entity_IssueCommand`.

| routine | literal | class chosen |
| --- | --- | --- |
| `008A4F00` | `luaMW_PilotBomb failed:` | `007ED880` true gives `dropkamikaze`; else `IsKindOf(10h)` gives `levelbomb`, false gives `divebomb` |
| `008A50D0` | `luaMW_PilotGunFire failed:` | target `IsKindOf(18h)` or `IsKindOf(0Fh)` gives `dogfight`; else unit `+3D0h` `IsKindOf(17h)` gives `kamikaze`, false gives `strafe` |
| `008A5310` | - | `torpedo`, unconditional |
| `008A4960` | `luaMW_PilotCloseToShip failed:` | `closetoship`, unconditional |

`008A4B10` is the matching stop: it reads the controller through entity `vtable[114h]`, and if slot
0's command pointer at `controller+54h` is `closetoship` it calls `007ED430(2)`. That is an
independent confirmation of `docs/COMMAND_EXECUTION.md`'s slot-0-at-`+54h` layout.

The three bomb classes the Lua binding picks are the same three `007EE8F0` discriminates with the
same `IsKindOf(10h)` test, so script-issued and AI-issued bombing agree by construction.

## The suppression predicate `00811F50`

`undefined4 __thiscall(unit)(char check_player)`, callers `00826D70`, `009F5B70`, `009F7480`.

It answers whether the unit's current command blocks the caller's action. With `check_player` set it
first refuses for the local player role and for `unit+184h`. Then, on `unit->vtable[174h]()`:

| class | result |
| --- | --- |
| `stop` `00E08F88` | 0, falls straight through |
| `retreat` `00E08F90`, `cruise` `00E08F70` | 1 |
| `attackmove` `00E08F78`, `moveto` `00E08F68` | 1 when the command's target record is populated and `0071C4F0` accepts it |
| `moveonpath` `00E08F80` | 1 when the path cursor test passes |
| anything else | 0 |

The six classes it names are all movement classes. None of the seventeen appears, which is why
`docs/COMMAND_CLASSES.md`'s reading of it as a name table did not survive contact with the body.

## The effect matrix

| class | cat | ordnance kind | needs surface target | self kind | target kind | HUD icon | AI weight |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `artillery` | 1 | - | - | - | - | 3 | - |
| `torpedo` | 2 | `2Bh` | only a surface-running one | - | - | 3 | `+34h` |
| `divebomb` | 2 | `2Ah` | yes | not `10h` | not `0Eh` | 3 | `+14h` |
| `levelbomb` | 2 | `31h` or `2Ah` | yes | `10h` | `1Ch` or not `0Eh` | 3 | `+24h` |
| `dropkamikaze` | 2 | `2Fh` | yes | - | - | 3 | `+64h` |
| `depthcharge` | 2 | `2Ch` | no | - | `8` | 3 | `+04h` |
| `strafe` | 1 | guns | or `IsKindOf(41h)` | - | - | 3 | `+54h` |
| `rocket` | 2 | `33h` | yes | - | not `0Eh` | 3 | `+74h` |
| `kamikaze` | 2 | - | yes | `17h` | `6` gates | 3 | `+64h` |
| `dogfight` | 1 | guns | no, airborne | not `16h` | `0Fh`/`18h` | 3 | `+54h` |
| `moveonpath` | 3 | - | - | - | - | 1 | - |
| `retreat` | 3 | - | - | - | - | 0 | - |
| `returntobase` | 3 | - | - | - | - | - | - |
| `land` | 3 | - | - | `0Ch` to survive issue | - | 4 | - |
| `closetoship` | 3 | - | - | - | - | - | - |
| `Leave` | 0 | - | - | - | `2` optional | - | - |
| `disband` | 0 | - | - | - | - | - | - |
| `tutorial` | 3 | - | - | - | - | - | - |

HUD icon is `00534870`'s constant: 0 hold (`stop`, `retreat`), 1 move (`follow`, `moveto`,
`moveonpath`), 2 cruise, 3 attack (all ten attack classes and `attackmove`), 4 land. The five
classes with no row there are never given one by that chain. AI weight is `009F9770`'s offset into
the 10h-stride tuning block `004D6CE0` returns; `dogfight` and `strafe` share `+54h` and `kamikaze`
and `dropkamikaze` share `+64h`.

## The bot sub-controller map `009F3D00`

`stop` `+BD8h`, `cruise` `+BC8h`, `follow` `+BE4h`, `land` `+C38h`, `moveto` `+C5Ch`, `moveonpath`
`+C64h`, and `artillery` with `attackmove` `+C70h`, or `+217Ch`/`+2254h` when bot `+B0Ch` is
non-zero, chosen by `00779AA0`. No other class has a slot.

## Coverage

Reconstructed with a controller-site table and a completion rule: `artillery`, `torpedo`,
`divebomb`, `levelbomb`, `dropkamikaze`, `depthcharge`, `strafe`, `rocket`, `kamikaze`, `dogfight`,
`closetoship`, `returntobase`, `land`, `Leave`, `disband`, `tutorial`. Sixteen of the seventeen.

`moveonpath` stays `coverage: partial`: only its two sites in this packet's routines (`0099A1DC`,
`009F3D00`'s `+C64h`) are recorded. The path machinery at `0071BDE0`, `0071DC80`, `0071E4C0`,
`0071FDE0`, `007207C0` and the `50h` path objects at controller `+1A4h` remain `contract: unread`,
as does `retreat`'s body beyond its factory and `007F16D0`'s record.

`contract: unread` in this packet: the thirteen bot task bodies themselves (`009C8C70`, `009B9030`,
`009AEBE0`, `009D4E30`, `009CD300`, `007B7FD0`, `009AF720`, `009AB570`, `009B41C0`, `009A2F40`,
`009A6970`, `009CA2B0`, `009BADB0`) - only their addresses, preconditions and validity predicates
are established, not what they fly. `0047B850`, `00604A50`, `00827F70`, `00828EC0` and `009229F0`
are cited by their call sites and results, not read. `00A08460`'s attack-class immediates belong to
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` (a peer's) and were not re-read.

## Open questions

* What kinds `0Eh`, `1Ch`, `41h`, `45h`, `46h` and `35h` are. They are only ever tested, never
  produced, on any path this packet read.
* Whether `007B9320`'s sixth exclusion, kind `2Dh`, has a command class. No singleton arm asks for
  it, so either it is ordnance with no order or the chain is defensive.
* The three-value return of the task validity predicates. `009A5D80` reads `1` as the failing
  branch and keys `2` against `+46Eh`, which reads as running/failed/succeeded, but the consumer was
  not read.
* `tutorial`'s purpose. A registered class with a constructor, a destructor and no reader.
