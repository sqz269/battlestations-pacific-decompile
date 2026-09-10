#pragma once

#include "bsp/singleton_lifetime.hpp"
#include "bsp/storage_pool.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp {

// Retained actual sink owner. Native +4 is an InterlockedDecrement target;
// reaching zero calls virtual +0 with ECX owner and NO stack arguments.
// Concrete particle-system owner bodies are a separate reconstruction boundary.
// This object is also the actual ParticleTimeSink used by 00B19A10.
class ParticleClockOwnedSink : public ParticleTimeSink {
public:
    explicit ParticleClockOwnedSink(long references) noexcept
        : reference_count_04(references) {}
    volatile long reference_count_04;
    virtual void destroy_zero_reference_00() noexcept = 0;
};

struct ParticleClockStringNode {
    ParticleClockStringNode* next_00;
    ParticleClockStringNode* previous_04;
    std::uint32_t string_length_08;
    char* string_data_0c;
};

// Exact Win32 2Ch record storage. The record owns both strings and the linked
// list; the sink is shared through its native reference counter. These spans
// must retain the actual allocations and actual sink owner, never copies.
struct ParticleClockOwnedRecord {
    std::uint32_t string_length_00;
    char* string_data_04;
    std::uint32_t unknown_08;
    ParticleClockStringNode* sentinel_0c;
    std::uint32_t list_count_10;
    std::uint32_t unknown_14_24[5];
    ParticleClockOwnedSink* sink_28;
};

// 004DDB40: __thiscall(secondary owner, sink), RET 4; secondary ECX unused.
void release_particle_clock_sink_004ddb40(ParticleClockOwnedSink&) noexcept;
// 004D45A0 includes the post-free tail omitted by the current decompiler:
// clear the list, free/zero its sentinel, then release the record's own string.
void destroy_particle_clock_record_004d45a0(
    ParticleClockOwnedRecord&, SizedStoragePool& actual_string_pool) noexcept;

class ConcreteParticleClockLifetimeAccess final : public ParticleClockLifetimeAccess {
public:
    ConcreteParticleClockLifetimeAccess(SingletonLifetimeDomain&,
        ParticleClock* volatile& global_00f8d420,
        SizedStoragePool& actual_string_pool) noexcept;

    SystemSingletonLifetimeOwner& manager_00415350() override;
    ParticleClock* allocate_00bf681b(std::size_t native_bytes) override;
    void enter_critical_section(SystemSingletonCriticalSection&) override;
    void leave_critical_section(SystemSingletonCriticalSection&) noexcept override;
    void register_00bd0c30(SystemSingletonLifetimeOwner&, ParticleClock*) override;

    // Native vtableCE7D38[0], ECX complete owner, stack flags, RET 4.
    // Returns the original address even when bit 0 frees it. Clears the same
    // global slot unconditionally. There is no native manager unregister call.
    ParticleClock* deleting_destructor_004de340(ParticleClock*, std::uint32_t flags) noexcept;
    // CE7D24[0] receives native this+4 and subtracts four. The host projection
    // takes the retained complete owner explicitly; it is not a binary thunk.
    ParticleClock* secondary_deleting_destructor_004de360(
        ParticleClock& complete_owner, std::uint32_t flags) noexcept;

    // Diagnostic observation of the last raw native +18h allocation bytes,
    // captured before placement construction and copied without a float load.
    std::uint32_t allocation_preimage_18() const noexcept { return preimage_18_; }

private:
    void destroy_00b1b680(ParticleClock&) noexcept;
    SingletonLifetimeDomain& domain_;
    ParticleClock* volatile& global_00f8d420_;
    SizedStoragePool& string_pool_;
    std::uint32_t preimage_18_ = 0;
};

} // namespace bsp
