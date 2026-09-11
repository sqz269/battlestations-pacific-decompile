# Game dynamics list (`game+30h`): floating debris

Addresses: 00447b80 004462d0 00447510 00447060 00445db0 00447480 00710bb0 00c34f70 00c43ea0 00c33650

Packet `cc_mission_tick`, worktree `agent/cc-mission-tick`. Ghidra was read-only for this packet;
every name below is a hypothesis, not a recovered symbol.

`docs/MISSION_STATE_FRAME.md` reconstructed `00447060`, the release of this list, and left open
"what the `game+30h` subsystem integrates per frame and what its 20h-byte record is". The producer
`00447510` answers the record question field by field, and the two integrators answer the first:
the list holds **detached rigid bodies floating in water** - debris and wreckage that a unit sheds -
each with a scene node, a lifetime, a buoyancy divisor and an anchor point in body space.

## The object

`game+30h` is two `_SECURE_SCL` MSVC vectors laid end to end, `20h` bytes total. Each vector is
`{ _Container_proxy*, first, last, end }`, which is why the begin/end pairs sit at `+4h/+8h` and
`+14h/+18h` and why `00447480` (`push_back`) is called with `ECX = list+10h`.

| offset | field | evidence |
| --- | --- | --- |
| `+00h` | pending vector, checked-iterator proxy | `00446EF0` erase arguments |
| `+04h`/`+08h`/`+0Ch` | pending vector first / last / end, stride `8h` | `00445DB0`, `00447BB0` |
| `+10h` | record vector proxy | `00447B18` (`EBP = this+10h`) |
| `+14h`/`+18h`/`+1Ch` | record vector first / last / end, stride `20h` | `00447D26`, `00447480` |

`00445DB0` `__thiscall int(list)`, body `00445DB0..00445DC2`: the pending vector's element count,
`0` when `first` is null, else `(last - first) >> 3`. It is the loop bound of the first pass and is
**re-read on every iteration** (`00447BA3`, `00447CE9`).

## The 20h record, from its producer

`00447510` `__thiscall void(list, const Spec* spec, bool flag)`, body `00447510..00447B73`,
`RET 8`. Three call sites, all with `ECX = [00E188A8]+30h`: `007CAEC7`, `00934150`'s site, and
`00935C13` (reached from `BSP_UnitInstance_Update 008255B0` through `00935540`, the bow-spray and
hull-breakup path of `docs/UNIT_INSTANCE_UPDATE.md` step 8). It creates the rigid body
(`operator new` at `00447812`, then `00C5D580`, `00C31DC0`, `00C31F60`, `00C31F90`, `00C37E50`,
`00C37F40`, `00C37E70`) and ends at `00447B06..00447B40` by building the record on the stack and
handing it to `00447480` with `ECX = list+10h`. That store sequence is the layout:

| offset | type | written from | read by |
| --- | --- | --- | --- |
| `+00h` | scene node `*` | `spec+00h` (`00447AE6`) | `00447B80` at `00447EFE`, `00448123`, `00448268` |
| `+04h` | float, lifetime seconds | `spec+04h` (`00447ADC`) | `00447B80` at `00447D79`, `00448150`, `00448191`, `00448220` |
| `+08h` | float | `spec+30h` (`00447B14`) | no reader found in `00447B80`, `004462D0` or `00447060` |
| `+0Ch` | rigid body `*` | the body this routine created (`00447B29`) | `00447B80` at `00447D57`; queued for release at `00448279` and `004470A7` |
| `+10h` | float, anchor x | `spec+24h` (`00447B0D`) | `00447B80` at `00447F89`; the node offset |
| `+14h` | float, anchor y | `spec+28h` (`00447AFF`) | `00447B80` at `00447DD5`, `00447F7A` |
| `+18h` | float, anchor z | `spec+2Ch` (`00447B30`) | `00447B80` at `00447DF5`, `00447F6C` |
| `+1Ch` | float, buoyancy divisor | `spec+54h` (`00447B37`) | `004462D0` divides by it; `00447B80` raises it at `004481E5` |

