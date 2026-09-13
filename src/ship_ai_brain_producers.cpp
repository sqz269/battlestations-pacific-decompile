#include "bsp/ship_ai_brain_producers.hpp"
#include "bsp/native_particle_object_state.hpp"
#include "bsp/native_renderer_worker_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Brain producers require MSVC Win32 x87 operations.
#endif

namespace bsp {
namespace {
void copy_bits(float& target, const volatile float& source) noexcept {
    float* const output = &target;
    const volatile float* const input = &source;
    __asm {
        mov eax, input
        movss xmm0, dword ptr [eax]
        mov eax, output
        movss dword ptr [eax], xmm0
    }
}
void copy_x87(float& target, const volatile float& source) noexcept {
    float* const output = &target;
    const volatile float* const input = &source;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov eax, output
        fstp dword ptr [eax]
    }
}
void draw_store(float& target, const NativeParticleUnitRandomAccess& access,
    float minimum, float maximum, bool negate) {
    float* const output = &target;
    const auto* const random = &access;
    __asm {
        push maximum
        push minimum
        mov edx, random
        mov ecx, 1
        call native_particle_random_range_00bd2f10
        cmp negate, 0
        je store_draw
        fchs
    store_draw:
        mov eax, output
        fstp dword ptr [eax]
    }
}
} // namespace

void ship_ai_brain_produce_tail_009f126b(ShipAiBrainProducerView view,
    const ShipAiBrainProducerContext& context) {
    float maximum;
    copy_x87(maximum, context.two_00ce3958); // 126B, spilled129F
    view.goal.raw_target_0b20 = 0;
    view.goal.filtered_target_0b24 = 0;
    view.goal.target_visible_0b28 = true;
    copy_bits(view.goal.goal_x_0b2c, context.zero_vector_00f87574[0]);
    copy_bits(view.goal.goal_y_0b30, context.zero_vector_00f87574[1]);
    copy_bits(view.goal.goal_z_0b34, context.zero_vector_00f87574[2]);
    view.goal.speed_commanded_0b38 = false;
    draw_store(view.period_b3c, context.random, 1.0f, maximum, false); // 12CD
    draw_store(view.countdown_b40, context.random, 0.0f, view.period_b3c, true); // 12F1
    copy_bits(view.torpedo_period_b44, context.one_00d7a24c); // 1316
    draw_store(view.torpedo_countdown_b48, context.random, 0.0f, 1.0f, true); // 1321
    copy_bits(view.ship_period_b4c, context.one_00d7a24c); // 1346
    draw_store(view.ship_countdown_b50, context.random, 0.0f, 1.0f, true); // 1351
    float goal_period;
    copy_bits(goal_period, context.two_00ce3958); // 1358
    copy_x87(maximum, context.two_00ce3958); // 1366
    copy_bits(view.goal.refresh_period_0b54, goal_period); // 137A
    draw_store(view.goal.refresh_countdown_0b58, context.random, 0.0f, maximum, true); // 1385

    const auto& torpedo_max_settings = context.settings_00424c40(context.settings_context); // 1392
    const auto& torpedo_min_settings = context.settings_00424c40(context.settings_context); // 1399
    float minimum;
    copy_x87(maximum, torpedo_max_settings.torpedo_avoidance_collect_timer_2); // 139E
    copy_x87(minimum, torpedo_min_settings.torpedo_avoidance_collect_timer_1); // 13B0
    draw_store(view.torpedo_period_b44, context.random, minimum, maximum, false); // 13B9
    const auto& ship_max_settings = context.settings_00424c40(context.settings_context); // 13C4
    const auto& ship_min_settings = context.settings_00424c40(context.settings_context); // 13CB
    copy_x87(maximum, ship_max_settings.ship_avoidance_collect_timer_2); // 13D0
    copy_x87(minimum, ship_min_settings.ship_avoidance_collect_timer_1); // 13E2
    draw_store(view.ship_period_b4c, context.random, minimum, maximum, false); // 13EB
    view.lock_04 = create_native_tracked_critical_section_00bd1860(); // 13F6, store13FF
}
} // namespace bsp
