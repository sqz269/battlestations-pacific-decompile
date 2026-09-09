#include "bsp/texture_atlas.hpp"
#include <cmath>
#include <cstdlib>
#include <limits>
#include <utility>

namespace bsp {
namespace {
// 00af5740: ECX buffer, stack output string, RET4/AL. Assembly uses signed
// byte comparisons and deletes embedded controls, rather than replacing tabs.
// Dynamic storage replaces the native unbounded global scratch at 00f8c2c8.
class Lines {
public:
    explicit Lines(std::string_view text) : text_(text) {}
    bool next(std::string& out) {
        if (cursor_ >= text_.size()) return false;
        auto end = text_.find('\n', cursor_);
        if (end == std::string_view::npos) end = text_.size();
        auto begin = cursor_;
        const auto printable = [](char c) {
            const auto b = static_cast<unsigned char>(c);
            return b >= 0x20 && b < 0x80;
        };
        while (begin < end && (!printable(text_[begin]) || text_[begin] == ' ')) ++begin;
        out.clear();
        for (auto i = begin; i < end; ++i) {
            if (printable(text_[i])) out.push_back(text_[i]);
        }
        // Native advances one beyond extent on an unterminated final line.
        // Saturating here has identical subsequent EOF behavior without overflow.
        cursor_ = end < text_.size() ? end + 1 : end;
        return true;
    }
private:
    std::string_view text_;
    std::size_t cursor_{};
};

bool equal_ascii(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? c + ('a' - 'A') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}

// 00aee3c0 (ECX line, stack output/index, RET8) and 00aee340
// (ECX token, stack source, RET4): ASCII spaces separate printable runs.
std::string_view token(std::string_view line, unsigned index) {
    std::size_t pos = 0;
    for (unsigned current = 0; pos < line.size(); ++current) {
        const auto begin = pos;
        while (pos < line.size() && line[pos] > ' ' && line[pos] <= '~') ++pos;
        if (current == index) return line.substr(begin, pos - begin);
        if (pos == line.size() || line[pos] != ' ') return {};
        while (pos < line.size() && line[pos] == ' ') ++pos;
    }
    return {};
}

// 00aee620: x87 FISTP under explicitly selected truncation mode, low-word store.
// Reject values outside its defined int32 conversion domain instead of relying
// on native masked exceptions or undefined C++ floating-to-integer conversions.
bool pack(double coordinate, std::uint16_t& out) {
    const double truncated = std::trunc(coordinate * 65535.0);
    if (!std::isfinite(truncated) ||
        truncated < std::numeric_limits<std::int32_t>::min() ||
        truncated > std::numeric_limits<std::int32_t>::max()) return false;
    out = static_cast<std::uint16_t>(static_cast<std::int32_t>(truncated));
    return true;
}

bool read_item(Lines& lines, TextureAtlasItem& item, std::string& detail) {
    std::string line;
    while (lines.next(line) && line != "{") {}
    unsigned mask = 0;
    while (lines.next(line) && line != "}") {
        if (!equal_ascii(token(line, 0), "Param")) continue;
        constexpr std::string_view fields[] = {"U1", "V1", "U2", "V2"};
        for (unsigned i = 0; i < 4; ++i) {
            if (!equal_ascii(token(line, 1), fields[i])) continue;
            const auto number = token(line, 2);
            if (number.empty()) { detail = "Missing numeric Param token"; return false; }
            const std::string terminated(number);
            char* end = nullptr;
            const double value = std::strtod(terminated.c_str(), &end);
            if (end == terminated.c_str() || !std::isfinite(value) ||
                std::abs(value) > std::numeric_limits<float>::max()) {
                detail = "Unsupported numeric Param value";
                return false;
            }
            // CRT atof in 00aee620 likewise accepts a numeric prefix. The native
            // x87 result is stored to float32 before final packed conversion.
            item.uv[i] = static_cast<float>(value);
            mask |= 1u << i;
            break;
        }
    }
    if (mask != 15) { detail = "Missing UV fields have no native initialized defaults"; return false; }
    for (std::size_t i = 0; i < 4; ++i) {
        if (!pack(item.uv[i], item.packed_uv[i])) {
            detail = "UV exceeds supported native integer conversion domain";
            return false;
        }
    }
    // Subtract in double, retaining the x87 no-float32-store boundary. Exact for
    // installed fixture fractions; arbitrary x87 precision modes are unverified.
    if (!pack(static_cast<double>(item.uv[3]) - item.uv[1], item.packed_uv[4]) ||
        !pack(static_cast<double>(item.uv[2]) - item.uv[0], item.packed_uv[5])) {
        detail = "UV extent exceeds supported native integer conversion domain";
        return false;
    }
    return true; // Native accepts EOF after complete fields without a closing brace.
}
}

TextureAtlasParseResult parse_texture_atlas_00aeeaf0(
    std::string_view text, std::string_view descriptor_path, const TextureAtlasLookup& lookup) {
    TextureAtlasParseResult result;
    Lines lines(text); // 00af55f0 rewinds the native input object before parsing.
    std::string line;
    if (!lines.next(line) || !equal_ascii(token(line, 0), "TextureAtlas")) return result;
    const auto texture_name = token(line, 1);
    result.status = TextureAtlasParseStatus::unsupported_malformed;
    if (texture_name.empty()) { result.detail = "Missing atlas texture filename"; return result; }
    if (!lookup) { result.detail = "Texture lookup callback is required"; return result; }
    const auto slash = descriptor_path.find_last_of("/\\");
    if (slash != std::string_view::npos) result.texture_path.assign(descriptor_path.substr(0, slash + 1));
    result.texture_path.append(texture_name);
    for (auto& c : result.texture_path) if (c == '\\') c = '/';
    result.texture = lookup(result.texture_path, 0); // Native does not reject null.
    while (lines.next(line) && line != "{") {}
    while (lines.next(line) && line != "}") {
        if (!equal_ascii(token(line, 0), "TextureItem")) continue;
        const auto name = token(line, 1);
        if (name.empty()) { result.detail = "Missing TextureItem name"; return result; }
        TextureAtlasItem item;
        item.name.assign(name);
        item.descriptor_path.assign(descriptor_path);
        item.texture = result.texture;
        // 00aee4d0 calls reverse helper 00467cf0: searches downwards, but never
        // compares offset zero. Constructor original ABI is ECX item/RET0c.
        const auto dot = item.name.find_last_of('.');
        if (dot != std::string::npos && dot > 0) item.name.resize(dot);
        if (!read_item(lines, item, result.detail)) return result;
        result.items.push_back(std::move(item));
    }
    result.status = TextureAtlasParseStatus::success;
    return result;
}
}
