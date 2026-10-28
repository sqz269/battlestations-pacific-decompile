# The aim state's release timer `009FA3A0`: its three host stubs (packet `cc13-reltimer`)

Addresses: `009FA3A0`, `009FA3C5`, `009FA3D0`, `009FA3D5`, `009FA3EA`, `009FA3F4`, `009FA3FB`,
`009FA405`, `007BB110`, `007BB133`, `007BBBA0`, `007CDC70`, `007CD9A0`, `007B9140`, `009D2DA0`,
`009D2F09`, `009D2F39`, `009D2F4D`, `009D15D0`, `009D15F0`, `009D2019`, `009D2027`, `009D2287`,
`00BD2F10`.

Code: `include/bsp/cc13_release_timer.hpp`, `src/cc13_release_timer.cpp`, and the binding
`AimTickBinding::accumulate_timer_009fa3a0` in `src/game_hosts_units.cpp`. The timer body stays
`bsp::release_timer_tick_009fa3a0` (`src/torpedo_first_release.cpp`). Three switches, all committed
OFF: `kReleaseTimerReseedDrawBound`, `kReleaseTimerPredicate007BB110Bound` (static_asserts the
reseed) and `kReleaseTimerNotify0DhBound`. Status: reconstructed, build-tested; the ON arms are
not game-validated (no runs in this packet).

## 1. The timer object

The aim state is `task+318h`, built inline by `009D2DA0` (`009D2F11 LEA EDI,[ESI+318h]`, vtable
`00D212FC` at `009D2F2B`). The timer is its member at `state+18h` (`009D2019 LEA EBP,[ESI+18h]`).
The constructor writes every timer field:

| timer | state | write | value |
|---|---|---|---|
| +0h | +18h | `009D2F41` | `[[state+4]+4]`, the unit |
| +4h | +1Ch | `009D2F34` from `009D2F09 MOVSS [00CE3800]` | 0.5f (`00 00 00 3F`) |
| +8h | +20h | `009D2F44` from `009D2F39 MOVSS [00D7A24C]` | 1.0f (`00 00 80 3F`) |
| +0Ch | +24h | `009D2F49` | 0 (enable) |
| +0Dh | +25h | `009D2F4D` | 0 (notify) |
| +10h | +28h | `009D2F51` | 0.0f (countdown) |

Nothing else writes `+4h`/`+8h` (the aim tick `009D15F0` only takes `EBP = state+18h` to pass it),
so the `009FA3EA` reseed is always `00BD2F10(ECX = 1, 0.5f, 1.0f)`. Both bounds are four-byte:
`MOVSS` at the stores, `FLD float` at `009FA3D5`/`009FA3E4`; the result is `FSTP float [ESI+10h]`
at `009FA3F1`.

The enter `009D15D0` clears `+24h` (`009D15E7`) but not `+25h`; only the constructor clears the
notify.

## 2. Who reads `timer+0Dh`: nobody

- Ghidra: `009FA3A0` has one caller, `009D15F0` (`009D2027`). The aim state is reached only through
  `task+318h` and its vtable (`009D2D80`, `009D15D0`, `009D1350`, `009D15F0`, `007B3DE0`, `007B3DF0`,
  `007B45E0`). None of those listings touches `+25h` or `[EBP+0Dh]`.
- Byte scans (`bsp.py scan-bytes`, `--limit 4000`, the spilled file read in full): `80 ?? 25 00`
  has 272 image hits (so the pattern is not vacuous), none in `009C0000`-`009D7FFF`; the band's only
  `+25h` byte access in the exported listings is the constructor's store at `009D2F4D`. `8A ?? 25`,
  `0F B6 ?? 25`, `0F BE ?? 25`, `84 ?? 25`, `F6 ?? 25`, `3A ?? 25` have no hit in the band.
- The task-relative form `task+33Dh` (disp32 `3D 03 00 00`) has no hit with `80`, `8A`, `0F B6`,
  `38`, `3A`, `84` or `F6`; `task+330h` (the timer's own address) is never taken (`8D ?? 30 03 00 00`
  none); `task+318h` is taken only by the constructor (`009D2F11`) and the destructor (`009D23DB`).

Verdict: the notify is a dead store in this image. Wiring it changes no behaviour;
`kReleaseTimerNotify0DhBound` only stores it on the slot and logs it.

## 3. What `007BB110` tests

