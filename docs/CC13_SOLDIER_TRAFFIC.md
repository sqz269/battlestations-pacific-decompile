# Soldier / traffic behaviour 0048A010-0048C230 and the SoldierGlobals loader 0084CB70

Packet cc13-soldier1, worker of the cc13 lead, 2026-10-12, base origin/main 9949ac763.
Reconstruct-only: nothing is wired into a game host and no game run was made.

Code: `include/bsp/soldier_traffic_cc13.hpp`, `src/soldier_traffic_cc13.cpp` (registered in
`cmake/startup.cmake`). Call-site rows: `reports/cc13_soldier_traffic_calls.json` (81 rows,
`tools/verify_report_calls.py`: 0 failed). Status of every routine below: **exported,
reconstructed, build-tested**. Nothing is fixture-tested, ABI-compatible or game-validated.

Evidence is the installed executable's bytes (Capstone over the disk image, every comparison
and every ECX read from the listing) plus the Ghidra pseudocode for structure. Names are
hypotheses, not recovered symbols.

## 1. Summary

- The window is not one subsystem but three object families that share the soldier types:
  1. the **landing-troop group** (E4h bytes, vtable 00CE66E0, constructor 004A0410), whose
     members run from a landing ship to a pad, fire, die and despawn: 0048A6D0, 0048AF00,
     0048B120, 0048BF80, 0048C090, 0048C190;
  2. the **scene traffic runtime** of `docs/SCENE_TRAFFIC_RUNTIME.md`: its view gate 0048A650 and
     member clear 0048A5A0;
  3. the **landing lane** (vtable 00CE6A68, constructor 004A66B0): 0048C230 rebuilds its path.
  0048A010 (clip test) and 0048A1E0 (muzzle effect) work on the member entities. 0048AEC0 has no
  reference anywhere in the image.
- **Verdict: not purely ambient** (section 5). Killing a landing-troop member with a projectile or
  a blast damages the landing ship by the member type's `Damage`; the members' positions and
  states, which this window drives with 20 stream-0 draws, decide those kills. The lane rebuild
  sets the distance at which the lane tick calls `BSP_LandingShip_FinishLanding`.
- Every RNG draw in the window is stream 0 (`XOR ECX,ECX` before each call). The interface
  routes each draw through a site key; 10 sites are presentation-only and 10 are
  gameplay-coupled (section 6).

## 2. SoldierGlobals, 0084CB70

`BSP_SoldierGlobals_LoadLua`, `__fastcall(ECX = SoldierGlobals*)`, body 0084CB70-0084CE14, only
caller 00484700 (rel32 census: 0048472C). The 20h block is the singleton 00F87544 behind
00485520 (`operator_new(20h)`, 00484700 installs vtable 00CE6458 and clears +1Ch).

| step | address | effect |
| --- | --- | --- |
| latch | 0084CB8E / 0084CBA2 | returns when +1Ch is set, else sets it first |
| state | 0084CBAA | `BSP_LuaStateOwner_Open(41h)` on a private state |
| scripts | 0084CBC0, 0084CC0A | `Scripts\global\luaMW_init.lua`, then `Scripts\datatables\SoldierGlobals.lua` |
| +4 | 0084CCB1 | `ShadowClipDistance` |
| +8 | 0084CCE6 | `FireEfxClipDistance` |
| +0Ch | 0084CD1B | `ClipDistance` |
| +10h | 0084CD50..0084CD67 | `MaxParatrooperGradient` x qword 3.1415927410125732 [00CE3D28] / qword 180.0 [00CE3D20]: radians |
| +14h | 0084CD91 | `ParachuteOpenAltitudeMin` |
| +18h | 0084CDC6 | `ParachuteOpenAltitudeMax` |

All six use `BSP_LuaObject_GetNumber` 00B66270 (no default): a missing key is whatever that
returns for nil.

This installation (modded; see the install notes), `scripts\datatables\soldierglobals.lua`,
mtime 2024-07-13 08:26:50, 299 bytes. That is the date of the untouched bulk of
`scripts\datatables` (29 files); only the 2025/2026-dated files there are local edits.

| line | key | value |
| --- | --- | --- |
| 3 | ShadowClipDistance | 100 |
| 4 | FireEfxClipDistance | 500 |
| 5 | ClipDistance | 500 |
| 6 | MaxParatrooperGradient | 20 (stored as 0.349 rad) |
| 7 | ParachuteOpenAltitudeMin | 300 |
| 8 | ParachuteOpenAltitudeMax | 400 |

