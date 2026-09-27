# The scene traffic runtime (packet `cc9_scene_traffic_runtime`, part 1: read)

Addresses: 004A5620, 004A50D0, 004A4B70, 00496920, 00487470, 0049A360, 0048A650, 004B48E0,
00496BD0.

Worker cc9-world-init, 2026-09-27, main bcaee249b. Ghidra was read only, and nothing is bound.
The earlier read of 004A50D0 and the scene-contents contract are in `docs/CONSTRUCT_WORLD.md`
section 20. The block reader 009514B0 and its host interface are already reconstructed in
`src/scene_traffic_groups.cpp` (`read_traffic_block`, `SceneTrafficHost`).

## 1. Summary

- **One runtime per item.** Each `traffic` item becomes one 170h runtime when the scene file is
  read: 009514B0 ends in the commit 004A5620 (0095154D).
- **It joins the walked list at construction.** The innermost base 00496920 appends it to the
  group's list, which the TrafficConfig walk 00487270 ticks. So a committed runtime is ticked
  every world tick through its slot 1, 0049A360.
- **The tick is view-gated.** Slot 3, 0048A650, lets the tick run only when 004B48E0 finds the
  runtime's path box in view.
- **The members are ambient.** The tick 00496BD0 moves the runtime's members along a camera path
  (007B04C0) and kills them (00926D90).
- **So traffic is camera-dependent ambient animation.** Binding it changes what is drawn near
  the camera on the 38 scenes that author items, and nothing on USN04 or USN02.

## 2. Construction (V)

**004A4B70, the traffic base** (`__thiscall(record, group)`, `RET 8`, 004A4B70..004A50A9):
- **Base first.** 00496920(group) installs vtable 00CE6600 and stores the parent group at `+8h`.
  When a group is given, it appends `this` to the group's list at `+0Ch` (sentinel `+10h`,
  count `+14h`) through 00485880 and 0048FD20 (0049696F..00496996). That is the list 00487270
  walks, so the runtime is walked from construction. Then `+18h = [00CE4ADC]` and
  `+1Ch = [00D7A24C]`.
- **The record.** 0049FA70(record) copies it into `+20h`, and the vtable becomes 00CE68D4.
- **Clears.** Three containers are constructed at `+6Ch`, `+78h` and `+8Ch`. The template
  vectors at `+E4h..+ECh`, `+F4h..+FCh` and `+104h..+12Ch` are zeroed. `+84h = 0.0` and
  `+88h = 0`.
- **The path parent.** `+D8h` is the path entity's parent (`[007ACA30(path)+3Ch]`) when it
  answers IsKindOf 44h, otherwise null (004A4C4B..004A4C87).
- **The template table.** `+DCh` is set when the first template's table is not
  `LandVehicleClasses` (00CE6710; 004A4C9E..004A4CD6), that is, for SoldierTypes.
- **The template walk** covers the record's template list at `+54h`:
  - SoldierTypes (`+DCh` set): 004B1400 on the name looked up in table 00CE6700, pushed into the
    vector at `+F0h`;
  - vehicle classes: 00964790(class, 1) then 0047B3C0, pushed into `+E0h`. The class's `+A0h`
    and `+A4h` dimensions keep a running maximum at `+134h` and after.
  - `+130h` and `+134h` are seeded from 00D7A24C and 00CE3958 before the walk.
- The rest, 004A4EF5..004A50A9, was not read.

**004A50D0** (section 20 of `docs/CONSTRUCT_WORLD.md`):
- the path box `+150h..+164h`, widened by `+138h`;
- the nearest kind-1Ch anchor at `+168h`;
- the state word `+16Ch`: 0 then 00487470, 1 then 00487470, ending at 3.

**00487470, the side remap** (`__fastcall(runtime)`, 00487470..00487768):
- **Mode.** `+16Ch == 0` selects mode A, anything else mode B (00487489..00487491).
- **Soldiers.** For each soldier template at `+F4h`, it walks the TrafficConfig list at
  `+40h`. Each node carries two names, at `+0Ch` and `+14h`. Mode A matches the template's name
  against `+14h` and replaces it with 004B1400 of `+0Ch`; mode B does the reverse. The match is
  case-insensitive (00BF7FBF).
- **Vehicles.** For each vehicle template at `+E4h`, it walks the list at `+4Ch`. The class
  name is `[00479BC0(slot)+54h]`. An empty compare name matches everything. A match is replaced
  with 0047B3C0(00965480(name)).
