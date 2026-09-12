# The command controller's per-frame update and the consumers of its permission bytes

Addresses: `0071F290` `00720180` `00721A40` `0071E7F0` `0071ECF0` `009F6AD0` `0089A8B0` `0089C360`
`0071D5D0` `0071D810` `0071D9E0` `0071F600` `00720450` `00720D80`-`00720E0A` `00836240` `008360C0`
`00836680` `00836920` `0080DC70` `0080E150` `0080E160` `0080E1B0` `00861970`-`008619C6` `008624C0`
`009F1BC0` `009F5610` `009F6A20` `009F6AC0` `00721280` `007214C0` `00721890` `00721980` `007219C0`
`00CFDA40` (vtable) `00D09F58` (vtable) `00D21B48` (vtable) `00D09DB8`/`00D09DCC`/`00D09DDC`/
`00D09DF0`/`00D09E00`/`00D09E0C` (strings) `00D7A248` `00D7A24C` `00F87574` (data).

Packet `cc2_director_update_arms`, read-only analysis of the shipped `battlestationspacific.exe`.
Every descriptive name here is a hypothesis, not a recovered symbol, **except** the six property
strings in the field table below, which are literals in the image. The controller is the object at
`unit+738h` (`docs/WEAPON_DIRECTOR.md`, `docs/COMMAND_EXECUTION.md`, `docs/DIRECTOR_TARGET_GATE.md`).

## Headline

`0071F290` is not a hold timer with some other work attached; it is **the whole per-frame driver of
the unit command controller**, and it does its work in seven arms whose order matters. The first
arm abandons the frame unless the session object at `+34h` reads live, and the last arm is the one
that actually moves the unit: it ticks the auto-target object at `+38h` with the frame delta and
then runs the command step `vtable[7Ch]`. Between them sit the hold countdown, an idle-to-queue
mode promotion, and two begin-command attempts whose **refusal terminates the command** by raising
its stage to 2.

Separately, the four "enable" bytes at `+220h`..`+223h` turn out to have recovered names, because
the reflection visitor `008360C0` hands a name string to its visitor for each one:
`artilleryEnabled`, `aaEnabled`, `torpedoEnabled`, `depthChargeEnabled`. That answers
`docs/WEAPON_DIRECTOR.md`'s open question about sub-kinds 4 and 6.

## `0071F290 BSP_CommandControllerBase_Update`, the rule table

`__thiscall(controller, float frameDelta)`, `RET 4` at `0071F3A2`, body `0071F290`-`0071F3A4`.
Coverage: **complete**. The address sits at `00CFDA4C`, `00D09ECC`, `00D09F64` and `00D0BDA4`, i.e.
slot `+0Ch` of the base controller vtable and of both director vtables, so the director does not
override it. `PUSH ECX` at `0071F290` reserves the spill slot the `COMISS` result uses at
`[ESP+4]`; the argument is at `[ESP+0Ch]` throughout.

| # | Arm | Body | Rule |
| --- | --- | --- | --- |
| 1 | session gate | `0071F294`-`0071F2C1` | `s = [this+34h]`; return unless `s != 0`, `s[+5Ch] != 0`, `s[+5Dh] == 0`, `s[+60h] == 0` and `s[+5Eh] == 0`. Every failure jumps to the epilogue at `0071F3A0`, so nothing else runs |
| 2 | path vector reset | `0071F2C7`-`0071F2F7` | when `p = [this+1A4h]` (path object 0, not the other nine) is non-null, store the shared default vector `00F87574`/`78`/`7C` into `p[+30h]`, `p[+34h]`, `p[+38h]` |
| 3 | hold countdown | `0071F2F8`-`0071F316` | `COMISS` the hold at `+40h` against `00D7A218` = `0.0f`; `JB` skips, so a negative (and an unordered/NaN) value is left alone; otherwise `[+40h] = hold - frameDelta` through x87. `docs/DIRECTOR_TARGET_GATE.md` owns this rule |
| 4 | mode promotion | `0071F317`-`0071F329` | `if ([+30h] == 0 && [+54h] != 0) [+30h] = 1`. Idle plus an occupied queue slot 0 becomes "queue head is current" |
| 5 | begin the override | `0071F32A`-`0071F34E` | `if ([+4Ch] == 0 && [+188h] != 0)`: `ok = this->vtable[78h](0)`; when `ok` is false, `0071D9E0(2)` |
| 6 | begin the queue head | `0071F34F`-`0071F372` | `if ([+44h] == 0 && [+54h] != 0)`: `ok = this->vtable[78h](1)`; when `ok` is false, `0071D810(2)` |
| 7 | the two steps | `0071F373`-`0071F39F` | return when `*(00E188A8)[+1FE4h] == 2`; else `if ([+38h] != 0) [+38h]->vtable[4](frameDelta)`, then `this->vtable[7Ch]()` |

