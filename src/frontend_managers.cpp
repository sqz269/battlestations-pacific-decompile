#include "bsp/frontend_managers.hpp"

namespace bsp {
namespace {

// 00E08CD8, 45 entries. The strings live at 00CF7370..00CF76CD in descending
// order, which is why the table indices run backwards through .rdata.
constexpr const char* kInterfaceNames[kInterfaceNameCount] = {
    "INTF_NONE",                    // 00
    "INTF_MAINMENU",                // 01
    "INTF_MISSIONTREE",             // 02
    "INTF_BRIEFING",                // 03
    "INTF_REWARDS",                 // 04
    "INTF_UNITLIB",                 // 05
    "INTF_VIDEOLIB",                // 06
    "INTF_MEDALS",                  // 07
    "INTF_CREDITS",                 // 08
    "INTF_DOWNLOADEDCONTENT",       // 09
    "INTF_LEADERBOARDS",            // 0A
    "INTF_ACHIEVEMENTS",            // 0B
    "INTF_OPTIONS",                 // 0C
    "INTF_CONTROLLAYOUT",           // 0D
    "INTF_KEYBOARDSETUP",           // 0E
    "INTF_MULTIMAINMENU",           // 0F
    "INTF_MULTIMODESELECTOR",       // 10
    "INTF_MULTILIVERANKEDSELECTOR", // 11
    "INTF_MULTILAN",                // 12
    "INTF_MULTICREATELANSERVER",    // 13
    "INTF_MULTIGAMELOBBY",          // 14
    "INTF_MULTISELECTPLAYER",       // 15
    "INTF_MULTIPLAYERS",            // 16
    "INTF_MULTIINGAME",             // 17
    "INTF_MULTIFRIENDS",            // 18
    "INTF_MULTISIGNIN",             // 19
    "INTF_MULTISESSIONBROWSER",     // 1A
    "INTF_MULTIERROR",              // 1B
    "INTF_MULTICREATENEWACCOUNTPC", // 1C
    "INTF_MULTISESSIONFILTER",      // 1D
    "INTF_MULTITACTSHIT",           // 1E
    "INTF_MULTIPOINTSSHIT",         // 1F
    "INTF_SCENE3D",                 // 20
    "INTF_MAP",                     // 21
    "INTF_PLANE",                   // 22
    "INTF_PLANEBOMBER",             // 23
    "INTF_PLANESPAWN",              // 24
    "INTF_CAPTAIN",                 // 25
    "INTF_BOMBVIEW",                // 26
    "INTF_TBOATHELMSMAN",           // 27
    "INTF_SUBMARINE",               // 28
    "INTF_FREECAMERA",              // 29
    "INTF_IDLECAMERA",              // 2A
    "INTF_MOVIECAMERA",             // 2B
    "INTF_MOVIECAMERANEW",          // 2C
};

// 004BEF00 builds a counted handle from the raw payload on the stack and
// 0042BCA0 destroys it after the swap, so the two calls bracket the body of
// 004CC460 and 00684600 with a net-zero reference.
struct PayloadTemporary {
    PayloadTemporary(FrontEndPayloadHost& host, void* payload) : host_(host), payload_(payload) {
        host_.retain_payload(payload_);
    }
    ~PayloadTemporary() { host_.release_payload(payload_); }
    PayloadTemporary(const PayloadTemporary&) = delete;
    PayloadTemporary& operator=(const PayloadTemporary&) = delete;

