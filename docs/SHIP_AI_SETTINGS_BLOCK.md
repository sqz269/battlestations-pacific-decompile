# The gameplay settings the ship AI reads (packet `cc_ai_settings_block`)

Addresses: 00822B70, 008387B0, 00838530, 008D0740, 009EC770, 0081ED40, 009EAFC0, 009F0100;
read as evidence 00424A10, 00424C40, 0083B5E0, 00822C20, 00835BF0, 009F1160, 009EACA0, 009EAE20,
009EB660, 009EC280, 009EC7C0, 009ECA20, 009EF350, 009EF910, 009F0AD0, 009F0D20, 009F0EA0,
009F1420, 009F1BC0, 009F3670, 00A08460, 009D5DF0, 009D5E00, 00432650, 0087D7B0.

`docs/GAME_EXECUTABLE.md` milestone 2r follow-up 6 asked which fields of the gameplay tuning
singleton the ship AI reads, and named "the approach tuning block behind `009E7FC0` / `009E6E80`
/ `009E74D0` / `009E9190`" as unresolved. This packet answers both.

**Headline.** The approach tuning block is not a separate table. `tune` is
`[brain+0AB0h] = *(unit+73Ch)`, the per-unit navigator parameter block
`docs/UNIT_COMMANDED_SPEED.md` already describes, and its first seven floats are the
`ShipGlobals.AttackMoveDirector` sub-table copied field by field by
`00822B70 BSP_UnitInstance_ResetNavigatorParams`. Base offset `+160h`, stride `4`, seven keys.
The eighth float the approach reads, `tune+1Ch`, is not part of that copy and has no producer in
the image other than the constructor's `-1.0f`.

Names here are hypotheses built from the shipped Lua key strings. They are not recovered symbols.

## 1. The approach tuning block

`009F1160 BSP_ShipAi_BrainRecordConstruct` seeds three adjacent brain fields from the unit:

| site | store | source |
| --- | --- | --- |
| `009F11A4`..`009F11B7` | `brain+0AA8h` | the unit (`param_2`) |
| same | `brain+0AACh` | `[unit+538h]` |
| same | `brain+0AB0h` | `[unit+73Ch]` |

`unit+73Ch` is the navigator parameter block: `0081F283..0081F28C` in
`BSP_UnitVehicleBase_Construct` (`0081ED40`) allocates it and stores the pointer, and
`0081F4A7..0081F4B2` frees it. So every `tune+N` in `docs/SHIP_AI_APPROACH_UPDATE.md` and
`docs/SHIP_AI_RING_SCAN.md` is `*(unit+73Ch) + N`.

**The producer.** `00822B70` (`__thiscall(unit)(char reset)`, `RET 4`, body
`00822B70-00822C15`, complete) copies seven singles when `reset` is non-zero. The image reloads
the singleton for every field, so there are seven separate `CALL 00424C40` at
`00822B88`..`00822BFF`, and it writes `+04h` before `+00h`.

| `tune` | settings | key under `ShipGlobals` | loader default | installed | read | store |
| --- | --- | --- | --- | --- | --- | --- |
| `+00h` | `+160h` | `AttackMoveDirector.MyDamageWeight` | 10 | 10.0 | `00822BA7` | `00822BAD` |
| `+04h` | `+164h` | `AttackMoveDirector.IdealDistWeight` | 4 | 4.0 | `00822B8D` | `00822B99` |
| `+08h` | `+168h` | `AttackMoveDirector.NearbyEnemyWeight` | 3 | 3.0 | `00822BBA` | `00822BC0` |
| `+0Ch` | `+16Ch` | `AttackMoveDirector.NearbyEnemyReference` | 1000 | 1000.0 | `00822BCE` | `00822BD4` |
| `+10h` | `+170h` | `AttackMoveDirector.NearestMoveDirWeight` | 1 | 1.0 | `00822BE2` | `00822BE8` |
| `+14h` | `+174h` | `AttackMoveDirector.PrevMoveDirWeight` | 0.25 | 0.25 | `00822BF6` | `00822BFC` |
| `+18h` | `+178h` | `AttackMoveDirector.PrevMoveDirRange` | 1.0472 | `DEG(60)` | `00822C0A` | `00822C10` |

`00822B70` also sets `+20h` to 1 at `00822B84`. Its two call sites are `00822CCB` in
`BSP_UnitInstance_SEntityInit` (the live one) and `00835C65` in
`BSP_WeaponDirector_ResetCommandStage`, which passes a literal `0` and is a no-op.

Each key then lands where the approach code expects it, which is the check that the mapping is
right rather than coincidental:

| reader | expression (`docs/SHIP_AI_APPROACH_UPDATE.md`) | key it resolves to |
| --- | --- | --- |
| `009E81FA` | `slot+2Ch = slot+18h / max * tune+0h`, `slot+18h` from the expected-damage rating `009E5DA0` | `MyDamageWeight` |
| `009E7489` | the fourth weight passed to the slot scorer `009E6870` with the standoff range | `IdealDistWeight` |
| `009E784B` | `-tune+4h`, the penalty on a rejected slot | `IdealDistWeight` |
| `009E964E`/`009E9655` | `interp(0, 0, tune+0Ch, tune+8h, abs(traffic acc))` | `NearbyEnemyReference` (abscissa), `NearbyEnemyWeight` (ordinate) |
| `009E75F2` | `slot+34h = interp(0, tune+10h, pi, 0, abs(wrap(nested+11ECh - angle)))` | `NearestMoveDirWeight` |
| `009E7656`/`009E7660` | `slot+38h = interp(0, tune+14h * gain, tune+18h, 0, ...)` | `PrevMoveDirWeight` (weight), `PrevMoveDirRange` (span) |

**`tune+1Ch`, the standoff-range override, has no producer.** `009E6EC5` tests it for
`>= 0.0f` and uses it directly as `nested+11E4h`. The only writer found in the image is the
constructor's `-1.0f` at `0081F26E`; `00822B70` does not touch it. The sweep was every
`MOV r32,[r32+73Ch]` in the image (`8B ?? 3C 07 00 00`, 33 matches, listed under Coverage), and
the block's two property-system serializers `009D5DF0` and `009D5E00` (registered under the
literal `navigatorParams` at `00D094C4` by `00818920` and `0082007B`) are both a bare `RET 4`,
so nothing is read into the block from a scene or save archive either. Provisional: the
`009E6ECA` arm therefore looks unreachable, but a writer reached through a pointer this sweep
loses (the block passed to a callee) would not appear.

**The block's constructor seed.** `0081F214..0081F278` fills the same seven floats with the
loader's own fallbacks before any settings exist, so a unit built early still runs the authored
defaults:

| offset | constant | value | site |
| --- | --- | --- | --- |
| `+00h` | `00CE38B8` | 10.0 | `0081F21C` |
| `+04h` | `00CE3D34` | 4.0 | `0081F228` |
| `+08h` | `00CE3854` | 3.0 | `0081F235` |
| `+0Ch` | `00CE3804` | 1000.0 | `0081F25C` |
| `+10h` | `00D7A24C` | 1.0 | `0081F269` (`XMM1`, loaded at `0081F205`) |
| `+14h` | `00CE3868` | 0.25 | `0081F242` |
| `+18h` | `00D05AAC` | 1.0471976 | `0081F24F` |
| `+1Ch`, `+24h`, `+28h` | `00D7A260` | -1.0 | `0081F26E`, `0081F273`, `0081F278` |
| `+21h` | immediate | 1 | `0081F27D` |

## 2. The field table

Every gameplay-settings offset a ship AI routine reads. `loader site` is the getter `CALL` in
`0083B5E0`; `default` is the fallback carried in the call (`-` where the getter takes none);
`installed` is `scripts/datatables/shipglobals.lua` in the installation. `readers` are the
Ghidra functions that contain the read, with the dereferencing instruction.

