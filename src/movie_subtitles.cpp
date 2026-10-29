// GUI Movie subtitle schedule (00AB0520 rows, 00AADBA0 selection). Evidence in
// docs/CC13_GUI_RENDER.md; names are hypotheses.
#include "bsp/movie_subtitles.hpp"

#include <memory>
#include <utility>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

namespace bsp {

std::string movie_subtitle_script_path_00ab0520(const std::string& movie_path)
{
    if (movie_path.size() < 4) {
        return kMovieSubtitleExtension;
    }
    return movie_path.substr(0, movie_path.size() - 4) + kMovieSubtitleExtension;
}

const char* movie_subtitle_text_widget_00ab0520(bool wide_screen) noexcept
{
    return wide_screen ? "SubtitlesWideScreen_Text" : "SubtitlesNormal_Text";
}

bool load_movie_subtitle_script_00ab0520(const std::string& source,
    std::vector<MovieSubtitle>& lines, std::string& error)
{
    lines.clear();
    error.clear();
    std::unique_ptr<lua_State, decltype(&lua_close)> owner(luaL_newstate(), &lua_close);
    if (!owner) { error = "Lua state allocation failed"; return false; }
    lua_State* L = owner.get();
    // 00B67720 pushes the numeric index then performs lua_gettable, preserving
    // indexed lookup semantics rather than substituting lua_rawgeti.
    const auto get_index = [L](int table_index, int index) {
        const int table = table_index > 0 ? table_index : lua_gettop(L) + table_index + 1;
        lua_pushnumber(L, static_cast<lua_Number>(index));
        lua_gettable(L, table);
    };
    if (luaL_loadbuffer(L, source.data(), source.size(), "mws") != 0 ||
        lua_pcall(L, 0, 0, 0) != 0) {
        error = lua_isstring(L, -1) ? lua_tostring(L, -1) : "script error";
        return false;
    }
    lua_getglobal(L, "subtitles");
    if (!lua_istable(L, -1)) {
        return true;
    }
    for (int row = 1;; ++row) {
        get_index(-1, row);
        if (lua_isnil(L, -1)) {
            break;
        }
        MovieSubtitle line;
        if (lua_istable(L, -1)) {
            get_index(-1, 1);
            if (lua_isstring(L, -1)) line.text = lua_tostring(L, -1);
            get_index(-2, 2);
            line.start = static_cast<float>(lua_tonumber(L, -1));
            get_index(-3, 3);
            line.end = static_cast<float>(lua_tonumber(L, -1));
            get_index(-4, 4);
            // 00AB0935/00AB0969 pass default 0 to 00B662F0, which accepts
            // only Lua BOOLEAN. Other truthy Lua values do not mean true here.
            line.always_show = lua_isboolean(L, -1) && lua_toboolean(L, -1) != 0;
            get_index(-5, 5);
            line.hide_for_english = lua_isboolean(L, -1) && lua_toboolean(L, -1) != 0;
            lua_pop(L, 5);
        }
        lines.push_back(std::move(line));
        lua_pop(L, 1);
    }
    return true;
}

float movie_subtitle_time_00aadba0(std::uint32_t frame_num, std::uint32_t frame_rate,
    std::uint32_t frame_rate_div) noexcept
{
    // IMUL keeps the low 32 bits; each FILD is corrected by 2^32 (00CE3978)
    // when its sign bit is set, i.e. both operands are read unsigned.
    const std::uint32_t scaled = frame_num * frame_rate_div;
    return static_cast<float>(static_cast<double>(scaled) / static_cast<double>(frame_rate));
}

MovieSubtitleStep movie_subtitle_update_00aadba0(const std::vector<MovieSubtitle>& lines,
    std::uint32_t& cursor, float time, bool subtitles_enabled,
    bool language_is_english) noexcept
{
    MovieSubtitleStep step;
    if (cursor >= lines.size()) {
        return step; // 00AADC32
    }
    if (time > lines[cursor].end) { // 00AADC47 FCOMIP, JBE skips
        step.expired_cleared = true;
        ++cursor;
    }
    if (cursor >= lines.size()) {
        return step; // 00AADC8E
    }
    const MovieSubtitle& line = lines[cursor];
    if (!(time >= line.start)) { // 00AADCA5 JC skips, unordered included
        return step;
    }
    step.index = cursor;
    const bool suppressed = (!line.always_show && !subtitles_enabled)
        || (line.hide_for_english && language_is_english);
    step.action = suppressed ? MovieSubtitleText::Clear : MovieSubtitleText::Show;
    return step;
}

bool movie_extratitle_visible_00aadba0(float start, float end, float time) noexcept
{
    return time > start && end > time;
}

} // namespace bsp
