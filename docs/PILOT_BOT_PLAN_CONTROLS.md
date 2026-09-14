# The pilot bot's planner inputs and where its task comes from (packet `cc7_pilot_bot_plan_controls`)

Addresses: `0099D300`, `0099ACD0`, `0099A170`, `0099A4C0`, `0099BE30`, `007C18B0`, `0077D600`,
`008358D0`, `0071E6C0`, `00720CD0`, `007EEC50`, `0071BE40`, `0071EB60`, `00521EA0`,
`00D09EC0` (vtable), `00D09F20` / `00D09FB8` (the two `+60h` slots).

> **Superseded in part by `docs/ENTITY_LUA_ORDER_PATH.md`.** The caller table at
> [Who issues the command](#who-issues-the-command--the-complete-list) is **not complete** — it
> came from `ghidra callers`, which under-reports, and lists 25 of the real **62** call sites. Two
> conclusions below are wrong as a result: the `Pilot*` bindings `008A4C90 PilotSetTarget`,
> `008A4150 PilotMoveTo` and `008A4590 PilotMoveToRange` **do** call `0077D600`, and the AI
> planners (`009FFEB0`, `00A02020`, `00A11FF0`, `00A13B60`, `00A14DD0`, `00A2F6F0`) **do** issue
> commands through it, so the "group-command-to-unit-command hand-off is unestablished" gap this
> document reports does not exist. Everything else here stands.

Worker `agent/cc7-pilot-bot-plan-controls`, 2026-09-14 UTC. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Ghidra was **read-only**: no rename, no comment, no
prototype, no tag, no write lock, no save. Every descriptive name is a hypothesis, not a
recovered symbol. Continues `docs/PLANE_AI_CONTROL.md`.

## Headline

Two results, and the second is the one that matters for gameplay.

1. **The planner's inputs are resolved.** `bot+2F4h` is the plane's **vehicle class descriptor**,
   and the three unknowns the yaw arm depended on now have names: `class+1B0h` is `YawSpd`,
   `unit+C64h` is the **pitch angle** and `unit+C68h` is the **bank angle**, both derived from the
   orientation matrix every plane step by `007C18B0`.
2. **The AI is not missing a steering law so much as it is starved of orders.** The task the bot
   needs is built from a **command** on the unit, and a command is issued by exactly one routine,
   `0077D600 BSP_Entity_IssueCommand`, whose callers are Lua mission-script bindings plus a
   handful of native systems (air operations, the shipyard, and **the scene database's deferred
   reference resolution**). In the reconstructed host none of those runs, so no unit ever receives
   a command, so `0099A170` never builds a task, so `0099ACD0` returns before producing anything.

Against the measurement in the brief — every plane's closest approach pinned at its t=0 distance,
`AAMACHINEGUN` assignments scaling with tick count while shots stay 0 — this predicts the observed
behaviour exactly, and predicts that no amount of further arm recovery will move those numbers
until something issues a command.

**What I did not do**: the four unread axis arms are still substantially unread. I spent the
packet on the inputs and on the task, per the brief's "if you have to choose, this half is worth
more". See [Coverage](#coverage) for exactly what remains.

## (a) The planner's inputs

### `bot+2F4h` is the plane's vehicle class descriptor

`0099BE30` constructs the object that `0099D300` and `0099B450` receive as `this` (the `task+4h`
sub-object). Read from the listing:

```
0099be45: CALL 00BF7CD1                ; array ctor: 3 elements of 0C8h at this+0, ctor 0099BDB0
0099be52: MOV EAX,[ESP+8]              ; the constructor's argument: the plane unit
0099be66: MOV [ESI+2F0h],EAX           ; bot+2F0h = unit
0099be6c: MOV EDX,[EAX+538h]
0099be72: MOV [ESI+2F4h],EDX           ; bot+2F4h = unit+538h
0099be78: MOV EAX,[EAX+9D4h]
0099be84: MOV [ESI+2F8h],EAX           ; bot+2F8h = unit+9D4h
0099bec9: CALL 0099B450                ; seed the plan slots
```

`unit+538h` is **the unit class block** — `docs/BOT_TASKS.md` line 97 names it that
(`docs/UNIT_INSTANCE_LAYOUT.md` `+538h`), and `docs/PLANE_CLASS_FIELDS.md` decodes the plane
family's descriptor in full. So every `[bot+2F4h]+N` in `0099D300` is an **authored per-plane-class
constant from `vehicleclasses.lua`**, not runtime state. The three the arms use:

| offset | name | reader | arm |
| --- | --- | --- | --- |
| `+1B0h` | `YawSpd` | `007D25A2` | the yaw blend's divisor |
| `+1B8h` | `SlideRatio` | `007D26BF` | the pitch arm's terminal branch |
| `+1D8h` | `NegativePitchRatio` | `007D2862` | the pitch arm's terminal branch |

All three are present in all 72 shipped plane rows. `NegativePitchRatio` appearing in a branch
that saturates the pitch command to `-1.0f` is an independent check that the branch is the
nose-down one.

Note the confirmation this gives the earlier packet: `bot+2F0h` is the unit, which
`docs/PILOT_COMMAND_PATH.md` had already stated, and `bot+2F8h` is `unit+9D4h`, the squadron/owner
pointer the bot tick's peer arm reads.

### `unit+C64h` and `unit+C68h` are attitude angles, not inferences any more

`docs/PLANE_AI_CONTROL.md` recorded "that `unit+C68h` is a bank angle is an inference". It is now
proved. A census of every write to both offsets finds four each, of which one is the plane
constructor's init and one is a scene record at a coincident offset; the live writer of both is
`007C18B0`, called from `007CE040 BSP_PlaneTickElement_FixedStep`, so both are refreshed every
plane step. Its callees are `BSP_Matrix_Copy4x4X87`, `BSP_Matrix_Multiply4x4`, `BSP_Vector3f_Cross`,
`BSP_Matrix_BuildRotationY` and `LIBCRT_atan2` — an attitude decomposition.

`unit+C64h`, from the raw orientation matrix at `unit+CCh` (`007C18F3 LEA EDI,[ESI+0CCh]`):

```
007c18f9: FLD [EDI+20h]    ; m20
007c1903: FLD [EDI+28h]    ; m22
007c1912: FMUL ST0         ; squares, summed
007c1932: CALL 00BF7030    ; sqrt(m20^2 + m22^2)
007c1943: FLD [EDI+24h]    ; m21
007c1952: CALL 00BF701A    ; atan2
007c1966: FSTP [ESI+C64h]
```

so `unit+C64h = atan2(m21, sqrt(m20² + m22²))` — the elevation of the matrix's **third row** above
the horizontal plane. That is the **pitch (climb) angle** if row 2 is the forward axis; the
identification is exact, the *label* depends on the forward-axis convention, which
`docs/PLANE_FREE_FLIGHT_PHYSICS.md` is the authority on.

`unit+C68h`, from the same matrix with yaw removed:

```
007c1a6d: CALL 00413920    ; Matrix_Multiply4x4 against a RotationY frame
007c1a77: CALL 004134F0    ; Matrix_Copy4x4X87 -> [ESP+4Ch]
007c1a7c: FLD [ESP+60h]    ; copy+14h = m11
007c1a80: FLD [ESP+5Ch]    ; copy+10h = m10
007c1a84: CALL 00BF701A    ; atan2(m10, m11)
007c1a94: FSTP [ESI+C68h]
```

so `unit+C68h = atan2(m10, m11)` on the de-yawed matrix — the **bank angle**. The decomposition
order (pitch straight off the raw matrix, bank off the matrix with the heading rotation divided
out) is the standard one.

Two further fields fall out, both wanted by anything that reimplements this: `unit+C74h` and
`unit+C78h` are the two angles' **rates**, each computed as
`00438B10 BSP_Math_SubtractWrappedAngle(current, previous) / dt` (`007C1B49` and `007C1B6F`, each
followed by `FDIV [ESP+14Ch]`, the routine's `dt` argument). `SubtractWrappedAngle` is itself
proof that both fields are angles. `unit+C6Ch` is the heading and is read at `007C18C1`;
`coverage: partial` — I did not read its derivation.

### The yaw arm, restated with its inputs resolved

`docs/PLANE_AI_CONTROL.md` gave this arm with three unknowns. With `bot+2F4h`, `unit+C64h` and
`unit+C68h` resolved it reads:

```
turn = 0
if (pitch > 0)                                        ; 0099E88E COMISS, JBE skips
    turn = clamp( pitch / (YawSpd * sin(bank) * planeTuning[+9Ch]), -1, +1 )
t    = min(1, InterpolateClamped(planeTuning[+7Ch], 0, planeTuning[+80h], 3.0f, |bank|))
yaw  = clamp( t * turn + (1 - t) * base, -1, +1 )     ; 0099E98D -> 00415620
slot1.desired = yaw ; slot1.active = 1                ; 0099EA3E / 0099EA46
```

`planeTuning` is `0042E740 BSP_GameTuning_GetSingleton() + 538h`. **`base`, the term at
`[ESP+18h]`, is still unread** (set at `0099E884`), so this arm remains **not implementable**, and
I have not implemented it. I report the algebra rather than a rule deliberately: with `base`
unknown, any C++ for this arm would be a guess wearing an address.

I did not attempt to rationalise why a *yaw* command divides pitch by `sin(bank)`; that is what
the listing computes, and an explanation I cannot evidence would be worth less than the algebra.

### The pitch arm, partial

The terminal branch at `0099E6EE`-`0099E741` saturates: when two `COMISS` gates pass, `XMM0` is
loaded with `-1.0f` (`0099E6FE MOVAPS XMM0,XMM4`, `XMM4 = 00D7A260`) and stored to slot 3
(`0099E739 [ESI+29Ch]`, active byte `0099E741 [ESI+2A0h]`). Alongside it a separate quantity is
formed into `[ESP+10h]` from `sin(bank)² * [ESP+2Ch] * SlideRatio * YawSpd`, plus
`NegativePitchRatio` times something (`0099E703`-`0099E729`). **The guards that reach this branch
and the other two pitch writes (`0099D36F`, `0099D679`) are unread**, so there is no pitch law
here, only a confirmed nose-down saturation branch and the class fields it uses.

## (b) Where the task comes from

`0099ACD0` emits no command without a task (`docs/PLANE_AI_CONTROL.md`). The task chain, upward:

```
0077D600 BSP_Entity_IssueCommand                 <- the ONLY order entry point
  -> ... the unit's command controller
008358D0 BSP_WeaponDirector_SetCommand           vtable +60h  -> 0071E6C0 PushCommandSlot
0099A170 BSP_Bot_InstallCommandTask              reads the command back, builds the task
  -> one of 13 BSP_BotTask_Make* factories
  -> the bot's task vector at bot+58h / bot+5Ch
0099ACD0 BSP_PilotBot_Tick                       consumes the task, runs 0099D300, commits
```

### Who allocates the task

`0099A170 BSP_Bot_InstallCommandTask`, called from `0099ACD0 BSP_PilotBot_Tick` itself and from
`0099A4C0`. It is the **only** caller of all thirteen task factories — `MakeRocket`,
`MakeCloseToShip`, `MakeDepthCharge`, `MakeDogfight`, `MakeDropKamikaze`, `MakeKamikaze`,
`MakeLand`, `MakeLevelBomb`, `MakeStop`, `MakeDiveBomb`, `MakeRetreat`, `MakeStrafe`,
`MakeTorpedo` — confirmed by `ghidra callers` on five of them, each returning `0099A170` alone.
`docs/BOT_TASKS.md` and `docs/ATTACK_COMMANDS.md` carry its body; it is already reconstructed in
`src/attack_commands.cpp`.

### What the task holds

The `task+4h` sub-object `0099BE30` builds: three `0C8h` sub-objects at `+0h` (ctor `0099BDB0`),
scalars from `+258h`, the five `{current, desired, active}` plan slots at `+274h`, the unit at
`+2F0h`, the class at `+2F4h`, the squadron/owner at `+2F8h`.

### What decides its target

`0099A170` does not choose a target itself. Per its ledger evidence and `docs/ATTACK_COMMANDS.md`
it reads the **current command** off the unit's director (`unit vtable[114h]` then
`0071BE40 BSP_WeaponDirector_CurrentCommand`), takes the target from the command
(`0071EB60 BSP_EntityCommand_ActiveTargetDescriptor`, `00521EA0 BSP_CommandTarget_ResolveObject`)
and picks the attack class with `007EEC50 BSP_Unit_ChooseAttackCommand`. **The target is the
command's target.** The bot is a command executor, not a target selector.

### Who issues the command — the complete list

`008358D0 BSP_WeaponDirector_SetCommand` is vtable slot `+60h`. Ghidra reports no callers for it
because it is only ever reached through a vtable, so I scanned the whole image for both absolute
and `E8`/`E9` relative references: **two absolute references, `00D09F20` and `00D09FB8`, and no
direct call anywhere.** `00D09F18` holds `00720CD0 BSP_WeaponDirector_IssueTargetCommand`, which
`docs/COMMAND_EXECUTION.md` fixes at vtable `+58h`, so the table base is `00D09EC0` and
`SetCommand` is `+60h`, matching that document's own heading.

Above that, `0077D600 BSP_Entity_IssueCommand` is the order entry point. Its complete caller set:

| caller | kind |
| --- | --- |
| `008A30D0 BSP_LuaBinding_NavigatorAttackMove` | Lua mission script |
| `008A75C0 BSP_LuaBinding_NavigatorCruise` | Lua mission script |
| `008A2D70 BSP_LuaBinding_NavigatorDirectMoveToRange` | Lua mission script |
| `008A2BC0 BSP_LuaBinding_NavigatorMoveToPos` | Lua mission script |
| `008A2F20 BSP_LuaBinding_NavigatorMoveToRange` | Lua mission script |
| `008A4F00 BSP_LuaBinding_PilotBomb` | Lua mission script |
| `008A4960 BSP_LuaBinding_PilotCloseToShip` | Lua mission script |
| `008A50D0 BSP_LuaBinding_PilotGunFire` | Lua mission script |
| `008A5310 BSP_LuaBinding_PilotTorpedo` | Lua mission script |
| `006CD350 BSP_AirOps_UpdateSlot` | air operations |
| `007F31A0 BSP_PlaneSquadron_OnPlaneLeftMap` | squadron |
| `0046AAB0 BSP_SceneDatabase_ResolveDeferredReferences` | **scene-authored orders** |
| `00844FC0 BSP_Shipyard_CreateLaunchedUnit`, `00846320 BSP_Shipyard_TickAdvance` | shipyard |
| `00853E10 BSP_SubmarineUnit_Serialize` | serialisation |
| `004650F0`, `005241D0`, `00525250`, `00526720`, `00535B20`, `005F9AA0`, `005F9B20`, `005F9BB0`, `005F9C40`, `005FAAE0` | unidentified; the `0052xxxx`/`005Fxxxx` cluster is most likely the player's order UI, **not established** |

Four of the nine Lua bindings are `luaMW_Pilot*` — `PilotBomb`, `PilotCloseToShip`, `PilotGunFire`,
`PilotTorpedo` — i.e. the mission script's own vocabulary for ordering an aircraft. That is the
layer IJN01 would use.

### The gap I did not close

`docs/AI_PLANNERS.md` and `docs/AI_GROUP_THINK.md` describe a **group** AI that issues its own
orders into `group+564Ch` (`00A2CBD0` the attack order, `00A2BE20` the issue helper). Those are
**8-byte AI command objects with their own vtables** (`00D22990`, `00D229E0`, `00D22A38`), a
different object family from the 26 entity command singletons the weapon director holds. **I did
not establish how, or whether, a group-level AI command becomes a per-unit director command.**
`0077D600`'s caller list contains no planner address, so either the hand-off goes through one of
the ten unidentified callers above, or the group AI steers its members by some other route. This
is the single most useful thing to settle next.

## What this means for the host

The bot is complete and idle. For an AI plane to fly anywhere, something must call
`0077D600 BSP_Entity_IssueCommand` for it — natively, that is a mission script or the scene
database's authored orders. Recovering more of `0099D300` will not by itself make a plane turn.

I am **not** proposing the host synthesise a command to fill the gap, and I have not written one.
A synthetic order with a chosen target would produce planes that fly at the fleet and gunnery
numbers that look validated, without any of it being the game's behaviour.

## Coverage

* **Read from the listing this packet**: `0099BE30` (the `+2F0h`/`+2F4h`/`+2F8h` caching),
  `007C18B0`'s two `atan2` arms and the two rate divisions, `0099D300`'s pitch terminal branch,
  the `00D09EC0` vtable region.
* **Resolved**: `bot+2F4h`; `class+1B0h`/`+1B8h`/`+1D8h`; `unit+C64h`, `+C68h`, `+C74h`, `+C78h`.
* **Enumerated exhaustively**: writes to `2F0h`/`2F4h` and `C64h`/`C68h` (disp32 scan validated by
  a linear sweep from every function start, x87 stores counted — see `docs/PLANE_AI_CONTROL.md`
  for the method and its one trap); every reference to `008358D0`; every caller of `0077D600`.
* **Still unread**: the yaw arm's `base` term at `0099E884`; the pitch arm's guards and its other
  two writes; the roll arm (`0099D6B7`), the power arm (`0099DC8F`) and the air-brake arm
  (`0099D8DD`) entirely; `unit+C6Ch`'s derivation; the group-command-to-unit-command hand-off.
* **Reconstructed / build-tested / validated**: **nothing this packet**. No C++ was written,
  because no arm is closed: the one arm whose structure is complete still has an unread input, and
  the brief asked for honest gaps over guessed inputs. Nothing needs registering in
  `cmake/startup.cmake`.
* **Inferred, not proved**: that `unit+C64h` is specifically *pitch* rather than another row's
  elevation, which depends on the forward-axis convention; that the `0052xxxx`/`005Fxxxx` callers
  of `0077D600` are the player order UI.

## Where the orders actually come from: the scripts, not a native planner

The packet above establishes that the bot is starved of orders: no command, no task, and
`0099ACD0` returns without producing anything. This follows that chain out to its source, because
the answer changes what "recover the plane AI" means.

`0077D600 BSP_Entity_IssueCommand`'s caller set (**undercounted here as 13; it is 47 callers over 62
call sites, see the note at the top of this file**) is Lua mission-script bindings plus native
callers. So the question "what orders a plane" is largely a question about the shipped scripts. They
answer it plainly:

```
$ grep -rhoE "Pilot[A-Za-z]*" .../scripts | sort | uniq -c | sort -rn
   1125 PilotSetTarget
    328 PilotMoveTo
    295 PilotMoveToRange
    251 PilotFires
    150 PilotRetreat
     96 PilotMoveOnPath
     78 PilotLand
```

197 script files call `PilotSetTarget`, and they are not only mission scripts:
`scripts/global/commandhelpers.lua` carries 23 pilot-order lines of its own, and
`scripts/global/luamw_init.lua` declares the binding surface. **A large part of what would be called
the plane AI is authored in Lua on top of native bindings, not compiled into the executable.** That
is why no native planner turns up: for the most part there is not one to find.

### IJN01 is the wrong mission to validate air combat in

This matters for the measurements in `docs/PLANE_FREE_FLIGHT_PHYSICS.md`. `IJN01` loads
`Scripts/missions/ijn/ijn_1_pearl.lua`, and that script issues **no** targeting orders at all - one
`PilotLand` and nothing else. It reaches its aircraft through `GenerateObject("JudySpawn1")` into
`Mission.Backups` and `Mission.DiveTable`, and the only thing it later does with `DiveTable` is
`Kill(unit, true)` cleanup.

Which fits the mission: Pearl Harbor is the strike the **player** flies. Its AI aircraft were never
scripted to prosecute an attack, so `AAMACHINEGUN` at 0 shots there is not only explained by the
missing chain - it is close to the authored behaviour. A mission that scripts an air attack is the
one to test against; `usn_19_coralus.lua` (23 lines) and `usn_1_marshall.lua` (15) are the densest
users of `PilotSetTarget` among the mission scripts.

