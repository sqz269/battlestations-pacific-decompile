# Gameplay tuning settings: the tail (packet `cc2_settings_tail`)

Addresses: 00836EF0 00836F80 00B685C0 0093C120 0093C210, and read-only 0082203D 00939F90
00939FA0 0093A470 0093A4F0 0088E320 0088E790 00821E80 0083A880 00871BA0.

Worker `agent/cc2-settings-tail`, 2026-09-11 UTC. Ghidra was read-only for this packet: no
renames, comments, prototypes, function creation or saves. Project `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. Every descriptive name here is a hypothesis, not a
recovered symbol. No run-time evidence: `bsp_game.exe` does not load a mission, so nothing below
was observed on a frame (`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 6).

This continues docs/GAMEPLAY_SETTINGS.md, which established the 76Ch object, its constructor
`00424A10` and the loader `0083B5E0`. It answers the three follow-ups `gameplay_settings_subobjects`,
`gameplay_settings_string_keys` and `repair_multiplier_naming`.

## Headline

The two repair multipliers are **not** crossed. The Lua key names match their consumers exactly
as the shipped comments describe them. What is crossed is the pair of Ghidra function names:
`BSP_RepairTask_ApplyFireDamage` at `0093C120` runs the **water** timer and
`BSP_RepairTask_ApplyWaterDamage` at `0093C210` runs the **fire** timer. The provisional
timer assignment in docs/UNIT_FIRE_AND_REPAIR.md (`+34h` fire, `+38h` water) is reversed, and so
are the two repair priorities that gate them.

The four `58h` sub-objects are `ShipGlobals["WeaponHitAccuracy"]`: one hit-probability curve per
weapon category (Artillery, AA, Torpedo, DepthCharge), ten range buckets by two target sizes.

## 1. The repair multiplier verdict

### The producer of the two timers

The timers are written only by the session message `9Eh` handler, which docs/UNIT_FIRE_AND_REPAIR.md
left unread. `00821E80 BSP_UnitInstance_HandleMessage` switches on `kind - 4Bh` through the byte
table at `00822400` and the target table at `00822394` (docs/SESSION_MESSAGE_DISPATCH.md). Kind
`9Eh` is index `53h`; `[00822453]` is `17h` and `[00822394 + 17h*4]` is `0082203D`.

```
0082203d: MOV EAX,dword ptr [ESI + 0x24]   ; the message's selector
00822040: SUB EAX,0x0
00822043: JZ 0x00822090                    ; selector 0
00822045: SUB EAX,0x1
00822048: JNZ 0x008220c2                   ; anything else: no arm
0082204a: CMP byte ptr [ESI + 0x20],AL     ; AL = 0
...
0082205a: JZ 0x00822076                    ; selector 1, flag clear -> 00939FA0
0082205c: CALL 0x0093a4f0                  ; selector 1, flag set
...
008220a1: JZ 0x008220bd                    ; selector 0, flag clear -> 00939F90
008220a3: CALL 0x0093a470                  ; selector 0, flag set
```

`ECX` at each call is `LEA ECX,[EDI + 0xa20]`, the repair task. The four callees:

| callee | body | writes |
| --- | --- | --- |
| `00939F90` | `00939F90..00939F9D` | `MOVSS [ECX + 0x38],XMM0`, a plain store |
| `0093A470` | `0093A470..0093A4E4` | adds to `+38h` and to `+3Ch`, clears `+44h` |
| `00939FA0` | `00939FA0..00939FAD` | `MOVSS [ECX + 0x34],XMM0`, a plain store |
| `0093A4F0` | `0093A4F0..0093A566` | adds to `+34h`, mirrors the total into `+40h`, clears `+44h` |

The selector comes from the two Lua bindings. `0088E320` logs `luaMW_SetFireDamage failed:`
(`00D115D4`) and `0088E790` logs `luaMW_SetWaterDamage failed:` (`00D11614`), so the names in
docs/UNIT_FIRE_AND_REPAIR.md are confirmed from the binary. Both fill the same message layout:

| field | fire `0088E320` | water `0088E790` |
| --- | --- | --- |
| vtable `+0h` | `0xD0334C` at `0088E4D7` | `0xD0334C` at `0088E942` |
| float `+1Ch` | `XMM0` at `0088E4DF` | `XMM0` at `0088E94A` |
| flag `+20h` | `SETZ` of the bool at `0088E4E5` | `SETZ` of the bool at `0088E954` |
| selector `+24h` | `EBP`, and `XOR EBP,EBP` at `0088E343` makes it **0** | `EBP`, and `MOV EBP,0x1` at `0088E7AF` makes it **1** |

