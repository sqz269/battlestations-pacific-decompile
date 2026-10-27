// Packet cc13-seg2: segment 2 leaves (0042A920..0043E4D0). See
// include/bsp/profile_seg2_cc13.hpp and docs/CC13_SEG2.md. Each body carries
// its address, the original ABI and every simplification next to it.

#include "bsp/profile_seg2_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include "bsp/engine_seg4_6_9_cc13.hpp"

namespace bsp::profile_seg2_cc13 {
namespace {

// 00D7A208, float -0.0: the negated rotations build (-0.0f - angle) with SUBSS.
constexpr float kNegativeZero = -0.0f;
// 00D7A24C, float 1.0: identity diagonal.
constexpr float kOne = 1.0f;
// 00CE3C88 / 00CE3C84, floats 512.0 and 384.0: the default extent pair.
constexpr float kDefaultExtentX = 512.0f;
constexpr float kDefaultExtentY = 384.0f;
// 00CE38B8, float 10.0.
constexpr float kTen = 10.0f;
// Vtables stored by the destructors and constructors (image addresses).
constexpr std::uint32_t kSingletonBaseVtable = 0x00ce3818;      // slot 0 = 00412440
constexpr std::uint32_t kProfileManagerVtable = 0x00ce411c;     // slot 0 = 00435D10
constexpr std::uint32_t kVehicleClassRegistryVtable = 0x00ce417c;  // slot 0 = 00437F30
constexpr std::uint32_t kScopedByteOverrideVtable = 0x00ce42c0;    // slot 0 = 0043B2C0
// Singleton globals.
constexpr std::uint32_t kGlobalConfigOwner = 0x00f878e4;
constexpr std::uint32_t kProfileManager = 0x00e17680;
constexpr std::uint32_t kVehicleClassRegistry = 0x00f8a0b8;
constexpr std::uint32_t kScopedByteOwner = 0x00f8d394;
// 00E1767D: an empty string in .data (byte 0).
constexpr std::uint32_t kEmptyName = 0x00e1767d;

void store_vtable(Raw object, std::uint32_t vtable) noexcept { object.at<std::uint32_t>(0) = vtable; }

// Shared body of the five observer-reference setters: the reference keeps the
// observed object at +14h and registers itself (EDX = the reference) with it.
void observer_ref_set(Raw ref, void* observed, Seg2Host& host) {
    if (ref.ptr(0x14) != observed) {
        if (ref.ptr(0x14) != nullptr) host.observer_unregister_006952a0(ref.ptr(0x14), ref.p);
        ref.ptr(0x14) = observed;
        if (observed != nullptr) host.observer_register_00694a60(observed, ref.p);
    }
}

void observer_ref_on_released(Raw ref, void* released, Seg2Host& host) {
    void* const observed = host.object_slot_04(released);
    if (observed == ref.ptr(0x14)) {
        ref.ptr(0x14) = nullptr;
        host.observer_unregister_006952a0(observed, ref.p);
    }
}

// 0042D510 / 0042E8C0: the same body twice. A stack identity is copied in
// through 004134F0.
Raw matrix_set_identity(Raw matrix, Seg2Host& host) {
    Mat16 identity{};
    identity.m[0] = kOne;
    identity.m[5] = kOne;
    identity.m[10] = kOne;
    identity.m[15] = kOne;
    host.matrix_copy_004134f0(matrix, identity);
    return matrix;
}

// The three adopt bodies (0042CEA0, 0042EAC0, 0042EB90): accept `other` when
// it is of `class_id`, or when it is a class-54h holder whose 00798D70 camera
// is of `class_id`; null at any step ends the call.
void* adopt_source(void* other, std::int32_t class_id, Seg2Host& host) {
    if (other == nullptr) return nullptr;
    if (host.object_is_class_5c(other, class_id)) return other;
    if (!host.object_is_class_5c(other, 0x54)) return nullptr;
    void* const camera = host.holder_camera_00798d70(other);
    if (camera == nullptr) return nullptr;
    if (!host.object_is_class_5c(camera, class_id)) return nullptr;
    return camera;
}

// The five string releases: when the data pointer is set, hand (data,
// length + 1, 1) back to the sized pool. The pool getter is called first and
// 00BD1510 consumes the three pushed arguments (RET 0Ch).
void release_pooled_string(Raw string, Seg2Host& host) {
    void* const data = string.ptr(0x4);
    if (data != nullptr) {
        const std::uint32_t bytes = string.at<std::uint32_t>(0x0) + 1;
        void* const pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, data, bytes, 1);
    }
}

// 00436370/00436460 (party rows, 004359E0) and 00437DC0/00437DF0 (registry
// cache, 00437BD0): resize to zero, then _free the begin pointer at +0.
void party_rows_destroy(Raw vector, Seg2Host& host) {
    host.party_rows_resize_004359e0(vector, 0);
    host.free_00bf6989(vector.ptr(0x0));
}

void registry_cache_destroy(Raw vector, Seg2Host& host) {
    host.registry_cache_resize_00437bd0(vector, 0);
    host.free_00bf6989(vector.ptr(0x0));
}

void singleton_base_destruct(Raw object, std::uint32_t global, Seg2Host& host) {
    host.global_u32(global) = 0;
    store_vtable(object, kSingletonBaseVtable);
}

}  // namespace

