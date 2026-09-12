#pragma once

namespace bsp {
struct FrontEndScreen;
struct FrontEndScreenTable;
struct GameSettings;
struct MainMenuMapGeometryState;
struct MainMenuSelectionListenerBindings;
class SettingsApplyHost;

// These methods resolve/dispatch the SAME live screen and renderer identities.
// Current00 is invoked by base004F71D0; current14 must be freshly dispatched
// after its return and registry publication, since current00 can change it.
// No cached table, replacement screen, successful default, or new renderer.
struct MainMenuRegistrationServices {
    virtual ~MainMenuRegistrationServices() = default;
    virtual int screen_current00(FrontEndScreen&) = 0;
    virtual void screen_current14(FrontEndScreen&, MainMenuSelectionListenerBindings&) = 0;
    // Read-only identity resolution for the actual publication [00F8D394].
    // Resolve only when reached, AFTER the five live settings reads. The
    // existing host must execute full00B29E60 on this receiver or throw at
    // that unresolved boundary; parameter writes alone are not completion.
    virtual SettingsApplyHost& renderer_00f8d394() = 0;
};

// References only. Base, selection, its widget.screen/layout, map geometry,
// and the two residual offset cells must all belong to one actual screen.
// +190..198 aliases existing map_geometry.base_offset. +1A8/+564/+565 alias
// selection.command.widget.screen; +64/+68/+110 alias selection. +2C8/+2EC
// are the existing layout cells. No field is copied into a second screen.
struct MainMenuRegistrationBindings {
    MainMenuSelectionListenerBindings& selection;
    FrontEndScreen& base;
    FrontEndScreenTable& registry_00e18b60;
    MainMenuMapGeometryState& map_geometry;
    float& map_offset_x_1a4;
    float& map_offset_z_1ac;
    const volatile float& base_y_00cef7bc;
    // The same global00CE54A0 is already command.zoom_in_00ce54a0.
    const volatile GameSettings& settings_00f88980;
    MainMenuRegistrationServices& services;
};

// Complete ordinary00582F30 caller, including existing spacing fragment.
// Native ECX=screen, no stack arguments, bare RET00583091 (length1).
// New C++ ABI; supported ids0..94, live canonical GUI/Text and required
// renderer service. An unfinished callback throws at the reached operation
// and retains prior native stores. This does not make registration resumable.
void register_main_menu_screen_00582f30(MainMenuRegistrationBindings&);
} // namespace bsp
