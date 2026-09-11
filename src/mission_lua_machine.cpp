// The mission Lua machine as an assembly. See include/bsp/mission_lua_machine.hpp
// for the evidence per rule; every name is a hypothesis, not a recovered symbol.
//
// Nothing here re-implements a native body. These are the pure rules that sit
// between the recovered host (bsp/mission_lua_host.hpp) and the rebuilt
// executable: which globals must exist before a mission chunk runs, what order
// the load walks its Lua hosts in, and the two record fields 008860b0 and
// 005e2f00 read.

#include "bsp/mission_lua_machine.hpp"

#include "bsp/mission_lua_host.hpp"

#include <cstring>

namespace bsp {

bool mission_lua_global_needs_owner_layer(const char* name) noexcept
{
    if (name == nullptr) {
        return false;
    }
    // A name the 00e0b7b8 table carries is placed by 006b8610 and needs
    // nothing else. Anything the installed scripts call that the table does
    // not carry has to come from one of the other four sources.
    return find_mission_lua_binding(name) == nullptr;
}

const char* lobby_settings_field_name(std::size_t slot) noexcept
{
    if (slot >= kLobbySettingsFieldCount) {
        // Slot 0Dh is the null pointer at 00e08940; the loop at 005e2f6f
        // skips it by index rather than by testing the pointer.
        return nullptr;
    }
    return kLobbySettingsFields[slot];
}

bool lobby_settings_field_is_game_owned(std::size_t slot) noexcept
{
    // 005e2f8b: the "read the stored value back" branch is taken only when the
    // lookup missed AND the slot is neither 0 nor 2.
    return slot == 0 || slot == 2;
}

std::vector<std::string> installed_mission_subdirectories()
{
    // Installed-file-checked against the Steam copy. Kept as data rather than
    // a directory scan so the rule is inspectable without the install present.
    return std::vector<std::string>{
        "COTP-IJN", "COTP-USN", "bsm", "chg", "ijn", "multi", "traininggrounds", "usn",
    };
}

bool mission_script_name_carries_subdirectory(const std::string& script_name) noexcept
{
    // 008860b0 does no directory walk: the name is concatenated verbatim
    // between "Scripts/missions/" and ".lua". Scripts/missions/ holds no loose
    // .lua file, so a name without a separator cannot resolve.
    return script_name.find('/') != std::string::npos
        || script_name.find('\\') != std::string::npos;
}

} // namespace bsp
