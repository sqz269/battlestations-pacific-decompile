# cc13-dyn1: the in-house Dyn block, unrecorded functions

Packet: the mitengine Dynamics block `00C30930..00C5DF60` (namespaces `Mit`/`Dyn`, 27 RTTI
classes). `reports/library_inventory/SUMMARY.md` section 4: in-house, no importable source.
At the start 116 of the block's 267 functions had no reconstruction record.

Code: `src/cc13_dyn1.cpp`, `include/bsp/cc13_dyn1.hpp`, namespace `bsp::cc13_dyn1`.
Nothing is wired. Every function works on the image's own storage by native byte offset
(the `Raw` helper), like the `native_dyn_*` files; names are descriptive hypotheses.
Calls that leave the file go through `Cc13Dyn1Calls`, one named hook per native call kind,
invoked at the point the binary calls; calls into code that already has a reconstruction
go to it directly (`dyn_body_recompute_bounds_00c55fc0`, `dyn_profile_scope_leave_00c5bcd7`,
`append_native_dyn_manifold_00c35260`).

Status words: every function below is *reconstructed* and *build-tested* (Win32 MSVC);
none is fixture-tested, ABI-compatible or game-validated.

## x87 convention

The image computes in x87 registers at the MSVC default 53-bit precision and rounds to
float only at each `FSTP float`. The C++ computes those intermediates in `double` and
narrows exactly where the image stores a float, so finite inputs give the same stored bits.
Operand direction of every reg-reg op was taken from bytes: `tools/x87_dc_form_scan.py`
lists the DC-form sites (00C56CF0 1, 00C57880 2, 00C57FF0 2, 00C582E0 3, 00C47CA0 1,
00C38470 5, 00C38810 9, 00C37860 2); each was traced with that reading. NaN: every float
branch follows the image's jump (JBE = below-or-equal-or-unordered, JA = above and ordered);
where that differs from the decompiler's condition the code says so (00C412D0's t_exit arms).
No function here draws from the shared RNG 00BD2F10.

## Reachability

Census: `local/dyn1_reach.py` (rel32 E8/E9 into each start plus every absolute dword equal
to it in every section; positive control 00C43AA0 = 17 sites), then deadness propagated from
functions with no reference. Six absolute-dword hits are not references (byte sequences that
straddle instructions, checked in the listing: 004BFC41, 00B27428, 00B60CA2, 00BEAB95 are
`00 81 C4 00` across an ADD ESP,imm; 00B5F819 is PUSH EAX; PUSH 0C3h; 00D64BE4 is unwind data).

* `Dyn::SAPBroadPhaseManager` (vtable 00D7A13C) is built only by `00C37030`, which nothing
  calls or references. `Dyn::SAPRadixBroadPhaseManager` (vtable 00D7A118) is built only by
  the Ghidra-undefined `00C37210` (`MOV [EAX],00D7A118` at 00C37212), also unreferenced. The
  scene uses `SAPBroadPhaseManager2` (00C36F10, docs/DYN_BODY_CREATION.md). Both classes'
  slots and their private callees never run.