**No claim is made here that those missions would produce air combat today.** They cannot: the chain
below is unbuilt regardless of which mission runs. The point is narrower - that `IJN01`'s zero is
weaker evidence than it looked, because that mission does not order its aircraft to attack in the
first place.

### The chain, end to end

Each link is recovered, authored, or named; none of it needs inventing:

| # | Link | State |
| --- | --- | --- |
| 1 | `00928A00` entity/Lua attach | **UNIMPLEMENTED** in the host. Until it runs, `GenerateObject("JudySpawn1")` cannot hand a script a usable entity, so no Lua order can name a unit. |
| 2 | The `Pilot*` bindings -> `0077D600 BSP_Entity_IssueCommand` | Native, caller set enumerated in this doc. Not built. |
| 3 | `008358D0` sets the director's command | Reached only through vtable slot `+60h`; Ghidra reports zero callers because the dispatch is purely by pointer. |
| 4 | `0099A170 BSP_Bot_InstallCommandTask` | Recovered here: the only caller of all thirteen `BotTask_Make*` factories, and it takes its target from the command. |
| 5 | `0099D300`'s five axis arms | One arm's structure recovered; its base term at `0099E884` unread; four arms unread. |
| 6 | `007C6500 BSP_PlaneTickElement_AdvancePose` | The pose integrator, under recovery separately. Without it a plane cannot turn even when commanded. |

Links 1 and 2 are the cheapest and gate everything after them. Recovering more of link 5 moves no
measurement while 1-4 are missing, which is why that work was stopped.

### What is not established

~~Whether the host can run these scripts at all once the entity attach exists... And the group-AI
question is still open: no planner address appears in `0077D600`'s caller set.~~

**Both halves of that paragraph were wrong, and both were mine to check before publishing.**

*The group-AI gap does not exist.* It was built on the same 25-row caller list the note at the top
of this file supersedes. The planners `009FFEB0`, `00A02020`, `00A11FF0`, `00A13B60`, `00A14DD0` and
`00A2F6F0` all issue commands through `0077D600`. There is no missing hand-off to find. The
undercount also made me write "nine Lua mission-script bindings plus four native callers" twice in
this document; `bsp.py callers` reports 47 and `ghidra xrefs` 62 call sites, and the one it omitted
was `PilotSetTarget`, the most-used binding in the shipped scripts by an order of magnitude.

*The scripts already run.* `USN01` at 3000 mission ticks shows the mission Lua layer working hard -
`FindEntity`, `SetInvincible`, `UnitSetFireStance`, `NavigatorSetTorpedoEvasion`, `AddListener` and
dozens more - and, decisively:

```
native PilotSetTarget  argc=2  phase=luaStageInit
host MissionLuaNative::PilotSetTarget [008a4c90] UNIMPLEMENTED, returning a neutral value
host WeaponDirector::issue_command [0071ecf0] concrete  calls=79
host SceneCommand::issue_command  [0077d600] concrete
```

**The script issues the order and the host drops it on the floor.** Both ends of the chain are
already built - the Lua layer above and the command path below, `0071ECF0` running 79 times from
other callers - and the `Pilot*` bindings in between are stubs returning a neutral value. That is
the whole of the remaining gap for directed air combat, and it is host wiring rather than recovery.

The entity-attach row in the table above should be read the same way: `00928A00` is
`coverage: complete` in `docs/MISSION_ENTITY_LUA_ATTACH.md` and `src/game_hosts_lua.cpp:1310` builds
the slot's four fields, so the `log_.unimplemented` call at line 1368 overstates what is missing.
Packet `cc7_pilot_order_bindings` recovers the five binding bodies.

# The axis arms read from the listing (packet `cc7_pilot_bot_axis_arms`)

Addresses: `0099D300`, `0099D46E`-`0099D510`, `0099E664`-`0099E752` (pitch), `0099E75E`,
`0099E81A`-`0099EA57` (yaw), `007C18B0`, `00419010`, `00415510`, `00415620`, `00415690`,
`00438B10`, `00BF701A`. Constants `00CE3854`, `00CE3D10`, `00CE65D0`, `00CE3C64`, `00D05B50`,
`00D7A208`, `00D7A220`, `00D7A24C`, `00D7A260`, `00E0E2F0`, `00E0E2F4`.

