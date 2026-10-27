// cc13-pool08: segment-34 routines rebuilt from the exported listing (bsp.py show <addr>
// --asm, read instruction by instruction on 2026-10-24). See
// include/bsp/pool_pool08_cc13.hpp for the host contract. Names are hypotheses.
// Build-tested only: no caller in src, not fixture- or game-validated.

#include "bsp/pool_pool08_cc13.hpp"

#include <cstring>

namespace bsp::pool08_cc13 {
namespace {

std::uint8_t* at(void* p, std::uint32_t offset) { return static_cast<std::uint8_t*>(p) + offset; }
template <class T> T& field(void* p, std::uint32_t offset) { return *reinterpret_cast<T*>(at(p, offset)); }
template <class T> T* pointer_from_dword(std::uint32_t value) {
    return reinterpret_cast<T*>(static_cast<std::uintptr_t>(value));
}

}  // namespace

// ---------------------------------------------------------------------------
// 006FC2A0 BSP_ElevatorPlatform_Construct (16 insns)
// ABI: __thiscall(ECX = platform), plain RET, EAX = platform (MOV EAX,ECX at 006FC2A8);
// no calls, no EH frame. Only caller 006CFAF0 BSP_MotherShipSite_Construct (the
// site's platform at +44h). Stores, in listing order: vtables 00CFB9F4 (+0h) and
// 00CFB9C8 (+4h); +24h/+28h/+2Ch/+34h = 0; byte +30h = 1; +20h = 00CF89B4;
// +54h = 0; +4Ch = [00D7A24C] = 1.0f (bytes 00 00 80 3F); +8h = platform + 4h
// (a self-link of the +4h sub-object). The live elevator model is
// GameUnitsHost CarrierElevator (src/game_hosts_units.cpp), which does not run this.
// ---------------------------------------------------------------------------
void* elevator_platform_construct_006fc2a0(void* platform) {
    field<std::uint32_t>(platform, 0x0) = 0x00CFB9F4u;
    field<std::uint32_t>(platform, 0x4) = 0x00CFB9C8u;
    field<std::uint32_t>(platform, 0x24) = 0;
    field<std::uint32_t>(platform, 0x28) = 0;
    field<std::uint32_t>(platform, 0x2C) = 0;
    field<std::uint32_t>(platform, 0x34) = 0;
    field<std::uint8_t>(platform, 0x30) = 1;
    field<std::uint32_t>(platform, 0x20) = 0x00CF89B4u;
    field<std::uint32_t>(platform, 0x54) = 0;
    field<float>(platform, 0x4C) = 1.0f;
    field<void*>(platform, 0x8) = at(platform, 0x4);
    return platform;
}

// ---------------------------------------------------------------------------
// 006FCC50 BSP_DepthCharge_WriteState (60 insns)
// ABI: __thiscall(ECX = shot; sink) RET 4; no EH frame. Slot +A8h of vtable 00CFBA80.
// After the bomb writer 006E0F40(ECX = shot; sink) it emits, through the sink's vtable,
// +4h(key "_depthCharge" 00CFBC70) open; +0Ch(key, value {2, float}) for "dragvert"
// (00CFBC64) +468h, "dragside" (00CFBC58) +46Ch, "divedepth" (00CFBC4C) +470h; +8h()
// close. Keys are {0, const char*}; each 8-byte struct is built on the stack by value
// (006FCC5E..006FCD10). The floats move with MOVSS, so no x87 rounding is involved.
// Same shape as the torpedo writer 00855D30 (src/gameplay_seg53_cc13.cpp), whose
// StateSinkEmit record is reused.
// ---------------------------------------------------------------------------
void depth_charge_write_state_006fcc50(void* shot, void* sink, Pool08Host& host) {
    host.bomb_projectile_write_state_006e0f40(shot, sink);
    StateSinkEmit open{};
    open.op = StateSinkEmit::Op::open_table;
    open.key = "_depthCharge";  // no value: type left at its default, as in 00855D30's model
    host.state_sink_emit(sink, open);
    struct Field { const char* key; std::uint32_t offset; };
    for (const Field& f : {Field{"dragvert", 0x468}, Field{"dragside", 0x46C}, Field{"divedepth", 0x470}}) {
        StateSinkEmit emit{};
        emit.op = StateSinkEmit::Op::field;
        emit.key = f.key;
        emit.type = StateSinkType::real;
        emit.real = field<float>(shot, f.offset);
        host.state_sink_emit(sink, emit);
    }
    StateSinkEmit close{};
    close.op = StateSinkEmit::Op::close_table;
    host.state_sink_emit(sink, close);
}

// ---------------------------------------------------------------------------
// 006FD210 BSP_DepthChargeClass_CreateProjectile (127 insns)
// ABI: __thiscall(ECX = bullet class; a1..a7) RET 1Ch, EAX = shot + 310h (006FD3E2).
// Slot +20h of the MDepthCharge class vtable. a5..a7 are never read.
// Steps: operator new(474h), memset 0; under EH state 0 (handler 00C83A08, whose
// funclet frees the block through 0042B100 if the constructor throws)
// 006FC960(ECX = block; a1); state -1. A null block skips the constructor and the
// body then dereferences null, as the binary does. +54h = a2, +314h = class. The shot's
// vtable +98h gets (0, [[00E188A8]+19CCh], &identity) where the identity 4x4 is built
// from [00D7A24C] = 1.0 and XORPS zero (006FD295..006FD2EF).
// x87 (006FD310..006FD36A): s = class+50h (FLD/FSTP to a float slot), velocity
// +318h/+31Ch/+320h = a4[i] * s, each product of two floats rounded once to float, so a
// float multiply is exact here; a4 is copied raw to +94h..+9Ch (ECX = +74h, +20h..+28h)
// and 0085DC80 re-orthonormalises the matrix at +74h. a3 is copied to +A4h..+ACh,
// +370h..+378h and +37Ch..+384h; bytes +C8h and +10Ch = 0; then every child in the
// +48h -> +44h sibling chain gets 0042ED50.
// Uncertainty: the +98h call's RET 0Ch is inferred from the unchanged ESP math after it.
// ---------------------------------------------------------------------------
void* depth_charge_class_create_projectile_006fd210(void* bullet_class, std::uint32_t a1,
    std::uint32_t a2, const float* a3, const float* a4, std::uint32_t, std::uint32_t,
    std::uint32_t, Pool08Host& host) {
    void* block = host.operator_new_00bf55be(0x474);
    host.memset_00bf79f0(block, 0, 0x474);
    void* shot = block != nullptr ? host.depth_charge_projectile_construct_006fc960(block, a1) : nullptr;
    field<std::uint32_t>(shot, 0x54) = a2;
    field<void*>(shot, 0x314) = bullet_class;
    float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    void* game = pointer_from_dword<void>(host.global_dword(0x00E188A8u));
    host.shot_vtable_98(shot, 0, field<std::uint32_t>(game, 0x19CC), identity);
    const float scale = field<float>(field<void*>(shot, 0x314), 0x50);
    const float velocity[3] = {a4[0] * scale, a4[1] * scale, a4[2] * scale};
    field<float>(shot, 0x318) = velocity[0];
    field<float>(shot, 0x31C) = velocity[1];
    field<float>(shot, 0x320) = velocity[2];
    void* matrix = at(shot, 0x74);
    for (std::uint32_t i = 0; i < 3; ++i) field<float>(matrix, 0x20 + i * 4) = a4[i];
    host.matrix_orthonormalize_basis_rows_0085dc80(matrix);
    void* sub = at(shot, 0x310);
    for (std::uint32_t i = 0; i < 3; ++i) field<float>(shot, 0xA4 + i * 4) = a3[i];
    for (std::uint32_t i = 0; i < 3; ++i) field<float>(sub, 0x60 + i * 4) = a3[i];
    for (std::uint32_t i = 0; i < 3; ++i) field<float>(sub, 0x6C + i * 4) = a3[i];
    field<std::uint8_t>(shot, 0xC8) = 0;
    field<std::uint8_t>(shot, 0x10C) = 0;
    for (void* node = field<void*>(shot, 0x48); node != nullptr; node = field<void*>(node, 0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(node);
    }
    return sub;
}

// ---------------------------------------------------------------------------
// 006FDDA0 BSP_TurningGunBase_Construct (22 insns)
// ABI: __thiscall(ECX = gun; arg) RET 4, EAX = gun; no EH frame. Callers 006FDED0
// and 00730E80. 0072E510(ECX = gun; arg), then vtables 00CFBD20 (+0h), 00CFBD08
// (+10h), 00CFBD00 (+24h), 00CFBCFC (+170h), 00CFBCF4 (+1E4h), 00CFBCDC (+310h);
// byte +490h = 0; floats +4C8h, +4C4h, +4C0h, +4BCh = 0 (XORPS, that order);
// +C4h = 22h; +4CCh = FFFFFFFFh. Field meanings: include/bsp/gun_aiming.hpp.
// ---------------------------------------------------------------------------
void* turning_gun_base_construct_006fdda0(void* gun, std::uint32_t arg, Pool08Host& host) {
    host.gun_construct_0072e510(gun, arg);
    field<std::uint32_t>(gun, 0x0) = 0x00CFBD20u;
    field<std::uint32_t>(gun, 0x10) = 0x00CFBD08u;
    field<std::uint32_t>(gun, 0x24) = 0x00CFBD00u;
    field<std::uint32_t>(gun, 0x170) = 0x00CFBCFCu;
    field<std::uint32_t>(gun, 0x1E4) = 0x00CFBCF4u;
    field<std::uint32_t>(gun, 0x310) = 0x00CFBCDCu;
    field<std::uint8_t>(gun, 0x490) = 0;
    for (std::uint32_t offset : {0x4C8u, 0x4C4u, 0x4C0u, 0x4BCu}) field<float>(gun, offset) = 0.0f;
    field<std::uint32_t>(gun, 0xC4) = 0x22;
    field<std::uint32_t>(gun, 0x4CC) = 0xFFFFFFFFu;
    return gun;
}

// ---------------------------------------------------------------------------
// 006FE390 BSP_NodeFactory_CreateDepthChargeLauncher (67 insns)
// ABI: __thiscall(ECX = device row; arg) RET 4, EAX = launcher. Reached through a
// factory table (data reference only). operator new(4DCh), memset 0; under EH state 0
// (handler 00C83A98, funclet frees through 0042B100) 006FDFC0(ECX = block; arg);
// a null block skips the constructor and the stores hit null, as in the binary.
// +3F4h = row, +354h = row (006FE3E9/006FE3EF), then state -1 (006FE3FA); the NativeString at +154h takes the row's name
// [row+54h]: 0041DD40(+154h; strlen or 0 when null, preserve 0), then when the data
// pointer is set memcpy(data, name, length) - the length without the terminator.
// ---------------------------------------------------------------------------
void* node_factory_create_depth_charge_launcher_006fe390(void* row, std::uint32_t arg, Pool08Host& host) {
    void* block = host.operator_new_00bf55be(0x4DC);
    host.memset_00bf79f0(block, 0, 0x4DC);
    void* launcher = block != nullptr ? host.depth_charge_launcher_construct_006fdfc0(block, arg) : nullptr;
    field<void*>(launcher, 0x3F4) = row;
    field<void*>(launcher, 0x354) = row;
    const char* name = field<const char*>(row, 0x54);
    void* header = at(launcher, 0x154);
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
    host.native_string_resize_0041dd40(header, length, 0);
    if (void* data = field<void*>(header, 4); data != nullptr) {
        host.memcpy_00bf7680(data, name, field<std::uint32_t>(header, 0));
    }
    return launcher;
}

// ---------------------------------------------------------------------------
// 006FE460 BSP_UnitInstance_Construct (17 insns)
// ABI: __thiscall(ECX = unit; flag) RET 4, EAX = unit; no EH frame. Only caller
// 006FE590 BSP_VehicleClassDestroyer_CreateInstance. 0081ED40(ECX = unit; flag), then
// the eight vptrs 00CFC3D0 (+0h), 00CFC3B8 (+10h), 00CFC3B0 (+24h), 00CFC3AC (+170h),
// 00CFC3A4 (+1E4h), 00CFC38C (+310h), 00CFC388 (+38Ch), 00CFC384 (+72Ch); +C4h = 7.
// The same table is kept as data in include/bsp/unit_instance.hpp.
// ---------------------------------------------------------------------------
void* unit_instance_construct_006fe460(void* unit, std::uint32_t flag, Pool08Host& host) {
    host.unit_vehicle_base_construct_0081ed40(unit, flag);
    field<std::uint32_t>(unit, 0x0) = 0x00CFC3D0u;
    field<std::uint32_t>(unit, 0x10) = 0x00CFC3B8u;
    field<std::uint32_t>(unit, 0x24) = 0x00CFC3B0u;
    field<std::uint32_t>(unit, 0x170) = 0x00CFC3ACu;
    field<std::uint32_t>(unit, 0x1E4) = 0x00CFC3A4u;
    field<std::uint32_t>(unit, 0x310) = 0x00CFC38Cu;
    field<std::uint32_t>(unit, 0x38C) = 0x00CFC388u;
    field<std::uint32_t>(unit, 0x72C) = 0x00CFC384u;
    field<std::uint32_t>(unit, 0xC4) = 7;
    return unit;
}

// ---------------------------------------------------------------------------
// 006FE670 BSP_UnitInstance_DetachFromWorldLists (19 insns)
// ABI: __thiscall(ECX = unit), plain RET; no EH frame. Slot +134h of vtable 00CFC3D0
// (called at 00928F2C from the on-killed base). 006DFFC0(ECX = unit), then the class
// list at [unit+30h]+6Ch (first node at +70h; node +4h next, +8h payload) is walked and
// the first node whose payload is the unit is erased through 004837D0(ECX = list; node).
// The live host does the equivalent unlink itself (world_lists_unlink_unit_00928f24 in
// src/game_hosts_units.cpp); this body is not switched in.
// ---------------------------------------------------------------------------
void unit_instance_detach_from_world_lists_006fe670(void* unit, Pool08Host& host) {
    host.unit_detach_from_mid_world_lists_006dffc0(unit);
    void* list = at(field<void*>(unit, 0x30), 0x6C);
    for (void* node = field<void*>(list, 4); node != nullptr; node = field<void*>(node, 4)) {
        if (field<void*>(node, 8) == unit) {
            host.unit_list_erase_node_004837d0(list, node);
            return;
        }
    }
}

}  // namespace bsp::pool08_cc13