Readers of the singleton (rel32 census of 00485520, 12 sites): 00489F30 (37h entity base, the
stream-0 phase draw recorded in `docs/CC13_BAND_0045_0048.md`), 0048A010, 0048A1E0, 0048A6D0,
004AFFE0 (vtable 00CE70A8, no direct caller), `BSP_Game_OnInit` 004E3F7D, and the paratrooper
code in segment 46 (007AB8C0 twice, 007AC000), which uses the gradient and the parachute
altitudes. 004886A0 is an orphan `JMP 00485520` with no reference. 00480B48 clears 00F87544 in a
deleting destructor.

## 3. The landing-troop group

### 3.1 Objects

- **Group** (E4h): created by 004A4520 (`operator_new(E4h)` + 004A0410) from
  `BSP_LandingShip_BeginLandingAtPad` 0074A9F7 with ECX = `[game+21D0h]` (the world's ambient
  manager; its tree at +28h holds the groups by pad slot). Constructor fields, read from
  `LandingTroopClass[ship class +814h]` (`scripts\datatables\autoload\landingtroopclasses.lua`,
  2024-07-13): `Speed` +24h, `MaxDeflection` +28h, `Distance` +2Ch, `RunMinTime` +30h,
  `RunMaxTime` +34h, `FireMinTime` +38h, `FireMaxTime` +3Ch, `DieChance` +40h, class index +44h,
  path parent +54h, its world matrix +58h..+94h, box margin +98h (10.0), box +9Ch/+A8h, three
  member lists at +B4h/+C0h/+CCh (head +4h, count +8h each), pad record +D8h, ship +DCh,
  CommandBuilding +E0h. Vtable 00CE66E0: 0048EFB0 (scalar deleting destructor), 004A2FC0 (tick),
  0048BF80 (member released).
- **Soldier type** (one row of `LandingTroopClass[n].Classes`): base 0048DC40 (vtable 00CE66DC)
  reads `Weight` +14h, `FireChance` +18h, `Damage` +1Ch, `MinDistance` +20h, `MaxDistance` +24h,
  `OutingDelay` +28h, `MaxEntity` +2Ch, `MininRow` +30h, `MaxinRow` +34h, `Defence` +39h and
  `FakeEffects` (pointer array +8h, count +0Ch; rows by 00487770: `ExplosionEfx` +0, `MaxTime`
  +4h, `MinTime` +8h, `Radius` +0Ch, `Weight` +10h). The derived constructor 0049FC80 (vtable
  00CE691C) adds the name vectors `AnimRun` +3Ch, `AnimFire` +4Ch, `AnimDie` +5Ch, `AnimIdle` +6Ch
  (8-byte strings). +38h is a byte the window treats as "animated soldier" (only soldiers get
  animations, only non-soldiers get the terrain-normal alignment); provisional: Type ==
  "Soldier".
- **Member node** (std::list node, payload from +8h): type +8h, entity +0Ch, path position
  +14h, path +18h (end distance at +1Ch, length scale at +28h), state +34h, timer +38h,
  fake-effect timer +3Ch, hold timer +40h, fade +44h.
- **Member entity**: an instance of the type's named class (+4h, vslot +8). Its class
  (`NamedClassBase_ReadLua` 0048F670, `soldierclasses.lua`) carries the `FakeFireEfx` effect +14h
  and the fire point +18h..+20h with a validity byte +24h. The entity keeps its class at +170h,
  its model at +178h, its clip bias at +17Ch, its current effect at +180h and a visibility byte
  at +5Ch.

### 3.2 The tick, 004A2FC0 (group slot 1, not in this window)

Every frame while `[00E081C0]` is set: 004B48E0 view-tests the box, then 004A0D20, 0048AF00,
0048A6D0(dt), 0048B120(dt), then the per-member pose and clip pass that calls 0048A010.

### 3.3 0048AF00, spawn missing member entities

`__fastcall(group)`. For each member with no entity: `[type+4]` vslot +8 instantiates (both
arms of the +38h test call the same slot); vslot +98h attaches it to the parent of the pad's
first path (`007ACA30` twice, `+30h`, `+3Ch`); vslot +114h(group) stores the owner; vslot
+A0h; a soldier plays `AnimRun[0]` with a random start and forced (004B0F80(.., 1, 1)).

### 3.4 0048B120, the member state machine

`__thiscall(group, float dt)`, RET 4, jump table 0048BF60 (7 entries). On entry it samples
"lists 0 and 2 are empty" once (counts +BCh and +D4h). Per member, in order:

1. `timer -= dt` (0048B1AB).
2. **Ship gone** (0048B1BE..0048B358): state not 0/3/6, ship +DCh null and not `Defence`:
   hold <= 0.2 (qword) draws hold = U(3, 15); 0.2 < hold < 1.0 kills it (state 3, timer
   U(8, 14), a random `AnimDie` for soldiers); otherwise hold -= dt.
3. **Fade** (0048B35B..0048B448): while 0 < fade < 1, fade -= dt / 3.0 (qword); above 0 it sets
   the visibility factor (00B6DA70(fade, 1)), else factor 0, state 6, timer -1.0.
4. **State**:

| state | entry | behaviour |
| --- | --- | --- |
| 0 wait | 0048B46A | hold > 0: position 0, hold -= dt. hold < 0: state 1, hold 0, random `AnimRun` (soldiers). hold == 0 exactly: advances like state 1 |
| 1 run | 0048B5A5 | position += Speed * dt; past the path end: state 4, position = end - 0.01 (qword) |
| 2 fire | 0048BA76 | not kind 3Ah: U(0,1) < FireChance * dt fires 0048A1E0(0). timer <= 0: timer U(RunMinTime, RunMaxTime), stop the effect (00867B10); within 5.0 of the end: `AnimIdle[0]` and state 5, else random `AnimRun` and state 1 |
| 3 die | 0048BC52 | 1 < timer < 6: factor (timer - 1)/6 (qwords), entity+184h = 1; timer < 1: factor 0. timer <= 0: non-Defence killed and erased; Defence back to state 0 with hold U(0, OutingDelay), factor 1, +184h = 0 |
| 4 arrive | 0048B626 | samples the path at end - 0.01 (007B03C0), basis 0085DAD0 with up [00F8758C..94], y = 0087FA20 height minus the path parent's +A8h, vehicles re-based on the terrain normal 0087FB90; writes entity +74h..+B0h, clears +C8h/+10Ch, invalidates child poses; state 5 |
| 5 idle | 0048B8A4 | vehicle: muzzle roll as in state 2. Soldier in list 1, lists 0/2 empty, no ship, CommandBuilding +E0h present and its vslot +210h false: state 6, timer U(10, 20) (written through 00428260's payload, +2Ch/+30h = node +34h/+38h). Else timer <= 0 and FireMaxTime > 0: state 2, timer U(FireMinTime, FireMaxTime), random `AnimFire` i, 0048A1E0(i + 1) |
| 6 leave | 0048BE2E | timer > 1: factor (timer - 2)/10 (qwords), +184h = 1; timer < 0.5: factor 0. timer <= 0: killed and erased |

"Killed" is entity vslot +114h(0) then `BSP_MissionEntity_Kill` 00926D90(cause 2); "erased" is
00484F10 on the node.

### 3.5 0048A6D0, fake effects and the die roll

`__thiscall(group, float dt)`, RET 4. Per member: fake timer -= dt; below 0, with FakeEffects
and state != 0: weighted pick r = U(0, sum of weights), subtract weights until r <= 0; timer
U(MinTime, MaxTime) of the pick. With a camera, |member - camera| (sqrt above 1e-10) times
float(sin(fov)) below `FireEfxClipDistance` spawns the pick's `ExplosionEfx` on the member model
at (U(-r, r) second draw, 0, U(-r, r) first draw). When the pick's Radius < 8.0, U(0,1) <
(dt / [path]+28h) * Speed * DieChance kills the member: state 3, timer 20.0, random `AnimDie`
(004B0F80(.., 0, 0)). The die roll therefore depends on where the camera is.

### 3.6 0048BF80, 0048C090, 0048C190

- 0048BF80 (group vslot +8, `__thiscall(group, entity)`): +18h = `[00F876A4]`, entity vslot
  +114h(0), then the first node holding the entity in each list is unlinked and freed (an owned
  path is deleted first). It does not kill. The caller is the entity side (owner back-pointer
  set by vslot +114h in 0048AF00).
- 0048C090 (`__fastcall`, from 0048CCBA and the deleting destructor 0048E021): every member
  entity is released and killed (cause 2), node +0Ch = 0.
- 0048C190 (`__fastcall`, from 00491F2A and 004A4782): ship +DCh = 0; every member fade = 0.98,
  timer U(3, 10). With the fade in 3.4 step 3 the members vanish over about 3 s.

## 4. Traffic runtime and landing lane

- **0048A650** (traffic runtime vtable 00CE68E4 slot 3): false unless 004B48E0 sees box +150h.
  With an anchor +168h whose side +54h differs from +16Ch, the side is copied, 00487470 re-runs
  the template remap and the result is false; a matching side ticks only when it is below 2.
  Without an anchor +16Ch becomes 1 when the local player slot `[game+18CCh + [game+18ECh]*4]`
  has +28h == 0, else 0; remap; true. This completes item 3 of `docs/SCENE_TRAFFIC_RUNTIME.md`
  section 5 (the arm at 0048A6C1 is `MOV AL,1`).
- **0048A5A0**: kills every member of the runtime's list +6Ch (vslot +114h(0), Kill(2), node+8 =
  0) and frees the nodes. Callers: 0049A360 (traffic slot 1), 0049A410 (lane slot 1), 0049FB20,
  0048C230.
