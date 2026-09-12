# The gun-side bot ticks

Addresses: `008FFA20`, `008FFF20`, `00902920`, `009030C0`, `006DF520`, `00959C20`,
`0072BBD0`, `0072C6A0`, `008FBC80`, `008FBDC0`, `008FBEC0`, `008FBEF0`, `008FF040`,
`008FF310`, `006DF1F0`, `008FE740`, `008FE9F0`, `008FEFD0`, `008FF260`, `0072BE40`,
`008FBCE0`, `006DEFF0`, `006DF170`, `0072BD10`, `0072BD30`, `0072BD50`, `0072BC80`,
`008FEA40`, `008FEAC0`, `006DF260`, `00727E70`, `00727F10`, `00728000`, `0072D2C0`,
`00901C20`, `00955630`, `006DEE40`, `00954210`, `0085AB50`, `00922E90`, `00927F10`.
Vtables `00CFDF88`, `00CFDFA0`, `00CFDFE8`, `00CFE000`, `00D18088`, `00D180A0`,
`00D18124`, `00D18140`, `00D18220`, `00D18238`, `00D18294`, `00D182B0`.

`docs/BOT_FIRE_TARGET.md` established which routines turn a chosen fire target into gun
angles and left five of them partial. This packet reads the five bodies, the class they
belong to, how one is attached to a gun, and what the gun consumes afterwards.
`docs/GAME_EXECUTABLE.md` milestone 2n names this as the boundary its AI engagement stops
at; the contracts in section 8 are what a director stand-in has to fill.

## 1. What drives a gun bot

`docs/FIXED_STEP_JOB_WAVES.md` describes wave 2: `00875B90` calls `008759B0(element,
0.05f)`, which walks the element's sub-list at `element+1Ch` and calls
`sub->vtable[+0Ch](0.05f)` on every enabled sub-node. **A gun bot is one of those
sub-nodes.** That document records its `vtable[+0Ch]` callee as uncovered; these five
routines are it, and the `dt` every one of them receives is the fixed step `0.05f`.

The sub-node fields `008759B0` uses land on the bot exactly as the base constructor
`0072BBD0` writes them: `+8h` prev, `+0Ch` next, `+10h` expired, `+11h` enabled (set to
`1`). The element the bots hang off is the gun's tick node at `gun+310h`, the same
sub-object `0085AD80` runs on (`docs/GUN_AIMING.md` section "The second vtable at +310h").

## 2. The class family

Every bot derives from the class `0072BBD0` constructs. That constructor installs two
vtables: the primary at `bot+0h` (base `00CFDFA0`) and a second base at `bot+1Ch` (base
`00CFDF88`). The second base is an entity observer: its slot `+8h` `0072BCE0` resolves
`bot+38h` and, when the destroyed entity is the bot's own fire target, calls
`bot->vtable[30h]()` to clear it.

| Class name | Ctor | Primary vtable | Second base | Size | Class descriptor | Tick |
| --- | --- | --- | --- | --- | --- | --- |
| - (abstract) | `0072BBD0` | `00CFDFA0` | `00CFDF88` | - | argument | `0071C490`, a stub |
| `AAGunnerBot` | `008FE740` | `00D180A0` | `00D18088` | `74h` | `[00E19998]` | `00902920` |
| `TailGunnerBot` | `008FE9F0` | `00D18140` | `00D18124` | `94h` | `[00E199A0]` | `008FFA20` |
| `AAFlakBot` | `008FEFD0` | `00D18238` | `00D18220` | `6Ch` | `[00E1999C]` | `009030C0` |
| `TorpedoBot` | `008FF260` | `00D182B0` | `00D18294` | `70h` | `[00E1998C]` | `008FFF20` |
| `ArtilleryGunnerBot` | `0072BE40` | `00CFE000` | `00CFDFE8` | `B8h` | `[00E19990]` | `006DF520` |
| `DepthChargeBot` | `008FF4A0` | `00D18338` | `00D1831C` | `64h` | `[00E19988]` | `008FC080` |

Sizes are the `operator_new` arguments in the factory `0072C6A0`. There is no class-id
byte and no `IsKindOf` on these objects; a bot is identified only by its vtable, so the
"class test" a caller has is the gun slot it was stored in.

### The class names are recovered strings

`load_robot_config_00901610` builds one class descriptor per name with `00900AF0(ECX =
name)` and stores each result in the global its constructor reads. `ECX` is loaded one
instruction before each call and the store lands after the next, so the pairing is exact:

| Name | String | Global | Store | Tick |
| --- | --- | --- | --- | --- |
| `PilotBot` | `00D17D44` | `[00F8A30C]` | `009019EB` | outside this packet |
| `TailGunnerBot` | `00D17D5C` | `[00E199A0]` | `009019FA` | `008FFA20` |
| `AAFlakBot` | `00D17D50` | `[00E1999C]` | `00901A09` | `009030C0` |
| `AAGunnerBot` | `00D17D6C` | `[00E19998]` | `00901A18` | `00902920` |
| `ArtillerySubDirectorBot` | `00D17D78` | `[00E19994]` | `00901A27` | outside this packet |
| `ArtilleryGunnerBot` | `00D17D90` | `[00E19990]` | `00901A36` | `006DF520` |
| `TorpedoBot` | `00D17DA4` | `[00E1998C]` | `00901A45` | `008FFF20` |
| `DepthChargeBot` | `00D17DB0` | `[00E19988]` | `00901A53` | `008FC080` |

`TorpedoBot` landing on the weapon sub-type 7 tick, which `docs/GUN_CLASS_FAMILY.md`
independently calls the torpedo sub-type, is the cross-check that the pairing is right.
The ledger already carries `read_<name>_parameters_*` and `validate_<name>_parameters_*`
routines for seven of the eight names; they are the descriptor's own property readers and
were not read here.

### Primary vtable slots

Read from the base `00CFDFA0` and from every override the five classes install.