// 0042A920, __thiscall(mover, node), RET 4: +1BCh = node.
void camera_mover_set_camera_node_0042a920(Raw mover, void* node) noexcept { mover.ptr(0x1bc) = node; }

// 0042A940, __thiscall(camera, float), RET 4: +1C4h = v; +1C0h = v when
// +1C0h > v (FCOMIP/JBE: unordered keeps +1C0h). The body leaves its FLD of
// the argument on the x87 stack (no FSTP ST0), unlike 0042A970.
void camera_set_shake_max_0042a940(Raw camera, float value) noexcept {
    camera.at<float>(0x1c4) = value;
    if (camera.at<float>(0x1c0) > value) camera.at<float>(0x1c0) = value;
}

// 0042A970, __thiscall(camera, float), RET 4: +1C8h = v; +1C0h = v when v > +1C0h.
void camera_set_shake_base_0042a970(Raw camera, float value) noexcept {
    camera.at<float>(0x1c8) = value;
    if (value > camera.at<float>(0x1c0)) camera.at<float>(0x1c0) = value;
}

// 0042A9D0, __thiscall(camera, observed), RET 4: clear +39Ch when it is `observed`.
void ship_camera_on_observed_released_0042a9d0(Raw camera, void* observed) noexcept {
    if (observed == camera.ptr(0x39c)) camera.ptr(0x39c) = nullptr;
}

// 0042AAD0, __thiscall(camera, observed), RET 4: clear +378h when it is `observed`.
void plane_chase_camera_on_observed_released_0042aad0(Raw camera, void* observed) noexcept {
    if (observed == camera.ptr(0x378)) camera.ptr(0x378) = nullptr;
}

// 0042BA40, __thiscall(entity), RET, result in ST0: 00414DB0 refreshes the
// pose, _CIatan2(+ECh, +F4h) is rounded to float (FSTP float) and negated.
float entity_get_heading_0042ba40(Raw entity, Seg2Host& host) {
    host.entity_pose_refresh_00414db0(entity);
    const float angle = static_cast<float>(host.ciatan2_00bf701a(static_cast<double>(entity.at<float>(0xec)),
                                                                 static_cast<double>(entity.at<float>(0xf4))));
    return -angle;
}

// 0042BA70, (ECX unused, out), RET 4: out = {512.0f, 384.0f}.
void camera_mover_get_default_extent_0042ba70(float out[2]) noexcept {
    out[0] = kDefaultExtentX;
    out[1] = kDefaultExtentY;
}

// 0042BA90, RET, ST0 = 10.0f (00CE38B8).
float camera_mover_return_ten_0042ba90() noexcept { return kTen; }

// 0042BAD0, __thiscall(mover), RET: when +3Ch is set, tail-jump to its vtable
// +E8h (ECX = +3Ch, no stack arguments); else AL = 0 (upper EAX bits are
// whatever the caller left; the callers test AL).
bool camera_mover_forward_to_target_slot_e8_0042bad0(Raw mover, Seg2Host& host) {
    if (mover.ptr(0x3c) != nullptr) return host.object_slot_e8(mover.ptr(0x3c));
    return false;
}

