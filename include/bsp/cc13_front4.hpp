#pragma once
// Packet cc13-front4: wiring-frontier callees, i.e. unreconstructed functions that
// block wiring of already-reconstructed callers (local/output/wire_readiness.json).
// docs/CC13_FRONT4.md lists every function with its outcome, ABI, the callers it
// unblocks and its uncertainty, plus the targets closed ledger-only or skipped.
//
// Objects are reached through cc13_sweepA::Raw (byte offsets into the Win32
// layout). Every call the image makes goes through a Front4Host method named for
// its target. Descriptive names are hypotheses, not recovered symbols. Nothing
// here is wired.

#include "bsp/cc13_sweepA.hpp"

#include <cstdint>

namespace bsp::cc13_front4 {

using cc13_sweepA::Raw;

struct Front4Host {
    virtual ~Front4Host() = default;

    // The address of the image global at `address` (00E08CD8 the interface-name
    // table, 00F87574 the look-at eye vector).
    virtual void* global_address(std::uint32_t address) = 0;
    // The pointer stored in the image global dword at `address` (00E188D8 the
    // current unit, 00E198C4 the front-end manager).
    virtual void* global_pointer(std::uint32_t address) = 0;


    // --- CRT / logging -----------------------------------------------------
    // 00BF6713 LIBCRT_unmatched_00bf6713, __cdecl(), the checked-iterator failure
    // (calls _invalid_parameter_noinfo). Modelled as returning, as the call site
    // continues after it.
    virtual void invalid_parameter_00bf6713() = 0;
    // 004254B0, __cdecl(const char* format, ...), a one-byte RET (the release
    // build's debug printf). Called with the format and one string argument.
    virtual void debug_printf_004254b0(const char* format, const char* argument) = 0;

    // --- front-end manager -------------------------------------------------
    // 004CC460 BSP_FrontEndManager_PushInterfaceRequest, __thiscall(ECX = manager,
    // int interface id, void* payload), RET 8.
    virtual void frontend_push_interface_request_004cc460(Raw manager, std::int32_t id, void* payload) = 0;

    // --- entity / scene lists ----------------------------------------------
    // 004837D0 BSP_UnitList_EraseNode, __thiscall(ECX = list, node), RET 4.
    virtual void unit_list_erase_node_004837d0(Raw list, void* node) = 0;
    // 009245A0 BSP_EntityAdjustWorldListCounters, __thiscall(ECX = entity,
    // delta_b8, delta_b4), RET 8.
    virtual void entity_adjust_world_list_counters_009245a0(Raw entity, std::uint32_t delta_b8,
                                                            std::uint32_t delta_b4) = 0;
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose, __thiscall(ECX = node), RET.
    virtual void scene_node_invalidate_subtree_pose_0042ed50(void* node) = 0;

    // --- 4x4 float matrices (row-major, 40h bytes) -------------------------
    // 004134F0 BSP_Matrix_Copy4x4X87, __thiscall(ECX = dst, const src*), RET 4.
    virtual void matrix_copy_004134f0(float* dst, const float* src) = 0;
    // 00413920 BSP_Matrix_Multiply4x4, __thiscall(ECX = left, dst*, const right*),
    // RET 8, EAX = dst (dst = left * right).
    virtual float* matrix_multiply_00413920(const float* left, float* dst, const float* right) = 0;
    // 00B63F10 BSP_Matrix_BuildLookAt, __fastcall(ECX = dst, EDX = const eye*;
    // const target*, float up.x, up.y, up.z), RET 10h, EAX = dst.
    virtual float* matrix_build_look_at_00b63f10(float* dst, const float* eye, const float* target,
                                                 float up_x, float up_y, float up_z) = 0;
    // 00B64780, __fastcall(ECX = dst, EDX = const float* angle), EAX = dst: the
    // rotation about Z (cos at m0/m5, sin at m1, -sin at m4).
    virtual float* matrix_rotation_z_00b64780(float* dst, const float* angle) = 0;
    // 0085D3D0 BSP_Matrix_OrthonormalizeFromRowX, __fastcall(ECX = matrix), RET.
    virtual void matrix_orthonormalize_0085d3d0(float* matrix) = 0;

    // --- movie camera ------------------------------------------------------
    // 005CC000 BSP_HudMovieScreen_EnsureMovCam, __thiscall(ECX = screen), RET.
    virtual void hud_movie_screen_ensure_movcam_005cc000(Raw screen) = 0;
    // 007A6910 BSP_MovCam_SetTarget, __thiscall(ECX = movcam, target), RET 4.
    virtual void movcam_set_target_007a6910(void* movcam, void* target) = 0;

