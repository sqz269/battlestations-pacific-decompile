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

// Two required native calls, in order: lazy 004BEC00 singleton getter, then
// 00A92C40 update of exactly that returned raw owner with +0.0f. No native
// C++ owner type is established; the borrowed handle keeps their identity.
// A frontend bridge must bind its current input-action publication and tick.
struct TextEditorActivationInput {
    virtual ~TextEditorActivationInput() = default;
    virtual void* get_input_action_owner_004bec00() = 0;
    virtual void update_input_action_owner_00a92c40(void* actual_owner,
        float seconds) = 0;
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
