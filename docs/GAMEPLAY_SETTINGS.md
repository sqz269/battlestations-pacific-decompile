# The gameplay tuning settings singleton (packet `cc2_gameplay_settings`)

Addresses: 00424A10, 00424C40, 0083B5E0; read as contracts 00B67980, 00B67800, 00B67720,
00B67690, 00B67700, 00B66270, 00B66290, 00B662B0, 00B66250, 00B66330, 00B662F0, 00B66380,
00B66BD0, 00B6A020, 00B69D40, 00B669A0, 00BF681B, 00BF7CD1, 00415350, 00BD0C30, 00BF7420.

The object the docs call "the settings singleton", "the game settings" or "`cfg`" is a `76Ch`
byte block behind `00424C40`, filled by a `6412`-instruction Lua-driven loader. This packet
reads the constructor in full, sweeps the loader's listing by script for every store it makes
into the object, and sweeps every caller of the getter for the offsets it reads back. The
result is the key table below: `212` of the object's offsets now have a Lua key, a type, the
loader's own fallback and the value the installed script ships, and `154` offsets have named
consumers.

Descriptive names here are hypotheses built from the shipped Lua key strings. They are not
recovered symbols.

## The object

`00424C40 BSP_GameSettings_GetSingleton` is a lazy singleton over `[00F8753C]`
(`00424C55` tests it, `00424CC4` stores it). On the miss path it takes the lifetime manager
from `00415350`, allocates through `operator_new` with `PUSH 76Ch` at `00424C9F`, runs the
constructor at `00424CB6` and registers the instance for destruction with `00BD0C30` at
`00424CD7`. `76Ch` is therefore the object's size, from the allocation, not from the last
field seen.

`00424A10` is the constructor. The ledger had it tagged `CG_array_ctor_helper_00424a10`,
which is wrong: it takes `this` in `ECX`, writes a vtable pointer (`00424A34`,
`[ESI] = 00CE3994`) and ends by tail-calling the loader with `ECX` still holding the object
(`00424BF5`/`00424BFC`). Its own writes are:

| site | what |
| --- | --- |
| `00424A34` | vtable `00CE3994` |
| `00424A3E`..`00424A4A` | `+1E0h`, `+1E4h`, `+1E8h` zeroed |
| `00424A55`..`00424A6D` | four `58h` sub-objects constructed by `00836EF0` at `+240h`, `+298h`, `+2F0h`, `+348h` (`EBX` counts `3..0`, `EBP` steps `58h`) |
| `00424A6F`..`00424A87` | `+3A4h`, `+3A8h`, `+3E8h`, `+3ECh`, `+3F0h` zeroed |
| `00424A8D`..`00424B05` | `+598h`..`+5B8h` floats: `1.0`, `1.0`, `1000.0`, `1.0`, `0.5`, `1.2`, `2000000.0`, `1.5`, `150.0` |
| `00424B0D`..`00424B2B` | `+664h`, `+668h`, `+66Ch`, `+674h`, `+678h`, `+67Ch` zeroed |
| `00424B31`..`00424BBA` | `+688h` byte `1`, then `+68Ch`..`+6B0h` floats `8.0`, `9.0`, `0.04`, `0.01`, `-1.0`, `1.0`, `1.5`, `0.0166667`, and `+6B4h` int `10` |
| `00424B39`..`00424B45` | `+6B8h`, `+6BCh`, `+6C0h` zeroed |
| `00424BD2`..`00424BDE` | `'eh_vector_constructor_iterator'(this+72Ch, size 4, count 2, 00415680, 0041DE40)`: two four-byte elements at `+72Ch` and `+730h` |
| `00424BE3`..`00424BEF` | `+740h`, `+744h`, `+748h` zeroed |

Everything in `+598h`..`+5B8h` and `+688h`..`+6B4h` is written again by the loader, so those
constructor values are a pre-load state, not what the game runs with. They are not identical
to the loader's own fallbacks: the constructor puts `1000.0` in `+5A0h` and the loader's
fallback for the same field (`DOFParams.Range2`) is `1.0`. The zeroed groups `+664h`..`+67Ch`
and `+6B8h`..`+6C0h` are never loaded and keep the constructor's zeros; `+664h`, `+668h`,
`+674h`, `+678h` and `+684h` are read back by `FUN_00935540`.

## The loader

`0083B5E0 BSP_GameSettings_LoadFromLuaGlobals` is the loader, and the constructor's tail call
at `00424BFC` is its only caller. Its body is `0083B5E0..00842951`. It is a self-contained
script run, not a reader over an already-loaded table:

1. `0083B60E` constructs a Lua state owner in its own frame (`00B66BD0`) and `0083B625` opens
   it with `00B6A020(41h)`.
2. `0083B63A` and `0083B6AE` build two script paths into a native string (`0041DD40` resize,
   `00BF7680` copy) of lengths `1Dh` and `22h`, and `0083B672`/`0083B6E6` run each through
   `00B69D40 BSP_LuaStateOwner_RunScriptWithOverrides`. The first path's literal is at
   `00CE7CBC` and the second's at `00D0B67C`. Resolved after the merge (worker follow-up, landed
   by the integrator): the first is `Scripts\global\luaMW_init.lua` (29 characters, the `1Dh`
   pushed into the string resize at `0083B62C`, run at `0083B672`), the global bootstrap, not a
   datatable; the second is `Scripts\datatables\ShipGlobals.lua` (34 characters, `22h` at
   `0083B6A0`, run at `0083B6E6`), the datatable read back through one global table, which itself
   runs `scripts/datatables/ScriptOptions.lua` near its end (the `FailureDebug` gate). Both files
   exist in the installation; the shape matches the tuning singleton keyed from `PlaneGlobals.lua`.
3. `0083B721` takes the globals table (`00B67980`) and `0083B73D` selects the single global
   the loader reads, `"ShipGlobals"` at `00D0B670`.
4. Everything after that is the key table. The idiom per key is
   `00B67800 GetByName(parent, &slot, key)`, optionally `00B67720 GetByIndex(slot, &slot, n)`
   for the one-based array keys, then one typed getter, then the store into `[ESI + offset]`.
   Sub-tables are entered by assigning the child wrapper into the current-table slot with
   `00B67690 BSP_LuaObject_Assign` (`10` call sites); that is what makes
   `Navigator.TurnMultipliers` and `Navigator.AutoThrust` read against their own parent.
5. `00842937` closes the state (`00B669A0`).

The two `*OrDefault` getters carry the fallback in the call: `00B66330` takes a float pushed
with `FSTP [ESP]` after an `FLD` of a `.rdata` constant (`RET 4`, `FLD [ESP+8]` on the miss
path), `00B66380` and `00B662F0` take an integer push. The plain `00B66270`, `00B66290`,
`00B662B0` and `00B66250` take no fallback, so a missing key leaves whatever those return.

### How the table was read

