#pragma once
#include <cstdint>

#include "bsp/native_string.hpp"
#include "bsp/text_input.hpp"

namespace bsp {

// Borrowed fields of one existing text owner. Native 00A97260 constructs the
// owner at screen+0Ch; these references must all belong to that same owner.
// This view does not construct a callback table or own editable storage.
struct TextEditorActivationOwner {
    TextInputCallbacks& callbacks; // its enabled flag projects owner+04h
    NativeString& editable_18;
    std::uint32_t& cursor_20;
    void*& selected_row_24;
};

// Required concrete input-action refresh. Native 00A966E0 calls 004BEC00
// followed by 00A92C40 with +0.0f. A frontend bridge must use its current
// GameInputActions owner; an empty implementation changes observable state.
struct TextEditorActivationInput {
    virtual ~TextEditorActivationInput() = default;
    virtual void refresh_current_actions_zero() = 0;
};

struct TextEditorActivationBindings {
    TextEditorActivationOwner owner;
    PlatformTextInput& platform;
    NativeStringStorage& strings;
    TextEditorActivationInput& input;
};

// Native ECX owner, stack enabled byte and selected-row/context DWORD, RET8.
// Typed projection of 00A966E0. Enabling clears editable text/cursor; both
// enable and disable clear the same platform queue via 00A965A0.
void set_text_editor_active_00a966e0(TextEditorActivationBindings& bindings,
    bool enabled, void* selected_row);

} // namespace bsp
