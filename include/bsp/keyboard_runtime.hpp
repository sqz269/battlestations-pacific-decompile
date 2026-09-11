#pragma once
#include "bsp/input_binding_install.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/keyboard_restore.hpp"

namespace bsp {
// Maps a retained native pointer word to a live reconstructed device. Nonzero
// words require this explicit translation; they are never cast to host pointers.
struct KeyboardDevicePointerHost {
    virtual ~KeyboardDevicePointerHost() = default;
    virtual InputDevice* resolve_cached_device(std::uint32_t native_word) = 0;
};

// Composition of 006aa090, 00a92260 and 00a93750 for the existing 006aa640
// settings application. The actions, device groups and pointer host outlive it.
// Device classes satisfy rebind_input_action_00a91e80's group-index contract.
class KeyboardActionRuntime final : public KeyboardRuntimeHost {
public:
    KeyboardActionRuntime(std::vector<InputActionRecord>&,
        const InputBindingDeviceGroups&, KeyboardDevicePointerHost&) noexcept;
    bool action_registered_00a92260(std::uint32_t) override;
    bool use_alternate_axis_slots_006aa090(InputSettings&,
        const std::string& device, const std::string& input) override;
    void bind_action_00a93750(std::int32_t action, std::int32_t slot,
        const KeyboardInputBinding&, float scale) override;
private:
    std::vector<InputActionRecord>& actions_;
    const InputBindingDeviceGroups& groups_;
    KeyboardDevicePointerHost& pointers_;
};
}
