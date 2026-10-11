// GUI Movie subtitle schedule (00AB0520 rows, 00AADBA0 selection). Evidence in
// docs/CC13_GUI_RENDER.md; names are hypotheses.
#include "bsp/movie_subtitles.hpp"

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
