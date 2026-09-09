#include "bsp/vfs_provider_manager.hpp"
#include "bsp/mounted_streams.hpp"
#include "bsp/vfs_pending.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
bool bounded_string(const std::string& value) noexcept {
    return value.size() <= INT32_MAX && value.find('\0') == std::string::npos;
}
std::shared_ptr<VfsProviderIdentity> identity(VfsProviderKind kind,
    const std::string& system_name) {
    auto result = std::make_shared<VfsProviderIdentity>();
    result->kind = kind;
    result->system_name = system_name;
    return result;
}
void bind_no_pending(VfsMount& mount) {
    // FileStore00be7cb0 / MPKG00bb9d30: XOR AL,AL; RET10h.
    mount.submit_pending = [](const std::string&, const std::string&,
        PhysicalReadCallback, std::uint32_t, DWORD& error) {
        error = ERROR_SUCCESS;
        return false;
    };
    // FileStore00be7cc0 / MPKG00bb9d40: plain RET.
    mount.pump_pending = [](PhysicalReadPumpReport& report, DWORD& error) {
        report = {};
        error = ERROR_SUCCESS;
        return true;
    };
    mount.stop_pending_submissions = [](DWORD& error) { error = ERROR_SUCCESS; return true; };
}
}
struct VfsProviderFactories::Impl {
    std::shared_ptr<FileStore> store;
    std::shared_ptr<VfsProviderIdentity> store_identity;
};
VfsProviderFactories::VfsProviderFactories() : impl_(std::make_unique<Impl>()) {}
VfsProviderFactories::~VfsProviderFactories() = default;
std::shared_ptr<FileStore> VfsProviderFactories::file_store_00be80b0_fragment() {
    if (!impl_->store) {
        auto store = std::make_shared<FileStore>();
        auto provider = identity(VfsProviderKind::file_store, "");
        impl_->store = std::move(store);
        impl_->store_identity = std::move(provider);
    }
    return impl_->store;
}
VfsProviderCreateStatus VfsProviderFactories::create_00bdb040_fragment(
    const std::string& system_path, const std::string& virtual_path,
    MpkgLogicalOpen current_vfs, VfsMount& output, std::string& error) {
    error.clear();
    if (!bounded_string(system_path) || !bounded_string(virtual_path)) {
        error = "Provider factory requires bounded NUL-free names.";
        return VfsProviderCreateStatus::failed;
    }
    if (auto physical = create_physical_directory_00bf4df0_fragment(system_path, virtual_path)) {
        auto result = bind_physical_directory_fragment(virtual_path, physical);
        if (!bind_physical_pending_fragment(result, physical, std::make_shared<PhysicalPendingReads>())) {
            error = "Cannot bind physical pending-read owner.";
            return VfsProviderCreateStatus::failed;
        }
        result.provider = identity(VfsProviderKind::physical_directory, system_path);
        output = std::move(result);
        return VfsProviderCreateStatus::created;
    }
    if (!system_path.empty() && _stricmp(system_path.c_str(), "filestore") == 0) {
        auto result = bind_file_store_fragment(virtual_path, file_store_00be80b0_fragment());
        result.provider = impl_->store_identity;
        bind_no_pending(result);
        output = std::move(result);
        return VfsProviderCreateStatus::created;
    }
    std::shared_ptr<MpkgArchive> archive;
    const auto status = create_mpkg_archive_00bb9d90_fragment(system_path,
        std::move(current_vfs), archive, error);
    if (status == MpkgCreateStatus::declined) return VfsProviderCreateStatus::declined;
    if (status == MpkgCreateStatus::failed) return VfsProviderCreateStatus::failed;
    auto result = bind_mpkg_archive_fragment(virtual_path, archive);
    result.provider = identity(VfsProviderKind::mpkg, system_path);
    bind_no_pending(result);
    output = std::move(result);
    return VfsProviderCreateStatus::created;
}

struct VfsProviderManager::Impl {
    explicit Impl(std::shared_ptr<VfsProviderFactories> value)
        : factories(std::move(value)), current(std::make_shared<VfsMountContext>()) {
        if (!factories) throw std::invalid_argument("VFS manager requires startup factories.");
    }
    std::shared_ptr<VfsProviderFactories> factories;
    std::shared_ptr<VfsMountContext> current;
    std::vector<VfsMountRegistration> registrations;
};
VfsProviderManager::VfsProviderManager(std::shared_ptr<VfsProviderFactories> factories)
    : impl_(std::make_unique<Impl>(std::move(factories))) {}
VfsProviderManager::~VfsProviderManager() = default;
VfsProviderCreateStatus VfsProviderManager::mount_system_path_00be1890_fragment(
    const std::string& system_path, const std::string& virtual_path, std::int32_t priority,
    std::uint8_t ownership, std::int32_t device_id,
    std::shared_ptr<VfsProviderIdentity>& output, std::string& error) {
    error.clear();
    VfsMount mount;
    std::weak_ptr<VfsMountContext> current = impl_->current;
    const auto status = impl_->factories->create_00bdb040_fragment(system_path, virtual_path,
        [current](const std::string& name, std::uint32_t flags) {
            auto live = current.lock();
            return live ? open_resource_memory_00bdf310_fragment(*live, name, flags)
                : VfsMemoryOpen{false, {}, "Current VFS manager has expired."};
        }, mount, error);
    if (status != VfsProviderCreateStatus::created) return status;
    auto provider = mount.provider;
    // Complete all potentially throwing vector/callback copies before publishing
    // the registration. Native failure/SEH mutation ordering is not replicated.
    auto registered = impl_->registrations;
    if (!register_ordered_mount_00be1740_fragment(registered, std::move(mount), priority, ownership)) {
        error = "Unsupported provider registration prefix or ordering.";
        return VfsProviderCreateStatus::failed;
    }
    auto next = make_registered_mount_context_fragment(registered);
    provider->device_id = device_id;
    impl_->registrations = std::move(registered);
    impl_->current->mounts = std::move(next.mounts); // Keep current aliases/error and weak identity.
    output = std::move(provider);
    return VfsProviderCreateStatus::created;
}
std::shared_ptr<const VfsProviderIdentity> VfsProviderManager::find_system_name_00bdb120(
    const std::string& system_name) const {
    if (!bounded_string(system_name)) return {};
    for (const auto& entry : impl_->registrations) {
        const auto& provider = entry.mount.provider;
        if (provider && provider->system_name.size() == system_name.size()
            && _stricmp(provider->system_name.c_str(), system_name.c_str()) == 0) return provider;
    }
    return {};
}
VfsMountContext& VfsProviderManager::context() noexcept { return *impl_->current; }
const std::vector<VfsMountRegistration>& VfsProviderManager::registrations() const noexcept {
    return impl_->registrations;
}
std::shared_ptr<FileStore> VfsProviderManager::file_store_00be80b0_fragment() {
    return impl_->factories->file_store_00be80b0_fragment();
}
bool VfsProviderManager::stop_pending_submissions(DWORD& error) {
    return begin_pending_shutdown_fragment(*impl_->current, error);
}
}
