#pragma once
// Segment 2 (0042A920-0043E840) camera movers, cc13-net3. See
// docs/CC13_SEGMENT2_CAMERA_MATH.md for the class map (mover base vtable
// 00CE3B28, constructor 00432750; every class's per-frame step is vtable slot
// DCh) and the evidence for each function. Names are hypotheses.
//
// This file reconstructs the INTF_BOMBVIEW (26h) pair that 0067B4D0 builds:
//   torpedo view  (3CCh bytes, vtable 00CE3E60): 00435120 construct,
//                 0042FEE0 set target, 00430020 step (not yet reconstructed)
//   bomb view     (3F8h bytes, vtable 00CE3FD0): 00435280 construct,
//                 004307F0 set target, 00430DA0 step
// and the orbit helper 0042D610 the bomb step calls.
// The mover base fields used here: the 4x4 pose matrix at +74h (rows +74h
// right, +84h up, +94h forward, +A4h position), the pose-valid byte +C8h,
// +10Ch, the child node chain +48h (linked through child+44h), and the
// sink at +1BCh whose vtable[34h] receives &mover+CCh.
#include <cstdint>

namespace bsp::seg2_cc13 {

struct Vec3 {
    float x{0.0f}, y{0.0f}, z{0.0f};
};

struct Mat4Rows {
    Vec3 right_74;    float w_80{0.0f};
    Vec3 up_84;       float w_90{0.0f};
    Vec3 forward_94;  float w_a0{0.0f};
    Vec3 position_a4; float w_b0{1.0f};
};

inline constexpr std::uint32_t kMoverBaseVtable = 0x00CE3B28u;
inline constexpr std::uint32_t kTorpedoViewVtable = 0x00CE3E60u;  // +0
inline constexpr std::uint32_t kBombViewVtable = 0x00CE3FD0u;     // +0
// The observer object both cameras embed at +384h (vtable 00CE3CF0): +14h,
// i.e. camera+398h, is the observed target; +10h is a byte set to 1.
inline constexpr std::uint32_t kTargetObserverVtable = 0x00CE3CF0u;

// The fields the two INTF_BOMBVIEW cameras add after the mover base.
struct WeaponViewCamera {
    Mat4Rows pose;                 // +74h
    std::uint8_t pose_valid_c8{0}; // +C8h
    std::uint8_t byte_10c{0};      // +10Ch
    void* target_398{nullptr};     // observer +14h
    // Torpedo view: +39Ch target position, +3A8h start position,
    // +3B4h start forward, +3C0h start up.
    // Bomb view: +39Ch forward, +3A8h up, +3B4h target position, +3C0h
    // camera position, +3CCh velocity, +3D8h, +3DCh offset, +3E8h, +3ECh,
    // +3F0h yaw, +3F4h pitch.
    Vec3 v_39c, v_3a8, v_3b4, v_3c0, v_3cc;
    float f_3d8{0.0f};
    Vec3 offset_3dc;
    float f_3e8{0.0f}, f_3ec{0.0f};
    float yaw_3f0{0.0f}, pitch_3f4{0.0f};
};

struct WeaponViewHost {
    virtual ~WeaponViewHost() = default;
    // 006952A0 (ECX = old target, EDX = &camera+384h) / 00694A60 (ECX = new
    // target, EDX = &camera+384h): BSP_Observer_UnregisterPair / RegisterPair.
    virtual void observer_unregister_006952a0(void* target) = 0;
    virtual void observer_register_00694a60(void* target) = 0;
    // [game+19FCh] is the world camera node; its world matrix (node+F0h) is
    // refreshed by 00B6DB70 when bit 2 of node+5Ch is clear, then copied into
    // camera+74h by 004134F0 (BSP_Matrix_Copy4x4X87).
    virtual Mat4Rows camera_node_world() = 0;
    // The target's world position (+FCh..+104h) after 00414DB0 when its +C8h
    // byte is clear.
    virtual Vec3 entity_world_position(void* entity) = 0;
    // target->vtable[5Ch](2Bh): is an MTorpedo.
    virtual bool is_torpedo(void* entity) = 0;
    // [[target+314h]+F4h], read only for a torpedo.
    virtual float torpedo_value_314_f4(void* entity) = 0;
    // launcher->vtable[34h](&out): the vector 004307F0 copies to +3CCh.
    virtual Vec3 launcher_vector_34(void* launcher) = 0;
    // BSS vector 00F87574..00F8757C (no initializer read here).
    virtual Vec3 global_vector_00f87574() = 0;
    virtual float tuning_camera_pos_smooth_e8() = 0;  // [0042E740()]+E8h
    // 0085DC80 BSP_Matrix_OrthonormalizeBasisRows(&camera+74h).
    virtual void orthonormalize_0085dc80(Mat4Rows& m) = 0;
    // 00419440 BSP_Vector3f_Length.
    virtual float vector_length_00419440(const Vec3& v) = 0;
    // 0085E8C0 (ECX = EDX = matrix, stack point, &axis, angle): rotate the
    // matrix about the axis through the point (not opened).
    virtual void rotate_about_axis_0085e8c0(Mat4Rows& m, const Vec3& point, const Vec3& axis,
                                            float angle) = 0;
    // 0042ED50 on each child of +48h, then 00414DB0 on the camera, then
    // [camera+1BCh]->vtable[34h](&camera+CCh).
    virtual void invalidate_children_and_publish() = 0;
};

// 00435120 __fastcall(obj) and 00435280 __fastcall(obj): the constructors'
// own fields (after 00432750). Both set the observer to {vtable 00CE3CF0,
// +4h..+Ch = 0, +10h = 1, +14h = 0}; 00435280 also zeroes +3F0h/+3F4h.
void construct_torpedo_view_00435120(WeaponViewCamera& camera) noexcept;
void construct_bomb_view_00435280(WeaponViewCamera& camera) noexcept;

// 0042FEE0 __thiscall(camera, target) RET 4.
void set_torpedo_view_target_0042fee0(WeaponViewCamera& camera, void* target,
                                      WeaponViewHost& host);
// 004307F0 __thiscall(camera, target, launcher) RET 8.
void set_bomb_view_target_004307f0(WeaponViewCamera& camera, void* target, void* launcher,
                                   WeaponViewHost& host);
// 0042D610 __thiscall(matrix, &point, &axis, angle) RET 0Ch: rotate by
// angle * |axis| about -axis/|axis| through the point (returns the matrix).
void orbit_about_axis_0042d610(Mat4Rows& m, const Vec3& point, const Vec3& axis, float angle,
                               WeaponViewHost& host);
// 00430DA0 __thiscall(camera, float dt) RET 4, vtable 00CE3FD0 slot DCh.
void bomb_view_step_00430da0(WeaponViewCamera& camera, float dt, WeaponViewHost& host);

}  // namespace bsp::seg2_cc13