Ghidra read-only. **Exported and reconstructed; build-tested** (`scripts/build.ps1`, and the one
existing `reconstructed_math` check still passes). Not fixture-tested and not game-validated. No
new tests were added.

## Method

The function is 1451 instructions, so the arms were read from a full listing dump rather than by
paging, and the stack slots were resolved with a reaching-definition walk over a CFG built from
that dump. **ESP is stable** between `0099D3DB PUSH EBP` and `0099EA66 POP EBP`: every `PUSH` or
`SUB ESP,n` in between opens an argument window that the following `CALL` closes, because every
callee is callee-clean — verified on the two that matter, `00419010` `RET 14h` (`004190B9`,
`004190CC`) and `00415690` `RET 4` (`004156DD`, `004156ED`). Displacements inside those windows
are corrected by the window's own depth before being compared.

## The five plan slots are 12-byte records

`include/bsp/plane_ai_control.hpp` already had `kPlanSlotBase = 0x274` and `kPlanSlotStride = 0x0C`.
The writes confirm the field order `{ prev, desired, active }`:

| axis | prev | desired | active | arm's store |
| --- | --- | --- | --- | --- |
| power | `+274h` | `+278h` | `+27Ch` | `0099DC8F` |
| yaw | `+280h` | `+284h` | `+288h` | `0099EA3E` |
| roll | `+28Ch` | `+290h` | `+294h` | `0099D6B7` |
| pitch | `+298h` | `+29Ch` | `+2A0h` | `0099E739` |
| air brake | `+2A4h` | `+2A8h` | `+2ACh` | `0099D8DD` |

`0099E9A9` reading `[ESI+280h]` when `[ESI+288h]` is clear is what fixes the order: it is the yaw
record's own `prev`, one field below its `desired`.

## The per-tick frame, `0099D46E`-`0099D510`

`EBX = BSP_GameTuning_GetSingleton() + 538h` (`0042E740` at `0099D46E`, `ADD EBX,538h` at
`0099D487`). Then, with `ECX = unit`:

```
0099d479  s   = unit[+340h] * 0.4              ; 00CE65D0 = 0.4 (double)
0099d491  [ESP+38h] = max(s, 1.0f)             ; FCOMI/JBE at 0099D497/0099D49B
0099d4b3  [ESP+28h] = 1.0f / max(s, 1.0f)
0099d4dc  [ESP+10h] = unit+C64h                ; pitch
0099d4d6  [ESP+34h] = unit+C68h                ; bank
0099d4e2  [ESP+1Ch] = |bank|                   ; AND EAX,7FFFFFFFh at 0099D4D1
0099d4fc  [ESP+20h] = sin(bank)                ; FSIN
0099d506  [ESP+30h] = cos(bank)                ; FCOS
0099d510  [ESP+2Ch] = cos(pitch)               ; FCOS
```

## Five corrections to this document's own yaw section

1. **`[ESP+18h]` is not an unread input.** It is computed in place at `0099E81A`-`0099E884`:
   `0099E823` stores zero into it, and the block only overwrites it when `[ESP+38h] > 0`. What the
   earlier reading called `base` and what it called `turn` are the same slot at different times.
2. **The first term's denominator is `cos(bank)`, not `sin(bank)`.** `0099E843 FMUL [ESP+30h]`, and
   `[ESP+30h]` is the `FCOS` at `0099D504`. `sin(bank)` is `[ESP+20h]` and belongs to the *second*
   term (`0099E908`).
3. **The second term's numerator is not `unit+C64h`.** The reaching definitions of `[ESP+10h]` at
   `0099E8F6` are `0099E3CB`, `0099E6D2`, `0099E6E8` and `0099E729` — the setup's pitch store at
   `0099D4DC` does **not** reach the yaw arm on any path. Three of the four are pitch-arm outputs,
   so the yaw arm is **driven by a pitch-arm side product**, and the two arms are not independent.
4. **The arm has one live entry.** It is reached only from `0099E75E JNZ 0099E81A` on a non-zero
   `task+2D4h`. The other edge, `0099E774 JBE 0099E812`, arrives with `ECX` already known zero from
   `0099E75C TEST ECX,ECX`, and nothing writes `ECX` in between, so `0099E812 TEST ECX,ECX / JZ
   0099EA57` always exits. That edge writes no yaw at all.
5. **The blend is asymmetric.** When the turn numerator is not positive, `0099E897 JBE 0099E960`
   skips the whole second block, and the `(1 - t)` scaling lives *inside* that block — so the base
   term reaches the final clamp unscaled, not multiplied by `(1 - t)`.

## The yaw arm, `0099E81A`-`0099EA46`

```
base = 0                                                    ; 0099E823
if ([ESP+38h] > 0)                                          ; 0099E820 COMISS / 0099E829 JBE
    base = clamp([ESP+6Ch] / (YawSpd * cos(bank) * tuning[+9Ch]), -1, +1) * [ESP+38h]
turn_term = 0                                               ; 0099E891
if ([ESP+10h] > 0) {                                        ; 0099E88E / 0099E897
    t    = min(1, InterpolateClamped(tuning[+7Ch], 0, tuning[+80h], 3.0f, |bank|))
    turn = clamp([ESP+10h] / (YawSpd * sin(bank) * tuning[+9Ch]), -1, +1)
    turn_term = t * turn ; base = (1 - t) * base            ; 0099E94A-0099E95C
}
yaw = clamp(turn_term + base, -1, +1)                       ; 0099E96C, 0099E98D
slot(yaw).desired = yaw ; slot(yaw).active = 1 ; task+2D4h = 0   ; 0099EA3E / EA46 / EA4D
```

`YawSpd` is `class+1B0h` through `task+2F4h` (`0099E82F`, `0099E8F0`). `3.0f` is `00CE3854`; the
clamp bounds are `00D7A260` = `-1.0f` and `00D7A24C` = `1.0f`. `00415690` is the in-place clamp,
`00415620` the by-value one, `00415510` the min.

Implemented as `plan_yaw_0099e81a` in `src/plane_ai_control.cpp`, with the three scratch
quantities as **parameters**, because they are located but not named (below).

## The pitch arm's terminal law, `0099E68D`-`0099E752`

The earlier reading saw only the `-1` branch. All three paths converge on the single store at
`0099E739`, and together they are a plain saturation:

| demand `[ESP+44h]` | path | stored to `slot(pitch).desired` |
| --- | --- | --- |
| `> 1.0f` | `0099E696` not taken | `+1.0f` (`XMM0 = XMM3` at `0099E6A1`) |
| `< -1.0f` | `0099E6F9` not taken | `-1.0f` (`XMM0 = XMM4` at `0099E6FE`) |
| otherwise | `0099E6F9 JBE 0099E72F` | the demand itself, still in `XMM0` from `0099E68D` |

so `slot(pitch).desired = clamp(demand, -1, +1)`, implemented as `plan_pitch_0099e68d`.

Two of the three paths also leave a side quantity in `[ESP+10h]` — the value the yaw arm then
consumes:

```
0099e69b  p = sin(bank)^2 * cos(pitch) * class[+1B8h] * class[+1B0h]   ; SlideRatio, YawSpd
0099e6ba  k = (XMM1 > 0) ? class[+1D8h] : 1.0f                         ; NegativePitchRatio
0099e6ce  [ESP+10h] = p - k * X
```

`X` is a live x87 stack value carried from before `0099E69B` — the `FMULP` at `0099E6CC` multiplies
`k` into whatever `ST(1)` already held. **It was not traced**, so the side product, and therefore
the yaw arm's turn numerator, is not closed. The `0099E703`-`0099E729` copy of this block under the
`-1` branch is identical in shape.

## The re-plan interval, `0099E996`-`0099EA2B`

After the store, the size of the change is folded into `task+2ECh`:

```
ref = slot(yaw).active ? slot(yaw).desired : slot(yaw).prev      ; 0099E996
d   = |ref - yaw|                                                ; 0099E9BF, 0099E9DF (XMM4 = -0.0f)
if      (d > 0.2)   task+2ECh = 0.1                              ; 00CE3D10, 00E0E2F4
else if (d > 0.08)  task+2ECh = min(task+2ECh, 0.16)             ; 00D05B50, 00E0E2F0
```

