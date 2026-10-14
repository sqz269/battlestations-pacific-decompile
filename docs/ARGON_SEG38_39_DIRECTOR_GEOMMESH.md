# Argon Reconstruction: Segments 38 & 39 (`0x0071bc30` through `0x00728960`)

## Scope and Status

- **Packet**: `argon-v2-seg38-39`
- **Address span**: `0x0071bc30` through `0x00728960` (77 reconstructed functions across Segments 38 and 39)
- **Header**: `include/bsp/director_geommesh_seg38_39_argon.hpp`
- **Source**: `src/director_geommesh_seg38_39_argon.cpp`
- **Report**: `reports/argon_seg38_39_director_geommesh.json`
- **Namespace**: `bsp::director_geommesh_seg38_39_argon`
- **Validation status**: `reconstructed_build_tested` (`abi_compatible: false`, `game_validated: false`, `coverage: "complete"`)
- **Architecture**: Explicit `Raw` byte-offset helper and `Seg38_39Host` pure-virtual host boundary. No `fn_at` casts or raw function-pointer casts are used. Descriptive C++ identifiers are provisional hypotheses and not recovered symbols.

---

## x87 `DC`-Form Audit

One `DC`-prefix x87 instruction occurs inside the 77 functions:

- **`0x0072309b` inside `0x00723030` (`BSP_GeomMeshElement_RootBoxCentre`)**:
  - Bytes: `dc c9` (`fmul st(1), st(0)`)
  - Assembly sequence (`0x00723091` through `0x007230ae`):
    - `0x00723091`: `FLD float ptr [ESP + 0x18]` pushes `sum_x = max_box[0] + min_box[0]` into `st(0)`.
    - `0x00723095`: `FLD double ptr [0x00d7a280]` pushes `0.5` into `st(0)`, shifting `sum_x` to `st(1)`.
    - `0x0072309b`: `FMUL ST1` (`dc c9`, `fmul st(1), st(0)`) multiplies `st(1)` by `0.5` while keeping `0.5` live in `st(0)`.
    - `0x0072309d`: `FXCH` swaps `sum_x * 0.5` into `st(0)` and `0.5` into `st(1)`.
    - `0x0072309f`: `FSTP float ptr [EAX]` stores `out_centre[0]` and leaves `0.5` in `st(0)`.
    - `0x007230a1`: `FLD float ptr [ESP + 0x1c]` pushes `sum_y`, `0x007230a5` (`d8 c9`, `fmul st(0), st(1)`) multiplies by `0.5`, and `0x007230a7` pops `out_centre[1]`.
    - `0x007230aa`: `FMUL float ptr [ESP + 0x20]` multiplies `0.5` in `st(0)` by `sum_z`, and `0x007230ae` pops `out_centre[2]`.
  - Both factors are commutative (`sum_x * 0.5`), and the reconstruction preserves the `double` load of `[0x00d7a280]` before narrowing each axis product to `float`.

---

## Reconstructed Functions (77)

### 1. WeaponDirector, DirectorPathObject, and CommandControllerBase (`0x0071bc30` through `0x00721030`)

