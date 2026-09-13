#include "bsp/native_unit_killed_tail.hpp"

namespace bsp {

void native_unit_on_killed_00779af0(const NativeUnitKilledTailView& view,
    void* const& controlled_listener_00e188dc, NativeUnitKilledTailHost& host)
{
    void* const unit = view.unit.canonical_unit; // EBX remains the original ECX.
    if (host.query_primary_18(unit) != nullptr) {
        void* const captured = controlled_listener_00e188dc; // 00779B05
        if (host.query_primary_18(unit) == captured) {
            host.publish_controlled_listener_004bca80(nullptr);
        }
    }

    host.current_world_unit_lists_built_193c() = 0;
    for (std::int32_t index = 0; index < kReconUnitDetectionCount; ++index) {
        const auto& record = view.detection[index];
        const std::int32_t level = record.force_10 != 0
            ? record.forced_level_08 : record.level_04;
        if (level > 0) {
            host.mark_recon_slot_dirty_00803ba0(index);
        }
    }
    host.on_killed_00928c80(unit);
}

} // namespace bsp