`task+2ECh` is a **re-plan interval in seconds**, not an urgency score: `0099D438` seeds it from
`00E0E2EC` and `0099D45E` forces `0.1` when `unit[+900h]` is `4` or `5`, and `0099E764`/`0099E76A`
compare it with `0.1` to decide whether to re-plan at all. The same `|Δ|` update appears at
`0099E7DB`/`0099E7FD` for another axis. Implemented as `axis_urgency_0099e996`.

`00E0E2F0` and `00E0E2F4` are in writable data; they read `0.16f` and `0.1f` in the image on disk,
but this packet did not establish that nothing writes them at startup.

## `unit+C6Ch` is the heading, and `007C18C1` only reads it

`007C18C1` saves the **previous** value before recomputing the attitude triple. The write is:

```
007c1a19  FLD  [ESP+70h]                                    ; pushed first, so ST(1) at the call
007c1a1d  FLD  [ESP+68h]                                    ; ST(0)
007c1a21  CALL 00BF701A LIBCRT_atan2                        ; which operand is y was not checked
007c1a32  FST  [ESI+C6Ch]                                   ; the raw heading
007c1aad  CALL 00438B10 BSP_Math_SubtractWrappedAngle(pi/2, [ESI+C6Ch])   ; 00CE3C64 = 1.5707963f
007c1aca  FST  [ESI+C6Ch]                                   ; the wrapped heading
```

`unit+C64h` (pitch) is stored at `007C1966` and `unit+C68h` (bank) at `007C1A94` in the same pass.
**The two `atan2` operands were not traced** — they come from `[ESP+68h]`/`[ESP+70h]`, filled by the
matrix work at `007C18F3`-`007C1A14` that this packet did not read — so the heading's derivation is
recovered only as far as "wrapped `atan2` of two orientation terms".

## What is still unread, named

* **The roll arm (`0099D6B7`), the power arm (`0099DC8F`) and the air-brake arm (`0099D8DD`)** were
  not read. Only their stores and slot offsets are established here.
* **The yaw arm's three scratch inputs.** Each is located to its reaching definitions and each has
  a zero default, but the non-zero producers are unread:
  | slot | read at | reaching definitions |
  | --- | --- | --- |
  | `[ESP+6Ch]` | `0099E82B` | `0099DDD0` (`= 0`), `0099DF87` (a `tuning+34h` blend) |
  | `[ESP+38h]` | `0099E81A` | `0099DD9D` (`= 0`), `0099E027` |
  | `[ESP+10h]` | `0099E8F6` | `0099E3CB` (`= 0`), `0099E6D2`, `0099E6E8`, `0099E729` |
* **The pitch demand `[ESP+44h]`** (`0099E664`-`0099E689`): both of its branches consume a live x87
  stack value, and `0099E673 FDIVR [ESP+18h]` divides by it. Untraced.
* **`X` at `0099E6CC`/`0099E723`**, the multiplicand of `NegativePitchRatio`.
* **The `atan2` operands** for the heading.

## Wiring contract

`plan_pitch_0099e68d` and `axis_urgency_0099e996` are complete laws and can be wired now.
`plan_yaw_0099e81a` is a complete law **given** its three scratch inputs; a host that cannot supply
them must refuse the yaw axis rather than pass zeros, because all three have a legitimate zero
default and zeros would silently produce a plausible centred yaw. The roll, power and air-brake
axes have no law here at all and must refuse.

> **Superseded by the next section.** All three scratch inputs are now produced, so the yaw axis is
> wirable apart from two opaque sources. The roll, power and air-brake stores turn out not to be
> steering laws at all.

# The yaw scratch producers, and what the other three stores actually are (packet `cc7_pilot_bot_axis_arms_2`)

Addresses: `0099D602`-`0099D6C6`, `0099D8C1`-`0099D8EB`, `0099DC07`-`0099DC97`,
`0099DDCA`, `0099DE8A`-`0099DF87`, `0099DFCC`-`0099E027`, `0099E5CB`-`0099E6D2`, `007D9A70`,
`00415550`. Constants `00CE3D30`, `00D7A390`, `00D7A3A0`.

Same method as the previous section: the full listing plus the reaching-definition walker.
Ghidra read-only. **Exported, reconstructed, build-tested**; the existing `reconstructed_math`
check still passes and no new tests were added. Not fixture-tested, not game-validated.

## All three yaw scratch inputs are now produced

### `[ESP+6Ch]`, the base numerator — a deadbanded, step-limited heading error

`0099DE8A`-`0099DF87`, reached only when `task+2CCh == 2` (`0099DE8A CMP ECX,2`); on every other
path it keeps the zero stored at `0099DDD0`.

```
h = unit->vtable[50h]()                                   ; 0099DE9E
e = SubtractWrappedAngle(task+2C0h, h) * q                ; 0099DEB8, 0099DEBD
                                                          ; q = 1 / max(unit[+340h] * 0.4, 1)
e = (e >= D) ? e - D : (e <= -D) ? e + D : 0              ; D = tuning+3Ch, 0099DECC-0099DF03
L = (class+1C8h + class+1ACh) * tuning+38h                ; 0099DF0F-0099DF24
e = (L > |e|) ? tuning+34h * e
              : (e > 0 ? e - (1-tuning+34h)*L : e + (1-tuning+34h)*L)
```

The middle case of the deadband zeroes the term outright (`0099DEFC XORPS XMM0,XMM0`). The two
comparisons are `FCOMI`/`JC` at `0099DED4` and a `FLD ST1` / `FCHS` / `FCOMIP` against `-D` at
`0099DEE6`-`0099DEEC`, read from the listing.

### `[ESP+38h]`, the base gain — a bank fade

`0099DFFB`-`0099E027`:

```
gain = InterpolateClamped(tuning+7Ch, 1.0f, tuning+80h, 0.0f, |bank|)
```

`0099E016 FLD1` supplies `y0 = 1` and `0099E006 FLDZ` supplies `y1 = 0`. This is the **reverse** of
the blend fraction inside the yaw arm, which runs `0` to `3.0f` over the same `x` range — so the
heading term carries full weight at low bank and fades to nothing at high bank, while the turn term
fades in. That is the arm's whole shape: **steer by heading when level, by the pitch-derived term
when banked.**

### `[ESP+10h]`, the turn numerator — a pitch-arm side product

`0099E69B`-`0099E6D2` (and the identical mirror at `0099E703`-`0099E729`):

```
p = sin(bank)^2 * cos(pitch) * class+1B8h * class+1B0h     ; SlideRatio, YawSpd
X = class+1ACh * (R * 0.9 + 0.1) * cos(bank)               ; 0099E630-0099E64A
k = inverted ? class+1D8h : 1.0f                           ; NegativePitchRatio
[ESP+10h] = p - k * X                                      ; 0099E6CE FSUBR
```

`X` is the live x87 value the previous section could not trace. The stack walks: `0099E5D7` calls
`007D9A70(unit+AB0h)` returning `R` in `ST0`; `0099E5DC`/`0099E5EE` form `R*0.9 + 0.1` (`00D7A390`
= `0.9`, `00D7A3A0` = `0.1`) and `0099E5FC` stores it to `[ESP+54h]`, emptying the stack;
`0099E630`-`0099E63A` push `class+1ACh * [ESP+54h] * cos(bank)` — **that** is `X`, and it survives
the `FSTP` at `0099E64A` and the demand's `FSTP` at `0099E689` because both pop values pushed above
it. All three pitch paths balance it: `0099E72F FSTP ST0` pops it on the unsaturated path.

`inverted` is settled, not guessed. The predicate register `XMM1` is loaded once at `0099E5E5` from
`[ESP+44h]`, and that slot has **exactly one** reaching definition, `0099DDCA` — which is
`(float)sign(cos(bank))` from `0099DD9A`-`0099DDCA`. So `k` is `NegativePitchRatio` precisely when
`cos(bank) < 0`, i.e. when the plane is inverted, and `1.0f` otherwise. The same predicate chooses
`[ESP+40h]` at `0099E60C`.

