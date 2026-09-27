# The Lua half of the wing construction (packet `cc9_wing_construction_lua`, `kWingConstructionLuaBound`)

Addresses: 007F4580 (the squadron's InitAll pass A, where the image constructs the wing), 007F4B49
(the plane's `+9D4h` squadron id), 00928760 / 00926BE0 (each construction's push).

Worker cc9-movie-camera, 2026-09-27, base main 0e3e73344. This is the Lua half specified in
`docs/SENTITY_INIT_ATTACH_ORDER.md` section 15.4. The units half is cc9-ships's
`kWingConstructionInPassABound` (`docs/CONSTRUCT_WORLD.md` section 30, main ccd306b25). That half
holds the wing records back in the creator batch and builds them through `create_units` inside
`on_squadron_pass_a_construct_wing`.

## 1. The binding (`src/game_hosts_lua.cpp`, switch in `include/bsp/game_hosts_lua.hpp`)

The effective switch is `kWingConstructionLuaBound && kWingConstructionInPassABound`, which the
code calls `kWingConstructionLuaActive`. With the units half OFF, this half changes nothing.

While it is active, the Lua host changes in these places:
1. **Pass A marks the hook's pushes.** `entity_attach_lua_self_vcall_9c` records the pending
   list's size before calling `on_squadron_pass_a_construct_wing`. After the call, every
   non-squadron node appended since then gets `wing_member = true`, `squadron_id` = the leader's
   entity id (007F4B49's `+9D4h`), and `class_index` = the squadron's class. These are the three
   fields the append set. The walk re-reads the list's size, so it reaches these nodes in the
   same pass A.
2. **The wing append in pass A is retired.** That is the loop over `[units_before, units_end)`.
3. **The squadron's wing range is retired.** `push_pending_squadron_00926be0` stores
   `units_end = units_before`, an empty range.
4. **The dedup's wing deferral is retired.** It no longer erases other pushers' nodes in the
   range, so `dedup_wing_deferred` stays 0.
5. **The squadron flag annotation stays** (section 15.4, item 4). So do `wing_append_skipped`
   and the load-walk path (item 5).

**New counter:** `init_all_wing_marked`, the nodes pass A marked. It is printed as
`wing_marked=` on the `summary SEntity::InitAll 00925f20` line.

No new units-host accessor is used. The hook, the pending list and the node fields all exist on
main.

## 2. Predictions (written before the pairs; `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`)

**This half alone** (the units half OFF), USN04 4700/4500, the committed pair:
- Identity on every gameplay, per-entity and native row.
- Every counter unchanged. `wing_marked=0` on both sides, because the active switch is false
  whenever `kWingConstructionInPassABound` is.
- `pair_diff` exit 0, or 1 for noise only.

**The joint flip** (both switches ON, cc9-ships's commit; predictions as the lead accepted them):
- `wing_appended` 40 -> 0 and `dedup_wing_deferred` 40 -> 0;
- `wing_marked` 0 -> 40;
- construction pushes stay 81, with 40 of them during pass A;
- `squadron_ids` 40 and `wing_member_tables` 40;
- gameplay moves only by the units half's small air move. It creates the planes in the image's
  order: USN04 hits 799 -> 801, releases reassigned, the same 40 victims.

## 3. Pairs and verdict

(filled after the runs)