// 0042D800, __stdcall(object), RET 4 (ECX unused): byte +0Ch = 0.
void camera_mover_clear_byte_c_0042d800(Raw object) noexcept { object.at<std::uint8_t>(0xc) = 0; }

// 0042DF80, __thiscall(camera, other), RET 4: when both follow the same ship
// (+3ACh), 0042DC20(other) copies the ship camera state, then +3F4h (dword)
// and +3F8h/+3FCh (FLD/FSTP float pairs) are copied.
void ship_captain_camera_copy_state_if_same_ship_0042df80(Raw camera, Raw other, Seg2Host& host) {
    if (camera.ptr(0x3ac) == other.ptr(0x3ac)) {
        host.ship_camera_copy_state_0042dc20(camera, other);
        camera.at<std::uint32_t>(0x3f4) = other.at<std::uint32_t>(0x3f4);
        camera.at<float>(0x3f8) = other.at<float>(0x3f8);
        camera.at<float>(0x3fc) = other.at<float>(0x3fc);
    }
}

// 0042CEA0, __thiscall(camera, other), RET 4: plane chase camera (class 51h)
// adopts through 0042CD50.
void plane_chase_camera_adopt_from_0042cea0(Raw camera, void* other, Seg2Host& host) {
    void* const source = adopt_source(other, 0x51, host);
    if (source != nullptr) {
        host.plane_chase_copy_if_same_target_0042cd50(camera, Raw{static_cast<std::uint8_t*>(source)});
    }
}

// 0042EAC0, __thiscall(camera, other), RET 4: ship captain camera (class 50h)
// adopts through 0042DF80.
void ship_captain_camera_copy_state_from_camera_0042eac0(Raw camera, void* other, Seg2Host& host) {
    void* const source = adopt_source(other, 0x50, host);
    if (source != nullptr) {
        ship_captain_camera_copy_state_if_same_ship_0042df80(camera, Raw{static_cast<std::uint8_t*>(source)}, host);
    }
}

// 0042EB90, __thiscall(camera, other), RET 4: byte +404h = 1 first, then the
// submarine camera (class 4Fh) adopts through 0042DFD0.
void submarine_camera_adopt_from_0042eb90(Raw camera, void* other, Seg2Host& host) {
    camera.at<std::uint8_t>(0x404) = 1;
    void* const source = adopt_source(other, 0x4f, host);
    if (source != nullptr) {
        host.submarine_copy_if_same_target_0042dfd0(camera, Raw{static_cast<std::uint8_t*>(source)});
    }
}

// 0042E0E0, __thiscall(entity, target, const float* offset), RET 8. Releases
// the reference-counted object at +3C0h (InterlockedDecrement of +4; at zero
// its vtable slot 0 with no arguments) and clears it, stores the new target
// (no AddRef here), copies the offset to +3C4h..+3CCh and again to
// +3D0h..+3D8h, then calls the entity's vtable +DCh(0.0f).
void entity_set_ref_target_and_offset_0042e0e0(Raw entity, void* target, const float offset[3],
                                               Seg2Host& host) {
    void* const old = entity.ptr(0x3c0);
    if (old != nullptr) {
        auto* const count = reinterpret_cast<long*>(static_cast<std::uint8_t*>(old) + 4);
        if (host.interlocked_decrement_ce2220(count) == 0) host.object_slot_00(old);
        entity.ptr(0x3c0) = nullptr;
    }
    entity.ptr(0x3c0) = target;
    entity.at<float>(0x3c4) = offset[0];
    entity.at<float>(0x3c8) = offset[1];
    entity.at<float>(0x3cc) = offset[2];
    entity.at<float>(0x3d0) = entity.at<float>(0x3c4);
    entity.at<float>(0x3d4) = entity.at<float>(0x3c8);
    entity.at<float>(0x3d8) = entity.at<float>(0x3cc);
    host.entity_slot_dc(entity.p, 0.0f);
}