    // --- class descriptors / planner teardown ------------------------------
    // 00749050 BSP_VehicleClass_ConstructBase, __thiscall(ECX = this), EAX = this.
    virtual void* vehicle_class_construct_base_00749050(Raw object) = 0;
    // 00A1C8B0 BSP_AiPlanner_OwnsGroup, __thiscall(ECX = planner, group), RET 4, AL.
    virtual bool ai_planner_owns_group_00a1c8b0(Raw planner, void* group) = 0;
    // 006952A0 BSP_Observer_UnregisterPair, __fastcall(ECX = endpoint, EDX = owner).
    virtual void observer_unregister_pair_006952a0(void* endpoint, void* owner) = 0;
    // 00A1D1D0 STL list remove(const value&), __thiscall(ECX = list), RET 4 (vendor body).
    virtual void stl_list_remove_00a1d1d0(Raw list, void* const* value) = 0;
    // 00A16B60 STL list clear, __thiscall(ECX = list) (vendor body).
    virtual void stl_list_clear_00a16b60(Raw list) = 0;
    // 00419CC0 BSP_SizedStoragePool_GetSingleton, __cdecl(), EAX.
    virtual void* sized_storage_pool_00419cc0() = 0;
    // 00BD1510 BSP_SizedStoragePool_ReturnBlock, __thiscall(ECX = pool, block,
    // size, flag), RET 0Ch.
    virtual void sized_storage_pool_return_block_00bd1510(void* pool, void* block, std::uint32_t size,
                                                          std::uint32_t flag) = 0;
    // 00695870 BSP_CallbackOwner_Destroy, __thiscall(ECX = owner), RET.
    virtual void callback_owner_destroy_00695870(Raw owner) = 0;

    // --- AI command follower pass -------------------------------------------
    // [[entity]+5Ch](entity, kind), __thiscall, RET 4, AL.
    virtual bool vcall_entity_is_kind_5c(void* entity, std::uint32_t kind) = 0;
    // 0077C8D0 BSP_Entity_RequestJoinFormation, __thiscall(ECX = entity, other), RET 4.
    virtual void entity_request_join_formation_0077c8d0(void* entity, void* other) = 0;
    // 009FFEB0 BSP_AiCommand_SendEmptySquadronHome, __thiscall(ECX = member), AL.
    virtual bool ai_command_send_empty_squadron_home_009ffeb0(void* member) = 0;
    // 00A10C20 BSP_AiGroup_LeaderPoint, __thiscall(ECX = group), EAX = const float*.
    virtual const float* ai_group_leader_point_00a10c20(void* group) = 0;
    // 00A02020 BSP_AiCommand_IssueMoveToMember, __fastcall(ECX = member, EDX = const point*).
    virtual void ai_command_issue_move_to_member_00a02020(void* member, const float* point) = 0;

