#pragma once

#include "bsp/main_menu_screen.hpp"
#include "bsp/mission_progress.hpp"
#include "bsp/mission_tree_data.hpp"

namespace bsp {

// 005C2F70..005C2F84. Native ECX = mission record, no stack arguments,
// bare RET, bool in AL; compatible with a one-argument __fastcall method.
// The record begins with its mission-id NativeString. The sole native call
// is 0090C560(ECX = [game+6B4h], &record.name); its AL is returned unchanged.
// `progress` makes that global dependency explicit and reuses the existing
// MissionProgress model. A missing id is false; any nonzero completion int
// is true. NativeStringCaseInsensitiveLess provides the existing key rule.
// New C++ interface and container layout, not a binary replacement.
bool mission_map_flag_visible_005c2f70(
    const MissionRecordData& record, const MissionProgress& progress);

// Native services used by 00599340. No defaults stand in for these callees.
// The selected record must remain valid through the side-index lookup.
struct MissionListRefreshHost {
    virtual ~MissionListRefreshHost() = default;

    // 00599355 / 00599379 -> 005806A0: global selected-mission lookup,
    // __cdecl, no arguments, EAX = record. Invalid native selection traps;
    // this reference contract therefore requires a valid selected record.
    virtual const MissionRecordData& selected_mission_005806a0() = 0;

    // 00599369 / 00599372 / 0059938D / 00599396 -> 00597870:
    // ECX = original screen, one page argument on stack, callee cleans it.
    // The host binds that screen and owns all list-builder side effects.
    virtual void build_mission_list_page_00597870(MainMenuPage page) = 0;
};

// 00599340..0059939C. Native ECX = main-menu screen, no stack arguments,
// no meaningful return, bare RET; conventional __thiscall / ECX-only ABI.
// Only page 9 acts. Snapshot screen+565h, get the selected mission, invoke
// existing mission_side_index_005c27e0, then rebuild page 7/6 (DLC) or 5/4
// (base), for side 0/1 respectively. No direct global-page assignment occurs.
void run_return_to_mission_list_00599340(MainMenuPage current_page,
    const MainMenuScreenState& screen, MissionListRefreshHost& host);

} // namespace bsp
