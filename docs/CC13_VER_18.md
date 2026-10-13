# CC13 VER 18: PRCP01 Junyo launch brake

Packet cc13-ver18 follows up the open item from docs/CC13_VER_15.md section 4. In PRCP01, Junyo's
launch brake (block+38h) was set at frame 1560 (78.05 s) and stayed set until the end of the run
(`held_at_end=2`), and IsReadyToSendPlanes answered false 202 times after that.

This was a static investigation: no new runs, no code change. The evidence is:
- image listings 006C5B70, 007ED740, 007B8BD0, 006CDC70, 006C5050 and 007F2C60;
- host files at origin/main 79b24f1df;
- the cc9 log `J:\PROG\battlestations-pacific-decompile-cc9\local\lead_kl2_off_PRCP01.log`;
- this installation's `scripts/missions/COTP-USN/PRCPUS/usn_03_battle_of_santa_cruz.lua`
  (mtime 2024-07-13, untouched bulk).

## 1. What the brake is in the image

| step | address | what it does |
|---|---|---|
| set | 006C5314-006C5348 (inside 006C5050) | For a flag-0 launch, stores the squadron *object pointer* in `[block+24h]+14h` = block+38h and registers an observer pair on it. |
| clear, path 1 | 006C5B70 (006C5B8C-006C5C0A) | Holds `ECX = [block+38h]`, the squadron object, and calls 007ED740 on it. If that returns true, it republishes every slot whose +28h equals block+38h (006BF150), unregisters the pair (006952A0) and zeroes block+38h (006C5C0A). |
| clear, path 2 | 0065AFF0 | The observer pair zeroes +14h when the observed squadron notifies, which happens when it is destroyed. |
| condition | 007ED740 | Returns `+3CCh > 0` and 007B8BD0 true for every one of the +3CCh members at +3D0h. |
| member test | 007B8BD0 | True when `(+72Ch)->vtable[38h]` is true (+900h == 7, airborne), when +900h == 6, or when +900h == 4 and +BF8h is set. State 5 and every other state give false. |
| gate | 006CDC70 | Runs 006C5B70 only when block+1Ch and +1Dh are clear, block+7Ch (the owner) is non-null and owner+5Dh is clear. It runs *after* 006C64B0 (the queued-slot launch) and 006CD240 in the same call. |
| squadron count | 007F2C60 (007F2D9D) | The squadron constructor zeroes +3CCh. +3CCh only grows at 007F4B60 (the spawn tail of 007F4580, the attach), 007F4FCE and 007ED144. |

Two things in the image tie the brake to a specific object:
- The block belongs to one mother-ship instance. 007593D0 BSP_MotherShip_InitAll calls 006CADD0
  for each instance.
- LaunchSquadron (0089E3C0) resolves its entity argument through 006BCD20 to *that entity's own*
  block. 006C5050 then reads the owner only through block+7Ch (006C50FC-006C5177: Party +54h,
  +58h, +188h, HomeBase via 00926420).

So a regenerated carrier gets its own block, owner, holder and elevator, and its squadrons are
built at, raised by and released against that live instance.

The host's 006C5B70 / 007ED740 substitution (`release_launch_brake_006c5b70`,
src/game_hosts_units.cpp:15448-15510) follows these predicates and **is not the cause**. It
cannot release Junyo's brake because Junyo's members never leave state 1 (section 2).

## 2. Root cause (A): host binds a regenerated carrier's air ops to the first, dead instance by name

What happens to the carriers in PRCP01:
- `luaEngineMovieInit` generates Junyo and Zuiho (ids 6 and 9).
- Phase 0 kills every Japanese ship (script line 599-601, `Kill(unit, true)`). The log shows
  `entity dead: unit=5 "Junyo" died=3.05 s` and `unit=8 "Zuiho"`.
- `luaUS03GotoPhase3` calls `GenerateObject("Junyo")` again (script line 1135). That makes id 26
  (unit 25) at the same authored position, and `Mission.Junyo` is that new entity.
- Zuiho is regenerated later in the same way (log line 69804).