| Slot | Base | Role | Evidence |
| --- | --- | --- | --- |
| `+00h` | `0072BE20` | scalar deleting destructor | `0072C6A0` calls `vtable[0](1)` at `0072C7A9` |
| `+04h` | `008FBC80` | `Attach(tickNode)` | `0072C6A0` calls it with `LEA EAX,[ESI+0x310]` (`0072C74C`) |
| `+08h` | `0071C480` | stub | - |
| `+0Ch` | `0071C490` | **the AI tick, `(float dt)`** | `008759B0`, section 1 |
| `+10h` | `0071C4A0` | stub | - |
| `+18h` | `0072BC80` | `SetSkillIndex(int)`, `this+34h = arg` | `00727E70` fans it over all six slots |
| `+30h` | `0072BD50` | `ClearFireTarget()` | `00728000` fans it over all six slots |
| `+38h` | `006DF170` | `SetFireTarget(Entity*)` | `00727F10` fans it over all six slots |
| `+40h` | `0072BD10` | `HasFireTarget()`: `bot+39h != 0 \|\| bot+38h != 0` | body read at `0072BD10` |
| `+44h` | `0072BD30` | `GetFireTargetEntity()`: `bot+38h == 0 ? null : 00521EA0(bot+38h)` | body read |
| `+48h`, `+4Ch` | class-specific | the property visitors | `008FEAC0`, `006DF260` |

`bot+38h` is a command-target record, the same kind `00521EA0`
`BSP_CommandTarget_ResolveObject` reads elsewhere. Slot `+38h` is overridden only by
`008FE9F0`'s class, whose `008FEA40` tail-calls the base `006DF170` and then forces
`bot+6Ch = -1.0f` (`00D7A260`) so the next tick recomputes its cached angles immediately.

## 3. How a bot reaches a gun and a unit

`0072C6A0` is the only constructor caller; it runs from `BSP_Gun_SetupFromDescriptor`
`0072E6D0`, `006ECBE0` and `007BD1A0`. It reads the weapon descriptor sub-type at
`[gun+3F4h]+80h` - the same field `00730160` `BSP_Gun_Fire` selects on
(`docs/GUN_CLASS_FAMILY.md`) - and fills six bot slots on the gun:

| Gun slot | Sub-type gate | Extra gate | Class | Site |
| --- | --- | --- | --- | --- |
| `+390h` | `== 1` | gun `IsKindOf(22h)`; `00922E90(gun, 0Fh) == 0` | `AAGunnerBot` `008FE740` / `00902920` | `0072C736` |
| `+390h` | `== 1` | gun `IsKindOf(22h)`; `00922E90(gun, 0Fh) != 0` | `TailGunnerBot` `008FE9F0` / `008FFA20` | `0072C713` |
| `+394h` | `== 5` or `== 6` | destroys `+390h`'s bot afterwards | `AAFlakBot` `008FEFD0` / `009030C0` | `0072C79E` |
| `+398h` | `2`, `3`, `4` or `6` | - | `ArtilleryGunnerBot` `0072BE40` / `006DF520` | `0072C81D` |
| `+39Ch` | `== 7` | - | `TorpedoBot` `008FF260` / `008FFF20` | `0072C870` |
| `+3A0h` | `== 8` | - | `DepthChargeBot` `008FF4A0` / `008FC080`, not reconstructed here | `0072C8CB` |
| `+3A4h` | `== 9` | - | `ArtilleryGunnerBot` `0072BE40` / `006DF520` | `0072C91D` |

The two sub-type 1 classes are an anti-aircraft gunner and a tail gunner, and
`00922E90(gun, 0Fh)` is what separates them, so entity kind `0Fh` is the owner an aircraft
tail gun sits under.

`00922E90(this, kind)` walks the entity parent chain - `this+3Ch`, then `+3Ch` of each
parent - and returns the first entry that answers `IsKindOf(kind)`. So the split between
`00902920` and `008FFA20` is "does this gun sit under an owner of kind `0Fh`".

The attach, primary slot `+4h`, is called with `gun+310h`. The base `008FBC80`:

| Site | Rule |
| --- | --- |
| `008FBC8B` | `00876020(gun+310h, bot)` - link the bot into the tick node's sub-list |
| `008FBC90`, `008FBC95` | `bot+50h = [gun+310h + 28h]`, the node's payload, which for a gun's node is the gun itself (`00875890` stores its `param_2` at node`+28h`, and `BSP_TickableGameEntity_Construct` `00929E85` passes the entity) |
| `008FBC9F` | `bot+50h -> IsKindOf(5)`; when true `bot->vtable[18h]([bot+50h]->vtable[12Ch]())`, the owner's `Skill` (`docs/AIR_OPERATIONS.md` names slot `12Ch` `Skill`) |
| `008FBCC8` | otherwise `bot->vtable[18h](1)` |

Each class then caches `bot+50h` in its own gun field, which is the direct proof that
`bot+50h` is the gun: every tick dereferences that cache at gun offsets (`+3F0h`,
`+3F4h`, `+3F8h`, `+3CCh`, `+480h`, `+484h`).

| Class | Attach override | Gun cache | Other work |
| --- | --- | --- | --- |
| `008FFA20` | `008FBEC0` | `+68h` | `+58h = 0`; `+70h = +74h = 0.0f` |
| `00902920` | `008FBDC0` | `+5Ch` | `+60h`..`+70h` zeroed |
| `009030C0` | `008FF040` | `+68h` | (slot `+4h` of `00D18238`) |
| `008FFF20` | `008FF310` | `+58h` | (slot `+4h` of `00D182B0`) |
| `006DF520` | `006DF1F0` | `+58h` | `+84h..+8Ch = (0,0,0)`; `+74h = 0`; `+78h = 1.0f`; `006DEFF0(bot)` |

The unit is **not** stored on the bot. `008FFF20` finds it at `008FFFCE` by walking the
same `+3Ch` parent chain from `bot+50h` to the first `IsKindOf(5)`.

## 4. The fields, from the image's own property names

Slots `+48h`/`+4Ch` are the property visitors; each pushes a literal name, a type code and
the field address. Type codes seen: `2` float, `3` byte, `5` float3, `6` float2.

`008FEAC0`, the `008FFA20` class (`94h` bytes):

| Offset | Name | Type |
| --- | --- | --- |
| `+58h` | `actFireState` | byte |
| `+59h` | `nextFireState` | byte |
| `+5Ch` | `fireDelay` | float |
| `+60h` | `horzAngleError` | float |
| `+64h` | `vertAngleError` | float |
| `+6Ch` | `aimTime` | float |
| `+70h` | `tr` | float2 - the cached aim pair |
| `+84h` | `angleError` | float |
| `+88h` | `aimPeriodMin` | float |
| `+8Ch` | `aimPeriodMax` | float |
| `+90h` | `shootRange` | float |