`R = 007D9A70(unit+AB0h)` was **not read**. It reads `[ECX+8]` then `[EAX+908h]` and runs a
five-argument interpolation with `00CE6630` and `00CE3868`; `R*0.9 + 0.1` maps it onto `[0.1, 1.0]`,
which is consistent with a normalised 0..1 factor, but this packet did not establish that.

## The other three stores are not steering laws

### `0099D6B7` (roll), and its yaw and pitch siblings, are a stick override

`0099D602`-`0099D6C6` is one block of three identical arms that runs **before** any computed arm:

| source | store | mode word cleared |
| --- | --- | --- |
| `unit+998h` (`0099D608`) | `slot(yaw).desired = clamp(v, -1, +1)`, active = 1 (`0099D63B`) | `task+2D4h` (`0099D64A`) |
| `unit+99Ch` (`0099D656`) | `slot(pitch).desired = clamp(v, -1, +1)`, active = 1 (`0099D679`) | `task+2D0h` (`0099D688`) |
| `unit+9A0h` (`0099D694`) | `slot(roll).desired = clamp(v, -1, +1)`, active = 1 (`0099D6B7`) | `task+2CCh` (`0099D6C6`) |

The guard is MSVC's exact-equality idiom — `UCOMISS XMM0,XMM2` / `LAHF` / `TEST AH,44h` / `JNP` —
which takes the skip branch only on an ordered compare equal to zero, so the arm runs whenever the
field is non-zero (and also on NaN). `XMM1` and `XMM3` are `-1.0f` (`00D7A260`) and `1.0f`
(`00D7A24C`) from `0099D610`/`0099D618`, so the clamp is to `[-1, +1]`.

**This closes the mode words.** `task+2CCh`, `+2D0h` and `+2D4h` are per-axis enables for the
computed arms, and a direct stick input cancels the one it overrides — which is why the yaw arm at
`0099E81A` is reached only through `0099E75E JNZ` on a non-zero `task+2D4h`, and why `task+2CCh == 2`
specifically is what enables the heading-hold base term. `unit+998h`/`+99Ch`/`+9A0h` sit alongside
the `unit+994h` throttle that `docs/UNIT_STATE_MESSAGE.md:193` records.

### `0099D8DD` (air brake) is the speed-hold arm's brake

`0099D8C1`-`0099D8EB`, gated on `task+2D8h == 1` and a `COMISS` of `XMM0` against `task+2B4h`:

```
slot(power).desired = XMM0 ; active = 1        ; 0099D8CF
slot(brake).desired = 1.0f ; active = 1        ; 0099D8DD, XMM3 = 00D7A24C
task+2D8h = 0                                  ; 0099D8EB
```

Full air brake when the quantity in `XMM0` exceeds the target at `task+2B4h`. **`XMM0`'s provenance
was not traced**, so the arm's trigger is unread; only its effect is established.

The sibling pair at `0099DC31`/`0099DC4B` on the same mode writes
`slot(power).desired = max(0.001f, [ESP+18h])` and `slot(brake).desired = max(0.0f, [ESP+44h])`
through `00415550 BSP_Math_MaxFloatByRef`, then clears `task+2D8h`. `[ESP+18h]` there is the bank
error `SubtractWrappedAngle(bank, [ESP+1Ch])` formed at `0099E554`-`0099E559` — reaching
definitions confirm a single producer — but `[ESP+44h]` was not traced.

### `0099DC8F` (power) is a ceiling, not a law

`0099DC7A`-`0099DC97`:

```
if (slot(power).prev > 0.6f) { slot(power).desired = 0.6f ; active = 1 }   ; 00CE3D30 = 0.6f
```

It stores the constant itself, not the previous value. Nothing else in the block computes a power
demand.

## Rules added

`yaw_base_numerator_0099de8a`, `yaw_base_gain_0099dffb`, `yaw_turn_numerator_0099e69b`,
`stick_override_0099d620` and `power_ceiling_0099dc7a` in `src/plane_ai_control.cpp`.

## Wiring contract

The **yaw axis is now wirable**. A host needs, per tick: the plane's `unit+340h`, the class fields
`+1ACh`, `+1B0h`, `+1B8h`, `+1C8h`, `+1D8h`, the tuning fields `+34h`, `+38h`, `+3Ch`, `+7Ch`,
`+80h`, `+9Ch`, the attitude triple, `task+2C0h` and `task+2CCh` — plus the two sources this packet
did not read, `unit->vtable[50h]()` and `007D9A70(unit+AB0h)`. Both are **required**: the heading
getter is the base term's only input, and `R` scales the turn term. A host without them must still
refuse the axis.

The **stick override must be applied first**, and it must clear the mode word, or the computed arms
will run on a tick the game would have given to the pilot's own input.

The **pitch axis** remains `clamp(demand, -1, +1)` with its demand untraced; **roll** has no
computed law here beyond the override; **power** has only the `0.6f` ceiling and the speed-hold
pair; **air brake** has only the speed-hold pair, with its trigger unread.

## What is still unread, named

* `unit->vtable[50h]`, the heading getter (`0099DE9E`).
* `007D9A70`, the speed factor `R` (`0099E5D7`).
* The pitch demand `[ESP+44h]` at `0099E664`-`0099E689`. `[ESP+18h]` is the bank error
  `SubtractWrappedAngle(bank, [ESP+1Ch])` (`0099E559`), scaled by `q` at `0099E600`-`0099E608`.
  `0099E673 FDIVR [ESP+18h]` then divides that by the **signed** `k*X*tuning+A0h` — `0099E652`
  keeps the signed value in `ST0` while `0099E656`-`0099E660` put only its magnitude in `[ESP+54h]`
  for the `0.001f` test — and `0099E67B`-`0099E685` is the small-magnitude fallback
  `[ESP+18h] * 100.0 * sign(cos(bank))` (`00D7A220` = 100.0, and `[ESP+44h]` there is still the old
  sign value). The shape is now visible, but the arm that sets the bank target `[ESP+1Ch]` at
  `0099E53D` was not read, so the demand is not closed.
* `XMM0` at `0099D8C6`, the speed-hold trigger, and `[ESP+44h]` at `0099DC46`.
* `task+2B4h`, `task+2C0h` and `task+2D8h`'s producers.

# The yaw arm's last two inputs (packet `cc7_yaw_remaining_inputs`)

Addresses: `0074E260`, `007D9A70`, `007D99C0`, `007C0F40`, `00419010`. Constants `00CE3854`,
`00CE3868`, `00CE6630`, `00CE74F8`, `00CEFF98`, `00D7A24C`; globals `00F8731C`, `00F87320`.

Ghidra read-only. **Exported, reconstructed, build-tested**; `reconstructed_math` still passes, no
new tests. Not fixture-tested, not game-validated.

## `unit->vtable[50h]` is the heading field, and every plane class agrees

Enumerated rather than sampled: slot `+50h` read from the vtable of each of the nine plane classes
in `docs/ENTITY_CLASS_IDS.md` — `00D05F20` (`0F`), `00D06638` (`10`), `00D1A000` (`11`),
`00D19D28` (`12`), `00D06920` (`13`), `00D00070` (`14`), `00D0BA80` (`15`), `00D00308` (`16`),
`00D1A2D8` (`17`). **All nine hold `0074E260`**, and its entire body is

```
0074e260  D9 81 6C 0C 00 00   FLD dword ptr [ECX + 0C6Ch]
0074e266  C3                  RET
```

Other families override the slot — `0042B8C0` on `Path`, `006DFD60` on `MDestroyer` and
`MTorpedoBoat`, `006D2040` on `MAirfield` — so this is the plane family's implementation, not a
universal one. That is why the slot had to be enumerated before being named.

So the base term's "current heading" is simply **`unit+C6Ch`**, the wrapped `atan2` that
`007C1ACA` writes (previous section). The base numerator is
`SubtractWrappedAngle(task+2C0h, unit+C6Ch) * q` with no dispatch involved, and `kUnitHeading` is
added to `include/bsp/plane_ai_control.hpp`.

## `007D9A70` — the speed factor `R`

`__thiscall(sub)` where `sub` is `unit+AB0h` (`0099E5D1 ADD ECX,0AB0h`), returning in `ST0`:

```
a      = InterpolateClamped(3.0f, 0, 6.0f, 0.25f, [[sub+8h]+908h])        ; 007D9AA9
ratio  = BSP_PlaneFlight_ForwardSpeed(sub) / class+184h                    ; 007D9AB4, 007D9ABC
c      = InterpolateClamped([00F8731C], 0, [00F87320], 1.0f, ratio)        ; 007D9AF0
v      = c + [[sub+10h]+C0h] * 0.6                                         ; 007D9B02, 00CEFF98
result = (a > v) ? a : min(v, 1.0f)                                        ; 007D9B1C / 007D9B55
R      = 007C0F40(sub+8h) * result * result                                ; 007D9B3F, 007D9B45
```

`007D99C0` is already named `BSP_PlaneFlight_ForwardSpeed`, and `class+184h` is in the speed block
`+184h`..`+1A4h` that `docs/VEHICLE_CLASS_FIELDS.md` records, so `ratio` is a speed fraction.
`007D9B3F FMUL ST0` squares `result` (`D8 C8`), the same encoding as `0099E69F`; the bomb-load
factor is multiplied in separately at `007D9B45` from memory.

### `007C0F40`, the bomb-load factor

```
m = 1.0f                                                       ; 007C0F41
for each weapon slot at plane+974h, count plane+994h:
    if (slot->vtable[210h](2Ah, 0)) {                          ; 007C0F72, 2Ah = MBomb
        w = slot->vtable[214h]()                               ; 007C0F9D
        m = InterpolateClamped(0, 1.0f, 0.8f, class+15Ch, w)   ; 007C0FC5, 00CE74F8 = 0.8f
        break                                                  ; 007C0F76
    }
if (plane+BC8h) m *= class+608h                                ; 007C0FCF, 007C0FE4
```

The `+974h`/`+994h` slot array and the `2Ah` kind are the same ones
`docs/ORDNANCE_KIND_IDENTITY.md` established, so this is literally "is the plane still carrying
bombs". `class+608h` is the top of the turbo block `+5FCh`..`+608h`, so turbo scales the result.

### The range, which matters as much as the formula

`R` is **non-negative and bounded by the bomb-load factor**, and this holds whatever the two
unidentified fields and the two runtime globals turn out to be:

* `a = InterpolateClamped(3, 0, 6, 0.25, ...)` has both y-endpoints in `[0, 0.25]`, so `a ∈ [0, 0.25]`
  for any input.
* `result = max(a, min(v, 1))`. Since `a ≥ 0`, `result ≥ 0` even if `v` is negative; since
  `a ≤ 0.25` and `min(v, 1) ≤ 1`, `result ≤ 1`. So **`result ∈ [0, 1]` unconditionally**, and
  `result² ∈ [0, 1]`.
* Therefore `R ∈ [0, m]`, and the yaw turn term's remap `R*0.9 + 0.1` lies in `[0.1, 0.9m + 0.1]`
  — that is `[0.1, 1.0]` for a clean plane with no turbo, where `m = 1`.

`c` and hence `v` depend on the two globals, but they cannot move the bound, because `result` is
pinned by `a` from below and by the explicit `1.0f` cap from above.

## What is still unread, named

* **`00F8731C` and `00F87320`**, the `ratio` interpolation's x-endpoints. Both read **zero in the
  image on disk**, so they are initialised at runtime and their values are not established. They
  affect `c`, not the bound above. If they really are equal at runtime the interpolation degenerates
  at `x0 == x1`; `00419010`'s body was not re-read, so that case is not characterised here.
* **`[[sub+8h]+908h]`** (the `a` input) and **`[[sub+10h]+C0h]`** (the `v` addend): the sub-object
  fields at `unit+AB0h+8h` and `+10h` were not identified.
* **`slot->vtable[214h]`**, the per-slot weight `007C0F40` interpolates on.
* Everything the previous section listed as open is still open: the pitch demand's bank target, the
  speed-hold trigger, and the producers of `task+2B4h`/`+2C0h`/`+2D8h`.

## Wiring contract

The heading getter needs no host hook at all — read `unit+C6Ch`. `R` needs
`BSP_PlaneFlight_ForwardSpeed`, `class+184h`, `class+15Ch`, `class+608h`, the ordnance query the
host already has from `docs/ORDNANCE_KIND_IDENTITY.md`, the turbo byte `plane+BC8h`, and the four
quantities named above as unread. A host that supplies the unread four as zero gets `a` from a zero
altitude term and `c` from a zero ratio — a **legal** result, not an obviously wrong one, so this
is another place to refuse rather than default. The bound `R ∈ [0, m]` is what a host can assert
against if it wires the axis and wants a cheap sanity check.

# The speed factor's four leaves (packet `cc7_speed_factor_leaves`)

Addresses: `00419010`, `007D9A70`, `007C0F40`, `006E4130`, `006E3700`. Globals `00F87300`-`00F8733F`.

Ghidra read-only. **Exported, reconstructed, build-tested**; `reconstructed_math` still passes, no
new tests. Not fixture-tested, not game-validated.

**All four close.** One of them closes by correcting this document's own speculation, and one of
them corrects a helper I had already written.

## `00F8731C` and `00F87320` are never written, and the degenerate case is the opposite of what I guessed

A byte scan of `.text` for every absolute reference into `00F87300`-`00F8733F` finds **26
references, all x87 reads** — `D9 05` (`FLD`), `D8 0D` (`FMUL`), `D8 35` (`FDIV`) — spread across
the plane-flight code at `007D92xx`-`007DCAxx`. There is **no write in any form**: no
`MOV [imm32], reg`, no `MOV reg, imm32` that would let a loop index the block, and `.data` and
`.rdata` contain no reference to any address in the window either. The whole 64-byte block reads
zero in the image.

So `007D9A70`'s second interpolation runs with `x0 == x1 == 0.0f`. The previous section flagged
that case and declined to characterise it. Reading `00419010`'s body settles it, and **the guess
that section did not make would have been wrong**:

```
00419010  FLD [ESP+0Ch]        ; x1
00419016  FLD [ESP+04h]        ; x0
0041901E  FUCOMIP ST0,ST2      ; compare x1 with x0
00419022  LAHF / TEST AH,44h
00419026  JP 00419033          ; taken only when they DIFFER
00419028  FSTP ST0 / FSTP ST0
0041902C  FLD [ESP+08h]        ; -> returns y0
00419030  RET 14h
```

The equality idiom again: equal gives `ZF=1, PF=0`, so `AH & 44h` is one bit, odd parity, `JP` not
taken, and the function returns **`y0`**. So `c` is a **constant `0.0f`** in this build — not `1.0`,
which is what a "ratio above the range saturates at `y1`" reading would have produced. `v` is
therefore `[controller+10h][+C0h] * 0.6` alone.

### A correction to `interpolate_clamped` in `src/plane_ai_control.cpp`

Reading the body also corrected the helper I wrote two packets ago, which clamped on the **x** side.
The native clamps on the **y** side: `00419063`-`004190CC` picks `min(y0,y1)` and `max(y0,y1)` into
`XMM0`/`XMM1` and bounds the interpolated value between them, returning the raw value at `004190BC`
and the two bounds at `00419094`/`004190AF`. For an ascending `x` pair the two are equivalent, but
they differ when `x0 > x1` and they differ decisively when `x0 == x1`. `yaw_base_gain_0099dffb`
passes a **descending** `y` pair (`y0 = 1`, `y1 = 0`), which is exactly the case the y-side clamp
exists for. The helper now matches the listing.

## The two sub-object fields

`unit+AB0h` is `kFlightController` (`include/bsp/plane_flight.hpp:77`). Its two fields resolve
without reading anything new:

* **`controller+8h` is the plane unit itself.** `007D9B35` passes it as `this` to `007C0F40`, and
  that function reads `+974h`/`+994h` (the weapon slot array), `+538h` (the class descriptor) and
  `+BC8h` — all unit offsets. So `[[sub+8h]+908h]` is **`unit+908h`**, the scalar
  `docs/PILOT_CONTROLS.md:215,218,222` pairs with `+904h` against `5.0f` and sets to `0` or
  `3600.0f` on state transitions. This interpolation's x range is `3..6`, which brackets that
  `5.0f` threshold. That doc did not name the field and neither does this one.