    FrontEndPayloadHost& host_;
    void* payload_;
};

// The refcount swap 00684600 and 004CC460 both run over a record's payload:
// release the old handle, store the new one, retain it. The order matters
// because the original releases before it stores.
void swap_payload(void*& slot, void* payload, FrontEndPayloadHost& host) {
    if (slot == payload) {
        return;
    }
    if (slot != nullptr) {
        host.release_payload(slot);
    }
    slot = payload;
    if (payload != nullptr) {
        host.retain_payload(payload);
    }
}

// The destructor pattern shared by all three managers: one pass that hides
// every live screen, then one that destroys them. 006888E0 collects the
// pointers into a scratch vector first; the two passes are what matters.
void release_screens(void** screens, std::size_t count, FrontEndManagerHost& host) {
    for (std::size_t i = 0; i < count; ++i) {
        if (screens[i] != nullptr) {
            host.hide_screen(screens[i]);
        }
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (screens[i] != nullptr) {
            host.destroy_screen(screens[i]);
            screens[i] = nullptr;
        }
    }
}

}  // namespace

const char* front_end_interface_name(int interface_id) noexcept {
    if (interface_id < 0 || static_cast<std::size_t>(interface_id) >= kInterfaceNameCount) {
        return nullptr;
    }
    return kInterfaceNames[static_cast<std::size_t>(interface_id)];
}

bool front_end_request_rejected_00683e90(FrontEndInterfaceLock& lock, int interface_id) noexcept {
    // 00683E90..00683EB0. The lock is a gate on in-session interfaces only.
    if (!lock.engaged) {
        return false;
    }
    if (interface_id == kMovieCameraNewInterface) {
        return false;
    }
    if (interface_id >= kFirstInGameInterface) {
        return true;
    }
    // 00683EA7: the first front-end request lifts the lock and goes through.
    lock.engaged = false;
    return false;
}

void push_interface_request_004cc460(FrontEndManagerBase& manager, FrontEndInterfaceLock& lock,
    int interface_id, void* payload, FrontEndPayloadHost& host) {
    // 004CC478: the fast test on the byte, then the shared gate.
    if (lock.engaged && front_end_request_rejected_00683e90(lock, interface_id)) {
        return;
    }
    const PayloadTemporary temporary(host, payload);
    // The body of 004CC460. Only the pending record is written, so
    // 006840F0 sees the mismatch and drives the manager's virtual +10h.
    manager.pending.interface_id = interface_id;
    swap_payload(manager.pending.payload, payload, host);
}

bool apply_interface_request_00684600(FrontEndManagerBase& manager, FrontEndInterfaceLock& lock,
    int interface_id, void* payload, FrontEndPayloadHost& host) {
    // 0068461C..00684636, the same gate inlined. A rejected request returns 0
    // and the derived override then leaves the screens alone.
    if (lock.engaged && interface_id != kMovieCameraNewInterface) {
        if (interface_id >= kFirstInGameInterface) {
            return false;
        }
        lock.engaged = false;
    }
    const PayloadTemporary temporary(host, payload);
    manager.pending.interface_id = interface_id;
    swap_payload(manager.pending.payload, payload, host);
    // The tail of 00684600: the pending record becomes the applied one,
    // which is what marks the request serviced.
    manager.applied.interface_id = manager.pending.interface_id;
    swap_payload(manager.applied.payload, manager.pending.payload, host);
    return true;
}

void deactivate_front_end_manager_00683aa0(FrontEndManagerBase& manager,
    FrontEndScreenSetHost& screens) {
    // 00683AA2..00683AB3. Both records survive, which is the whole reason the
    // drain can lower a manager and raise it again without rebuilding it.
    manager.active = false;
    screens.set_gui_interface_set(nullptr, 0);
    screens.set_game_interface_set(nullptr, 0);
}

void activate_front_end_manager_00684700(FrontEndManagerRegistry& registry,
    FrontEndManagerBase& manager, FrontEndManagerBase* replay_exempt,
    FrontEndInterfaceLock& lock, FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens) {
    // 00684700..00684778. Every other registered manager goes down first, which
    // is the only mutual exclusion the front end has.
    for (FrontEndManagerBase* other : registry.managers) {
        if (other != nullptr && other != &manager) {
            deactivate_front_end_manager_00683aa0(*other, screens);
        }
    }
    // 0068477A.
    manager.active = true;
    // 0068477E..00684797. The in-mission interface manager at 00E198C4 is the
    // one manager that does not replay its record on the way up, and the
    // attract screen at 00E198BC is not exempt. Everything else re-issues what
    // it had,
    // through its own virtual +10h; the base implementation is used here, and
    // the three overrides all begin with exactly this call.
    if (&manager != replay_exempt) {
        apply_interface_request_00684600(
            manager, lock, manager.pending.interface_id, manager.pending.payload, payloads);
    }
}

void front_end_manager_init_default_00683a90() noexcept {
    // 00683A90 is a bare RET: a manager with nothing to build inherits it,
    // which is why the shell calls virtual +4h unconditionally.
}

void init_options_menu_006898c0(OptionsMenuManager& manager, FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads) {
    host.open_load_block(kOptionsMenuLoadBlock);
    // 0068991E. Init publishes the global before it builds anything, so the
    // screens it constructs can already reach the manager through 00E198B8.
    set.options = &manager;
    host.report_loading_progress(kOptionsMenuProgressSteps[0]);
    manager.screens[0] = host.create_screen(0x270, 0x005F6030u);
    host.register_screen(manager.screens[0]);
    host.report_loading_progress(kOptionsMenuProgressSteps[1]);
    manager.screens[1] = host.create_screen(0x30, 0x00527D00u);
    host.register_screen(manager.screens[1]);
    // No progress report before the third screen.
    manager.screens[2] = host.create_screen(0x250, 0x00555770u);
    host.register_screen(manager.screens[2]);
    // 006899DA. INTF_OPTIONS is committed directly, not pushed: the options
    // manager has one page and never waits for 006840F0.
    apply_interface_request_00684600(manager.base, lock, kInterfaceOptions, nullptr, payloads);
    host.close_load_block();
}

void init_main_menu_00686380(MainMenuManager& manager, FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads,
    FrontEndScreenSetHost& screens, FrontEndManagerRegistry& registry) {
    host.open_load_block(kMainMenuLoadBlock);
    // 00686380 stores the two paths in the manager rather than in locals,
    // because the destructor needs them to unload the same files.
    manager.music_paths[0] = kMainMenuTitleMusic;
    manager.music_paths[1] = kMainMenuCreditsMusic;
    for (const std::string_view path : manager.music_paths) {
        host.cache_music(path);
    }
    host.register_locale_table(kMainMenuLocaleTable);
    host.reload_locale_tables();
    // A mission result waiting to be shown clears the return flag and raises
    // 00E08874 instead, before any screen is built.
    if (host.mission_result_pending()) {
        host.clear_returning_from_mission();
        host.raise_front_end_dirty_flag();
    }
    // The same self-publish the options manager does, before the screens.
    set.main_menu = &manager;
    constexpr std::uint32_t kScreenConstructors[7] = {
        0x005902E0u, 0x005CA880u, 0x00626630u, 0x0051E4D0u, 0x0052FCE0u, 0x00563370u, 0x005098B0u,
    };
    constexpr std::size_t kScreenSizes[7] = {0x578, 0x34, 0x260, 0x138, 0x5C, 0x16C, 0x5D0};
    for (std::size_t i = 0; i < manager.screens.size(); ++i) {
        host.report_loading_progress(kMainMenuProgressSteps[i]);
        manager.screens[i] = host.create_screen(kScreenSizes[i], kScreenConstructors[i]);
        host.register_screen(manager.screens[i]);
    }
    host.report_loading_progress(kMainMenuProgressSteps[7]);
    // 006868AD..006868BD. This is the only place INTF_REWARDS is chosen, and
    // 004E4250 later reads the same field back to decide whether the shell
    // still needs to push INTF_MAINMENU.
    const int page = host.returning_from_mission() ? kInterfaceRewards : kInterfaceMainMenu;
    apply_interface_request_00684600(manager.base, lock, page, nullptr, payloads);
    // 006868C9: the main menu raises itself at the end of its own Init.
    activate_front_end_manager_00684700(
        registry, manager.base, set.in_mission, lock, payloads, screens);
    // 006868CC.
    host.set_gui_layer_enabled(1, true);
    host.close_load_block();
}

void init_multi_menu_00689540(MultiMenuManager& manager, FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads) {
    // At the head of 00689540 the block name is the literal
    // "GVMultiMenu::Init", opened
    // through BSP_FileBlock_Construct rather than the package helper the other
    // two use.
    host.open_load_block(kMultiMenuLoadBlock);
    host.refresh_multiplayer_state();
    if (host.downloadable_content_present()) {
        host.refresh_downloadable_content();
    }
    host.refresh_multiplayer_state();
    host.refresh_online_services();
    if (host.profile_name_empty()) {
        host.create_default_profile();
    }
    // 00689685.
    set.multi_menu = &manager;
    constexpr std::uint32_t kScreenConstructors[6] = {
        0x005E6D90u, 0x005EA8C0u, 0x005E3290u, 0x00574240u, 0x005D2F90u, 0x005CFDA0u,
    };
    constexpr std::size_t kScreenSizes[6] = {0xF8, 0x100, 0x300, 0x2D0, 0x5C, 0x90};
    for (std::size_t i = 0; i < manager.screens.size(); ++i) {
        manager.screens[i] = host.create_screen(kScreenSizes[i], kScreenConstructors[i]);
        host.register_screen(manager.screens[i]);
    }
    // 006897D3.
    apply_interface_request_00684600(
        manager.base, lock, kInterfaceMultiMainMenu, nullptr, payloads);
    host.close_load_block();
}

void destroy_options_menu_00689a10(OptionsMenuManager& manager, FrontEndManagerSet& set,
    FrontEndManagerHost& host) {
    release_screens(manager.screens.data(), manager.screens.size(), host);
    // 00689B53: the destructor clears its own global, so 004DA650's clear is a
    // second write to an already null slot.
    if (set.options == &manager) {
        set.options = nullptr;
    }
}

void destroy_main_menu_00686c90(MainMenuManager& manager, FrontEndManagerSet& set,
    FrontEndManagerHost& host) {
    release_screens(manager.screens.data(), manager.screens.size(), host);
    // 00686C90 clears its own global.
    if (set.main_menu == &manager) {
        set.main_menu = nullptr;
    }
    // In 00686C90 the two handles are released with an
    // InterlockedDecrement on handle+4h and destroyed through their own
    // virtual +00h when the count reaches zero.
    for (void*& handle : manager.music_handles) {
        handle = nullptr;
    }
    // Later in 00686C90 the two music files and their .def siblings leave the
    // FileStore again, in the same order they were cached.
    for (const std::string_view path : manager.music_paths) {
        host.remove_music(path);
    }
}

void destroy_multi_menu_006888e0(MultiMenuManager& manager, FrontEndManagerSet& set,
    FrontEndManagerHost& host) {
    release_screens(manager.screens.data(), manager.screens.size(), host);
    // 006888E0 clears its own global.
    if (set.multi_menu == &manager) {
        set.multi_menu = nullptr;
    }
    // At the tail of 006888E0 the member vector at +5Ch is freed. Nothing
    // this packet traced ever fills it.
    manager.owned_58.clear();
}

void construct_front_end_managers_004e4171(FrontEndManagerSet& set,
    FrontEndManagerRegistry& registry, FrontEndInterfaceLock& lock, FrontEndManagerHost& host,
    FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens) {
    // 004E4171: the options manager first, even though it is the one the front
    // end shows last.
    if (set.options == nullptr) {
        OptionsMenuManager* options = host.create_options_manager();
        if (options != nullptr) {
            registry.managers.push_back(&options->base);
            init_options_menu_006898c0(*options, set, lock, host, payloads);
        }
    }
    // 004E41B0.
    if (set.main_menu == nullptr) {
        MainMenuManager* main_menu = host.create_main_menu_manager();
        if (main_menu != nullptr) {
            registry.managers.push_back(&main_menu->base);
            init_main_menu_00686380(
                *main_menu, set, lock, host, payloads, screens, registry);
        }
    }
    // 004E41F0.
    if (set.multi_menu == nullptr) {
        MultiMenuManager* multi_menu = host.create_multi_menu_manager();
        if (multi_menu != nullptr) {
            registry.managers.push_back(&multi_menu->base);
            init_multi_menu_00689540(*multi_menu, set, lock, host, payloads);
        }
    }
}

void destroy_front_end_managers_004da650(FrontEndManagerSet& set, FrontEndManagerHost& host) {
    // The head of 004DA650.
    host.reset_interface_stack();
    // 004DA650 tests each global, calls its virtual +00h with the delete flag
    // and clears the global.
    if (set.main_menu != nullptr) {
        destroy_main_menu_00686c90(*set.main_menu, set, host);
        set.main_menu = nullptr;
    }
    if (set.multi_menu != nullptr) {
        destroy_multi_menu_006888e0(*set.multi_menu, set, host);
        set.multi_menu = nullptr;
    }
    if (set.options != nullptr) {
        destroy_options_menu_00689a10(*set.options, set, host);
        set.options = nullptr;
    }
    // The in-mission interface manager at 00E198C4 is destroyed the same way;
    // its body is not this packet's.
    set.in_mission = nullptr;
}

void teardown_front_end_managers_004db190(FrontEndManagerSet& set, FrontEndManagerHost& host) {
    // The head of 004DB190.
    host.raise_front_end_dirty_flag();
    // The in-mission interface manager goes down first and is not destroyed
    // here.
    if (set.in_mission != nullptr) {
        host.lower_in_mission_interface();
    }
    // Then 004DB190 takes the multiplayer menu, the options manager and the
    // main menu, in that order. The last two are the reverse of 004DA650.
    if (set.multi_menu != nullptr) {
        destroy_multi_menu_006888e0(*set.multi_menu, set, host);
        set.multi_menu = nullptr;
    }
    if (set.options != nullptr) {
        destroy_options_menu_00689a10(*set.options, set, host);
        set.options = nullptr;
    }
    if (set.main_menu != nullptr) {
        destroy_main_menu_00686c90(*set.main_menu, set, host);
        set.main_menu = nullptr;
    }
}

bool service_pending_interface_requests_006840f0(FrontEndManagerSet& set,
    FrontEndInterfaceLock& lock, FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens) {
    // 006840F3, 00684130, 0068416C: the main menu, the multiplayer menu, then
    // the options manager, in that order.
    FrontEndManagerBase* order[3] = {
        set.main_menu != nullptr ? &set.main_menu->base : nullptr,
        set.multi_menu != nullptr ? &set.multi_menu->base : nullptr,
        set.options != nullptr ? &set.options->base : nullptr,
    };
    bool serviced = false;
    for (FrontEndManagerBase* manager : order) {
        if (manager == nullptr || !manager->active) {
            continue;
        }
        // 00684104..00684114. An inactive manager is skipped outright, so a
        // request pushed at a lowered manager waits until it is raised again.
        const bool same_id = manager->applied.interface_id == manager->pending.interface_id;
        const bool same_payload = manager->applied.payload == manager->pending.payload;
        if (same_id && same_payload) {
            continue;
        }
        // 00684121..0068412B: the manager's virtual +10h, which begins with
        // 00684600 and then raises its own screen set. The screen-set step is
        // per manager and is not modelled here beyond the shared hide.
        apply_interface_request_00684600(*manager, lock, manager->pending.interface_id,
            manager->pending.payload, payloads);
        screens.set_gui_interface_set(&manager->applied.interface_id, 1);
        serviced = true;
    }
    return serviced;
}

bool run_front_end_manager_request(FrontEndManagerRequest request, FrontEndManagerSet& set,
    FrontEndScreenSetHost& screens) {
    // The three lowering requests deactivate; nothing here destroys. The
    // original loads the global and calls through it without a null test, so a
    // missing manager is a null dereference there and a false return here.
    FrontEndManagerBase* target = nullptr;
    switch (request) {
    case FrontEndManagerRequest::kLowerMainMenu:
        target = set.main_menu != nullptr ? &set.main_menu->base : nullptr;
        break;
    case FrontEndManagerRequest::kLowerMultiMenu:
        target = set.multi_menu != nullptr ? &set.multi_menu->base : nullptr;
        break;
    case FrontEndManagerRequest::kLowerOptions:
        target = set.options != nullptr ? &set.options->base : nullptr;
        break;
    case FrontEndManagerRequest::kRaiseMultiMenu:
    case FrontEndManagerRequest::kRaiseOptions:
    default:
        return false;
    }
    if (target == nullptr) {
        return false;
    }
    deactivate_front_end_manager_00683aa0(*target, screens);
    return true;
}

void raise_options_menu_004bac20(FrontEndManagerSet& set, FrontEndManagerRegistry& registry,
    FrontEndInterfaceLock& lock, FrontEndManagerHost& host, FrontEndPayloadHost& payloads,
    FrontEndScreenSetHost& screens) {
    // In 004BAC20 the manager is built only once; a second 14h just raises it.
    if (set.options == nullptr) {
        OptionsMenuManager* options = host.create_options_manager();
        if (options != nullptr) {
            registry.managers.push_back(&options->base);
            init_options_menu_006898c0(*options, set, lock, host, payloads);
        }
        // The Lua collection only runs after a fresh construction.
        if (host.script_gc_enabled()) {
            host.run_script_gc();
        }
    }
    // Then 004BAC20 raises it.
    if (set.options != nullptr) {
        activate_front_end_manager_00684700(
            registry, set.options->base, set.in_mission, lock, payloads, screens);
    }
    // 004BAC20 leaves the game in state 15h.
    host.set_game_state(0x15);
}

void raise_multi_menu_004bfc70(FrontEndManagerSet& set, FrontEndManagerRegistry& registry,
    FrontEndInterfaceLock& lock, bool multiplayer_entry_pending, FrontEndManagerHost& host,
    FrontEndPayloadHost& payloads, FrontEndScreenSetHost& screens) {
    // The same lazy construction 004BAC20 does.
    if (set.multi_menu == nullptr) {
        MultiMenuManager* multi_menu = host.create_multi_menu_manager();
        if (multi_menu != nullptr) {
            registry.managers.push_back(&multi_menu->base);
            init_multi_menu_00689540(*multi_menu, set, lock, host, payloads);
        }
        if (host.script_gc_enabled()) {
            host.run_script_gc();
        }
    }
    if (set.multi_menu == nullptr) {
        host.set_game_state(0x08);
        return;
    }
    FrontEndManagerBase& base = set.multi_menu->base;
    // 004BFCF0: when the online layer is not signed in the menu is forced back
    // to its own root page before it is raised.
    if (!host.online_signed_in()) {
        push_interface_request_004cc460(
            base, lock, kInterfaceMultiMainMenu, nullptr, payloads);
    }
    // 004BFD0E.
    activate_front_end_manager_00684700(registry, base, set.in_mission, lock, payloads, screens);
    // 004BFD10..004BFD46: an explicit entry into multiplayer asks for the mode
    // selector and, because activate has already replayed the old record, the
    // new request is applied here instead of waiting for 006840F0.
    if (multiplayer_entry_pending) {
        host.clear_multiplayer_entry_reason();
        push_interface_request_004cc460(
            base, lock, kInterfaceMultiModeSelector, nullptr, payloads);
        const bool same_id = base.applied.interface_id == base.pending.interface_id;
        const bool same_payload = base.applied.payload == base.pending.payload;
        if (!same_id || !same_payload) {
            apply_interface_request_00684600(
                base, lock, base.pending.interface_id, base.pending.payload, payloads);
        }
    }
    // The tail of 004BFC70 leaves the game in state 8.
    host.set_game_state(0x08);
}

}  // namespace bsp
