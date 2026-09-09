#include "bsp/resource_path.hpp"
#include <cstdint>
#include <algorithm>
#include <utility>

namespace bsp {
namespace {
bool slash(char c) { return c == '/' || c == '\\'; }
}
bool canonicalize_resource_path_00bee390_fragment(const std::string& input, std::string& output) {
    if (input.size() > INT32_MAX || !std::all_of(input.begin(), input.end(),
        [](unsigned char c) { return c != 0 && c < 128; })) return false;
    std::string result;
    std::size_t read = 0;
    if (!input.empty() && slash(input[0])) { result = "/"; ++read; }
    std::size_t barrier = result.size();
    bool boundary = true;
    while (read != input.size()) {
        const char c = input[read];
        if (slash(c)) {
            ++read;
            if (!boundary) { result += '/'; boundary = true; }
            continue;
        }
        if (boundary && c == '.') {
            const auto remaining = input.size() - read;
            if (remaining == 1) {
                ++read;
                if (!result.empty()) result.pop_back();
                boundary = false;
                continue;
            }
            if (slash(input[read + 1])) { read += 2; continue; }
            if (input[read + 1] == '.' && (remaining == 2 || slash(input[read + 2]))) {
                if (result.size() == barrier) {
                    result += "../";
                    barrier = result.size();
                } else {
                    result.pop_back();
                    while (!result.empty() && result.back() != '/') result.pop_back();
                }
                read += 2;
                boundary = true;
                continue;
            }
        }
        boundary = false;
        result += c >= 'A' && c <= 'Z' ? static_cast<char>(c + 32) : c;
        ++read;
    }
    if (result.size() > INT32_MAX) return false;
    output = std::move(result);
    return true;
}

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
bool join_resource_path_00bee520_fragment(const std::string& directory,
    const std::string& child, std::string& output) {
    if (directory.empty()) return canonicalize_resource_path_00bee390_fragment(child, output);
    if (child.empty()) return canonicalize_resource_path_00bee390_fragment(directory, output);
    if (directory.size() > INT32_MAX || child.size() > INT32_MAX
        || directory.size() + child.size() + 1 > INT32_MAX) return false;
    return canonicalize_resource_path_00bee390_fragment(directory + '/' + child, output);
}

}