`0083B5E0`'s listing was dumped in full and swept by `local/extract_settings.py` (kept out of
the commit; the method is what matters). The sweep gives every instruction an `ESP` delta, then
uses the routine's exception-state stores - `MOV byte ptr [ESP + 0CBCh/0CC0h/0CC4h], imm`, `806`
of them, all addressing one frame slot - as anchors and back-propagates each anchor's `ESP`
through the instructions before it. Without that, an unknown callee's stack cleanup shifts every
later `[ESP + disp]` and the parent-table slots stop matching; with it, all `258` getter call
sites resolve to a full key path. The key strings and float fallbacks are read from the PE's
sections on disk rather than through the bridge.

Each key was then looked up in the installed
`scripts/datatables/shipglobals.lua`. `254` of `258` are assigned there. The four that are not
run on the loader's fallback: `LandAvoidance.WayCheckerUpdateTime` (`1.5`),
`VizbeomlesDolgok.WaterSectionNum` (`6`), `Physics.TorpedoForcePower` (`2`) and
`Sounds.Ship.OrrHullamFreqLow` (`0.5`).

## The key table

`installed` is the value in the shipped `scripts/datatables/shipglobals.lua`. A `*` marks a key
the file assigns twice; for the `+3ACh`..`+3E0h` damage block the second assignment is inside
`if FailureDebug then` at line `746`, and the installed `scripts/datatables/ScriptOptions.lua`
line 1 sets `FailureDebug = false`, so the first block's values are the ones that run.
`consumers` are the functions that read the offset from the getter's result; the sweep is
listed under "The consumer survey" below.