`EBP` provenance is the whole-listing filter for `EBP` in each body: `0088E320` writes it once
(`XOR EBP,EBP`, `0088E343`) and `0088E790` writes it once (`MOV EBP,0x1`, `0088E7AF`).

So `SetFireDamage` writes `task+38h` and `SetWaterDamage` writes `task+34h`.

### The two consumers

Both are `__thiscall(task, float dt)` and identical except for the fields.

```
0093c120: ...                              ; BSP_RepairTask_ApplyFireDamage, 0093C120..0093C20B
0093c126: CMP dword ptr [ESI + 0x24],0x4   ; priority 4
0093c12c: CALL 0x00424c40                  ; the settings singleton
0093c131: MOVSS XMM0,dword ptr [EAX + 0x3c8]
0093c18f: MOVSS XMM0,dword ptr [ESI + 0x34]   ; the timer it runs down
0093c1c3: MOVSS dword ptr [ESI + 0x40],XMM0   ; the field it clears at expiry
0093c1dd: FMUL float ptr [ESI + 0x2c]         ; damage per second
0093c1ea: FDIV float ptr [ESP + 0x4]          ; divided by the multiplier
```

```
0093c210: ...                              ; BSP_RepairTask_ApplyWaterDamage, 0093C210..0093C2FB
0093c216: CMP dword ptr [ESI + 0x24],0x3   ; priority 3
0093c21c: CALL 0x00424c40
0093c221: MOVSS XMM0,dword ptr [EAX + 0x3cc]
0093c27f: MOVSS XMM0,dword ptr [ESI + 0x38]   ; the timer it runs down
0093c2b3: MOVSS dword ptr [ESI + 0x3c],XMM0   ; the field it clears at expiry
0093c2cd: FMUL float ptr [ESI + 0x30]
0093c2da: FDIV float ptr [ESP + 0x4]
```

`0093C120` runs `+34h`, which `SetWaterDamage` writes, so it is the water step and `+3C8h`
`PumpRepairMultiplier` is the water divisor. `0093C210` runs `+38h`, which `SetFireDamage`
writes, so it is the fire step and `+3CCh` `FireRepairMultiplier` is the fire divisor. The
divisor pairing also agrees with the setters' own pairing: `0093A470` (fire) accumulates
`+38h` with `+3Ch` and `0093C210` clears `+3Ch`; `0093A4F0` (water) pairs `+34h` with `+40h`
and `0093C120` clears `+40h`.

**Verdict.** The authored key names are correct. `PumpRepairMultiplier` (`+3C8h`, the bilge
pump) divides water damage and `FireRepairMultiplier` (`+3CCh`) divides fire damage, exactly as
the shipped comments gloss them. Three things elsewhere are wrong and are listed in the
`corrections` block of `reports/gameplay_settings_tail.json`: the two Ghidra function names are
swapped, docs/UNIT_FIRE_AND_REPAIR.md's provisional `+34h` = fire is reversed, and the repair
priorities are fire `3`, water `4`, not fire `4`, water `3`. Both multipliers are `3` in the
installed file, so no shipped run behaves differently either way; the naming matters only when
one of them is retuned.

## 2. The four `58h` sub-objects: `WeaponHitAccuracy`

`00836EF0` (`00836EF0..00836F77`, `__fastcall(this)`, no calls) stores twenty-two floats:
`+0h` from `[00CE3D08]` = `100.0`, `+4h` from `[00CE386C]` = `200.0`, and `+8h`..`+54h` all from
`[00CE3800]` = `0.5`. `00424A55..00424A6D` runs it four times at `+240h`, `+298h`, `+2F0h`,
`+348h`.

`00836F80` (`00836F80..008376AB`, `__thiscall(this=sub-object, LuaObject* table)`) is the filler.
It reads eleven keys, each twice, with `00B67800` (field by name), `00B67720` (element by index,
`1` then `2`) and `00B66270` (`GetNumber`). The `PUSH 0x1` / `PUSH 0x2` before each `00B67720`
is what splits the record into two halves.