// 0042F650, __thiscall(mover, target), RET 4. The target at +3ACh is
// re-registered with the observer sub-object at +10h even when unchanged
// (unregister the old one when set, store, register the new one when set).
// The focus reference at +3B0h (observed at +3C4h) is unregistered and
// cleared, +3D4h = 0.0f and byte +3D8h = 0.
void camera_mover_set_target_0042f650(Raw mover, void* target, Seg2Host& host) {
    void* const observer = mover.p + 0x10;
    if (mover.ptr(0x3ac) != nullptr) host.observer_unregister_006952a0(mover.ptr(0x3ac), observer);
    mover.ptr(0x3ac) = target;
    if (target != nullptr) host.observer_register_00694a60(target, observer);
    if (mover.ptr(0x3c4) != nullptr) {
        host.observer_unregister_006952a0(mover.ptr(0x3c4), mover.p + 0x3b0);
        mover.ptr(0x3c4) = nullptr;
    }
    mover.at<float>(0x3d4) = 0.0f;
    mover.at<std::uint8_t>(0x3d8) = 0;
}

// 00432E60, __thiscall(mover, target, mode), RET 8. +3F4h = mode, then
// 0042F650(target). With a target and +3E8h == 0, +3A4h = settings(00424C40)
// +450h * (target+538h)->+A0h (one FMUL of two floats, rounded once by the
// FSTP; the float product below is the same value). Then, when +3E8h == 0,
// +3FCh and +3F8h = 0.0f.
void camera_mover_set_target_and_mode_00432e60(Raw mover, void* target, std::uint32_t mode, Seg2Host& host) {
    mover.at<std::uint32_t>(0x3f4) = mode;
    camera_mover_set_target_0042f650(mover, target, host);
    if (mover.ptr(0x3ac) != nullptr) {
        const Raw unit_class{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(mover.ptr(0x3ac))}.ptr(0x538))};
        if (mover.at<std::uint32_t>(0x3e8) == 0) {
            const Raw settings = host.game_settings_00424c40();
            mover.at<float>(0x3a4) = settings.at<float>(0x450) * unit_class.at<float>(0xa0);
        }
    }
    if (mover.at<std::uint32_t>(0x3e8) == 0) {
        mover.at<float>(0x3fc) = 0.0f;
        mover.at<float>(0x3f8) = 0.0f;
    }
}

// 0042BAF0: XOR AL,AL; RET 4.
bool entity_hit_not_handled_0042baf0(void*) noexcept { return false; }
// 0042BB20: MOV AL,1; RET 0Ch.
bool entity_aim_offset_still_valid_0042bb20(void*, void*, void*) noexcept { return true; }
// 0042BB40: XOR AL,AL; RET.
bool return_false_0042bb40() noexcept { return false; }
// 0042BB70: RET 4.
void tick_element_place_step_pose_0042bb70(void*) noexcept {}
// 0042BB80: RET 4.
void tick_element_tick_advance_0042bb80(void*) noexcept {}
// 0042BB90: RET.
void tick_element_commit_step_pose_0042bb90() noexcept {}
// 0042BBA0: RET 4.
void tick_element_unused_slot_0042bba0(void*) noexcept {}
// 0042BBB0: XOR AL,AL; RET.
bool tick_element_predicate_0042bbb0() noexcept { return false; }

// 0042AE20, __thiscall(v, out, float s), RET 8: out = v / s, three DIVs. Main thread
// (MovCam), 24-bit precision control: each FDIV rounds to 24 bits with the extended
// exponent before its FSTP DWORD (cc13-fixture9).
void vector3_divide_scalar_to_0042ae20(const float v[3], float out[3], float s) noexcept {
    out[0] = static_cast<float>(cc13x87::x87_div24(v[0], s));
    out[1] = static_cast<float>(cc13x87::x87_div24(v[1], s));
    out[2] = static_cast<float>(cc13x87::x87_div24(v[2], s));
}

// 0042AE50, __thiscall(v, float s), RET 4, EAX = v: v *= s. Its only image caller is
// BSP_PlaneWanderer_FixedStep (007BE2A3), judged at the main thread's 24-bit precision
// control: each FMUL rounds to 24 bits with the extended exponent (cc13-fixture9).
float* vector3_scale_in_place_0042ae50(float v[3], float s) noexcept {
    v[0] = static_cast<float>(cc13x87::x87_mul24(s, v[0]));
    v[1] = static_cast<float>(cc13x87::x87_mul24(v[1], s));
    v[2] = static_cast<float>(cc13x87::x87_mul24(s, v[2]));
    return v;
}