| offset | key under `ShipGlobals` | type | loader default | installed | read at | consumers |
| --- | --- | --- | --- | --- | --- | --- |
| `+008h` | `Sinking.DragMultiplier` | float | 1 | 2.0 | 0083d6ab | `BSP_TickableGameEntity_TickAdvance` |
| `+00Ch` | `Sinking.DragMultiplierZ` | float | 1 | 0.3 | 0083d6f0 | `BSP_TickableGameEntity_TickAdvance` |
| `+010h` | `Sinking.LeakSize` | float | 1 | 1.0 | 0083d735 | `BSP_TickableGameEntity_TickAdvance` |
| `+014h` | `Sinking.DragPower` | float | 1 | 1 | 0083d666 | `BSP_TickableGameEntity_TickAdvance` |
| `+018h` | `Sinking.StartAngularVelocity` | float | 30 | -10 | 0083d605 | - |
| `+01Ch` | `Sinking.SectionNum` | int | 4 | 6 | 0083d5bc | `BSP_TickableGameEntity_TickAdvance` |
| `+020h` | `Debris.SplashSpeed.[1]` | float | 5 | 5 | 0083d7b8 | `BSP_GameDynamicsList_UpdateFrame` |
| `+024h` | `Debris.SplashSpeed.[2]` | float | 20 | 20 | 0083d82e | `BSP_GameDynamicsList_UpdateFrame` |
| `+028h` | `Debris.SplashFXID.[1]` | float | 9 | 52 | 0083d8a4 | `BSP_GameDynamicsList_UpdateFrame` |
| `+02Ch` | `Debris.SplashFXID.[2]` | float | 52 | 44 | 0083d91a | `BSP_GameDynamicsList_UpdateFrame` |
| `+030h` | `PlayerArtilleryThrow.AfterShot_FireTime` | float | 3 | 0.2 | 0083d998 | `FUN_0095dc40` |
| `+034h` | `PlayerArtilleryThrow.AfterShot_WaitTime` | float | 3 | 0.2 | 0083d9de | `FUN_0095dc40` |
| `+038h` | `PlayerArtilleryThrow.ThrowIncrementTime_HasTarget` | float | 12 | 2.0 | 0083da21 | `FUN_0095dc40` |
| `+03Ch` | `PlayerArtilleryThrow.ThrowIncrementTime_NoTarget` | float | 8 | 4.0 | 0083da96 | `FUN_0095dc40` |
| `+040h` | `PlayerArtilleryThrow.ThrowDecrementTime` | float | 10 | 2.0 | 0083db0b | `FUN_0095dc40` |
| `+044h` | `PipeSightParams.pipesight_enabled` | bool | - | true | 0083db94 | `FUN_0064dd30` |
| `+048h` | `PipeSightParams.blur_heavy_add` | float | 0.3 | 0.0 | 0083dca5 | `FUN_0064dd30` |
| `+04Ch` | `PipeSightParams.blur_medium_add` | float | 0.3 | 0.0 | 0083dceb | `FUN_0064dd30` |
| `+050h` | `PipeSightParams.blur_light_add` | float | 0.3 | 0.0 | 0083dd31 | `FUN_0064dd30` |
| `+054h` | `PipeSightParams.blur_aa_add` | float | 0.3 | 0.0 | 0083dd77 | - |
| `+058h` | `PipeSightParams.blur_spring` | float | 1 | 0.0 | 0083dc19 | `FUN_0064dd30` |
| `+05Ch` | `PipeSightParams.blur_drag` | float | 0.9 | 0.0 | 0083dbd7 | `FUN_0064dd30` |
| `+060h` | `PipeSightParams.blur_rate` | float | 0.4 | 0.0 | 0083dc5f | `FUN_0064dd30` |
| `+064h` | `PipeSightParams.zoom_heavy_add` | float | 0.3 | 0.05 | 0083de8b | `FUN_0064dd30` |
| `+068h` | `PipeSightParams.zoom_medium_add` | float | 0.3 | 0.0 | 0083ded1 | `FUN_0064dd30` |
| `+06Ch` | `PipeSightParams.zoom_light_add` | float | 0.3 | 0.0 | 0083df17 | `FUN_0064dd30` |
| `+070h` | `PipeSightParams.zoom_aa_add` | float | 0.3 | 0.0 | 0083df5d | `FUN_0064dd30` |
| `+074h` | `PipeSightParams.zoom_spring` | float | 1 | 1.0 | 0083ddff | `FUN_0064dd30` |
| `+078h` | `PipeSightParams.zoom_drag` | float | 0.9 | 0.5 | 0083ddbd | `FUN_0064dd30` |
| `+07Ch` | `PipeSightParams.zoom_rate` | float | 0.4 | 0.5 | 0083de45 | `FUN_0064dd30` |
| `+160h` | `AttackMoveDirector.MyDamageWeight` | float | 10 | 10.0 | 0083c96c | `BSP_UnitInstance_ResetNavigatorParams` |
| `+164h` | `AttackMoveDirector.IdealDistWeight` | float | 4 | 4.0 | 0083c9b8 | `BSP_UnitInstance_ResetNavigatorParams` |
| `+168h` | `AttackMoveDirector.NearbyEnemyWeight` | float | 3 | 3.0 | 0083ca01 | `BSP_UnitInstance_ResetNavigatorParams` |
| `+16Ch` | `AttackMoveDirector.NearbyEnemyReference` | float | 1000 | 1000.0 | 0083ca47 | `BSP_UnitInstance_ResetNavigatorParams` |
| `+170h` | `AttackMoveDirector.NearestMoveDirWeight` | float | 1 | 1.0 | 0083ca8c | `BSP_UnitInstance_ResetNavigatorParams` |
| `+174h` | `AttackMoveDirector.PrevMoveDirWeight` | float | 0.25 | 0.25 | 0083cad8 | `BSP_UnitInstance_ResetNavigatorParams` |
| `+178h` | `AttackMoveDirector.PrevMoveDirRange` | float | 1.0472 | DEG(60) | 0083cb24 | `BSP_UnitInstance_ResetNavigatorParams` |
| `+190h` | `ShipAvoidance.CollectTimer.[1]` | float | - | 1 * | 0083b7ad | `FUN_009f1160` |
| `+194h` | `ShipAvoidance.CollectTimer.[2]` | float | - | 2 * | 0083b810 | `FUN_009eaca0`, `FUN_009eb660`, `FUN_009f0d20` |
| `+198h` | `ShipAvoidance.CollectDist` | float | - | 450 | 0083b85d | `FUN_009f1420` |
| `+19Ch` | `ShipAvoidance.CollectHitTime` | float | - | 10 * | 0083b899 | `FUN_009f1420` |
| `+1A0h` | `ShipAvoidance.NearbyShip_ArriveTimeMin` | float | 3 | 2.0 | 0083b8df | - |
| `+1A4h` | `ShipAvoidance.NearbyShip_ArriveDistMin` | float | 0.25 | 0.4 | 0083b925 | - |
| `+1A8h` | `ShipAvoidance.NearbyShip_PosSpeedCorrig` | float | 2 | 0.5 | 0083b96b | `FUN_009eae20` |
| `+1ACh` | `ShipAvoidance.NearbyShip_EstPos_DistLimitMul` | float | 0.7 | 4.0 | 0083b9b1 | - |
| `+1B0h` | `ShipAvoidance.NearbyShip_EstPos_MinShipLength` | float | 100 | 5 | 0083b9f7 | - |
| `+1B4h` | `ShipAvoidance.NearbyShip_EstPos_ShipLengthLimitMul` | float | 1.8 | 4.0 | 0083ba3d | - |
| `+1B8h` | `ShipAvoidance.NearbyShip_MyMinSpdRatio` | float | 0.6 | 0.2 | 0083ba83 | `FUN_009f0ea0` |
| `+1BCh` | `ShipAvoidance.NearbyShip_EstPos_ShipSpdMul` | float | 0.75 | 1.0 | 0083bac9 | - |
| `+1C0h` | `ShipAvoidance.NearbyShip_EstPos_SizeDecMul` | float | 0.3 | 0.3 | 0083bb0f | - |
| `+1C4h` | `ShipAvoidance.NearbyShip_EstPos_SizeDecMin` | float | 0.1 | 0.2 | 0083bb55 | - |
| `+1C8h` | `ShipAvoidance.NearbyShip_NextCornerReachDistAddOn` | float | 100 | 100 | 0083bb9b | - |
| `+1CCh` | `ShipAvoidance.NearbyShip_MovePathLineCheckThreshold` | float | 200 | 150 | 0083bbe1 | - |
| `+1D0h` | `ShipAvoidance.NearbyShip_GoAwaySpdAdd` | float | 3 | 2.0 | 0083bcb3 | - |
| `+1D4h` | `ShipAvoidance.NearbyShip_WayClearCheckTime` | float | 0.5 | 0.25 | 0083bc27 | `FUN_009ef910` |
| `+1D8h` | `ShipAvoidance.HitDetector_LastHitDistAddOn` | float | 30 | 50 | 0083bc6d | `FUN_009eb660` |
| `+1ECh` | `TorpedoAvoidance.CollectTimer.[1]` | float | - | 1 * | 0083bf3f | `FUN_009f1160` |
| `+1F0h` | `TorpedoAvoidance.CollectTimer.[2]` | float | - | 2 * | 0083bfa8 | `FUN_009eaca0`, `FUN_009f0ad0` |
| `+1F4h` | `LandAvoidance.CheckMovePosZoneTime.[1]` | float | - | 2.5 | 0083c032 | `FUN_009eca20` |
| `+1F8h` | `LandAvoidance.CheckMovePosZoneTime.[2]` | float | - | 3 | 0083c09b | - |
| `+1FCh` | `LandAvoidance.CheckShipPosZoneTime.[1]` | float | - | 2.5 | 0083c104 | `FUN_009eca20` |
| `+200h` | `LandAvoidance.CheckShipPosZoneTime.[2]` | float | - | 3 | 0083c16d | - |
| `+204h` | `LandAvoidance.CheckTravelZoneTime.[1]` | float | - | 3 | 0083c1d6 | `FUN_009eca20` |
| `+208h` | `LandAvoidance.CheckTravelZoneTime.[2]` | float | - | 4 | 0083c23f | `FUN_009eca20` |
| `+20Ch` | `LandAvoidance.CollectTimer.[1]` | float | - | 1 * | 0083c2ab | - |
| `+210h` | `LandAvoidance.CollectTimer.[2]` | float | - | 2 * | 0083c31a | - |
| `+214h` | `LandAvoidance.YTurnDirDiff.[1]` | float | 1.8 | 1.8 | 0083c393 | `FUN_009ef910` |
| `+218h` | `LandAvoidance.YTurnDirDiff.[2]` | float | 2.1 | 2.1 | 0083c40c | `FUN_009ef910` |
| `+21Ch` | `LandAvoidance.WayCheckerUpdateTime` | float | 1.5 | (absent) | 0083c46c | - |
| `+220h` | `AvoidanceCheat.TurnSpdLimit` | float | 5 | 4.0 | 0083c4d9 | `BSP_UnitInstance_ApplyPropellerTurnAssist` |
| `+224h` | `AvoidanceCheat.TurnSpdMul` | float | 0.6 | 0.5 | 0083c531 | `BSP_UnitInstance_ApplyPropellerTurnAssist` |
| `+228h` | `AvoidanceCheat.StopSpdMul` | float | 2 | 2.0 | 0083c57d | `BSP_UnitInstance_GetForwardAcceleration` |
| `+22Ch` | `Retreat.ExitDist` | float | - | 1000 | 0083c6f2 | `FUN_00826d70` |
| `+230h` | `Retreat.ExitTime` | float | - | 60 | 0083c734 | `FUN_00826d70` |
| `+234h` | `Retreat.WarningRepeatTime` | float | - | 10 | 0083c776 | `FUN_00826d70` |
| `+238h` | `Hack.RotationAdd` | float | 0 | 0.0 | 0083c5fa | `BSP_ShipClass_ReadLuaFields` |
| `+23Ch` | `Hack.HeightAdd` | float | 0 | 0.0 | 0083c642 | `BSP_ShipClass_ReadLuaFields` |
| `+3A0h` | `ColliDolgok.ColliDamageMultiplier` | float | 0 | 1.0 | 0083dfd4 | - |
| `+3ACh` | `FireTickDamage` | float | 0 | 40 * | 0083e1f5 | `BSP_UnitSubObjectA20_Construct`, `FUN_0095eb40` |
| `+3B0h` | `WaterTickDamage` | float | 0 | 100 * | 0083e1b3 | `BSP_Ai_TargetWeight`, `BSP_UnitSubObjectA20_Construct`, `FUN_0095e9a0` +1 |
| `+3B4h` | `BodyRepairTickPercentage` | float | 0.2 | 0.1 * | 0083e23e | `BSP_RepairTask_RepairHull` |
| `+3B8h` | `GunRepairTickPercentage` | float | 2 | 2 * | 0083e290 | `BSP_RepairTask_RepairSubObjects` |
| `+3BCh` | `FireFailureChance` | float | 1 | 3 * | 0083e2de | - |
| `+3C0h` | `FireFailureDamageDuration` | float | 1 | 10 * | 0083e32c | `FUN_00827b90` |
| `+3C4h` | `ExplosionDamagePercentage` | float | 1 | 35 * | 0083e374 | `FUN_00827b90` |
| `+3C8h` | `PumpRepairMultiplier` | float | 2 | 3 * | 0083e3c6 | `BSP_InGameHudRootScreen_UpdateUnitRowWidgets`, `BSP_RepairTask_ApplyFireDamage` |
| `+3CCh` | `FireRepairMultiplier` | float | 2 | 3 * | 0083e412 | `BSP_InGameHudRootScreen_UpdateUnitRowWidgets`, `BSP_RepairTask_ApplyWaterDamage` |
| `+3D0h` | `FailureRepairMultiplier` | float | 2 | 3 * | 0083e45e | `BSP_RepairTask_RepairFailures` |
| `+3D4h` | `BodyRepairMultiplier` | float | 2 | 2 * | 0083e4aa | `BSP_RepairTask_RepairHull` |
| `+3D8h` | `GunRepairMultiplier` | float | 2 | 4 * | 0083e4f6 | `BSP_RepairTask_RepairSubObjects` |
| `+3DCh` | `FailureChance` | float | 5 | 100 * | 0083e542 | - |
| `+3E0h` | `FailureDamageThreshold` | float | 100 | 100 * | 0083e594 | `BSP_ShipSystems_RollComponentFailure` |
| `+3F4h` | `VizbeomlesDolgok.KillDepth` | float | - | -200.0 | 0083ea71 | `BSP_UnitInstance_UpdateShipMotion` |
| `+3F8h` | `VizbeomlesDolgok.PTBoatLeakSize` | float | - | 100.0 | 0083eaad | - |
| `+3FCh` | `VizbeomlesDolgok.LeakPerHP` | float | 0.1 | 0.1 | 0083edda | - |
| `+400h` | `VizbeomlesDolgok.MaxLeakPercent` | float | 0.01 | 0.02 | 0083ee26 | `FUN_0074f490` |
| `+404h` | `VizbeomlesDolgok.EnnyiVizEsKeszPercent` | float | 0.8 | 0.2 | 0083ee72 | `BSP_LeakManager_Update`, `FUN_0074f490`, `FUN_0074fa80` |
| `+408h` | `VizbeomlesDolgok.WaterSectionNum` | float | 6 | (absent) | 0083ed8e | `FUN_00822c20` |
| `+40Ch` | `VizbeomlesDolgok.DologSzorzo` | float | 4 | 2 | 0083eebe | `FUN_0074f490` |
| `+410h` | `Formacio.HeadingMul` | float | - | 0.005 | 0083ef21 | - |
| `+414h` | `Formacio.HeadingPower` | float | - | 1 | 0083ef63 | - |
| `+418h` | `Formacio.ThrustMul` | float | - | 0.01 | 0083efa5 | - |
| `+41Ch` | `Formacio.FollowerSpeedMul` | float | - | 1.5 | 0083efe7 | - |
| `+420h` | `Formacio.FormationMaxCount` | float | - | 24 | 0083f0ef | `FUN_0077f940` |
| `+424h` | `Formacio.FollowerMaxDist` | float | - | 4000.0 | 0083f029 | `FUN_0070ed30` |
| `+42Ch` | `Formacio.FormationShipDist` | float | - | 250.0 | 0083f06b | `FUN_0070ed30`, `FUN_0070efd0` |
| `+430h` | `Formacio.UpdateInterval` | float | - | 10.0 | 0083f0ad | `BSP_UnitInstance_UpdateShipMotion` |
| `+434h` | `Formacio.PlayerFollowerMaxDist` | float | - | 4000.0 | 0083f131 | `BSP_HudFormationScreen_Register` |
| `+438h` | `Navigator.TurnMultipliers.TurnMultiplierMaxSpeed.[2]` | float | - | 2.0 * | 0083d104 | `FUN_0082e850` |
| `+43Ch` | `Navigator.TurnMultipliers.TurnMultiplierMaxSpeed.[1]` | float | - | 1.0 * | 0083d095 | - |
| `+440h` | `Navigator.TurnMultipliers.TurnMultiplierMinSpeed.[2]` | float | - | 0.4 * | 0083cf48 | - |
| `+444h` | `Navigator.TurnMultipliers.TurnMultiplierMinSpeed.[1]` | float | - | 0.0 * | 0083ced9 | `BSP_ShipClass_ComputeRudderCurveDenominator` |
| `+448h` | `Navigator.TurnMultipliers.TurnMultiplierMedSpeed.[2]` | float | - | 1.5 * | 0083d026 | `BSP_ShipClass_ComputeRudderCurveDenominator` |
| `+44Ch` | `Navigator.TurnMultipliers.TurnMultiplierMedSpeed.[1]` | float | - | 0.5 * | 0083cfb7 | `BSP_ShipClass_ComputeRudderCurveDenominator` |
| `+450h` | `ShipCamera.ZoomOffset` | float | - | 1.25 | 0083f194 | `FUN_00432e60` |
| `+454h` | `ShipCamera.LengthMult` | float | - | 1.0 | 0083f1d6 | `FUN_00433800`, `FUN_00433cb0` |
| `+458h` | `ShipCamera.MinCameraAngle` | float | -89 | -89 | 0083f222 | `FUN_0064da40`, `FUN_00651370` |
| `+45Ch` | `ShipCamera.MaxCameraAngle` | float | 89 | 89 | 0083f26e | - |
| `+460h` | `ShipCamera.FollowCamDelay` | float | 0.3 | 1 | 0083f2ba | `FUN_005484f0` |
| `+464h` | `Repair.RepairScriptInterval` | float | - | 2.0 | 0083f3e3 | `FUN_00827820` |
| `+468h` | `Repair.RepairStepValue` | float | - | 0.025 | 0083f425 | - |
| `+46Ch` | `Repair.RepairTeamCount` | float | - | 3 | 0083f31d | `BSP_UnitInstance_RaiseRepairLevel`, `FUN_00822c20` |
| `+470h` | `Repair.RepairTeamBonus` | float | - | 0.60 | 0083f35f | - |
| `+474h` | `Repair.RepairTeamPenalty` | float | - | 0.10 | 0083f3a1 | - |
| `+478h` | `Repair.RepairUnderwaterModifier` | float | - | 0.70 | 0083f467 | - |
| `+494h` | `Repair.RepairZoneMultiplier` | float | - | 5.00 | 0083f511 | `FUN_00825450` |
| `+498h` | `Repair.TorpedoRestockTime` | float | - | 1.00 | 0083f54d | `FUN_00825450` |
| `+49Ch` | `Submarine.SubmarinePeriscopeLevel` | float | - | 20.0 | 0083f5ef | `BSP_Entity_IsSurfaceTarget` |
| `+4A0h` | `Submarine.SubmarineMediumLevel` | float | - | 50.0 | 0083f631 | - |
| `+4A4h` | `Submarine.SubmarineDeepLevel` | float | - | 80.0 | 0083f673 | - |
| `+4A8h` | `Submarine.SubmarineTorpedoRange` | float | - | 400.0 | 0083f5ad | - |
| `+4ACh` | `Submarine.SubmarineDepthDamage` | float | - | 9.0 | 0083f81c | `FUN_008551c0` |
| `+4B0h` | `Submarine.SubmarineDamageDepth` | float | - | 70.0 | 0083f85e | `FUN_008551c0` |
| `+4B4h` | `Submarine.SubmarineDepthSpeedMul` | float | - | 0.834 | 0083f8a0 | `BSP_UnitInstance_UpdateShipMotion` |
| `+4B8h` | `Submarine.SubmarineAirWarningLimit` | float | - | 0.35 | 0083f8e2 | `FUN_00855250` |
| `+4BCh` | `Submarine.SubmarineAirNeedLimit` | float | - | 0.16 | 0083f924 | `FUN_00855250` |
| `+4C0h` | `Submarine.SubmarineAirEnoughLimit` | float | - | 0.5 | 0083f966 | `FUN_00855250` |
| `+4C4h` | `Submarine.PeriscopeRepairTime` | float | 30 | 60.0 | 0083f9b2 | `BSP_UnitController_ClearShapeCollisionBits`, `FUN_0064dd30` |
| `+4C8h` | `SubAttack.MaxTorpedoRange` | float | - | 800.0 | 0083f6d6 | - |
| `+4CCh` | `SubAttack.TooCloseDist` | float | - | 400.0 | 0083f718 | - |
| `+4D0h` | `SubAttack.FarEnoughDist` | float | - | 480.0 | 0083f75a | - |
| `+4D4h` | `SubAttack.SubmarineLostTime` | float | - | 30 | 0083f79c | - |
| `+4D8h` | `MotherShip.ElevatorSpeed` | float | - | 4.0 | 0083fb4c | `FUN_006cfaf0` |
| `+4DCh` | `MotherShip.ElevatorDepth` | float | - | 7.0 | 0083fb8e | `FUN_006d0930` |
| `+588h` | `Physics.TBoatNyomatekSzorzo` | float | 30000 | 1500 | 0083fddf | `BSP_UnitController_ApplyShipForces` |
| `+58Ch` | `Physics.TBoatMotorMaxSzog` | float | 6 | 10.0 | 0083fe28 | - |
| `+590h` | `Physics.TorpedoForce` | float | 1 | -75 | 0083fe7c | - |
| `+594h` | `Physics.TorpedoForcePower` | float | 2 | (absent) | 0083fec8 | - |
| `+598h` | `DOFParams.Dist` | float | 1 | 50 | 0083fc08 | `FUN_006515b0` |
| `+59Ch` | `DOFParams.Range1` | float | 1 | 300 | 0083fc4a | - |
| `+5A0h` | `DOFParams.Range2` | float | 1 | 550 | 0083fc8f | - |
| `+5A4h` | `DOFParams.MinAmount` | float | 1 | 0.6 | 0083fcd7 | - |
| `+5A8h` | `DOFParams.MaxAmount` | float | 0 | 0.1 | 0083fd1f | - |
| `+5ACh` | `TorpedoHit.LifeTime` | float | - | 4.1 | 0083fa1a | `FUN_00827dd0` |
| `+5B0h` | `TorpedoHit.Upthrust` | float | - | 3000000 | 0083fa5f | `BSP_UnitController_ApplyHydroForces` |
| `+5B4h` | `TorpedoHit.UpDist` | float | - | 4.2 | 0083faa3 | - |
| `+5B8h` | `TorpedoHit.MassRatio` | float | - | 900 | 0083fae7 | `BSP_UnitController_ApplyHydroForces` |
| `+64Ch` | `Sounds.ShipDeadMeatSoundTimeMin` | float | 3 | 10.0 | 008407ba | `BSP_UnitInstance_OnWrecked`, `BSP_UnitInstance_Update` |
| `+650h` | `Sounds.ShipDeadMeatSoundTimeMax` | float | 6 | 15.0 | 00840800 | `BSP_UnitInstance_OnWrecked`, `BSP_UnitInstance_Update` |
| `+654h` | `Sounds.PlaneDeadMeatFreq0` | float | 0.15 | 1.0 | 00840849 | - |
| `+658h` | `Sounds.PlaneDeadMeatFreq1000` | float | 1 | 0.5 | 00840891 | - |
| `+65Ch` | `Physics.WreckChance` | float | 0 | 0.3 | 0083fd99 | `BSP_ShipInstance_OnHealthChanged` |
| `+680h` | `DebrisStruct.DebrisEnabled` | bool | 0 | true | 0084124e | `BSP_UnitInstance_Update` |
| `+684h` | `DebrisStruct.DebrisNumPerMeter` | float | 0.3 | 0.3 | 00841294 | `FUN_00935540` |
| `+688h` | `Flag.NeedFlag` | bool | - | true | 0084091a | `FUN_0095de00` |
| `+68Ch` | `Flag.WindPowerMin` | float | - | 8.0 | 00840945 | - |
| `+690h` | `Flag.WindPowerMax` | float | - | 9.0 | 00840981 | - |
| `+694h` | `Flag.WindPowerChange` | float | - | 0.04 | 008409c0 | - |
| `+698h` | `Flag.WindDirChange` | float | - | 0.01 | 00840a86 | - |
| `+69Ch` | `Flag.WindDirMin` | float | - | -1 | 00840a02 | - |
| `+6A0h` | `Flag.WindDirMax` | float | - | 1 | 00840a44 | - |
| `+6A4h` | `Flag.Gravity` | float | - | 0.75 * | 00840bd5 | - |
| `+6A8h` | `Flag.Drag` | float | - | 1.5 | 00840b93 | - |
| `+6ACh` | `Flag.DtMul` | float | - | 1.5 | 00840ac8 | - |
| `+6B0h` | `Flag.DtStep` | float | - | 0.01666 | 00840b0a | - |
| `+6B4h` | `Flag.NumIteration` | float | - | 10 | 00840b4c | - |
| `+6C4h` | `Navigator.AutoThrust.SteerValueMin_Slow` | float | - | 0.4 | 0083cba8 | - |
| `+6C8h` | `Navigator.AutoThrust.SteerValueMax_Slow` | float | - | 1.0 | 0083cbea | - |
| `+6CCh` | `Navigator.AutoThrust.HdgDiffValueMin_Slow` | float | - | DEG(25) | 0083cc2c | `BSP_UnitBot_ComputeThrottleCeiling` |
| `+6D0h` | `Navigator.AutoThrust.HdgDiffValueMax_Slow` | float | - | DEG(75) | 0083cc6e | - |
| `+6D4h` | `Navigator.AutoThrust.ThrustMin_Slow` | float | - | 0.5 | 0083ccb0 | `BSP_UnitBot_ComputeThrottleCeiling` |
| `+6D8h` | `Navigator.AutoThrust.SteerValueMin_Fast` | float | - | 0.75 | 0083ccf2 | - |
| `+6DCh` | `Navigator.AutoThrust.SteerValueMax_Fast` | float | - | 1.5 | 0083cd34 | - |
| `+6E0h` | `Navigator.AutoThrust.HdgDiffValueMin_Fast` | float | - | DEG(45) | 0083cd76 | `BSP_UnitBot_ComputeThrottleCeiling` |
| `+6E4h` | `Navigator.AutoThrust.HdgDiffValueMax_Fast` | float | - | DEG(90) | 0083cdb8 | - |
| `+6E8h` | `Navigator.AutoThrust.ThrustMin_Fast` | float | - | 0.75 | 0083cdfa | `BSP_UnitBot_ComputeThrottleCeiling` |
| `+6ECh` | `Navigator.AutoThrust.HdgDiffDangerMul` | float | - | 6.0 | 0083ce3c | `BSP_UnitBot_ComputeThrottleCeiling` |
| `+6F0h` | `Navigator.PathFinderParams.LengthModifier_DirDiffMin` | float | 0.261799 | DEG(15) | 0083d4bf | `FUN_009ec280` |
| `+6F4h` | `Navigator.PathFinderParams.LengthModifier_DirDiffMax` | float | 1.5708 | DEG(80) | 0083d50b | `FUN_009ec280` |
| `+6F8h` | `Navigator.PathFinderParams.LengthModifier_LengthAddon` | float | 1500 | 1200 | 0083d557 | `FUN_009ec280` |
| `+6FCh` | `Navigator.IslandAttackTurnCircleMultiplier` | float | - | 4.0 | 0083d15a | - |
| `+700h` | `Navigator.FlockPredictTime` | float | - | 3.0 | 0083d19d | - |
| `+704h` | `Navigator.FlockSeparationStrength` | float | - | 1.1 | 0083d1df | - |
| `+708h` | `Navigator.FlockFollowStrength` | float | - | 1.0 | 0083d221 | - |
| `+70Ch` | `Navigator.FlockSeparationMinDistMult` | float | - | 0.8 | 0083d263 | - |
| `+710h` | `Navigator.FlockSeparationMaxDistMult` | float | - | 2.0 | 0083d2a5 | - |
| `+714h` | `Navigator.FlockMoveTreshold` | float | - | 0.5 | 0083d2e7 | - |
| `+718h` | `Navigator.FlockThrustTreshold` | float | - | 0.2 | 0083d329 | - |
| `+71Ch` | `Navigator.FlockBackSteerSpeed` | float | - | -0.1 | 0083d36b | - |
| `+720h` | `Navigator.FlockVectorMultiplier` | float | - | 1.5 | 0083d3ad | - |
| `+724h` | `Navigator.FlockLeaderMaxHdg` | float | - | DEG(20) | 0083d3ef | - |
| `+728h` | `Navigator.FlockLeaderMaxSideMove` | float | - | 0.2 | 0083d431 | - |
| `+734h` | `VizbeomlesDolgok.SinkEffectEnnyiMeterenkent` | float | 5 | 7 | 0083ed04 | - |
| `+738h` | `VizbeomlesDolgok.SinkEffectIlyenGyakran` | int | 1000 | 2000 | 0083ed45 | - |
| `+74Ch` | `Fire.FireDamagePerFireTick` | float | - | 10.0 | 008419d8 | `BSP_ShipClass_ReadLuaFields` |
| `+750h` | `AAGunnerErrorModifier.VersusAI` | float | 2 | 2.0 | 00841a88 | `BSP_GunBot_LeadAimTick` |
| `+754h` | `AAGunnerErrorModifier.VersusPlayer` | float | 1.4 | 1.2 | 00841a3f | `BSP_GunBot_LeadAimTick` |
| `+758h` | `AAGunnerErrorModifier.CalcTargetPosTimeAddFix` | float | 0 | 0.05 | 00841ad0 | `FUN_00901c20` |
| `+75Ch` | `AAGunnerErrorModifier.CalcTargetPosTimeAddMul` | float | 0 | 0.1 | 00841b18 | `FUN_00901c20` |
| `+760h` | `AAGunnerErrorModifier.TurnOffAAGunThrow` | bool | 0 | false | 00841b5c | `BSP_Gun_Fire` |
| `+764h` | `SubTorpedoDelay` | float | 1.5 | 0.5 | 00842608 | - |
| `+768h` | `ShipTorpedoDelay` | float | 1 | 0.5 | 0084264a | - |