| offset | key | element | store | default |
| --- | --- | --- | --- | --- |
| `+00h` | `TargetReferenceSizes` | `[1]` | `00836FD3` | `100.0` |
| `+04h` | `TargetReferenceSizes` | `[2]` | `00837025` | `200.0` |
| `+08h` | `Accuracy_10percent_Range` | `[1]` | `00837077` | `0.5` |
| `+0Ch` | `Accuracy_20percent_Range` | `[1]` | `00837119` | `0.5` |
| `+10h` | `Accuracy_30percent_Range` | `[1]` | `008371BB` | `0.5` |
| `+14h` | `Accuracy_40percent_Range` | `[1]` | `0083725D` | `0.5` |
| `+18h` | `Accuracy_50percent_Range` | `[1]` | `008372FF` | `0.5` |
| `+1Ch` | `Accuracy_60percent_Range` | `[1]` | `008373A1` | `0.5` |
| `+20h` | `Accuracy_70percent_Range` | `[1]` | `00837443` | `0.5` |
| `+24h` | `Accuracy_80percent_Range` | `[1]` | `008374E5` | `0.5` |
| `+28h` | `Accuracy_90percent_Range` | `[1]` | `00837587` | `0.5` |
| `+2Ch` | `Accuracy_100percent_Range` | `[1]` | `00837629` | `0.5` |
| `+30h` | `Accuracy_10percent_Range` | `[2]` | `008370C8` | `0.5` |
| `+34h` | `Accuracy_20percent_Range` | `[2]` | `0083716A` | `0.5` |
| `+38h` | `Accuracy_30percent_Range` | `[2]` | `0083720C` | `0.5` |
| `+3Ch` | `Accuracy_40percent_Range` | `[2]` | `008372AE` | `0.5` |
| `+40h` | `Accuracy_50percent_Range` | `[2]` | `00837350` | `0.5` |
| `+44h` | `Accuracy_60percent_Range` | `[2]` | `008373F2` | `0.5` |
| `+48h` | `Accuracy_70percent_Range` | `[2]` | `00837494` | `0.5` |
| `+4Ch` | `Accuracy_80percent_Range` | `[2]` | `00837536` | `0.5` |
| `+50h` | `Accuracy_90percent_Range` | `[2]` | `008375D8` | `0.5` |
| `+54h` | `Accuracy_100percent_Range` | `[2]` | `0083767A` | `0.5` |

The key strings are `00D0A17C` and then `00D0A160` down to `00D0A064` in steps of `1Ch`.

### The loader block that fills them

`0083C795`..`0083C919`. The parent table is `WeaponHitAccuracy` (`00D0B344`, pushed at
`0083C795`); each sub-table is fetched with `00B67690` and passed to `00836F80`.

| site | key string | key | `this` |
| --- | --- | --- | --- |
| `0083C81A` | `00CE5454` | `Artillery` | `settings+240h` (`0083C814`) |
| `0083C86F` | `00CFA420` | `AA` | `settings+298h` (`0083C869`) |
| `0083C8C4` | `00CE544C` | `Torpedo` | `settings+2F0h` (`0083C8BE`) |
| `0083C919` | `00CFA700` | `DepthCharge` | `settings+348h` (`0083C913`) |

So the four sub-objects are **weapon categories, not ship categories and not the failure model**.
The installed `scripts/datatables/shipglobals.lua` line 148 authors the same four names under
`ShipGlobals["WeaponHitAccuracy"]` in the same order, and its Hungarian comments give the model:
the weapon's maximum range is divided into ten parts and each part carries a hit chance in `0..1`
that also depends on the target's size, with element `[1]` for a small target and `[2]` for a
large one; `TargetReferenceSizes` is the small and large reference size in metres. The installed
values are `Artillery {100, 250}`, `AA {80, 200}`, `Torpedo {100, 300}`, `DepthCharge {100, 200}`,
so no shipped category keeps the constructor's `200.0` default for the large size.

### Who reads them

`contract: unread`. No native consumer was located. The searches that came up empty are
`LEA ECX,[EAX+240h]` (`8D 88 40 02 00 00`, no matches), `MOVSS XMM0,[EAX+240h]`
(`F3 0F 10 80 40 02 00 00`, no matches) and `ADD EAX,240h` (`05 40 02 00 00`, two matches at
`004C6FEB` and `0062EA80`, neither near a settings read). The one `FLD [EAX+240h]` in the image,
`009FCE81`, is a false positive: its `EAX` comes from `0099C210`, which returns
`[00F8A30C] + kind*248h + 0Ch`, a different table. The consumer therefore reaches the sub-object
through a computed base, most likely `settings + 240h + category*58h`; it is expected on the gun
aiming or bot fire path, which other packets own.