Rule 4 is satisfied for every field except `+08h`, which the producer writes and no reader in the
three routines read here touches.

## The fixed-step pass, `004462D0`: buoyancy

`__thiscall void(list, float step)`, body `004462D0..00446525`. Called only from `00875E24`, inside
the `00875BB0` fixed-step loop, with `ECX = [00E188A8]+30h` and the step `0.05f`
(`docs/IN_MISSION_SUBSYSTEM_TICK.md`). It runs **N times per display frame**, where N is the number
of fixed steps that frame, which can be zero. It walks the record vector by index without ever
removing anything, and per record:

1. `00C32000(body)` - the body's world transform; `00C31F90(&min, &max)` - its box.
2. `0078CF20(transform+24h, transform+2Ch)` - the water height at the body's world x and z.
3. Half-height: `h = 0.5 * (|m[1]| * (max.x-min.x) + |m[4]| * (max.y-min.y) + |m[7]| * (max.z-min.z))`,
   the three matrix components taken from `transform+04h`, `+10h`, `+1Ch` with the absolute value
   written as `-0.0f - v` (`00D7A208` holds `-0.0f`) and `0.5` from the double at `00D7A280`.
4. `submersion = clamp((waterHeight - (centerY - h)) / ((centerY + h) - (centerY - h)), 0, 1)`,
   `centerY` = `transform+28h`, the clamp ceiling `1.0f` from `00D7A24C`.
5. `force = 00C31FC0(body) * 10.0 * submersion / record[+1Ch]`, the `10.0` from the double at
   `00CE3DC0`; applied with `00C32050`. Damping follows:
   `00C37DE0(00E0819C * submersion)` and `00C37E00(00E0819C * submersion)`, with `00E0819C`
   currently `1.0f`.

`record+1Ch` is therefore a **divisor of buoyancy**: a larger value makes the piece float less.
That is what the frame pass manipulates when a record starts to expire.

Coverage: **partial**. The loop shape, the record stride, the submersion formula and the five
physics calls are established; the physics-block callees (`00C32000`, `00C31F90`, `00C31FC0`,
`00C32050`, `00C37DE0`, `00C37E00`, `0078CF20`) were not read and are `contract: unread`.

## The display-frame pass, `00447B80`

`__thiscall void(list, float scaledDelta)`, body `00447B80..0044831C`, `RET 4` at `0044831A`, SEH
frame `00C5FEFB`. Sole caller `004C40A0` at `004C40DD`. Two loops.

### Loop 1, `00447BA0..00447CF3`: the pending group switches

The pending vector's elements are `{ void* owner, float seconds }`, stride `8h`. Per pass:

1. `element.seconds -= scaledDelta` (`00447BCB..00447BDA`).
2. If the result is `>= 0` (`00447BFA` `COMISS` against zero, `JBE` takes the keep arm), advance
   the index and continue.
3. Otherwise fire and remove. The fire is `00710BB0(element.owner, groupIndex)` at `00447C6D`,
   with `groupIndex` computed at `00447C1A..00447C4E` from the owner's own `10h`-stride vector at
   `owner+16Ch`/`owner+170h`: `count ? count - 1 : 0`, **the last group**.
4. The removal at `00447C72..00447CE2` is a swap-erase: the last element is copied over the current
   slot and `last` moves back `8h`. The index is **not** advanced, so the swapped-in element is
   processed on the next pass, and the loop bound is re-read through `00445DB0` at `00447CE9`.

`00710BB0` `__thiscall void(owner, uint group)`, body `00710BB0..00710E00`, read at its own body:
it walks the `10h`-stride group vector at `owner+16Ch`; for every group **other** than `group`,
each node in the group's inner vector whose visibility factor `node+ACh` equals `1.0f` is hidden
(`BSP_SceneNode_SetVisibilityFactor(node, 0.0f, 0)`) and displaced by `+100000.0` in y (the double
at `00CF81F0`) through `node->vtable[2Ch]`; for the selected `group`, each node whose factor equals
`00D7A218` (`0.0f`) is shown (`1.0f`) and displaced back by `-100000.0`. It is a model-group
switch: exactly one group of a model is visible, the rest are parked far away. A pending entry is
therefore a **deferred switch to an object's last model group**, a destroyed or wrecked state.

