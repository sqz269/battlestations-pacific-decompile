#include "bsp/system_time_constants.hpp"
#include "bsp/camera_frame_state.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error System time constants require MSVC Win32 x87/SSE instruction semantics.
#endif

namespace bsp {
namespace {
class CapturedSingletonGuard {
public:
    CapturedSingletonGuard(ParticleClockLifetimeAccess& access,
        SystemSingletonCriticalSection* section) : access_(access), section_(section) {
        if (section_) {
            access_.enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedSingletonGuard() {
        if (section_) {
            --section_->recursion_18;
            access_.leave_critical_section(*section_);
        }
    }
    CapturedSingletonGuard(const CapturedSingletonGuard&) = delete;
    CapturedSingletonGuard& operator=(const CapturedSingletonGuard&) = delete;
private:
    ParticleClockLifetimeAccess& access_;
    SystemSingletonCriticalSection* section_;
};
}

ParticleClock* particle_clock_singleton_004de4b0(
    ParticleClock* volatile& global_00f8d420, ParticleClockLifetimeAccess& access) {
    auto* existing = global_00f8d420;
    if (existing) return existing; // Native fast path returns the first load.
    auto& manager = access.manager_00415350();
    {
        CapturedSingletonGuard guard(access, manager.section_10);
        if (!global_00f8d420) {
            auto* allocated = access.allocate_00bf681b(0x1c);
            if (allocated) {
                volatile auto* initialized = allocated;
                initialized->secondary_vtable_04 = 0x00ce7d08u;
                initialized->sinks = nullptr;
                initialized->sink_count = 0;
                initialized->native_capacity_10 = 0;
                initialized->native_extra_14 = 0;
                initialized->base_vtable_00 = 0x00ce7d38u;
                initialized->secondary_vtable_04 = 0x00ce7d24u;
                // Native never writes +18h here. Keep the allocator preimage.
            }
            global_00f8d420 = allocated;
            auto& registration_manager = access.manager_00415350();
            access.register_00bd0c30(registration_manager, global_00f8d420);
        }
    } // Unlock the originally captured section before the final global load.
    return global_00f8d420;
}

void store_foliage_manager_time_00af0460(const FoliageGroupManager& owner, float* output) {
    const auto* value = &owner.shader_time;
    __asm {
        mov eax, value
        fld dword ptr [eax]
        mov eax, output
        fstp dword ptr [eax]
    }
}
void store_foliage_time_00ad5700(const SystemFoliageTimeOwner& owner, float* output) {
    const auto* value = &owner.scalar_08;
    __asm {
        mov eax, value
        fld dword ptr [eax]
        mov eax, output
        fstp dword ptr [eax]
    }
}
std::uint8_t foliage_time_byte_00ad5740(const SystemFoliageTimeOwner& owner) noexcept {
    return owner.byte_11;
}
const SystemRenderTimeRegion* render_time_region_00b0cf30(
    const SystemRenderTimeOwner& owner) noexcept {
    return &owner.region_84;
}
const ClockTimestamp* frame_clock_interval_00bee070(FrameClock& owner) noexcept {
    return &owner.interval;
}
const ClockTimestamp* ConcreteSystemTimeTimerVirtuals::interval_1c(FrameClock& owner) {
    return frame_clock_interval_00bee070(owner);
}

const SystemFogState* write_system_time_constants_00b46cb4(
    CameraFrameState& camera, FrameClock* captured_timer, float* output,
    std::size_t output_words, SystemTimeBindings& bindings) {
    if (!output || output_words < 302) {
        throw std::invalid_argument("system time constants need 302 initialized float words");
    }
    const auto* captured_time = &captured_timer->accumulated;
    __asm {
        mov eax, captured_time
        movss xmm0, dword ptr [eax]
        mov eax, output
        movss dword ptr [eax + 528], xmm0 // c33.x
    }
    auto* particle = particle_clock_singleton_004de4b0(
        bindings.particle_00f8d420, bindings.particle_lifetime);
    const auto* particle_time = &particle->shader_time;
    const auto* manager_slot = &bindings.foliage_manager_00f8c274;
    FoliageGroupManager* manager;
    __asm {
        mov eax, particle_time
        movss xmm0, dword ptr [eax]
        mov eax, manager_slot
        mov eax, dword ptr [eax]
        mov manager, eax // 00B46CCC: capture before c33.y store
        mov eax, output
        movss dword ptr [eax + 532], xmm0
    }
    store_foliage_manager_time_00af0460(*manager, output + 134);
    auto* foliage = bindings.foliage_00f8c210;
    store_foliage_time_00ad5700(*foliage, output + 135);
    auto* timer = bindings.timer_01090ab0;
    const auto* pair = bindings.timer_virtuals.interval_1c(*timer);
    const auto* renderer_slot = &bindings.renderer_00f8d39c;
    SystemRenderTimeOwner* first_renderer;
    // Same arithmetic as timestamp_seconds_x87, with the native renderer
    // capture after the second FILD and before FDIVP and the output spill.
    __asm {
        mov eax, pair
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        mov ecx, renderer_slot
        mov ecx, dword ptr [ecx]
        mov first_renderer, ecx
        fdivp st(1), st(0)
        mov edx, output
        fstp dword ptr [edx + 544] // c34.x
    }
    const auto* first_region = render_time_region_00b0cf30(*first_renderer);
    const auto* first_byte = &first_region->byte_24;
    SystemRenderTimeOwner* second_renderer;
    __asm {
        mov eax, first_byte
        movzx ecx, byte ptr [eax]
        cvtsi2ss xmm0, ecx
        mov ecx, renderer_slot
        mov ecx, dword ptr [ecx]
        mov second_renderer, ecx // 00B46D27: second independent global load
        mov eax, output
        movss dword ptr [eax + 548], xmm0 // c34.y
    }
    const auto* second_region = render_time_region_00b0cf30(*second_renderer);
    const auto* second_scalar = &second_region->scalar_28;
    const auto* camera_scalar = &camera.camera.projection.fov; // native +1C4
    const auto* camera_byte = &camera.byte_174;
    const auto* foliage_slot = &bindings.foliage_00f8c210;
    SystemFoliageTimeOwner* final_foliage;
    __asm {
        mov eax, camera_scalar
        fld dword ptr [eax]
        fld1
        mov eax, second_scalar
        movss xmm0, dword ptr [eax]
        fdivrp st(1), st(0)
        mov eax, camera_byte
        movzx edx, byte ptr [eax]
        mov ecx, foliage_slot
        mov ecx, dword ptr [ecx]
        mov final_foliage, ecx // 00B46D51: capture before the three stores
        mov eax, output
        movss dword ptr [eax + 552], xmm0 // c34.z, raw bits
        cvtsi2ss xmm0, edx
        movss dword ptr [eax + 1200], xmm0 // c75.x
        fstp dword ptr [eax + 556] // c34.w, delayed reciprocal spill
    }
    const auto final_byte = foliage_time_byte_00ad5740(*final_foliage);
    const auto* fog_slot = &camera.fog_184;
    const SystemFogState* captured_fog;
    __asm {
        mov ecx, fog_slot
        mov esi, dword ptr [ecx]
        mov captured_fog, esi // 00B46D79: before final conversion/store
        movzx eax, final_byte
        cvtsi2ss xmm0, eax
        mov eax, output
        movss dword ptr [eax + 1204], xmm0 // c75.y
    }
    return captured_fog;
}
}