// 0042AEB0, __thiscall(v, w), RET 4, EAX = v: v += w.
float* vector3_add_in_place_0042aeb0(float v[3], const float w[3]) noexcept {
    v[0] = w[0] + v[0];
    v[1] = w[1] + v[1];
    v[2] = w[2] + v[2];
    return v;
}

// 0042AEE0, __thiscall(v, w), RET 4, EAX = v: v -= w.
float* vector3_subtract_in_place_0042aee0(float v[3], const float w[3]) noexcept {
    v[0] = v[0] - w[0];
    v[1] = v[1] - w[1];
    v[2] = v[2] - w[2];
    return v;
}

// 0042B220, __fastcall(ECX = a, EDX = b), RET, ST0: listing order
// (a.y*b.y + a.x*b.x) + a.z*b.z on the x87 stack, then FSTP float / FLD, so
// the result is float-rounded (the decompiler's operand order is wrong).
// The caller (MovCam step) runs on the main thread at 24-bit precision control, so every
// FMUL and FADD rounds to 24 bits with the extended exponent (cc13-fixture9).
float vector3_dot_0042b220(const float a[3], const float b[3]) noexcept {
    using cc13x87::x87_add24;
    using cc13x87::x87_mul24;
    const double y = x87_mul24(a[1], b[1]);
    const double x = x87_mul24(a[0], b[0]);
    const double z = x87_mul24(a[2], b[2]);
    return static_cast<float>(x87_add24(x87_add24(y, x), z));
}

// 0042B3C0 / 0042B3F0 / 0042B420, __thiscall(matrix, float angle), RET 4:
// 00B64640/00B646E0/00B64780 build into a 40h stack temporary, 004134F0
// copies it into ECX. EAX is not set to `this` (unlike the negated forms).
void matrix_set_rotation_x_0042b3c0(Raw matrix, float angle, Seg2Host& host) {
    Mat16 temp{};
    host.build_rotation_x_00b64640(temp, angle);
    host.matrix_copy_004134f0(matrix, temp);
}

void matrix_set_rotation_y_0042b3f0(Raw matrix, float angle, Seg2Host& host) {
    Mat16 temp{};
    host.build_rotation_y_00b646e0(temp, angle);
    host.matrix_copy_004134f0(matrix, temp);
}

void matrix_set_rotation_z_0042b420(Raw matrix, float angle, Seg2Host& host) {
    Mat16 temp{};
    host.build_rotation_z_00b64780(temp, angle);
    host.matrix_copy_004134f0(matrix, temp);
}

// 0042B450 / 0042B490, __thiscall(matrix, float angle), RET 4, EAX = matrix:
// the same with angle = -0.0f - angle (SUBSS, single precision). The third
// sibling 0042B4D0 already exists as rotation_z_0042b4d0 in
// src/marker_manager_cc13.cpp and is not duplicated here.
Raw matrix_set_rotation_x_negated_0042b450(Raw matrix, float angle, Seg2Host& host) {
    Mat16 temp{};
    const float negated = kNegativeZero - angle;
    host.build_rotation_x_00b64640(temp, negated);
    host.matrix_copy_004134f0(matrix, temp);
    return matrix;
}

Raw matrix_set_rotation_y_negated_0042b490(Raw matrix, float angle, Seg2Host& host) {
    Mat16 temp{};
    const float negated = kNegativeZero - angle;
    host.build_rotation_y_00b646e0(temp, negated);
    host.matrix_copy_004134f0(matrix, temp);
    return matrix;
}

// 0042B680, __thiscall(matrix, const source*), RET 4, EAX = matrix: 004134F0.
Raw matrix_assign_0042b680(Raw matrix, const Mat16& source, Seg2Host& host) {
    host.matrix_copy_004134f0(matrix, source);
    return matrix;
}

// 0042B7F0, __thiscall(matrix), RET, EAX = matrix: 0085DC80.
Raw matrix_orthonormalize_return_this_0042b7f0(Raw matrix, Seg2Host& host) {
    host.matrix_orthonormalize_0085dc80(matrix);
    return matrix;
}

// 0042D510, __thiscall(matrix), RET, EAX = matrix.
Raw matrix_set_identity_0042d510(Raw matrix, Seg2Host& host) { return matrix_set_identity(matrix, host); }

