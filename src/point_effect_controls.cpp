#include "bsp/point_effect_controls.hpp"
#include "bsp/point_effect_entry_array.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point-effect controls require MSVC Win32.
#endif

namespace bsp {
namespace {
class CapturedControlSection final {
public:
    explicit CapturedControlSection(SystemSingletonCriticalSection* section)
        : section_(section) {
        if (section_) {
            singleton_enter_critical_section(*section_);
            ++section_->recursion_18;
        }
    }
    ~CapturedControlSection() {
        if (section_) {
            --section_->recursion_18;
            singleton_leave_critical_section(*section_);
        }
    }
    CapturedControlSection(const CapturedControlSection&) = delete;
    CapturedControlSection& operator=(const CapturedControlSection&) = delete;
private:
    SystemSingletonCriticalSection* section_;
};
}

void restart_point_effect_00866f50(PointEffectInstanceStorage& effect,
    PointEffectRowRuntime& runtime) {
    auto* const definition = effect.template_84;
    effect.option_08 = 0;
    effect.field_0a = 0;
    const auto rows = runtime.template_rows(*definition);
    auto cursor = reinterpret_cast<std::uintptr_t>(rows.rows_08);
    const auto end = cursor + rows.count_0c * 4u;
    std::uint32_t offset = 0;
    while (cursor != end) {
        auto* const row = *reinterpret_cast<void* const*>(cursor);
        if (runtime.row_fields(row).gated_10 != 0) {
            auto* const result = runtime.create_virtual_18(row, effect);
            RenderCommandReference* temporary = result;
            auto** const destination = reinterpret_cast<RenderCommandReference**>(
                reinterpret_cast<std::uintptr_t>(effect.entries_0c.begin) + offset);
            assign_point_effect_entry_reference_006fbeb0(destination, &temporary);
            // Native disarms temporary unwind before this captured-value release.
            if (result) release_render_command_reference(*result);
        }
        cursor += 4u;
        offset += 4u;
    }
}

void stop_point_effect_00867b10(PointEffectInstanceStorage& effect,
    PointEffectChildEvents& events, EffectManager* volatile& global,
    EffectManagerLifetimeAccess& lifetime) {
    auto* const manager = effect_manager_singleton_00866440(global, lifetime);
    CapturedControlSection lock(manager->section_04);
    const bool stopped = effect.option_08 != 0;
    effect.field_0a = 0;
    if (stopped) return;
    auto cursor = reinterpret_cast<std::uintptr_t>(effect.entries_0c.begin);
    effect.option_08 = 1;
    const auto end = cursor + static_cast<std::uint32_t>(effect.entries_0c.count) * 4u;
    while (cursor != end) {
        auto** const slot = reinterpret_cast<RenderCommandReference**>(cursor);
        if (auto* const child = *slot) {
            if (events.child_fields(*child).active_0c != 0)
                events.deactivate_virtual_30(*child);
            // Cancellation and reserve may change this exact source slot.
            append_point_effect_entry_array_00867320(effect.auxiliary_18, slot);
            clear_point_effect_entry_reference_006cf070(slot);
            *slot = nullptr;
        }
        cursor += 4u;
    }
}

CameraTransform*& consume_point_effect_parent_0042d9a0(CameraTransform*& slot,
    CameraTransform* consumed, PointEffectInstanceLinks& links) {
    if (auto* const prior = slot) {
        release_render_command_reference(links.parent_reference(*prior));
        slot = nullptr;
    }
    slot = consumed;
    return slot;
}

} // namespace bsp
