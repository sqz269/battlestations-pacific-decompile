#include "bsp/ingame_interface.hpp"

// Packet in_mission_interface_manager. Evidence: docs/IN_MISSION_INTERFACE_MANAGER.md.
namespace bsp {
namespace {

// 00E08CD8 entries 20h..35h, read back from the image.
const char* const kInGameInterfaceNames[] = {
    "INTF_SCENE3D",             // 20h
    "INTF_MAP",                 // 21h
    "INTF_PLANE",               // 22h
    "INTF_PLANEBOMBER",         // 23h
    "INTF_PLANESPAWN",          // 24h
    "INTF_CAPTAIN",             // 25h
    "INTF_BOMBVIEW",            // 26h
    "INTF_TBOATHELMSMAN",       // 27h
    "INTF_SUBMARINE",           // 28h
    "INTF_FREECAMERA",          // 29h
    "INTF_IDLECAMERA",          // 2Ah
    "INTF_MOVIECAMERA",         // 2Bh
    "INTF_MOVIECAMERANEW",      // 2Ch
    "INTF_ENGINEMOVIECAMERA",   // 2Dh
    "INTF_AIRFIELD",            // 2Eh
    "INTF_COMMANDBUILDING",     // 2Fh
    "INTF_SHIPYARD_STAREDUMB",  // 30h
    "INTF_SHIPYARD",            // 31h
    "INTF_AIRBASE",             // 32h
    "INTF_LAUNCHLANDING",       // 33h
    "INTF_LIMBO",               // 34h
    "INTF_SUPPORTMANAGER",      // 35h
};

// Level-1 screen-set lists, in argument order (the reverse of the push order).
const int kSetScene3d[] = {0x29, 0x49, 0x44, 0x35};                                    // 0068AF40
const int kSetPlane[] = {0x29, 0x49, 0x44, 0x27, 0x4D, 0x3E, 0x3F, 0x35, 0x50};        // 0068B008
const int kSetPlaneBomber[] = {0x29, 0x49, 0x44, 0x27, 0x4D, 0x3E, 0x25, 0x26, 0x2E,
    0x35, 0x50};                                                                       // 0068AFB8
const int kSetPlaneSpawn[] = {0x44, 0x27, 0x4D, 0x50, 0x41, 0x35};                     // 0068B055
const int kSetCaptain[] = {0x29, 0x49, 0x44, 0x27, 0x4D, 0x45, 0x46, 0x26, 0x2E,
    0x35, 0x50};                                                                       // 0068B115
const int kSetBombView[] = {0x27, 0x4D, 0x24};                                         // 0068B168
const int kSetTBoatHelmsman[] = {0x29, 0x49, 0x44, 0x27, 0x4D, 0x45, 0x4A, 0x26, 0x2E,
    0x35, 0x50};                                                                       // 0068B093
const int kSetSubmarine[] = {0x29, 0x49, 0x44, 0x27, 0x4D, 0x45, 0x47, 0x48, 0x2E,
    0x35, 0x50};                                                                       // 0068B0D4
const int kSetFreeCamera[] = {0x2B};                                                   // 0068AD5B
const int kSetIdleCamera[] = {0x2C, 0x29, 0x49};                                       // 0068AD7C
const int kSetMovieCamera[] = {0x36};                                                  // 0068ADA9
const int kSetMovieCameraNew[] = {0x37};                                               // 0068ADC9
const int kSetEngineMovieCamera[] = {0x38};                                            // 0068ADEA
const int kSetCommandBuilding[] = {0x29, 0x49, 0x44, 0x27, 0x4D, 0x2F, 0x26, 0x2E,
    0x35, 0x50};                                                                       // 0068B1A1
const int kSetLaunchLanding[] = {0x29, 0x49, 0x44, 0x27, 0x4D, 0x35};                  // 0068B1E7
const int kSetLimbo[] = {0x32};                                                        // 0068B212

// Level-1 input-context lists, in argument order after the game pointer.
const int kCtxDefaultHud[] = {4, 0x11, 0x12, 0x0C, 0x0B};   // 0068AF53, 0068AD8F
const int kCtxPlane[] = {0x0A, 4, 0x11, 0x12, 0x0C, 0x0B};  // 0068B027
const int kCtxPlaneBomber[] = {9, 4, 0x11, 0x12, 0x0C, 0x0B}; // 0068AFDA
const int kCtxPlaneSpawn[] = {0x17, 4, 0x11, 0x12, 0x0C, 0x0B}; // 0068B06E
const int kCtxShip[] = {6, 4, 0x11, 0x12, 0x0C, 0x0B};      // 00689FA0 and 00689FE0
const int kCtxSubmarine[] = {7, 4, 0x11, 0x12, 0x0C, 0x0B}; // 00689FC0
const int kCtxBombView[] = {5, 4, 0x0A};                    // 0068B17A
const int kCtxFreeCamera[] = {2};                           // 0068AD6A
const int kCtxMovie[] = {3};                                // 0068ADB2, 0068ADD8, 0068ADF9
const int kCtxLaunchLanding[] = {0x16, 0x11, 0x12};         // 0068B1FF
const int kCtxLimbo[] = {0x13};                             // 0068B21B

template <std::size_t N>
constexpr std::size_t count_of(const int (&)[N]) noexcept {
    return N;
}

InGameInterfaceScreenSet make_set(const int* screens, std::size_t screen_count,
    const int* contexts, std::size_t context_count) noexcept {
    InGameInterfaceScreenSet set{};
    set.screen_ids = screens;
    set.screen_count = screen_count;
    set.input_contexts = contexts;
    set.input_context_count = context_count;
    set.sets_screen_set = true;
    set.sets_input_contexts = contexts != nullptr;
    return set;
}

// The manager offset each arm hands the unit to, after the two level setters.
// 0 means the arm has no such call. The bomb-view arm is not in this table
// because its two calls straddle the level setters.
struct ArmScreenHandoff {
    std::uint16_t first;
    std::uint16_t second;
};

ArmScreenHandoff arm_handoff(int interface_id) noexcept {
    switch (interface_id) {
    case kInterfacePlane: return {0x6C, 0x68};        // 0068B03B, 0068B047
    case kInterfacePlaneBomber: return {0x70, 0x68};  // 0068AFEE, 0068AFFA
    case kInterfacePlaneSpawn: return {0x74, 0};      // 0068B082
    case kInterfaceCaptain: return {0x7C, 0x78};      // 0068B13C, 0068B145
    case kInterfaceTBoatHelmsman: return {0x80, 0x78}; // 0068B0BA, 0068B0C6
    case kInterfaceSubmarine: return {0x84, 0x78};    // 0068B0FB, 0068B107
    case kInterfaceCommandBuilding: return {0xB0, 0}; // 0068B1D6
    default: return {0, 0};
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// The 42 HUD screens, 0068CC70 in construction order
// ---------------------------------------------------------------------------

const InGameHudScreenSlot kInGameHudScreens[kInGameHudScreenCount] = {
    {0x40, 0x44, 0x0068BF60u, 0x00CF5A9Cu, 0x174, 0},
    {0x44, 0x27, 0x0068A8A0u, 0x00CF7A38u, 0x034, 1},
    {0x48, 0x4D, 0x00640C80u, 0x00CF5950u, 0x0B0, 2},
    {0x4C, 0x26, 0x0051E950u, 0x00CEC9B0u, 0x050, 3},
    {0x50, 0x2E, 0x005472F0u, 0x00CEDF34u, 0x114, 4},
    {0x54, 0x4C, 0x005AAEF0u, 0x00CF0134u, 0x614, 5},
    {0x58, 0x35, 0x005C0810u, 0x00CF1404u, 0x1B0, 6},
    {0x5C, 0x2A, 0x00538E30u, 0x00CED6FCu, 0x1AC, 7},
    {0x60, 0x39, 0x005EE440u, 0x00CF2D20u, 0x074, 8},
    {0x64, 0x3B, 0x005F9440u, 0x00CF3B7Cu, 0x090, 9},
    {0x68, 0x3E, 0x00608E60u, 0x00CF4544u, 0x194, 10},
    {0x6C, 0x3F, 0x00606470u, 0x00CF43ACu, 0x110, 11},
    {0x70, 0x25, 0x00000000u, 0x00CEC590u, 0x030, 12},
    {0x74, 0x41, 0x0060CAE0u, 0x00CF4A7Cu, 0x02C, 13},
    {0x78, 0x45, 0x0064B780u, 0x00CF5E30u, 0x1B0, 14},
    {0x7C, 0x46, 0x00000000u, 0x00CF7978u, 0x038, 15},
    {0x80, 0x4A, 0x00000000u, 0x00CF79F8u, 0x030, 16},
    {0x84, 0x47, 0x00650B00u, 0x00CF639Cu, 0x07C, 17},
    {0x88, 0x48, 0x00650140u, 0x00CF61D4u, 0x024, 18},
    {0x8C, 0x24, 0x00000000u, 0x00CF7938u, 0x058, 19},
    {0x90, 0x2B, 0x00540BA0u, 0x00CEDCA8u, 0x090, 20},
    {0x94, 0x2C, 0x0054F8C0u, 0x00CEE408u, 0x028, 21},
    {0x98, 0x36, 0x005CE610u, 0x00CF1968u, 0x084, 22},
    {0x9C, 0x37, 0x005CC120u, 0x00CF17A8u, 0x038, 23},
    {0xA0, 0x38, 0x005CB0A0u, 0x00CF17D0u, 0x020, 24},
    {0xA4, 0x33, 0x005BA7B0u, 0x00CF0ED8u, 0x0E8, 25},
    {0xA8, 0x34, 0x0054D680u, 0x00CEE290u, 0x07C, 26},
    {0xAC, 0x23, 0x005178B0u, 0x00CEC194u, 0x02C, 27},
    {0xB0, 0x2F, 0x00000000u, 0x00CF79B8u, 0x024, 28},
    {0xB4, 0x49, 0x0067BE30u, 0x00CF6D68u, 0x00C, 29},
    {0xCC, 0x29, 0x00525A90u, 0x00CECCF8u, 0x0F0, 30},
    {0xD4, 0x5A, 0x00636D90u, 0x00CF569Cu, 0x034, 31},
    {0xD8, 0x3C, 0x005FFCB0u, 0x00CF3EF4u, 0x164, 32},
    {0xDC, 0x3D, 0x00604780u, 0x00CF41A0u, 0x00C, 33},
    {0xE0, 0x5E, 0x0060FD90u, 0x00CF4B98u, 0x048, 34},
    {0xE4, 0x19, 0x005D3540u, 0x00CF1D70u, 0x028, 35},
    {0xE8, 0x32, 0x00565D40u, 0x00CEEBACu, 0x01C, 36},
    {0xB8, 0x43, 0x00643DA0u, 0x00CF5998u, 0x018, 37},
    {0xBC, 0x4E, 0x006735D0u, 0x00CF6B44u, 0x480, 38},
    {0xC0, 0x4F, 0x0061A480u, 0x00CF4DECu, 0x270, 39},
    {0xC4, 0x50, 0x006821C0u, 0x00CF7184u, 0x07C, 40},
    {0xC8, 0x51, 0x0052B030u, 0x00CED0ACu, 0x02C, 41},
};

std::size_t in_game_hud_screen_index_at(std::uint16_t offset) noexcept {
    for (std::size_t i = 0; i < kInGameHudScreenCount; ++i) {
        if (kInGameHudScreens[i].offset == offset) {
            return i;
        }
    }
    return kInGameHudScreenCount;
}

std::size_t in_game_hud_screen_index_for_slot(int registry_slot) noexcept {
    for (std::size_t i = 0; i < kInGameHudScreenCount; ++i) {
        if (static_cast<int>(kInGameHudScreens[i].registry_slot) == registry_slot) {
            return i;
        }
    }
    return kInGameHudScreenCount;
}

const char* in_game_interface_name(int interface_id) noexcept {
    if (interface_id < kFirstInGameInterface || interface_id > kLastInGameInterface) {
        return nullptr;
    }
    return kInGameInterfaceNames[static_cast<std::size_t>(interface_id - kFirstInGameInterface)];
}

bool in_game_interface_is_camera_mode(int interface_id) noexcept {
    // 0068AD20..0068AD34.
    return interface_id == kInterfaceFreeCamera || interface_id == kInterfaceMovieCamera
        || interface_id == kMovieCameraNewInterface || interface_id == kInterfaceEngineMovie;
}

bool in_game_interface_suppresses_ambience(int interface_id) noexcept {
    // 0068B255..0068B27C: the same four ids plus INTF_LIMBO.
    return in_game_interface_is_camera_mode(interface_id) || interface_id == kInterfaceLimbo;
}

InGameInterfaceScreenSet in_game_interface_screen_set(int interface_id) noexcept {
    switch (interface_id) {
    case kInterfaceScene3d:
        // 0068AF40, the null-payload single-player arm. With a payload the 20h
        // case never reaches a setter; it redispatches instead.
        return make_set(kSetScene3d, count_of(kSetScene3d), kCtxDefaultHud,
            count_of(kCtxDefaultHud));
    case kInterfacePlane:
        return make_set(kSetPlane, count_of(kSetPlane), kCtxPlane, count_of(kCtxPlane));
    case kInterfacePlaneBomber:
        return make_set(kSetPlaneBomber, count_of(kSetPlaneBomber), kCtxPlaneBomber,
            count_of(kCtxPlaneBomber));
    case kInterfacePlaneSpawn:
        return make_set(kSetPlaneSpawn, count_of(kSetPlaneSpawn), kCtxPlaneSpawn,
            count_of(kCtxPlaneSpawn));
    case kInterfaceCaptain:
        return make_set(kSetCaptain, count_of(kSetCaptain), kCtxShip, count_of(kCtxShip));
    case kInterfaceBombView:
        return make_set(kSetBombView, count_of(kSetBombView), kCtxBombView,
            count_of(kCtxBombView));
    case kInterfaceTBoatHelmsman:
        return make_set(kSetTBoatHelmsman, count_of(kSetTBoatHelmsman), kCtxShip,
            count_of(kCtxShip));
    case kInterfaceSubmarine:
        return make_set(kSetSubmarine, count_of(kSetSubmarine), kCtxSubmarine,
            count_of(kCtxSubmarine));
    case kInterfaceFreeCamera:
        return make_set(kSetFreeCamera, count_of(kSetFreeCamera), kCtxFreeCamera,
            count_of(kCtxFreeCamera));
    case kInterfaceIdleCamera:
        return make_set(kSetIdleCamera, count_of(kSetIdleCamera), kCtxDefaultHud,
            count_of(kCtxDefaultHud));
    case kInterfaceMovieCamera:
        return make_set(kSetMovieCamera, count_of(kSetMovieCamera), kCtxMovie,
            count_of(kCtxMovie));
    case kMovieCameraNewInterface:
        return make_set(kSetMovieCameraNew, count_of(kSetMovieCameraNew), kCtxMovie,
            count_of(kCtxMovie));
    case kInterfaceEngineMovie:
        return make_set(kSetEngineMovieCamera, count_of(kSetEngineMovieCamera), kCtxMovie,
            count_of(kCtxMovie));
    case kInterfaceCommandBuilding:
        return make_set(kSetCommandBuilding, count_of(kSetCommandBuilding), kCtxShip,
            count_of(kCtxShip));
    case kInterfaceLaunchLanding:
        return make_set(kSetLaunchLanding, count_of(kSetLaunchLanding), kCtxLaunchLanding,
            count_of(kCtxLaunchLanding));
    case kInterfaceLimbo:
        return make_set(kSetLimbo, count_of(kSetLimbo), kCtxLimbo, count_of(kCtxLimbo));
    case kInterfaceMap:
        // 21h shares the default arm 0068B230: clear level 1, leave the input
        // contexts alone.
        return make_set(nullptr, 0, nullptr, 0);
    case kInterfaceAirfield:
    case kInterfaceShipyardStareDumb:
    case kInterfaceShipyard:
    case kInterfaceAirbase:
    case kInterfaceSupportManager:
        // 2Eh, 30h, 31h, 32h and 35h jump straight to the tail at 0068B23A and
        // touch neither level.
        return InGameInterfaceScreenSet{};
    default:
        // Out of range: the ja at 0068AD4E takes the same default arm as 21h.
        return make_set(nullptr, 0, nullptr, 0);
    }
}

InGameSceneInterfaceChoice choose_scene_interface_0068ae0b(bool has_unit, bool multiplayer,
    InGameInterfaceUnitQuery& query) noexcept {
    InGameSceneInterfaceChoice choice{};
    if (!has_unit) {
        // 0068AF2F. game+1FE4h selects the arm.
        if (multiplayer) {
            choice.interface_id = kInterfaceIdleCamera; // 0068AFAB
            choice.chosen = true;
        }
        return choice;
    }
    if (query.unit_is_kind_of(kUnitTypeTorpedoBoat)) {
        choice.interface_id = kInterfaceTBoatHelmsman;
        choice.chosen = true;
    } else if (query.unit_is_kind_of(kUnitTypeSubmarine)) {
        choice.interface_id = kInterfaceSubmarine;
        choice.chosen = true;
    } else if (query.unit_is_kind_of(kUnitTypeShip)) {
        choice.interface_id = kInterfaceCaptain;
        choice.chosen = true;
    } else if (query.unit_is_kind_of(kUnitTypePlane)) {
        choice.interface_id = query.plane_is_in_flight() ? kInterfacePlane : kInterfacePlaneSpawn;
        choice.chosen = true;
    } else if (query.unit_is_kind_of(kUnitTypeAirfield)) {
        choice.interface_id = kInterfaceAirfield;
        choice.chosen = true;
    } else if (query.unit_is_kind_of(kUnitTypeCommandBuilding)) {
        choice.interface_id = kInterfaceCommandBuilding;
        choice.chosen = true;
    } else if (query.unit_is_kind_of(kUnitTypeShipyard)) {
        choice.interface_id = kInterfaceShipyardStareDumb;
        choice.chosen = true;
    } else if (query.unit_is_kind_of(kUnitTypeCarrierGroupHost) && query.has_delegate_unit()) {
        // 0068AF18: the same id, but with the unit at +3D0h.
        choice.interface_id = kInterfaceScene3d;
        choice.chosen = true;
        choice.redispatch_with_delegate = true;
    }
    return choice;
}

void collapse_in_game_overlays_0068ab80(InGameInterfaceCollapseState& state, bool clear_level2,
    bool run_extra_hook) noexcept {
    if (state.level3_non_empty) {
        if (state.camera_screen_flag_30) {
            state.camera_screen_flag_0a = true; // 0068ABA6
        }
        state.level3_cleared = true; // 004F8670(0) and 004D8B70(game, 0)
    }
    if (run_extra_hook) {
        state.extra_hook_ran = true; // 0051E8E0
    }
    if (clear_level2 && state.screen_4c_wanted) {
        state.level2_cleared = true; // 004F85D0(0) and 004D8AE0(game, 0)
    }
}

bool apply_in_game_interface_0068aca0(InGameInterfaceManager& manager, InGameInterfaceHost& host,
    int interface_id, bool has_payload) noexcept {
    // 0068ACC7. The base writes both records and consults the lock 00E19894
    // through 00683E90; a rejected request never reaches the HUD.
    if (!host.apply_base_request(interface_id, has_payload)) {
        return false;
    }

    int id = interface_id;
    if (has_payload && host.payload_unit_is_dead()) {
        // 0068ACE8: the unit is gone, so the HUD is forced to limbo whatever the
        // caller asked for. The base record keeps the requested id.
        host.limbo_screen_take_unit();
        id = kInterfaceLimbo;
    }

    host.hud_root_screen_set_interface(id, has_payload); // 0068AD07
    if (host.mission_overlay_present()) {
        host.tick_mission_overlay(); // 0068AD1B
    }
    host.collapse_overlays(in_game_interface_is_camera_mode(id), true); // 0068AD43

    if (id == kInterfaceScene3d) {
        const InGameSceneInterfaceChoice choice =
            choose_scene_interface_0068ae0b(has_payload, host.is_multiplayer(), host.unit_query());
        if (!has_payload) {
            // 0068AF2F. Multiplayer redispatches 2Ah with the same null payload;
            // single player installs the bare scene set here.
            if (choice.chosen) {
                host.redispatch(choice.interface_id, false); // 0068AFAB
            } else {
                const InGameInterfaceScreenSet set =
                    in_game_interface_screen_set(kInterfaceScene3d);
                host.set_level1_screen_set(set.screen_ids, set.screen_count);
                host.set_level1_input_contexts(set.input_contexts, set.input_context_count);
            }
            // Both arms end at 0068AF68, which only stops the ambience.
            if (manager.ambient_sound_instance != 0) {
                host.stop_ambient_sound();
                manager.ambient_sound_instance = 0;
            }
            return true;
        }
        if (choice.chosen) {
            // The delegate case hands on the unit at +3D0h; both are non-null.
            host.redispatch(choice.interface_id, true);
        }
        // Every 20h-with-unit arm, matched or not, falls into the shared tail.
    } else if (id == kInterfaceBombView) {
        // 0068B153. The query straddles the two setters and its second result is
        // stored back on the bomb-view screen.
        host.bomb_view_screen_bind(host.hud_root_screen_query());
        const InGameInterfaceScreenSet set = in_game_interface_screen_set(id);
        host.set_level1_screen_set(set.screen_ids, set.screen_count);
        host.set_level1_input_contexts(set.input_contexts, set.input_context_count);
        host.bomb_view_screen_store(host.hud_root_screen_query());
    } else {
        const InGameInterfaceScreenSet set = in_game_interface_screen_set(id);
        if (set.sets_screen_set) {
            host.set_level1_screen_set(set.screen_ids, set.screen_count);
        }
        if (set.sets_input_contexts) {
            host.set_level1_input_contexts(set.input_contexts, set.input_context_count);
        }
        const ArmScreenHandoff handoff = arm_handoff(id);
        if (handoff.first != 0) {
            host.screen_receive_unit(handoff.first);
        }
        if (handoff.second != 0) {
            host.screen_receive_unit(handoff.second);
        }
    }

    // 0068B23A, the shared tail: the unit ambience.
    if (!has_payload || !host.unit_query().unit_is_kind_of(kUnitTypeShip)
        || in_game_interface_suppresses_ambience(id)) {
        if (manager.ambient_sound_instance != 0) {
            host.stop_ambient_sound();
            manager.ambient_sound_instance = 0;
        }
        return true;
    }

    const std::uint32_t source = host.unit_ambient_sound_source();
    if (manager.ambient_sound_instance != 0 && manager.ambient_sound_source == source) {
        return true; // 0068B2AD, already playing the right one
    }
    if (manager.ambient_sound_instance != 0) {
        host.stop_ambient_sound();
        manager.ambient_sound_instance = 0;
    }
    manager.ambient_sound_source = source; // 004E7BB0, a refcounted assign
    if (source != 0) {
        manager.ambient_sound_instance = host.start_ambient_sound(source);
    }
    return true;
}

}  // namespace bsp