```
007bb110: EAX = [ECX+DECh] + 60h                ; channel C of the actuator block
007bb119: CMP byte [EAX],0 / JZ -> AL = 1        ; channel disabled
007bb11e: UCOMISS [EAX+4], [00D7A24C] 1.0f; LAHF; TEST AH,44h; JP 007BB133
007bb130: AL = 1                                 ; ordered equal: bay fully open
007bb133: AL = ([ECX + 9C3h + 8*word[00F876B8]] == 0)
```

It refuses only while the bay channel is enabled, not exactly at 1.0f, and the step-record byte
`9C3h` is set. `007CDC70` publishes `9C3h` from `007CD9A0`, which walks the plane's children and
stops at the FIRST `IsKindOf(25h)` child whose platform record (`[[plane+538h]+94h][child+38Ch]`)
has `+0Dh` (`UseBayDoor`, `0096127A`) set. The answer is that one child's `vtable[210h](2Ah, 0)`,
whether or not it is loaded; a later bay-door rack is never asked, and with no such child the
answer is 0. So no release happens while the first bay-door rack holds a round and the bay is not
fully open. It does not test the ordnance kind or a release count.

`docs/TORPEDO_FIRST_RELEASE.md` said "the device at unit+DECh enabled, its +64h not 1.0, and the
per-slot byte clear". That conjunction is the REFUSAL condition with its last term inverted, and
this packet corrected it there. The code (`src/cc13_seg47.cpp` `unit_can_release_ordnance_007bb110`)
was already right.

A second or later release: the image allows it. `007BBBA0` does nothing but open the bay (only
when `9C3h` is set) and raise `unit+C20h`, so the timer fires again each time the reseeded countdown
runs out while the aim state is current and `007BB110` passes. The host substitution
(`ordnance_has_torpedo_2bh && torpedo_releases < 1`) caps it at one.

In this installation `UseBayDoor = true` is authored on 26 platforms of `vehicleclasses.lua`,
including platform 50 (the bomb platform) of the TBD, TBF, B5N and B6N rows. Only planes whose
class has `PartAnims.BayDoor` get channel C enabled (`wing_fold_class_007eabc0`, `bay_door_classes`
in the summary): 0 in the USN01, BSM01 and USN04 logs below, 495 in USN21, 18 in BSM09.

## 4. `unit+9C0h + 8*[00F876B8]`

`unit+9C0h` is a two-entry, stride-8 step record. `007CDC70` (`BSP_Plane_PublishStepRecord`) writes
the entry at the CURRENT buffer index `word[00E0B6CC]` each step: `+0` = `007B9140(plane, 0)`,
`+1` = channel C idle-or-open, `+2` = `unit+520h`, `+3` = `007CD9A0`, `+4` = `007BCC20` (float).
`word[00F876B8]` is the index before the per-step flip (`00875C93`), so `007BB110` and
`009FA3FB` read the entry published on the previous step.

`009FA3FB` reads `+0`: `007B9140(plane, 0)`, which answers 1 for kind 17h (`MPlaneKamikaze`,
`007B914A`), else whether any rack (`+974h`, count `+994h`) answers `vtable[210h](2Ah, 0)`. The
notify is set when the plane held no round at the previous publish. On the first fire the torpedo
is still aboard, so the notify stays clear.

Host data: there is no step record. `GameUnitsHost::plane_holds_rack_round_007b9140` and
`Impl::plane_rack_holds_round` model the rack walk (LABELLED counts); channel C is
`actuator_block_dec.channel_c`. The per-platform `UseBayDoor` was not in the host; this packet reads
it from the live `VehicleClass` table (section 6).

## 5. Host callers and switches

`AimTickBinding::accumulate_timer_009fa3a0` is the only binding. `torpedo_aim_tick_full_009d15f0`
reaches it through the call site `009D2027` (`src/torpedo_aim_tick.cpp:305`), from `run_torpedo_aim_tick_009d15f0`, which
the torpedo task runs whenever its current state is `kAim`. No switch routes around it. The arm
`009D2287` (`arm_release_timer_009d2287`) enables it with a zero countdown. A fire reaches
`Impl::release_ordnance_007bbba0`; with `kReleaseIssueStageBound` ON the drop then waits for the
rack, and `kBayRequestNoForceEnableBound` (ON) leaves a disabled channel C disabled. The other
`007BBBA0` sites (`009D25B3`, `009D26F8`, `009D2938`, `009D29CB`, `009D4956`) go through
`TorpedoArmBinding::request_ordnance_release`, untouched here.

