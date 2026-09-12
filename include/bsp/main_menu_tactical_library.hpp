#pragma once
#include "bsp/frontend_managers.hpp"
#include "bsp/mission_briefing_start.hpp"

namespace bsp {
// Borrowed fields of the SAME screen published in MainMenuManager::screens[6]
// (native manager+70). No constructor, owned storage or default values here.
// 5885F2 stores the SECOND selected MissionRecord at9C;5885F8 stores the FIRST
// record's side index atA0. These are distinct from the old request-summary API.
struct MainMenuTacticalLibraryFields {
    void* screen_identity;
    std::int32_t& mode_94;
    std::int32_t& selector_98;
    const MissionRecordData*& selected_mission_9c;
    std::uint32_t& side_index_a0;
    std::uint8_t& flag_a4;
};
struct MainMenuTacticalLibraryBindings {
    MainMenuManager* const volatile& manager_00e198ac;
    const volatile std::uint32_t& selected_group_00e194d8;
    const volatile std::uint32_t& selected_mission_00e194dc;
    FrontEndInterfaceLock& interface_lock;
    FrontEndPayloadHost& payloads;
    // Pure storage accessors for the actual manager slots:1 is native+5C,
    //6 is native+70. They do not create tables/screens or cause callbacks.
    const MissionTreeTables& (*mission_tables)(void* screen_identity);
    MainMenuTacticalLibraryFields (*library_fields)(void* screen_identity);
};

// Complete normal sequences005885D0/005886C0, ECX screen (unused after the
// global selection getters), no stack arguments, RET. Existing actual tables,
// manager pending request and live screen fields are used in native order.
// Manager/screen/table storage must remain alive across payload callbacks;
// current global manager and screen slots may be rebound. Native STL failure,
// original screen/record layouts, SEH and callback ABI remain external.
void request_tactical_library_with_selection_005885d0(MainMenuTacticalLibraryBindings&);
void request_tactical_library_005886c0(MainMenuTacticalLibraryBindings&);
} // namespace bsp