### Loop 2, `00447CF6..004482FD`: one record per pass

Captured once before the loop: `owner = [00E188A8]+18h` (`00447CF6`, the same sub-object
`00447060` captures at `0044706A`) and the interpolation numerator `00F876AC` (`00447CFE`).

Per record, in listing order:

1. `body = record[+0Ch]`, saved (`00447D57`).
2. `record[+04h] -= scaledDelta` (`00447D79`).
3. `alpha = 00F876AC / 0.05f` (`00447D8F`, dividing by the float at `00D0DE84`), then
   `00C43EA0(body, out34, alpha)` at `00447DA7`: twelve floats lerped between the body's previous
   state at `[body+4]+84h` and its current state at `body+8h`, `out[i] = prev[i] + alpha*(cur[i]-prev[i])`,
   `RET 8`. This is the render interpolation for the fixed step that `00875BB0` just advanced.
4. `00C33650(out34, out44)` at `00447DB5`: expand the `3x4` into a `4x4`, zero in the fourth
   column and `1.0f` (`00D7A24C`) at `+3Ch`, `RET 4`.
5. The body's interpolated world position is the matrix's fourth row, saved before it is modified
   (`00447DE1..00447E24`). Then the fourth row is corrected by the record's anchor:
   `row3 -= M3x3^T * (record[+10h], record[+14h], record[+18h])` (`00447E0B..00447EB1`).
6. `node = record[+00h]`. When `node+5Ch` bit 1 is clear, `BSP_Transform_RefreshWorldMatrix(node)`
   (`00447EDE` and again at `00447F0B`; the listing calls it twice through two separate loads of
   the same pointer).
7. **The water-entry test**, `00447F99..00447FEC`: `y = node.M[0][1]*record[+10h] +
   node.M[1][1]*record[+14h] + node.M[2][1]*record[+18h] + node.M[3][1]`, the four matrix cells at
   `node+F4h`, `+104h`, `+114h`, `+124h`. It fires when `y > 0` **and** the body's own interpolated
   world y is `< 0`: the anchor is above the water plane while the body's origin is under it.
8. On a hit: `00C31F40(body, &velocity)` at `00447FFE` (the linear velocity,
   `docs/UNIT_CONTROLLER_UPDATE.md`), then the settings singleton `00424C40()` at `00448003`.
   If `-settings[+24h] > velocity.y` the effect id is `(int)settings[+2Ch]`; else if
   `-settings[+20h] > velocity.y` it is `(int)settings[+28h]`; else nothing spawns. With an id
   `> -1`, `BSP_EffectHandle_Acquire(&handle, 1)` at `00448046` (the id travels in EDX),
   `[00E188A8]+19ECh` becomes EDX for `008685E0` at `0044809A`, and the spawned object's `+9h`
   byte is set to `1`. The two ref-counted handles are released through `00CE2220` on both paths.
9. `node->vtable[34h](out44)` at `00448130`: the node takes the corrected interpolated matrix.
10. **Keep or release**, `0044814A..004482FD`:
    - Release when `record[+04h] <= 0` (`00448150`) **or** the body's interpolated world y is below
      the double `-50.0` at `00CE4938` (`0044815F`). The release is
      `BSP_Node_UnlinkAndRelease(node)` at `0044826B`, then `00C34F70(owner, body)` at `00448279` -
      the same deferred-destruction queue push `00447060` uses - then a swap-erase of the `20h`
      record (`REP MOVSD` of eight dwords at `004482DA`, `last -= 20h`). The index is **not**
      advanced.
    - Otherwise, when `record[+04h] < 5.0f` (the float at `00CE3850`), the piece is in its last
      five seconds: if `record[+1Ch] < 1.0f` it is raised to `1.5f` (`00CE380C`) at `004481E5`, so
      the buoyancy `004462D0` computes drops and the piece starts to sink; and
      `BSP_SceneNode_SetVisibilityFactor(node, record[+04h] / 5.0, 0)` at `0044823D` fades it out,
      the divisor being the double `5.0` at `00D7A370`. Then the index advances.

