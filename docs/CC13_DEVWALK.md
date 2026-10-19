# CC13 DEVWALK: 007C0D90, the release-order device walk, per rack

Packet cc13-devwalk. Code: `include/bsp/cc13_devwalk.hpp` (the walk) and
`src/game_hosts_units.cpp` (`run_device_walk_per_rack_007c0d90`, `rack_delay_draw_007c0e88`).
Status: reconstructed and build-tested. kPlaneDeviceWalkPerRackBound is ON (main cff560b45, OCEAN_WAVE_FIELD 97: physics23's pairs judged it faithful); kPlaneDeviceWalkSharedDrawBound is OFF until physics23 pairs it. Not game-validated beyond those pairs.
Names are hypotheses, not recovered symbols.

## The image (disk bytes 007C0D90-007C0F0D)

`void __thiscall(ECX = unit)`, plain RET. Its one caller is 007CE040.

The walk runs over the children on unit+48h, following +44h. For each child:

1. Skip it unless it is IsKindOf(25h) (vt+5Ch, 007C0DBF), holds 2Ah (vt+210h(2Ah, 0),
   007C0DD7), and is not busy (vt+1FCh, 007C0DEB).
2. Take desc = vt+220h(0) (007C0E01). Fire the rack through vt+1F0h(delay so far)
   (007C0E17), which is 006E3550: toRepeatTime += delay, dropBombs = 1. Set found = 1.
3. Leave the walk at once (to 007C0EE0) if the plane is not IsKindOf(10h) (007C0E28), or
   if desc answers 2Ch, 2Bh or 33h (007C0E3B, 007C0E4E, 007C0E61).
4. Otherwise add a delay: delay += 00BD2F10(ECX = 1, [00CE3860] 0.9f, [00CE6448] 1.1f)
   times class+1F4h BombDelay (007C0E67-007C0EA0). This draw comes before the 31h test.
5. If desc answers 31h, call 007BCCF0 and then 006E3F90 to set the target point
   (007C0EC9). Then go to the next child.

After the walk:

- 007C0ED9: if nothing was fired, return.
- 007C0EE2: set unit+C25h = 1.
- 007C0EEB: if the LAST fired desc answers 33h (a rocket), return.
- Otherwise call 007EEF30(ECX = [unit+9D4h], unit), which issues the release orders to the
  flight.

## What the host did before (all existing switches ON)

There are two paths:

- **Level-bomber path.** `run_level_bomber_issue_007c0e67` walks the per-rack state, and
  kLevelBomberIssueTailBound adds the C25h + 007EEF30 tail.
- **Every other plane.** The binding's `unit_carries_droppable_device_007c0d90` decides
  from the plane-wide ordnance mask. kPlaneDeviceWalkBound then fires the first rack that
  still has ammo.

Four gaps remained:

| | Gap | Switch |
|---|---|---|
| a | A rocket (33h) rack never takes the 007C0EEB return. The mask path issues release orders anyway, and fires the first rack with ammo even when that rack is the rocket. | kPlaneDeviceWalkPerRackBound |
| b | The plane-wide mask decides, not the first eligible rack, so mixed racks are decided wrongly. `lb_level_bomber_racks` treats ANY 2B/2C/33 rack as deciding (labelled). | kPlaneDeviceWalkPerRackBound |
| c | The level bomber's next-rack draw comes from the host's private release sequence (`lb_draw("rk")`), not from the shared stream 1. | kPlaneDeviceWalkSharedDrawBound |
| d | The ctl+3ECh store at 007EE802. | none (see below) |

## The two switches (PerRack ON since cff560b45, OCEAN_WAVE_FIELD 97; SharedDraw OFF)

**`bsp::cc13_devwalk::kPlaneDeviceWalkPerRackBound`** is the behaviour switch. It
replaces the whole body of `run_release_order_issue_007c0d90`:

- The census's BombPlatform racks are walked in platform order, as described above.
- "Busy" means the rack's dropBombs on whichever rack tick will run it:
  - `lb_rack_dropping[i]` when `lb_level_bomber_racks()` holds;
  - otherwise `rack_dropping` together with `rack_active`.
- Fires go to that same tick:
  - the per-rack level-bomber tick takes the delay;
  - the single-rack tick takes one active rack. A second fire there is recorded only.
- A rocket rack's fire is recorded only (`Plane::device_walk_rocket_rack_fire`), because
  its rounds belong to the opportunistic-rocket path. C25h is still raised, and when the
  rocket is the last fire no release orders are issued.
- The paratrooper target point (006E3F90) is recorded only.
- With this switch ON, the values of the switches it bypasses are ignored inside
  `run_release_order_issue_007c0d90`: the level branch of kPlaneLevelBombTaskBound,
  kLevelBomberIssueTailBound, kPlaneDeviceWalkBound, kReleaseIssueStageBound /
  kReleaseIssueStageValsBound and kRackBulletKindBound.
- Diagnostics (cc13-devwalk2): a level-bomber issue keeps the OFF path's log line
  (`level bomber issue: ... racks=%d last_delay=...`), its `Plane::release_issue_level_bomber_007c0e67`
  record, and the `lb_rack_issues` / `torpedo_orders_issue_ticks` counts, including on a tick
  that fires nothing. ON and OFF pairs therefore compare line for line.

**`bsp::cc13_devwalk::kPlaneDeviceWalkSharedDrawBound`** is the draw sub-switch.
It covers both stream-1 draws on the issue path, in call order, taken from
`GameGunneryHost::death_mode_draw_00bd2f10(1, unit, 0.9, 1.1)`, the shared stream 1:

1. `rack_delay_draw_007c0e88`, the next-rack delay draws inside 007C0D90 (007C0E88). It is
   used by both the existing level-bomber path and the per-rack walk, so this switch works on
   its own.
2. `issue_interval_draw_007ceab3`, the next-issue interval C28h that the issue stage draws
   right after 007C0D90 returns (007CEA9A-007CEABE, under kReleaseIssueIntervalDrawBound).
   It was added in cc13-devwalk2. Without it, ON would have moved one draw of the pair onto
   stream 1 and left the other private.

These are the only 00BD2F10 calls on the path. The census covered the issue stage
007CE900-007CEB40 (Capstone over the disk bytes: 007C0D90 at 007CEA8D, 00BD2F10 at 007CEAB3,
nothing else) and the direct calls from 007C0D90, 007EEF30, 006E3550, 007BCCF0 and 006E3F90
to depth 6, which reach no other 00BD2F10. The descriptor vtable calls (vt+210h, vt+220h,
vt+1FCh) are not followed.

- Under `BSP_GUNNERY_RNG_STREAMS=1` the draw falls into that API's death_mode measurement
  key. This is labelled.
- The delay arithmetic is float: the main thread runs x87 at 24-bit precision control.

With both switches OFF, behaviour is byte-identical. `rack_delay_draw_007c0e88` and
`issue_interval_draw_007ceab3` return the same private draws they replace (`#lbrk`, `#issue`),
and the per-rack member is never called.

## The ON-lookup rule

Every container the ON path reads, who fills it, and whether anything empties it first:

- **`rack_bullet_class_per_rack`.** Pushed once by `plane_rack_census` (guarded by
  `rack_census_done`). It is never cleared.
- **`rack_ammo_per_rack`.** Seeded by `lb_rack_state_seed`, which the walk calls first, or
  by the binding's seed. Both clear it and refill it in the same block, and only while
  `rack_ammo < 0`. The unequipped census arm assigns zeros of the same size.
- **`lb_rack_dropping` / `lb_rack_to_repeat`.** Resized by `lb_rack_state_seed` to
  `rack_ammo_per_rack`'s size just before the read.
- **Sizes.** The walk takes `min()` of the three sizes. All three come from the same
  BombPlatform pushes, so they are equal once seeded.

## 007EE7F0 and ctl+3ECh (cc9's grant)

The receiver is the squadron. 007EEF30 is called with ECX = [unit+9D4h]; its members are at
+3D0h and its count at +3CCh. 007EE802 stores a BYTE: `[squadron+3ECh] = 0`.

- CC13_FIX16 lists nine byte writers of squadron+3ECh and no reader in the squadron code.
  It is a dirty or refresh byte.
- It is NOT the bot task's float gunfire rest-end timer at task+3ECh, which 0099C7D3 seeds.
  That object is the pilot task, not the squadron, and the field is a 4-byte float.
- Effect: none known. The store is recorded here and NOT wired; no switch was added. The
  grant was not used and 007EE7F0 was not leased.

## Predicted effect when ON

- **kPlaneDeviceWalkPerRackBound**
  - The per-rack decision itself does not change planes whose racks are all one class
    (torpedo, dive or level bombers on the measured rows), apart from the target-point and
    rocket records. They can still move: an issue tick that meets a busy rack (vt+1FCh,
    dropBombs still set) now skips it and fires the next free rack, as 007C0DEB does. The
    mask path found nothing while a single rack was dropping.
  - It changes planes that carry a rocket rack. A fighter-bomber whose first eligible rack
    is a rocket raises C25h and issues no release orders; before, it issued orders and
    marked the rocket rack as dropping.
  - It also changes kind-10h planes with mixed racks: a plain rack listed before a
    2B/2C rack now fires with the level delay first.
- **kPlaneDeviceWalkSharedDrawBound**
  - Moves the level bombers' next-rack draws and every issuing plane's C28h interval draw
    onto stream 1. From the first issue onwards (any torpedo, dive or level bomber reaching
    007C0D90) this shifts every later stream-1 consumer: death modes, ship-AI torpedo draws
    and gunnery.
  - WINS rows: u1a-u1e, j8h, i08h and w_l10. With the C28h draw included, w_e8 is also
    expected to move if any plane in it reaches the issue stage.
