#pragma once
// cc13-pool08: seven segment-34 routines from the progress-board pool whose earlier
// records carried no body credit (metadata_only / credit_withdrawn, Oct24 audit).
// Each is rebuilt over raw object memory; every call the binary makes is a
// Pool08Host hook invoked at the binary's point with the binary's arguments.
// Library code is bound, not rebuilt: operator new (00BF55BE), memset (00BF79F0),
// memcpy (00BF7680). Names are hypotheses, not recovered symbols. Reconstructed and
// build-tested only; nothing here is called from a live host path.

#include "bsp/gameplay_seg53_cc13.hpp"  // StateSinkEmit

#include <cstdint>

namespace bsp::pool08_cc13 {

using gameplay_seg53_cc13::StateSinkEmit;
using gameplay_seg53_cc13::StateSinkType;

struct Pool08Host {
    virtual ~Pool08Host() = default;

    // --- library bindings (no reconstruction credit) ---
    virtual void* operator_new_00bf55be(std::uint32_t bytes) = 0;
    virtual void memset_00bf79f0(void* dst, std::int32_t value, std::uint32_t bytes) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t bytes) = 0;

    // --- globals, read as dwords at their image address ---
    virtual std::uint32_t global_dword(std::uint32_t address) = 0;

    // --- game code, called not rebuilt here ---
    // 006E0F40 BSP_BombProjectile_WriteState, __thiscall(ECX = shot; sink) RET 4.
    virtual void bomb_projectile_write_state_006e0f40(void* shot, void* sink) = 0;
    // The state sink's own vtable: +4h(key) open, +0Ch(key, value) field, +8h() close,
    // each key/value an 8-byte {type, payload} struct by value (see StateSinkEmit).
    virtual void state_sink_emit(void* sink, const StateSinkEmit& emit) = 0;
    // 006FC960 BSP_DepthChargeProjectile_Construct, __thiscall(ECX = shot; arg) RET 4 -> EAX.
    virtual void* depth_charge_projectile_construct_006fc960(void* shot, std::uint32_t arg) = 0;
    // The new shot's vtable +98h, __thiscall(ECX = shot; 0, [[00E188A8]+19CCh], const
    // float matrix[16]*). Assumed RET 0Ch (the caller does not rebalance ESP).
    virtual void shot_vtable_98(void* shot, std::uint32_t zero, std::uint32_t game_19cc,
        const float* matrix) = 0;
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows, __thiscall(ECX = matrix), RET.
    virtual void matrix_orthonormalize_basis_rows_0085dc80(void* matrix) = 0;
    // 0042ED50 BSP_SceneNode_InvalidateSubtreePose, __thiscall(ECX = node), RET.
    virtual void scene_node_invalidate_subtree_pose_0042ed50(void* node) = 0;
    // 006FDFC0 BSP_DepthChargeLauncher_Construct, __thiscall(ECX = gun; arg) RET 4 -> EAX.
    virtual void* depth_charge_launcher_construct_006fdfc0(void* gun, std::uint32_t arg) = 0;
    // 0041DD40 BSP_NativeString_Resize(ECX = header; length, preserve) RET 8.
    virtual void native_string_resize_0041dd40(void* header, std::uint32_t length,
        std::uint32_t preserve) = 0;
    // 0072E510 BSP_Gun_Construct, __thiscall(ECX = gun; arg) RET 4.
    virtual void gun_construct_0072e510(void* gun, std::uint32_t arg) = 0;
    // 0081ED40 BSP_UnitVehicleBase_Construct, __thiscall(ECX = unit; flag) RET 4.
    virtual void unit_vehicle_base_construct_0081ed40(void* unit, std::uint32_t flag) = 0;
    // 006DFFC0 BSP_UnitInstance_DetachFromMidWorldLists, __thiscall(ECX = unit), RET.
    virtual void unit_detach_from_mid_world_lists_006dffc0(void* unit) = 0;
    // 004837D0 BSP_UnitList_EraseNode, __thiscall(ECX = list; node) RET 4.
    virtual void unit_list_erase_node_004837d0(void* list, void* node) = 0;
};

// 006FC2A0, __thiscall(ECX = platform), RET, EAX = platform; no calls, no EH frame.
void* elevator_platform_construct_006fc2a0(void* platform);
// 006FCC50, __thiscall(ECX = shot; sink) RET 4.
void depth_charge_write_state_006fcc50(void* shot, void* sink, Pool08Host& host);
// 006FD210, __thiscall(ECX = class; a1..a7) RET 1Ch -> EAX = shot + 310h.
void* depth_charge_class_create_projectile_006fd210(void* bullet_class, std::uint32_t a1,
    std::uint32_t a2, const float* a3, const float* a4, std::uint32_t a5, std::uint32_t a6,
    std::uint32_t a7, Pool08Host& host);
// 006FDDA0, __thiscall(ECX = gun; arg) RET 4 -> EAX = gun.
void* turning_gun_base_construct_006fdda0(void* gun, std::uint32_t arg, Pool08Host& host);
// 006FE390, __thiscall(ECX = device row; arg) RET 4 -> EAX = launcher.
void* node_factory_create_depth_charge_launcher_006fe390(void* row, std::uint32_t arg, Pool08Host& host);
// 006FE460, __thiscall(ECX = unit; flag) RET 4 -> EAX = unit.
void* unit_instance_construct_006fe460(void* unit, std::uint32_t flag, Pool08Host& host);
// 006FE670, __thiscall(ECX = unit), RET.
void unit_instance_detach_from_world_lists_006fe670(void* unit, Pool08Host& host);

}  // namespace bsp::pool08_cc13