## 6. The switches

`kReleaseTimerReseedDrawBound`. After a fire, `owner_.release_ordnance_007bbba0` runs first
(`009FA3D0`), then the countdown is `GameGunneryHost::death_mode_draw_00bd2f10(1, unit, 0.5f, 1.0f)`
(`009FA3EA`), the shared stream-1 generator, as the other cc13 stream-1 draws take it. Without a
gunnery host it leaves the `0.0f` and records `ReleaseTimer::reseed_bounds`. One line per draw:
`release timer reseed: unit=... t=... countdown=... (009FA3EA 00BD2F10 stream 1)`.

What this switch fixes is the countdown VALUE: U([timer+4h], [timer+8h]) = U(0.5f, 1.0f) instead of
0.0f. It does not reproduce the image's draw sequence. In the image, stream 1 is per-thread and
reseeded from the clock, so its exact sequence is not reproducible
(`docs/CC13_RNG_STREAM_CENSUS.md`), and no host draw order can match it bit for bit. The draw still
has its own sub-switch, because in the host it consumes the shared stream and so shifts every later
host stream-1 draw.

`kReleaseTimerPredicate007BB110Bound`. `bsp::cc13_release_timer::unit_can_release_007bb110(
channel_c.enabled, channel_c.value, bay)` replaces the substitution. `bay` is computed only when the
first two arms do not decide (the image's short-circuit): the first rack, in host rack order, whose
`bomb_rack_bay_doors` entry is set answers `plane_rack_holds_round`; no such rack answers 0.
`bomb_rack_bay_doors` runs a Lua chunk over `VehicleClass[type].BSPGun` p1..pn reading
`Platforms[p<i>_key].UseBayDoor == true`, then keeps the rows `ordnance_racks` keeps. The flags
depend only on the class, so they are cached per (Lua host, vehicle class) in
`src/cc13_release_timer.cpp`. The chunk runs once per class rather than on every refused tick, and
so overwrites the Lua host's `last_status_`/`last_chunk_` only once per class. A result is cached
only once BSPGun exists (the class's `BSPGun.n` is positive, so the gunnery flatten has run) and the
chunk succeeded. A failed load or call answers all-false and is retried on the next refused tick. It static_asserts
the reseed: with the real test and a zero countdown the timer would fire on every tick.

`kReleaseTimerNotify0DhBound`. In the fire arm, after the draw, `[9C0h] == 0` is evaluated as
"not kind 17h and no rack (class read) holds a round", the walk of
`GameUnitsHost::plane_holds_rack_round_007b9140`, and `torpedo_aim_timer_notify_0d` is set. One line
per store: `release timer notify: unit=... (009FA405, no reader)`.

OFF, each of the three keeps the old code path verbatim: the same `log.unimplemented` records, the
same inputs to `release_timer_tick_009fa3a0`, the same countdown, stored before
`release_ordnance_007bbba0` as before. The new slot field `torpedo_aim_timer_notify_0d` is written
only ON. ON, the predicate (and its implemented `Unit::can_release_007bb110` record) runs only on a
tick where the image consults it, a strictly negative countdown of an enabled timer; the reseed and
the notify run only in the fire arm.

## 7. ON-only lookups: container, filler, clearing

| lookup | container | filled by | cleared or moved before the read? |
|---|---|---|---|
| channel C | `GameUnitSlot::actuator_block_dec.channel_c` | `wing_fold_class_007eabc0` at the plane's first block step (`PartAnims.BayDoor`); stepped by `plane_actuator_block_step_007de3a0`; opened by `release_ordnance_007bbba0` | no; the slot lives for the unit. Before the plane's first block step the channel reads the default (disabled), so the test answers 1; not checked whether that step always precedes the first aim tick. |
| `BSPGun` rows | Lua `VehicleClass[type].BSPGun` | the gunnery host's flatten (`src/game_hosts_gunnery.cpp`, `row.BSPGun = f`) at mission load | no; nothing assigns `BSPGun` afterwards. `ordnance_racks` already depends on it. |
| `Platforms[k].UseBayDoor` | Lua `VehicleClass[type].Platforms` | `vehicleclasses.lua` (autoload) | no; read-only datatable. |
| rack rounds | `GameUnitSlot::rack_ammo_per_rack` / `rack_ammo`, else the authored Ammo | the release issue seed and the rack tick | spent by the rack tick (`--rack_ammo_per_rack`) before the next aim tick, which is the step-record lag below |
| draw | the gunnery host's stream-1 generator | `GameGunneryHost` | shared: every draw moves every later stream-1 consumer |

## 8. Simplifications and uncertainties

- The step record is read live, not one step late. The image reads the entry published the step
  before; a round spent earlier in the same step is still counted there. LABELLED.
- `9C3h`'s rack order: the image walks the device list `unit+48h`; the host's `ordnance_racks`
  order (BSPGun rows) stands for it. Most authored bay-door classes have one bay-door platform
  (50); the B6N and G4M rows have two (50 and 51), where the order can matter.
- `007B9140(plane, 0)` against the host's flag-1 model: `vtable[210h](2Ah, flag)` differs only with
  `[00E17BF2]` set (multiplayer); single player is the same.
- The notify persists on the slot across torpedo tasks; the image clears it per task constructor
  (`009D2F4D`). No reader, so no effect.
- Who opens the bay before the aim fire for a class with `PartAnims.BayDoor` is not settled here.
  With the predicate ON such a plane is refused while channel C is below 1.0f, and the timer keeps
  trying every tick (countdown stays negative). `timer_blocked_007BB110` in the aim tick summary
  counts those refusals.
- x87: one `FSUB` then a `float` store at `009FA3B0`, and the draw stored `float`; 24 and 53 bits
  give the same float. Modelled as float.

## 9. Reach in the WINS logs

From the per-plane `aim tick 009D15F0` summary lines of the reference logs cc13-stubcensus used
(last row per plane; `local/reltimer_count.py`):

| mission | log | planes with aim ticks | arms | timed fires (= added draws, reseed ON) | channel C bay classes |
|---|---|---|---|---|---|
| JM08 | cc9-gunnery52 `g53_lfoff_j8.log` | 0 | 0 | 0 | - |
| USN04 | cc9-physics1 `p23h_off_u04.log` | 16 | 0 | 0 | 0 |
| USN21 | cc9-physics1 `p23j_off_u21f_def.log` | 234 | 1 | 1 | 495 |
| USN01 | cc9-physics1 `p23j_off_u1i_def.log` | 10 | 5 | 5 | 0 |
| BSM09 | cc9-physics1 `p23j_off_b9_def.log` | 19 | 1 | 1 | 18 |
| BSM01 | cc9-lua52 `l69r_b1c_main_ref.log` | 380 | 46 | 45 | 0 |
| IJN08, USN15, USNOS, LOMP06, JM06 | | 0 | 0 | 0 | - |

The binding is reached in five missions (USN04, USN21, USN01, BSM09, BSM01); it fires in four,
52 fires in all, at most one per plane. Every fired plane left the aim state on the tick after its
fire (aim ticks minus first-fire tick = 1 in all 52), so neither the reseed nor the real predicate
gets a second chance in these runs.

Expected ON effect:
- Reseed: one more stream-1 draw per fire (1 / 5 / 1 / 45 in USN21 / USN01 / BSM09 / BSM01), so
  every later host stream-1 draw in those missions shifts (RNG-coupled divergence: AA, death modes,
  rack delays). That shift belongs to the host's shared stream and is not a fidelity target, since the
  image's stream 1 is clock-seeded (section 6). The `timer_009FA3A0=` column of the summary moves from 0.00 to the drawn value less the
  ticks spent after it. New `release timer reseed` lines.
- Predicate: USN01 and BSM01 have no enabled channel C, so `007BB110` answers 1 and the first fires
  are unchanged. In USN21 and BSM09 a bay-door plane is refused until its bay is at 1.0f:
  `timer_blocked_007BB110` can rise and `first_fire_at_aim_tick` move or vanish. The
  `Unit::can_release_007bb110` unimplemented record is replaced by an implemented one.
- Notify: no behaviour; a `release timer notify` line only for a fire with no round aboard (none
  expected, since every fire above is a first fire).

## 10. The bay opener (packet `cc13-reltimer2`, switch `kReleaseBayOpenCc13Bound`)

physics24's pairs showed `kReleaseTimerPredicate007BB110Bound` failing by mechanism: BSM09 ref
`timer_blocked=378`, `timer_fires=0`, torpedo drops 3 -> 0 and the mission lost; USN21 def drops
5 -> 0. `007BB110` refuses while channel C (`[unit+DECh]+60h`) is enabled, not at exactly 1.0f, and
`9C3h` is set, and the only host writer of the channel's target was `007BBBA0`, which runs behind
that very test. The image opens the bay earlier, in two pieces.

### 10.1 `0099AFCC-0099B055` in `0099ACD0` BSP_PilotBot_Tick: `unit+DF0h`

- `0099AFCC-0099B003`: `mode` = the first task in the bot's list `bot+58h/+5Ch` whose `vtable[+50h]()`
  is not 1; 1 when none answers otherwise.
- `0099B006 CMP EBP,2 / SETE DL`: flag = (mode == 2), kept at `[ESP+10h]` (read as `[ESP+0Ch]` after
  `0099B019 POP EBP`).
- `0099B012-0099B049`: in mode 1 only, when `[unit+9D4h]` (the squadron) and its `+3D0h` (the
  leader) are set and the leader is not this unit, flag = the leader's step byte `9C1h` (channel C
  disabled or at exactly 1.0f; `007CDC70`).
- `0099B055` reaches `007BB150`: `unit+DF0h` = flag ? `9C3h` : 0.
- `0099B00C` also stores `unit+DF8h` = mode; that store is not modelled (the opener reads only
  `DF0h`).
- Torpedo task (vtable `00D213C8`, slot `+50h` at `00D21418`) = `009D49E0`: in the aim (`task+710h`),
  done (`+618h`), goaway (`+6D8h`), attackrun (`+6B4h`) and prepare (`+740h`) states it answers
  `task+528h ? 2 : 0`; elsewhere 1. `task+528h` is `approach+130h`, the aim-solution byte the aim
  tick writes at `009D2021` (interpolated range * speed + 200 > range). Its only other writers are
  `009D05AB` in `009D0380` BSP_BotApproachTorpedo_Reset (0, reached only from `009D2DA0` at task
  construction) and the unreferenced setter `009D1330`.
- Dive-bomb task slot `+50h` = `009C8290`: in the attacking states (`009C7910`: aimdive, aimglide,
  done, flyabove, goaway, prepare, attackrun, turndown) it answers 2 in aimdive and 0 in the rest;
  elsewhere 1.

### 10.2 `007C6A8D-007C6B2E` in `007C6820` BSP_Plane_DriveActuatorChannels: channel C's target

Skipped entirely when `unit+520h` is set. With drop-kamikaze ordnance (`007B91C0(2Fh, 0)`) the
channel is forced open: `+64h` = 1.0f, `+61h` = 1, `+68h` = 1, unless the target is already 1 and
the value exactly 1.0f. Otherwise `BL` = (`+DF0h` || `+C25h` || `+C20h`) ? `9C3h` : 0 and the
target `+61h` = `BL`; a change marks `+68h`, and a moving channel raises the block's `+11h`. The
block step then drives the value toward the target, and once it reaches 1.0f `007BB110` passes.
Other `+61h` writers found by rel32 + absolute scans: `007BBBA0` and the unreferenced `007B8840`.
`007BB150`'s rel32 callers are `0060887C` (HUD) and `0099B055`.

So the opener is the pilot think's `DF0h` plus the plane tick, and its trigger is the torpedo aim
solution (release distance + 200 m) in the aim state, or a dive bomber entering aimdive. The host
task state that reaches it: `torpedo_state` aim/done/goaway/attackrun/prepare with
`torpedo_approach.aim_solution_130` set (written by the host's aim tick), or `dive_bomb_state`
kAimDive.

### 10.3 Host binding

`GameUnitsHost::Impl::bay_open_step_cc13` (`src/game_hosts_units.cpp`), run per plane in the block
step loop just before `plane_actuator_block_step_007de3a0`, under `kReleaseBayOpenCc13Bound`.
The pure pieces are `pilot_step_flag_df0_0099b055`, `step_byte_9c1_007cdc84` and
`plane_bay_target_007c6a8d` (`src/cc13_release_timer.cpp`). The torpedo task install also clears
`aim_solution_130` (`009D05AB`). `kReleaseTimerPredicate007BB110Bound` now `static_assert`s this
switch, so the predicate cannot be flipped alone. New lines: `bay target:` per target write and
`summary bay opener steps= target_writes=`.

Substitutions, labelled:
- `DF0h` is refreshed every fixed step, not on the pilot think's own timer.
- `9C1h`/`9C3h` are read live from the channel and the racks, not from the previous step record
  (`[00F876B8]`).
- `mode` is modelled for the torpedo (`009D49E0`) and dive-bomb (`009C8290`) tasks only; a plane
  with neither installed is left alone (its target is not touched). The other tasks whose `+50h`
  can answer other than 1 are unmodelled mode sources: LevelBomb `009B8450`, Rocket `007B6F90`,
  CloseToShip `009A2BD0`, DepthCharge `009A5D80` and DropKamikaze `009ADDB0`. That is no regression,
  because the predicate it serves is read by the torpedo release timer only.
  Packet cc13-lbbay (2026-10-28) models LevelBomb `009B8450` under `kLevelBombBayOpenerCc13Bound`
  (`include/bsp/level_bomb_bay_cc13.hpp`, OFF). In attackrun, aim, prepare, release and goaway it
  answers 2 or 0 from task+4C4h = approach+CCh (`lb_ap.close_cc`), and 1 otherwise. The same packet
  applies `007CC8E0`'s bay test (`007BB110` inlined, `007CC8F4-007CC928`) to the level bomber's rack
  tick under `kLevelBombDropBayGateCc13Bound`, which `static_assert`s the opener. After that packet,
  the predicate is no longer read by the torpedo release timer only.
- The opener runs before the block step in the same fixed step; the image's order between the
  pilot think, the plane tick and the block step is not established.

ON-only lookups (container, filler, clearing):
- `plane_squadron_registry().find_by_member_unit(index)` for the leader: filled by the squadron
  spawns (`game_hosts_script_orders.cpp`, `game_hosts_ai.cpp`), a held wing slot stored as
  `kPlaneSquadronNoUnit`, cleared with the squadron record. When the leader dies, the host does move
  the next wingman to `+3D0h`: `remove_member_unit_007f3970` erases the seat. The only difference is
  a window under `kSquadronRemovesDeadBound`. In it, the death path has already cleared the seat to
  `kPlaneSquadronNoUnit` before the erase, while the image still holds the old pointer. Under that
  switch the opener therefore takes the first occupied seat (`flight_leader()`), and without it
  `member_units[0]`. A leader that is `kPlaneSquadronNoUnit`, out of range, empty or this unit is
  ignored.
- `bay_rack_holds_round_9c3(s)`: the racks and the cached `bomb_rack_bay_doors` flags (section 6),
  filled at the gunnery flatten, read per call.
- `torpedo_state`/`dive_bomb_state` and `aim_solution_130`: set at the task install and by the aim
  tick, cleared at the install (`009D05AB`).

### 10.4 Reach and expected ON effect

The host's predicate is read only by the torpedo release timer (`AimTickBinding`), so the opener
changes releases only for torpedo bombers; for dive bombers it moves channel C (aimdive) but no
host dive release reads `007BB110`. physics24's predicate-ON logs
(`cc9-physics1/local/p24h2_*`) list the planes that were refused, all in the aim state with the
timer enabled, i.e. with the aim solution set, so each would now get `DF0h` = `9C3h` = 1, a channel
C target of 1, and a release once the channel reaches 1.0f:
- BSM09 ref: 13 planes refused (Yorktown_sqn01/-II/-III, sqn11/-II, sqn16/-II/-III, sqn17/-II/-III,
  sqn18, sqn19); OFF fired Yorktown_sqn01|.-II and .-III.
- BSM09 def: 8 refused (Yorktown_sqn01|.-II, sqn12/-II, sqn16/-II, sqn18/-II, sqn24); OFF fired
  Yorktown_sqn12|.-III, sqn16|.-III, sqn18|.-III, sqn21.
- USN21 ref: 7 refused (Intrepid_sqn31/-II/-III, Cabot_sqn56/-II/-III, Lexington_sqn94); OFF fired
  Intrepid_sqn31|.-III and Cabot_sqn56|.-III.
- USN21 def: 7 refused (Intrepid_sqn31/-II/-III, Cabot_sqn56/-II/-III, Franklin_sqn60); OFF fired
  Enterprise_sqn23|.-III and Cabot_sqn56|.-III.

No dive release is gated by the predicate in these missions. Expected with all three switches ON:
fires return in those squadrons, delayed by the bay travel (channel C rate 1.0, so about one second
after the solution) and then spaced by the 0.5-1.0 s reseed, possibly more than one fire per plane.
Wingmen in mode 1 (outside the listed states) take the leader's `9C1h`, so a wingman whose leader's
channel is disabled or fully open opens its own loaded bay early; that is the image's rule. With
only this switch ON (predicate OFF) the substituted predicate ignores channel C, so releases are
unchanged and only `bay target:` lines and channel C motion appear.
