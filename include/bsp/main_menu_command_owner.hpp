#pragma once

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/main_menu_command_bar.hpp"

namespace bsp {

// Semantic owner of the native 0xAC-byte screen published at 00E1930C.
// Constructor0054A8D0 installs vtable00CEDFDC; binder0054C050 owns two pages.
// This is a new C++ interface, not the native layout, callback ABI or allocator.
// Addresses, coverage and uncertainties: docs/MAIN_MENU_COMMAND_OWNER.md.
struct MainMenuCommandWidgetBinding {
    std::uint16_t field_offset;
    std::uint16_t page_offset;
    std::string_view name;
    std::uint32_t call_site;
};

const std::array<MainMenuCommandWidgetBinding, 22>&
main_menu_command_widget_bindings() noexcept;

struct MainMenuCommandOwner {
    MainMenuCommandBarState bar{}; // Reuses +04/+05/+20 and +5C label cache.
    GuiLayoutPage* help_page_24{};
    GuiLayoutPage* button_page_28{};
    // Borrowed handles in binding-table order, owned by the pages above.
    std::array<GuiLayoutWidget*, 22> widgets{};

    // Bridge for MainMenuCommandBarHost's existing field-offset interface.
    // +24/+28 resolve page roots; unknown offsets return null.
    GuiLayoutWidget* widget_at(std::uint16_t field_offset) const noexcept;
};

// ECX-only native leaf0054A960: EAX=58h, RET; screen registry identity.
std::int32_t main_menu_command_screen_id_0054a960() noexcept;
// Native virtual+04 only clears AL (upper EAX is unspecified).
bool main_menu_command_virtual04_0054a970() noexcept;
// Native virtual+1C is RET; virtual+20 is RET4. These are evidenced no-ops.
void exit_main_menu_command_owner_00549570() noexcept;
void update_main_menu_command_owner_00549580(float seconds) noexcept;

// 00549620: ECX screen, one stack dword, RET4. Low byte selects the help
// widgets, then is stored at +20 after both callbacks (including reentrancy).
void set_main_menu_help_variant_00549620(MainMenuCommandBarState& state,
    std::uint8_t variant, MainMenuCommandResetHost& host);

// The native +1C callback interface is represented by the owner reference;
// hosts must bind real callback storage, not cast this semantic object to it.
struct MainMenuCommandOwnerHost {
    virtual ~MainMenuCommandOwnerHost() = default;
    virtual void register_screen_004f71d0(MainMenuCommandOwner& owner) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    virtual GuiLayoutPage* load_page_00aa5840(void* manager,
        std::string_view name, std::uint8_t screen_flag, bool retain_existing) = 0;
    virtual GuiLayoutWidget* find_child_00aa7e00(GuiLayoutWidget& parent,
        std::string_view name, std::uint32_t unused) = 0;
    virtual void set_visible_virtual34(GuiLayoutWidget& widget, bool visible) = 0;
    // Host access to the existing Text companion; no native call at this step.
    virtual GuiTextWidget& text_state(GuiLayoutWidget& widget) = 0;
    virtual void set_text_cstring_00abbe50(GuiLayoutWidget& widget,
        std::string_view source, bool localize) = 0;
    virtual void set_text_source_00abaed0(GuiLayoutWidget& widget,
        std::string_view source, bool localize) = 0;
    virtual void set_text_color_virtual50(GuiLayoutWidget& widget,
        const GuiTextColor& color) = 0;
    virtual void configure_text_shadow_00ab6c30(GuiLayoutWidget& widget,
        std::uint8_t enabled, std::uint32_t position, float offset,
        const GuiTextColor& color) = 0;
    // 00531130 copies wide codes to widget+1A4, owner to+1AC, argument to+1B0.
    virtual void bind_text_codes_00531130(GuiLayoutWidget& widget,
        std::u16string_view codes, MainMenuCommandOwner& callback_owner,
        GuiLayoutWidget& callback_argument) = 0;
    // 00AA6BC0 stores callback owner at+DC and the byte at+79.
    virtual void set_widget_callback_00aa6bc0(GuiLayoutWidget& widget,
        MainMenuCommandOwner* callback_owner, std::uint8_t flag) = 0;
};

// Complete normal-path0054C050 (ECX owner, RET). Native pooled temporaries,
// SEH and allocation/missing-widget failure behavior are excluded. The supplied
// command host resolves its offsets through this owner and implements native
// GUI operations. New API reports missing required pages/widgets by exception.
void bind_main_menu_command_owner_0054c050(MainMenuCommandOwner& owner,
    const MainMenuCommandBarEnvironment& environment,
    MainMenuCommandOwnerHost& host, MainMenuCommandBarHost& command_host);

// Complete normal-path00549FC0 (screen virtual+18, ECX owner, RET).
// Reads style_61f from game00E188A8 at each native callback boundary; the host
// must keep this reference live. Noncanonical shadow-enable bytes are retained.
void enter_main_menu_command_owner_00549fc0(MainMenuCommandOwner& owner,
    const std::uint8_t& style_61f, MainMenuCommandOwnerHost& host);

// 0054B500: ECX owner, vector pointer stack, RET4. Append +24 then +28,
// retaining nulls and prior output entries as the native vector pushes do.
void collect_main_menu_command_pages_0054b500(const MainMenuCommandOwner& owner,
    std::vector<GuiLayoutPage*>& output);

struct MainMenuCommandOwnerLifetimeHost {
    virtual ~MainMenuCommandOwnerLifetimeHost() = default;
    virtual void* gui_manager_004c12b0() = 0;
    virtual void remove_and_destroy_page_00aa31f0(void* manager,
        GuiLayoutPage* page) = 0;
    virtual void destroy_callback_owner_00695870(MainMenuCommandOwner& owner) = 0;
    virtual void unregister_screen_004f71a0(MainMenuCommandOwner& owner) = 0;
};

// 0054A990 normal destructor body: destroy +24/+28 pages, destroy the five
// cached strings in reverse order, callback owner+08, screen base. Does not
// clear native pointer fields or free the enclosing allocation (0054ABF0).
void destroy_main_menu_command_owner_0054a990(MainMenuCommandOwner& owner,
    MainMenuCommandOwnerLifetimeHost& host);

}