* `00C445B0` (the world ray cast) has two callers, both Ghidra-undefined one-instruction
  wrappers after INT3 padding: `00C4DA70` (Ghidra folds it into 00C4D980's body) and
  `00C55DB0` (folded into 00C55D30). Neither has a reference. **The wire-readiness edge
  "00C4D980 blocked on 00C445B0" is a body-range artefact**: 00C4D980 ends at its RET at
  00C4DA6A.
* 64 unrecorded functions are unreachable by this census (list below).

## Reconstructed: reachable

| Address | C++ | ABI | Notes |
|---|---|---|---|
| 00C31DC0 | `body_first_shape_00c31dc0` | fastcall ECX body | `[B+70h]`, the shape chain (next `S+208h`) |
| 00C31EA0 | `body_owner_00c31ea0` | fastcall | `[B+6Ch]` |
| 00C31F20 | `body_angular_velocity_00c31f20` | thiscall, RET 4 | `M+0Ch..14h`, M = `[B+4]` |
| 00C31F40 | `body_linear_velocity_00c31f40` | thiscall, RET 4 | `M+00h..08h` |
| 00C31F60 | `body_set_box_00c31f60` | thiscall, RET 8 | `B+38h..4Ch` = lo, hi |
| 00C31F90 | `body_box_00c31f90` | thiscall, RET 8 | reverse copy (record replaced, see adoption notes) |
| 00C35300 | `body_position_00c35300` | fastcall | `B+2Ch` |
| 00C37F10 | `body_inertia_00c37f10` | thiscall, RET 4 | `1.0 / M+54h[i]`, no zero guard |
| 00C37F60 | `body_add_force_at_point_00c37f60` | thiscall, RET 8 | force, torque `(p - B+2Ch) x f` (each product rounded to float before the subtraction: 24-bit precision assumed, see docs/CC13_FPU_PRECISION.md, cc13-fix13), wake `&= ~12h` |
| 00C56CF0 | `body_set_transform_00c56cf0` | thiscall, RET 4 / tail JMP | pose, proxy box, broad-phase slot 2 |
| 00C32430 | `shape_local_transform_00c32430` | fastcall | `S+34h` |
| 00C32450 | `shape_group_00c32450` | fastcall | `[S+2Ch]` |
| 00C32490 | `shape_body_00c32490` | fastcall | `[S+4]` |
| 00C47F60/F90/FC0/FF0, 00C48020/50 | `shape_collide_mask_*`, `shape_group_*` | thiscall, RET 4 via tail JMP | clear/set/assign `S+30h` or `S+2Ch`, then lock 0109ECD8, 00C43AA0(`[S+4]`), unlock |
| 00C33650 | `transform_expand_3x4_to_4x4_00c33650` | thiscall, RET 4 | `+3Ch` = 1.0f (00D7A24C) |
| 00C336C0 | `transform_from_4x4_00c336c0` | thiscall, RET 4 | drops column 3 |
| 00C32CE0 | `hull_copy_vertices_00c32ce0` | thiscall, RET 4 | 16-byte to 12-byte vertices |
| 00C35810 | `u16_vector_assign_00c35810` | ESI vector, EDI source | 00BF55BE / 00BF6989 |
| 00C37430 | `hull_copy_indices_00c37430` | thiscall, RET 4 | 00C35810 with EDI = `[ECX]` |
| 00C357B0 | `object_set_vec58_and_notify_00c357b0` | thiscall, RET 4 | then `CALL [[ECX]]` |
| 00C357D0 | `object_vec58_00c357d0` | fastcall | `O+58h` |
| 00C57880 | `box_shape_bounds_00c57880` | BoxShape slot 0 | +0.02 (00D7A2F8), box rotation, 00C55FC0 |
| 00C57FF0 | `cylinder_shape_bounds_00c57ff0` | CylinderShape slot 0 | extents (r, h, r) |
| 00C58520 | `sphere_shape_bounds_00c58520` | SphereShape slot 0 | `t -/+ (r + 0.02)` |
| 00C58690 | `terrain_shape_bounds_00c58690` | TerrainShape slot 0 | heightfield min/max |
| 00C57B90 | `box_shape_construct_00c57b90` | thiscall(ECX body; shape, desc), RET 8 | |
| 00C582E0 | `cylinder_shape_construct_00c582e0` | EDI desc; stack (shape, body), RET 8 | builds the frame from the axis |
| 00C585B0 | `sphere_shape_construct_00c585b0` | thiscall(ECX body; shape, desc), RET 8 | |
| 00C47AA0 / 00C38470 | `box_shape_support_*` | slots 2 / 3, RET 0Ch | float / double GJK support |
| 00C48080 / 00C38970 | `sphere_shape_support_*` | slots 2 / 3 | |
| 00C47CA0 / 00C38810 | `cylinder_shape_support_*` | slots 2 / 3 | eps 1e-6f (00D7A288) / 1e-7 (00D7A298) |
| 00C35630 | `convex_shape_copy_indices_00c35630` | thiscall, RET 4 | 00C35810 inlined on `[S+210h]` |
| 00C356D0 | `convex_shape_copy_vertices_00c356d0` | thiscall, RET 4 | |
| 00C30930 | `delete_critical_section_00c30930` | EAX section | funclets only |
| 00C31CC0 / 00C31D70 | `free_owner_buffers_*` | ESI owner | twins; 00BF65AC then 00BF6989 |
| 00C32420 | `shape_restore_base_vtable_00c32420` | fastcall | funclet 00CC84B3 |
| 00C32D50 | `zero_pair_00c32d50` | fastcall, EAX = ECX | caller 006F9CD0 |
| 00C37860 | `barycentric_2d_00c37860` | EAX/ECX/EDX outs, 8 stack floats, RET 20h | no zero-denominator guard |
| 00C38040 | `profile_scope_leave_00c38040` | fastcall | = inline fragment 00C5BCD7 |

The three primitive constructors take the vtable words as parameters defaulting to the
image's addresses (00D7A04C base, then the class vtable), stored at the image's two points;
their SEH frames are not reproduced. The box descriptor is read as transform at desc+14h and
extents at desc+44h, the sphere as radius desc+14h and centre desc+18h, the cylinder as
translation desc+14h, axis desc+20h, half height desc+2Ch, radius desc+30h: these differ from
each other and from `DynShapeDescriptor` in ship_hull_shapes.hpp, which describes the convex
mesh record. Raw offsets are used for that reason.

## Reconstructed: unreachable (small)

`00C412D0` segment/box slab test, `00C445B0` world ray cast, `00C377F0` 2D orientation,
`00C32BB0`/`00C32BD0`/`00C32BE0` (SAPBroadPhaseManager slots 6/5/4), `00C32C20`/`00C32C30`/
`00C40D70` (SAPRadix slots 6/4/5), `00C327F0`, `00C32830`, `00C32A60` (free helpers, no
reference), `00C36C00` (manifold bind, no reference). ABIs in the header.

## Recorded only

* `00C357A0`: `JMP 00408040`; record points at `dyn_convex_shape_free_00408040`.
* 52 unreachable functions recorded as `unreferenced_dead_chain_recorded` with their census
  reason (`local/dyn1_ledger.py`, list `U`): 00C316F0, 00C320D0, 00C32A90, 00C33250, 00C33350,
  00C33420, 00C33490, 00C33510, 00C35390, 00C35A00, 00C36850, 00C368F0, 00C369E0, 00C37030,
  00C37180, 00C37390, 00C37580, 00C37A30, 00C37B10, 00C39040, 00C39100, 00C3F190, 00C40800,
  00C40900, 00C40A30, 00C40B30, 00C40CC0, 00C40D50, 00C40E50, 00C40FC0, 00C410E0, 00C41190,
  00C41230, 00C44360, 00C48100, 00C48E30, 00C4C550, 00C4C5D0, 00C4C700, 00C4CBB0, 00C4CBF0,
  00C4CC90, 00C4CE60, 00C4D050, 00C4D370, 00C4D5A0, 00C54DA0, 00C55A60, 00C55D30, 00C55DF0,
  00C5A850, 00C5A8B0, 00C5DAA0.

## Skipped

* `00C58EB0` `Dyn::TerrainConvexIntersect` slot 0 (vtable 00D7A1C4): reachable, 1729
  instructions of x87. Needs its own packet at max effort.
* `00C36990`: `JMP 00407960` (Mit pool <36,128> free), whose target has a name but no
  reconstruction to point at.

## Blocked callers this unblocks

From `local/output/wire_readiness.json` (main checkout), rows `blocked_on_callees` with a
missing callee in this block:

| Caller | Was missing (this block) | Still missing elsewhere |
|---|---|---|
| 00447510 game_dynamics_list_add | 00C31DC0, 00C31F60, 00C336C0 | none |
| 00929CB0 game_entity_tick_place_pose | 00C33650 | 0042D0D0 |
| 0092B350 tickable_game_entity_tick_advance | 00C31F20, 00C31F40, 00C31DC0, 00C33650, 00C37F60, 00C47F60 | 00935540 |
| 0092BD30 unit_controller_clear_hull_shape_fields (2 wired callers) | 00C31DC0, 00C48020, 00C48050 | none |
| 006FA540 DynWorld_ForEachBodyDebugDraw | 00C31DC0, 00C32430, 00C33650 | none |
| 006FA6D0 DynWorld_ForEachBodyTransform | 00C31DC0, 00C33650, 00C32430 | none |
| 006FB030 ConvexHull_DebugDrawEdges | 00C32CE0, 00C37430 | none |
| 007BAD00 plane_contact_pair_ram | 00C32490 | none |
| 007D5D20 pilot_cmd_axis_name | 00C336C0 | 007EC5A0, 007EABC0, 007C11E0 |
| 00852210 unit_create_motion_controller_b | 00C32450, 00C31DC0 | none |
| 00854650 submarine_periscope_should_auto_raise | 00C32430, 00C357B0, 00C357D0 | 0080E490, 008255B0, 00933BB0 |
| 00934150 unit_parts_detach_part (1 wired caller) | 00C31F40 | none |
| 009373C0 submarine_break_periscope (1 wired caller) | 00C47FC0 | none |
| 009377E0 unit_contact_point_velocity | 00C31EA0, 00C31F20, 00C31F40, 00C35300, 00C32450, 00C32490 | none |
| 00939E50 controller_b_periscope_shape | 00C32450, 00C31DC0 | none |
| 00C4D980 flush_native_dyn_pending_bodies (1 wired caller) | 00C445B0 (spurious edge, see Reachability) | none |

## Adoption notes (host files are not edited here)

* **00C31F90, `GameFixedStepHost::body_box_extent_00c31f90`** (src/game_hosts_fixed_step.cpp,
  `DynamicsBuoyancy` host, LIVE for the buoyancy step 004462D0): the image writes two vectors,
  `lo = B+38h..40h` and `hi = B+44h..4Ch`; the host logs unimplemented and returns one zero
  `DynamicsVec3`. Its ledger record called it reconstructed from source; the record now points
  at `body_box_00c31f90`. Adopting needs the host signature to return both vectors. No RNG.
* **00C5715C** (verified, not mine): `dyn_body_world_bounds_00c5715c` in dyn_collision_pass.cpp
  has the same rule as the image (centre/half with the 0.5 double, centre through the 3x4,
  half through nine `|m|`, min then max). The image keeps each sum in 53-bit registers and
  rounds once per stored float; the host sums in float. Results can differ in the last bit of
  a proxy bound (MINOR). `rotate_box` in cc13_dyn1.cpp is the bit-exact form.
* **Per-tick use.** Plane flight's fixed step `007CE040` calls `00C56CF0` (pose set, proxy
  refresh) every tick; the other callers are 007F2920, 008A0A10, 008A0F10, 0092D620, 0092D650.
  The tickable-entity advance `0092B350` (a vtable slot, no direct caller) calls `00C37F60`,
  `00C31F20`, `00C31F40`, `00C31DC0`, `00C33650` and `00C47F60`. `00C37F60` and `00C56CF0`
  are the x87 sites among them; the float stores listed in their records are where a host
  using float arithmetic can drift. No RNG.
* **cc9 host touch points checked:** ship_buoyancy_elements.cpp and the ocean files
  (ocean_wave_field.cpp, ocean_height.cpp, ocean_spectrum.cpp, world_ocean.cpp,
  fixed_step_job_waves.cpp, native_fixed_step_job_waves.cpp) name no address in this block.
  The buoyancy step reaches it through the game_dynamics_list.hpp hooks (00C31F40, 00C33650)
  and the fixed-step host (00C31F90, above).

## Remaining

00C58EB0 (above), 00C36990, and optional bodies for the unreachable small functions under
~80 instructions (00C32A90, 00C33350, 00C33420, 00C33490, 00C368F0, 00C369E0, 00C37580,
00C37A30, 00C3F190, 00C36850, 00C48100, 00C5DAA0, 00C37180, 00C40CC0, 00C40E50, 00C410E0,
00C41190, 00C41230, 00C4C550, 00C4C5D0, 00C4CBF0, 00C4CE60, 00C55D30, 00C55DF0, 00C5A850,
00C320D0 and its three wrappers).