## The two `AvoidZoneDepths` records at `+80h` and `+F0h`

`docs/GAME_TUNING_SINGLETON.md` and `docs/VEHICLE_CLASS_LUA_LOAD.md` both ask what fills
`settings+80h..+F0h`, and name `0083B5E0` as the candidate. It is, at `00841B7B..008425A1`.
The block is one body run twice over a counter in `EBP`:

```
00841b7b: XOR EBP,EBP
00841b7d: TEST EBP,EBP
00841b7f: JNZ 0x00841b8e
00841b81: LEA EDI,[ESI + 0x80]     ; pass 0 target
00841b87: MOV EAX,0xd0a228         ; "AvoidZoneDepthsSingle"
00841b8c: JMP 0x00841b99
00841b8e: LEA EDI,[ESI + 0xf0]     ; pass 1 target
00841b94: MOV EAX,0xd0a210         ; "AvoidZoneDepthsMulti"
00841b99: PUSH EAX                 ; GetByName(ShipGlobals, &slot, <that key>)
```

Each pass then reads one class key at a time through `00B66290 GetInteger` and stores the
result at a fixed displacement from `EDI`. The `28` stores give the record layout, `70h`
bytes wide, which is exactly the `+80h`..`+EFh` and `+F0h`..`+15Fh` gap:

| class key | slots | record offsets |
| --- | --- | --- |
| `MotherShip` | `[1]`, `[2]` | `+00h`, `+04h` |
| `Destroyer` | `[1]`, `[2]` | `+08h`, `+0Ch` |
| `TBoat` | `[1]`, `[2]` | `+10h`, `+14h` |
| `SmallLandingShip` | `[1]`, `[2]` | `+18h`, `+1Ch` |
| `LargeLandingShip` | `[1]`, `[2]` | `+20h`, `+24h` |
| `BattleShip` | `[1]`, `[2]` | `+28h`, `+2Ch` |
| `CargoShip` | `[1]`, `[2]` | `+30h`, `+34h` |
| `LightCruiser` | `[1]`, `[2]` | `+38h`, `+3Ch` |
| `HeavyCruiser` | `[1]`, `[2]` | `+40h`, `+44h` |
| `MiniSub` | `[1]`..`[5]` | `+48h`, `+4Ch`, `+50h`, `+54h`, `+58h` |
| `Submarine` | `[1]`..`[5]` | `+5Ch`, `+60h`, `+64h`, `+68h`, `+6Ch` |

The shipped comment above `AvoidZoneDepthsSingle` says the first value is the depth of the
"glass wall" zone the ship physically collides with and the second is the depth of the zone
path-finding avoids; for the two submarine classes slots `[2]`..`[5]` are the four dive
depths. `BattleShip` is `{0, 11}` in the single-player table and `{3, 11}` in the multiplayer
one; the submarines are `{0, 11, 26, 46, 86}` and `{1, 11, 46, 46, 86}`.