- **0048C230** (lane, only caller 0049A42C in the lane tick 0049A410, which runs it once when
  lane+180h is clear and ship+118Bh is set): sets +180h; walks `[game+19CCh]+34Ch` for the first
  entity answering IsKindOf(44h) whose height 0087FA50 at the ship's landing point
  (+11B4h/+11BCh) exceeds -1000.0 and stores it at +D8h; builds a new 210h path attached to it,
  copies the old points, appends landing + forward * (start - length) and landing + forward *
  ((end - start) + 1.0) at island height; +190h = length at the last point - 1.0; +28h += 2;
  rebuilds the box +19Ch/+1A8h grown by ((end - start) * +188h + 1.0) * +3Ch plus +138h..+140h;
  finally 0048A5A0. No RNG.

The lane itself: 004A6830 creates it from `BSP_UnitInstance_SEntityInit` (00823F04) when the
unit's class has entries in its vectors at +6E0h (50h records) or +700h (1Ch records). Its tick
0049A410 advances +84h and, when +84h + +18Ch * k reaches +190h, calls
`BSP_LandingShip_FinishLanding` 0074AD90 (0049A529).

## 5. Gameplay verdict

**Not purely ambient.** The writes this window makes outside the soldier objects, and the
readers of soldier state outside the soldier code:

Writes from the window to non-soldier objects:

| site | target | kind |
| --- | --- | --- |
| 0048A1E0, 0048A6D0 | point effects (008687C0), the effect system | presentation |
| 0048A6D0, 0048A1E0, 0048A010 | camera `[game+1ED4h]` pose cache +C8h..+10Ch through the inlined `BSP_EntityPose_RefreshWorld` | idempotent cache |
| 0048B120 state 4 | member entity local matrix and pose flags | own entity |
| 0048A5A0, 0048B120, 0048C090 | `BSP_MissionEntity_Kill` 00926D90 on member entities | own entities |
| 0048A650 | traffic runtime +16Ch and its template remap 00487470 | traffic runtime |
| 0048C230 | lane +D8h, +20h, +28h, +180h, +190h, box | lane (see below) |

No routine in the window writes a unit, the CommandBuilding, the pad, the ship or any Lua value.

Readers of soldier state outside the window (rel32 and absolute-dword scans with
`local/sold_rel32.py`, displacement scans for `[game+21D0h]` and `+210h` vcalls):

1. **Projectile and blast kills damage the landing ship.** 00428420 (from
   `BSP_Projectile_TraceSegmentAndImpact` 0084BF00) and `BSP_Explosion_NotifyBlastSites` 00428800
   walk the groups in `[[game+21D0h]+28h]`, test members whose state is not 3 against the hit,
   set state 3, draw a die animation (NextU32 with ECX = 0) and call 00480C20 with the type's
   `Damage` (+1Ch; 004287DB, 00428B0C). 00480C20 calls the group's ship +DCh vslot +1ACh unless
   `[game+1FE4h] == 2`; for the landing ship (primary vtable 00CFFA30, written at 0074BB2C) that
   slot is `BSP_UnitInstance_AddDamage` 0095DA00 (00CFFBDC). Which members exist, where they
   stand and whether they are dying is what 0048AF00/0048B120/0048A6D0/0048C190 decide, with
   stream-0 draws. `docs/COLLISION_SHAPES.md`'s note that "nothing on the blast path depends on
   its result" holds for the blast routine's arguments, not for the ship.