Coverage: **complete** for both loops. Every branch of `00447B80..0044831C` is covered by the
reconstruction except the ref-count bookkeeping of the effect handles
(`00448046..00448103`), which is modelled as two host calls; `008685E0`, `00870CD0` and the
`00CE221C`/`00CE2220` pair were not re-read here (`00870CD0` is already named in the ledger).

## Insertion and removal, in one place

| operation | routine | what it does |
| --- | --- | --- |
| insert a record | `00447510` -> `00447480` (`ECX = list+10h`) | `push_back` of the `20h` record; `00447480` grows through `004473F0` when full, otherwise copies in place through `00446AE0` and moves `last` by `20h` |
| insert a pending switch | not found | no writer of the `+4h/+8h` vector was located in this packet; see the follow-up |
| remove one record | `00447B80` at `004482CC..004482FD` | swap-erase, `20h`, index not advanced |
| remove one pending | `00447B80` at `00447C72..00447CE2` | swap-erase, `8h`, index not advanced |
| remove everything | `00447060` | `erase(begin, end)` on both vectors; already reconstructed in `src/mission_state_frame.cpp` |

## The deferred-release queue, `00C34F70`

`__thiscall void(owner, void* body)`, body `00C34F70..00C35001`, `RET 4`. `owner` is
`[00E188A8]+18h`. A hand-rolled growable dword array: data at `owner+438h`, size at `+43Ch`,
capacity at `+440h`; when size equals capacity it allocates `2*capacity + 2` dwords through
`operator new 00BF55BE`, copies, frees the old block with `_free 00BF6989`, then appends and bumps
the size. Nine callers, of which two are this list's (`00447060`, `00447B80`). It is not a vtable;
see Corrections.

## The host

`include/bsp/game_dynamics_list.hpp` / `src/game_dynamics_list.cpp`. The record and the pending
entry are typed structs; `tick_game_dynamics_frame_00447b80(list, scaled_delta, alpha, host)` is
loop 1 plus loop 2 with both swap-erases and the non-advancing index; `apply_dynamics_buoyancy_004462d0`
is the fixed-step submersion rule over the same records. The pure rules -
`dynamics_interpolation_alpha`, `dynamics_submersion_fraction`, `dynamics_fade_factor`,
`dynamics_splash_effect_id`, `dynamics_should_release`, `dynamics_expiring_buoyancy_divisor` -
carry no host at all.

Host methods in call order, with the native call site and the owner area:

| # | host method | site | native | owner |
| --- | --- | --- | --- | --- |
| 1 | `select_visible_model_group_00710bb0` | `00447C6D` | `00710BB0` | render |
| 2 | `interpolate_body_transform_00c43ea0` | `00447DA7` | `00C43EA0` | unit |
| 3 | `expand_transform_to_4x4_00c33650` | `00447DB5` | `00C33650` | pure (host anyway: it is a physics-block routine) |
| 4 | `refresh_node_world_matrix_00b6db70` | `00447EDE`, `00447F0B` | `00B6DB70` | render |
| 5 | `node_world_matrix_column_y` | `00447F99` | none (inline) | render |
| 6 | `body_linear_velocity_00c31f40` | `00447FFE` | `00C31F40` | unit |
| 7 | `splash_settings_00424c40` | `00448003` | `00424C40` | session |
| 8 | `acquire_effect_handle_00870cd0` | `00448046` | `00870CD0` | sound |
| 9 | `spawn_effect_008685e0` | `0044809A` | `008685E0` | sound |
| 10 | `set_node_world_matrix_vtable34` | `00448130` | indirect, `node->vtable[34h]` | render |
| 11 | `unlink_and_release_node_00b6dfa0` | `0044826B` | `00B6DFA0` | render |
| 12 | `queue_body_release_00c34f70` | `00448279` | `00C34F70` | unit |
| 13 | `set_node_visibility_00b6da70` | `0044823D` | `00B6DA70` | render |
| 14 | `body_world_transform_00c32000` | inside `004462D0` | `00C32000` | unit |
| 15 | `body_box_00c31f90` | inside `004462D0` | `00C31F90` | unit |
| 16 | `water_height_0078cf20` | inside `004462D0` | `0078CF20` | render |
| 17 | `apply_buoyancy_force_00c32050` | inside `004462D0` | `00C32050` | unit |
| 18 | `set_body_damping_00c37de0` | inside `004462D0` | `00C37DE0`, `00C37E00` | unit |

