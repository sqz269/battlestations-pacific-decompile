#pragma once
#include <string_view>
#include <vector>

#include "bsp/frontend_prompts.hpp"

// Descriptive names are hypotheses. Evidence, native ABI and host limits:
// docs/FRONTEND_PROMPT_LAYOUT.md. These C++ entry points are not binary hooks.
namespace bsp {

struct FrontEndPromptLayoutHost {
    virtual ~FrontEndPromptLayoutHost() = default;
    virtual float measured_button_width_114(PromptWidget button) = 0;
    virtual float group_width_00aa6740() = 0; // screen+38h, size.x
    virtual void set_button_local_x_00aa78d0(PromptWidget button, float x) = 0;
};

// ECX=screen, RET. Ghidra stores 00530A60..00530C17 as a shared tail in
// 00532360, whose main body jumps here. Re-reads flags and widths across calls.
void layout_prompt_buttons_00530a60(const FrontEndPromptScreen& screen,
    FrontEndPromptLayoutHost& host);

// Actual GUI objects supplied by the owner; each button must provide writable
// native fields through +1BCh. The surrounding screen remains a host projection.
// The constructor/register layer supplies root/background/group/listener.
struct FrontEndPromptWidgets {
    void* root_14{};
    void* background_18{};
    void* title_1c{};
    void* message_20{};
    void* navigation_24{};
    void* yes_28{};
    void* no_2c{};
    void* accept_30{};
    void* restart_34{};
    void* group_38{};
    void* listener_10{};
    void* button(PromptWidget which) const noexcept;
};

// Actual object reads stay in the adapter; these two concrete GUI methods
// still require the engine implementation (including layout notifications).
struct FrontEndPromptLayoutNativeCalls {
    virtual ~FrontEndPromptLayoutNativeCalls() = default;
    virtual float widget_width_00aa6740(void* widget) = 0;
    virtual void widget_set_local_x_00aa78d0(void* widget, float x) = 0;
};
class ActualFrontEndPromptLayoutHost final : public FrontEndPromptLayoutHost {
public:
    ActualFrontEndPromptLayoutHost(FrontEndPromptWidgets& widgets,
        FrontEndPromptLayoutNativeCalls& calls) : widgets_(widgets), calls_(calls) {}
    float measured_button_width_114(PromptWidget button) override;
    float group_width_00aa6740() override;
    void set_button_local_x_00aa78d0(PromptWidget button, float x) override;
private:
    FrontEndPromptWidgets& widgets_;
    FrontEndPromptLayoutNativeCalls& calls_;
};

struct FrontEndPromptButtonHost {
    virtual ~FrontEndPromptButtonHost() = default;
    virtual NativeStringStorage& strings() = 0;
    virtual void set_localized_button_00abaed0(PromptWidget button,
        const NativeString& source, bool flag) = 0;
};
// ECX=screen, stack widget, RET4. scratch projects the vector at screen+268h
// (begin +26Ch, end +270h). It owns its NativeStrings. The callback may mutate
// scratch: the last CURRENT item is destroyed after the localized-text call.
void prepare_prompt_button_00532110(std::vector<NativeString>& scratch,
    PromptWidget button, FrontEndPromptButtonHost& host);

struct FrontEndPromptEnterHost {
    virtual ~FrontEndPromptEnterHost() = default;
    virtual NativeStringStorage& strings() = 0;
    virtual void* find_child_00aa7e00(void* root, const NativeString& name,
        bool recursive) = 0;
    virtual void widget_flag(void* widget, int vtable_slot, bool value) = 0;
    virtual bool mission_present_00e198c4() = 0;
    virtual FrontEndScreen* previous_screen_00e1930c() = 0;
    virtual void previous_screen_exit_v1c(FrontEndScreen& screen) = 0;
    virtual void previous_screen_commit_004f83b0(FrontEndScreen& screen) = 0;
    // Null iff the owner at 00E198B4 is null. Otherwise its +54h screen must
    // exist; native does not guard that inner pointer before reading +5h.
    virtual FrontEndScreen* overlay_screen_00e198b4_54() = 0;
    virtual void overlay_input_v0c(FrontEndScreen& screen) = 0; // ECX=screen+8h
    virtual bool alternate_input_00f88a30() = 0;
    virtual void gui_reset_screens_00aa0f70() = 0; // 004C12B0 manager first
    virtual void button_callback_00531130(void* button, std::u16string_view text,
        void* listener, void* event) = 0;
    virtual void widget_color_v50(void* widget, const float (&rgba)[4]) = 0;
};
// Full entry control flow, ECX=screen, RET. Native temporary allocation and
// exception cleanup are projected; direct button writes use actual offsets.
void enter_prompt_screen_00531380(FrontEndPromptScreen& screen,
    FrontEndPromptWidgets& widgets, FrontEndPromptEnterHost& host);
}
