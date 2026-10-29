#pragma once
// Subtitle schedule of the GUI Movie widget: the `.mws` script 00AB0520 loads
// beside the movie and the per-frame selection 00AADBA0 runs while it plays.
// These are the bodies behind MoviePlayerHost::prepare_movie_subtitles and
// update_movie_subtitles in bsp/movie_player.hpp; the host owns the file probe,
// Text widget and Bink handle. The optional script helper uses a separate Lua
// state and covers only the subtitle rows, not the complete native loader.
//
// Names are hypotheses, not recovered symbols. Evidence is in
// docs/CC13_GUI_RENDER.md. Not binary compatible: the native record is a
// 14h-byte {NativeString, float, float, byte, byte} in an std::vector at
// movie+10Ch with a cursor at movie+11Ch.
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {

// 00AB0571: the four characters at 00D5C330.
inline constexpr char kMovieSubtitleExtension[] = ".mws";

// 00AB0520 (00AB057F..00AB05B5): the movie path with its last four characters
// replaced by ".mws". The native substring is (0, size - 4); a name shorter
// than four characters is not handled here and returns just the extension.
std::string movie_subtitle_script_path_00ab0520(const std::string& movie_path);

// 00AB0620..: the Text child that shows the lines, chosen by the wide-screen
// byte at [0109CF04]+0Dh.
const char* movie_subtitle_text_widget_00ab0520(bool wide_screen) noexcept;

// One row of the script's global `subtitles` table, as 00AB07E8..00AB097E
// reads it: [1] string, [2] and [3] numbers narrowed to float, [4] and [5]
// booleans read with default false (00B662F0 with 0).
struct MovieSubtitle {
    std::string text;           // +00h, a lockit key or literal text
    float start{};              // +08h, seconds
    float end{};                // +0Ch, seconds
    bool always_show{false};    // +10h, shown even with subtitles off
    bool hide_for_english{false}; // +11h, suppressed when the language is "english"
};

// 00AB06E0..00AB0980: run the script in a Lua state of its own
// (BSP_LuaStateOwner_Open, RunScriptWithOverrides), then read
// subtitles[1], subtitles[2], ... until the first nil row. Here the state has
// no libraries opened; the installed movies/us_outro.mws uses a table constructor.
// The native state's library set is not established. The native indexed reads
// use lua_gettable (00B67720), and fields 4/5 require exact booleans or default
// false (00B662F0). This typed projection does not reproduce the native file
// probe, widget ownership, extratitles, or malformed-script error propagation.
// Returns false and fills `error` when the
// chunk does not load or run; `lines` is then empty.
bool load_movie_subtitle_script_00ab0520(const std::string& source,
    std::vector<MovieSubtitle>& lines, std::string& error);

// 00AADBCD..00AADC0A: playback time in seconds from BinkGetRealtime,
// FrameNum * FrameRateDiv / FrameRate with both dwords loaded unsigned.
float movie_subtitle_time_00aadba0(std::uint32_t frame_num, std::uint32_t frame_rate,
    std::uint32_t frame_rate_div) noexcept;

enum class MovieSubtitleText : std::uint8_t {
    Unchanged,
    Clear, // 00ABBE50 with "" (00CE3A0C)
    Show,  // 00ABAED0 with the record's text, localised (flag 1)
};

struct MovieSubtitleStep {
    bool expired_cleared{false}; // the current line ended: text cleared, cursor advanced
    MovieSubtitleText action{MovieSubtitleText::Unchanged};
    std::uint32_t index{};       // the record `action` refers to
};

// 00AADBA0's subtitle half, run once per frame while the script was found
// (+104h). At most one line retires per frame (t > end), then the line at the
// cursor, once t >= start, is cleared when (!always_show && !subtitles_enabled)
// or (hide_for_english && language_is_english), and shown otherwise. The show
// is re-issued every frame while the line is current. `subtitles_enabled` is
// byte +14h of the 004C1710 settings singleton; `language_is_english` is the
// case-insensitive compare of [00F8BC4C]+4018h with "english".
MovieSubtitleStep movie_subtitle_update_00aadba0(const std::vector<MovieSubtitle>& lines,
    std::uint32_t& cursor, float time, bool subtitles_enabled,
    bool language_is_english) noexcept;

// 00AADD65..00AADD81: each `extratitles` record (24h bytes at +124h) is shown
// while start (+08h) < t < end (+0Ch), both compares ordered (FCOMI then JBE),
// so a NaN anywhere hides it. The result goes to the record's widget +34h.
bool movie_extratitle_visible_00aadba0(float start, float end, float time) noexcept;

} // namespace bsp
