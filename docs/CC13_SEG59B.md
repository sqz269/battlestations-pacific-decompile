# CC13 seg59b: segment 59 gun bots (packet `cc13-seg59A`)

Grant: the cc9 gameplay lead opened segment 59 (008EE670-00922240) to cc13 on 2026-10-18 on its
gameplay terms (gunner bots, the torpedo bot, tail gunner, pilot bot). The earlier `cc13-seg59a`
pass (docs/CC13_SEG59A.md) covered the scoring band from 00905300; this pass covers the gun bots.
Code: `src/cc13_seg59b.cpp`, `include/bsp/cc13_seg59b.hpp`. Nothing is wired into bsp_game; no
game runs. Status of every reconstructed function: reconstructed, build-checked; not
fixture-tested, not ABI-compatible, not game-validated. Names are hypotheses.

## Model

Every routine takes the original bot as raw bytes and reaches the engine through `Seg59bHost`.
The property archive (save/load) is reached through four virtual-slot methods: +04h open section,
+08h close section, +0Ch write field, +10h read field; each key and value is the image's 8-byte
by-value pair `{kind, value}` (kind 0 = C string key, 2 = float: its bits when written, its
address when read).

## Torpedo bot (vtable 00D182B0)

The gameplay slots of the torpedo bot were already hosted by cc9 (docs/SHIP_AI_OPEN_ITEMS.md,
TorpedoBot slot by slot). The rows below are what was left.

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008FF260` | BSP_GunBot_ConstructTorpedoClass | reconstructed | `torpedo_bot_construct_008ff260`: `0072BBD0(bot, [00E1998C])` at 008FF269, primary vtable `00D182B0` (008FF276), observer vtable `00D18294` at +1Ch (008FF27C), +68h = 10000.0f (`00CE3D64`, MOVSS bit copy, 008FF283). |
| `008FF360` | BSP_GunBot_ReadPropertiesTorpedoClass | reconstructed | `torpedo_bot_read_properties_008ff360`, slot +48h: section "_bot" (008FF381), `008FE660` (008FF386), close (008FF392), then `thinkTimeLeft` -> +5Ch (008FF3BE) and `targetAngle` -> +60h (008FF3EA) by address, then +58h = +50h (008FF3EF). Save/load only. |
| `008FF400` | BSP_GunBot_WritePropertiesTorpedoClass | reconstructed | `torpedo_bot_write_properties_008ff400`, slot +4Ch: the writer twin through slot +0Ch with the float bits of +5Ch (008FF462) and +60h (008FF492). Save/load only. |
| `009035A0` | BSP_TorpedoBot_SetFireTarget | ledger-only | Hosted inline at the cc9 gunnery caller (src/game_hosts_gunnery.cpp, 00727F44 -> 009035A0) and `torpedo_gun_set_bot_fire_target_00727f10` (src/ship_torpedo_ver_cc13.cpp). Spot check: 00521EA0(bot+38h) vs target (009035B5 SETNZ), 006DF170 stores (009035B8), 00903420 recomputes only on a change (009035C3) with the current +6Ch flag. No defect seen. |
| `008FD620` | BSP_RobotConfig_ConstructTorpedoBot | ledger-only | Hosted: src/robot_config.cpp `construct_descriptor<TorpedoBotConfig>("TorpedoBot", 0x00d17ff8)`. Base vtable `00D17DC0`, `00438E40` name duplicate into +8, derived vtable `00D17FF8`. |
| `008FD830` | BSP_RobotConfig_DestructTorpedoBot | ledger-only | Hosted: src/robot_config.cpp `delete_robot_descriptor`. Base vtable, name freed by `00BF6989` and cleared. |

No torpedo-bot body here draws on the shared RNG `00BD2F10`. No adoption note: the host matches the
binary on every row checked.

## Gun-bot class machinery

The class names in the image's ledger ("lead", "turret", "ballistic") map to robot classes by the
descriptor global each constructor passes to `0072BBD0`: lead = AAGunnerBot `[00E19998]`, turret =
TailGunnerBot `[00E199A0]`, ballistic = AAFlakBot `[00E1999C]`. Vtable slots: +04h attach, +30h
clear fire target, +38h set fire target, +48h / +4Ch save/load reader / writer.

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008FBC80` | BSP_GunBot_Attach | reconstructed | `gun_bot_attach_008fbc80`: `00876020(ECX = parent, bot)` (008FBC8B), +50h = [parent+28h], then bot vt[18h](unit vt[12Ch]()) when the unit passes vt[5Ch](5) (008FBCBC), else vt[18h](1) (008FBCCC). Live at mission load. |
| `008FE170` | BSP_GunBot_ReadTargetTypeProperties | reconstructed | `gun_bot_read_target_type_properties_008fe170` on the bot+38h record: section "trgType", two index-keyed bytes, then "eId" (word +2, read through the archive argument's own stack slot), "e" {4, +4}, "pos" {5, +8}, "range" {2, +14h}. Save/load. |
| `008FE2A0` | BSP_GunBot_WriteTargetTypeProperties | reconstructed | `gun_bot_write_target_type_properties_008fe2a0`: the writer twin; byte values store only the low byte (008FE2CE). Save/load. |
| `008FE660` | BSP_GunBot_ReadBaseProperties | reconstructed | `gun_bot_read_base_properties_008fe660`: section "_process" with `008746F0` (008FE687), "level" into +34h (008FE6C1), then `008FE170` (008FE6C4). Save/load. |
| `008FE6D0` | BSP_GunBot_WriteBaseProperties | reconstructed | `gun_bot_write_base_properties_008fe6d0`: `00874700` (008FE6F7), "level" from +34h, then `008FE2A0` (008FE72C). Save/load. |
| `008FE740` | BSP_GunBot_ConstructLeadClass | reconstructed | `aa_gunner_bot_construct_008fe740`: `0072BBD0` (008FE749), vtables `00D180A0` / `00D18088`, +5Ch = 0. |
| `008FBDC0` | BSP_GunBot_AttachLeadClass | reconstructed | `aa_gunner_bot_attach_008fbdc0`: `008FBC80` (008FBDC8), +5Ch = +50h, +60h..+70h = 0.0f. |
| `008FE7A0` | BSP_GunBot_ClearFireTargetLeadClass | reconstructed | `aa_gunner_bot_clear_fire_target_008fe7a0`: `0072BD50` (008FE7A3), then `[bot+5Ch]`->vt[1E8h](0) when the owner slot is 8 or `00927F10` holds it (008FE7B7). |
| `008FE800` | BSP_GunBot_ReadPropertiesLeadClass | reconstructed | `aa_gunner_bot_read_properties_008fe800`: "_bot", `008FE660` (008FE826), calcTick, angleErrTick, actAngleErr, dAngleErr; +5Ch = +50h. Save/load. |
| `008FE8F0` | BSP_GunBot_WritePropertiesLeadClass | reconstructed | `aa_gunner_bot_write_properties_008fe8f0`: `008FE6D0` (008FE916) and the four fields. Save/load. |
| `008FE9F0` | BSP_GunBot_ConstructTurretClass | reconstructed | `tail_gunner_bot_construct_008fe9f0`: `0072BBD0` (008FE9F9), byte +58h = 0, +68h = 0, vtables `00D18140` / `00D18124`. |
| `008FBEC0` | BSP_GunBot_AttachTurretClass | reconstructed | `tail_gunner_bot_attach_008fbec0`: `008FBC80` (008FBEC8), +68h = +50h, byte +58h = 0, +70h/+74h = 0.0f. |
| `008FEA60` | BSP_GunBot_ClearFireTargetTurretClass | reconstructed | `tail_gunner_bot_clear_fire_target_008fea60`: `0072BD50` (008FEA63), `00927F10` (008FEA77), `[bot+68h]`->vt[1E8h](0). |
| `008FEAC0` | BSP_GunBot_ReadPropertiesTurretClass | reconstructed | `tail_gunner_bot_read_properties_008feac0`: `008FE660` (008FEAE6) and eleven fields (actFireState .. shootRange); +68h = +50h. Save/load. |
| `008FECF0` | BSP_GunBot_WritePropertiesTurretClass | reconstructed | `tail_gunner_bot_write_properties_008fecf0`: `008FE6D0` (008FED16) and the same eleven fields. Save/load. |
| `008FEFD0` | BSP_GunBot_ConstructBallisticClass | reconstructed | `aa_flak_bot_construct_008fefd0`: `0072BBD0` (008FEFD9), vtables `00D18238` / `00D18220`, +64h = 0.0f, +68h = 0. |
| `008FDBA0` | BSP_GunBot_ClearFireTargetBallisticClass | reconstructed | `aa_flak_bot_clear_fire_target_008fdba0`: `0072BD50` (008FDBA3), `00927F10` (008FDBB7), `[bot+68h]`->vt[1E8h](0). |
| `008FF060` | BSP_GunBot_ReadPropertiesBallisticClass | reconstructed | `aa_flak_bot_read_properties_008ff060`: `008FE660` (008FF086), horzAngleError, vertAngleError, distError; +68h = +50h. Save/load. |
| `008FF130` | BSP_GunBot_WritePropertiesBallisticClass | reconstructed | `aa_flak_bot_write_properties_008ff130`: `008FE6D0` (008FF156) and the three floats. Save/load. |
| `008FF4A0` | BSP_GunBot_ConstructDepthChargeClass | reconstructed | `depth_charge_bot_construct_008ff4a0`: `0072BBD0` (008FF4A9), vtables `00D18338` / `00D1831C`, +58h = 0, +60h = -1.0f (`00D7A260`). |
| `008FF560` | BSP_GunBot_ReadPropertiesDepthChargeClass | reconstructed | `depth_charge_bot_read_properties_008ff560`: `008FE660` (008FF586), thinkTimeLeft; +58h = +50h. Save/load. |
| `008FF5D0` | BSP_GunBot_WritePropertiesDepthChargeClass | reconstructed | `depth_charge_bot_write_properties_008ff5d0`: `008FE6D0` (008FF5F6), thinkTimeLeft. Save/load. |
| `008FE120` | BSP_Entity_AsClass05OrNull | reconstructed | `entity_as_kind05_or_null_008fe120`: null or vt[5Ch](5). |
| `008FE140` | BSP_Entity_AsShipOrNull | reconstructed | `entity_as_kind06_or_null_008fe140`: null or vt[5Ch](6); called by the torpedo tick 008FFF20. |
| `008FEA40` | BSP_GunBot_SetFireTargetTurretClass | ledger-only | Hosted at the cc9 gunnery caller (game_hosts_gunnery.cpp, kTailGunnerAimErrorBound): `006DF170`, then +6Ch = -1.0f. |
| `008FF040` | BSP_GunBot_AttachBallisticClass | ledger-only | Hosted with cc9's labelled substitution (kFlakErrorRerollBound): the `008FDBE0` roll (shared RNG `00BD2F10`) happens at the first flak tick instead of at attach (008FF055). |

The three ClearFireTarget overrides send the gun's vt[1E8h](0); the cc9 host's clear does not (an
open item already listed in docs/SHIP_AI_OPEN_ITEMS.md for the torpedo twin). Its meaning was not
read here either, so it is a named host hook.

## Robot descriptors

| Address | Name (hypothesis) | Status | Notes |
| --- | --- | --- | --- |
| `008FB770` | BSP_RobotConfig_ReadParametersBase | reconstructed | `robot_config_read_parameters_base_008fb770`: `RET 8`, reads nothing. |
| `008FB750` | BSP_RobotConfig_ConstructBase | ledger-only | src/robot_config.cpp `construct_descriptor`. |
| `008FC660` `008FC6B0` `008FC9F0` `008FCD40` `008FCF10` `008FD350` `008FD860` | Construct PilotBot, AAFlakBot, TailGunnerBot, AAGunnerBot, ArtillerySubDirectorBot, ArtilleryGunnerBot, DepthChargeBot | ledger-only | src/robot_config.cpp `construct_descriptor` with the same name strings and derived vtables. |
| `008FC570` `008FC680` `008FC9C0` `008FCD10` `008FCEE0` `008FD320` `008FD5F0` `008FDAC0` | Destruct base and the seven classes | ledger-only | src/robot_config.cpp `delete_robot_descriptor`. |