| offset | key under `ShipGlobals` | getter | loader site | default | installed | readers |
| --- | --- | --- | --- | --- | --- | --- |
| `+004h` | none; see section 3 | immediate / `00B66250` | `0083BCD5` (loader), `008D0849` (binding) | - | 1, cleared by one mission | `009EC770` `009EC795`; `009EF350` `009EF389`; `009F0EA0` `009F107A`; `009F1420` `009F1B78` |
| `+160h`..`+178h` | `AttackMoveDirector.*` | `00B66330` | `0083C96C`..`0083CB24` | see section 1 | see section 1 | via `tune`, section 1 |
| `+190h` | `ShipAvoidance.CollectTimer.[1]` | `00B66270` | `0083B7AD` | - | 1 | `009F1160` `009F13E2` |
| `+194h` | `ShipAvoidance.CollectTimer.[2]` | `00B66270` | `0083B810` | - | 2 | `009EACA0` `009EAD58`; `009EB660` `009EBEE3`; `009F0D20` `009F0DCD`; `009F1160` `009F13E2` |
| `+198h` | `ShipAvoidance.CollectDist` | `00B66270` | `0083B85D` | - | 450 | `009F1420` `009F19BD` |
| `+19Ch` | `ShipAvoidance.CollectHitTime` | `00B66270` | `0083B899` | - | 10 | `009F1420` `009F19AE` |
| `+1A0h` | `ShipAvoidance.NearbyShip_ArriveTimeMin` | `00B66330` | `0083B8DF` | 3 | 2.0 | `009EAFC0` `009EB18C` (`LEA`, floor through `00415550`) |
| `+1A4h` | `ShipAvoidance.NearbyShip_ArriveDistMin` | `00B66330` | `0083B925` | 0.25 | 0.4 | `009EAFC0` `009EB0BA` |
| `+1A8h` | `ShipAvoidance.NearbyShip_PosSpeedCorrig` | `00B66330` | `0083B96B` | 2 | 0.5 | `009EAE20` `009EAECA` |
| `+1ACh` | `ShipAvoidance.NearbyShip_EstPos_DistLimitMul` | `00B66330` | `0083B9B1` | 0.7 | 4.0 | `009EAFC0` `009EB340` |
| `+1B0h` | `ShipAvoidance.NearbyShip_EstPos_MinShipLength` | `00B66330` | `0083B9F7` | 100 | 5 | `009EAFC0` `009EB334` (`LEA`, floor on `[unit+9C8h]` through `00415550`) |
| `+1B4h` | `ShipAvoidance.NearbyShip_EstPos_ShipLengthLimitMul` | `00B66330` | `0083BA3D` | 1.8 | 4.0 | `009EAFC0` `009EB350` |
| `+1B8h` | `ShipAvoidance.NearbyShip_MyMinSpdRatio` | `00B66330` | `0083BA83` | 0.6 | 0.2 | `009F0EA0` `009F0F6D` |
| `+1BCh` | `ShipAvoidance.NearbyShip_EstPos_ShipSpdMul` | `00B66330` | `0083BAC9` | 0.75 | 1.0 | `009EAFC0` `009EB20F` |
| `+1C0h` | `ShipAvoidance.NearbyShip_EstPos_SizeDecMul` | `00B66330` | `0083BB0F` | 0.3 | 0.3 | `009EAFC0` `009EB28F` |
| `+1C4h` | `ShipAvoidance.NearbyShip_EstPos_SizeDecMin` | `00B66330` | `0083BB55` | 0.1 | 0.2 | `009EAFC0` `009EB288` (`LEA`, cap through `00415510`) |
| `+1C8h` | `ShipAvoidance.NearbyShip_NextCornerReachDistAddOn` | `00B66330` | `0083BB9B` | 100 | 100 | `009F0100` `009F06DA` |
| `+1CCh` | `ShipAvoidance.NearbyShip_MovePathLineCheckThreshold` | `00B66330` | `0083BBE1` | 200 | 150 | `009F0100` `009F072F`, `009F098B` |
| `+1D0h` | `ShipAvoidance.NearbyShip_GoAwaySpdAdd` | `00B66330` | `0083BCB3` | 3 | 2.0 | `009EAFC0` `009EB167` |
| `+1D4h` | `ShipAvoidance.NearbyShip_WayClearCheckTime` | `00B66330` | `0083BC27` | 0.5 | 0.25 | `009EF910` `009EF948` |
| `+1D8h` | `ShipAvoidance.HitDetector_LastHitDistAddOn` | `00B66330` | `0083BC6D` | 30 | 50 | `009EB660` `009EB686` |
| `+1ECh` | `TorpedoAvoidance.CollectTimer.[1]` | `00B66270` | `0083BF3F` | - | **1.5** | `009F1160` `009F13B0` |
| `+1F0h` | `TorpedoAvoidance.CollectTimer.[2]` | `00B66270` | `0083BFA8` | - | 2 | `009EACA0` `009EAD4E`; `009F0AD0` `009F0BAC` |
| `+1F4h` | `LandAvoidance.CheckMovePosZoneTime.[1]` | `00B66270` | `0083C032` | - | 2.5 | `009ECA20` `009ECEE3` |
| `+1FCh` | `LandAvoidance.CheckShipPosZoneTime.[1]` | `00B66270` | `0083C104` | - | 2.5 | `009ECA20` `009ECB65` |
| `+204h` | `LandAvoidance.CheckTravelZoneTime.[1]` | `00B66270` | `0083C1D6` | - | 3 | `009ECA20` `009ED01F` |
| `+208h` | `LandAvoidance.CheckTravelZoneTime.[2]` | `00B66270` | `0083C23F` | - | 4 | `009ECA20` `009ED02E` |
| `+214h` | `LandAvoidance.YTurnDirDiff.[1]` | `00B66330` | `0083C393` | 1.8 | 1.8 | `009EF910` `009F007F` |
| `+218h` | `LandAvoidance.YTurnDirDiff.[2]` | `00B66330` | `0083C40C` | 2.1 | 2.1 | `009EF910` `009F0089` |
| `+240h` | `WeaponHitAccuracy.Torpedo` | `00836F80` | `0083C8C4` | see `docs/GAMEPLAY_SETTINGS_TAIL.md` | see there | `009F1BC0` `009F2D87` through `008387B0`, kind 7 |
| `+3B0h` | `WaterTickDamage` | `00B66330` | `0083E1B3` | 0 | 100 | `00A08460` `00A09629` |
| `+4D4h` | `SubAttack.SubmarineLostTime` | `00B66270` | `0083F79C` | - | 30 | `009F3670` `009F36A4`, `009F36E9` |
| `+6CCh` | `Navigator.AutoThrust.HdgDiffValueMin_Slow` | `00B66270` | `0083CC2C` | - | `DEG(25)` | `009EC7C0` `009EC850` |
| `+6D0h` | `Navigator.AutoThrust.HdgDiffValueMax_Slow` | `00B66270` | `0083CC6E` | - | `DEG(75)` | `009EC7C0` `009EC840` |
| `+6D4h` | `Navigator.AutoThrust.ThrustMin_Slow` | `00B66270` | `0083CCB0` | - | 0.5 | `009EC7C0` `009EC8DA` |
| `+6E0h` | `Navigator.AutoThrust.HdgDiffValueMin_Fast` | `00B66270` | `0083CD76` | - | `DEG(45)` | `009EC7C0` `009EC8B8` |
| `+6E4h` | `Navigator.AutoThrust.HdgDiffValueMax_Fast` | `00B66270` | `0083CDB8` | - | `DEG(90)` | `009EC7C0` `009EC8A8` |
| `+6E8h` | `Navigator.AutoThrust.ThrustMin_Fast` | `00B66270` | `0083CDFA` | - | 0.75 | `009EC7C0` `009EC910` |
| `+6ECh` | `Navigator.AutoThrust.HdgDiffDangerMul` | `00B66270` | `0083CE3C` | - | 6.0 | `009EC7C0` `009EC7D4` |
| `+6F0h` | `Navigator.PathFinderParams.LengthModifier_DirDiffMin` | `00B66330` | `0083D4BF` | 0.261799 | `DEG(15)` | `009EC280` `009EC310` |
| `+6F4h` | `Navigator.PathFinderParams.LengthModifier_DirDiffMax` | `00B66330` | `0083D50B` | 1.5708 | `DEG(80)` | `009EC280` `009EC323` |
| `+6F8h` | `Navigator.PathFinderParams.LengthModifier_LengthAddon` | `00B66330` | `0083D557` | 1500 | 1200 | `009EC280` `009EC336` |

