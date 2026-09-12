#include "bsp/live_effect_event_registry.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Live effect event registry requires MSVC Win32.
#endif

namespace bsp {
namespace {
class CapturedSection final {
public:
    explicit CapturedSection(SystemSingletonCriticalSection* section) : section_(section) {
        if (section_) { singleton_enter_critical_section(*section_); ++section_->recursion_18; }
    }
    ~CapturedSection() {
        if (section_) { --section_->recursion_18; singleton_leave_critical_section(*section_); }
    }
    CapturedSection(const CapturedSection&) = delete;
    CapturedSection& operator=(const CapturedSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};
std::int32_t signed_word(std::uint32_t word) noexcept {
    std::int32_t value;
    std::memcpy(&value, &word, 4);
    return value;
}
void** cell(void** base, std::uint32_t index) noexcept {
    return reinterpret_cast<void**>(reinterpret_cast<std::uintptr_t>(base) + index * 4u);
}
}

void register_live_effect_event_00866a10(NativeLiveEffectManagerStorage& manager, void* event,
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) {
    auto* const lock = effect_deletion_lock_singleton_00866500(global, access);
    CapturedSection section(lock->section_04);
    volatile auto& array = manager.references_1c;
    const auto capacity = static_cast<std::uint32_t>(array.capacity_08);
    const auto count = static_cast<std::uint32_t>(array.count_04);
    if (count == capacity) {
        const auto grown = capacity * 2u + 2u;
        if (grown > static_cast<std::uint32_t>(array.capacity_08)) {
            array.capacity_08 = signed_word(grown);
            const auto bytes = grown > 0x3fffffffu ? 0xffffffffu : grown * 4u;
            auto* const replacement = static_cast<void**>(singleton_lifetime_allocate(
                {SingletonAllocationKind::pointer_slots, bytes, bytes}));
            if (array.data_00) {
                std::uint32_t index = 0;
                if (static_cast<std::uint32_t>(array.count_04) != 0) {
                    do {
                        *cell(replacement, index) = *cell(array.data_00, index);
                        ++index;
                    } while (index < static_cast<std::uint32_t>(array.count_04));
                }
                singleton_lifetime_free(array.data_00);
            }
            array.data_00 = replacement;
        }
    }
    const auto current_count = static_cast<std::uint32_t>(array.count_04);
    void** const destination = cell(array.data_00, current_count);
    *destination = event;
    array.count_04 = signed_word(static_cast<std::uint32_t>(array.count_04) + 1u);
}

void unregister_live_effect_event_00866b00(NativeLiveEffectManagerStorage& manager, void* event,
    EffectManager* volatile& global, EffectManagerLifetimeAccess& access) {
    auto* const lock = effect_deletion_lock_singleton_00866500(global, access);
    CapturedSection section(lock->section_04);
    volatile auto& array = manager.references_1c;
    const auto count = static_cast<std::uint32_t>(array.count_04);
    void** const end = cell(array.data_00, count);
    auto cursor = reinterpret_cast<std::uintptr_t>(array.data_00);
    if (cursor != reinterpret_cast<std::uintptr_t>(end)) {
        for (;;) {
            if (*reinterpret_cast<void**>(cursor) == event) {
                const auto last = static_cast<std::uint32_t>(array.count_04) - 1u;
                *reinterpret_cast<void**>(cursor) = *cell(array.data_00, last);
                array.count_04 = signed_word(static_cast<std::uint32_t>(array.count_04) - 1u);
                break;
            }
            const auto current_count = static_cast<std::uint32_t>(array.count_04);
            auto* const current_data = array.data_00;
            cursor += 4u;
            if (cursor == reinterpret_cast<std::uintptr_t>(cell(current_data, current_count))) break;
        }
    }
}

} // namespace bsp
