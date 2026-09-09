#pragma once
#include "bsp/physical_directory.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/vfs_search_defaults.hpp"
#include <utility>

// Diagnostic provider setup. The native search lists and lookup algorithms are
// recovered; this single loose root is supplied by the probe. It does not
// reconstruct archive providers, mount insertion or original-game priority.
class LooseAssetProbe {
public:
    explicit LooseAssetProbe(std::string root) : directory_(std::move(root)),
        registrations_(bsp::make_asset_search_registrations_00738360_fragment()) {
        mounts_.mounts.push_back({"",
            [this](const std::string& name) { return directory_.exists_00bf3f70_fragment(name); },
            [this](const std::string& name, std::string& out) { return directory_.resolve_00bf0fb0(name, out); }});
    }
    LooseAssetProbe(const LooseAssetProbe&) = delete;
    LooseAssetProbe& operator=(const LooseAssetProbe&) = delete;
    bool resolve(const std::string& requested, std::string& physical,
                 std::string& error, std::string* logical = nullptr) {
        auto name = requested;
        if (!bsp::resolve_existing_resource_00bdf4c0_fragment(mounts_, registrations_, name)
            || !directory_.build_path_00bf3970(name, physical)) {
            error = "Loose provider could not resolve: " + requested;
            return false;
        }
        if (logical) *logical = name;
        return true;
    }
private:
    bsp::PhysicalDirectory directory_;
    bsp::VfsCandidateRegistrations registrations_;
    bsp::VfsMountContext mounts_;
};