2. **`luaMW_IsLandingTrafficActive`** (0089AF40, `IsLandingTrafficActive` in
   `src/mission_lua_host.cpp`) returns the unit's vslot +210h. For a CommandBuilding (vtable
   00CFB028, constructor in `BSP_CommandBuilding_Construct`) that slot is 006F3A20, which asks
   0048CBB0 for each pad: a group with a ship (+DCh) and members in list 0 (+BCh). No script of
   this installation calls the native (621 .lua files, and the .scn/.dat/.def/.bak files, scanned
   for the name). 0048B120 asks the same slot for its withdraw (state 5).
3. **The lane end decides `LandingFinished`.** 0049A410 calls `BSP_LandingShip_FinishLanding`
   (Lua `LandingFinished` = true, ship +118Ch, `BSP_MissionEvents_ReportShipLanded` 00986820)
   when the lane progress reaches +190h, which 0048C230 sets. This is a second caller besides
   the ramp path 0074B2D9 that `src/building_pads.cpp` and `publish_landing_unloads_0074ad90`
   model; the host models only the ramp path. JM08 (`scripts\missions\COTP-IJN\jm08.lua:621`)
   reads `unit.LandingFinished`.
4. SoldierGlobals: read only by presentation code and the paratroopers (section 2). The
   paratroopers (segment 46) were not examined for gameplay reach.
5. The 37h entity base 00489F30 draws a phase from stream 0 scaled by ClipDistance for every
   anim/bullet/buoy/cloud/vehicle-visual entity (recorded in `docs/CC13_BAND_0045_0048.md`).

Purely ambient: 0048A010, 0048A1E0, the fake-effect visuals of 0048A6D0, and the scene traffic
of `docs/SCENE_TRAFFIC_RUNTIME.md` (0048A650, 0048A5A0), which has no ship and no damage path.

## 6. RNG sites

All twenty: ECX = 0 at the call (stream 0). `kSoldierRngSites` in the header carries the same
table.

| call | routine | draw | class |
| --- | --- | --- | --- |
| 0048A7D7 | 0048A6D0 | U(0, sum of weights) | gameplay-coupled (feeds the die roll) |
| 0048A880 | 0048A6D0 | U(MinTime, MaxTime) | gameplay-coupled (paces the die roll) |
| 0048AC00 | 0048A6D0 | U(-Radius, Radius) z | presentation |
| 0048AC47 | 0048A6D0 | U(-Radius, Radius) x | presentation |
| 0048AD8B | 0048A6D0 | U(0, 1) die roll | gameplay-coupled |
| 0048AE02 | 0048A6D0 | NextU32 % AnimDie | presentation |
| 0048AED0 | 0048AEC0 | U(0, 1) | unreferenced |
| 0048B253 | 0048B120 | U(3, 15) hold | gameplay-coupled |
| 0048B2AC | 0048B120 | U(8, 14) die timer | gameplay-coupled |
| 0048B2FA | 0048B120 | NextU32 % AnimDie | presentation |
| 0048B52C | 0048B120 | NextU32 % AnimRun | presentation |
| 0048B91B | 0048B120 | U(10, 20) withdraw | gameplay-coupled |
| 0048B98F | 0048B120 | U(FireMinTime, FireMaxTime) | gameplay-coupled (stationary time) |
| 0048B9BC | 0048B120 | NextU32 % AnimFire | presentation |
| 0048BA4D | 0048B120 | U(0, 1) vehicle muzzle | presentation |
| 0048BAAA | 0048B120 | U(0, 1) firing muzzle | presentation |
| 0048BB01 | 0048B120 | U(RunMinTime, RunMaxTime) | gameplay-coupled (stationary time) |
| 0048BBF0 | 0048B120 | NextU32 % AnimRun | presentation |
| 0048BD9A | 0048B120 | U(0, OutingDelay) | gameplay-coupled |
| 0048C1FD | 0048C190 | U(3, 10) | gameplay-coupled |

Also reached: 004B0F80 (play animation) draws U(0, duration) with ECX = 0 at 004B102F when its
random-start argument is set and the animation changes (0048AF00, and the run/fire/idle calls
of 0048B120); 00428420/00428800 draw NextU32 with ECX = 0 for the die animation.

**Wiring rule (cc9 RNG rule).** The image interleaves all of these with every other stream-0
user. A host that wires the presentation sites must route them to a keyed presentation stream
(key suggestion: group pad slot + member index + the site key in `kSoldierRngSites`), never to
stream 0. The gameplay-coupled sites decide the troops' exposure to hits and therefore landing
ship damage; image parity would need them on stream 0 in image order, which also shifts every
later stream-0 draw. Land them behind an OFF switch, as the other stream-0 couplings were, and
judge them only with option-on pairs.

