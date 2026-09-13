#include "bsp/native_unit_wreck_tail.hpp"

namespace bsp {

NativeUnitWreckContinuation native_unit_wreck_effect_release_00824f39(
    NativeUnitWreckEffectReleaseView unit, NativeUnitWreckEffectReleaseContext& context) {
    void* const game = context.game_00e188a8; //824F39: capture once for both+38 reads
    auto game_fields = context.access.borrow_game(game);
    if (game_fields.session_38 == nullptr) return NativeUnitWreckContinuation::at_00824fe5;
    auto session = context.access.borrow_session(game_fields.session_38); //824F48 reload
    if (session.mode_60 != 4u) return NativeUnitWreckContinuation::at_00824fe5;

    auto release = [&](void* volatile& cell) {
        void* const captured = cell;
        if (captured == nullptr) return;
        // The preceding PUSH belongs to8674C0, not to the no-input getter.
        auto* const manager = live_effect_manager_singleton_004d1100(
            context.manager_00f8765c, context.lifetime_01090aa0);
        context.access.call_008674c0(*manager, captured);
        // Reborrow the actual cell: the preceding provider can replace it.
        context.access.call_00484620(cell, nullptr);
    };
    release(unit.effect_9ec);
    release(unit.effect_9f0);
    release(unit.effect_9f4);
    return NativeUnitWreckContinuation::at_00824fe5;
}

} // namespace bsp