The class ordering here is the loader's store order, not a class-id table read from the
executable. `docs/CLASS_ID_TABLE.md` owns the class ids; if the enumeration there differs, its
value wins and the mapping above is still the record layout.

## What the other docs cited as unread

| doc and citation | what it is |
| --- | --- |
| `docs/UNIT_FIRE_AND_REPAIR.md` `+3B4h` "hull repair scale" | `BodyRepairTickPercentage`, installed `0.1` |
| the same doc `+3B8h` "subobject repair scale" | `GunRepairTickPercentage`, installed `2` |
| the same doc `+3C8h` "fire damage divisor" | `PumpRepairMultiplier`, installed `3` |
| the same doc `+3CCh` "water damage divisor" | `FireRepairMultiplier`, installed `3` |
| the same doc `+3D0h` "failure repair rate" | `FailureRepairMultiplier`, installed `3` |
| the same doc `+3D4h` "hull repair rate" | `BodyRepairMultiplier`, installed `2` |
| the same doc `+3D8h` "subobject repair rate" | `GunRepairMultiplier`, installed `4` |
| the same doc and `docs/MISSION_RESULT_DECISION.md` `+3DCh`/`+3E0h` "fallback failure-chance numerator and denominator" | `FailureChance` `100` and `FailureDamageThreshold` `100` |
| the same doc `+46Ch` "maximum crew level per category" | `Repair.RepairTeamCount` |
| the ignition fallback pair | `FireFailureChance` `3` at `+3BCh` and `FireFailureDamageDuration` `10` at `+3C0h`; the fire tick itself is `FireTickDamage` `40` at `+3ACh` and `WaterTickDamage` `100` at `+3B0h` |
| `docs/GAME_DYNAMICS_LIST.md` `+20h`..`+2Ch` "two velocity thresholds and two effect ids" | `Debris.SplashSpeed[1..2]` `5`/`20` and `Debris.SplashFXID[1..2]` `52`/`44` |
| `docs/UNIT_DEATH_MESSAGE_AND_SINK.md` `+64Ch`/`+650h` into `00BD2F10` | `Sounds.ShipDeadMeatSoundTimeMin` `10.0` and `..Max` `15.0`, a random delay in seconds |
| the same doc `+65Ch` compared against a `0..1` draw | `Physics.WreckChance`, installed `0.3` |
| `docs/UNIT_RUDDER_CURVE.md` `+438h`..`+44Ch` | `Navigator.TurnMultipliers.TurnMultiplier{Max,Min,Med}Speed[1..2]`, already established there; this sweep reproduces the same six bindings independently |
| `docs/CRUISE_COMMAND.md` `+160h`..`+178h` | the seven `AttackMoveDirector` weights, consumed by `BSP_UnitInstance_ResetNavigatorParams` |
| `docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` `DamageCalcTime`, `MaxTargetKillRatio` | **not this object**: those are `+60h` and `+5Ch` of the game tuning singleton (`docs/GAME_TUNING_SINGLETON.md`), a different block |

