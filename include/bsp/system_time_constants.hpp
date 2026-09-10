#pragma once

#include "bsp/frame_clock.hpp"
#include "bsp/render_tail.hpp"
#include "bsp/camera_transform.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
struct CameraFrameState;
struct SystemFogState;

// Retained projections of the actual owners; the references must remain valid.
// No field constructor or time unit is inferred from these getters.
struct SystemFoliageTimeOwner {
    const float& scalar_08;
    const std::uint8_t& byte_11;
};
struct SystemRenderTimeRegion {
    const std::uint8_t& byte_24;
    const float& scalar_28;
};
struct SystemRenderTimeOwner {
    SystemRenderTimeRegion region_84;
    // Same00F8D39C owner as the prefix's earlier captured matrix source.
    const CameraMatrix& matrix_1d8;
};

// The captured critical section is manager+10h; its counter is section+18h.
// The adapter owns the actual Win32 critical section and its retained mapping.
struct SystemSingletonCriticalSection {
    void* native_section;
    std::uint32_t& recursion_18;
};
struct SystemSingletonLifetimeOwner {
    void* native_owner;
    SystemSingletonCriticalSection* const& section_10;
};
class ParticleClockLifetimeAccess {
public:
    virtual ~ParticleClockLifetimeAccess() = default;
    virtual SystemSingletonLifetimeOwner& manager_00415350() = 0;
    // 00BF681B gets 0x1c. A nonnull projection must retain the allocation's
    // +18h bit pattern in shader_time, including on the first shader read.
    // Value-initializing ParticleClock is not an implementation of this call.
    virtual ParticleClock* allocate_00bf681b(std::size_t native_bytes) = 0;
    virtual void enter_critical_section(SystemSingletonCriticalSection&) = 0;
    virtual void leave_critical_section(SystemSingletonCriticalSection&) noexcept = 0;
    // Called even for null, after the second manager getter and slot reload.
    // Actual 00BD0C30 ignores null and registers nonnull for LIFO destruction.
    virtual void register_00bd0c30(SystemSingletonLifetimeOwner&, ParticleClock*) = 0;
};

// 004DE4B0: no native inputs, EAX singleton, RET. Reconstructs the lazy
// double-check, captured lock, allocation initialization, publication, second
// manager lookup, registration and final reload. Uses the same ParticleClock
// object consumed by set_particle_clock_time_00b19a10, never a second clock.
ParticleClock* particle_clock_singleton_004de4b0(
    ParticleClock* volatile& global_00f8d420, ParticleClockLifetimeAccess&);

// Getter plus the caller's FSTP float32. Native getter ABI is ECX owner,
// ST0 float10 result, RET; the sink interface avoids an extra float return spill.
void store_foliage_manager_time_00af0460(const FoliageGroupManager&, float*);
void store_foliage_time_00ad5700(const SystemFoliageTimeOwner&, float*);
// Native ECX owner, AL byte result, RET. The full byte is significant.
std::uint8_t foliage_time_byte_00ad5740(const SystemFoliageTimeOwner&) noexcept;
// Native ECX owner, EAX owner+84h, RET. Returns the actual embedded region.
const SystemRenderTimeRegion* render_time_region_00b0cf30(
    const SystemRenderTimeOwner&) noexcept;
// Concrete vtable00D68D50+1Ch entry: LEA EAX,[ECX+40h]; RET.
// Ghidra had no function at this entry when inspected; bytes 8d4140c3.
const ClockTimestamp* frame_clock_interval_00bee070(FrameClock&) noexcept;

class SystemTimeTimerVirtuals {
public:
    virtual ~SystemTimeTimerVirtuals() = default;
    // Native virtual+1Ch on the freshly reloaded global01090AB0 owner.
    // Implementations must dispatch that actual owner; the concrete frame
    // clock adapter calls frame_clock_interval_00bee070(owner).
    virtual const ClockTimestamp* interval_1c(FrameClock& owner) = 0;
};
class ConcreteSystemTimeTimerVirtuals final : public SystemTimeTimerVirtuals {
public:
    const ClockTimestamp* interval_1c(FrameClock& owner) override;
};

struct SystemTimeBindings {
    FrameClock* const volatile& timer_01090ab0;
    ParticleClock* volatile& particle_00f8d420;
    FoliageGroupManager* const volatile& foliage_manager_00f8c274;
    SystemFoliageTimeOwner* const volatile& foliage_00f8c210;
    SystemRenderTimeOwner* const volatile& renderer_00f8d39c;
    ParticleClockLifetimeAccess& particle_lifetime;
    SystemTimeTimerVirtuals& timer_virtuals;
};

// Native fragment00B46CB4..00B46D96. captured_timer is the actual slot value
// loaded at00B46C89, before the preceding axis stores. Writes only c33, c34,
// c75.xy of initialized storage; >=302 words required. Returns camera+184h
// captured after00AD5740 and before the final c75.y store. No null-owner
// fallback, range normalization, default clock values or guessed units.
const SystemFogState* write_system_time_constants_00b46cb4(
    CameraFrameState&, FrameClock* captured_timer, float* output,
    std::size_t output_words, SystemTimeBindings&);
}
