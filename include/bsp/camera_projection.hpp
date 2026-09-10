#pragma once
#include <array>
#include <cstdint>

namespace bsp {
using CameraMatrix = std::array<float, 16>;
//004134f0: sequential x87 copy; native ECX dst/stack src/RET4 returns dst.
void copy_camera_matrix_004134f0(CameraMatrix&, const CameraMatrix&);
// References to the actual camera fields. Near/far are not adjacent to aspect
// in the native owner; the two intervening words are not projection inputs.
struct CameraProjectionBacking {
    float& fov;
    float& aspect;
    float& near_plane;
    float& far_plane;
    CameraMatrix& original;
    CameraMatrix& cached;
    std::uint32_t& valid_flags;
};
// New C++ interface, not a native camera object. Borrowed fields must outlive
// this view. Binding does not read or initialize them. Default construction
// owns zeroed diagnostic storage, not recovered native constructor defaults.
struct CameraProjection {
private:
    struct OwnedStorage {
        float fov{}, aspect{}, near_plane{}, far_plane{};
        CameraMatrix original{}, cached{};
        std::uint32_t valid_flags{};
    } owned_;
public:
    CameraProjection() noexcept;
    explicit CameraProjection(CameraProjectionBacking) noexcept;
    // Value construction owns a new copy. Assignment writes through the
    // target's existing references; neither copy nor move rebinds that target.
    CameraProjection(const CameraProjection&) noexcept;
    CameraProjection(CameraProjection&&) noexcept;
    CameraProjection& operator=(const CameraProjection&) noexcept;
    CameraProjection& operator=(CameraProjection&&) noexcept;
    float& fov;
    float& aspect;
    float& near_plane;
    float& far_plane; // native +1C4,+1C8,+1D4,+1D8
    CameraMatrix& original;
    CameraMatrix& cached; // +1E0,+2A0
    std::uint32_t& valid_flags; // +2F0
};
// Native ECX destination; four stack floats; RET10h. No input validation.
// Inlines00412e20 FSINCOS tangent with native float32 spill boundaries.
void build_projection_00b642f0(CameraMatrix&, float fov, float aspect,
    float near_plane, float far_plane);
void set_camera_fov_00b6fbb0(CameraProjection&, float);
void set_camera_aspect_00b6fbd0(CameraProjection&, float);
void set_camera_near_00b6fbf0(CameraProjection&, float);
void set_camera_far_00b6fc10(CameraProjection&, float);
void set_camera_projection_00b6fd60(CameraProjection&, const CameraMatrix&);
const CameraMatrix& get_camera_projection_00b6fcf0(CameraProjection&);
}