`006DF260`, the `006DF520` class (`B8h` bytes):

| Offset | Name | Type |
| --- | --- | --- |
| `+5Ch` | `Error` | float2 - the live horizontal and vertical error |
| `+74h` | `calcErrTick` | float |
| `+7Ch` | `delayedFire` | byte |
| `+80h` | `fireDelayTime` | float |
| `+84h` | `ErrorOffset` | float3 |

Both visitors end by copying `bot+50h` into the class's gun cache, which is how a loaded
bot re-acquires its gun. The other three classes' name blocks sit in the same `.rdata`
run (`00D180F0`..`00D18218`, `00D18288`) and were not attributed to offsets here:
**contract: unread** for `00902920`, `009030C0` and `008FFF20` field names.

Base fields written by `0072BBD0`: `+8h`/`+0Ch`/`+10h`/`+11h` the sub-node links and
flags, `+18h = 2`, `+1Ch` the observer vtable, `+30h` the class descriptor pointer,
`+34h = 1` the skill index, `+38h` the fire-target command record, `+40h`..`+48h` a float3
from `00F87574`, `+50h = 0` the gun, `+54h = [00CF4888]` an idle timer.

### The skill records

Each class descriptor holds an array of records indexed by `bot+34h`. Offsets are stated
against the descriptor pointer, as the code reads them, because no producer for the array
base was read.

| Class | Stride | Fields read | Site |
| --- | --- | --- | --- |
| `008FFA20` | `24h` | `+0Ch` -> `angleError`, `+10h` -> `aimPeriodMin`, `+14h` -> `aimPeriodMax`, `+18h` -> `shootRange`; `+20h`, `+24h`, `+28h`, `+2Ch` -> the four lead scalars | `008FBEF0` (the `SetSkillIndex` override), `008FFB87` |
| `00902920` | `10h` | `+0Ch` -> an error weight, `+14h` -> a degree spread | `00902C2x`, `00902D3x` |
| `006DF520` | `1Ch` | `+10h` -> an exponent scale, `+14h` -> the lead re-roll period, `+18h`, `+1Ch`, `+20h`, `+24h` -> the four lead scalars | `006DEFF0`, `006DF6xx` |
| `008FFF20` | `14h` | `+0Ch`, `+10h` -> a random spread range in degrees | `0090091x` |

## 5. The shared prologue

Four of the five ticks open with the same three steps.

| Step | Sites (`008FFA20`, `00902920`, `009030C0`, `008FFF20`) | Rule |
| --- | --- | --- |
| 1 | `008FFA2D`, `0090292D`, `009030CA`, `008FFF4B` | `target = 00521EA0(bot+38h)` |
| 2 | `008FFA3B`..`008FFA5F`, `00902937`..`0090295D`, `009030D2`..`009030FA`, `008FFF55`..`008FFF7B` | drop the target unless: it is alive (`target+5Dh == 0`), `bot+50h != 0`, the gun's parent `[gun+3Ch]` exists and is alive, and `00803510([gun+54h], [target+54h])` - the side relation `docs/SHIP_AI_STATES.md` records at `00816359` - is true. On failure `bot->vtable[38h](0)` |
| 3 | `008FFA85`, `00902985`, `00903122`, `008FFFA2` | `008FBCE0(bot)(dt, gun)` |
| 4 | `008FFA99`, `00902999`, `00903136`, `0090003B` | the side gate: run only when `[gun+1ACh] == 8` or `00927F10([gun+1ACh])`. `00927F10` reads `[[00E188A8] + 18CCh + side*4] + 9`; `docs/HUD_CENTRAL_UPDATES.md` uses the identical idiom on a unit |

`006DF520` differs: its step 2 is a single `008FB890` test (`006DF541`) and its step 4 is
the same side gate at `006DF577`, inverted into an early return.

`008FBCE0(bot)(float dt, Gun* gun)` is the idle timer. While `bot+38h` and `bot+39h` are
both zero it advances `bot+54h` by `dt`; once `bot+54h` passes `[[bot+30h]+4h]` it is
pinned at `FLT_MAX` (`00D7A248`) and, if the gun answers `IsKindOf(22h)`, the gun is sent
back to its rest angles with `0085AD00` `BSP_TurningGun_AimToRestAngles`. When the side
gate is false the timer is instead reset to `[[bot+30h]+4h] * 0.25` (`00D7A348`). With a
target present it is held at `0`.

## 6. The five aim-and-fire rules

### 6.1 `008FFA20`, the turret bot - complete

`__thiscall(bot)(float dt)`, body `008FFA20..008FFF1F`. Slots `+390h` on a gun of sub-type
`1` under an owner of kind `0Fh`.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `008FFAC9` | when `[[gun+3F0h]->vtable[114h]()]+221h` is clear, `vtable[38h](0)` - the owning unit's weapon director must have its artillery flag set (`docs/WEAPON_DIRECTOR.md` gives `+221h` as the sub-kind 4 flag) |
| 2 | `008FFAE9`, `008FFAFC` | return unless the gun cache and `vtable[44h]()` are both non-null |
| 3 | `008FFB1A`, `008FFB30` | `bot+78h..+80h = TransformAffinePoint(target+0CCh)`, the target pose origin |
| 4 | `008FFB35`, `008FFB62` | `bot+6Ch -= dt`; recompute only when the result is negative |
| 5 | `008FFB87` | `lead = descriptor + skill*24h`, four floats at `+20h`, `+24h`, `+28h`, `+2Ch` |
| 6 | `008FFBC6` | `target->vtable[100h](out, (0.6,0.6,0.6), 00F87574, lead0..lead3)` - the **target entity's** predictor, not the gun's; result stored at `bot+78h`..`+80h` |
| 7 | `008FFBF0` | the target pose origin is re-sampled over the same three floats |
| 8 | `008FFBFD` | `muzzle = [gun+3CCh]`, refreshed; the aim vector is `bot+78h - muzzle+120h..128h` |
| 9 | `008FFCAF`, `008FFCBD` | `00414E10` the gun's derived affine inverse, then `008FDAF0` -> `(bot+70h, bot+74h)` |
| 10 | `008FFCD9`, `008FFD05` | one `00BD2F10(0, bot+84h)` draw per angle, added as `draw * pi/180` (`00CE3D28` over `00CE3D20`) |
| 11 | `008FFD37` | `bot+6Ch = 00BD2F10(bot+88h, bot+8Ch)`, the next aim period |
| 12 | `008FFD52` | `0085ABA0(gun, bot+70h, bot+74h)` every tick, cached pair or not |
| 13 | `008FFD6E`, `008FFDB9` | `distance = 0042B2F0(targetOrigin - muzzle+120h)` |
| 14 | `008FFE16`, `008FFE43` (not firing) and `008FFEA5`, `008FFED1` (firing) | the hysteresis of `docs/BOT_FIRE_TARGET.md` section 3, unchanged; the four `00438B10` calls are the angle halves |
| 15 | `008FFF12` | `008FEF40(bot)(request, dt)`, the debounce, which drives `gun->vtable[1E8h]` |

