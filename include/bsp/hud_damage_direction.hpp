#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// The member at ship screen+18Ch, initialized inline by 0064B780:
// widgets +0..+C = null, intensities +10..+1C = 0, enabled +20 = 1.
// 0064C0F0 binds the four Villanas*_Icon widgets; 0064E31A supplies enabled.
// 0064B170 and 0064ABD0 must borrow this same storage. Widget pointers are
// the host GUI objects, not executable native virtual tables.
struct HudDamageDirectionStorage {
    void* widgets[4];
    float intensity[4];
    std::uint32_t enabled;
};
static_assert(sizeof(void*) != 4 || offsetof(HudDamageDirectionStorage, intensity) == 0x10);
static_assert(sizeof(void*) != 4 || offsetof(HudDamageDirectionStorage, enabled) == 0x20);
static_assert(sizeof(void*) != 4 || sizeof(HudDamageDirectionStorage) == 0x24);

// A narrow view of the camera and math calls used by 0064B170. The camera
// and FOV are queried separately, in native order, so a live typed camera
// owner needs neither a shadow game allocation nor a copied +1C4h field.
struct HudDamageDirectionCalls {
    virtual ~HudDamageDirectionCalls() = default;
    virtual void* damage_direction_camera() = 0;
    virtual float damage_direction_camera_fov() = 0;
    virtual const float* node_inverse_world_matrix_00b6e0d0(void* node) = 0;
    virtual void vector3_transform_affine_point_004142e0(
        const float* point, float* out, const float* matrix) = 0;
    virtual void geometry_normalize_with_floor_0042b260(float* xyz) = 0;
    virtual float geometry_asin_clamped_0042cf10(float value) = 0;
    virtual double crt_atan2_00bf701a(double y, double x) = 0;
};

} // namespace bsp
