#include "bsp/game_input_settings_runtime.hpp"

namespace bsp::game {
GameInputSettingsRuntime::GameInputSettingsRuntime(
    GameInputSettingsRuntimeBindings bindings) noexcept
    : scripts_{bindings.strings, bindings.bootstrap, bindings.files},
      keyboard_{bindings.strings},
      tables_{scripts_, keyboard_, bindings.crt_sse2_conversion,
          bindings.one_00d7a24c, bindings.base_zero_replacement_00cf7fe8,
          bindings.stack_policy.descriptor_flag_stack_preimage,
          bindings.stack_policy.vector_opaque_stack_preimage,
          bindings.stack_policy.sensitivity_default_stack_preimage},
      lifetime_{bindings.settings_publication_00e198e8,
          bindings.manager_publication_01090aa0, tables_} {}

void* GameInputSettingsRuntime::get() {
    return get_native_input_settings_005547d0(lifetime_);
}
} // namespace bsp::game
