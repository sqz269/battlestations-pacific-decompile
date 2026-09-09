#include "bsp/vfs_mount_registration.hpp"
#include <algorithm>
#include <utility>

namespace bsp {
namespace {
bool slash(char c) { return c == '/' || c == '\\'; }
}
bool prepare_mount_prefix_00be1740_fragment(const std::string& input, std::string& output) {
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
    // Native00584110 stops at its first character, even when it is '/'.
    while (result.size() > 1 && result.back() == '/') result.pop_back();
    if (result.size() > INT32_MAX) return false;
    output = std::move(result);
    return true;
}
bool register_ordered_mount_00be1740_fragment(
    std::vector<VfsMountRegistration>& registrations, VfsMount mount,
    std::int32_t priority, std::uint8_t native_ownership_flag) {
    if (!mount.exists || !mount.resolve) return false;
    for (std::size_t i = 1; i < registrations.size(); ++i)
        if (registrations[i - 1].priority < registrations[i].priority) return false;
    if (!prepare_mount_prefix_00be1740_fragment(mount.prefix, mount.prefix)) return false;
    const auto position = std::find_if(registrations.begin(), registrations.end(),
        [priority](const VfsMountRegistration& current) { return current.priority < priority; });
    registrations.insert(position, {priority, std::move(mount), native_ownership_flag});
    return true;
}
VfsMountContext make_registered_mount_context_fragment(
    const std::vector<VfsMountRegistration>& registrations) {
    VfsMountContext context;
    context.mounts.reserve(registrations.size());
    for (const auto& entry : registrations) context.mounts.push_back(entry.mount);
    return context;
}
}