Rows 14 to 18 are listed without a call-site address on purpose: they are inside `004462D0`, whose
per-call listing addresses were not transcribed, and `reports/mission_tick.json` carries only the
rows whose site was read.

## Corrections

- **`00C34F70` is a function, not a vtable.** See `docs/IN_MISSION_SUBSYSTEM_TICK.md`; repeated
  here because the follow-up row that named it lives in `docs/MISSION_STATE_FRAME.md`.
  Was: "the vtable `00c34f70`". Is: a dword-array `push_back` on `[00E188A8]+18h`.
- **`record+0Ch` is a rigid body, not an opaque handle.**
  `include/bsp/mission_state_frame.hpp` types it `std::uint32_t` and comments "only record+0Ch is
  ever read", which is true of `00447060` alone. Was: `record_handles`, "the element type was not
  recovered". Is: the physics body created by `00447510` at `00447812`, whose transform, velocity,
  box and damping the two integrators use. The release is a deferred destruction queue push.
  The `mission_state_frame.hpp` declarations are left untouched; the new header builds on them.
- **`kDynamicsOwnerOffset` names the deferred-release queue.**
  `[00E188A8]+18h` is captured for the same reason in both routines: `00447060` at `0044706A` and
  `00447B80` at `00447CF6`. Was: "the owning subsystem is not identified". Is: whatever owns
  `+438h..+440h` is a release queue, drained elsewhere.
- **The pending vector holds `{owner, seconds}`, not opaque eight-byte elements.**
  `include/bsp/mission_state_frame.hpp` says "the 8h range is copied wholesale by the erase, so its
  contents are opaque here". Was: opaque. Is: `+0h` an object with a model-group vector at `+16Ch`,
  `+4h` a countdown in seconds. Evidence: `00447BCB` and `00447C1A..00447C6D`.

## no_ghidra_function

none. Bodies: `00447B80..0044831C`, `004462D0..00446525`, `00447510..00447B73`,
`00447060..0044713A`, `00445DB0..00445DC2`, `00447480..00447502`, `00710BB0..00710E00`,
`00C34F70..00C35001`, `00C43EA0..00C43FFB`, `00C33650..00C336BB`.

## Follow-up packets

| packet | addresses | what is open |
| --- | --- | --- |
| `dynamics_pending_producer` | 00447b80 00710bb0 | Nothing in this packet writes the `+4h/+8h` pending vector. Find the routine that schedules a deferred model-group switch; its argument is the countdown, and `00710BB0` always switches to the *last* group, which only makes sense for a two- or three-state model. |
| `debris_spawn_spec` | 00447510 007caad0 00934150 00935540 | The `Spec` `00447510` reads (`+0h`, `+4h`, `+8h`, `+24h`, `+28h`, `+2Ch`, `+30h`, `+54h`) and the second argument (`[ESP+114h]`, a bool tested at `00447974`). `record+8h` is written from `spec+30h` and has no reader in the three routines read here. |
| `debris_splash_settings` | 00424c40 00870cd0 008685e0 | The four settings fields `+20h`, `+24h`, `+28h`, `+2Ch` of `00424C40()` - two velocity thresholds and two effect ids - and the `008685E0` spawn ABI, including the `+9h` byte the caller sets on the result. |
| `physics_block_body_api` | 00c31f40 00c31f90 00c31fc0 00c32000 00c32050 00c37de0 00c37e00 00c43ea0 00c33650 0078cf20 | The rigid-body accessors this list uses. `00C43EA0` and `00C33650` are read here; the rest are `contract: unread`, and `0078CF20` (water height at x,z) is the one a headless mission frame cannot fake. |
