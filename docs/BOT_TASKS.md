# The thirteen bot task bodies (packet `cc2_bot_tasks`)

Addresses: `009C8C70` `009B9030` `009AEBE0` `009D4E30` `009CD300` `007B7FD0` `009AF720` `009AB570`
`009B41C0` `009A2F40` `009A6970` `009BADB0` `009CA2B0` (the thirteen factories);
`009C7710` `009B7990` `009ADBF0` `009D3050` `009CC230` `007B6A40` `009AEEF0` `009A9810` `009B3240`
`009A2730` `009A5240` `009C9D00` (the twelve class constructors); `0099C6F0` (the base class);
`009F9CE0` (the approach-controller base); `007B8AD0`, `0099B740`, `0099B660`, `0099C230`,
`00411E70`, `009F9980`, `009A5420`, `009A5D80`, `009A65F0`, `009A6500`, `009A2D60`, `009C8920`,
`009D4A70`, `009B8C90`, `009CD020`, `009AAF30`, `009B3C60`, `009AF480`, `007B7AF0`, `009C8A90`,
`009B8D80`, `009AE1B0`, `009A3090`, `009A2500`, `009A1D60`, `009A3390`, `009A4DC0`, `009C73A0`,
`009D2DA0`, `009CC020`, `007B6830`, `009AED20`, `009AB740`, `009B42D0`, `009A6A60`, `009C9BB0`.

`docs/ATTACK_COMMANDS.md` established the factory dispatch `0099A170`, the preconditions at each
arm, the per-class constant getter and the still-valid predicate. This packet reads the thirteen
bodies those arms build.

## The headline correction: there are no ship bot tasks

The packet brief expected ship tasks (artillery, torpedo, depth charge, close-to-ship) and plane
tasks. **All thirteen are aircraft tasks.** Three independent lines of evidence:

* Every tuning constant any of the thirteen reads is under the `Pilot/` prefix of the game-tuning
  singleton, including `Pilot/Torpedo/*`, `Pilot/DepthCharge/*` and `Pilot/CloseToShip/*`
  (`docs/GAME_TUNING_SINGLETON.md` rows `+424`..`+668`). There is no `Ship/` or `Vessel/` reader
  among them.
* Every one of the thirteen constructors builds two initial states named `moveto (<Class>)` and
  `follow (<Class>)`, and the per-tick update writes **altitude limits** into the control block at
  `unit+9D4h` (`+394h`, `+398h`, `+39Ch`). Depth-charge, torpedo and close-to-ship all take the
  same altitude path.