| Address | Ghidra Name | Reconstructed Symbol | Original ABI | Summary |
|---|---|---|---|---|
| `0x0071bc30` | `TRIV_body_0071bc30` | `triv_body_0071bc30` | `__fastcall(ECX=self)`, `RET` | Stores vtable `0x00cfd93c` at `+0x00`. |
| `0x0071bdb0` | `TRIV_body_0071bdb0` | `triv_body_0071bdb0` | `__fastcall(ECX=self) -> EAX`, `RET` | Returns `self + 0x10`. |
| `0x0071bdc0` | `TRIV_body_0071bdc0` | `triv_body_0071bdc0` | `__thiscall(ECX=self, stack arg1)`, `RET 0x4` | No-op virtual slot returning with `RET 0x4`. |
| `0x0071bdd0` | `TRIV_body_0071bdd0` | `triv_body_0071bdd0` | `__thiscall(ECX=self, stack arg1)`, `RET 0x4` | No-op virtual slot returning with `RET 0x4`. |
| `0x0071bfc0` | `BSP_WeaponDirector_SlotPathObject` | `bsp_weapon_director_slot_path_object_0071bfc0` | `__thiscall(ECX=self, stack index) -> EAX`, `RET 0x4` | Reads path-object pointer `[self + 0x1a4 + index * 4]`. |
| `0x0071c0b0` | `CG_adjustor_thunk_0071c0b0` | `cg_adjustor_thunk_0071c0b0` | `__thiscall(ECX=self, stack index)`, tail jump | Loads `ECX = [self + 0x1a4] + 0x10` and tail-jumps to `0x007ae060` (`0x0071c0b9`). |
| `0x0071c150` | `BSP_WeaponDirector_ClassTest5C` | `bsp_weapon_director_class_test_5c_0071c150` | `__thiscall(ECX=self, stack other)`, `RET 0x4` | Calls `0x00778890` (`0x0071c156`) on `[self + 0x34]`; if true and `other != nullptr`, resolves `other->vtable[0x114]()` (`0x0071c171`) and `[self + 0x34]->vtable[0x114]()` (`0x0071c185`) and invokes `vtable[0x4c]` (`0x0071c18c`). |
| `0x0071c1a0` | `BSP_WeaponDirectorObserver_ForwardSlot8` | `bsp_weapon_director_observer_forward_slot8_0071c1a0` | `__fastcall(ECX=self)`, tail jump | Indirect tail jump through `vtable[0x08]` (`0x0071c1a5`). |
| `0x0071c280` | `BSP_WeaponDirector_VSlot3C_Noop` | `bsp_weapon_director_vslot_3c_noop_0071c280` | `__thiscall(ECX=self, stack arg1)`, `RET 0x4` | No-op slot `+0x3c` of `0x00cfda40`. |
| `0x0071c370` | `TMPL_OwnedPtr_DeleteReset_0071c370` | `tmpl_owned_ptr_delete_reset_0071c370` | `__fastcall(ECX=slot)`, `RET` | If `*slot != nullptr`, invokes `vtable[0](1)` (`0x0071c37f`) and zeroes `*slot`. |
| `0x0071c390` | `TMPL_OwnedPtr_DeleteReset_0071c390` | `tmpl_owned_ptr_delete_reset_0071c390` | `__fastcall(ECX=slot)`, `RET` | If `*slot != nullptr`, invokes `vtable[0](1)` (`0x0071c39f`) and zeroes `*slot`. |
| `0x0071c470` | `BSP_Director_AttachTickSubnode` | `bsp_director_attach_tick_subnode_0071c470` | `__thiscall(ECX=self, stack owner)`, `RET 0x4` | Pushes `self`, loads `ECX = owner` (`[ESP+0x8]`), and calls `0x00876020` (`0x0071c475`). |
| `0x0071c490` | `TRIV_body_0071c490` | `triv_body_0071c490` | `__thiscall(ECX=self, stack arg1)`, `RET 0x4` | No-op returning with `RET 0x4`. |
| `0x0071c5d0` | `TRIV_body_0071c5d0` | `triv_body_0071c5d0` | `__fastcall(ECX=self)`, `RET` | Stores vtable `0x00ce4974` at `+0x00`. |
| `0x0071d540` | `TRIV_body_0071d540` | `triv_body_0071d540` | `__thiscall(ECX=self, stack arg1, stack arg2) -> AL`, `RET 0x8` | Returns `0` (`XOR AL, AL`). |
| `0x0071d5b0` | `TRIV_body_0071d5b0` | `triv_body_0071d5b0` | `__fastcall(ECX=self) -> EAX`, `RET` | Returns `1` (`MOV EAX, 1`). |
| `0x0071d5c0` | `TRIV_body_0071d5c0` | `triv_body_0071d5c0` | `__thiscall(ECX=self, stack arg1)`, `RET 0x4` | No-op returning with `RET 0x4`. |
| `0x0071d5f0` | `CG_adjustor_thunk_0071d5f0` | `cg_adjustor_thunk_0071d5f0` | `__thiscall(ECX=self, stack flags)`, tail jump | Adjusts `ECX -= 0x1c` and jumps to `0x0071e410` (`0x0071d5f3`). |
| `0x0071db50` | `BSP_WeaponDirector_AssignSlotTarget` | `bsp_weapon_director_assign_slot_target_0071db50` | `__thiscall(ECX=self, stack src_target)`, `RET 0x4` | Copies a `0x18`-byte `SceneCommandTarget` (`+0x00` byte, `+0x01` byte, `+0x02` word, `+0x04` dword, and four floats at `+0x08..+0x14`). |
| `0x0071ea60` | `BSP_RefPtr_Release_0071ea60` | `bsp_ref_ptr_release_0071ea60` | `__fastcall(ECX=self) -> EAX`, `RET` | Decrements `[held + 0x04]` via `[0x00ce2220]` (`0x0071ea6e`), invokes `vtable[0]()` (`0x0071ea7e`) at zero, clears `*self`, and returns `self`. |
| `0x0071f0e0` | `BSP_RefPtr_Release_0071f0e0` | `bsp_ref_ptr_release_0071f0e0` | `__fastcall(ECX=self)`, `RET` | Decrements `[held + 0x04]` via `[0x00ce2220]` (`0x0071f0ee`), invokes `vtable[0]()` (`0x0071f0fe`) at zero, and clears `*self`. |
| `0x0071f110` | `BSP_RefPtr_Assign_0071f110` | `bsp_ref_ptr_assign_0071f110` | `__thiscall(ECX=self, stack src_slot)`, `RET 0x4` | Assigns `*src_slot` into `[self + 0x04]` with `InterlockedIncrement` `[0x00ce221c]` (`0x0071f129`) and `InterlockedDecrement` `[0x00ce2220]` (`0x0071f137`). |
| `0x0071f150` | `BSP_WeaponDirector_FireTarget` | `bsp_weapon_director_fire_target_0071f150` | `__thiscall(ECX=self) -> EAX`, `RET` | Calls `0x0071ebf0` (`0x0071f150`) and tail-jumps to `0x00521ea0` (`0x0071f157`). |
| `0x0071f3b0` | `BSP_RefCountedPtr_Release` | `bsp_ref_counted_ptr_release_0071f3b0` | `__fastcall(ECX=self)`, `RET` | Releases ref-counted pointer at `*self` via `[0x00ce2220]` (`0x0071f3be`) and `vtable[0]()` (`0x0071f3ce`), then zeroes `*self`. |
| `0x0071f460` | `BSP_RefHolderDB04_Destruct` | `bsp_ref_holder_db04_destruct_0071f460` | `__fastcall(ECX=self)`, `RET` | Installs vtable `0x00cfdb04` at `+0x00` and releases `[self + 0x04]`. |
| `0x0071f530` | `BSP_RefHolderDB04_Destruct_0071f530` | `bsp_ref_holder_db04_destruct_0071f530` | `__fastcall(ECX=self)`, `RET` | Installs vtable `0x00cfdb04` at `+0x00` and releases `[self + 0x04]`. |
| `0x0071f9f0` | `BSP_Vec3Vector_Assign` | `bsp_vec3_vector_assign_0071f9f0` | `__thiscall(ECX=self, stack other) -> EAX`, `RET 0x4` | `std::vector<Vec3>` copy-assignment calling `0x00585e70` (`0x0071fa21`), `0x00419330` (`0x0071fa6d`), `0x00415290` (`0x0071fade`, `0x0071fb11`), `0x0041c4d0` (`0x0071faf1`), `0x00bf65ac` (`0x0071fb07`), `0x00420540` (`0x0071fb19`), and `0x0071f0b0` (`0x0071fb30`). |
| `0x0071fc20` | `TRIV_body_0071fc20` | `triv_body_0071fc20` | `__fastcall(ECX=self) -> EAX`, `RET` | Returns `self + 0x40`. |
| `0x0071fc30` | `TRIV_body_0071fc30` | `triv_body_0071fc30` | `__fastcall(ECX=self) -> AL`, `RET` | Returns `1` (`MOV AL, 1`). |
| `0x0071fc70` | `BSP_DirectorPathObject_Destruct` | `bsp_director_path_object_destruct_0071fc70` | `__fastcall(ECX=self)`, `RET` | Frees `[self + 0x44]` via `0x00bf65ac` (`0x0071fc9d`), zeroes `+0x44..+0x4c`, restores vtable `0x00cfdb04` at `+0x10`, releases `[self + 0x14]`, and sets vtable `0x00cfd93c` at `+0x00`. |
| `0x0071fd10` | `BSP_DirectorPathObject_CopyFrom` | `bsp_director_path_object_copy_from_0071fd10` | `__thiscall(ECX=self, stack other)`, `RET 0x4` | Copies `+0x08/+0x0c`, copies path source ref via `0x007b1bf0` (`0x0071fd4c`) and point vector via `0x0071f9f0` (`0x0071fd5a`); when `[other + 0x14]->vtable[0x24]()` (`0x0071fd75`) equals `other + 0x40`, creates a local path source for `self + 0x40` via `0x007b2250` (`0x0071fd85`), assigns it via `0x0071f110` (`0x0071fd95`), and releases the temporary reference. |
| `0x00720180` | `BSP_CommandControllerBase_Construct` | `bsp_command_controller_base_construct_00720180` | `__thiscall(ECX=self, stack owner) -> EAX`, `RET 0x4` | Initializes `CommandControllerBase`: attaches tick subnode to `owner` via `0x00876020` (`0x007201cb`), constructs 10 command slots via `0x00bf7cd1` (`0x00720236`), requeues via `0x00876120` (`0x00720329`), fills 20 dwords at `+0x1d0` with `0xd01502f9`, and allocates 10 `0x50`-byte `DirectorPathObject` instances via `0x00bf681b` (`0x007203c2`) into `+0x1a4`. |
| `0x00720d80` | `TRIV_body_00720d80` | `triv_body_00720d80` | `__fastcall(ECX=self) -> AL`, `RET` | Returns `uint8_t` at `[self + 0x3c]`. |
| `0x00720d90` | `TRIV_body_00720d90` | `triv_body_00720d90` | `__fastcall(ECX=self) -> AL`, `RET` | Returns `uint8_t` at `[self + 0x3d]`. |
| `0x00720da0` | `TRIV_body_00720da0` | `triv_body_00720da0` | `__fastcall(ECX=self) -> AL`, `RET` | Returns `uint8_t` at `[self + 0x220]`. |
| `0x00720db0` | `TRIV_body_00720db0` | `triv_body_00720db0` | `__fastcall(ECX=self) -> AL`, `RET` | Returns `uint8_t` at `[self + 0x221]`. |
| `0x00720dc0` | `TRIV_body_00720dc0` | `triv_body_00720dc0` | `__fastcall(ECX=self) -> AL`, `RET` | Returns `uint8_t` at `[self + 0x222]`. |
| `0x00720dd0` | `TRIV_body_00720dd0` | `triv_body_00720dd0` | `__fastcall(ECX=self) -> AL`, `RET` | Returns `uint8_t` at `[self + 0x223]`. |
| `0x00720f50` | `TRIV_body_00720f50` | `triv_body_00720f50` | `__thiscall(ECX=self, stack arg1)`, `RET 0x4` | No-op returning with `RET 0x4`. |
| `0x00721030` | `BSP_GameUnitMessage_ReadTargetDescriptor` | `bsp_game_unit_message_read_target_descriptor_00721030` | `__thiscall(ECX=self, stack out_target) -> EAX`, `RET 0x4` | Copies message fields `+0x24..+0x38` into the `0x18`-byte target descriptor `out_target`. |

