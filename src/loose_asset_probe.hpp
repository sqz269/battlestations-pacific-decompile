#pragma once
#include "bsp/physical_directory.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/vfs_search_defaults.hpp"
#include "bsp/vfs_mount_registration.hpp"
#include <utility>

// Diagnostic provider setup. Replays startup's first two physical mounts with
// a supplied installation root, using recovered factory and insertion policies.
// Filestore/packages and later cache population remain separate dependencies.
class LooseAssetProbe {
public:
    explicit LooseAssetProbe(std::string root) :
        registrations_(bsp::make_asset_search_registrations_00738360_fragment()) {
        directory_ = bsp::create_physical_directory_00bf4df0_fragment(root, ".");
        const auto persistent = bsp::create_physical_directory_00bf4df0_fragment(root, "persistent_data");
        if (!directory_ || !persistent) return;
        const auto bind = [](std::string prefix, const std::shared_ptr<bsp::PhysicalDirectory>& owner) {
            return bsp::VfsMount{std::move(prefix),
                [owner](const std::string& name) { return owner->exists_00bf3f70_fragment(name); },
                [owner](const std::string& name, std::string& out) { return owner->resolve_00bf0fb0(name, out); }};
        };
        std::vector<bsp::VfsMountRegistration> registered;
        if (!bsp::register_ordered_mount_00be1740_fragment(registered, bind(".", directory_), 0, 1)
            || !bsp::register_ordered_mount_00be1740_fragment(registered,
                bind("persistent_data", persistent), 99, 1)) return;
        mounts_ = bsp::make_registered_mount_context_fragment(registered);
        ready_ = true;
    }
    LooseAssetProbe(const LooseAssetProbe&) = delete;
    LooseAssetProbe& operator=(const LooseAssetProbe&) = delete;
    bool resolve(const std::string& requested, std::string& physical,
                 std::string& error, std::string* logical = nullptr) {
        auto name = requested;
        if (!ready_ || !bsp::resolve_existing_resource_00bdf4c0_fragment(mounts_, registrations_, name)) {
            error = "Loose provider could not resolve: " + requested;
            return false;
        }
        // Both physical providers have the same system root. The persistent
        // mount strips its virtual prefix before constructing a system path.
        const std::string persistent_prefix = "persistent_data/";
        const auto suffix = name.size() > persistent_prefix.size()
            && name.compare(0, persistent_prefix.size(), persistent_prefix) == 0
            ? name.substr(persistent_prefix.size()) : name;
        if (!directory_->build_path_00bf3970(suffix, physical)) return false;
        if (logical) *logical = name;
        return true;
    }
private:
    std::shared_ptr<bsp::PhysicalDirectory> directory_;
    bsp::VfsCandidateRegistrations registrations_;
    bsp::VfsMountContext mounts_;
    bool ready_{};
};
