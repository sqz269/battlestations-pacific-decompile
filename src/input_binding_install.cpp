#include "bsp/input_binding_install.hpp"
#include "bsp/input_tick.hpp"

#include <cstddef>
#include <limits>
#include <stdexcept>

namespace bsp {

bool input_action_registered_00a92260(
    const std::vector<InputActionRecord>& actions, std::uint32_t action) noexcept {
    return action < actions.size() && actions[action].registered;
}

InputActionBinding copy_input_action_binding_00a93100(
    const InputActionBinding& source) {
    // The standard value copy owns new required/forbidden modifier vectors,
    // matching the two 00a92ee0 calls at 00a9315c and 00a93174.
    auto result = source;
    const float* scale_source = &source.scale;
    float* scale_destination = &result.scale;
    // Native 00a93179..00a93180 uses x87 rather than a raw word copy;
    // preserve its signaling-NaN conversion and caller FP environment.
    __asm {
        mov eax,scale_source
        mov edx,scale_destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
    return result;
}

void reserve_input_action_bindings_00a93220(
    std::vector<InputActionBinding>& bindings, std::int32_t capacity) {
    const auto requested = static_cast<std::size_t>(capacity < 1 ? 1 : capacity);
    if (requested <= bindings.capacity()) return;

    std::vector<InputActionBinding> replacement;
    replacement.reserve(requested);
    for (const auto& binding : bindings) {
        replacement.push_back(copy_input_action_binding_00a93100(binding));
    }
    // 00a932c0..00a9332e destroys the old arrays, then stores the new base and
    // capacity; _free returns despite the original export's false early exits.
    bindings.swap(replacement);
}

void resize_input_action_bindings_00a93500(
    std::vector<InputActionBinding>& bindings, std::int32_t count) {
    if (count < 0) throw std::out_of_range("negative input binding count");
    const auto requested = static_cast<std::size_t>(count);
    if (bindings.capacity() < requested) {
        reserve_input_action_bindings_00a93220(bindings, count);
    }
    // Existing InputActionBinding defaults match 00a9354a..00a93574,
    // including DAT_00d7a24c=3f800000h (1.0f). Host vector destruction owns
    // the two modifier arrays which native explicitly shrinks and frees.
    bindings.resize(requested);
}

void install_input_action_binding_00a93750(
    std::vector<InputActionRecord>& actions, std::int32_t action,
    std::int32_t slot, const InputBindingInstallSource& source, float scale,
    const InputBindingDeviceGroups& groups) {
    if (action < 0 || static_cast<std::size_t>(action) >= actions.size()) {
        throw std::out_of_range("input action index");
    }
    if (slot < 0) throw std::out_of_range("input binding slot");
    if (slot == std::numeric_limits<std::int32_t>::max()) {
        throw std::length_error("input binding slot count overflows native int32");
    }

    auto& bindings = actions[static_cast<std::size_t>(action)].bindings;
    if (bindings.size() <= static_cast<std::size_t>(slot)) {
        resize_input_action_bindings_00a93500(bindings, slot + 1);
    }
    auto& binding = bindings[static_cast<std::size_t>(slot)];
    binding.device_class = source.device_class;
    binding.device_index = source.device_index;
    binding.cached_device = source.cached_device;
    binding.input_code = source.input_code;
    binding.response_curve = source.device_class == 2;
    binding.force_unit_scale = (source.flag_word & 0xffu) != 0;
    binding.scale = scale;
    rebind_input_action_00a91e80(bindings, groups);
}

} // namespace bsp
