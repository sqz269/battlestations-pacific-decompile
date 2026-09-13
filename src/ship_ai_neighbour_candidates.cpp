#include "bsp/ship_ai_neighbour_candidates.hpp"
#include "bsp/ship_ai_sector_scan.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Candidate walk requires MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
void copy_bits(volatile float& destination, const volatile float& source) noexcept {
    volatile float* const target = &destination;
    const volatile float* const value = &source;
    __asm { mov eax, value }
    __asm { movss xmm0, dword ptr [eax] }
    __asm { mov eax, target }
    __asm { movss dword ptr [eax], xmm0 }
}
void copy_x87(float& destination, const volatile float& source) noexcept {
    float* const target = &destination;
    const volatile float* const value = &source;
    __asm { mov eax, value }
    __asm { fld dword ptr [eax] }
    __asm { mov eax, target }
    __asm { fstp dword ptr [eax] }
}
void subtract(float& out_value, const float& left, const float& right) noexcept {
    const float* const a = &left;
    const float* const b = &right;
    float* const target = &out_value;
    __asm {
        mov eax, a
        fld dword ptr [eax]
        mov eax, b
        fsub dword ptr [eax]
        mov eax, target
        fstp dword ptr [eax]
    }
}
bool within_altitude(const volatile float& candidate_y, const volatile float& self_y) noexcept {
    const volatile float* const candidate = &candidate_y;
    const volatile float* const self = &self_y;
    const float negative_zero = kShipAiSectorNegateZero;
    const double band = kShipAiNeighbourAltitudeBand;
    float delta, magnitude;
    std::uint8_t accepted;
    __asm {
        mov eax, candidate
        fld dword ptr [eax]
        mov eax, self
        fsub dword ptr [eax]
        fstp delta
        fldz
        fld delta
        fcomip st(0), st(1)
        fstp st(0)
        jbe negative_side
        movss xmm0, delta
        jmp compare_band
    negative_side:
        movss xmm0, negative_zero
        subss xmm0, delta
    compare_band:
        movss magnitude, xmm0
        fld magnitude
        fld band
        fcomip st(0), st(1)
        fstp st(0)
        seta accepted
    }
    return accepted != 0;
}
void hull_radius(float& out_value, const volatile float& self_length,
    const volatile float& other_length) noexcept {
    const volatile float* const self = &self_length;
    const volatile float* const other = &other_length;
    const double half = 0.5;
    float* const target = &out_value;
    __asm {
        mov eax, self
        fld dword ptr [eax]
        mov eax, other
        fadd dword ptr [eax]
        fmul half
        mov eax, target
        fstp dword ptr [eax]
    }
}
void closing_range(float& out_value, const volatile float& other_speed,
    const volatile float& self_speed, const float& seconds) noexcept {
    const volatile float* const other = &other_speed;
    const volatile float* const self = &self_speed;
    const float* const time = &seconds;
    float* const target = &out_value;
    __asm {
        mov eax, other
        fld dword ptr [eax]
        mov eax, self
        fadd dword ptr [eax]
        mov eax, time
        fmul dword ptr [eax]
        mov eax, target
        fstp dword ptr [eax]
    }
}
bool inside_radius(float hull, float closing, float minimum, float delta_x, float delta_z) noexcept {
    float reach, radius_squared, distance_squared;
    std::uint8_t accepted;
    __asm {
        fld closing
        fld minimum
        fcomip st(0), st(1)
        fstp st(0)
        jbe use_closing
        movss xmm0, minimum
        jmp add_hull
    use_closing:
        movss xmm0, closing
    add_hull:
        fld hull
        movss reach, xmm0
        fadd reach
        fstp radius_squared
        fld radius_squared
        fmul st(0), st(0)
        fstp radius_squared
        fld delta_z
        fld delta_x
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fstp distance_squared
        fld distance_squared
        fld radius_squared
        fcomip st(0), st(1)
        fstp st(0)
        seta accepted
    }
    return accepted != 0; // Strict, including rejection of unordered/equality.
}
} // namespace

bool ship_ai_neighbour_timer_due_009f15cc(volatile float& countdown,
    const volatile float& period, float step) noexcept {
    volatile float* const value = &countdown;
    const volatile float* const interval = &period;
    float spill;
    std::uint8_t due;
    __asm {
        fld step
        mov eax, value
        fld dword ptr [eax]
        fstp spill
        fld spill
        fxch
        fcomi st(0), st(1)
        jb wait_more
        mov ecx, interval
        fsubr dword ptr [ecx]
        mov due, 1
        faddp st(1), st(0)
        jmp store_timer
    wait_more:
        fsubp st(1), st(0)
        mov due, 0
    store_timer:
        fstp dword ptr [eax]
    }
    return due != 0;
}

void ship_ai_walk_neighbour_candidates_009f1856(bool due, ShipAiNeighbourCandidateHost& host) {
    if (!due) return;
    const void* const initial_self = host.current_self_aa8();
    const auto initial = host.unit_fields(initial_self);
    if (!initial.world_valid_c8) host.refresh_pose_00414db0(initial_self);
    const void* node = host.world_list6_head_64();
    float origin_x, origin_z;
    copy_bits(origin_x, initial.world_x_fc);
    const std::int32_t count = host.world_list6_count_60();
    copy_bits(origin_z, initial.world_z_104);
    if (count <= 0) return;
    std::uint32_t remaining = static_cast<std::uint32_t>(count);
    do {
        const void* const candidate = host.node_payload_08(node);
        if (candidate != host.current_self_aa8()) {
            const auto other = host.unit_fields(candidate);
            if (!other.world_valid_c8) host.refresh_pose_00414db0(candidate);
            const void* const altitude_self = host.current_self_aa8();
            const auto self_y = host.unit_fields(altitude_self);
            if (!self_y.world_valid_c8) host.refresh_pose_00414db0(altitude_self);
            if (within_altitude(other.world_y_100, self_y.world_y_100)) {
                if (!other.world_valid_c8) host.refresh_pose_00414db0(candidate);
                float other_x, other_z, dx, dz, hull, closing, minimum;
                copy_x87(other_x, other.world_x_fc);
                const void* const radius_self = host.current_self_aa8();
                const void* const self_class = host.unit_class_538(radius_self);
                copy_x87(other_z, other.world_z_104);
                const void* const other_class = host.unit_class_538(candidate);
                subtract(dx, origin_x, other_x);
                subtract(dz, origin_z, other_z);
                hull_radius(hull, host.unit_fields(radius_self).field_9c8, other.field_9c8);
                const auto& first_settings = host.settings_00424c40();
                closing_range(closing, host.class_field_500(other_class), host.class_field_500(self_class),
                    first_settings.ship_avoidance_collect_hit_time);
                const auto& second_settings = host.settings_00424c40();
                copy_x87(minimum, second_settings.ship_avoidance_collect_dist);
                if (inside_radius(hull, closing, minimum, dx, dz)) host.admit_009f0d20(candidate);
            }
        }
        --remaining;
        node = host.node_next_04(node);
    } while (remaining != 0);
}
} // namespace bsp
