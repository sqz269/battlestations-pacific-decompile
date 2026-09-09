#include "bsp/texture_atlas.hpp"

namespace bsp {
namespace {
bool equal_ascii_case_insensitive(std::string_view left, std::string_view right) {
    if (left.size() != right.size()) return false;
    const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; };
    for (std::size_t i = 0; i < left.size(); ++i)
        if (lower(left[i]) != lower(right[i])) return false;
    return true;
}

// 00aee0f0: full equality through00435c40 is case-insensitive; suffix
// comparison through0043e9a0 is case-sensitive. Do not fold the stored name.
bool matches(const TextureAtlasItem& item, std::string_view query) {
    if (equal_ascii_case_insensitive(item.name, query)) return true;
    if (item.name.size() <= query.size()) return false;
    const auto offset = item.name.size() - query.size();
    return item.name[offset - 1] == '/'
        && std::string_view(item.name).substr(offset) == query;
}
}

const TextureAtlasItem* find_texture_atlas_item_00aefb20(
    const std::vector<TextureAtlasItem>& items, const char* name) {
    if (!name || !*name) return nullptr;
    std::string query(name);
    // 00467cf0 searches backwards, excluding zero;0043bbf0 resizes to
    // that position (its20h argument is a fill character, not an erase count).
    const auto dot = query.find_last_of('.');
    if (dot != std::string::npos && dot > 0) query.resize(dot);
    for (auto& c : query) if (c == '\\') c = '/';
    const auto first = query.find_first_not_of('/');
    if (first == std::string::npos) return nullptr; // Malformed native domain.
    query.erase(0, first);
    for (auto& c : query) if (c >= 'A' && c <= 'Z') c += 32;
    for (const auto& item : items) if (matches(item, query)) return &item;
    const auto slash = query.find_last_of('/');
    if (slash != std::string::npos && slash > 0) {
        const std::string_view basename(query.data() + slash + 1, query.size() - slash - 1);
        for (const auto& item : items) if (matches(item, basename)) return &item;
    }
    return nullptr;
}
}
