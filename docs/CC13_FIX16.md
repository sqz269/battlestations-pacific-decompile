# CC13 fix16: bomb-release Lua natives

Packet cc13-fix16 (2026-10-17). Six mission Lua natives that no host answered now have
host answers in `GameScriptOrdersHost` (src/game_hosts_script_orders.cpp), each behind
its own switch in include/bsp/game_hosts_script_orders.hpp, all committed **OFF**. With
a switch off, `GameScriptOrdersHost::handles` refuses the row exactly as before: the
mission Lua host records it UNIMPLEMENTED and nothing is pushed. The new code is inside
`if constexpr` on the switch and the summary line prints only after a call, so an OFF
build behaves as before. No game runs were made; status is build-tested only.

| Native | Address | Switch |
|---|---|---|
| PlaneForceRelease | 0089EBA0 | `kLuaPlaneForceReleaseBound` |
| SquadronForceRelease | 0089E850 | `kLuaSquadronForceReleaseBound` |
| PilotBomb | 008A4F00 | `kLuaPilotBombBound` |
| PlaneBombFallingTime | 008A54C0 | `kLuaPlaneBombFallingTimeBound` |
| PlaneReloadBombPlatforms | 0089F080 | `kLuaPlaneReloadBombPlatformsBound` |
| GetPlaneIsReloading | 008BEBE0 | `kLuaGetPlaneIsReloadingBound` |

Every one is `__fastcall` with ECX the Lua state, plain RET, and reads argument 0
through 00888AA0 with no null test. No CALL to the image RNG 00BD2F10 appears in them (callee lists and
the bodies of 007BBBA0, 007BCC80, 007ED650, 007C1D80, 006E3410 and 007B92E0 read whole).
PilotBomb's task install runs the same 0099A170 path PilotSetTarget uses, whose task
constructors are where any draw would happen.

The units host gained three public methods (include/bsp/game_hosts_units.hpp), each a
thin call into code the host already runs:

- `request_ordnance_release_007bbba0(index)` wraps the host routine
  `release_ordnance_007bbba0(slot)`, the same routine every bot task's 007BBBA0 reaches
  (bay channel C, `torpedo_issue_requests_c20` through 007BBC00, and the release stage's
  spawn).
- `plane_any_rack_empty_007b92e0(index)` asks each rack the same round test
  `plane_holds_rack_round_007b9140` asks (both now share `Impl::plane_rack_holds_round`,
  a pure extraction).
- `plane_reload_bomb_platforms_007c1d80(index)` rearms the plane's single racks in the
  host's own rack state (see PlaneReloadBombPlatforms).

The receiver side of PilotSetTarget (the 0099A170 install and the 007ECF80 squadron
fan-out) was moved unchanged into
`GameScriptOrdersHost::install_pilot_attack_task_after_delivery`, so PilotBomb can use
it. With the label "PilotSetTarget" and the old counter, the log lines and counts are
the same as before.

## The step record that both force-release natives read

Plane +9C0h..+9CFh holds two `{byte, byte, byte, byte, float}` entries. 007CDC70 writes
the entry at index word[00E0B6CC] once per step, and consumers read the entry at
word[00F876B8], the previous step's. Byte +9C0h is `007B9140(plane, 0)`
(BSP_Unit_CanDropOrdnance), written at 007CDCBC/007CDCC1. Byte +9C3h is 007CD9A0, the
one 007BBBA0 itself checks. A disp32 census of +9C0h..+9C7h with an index register
finds these readers of +9C0h: 004648FB (movie ForceRelease), 007BA747, 0089E98A,
0089ECBD, 009A85EF, 009A8745 and 009FA3FB. The only writers are 007CDC70's stores and
the plane constructor. The host keeps no step record. It evaluates
`007B9140(plane, 0)` live:

- kind 17h (MPlaneKamikaze) answers 1 at 007B914A.
- Otherwise `plane_holds_rack_round_007b9140`. 007B9140 walks +974h racks asking
  vtable[210h](2Ah, flag). The flag-1 arm of 006E3FE0 also needs
  `[+3F0h]->vtable[1E4h]()`, which for a plane is [00E17BF2]. 005E2FB2 stores 0 there in
  single player, so flag 0 and flag 1 answer alike.

Labelled divergence: the host reads the byte live, so its answer can run one step ahead
of the image's.

## PlaneForceRelease 0089EBA0

Image: 0089EC9F 00888AA0 (no null or class test). 0089ECB6 MOVZX EDX,word [00F876B8].
0089ECBD `CMP byte [ESI+EDX*8+9C0h],0` and JZ past. 0089ECC9 CALL 007BBBA0 with ECX
the plane. 0089ECD2 00B66400 returns the result count, 0.

007BBBA0 (BSP_Unit_RequestOrdnanceRelease, read whole) works on the device at
unit+DECh:

- When +60h is set, the +64h channel value is not 1.0f, and the plane's own byte
  +9C3h[idx*8] is set, it raises +64h to 1.0f and sets +61h and +68h. While +68h is
  set it also sets +11h.
- It always increments unit+C20h.

The host's `release_ordnance_007bbba0` already models this for the bot tasks: channel C
of `actuator_block_dec`, then `torpedo_issue_requests_c20` through 007BBC00. The release
stage 007CE9FD (`plane_release_issue_stage_007ce9fd`, unit stage at
game_hosts_units.cpp `in.issue_requests_c20`) spends unit+C20h, and so do the follow
states' release tests (009C1FFD). With `kReleaseIssueStageBound` on, the spawn waits
for the rack tick.

Host: `entity_from_argument(0)`, then (kind 17h or `plane_holds_rack_round_007b9140`)
leads to `GameUnitsHost::request_ordnance_release_007bbba0`.

Labelled divergences:

- The +9C0h byte is evaluated live, not one step late (above).
- The +9C3h guard inside 007BBBA0 is not applied, as `release_ordnance_007bbba0`
  already documents: the host keeps no such byte.
- An argument with no units-host slot pushes nothing and records UNIMPLEMENTED; the
  image reads +9C0h of whatever object it is handed.

## SquadronForceRelease 0089E850

Image: 0089E950 00888AA0. 0089E967 `CMP [EDI+3CCh],EBP` / JLE. For ESI = 0..count-1:

- slot ESI <= 4 reads [EBP] (EBP walking +3D0h), else 0 (0089E977-0089E981);
- 0089E98A applies the same byte test;
- 0089E9A0 calls 007BBBA0 on the slot again. Ghidra shows it reading the slot twice,
  once for the test and once for ECX.

No result.

Host: argument 0 as `GetSquadronPlanes` resolves it (`argument_ptr_field`, then the
registry record through `find_by_member_or_departed_unit` with
`kGetSquadronPlanesDepartedLeadBound`). Each of the record's `member_units` that is
not `kPlaneSquadronNoUnit` gets the PlaneForceRelease test and call.

Labelled divergences:

- `member_units` stands for the compacted +3D0h array.
- A plane argument resolves to its squadron, as in GetSquadronPlanes; the image would
  read a plane's +3CCh.

## PilotBomb 008A4F00

Image:

- 008A4FFF 00888AA0 (argument 0); 008A5034 0088A810 reads argument 1 as a command
  target.
- 008A504C 007ED880(unit, 0). That is 007B93E0, kind 2Fh, asked of each of the +3CCh
  members at +3D0h. True loads 00E08F30 dropkamikaze.
- Otherwise 008A5065 vtable[5Ch](10h): true gives 00E08F28 levelbomb, false gives
  00E08F20 divebomb.
- 008A507F 0077D600(unit, class, &target, 1). No result.

Unlike PilotSetTarget, 007EEC50 is never consulted: the order is issued whatever the
target is.