### What arm 7 actually calls

`[controller+38h]` is the `40h`-byte object `008366D0` allocates at `00836753`-`0083676A` and
`009F6A20 BSP_WeaponDirector_AutoTargetConstruct` fills. That constructor installs vtable
`00D21B48` at `009F6A45`, and `00D21B48 + 4` holds **`009F5DA0`**, the bot fire-target think of
`docs/BOT_FIRE_TARGET.md`. So arm 7's first call *is* the auto-target think, driven once per frame
with the frame delta, and arm 3's countdown is the gate that same think consults at step 13.

`vtable[7Ch]` is `00BF698E` on the base table (the shared stub, i.e. the base controller does
nothing) and **`00836920 BSP_WeaponDirector_Step`** on the derived director table at `00D09FD4`.
That is where the queued command is actually executed; `docs/COMMAND_EXECUTION.md` owns it.

`vtable[78h]` is `0071F600 BSP_WeaponDirector_BeginCommandBase` on the base table (`00CFDAB8`) and
`00835C70` on the derived table (`00D09FD0`). Its one argument selects the pair it begins: `0`
the override (`+188h`, `+18Ch`), non-zero the queue head (`+54h`, `+58h`), and it writes the
matching accepted byte (`+4Ch` or `+44h`), which is why arms 5 and 6 run at most once per command.

`0071D810 BSP_WeaponDirector_RaiseCommandStage` (`+48h`, the queue head) and `0071D9E0` (`+50h`,
the override) are the same body twice, read here to settle what a refusal means:

```
if ([this+stageField] >= stage) return;      // monotonic
[this+stageField] = stage;
if (stage != 2) return;
if (*(00E188A8)[+1FE4h] == 2) return;
msg = 0071C730(&stackMsg)(isQueueHead, 0);   // 1 for 0071D810, 0 for 0071D9E0
0077C2A0([this+34h], msg, 7, 0);
```

Stage 2 is the terminal stage (`00836920` tests `[ESI+48h] == 2`), so **a command whose begin is
refused is completed immediately and the completion is broadcast**. Note the ordering: the
session-mode-2 return of arm 7 is *after* arms 5 and 6, so a client in mode 2 still terminates a
refused command locally even though the send inside the raise is suppressed.

### The session-live predicate of arm 1

The same four-byte test, in the same order and with the same polarity, is inlined at `0077A564`,
`0077BD8E`, `0077C9B5`, `0077CE66` and `0077CEFB` in the session code, which is what makes it the
session object's own "live" predicate rather than anything the controller owns. What the four
bytes individually mean is **contract: unread**; this packet did not find their writers.

## `00720180 BSP_CommandControllerBase_Construct`, whole

`__thiscall(this)(owner) -> this` in `EAX`, `RET 4` at `00720440`, body `00720180`-`00720442`, one
SEH frame (handler `00C85357`). One stack argument: `008363E0` pushes its own first argument at
`008363FE` and nothing else. Coverage: **complete**.

1. `007201AB`-`007201E4` builds the two embedded bases: vtable `00CFD99C` at `+0h`, `+4h`/`+8h`/
   `+0Ch` zero, `+10h` = 0, `+11h` = 1, `+14h` = 0, `+18h` = 2, then `00876020(owner)(this)`
   registers the controller with the owner, then the secondary vtable `00CE3CD4` at `+1Ch`,
   `+20h`/`+24h`/`+28h` zero and `+2Ch` = 0.
2. `007201FB`-`00720201` overwrites both vtable pointers with the real ones, `00CFDA40` and
   `00CFDA28`.
3. `0072020B`-`00720219` **`[this+34h] = owner[+28h]`**. So the session endpoint is not the
   constructor's argument; it is a field of it. The argument itself is `unit->vtable[60h](1)`
   (`008366D0` step 1).
4. `00720208`-`00720233` zeroes the execution state: `+30h` mode, `+38h`, **`+3Ch` allowFire and
   `+3Dh` allowMove both zero**, `+40h` = `-1.0f` (`00D7A260`), `+44h`, `+48h`, `+4Ch`, `+50h`.
