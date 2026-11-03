#include "bsp/game_unit_group_storage.hpp"
#include "bsp/native_unit_group_storage.hpp"

#include <cstring>

namespace bsp::game {

void GameUnitGroupStorage::initialize() noexcept {
    if (initialized_) return;
    // bsp.py show 0070DAB0: the sole external input is CF4888, 999.0f.
    // The original constructor does not initialize +504h; the speed caller
    // publishes the actual class-speed reduction before any speed read.
    static constexpr std::uint32_t native_cf4888_bits = 0x4479c000u;
    bsp::construct_native_unit_group_0070dab0(bytes_.data(), native_cf4888_bits);
    initialized_ = true;
}

bool GameUnitGroupStorage::capture_members(
    const std::vector<bsp::ShipAiUnitGroupMember>& members) noexcept {
    if (members.size() > 24) return false; // whole bodies require native capacity
    initialize();
    static_assert(sizeof(bsp::ShipAiUnitGroupMember) == 0x34);
    if (!members.empty()) {
        std::memcpy(bytes_.data() + 0x18, members.data(), members.size() * 0x34);
    }
    const auto count = static_cast<std::int32_t>(members.size());
    std::memcpy(bytes_.data() + 0x4f8, &count, sizeof(count));
    return true;
}

void GameUnitGroupStorage::publish_speed_ceiling(float speed) noexcept {
    initialize();
    std::memcpy(bytes_.data() + 0x504, &speed, sizeof(speed));
}

float GameUnitGroupStorage::speed_ceiling() const noexcept {
    float speed;
    std::memcpy(&speed, bytes_.data() + 0x504, sizeof(speed));
    return speed;
}

} // namespace bsp::game
