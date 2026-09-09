#pragma once
#include "bsp/mounted_streams.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/vfs_search_defaults.hpp"
#include "bsp/vfs_mount_registration.hpp"
#include <utility>

// Diagnostic startup setup: the three initial mounts, supplied physical root,
// native texture/shader search groups and explicit cache priming. Package scans,
// original startup preload selection and native manager lifetime are separate.
class AssetStreamProbe {
public:
    explicit AssetStreamProbe(const std::string& root)
        : store_(std::make_shared<bsp::FileStore>()),
          registrations_(bsp::make_asset_search_registrations_00738360_fragment()) {
        const auto physical = bsp::create_physical_directory_00bf4df0_fragment(root, ".");
        const auto persistent = bsp::create_physical_directory_00bf4df0_fragment(root, "persistent_data");
        if (!physical || !persistent) return;
        auto root_mount = observe(bsp::bind_physical_directory_fragment(".", physical), false);
        auto persistent_mount = observe(bsp::bind_physical_directory_fragment("persistent_data", persistent), false);
        auto cache_mount = observe(bsp::bind_file_store_fragment(".", store_), true);
        std::vector<bsp::VfsMountRegistration> registered;
        if (!bsp::register_ordered_mount_00be1740_fragment(registered, std::move(root_mount), 0, 1)
            || !bsp::register_ordered_mount_00be1740_fragment(registered, std::move(persistent_mount), 99, 1)
            || !bsp::register_ordered_mount_00be1740_fragment(registered, std::move(cache_mount), 300, 0)) return;
        mounts_ = bsp::make_registered_mount_context_fragment(registered);
        ready_ = true;
    }
    AssetStreamProbe(const AssetStreamProbe&) = delete;
    AssetStreamProbe& operator=(const AssetStreamProbe&) = delete;
    bool read(const std::string& requested, std::shared_ptr<bsp::MemoryStream>& stream,
              std::string& error, std::string* logical = nullptr) {
        auto name = requested;
        if (!resolve(name, error)) return false;
        auto opened = bsp::open_resource_memory_00bdf310_fragment(mounts_, name);
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            error = opened.error.empty() ? "Resource stream unavailable or incomplete: " + name : std::move(opened.error);
            return false;
        }
        if (logical) *logical = name;
        stream = std::move(opened.stream);
        error.clear();
        return true;
    }
    bool cache_resolved(const std::string& requested, std::string& error) {
        auto name = requested;
        return resolve(name, error) && bsp::cache_resource_00be7ab0_fragment(*store_, mounts_, name, error);
    }
    std::size_t cached_opens() const noexcept { return cached_opens_; }
    std::size_t physical_opens() const noexcept { return physical_opens_; }
    std::size_t cache_entries() const noexcept { return store_->size(); }
private:
    bool resolve(std::string& name, std::string& error) {
        if (!ready_ || !bsp::resolve_existing_resource_00bdf4c0_fragment(mounts_, registrations_, name)) {
            error = "Mounted providers could not resolve: " + name;
            return false;
        }
        return true;
    }
    bsp::VfsMount observe(bsp::VfsMount mount, bool cache) {
        auto open = std::move(mount.open_read_only);
        mount.open_read_only = [this, cache, open = std::move(open)](const std::string& name) {
            auto result = open(name);
            if (result.provider_opened) ++(cache ? cached_opens_ : physical_opens_);
            return result;
        };
        return mount;
    }
    std::shared_ptr<bsp::FileStore> store_;
    bsp::VfsCandidateRegistrations registrations_;
    bsp::VfsMountContext mounts_;
    std::size_t cached_opens_{}, physical_opens_{};
    bool ready_{};
};