Two loader keys sit inside the same blocks but are never read by the ship AI in this sweep:
`+1F8h`/`+200h` (the `[2]` halves of `CheckMovePosZoneTime` and `CheckShipPosZoneTime`,
`0083C09B` / `0083C16D`) and `+20Ch`/`+210h` (`LandAvoidance.CollectTimer`, `0083C2AB` /
`0083C31A`). They are in the loader-order table because a settings host must still fill them.

**How the `+180h` base rows were read.** `009EAFC0` and `009F0100` do not address the singleton
directly. Both do `MOV reg,EAX` then `ADD reg,180h` right after the getter call
(`009EB081`/`009EB087`, `009F02CF`/`009F02E6`) and then use eight-bit displacements, so
`[EDI+24h]` is `+1A4h`. The whole listing of each function was filtered for `EDI`
(`docs/WORKER_VERIFICATION_CHECKLIST.md` rule 8) rather than reading forward from the call:

- `009EAFC0`: `EDI` is written once, at `009EB081`/`009EB087`, and the only `POP EDI` before
  `009EB334` is `009EB2CC`, which is on the early-return path `009EB2B7 JC 009EB308` skips.
  Every `[EDI+disp]` listed above is therefore in the singleton epoch.
- `009F0100`: `EDI` is set at `009F02CF`/`009F02E6`, spilled to `[ESP+40h]` at `009F02FD`,
  **redefined at `009F07C6` (`MOV EDI,[EBP]`, a neighbour record)** and restored from the spill
  at `009F09DC` at the end of each loop iteration. Only `009F06DA` and `009F072F` are claimed
  from `EDI`; `009F098B`/`009F09A0` are claimed from `ECX` reloaded from the spill at
  `009F0954`. The reads at `009F0854`, `009F085F` and `009F08BA` are in the neighbour-record
  epoch and are **not** settings reads.