The host keys a carrier's air ops by *unit name* and resolves the owner as the **first** unit row
with that name. That row is the dead instance, still sinking:
- `src/game_hosts_script_orders.cpp:771-778` (`create_air_ops_squadron_006c5050`) gives the owner,
  and so the spawn position. Junyo_sqn03 is made at (500.7 **-58.1** -2529.6), which is unit 5's
  sinking hull. Unit 5 reaches kill depth at 107.60 s with sinkTime 104.55. Zuiho_sqn07 is made at
  y = -247.3 in the same way.
- `src/game_hosts_units.cpp:13699-13707` (`base_launch_task_tick_007f1f00`) uses
  `owner = first slot named "Junyo"`, and then `gunnery->unit_dead(owner)` returns on every tick.
  The launch task therefore never sends a member to the site (Enterprise logs
  `base launch task: ... sends`; Junyo logs none), and no member leaves +900h == 1. Junyo's elevator
  summary reads `releases=0 top_releases=0`.
- `src/game_hosts_units.cpp:12452-12468` (`landing_deck_006c0750`) builds the holder once, with
  `d.owner` = the first instance, and never rebuilds it for the new owner. The Junyo holder was
  built at log line 8832, before the new Junyo existed.
- The same first-match rule appears at `src/game_hosts_units.cpp:11909-11910`, `11948-11949` and
  `12626-12627`, and at `src/game_hosts_lua.cpp:4042-4049` (006C5101's owner skill). The skill
  comes from the dead instance's scene skill (0). The image uses the live instance's skill, which
  the script set with `SetSkillLevel(Mission.Junyo, SKILL_SPNORMAL)` at line 1142.
- `src/game_hosts_lua.cpp:3264-3266` adds to this. `GenerateObject` re-registers the deck under
  the same name (`AirOpsDeckRegistry::set` replaces the row in place, and `bind_entity_id(26,
  "Junyo")` points at it), so ids 6 and 26 both reach one deck whose owner state belongs to unit 5.

The consequence: 007ED740 sees three members in state 1 forever, so 007B8BD0 answers false. The
dead instance never notifies the observer pair, because the host carries no observer. So
block+38h stays set. The two `held_at_end` decks are Junyo (set at 78.05 s) and Zuiho (set when
Zuiho_sqn07 was made, log line 76032).

**Image vs host.**
- Image: LaunchSquadron(Mission.Junyo) goes to id 26's own block. Its owner is the live Junyo, so
  the squadron is built in that hull, its members are raised by its own elevator at the
  AirField/PlaneSendInterval cadence, and they take off (+900h 7).
- Image: 007ED740 then answers true and 006C5B70 clears block+38h.
- Host: everything is bound to the dead unit 5, so the brake never clears.

Confidence:
- High on the mechanism: the host code is first-match by name, and the log shows the spawn depth
  and the missing launch-task sends.
- High that the image binds by object: 006BCD20 by entity, and block+7Ch in 006C5050.

## 3. Second divergence (B): premature release on the 006C64B0 launch path

All six Enterprise releases come 8-9 log lines after their squadron's creation, in the same tick:
- sqn01 at 30.90 s and sqn02 at 31.90 s;
- sqn04 at 80.90 s and sqn05 at 81.90 s;
- sqn06 at 330.89 s;
- sqn08 at 482.86 s.

These launches go through `air ops queued launch` (006C64B0 -> 006C7490 -> 006C5050). In the same
006CD240 tick, the host's release loop sees:
- the fused flight leader with its create-time seed, +900h = 7;
- wings 1-2 still `kPlaneSquadronNoUnit`, because they are held for pass A and skipped.

That gives `resolved=1, every=true`, so it releases at once (`src/game_hosts_units.cpp:15478-15496`).

In the image, 006C5B70 runs in the same 006CDC70 call, right after 006C64B0. The squadron object
is fresh, and its +3CCh is 0: 007F2D9D zeroes it, and the attach that increments it at 007F4B60 is
the next InitAll, per the host's own cc9_sentity_init_all reading. So 007ED740 returns
`0 < 0` = false and the brake **holds**. Later it releases only when all three members are
airborne (7), in state 6, or in state 4 on the ground.

The Junyo launch is not affected. It comes from the LaunchSquadron binding, whose InitAll at
0089E613 runs the attach inline, and the host log shows no premature release for it.

Confidence:
- High on the image predicate and the host order.
- Medium on the claim that no attach runs inside 006C5050 itself. That relies on the cc9 reading of
  004F0AD0's deferral, which I did not re-read here.

## 4. Predicted PRCP01 effect (image vs current host)

**A (Junyo and Zuiho).**
- Junyo_sqn03, fighters of class 150, starts inside the live Junyo. Its three planes are raised and
  launched, and the brake clears when the third is airborne. By Enterprise's cadence (raises about
  12 s apart) that is roughly 115-125 s.
