// Reconstruction of the multiplayer menu's six screen objects and the online
// refresh 00689510 runs. See include/bsp/multi_menu_screens.hpp and
// docs/MULTI_MENU_SCREENS.md.
#include "bsp/multi_menu_screens.hpp"

namespace bsp {

const MultiMenuScreenDescriptor* multi_menu_screen_by_id(MultiMenuScreenId id) noexcept
{
    for (const MultiMenuScreenDescriptor& screen : kMultiMenuScreens) {
        if (screen.screen_id == id) {
            return &screen;
        }
    }
    return nullptr;
}

const MultiMenuScreenDescriptor* multi_menu_screen_for_interface(int interface_id) noexcept
{
    // The chat overlay carries -1 because 00687330 never returns its screen id;
    // asking for interface -1 must not find it.
    if (interface_id < 0) {
        return nullptr;
    }
    for (const MultiMenuScreenDescriptor& screen : kMultiMenuScreens) {
        if (screen.interface_id == interface_id) {
            return &screen;
        }
    }
    return nullptr;
}

std::optional<MultiMenuOnlineRecord> refresh_online_player_list_00689310(
    MultiMenuOnlineRefreshState& state, MultiMenuOnlineHost& host)
{
    // 00689369..00689380: the local record, fetched with an empty query record
    // and the literal 1. Its name is the only field the scan uses.
    const MultiMenuOnlineRecord query{};
    const MultiMenuOnlineRecord local =
        host.online_local_record(query, kMultiMenuOnlineLocalSelector);

    // 006893A7..006893BF: the client fills the list the scan walks.
    const std::vector<MultiMenuOnlineRecord> players = host.online_player_list();

    // 006893D3..0068945B: walk the whole list, no early exit. 00689419 compares
    // min(len(local), len(entry)) bytes with 004B3FC0 and 00689425..00689438
    // then requires the lengths to be equal, so the test is a full string
    // equality. Each match is copied by 006883B0 into a stack record; the
    // original discards it, this keeps the last one.
    std::optional<MultiMenuOnlineRecord> matched;
    for (const MultiMenuOnlineRecord& entry : players) {
        if (entry.name == local.name) {
            matched = entry;
        }
    }

    // 00689466 and 00689476: the dirty byte, then the client notification. Both
    // run whether or not the scan matched anything.
    state.player_list_dirty = true;
    host.online_notify_refreshed(kMultiMenuOnlineNotifyReason);

    // 0068947C..00689486 destroy the list and free its head proxy; the vector
    // above expires here instead.
    return matched;
}

bool activate_multi_menu_00689510(
    MultiMenuOnlineRefreshState& state, MultiMenuOnlineHost& host)
{
    // 00689513: the base activate always runs first.
    host.activate_front_end_manager_base();

    // 00689518..00689527: the client's +68h predicate is the only gate.
    if (!host.online_refresh_available()) {
        return false;
    }

    // 0068952C is a tail jump, so the refresh returns straight to this caller.
    refresh_online_player_list_00689310(state, host);
    return true;
}
}