Host:

- `entity_issue_command(unit, class, target, 1)`, then
  `install_pilot_attack_task_after_delivery` (0099A170 and the 007ECF80 fan-out), as
  PilotSetTarget does.
- 007ED880 is answered as `choose_attack_class_007eec50` answers it: the ordered unit's
  ordnance OR its live squadron members'.
- vtable[5Ch](10h) is `unit_is_kind_of(unit, 0x10)` on the ordered unit, as for
  `self_is_level_bomber`.

Labelled divergences:

- The two substitutions above, inherited from the chooser.
- The install happens at the delivery, not one bot tick later, as for PilotSetTarget.
- An unresolved unit pushes nothing and records UNIMPLEMENTED.

## PlaneBombFallingTime 008A54C0

Image:

- 008A55C3 00888AA0.
- 008A55D6 `CMP byte [ESI+0C8h],0` / JNZ; 008A55E1 00414DB0 refreshes the world pose.
- 008A55E6 `FLD float [ESI+100h]` (world y); 008A55F2 007BCC80(plane, y).
- 008A55FC FSTP float; 008A55FF 00B66480 PushFloat. One result.

007BCC80 (RET 4, read whole) runs as follows:

1. 007BCC86 `COMISS 0,h` / JC. h > 0 or unordered (NaN) continue; otherwise FLDZ.
2. vtable[34h] (007BBB70, the world velocity unit+AC8h). vy = float(v.y - 3.0f). The
   3.0f is [00E08E54], read by three literal readers (006E0AB3, 007C2266, 007BCCA4)
   and written by none.
3. disc = float(float(vy*vy) + float((h+h) * 9.81)), with the qword [00CF9058]
   9.8100004196167.
4. root = float(sqrt(disc)) through 00BF7030.
5. The result is float(float(root + vy) / 9.81).

Host: `unit_pose` translation y and `unit_linear_velocity` y (vtable[34h] is the world
velocity, per that accessor). `plane_bomb_fall_time_24bit_007bcc80` runs the arithmetic
in the image's order with each x87 result rounded to float (main thread, 24-bit
precision control). The result goes out through `push_number_float_00b66480`.

Labelled divergences and uncertainty:

- The shared `weapon_fall_time_007bcc80` (dive_bomb_task.cpp) differs in two places:
  it rounds the discriminant once from double, and it returns 0 for a NaN height
  where the image continues. This native does not use it. The difference is
  reported, not changed here.
- An x87 FMUL by a qword under 24-bit control rounds once. The C++ `float(double *
  double)` rounds twice and can differ by one ulp on a tie.
- The stale/refresh hooks are this host's never-stale pair, as for GetDirection.

## PlaneReloadBombPlatforms 0089F080

Image: 0089F17F 00888AA0, then 0089F198 007ED650 with ECX the entity. 007ED650
(007ED650-007ED684) works in two steps:

1. For each +3CCh member at +3D0h it calls 007C1D80. There is no slot cap here: it
   reads [EDI] directly.
2. 007ED67C sets byte squadron+3ECh = 1.

007C1D80 walks the plane's device list (+48h, next +44h) and calls vtable[200h] on
each IsKindOf(25h) device. For a single rack (vtable 00CF96A8) that slot is 006E3410
BSP_MBombPlatform_Rearm:

- +490h = +48Ch (the reload countdown back to ReloadTime);
- vtable[210h](2Ah, 0) true sets ammo +484h = orgAmmo +488h - 1;
- otherwise ammo = orgAmmo, and a tail-jump to vtable[224h] attaches the next round.

Host: the squadron record as for SquadronForceRelease. Each member then goes through
`plane_reload_bomb_platforms_007c1d80`:

- each single rack's `rack_reload_left_per_rack` is reset to `rack_reload_time_per_rack`;
- when the racks are seeded (`rack_ammo >= 0`), each `rack_ammo_per_rack[i]` is set
  back to the seeded orgAmmo (authored ? max(Ammo, 0) : 1), with `rack_ammo` adjusted
  by the same amount. This is the rule `rack_device_reload_slice_006e57a4` and the
  issue seed use.
- a refilled torpedo rack (bullet class 2Bh) puts back the plane's kind-2Bh ordnance
  bit. The gunnery host clears that bit at a drop as its stand-in for "the torpedo
  rack is empty" (game_hosts_gunnery.cpp, packet cc8_torpedo_breakoff); an unequipped
  bag (0) keeps it clear, as `store_unit_ordnance` does.

Readers of what this writes:

- `rack_ammo_per_rack` is read by `rack_holds_round` (squadron ammo type 007EDAD0,
  CanDropOrdnance 007B9140, GetPlaneIsReloading) and by the rack tick and level-bomber
  issue.
- `rack_reload_left_per_rack` is read by the device reload slice.
- The 2Bh bit is read by every 007B93F0 answer. The gunnery host's own note on the
  clear (game_hosts_gunnery.cpp, packet cc8_torpedo_breakoff) records that the torpedo
  transition rule 009D4030 reaches goaway's done only with the bit clear.

Labelled divergences:

- No round entity is created (006E3E70 is not hosted).
- Multi racks (26h) keep no per-round state, and `rack_holds_round` answers their
  authored Ammo either way.
- The rocket racks' `orx_rack_ammo` is not restored: the host does not keep its
  seeded value apart from the count.
- squadron+3ECh is not kept. A disp32 census of +3ECh finds byte writers at 007ED0D7,
  007ED23E, 007ED67C, 007ED6C9, 007F1D5D, 007F3A1D and 007F4B6E, and no byte reader in
  the squadron code; the only byte read, 009DA21D, is in the ship AI. The reader may
  use a different base, so this is "no reader found", not "no reader".
- A plane argument resolves to its squadron.

## GetPlaneIsReloading 008BEBE0

Image:

- 008BECDF 00888AA0.
- 008BECFF stores result byte = 0.
- 008BED04 vtable[5Ch](0Fh): true runs 007B92E0 on the entity. Otherwise ECX =
  [entity+3D0h], and when that is non-null 007B92E0 runs on it (008BED0E-008BED18).
- 008BED2A 00B66450 PushBoolean. One result.

007B92E0 walks the +994h racks at +974h and asks each vtable[20Ch]. For a single rack
that is 006E3570 BSP_BombPlatform_IsEmpty, `vtable[21Ch](2Ah) + [+484h] == 0`.

Host:

- A slot answering kind 0Fh asks `plane_any_rack_empty_007b92e0` of itself.
- Otherwise the registry record's `flight_leader()` (+3D0h slot 0) is asked.
- Anything else pushes nothing and records UNIMPLEMENTED.

Labelled divergences:

- The per-rack count is the host's (seeded count, else the authored loadout), the same
  count CanDropOrdnance uses.
- Racks whose projectile class the host could not read are skipped, as
  `plane_holds_rack_round_007b9140` skips them.
- A fused squadron slot answers as its wing-0 plane, so after that plane dies it does
  not move on to the new +3D0h[0].
- A non-plane, non-squadron entity pushes nothing; the image reads its +3D0h.

## Missions that reach each native (this installation's Lua)

Paths are under `scripts/` of the game root in config/target.json. COTP-IJN and PRCPIJN
carry copies of the ijn/JM missions.

- **PlaneForceRelease:** ijn/JM/jm05.lua:3388 (Paratroopers within 300 of
  Airfields[1] with ammo), and jm13.lua:2661 / :2674 (Dakota, Dakota2 within 300 of CB)
  and :3270 (luaJM13Drop, behind `not GetPlaneIsReloading(IJNParatrooper)`). jm06 :2553
  is commented out.
  - OFF: each call is UNIMPLEMENTED and nothing drops.
  - ON: each listed plane whose rack holds a round gets a 007BBBA0 request.
    Paratrooper (kind 31h) racks then depend on the release stage and rack tick; this
    packet did not establish whether they spawn for 31h racks.
