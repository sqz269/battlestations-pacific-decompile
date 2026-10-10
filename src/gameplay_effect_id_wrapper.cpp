#include "bsp/gameplay_effect_id_wrapper.hpp"

namespace bsp {
void** acquire_gameplay_effect_by_id_00870cd0(void*& out, std::int32_t id,
    const volatile std::uint32_t& actual_flag_word,
    GameplayEffectAcquisitionContext& context) {
    void** const captured_output = &out;
    const std::int32_t captured_id = id;
    auto* const manager = get_gameplay_effect_manager_004c1650(context.manager);
    const std::uint32_t current_flag = actual_flag_word;
    (void)acquire_gameplay_effect_by_id_008700e0(
        *manager, *captured_output, captured_id, current_flag, context);
    return captured_output;
}
} // namespace bsp