## 7. Uncertainties

- +38h of the soldier type as "animated soldier" is inferred from its uses, not from its writer.
- Class kind 3Ah (0048A1E0, 0048B120 state 2) is not identified.
- 00865FE0 is read as "previous effect ready"; only its boolean use is established.
- The path +28h used as the die-roll length scale is not identified beyond its use.
- The fake-effect pick indexes past the end when no weight brings r to 0; the image faults there
  (checked iterator), the C++ clamps. Index draws over an empty vector divide by zero in the
  image; the C++ skips them. Neither happens with this installation's tables (vehicle rows carry
  four empty `AnimDie` names).
- x87: intermediates in double, rounded at each float store; the product/limit comparisons of
  0048A010, 0048A1E0 and 0048A6D0 are done in extended precision in the image.
- 0049A410's exact progress formula (`+18Ch * [00D7A280]`) was read from the pseudocode only.

## 8. Ledger and Ghidra

13 names added with `ledger add-name` and applied with `tools/ghidra_annotate.py --apply`
(prior state recorded in `local/ghidra-annotations-20261012T131847Z.json`: every one was
`FUN_<address>` with no comment). `_free` 00BF9DC8 kept `void __cdecl _free(void * _Memory)`
before and after; no function was created or deleted.

| address | name |
| --- | --- |
| 0084CB70 | BSP_SoldierGlobals_LoadLua |
| 0048A010 | BSP_SoldierEntity_UpdateClipVisibility |
| 0048A1E0 | BSP_SoldierEntity_SpawnMuzzleEffect |
| 0048A5A0 | BSP_TrafficRuntime_KillAllMembers |
| 0048A650 | BSP_TrafficRuntime_ViewGate |
| 0048A6D0 | BSP_LandingTroop_TickFakeEffects |
| 0048AEC0 | BSP_TroopMember_RollMuzzleEffect_Unreferenced |
| 0048AF00 | BSP_LandingTroop_SpawnMissingMemberEntities |
| 0048B120 | BSP_LandingTroop_TickMembers |
| 0048BF80 | BSP_LandingTroop_OnMemberEntityReleased |
| 0048C090 | BSP_LandingTroop_KillAllMemberEntities |
| 0048C190 | BSP_LandingTroop_OnShipLeft |
| 0048C230 | BSP_LandingLane_RebuildPathOntoIsland |

Not named (outside the lease, recorded here for the owners): 004A2FC0 (group tick), 004A0410
(group constructor), 004A4520 (group creator), 00480C20 (member kill -> ship damage), 0048CBB0
(pad has landing traffic), 006F3A20 (CommandBuilding vslot +210h), 0049A410 (lane tick),
004A66B0/004A6830 (lane constructor/creator), 00428420 (projectile hit on troops).

## 9. Host wiring (not done)

- Landing troops: the group needs a home on the building-pad model (`src/building_pads.cpp`,
  cc9) next to the ramp unload; the projectile and blast hooks (00428420/00428800) would feed
  `BSP_UnitInstance_AddDamage` on the ship. All of it is gameplay; behind an OFF switch.
- Lane: the host's landing path ends at the ramp unload; 0049A410's second
  `FinishLanding` caller would need the lane object. cc9's lane.
- Presentation (0048A010, 0048A1E0, fake effects): renderer side; keyed presentation streams.

## 10. Paratroopers (segment 46): gameplay reach

Follow-up for cc9, 2026-10-12, read-only (no names, no Ghidra writes), base origin/main
85360812c. These are the other SoldierGlobals users of section 2. **Verdict: gameplay.** A
paratrooper is a full game entity. It captures enemy command buildings and damages them, and two
of the three SoldierGlobals fields it uses decide whether, when and where it lands.

### 10.1 The objects

- **`MParatrooper`** (class id 31h, chain 31h -> 2Ah `MBomb` -> 29h `MBullet`; see
  `docs/ORDNANCE_KIND_IDENTITY.md`). It is 4BCh bytes. The constructor is 007AB8C0 (on top of
  `BSP_BombProjectile_Construct` 006E2670). Its primary vtable is 00D05060 (106 slots) and it has
  six secondary vtables. The +3E4h subobject's vtable is 00D04FD4, whose slot +8h is 007AB360 and
  slot +10h is 007AB590.
- **The `Paratrooper` bullet descriptor** has vtable 00CFA604. Its slots are:
  - +18h 007AC780, the Lua reader;
  - +20h 007AC2C0, create with a pose;
  - +24h 007AC4C0, create.