* The only routine in the image that calls the shared tail helper `0099B740` outside the task
  classes is `009998A0 BSP_PilotBot_Update` (a peer's; `docs/UNIT_COMMAND_PRODUCERS.md`).

`depthcharge`'s gate is `IsKindOf(8)` = submarine on the **target**, not on the unit
(`docs/ATTACK_COMMANDS.md`); the task is a plane hunting a submarine. `closetoship` is a carrier
plane closing on a ship. Artillery and attackmove never reach a task: `0099A170` resolves
`attackmove` through `007EEC50` into one of the concrete ordnance classes first, and no arm of
`0099A170` builds an artillery task at all.

## The second correction: no task issues a unit command

The brief expected orders through `0077D600 BSP_Entity_IssueCommand`, the controller's
`008358D0 BSP_WeaponDirector_SetCommand` and the fire target `00835860
BSP_WeaponDirector_SetFireTarget`. **None of the thirteen classes reaches any of the three.**

* `0077D600` has 47 callers (full list read); all are scene-load, entity-order, menu, lobby or Lua
  binding routines. No address in the task class bodies appears.
* `008358D0` has zero callers in the call graph.
* `00835860` has eight callers, all in segments 5, 41, 50, 52, 56 and `009F5DA0`; none is a task.

What the tasks write instead is the pilot control block at `unit+9D4h`: a desired cruising altitude
at `+394h`, a second altitude at `+398h`, a third value at `+39Ch`, each behind a "not overridden"
byte, with a dirty flag at `+3ADh`. The motion controller that consumes those is a peer's contract
and was not read. Weapon release happens inside the per-class `*/release`, `*/aim` and
`*/attackrun` state objects, which this packet did not open; `contract: unread`.

## The task object

`0099A170` calls one factory per class with `ECX = bot`, `EDX = target` (`0099A30B`, `0099A438`,
`0099A45E` set `EDX,ESI` then `ECX,EBX` before the call; `retreat` and `stop` at `0099A471` and
`0099A482` set only `ECX`). Each factory is a `__fastcall` allocation wrapper:

```
operator_new(<size>)               ; 00BF681B
if (result == 0) return 0          ; no construction on failure
ctor(this = result, bot, target)   ; __thiscall, RET 8
return result
```

`0099A020` then appends the pointer to the bot's task vector at `bot+58h` (data), `bot+5Ch`
(count), `bot+60h` (capacity).

### Layout, from the constructors

Byte offsets in the whole object. `stop` stops at `3F8h`; every other class continues.

| offset | size | written at | meaning | evidence |
| --- | --- | --- | --- | --- |
| `+0h` | 4 | `0099C743`, then the derived ctor | primary vtable, 26 slots | base `00D05708`, per-class below |
| `+4h` | - | `0099C729` | sub-object, ctor `0099BE30(this+4, bot->+4 ? bot->+4 - 310h : 0)` | `0099C70D`..`0099C729` |
| `+300h` | 4 | `0099C74B` | the task kind id, the constructor's third argument | `MOV [ESI+300h],EAX` |
| `+304h` | 4 | `0099C75D` | `1.0f` (`00D7A24C`, bytes `00 00 80 3F`) | `MOVSS` |
| `+308h` | 4 | `0099C76C` | `-(00BD2F10(0.0f, 1.0f))`, `ECX=1` | `FCHS; FSTP` |
| `+30Ch` | 4 | `0099C772` | `00D05704` | immediate store |
| `+310h` | 4 | `0099C77C`, then the derived ctor | **the current state object pointer**; `0` in the base | see "the state machine" |
| `+314h` | - | `0099C790` | sub-object, ctor `009FAAD0(this+314h, this)` | |
| `+340h` | 4 | `0099C87A` | `00CE7804` | only when the descriptor row is non-null |
| `+344h` | 4 | `0099C888` | `desc->+230h + [00D1F3A0]` (double add) | `FADD double ptr` |
| `+348h` | 4 | `0099C871` | `desc->+230h` | `FSTP [EBP+34h]`, `EBP = this+314h` |
| `+34Ch` | 4 | `0099C864` | `t = desc->+220h * [00D7A2B0]`; `[00CE3E20] > t ? [00D05B50] : t` | `FCOMIP`/`JBE` at `0099C848` |
| `+38Ch` | - | `0099C79C` | sub-object, ctor `009FCEF0(this+38Ch, this)` | |
| `+3E8h` | 4 | `0099C7BF` | `00BD2F10([00D7A260], 0.0f)`, `ECX=0` | |
| `+3ECh` | 4 | `0099C7D3` | `-1.0f` (`00D7A260`) | |
| `+3F0h` | 1 | `0099C7DB` | byte `1` | |
| `+3F4h` | 4 | `0099C7CD` | **the owning bot** (the constructor's second argument) | `MOV [ESI+3F4h],EDI` |
| `+3F8h` | 4 | `009F9CE0`, then the derived ctor | secondary vtable, the approach controller | |
| `+3FCh` | 4 | `009F9CE0` | **the unit** (`bot+50h`) | `param_1[1] = param_2` |
| `+400h` | 4 | `009F9CE0` | `unit->+538h`, the unit class block (`docs/UNIT_INSTANCE_LAYOUT.md` `+538h`) | |
| `+404h` | 4 | `009F9CE0` | `unit->+9D4h`, **the pilot control block** | every `+54h` override dereferences it |
| `+408h` | 4 | `009F9CE0` | `unit->+DF4h` | |
| `+40Ch` | 4 | `009F9CE0` | `[00F8A30C] + unit->+DF4h->+34h * 248h + 0Ch`, a class descriptor row | same table and stride as `0099C7EB` |
| `+410h`..`+418h` | 12 | `009F9CE0` | zero | |
| `+41Ch` | 4 | `009F9CE0` | **the speed ratio** `max(1.0f, classBlock->+188h / <ReferenceSpeed>)` | `if (1.0 < r) f = r; else f = 1.0f` |
| `+420h` | 4 | `009F9CE0` | `-1.0f` (`00D7A260`) | |
| `+424h`.. | - | per class | class-specific approach fields, including the latched target | table below |

The descriptor-row index at `+40Ch` comes from the unit; the one the base constructor reads at
`0099C7E2` comes from `bot->+34h`. They are separate lookups into the same `248h`-stride table.

### The constructor shape, twelve times

Every derived constructor is the same nine steps. Depth charge (`009A5240`, body
`009A5240`-`009A52E4`, `RET 8`) is the reference:

| step | site | code |
| --- | --- | --- |
| 1 | `009A5266` | `0099C6F0(this, bot, kind)`, the base class |
| 2 | `009A5281` | `<approach ctor>(this+3F8h, bot, target)` |
| 3 | `009A5291` | `*this = <primary vtable>` |
| 4 | `009A5297` | `*(this+3F8h) = <secondary vtable>` |
| 5 | `009A529D` | `*(this+<tertiary offset>) = <tertiary vtable>`, replacing the one the approach ctor wrote |
| 6 | `009A52A7` | `AL = 007B8AD0(*(this+3FCh))` |
| 7 | `009A52AE`..`009A52BC` | `this+310h = AL ? this+<movetoState> : this+<followState>` |
| 8 | `009A52C2` | `state->vtable[4]()`, enter the initial state |
| 9 | `009A52CC` | `009F9980(this+3F8h, this)` |

`007B8AD0` is `bool __fastcall(void* unit) { return unit->+9D8h == 0; }` (body `007B8AD0`, three
instructions). So a unit with nothing at `+9D8h` starts in `moveto`; otherwise it starts in
`follow`. `007B8AD0` is called again at the head of every per-tick update.

`kamikaze` (`009AEEF0`) and `land` (`009B3240`) carry one extra `LEA` before step 6 (`009AEF69`
`ESI+654h`, `009B32BA` `ESI+620h`); `dogfight` (`009A9810`) runs step 9 before step 6. Those three
are `coverage: partial` for the extra store, which was not traced.

### The thirteen classes

| class | kind | factory | size | constructor | approach ctor | primary vt | secondary vt | tertiary vt | `moveto` | `follow` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `closetoship` | `0` | `009A2F40` | `554h` | `009A2730` | `009A2500` | `00D1F4E8` | `00D1F4DC` | `+448h` `00D1F4D8` | `+45Ch` | `+498h` |
| `depthcharge` | `1` | `009A6970` | `7BCh` | `009A5240` | `009A4DC0` | `00D1F738` | `00D1F734` | `+4F4h` `00D1F730` | `+508h` | `+544h` |
| `dogfight` | `2` | `009AB570` | `758h` | `009A9810` | `009A94E0` | `00D1F9B0` | `00D1F9AC` | `+4FCh` `00D1F9A8` | `+510h` | `+54Ch` |
| `land` | `3` | `009B41C0` | `670h` | `009B3240` | `009B2E50` | `00D1FFA0` | `00D1FF94` | `+4B0h` `00D1FF90` | `+4C4h` | `+500h` |
| `levelbomb` | `4` | `009B9030` | `6F8h` | `009B7990` | `009B75E0` | `00D20210` | `00D2020C` | `+4D4h` `00D20208` | `+4E8h` | `+534h` |
| `dropkamikaze` | `5` | `009AEBE0` | `74Ch` | `009ADBF0` | `009AD720` | `00D1FC70` | `00D1FC68` | `+4BCh` `00D1FC64` | `+4D0h` | `+50Ch` |
| `divebomb` | `8` | `009C8C70` | `7E0h` | `009C7710` | `009C73A0` | `00D20E18` | `00D20E10` | `+4DCh` `00D20E0C` | `+4F0h` | `+52Ch` |
| `retreat` | `9` | `009CA2B0` | `564h` | `009C9D00` | `009C9BB0` | `00D20F60` | `00D20F58` | `+464h` `00D20F54` | `+478h` | `+4C4h` |
| `strafe` | `Ah` | `009CD300` | `6FCh` | `009CC230` | `009CC020` | `00D210E0` | `00D210D8` | `+4CCh` `00D210D4` | `+4E0h` | `+51Ch` |
| `rocket` | `Bh` | `007B7FD0` | `708h` | `007B6A40` | `007B6830` | `00D05918` | `00D05910` | `+4D0h` `00D0590C` | `+4E4h` | `+520h` |
| `kamikaze` | `Ch` | `009AF720` | `6A0h` | `009AEEF0` | `009AED20` | `00D1FD40` | `00D1FD3C` | `+4D4h` `00D1FD38` | `+4E8h` | `+524h` |
| `torpedo` | `Eh` | `009D4E30` | `7DCh` | `009D3050` | `009D2DA0` | `00D213C8` | `00D213C0` | `+530h` `00D213BC` | `+544h` | `+580h` |
| `stop` | `Fh` | `009BADB0` | `3F8h` | (none) | (none) | `00D20500` | - | - | - | - |

`stop` is the base class instantiated directly: `009BADB0` allocates `3F8h`, calls
`0099C6F0(this, bot, 0Fh)` at `009BADEC` and stores `00D20500` at `009BADF1`. It has no approach
controller, no state objects, and `+310h` stays `0`.

`0099C6F0` has 22 callers. The twelve above plus `009BACB0`, `009BAD30`, `009BAFC0`, `009BB640`,
`009BB910`, `009BBAD0`, `009BCE50`, `009C3000`, `009CF8E0` and `009BADB0` itself. The nine extra are
the non-attack classes (`moveto` reaches `009C3000` through `009C3BE0`); their kind ids fill `6`,
`7`, `Dh` and values above `Fh` and were not read. `contract: unread`.

`dogfight`'s factory `009AB570` is the only one that runs code before `operator_new`: `009AB490`
at `009AB5A3` and `009AB500` at `009AB5CC`. Neither was read. `coverage: partial`.

## The interface

26 slots, `+0h`..`+64h`. The base table is `00D05708`..`00D0576F`; the string `"not defined"` at
`00D05770` bounds it. Read as raw data with `ghidra bytes`, no Ghidra function at most targets.

| slot | base | `stop` | `depthcharge` | established meaning |
| --- | --- | --- | --- | --- |
| `+0h` | `007B41A0` | `009BBE00` | `009A6950` | scalar deleting destructor (Ghidra's own label on `00D1F738`) |
| `+4h` | `0099B6E0` | `0099B6E0` | `009A5AF0` | `contract: unread` |
| `+8h` | `007B40C0` | `007B40C0` | `007B40C0` | never overridden in the three tables read |
| `+Ch` | `007B4170` | `007B4170` | `009A5F10` | `contract: unread`; `009A5F10` has no Ghidra function |
| `+10h` | `007B40D0` | `009BAC50` | `009A54F0` | `contract: unread` |
| `+14h` | `007B40E0` | `009BAC60` | `009A5500` | `contract: unread` |
| `+18h` | `009A1A90` | `009A1A90` | `009A68C0` | `contract: unread` |
| `+1Ch` | `0099C230` | `0099C230` | `009A65F0` | **the break-off test**, below |
| `+20h` | `007B40F0` | `007B40F0` | `009A5780` | `contract: unread` |
| `+24h` | `0099B6A0` | `0099B6A0` | `009A5D00` | `contract: unread` |
| `+28h` | `007B4100` | `007B4100` | `007B4100` | never overridden |
| `+2Ch` | `007B4110` | `007B4110` | `009A54C0` | `contract: unread` |
| `+30h` | `0099B6F0` | `009BAC40` | `0099B6F0` | `contract: unread` |
| `+34h` | `0099B700` | `0099B700` | `0099B700` | never overridden |
| `+38h` | `0099B710` | `0099B710` | `0099B710` | the abandon test `0099B740` calls; see below |
| `+3Ch` | `0099B730` | `0099B730` | `009A54D0` | **the command-class constant getter** (`docs/ATTACK_COMMANDS.md`) |
| `+40h` | `0099C2C0` | `009B9590` | `009A5DB0` | **the still-valid predicate** (`docs/ATTACK_COMMANDS.md`) |
| `+44h` | `007B4120` | `007B4120` | `007B4120` | never overridden |
| `+48h` | `007B4130` | `007B4130` | `009A54B0` | `contract: unread` |
| `+4Ch` | `0099B720` | `0099B720` | `009A5D50` | `contract: unread`; no Ghidra function at `009A5D50` |
| `+50h` | `007B4140` | `007B4140` | `009A5D80` | **the three-value step result**, below |
| `+54h` | `0099B660` | `0099B660` | `009A6500` | **the per-tick cruise-profile update**, below |
| `+58h` | `007B4180` | `007B4180` | `009A5710` | `contract: unread` |
| `+5Ch` | `007B4150` | `009BAC20` | `009A54E0` | `contract: unread` |
| `+60h` | `0099D060` | `009BAC30` | `009A5ED0` | `contract: unread` |
| `+64h` | `007B4160` | `009B95D0` | `009A66C0` | `contract: unread` |

`0099B660`, the base `+54h`, is an empty body (Ghidra labels it `TRIV_body_0099b660`); every
override calls it first, so the base does nothing per tick.

### Slot `+54h`, the per-tick cruise profile

Ten of the thirteen override it: `007B7AF0` (rocket), `009A2D60` (closetoship), `009A6500`
(depthcharge), `009AAF30` (dogfight), `009AF480` (kamikaze), `009B3C60` (land), `009B8C90`
(levelbomb), `009C8920` (divebomb), `009CD020` (strafe), `009D4A70` (torpedo). `dropkamikaze`,
`retreat` and `stop` inherit the empty base. All ten share one body shape:

| step | rule |
| --- | --- |
| 1 | call the base `0099B660` (empty) |
| 2 | `if (007B8AD0(this->+3FCh))` — only when the unit has nothing at `+9D8h`, i.e. is not following |
| 3 | `ctl = this->+404h` |
| 4 | `if (ctl->+38Dh == 0 && ctl->+380h < 0.0f && ctl->+3A9h == 0) { ctl->+394h = <cruising alt>; ctl->+3ADh = 1; }` then `ctl->+3A9h = 0` |
| 5 | `if (ctl->+38Ch == 0 && ctl->+37Ch < 0.0f && ctl->+3AAh == 0) { ctl->+398h = <second alt>; ctl->+3ADh = 1; }` then `ctl->+3AAh = 0` |
| 6 | divebomb and torpedo only: `if (ctl->+38Eh == 0 && ctl->+384h < 0.0f && ctl->+3ABh == 0) { ctl->+39Ch = [00CE4C04]; ctl->+3ADh = 1; }` then `ctl->+3ABh = 0` |
| 7 | the attack-distance clamp, where the class has one |
| 8 | a class tail call |

The `< 0.0f` test is `FCOM`-style in the decompiler output (`x <= 0.0 && x != 0.0`); the byte at
`+38Ch`/`+38Dh`/`+38Eh` is an "already overridden" flag and `+3A9h`/`+3AAh`/`+3ABh` a one-shot that
the update always clears. `+3ADh` is the dirty flag the motion controller reads.

The constants, with their tuning names and defaults from `docs/GAME_TUNING_SINGLETON.md`:

| class | `+54h` body | `+394h` cruising alt | `+398h` second alt | attack-distance clamp | tail |
| --- | --- | --- | --- | --- | --- |
| `closetoship` | `009A2D60` | `+424h` `Pilot/CloseToShip/CruisingAlt` = 1200 | `+428h` `Pilot/CloseToShip/DropAlt` = 1000 | none | none |
| `depthcharge` | `009A6500` | `+4A4h` `Pilot/DepthCharge/CruisingAlt` = 700 | `+498h` `Pilot/DepthCharge/AimAltRange/2` = 60 | `this->+43Ch = max(this->+43Ch, +4ACh AttackDist(1800) * this->+41Ch)` | `009A5F50(0)`, `0099B740` |
| `divebomb` | `009C8920` | `+4C0h` `Pilot/DiveBomb/CruisingAlt` = 1300 | `00BD2F10(0, [00CE5380]) + ` `+4CCh` `BeginAltRange/1` = 1000 | `+4C4h` `AttackDist` = 1100 | `0099B740` |
| `torpedo` | `009D4A70` | `+430h` `Pilot/Torpedo/CruisingAlt` = 500 | `*(float*)this->+40Ch`, replaced by `[00CE3850]` when below `[00D7A370]` | `+434h` `AttackDist` = 2200 | `0099B740` |
| `levelbomb` | `009B8C90` | `+444h` `Pilot/LevelBomb/CruisingAlt` = 1300 | `+448h` `Pilot/LevelBomb/DropAlt` = 1300 | `+44Ch` `AttackDist` = 2000 | `0099B740` |
| `strafe` | `009CD020` | `+654h` `Pilot/Strafe/CruisingAlt` = 1000 | `contract: unread` | `+658h` `AttackDist` = 2000, read by `009CA4A0` `009CADB0` `009CCED0` | `0099B740` |
| `dogfight` | `009AAF30` | `+640h` `Pilot/Dogfight/CruisingAlt` = 1400 | none in the body read | `+644h` `AttackDist` = 2000, read by `009AAC70` | `009AAC70(0)`, `0099B740` |
| `rocket` | `007B7AF0` | `+660h` `Pilot/Strike/CruisingAlt` = 500 | `contract: unread` | `+664h` `Pilot/Strike/AttackDist` = 1800, read by `007B41E0` `007B4F60` `007B78F0` | `0099B740` |
| `land` | `009B3C60` | `+514h` `Pilot/Landing/CruisingAlt` = 1400 | `contract: unread` | none read | `0099B740` |
| `kamikaze` | `009AF480` | `this->+424h` (a task field, not tuning) | `this->+430h` | `contract: unread` | `0099B740` |

`kamikaze` is the one class that reads its profile out of its own object rather than the singleton.
`Pilot/Kamikaze` has two variants in the tuning table, `RocketLike` (`+45Ch`..`+474h`) and
`FighterLike` (`+478h`..`+490h`), and `009AB920` is the routine the tuning doc records as reading
both `ReferenceSpeed` rows. The routine that picks a variant and copies it into `+424h`/`+430h` was
not read: `contract: unread`.

The reference speeds that feed the ratio at `+41Ch`, from the same table: `Pilot/CloseToShip` KMH(300)
(`+42Ch`, read at `009A1D60` `009A1DC0`), `Pilot/DepthCharge` KMH(270) (`+4B8h`, `009A3390`
`009A35D0`), `Pilot/Torpedo` KMH(300) (`+440h`, `009D0380`), `Pilot/LevelBomb` KMH(300) (`+458h`),
`Pilot/DiveBomb` KMH(280) (`+4D8h`, `009C3EA0`), `Pilot/Dogfight` KMH(300) (`+650h`, `009A6C10`
`009A6D40`), `Pilot/Strafe` KMH(280) (`+65Ch`, `009CA4A0`), `Pilot/Strike` KMH(280) (`+668h`,
`007B41E0`), `Pilot/Landing` KMH(140) (`+52Ch`, `009AFE70` `009AFFF0`).

### Slot `+1Ch`, the break-off test

Four overrides read. `009A65F0` (depthcharge), `009C8A90` (divebomb), `009B8D80` (levelbomb),
`009AE1B0` (dropkamikaze); all four call the base `0099C230` first. Shape:

| step | rule |
| --- | --- |
| 1 | `if (!0099C230(this)) return 0` |
| 2 | `t = this->+<latched target>`; `if (t == 0 || t->+5Dh != 0) return 1` |
| 3 | `if ((this->+404h->+369h == 0 \|\| [00E17BF2] == 0) && <class extra>)` |
| 4 | `d = distance(BSP_EntityPose_GetWorldPositionRefreshed(), (*(void**)(this+3F8h))->vtable[0]())` |
| 5 | `if (<SafeDist> * this->+41Ch <= d) return 1` |
| 6 | `return 0` |

| class | latched target | class extra at step 3 | SafeDist |
| --- | --- | --- | --- |
| `depthcharge` `009A65F0` | `+48Ch` | `!009A5420(this->+310h) \|\| this->+46Ch == 0` | `+4B0h` `Pilot/DepthCharge/SafeDist` = 250 |
| `divebomb` `009C8A90` | `+440h` | `this->+4C9h == 0` | `+4C8h` `Pilot/DiveBomb/SafeDist` = 100 |
| `levelbomb` `009B8D80` | `+440h` | `this->+4C6h == 0` | `+450h` `Pilot/LevelBomb/SafeDist` = 1000 |
| `dropkamikaze` `009AE1B0` | `+440h` | `this->+4B9h == 0` | no distance test in the body read |

The latched offsets agree with `docs/ATTACK_COMMANDS.md`'s predicate table, which derived them from
the `+40h` predicates independently. `t->+5Dh` is the byte that marks a target no longer engageable;
its producer was not read, so that reading is provisional.

### Slot `+50h`, the three-value step result

`009A5D80` (depthcharge, three instructions plus the call):

```
if (009A5420(this, this->+310h))  return (this->+46Eh != 0) ? 2 : 0;
return 1;
```

`009A5420(this, state)` returns 1 when `state` is one of exactly seven pointers:
`this+6C4h`, `this+5DCh`, `this+6A0h`, `this+784h`, `this+67Ch`, `this+6E4h`, `this+7A0h`. Those are
precisely the seven `DepthCharge/*` states of the table below, i.e. every state except
`moveto (DepthCharge)` and `follow (DepthCharge)`.

So slot `+50h` returns **`1` while the task is still approaching** and `0` or `2` once it has
entered its own attack states, with `2` selected by the byte at `+46Eh`. `docs/ATTACK_COMMANDS.md`
read `1` as the failing branch from the other side; the two readings are consistent only if
"approach not finished" is what the consumer treats as failure. The consumer was not read, so the
meaning of `0` against `2` stays provisional.

### Slot `+38h` and the abandon path

`0099B740(task)` is the tail of eight of the ten `+54h` overrides and of `009998A0
BSP_PilotBot_Update`:

```
if (task->+2FCh != 0 && task->+2F4h != 0 && task->+2F4h == *(task->+2FCh + 3D0h)) {
    if (task->vtable[38h]()) 007ED3F0(1);
}
```

`007ED3F0` has two callers, `0099B740` and `009A2810`, and no callees. `docs/ATTACK_COMMANDS.md`
records the sibling `007ED430(2)` as the close-to-ship stop that `008A4B10` issues, so `007ED3F0(1)`
is read as the matching abandon. That naming is a hypothesis; the body was not read.

## The state machines

Each class registers its state objects by name through `00411E70(name, state)`, called from the
approach constructor or a helper it calls. The names are literal strings in the image and carry the
class, so each row is attributed by its own string, not by the call site. Offsets below are
converted to whole-object offsets and were cross-checked against the `moveto`/`follow` pair the
constructor stores at `+310h` in step 7; all eleven agree.

| class | registrar | states (whole-object offsets) |
| --- | --- | --- |
| `closetoship` | `009A2500` | `moveto` `+45Ch`, `follow` `+498h`, `stayclose` `+530h` |
| `depthcharge` | `009A3090` | `moveto` `+508h`, `follow` `+544h`, `done` `+5DCh`, `attackrun` `+67Ch`, `goaway` `+6A0h`, `aim` `+6C4h`, `prepare` `+6E4h`, `leave` `+784h`, `turnto` `+7A0h` |
| `torpedo` | `009D2DA0` | `moveto` `+544h`, `follow` `+580h`, `done` `+618h`, `attackrun` `+6B4h`, `goaway` `+6D8h`, `aim` `+710h`, `prepare` `+740h` |
| `divebomb` | `009C73A0` | `moveto` `+4F0h`, `follow` `+52Ch`, `prepare` `+5C4h`, `done` `+664h`, `goaway` `+704h`, `aimdive` `+734h`, `aimglide` `+754h`, `flyabove` `+778h`, `turndown` `+79Ch`, `attackrun` `+7BCh` |
| `levelbomb` | `009B42D0` | `moveto` `+4E8h`, `follow` `+534h`, `attackrun` `+5CCh`, `aim` `+5F4h`, `prepare` `+610h`, `release` `+6B4h`, `goaway` `+6D8h` |
| `dropkamikaze` | `009AB740` | `moveto` `+4D0h`, `follow` `+50Ch`, `attackrun` `+5A4h`, `prepare` `+5C0h`, `done` `+664h`, `release` `+708h`, `goAway` `+72Ch` |
| `kamikaze` | `009AED20` | `moveto` `+4E8h`, `follow` `+524h`, `prepare` `+5BCh`, `gotowards` `+654h`, `aim` `+678h` |
| `dogfight` | `009A6A60` | `moveto` `+510h`, `follow` `+54Ch`, `prepare` `+5E4h`, `aim` `+67Ch`, `maneuver` `+6A4h`, `attackrun` `+6E4h`, `avoid_roll` `+708h`, `avoid_turn` `+730h` |
| `strafe` | `009CC020` | `moveto` `+4E0h`, `follow` `+51Ch`, `prepare` `+5B4h`, `gotowards` `+64Ch`, `aim` `+670h`, `goaway` `+690h`, `attackrun` `+6D8h` |
| `rocket` | `007B6830` | `moveto` `+4E4h`, `follow` `+520h`, `prepare` `+5B8h`, `gotowards` `+650h`, `aim` `+674h`, `goaway` `+694h`, `attackrun` `+6E4h` |
| `retreat` | `009C9BB0` | `moveto` `+478h`, `enterzone` `+494h`, `leave` `+4ACh`, `follow` `+4C4h` |
| `land` | not found | `moveto` `+4C4h`, `follow` `+500h` from the constructor; the rest `contract: unread` |
| `stop` | - | none; `+310h` stays `0` |

`00411E70` has 23 callers; the twelve above plus `009A1B70`, `009A2610`, `009A6A60`'s siblings
`009BC6B0`, `009BCC60`, `009BCD50`, `009BDD70`, `009C2DF0`, `009C2EF0`, `009CF710`, `009E4F90` and
`009F39C0`, which belong to the non-attack classes and to the shared `moveto`/`follow` state
implementations `009C2AC0` and `009C2980`.

The generic `moveto` state is constructed by `009C2AC0(approach, target, range, range, mode)` and
`follow` by `009C2980(approach, [00CE3D08])`. The ranges each class passes:

| class | `009C2AC0` third and fourth arguments | fifth |
| --- | --- | --- |
| `closetoship` | `approach->+48h` twice, where `009A1D60` set `+48h = approach->+0Ch->+398h` | `approach->+4Ch = approach->+24h * 1500.0` (`[00CF00F8]`) |
| `depthcharge` | `approach->+3Ch + approach->+34h` twice | `approach->+40h` |
| `divebomb` | `approach->+A8h - [00D7A220]`, then `approach->+A8h` | `contract: unread` |
| `dropkamikaze` | `approach->+A8h - <float>`, then `approach->+A8h` | `approach->+ACh` |
| `torpedo` | `approach->+78h + approach->+74h` | `contract: unread` |
| `strafe`, `rocket` | `approach->+40h` twice | `approach->+38h` |
| `kamikaze` | `approach->+4Ch` twice | `approach->+48h` |
| `land` | `approach->+30h` twice | a local |
| `dogfight`, `retreat` | no `009C2AC0` call; `follow` only | - |

`009A1D60` (closetoship, body `009A1D60`-`009A1DBA`, `RET 8`) is short enough to give in full:

```
009A1D64  tuning = 0042E740()
009A1D7A  009F9CE0(this, unit, (float)tuning->+42Ch)      ; Pilot/CloseToShip/ReferenceSpeed
009A1D89  *this = 00D1F470
009A1D8F  009A1CA0(this+2Ch, target)
009A1D94  *(this+2Ch) = 00CF5C94
009A1DA6  this->+44h = 0                                   ; byte
009A1DAB  this->+4Ch = this->+24h * 1500.0                 ; the speed ratio times [00CF00F8]
009A1DB6  this->+48h = *(float*)(this->+0Ch + 398h)        ; unit->+9D4h->+398h
```

`009A3390` (depthcharge, the same position) is the widest of the approach constructors and sets, in
order: `+30h` from `00BD2F10([00CE3860], [00CE6448])` scaled by `007C47F0()`; `+34h` from
`00BD2F10(0, AimAltRange/2 - AimAltRange/1)` = `60 - 20`; `+38h` from
`00BD2F10(ManeuverAltRange/1, ManeuverAltRange/2)` = `(80, 150)`; `+3Ch` = `AimAltRange/1` = 20;
`+40h` = `FlyAboveDist(400) * speedRatio + 00BD2F10([00CE6448], [00CEB4B4]) * classBlock->+268h`;
`+44h` and `+48h` = `+40h * 1.5` (`[00CE3D78]`); `+54h` = `[00CFDEB0]`; `+58h` = `[00CE38C8]`;
`+74h` = byte 1; `+78h` = `0FFh`; and `+F0h`..`+F8h` = the unit's world position after
`BSP_EntityPose_RefreshWorld()` when `unit->+C8h == 0`. So the depth-charge `moveto` range is
`(AimAltRange/1) + (AimAltRange/2 - AimAltRange/1)` randomised, i.e. an aim altitude drawn between
20 and 60 metres.

`00BD2F10` takes two floats on the stack and a selector in `ECX`; `ECX = 0` and `ECX = 1` both
appear in `0099C6F0`. It is the shared random/interpolate helper of this subsystem and was not read:
`contract: unread`. Every range above therefore has a named low and high bound but an unproven
distribution.

## Host table

One row per native call site the reconstruction models. `this`/args/ret from the calling convention
at the site; `gate` is the condition under which the site runs.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `009A6991` | `00BF681B operator_new` | not modelled | `- / size / void*` | always; `0` skips construction |
| `009A5266` | `0099C6F0` | `construct_base` | `this / bot, kind / this` | always |
| `009A5281` | `009A4DC0` | `construct_approach` | `this+3F8h / bot, target / this+3F8h` | always |
| `009A52A7` | `007B8AD0` | `unit_has_follow_target` | `unit / - / bool` | always, in the ctor and in every `+54h` |
| `009A52C7` | `state->vtable[4]` | `enter_state` | `state / - / void` | always |
| `009A52CC` | `009F9980` | `register_approach` | `this+3F8h / this / void` | always |
| `009A6506` | `0099B660` | not modelled (empty base) | `this / - / void` | every `+54h` tick |
| `009A6512`, `009A6535` | `0042E740 BSP_GameTuning_GetSingleton` | `game_tuning` | `- / - / void*` | each constant fetch |
| `009A65B1` | `0099C230` | `base_break_off` | `this / - / bool` | head of `+1Ch` |
| `009A65D1` | `009A5420` | `in_attack_state` | `this / state / bool` | depthcharge `+1Ch` and `+50h` |
| (in `009A65F0`) | `BSP_EntityPose_GetWorldPositionRefreshed` | `unit_world_position` | `unit / - / vec3` | `+1Ch` step 4 |
| (in `009A65F0`) | `(*(void**)(this+3F8h))->vtable[0]` | `approach_reference_point` | `approach / out / vec3` | `+1Ch` step 4 |
| (in `009A65F0`) | `BSP_Vector3_LengthFloatThreshold` | not modelled | `- / vec3 / float` | `+1Ch` step 4 |
| `009A52CC`-tail | `0099B740` | `abandon_if_stale` | `this / - / void` | tail of eight `+54h` bodies |
| (in `0099B740`) | `task->vtable[38h]` | `should_abandon` | `this / - / bool` | when `+2F4h == *(+2FCh + 3D0h)` |
| (in `0099B740`) | `007ED3F0` | `abandon_command` | `? / 1 / void` | when `should_abandon` |
| `009A3090`'s nine | `00411E70` | `register_state_name` | `- / name, state / void` | construction only |
| `009A1D7A` | `009F9CE0` | `construct_speed_reference` | `this / unit, ref_speed / this` | every approach ctor |

`009A65B1`, `009A65D1` and the three inner sites of `009A65F0` are decompiler-visible calls inside
`009A65F0`; the exact site addresses inside that body were not transcribed from the listing, so they
are marked in `reports/bot_tasks.json` only where the listing confirmed them.

## Coverage

`complete`: the factory shape and the thirteen allocation sizes; the base constructor `0099C6F0`;
the approach base `009F9CE0`; the constructor shape and its nine steps for all twelve derived
classes; the thirteen kind ids; the three vtable addresses per class; the initial-state pair per
class; `007B8AD0`; the 26-slot interface table for the base, `stop` and `depthcharge`; slot `+54h`
for all ten overrides; slot `+1Ch` for four; slot `+50h` for depthcharge; `0099B740`; the state-name
tables for eleven classes.

`partial`: `kamikaze` `009AEEF0` and `land` `009B3240` (one extra `LEA` each, `009AEF69` and
`009B32BA`, not traced); `dogfight` `009AB570` (`009AB490` and `009AB500` before the allocation);
the `+54h` second-altitude constant for `strafe`, `rocket` and `land`.

`contract: unread`: every state object's own body, which is where weapon release happens; the
motion controller behind `unit+9D4h`; `009998A0 BSP_PilotBot_Update`; `00BD2F10`; `007C47F0`;
`0099BE30`, `009FAAD0`, `009FCEF0`, `009F9980`, `009A1CA0`; the nine non-attack task classes;
`land`'s state names; `kamikaze`'s variant selection at `009AB920`; nineteen of the 26 interface
slots; the consumer of the `+50h` three-value return.

## Open questions

* Which slot the scheduler calls per tick. `0099B740` is reached from `009998A0
  BSP_PilotBot_Update` and from eight `+54h` bodies, which makes `+54h` the tick, but the loop over
  `bot+58h` that would prove it was not located; `009998A0` is 87 listing lines and contains only
  two indirect calls.
* What `+9D8h` on the unit is. `007B8AD0` tests it once per construction and once per tick, and it
  is the single switch between `moveto` and `follow` for all thirteen classes.
* Whether `0099A020`'s growth path is a defect. The decompiled body frees the old buffer and
  returns before storing the newly allocated one; only the non-growth path stores. The assembly was
  not checked, so this is not asserted.
* The distribution `00BD2F10` implements. Every approach range is stated as a pair of tuning bounds
  passed to it.
* Why `kamikaze` alone caches its altitude profile in the task rather than reading the singleton.

## Correction from docs/PLANE_FLIGHT.md (packet cc2_plane_flight)

- **Was:** the tasks write "the pilot control block at unit+9D4h", with a desired cruising altitude at +394h, a second altitude at +398h, a third value at +39Ch and a dirty flag at +3ADh
  **Is:** unit+9D4h is the plane's squadron. Those four fields are squadron fields: a formation-level cruising altitude and its limits. The pilot control block is unit+9E4h, five float axes and three bytes.
  **Evidence:** scanning .text for every MOV [reg+9D4h] gives ten stores. 007ED0E6 in 007ED0D0 writes plane+9D4h = squadron together with plane+9D8h = spawnIndex and the sorted insert into squadron+3D0h[]; 007F4B49 does the same in 007F4580; 007F3A07 clears it in BSP_Squadron_RemovePlane; 007CFE6C zeroes it in the plane unit constructor. 009FBA50 at 009FBA90 reads approach+0Ch (= unit+9D4h) and then +394h as a shared altitude ceiling. The existing readers agree: [[unit+9D4h]+3D0h] == unit (docs/HUD_CENTRAL_UPDATES.md), [unit+9D4h]+3CCh (docs/MISSION_RESULT_DECISION.md), 007B97E0 MOV EAX,[ECX+9D4h]; RET (docs/GUNNERY_TABLES.md).
