#pragma once

#include "bsp/light_type_bootstrap.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_render_batch_lifetime.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdint>

namespace bsp {

// All cells refer to the application's existing source publications. This
// bundle does not create aliases or shadow the host's cells. The caller keeps
// them stable through manager drain; each mutable value may change normally.
struct NativeVfsPublicationCells {
    void* volatile& manager_01090aa0;
    void* volatile& vfs_0109ceec;
    void* volatile& stream_pool_0109dc28;
    NativeRenderBatchPoolStorage* volatile& batch_pool_0108fe8c;
    NativeRenderBatchLockOwner* volatile& batch_lock_0109dbbc;
    NativeStringPoolStorage* volatile& string_pool_01090aa8;
    volatile std::uint32_t& string_returns_disabled_01090aa4;
    TypeIdCounterStorage* volatile& type_counter_0109db7c;
};

// Retained contexts for the current raw physical-stream route. The caller
// owns the cells, including actual01090AA0, and must drain that manager
// while this bundle and the borrowed table/callbacks remain alive. This does
// not construct a VFS manager/provider, open a device, or run shutdown itself.
class NativeVfsOwnerServices final {
public:
    NativeVfsOwnerServices(NativeVfsPublicationCells,
        const volatile std::uint32_t* actual_batch_table_00d5e5ac,
        const SingletonLifetimeCallbacks& invalid_parameters);
    NativeVfsOwnerServices(const NativeVfsOwnerServices&) = delete;
    NativeVfsOwnerServices& operator=(const NativeVfsOwnerServices&) = delete;

    // Install borrowed contexts before their profiles register. The binding
    // object and this bundle must outlive the raw manager's complete drain.
    void bind_deletion(NativeSingletonDeletionBindings&) noexcept;

    SoundLifetimeAccess lifetime() const noexcept { return lifetime_; }
    ActualNativeStringPoolStorage& strings() noexcept { return strings_; }
    NativePhysicalFileDateContext& physical() noexcept { return physical_; }
    NativeRenderBatchLifetime& batches() noexcept { return batches_; }
    NativePhysicalStreamOpenContext& streams() noexcept { return streams_; }
    TypeIdCounterLifetime& types() noexcept { return types_; }

    void* volatile& vfs_publication_0109ceec() noexcept { return cells_.vfs_0109ceec; }
    void* volatile& stream_pool_publication_0109dc28() noexcept { return cells_.stream_pool_0109dc28; }
    NativeRenderBatchPoolStorage* volatile& batch_pool_publication_0108fe8c() noexcept {
        return cells_.batch_pool_0108fe8c;
    }
    NativeRenderBatchLockOwner* volatile& batch_lock_publication_0109dbbc() noexcept {
        return cells_.batch_lock_0109dbbc;
    }
    NativeStringPoolStorage* volatile& string_pool_publication_01090aa8() noexcept {
        return cells_.string_pool_01090aa8;
    }
    volatile std::uint32_t& string_returns_disabled_01090aa4() noexcept {
        return cells_.string_returns_disabled_01090aa4;
    }
    TypeIdCounterStorage* volatile& type_counter_publication_0109db7c() noexcept {
        return cells_.type_counter_0109db7c;
    }

private:
    NativeVfsPublicationCells cells_;
    SoundLifetimeAccess lifetime_;
    ActualNativeStringPoolStorage strings_;
    NativePhysicalFileDateContext physical_;
    NativeRenderBatchLifetime batches_;
    NativePhysicalStreamOpenContext streams_;
    TypeIdCounterLifetime types_;
};

} // namespace bsp
