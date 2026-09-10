#pragma once
#include "bsp/mounted_streams.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/vfs_search_defaults.hpp"
#include "bsp/vfs_mount_registration.hpp"
#include "bsp/resource_preload.hpp"
#include "bsp/vfs_provider_manager.hpp"
#include "bsp/package_scan.hpp"
#include <utility>

// Diagnostic startup uses actual factory/manager mounting and two package scans,
// then the native search groups and preload policy. Installation root remains
// a supplied host input. Full application/native teardown are separate.
class AssetStreamProbe {
public:
    explicit AssetStreamProbe(const std::string& root) {
        manager_ = std::make_unique<bsp::VfsProviderManager>(
            std::make_shared<bsp::VfsProviderFactories>());
        std::shared_ptr<bsp::VfsProviderIdentity> provider;
        std::string error;
        for (const auto& request : {std::pair<const char*, std::int32_t>{".", 0},
            {"persistent_data", 99}}) {
            if (manager_->mount_system_path_00be1890_fragment(root, request.first,
                request.second, 1, -1, provider, error) != bsp::VfsProviderCreateStatus::created) return;
        }
        if (manager_->mount_system_path_00be1890_fragment("filestore", ".", 300, 0, -1,
            provider, error) != bsp::VfsProviderCreateStatus::created) return;
        const bsp::PackageScanCallbacks scans{
            [this](const std::string& directory, const std::string& extension, std::uint32_t flags,
                std::vector<std::string>& output, std::string& message) {
                return bsp::enumerate_resources_00bdd990_fragment(manager_->context(),
                    directory, extension, flags, output, message);
            },
            [this](const std::string& name) { return manager_->find_system_name_00bdb120(name) != nullptr; },
            [this](const std::string& name, const std::string& prefix, std::int32_t priority,
                std::uint8_t ownership, std::int32_t device, std::string& message) {
                std::shared_ptr<bsp::VfsProviderIdentity> identity;
                const auto status = manager_->mount_system_path_00be1890_fragment(
                    name, prefix, priority, ownership, device, identity, message);
                return status == bsp::VfsProviderCreateStatus::created ? bsp::PackageMountStatus::mounted
                    : status == bsp::VfsProviderCreateStatus::declined ? bsp::PackageMountStatus::declined
                    : bsp::PackageMountStatus::failed;
            }};
        if (!bsp::startup_scan_packages_0073d881_fragment(scans, package_scans_)) return;
        registrations_ = bsp::make_asset_search_registrations_00738360_fragment();
        store_ = manager_->file_store_00be80b0_fragment();
        // Observe only subsequent asset opens; initial scan source reads are
        // outside these historical preload counters.
        for (auto& mount : manager_->context().mounts) {
            const bool cache = mount.provider->kind == bsp::VfsProviderKind::file_store;
            mount = observe(std::move(mount), cache);
        }
        ready_ = true;
    }
    AssetStreamProbe(const AssetStreamProbe&) = delete;
    AssetStreamProbe& operator=(const AssetStreamProbe&) = delete;
    bool read(const std::string& requested, std::shared_ptr<bsp::MemoryStream>& stream,
              std::string& error, std::string* logical = nullptr, std::uint32_t flags = 2) {
        auto name = requested;
        if (!resolve(name, error)) return false;
        auto opened = bsp::open_resource_memory_00bdf310_fragment(manager_->context(), name, flags);
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
        return resolve(name, error) && bsp::cache_resource_00be7ab0_fragment(*store_, manager_->context(), name, 2, error);
    }
    bool preload_startup_scripts(std::size_t& completed, std::string& error) {
        if (!ready_) { completed = 0; error = "Startup mounts unavailable."; return false; }
        return bsp::preload_startup_scripts_0073d410_fragment(*store_, manager_->context(), completed, error);
    }
    bool enumerate(const std::string& directory, const std::string& extension,
        std::uint32_t flags, std::vector<std::string>& output, std::string& error) {
        return ready_ && bsp::enumerate_resources_00bdd990_fragment(
            manager_->context(), directory, extension, flags, output, error);
    }
    const std::array<bsp::PackageScanPass, 2>& package_scans() const noexcept { return package_scans_; }
    struct OpenObservation { std::string name; std::uint32_t flags; bool cache; };
    const std::vector<OpenObservation>& opens() const noexcept { return opens_; }
    std::size_t cached_opens() const noexcept { return cached_opens_; }
    std::size_t physical_opens() const noexcept { return physical_opens_; }
    std::size_t cache_entries() const noexcept { return store_->size(); }
    // Exposes the native mutable-name existence query without opening a stream.
    bool resolve(std::string& name, std::string& error) {
        if (!ready_ || !bsp::resolve_existing_resource_00bdf4c0_fragment(manager_->context(), registrations_, name)) {
            error = "Mounted providers could not resolve: " + name;
            return false;
        }
        return true;
    }
private:
    bsp::VfsMount observe(bsp::VfsMount mount, bool cache) {
        auto open = std::move(mount.open_read_only);
        mount.open_read_only = [this, cache, open = std::move(open)](
            const std::string& name, std::uint32_t flags) {
            auto result = open(name, flags);
            if (result.provider_opened) {
                ++(cache ? cached_opens_ : physical_opens_);
                opens_.push_back({name, flags, cache});
            }
            return result;
        };
        return mount;
    }
    std::shared_ptr<bsp::FileStore> store_;
    bsp::VfsCandidateRegistrations registrations_;
    std::unique_ptr<bsp::VfsProviderManager> manager_;
    std::array<bsp::PackageScanPass, 2> package_scans_;
    std::size_t cached_opens_{}, physical_opens_{};
    std::vector<OpenObservation> opens_;
    bool ready_{};
};
