#include "bsp/vfs_mounts.hpp"
#include "bsp/resource_path.hpp"
#include <cstring>
#include <utility>

namespace bsp {
namespace {
bool valid_string(const std::string& value) {
    return value.size() <= INT32_MAX && value.find('\0') == std::string::npos;
}
bool prepare(const VfsMountContext& context, const std::string& name,
             bool resolving, std::string& normalized) {
    if (!valid_string(name)) return false;
    for (const auto& mount : context.mounts) {
        if (!valid_string(mount.prefix) ||
            (resolving ? !mount.resolve : !mount.exists)) return false;
    }
    normalized = name;
    return normalize_resource_path_00bee690(normalized);
}
bool suffix_for_mount(const std::string& name, const std::string& prefix,
                      std::string& suffix) {
    if (prefix.empty()) { suffix = name; return true; }
    if (prefix.size() >= name.size() || name[prefix.size()] != '/') return false;
    if (_stricmp(name.substr(0, prefix.size()).c_str(), prefix.c_str()) != 0)
        return false;
    suffix = name.substr(prefix.size() + 1);
    return true;
}
}
bool exists_resource_00bdd440_fragment(VfsMountContext& context,
                                      const std::string& name) {
    std::string normalized;
    if (!prepare(context, name, false, normalized)) return false;
    context.error_code = -1;
    for (const auto& mount : context.mounts) {
        std::string suffix;
        if (suffix_for_mount(normalized, mount.prefix, suffix) && mount.exists(suffix))
            return true;
    }
    return false;
}
bool direct_resolve_resource_00bdd6e0_fragment(
    VfsMountContext& context, const std::string& name, std::string& output) {
    std::string normalized;
    if (!prepare(context, name, true, normalized)) return false;
    context.error_code = -1;
    for (const auto& mount : context.mounts) {
        std::string suffix, resolved;
        if (!suffix_for_mount(normalized, mount.prefix, suffix)) continue;
        if (!mount.resolve(suffix, resolved)) continue;
        if (!valid_string(resolved)) return false;
        if (!mount.prefix.empty()) {
            if (mount.prefix.size() + 1 + resolved.size() > INT32_MAX) return false;
            resolved = mount.prefix + '/' + resolved;
        }
        output = std::move(resolved);
        return true;
    }
    return false;
}
}
