#include "bsp/native_unit_lifecycle_services.hpp"

namespace bsp {

void mark_native_recon_slot_dirty_00803ba0(std::int32_t index,
    NativeReconDirtyAccess& access) {
    if (auto* dirty = access.current_slot_dirty_25(index)) *dirty = 1;
    const auto world = access.current_world_00e188a8();
    const std::int32_t player = world.local_player_index_18ec;
    if (player < 0 || player > 7) return;
    if (index == access.player_context_28(world.canonical_world, player))
        world.unit_lists_built_193c = 0;
}

void publish_native_controlled_listener_004bca80(void* handle,
    NativeControlledListenerPublication publication,
    NativeControlledListenerRenderer& renderer) {
    publication.controlled_listener_00e188dc = handle;
    void* const captured = publication.renderer_00f8d39c;
    renderer.set_listener_00b0d7b0(captured, handle);
}

} // namespace bsp
