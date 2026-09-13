#include "bsp/native_vfs_owner_services.hpp"

namespace bsp {

NativeVfsOwnerServices::NativeVfsOwnerServices(
    NativeVfsPublicationCells cells,
    const volatile std::uint32_t* actual_batch_table_00d5e5ac,
    const SingletonLifetimeCallbacks& invalid_parameters)
    : cells_(cells),
      lifetime_(cells_.manager_01090aa0),
      strings_(cells_.string_pool_01090aa8, cells_.string_returns_disabled_01090aa4,
          cells_.manager_01090aa0),
      physical_{cells_.vfs_0109ceec, strings_, invalid_parameters},
      batches_(lifetime_, cells_.batch_pool_0108fe8c, cells_.batch_lock_0109dbbc,
          actual_batch_table_00d5e5ac),
      streams_{physical_, cells_.stream_pool_0109dc28, lifetime_, batches_},
      types_(lifetime_, cells_.type_counter_0109db7c) {}

void NativeVfsOwnerServices::bind_deletion(
    NativeSingletonDeletionBindings& bindings) noexcept {
    bindings.actual_string_pool_publication_01090aa8 = &cells_.string_pool_01090aa8;
    bindings.actual_string_returns_disabled_01090aa4 =
        &cells_.string_returns_disabled_01090aa4;
    bindings.physical_stream_pool = &streams_;
    bindings.render_batch_lifetime = &batches_;
    bindings.type_id_counter_lifetime = &types_;
}

} // namespace bsp
