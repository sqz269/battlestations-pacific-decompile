// Packet cc13-ver22, docs/CC13_VER_22.md. Names are hypotheses.
#include "bsp/dogfight_ver_cc13.hpp"

namespace bsp {

bool dogfight_aim_adopts_gun_target_009a797f(const DogfightAimAdoptFacts& f) noexcept {
    if (!f.gun_target_present) return false;          // 009A798A JE 009A79AE
    if (f.gun_target_is_current) return false;        // 009A7992 JE
    if (!f.gun_target_has_squadron) return false;     // 009A799C JE
    return f.squadron_is_target_squadron;             // 009A79A4 JNE, else 009A79A9 CALL
}

unsigned dogfight_cone_tuning_offset(DogfightConeState s) noexcept {
    switch (s) {
    case DogfightConeState::kAim:
        return 0x678u;
    case DogfightConeState::kAttackRun:
    case DogfightConeState::kMoveTo:
    case DogfightConeState::kFollowFlyTo:
        return 0x670u;
    case DogfightConeState::kFollowHold:
        return 0x66Cu;
    default:
        return 0u;
    }
}

}  // namespace bsp
