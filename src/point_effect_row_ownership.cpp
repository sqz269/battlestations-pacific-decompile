#include "bsp/point_effect_row_ownership.hpp"
#include "bsp/point_effect_entry_array.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Point-effect row ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(PointEffectInstanceStorage) == 0x114);

std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
} // namespace

RenderCommandReference** assign_point_effect_entry_reference_006fbeb0(
    RenderCommandReference** destination, RenderCommandReference* const* source) noexcept {
    // This canonical helper captures old only after this incoming slot load.
    RenderCommandReference* const incoming = *source;
    assign_render_command_reference(*destination, incoming);
    return destination;
}

void clear_point_effect_entry_reference_006cf070(RenderCommandReference** slot) noexcept {
    RenderCommandReference* const captured = *slot;
    if (captured) {
        release_render_command_reference(*captured);
        *slot = nullptr; // intentionally overwrites a terminal reentry replacement
    }
}

void initialize_point_effect_rows_008682d5(
    PointEffectInstanceStorage& effect, PointEffectRowRuntime& runtime) {
    const auto size_view = runtime.template_rows(*effect.template_84);
    resize_point_effect_entry_array_008672a0(effect.entries_0c,
        signed_word(size_view.count_0c));

    // Both owners are loaded only after the preceding operation has returned.
    auto& node = effect.node_110->transform;
    if ((node.valid_flags & 2u) == 0) refresh_camera_world_00b6db70(node);
    auto& reference = runtime.reference_e188a8_19fc();
    if ((reference.valid_flags & 2u) == 0) refresh_camera_world_00b6db70(reference);
    if (effect.option_08 != 0) return;

    const auto rows = runtime.template_rows(*effect.template_84);
    auto cursor = reinterpret_cast<std::uintptr_t>(rows.rows_08);
    const auto end = cursor + rows.count_0c * 4u;
    std::uint32_t output_offset = 0;
    while (cursor != end) {
        void* const row = *reinterpret_cast<void* const*>(cursor);
        const auto fields = runtime.row_fields(row);
        if (fields.gated_10 != 0 && fields.admitted_1c != 0) {
            RenderCommandReference* const result = runtime.create_virtual_18(row, effect);
            RenderCommandReference* temporary = result;
            // Native86834C reloads backing AFTER the factory, while EDI keeps
            // the original row offset. Retaining the destination value cannot
            // throw under the canonical nonthrowing terminal contract.
            auto** const destination = reinterpret_cast<RenderCommandReference**>(
                reinterpret_cast<std::uintptr_t>(effect.entries_0c.begin) + output_offset);
            assign_point_effect_entry_reference_006fbeb0(destination, &temporary);
            // Normal native release uses captured ESI, not a reloaded slot.
            if (result) release_render_command_reference(*result);
        }
        cursor += 4u;
        output_offset += 4u;
    }
}

} // namespace bsp
