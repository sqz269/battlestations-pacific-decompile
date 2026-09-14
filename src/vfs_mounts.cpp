#include "bsp/vfs_mounts.hpp"
#include "bsp/resource_path.hpp"
#include "bsp/vfs_native_access.hpp"
#include <cstring>
#include <utility>

namespace bsp {
namespace {
enum class Operation { exists, resolve, open };
bool valid_string(const std::string& value) {
    return value.size() <= INT32_MAX && value.find('\0') == std::string::npos;
}
bool prepare(const VfsMountContext& context, const std::string& name,
             Operation operation, std::string& normalized) {
    if (!valid_string(name)) return false;
    for (const auto& mount : context.mounts) {
        if (!valid_string(mount.prefix)) return false;
        if ((operation == Operation::exists && !mount.exists)
            || (operation == Operation::resolve && !mount.resolve)
            || (operation == Operation::open && !mount.open_read_only)) return false;
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
    if (context.native_access) return context.native_access->exists(name);
    std::string normalized;
    if (!prepare(context, name, Operation::exists, normalized)) return false;
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
    if (context.native_access) return context.native_access->direct_resolve(name, output);
    std::string normalized;
    if (!prepare(context, name, Operation::resolve, normalized)) return false;
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
bool apply_resource_alias_00bdca80_fragment(const std::vector<VfsAlias>& aliases,
                                         std::string& name) {
    if (!valid_string(name)) return false;
    for (const auto& entry : aliases)
        if (!valid_string(entry.from) || !valid_string(entry.to)) return false;
    for (const auto& entry : aliases) {
        if (entry.from.size() == name.size() && _stricmp(entry.from.c_str(), name.c_str()) == 0) {
            name = entry.to;
            break;
        }
    }
    return true;
}
VfsMemoryOpen open_resource_memory_00bdf310_fragment(VfsMountContext& context,
    const std::string& name, std::uint32_t flags) {
    if (context.native_access) return context.native_access->open(name, flags);
    if (flags != 2 && flags != 0x32)
        return {false, {}, "Unsupported VFS open flags."};
    std::string normalized;
    if (!prepare(context, name, Operation::open, normalized)
        || !apply_resource_alias_00bdca80_fragment(context.aliases, normalized))
        return {false, {}, "Unsupported VFS name, alias or missing read-only provider."};
    context.error_code = -1;
    std::string last_error;
    for (const auto& mount : context.mounts) {
        std::string suffix;
        if (!suffix_for_mount(normalized, mount.prefix, suffix)) continue;
        auto opened = mount.open_read_only(suffix, flags);
        if (opened.provider_opened) return opened;
        if (!opened.error.empty()) last_error = std::move(opened.error);
    }
    return {false, {}, last_error.empty() ? "No provider opened resource: " + normalized : std::move(last_error)};
}
bool enumerate_resources_00bdd990_fragment(VfsMountContext& context,
    const std::string& directory, const std::string& extension, std::uint32_t flags,
    std::vector<std::string>& output, std::string& error) {
    error.clear();
    if (context.native_access) {
        const auto names = context.native_access->enumerate(directory, extension, flags);
        for (const auto& name : names) {
            bool duplicate = false;
            for (const auto& previous : output) {
                if (name.size() == previous.size() && _stricmp(name.c_str(), previous.c_str()) == 0) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) output.push_back(name);
        }
        return true;
    }
    if (!valid_string(directory) || !valid_string(extension)) {
        error = "Unsupported VFS enumeration query.";
        return false;
    }
    for (const auto& name : output) if (!valid_string(name)) {
        error = "Unsupported existing VFS enumeration name.";
        return false;
    }
    for (const auto& mount : context.mounts) if (!valid_string(mount.prefix)) {
        error = "Unsupported VFS enumeration prefix.";
        return false;
    }
    context.error_code = -1;
    auto result = output;
    for (const auto& mount : context.mounts) {
        std::string suffix;
        if (!suffix_for_mount(directory, mount.prefix, suffix)) continue;
        if (!mount.enumerate) {
            error = "Matching VFS provider has no reconstructed enumeration operation.";
            return false;
        }
        std::vector<std::string> names;
        if (!mount.enumerate(suffix, extension, flags & 0xffu, names, error)) return false;
        for (auto& name : names) {
            if (!valid_string(name)) {
                error = "Provider returned an unsupported enumeration name.";
                return false;
            }
            bool duplicate = false;
            for (const auto& prior : result) {
                if (prior.size() == name.size() && _stricmp(prior.c_str(), name.c_str()) == 0) {
                    duplicate = true;
                    break;
                }
            }
            if (!duplicate) result.push_back(std::move(name));
        }
    }
    output = std::move(result);
    error.clear();
    return true;
}
}