### One correction to record

`docs/UNIT_FIRE_AND_REPAIR.md` reads `+3C8h` as the fire-damage divisor and `+3CCh` as the
water-damage divisor, and the consumer sweep agrees (`+3C8h` is read by `0093C120`
`BSP_RepairTask_ApplyFireDamage`, `+3CCh` by `0093C210` `BSP_RepairTask_ApplyWaterDamage`).
The Lua keys the loader binds to those two offsets are the other way round:
`0083E3CB` stores `PumpRepairMultiplier` into `+3C8h` and `0083E417` stores
`FireRepairMultiplier` into `+3CCh`, and the shipped comments gloss `PumpRepairMultiplier` as
"the water gives this many times less damage over time" and `FireRepairMultiplier` as "the
fire gives this many times less damage over time". So the authored key names and the code's use
of them are crossed. Both are `3` in the installed file, so the swap has no effect on a shipped
run; it matters only if either value is retuned. The consumer bodies were not re-read here -
the names come from the ledger and from that doc - so this is recorded as an observation for
the owner of `docs/UNIT_FIRE_AND_REPAIR.md`, not as a change to it.

## The consumer survey

`00424C40` has `102` callers (`python tools/bsp.py callers 00424c40`; `ghidra xrefs` reports
only `41` and under-reports here). Each caller's listing was swept rather than decompiled: after
a `CALL 0x00424c40` the sweep follows `EAX` and any register it is copied into, records every
`[reg + disp]` read while that register still holds the pointer, and drops the register when it
is redefined or when a call clobbers it. That gives `202` read sites over `154` distinct
offsets and `83` distinct consumer functions, listed per offset in the `consumers` column above
and in full in `reports/gameplay_settings.json`.

`26` consumed offsets have no key because the loader does not write them directly:
`+004h` `+3A8h` `+3E8h` `+3ECh` `+4E0h` `+4E4h` `+4E8h` `+4ECh` `+4F0h` `+4F4h` `+4F8h` `+500h` `+50Ch` `+510h` `+514h` `+5BCh` `+5E0h` `+604h` `+664h` `+668h` `+674h` `+678h` `+72Ch` `+730h` `+740h` `+744h`. `+3E8h`/`+3ECh` are the failure descriptor vector
(`docs/UNIT_FAILURE_SIMULATION.md`), `+4E0h`..`+514h` the three `38h` physics material records
(`docs/SHIP_HULL_BODY.md`), `+5BCh`..`+604h` the per-class engine sound records, `+664h`..`+684h`
the constructor-zeroed group, `+72Ch`/`+730h` the two-element vector the constructor builds and
`+740h`/`+744h` the constructor-zeroed trio. `84` keyed offsets have no consumer in
this sweep; a read through a pointer the sweep loses (a spill to the stack, or the pointer
passed to a callee) does not appear.

## Coverage

| routine | coverage |
| --- | --- |
| `00424C40` | complete (`00424C40..00424CFF`) |
| `00424A10` | complete (`00424A10..00424C15`) |
| `0083B5E0` | partial: every store into the settings object in `0083B5E0..00842951` is covered, and the `258` typed getter call sites all resolve to a key path. Not read: the four `58h` sub-object constructions at `+240h`..`+39Fh` (`00836EF0`, `00836F80`), the failure descriptor vector build at `0083E5D8..0083E9xx`, the effect-name reads through `00871BA0`/`00870CD0`, and the string keys that do not land in a scalar field (`ShipAvoidance.RightOfWayValues`, `ColliDolgok.CollisionEffect`, `Failures[n].FailureName`/`SectionName`, `VizbeomlesDolgok.SinkEffect`, `FreeCameraShot[n].BulletEffect`/`ExplosionEffect`/`SplashEffect`) |
| the loop bodies at `0083FEE7..008403B7` (physics materials), `00840500..008408xx` (per-class sounds), `008413xx..008418xx` (free-camera shots) | partial: only the first iteration's keys resolve, because the key string is selected by index at run time; `docs/SHIP_HULL_BODY.md` already names the three physics material indices |

