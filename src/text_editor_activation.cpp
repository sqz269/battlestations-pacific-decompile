#include "bsp/text_editor_activation.hpp"

#include <cstring>

namespace bsp {

void set_text_editor_active_00a966e0(TextEditorActivationBindings& bindings,
    bool enabled, void* selected_row) {
    auto& owner = bindings.owner;
    owner.callbacks.enabled = enabled;
    owner.selected_row_24 = selected_row;
    bindings.platform.enable_00a965a0(enabled);
    void* const input_owner = bindings.input.get_input_action_owner_004bec00();
    bindings.input.update_input_action_owner_00a92c40(input_owner, 0.0f);

    if (!enabled) return;
    owner.editable_18.resize_0041dd40(bindings.strings, 0, false);
    // The native branch reloads data/length after resize and calls memcpy only
    // if data is nonnull. A valid zero-length NativeString has null data; keep
    // the conditional schedule for a stale same-length header.
    if (auto* data = owner.editable_18.data())
        std::memcpy(data, "", owner.editable_18.length());
    owner.cursor_20 = 0;
}

} // namespace bsp