## 3. The nine string reads of `0083B5E0`

`00B662B0 BSP_LuaObject_GetString` is `lua_tolstring(L, index, 0)` (`00B662B0..00B662C1`) and
returns the borrowed `char*`; the loader measures it with an inline `strlen` and copies it into a
stack NativeString before use. `00B685C0 BSP_LuaObject_ConstructStringOrDefault`
(`00B685C0..00B68622`) tests the reference's kind field `+4h` for `2` and the Lua type for `4`,
and assigns the fallback when either fails. A NativeString is `{u32 length; char* data}`, from the
`memcpy` argument order at `008418CE..008418D4` (`00BF7680(dst=[EBP+4], src=[EDI+4], n=[EBP])`).

| key | site | destination | consumer |
| --- | --- | --- | --- |
| `ShipAvoidance.RightOfWayValues[n][1]` | `0083BD7F` | vector at `settings+1DCh` | `0083A880` push, entry `{class index, float}` |
| `CollisionEffect` | `0083E016` | `settings+3A4h` | `00871BA0`, handle stored at `0083E08C` |
| `FailureName` | `0083E7C4` | `contract: unread` | kept at `[ESP+88h]` into the failure build |
| `SectionName` | `0083E8DA` | `contract: unread` | `00438E10` at `0083E956`, table `00E08138` |
| `SinkEffect` | `0083EAEC` | `settings+72Ch` | `00871BA0`, handle stored at `0083EB60` |
| `BulletEffect` | `008414A7` | shot record `+24h` | `00871BA0`, handle stored at `008414E6` |
| `ExplosionEffect` | `0084159A` | shot record `+28h` | `00871BA0`, handle stored at `008415D9` |
| `SplashEffect` | `0084168D` | shot record `+2Ch` | `00871BA0`, handle stored at `008416CF` |
| `Name` | `008418A4` | shot record `+0h` | copied into the record's NativeString |

The five `00871BA0` sites all assign through the same smart-pointer idiom: the returned handle is
dereferenced once (`MOV EAX,[EAX]`), compared with the old value, stored, the new one retained
through `[00CE221C]` and the old one released through `[00CE2220]` with a virtual destructor call
when the count reaches zero. The effect object itself is a contract.

**`RightOfWayValues`.** `0083BCCF` sets `EBX = ESI + 1DCh`, so the destination vector is at
`settings+1DCh`; `0083A880` reads its `+4h`, `+8h` and `+Ch` (the `+1E0h`, `+1E4h`, `+1E8h` the
constructor zeroes at `00424A3E`) and its `SAR ECX,3` at `0083A897` fixes the element stride at 8.
Each loop pass reads element `[1]` as a string and element `[2]` as a number (`0083BDF4`,
`00B66270`), then walks the 97-entry `char*` table at `00E0CD80` with `strcmp` (`00BF7FBF` at
`0083C675`) to turn the name into an index. On a match it fills `{index, value}` at `0083BE5C` and
`0083BE63` and pushes it. `CMP EDI,0x61` at `0083BE47` and `0083C68D` bounds the table at 97
entries; it is the same class-name table docs/LUA_BINDING_AI.md and
docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md walk.

**The free-camera-shot record.** `00841411` sets `EBP = [EDI+8] - 30h` after the emplace, so the
record is `30h` bytes and `EBP` is the element just appended. `EBP` provenance over
`00841100..008418A4` is three writes: `MOV EBP,[ESI+744h]` (`008412E2`), `XOR EBP,EBP`
(`008413AC`) and the pair `MOV EBP,[EDI+8]` / `ADD EBP,-0x30` (`008413EE`, `00841411`); the last
is the one live at all four string sites. `Name` is the only `00B685C0` site in the image, with
the empty string at `00CE3A0C` as its fallback.

## Host table

One row per native call site the reconstruction models.

