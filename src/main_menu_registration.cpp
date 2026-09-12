#include "bsp/main_menu_registration.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/game_settings.hpp"
#include "bsp/main_menu_map_point_geometry.hpp"
#include "bsp/main_menu_selection_listener.hpp"
#include "bsp/main_menu_spacing.hpp"
#include <cstdint>
#include <stdexcept>

namespace bsp {
namespace {
//00582FF3/00583015 are MOVSS loads, not x87 arithmetic. Carry their raw bits
// through the intervening stores, including NaN payloads and signed zero.
std::uint32_t load_float_bits(const volatile float& source) noexcept {
    const volatile float* address = &source;
    std::uint32_t bits;
    __asm {
        mov eax, address
        movss xmm0, dword ptr [eax]
        movd bits, xmm0
    }
    return bits;
}
void store_float_bits(float& destination, std::uint32_t bits) noexcept {
    float* address = &destination;
    __asm {
        mov eax, address
        movd xmm0, bits
        movss dword ptr [eax], xmm0
    }
}
} // namespace

void register_main_menu_screen_00582f30(MainMenuRegistrationBindings& b) {
    //004F71D7 calls current00 before004F71D9 publishes the SAME base pointer.
    const int id = b.services.screen_current00(b.base);
    if (id < 0 || id >= kFrontEndScreenSlotCount)
        throw std::logic_error("582F30 registration requires a valid current00 screen id");
    register_front_end_screen_004f71d0(b.registry_00e18b60, b.base, id); //582F37
    b.services.screen_current14(b.base, b.selection); //582F43, fresh table

    auto& selection = b.selection;
    auto& command = selection.command;
    auto& widget = command.widget;
    auto& layout = widget.layout;
    auto* page = compute_main_menu_spacing_00582f45_fragment(layout, widget.owners);
    if (!page) throw std::logic_error("582F30 requires its captured authored +470 page");
    widget.owners.owner(*page).set_visible34(false); //582FF1, actual current34

    const auto base_x = load_float_bits(command.zoom_in_00ce54a0); //582FF3
    selection.field_64 = 0; //582FFB
    selection.field_68 = 0; //582FFE
    layout.objective_page_2c8 = nullptr; //583001
    layout.objective_background_2ec = nullptr; //583007
    store_float_bits(b.map_geometry.base_offset[0], base_x); //58300D
    const auto base_y = load_float_bits(b.base_y_00cef7bc); //583015
    store_float_bits(b.map_geometry.base_offset[1], base_y); //58301D
    store_float_bits(b.map_geometry.base_offset[2], 0); //583028, XORPS positive0
    store_float_bits(b.map_offset_x_1a4, 0); //583030
    store_float_bits(widget.screen.map_offset_y, 0); //583038
    store_float_bits(b.map_offset_z_1ac, 0); //583040
    selection.active_mission_group_110 = nullptr; //583048
    widget.screen.dlc_campaign = false; //58304E, byte565 first
    widget.screen.us_campaign = false; //583054, byte564 second

    // Original load order before the final receiver read: sync, samples,
    // fullscreen, height, width. Force=1 bypasses00B29E60's no-change arm.
    const bool vsync = b.settings_00f88980.vsync_60; //58305A
    const int samples = b.settings_00f88980.antialias_58; //583061
    const bool fullscreen = b.settings_00f88980.fullscreen_1e; //583067
    const int height = b.settings_00f88980.height_18; //583071
    const int width = b.settings_00f88980.width_14; //583078
    auto& renderer = b.services.renderer_00f8d394(); //583080, fresh publication
    renderer.renderer_change_presentation_mode(width, height, fullscreen,
        samples, vsync, true); //583087, RET18; returned AL ignored
}
} // namespace bsp