### 6.2 `009030C0`, the bomb and depth-charge bot - complete

Body `009030C0..0090341D`. Slot `+394h`, sub-types `5` and `6`.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `00903154` | `piVar8 = bot+50h` unless `gun->IsKindOf(5)` fails, in which case null |
| 2 | `00903163` | branch on `[[gun+3F4h]+95h]` and that null test |
| 3 | `009031CF` / `00903219` | `00901C20(shooterPos, target, projSpeed, shooterVel, &aimPoint, &distance)`, `RET 10h`. The muzzle node is `[[gun+3F8h]+34h]`, the speed its `+50h`; the second arm passes `00F87574` (all zeros) as the shooter velocity |
| 4 | `0090326D`, `0090327B` | `00414E10` then `008FDAF0` -> `(h, v)` |
| 5 | `00903288`..`009032C4` | `h += bot+58h`, `v += bot+5Ch` (the class's own bias pair) |
| 6 | `009032C6`..`009032E9` | when `v < -0.02` (`00D7A320`): `v = v - (0.02 + v) * 0.5` (`00D7A2F8` double, `00D7A280`), i.e. `0.5v - 0.01`. When the target answers `IsKindOf(0Fh)` instead, `v = 0` |
| 7 | `00903302` | `0085ABA0(gun, h, v)` |
| 8 | `0090330D`, `0090331B` | `if (bot+64h < [gun+474h]) 008FDBE0(bot)` |
| 9 | `0090332C`..`00903344` | fire only while `[muzzle+58h] < distance < [muzzle+60h]` |
| 10 | `0090335E`, `0090339A` | and `\|00438B10(gun+480h, h)\| < 0.0174533` and `\|00438B10(gun+484h, v)\| < 0.0174533` (`00CE3984`, one degree) |
| 11 | `009033D2` | and bit `0` of `[gun+3F0h]+634h` is clear |
| 12 | `009033F6` | `target->vtable[5Ch](2)` - the result is discarded, `EAX` is overwritten at `009033FD` |
| 13 | `0090340A` | tail `JMP gun->vtable[1E8h](fire)` |

### 6.3 `006DF520`, the muzzle-solution bot - complete

Body `006DF520..006DFC6C`. Slots `+398h` (sub-types `2`, `3`, `4`, `6`) and `+3A4h`
(sub-type `9`).

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `006DF585`, `006DF596` | run only while the gun cache is set and `vtable[40h]()` says a target exists |
| 2 | `006DF5D4`, `006DF5E9` | `bot+74h -= dt`; when negative, `bot+74h = bot+78h = 00BD2F10(3.0, 8.0)` (`00CE3854`, `00CE3918`) and `006DEFF0(bot)` re-rolls the error envelope with `2^(k*rand(0,1))`, `k` from `descriptor + 10h + skill*1Ch` |
| 3 | `006DF623`, `006DF651` | `bot+5Ch = 00419010 InterpolateClamped(0, bot+6Ch, bot+78h, bot+64h, bot+74h)`; `bot+60h` the same over `bot+70h` -> `bot+68h`. The errors decay as the period runs out |
| 4 | `006DF980` (no target), `006DF694`, `006DF6B3`, `006DF6D2` | with no target: `0059BD20`, zero `ErrorOffset`, `bot+B4h = -1.0f`. With one: `0042AC60 BSP_Math_StepTowards` on `bot+84h`..`+8Ch` at `dt*30` (`00CE7630`), `bot+B4h -= dt`, and on expiry a fresh `target->vtable[100h]` lead into `bot+A8h`..`+B0h` using `descriptor + 18h..24h + skill*1Ch` |
| 5 | `006DF82E`, `006DF861` | aim point = target origin + `ErrorOffset`; `distance = 0042B2F0(aimPoint - gun+0FCh)` |
| 6 | `006DF8BF`, `006DFA47` | a gravity drop pre-estimate: `s = distance * 9.81 (00CF9058) / v^2`; `pitch = asin(s) * 0.5` clamped to `pi/4` (`00CEB5A8`); the point is pushed out along the target velocity by `distance / (v * cos(pitch) * [muzzle+5Ch])` |
| 7 | `006DFA60`..`006DFAB0` | gate on `s <= 1.0`, `bot+50h != 0`, `[gun+3Ch] != 0` and `[gun+3Ch]->IsKindOf(5)` |
| 8 | `006DFAD4` | `00955630(unit, aimPoint, muzzlePos, muzzleSpeed, &vert, &horz)` -> `bool`; it solves the flat gravity root, converts to a world direction and re-extracts the pair in the mount's local frame |
| 9 | `006DFAE9` | `gun+408h..+410h = the aim point` |
| 10 | `006DFB1C`, `006DFB36` | `horz = 00438AA0(horz, bot+5Ch)`, `vert = 00438AA0(vert, bot+60h)` - the per-bot errors, wrapped |
| 11 | `006DFB54` | `0085ABA0(gun, horz, vert)` |
| 12 | `006DFB8C`, `006DFBB6` | arm the shot when the solver succeeded, `0085ABA0` succeeded, `delayedFire` is clear, and `006DEE40` says the gun is inside `0.0017453` rad (`00CF9054`, one tenth of a degree) of both commanded angles |
| 13 | `006DFBD6` | `fireDelayTime = 00BD2F10(0, 0.1)` (`00D7A2F0`); `delayedFire = 1` |
| 14 | `006DFBF0`..`006DFC30` | the inhibit bit of `[gun+3F0h]+634h` is bit `3` when `[[gun+3F8h]+34h]+8 == 0Bh`, bit `2` when `== 0Ah`, bit `1` otherwise |
| 15 | `006DFC40`..`006DFC6C` | while `delayedFire` and not inhibited, `fireDelayTime -= dt`; on expiry `gun->vtable[1F0h]()` and `delayedFire = 0`. This class does **not** use `008FEF40` |

`006DEE40` is `__stdcall(float a, float b, float tol) -> int`, `RET 0Ch` at `006DEE7B`:
`\|00438B10(a, b)\| <= tol`, the absolute value taken with `AND EAX, 7FFFFFFFh`.

### 6.4 `00902920`, the swinging-error bot - partial

Body `00902920..009030B2`. Slot `+390h`, sub-type `1`, no owner of kind `0Fh`.

The aim point is the same `00901C20` solution `009030C0` uses (`009029xx`), converted by
`008FDAF0`. What is different is the error model: `bot+64h`/`+68h` are a live error pair
and `bot+6Ch`/`+70h` their rates, and `bot+60h` is the remaining time.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `00902A74`, `00902ACB`, `00902AD9` | `00901C20`, then `00414E10` and `008FDAF0` give the base pair; `bot+60h -= dt` and on expiry a new error target is picked |
| 2 | `00902B38`, `00902B5C` | `span = 00419010 InterpolateClamped(0, 1.0, 6.0, 0.2, skill)` (`00CE6630`, `00CE54A0`); `bot+60h = 00BD2F10(3.0, 8.0) * span` |
| 3 | `00902C86`, `00902CAC`, `00902CC7`, `00902CD0` | when `[descriptor + 0Ch + skill*10h] > 0`, a second `vtable[100h]` sample with lead scalars `(-1.0, 1.0, 1.0, 1.0)` and direction `(0.8, 0.5, 0.8)` gives an angular offset that is scaled by that weight |
| 4 | `00902CF0` | plus `00BD2F10(0, [descriptor + 14h + skill*10h]) * pi/180` |
| 5 | `00902D35`, `00902D3E`, `00902D4F` | when the target answers `IsKindOf(6)` the error is divided by game setting `+754h`, or `+750h` when `00521E70` accepts the slot |
| 6 | `00902D77`, `00902D95` | two `00BD2F90` draws split the error over the axes; the rates become `(newError - current) / remainingTime` |
| 7 | `00902E6D`, `00902EF7` | the errors integrate by `rate * dt`; either axis leaving `25 / distance` (`00CE3880`) is clamped by `00415690`, the remaining time reset to `1.0` and the rates halved and negated (`00CEC9E0` = `-0.5`) |
| 8 | `00902F3E`, `00902F58` | `h = 00438AA0(h, bot+64h)`, `v = 00438AA0(v, bot+68h)`; a negative `v` is halved (`00D7A280`) |
| 9 | `00902FB0` | `0085ABA0(gun, h, v)` |
| 10 | `00902FFC`, `0090302A` | the fire test needs the aim accepted, `distance < [muzzle+60h] * 0.9` (`00D7A390`) and both `00438B10` deltas small |
| 11 | `009030A8` | tail `JMP gun->vtable[1E8h](fire)` |

Coverage: **partial**. The exact composition of the fire byte between `00903010` and
`009030A8` was read from pseudocode only; the listing was not filtered for it.

### 6.5 `008FFF20`, the torpedo bot - partial

Body `008FFF20..0090099B`. Slot `+39Ch`, sub-type `7`.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `008FFFE9`, `00900018` | extra target drop: when the target answers `IsKindOf(8)` and the unit found on the parent chain does not, and `00852820` is false, `vtable[38h](0)` |
| 2 | `008FFFCE` | walk `bot+50h`'s `+3Ch` chain to the first `IsKindOf(5)`; return if `unit+184h` is set |
| 3 | `009000D8` | when the target answers `IsKindOf(6)`, `008FE140` |
| 4 | `0090003B`..`00900100` | `bot+5Ch -= dt`; return while it is still positive, then reset it to `0.2f` (`00CE54A0`) |
| 5 | `00900110` | return unless `bot+64h == 0` or `008527E0` accepts |
| 6 | `00900120`, `009001A8`, `009001E6` | `distance = 0042B2F0(...)`; over `00415510 BSP_Math_MinFloatByRef` -> `gun->vtable[1E8h](0)` and return |
| 7 | `0090025E` | `008FBB00(runSpeed, targetVelocity, out)` solves the torpedo intercept; `runSpeed` is `[[gun+3F8h]+34h]+0E4h` |
| 8 | `00900328`, `0090032F`, `0090033F`, `00900348`, `0090035A`, `00900380` | `0042B260` normalise, `00414E10` derived affine inverse, `0042D0D0` transform, `0042B260` again, `00521370` to a pair, negate, then `0085AB50(gun, h, 00E0B588 = pi/4)` |
| 9 | `009003C6` | `bot+60h = the filtered value`; when it equals `FLT_MAX` (`00D7A278`) the whole shot is abandoned |
| 10 | `009003DD` | `0085ABA0(gun, bot+60h, 0.0f)` - horizontal only |
| 11 | `009003F8`, `0090044A` | fire only while `\|00438B10(bot+60h, gun+480h)\| < 1 degree` and `gun->vtable[1D0h](1)` accepts |
| 12 | `0090058A`..`009007C7` | a friendly-fire scan: every kind-`6` entity within `[00D09FE8]` is projected forward by `1000` (`00CE47A0`) seconds of its own velocity and tested against the torpedo run with `004F3730` at `009006EE`; a predicted crossing inside `[00CE3CA8]/1000 + [00CE4D70]` aborts the shot |
| 13 | `00900830`, `0090083E` | a final aim jitter of `+/- 00BD2F10(descriptor+0Ch, descriptor+10h)` degrees, sign from `00BD2FC0 BSP_RandomThreads_NextU32` |
| 14 | `00900964` | bit `2` of `[gun+3F0h]+634h` aborts |
| 15 | `009008F3`, `00900912`, `00900927`, `0090093B`, `00900951` | the shot is a command object: `00BF681B operator_new(24h)`, `0072C970(gun, unit)`, `007311B0(heading, value, unit)`, `0072AC20(gun, object)`, then `gun->vtable[1F0h]()` and `00951FC0` |
| 16 | `0090097B` | every abort path converges on `gun->vtable[1E8h](0)` |

Coverage: **partial**. Steps 7 and 12 name their callees from the call shape only;
`008FBB00`, `004F3730`, `008527E0`, `008FE140` and `00952xxx` are **contract: unread**.

## 7. `00959C20`, the commanded aim path

Not a bot. `bool __thiscall(unit)(CmdMsg* msg)`, `RET 4` at `0095A43E`, body
`00959C20..0095A5BF`, reached from `BSP_Unit_HandleMessage`'s byte jump table at
`0095AC16` through the call at `0095ACE8`. Its own five-way jump table is at `0095A5C0`,
selected by `[msg+1Ch]`.

Message fields: `+1Ch` kind, `+20h`/`+24h`/`+28h` a float3, `+2Ch` and `+30h` two floats,
`+34h` the trigger byte, `+35h` an alternate trigger byte, `+36h` "has target", `+38h` a
target handle resolved by `00521E30`.

All four device arms walk the same list: head `[unit+48h]`, next `[device+44h]`, filter
`device->vtable[5Ch](20h)` and then `00954210(kind, device)`.

`00954210` is `__cdecl(int kind, Device* dev) -> bool`, `RET 8`, body
`00954210..009542AC`. It requires `dev != 0` and `00729F10(dev)`, then splits on the kind:
`1` and `2` accept `[dev+3F4h]+80h` in `{1, 5, 6}` and kind `2` additionally accepts
`005459E0(dev)`; kind `3` is `005459B0(dev)`; kind `4` is sub-type `7`; kind `5` is
`0080F750(dev)`.

| Arm | `0085ABA0` site | Angles |
| --- | --- | --- |
| kind 1/2, `00959C91` | `00959E01` | `00957BD0` builds a solution from the message float3 and its two floats, `00955830` splits it into a pair; gated by `007F60A0` on the platform from `005472D0(unit+5CCh, device+38Ch)` |
| kind 3, `00959F72` | `0095A0EF` | `004B4D80` turns `(msg+30h, msg+2Ch)` into a direction, `00957740` refines it, `00955630` solves the gravity arc against `[[device+3F8h]+34h]+50h` |
| kind 4, `0095A1CC` | `0095A28F` | `0085AB50(device, -00438B10(msg+2Ch, unit->vtable[50h]()), 00E0B588)` - a heading relative to the unit's own; `0085ABA0(device, that, 0.0f)`, skipped when it equals `FLT_MAX`. The `0095A1C4` site that `docs/BOT_FIRE_TARGET.md` left unread is the loop advance of this arm, not an aim call |
| kind 5, `0095A441` | `0095A4FC` | sub-type `9` devices only; `00957740` then `00955630` |

The trigger in every arm is `device->vtable[1E8h](msg+34h)` when the gun is inside the
commanded window and `(0)` when it is not. The windows are `00D1A8A0` (three degrees,
double) for kinds 1/2 and 5, `00D0C26C` (two degrees) for kind 3 and `00CEDF5C` (five
degrees) for kind 4.

## 8. What the gun consumes

Nothing a bot computes is returned to the caller: every tick returns `void` and writes
through the gun.

| What | Where | Written by | Read by |
| --- | --- | --- | --- |
| target angles | `gun+494h`, `gun+498h` | `0085ABA0` from every tick | `0085AD80` steps `gun+480h`/`+484h` toward them |
| aim-accepted mark | `gun+4A0h` | `0085ABA0` step 1, `-1.0f` | not read by this packet |
| the trigger | `gun+454h` | `gun->vtable[1E8h]` = `0072D2C0` | `docs/GUN_AIMING.md`'s fire path |
| the fire-start delay | `gun+478h` | `0072D2C0` on a rising edge, `00BD2F10(0, [00CE81A8])` | - |
| the aim point | `gun+408h`..`+410h` | `006DF520` step 9 and `00959C20` kind 3 | - |

`0072D2C0` `__thiscall(gun)(char wantFire)` forces `wantFire` to `0` when the gun has no
unit, the unit is dead (`[gun+3F0h]+5Dh`) or the gun is dead (`gun+5Dh`). On a change it
replicates opcode `0AFh` for `MRFSGun` (`IsKindOf(23h)`), stores the byte at `gun+454h`,
and either seeds `gun+478h` or calls `0072B4C0`.

`vtable[1F0h]`, the immediate fire `008FFF20` and `006DF520` use, is `006FDF60` on
`MRTGun` (`00CFBF58+1F0h`) and its descendants. The base gun vtable `00CFE0A8` ends at
`+1ECh`, so that slot exists only on the turning classes: **contract: unread** for
`006FDF60`'s body.

### What a director stand-in must provide

The gun-side ticks never read a weapon director field directly. The only director-derived
inputs are:

1. `[gun+3F0h]->vtable[114h]()` at `008FFAC9`, whose `+221h` byte (`docs/WEAPON_DIRECTOR.md`,
   the sub-kind 3 artillery flag is `+220h` and `+221h` the sub-kind 4 flag) gates the
   turret bot entirely.
2. the fire target, which arrives as `bot+38h` through `bot->vtable[38h](entity)`. The gun
   level fan-out is `00727F10`, `RET 8` at `00727FE9` and `00727FFC`, which calls
   `vtable[38h]` on all six bot slots, with `+398h` and `+394h` routed to `vtable[30h]`
   (clear) when the target answers `IsKindOf(0Fh)` or `IsKindOf(0Eh)`. Its only caller is
   `00864FE0`, the unit-side gunnery pass, which reads the director through
   `entity->vtable[114h]()`, `0071EBF0` and `00521EA0` at `0086549C`..`008654A3` and calls
   `00727F10` at `00865833`. `00864FE0` is **contract: partially read** - the two bot
   arms and the director lookup only.
3. the skill index, `unit->vtable[12Ch]()`, applied once at attach and re-appliable through
   `00727E70` -> `vtable[18h]`.
4. the side gate, `[gun+1ACh]`, and the player-control inhibit bits of `[gun+3F0h]+634h`.

### `director+3Ch` allowFire does not reach the gun through these bodies

`docs/GUN_PLATFORM_ARC.md` records the six `vtable[1E8h]` dispatch sites and infers that
the `allowFire` byte must run through the two of them this packet owns. It does not.

| Body | Evidence |
| --- | --- |
| `008FFF20` | the whole listing filtered for `0x114` has no hit, so the tick never fetches a director. Its four `[reg+3Ch]` reads are `008FFF62`, `008FFFDF`, `00900062` and `00900131`, all the entity parent chain `00922E90` walks, and the rest are `[ESP+3Ch]` stack slots |
| `00959C20` | the same filter has no hit either. Its two `+1xxh` unit reads are `[unit+1B4h]` at `00959DA4` and `[unit+1BCh]` at `0095A048`, both passed to `vtable[154h]`, not director fields |
| `00864FE0` | the unit-side pass that does fetch a director (`0086549C`) has no `[reg+3Ch]` read anywhere in its listing |

So the only weapon-director field any gun-side AI body reads is `director+221h`, at
`008FFAC9` in `008FFA20`. `director+3Ch` and `director+238h` are **not** read by any of the
five ticks, and the reader of `allowFire` is still **contract: unread**: a byte-pattern scan
of the image for the three plausible encodings of a byte load at displacement `3Ch`
(`80 ?? 3c 00`, `8A ?? 3c`, `0F B6 ?? 3c`) returns one genuine hit, the two-instruction
predicate at `009F6AD0` (`cmp byte [ecx+3Ch], 0; sete al; ret`), and a scan for the literal
dword `009F6AD0` finds no reference to it anywhere in the image. Every other hit decodes as
the first byte of a four-byte displacement.

### The two `vtable[1E8h]` sites with no Ghidra function

`docs/GUN_PLATFORM_ARC.md` left `006DF508` and `008FC22C` undecoded. Both are tail
dispatches of `gun->vtable[1E8h](0)` inside gun-bot routines Ghidra has no function for.

| Site | Containing routine | Body | What it is |
| --- | --- | --- | --- |
| `006DF508` | `006DF4C0` | `006DF4C0-006DF513` | `ArtilleryGunnerBot`'s override of primary slot `+38h` (`SetFireTarget`). It calls the base `006DF170` at `006DF4C8`, runs `0072D5B0(gun, 0, 0)` at `006DF4EC` when the side gate passes, and when `vtable[44h]()` then answers null it tail-jumps `gun->vtable[1E8h](0)` at `006DF50E`: clearing the target drops the fire request in the same call |
| `008FC22C` | `008FC080` | `008FC080-008FC3F2` | `DepthChargeBot`'s tick, primary vtable `00D18338` slot `+0Ch`. It has the same shape as the other five (`[bot+50h]+1ACh` side gate at `008FC116`, gun cache at `bot+58h`, `vtable[44h]` at `008FC13B`) and tail-jumps `gun->vtable[1E8h](0)` at `008FC232` on its no-shot path. Not reconstructed by this packet |

Both start after `int3` padding (`006DF4B3`-`006DF4BF` and `008FC07F`) and end at a
`RET 4`, so the extents are exact.

## 9. Corrections to earlier documents

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/BOT_FIRE_TARGET.md`: "a lead point from `gun->vtable[100h]`" | the predictor is a virtual of the **fire target entity**: `008FFAF5` fetches it with `bot->vtable[44h]()` and `008FFBC6` calls `[that+100h]` | `008FFB04`..`008FFBC6` in the listing |
| `docs/BOT_FIRE_TARGET.md`: `009030C0` "takes a different branch when `[[gun+3F4h]+95h]` is set and **the unit** answers `IsKindOf(5)`" | the object tested is `bot+50h`, the gun | `00903154 PUSH 5 / CALL EAX` with `ECX` from `[ESI+0x50]`, and `008FBC90` proving `bot+50h` is the node payload |
| `docs/BOT_FIRE_TARGET.md`: `00959C20` "the `0095A1C4` arm is unread" | `0095A1C4` is the loop advance of the kind-4 arm that begins at `0095A1CC`; the arm's aim call is `0095A28F` | the jump table at `0095A5C0` |
| `include/bsp/bot_fire_target.hpp`: `kGunBotOffGun = 0x68` | `+68h` is the gun cache of the `008FFA20` class only. `00902920` caches at `+5Ch`, `008FFF20` and `006DF520` at `+58h`; the shared field is `bot+50h` | the five attach overrides in section 3 |
| `docs/FIXED_STEP_JOB_WAVES.md`: "`008759B0`'s sub-list is covered as a rule but its `vtable[+0Ch]` callee is not" | the callee for a gun's sub-list is one of these five ticks, and the `dt` is the wave's `0.05f` | `008FBC8B` linking the bot into `gun+310h` |
| `docs/GUN_PLATFORM_ARC.md`: "the `allowFire` path therefore runs through the two bodies another packet owns" | neither `008FFF20` nor `00959C20` fetches a weapon director at all, so neither can read `director+3Ch` | both listings filtered for `0x114` have no hit; the section above |
| `docs/GUN_PLATFORM_ARC.md`: `006DF508` and `008FC22C` are "no Ghidra function / not decoded" | they are tail dispatches inside `006DF4C0` (`006DF4C0-006DF513`) and `008FC080` (`008FC080-008FC3F2`) | `int3` padding before each start and the `RET 4` at `006DF511` and `008FC3F0` |
| this document's first revision: the `009030C0` class was called "the bomb and depth-charge bot" and the `008FF4A0` class "the sub-type 8 bot" | `009030C0` is `AAFlakBot` and the sub-type 8 class is `DepthChargeBot`; the five class names are recovered strings, not descriptions | the name-to-global pairing in `load_robot_config_00901610`, section 2 |

## 10. Routine table

| Routine | Body | Coverage |
| --- | --- | --- |
| `008FFA20` | `008FFA20..008FFF1F` | complete |
| `009030C0` | `009030C0..0090341D` | complete |
| `006DF520` | `006DF520..006DFC6C` | complete |
| `00902920` | `00902920..009030B2` | partial: the fire byte's composition `00903010..009030A8` from pseudocode only |
| `008FFF20` | `008FFF20..0090099B` | partial: steps 7 and 12 by call shape only |
| `00959C20` | `00959C20..0095A5BF` | complete for the five arms' aim and trigger calls; the solution builders are unread |
| `0072BBD0`, `0072C6A0`, `008FBC80`, `008FBCE0`, `008FBEC0`, `008FBDC0`, `008FBEF0`, `006DF1F0`, `006DEFF0`, `008FEAC0`, `006DF260`, `0072BD10`, `0072BD30`, `00727E70`, `00727F10`, `00728000`, `0072D2C0`, `00954210`, `0085AB50`, `00922E90`, `00927F10`, `00875890` | - | complete |
| `00901C20` | `00901C20..0090227F` | complete, no loop; see section 6.2 |
| `00955630` | `00955630..0095581F` | complete |
| `006DEE40` | `006DEE40..006DEE80` | complete |
| `006DF170`, `0072BD50`, `008FEA40`, `008FF040`, `008FF310` | - | partial: entry and exit only |
| `00864FE0` | - | partial: the director lookup and the two bot arms |
| `006DF4C0` | `006DF4C0..006DF513` | complete; no Ghidra function |
| `008FC080` | `008FC080..008FC3F2` | `DepthChargeBot`'s tick; no Ghidra function; only the prologue shape and the `008FC232` tail were read |
| `00901610` | - | partial: the eight `00900AF0` calls and their stores at `009019EB`..`00901A53` |
| `008FBB00`, `00957BD0`, `00955830`, `00957740`, `004F3730`, `008527E0`, `008FE140`, `008FDBE0`, `006FDF60`, `008FF4A0`, `0085E4D0` | - | contract: unread |

## 11. Open questions

- The field names of the `AAGunnerBot`, `AAFlakBot` and `TorpedoBot` classes. Their name
  blocks are at `00D180F0`..`00D18218` and `00D18288` and the visitors are
  `008FE800`/`008FE8F0`, `008FF060`/`008FF130` and `008FF360`/`008FF400`.
- Where the skill arrays inside the class descriptors begin. The class names are settled
  but the objects are not: `00900AF0`, which builds one from a name, and the ledger's
  `read_<name>_parameters_*` routines (`008FC6D0`, `008FCA10`, `008FCD60`, `008FCF30`,
  `008FD370`, `008FD640`, `008FD880`, `009973B0`) are **contract: unread**. Only
  descriptor-relative offsets are proven here.
- Who reads `director+3Ch` `allowFire`. This packet proves it is none of `008FFF20`,
  `00959C20` or `00864FE0`, and the byte-pattern scan found no reader; the question goes
  back to whoever owns the weapon director.
- `008FC080`, `DepthChargeBot`'s tick, and the `ArtillerySubDirectorBot` and `PilotBot`
  classes at `[00E19994]` and `[00F8A30C]`.
- The four lead scalars `vtable[100h]` takes, and the `(0.6, 0.6, 0.6)` direction
  (`00CE3D30`) every caller passes as its second argument.
- `gun+474h` and `008FDBE0`, the `009030C0` step 8 pair.
- Whether bit `1` of `[unit+634h]` really means "a human is aiming this gun"; three ticks
  read three different bits of the same dword.

## Correction from docs/DIRECTOR_UPDATE_ARMS.md (packet cc2_director_update_arms)

- **Was:** director+221h described as the artillery flag: 'the owning unit's weapon director must have its artillery flag set'
  **Is:** +221h is aaEnabled; artilleryEnabled is +220h. The gun bot's block also runs when the byte is clear, not set
  **Evidence:** 008FFAD5 CMP byte ptr [EAX+221h],0 followed by JNZ 008FFAE9, which skips the block when the byte is non-zero

## Correction from docs/UNIT_GUNNERY_PASS.md (packet cc2_unit_gunnery_pass)

## Correction from docs/GUN_BOT_REMAINDER.md (packet cc2_gun_bot_remainder)

- **Was:** 00902C86, 00902CAC, 00902CC7 and 00902CD0 are a second vtable[100h] sample
  **Is:** those four sites are 008FDAF0, 00438B10, 00438B10 and 00438AA0; the only vtable[100h] call in 00902920 is at 00902BFD
  **Evidence:** the Ghidra listing of 00902920 contains exactly one [EDX+0x100] load, at 00902bf0, and its CALL EDX is at 00902bfd
- **Was:** the four lead scalars vtable[100h] takes and the (0.6, 0.6, 0.6) direction every caller passes are open questions
  **Is:** they are SectionTargetChance, EngineRoomWeight, MagazineWeight and FueltankWeight, and the third argument is a fraction of the hull half-extents rather than a direction
  **Evidence:** 008FCA10 stores those four Lua keys at TailGunnerBot descriptor +20h, +24h, +28h and +2Ch, which are the four floats 008ffb8c-008ffbaa push; 00816820-008168d5 draws the box against [ship+538h]+0A0h/+0A4h/+0A8h
- **Was:** 008FBB00(runSpeed, targetVelocity, out) solves the torpedo intercept
  **Is:** it also takes the shooter and target points in ECX and EDX, and what it solves is an intercept whose linear coefficient is half the exact one, so it under-leads every target that is not stationary and not exactly abeam
  **Evidence:** 008fbb00's RET 0Ch leaves two register arguments; 008fb8d0 builds b = dot(d, v) at 008fb949-008fb96b, disc = b*b - 4*a*c at 008fba02 and divides by a + a at 008fba40, where the exact form needs 2*dot(d, v)
- **Was:** the friendly entity is projected forward by 1000 seconds of its own velocity and 004F3730 tests it against the torpedo run
  **Is:** the projection scales the entity's +94h/+9Ch forward vector, and 004F3730 is a two-dimensional segment crossing that reads only the x and z components
  **Evidence:** 00900630 and 00900647 read [EDI+0x94] and [EDI+0x9c]; 004f3730 and 004f3630 index only [0] and [1] of all their float arguments

## Correction from docs/GAMEPLAY_LOOSE_ENDS_1.md (packet cc2_gameplay_loose_ends_1)

- **Was:** Primary vtable slot +10h is a stub (0071C4A0).
  **Is:** The body is two byte stores, but the effect is not inert: it is the retire request that makes the owning tick node's sweeper unlink and delete the sub-node on its next pass.
  **Evidence:** 0071C4A0 sets [this+10h] = 1 and [this+11h] = 0; 008759E1 CMP byte ptr [ESI+0x10],BL in BSP_TickElement_RunSubNodes falls into the removal path at 008759E6 when it is set, and 00875A5A calls slot 0 with PUSH 1.