- **So** the TrafficGlobals.lua party pairs (`docs/TRAFFIC_CONFIG.md`) swap the item's templates
  between the two sides' equivalents.

## 3. The tick (V, partial)

The vtable is 00CE68E4: slot 0 is 004A5440 (the deleting destructor), slot 1 0049A360, slot 2
004969D0 and slot 3 0048A650.

**0049A360** (slot 1, `(float delta)`, 0049A360..0049A404, from the pseudocode, which drops a
branch):
- it picks a time scale against `[+18h]` and `[00F876A4]` minus the double 00CE6840, stored at
  `+130h`;
- it advances `+84h` by delta times the scale;
- it calls slot 3; its result gates the branch the pseudocode drops;
- it ends in 00496BD0.

**0048A650** (slot 3, 0048A650..0048A6C4):
1. **The view test.** 004B48E0(game, `&this+150h`) reads `[game+1ED4h]+1BCh` and asks
   00B71590 to classify the box. The answer 0AAAh means false. When the answer is false, the
   routine returns false.
2. **With an anchor** `+168h`: the anchor's side `+54h` replaces `+16Ch` when they differ, then
   00487470 re-runs and the routine returns false. When they already match, a state below 2 goes
   on to the arm at 0048A6C1 (not read); otherwise it returns false.
3. **Without an anchor:** `+16Ch` becomes 1 when the local player slot
   `[game+18CCh + [game+18ECh]*4]` has `+28h == 0`, and 0 otherwise.

So the state word is the side the traffic currently belongs to: the anchor's owner, or the local
player's. A captured anchor flips the whole item's templates.

**00496BD0** (00496BD0..00497B7F, 4 KB, callees only):
- `BSP_CameraPath_SampleWorld` 007B04C0 and 007B03C0 over the item's path;
- 4x4 matrix copies (004134F0) and subtree pose invalidation (0042ED50);
- `BSP_MissionEntity_Kill` 00926D90;
- fmod (00BF857A) for looping positions along a cyclic path;
- 0087FA20 and 0087FB90 (unread, the likely member creators);
- 0085DAD0 and 004B0F80.

The members' positions follow the path at the item's `speed`, laid out in `rowCount` x
`columns` with the gaps and deviations of `docs/SCENE_TRAFFIC_BLOCK.md`.

## 4. What this means for binding

- **The two reference missions** author an empty block, so every binding is identity there.
- **The measuring mission** is JM01, "Vanilla - Attack on Pearl Harbor", 5 items. What moves
  there is ambient members appearing, walking or driving along paths, and being killed. That
  happens only while their path box is in view (004B48E0), so it depends on the camera the
  harness leaves in place.
- **Gameplay reach is unknown until 0087FA20 and 0087FB90 are read.** It depends on whether the
  members are combat entities (the record has `HP`), for example targetable land vehicles, or
  scenery.
- **Recommendation.** Read 0087FA20, 0087FB90 and 00496BD0's listing next, and the unread tails
  (004A4EF5.., 0048A6C1..). Bind only if the members reach gameplay, since otherwise it is
  view-gated scenery.

## 5. Parts left

1. The member creators 0087FA20 and 0087FB90, and 00496BD0's listing. It is x87-heavy and too
   long for the pseudocode.
2. 004A4B70's tail, 004A4EF5..004A50A9.
3. 0048A650's state-below-2 arm at 0048A6C1.
4. 004969D0 (slot 2) and the destructor 004A5440.
5. The binding:
   - the scene-contents `SceneTrafficHost` (the contract in `docs/CONSTRUCT_WORLD.md`
     section 20);
   - the runtime in new files registered through `cmake/startup.cmake`, which is leased to
     cc9-plane-release until 16:39Z today;
   - the TrafficConfig walk calling each runtime's slot 1.

## 6. Coverage

| routine | coverage |
| --- | --- |
| 00496920 | complete |
| 00487470 | complete |
| 0048A650 | partial: 0048A6C1.. unread |
| 004A4B70 | partial: 004A4B70..004A4EF5 |
| 004A50D0 | complete (docs/CONSTRUCT_WORLD.md section 20) |
| 0049A360 | partial (pseudocode) |
| 00496BD0 | callees only |
| 004B48E0 | complete; 00B71590 unread |