    // --- debug draw --------------------------------------------------------
    // 0085F990 BSP_DebugDraw_LineRgbaFloats, __fastcall(ECX = const p0*, EDX =
    // const p1*; float rgba0[4], float rgba1[4] by value), RET 20h.
    virtual void debug_draw_line_rgba_floats_0085f990(const float* p0, const float* p1,
                                                      const float* rgba0, const float* rgba1) = 0;
    // 00860AD0 BSP_DebugDraw_ArcAroundAxis, __fastcall(ECX = const centre*, EDX =
    // const axis*; float radius, float start, float sweep, u32 rgba, u32
    // segments), RET 14h.
    virtual void debug_draw_arc_around_axis_00860ad0(const float* centre, const float* axis, float radius,
                                                     float start, float sweep, std::uint32_t rgba,
                                                     std::uint32_t segments) = 0;
};

// 0068AA40 BSP_InGameInterface_RequestSceneForUnit, __thiscall(ECX = manager,
// unit), RET 4.
void ingame_interface_request_scene_for_unit_0068aa40(Front4Host& host, Raw manager, void* unit);

// 006881F0 BSP_MultiMenu_PopRequestInterface, __thiscall(ECX = menu), RET.
void multi_menu_pop_request_interface_006881f0(Front4Host& host, Raw menu);

// 00928570, __thiscall(ECX = entity), RET: leave the parent's list at
// [entity+30h]+24h.
void game_entity_unlink_from_parent_list_00928570(Front4Host& host, Raw entity);

// 009258F0 BSP_SceneNode_AttachToParents, __thiscall(ECX = instance,
// hierarchy parent, world node, const matrix*), RET 0Ch.
void scene_node_attach_to_parents_009258f0(Front4Host& host, Raw instance, void* hierarchy_parent,
                                           void* world_node, const float* matrix);

// 00B179C0, __thiscall(ECX = material), RET: EAX = (int16)[material+34h].
std::int32_t material_texture_count_00b179c0(Raw material);
// 00B17370, __thiscall(ECX = object), RET: EAX = [object+0Ch].
std::uint32_t object_word_0c_00b17370(Raw object);
// 00B0D1C0, __thiscall(ECX = render service), RET: EAX = [service+1CCh].
std::uint32_t render_service_word_1cc_00b0d1c0(Raw service);
// 00B0D0D0, __thiscall(ECX = render service), RET: EAX = [service+1D4h].
std::uint32_t render_service_word_1d4_00b0d0d0(Raw service);

// 0085E4D0 BSP_Matrix_RotateByAngularVelocity, __fastcall(ECX = out, EDX = in;
// float wx, wy, wz, dt), RET 10h. Returns false on the 0085E871 exit (out not
// written).
bool matrix_rotate_by_angular_velocity_0085e4d0(Front4Host& host, float* out, const float* in, float wx,
                                                float wy, float wz, float dt);

// 00860BE0 BSP_DebugDraw_WireSphereMarker, __fastcall(ECX = const centre*, EDX =
// const float rgba[4]*; float radius, u32 arc rgba, u8 detailed), RET 0Ch.
void debug_draw_wire_sphere_marker_00860be0(Front4Host& host, const float* centre, const float* rgba,
                                            float radius, std::uint32_t arc_rgba, std::uint8_t detailed);

// 005CD160 BSP_MovCam_EnsureTarget, __thiscall(ECX = screen), RET, EAX = [screen+30h].
void* movcam_ensure_target_005cd160(Front4Host& host, Raw screen);

// --- one-instruction accessors (__thiscall(ECX = object), RET unless noted) ---
// 006529E0: AL = byte [+4] (the overlay-busy flag of the object at [00E198C4]+BCh).
std::uint8_t overlay_busy_006529e0(Raw overlay);
// 00951F10: EAX = [+128h], the vehicle class descriptor's equipment count.
std::int32_t vehicle_class_equipment_count_00951f10(Raw descriptor);
// 00952140, __thiscall(ECX unused; unused, iterator), RET 8: AL = ([iterator+4] == 0).
bool device_iterator_at_end_00952140(Raw self, std::uint32_t unused, Raw iterator);
// 006AC220: EAX = [+1F8h], the landing pad's occupant.
void* landing_pad_occupant_006ac220(Raw pad);
// 00A3E540: AL = byte [+120h] (the storage gate of [00F8ABE8]).
std::uint8_t storage_gate_00a3e540(Raw owner);
// 007D7A80: AL = byte [+94h] of the plane.
std::uint8_t plane_byte_94_007d7a80(Raw plane);
// 00939F80 / 00939F70: ST0 = float [+34h] / [+38h] (water / fire amounts).
float water_amount_00939f80(Raw object);
float fire_amount_00939f70(Raw object);
// 0080E490 BSP_UnitInstance_GetPartsObject: EAX = [+1018h].
void* unit_parts_object_0080e490(Raw unit);
// 00BBDDF0 BSP_Atmosphere_GetCloudManager: EAX = [+3Ch].
void* atmosphere_cloud_manager_00bbddf0(Raw atmosphere);
// 0080E160 BSP_UnitInstance_WeaponDirectorDirect: EAX = [+738h].
void* unit_weapon_director_0080e160(Raw unit);

// 007D1E50, __thiscall(ECX = this), EAX = this: the plane class descriptor base
// constructor (vptrs 00D05EAC at +0 and 00D05EA8 at +6Ch).
void* plane_class_construct_base_007d1e50(Front4Host& host, Raw object);

// 00A1EEF0 BSP_AiPlanner_DestructBase, __thiscall(ECX = planner), RET.
void ai_planner_destruct_base_00a1eef0(Front4Host& host, Raw planner);

// 00A10DC0 BSP_AiCommand_FollowerPass, __thiscall(ECX = command), RET.
void ai_command_follower_pass_00a10dc0(Front4Host& host, Raw command);

} // namespace bsp::cc13_front4
