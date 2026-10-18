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
