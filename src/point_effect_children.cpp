#include "bsp/point_effect_children.hpp"
#include "bsp/point_effect_entry_array.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point-effect child updates require MSVC Win32.
#endif

namespace bsp {
namespace {
// Native loads/stores the original delta through x87 before EACH virtual28.
// Preserve its NaN/exception behavior rather than just copying its DWORD.
__declspec(naked) void __fastcall copy_delta(float*, const float*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
// 867996..A3 uses COMISS/JBE against actual D7A218 (+0), not an x87 test.
__declspec(naked) bool __fastcall positive_delta(const float*) {
    __asm {
        movss xmm0,dword ptr [ecx]
        xorps xmm1,xmm1
        comiss xmm0,xmm1
        seta al
        ret
    }
}
RenderCommandReference** slot_at(std::uintptr_t cursor) noexcept {
    return reinterpret_cast<RenderCommandReference**>(cursor);
}
void update_child(PointEffectChildEvents& events, RenderCommandReference& child,
    const float& delta, void* reference) {
    float argument;
    copy_delta(&argument, &delta);
    events.update_virtual_28(child, argument, reference);
}
}

void update_point_effect_children_00867790(PointEffectInstanceStorage& effect,
    float delta, void* reference, PointEffectRowRuntime& rows,
    PointEffectChildRows& restart, PointEffectChildEvents& events) {
    // The definition pointer is loaded first; no callback occurs in projections.
    auto* const definition = effect.template_84;
    auto cursor = reinterpret_cast<std::uintptr_t>(effect.entries_0c.begin);
    auto component = reinterpret_cast<std::uintptr_t>(rows.template_rows(*definition).rows_08);
    const auto end = cursor + static_cast<std::uint32_t>(effect.entries_0c.count) * 4u;
    std::uint32_t offset = 0;
    while (cursor != end) {
        auto** const slot = slot_at(cursor);
        if (auto* const child = *slot) {
            const auto fields = events.child_fields(*child);
            if (fields.type_18 != 1 && fields.type_18 != 4)
                update_child(events, *child, delta, reference);
            // Update/completion/deactivation can replace the captured slot.
            if (events.child_fields(**slot).active_0c == 0 ||
                events.complete_virtual_08(**slot) != 0) {
                if (events.child_fields(**slot).active_0c != 0)
                    events.deactivate_virtual_30(**slot);
                append_point_effect_entry_array_00867320(effect.auxiliary_18, slot);
                clear_point_effect_entry_reference_006cf070(slot);
                *slot = nullptr; // Native also writes when the captured slot was null.
            }
        }
        if (!*slot && effect.option_08 == 0) {
            auto* const* const row_slot = reinterpret_cast<void* const*>(component);
            bool create = restart.restart_virtual_08(*row_slot) != 0;
            if (!create) {
                const auto fields = restart.restart_fields(*row_slot);
                create = fields.gated_10 == 0 && fields.sample_14 == effect.sample_count_88;
            }
            if (create) {
                RenderCommandReference* const result = rows.create_virtual_18(*row_slot, effect);
                RenderCommandReference* temporary = result;
                // Factory may replace the backing; only destination is reloaded.
                auto** const destination = slot_at(
                    reinterpret_cast<std::uintptr_t>(effect.entries_0c.begin) + offset);
                assign_point_effect_entry_reference_006fbeb0(destination, &temporary);
                if (result) release_render_command_reference(*result);
            }
        }
        component += 4u;
        cursor += 4u;
        offset += 4u;
    }
    auto auxiliary = reinterpret_cast<std::uintptr_t>(effect.auxiliary_18.begin);
    auto auxiliary_end = [&]() noexcept {
        return reinterpret_cast<std::uintptr_t>(effect.auxiliary_18.begin) +
            static_cast<std::uint32_t>(effect.auxiliary_18.count) * 4u;
    };
    while (auxiliary != auxiliary_end()) {
        auto** current = slot_at(auxiliary);
        update_child(events, **current, delta, reference); // No type/null gate here.
        if (events.complete_virtual_08(**current) != 0)
            erase_point_effect_entry_array_unordered_00867210(effect.auxiliary_18, &current);
        else
            auxiliary += 4u;
        // On erase native retains its captured iterator, visits swapped-in tail.
    }
    if (positive_delta(&delta)) ++effect.sample_count_88;
}
} // namespace bsp