// 0042E8C0: byte-identical body to 0042D510.
Raw matrix_set_identity_0042e8c0(Raw matrix, Seg2Host& host) { return matrix_set_identity(matrix, host); }

// 0042D5A0, __thiscall(matrix, forward, up), RET 8, EAX = matrix: row 2
// (+20h..+28h) = forward and row 1 (+10h..+18h) = up (FLD/FSTP float
// copies), then 0085DC80.
Raw matrix_set_forward_up_orthonormalize_0042d5a0(Raw matrix, const float forward[3], const float up[3],
                                                  Seg2Host& host) {
    matrix.at<float>(0x20) = forward[0];
    matrix.at<float>(0x24) = forward[1];
    matrix.at<float>(0x28) = forward[2];
    matrix.at<float>(0x10) = up[0];
    matrix.at<float>(0x14) = up[1];
    matrix.at<float>(0x18) = up[2];
    host.matrix_orthonormalize_0085dc80(matrix);
    return matrix;
}

// 0042AFF0, __thiscall(node), RET, EAX = node+F0h: 00B6DB70 first when bit 1
// of byte +5Ch (world valid) is clear.
float* transform_get_world_matrix_0042aff0(Raw node, Seg2Host& host) {
    if ((node.at<std::uint8_t>(0x5c) & 2) == 0) host.transform_refresh_world_00b6db70(node);
    return &node.at<float>(0xf0);
}

// 0042D210, __thiscall(node, out), RET 4: the same refresh, then out = the
// world translation +120h..+128h (dword copies).
void transform_get_world_position_0042d210(Raw node, float out[3], Seg2Host& host) {
    if ((node.at<std::uint8_t>(0x5c) & 2) == 0) host.transform_refresh_world_00b6db70(node);
    auto* const dst = reinterpret_cast<std::uint32_t*>(out);
    dst[0] = node.at<std::uint32_t>(0x120);
    dst[1] = node.at<std::uint32_t>(0x124);
    dst[2] = node.at<std::uint32_t>(0x128);
}

// 0042D870, __thiscall(ref, observed), RET 4, EAX not set.
void observer_ref_set_0042d870(Raw ref, void* observed, Seg2Host& host) { observer_ref_set(ref, observed, host); }
// 0042E700 / 0042E800 / 0042ECA0: the same body with EAX = ref.
Raw observer_ref_set_0042e700(Raw ref, void* observed, Seg2Host& host) {
    observer_ref_set(ref, observed, host);
    return ref;
}
Raw observer_ref_set_0042e800(Raw ref, void* observed, Seg2Host& host) {
    observer_ref_set(ref, observed, host);
    return ref;
}
Raw observer_ref_set_0042eca0(Raw ref, void* observed, Seg2Host& host) {
    observer_ref_set(ref, observed, host);
    return ref;
}

// 0042EC20 / 0042EC50, __thiscall(ref, released), RET 4: released's vtable
// +4h gives the observed object; when it is +14h, clear +14h and unregister
// (ECX = observed, EDX = ref).
void observer_ref_on_observed_released_0042ec20(Raw ref, void* released, Seg2Host& host) {
    observer_ref_on_released(ref, released, host);
}
void observer_ref_on_observed_released_0042ec50(Raw ref, void* released, Seg2Host& host) {
    observer_ref_on_released(ref, released, host);
}

// 0042E840, __fastcall(ECX = object), RET: object when non-null and its
// vtable +5Ch(46h) is true, else null.
void* class_46_dynamic_cast_0042e840(void* object, Seg2Host& host) {
    if (object != nullptr && host.object_is_class_5c(object, 0x46)) return object;
    return nullptr;
}

// 0042E950, __thiscall(unit), RET: +158h, or the empty string at 00E1767D.
const char* unit_instance_name_or_fallback_0042e950(Raw unit, Seg2Host& host) {
    const char* const name = static_cast<const char*>(unit.ptr(0x158));
    if (name == nullptr) return static_cast<const char*>(host.image_address(kEmptyName));
    return name;
}

