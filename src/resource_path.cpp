#include "bsp/resource_path.hpp"
#include <cstdint>

namespace bsp {
void lowercase_resource_name_004bcc00(std::string& name) {
    for (char& c : name) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
}
bool normalize_resource_path_00bee690(std::string& path) {
    if (path.size() > INT32_MAX) return false;
    lowercase_resource_name_004bcc00(path);
    for (char& c : path) if (c == '\\') c = '/';
    std::size_t first = 0;
    while (first < path.size() && path[first] == ' ') ++first;
    if (first == path.size()) { path.clear(); return true; }
    std::size_t end = path.size();
    while (end > first && path[end - 1] == ' ') --end;
    path = path.substr(first, end - first);
    const auto nul = path.find('\0');
    if (nul != std::string::npos)
        for (std::size_t i = nul; i < path.size(); ++i) path[i] = '\0';
    return true;
}
bool shader_descriptor_name_00b2ebb0_fragment(const std::string& name, std::string& output) {
    if (name.size() > INT32_MAX || name.find('\0') != std::string::npos) return false;
    std::string changed = name;
    std::size_t position = 0;
    while ((position = changed.find(".mshd", position)) != std::string::npos) {
        changed.replace(position, 5, ".shfx");
        position += 5;
    }
    output = std::move(changed);
    return true;
}
}