- **007AC780 fields** (after `BSP_BombClass_ReadLuaFields`):

  | field | offset |
  | --- | --- |
  | `CapturePower` | +D8h |
  | `SlowFactorClosed` | +DCh |
  | `SlowFactorOpened` | +E0h |
  | `OpenDuration` | +E4h |
  | `CloseDuration` | +E8h |
  | `SoldierClass` | +ECh (004B1400) |
  | `SoldierAnim` | +F0h |
  | `CaptureDuration` | +F8h |
  | `Damage` | +FCh |

  The key strings are at 00D05220..00D05280 and 00CE7164.
- **This installation's row:** `scripts\datatables\classtables\arcade\bulletclasses.lua:683-702`
  (`ArcadeTable[22]`, "JP Paratrooper"). It sets `CapturePower` 12, `CaptureDuration` 30,
  `Damage` 42, `SlowFactorClosed` 0.2, `SlowFactorOpened` 2, `OpenDuration` 1,
  `CloseDuration` 2, `SoldierClass` "USMarine_shiptraffic" and `SoldierAnim` "fire2".
  - This file is **locally modified** (mtime 2026-05-09 23:04:46).
  - The realistic table is also modified (2025-06-02). A second Paratrooper row starts at
    line 2638 of the arcade file.

### 10.2 What it does

| routine | slot | effect |
| --- | --- | --- |
| 007AB8C0 | constructor | `+468h = U(ParachuteOpenAltitudeMin, Max)` (stream 1, see 10.6); class id 31h at +C4h; clears the state bytes |
| 007ABAB0 | +130h | `BSP_GameEntity_RegisterInParentEntityList`, then `BSP_UnitList_PushBack` into the world's lists `[+30h]+30h`, `+210h`, `+264h` |
| 007ABAF0 | +134h | unregisters (erases from `+264h`) |
| 007AB230 / 007AB330 | +164h (message 71h) | the drop target `+47Ch` with y from `BSP_World_GroundHeightAt`; in multiplayer mode 1 routes message 71h and copies `[+3BCh]+180h` to `+4B8h`. Callers: `BSP_MBombPlatform_Drop` 006E4D50, 007AB8A0 |
| 007ABB30 | +19Ch | air drag scaled between SlowFactorClosed and SlowFactorOpened by the chute fraction `+46Ch`, then `BSP_BombProjectile_AdvanceInAir`; steers toward `+47Ch`, moved out of avoid zones once (`+488h`) |
| 007AB360 | sub +8h | when height above ground is at or below `+468h`, switches from "fall" to "open" (`+46Ch = 1`). On a target building `+48Ch`: same side kills itself (cause 2); enemy (`BSP_Party_RelativeTo` 00803510 == 1) calls the building's vslot +1ACh with `(Damage +FCh - building +368h) * dt` when positive (007AB509..007AB549). Then slot +19Ch |
| 007AC000 | +194h | landing: ground reached sets `+470h`. If the ground normal's `acos` exceeds SoldierGlobals `+10h` (`MaxParatrooperGradient`, 007AC28F..007AC2A5), `+471h` (bad landing) is set. Landed with the chute closed: a bad landing is killed; otherwise 006F2C30 picks the nearest enemy command building within its capture radius into `+48Ch` (none: killed), registers a fixed-step callback, and kills itself when `+474h >= CaptureDuration` |
| 007AB0B0 | +DCh | animation clock fall/open/close. Once "close" has played, every frame adds dt to `+474h` (the capture timer) and fires the soldier's muzzle effect 0048A1E0. Then the clip test 0048A010 on the soldier entity `+478h` |
| 007AB590 | sub +10h | once (`+490h`): adds itself to the building's capturer array (006F39B0, `+7DCh`/`+7E0h`), hides the chute, instantiates the `SoldierClass` entity into `+478h`, faces it at the building, plays `SoldierAnim` (004B0F80(.., 1, 0)), then the clip test |
| 007AAE10 | +7Ch | `BSP_UnitInstance_OnKilledLua`; kills `+478h`; removes itself from the building's capturers (006F34B0) |
| 007AB7D0 | +17Ch | network create message B9h (006E0D20) and `BSP_Entity_SendMessageToPeer` |
| 007AC530 | +A0h | loads the "fall", "open" and "close" animations and starts "fall" |

Neither 007AB0B0 nor 007AB330 has a Ghidra function; the bodies were read from the disk bytes.
007AB360 and 007AB590 run with ECX = paratrooper+3E4h.