5. `00720236` `00BF7CD1` memsets the ten command slots (`LEA EAX,[ESI+54h]`, `1Ch`, `0Ah`).
6. `0072023B`-`0072028E` the previous-command record `+16Ch`-`+184h`: pointer zero, the three
   bytes/words at `+170h`-`+174h` zero, `+178h`/`+17Ch`/`+180h` the default vector, `+184h` = 0.
7. `00720296`-`007202E6` the override record `+188h`-`+1A0h`, the same shape.
8. `007202F3` `[+1CCh] = 1FFh`, the value the sub-kind 2 message later overwrites.
9. `007202FD`-`00720312` **`[+220h] = [+221h] = [+222h] = [+223h] = 1`**: all four weapon enables
   default to permitted.
10. `00720319`-`00720329` `[this+18h] = 1`, and when `[this+4h]` is non-null `00876120(this)`.
11. `0072032E`-`007203A5` twenty dwords at `+1D0h`-`+21Fh`, every one `D01502F9h` (about `-1.004e10`
    as a float, a "no value" sentinel). This block is new; `docs/WEAPON_DIRECTOR.md`'s field table
    jumped from `+1CCh` to `+220h`. Its consumer was not found.
12. `007203AB`-`0072042A` the ten path objects: `operator_new(50h)` per slot, stored into the
    pointer array at `+1A4h` (`LEA EDI,[ESI+1A4h]`, `EDI += 4` per iteration). Each object gets
    vtable `00CFDB24` at `+0h` and `00CFDB10` at `+10h`, `+4h` = the controller, `+8h` = 1,
    `+0Ch` = 5, `+14h`/`+18h` = 0, `+1Ch` = 1, `+20h` = 1 (byte), `+24h` = the controller,
    `+28h` = 0 (byte), `+2Ch` = `-1`, `+3Ch` = `-1.0f`, `+44h`/`+48h`/`+4Ch` = 0. An allocation
    failure stores a null pointer and the loop continues.

## The permission bytes: names, writers, consumers

`008360C0` is a reflection visitor, `__thiscall(controller)(visitor)`, `RET 4`. For each field it
builds a two-dword stack record and hands the visitor a name pointer, then a `{tag, value}` record
(tag 3 for a bool, 4 for the id). The name strings are literals in `.rdata`:

| Offset | Name string | Address | Sub-kind that writes it |
| --- | --- | --- | --- |
| `+220h` | `artilleryEnabled` | `00D09E0C` | 3 (`0071C231`) |
| `+221h` | `aaEnabled` | `00D09E00` | 4 (`0071C246`) |
| `+222h` | `torpedoEnabled` | `00D09DF0` | 5 (`0071C25B`) |
| `+223h` | `depthChargeEnabled` | `00D09DDC` | 6 (`0071C270`) |
| `+238h` | `fireTargetID` | `00D09DCC` | `00836240` |
| `+23Ch` | `fireTargetIsPrimary` | `00D09DB8` | `00836240` |

The visitor also emits `_gameUnit` (`00D09E20`) first and the six avoidance/cruise names
`docs/WEAPON_DIRECTOR.md` already records.

### The consumers, with call sites