### 2. GeomMesh, GeomMeshElement, and Narrowphase Queries (`0x00721cb0` through `0x00727a90`)

| Address | Ghidra Name | Reconstructed Symbol | Original ABI | Summary |
|---|---|---|---|---|
| `0x00721cb0` | `BSP_GeomMeshResourceItem_GetRuntimeTypeToken` | `bsp_geom_mesh_resource_item_get_runtime_type_token_00721cb0` | `__thiscall(ECX=self) -> EAX`, `RET` | Returns `[0x00e19be4]`. |
| `0x00721cc0` | `BSP_GeomMeshResourceItem_GetRuntimeTypeNameAddress` | `bsp_geom_mesh_resource_item_get_runtime_type_name_address_00721cc0` | `__thiscall(ECX=self) -> EAX`, `RET` | Returns `[0x00e19bf0]`. |
| `0x00722210` | `BSP_GeomMesh_RadixSortKey0` | `bsp_geom_mesh_radix_sort_key0_00722210` | `__fastcall(ECX=records, EDX=count, stack scratch)`, `RET 0x4` | Probes stack via `0x00bf9410` (`0x00722215`), zeroes `0x1000`-byte histogram table via `0x00bf79f0` (`0x00722236`), and runs 4-pass byte radix sort on 16-byte records keyed by dword `+0x00`. |
| `0x00723030` | `BSP_GeomMeshElement_RootBoxCentre` | `bsp_geom_mesh_element_root_box_centre_00723030` | `__thiscall(ECX=self, stack out_centre)`, `RET 0x4` | When `[self + 0x28] > 0`, computes `(min_box[0] + max_box[0]) * 0.5` (`[0x00d7a280]`, using `dc c9` at `0x0072309b`); otherwise copies fallback vector `[0x00f87574..0x00f8757c]`. |
| `0x007230f0` | `BSP_GeomMesh_RadixSortByAxis` | `bsp_geom_mesh_radix_sort_by_axis_007230f0` | `__stdcall(stack axis, stack start, stack count)`, `RET 0xc` | Dispatches axis `0`, `1`, or `2` on `[0x00e19bd8] + start * 16` and scratch `[0x00e19bdc]` to `0x00722210` (`0x0072315a`), `0x00722400` (`0x0072313a`), or `0x007225f0` (`0x0072311a`). |
| `0x007238e0` | `BSP_GeomMeshElement_TraceSegmentNode` | `bsp_geom_mesh_element_trace_segment_node_007238e0` | `__thiscall(ECX=self, stack query_record, stack node_index, stack first_ordinal, stack ordinal_count) -> AL`, `RET 0x10` | Tests node AABB via `0x0085cad0` (`0x0072390c`); recurses into children `2*n+1` (`0x00723a48`) and `2*n+2` (`0x00723a67`) when `ordinal_count > 0x14`, or tests leaf triangles via `0x0085d020` (`0x007239f9`) with bounds checks via `0x00bf6713` (`0x0072394a`, `0x0072396c`, `0x0072399f`). |
| `0x00723aa0` | `BSP_GeomMeshElement_TraceSegment` | `bsp_geom_mesh_element_trace_segment_00723aa0` | `__thiscall(ECX=self, stack from, stack to, stack out_hit) -> AL`, `RET 0xc` | Builds `0x3c`-byte `SegmentQuery` via `0x00722b20` (`0x00723b00`) and invokes root node trace `0x007238e0` (`0x00723b23`). |
| `0x00723b70` | `BSP_GeomMeshElement_TestSphere` | `bsp_geom_mesh_element_test_sphere_00723b70` | `__thiscall(ECX=self, stack centre, stack radius, stack out_distance) -> AL`, `RET 0xc` | Rejects against root AABB via `0x0085bf90` (`0x00723b8d`), walks indexed triangles via `0x0085df60` (`0x00723cea`) with bounds checks via `0x00bf6713` (`0x00723bf6`, `0x00723c26`, `0x00723c5e`, `0x00723c9a`, `0x00723cd2`), and writes `sqrt(best_dist_sq)` via `0x00bf7030` (`0x00723d34`). |
| `0x00723d60` | `BSP_GeomMesh_TraceSegmentClosestElement` | `bsp_geom_mesh_trace_segment_closest_element_00723d60` | `__thiscall(ECX=self, stack from, stack to, stack out_hit) -> EAX`, `RET 0xc` | Walks `0x2c`-stride elements at `[self + 0x0c, self + 0x10)` with bounds checks via `0x00bf6713` (`0x00723df1`, `0x00723e30`), calling `0x00723aa0` (`0x00723e06`) and shortening the segment far endpoint on each hit. |
| `0x00723e90` | `BSP_GeomMesh_TraceSegmentIntoHitRecord` | `bsp_geom_mesh_trace_segment_into_hit_record_00723e90` | `__thiscall(ECX=self, stack to_world, stack to_local, stack from, stack to, stack record) -> AL`, `RET 0x14` | Transforms `to` and `from` into local space via `0x004142e0` (`0x00723ea5`, `0x00723ed8`), traces closest element via `0x00723d60` (`0x00723f12`), transforms hit point to world space via `0x004142e0` (`0x00723f35`), and populates `record` (`+0x08..+0x10`, `+0x30`, `+0x34`, `+0x38`). |
| `0x00723f80` | `BSP_GeomMesh_CollectSphereHits` | `bsp_geom_mesh_collect_sphere_hits_00723f80` | `__thiscall(ECX=self, stack body_f0, stack to_local, stack centre, stack radius, stack record) -> AL`, `RET 0x14` | Transforms `centre` to local space via `0x004142e0` (`0x00723f9b`), tests each element via `0x00723b70` (`0x0072402c`) with bounds checks via `0x00bf6713` (`0x00724013`, `0x0072406e`, `0x0072409c`), and appends `0x10`-byte part hits via `0x006d2e30` (`0x007240b5`). |
| `0x00724510` | `BSP_UnitPartCollisionShape_TraceSegment` | `bsp_unit_part_collision_shape_trace_segment_00724510` | `__thiscall(ECX=self, stack from, stack to, stack record) -> AL`, `RET 0xc` | Forwards `[self + 0x24]` and `[self + 0x1c] + 0x50` / `+ 0x90` to `0x00723e90` (`0x00724530`). |
| `0x00724a20` | `BSP_GeomMesh_DebugDrawWireframe` | `bsp_geom_mesh_debug_draw_wireframe_00724a20` | `__thiscall(ECX=self, stack matrix, stack rgba)`, `RET 0x8` | Iterates triangles `[self + 0x2c, self + 0x30)` with checked bounds via `0x00bf6713` (`0x00724a31`, `0x00724a54`, `0x00724a60`, `0x00724a74`, `0x00724aa1`, `0x00724ad9`, `0x00724b06`, `0x00724b3e`, `0x00724b67`, `0x00724cc6`), transforms vertices via `0x004142e0` (`0x00724bd7`, `0x00724be8`, `0x00724c36`, `0x00724c4a`, `0x00724c9b`, `0x00724caf`), and draws triangle edges via `0x0085f990` (`0x00724bf1`, `0x00724c53`, `0x00724cb8`). |
| `0x00725550` | `BSP_GeomMeshComponent_DebugDraw` | `bsp_geom_mesh_component_debug_draw_00725550` | `__fastcall(ECX=self)`, `RET` | Refreshes `[self + 0x04]` world matrix via `0x00b6db70` (`0x0072555f`) when bit 1 of `owner + 0x5c` is clear, then draws `[self + 0x08]` wireframe via `0x00724a20` (`0x00725572`). |
| `0x00725830` | `BSP_GeomMeshElement_Destruct` | `bsp_geom_mesh_element_destruct_00725830` | `__fastcall(ECX=self)`, `RET` | Installs vtable `0x00cfdbc8`, frees `+0x20` and `+0x24` via `0x00bf6989` (`0x00725841`, `0x00725851`), frees `+0x14` via `0x00bf65ac` (`0x00725861`), and zeroes `+0x14..+0x1c`. |
| `0x00726710` | `BSP_GeomMeshElement_CopyConstruct` | `bsp_geom_mesh_element_copy_construct_00726710` | `__thiscall(ECX=self, stack other) -> EAX`, `RET 0x4` | Installs vtable `0x00cfdbc8`, copies `+0x04..+0x0c`, copy-constructs index vector `+0x10` via `0x0058aac0` (`0x00726737`), and copies `+0x20..+0x28`. |
| `0x007268d0` | `BSP_GeomMeshResourceItem_Construct` | `bsp_geom_mesh_resource_item_construct_007268d0` | `__fastcall(ECX=self) -> EAX`, `RET` | Calls `0x00b868b0` (`0x007268e9`), installs vtable `0x00cfdbd8`, and zeroes the three embedded vectors (`+0x0c..+0x14`, `+0x1c..+0x24`, `+0x2c..+0x34`). |
| `0x00726930` | `BSP_GeomMeshResourceItem_AcceptsRuntimeTypeToken` | `bsp_geom_mesh_resource_item_accepts_runtime_type_token_00726930` | `__thiscall(ECX=self, stack token) -> AL`, `RET 0x4` | Scans `[0x00e19be4, 0x00e19bf0)` and returns `true` when `token` matches one of the three dwords. |
| `0x00726960` | `BSP_GeomMeshResourceItem_Destruct` | `bsp_geom_mesh_resource_item_destruct_00726960` | `__fastcall(ECX=self)`, `RET` | Frees `+0x2c` and `+0x1c` via `0x00bf65ac` (`0x0072698c`, `0x007269a5`), tidies element vector `+0x08` via `0x007256a0` (`0x007269b9`), and destroys base via `0x00b86890` (`0x007269c8`). |
| `0x00726d80` | `BSP_GeomMeshElementVector_Reserve` | `bsp_geom_mesh_element_vector_reserve_00726d80` | `__thiscall(ECX=self, stack new_capacity)`, `RET 0x4` | Throws via `0x00725c50` (`0x00726dae`) when `new_capacity > 0x05d1745d`; when growing, allocates via `0x007227e0` (`0x00726ddc`), checks iterator bounds via `0x00bf6713` (`0x00726df3`, `0x00726e00`), copy-constructs via `0x00726a80` (`0x00726e1a`), destroys old range via `0x00725670` (`0x00726e52`), and frees old buffer via `0x00bf65ac` (`0x00726e5b`). |
| `0x00727a90` | `BSP_GeomMeshResourceParser_Parse` | `bsp_geom_mesh_resource_parser_parse_00727a90` | `__thiscall(ECX=self, stack node_handle) -> EAX`, `RET 0x4` | Allocates `0x50` bytes via `0x00bf681b` (`0x00727aa9`), constructs item via `0x007268d0` (`0x00727ac1`), and parses payload via `0x00727310` (`0x00727ad7`). |

