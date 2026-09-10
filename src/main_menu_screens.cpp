#include "bsp/main_menu_screens.hpp"

namespace bsp {

const MainMenuScreenClass* main_menu_screen_by_field_offset(int offset) noexcept {
    for (const MainMenuScreenClass& screen : kMainMenuScreens) {
        if (static_cast<int>(screen.manager_field_offset) == offset) {
            return &screen;
        }
    }
    return nullptr;
}

const MainMenuScreenClass* main_menu_screen_by_id(int screen_id) noexcept {
    for (const MainMenuScreenClass& screen : kMainMenuScreens) {
        if (screen.screen_id == screen_id) {
            return &screen;
        }
    }
    return nullptr;
}

bool is_campaign_mission_list_page(MainMenuPage page) noexcept {
    // 00598B7B..00598B9C tests 4, 5, 6 and 7 in that order before doing
    // anything else, so the four form one group.
    switch (page) {
    case MainMenuPage::CampaignUsn:
    case MainMenuPage::CampaignUsnDlc:
    case MainMenuPage::CampaignIjn:
    case MainMenuPage::CampaignIjnDlc:
        return true;
    default:
        return false;
    }
}

bool is_downloadable_content_page(MainMenuPage page) noexcept {
    // 00598B9E: only 5 and 7 store 1 into screen+55Ch.
    return page == MainMenuPage::CampaignUsnDlc || page == MainMenuPage::CampaignIjnDlc;
}

TacticalLibraryRequest open_tactical_library_005885d0(
    std::uint32_t selection_a, std::uint32_t selection_b) noexcept {
    // 005885D0..00588633. The two selection values are resolved first
    // (005885D4 through 005C27E0, then 005885E4), written to +9Ch and +A0h
    // with the byte at +A4h cleared, then mode 5 and selector 63h go in and
    // 004CC460 pushes interface 0Bh with a null payload.
    TacticalLibraryRequest request{};
    request.mode = 5;                 // 00588610
    request.selector = 0x63;          // 0058861A
    request.has_selection = true;
    request.selection_a = selection_a; // 005885F2, screen+9Ch
    request.selection_b = selection_b; // 005885F8, screen+A0h
    return request;
}

TacticalLibraryRequest open_tactical_library_005886c0() noexcept {
    // 005886C0..005886EB. The push happens first at 005886CA and the fields
    // follow, so the servicing pass can observe the request before the mode is
    // in place. Nothing writes +9Ch/+A0h here.
    TacticalLibraryRequest request{};
    request.mode = 4;        // 005886D7
    request.selector = 0x63; // 005886E1
    request.has_selection = false;
    return request;
}

void start_title_music_005884a0(MainMenuMusicState& state, std::string_view title_music_path,
    TitleMusicHost& host) {
    // 005884B9: the movie gate. 00584A30 returns non-zero while the intro clip
    // is up and the whole body is skipped.
    if (host.movie_clip_active(kIntroMovieClip)) {
        return;
    }
    // 005884CB: already started. The manager holds one stream for its lifetime.
    if (state.title_stream != nullptr) {
        return;
    }
    // 005884D5 operator new(54h), 00588528 the constructor over the manager's
    // +40h path. A failed allocation falls to 0058852F, which zeroes EAX.
    void* stream = host.create_music_stream(title_music_path);
    // 00588537: stored unconditionally, null included.
    state.title_stream = stream;

    // 0058853D..0058854A: a temporary copy of the native string at 00E19504.
    const std::string_view track = host.selected_track_name();
    // 0058855C. No null check on the stream; see the header note.
    host.open_stream(stream, track);
    // 00588569..00588587 releases the temporary through the sized storage pool.
    // Nothing observable follows from it, so it is not modelled.

    // 0058858C: the volume float, then the two tail calls.
    host.set_stream_volume(stream, host.music_volume()); // 0058859F, 00A864F0
    host.play_stream(stream);                            // 005885AD, 00A85C20
}

} // namespace bsp