- `luaAirfieldManager(Mission.Junyo, {150}, {158, 162}, Mission.USUnits, 700)` (script line 730)
  then sees `IsReadyToSendPlanes` true and launches a strike squadron of class 158 or 162. It keeps
  up to NumSlots = 2 squadrons up, against `Mission.USUnits`, replacing losses, until Junyo dies.
  Junyo died at 428.47 s in this run.
- If Junyo lives past `luaUS03JunyoAngry` (600 s after phase 3), it goes ELITE with 8 slots and
  the Enterprise group goes SKILL_STUN.
- Zuiho does the same from its regeneration until it dies (733.64 s in this run).
- So **yes: in the image the Japanese carriers launch several more strikes**. At present the host
  gives exactly one stuck fighter squadron per carrier.
- Phase 3 fails on `Mission.Enterprise.Dead` (script line 734-739). The extra fighters also defend
  Junyo against the player's strikes, so Junyo's death time and Enterprise's survival both move.
- **The current PRCP01 WIN must be treated as provisional** until A is adopted and the pair is
  re-judged.
- RNG: every extra launch makes one `luaRnd` class pick (commandhelpers.lua:7872). I have not
  verified which stream luaRnd draws from. More planes also add gunnery and AA draws on the shared
  stream, so pairs will not be same-RNG.

**B (Enterprise cadence).**
- Each queued Enterprise launch holds the deck until its three planes are airborne. With raises at
  33.55, 45.55 and 57.55 s plus the takeoff, that is about 60 s for sqn01. 006C64B0 then launches
  the next queued slot.
- So sqn02 moves from 31.9 s to about 60 s, and sqn05 moves about 30-40 s after sqn04.
- The US strike cadence is slower. This tends to make PRCP01 harder, in the same direction as A.

## 5. Adoption notes (no edit made: the files are leased)

`lease check` at 2026-10-13:
- `src/game_hosts_units.cpp`: cc13-ver17.
- `src/game_hosts_script_orders.cpp`: cc13-fix7b.
- `src/air_operations.cpp` and `include/bsp/air_operations.hpp`: cc13-fix7.
- `src/game_hosts_lua.cpp`: unleased, but it holds only the skill site, which has no effect until A
  lands.

**A: `kAirOpsOwnerByInstanceBound` (OFF).**
- Resolve a deck's owner as the instance the deck was last registered for, not as the first row
  with the name. `AirOpsDeckRegistry` already records `bind_entity_id(entity, name)`. Add a reverse
  `owner_entity_id(name)` that returns the most recently bound id, and use unit = id - 1 (the host's
  entity-id rule; log: id 26 is unit 25).
- Use it at script_orders.cpp:771-778; units.cpp:11909, 11948, 12626 and 13701; and lua.cpp:4042.
- In `landing_deck_006c0750`, rebuild the holder and elevator when `owner` differs from `d.owner`.
  This matches 007593D0 building a fresh holder for the new instance.
- Minimal equivalent for PRCP01: take the *last* matching row that is not dead. This is enough for
  this mission, but it is not the image's rule.
- With the switch OFF nothing changes. With it ON, only a mission that regenerates a carrier under
  its old name gets a different owner; every other mission resolves the same unit as today.

**B: `kBaseLaunchBrakeAttachGateBound` (OFF).**
- In `release_launch_brake_006c5b70`, hold the brake while any `member_units` entry is still
  `kPlaneSquadronNoUnit`, or while the squadron's pass C has not yet applied bag `State` 1 (the
  `base launch inside start` point). This stands in for the image's `+3CCh == 0` before the attach.
- Expected: the Enterprise release times above move to after each squadron's third takeoff.

## Status

- Exported: the listings named above were read.
- Reconstructed: nothing new.
- Build-tested and game-validated: no. No run was made, and the predictions are static.
- Ledger and Ghidra: no renames. The existing names fit.