// 0042AC00 / 00435750 / 00435770, __thiscall(object), RET: clear the
// singleton global (00F878E4 / 00E17680 / 00F8A0B8) and store the singleton
// base vtable 00CE3818.
void global_config_owner_destruct_singleton_base_0042ac00(Raw object, Seg2Host& host) {
    singleton_base_destruct(object, kGlobalConfigOwner, host);
}
void profile_manager_owner_destruct_singleton_base_00435750(Raw object, Seg2Host& host) {
    singleton_base_destruct(object, kProfileManager, host);
}
void vehicle_class_registry_destruct_singleton_base_00435770(Raw object, Seg2Host& host) {
    singleton_base_destruct(object, kVehicleClassRegistry, host);
}

// 00435850, __thiscall(holder), RET: _free(+30h) and clear it when set, then
// tail-jump to 00B65E80 with ECX = holder+38h.
void lua_script_holder_close_00435850(Raw holder, Seg2Host& host) {
    if (holder.ptr(0x30) != nullptr) {
        host.free_00bf6989(holder.ptr(0x30));
        holder.ptr(0x30) = nullptr;
    }
    host.lua_state_owner_close_00b65e80(Raw{holder.p + 0x38});
}

// 004358C0, __thiscall(manager), RET (SEH frame 00C5EE08 only for unwinding):
// vtable 00CE411C, 00B67700 on +4, global 00E17680 = 0, vtable 00CE3818.
void profile_manager_destruct_004358c0(Raw manager, Seg2Host& host) {
    store_vtable(manager, kProfileManagerVtable);
    host.lua_object_destruct_00b67700(Raw{manager.p + 0x4});
    host.global_u32(kProfileManager) = 0;
    store_vtable(manager, kSingletonBaseVtable);
}

// 004356F0, __thiscall(registry), RET: the two 800h-dword index tables at
// +10h and +2010h become the identity permutation (+10h[i] written first).
void vehicle_class_registry_reset_index_tables_004356f0(Raw registry) noexcept {
    for (std::uint32_t i = 0; i < 0x800; ++i) {
        registry.at<std::uint32_t>(0x10 + i * 4) = i;
        registry.at<std::uint32_t>(0x2010 + i * 4) = i;
    }
}

// 00437E10, __thiscall(registry), RET, EAX = registry. Vtable 00CE417C, the
// cache vector +4h..+Ch and the party-row vector +4010h..+4018h zeroed, the
// index tables set as in 004356F0, then 00435960(+4010h, 400h). The SEH frame
// (00C5F191) only destroys the two vectors if the reserve throws.
Raw vehicle_class_registry_construct_00437e10(Raw registry, Seg2Host& host) {
    store_vtable(registry, kVehicleClassRegistryVtable);
    registry.at<std::uint32_t>(0x4) = 0;
    registry.at<std::uint32_t>(0x8) = 0;
    registry.at<std::uint32_t>(0xc) = 0;
    registry.at<std::uint32_t>(0x4010) = 0;
    registry.at<std::uint32_t>(0x4014) = 0;
    registry.at<std::uint32_t>(0x4018) = 0;
    vehicle_class_registry_reset_index_tables_004356f0(registry);
    host.party_rows_reserve_00435960(Raw{registry.p + 0x4010}, 0x400);
    return registry;
}

// 00437EA0, __thiscall(registry), RET: vtable 00CE417C; 004359E0(+4010h, 0)
// and _free(+4010h begin); 00437BD0(+4h, 0) and _free(+4h begin); global
// 00F8A0B8 = 0; vtable 00CE3818.
void vehicle_class_registry_destruct_00437ea0(Raw registry, Seg2Host& host) {
    store_vtable(registry, kVehicleClassRegistryVtable);
    party_rows_destroy(Raw{registry.p + 0x4010}, host);
    registry_cache_destroy(Raw{registry.p + 0x4}, host);
    host.global_u32(kVehicleClassRegistry) = 0;
    store_vtable(registry, kSingletonBaseVtable);
}

// 00437DC0 / 00437DF0, __thiscall(vector), RET.
void registry_cache_destroy_00437dc0(Raw vector, Seg2Host& host) { registry_cache_destroy(vector, host); }
void registry_cache_destroy_00437df0(Raw vector, Seg2Host& host) { registry_cache_destroy(vector, host); }
// 00436370 / 00436460, __thiscall(vector), RET.
void party_rows_destroy_00436370(Raw vector, Seg2Host& host) { party_rows_destroy(vector, host); }
void party_rows_destroy_00436460(Raw vector, Seg2Host& host) { party_rows_destroy(vector, host); }

