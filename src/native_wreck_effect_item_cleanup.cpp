#include "bsp/native_wreck_effect_item_cleanup.hpp"
#include "bsp/live_effect_update.hpp"

namespace bsp {
namespace {
class CapturedItemSection final {
public:
    explicit CapturedItemSection(SystemSingletonCriticalSection* section)
        : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedItemSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedItemSection(const CapturedItemSection&) = delete;
    CapturedItemSection& operator=(const CapturedItemSection&) = delete;
private:
    SystemSingletonCriticalSection* const section_;
};
} // namespace

void cleanup_native_wreck_effect_item_008673b0(NativeWreckEffectItemView item,
    NativeWreckEffectItemCleanupContext& context) {
    auto* const manager = effect_manager_singleton_00866440(
        context.lock_publication_00f87650, context.lock_lifetime);
    const CapturedItemSection lock(manager->section_04);
    const volatile auto& entries = item.entries_0c;
    auto cursor = reinterpret_cast<std::uintptr_t>(entries.data_00);
    const auto end = cursor + static_cast<std::uint32_t>(entries.count_04) * 4u;
    while (cursor != end) {
        auto& cell = *reinterpret_cast<void* volatile*>(cursor);
        if (void* const captured = cell) {
            release_native_render_actual_owner(context.actual_references, captured);
            cell = nullptr;
        }
        cell = nullptr;
        cursor += 4u;
    }

    const volatile auto& auxiliary = item.auxiliary_18;
    void** const position = auxiliary.data_00;
    const auto first = reinterpret_cast<std::uintptr_t>(position);
    auto current_end = first + static_cast<std::uint32_t>(auxiliary.count_04) * 4u;
    while (first != current_end) {
        // Complete131-byte867210 and81B010 bodies are identical, including
        // current header/tail reloads and both current virtual0 call sites.
        erase_live_effect_reference_unordered_0081b010(item.auxiliary_18,
            &position, context.actual_references);
        const auto count = static_cast<std::uint32_t>(auxiliary.count_04);
        current_end = reinterpret_cast<std::uintptr_t>(auxiliary.data_00) + count * 4u;
    }
    item.field_09 = 1;
}
} // namespace bsp