| site | callee | name | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `00836FCE`, `00837020`, … `0083767A` (22) | `00B66270` | `read_number` | the element reference / none / float | none; the block is unconditional |
| `00836FAE`, `00836FFE`, … (22) | `00B67800` | folded into `read_number` | the table / out, key / field reference | none |
| `00836FC2`, `00837014`, … (22) | `00B67720` | folded into `read_number` | the field / out, index | none |
| `0083C81A`, `0083C86F`, `0083C8C4`, `0083C919` | `00836F80` | `load_weapon_hit_accuracy_profile_00836f80` | `settings+240h`/`+298h`/`+2F0h`/`+348h` / the sub-table / none | none |
| `00424A62` | `00836EF0` | `apply_weapon_hit_accuracy_defaults_00836ef0` | the sub-object / none / none | the `EBX` counter `3..0` |
| `0083E06B`, `0083EB45`, `008414CE`, `008415C1`, `008416B7` | `00871BA0` | `contract: unread` (`BSP_EffectHandle_AcquireByName`) | a handle slot / the name, `1` / the handle | `docs/GAMEPLAY_SETTINGS.md` effect block |
| `0083BE6C` | `0083A880` | `contract: unread`, vector push | `settings+1DCh` / the 8-byte entry / none | a class name matched in `00E0CD80` |
| `0083E956` | `00438E10` | `contract: unread` | a stack NativeString / `00E08138` slot | the `SectionName` block |
| `0093C12C`, `0093C21C` | `00424C40` | the settings singleton | none / none / the object | `task+24h` equals the step's priority |
| `0093C16A`, `0093C25A` | `008E6430` | `contract: unread`, difficulty modifier | none / `3`, the unit / float | `[00F88C30]` live, `[00E0C978]` set, `+ACh` set |
| `0093C1FA`, `0093C2EA` | `vtable[1ACh]` | `kEntityVtableSlotAddDamage` | the unit / the damage / none | the delta is above zero |

## Coverage

| routine | coverage |
| --- | --- |
| `00836EF0` | complete (`00836EF0..00836F77`) |
| `00836F80` | complete for the key-to-offset mapping (`00836F80..008376AB`); the SEH scope records and the NativeString temporaries are not modelled |
| `0093C120`, `0093C210` | complete (`0093C120..0093C20B`, `0093C210..0093C2FB`) |
| `0082203D` arm of `00821E80` | complete for selectors `0` and `1`; the default arm at `008220C2` is unread |
| `00939F90`, `00939FA0` | complete |
| `0093A470`, `0093A4F0` | partial: the field writes are covered; the trailing comparison against `unit+370h` and the calls `00983780` / `009832F0` are unread |
| `0088E320`, `0088E790` | partial: only the message fill and the selector are covered |
| `0083B5E0` | extended by `0083C795..0083C919` (the sub-object block) and the nine string sites; the rest is as docs/GAMEPLAY_SETTINGS.md leaves it |
| the `FailureName` / `SectionName` destinations | `contract: unread`; they sit in the failure descriptor build `0083E5D8..0083E9xx` |
| the `WeaponHitAccuracy` consumer | `contract: unread` |

## Follow-ups

| name | addresses | what is left |
| --- | --- | --- |
| `weapon_hit_accuracy_consumer` | 00424c40 | Find the reader of `settings+240h + category*58h` and confirm the bucket interpolation |
| `repair_task_name_swap` | 0093c120 0093c210 | Apply the two Ghidra renames and correct docs/UNIT_FIRE_AND_REPAIR.md's timer table and priorities |
| `failure_record_string_fields` | 0083e5d8 00438e10 | Where `FailureName` and `SectionName` land in the failure descriptor |

## Correction from docs/SHIP_AI_SETTINGS_BLOCK.md

Packet `cc_ai_settings_block` (main, after c42aeab7) read every gameplay-settings field the ship
AI reads (42 offsets with key, getter, loader site, fallback, installed value and reader) and
closes three items here: `008387B0` is the WeaponHitAccuracy consumer this doc recorded as unread
(`009F1BC0` calls it at `009F2D87` with kind 7, Torpedo, and stores the answer at `nested+12B4h`);
the installed values at `+1ECh` and `+210h` are both 1.5; and `settings+4h` is
`AvoidAllShipCollision`, set to 1 by the loader at `0083BCD5` with no key and overridden only by
`luaMW_NavigatorSetAvoidAllShipCollision` `008D0740` at `008D0852` (usn_09_leyte.lua, false). The
approach tuning block the ship AI reads is not a separate table: `[brain+0AB0h] = *(unit+73Ch)`,
whose `+0h..+18h` are the seven `ShipGlobals.AttackMoveDirector` keys copied from
`settings+160h..+178h` by `00822B70`.