### 10.3 Writes to non-paratrooper objects

| site | target |
| --- | --- |
| 007AB549 | enemy command building vslot +1ACh (damage per second; the building's override was not read) |
| 007AB5B6 (006F39B0) / 007AAE3E (006F34B0) | building capturer array `+7DCh`/`+7E0h`, under its critical section `+764h` |
| 007ABAB0 | the world entity lists |
| 007AB230 | multiplayer session message 71h |
| 007AB7D0 | network create message |
| 007AB590, 007AAE10, 007AC000 | the soldier entity it owns (`+478h`), and itself through `BSP_MissionEntity_Kill` |

### 10.4 Outside readers (rel32 and absolute-dword scans)

- **The command building's capture step.** 006F6760 runs from `BSP_FixedStepCallback_RepeatingStep`
  006F7360. It walks the capturer array and adds each capturer's class `CapturePower`
  (`[[cap+314h]+D8h]`) to the capturer's side `+54h`. It also reads `+4B8h` and vslot +108h, and
  reports captures (`BSP_WarningManager_ReportCaptureShip`). This is the capture path.
  `GetCapturePercentage` 0089B840 and `SetCaptureValue` 0089B0C0 are the Lua face of the same
  building. The building's other displacement readers: `BSP_RewardsScreen_Enter` 0062C2C0 and
  006F5AE0.
- **006F2C30** (nearest enemy command building in `[game+19CCh]+16Ch` within its radius
  +7C8h/+7A0h/+7C4h) has six callers. Only 007AC077 is the paratrooper's; the other five are
  006EB3B5, 0074A51A, 00812937, 008129C9 and 008207B5.
- **Descriptor and vtables.** The only absolute references are the descriptor slots 00CFA61C,
  00CFA624, 00CFA628 and the vtable slots. The constructor's only callers are 007AC30F and
  007AC50D.
- **Lua.** Deaths go through `BSP_UnitInstance_OnKilledLua`. This installation's
  `achievements.lua:45/829` counts "islands captured by paratroopers". No mission script uses a
  paratrooper-specific native; `IsLandingTrafficActive` is unrelated.
- **Other readers:** the AI `Paratroopers` hit-chance weights in `highlvlaiglobals.lua` and the
  levelbomb paradrop admission path (`docs/ORDNANCE_KIND_IDENTITY.md`).

### 10.5 Damage, capture, collision

- **Damage:** to enemy command buildings while landed (10.3).
- **Capture:** `CapturePower` while it is in the capturer array, until `CaptureDuration` runs out.
- **Its own death:** it is killed when:
  - it lands on a slope steeper than `MaxParatrooperGradient`;
  - it finds no enemy building in range;
  - it targets a friendly building;
  - its capture time ends.
  The last three are skipped in multiplayer mode 2.
- **Collision:** none of its own beyond the `MBomb` base (`BSP_BombProjectile_AdvanceInAir`,
  not read here).

### 10.6 RNG

| call | routine | draw | stream | class |
| --- | --- | --- | --- | --- |
| 007AB94A | 007AB8C0 | U(ParachuteOpenAltitudeMin, Max) into `+468h` | **1** (`XOR EBX,EBX` at 007AB8E9, `LEA ECX,[EBX+1]` at 007AB941) | gameplay-coupled: the chute opening height sets the descent and drift, so it decides landing time, place and slope check |
| 004B102F via 007AB6AC | 004B0F80 from 007AB590 | U(0, duration) of `SoldierAnim` (random start = 1) | 0 | presentation-only (animation phase) |

The scan covered every rel32 call to 00BD2F10, 00BD2FC0 and 00BD2ED0 in 007AAD00..007ACCFF;
007AB94A is the only direct one. The `MBomb`/`MBullet` base code was not scanned. The
stream-1 draw shares stream 1 with the other stream-1 users (the landing ship's 0074A9C8
draw in `BSP_LandingShip_BeginLandingAtPad`, the plane tasks). A host must keep it on stream 1
in image order, or put it behind an OFF switch; it is not presentation. The 004B0F80 phase draw
goes to a keyed presentation stream when wired.

### 10.7 Uncertain

- 00BF9940 is read as `acos`; only its use against a dot product in [-1, 1] establishes it.
- The building's vslot +1ACh override (the damage receiver) was not opened.
- `+4B8h` as the dropping player (capture attribution) is provisional.
- Multiplayer mode values: 1 sends message 71h; 2 skips the kills.
