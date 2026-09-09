#pragma once
#include "bsp/mpkg_provider.hpp"
#include "bsp/file_store.hpp"
#include "bsp/vfs_mount_registration.hpp"
#include <memory>

namespace bsp {
enum class VfsProviderCreateStatus { created, declined, failed };

// Explicit application-owned projection of the three startup factories in
// registration order. Share this object across managers to share the native
// FileStore factory's lazy provider. It does not represent arbitrary factories.
class VfsProviderFactories {
public:
    VfsProviderFactories();
    ~VfsProviderFactories();
    VfsProviderFactories(const VfsProviderFactories&) = delete;
    VfsProviderFactories& operator=(const VfsProviderFactories&) = delete;
    std::shared_ptr<FileStore> file_store_00be80b0_fragment();
    //00bdb040 ECX manager, system/virtual paths stack, RET8; first nonnull.
    // Typed factory failure is terminal; native bad-input/allocator behavior
    // is not reproduced. Decline/failure leave output unchanged.
    VfsProviderCreateStatus create_00bdb040_fragment(const std::string& system_path,
        const std::string& virtual_path, MpkgLogicalOpen current_vfs,
        VfsMount& output, std::string& error);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Concrete startup provider manager. Callback owners replace native intrusive
// lifetime; arbitrary callbacks/error handlers/unmount ABI remain outside scope.
// Serialized access; no mount mutation during traversal. Live MPKG source
// callbacks hold a weak reference to this manager's stable current context.
class VfsProviderManager {
public:
    explicit VfsProviderManager(std::shared_ptr<VfsProviderFactories> factories);
    ~VfsProviderManager();
    VfsProviderManager(const VfsProviderManager&) = delete;
    VfsProviderManager& operator=(const VfsProviderManager&) = delete;
    VfsProviderManager(VfsProviderManager&&) = delete;
    VfsProviderManager& operator=(VfsProviderManager&&) = delete;
    //00be1890 ECX manager, five stack args in this order, RET14h; provider/null.
    // Construct before register; update shared device ID; preserve ownership
    // byte. Startup +90 handler00530620 is a verified RET, so no callback is
    // invented here. Host diagnostics distinguish decline and guarded failure.
    VfsProviderCreateStatus mount_system_path_00be1890_fragment(
        const std::string& system_path, const std::string& virtual_path,
        std::int32_t priority, std::uint8_t ownership, std::int32_t device_id,
        std::shared_ptr<VfsProviderIdentity>& output, std::string& error);
    //00bdb120 ECX manager, system-name stack, RET4. Iteration-order first match
    // uses stored length then _stricmp. An empty query can find FileStore.
    std::shared_ptr<const VfsProviderIdentity> find_system_name_00bdb120(
        const std::string& system_name) const;
    VfsMountContext& context() noexcept;
    const std::vector<VfsMountRegistration>& registrations() const noexcept;
    std::shared_ptr<FileStore> file_store_00be80b0_fragment();
    // Host shutdown: stop current providers, then use the explicit manager pump
    // until drained before destruction. Never cancels or synthesizes callbacks.
    bool stop_pending_submissions(DWORD& error);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