### 3. HitRecord, Gun Barrel/Bot Helpers, SceneNode, and GunBase (`0x00727b20` through `0x00728960`)

| Address | Ghidra Name | Reconstructed Symbol | Original ABI | Summary |
|---|---|---|---|---|
| `0x00727b20` | `BSP_HitRecordDBFC_Construct` | `bsp_hit_record_dbfc_construct_00727b20` | `__thiscall(ECX=self, stack src_record) -> EAX`, `RET 0x4` | Sets vtable `0x00cfdbfc`, resets `self + 0x04` via `0x00470470` (`0x00727b2c`), and copies `[src_record + 0x3c]` to `[self + 0x58]`. |
| `0x00727b90` | `BSP_HitRecordDC04_Construct` | `bsp_hit_record_dc04_construct_00727b90` | `__thiscall(ECX=self, stack src_record) -> EAX`, `RET 0x4` | Sets vtable `0x00cfdbfc`, resets `self + 0x04` via `0x00470470` (`0x00727b9c`), copies `[src_record + 0x3c]` to `[self + 0x58]`, and installs vtable `0x00cfdc04`. |
| `0x00727c30` | `BSP_Gun_IndexOfReadiestBarrel` | `bsp_gun_index_of_readiest_barrel_00727c30` | `__fastcall(ECX=self) -> EAX`, `RET` | Returns index of minimum float reload timer in `[self + 0x414]` across `[self + 0x448]` barrels (4-way unrolled loop plus remainder). |
| `0x00727d70` | `BSP_Gun_CountReadyBarrels` | `bsp_gun_count_ready_barrels_00727d70` | `__thiscall(ECX=self, stack horizon) -> EAX`, `RET 0x4` | Counts timers in `[self + 0x414]` across `[self + 0x448]` barrels that are `<= horizon` (4-way unrolled loop plus remainder). |
| `0x00727e10` | `BSP_Gun_SetPair43C` | `bsp_gun_set_pair_43c_00727e10` | `__thiscall(ECX=self, stack val_43c, stack val_440)`, `RET 0x8` | Writes `[self + 0x43c] = val_43c` and `[self + 0x440] = val_440`. |
| `0x00727e70` | `BSP_Gun_SetBotSkill` | `bsp_gun_set_bot_skill_00727e70` | `__thiscall(ECX=self, stack skill)`, `RET 0x4` | Invokes `vtable[0x18](skill)` on non-null bot slots `+0x394`, `+0x390`, `+0x398`, `+0x3a0`, `+0x3a4`, and `+0x39c`. |
| `0x00727f10` | `BSP_Gun_SetBotFireTarget` | `bsp_gun_set_bot_fire_target_00727f10` | `__thiscall(ECX=self, stack target, stack flag)`, `RET 0x8` | Fans `target` and `flag` across the six bot slots (`+0x390`, `+0x39c` with `vtable[0x3c](flag)`, `+0x3a0`, `+0x3a4`, `+0x398` gated on `!IsKindOf(0x0f)`, and `+0x394` gated on `IsKindOf(0x0f) || IsKindOf(0x0e)`). |
| `0x00728000` | `BSP_Gun_ClearBotFireTarget` | `bsp_gun_clear_bot_fire_target_00728000` | `__fastcall(ECX=self)`, `RET` | Invokes `vtable[0x30]()` on non-null bot slots `+0x398`, `+0x394`, `+0x390`, `+0x39c`, and `+0x3a0`. |
| `0x007280a0` | `BSP_Gun_ApplyClearDestroyedFailure` | `bsp_gun_apply_clear_destroyed_failure_007280a0` | `__thiscall(ECX=self, stack message)`, `RET 0x4` | When `message->vtable[0x0c](0x69)` is true, clears `[self + 0x3b8]`, invokes `self->vtable[0x1b4](0)`, and calls `0x008781f0` (`0x007280cd`), `0x00925a00` (`0x007280d4`), and `0x00956c20` (`0x007280df`) on `[self + 0x3f0]`. |
| `0x00728340` | `BSP_SceneNode_ClearFlagsRecursive` | `bsp_scene_node_clear_flags_recursive_00728340` | `__thiscall(ECX=self, stack mask)`, `RET 0x4` | Clears `mask` bits from `[self + 0x48]` and recurses over child list `[self + 0x34]` (sibling link `+0x3c`) via `0x00728340` (`0x00728357`). |
| `0x00728480` | `BSP_FreeOwnedBlock_00728480` | `bsp_free_owned_block_00728480` | `__fastcall(ECX=self)`, `RET` | Frees `*self` via `0x00bf6989` (`0x00728487`) when non-null. |
| `0x007284b0` | `BSP_OwnedPtr_Delete_007284b0` | `bsp_owned_ptr_delete_007284b0` | `__fastcall(ECX=self)`, `RET` | Invokes `vtable[0](1)` (`0x007284bf`) on `*self` when non-null and zeroes `*self`. |
| `0x007284d0` | `BSP_FreeOwnedBlock_007284d0` | `bsp_free_owned_block_007284d0` | `__fastcall(ECX=self)`, `RET` | Frees `*self` via `0x00bf6989` (`0x007284da`) when non-null and zeroes `*self`. |
| `0x00728520` | `BSP_OwnedPtr_Delete_00728520` | `bsp_owned_ptr_delete_00728520` | `__fastcall(ECX=self)`, `RET` | Invokes `vtable[0](1)` (`0x0072852f`) on `*self` when non-null and zeroes `*self`. |
| `0x00728610` | `BSP_NativeVector12_Grow_00728610` | `bsp_native_vector12_grow_00728610` | `__fastcall(ECX=self)`, `RET` | Sets capacity `[self + 0x08] = 2 * cap + 2`, allocates `cap * 12` bytes via `0x00bf55be` (`0x00728627`), copies `[self + 0x04]` 12-byte elements, frees old buffer via `0x00bf6989` (`0x0072866f`), and stores new pointer at `[self + 0x00]`. |
| `0x00728960` | `BSP_GunBase_Construct` | `bsp_gun_base_construct_00728960` | `__thiscall(ECX=self, stack flag) -> EAX`, `RET 0x4` | Calls `0x0087b670` (`0x00728968`), installs vtables `0x00cfdc58`, `0x00cfdc40`, `0x00cfdc38`, `0x00cfdc34`, `0x00cfdc2c`, and `0x00cfdc14`, sets `[self + 0xc4] = 0x1e`, and returns `self`. |

---

## Skipped

- **`0x00727310` (`BSP_GeomMeshResourceParser_ParsePayload`)**: Larger multi-section resource payload parser (`0x00727310` through `0x00727a8d`) outside the 77 bounded functions in `argon-v2-seg38-39`. Reached as a host boundary (`geom_mesh_resource_parser_parse_payload_00727310`) from `0x00727a90` (`0x00727ad7`).