- **SquadronForceRelease:** ijn/JM/jm02.lua:3515 (luaJM2MovieDrop, FindEntity("MovieBetty
  01")) and COTP-USN/usn_12_indochina.lua:1559 (PRCPUS :1819). The two calls in
  global/commandhelpers.lua (:15471, :15529) are in luaPlayerAirbaseManager, whose only
  caller (chg_6_forcez.lua:165) is commented out.
- **PilotBomb:** COTP-USN/usn_10_battle_of_capeengano.lua:1148 / :1150 (PRCPUS :1196 /
  :1198), JAP_StartingPlane01/02 onto USN_Iowa[1].
  - ON: a levelbomb, divebomb or dropkamikaze task where today the row is refused.
  - commandhelpers :5579 is in luaAirstrikeManager_think; no mission calls
    luaAirstrikeManager.
- **PlaneBombFallingTime:** ijn/JM/jm02.lua:2430 (the selected player Betty's delay
  before luaJM2CheckFailDlgs).
  - OFF: the nil result makes luaDelay's delay nil.
  - ON: the fall time, typically a few seconds.
- **PlaneReloadBombPlatforms:** ijn/JM/jm05.lua:5505 (the catapulted Pete from
  GetLastCatapulted, when it is not the selected unit), and multi/competitive06 (eight
  calls), competitive08 :865, competitive12 :1462 and competitive14 :1667.
  commandhelpers :3527 / :3543 / :3576 / :14795 are developer cheat keys (Ctrl+Alt+
  Shift+F5/F6/F7); :14947 sits inside a `--[[ ]]` block.
- **GetPlaneIsReloading:** ijn/JM/jm01.lua:1946 / :1962 (PlayerUnit return hint),
  jm02 :3899, jm07 :1044 (`not GetPlaneIsReloading(unit)` gates an attack), jm13
  :1156 / :1165 / :1174 / :3267, and jm14 :1348.
  - OFF: each call is UNIMPLEMENTED, so the script reads nil (false).
  - ON: true once any single rack of the plane is empty. jm13's luaJM13Drop then
    stops calling PilotMoveTo and PlaneForceRelease.

## Notes for cc9 (release decision, goaway)

- PlaneForceRelease and SquadronForceRelease raise unit+C20h through
  `release_ordnance_007bbba0`. The follow states return early on
  `torpedo_release_pending_c25 || torpedo_issue_requests_c20 > 0` (009C1FFD;
  game_hosts_units.cpp, the follow tick and the moveto-follow arm). Only the release
  stage 007CE9FD lowers +C20h. A forced release on a plane whose stage never spends the
  request would therefore hold that plane in the follow state's release arm. Check
  this for any class where the forced natives are switched on.
- PlaneReloadBombPlatforms ON puts back the kind-2Bh bit of a refilled torpedo plane.
  The gunnery host's note on the clear says 009D4030's goaway reaches done only with
  that bit clear, so a rearmed torpedo plane would stay out of `done`, as the image's
  does once +484h is refilled. This packet did not re-read 009D4030 to confirm it.
- The host's 007BBBA0 omits the +9C3h guard (007CD9A0, bay-door rack holds a bomb).
  The image refuses the channel raise when that byte is clear but still increments
  unit+C20h.

## Flip guidance

Each switch is independent and none was flipped. Judge each from paired runs of the
missions above:

- GetPlaneIsReloading and PlaneBombFallingTime are reads and move only script
  branches.
- PlaneForceRelease, SquadronForceRelease and PilotBomb create releases or orders.
- PlaneReloadBombPlatforms changes rack counts; it is reached in jm05 only through
  GetLastCatapulted.