// 00437F50, __cdecl(void), RET: lazy getter of 00F8A0B8 (operator new 401Ch,
// constructor 00437E10, 00BD0C30 registers the global itself).
void* vehicle_class_registry_get_00437f50(void*& global_00f8a0b8, cc13_sys::LazySingletonHost& host) {
    static const cc13_sys::LazySingletonGetterRow row{0x00437f50, 0x00f8a0b8, 0x401c, 0x00437e10, 0, 0, 0, 0};
    return cc13_sys::lazy_singleton_get(row, global_00f8a0b8, host);
}

// 0042E740, __cdecl(void), RET: lazy getter of 00F87440 (operator new 6D0h,
// constructor 007E2A20, 00BD0C30 registers the global itself).
void* game_tuning_get_0042e740(void*& global_00f87440, cc13_sys::LazySingletonHost& host) {
    static const cc13_sys::LazySingletonGetterRow row{0x0042e740, 0x00f87440, 0x6d0, 0x007e2a20, 0, 0, 0, 0};
    return cc13_sys::lazy_singleton_get(row, global_00f87440, host);
}

// 0043AB50, __thiscall(scope, byte value), RET 4, EAX = scope: vtable
// 00CE42C0, byte +4 = [00F8D394]+10h, then [00F8D394]+10h = value.
void scoped_byte_override_f8d394_construct_0043ab50(Raw scope, std::uint8_t value, Seg2Host& host) {
    store_vtable(scope, kScopedByteOverrideVtable);
    scope.at<std::uint8_t>(0x4) = raw_from_u32(host.global_u32(kScopedByteOwner)).at<std::uint8_t>(0x10);
    raw_from_u32(host.global_u32(kScopedByteOwner)).at<std::uint8_t>(0x10) = value;
}

// 0043AB80, __thiscall(scope), RET: read the saved byte +4, store vtable
// 00CE42C0, then restore [00F8D394]+10h.
void scoped_byte_override_f8d394_destruct_0043ab80(Raw scope, Seg2Host& host) {
    const std::uint8_t saved = scope.at<std::uint8_t>(0x4);
    store_vtable(scope, kScopedByteOverrideVtable);
    raw_from_u32(host.global_u32(kScopedByteOwner)).at<std::uint8_t>(0x10) = saved;
}

// 00431F20, __thiscall(string), RET.
void native_string_destroy_storage_00431f20(Raw string, Seg2Host& host) { release_pooled_string(string, host); }
// 00431F40, __stdcall(string), RET 4 (ECX unused).
void native_string_destroy_storage_00431f40(Raw string, Seg2Host& host) { release_pooled_string(string, host); }
// 00436490, __thiscall(owner), RET: the string at +0Ch.
void native_string_at_0c_destroy_storage_00436490(Raw owner, Seg2Host& host) {
    release_pooled_string(Raw{owner.p + 0xc}, host);
}
// 0043BAE0, __thiscall(owner), RET: the string at +8h.
void native_string_at_8_destroy_storage_0043bae0(Raw owner, Seg2Host& host) {
    release_pooled_string(Raw{owner.p + 0x8}, host);
}
// 0043D090, __stdcall(owner), RET 4 (ECX unused): the string at +8h.
void native_string_at_8_destroy_storage_0043d090(Raw owner, Seg2Host& host) {
    release_pooled_string(Raw{owner.p + 0x8}, host);
}

}  // namespace bsp::profile_seg2_cc13

// oct18b-nA1: newly defined, assembly-reviewed leaves; build-only interfaces.

namespace bsp::profile_seg2_cc13 {
// 0043B2C0: byte restore precedes optional free; no singleton-null guard.
Raw scoped_byte_override_scalar_delete_0043b2c0(Raw scope, std::uint32_t flags,
                                                ScopedByteOverrideDeleteHost& host) {
    scoped_byte_override_f8d394_destruct_0043ab80(scope, host);
    if (flags & 1u) host.free_00bf65ac(scope.p);
    return scope;
}
}
