#include "bsp/keyboard_runtime.hpp"
#include "bsp/keyboard_axis_pairs.hpp"

namespace bsp {
KeyboardActionRuntime::KeyboardActionRuntime(std::vector<InputActionRecord>& actions,
    const InputBindingDeviceGroups& groups, KeyboardDevicePointerHost& pointers) noexcept
    : actions_(actions), groups_(groups), pointers_(pointers) {}
bool KeyboardActionRuntime::action_registered_00a92260(std::uint32_t action) {
    return bsp::input_action_registered_00a92260(actions_, action);
}
bool KeyboardActionRuntime::use_alternate_axis_slots_006aa090(InputSettings& settings,
    const std::string& device, const std::string& input) {
    return bsp::use_alternate_axis_slots_006aa090(settings, device, input);
}
void KeyboardActionRuntime::bind_action_00a93750(std::int32_t action, std::int32_t slot,
    const KeyboardInputBinding& binding, float scale) {
    InputBindingInstallSource source;
    source.device_class = binding.device_type;
    source.device_index = static_cast<std::uint32_t>(binding.device_index);
    const auto pointer_word = static_cast<std::uint32_t>(binding.unknown_08);
    source.cached_device = pointer_word == 0 ? nullptr : pointers_.resolve_cached_device(pointer_word);
    source.input_code = static_cast<std::uint32_t>(binding.key);
    source.flag_word = binding.slider ? 1u : 0u;
    bsp::install_input_action_binding_00a93750(actions_, action, slot, source, scale, groups_);
}
}