The gameplay-settings singleton is the only tuning singleton the ship AI reads. The global
config singleton `00432650` (`Globals.lua`, loader `0087D7B0 BSP_GlobalConfig_LoadFromLuaGlobals`)
has 42 callers and none of them is in `009D0000..009F5000`.

## 3. `settings+4h`, the `AvoidAllShipCollision` gate

`+4h` is one of the offsets `docs/GAMEPLAY_SETTINGS.md` lists as "consumed, no key because the
loader does not write them directly". It has two producers.

**The default is on.** `0083BCD5 MOV byte ptr [ESI+4],1`, on the loader's straight-line path
between the last `ShipAvoidance` key (`+1D0h`, stored at `0083BCB8`, so `ESI` is the settings
object) and the `RightOfWayValues` vector build at `0083BCCF`. It is an immediate store with no
Lua key, which is why the key sweep did not list it. It is the only other store to `+4h` in the
loader's `0083B5E0..00842951`.

**The override.** `008D0740`, the binding whose failure string is
`luaMW_NavigatorSetAvoidAllShipCollision`, reads the Lua boolean with `00B66250` at `008D0849`
and stores it at `008D0852` (`MOV byte ptr [EDI+4],AL`, `EDI` from `00424C40` at `008D0840`). In
the shipped data only `scripts/missions/COTP-USN/usn_09_leyte.lua` line 560 calls it, with
`false`, so ship-to-ship collision avoidance is enabled everywhere except that mission after the
call.

The constructor `00424A10` never writes `+4h`: its first store is the vtable at `00424A34` and
its next is `+1E0h` at `00424A3E`, with no block clear in `00424A10..00424C15`. The byte is
therefore whatever `00BF681B` returned until the loader's tail-called run reaches `0083BCD5`,
which is inside the same constructor call chain (`00424BFC` tail-calls the loader), so no reader
can observe the uninitialised state.

Four ship AI routines read it. Three take an early return when it is zero; `009F1420` turns it
into a side filter:

| reader | site | use |
| --- | --- | --- |
| `009EC770` | `009EC795` | `JZ 009EC7B9`, which is `XOR AL,AL / RET 4` |
| `009EF350` | `009EF389` | `JZ 009EF3A7` |
| `009F0EA0` | `009F107A` | `JZ 009F109B` |
| `009F1420` | `009F1B78` | `SETNZ CL` then `LEA ECX,[ECX*4-1]` into `brain+3F8h`: `3` when set, `-1` when clear |

`009EC770` (`__thiscall(record)(int side)`, `RET 4`, body `009EC770-009EC7BE`) is the predicate
that consumes the filter. It returns false when `(int)record+3F0h` is negative (`009EC773`, the
`-1` the filter writes), false when the byte at `0080E160(record+3FCh)+241h` is zero
(`009EC787`), false when `settings+4h` is zero (`009EC795`), and otherwise accepts when either
side is `3` or the two sides are equal (`009EC79F`..`009EC7B1`). `009F1420` writes the field at
`+3F8h` on a base eight bytes below `009EC770`'s, the shift `docs/SHIP_AI_APPROACH_UPDATE.md`
records for this object.

## 4. `008387B0`, the `WeaponHitAccuracy` consumer

`docs/GAMEPLAY_SETTINGS_TAIL.md` records the four `58h` sub-objects at `+240h`, `+298h`, `+2F0h`
and `+348h` and says their consumer is `contract: unread`, reached "through a computed base,
most likely `settings + 240h + category*58h`". The consumer is `008387B0`, and the base is
computed but not by that formula: it is a four-way branch on an integer weapon kind.

`008387B0`, `__thiscall(settings)(int kind, float range, float scale, float target_size)`,
`RET 10h`, body `008387B0-00838897`, complete:

```
008387B4..008387C6   kind in {2,3,4,6} -> ECX += 240h   Artillery
008387F4..008387FC   kind in {1,5}     -> ECX += 298h   AA
0083882A..0083882D   kind == 7         -> ECX += 2F0h   Torpedo
0083885B..00838863   kind in {8,9}     -> ECX += 348h   DepthCharge
00838891             otherwise         -> FLD [ESP+0Ch]; RET 10h   (scale unchanged)
```

Each arm is the same five instructions: push `target_size` then `range`, call
`00838530(profile, range, target_size)`, and `FMUL` the answer by `scale`. The argument roles
come from the stack, not from a call site: the callee cleans eight bytes (`00838530` is
`__thiscall` with two float arguments), which leaves `[ESP+0Ch]` at the `FMUL` equal to the
third argument.

