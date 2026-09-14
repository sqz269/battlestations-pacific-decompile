# The pilot bot's planner inputs and where its task comes from (packet `cc7_pilot_bot_plan_controls`)

Addresses: `0099D300`, `0099ACD0`, `0099A170`, `0099A4C0`, `0099BE30`, `007C18B0`, `0077D600`,
`008358D0`, `0071E6C0`, `00720CD0`, `007EEC50`, `0071BE40`, `0071EB60`, `00521EA0`,
`00D09EC0` (vtable), `00D09F20` / `00D09FB8` (the two `+60h` slots).

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
   `0077D600 BSP_Entity_IssueCommand`, whose callers are nine Lua mission-script bindings plus a
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

`0077D600 BSP_Entity_IssueCommand`'s caller set is nine Lua mission-script bindings plus four native
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

Whether the host can run these scripts at all once the entity attach exists - the mission Lua state,
`global_script_folders` and the entry-point calls are already concrete, but nothing has exercised an
order path end to end. And the group-AI question is still open: `docs/AI_PLANNERS.md`'s planners
write 8-byte objects with their own vtables into `group+564Ch`, a different family from the entity
commands the director holds, and no planner address appears in `0077D600`'s caller set. How a group
order reaches a member unit is unknown.
