// Fixture check for the decal definition loader 00740840 against the installed
// scripts/datatables/decals.lua. See docs/APP_INIT_TAIL.md.
//
// The probe supplies what the native phase does before the parse: it opens a
// Lua state and runs the table script. The parse itself is the reconstruction.

#include "bsp/app_init_tail.hpp"
#include "bsp/gui_lua_runtime.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

// config/target.json's installed copy. Overridden by argv[1].
constexpr const char kDefaultGameRoot[] =
    "I:/SteamLibrary/steamapps/common/Battlestations Pacific";

} // namespace

int main(int argc, char** argv)
{
    const std::string root = argc > 1 ? argv[1] : kDefaultGameRoot;
    const std::string path = root + "/" + bsp::kDecalTablePath;

    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cout << "decal table not found: " << path << '\n';
        return 2;
    }
    std::ostringstream text;
    text << input.rdbuf();

    bsp::GuiLua51Host lua;
    std::string error;
    if (!lua.execute_archive(text.str(), error)) {
        std::cout << "decals.lua did not run: " << error << '\n';
        return 3;
    }

    // The native reads 0109EEA4 after each Lua call; every integer in this
    // table is small enough that both conversion paths agree.
    const bool crt_sse2_conversion = true;
    const std::vector<bsp::DecalDefinition> records =
        bsp::load_decal_definitions_00740840(lua, crt_sse2_conversion);

    std::cout << "decal definitions parsed: " << records.size() << '\n';
    for (const bsp::DecalDefinition& record : records) {
        std::cout << "  " << record.name
                  << " size=" << record.size
                  << " radius=" << record.radius
                  << " maxnum=" << record.max_count
                  << " texture=" << record.texture_name
                  << " shader=" << record.shader_name
                  << " life=" << record.life_time
                  << " fade=" << record.fade_out_time << '\n';
    }
    if (records.empty()) {
        return 4;
    }
    for (const bsp::DecalDefinition& record : records) {
        // Every installed entry carries all seven keys, so a zero here means
        // the walk lost a field rather than that the table omitted one.
        if (record.name.empty() || record.texture_name.empty()
            || record.shader_name.empty() || record.max_count == 0
            || record.size <= 0.0f || record.radius <= 0.0f) {
            std::cout << "incomplete record: " << record.name << '\n';
            return 5;
        }
    }
    return 0;
}
