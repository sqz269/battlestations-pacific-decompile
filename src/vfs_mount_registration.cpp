#include "bsp/vfs_mount_registration.hpp"
#include "bsp/resource_path.hpp"
#include <algorithm>
#include <utility>

namespace bsp {
bool prepare_mount_prefix_00be1740_fragment(const std::string& input, std::string& output) {
    std::string result;
    if (!canonicalize_resource_path_00bee390_fragment(input, result)) return false;
    // Native00584110 stops at its first character, even when it is '/'.
    while (result.size() > 1 && result.back() == '/') result.pop_back();
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
