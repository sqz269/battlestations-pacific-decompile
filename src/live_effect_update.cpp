#include "bsp/live_effect_update.hpp"

#include <atomic>
#include <cstring>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Live effect update requires MSVC Win32.
#endif

namespace bsp {
namespace {
void** slot(std::uintptr_t value) noexcept { return reinterpret_cast<void**>(value); }
std::uintptr_t tail(NativeRenderPointerArrayStorage& array) noexcept {
    volatile auto& actual = array;
    const auto count = static_cast<std::uint32_t>(actual.count_04);
    return reinterpret_cast<std::uintptr_t>(actual.data_00) + count * 4u - 4u;
}
void retain(void* owner) noexcept {
    auto* count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
        static_cast<std::byte*>(owner) + 4));
    count->fetch_add(1, std::memory_order_seq_cst);
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
__declspec(naked) void __fastcall copy_delta(float*, const float*) {
    __asm {
        fld dword ptr [edx]
        fstp dword ptr [ecx]
        ret
    }
}
}

void erase_live_effect_reference_unordered_0081b010(NativeRenderPointerArrayStorage& array,
    void** const* position, NativeRenderActualOwners& owners) {
    void** const destination = *position;
    void** const first_tail = slot(tail(array));
    if (destination != first_tail) {
        void* const incoming = *first_tail;
        void* const old = *destination;
        if (old != incoming) {
            *destination = incoming;
            if (incoming) retain(incoming);
            if (old) release_native_render_actual_owner(owners, old);
        }
    }
    void** const current_tail = slot(tail(array));
    if (void* const captured = *current_tail) {
        release_native_render_actual_owner(owners, captured);
        *current_tail = nullptr;
    }
    volatile auto& actual = array;
    actual.count_04 = signed_word(static_cast<std::uint32_t>(actual.count_04) - 1u);
}

void update_live_effect_manager_00867ee0(NativeLiveEffectManagerStorage& manager,
    float delta, void* reference, LiveEffectUpdateBindings& bindings) {
    volatile auto& array = manager.effects_10;
    const auto count = static_cast<std::uint32_t>(array.count_04);
    auto cursor = reinterpret_cast<std::uintptr_t>(array.data_00);
    const auto first_end = cursor + count * 4u;
    while (cursor != first_end) {
        if (void* const raw = *slot(cursor)) {
            float argument;
            copy_delta(&argument, &delta);
            advance_point_effect_00867d00(*static_cast<PointEffectInstanceStorage*>(raw),
                argument, reference, bindings.advance);
        }
        cursor += 4u;
    }
    float argument;
    copy_delta(&argument, &delta);
    dispatch_native_effect_entries_00866c60(manager, argument, reference,
        bindings.actual_delta_00f87608, bindings.actual_reference_00f8760c,
        bindings.actual_job_00f87658, bindings.actual_frame_0109cf08,
        bindings.domain, bindings.frame_lifetime);

    cursor = reinterpret_cast<std::uintptr_t>(array.data_00);
    auto current_end = [&]() noexcept {
        const auto current_count = static_cast<std::uint32_t>(array.count_04);
        return reinterpret_cast<std::uintptr_t>(array.data_00) + current_count * 4u;
    };
    while (cursor != current_end()) {
        void** current = slot(cursor);
        copy_delta(&argument, &delta);
        update_point_effect_children_00867790(*static_cast<PointEffectInstanceStorage*>(*current),
            argument, reference, bindings.advance.rows, bindings.restart_rows, bindings.advance.events);
        auto& point = *static_cast<PointEffectInstanceStorage*>(*current); // Reload after callback.
        bool retire = point.field_09 != 0 && point.auxiliary_18.count <= 0 && point.field_0a == 0;
        if (retire) {
            auto entry = reinterpret_cast<std::uintptr_t>(point.entries_0c.begin);
            const auto end = entry + static_cast<std::uint32_t>(point.entries_0c.count) * 4u;
            while (entry != end) {
                if (*reinterpret_cast<RenderCommandReference**>(entry)) { retire = false; break; }
                entry += 4u;
            }
        }
        if (retire)
            erase_live_effect_reference_unordered_0081b010(manager.effects_10, &current, bindings.point_owners);
        else
            cursor += 4u;
    }
    flush_effect_deletions_008671a0(manager, bindings.deletion);
}
} // namespace bsp