## Host table

One row per native call site inside `0083B5E0`, grouped by callee. The containing function is
`0083B5E0` for every row.

| callee | sites | first site | contract |
| --- | --- | --- | --- |
| `00B66BD0` | 1 | `0083B60E` | `BSP_LuaStateOwner_Construct`, this = the loader's own state owner |
| `00B6A020` | 1 | `0083B625` | `BSP_LuaStateOwner_Open`, one integer argument `41h` |
| `0041DD40` | 9 | `0083B63A` | `BSP_NativeString_Resize(length, 1)` |
| `00BF7680` | 10 | `0083B655` | `_memcpy`, the script path literal into that string |
| `00B69D40` | 2 | `0083B672` | `BSP_LuaStateOwner_RunScriptWithOverrides(path, 0)` |
| `00419CC0` | 21 | `0083B692` | `BSP_SizedStoragePool_GetSingleton` |
| `00BD1510` | 20 | `0083B699` | `BSP_SizedStoragePool_ReturnBlock` |
| `00BD1120` | 1 | `0083E824` | `BSP_SizedStoragePool_AllocateBlock` |
| `00B67980` | 1 | `0083B721` | `BSP_LuaStateOwner_GetGlobals(&slot)` |
| `00B67800` | 326 | `0083B73D` | `BSP_LuaObject_GetByName(parent, &slot, key)` |
| `00B67720` | 62 | `0083B79E` | `BSP_LuaObject_GetByIndex(table, &slot, one-based index)` |
| `00B67690` | 10 | `0083C7F3` | `BSP_LuaObject_Assign`, enters a sub-table |
| `00B67700` | 396 | `0083B751` | `BSP_LuaObject_Destruct`, the per-slot teardown |
| `00B65F50` | 7 | `0083C7BD` | `BSP_LuaObject_Construct` |
| `00B66270` | 107 | `0083B7AD` | `BSP_LuaObject_GetNumber`, returns in `ST0` |
| `00B66330` | 131 | `0083B8DF` | `BSP_LuaReference_GetFloatOrDefault(fallback)` |
| `00B66290` | 28 | `00841BE5` | `BSP_LuaObject_GetInteger` |
| `00B66380` | 2 | `0083D5BC` | `BSP_LuaReference_GetIntegerOrDefault(fallback)` |
| `00B66250` | 2 | `0084091A` | `BSP_LuaObject_GetBoolean` |
| `00B662F0` | 4 | `0083DB94` | `BSP_LuaReference_GetBooleanOrDefault(fallback)` |
| `00B662B0` | 8 | `0083BD7F` | `BSP_LuaObject_GetString` |
| `00B685C0` | 1 | `008418A4` | `BSP_LuaObject_ConstructStringOrDefault` |
| `00B65FB0` | 11 | `0083BD4E` | `BSP_LuaObject_IsNil`, gates the optional blocks |
| `00B661B0` | 3 | `0083C5C4` | `BSP_LuaObject_IsTable` |
| `00B66000` | 1 | `0084090D` | `BSP_LuaObject_IsBoolean` |
| `00B677E0` | 2 | `0083BD70` | `BSP_LuaObject_ArgumentAt` |
| `00B669A0` | 1 | `00842937` | `BSP_LuaStateOwner_Close` |
| `0041E870` | 9 | `00840DC9` | `BSP_NativeString_Assign` |
| `00438E10` | 1 | `0083E956` | `BSP_CString_CompareInsensitive` |
| `00BF7FBF` | 1 | `0083C675` | `__stricmp` |
| `00BF6713` | 31 | `0083BCE1` | CRT helper, `contract: unread` |
| `00BF7420` | 1 | `00840B51` | CRT float-to-int helper; the only site is `Flag.NumIteration` |
| `00871BA0` | 6 | `0083E06B` | `BSP_EffectHandle_AcquireByName`, the named effect keys |
| `00870CD0` | 1 | `0083E126` | `BSP_EffectHandle_Acquire` |
| `004C1400` | 6 | `00840DD6` | `BSP_ResourceManager_GetSingleton` |
| `00836F80` | 4 | `0083C81A` | `contract: unread`, this = `settings+348h`, one pointer argument |
| `00838E40`, `00488BF0` | 1 each | `00840D6C`, `00840DBB` | STL instantiations, `contract: unread` |
| `00839700`, `00839590`, `0083A880`, `0083A910`, `0083A9E0`, `0083B350`, `0083B480`, `00421470`, `0041F370`, `0041E490`, `0045AFD0`, `00B80D70` | 1..6 each | see `reports/gameplay_settings.json` | `contract: unread`; these sit in the effect, sound and free-camera-shot blocks this packet did not read |
| `[00CE221C]`, `[00CE2220]`, `CALL EBP`/`EDX`/`EAX` | 8, 7, 24 | `0083E0A6` | indirect: reference counting and virtual dispatch inside the effect and failure blocks, `contract: unread` |

## Reconstruction

`include/bsp/gameplay_settings.hpp` declares `GameplayTuningSettings`, the `76Ch` object with
every established offset named and `static_assert`ed, padded with `gap_<offset>h` arrays over
the ranges this packet did not read. The name avoids the existing `GameplaySettings` in
`include/bsp/game_settings.hpp`, which is the OPTIONS-file structure and a different thing.
`src/gameplay_settings.cpp` has the constructor defaults as a pure function and the loader as a
sequence over `GameplayTuningRowView`, one virtual per native getter call site. Both files are
generated from the swept table, so no offset is transcribed by hand.

Nothing here is a binary-compatible replacement, and the loader reconstruction covers the keyed
part only; the sub-object, failure-vector and effect blocks are contracts.

## Follow-ups

| name | addresses | what is left |
| --- | --- | --- |
| `gameplay_settings_subobjects` | 00836ef0 00836f80 0083b5e0 | The four `58h` sub-objects at `+240h`..`+39Fh`: what `00836EF0` builds and which loader block fills them |
| `gameplay_settings_effect_keys` | 0083b5e0 00871ba0 00870cd0 0083a910 0045afd0 | The named-effect and per-class sound blocks, including the `+5BCh`..`+604h` engine sound records `BSP_UnitInstance_UpdateEngineAudio` reads |
| `gameplay_settings_string_keys` | 0083b5e0 00b662b0 00b685c0 | Where the eight `GetString` results land: `RightOfWayValues`, `CollisionEffect`, `SinkEffect`, the failure names and the free-camera-shot effect names |
| `repair_multiplier_naming` | 0093c120 0093c210 0083b5e0 | Settle the crossed `PumpRepairMultiplier`/`FireRepairMultiplier` naming against the two consumer bodies |

## Correction from docs/GAMEPLAY_SETTINGS_TAIL.md (packet cc2_settings_tail)

The "crossed" `PumpRepairMultiplier` / `FireRepairMultiplier` observation is resolved the other
way: the key names are correct and the two consumer function names were swapped. `+3C8h
PumpRepairMultiplier` divides the water timer step (`0093C120`, which runs `task+34h`, the
water timer) and `+3CCh FireRepairMultiplier` the fire timer step (`0093C210`, `task+38h`).
