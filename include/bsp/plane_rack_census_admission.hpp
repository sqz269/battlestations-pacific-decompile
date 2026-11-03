#pragma once

namespace bsp::game {

struct SingleRackCensusIdentity {
    int platform_key{-1};
    int device_class{-1};
    int projectile_kind{0};
};

// Only compare ammunition with the device that produced its census entry.
// Equal rack counts do not establish identity: Rikufu equipment 2 replaces
// device 89 bombs with device 416 rockets at the same platform keys.
inline constexpr bool admits_single_rack_round_census(
    const SingleRackCensusIdentity& metadata,
    const SingleRackCensusIdentity& resolved, int live_rounds) noexcept {
    return live_rounds >= 0 && resolved.platform_key >= 0 && resolved.device_class >= 0
        && (resolved.projectile_kind == 0x2A || resolved.projectile_kind == 0x2C
            || resolved.projectile_kind == 0x31)
        && metadata.platform_key == resolved.platform_key
        && metadata.device_class == resolved.device_class
        && metadata.projectile_kind == resolved.projectile_kind;
}

} // namespace bsp::game
