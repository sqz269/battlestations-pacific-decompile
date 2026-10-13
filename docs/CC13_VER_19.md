# CC13 VER 19: the anti-aircraft target pick, image against host

Addresses: `00864FE0` (BSP_UnitGunneryAi_Tick), `00863990`, `008633D0`, `00862820`, `00727BD0`,
`00863640`, `007788D0`, `008636A0`, `008627A0`, `008527A0`, `008527E0`, `00729BC0`, `00729B90`,
`005459E0`, `008FBE00`, `00727F10`, `00427EB0`, `00861C20`, `008945F0`, `0070C0B0`.

Packet cc13-ver19, 2026-10-13. Read-only verification of which aircraft a ship's AA guns pick.
docs/CC13_VER_16.md covered flak lethality; this covers the pick, `00864FE0` and its callees.
Evidence is `bsp.py show` pseudocode and `disasm-raw` of the disk image. No game runs were made.
The host side is `src/game_hosts_gunnery.cpp` (cc9's, read only), class `GunneryPassBinding`,
over the reconstructed pass `src/unit_gunnery_pass.cpp`.

Status of the new code: `src/aa_pick_ver_cc13.cpp` and `include/bsp/aa_pick_ver_cc13.hpp` are
**reconstructed and build-tested**. They are pure fragments and nothing calls them. They are not
ABI-compatible and not game-validated.

## 1. What was already reconstructed, and checked again here

`unit_gunnery_pass_tick_00864fe0` already projects the whole body. I re-read the pseudocode of
`00864FE0` end to end and compared it step by step:

- **Cadence and latch.** The tick runs every `GlobalConfig+88h` seconds; on the host that is `think_time`.
- **The `+59h` sweep suppressor.** `00864FE0` clears it when `unit+61h` is 0. `unit+61h` has no writer, so the sweep always runs.
- **Sweep gate.** The sweep needs `this+70h+i` and `this+80h+i*4`. Category 7 also needs `this+7Ch`, and category 7 never sweeps.
- **The `this+60h` slot-4 gate.** It is checked per category, before the gather.
- **The sweep chain.** It runs `00863990`, then `00862440`, then `00864D90`.
- **The ordered insert.** At `008652B2`/`008652C5` the new entry goes before the first entry of a lower rank, or of equal rank and smaller distance.
- **The walk.** It runs from the end of the order array (`00865789`, `00865822`): rank 1 first, the nearest first within a rank.
- **The director arms.** The newest command target (`0071EBF0` -> `00521EA0`) is skipped when it equals the fire target. The fire target is `this+5Ch` slot 4. Both are scored by `00863990` as a whole, then expanded through `target->vtable[FCh]`. Each element is tested for visibility only and appended unsorted, so the fire target's elements are tried first of all.
- **Per gun.** `005459E0` (kind 5/6) together with IsKindOf(0Fh) applies the `00729B90` MinRange skip against the sort distance. That distance includes the +100.0 leader penalty (`0x4059000000000000`, read at `00D7A220`). Then `00729BC0`. Category 7 refuses the fire target unless `this+7Dh` is set. Then `00727F10`. Kinds 2, 3, 4 and 6 add a target record. With no candidate, `00728000` clears the gun.

`00863990` (asm `00863990..00863A73`):
- It needs `008633D0`, and `[ai+50h]` must answer `IsKindOf(5)`.
- The distance is pose to pose: unit `+FCh` to target `+FCh`.
- The range test is strict: `distance < unit+430h+i*4`.
- `+100.0` is added when the target answers `IsKindOf(0Fh)` and `007B8AD0` (`[t+9D8h]==0`).

`00862820` checks the four liveness bytes, then the per-instance allow byte `[ai+CCh+cat*61h+class]`, then the rank, then the category-7 and kind-8 arms. The allow byte's only writer is `00861C20`, called only from the Lua native `SetWeaponDirectorTargetEnable` (`008945F0`). No script in this installation calls it (docs/DEPTHCHARGE_FLAK_ADMISSION.md), so the host's "taken as set" is faithful here.

`00727BD0` (asm `00727BD0..00727C1F`):
- The rank is 1 for the first authored id. It advances only past non-zero ids.
- The AAMACHINEGUN row (`00E0944C`) ranks MPlaneKamikaze 17h, then TorpedoBomber 11h, DiveBomber 12h, 15h, 16h, Bomber 10h, Fighter 13h, then TorpedoBoat 0Eh and the soft surface classes.
- FLAK (`00E09A5C`) is the same, with 14h after 16h, and has no ships except the torpedo boat.
- LIGHTARTILLERYFLAK (`00E09BE0`) ranks air first, then every surface class.

**Answers to the packet's questions, image side:**
- **Priority** is the authored class rank, then distance. Within a rank, a wing member up to 100 m farther than its flight leader is preferred over the leader.
- **Range** is the category range on pose distance, then per gun the round's `+60h`.
- **Altitude** has no term of its own. It enters only through the 3D distance, the fire window and the kind 5/6 MinRange.
- **Attacker first:** there is none. No term reads who is attacking the ship.
- **The director's fire target outranks everything.** It is a ship, because 009F5B70 requires `IsKindOf(6)`. Its elements are tried before every swept plane by any category whose rank row lists it and whose range reaches it: AAMACHINEGUN and LIGHTARTILLERYFLAK do.
- **Sticky retargeting:** there is none. Each body re-walks from scratch, and `00727F10` re-sends the same target.

The host reproduces all of this. The divergences below are in host callbacks, not in the
reconstructed rule table.

## 2. Divergences

**D1. Director-arm candidate distance origin.**
- Image: `008654E1` `MOV ECX,[esp+1Ch]` (the unit, stored at `00865042`) `CALL 00427EB0` gives unit `+FCh`. The second arm uses `piVar2[3Fh..41h]`, also unit `+FCh`. Each element is measured at its own `+FCh` (pseudocode `iVar7 + 0xfc` in both arms).
- Host: `unit_world_position_00427eb0` and `entity_world_position` both return `unit_aim_point`, the pose raised by the class `Height` (game_hosts_gunnery.cpp:5578, :5636).
- **Effect on the pick:** this distance feeds only the kind 5/6 MinRange skip, and only for planes. The automatic fire target is always a ship (009F5B70 `IsKindOf(6)`); only a scripted fire target or an order's command target could be a plane.
- **Mission effect:** none expected on USN04 or PRCP01.
- **Adoption:** use `unit_pose` origins. Not worth a pair on its own; fold it into D2's.

**D2. Per-gun range origin in `00729BC0`.**
- Image: `00729C35..00729C7A` measures the gun entity's `+FCh` (the mount, refreshed by `00414DB0`) against the target's `+FCh`. The pass is `!(distance > [ammo+60h])` (`00729C8A` JA).
- Host: `gun_inputs` (game_hosts_gunnery.cpp:5671..5679) measures the unit aim point against the target aim point. Both are raised by their class `Height`, and the gun's own mount offset is not used.
- The host already computes the image's quantity a few lines later, in `bind_aa_acceptance` (:5779..5784): `len`, from `gun_muzzle_point` (the mount when `mount_known`) to the target pose.
- **Effect on the pick:**
  - A gun's maximum-range edge moves by its mount's position along the hull: up to about half the hull length for bow and stern mounts.
  - The vertical term moves by (plane Height - ship Height).
  - Bow mounts accept an incoming plane slightly earlier and stern mounts slightly later. The order does not change; only when each gun's slot first accepts.
- **Mission effect:** a few percent of each AA gun's range at the edge, so on USN04/PRCP01 dive and torpedo strikes the first AA rounds come seconds earlier or later per mount. Total rounds at a strike should move by a few percent. Kills are RNG-coupled (stream 1), so a pair shows moved per-plane fates with no sign attributable per plane.
- **Adoption:** behind `kAaSlotRangeOriginBound`, set `in_range = len <= range` using `bind_aa_acceptance`'s `len`. When `mount_known` is false, `gun_muzzle_point` already falls back to the aim point.

**D3. The fire-target leader fallback in `00863640`.**
- Image: `0086365A..0086368B`. When `director->vtable[2Ch]` (director+238h) is null, `[director+34h]` -> `007788D0` gives the formation leader (`[[unit+284h]+14h]`). If the leader answers `IsKindOf(6)`, the provider returns `[leader+738h]->vtable[2Ch]()`, the leader's fire target.
- Host: `run_gunnery_pass` (game_hosts_gunnery.cpp:6676..6705) takes only the unit's own `ship_ai` row `fire_target`, with no fallback.
- **Effect on the pick:**
  - A formation member with no target of its own inherits the leader's ship target.
  - Its AAMACHINEGUN and LIGHTARTILLERYFLAK guns then try that ship before any plane, if it lies within the category range and the gun's slot accepts it.
  - With the fallback, an escort's AA turns to a surface target in mixed surface and air fights. Without it (the host), the escort keeps shooting planes.
- **Mission effect:** USN04 and PRCP01 strikes are carrier-versus-carrier. The enemy ships are far beyond AA category range, so the leader's target, if any, fails `00863990`'s range test. Expect no change there. Expect movement only in missions where a formation fights ships and aircraft at once. There, image planes survive more than host planes.
- **Adoption:** `aa_pick_director_fire_target_00863640` here. The host needs the formation leader (`ship_ai` holds formation data) and the leader's row `fire_target`. Put it behind `kDirectorLeaderFireTargetBound`, OFF.

**D4. The submarine category gate `008627A0`.**
- Image: `008636A0` installs `00D0D32C` on an `IsKindOf(8)` unit (`0086371E..00863743`). Its slot 4 (`008627A0`) answers as follows:
  - Category 7: `008527E0`, shallow enough.
  - Categories 1 to 6: `008527A0`, above deck-gun depth: y > band0 - 3.0.
  - Any other category: false.
- A refused gate skips both the gather and the assignment (`008651D1` JE `00865773`), so the guns **keep** their previous target.
- Host: `category_gate_slot4` answers true for every unit (game_hosts_gunnery.cpp:5224..5229, which says "a unit that is not class 8").
- **Effect on the pick:** a submerged submarine's AA and deck guns keep picking targets on the host, and its PLANEGUN and depth-charge categories too. In the image the categories freeze while the boat is below the depth.
- **Mission effect:** none on USN04 or PRCP01 unless a submarine is present. In submarine missions, fewer AA picks while submerged.
- **Adoption:** `aa_pick_submarine_category_gate_008627a0`, with `submarine_is_above_deck_gun_depth_008527a0` (gameplay_seg53_cc13.cpp) and `submarine_is_shallow_008527e0` (gun_bot_remainder.cpp). The "freeze, do not clear" semantics are already in `unit_gunnery_pass_tick_00864fe0` (`continue` before the walk). Put it behind `kSubmarineCategoryGateBound`, OFF.

**D5. The neutral-side refusal in `00729BC0`.**
- Image: `00729C00..00729C13`. A target answering `IsKindOf(5)` with `[t+54h] == 2`, the neutral party (docs/RECON_SLOT_LISTS.md section 5), is refused.
- Host: `gun_inputs` has no side test.
- **Reach:** the sweep reads the enemy list, which never holds a party-2 unit, and the fire target comes from the party scan. No reach is expected; this is a recorded difference, not a predicted mover.

**D6. Candidate capacity.**
- Image: the stack array is 50h pairs (`_vector_constructor_iterator_(…,8,50h,…)`), and I read no count guard in the sweep (`008651D7..00865442`) or the arms.
- Host: it clamps at `kUnitGunneryCandidateCapacity` (50h).
- **Reach:** more than 80 accepted candidates in one category of one unit at once. Not expected in USN04 or PRCP01. An overflow in the image would corrupt the stack frame rather than drop entries. Recorded only.

Checked and faithful:
- the range gate and penalty in `00863990`
- the mask in `008633D0`
- the four liveness bytes in `00862820` (docs/CC13_VER_13.md)
- the class arms and rank
- the `00727BD0` table
- the ordered insert and walk
- the kind 5/6 test (`005459E0`) and the `00729B90` MinRange (both records)
- the ammunition-index choice and the bot-slot dispatch in `00729BC0`
- the `008FBE00` armour test (`max(+B0h,+B8h) <= [t+354h]+4Ch` refuses, `008FBE37..008FBE7E`)
- the visibility cache order
- the `+7Dh` torpedo rule

## 3. Doc corrections made

- `docs/FLAK_PROXIMITY_BURST.md` step 3 said no writer exists for `class+D8h`. `0070C0B0` writes `MinRange / V0` (`0070C0BC..0070C0C2`). The host computes it as `flak_fuse_time` (`src/bullet_engagement_range.cpp:119`), and nothing reads it. The stale comment at game_hosts_gunnery.cpp:9462 ("no reader ...") is cc9's file and is left for cc9.
- `docs/AA_TARGETING.md`:72, the flak minimum air range row: kind 5 is ON and kind 6's second ammunition is loaded and read. The row now says faithful.

## 4. Handoff

- No Ghidra annotations were made. The existing names (`BSP_UnitGunneryTarget_DirectorFireTarget`, `BSP_UnitGunneryCategoryGate_Class8`, `BSP_UnitGunneryAi_InstallPolicies`, `BSP_Gun_BotSlotAcceptsTarget`) already fit.
- Open:
  - `IsKindOf(2)` and `director->vtable[48h](2)` in `008636A0`.
  - Whether `[leader+738h]` can be null (the image does not test it).
  - The bot `vtable[38h]` setters `00727F10` calls, which are where any per-bot stickiness would live (docs/GUN_BOT_TICKS.md).