| Consumer | Reads | Call sites | What the permission gates |
| --- | --- | --- | --- |
| `0080DC70` | `+3Ch` **and** `+3Dh` | `008171F6` (in `00816E30`'s arms, `docs/ENTITY_COMMAND_ARMS.md`), `009F83CB` (in `009F8160 BSP_Bot_RevalidateCurrentCommand`) | `allowFire && allowMove` as one predicate; either byte clear answers false |
| `009F5610 BSP_WeaponDirector_AutoTargetEnabled` | `+3Dh` | vtable of the `+38h` object | **allowMove**, not allowFire, plus slot 0's category |
| `008624C0` | `+3Ch`, `+3Dh`, `+220h`..`+223h` | `00865073` in `00864FE0 BSP_UnitGunneryAi_Tick`, `00863AE7` and `00863AF9` in `00863A80` | the weapon panel: see its own table below |
| `009F1BC0` | `+220h`..`+223h` | `009F2ADA`, `009F2B13`, `009F2B54`, `009F2BA8` and again at `009F2E00`..`009F2E4F`, each through `0080E160` | caches the four enables into the AI record at `+12B8h`..`+12BBh`, ANDing `aaEnabled` with `008637D0` |
| `00720450` (base vtable `+4Ch`, derived `00836680`) | `+3Ch`, `+3Dh` | `00836689` | copies one controller's state into another, stance bytes included |
| `00721280` + `007214C0` | `+3Ch`, `+3Dh`, `+220h`..`+223h` | the network state snapshot, out | `+3Ch`/`+3Dh` to message `+4h`/`+5h`; `+220h`..`+223h` to message `+40h`..`+43h` |
| `00721890` + `00721980` / `007219C0` | writes `+220h`..`+223h` | the snapshot, back in | message `+40h`..`+43h` to `+220h`..`+223h` |
| `008FFA20 BSP_GunBot_TurretAimAndFireTick` | `+221h` | `008FFAD5`, after `0080E150` at `008FFAC9` | the only gun-side read, and `docs/GUN_BOT_TICKS.md` calls it "the artillery flag": it is `aaEnabled` |

`009F5610`, complete, `009F5610`-`009F563C`, `__thiscall(autoTarget) -> bool`:

```
director = [autoTarget+0Ch]          // 009F6A20 writes it at 009F6AA5
if (director[+3Dh] == 0) return false
cmd = [director+54h]
if (cmd == 0) return true
cat = cmd->vtable[0Ch]()
return !(cat == 1 || cat == 2)
```

It is the same category rule as `0071DF70` but reads only slot 0, and it adds the `allowMove` test
that `0071DF70` does not have. The provenance of `[ecx+0Ch]` is the constructor's own store, not a
guess: `009F6A20` writes the director into both `+4h` (`009F6AA5` writes `+0Ch`, `009F6A42` writes
`+4h`) of the auto-target object.

### `008624C0`, the weapon-panel mirror

`__thiscall(view)(char force)`, where `view[+0h]` is the panel record, `view[+4h]` the controller,
`view[+8h]` a countdown and `view[+0Ch]`..`view[+10h]` the previously mirrored bytes. Complete.

| # | Rule | Site |
| --- | --- | --- |
| 1 | `panel[+7Ch] = controller[+3Dh]`, every call, never diffed | `008624C6` |
| 2 | `view[+8h] -= 1`; when it reaches 0 or `force` is set, reload it with 10 and set `refreshAll` | `008624E5` |
| 3 | on `refreshAll` or a change in `controller[+3Ch]`: **allowFire set** fills `panel[+70h]`..`panel[+7Bh]` with `01010101` (preserving `+77h` and `+78h`); **allowFire clear** zeroes the same twelve bytes, then re-sets `panel[+70h]` and `panel[+71h]` when `panel[+50h]` answers `vtable[5Ch](0Fh)` | `008624F2` |
| 4 | on `refreshAll` or a change: `torpedoEnabled` -> `00861D70`, `artilleryEnabled` -> `00861CD0`, `aaEnabled` -> `00861D20` | `00862566`, `0086258B`, `008625B0` |
| 5 | on a change only, never on the periodic refresh: `depthChargeEnabled` -> `00861DC0` | `008625D3` |

So `allowFire` is the master switch over a twelve-byte per-weapon enable array, and the four named
enables are per-category switches pushed through four setters.

## `00721A40 BSP_WeaponDirector_ApplyGameUnitMessage`, whole

`__thiscall(wrapper, message)`, `RET 4`, body `00721A40`-`00721C58`. `docs/CRUISE_COMMAND.md` and
`docs/DIRECTOR_TARGET_GATE.md` read the `5Ch` arm; this packet read the rest. Coverage: **complete
as an arm table**; the bodies the arms call are each owned elsewhere.

Entry gate: the director must exist, `[director+34h]` must exist, `session->vtable[13Ch]()` must
return non-null and that object's `+5Eh` byte must be clear. Then a seven-way `IsA` cascade:

| Kind | Arm | Action |
| --- | --- | --- |
| `5Ah` | `00721A93` | `director->vtable[38h](msg)` = `00835640` over `0071C1E0`, the permission and enable receiver |
| `5Bh` | `00721AB6` | `director->vtable[60h](00E08F80, BSP_CommandTarget_FromEntity(00720FA0(0), 0))`, then `0071C1B0(msg+24h)` |
| `5Ch` | `00721AEE`-`00721B87` | in session mode 2, resolve the descriptor or drop the message; then `msg[+20h]` non-zero -> `vtable[60h]` (`008358D0`, queue), zero -> `0071E7F0` (override) |
| `5Dh` | `00721B8A`-`00721BD0` | `msg[+20h] == 0` -> `ClearOverrideCommand`; else `msg[+24h] < 0` -> `ClearAllCommandSlots`; else `InternalClearPrimaryCommand(msg[+24h])` |
| `5Eh` | `00721BD3`-`00721BFC` | `entity = msg->00720FE0(msg[+23h])`; `00836240 BSP_WeaponDirector_StoreFireTarget(entity)` |
| `60h` | `00721BFF`-`00721C1F` | `0071C0B0(msg[+20h])` |
| `5Fh` | `00721C22`-`00721C50` | `msg[+2Ch]` non-zero -> `007207C0(msg+20h)`; else `0071F5D0()` |

### This closes the `+238h` question

`docs/WEAPON_DIRECTOR.md` left "who writes `+238h`" open because a scan for `mov [reg+238h], reg`
found only the constructor's zero and `00836210`'s clear. The write exists but uses a different
displacement: `00836240 BSP_WeaponDirector_StoreFireTarget` takes `ESI = this+224h` (the embedded
reference object) and stores at **`[ESI+14h]`**, which is `this+238h`.
`__thiscall(director)(entity, force)`, `RET 8`, body `00836240`-`00836299`, complete:

1. `00836244`-`00836257` the receive-side gate, the twin of `00835860`'s send-side one: a
   **non-forced** store is dropped when `fireTargetIsPrimary` is set **and** a target is held.
2. `00836264` `[this+23Ch] = force`, so the force byte *is* `fireTargetIsPrimary`.
3. `0083626C`-`00836295` when the target changes: unregister the old observer pair through
   `006952A0`, `[this+238h] = entity`, register the new pair.

## `0071E7F0 BSP_WeaponDirector_SetOverrideCommand` and `0071ECF0`, the two entry paths

`0071E7F0`, `__thiscall(director)(command, target)`, `RET 8` at `0071E914`, body
`0071E7F0`-`0071E916`. Coverage: **complete**.

1. `0071E7FD`-`0071E81B` no-op guard: already in mode 2 with the same command and, per
   `0071E200`, the same target record -> return.
2. `0071E823` `0071D6D0(command, target)` must answer true, or return.
3. `0071E832`-`0071E870` the queue veto: when slot 0 holds `00E08F78` the new target must resolve
   (`00521EA0`) to the same object as slot 0's; otherwise a slot-0 command of category 1 or 2
   refuses the override outright. Same category rule as `0071DF70` and `009F5610`.
4. `0071E876`-`0071E8F1` swap the observer pair, `[+188h] = command`, copy the `18h`-byte parameter
   record into `+18Ch`.
5. `0071E8F3`-`0071E90E` `[+30h] = 2`, `[+4Ch] = 0`, `[+50h] = 0`, then `vtable[6Ch](0)`.

`0071ECF0 BSP_WeaponDirector_IssueCommand` is read here only as the contrast and is owned by
`docs/CRUISE_COMMAND.md`: it forwards to the AI group through `00A2BD90`, calls `vtable[30h]` to
make room, then builds `MT_GAMEUNIT_SETCMD` with flag **1** and routes it. So the two paths differ
by the message flag `00721A40` branches on at `00721B36`: `0071ECF0` queues, `0071D980` overrides,
and `0071E7F0` is the receive-side landing point of the override.

## `009F6AD0`: nothing holds its address

`009F6AD0`, body `009F6AD0`-`009F6AD7`, `CMP byte ptr [ECX+3Ch],0 / SETE AL / RET`. A scan of the
**whole file on disk** for the little-endian dword `009F6AD0` and for every `E8`/`E9` relative
displacement that resolves to it returns **zero hits**. It is in no vtable, no jump table and no
function-pointer table, and no instruction calls or jumps to it.

It is not alone. The scan found the same answer for two other clusters of one-line accessors on
this same class:

| Cluster | Bodies | References |
| --- | --- | --- |
| `00720D80`-`00720E0A` | `get +3Ch`, `get +3Dh`, `get +220h`, `get +221h`, `get +222h`, `get +223h`, and a `RET 10h` setter that writes all four enables at once | 0 |
| `00861970`-`008619C6` | six getters of the shape `MOV EAX,[ECX+4] / MOV AL,[EAX+disp]`, for `+3Ch`, `+222h`, `+220h`, `+221h`, `+223h`, `+3Dh` on a wrapper whose `+4h` is the controller | 0 |
| `009F6AC0`-`009F6B06` | `MOV EAX,[ECX+4]`, the `+3Ch` predicate, a `[ECX] = arg` setter, a `[[ECX]+4]` getter, and a `[ECX] = 00D21B50` vtable install | 0 |

So the answer to "what holds `009F6AD0`" is **nothing**: it is one of a family of out-of-line
copies of inline accessors that the compiler emitted for this class and the linker kept, none of
which any code references.

Whose `this` it takes cannot be settled from a call site, because there is none. The evidence
favours the **controller**, and only provisionally: `00720D80` is the byte-for-byte same access
(`[ECX+3Ch]`, `this` in `ECX`, no arguments) on the controller, and the two neighbours that do
name their object, `009F6AC0` and `009F6AF0`, read `+4h`, which on the auto-target object is the
director. The alternative reading, `this` = the auto-target object, is weak: `009F6A20` writes that
object's `+3Ch` with `MOVSS` from `00D7A248` = `FLT_MAX` (`009F6AA8`), and a `CMP byte ..., 0`
against the low byte of a float is not a meaningful predicate. Reading it as the controller makes
it `bool fireForbidden()`, the negation of `allowFire`. **Provisional.**

## The two Lua tails

`0089C360 BSP_LuaBinding_GetFireTarget`, tail after the two virtual hops. Coverage: now complete.

```
entity   = BSP_ObjectHandle_FromLuaTable(arg 1)
director = entity->vtable[114h]()          // 0080E150, [unit+738h]
target   = director->vtable[2Ch]()         // 008364E0, [director+238h]
if (target == 0) { PushNil(); }
else {
    key = BSP_NativeString_FromInt(*(u16*)(target + 174h));
    BSP_LuaObject_GetByLiteral("thisTable");
    BSP_LuaObject_GetByNativeString(key);
    BSP_LuaObject_PushValue();
    // then the temporary string block is returned to the sized storage pool
}
return BSP_LuaObject_ResultCount();
```

So the binding returns the fire target's Lua entity table, keyed by the decimal form of the
entity's `u16` id at `+174h`, or `nil`.

`0089A8B0 BSP_LuaBinding_SetFireTarget`, tail after the three argument branches. Coverage: now
complete.

```
entity->vtable[114h]()                     // the director, result discarded by the decompiler
                                           // but it is the ECX of the next call
00835860(target, 1)                        // BSP_WeaponDirector_SetFireTarget, force = 1
return BSP_LuaObject_ResultCount();         // nothing was pushed, so 0
```

The tail pushes nothing, so the binding returns zero Lua results. The `force` argument is the
constant `1`, which is why a script `SetFireTarget` always wins over `fireTargetIsPrimary` while
the network path (`00836240`, force from `msg+23h`) can be refused.

## Host table

| Step | Site | Callee | Host method | Arguments | Condition |
| --- | --- | --- | --- | --- | --- |
| reset path vector | `0071F2D9` | inline store | `reset_path_vector` | - | `[this+1A4h] != 0` |
| begin the override | `0071F340` | `vtable[78h]` = `0071F600` | `begin_command` | ECX = this; (0); ret bool | `[+4Ch] == 0 && [+188h] != 0` |
| terminate the override | `0071F34A` | `0071D9E0` | `raise_override_stage` | ECX = this; (2) | the begin refused |
| begin the queue head | `0071F364` | `vtable[78h]` = `0071F600` | `begin_command` | ECX = this; (1); ret bool | `[+44h] == 0 && [+54h] != 0` |
| terminate the queue head | `0071F36E` | `0071D810` | `raise_queue_stage` | ECX = this; (2) | the begin refused |
| tick the auto-target | `0071F395` | `[+38h]->vtable[4]` = `009F5DA0` | `step_auto_target` | ECX = `[+38h]`; (float dt); RET 4 | session mode != 2 and `[+38h] != 0` |
| step the commands | `0071F39E` | `vtable[7Ch]` = `00836920` | `step_commands` | ECX = this | session mode != 2 |

## Reconstruction

`include/bsp/director_update_arms.hpp` and `src/director_update_arms.cpp` carry the seven arms as
`run_command_controller_update` over `CommandControllerUpdateHost`, the permission set as
`DirectorPermissions` with the recovered names, and the consumer rules
`director_allows_fire_and_move` (`0080DC70`), `auto_target_enabled` (`009F5610`),
`store_fire_target_accepted` (`00836240`'s gate) and `weapon_panel_sync` (`008624C0`). The hold
countdown is not duplicated; the header includes `bsp/director_target_gate.hpp` and calls its
`update_auto_target_hold`. Build-tested, one focused case in `tests/math_tests.cpp`. Not
ABI-compatible and not game-validated.

## Coverage

| Routine | Coverage |
| --- | --- |
| `0071F290` | complete, `0071F290`-`0071F3A4`, every arm with its call sites |
| `00720180` | complete, `00720180`-`00720442` |
| `00721A40` | complete as an arm table; each arm's callee is owned elsewhere |
| `0071E7F0` | complete, `0071E7F0`-`0071E916` |
| `00836240` | complete, `00836240`-`00836299` |
| `0080DC70`, `009F5610`, `0071D5D0`, `00720D80`-`00720E0A`, `00861970`-`008619C6`, `009F6AC0`-`009F6B06` | complete, decoded from raw bytes |
| `0071D810`, `0071D9E0` | complete as stage raisers and senders |
| `008624C0` | complete as a decision table; the four setters `00861CD0`/`00861D20`/`00861D70`/`00861DC0` and the panel record are unread |
| `008360C0` | complete as the name source; the visitor's `vtable[4]`/`[8]`/`[0Ch]` are unread |
| `0071ECF0` | read only as the contrast; `docs/CRUISE_COMMAND.md` owns it |
| `0089A8B0`, `0089C360` | the tails are now complete; the three argument branches stay with `docs/WEAPON_DIRECTOR.md` |
| `009F1BC0` | partial: only the two four-enable cache blocks at `009F2ADA`-`009F2BAD` and `009F2E00`-`009F2E54`; `008637D0` is unread |
| `0071F600`, `00836920`, `00835C70` | not re-read; `docs/COMMAND_EXECUTION.md` and the ledger own them |
| `00720450` | partial: the stance and previous-command copy and the slot loop's shape; `0071DB90`, `0071FD10` and `006F7DD0` are unread |
| the session bytes `+5Ch`/`+5Dh`/`+5Eh`/`+60h` | contract: unread. Their writers were not found |

## Corrections to earlier documents

| Document | Was | Is | Evidence |
| --- | --- | --- | --- |
| `docs/WEAPON_DIRECTOR.md` field table | `+221h` "sub-kind 4 flag", `+223h` "sub-kind 6 flag" | `+221h` is `aaEnabled` and `+223h` is `depthChargeEnabled`, recovered strings | `008360C0` passes `00D09E00` and `00D09DDC` at `00836114` and `008361A1` |
| `docs/WEAPON_DIRECTOR.md` field table | `+23Ch` "byte that gates an unforced fire-target change" | its name is `fireTargetIsPrimary`; `+238h` is `fireTargetID` | `008361FA` passes `00D09DB8`, `008361C9` passes `00D09DCC` |
| `docs/WEAPON_DIRECTOR.md` open questions | "Who writes `+238h` ... the local write was not found" | `00836240 BSP_WeaponDirector_StoreFireTarget` writes it, reached from `00721A40`'s `5Eh` arm at `00721BF1`. The scan missed it because the store is `MOV [ESI+14h],EDI` with `ESI = this+224h` | `0083626D LEA ESI,[ECX+224h]`, `00836287 MOV [ESI+14h],EDI` |
| `docs/WEAPON_DIRECTOR.md` open questions | "what sub-kinds 4 and 6 mean next to the artillery and torpedo flags" | 4 is anti-aircraft, 6 is depth charges; the four are one per weapon category, not two pairs | the four name strings above |
| `docs/WEAPON_DIRECTOR.md` construction note | `008363E0` "when both are false it sets `+3Ch` = 1 and `+3Dh` = 1" | true, and the starting point is explicit: `00720180` zeroes both at `0072021F`/`00720222` and sets all four enables to 1 at `007202FD`-`00720312` | the constructor listing |
| `docs/WEAPON_DIRECTOR.md` open questions | "Whether a periodic director step aims guns" | yes: `0071F290` arm 7 calls `[+38h]->vtable[4](dt)` = `009F5DA0`, the bot fire-target think, and then `vtable[7Ch]` = `00836920` | `00D21B4C` holds `009F5DA0`; `00D09FD4` holds `00836920` |
| `docs/DIRECTOR_TARGET_GATE.md` | `0071F290` "**Ghidra has no function here**" | Ghidra now has `BSP_CommandControllerBase_Update` with body `0071F290`-`0071F3A4` | `tools/bsp.py ghidra proto 0071F290` |
| `docs/DIRECTOR_TARGET_GATE.md` | `0071F290` "lifts mode from 0 to 1 ... sends `0071D9E0(2)` and `0071D810(2)` behind the `+4Ch` and `+44h` accepted flags" | the send is conditional on `vtable[78h]` **refusing**, not on the accepted flag alone; the flag only decides whether the arm runs at all | `0071F342 TEST AL,AL / JNZ 0071F34F` |
| `docs/DIRECTOR_TARGET_GATE.md` open questions | "`0071F290`'s other arms: what `[+1A4h]` is, and what `vtable[78h]` answers for 0 and 1" | `[+1A4h]` is the first of the ten `50h`-byte path objects `00720180` allocates; `vtable[78h]` is `0071F600`, 0 = the override pair, non-zero = the queue-head pair | `007203AB`-`0072042A`; the ledger entry on `0071F600` |
| `docs/GUN_BOT_TICKS.md` line 232 and 405 | `director+221h` described as "the artillery flag" / "the sub-kind 4 flag" | `+221h` is `aaEnabled`; `+220h` is `artilleryEnabled`. The gun bot's one director read is the anti-aircraft enable, and it runs the block when the byte is **clear** (`JNZ` at `008FFADC` skips) | `008FFAD5 CMP byte ptr [EAX+221h],0` |

## Open questions

- The four session bytes `+5Ch`, `+5Dh`, `+5Eh`, `+60h`. The predicate's shape is settled and it is
  shared with five inlined copies in the session code, but no writer was found.
- The twenty `D01502F9h` sentinels at `+1D0h`-`+21Fh`. Nothing in this packet reads them.
- Whether the shared vector at `00F87574`-`7C` is a plain zero. It lies past the raw `.data` in the
  file, so it is zero at load unless a static initialiser writes it; with 262 references across the
  image it was not chased.
- `008637D0`, the per-weapon availability test `009F1BC0` ANDs with `aaEnabled`.
- The fourth vtable holding `0071F290` at `00D0BDA4`, still open from
  `docs/DIRECTOR_TARGET_GATE.md`.
- Whether the three orphan accessor clusters are dead in the shipped build only, or were dead in
  every configuration. Nothing in a shipped image can answer that.

## Correction from docs/GAMEPLAY_LOOSE_ENDS_2.md (packet cc2_gameplay_loose_ends_2)

- **Was:** open question: the twenty D01502F9h sentinels at +1D0h-+21Fh. Nothing in this packet reads them
  **Is:** nothing in the image reads them; the bit pattern is the float -1.0e10
  **Evidence:** the immediate 0D01502F9h decodes to exactly one instruction image-wide, 0072032E MOV EAX,0D01502F9h, and the only other raw occurrence (00CE4ADC) sits inside a string blob; every 1D0h-21Fh access in 00700000-00730000 belongs to a gun class or to the fill
- **Was:** open question: whether the shared vector at 00F87574-7Ch is a plain zero; with 262 references it was not chased
  **Is:** it is the zero vector for the whole run
  **Evidence:** .data rawptr 00A08000 rawsize 10000h backs only 00E08000-00E18000 so 00F87574 is in the zero-filled tail; scanning .text for the three absolute addresses gives 520, 516 and 520 decoded operand references and not one store of any form (no MOVSS/MOVUPS/FSTP/MOV to those addresses)
- **Was:** open question: the fourth vtable holding 0071F290 at 00D0BDA4
  **Is:** 00D0BDA4 is slot +0Ch of 00D0BD98, the table FUN_0084D810 installs on the class that also owns 0084E010 at +7Ch
  **Evidence:** 0084D849 MOV dword ptr [ESI],0D0BD98h; the code-pointer run from 00D0BD98 puts 0071F290 at +0Ch and 0084E010 at +7Ch
- **Was:** open question: 008637D0, the per-weapon availability test 009F1BC0 ANDs with aaEnabled
  **Is:** __thiscall(unit, Entity* target), RET 4: true when any category in the list at 00E0A510 is present on the unit, accepted by [unit+60h]->vtable[4h], and willing to engage the target
  **Evidence:** 008637D5 and 0086381A bound the loop with CMP ... ,0Ch; 008637F2 CMP byte [EAX+EDI+70h],0; 008637F9-00863802 the vtable[4h] call; 0086380E CALL 008633D0; 00863837 MOV AL,1