`00838530` (body `00838530-008386E5`, read only as far as the packet needed) blends the
small-target row `+8h..+2Ch` and the large-target row `+30h..+54h` using `00419010` over the two
`TargetReferenceSizes` at `+0h`/`+4h`, then walks the ten bucket values from `+8h` with a
counter capped at `0Ah` (`008385A1 CMP ECX,0xA`). Coverage: partial, the interpolation tail
`008385C4..008386E5` is not transcribed here.

The ship AI's call is `009F2D87` in `BSP_ShipAi_ApproachFrameState` with `PUSH 7` at `009F2D85`,
so the ship AI asks for the **Torpedo** profile at range `[nested+1280h]`, and stores the answer
at `nested+12B4h` (`009F2D8C`). That is the field `009E6E80`'s tail reads when it clamps the
standoff range (`docs/SHIP_AI_APPROACH_UPDATE.md`, "when `nested+12BAh` is set,
`nested+12B4h > 0` and `0080DF40(unit) > 0`"). The other caller is `00901BA0`, on the gun-bot
path, which the tail doc expected.

## 5. The loader order for a settings host

`src/ship_ai_settings_block.cpp` carries the same table as data
(`ship_ai_settings_keys`), ordered by the getter call site so a host fills the object in the
order the image does. The order is not offset order: `+1D0h` is loaded after `+1D4h`/`+1D8h`
(`0083BCB3` against `0083BC27`/`0083BC6D`), and the whole `AttackMoveDirector` block at
`+160h`..`+178h` is loaded after every avoidance block, at `0083C96C`..`0083CB24`.

## Host table

Steps 1 to 7 are one run of `0083B5E0` at process start; step 8 runs per unit; step 9 is the
mission script and may never run.

| # | host method | address | native | note |
| --- | --- | --- | --- | --- |
| 1 | `open_script_state_0083b60e` | `0083B60E` | `00B66BD0` | construct the Lua state owner in the loader's own frame |
| 1b | (same) | `0083B625` | `00B6A020` | open it with `41h` |
| 2 | `run_script_0083b672` | `0083B672` | `00B69D40` | `Scripts\global\luaMW_init.lua`, `1Dh` characters |
| 3 | `run_script_0083b6e6` | `0083B6E6` | `00B69D40` | `Scripts\datatables\ShipGlobals.lua`, `22h` characters |
| 4 | `select_global_table_0083b73d` | `0083B721` | `00B67980` | take the globals table |
| 4b | (same) | `0083B73D` | `00B67800` | select `ShipGlobals` (`00D0B670`) |
| 5 | `enter_subtable_00b67690` | `0083C7F3` | `00B67690` | one of the ten sub-table entries; this one is `WeaponHitAccuracy.Artillery` |
| 6a | `read_number_00b66270` | `0083B7AD` | `00B66270` | no fallback; the first row of the key table |
| 6b | `read_float_or_default_00b66330` | `0083B8DF` | `00B66330` | fallback pushed with `FSTP [ESP]` from `.rdata` |
| 6c | array element | `0083B79E` | `00B67720` | 1-based index for the `[1]`/`[2]` keys |
| 6d | profile filler | `0083C81A` | `00836F80` | one `WeaponHitAccuracy` sub-object |
| 6e | the unkeyed gate | `0083BCD5` | (immediate) | `settings+4h = 1`, section 3 |
| 7 | `close_script_state_00842937` | `00842937` | `00B669A0` | close the state |
| 8 | `reset_navigator_params_00822ccb` | `00822CCB` | `00822B70` | per unit, in `BSP_UnitInstance_SEntityInit` |
| 8b | (no-op call site) | `00835C65` | `00822B70` | `BSP_WeaponDirector_ResetCommandStage` passes a literal `0` |
| 9 | `set_avoid_all_ship_collision_008d0852` | `008D0849` | `00B66250` | the Lua boolean the binding stores at `008D0852` |

## Coverage

| routine | coverage |
| --- | --- |
| `00822B70` | complete (`00822B70-00822C15`), reconstructed and build-tested |
| `008387B0` | complete (`008387B0-00838897`); the category dispatch is reconstructed, the product is described but not reimplemented because `00838530` is partial |
| `00838530` | partial: `00838530-008385C2` read (the size blend and the bucket walk); `008385C4-008386E5` not transcribed |
| `008D0740` | partial: only the settings store `008D0840..008D0852`; the binding's Lua frame handling is not read |
| `009EC770` | complete (`009EC770-009EC7BE`); the projection covers the side arm `009EC79B-009EC7BE` only and is marked `partial_projection` because the two gates before it need a host |
| `009EAFC0` | not reconstructed: read only for its `EDI` epochs and its nine settings reads |
| `009F0100` | not reconstructed: read only for its `EDI`/`[ESP+40h]` epochs and its three settings reads |
| `0081ED40` | partial: only the navigator-block seed `0081F214-0081F285` |
| `0083B5E0` | not re-read. The loader sites and defaults come from `docs/GAMEPLAY_SETTINGS.md`'s sweep; every site this doc cites was re-checked individually for its callee and its store (`local/loader_sites.py`) |
| the settings sweep | the 18 segment-67 callers of `00424C40` plus `00A08460`, every `CALL 00424C40` in each, 50 call sites; 7 that carry the pointer into another register were resolved by hand |
| `tune+1Ch` producer search | all 33 `MOV r32,[r32+73Ch]` sites in `.text`; a write through a pointer passed to a callee would be missed |

## Corrections

| Was | Is | Evidence |
| --- | --- | --- |
| `docs/SHIP_AI_APPROACH_UPDATE.md` line 408 and `docs/SHIP_AI_RING_SCAN.md` line 12: "`tune = [brain+0AB0h]` ... has no producer" | `tune` is `*(unit+73Ch)`, the navigator parameter block, and its `+0h`..`+18h` are `ShipGlobals.AttackMoveDirector` copied by `00822B70`. The header's role names for those seven fields can be replaced by the Lua names. | `009F11A4`..`009F11B7`; `00822B8D`..`00822C10`; the six reader-to-key matches in section 1 |
| `docs/SHIP_AI_APPROACH_UPDATE.md`: `nested+12B4h` is "written by routines here and read mostly by the unread slot scorers" | Its producer is the settings: `009F2D87` calls `008387B0` with kind 7, the `WeaponHitAccuracy.Torpedo` profile, and stores the answer at `009F2D8C`. | `009F2D62`..`009F2D8C`; `0083882A`..`00838858` |
| `docs/GAMEPLAY_SETTINGS_TAIL.md` "Who reads them": `contract: unread`, no native consumer located for the four `58h` sub-objects | `008387B0` is the accessor, with `ADD ECX,240h/298h/2F0h/348h` selected by weapon kind. Callers `009F1BC0` (ship AI, kind 7) and `00901BA0` (gun bot). The doc's byte searches missed it because the base register is `ECX`, not `EAX` (`81 C1 40 02 00 00`). | `008387D3`, `00838809`, `0083883A`, `00838870`; `009F2D87` |
| `docs/GAMEPLAY_SETTINGS.md` key table `+1ECh` `TorpedoAvoidance.CollectTimer.[1]` installed `1` | Installed is **1.5**. The `*` mark means the key name appears three times in the file; the row took `ShipAvoidance`'s value. `shipglobals.lua` line 279 is `TorpedoAvoidance.CollectTimer = { 1.5, 2 }`. | `scripts/datatables/shipglobals.lua` lines 243, 279, 285 |
| `docs/GAMEPLAY_SETTINGS.md` key table `+210h` `LandAvoidance.CollectTimer.[2]` installed `2` | Installed is **1.5**. Same cause: `shipglobals.lua` line 285 is `LandAvoidance.CollectTimer = { 1, 1.5 }`. | same |
| `docs/GAMEPLAY_SETTINGS.md` consumer survey: `+1A0h`, `+1A4h`, `+1ACh`, `+1B0h`, `+1B4h`, `+1BCh`, `+1C0h`, `+1C4h`, `+1C8h`, `+1CCh`, `+1D0h`, `+4D4h`, `+6D0h`, `+6E4h` have no consumer | All fourteen are read by the ship AI. The survey drops a register when it is redefined, and `ADD reg,180h` is a redefinition, so the `009EAFC0`/`009F0100` base idiom was invisible to it; `+6D0h`/`+6E4h`/`+4D4h` come through a second `MOV reg,EAX` copy. | section 2's readers column |
| `docs/GAMEPLAY_SETTINGS.md` consumer survey lists `+004h` among the "26 consumed offsets [with] no key because the loader does not write them directly" | The loader does write it, directly: `0083BCD5 MOV byte ptr [ESI+4],1`. It has no *key*, which is a different thing, and the default it establishes (avoid every ship, not only own-side) is what three of its four readers need to do anything at all. | `0083BCD5`, between the `+1D0h` store at `0083BCB8` and the `+1DCh` vector build at `0083BCCF` |
| `docs/GAME_EXECUTABLE.md` milestone 2r follow-up 6 groups `settings+588h` with the ship AI block | `+588h` `Physics.TBoatNyomatekSzorzo` is not a ship AI read. Its only reader is `BSP_UnitController_ApplyShipForces` at `00937592`, the force integrator in segment 61. The other five offsets in that list are ship AI reads. | `00937440` sweep: `call@0093757A read@00937592 +588h` |

## Follow-up packets

1. **`ship_ai_nearby_ship_estimate`**, `009EAFC0` whole (`009EAFC0-009EB651`). Nine of the
   `ShipAvoidance.NearbyShip_*` keys are consumed there and the Hungarian comments in
   `shipglobals.lua` lines 247..260 give the intended model for each, so the routine is unusually
   well specified from the outside. Not attempted here: this packet only established the `EDI`
   epochs.
2. **`ship_ai_path_neighbour_filter`**, `009F0100` whole (`009F0100-009F0ACD`). It holds the
   settings base in `[ESP+40h]` across a loop whose body redefines `EDI`, so the remaining
   `NearbyShip` reads need the same epoch discipline. `009D8860` is its per-record helper.
3. **`weapon_hit_accuracy_evaluate`**, `00838530` whole. The bucket interpolation
   `008385C4..008386E5` is the actual hit-probability curve and is what makes
   `nested+12B4h` a number rather than a placeholder. `00419010` is its blend helper.
4. **`navigator_params_range_override`**, the producer of `*(unit+73Ch)+1Ch`. If there is none,
   `009E6E80`'s `009E6ECA` arm is dead and the approach reconstruction can say so; the sweep here
   covers only direct `73Ch` displacements.
5. **`settings_unkeyed_stores`**, the other immediate stores `0083B5E0` makes into the
   singleton. `0083BCD5` was found by grepping the loader listing for `[ESI + 0x4]`; the key
   sweep in `docs/GAMEPLAY_SETTINGS.md` only followed stores whose source is a getter result, so
   every unkeyed immediate store in `0083B5E0..00842951` is still unenumerated. Some of the
   twenty-six "consumed, no key" offsets are probably these.

## no_ghidra_function

none. Every routine this packet cites is the start of a Ghidra function, including the two
`RET 4` serializer stubs `009D5DF0` and `009D5E00`.

## What `bsp_game.exe` does with this today

The executable implements exactly one host from this packet,
`navigator_params_reset_from_tuning_00822b70` in `src/game_hosts_commands.cpp`, and it is wired
to the **no-op** call site `00835C65`, which passes a literal `0` (`00835C63 PUSH 0`). The live
site `00822CCB` pushes `1` (`00822CBA PUSH 1`) and is not modelled. So the executable never
performs the tuning copy, and anything asking it for `tune` would get the constructor seed.

That is harmless today only by coincidence: the constructor seeds, the loader fallbacks and the
installed `AttackMoveDirector` values are the same seven numbers. A mission that authored
different `AttackMoveDirector` values would diverge.

Two runs were attempted and neither reached a mission frame, so no run log supports or
contradicts anything above. Without `--game-root` the run stops at Direct3D device creation
(`device_hr=0x80004005`, `frames_presented=0`), with only the thirteen `StartupHost` methods
called. With `--game-root` pointed at the installation it reaches Phase 5
`load_game_settings 008D8190`, reads the user's `options.txt`, and then takes an access violation
(`0xC0000005`) in code this packet did not touch. Both logs are in `local/scratch/`.

## Uncertainties

- No run-time evidence for any claim here; see the section above for why.
- The loader defaults and installed values for rows this packet did not re-read individually come
  from `docs/GAMEPLAY_SETTINGS.md`'s script sweep of `0083B5E0`. Every loader site cited here was
  re-checked for its callee and its store; the fallback constants were not re-decoded except the
  seven in section 1, which were read from `.rdata`.
- `UnitNavigatorParamsBlock` stops at `+2Ch`. The allocation size is unread, so fields above
  `+28h` are unknown rather than absent.
- `009EC770`'s name `BSP_ShipAi_ObstacleSideAccepted` is a hypothesis from its three gates and
  its single caller's argument (`BSP_ShipAi_RefreshTurnClearance`, `009EF910`), not from a
  string.
- `0083BCD5` is claimed as unconditional from its position on the loader's straight-line key
  sequence, not from a control-flow reconstruction of a `6412`-instruction routine.
- The `+240h` row in section 2's field table records a call, not a dereference: the ship AI never
  loads a float from the sub-object itself, it hands the singleton to `008387B0`.
