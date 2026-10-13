// Non-gameplay leaves from the 0090-00BF naming sweep (cc13-sweep9x).
// Evidence and ABI: include/bsp/band_0090_cc13.hpp, docs/CC13_BAND_0090_00BF.md.
#include "bsp/band_0090_cc13.hpp"

#include <cmath>

namespace bsp::band0090 {

int recommend_detail_level(const HardwareSnapshot& hw, const std::vector<std::uint32_t>& gpu_scores)
{
    // 0098DF9B..0098DFC0: memory above 499 and CPU above 63Fh, else 0.
    if (hw.memory_mb <= 499u || hw.cpu_mhz <= 0x63Fu) {
        return 0;
    }
    std::uint32_t score = 0;
    if (hw.gpu_row == -1) {
        // Unknown adapter: row 0 ('Unknown'), but only when +20h is above 100h.
        if (hw.field_20 < 0x101u) {
            return 0;
        }
        score = gpu_scores.at(0);
    } else {
        score = gpu_scores.at(static_cast<std::size_t>(hw.gpu_row));
    }
    if (score == 0) {
        return 0;
    }
    const int memory_tier = hw.memory_mb < 0x201u ? 1 : (hw.memory_mb < 1000u ? 2 : 3);
    const int cpu_tier = hw.cpu_mhz < 0x9C4u ? 1 : (hw.cpu_mhz < 0xC80u ? 2 : 3);
    // 0098E04A..0098E082. The image sums in x87 extended precision; this sums in
    // double. Both round to float at the FSTP, so a difference needs a sum within
    // one double ulp of a float rounding boundary (not reached by the tier/score
    // values in this installation's table).
    const double sum = static_cast<double>(cpu_tier) * 1.600000023841858
                       + static_cast<double>(memory_tier) * 0.5
                       + static_cast<double>(score) * 0.8999999761581421;
    const float weighted = static_cast<float>(static_cast<double>(static_cast<float>(sum)) / 3.0);
    if (weighted > 0.0f && weighted < 1.5f) {
        return 1;
    }
    if (weighted >= 1.5f && weighted < 2.5f) {
        return 2;
    }
    return 3;
}

bool supports_display_mode(const HardwareSnapshot& hw, std::int32_t width, std::int32_t height)
{
    for (const auto& mode : hw.display_modes) {
        if (mode.first == width && mode.second == height) {
            return true;
        }
    }
    return false;
}

std::string build_default_options_text(const HardwareSnapshot& hw, const std::vector<std::uint32_t>& gpu_scores)
{
    const int level = recommend_detail_level(hw, gpu_scores);
    std::int32_t width = 0x280;
    std::int32_t height = 0x1E0;
    if (level == 1) {
        if (supports_display_mode(hw, 800, 600)) {
            width = 800;
            height = 600;
        }
    } else if (level == 2 || level == 3) {
        if (level == 3 && supports_display_mode(hw, 0x500, 0x400)) {
            width = 0x500;
            height = 0x400;
        } else if (supports_display_mode(hw, 0x400, 0x300)) {
            width = 0x400;
            height = 0x300;
        }
    }

    std::string text;
    text += "fullscreen 1\n";
    text += "Resolution " + std::to_string(width) + " " + std::to_string(height) + "\n";
    text += "Vsync 1\n";
    text += "ShaderModel " + std::to_string(hw.shader_model) + "\n";
    if (level == 3 && !hw.antialias_levels.empty()) {
        text += "Antialias " + std::to_string(hw.antialias_levels.back()) + "\n";
    } else {
        text += "Antialias 0\n";
    }
    text += "Clouds 1\n";
    text += level == 3 ? "Foliage 1\n" : "Foliage 0\n";
    text += level < 2 ? "Shadow 0\n" : "Shadow 1\n";
    text += level < 2 ? "Reflection 0\n" : "Reflection 1\n";
    text += level < 2 ? "OceanDetail 0\n" : "OceanDetail 1\n";
    text += level < 2 ? "TextureDetail 0\n" : "TextureDetail 2\n";
    text += level < 2 ? "ObjectDetail 0\n" : "ObjectDetail 2\n";
    text += hw.sound_device_found ? "SoundEnabled 1\n" : "SoundEnabled 0\n";
    const char* language = "english";
    switch (hw.language_lcid) {
    case 0x407: language = "deutsch"; break;
    case 0x40C: language = "francais"; break;
    case 0x410: language = "italiano"; break;
    case 0x415: language = "polski"; break;
    case 0x419: language = "russian"; break;
    default: break;
    }
    text += "Language ";
    text += language;
    text += "\n";
    return text;
}

std::array<std::uint32_t, 256> build_crc32_table()
{
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < 256; ++i) {
        std::uint32_t value = i;
        for (int bit = 0; bit < 8; ++bit) {
            value = (value & 1u) != 0 ? (value >> 1) ^ 0xEDB88320u : value >> 1;
        }
        table[i] = value;
    }
    return table;
}

void crc32_update(std::uint32_t& crc, const std::uint8_t* bytes, std::size_t count)
{
    static const std::array<std::uint32_t, 256> table = build_crc32_table();
    for (std::size_t i = 0; i < count; ++i) {
        crc = (crc >> 8) ^ table[(crc ^ bytes[i]) & 0xFFu];
    }
}

void set_dialog_target_level(DialogLevelState& state, float target)
{
    state.target_224 = target;
    const float difference = state.current_220 - target;
    state.fade_rate_228 = std::fabs(static_cast<float>(static_cast<double>(difference) * 0.5));
}

}  // namespace bsp::band0090