* **`controller+10h` is the dynamics block**, which `include/bsp/plane_flight.hpp:370` already
  records (`dyn+04h`, `+10h`, `+1Ch`, `+28h`, `+34h`, `+40h`). `007D9AF9` loads it as a **pointer**
  and reads `+C0h`, which is beyond the fields that header lists. **`dyn+C0h`'s meaning is not
  established** — and with `c` pinned at zero it is now the *only* thing that moves `v`.

## `slot->vtable[214h]` is the remaining bomb-load fraction

Only the bomb-platform vtables are long enough to have slot `214h`; on the shorter gun vtables that
displacement lands past the end, in the string data that follows a vtable (`00CFE75C` reads
`"ing"`, `00CFACCC` reads `"ePar"` — both mid-string), which is why the slot had to be read per
class rather than taken from the base.

| class | vtable | `+214h` |
| --- | --- | --- |
| `MBombPlatform` | `00CF96A8` | `006E4130` |
| `MMultipleBombPlatform` | `00CF9918` | `006E3700` |

```
006E3700 (multiple):  return (float)vtable[21Ch](2Ah) / (int)this+488h
006E4130 (single):    if (this+488h == 0) return 0.0f
                      return (float)(vtable[21Ch](2Ah) + this+484h) / (int)this+488h
```

Both are `FILD`/`FIDIV` integer divides. `vtable[21Ch](2Ah)` is a **count** of kind-`2Ah` items —
the same `2Ah = MBomb` id space `docs/ORDNANCE_KIND_IDENTITY.md` established — and `+488h` is the
platform's capacity, so `w` is the **fraction of the bomb load still aboard**, in `[0, 1]`.

That makes `007C0F40` legible: `InterpolateClamped(0, 1.0f, 0.8f, class+15Ch, w)` gives `m = 1.0`
with no bombs left and `m = class+15Ch` at four-fifths loaded or more. **It is a handling penalty
for carrying bombs**, which is why `R`, and therefore the yaw turn term, degrades with a full load.

## What this does to the model

`c = 0` and `ratio` is computed but unused, so in this build

```
a      = InterpolateClamped(3, 0, 6, 0.25, unit+908h)      in [0, 0.25]
v      = dyn+C0h * 0.6
result = max(a, min(v, 1))                                  in [0, 1]
R      = m * result^2                                       in [0, m]
```

The bound from the previous section is **unchanged**, and now rests on less: `a` alone pins
`result` from below and the explicit `1.0f` caps it from above.

## What is still unread, named

* **`dyn+C0h`** — the only live input to `v`.
* **`unit+908h`'s meaning.** `docs/PILOT_CONTROLS.md` established its producers and its `5.0f`
  threshold but explicitly did not name it, and this packet did not either.
* **`slot->vtable[21Ch]`**, the kind-`2Ah` count.
* The scan is of `battlestationspacific.exe` only. This installation carries mod artefacts
  (`docs/AA_VERTICAL_WINDOW.md` and others record it), so a loaded module could in principle write
  those globals; that was not checked, and the claim here is about the shipped executable.
* Unchanged from before: the pitch demand's bank target, the speed-hold trigger at `0099D8C6`, and
  the producers of `task+2B4h`/`+2C0h`/`+2D8h`.

## Wiring contract

Of the yaw arm's inputs, a host now needs from this packet: `unit+908h`, `BSP_PlaneFlight_ForwardSpeed`,
`class+184h`, `[controller+10h]+C0h`, and, for the bomb-load factor, the platform counts
`+484h`/`+488h` plus the kind-`2Ah` count, `class+15Ch`, `plane+BC8h` and `class+608h`. The two
globals need nothing — they are zero and `c` collapses to zero with them.

`dyn+C0h` is the one remaining input a host cannot supply honestly. Defaulting it to zero gives
`v = 0`, hence `result = a`, hence a **well-formed** `R` that is simply wrong whenever the dynamics
term is non-zero — the same legal-looking failure as before, so the axis should still refuse until
that field is read.

# `dyn+C0h` is runtime-only (packet `cc7_dyn_c0`)

Addresses: `007D8470` `BSP_PlaneDynamics_IntegrateStep` (write at `007D902F`), `007DB1F0` (write at
`007DB2A4`), `007DB680` `BSP_PlaneFlight_CoreLaw` (write at `007DC6C5`), `007D9AFC` (the read).

Ghidra read-only. **Exported / read only** — no C++ behaviour was added; the header comment on the
existing parameter was updated. The build and `reconstructed_math` still pass. No new tests.

## The answer

**It cannot be supplied from authored data.** `dyn+C0h` is a per-tick scalar that the plane's own
physics pipeline maintains, and it has three producers, all of them runtime:

| site | function | what it writes |
| --- | --- | --- |
| `007DB2A4` | `FUN_007DB1F0` | `FSTP [EAX+0C0h]` with `[EDI+2Ch]`, alongside copying `[EDI+30h/34h/38h]` into a separate 3-float destination — so a `{scalar, vec3}` group is being copied out of a source object |
| `007DC6C5` | `BSP_PlaneFlight_CoreLaw` | `MOVSS [ECX+0C0h],XMM0`, where a predicate `[eax+72Ch]->vtable[38h]()` answering **true** skips the store entirely (`007DC6BD JNE`), answering false **zeroes** it (`007DC6BF XORPS`), and an earlier `JE` path stores a computed `XMM0` |
| `007D902F` | `BSP_PlaneDynamics_IntegrateStep`, its tail | a **decay with a floor**: `007D8FFE` loads the previous value, `007D9006 COMISS` compares it, `007D900F JBE` returns **without writing** when it is not above the comparand, and otherwise a subtracted-and-compared value is stored |

All three resolve the base the same way — `MOV reg,[ESI+10h]` then `[reg+0C0h]` — which confirms
the field is on the block `controller+10h` points at, the same dereference `007D9AF9` does.

`include/bsp/plane_flight.hpp` models that block only as far as `dyn+04h`..`+6Ch`, and its own
comment says "the unread tail integrates". `dyn+C0h` is written **by that unread tail**, so this is
not a gap in the field's identification so much as the boundary of what this repo has read of the
integrator.

## What I did not establish

* **What the scalar means.** It is normalised-ish — the yaw law uses it as `v = dyn+C0h * 0.6`
  against a floor `a ∈ [0, 0.25]` and a cap of `1`, so anything above about `1.67` saturates — and
  it builds from a source object, is zeroed or re-set by a predicate in the core law, and decays in
  the integrator. That is consistent with several readings and I did not pick one.
* **The comparand at `007D9006`** (`XMM0` at the decay), and the source object `[EDI+2Ch]` at
  `007DB2A4`, and the `+72Ch` predicate at `007DC6B6`.
* The other reads of the same displacement in the plane code (`007D7A93`, `007D83A2`, `007D88CD`,
  `007D8ABB`, `007DEF9F`) were not attributed to an object; only the three writes above were.

## Wiring contract, final for this axis

**The yaw axis stays refused, for a named structural reason rather than an unknown one.** Every
other input to `plan_yaw_0099e81a` is now either authored or derived from authored data:

```
heading error   unit+C6Ch (vtable[50h]) and task+2C0h          available
q               unit+340h                                      available
deadband/step   tuning+34h/38h/3Ch, class+1ACh/1C8h            available
base gain       tuning+7Ch/80h                                 available
turn term       class+1ACh/1B0h/1B8h/1D8h, the attitude triple available
R               unit+908h, forward speed, class+184h,
                the bomb-load factor                           available
                dyn+C0h                                        RUNTIME ONLY
```

A host that runs the plane flight integrator would have `dyn+C0h` for free. A host that does not
cannot obtain it at all, and must refuse the axis — **not** default it to zero, which yields
`v = 0`, `result = a`, and a well-formed `R` that is wrong whenever the plane's dynamics term is
non-zero.

## Caveat carried forward

The scans behind this packet and the previous one cover `battlestationspacific.exe` only. This
installation carries mod artefacts, so a loaded module writing `00F8731C`/`00F87320`, or this
field, is not excluded.

## Still open, unchanged

The pitch demand's bank-target arm, the speed-hold trigger `XMM0` at `0099D8C6`, and the producers
of `task+2B4h`/`+2C0h`/`+2D8h`.
