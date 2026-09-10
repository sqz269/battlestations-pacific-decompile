#include "bsp/particle_clock_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Particle clock lifetime requires MSVC Win32 native pointer and LONG widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
static_assert(sizeof(ParticleClockStringNode) == 0x10);
static_assert(sizeof(ParticleClockOwnedRecord) == kParticleClockRecordStride);
static_assert(offsetof(ParticleClockOwnedRecord, sentinel_0c) == 0x0c);
static_assert(offsetof(ParticleClockOwnedRecord, sink_28) == kParticleClockRecordSinkOffset);

void release_particle_clock_sink_004ddb40(ParticleClockOwnedSink& sink) noexcept {
    if (::InterlockedDecrement(&sink.reference_count_04) == 0) {
        sink.destroy_zero_reference_00(); // no deleting-destructor flag is pushed
    }
}

void destroy_particle_clock_record_004d45a0(
    ParticleClockOwnedRecord& record, SizedStoragePool& string_pool) noexcept {
    // 004D05E0: capture first node before resetting the sentinel and count.
    auto* cursor = record.sentinel_0c->next_00;
    record.sentinel_0c->next_00 = record.sentinel_0c;
    record.sentinel_0c->previous_04 = record.sentinel_0c;
    record.list_count_10 = 0;
    while (cursor != record.sentinel_0c) {
        auto* next = cursor->next_00;
        if (cursor->string_data_0c) {
            string_pool.release_00bd1510(cursor->string_data_0c,
                cursor->string_length_08 + 1u);
        }
        singleton_lifetime_free(cursor);
        cursor = next;
    }
    singleton_lifetime_free(record.sentinel_0c);
    record.sentinel_0c = nullptr; // 004D45D9, after _free returns
    if (record.string_data_04) {
        string_pool.release_00bd1510(record.string_data_04,
            record.string_length_00 + 1u);
    }
    // Native leaves record.string_data_04 and its length unchanged.
}

ConcreteParticleClockLifetimeAccess::ConcreteParticleClockLifetimeAccess(
    SingletonLifetimeDomain& domain, ParticleClock* volatile& global,
    SizedStoragePool& string_pool) noexcept
    : domain_(domain), global_00f8d420_(global), string_pool_(string_pool) {}

SystemSingletonLifetimeOwner& ConcreteParticleClockLifetimeAccess::manager_00415350() {
    return domain_.get_manager_00415350()->system_owner();
}

ParticleClock* ConcreteParticleClockLifetimeAccess::allocate_00bf681b(
    std::size_t native_bytes) {
    if (native_bytes != 0x1c) {
        throw std::invalid_argument("particle clock native allocation must be 0x1c bytes");
    }
    void* raw = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, native_bytes, sizeof(ParticleClock)});
    if (!raw) return nullptr;
    // Read representation bytes BEFORE any host member initialization. A
    // signaling NaN must survive unchanged; do not materialize a float value.
    std::memcpy(&preimage_18_, static_cast<unsigned char*>(raw) + 0x18,
        sizeof(preimage_18_));
    auto* clock = ::new (raw) ParticleClock;
    std::memcpy(&clock->shader_time, &preimage_18_, sizeof(preimage_18_));
    return clock;
}

void ConcreteParticleClockLifetimeAccess::enter_critical_section(
    SystemSingletonCriticalSection& section) {
    singleton_enter_critical_section(section);
}

void ConcreteParticleClockLifetimeAccess::leave_critical_section(
    SystemSingletonCriticalSection& section) noexcept {
    singleton_leave_critical_section(section);
}

void ConcreteParticleClockLifetimeAccess::register_00bd0c30(
    SystemSingletonLifetimeOwner& owner, ParticleClock* clock) {
    static_cast<ConcreteSingletonLifetimeManager*>(owner.native_owner)->register_object(clock);
}

void ConcreteParticleClockLifetimeAccess::destroy_00b1b680(ParticleClock& clock) noexcept {
    clock.base_vtable_00 = 0x00ce7d38u;
    clock.secondary_vtable_04 = 0x00ce7d24u;
    // 004DE290 changes the embedded container vtable before virtual +10 calls.
    clock.secondary_vtable_04 = 0x00ce7d08u;
    // 004DDA40 reloads both the array and count after releasing each sink.
    while (clock.sink_count != 0) {
        release_particle_clock_sink_004ddb40(
            *clock.owned_records[clock.sink_count - 1u].sink_28);
        if (clock.sink_count != 0) {
            destroy_particle_clock_record_004d45a0(
                clock.owned_records[clock.sink_count - 1u], string_pool_);
            --clock.sink_count;
        }
    }
    // 004DC410(0) runs twice in this destructor path. After the drain both
    // calls simply store zero count; capacity and the raw array pointer remain.
    clock.sink_count = 0;
    clock.sink_count = 0;
    singleton_lifetime_free(clock.owned_records); // 004DE2D7 array delete/free
    global_00f8d420_ = nullptr; // unconditional 00B1B6BD, not compare/exchange
    clock.base_vtable_00 = 0x00ce3818u;
}

ParticleClock* ConcreteParticleClockLifetimeAccess::deleting_destructor_004de340(
    ParticleClock* clock, std::uint32_t flags) noexcept {
    auto* const original_address = clock;
    destroy_00b1b680(*clock);
    if ((flags & 1u) != 0) {
        clock->~ParticleClock();
        singleton_lifetime_free(clock);
    }
    return original_address;
}

ParticleClock* ConcreteParticleClockLifetimeAccess::secondary_deleting_destructor_004de360(
    ParticleClock& owner, std::uint32_t flags) noexcept {
    return deleting_destructor_004de340(&owner, flags);
}
} // namespace bsp
