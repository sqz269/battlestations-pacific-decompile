#pragma once

#include "bsp/cc13_seg47.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace bsp {

// Read-only projection of the selected platform's bullet class. This is not a
// native object or a second class owner. The rack's Lua-loaded IgnitionDelay
// and AntiAir fields, and the selector caller's shared F874FD flag, are the
// only inputs read by 006E3260 / 007B80C0 (bsp.py show at those addresses).
class NativeRocketClassQuery final : public cc13_seg47::TargetFlagGlobalsHost {
    struct Fields {
        std::array<std::byte, 0xe0> unread_prefix;
        float ignition_delay_e0;
        std::uint8_t anti_air_e4;
    };
    static_assert(offsetof(Fields, ignition_delay_e0) == 0xe0);
    static_assert(offsetof(Fields, anti_air_e4) == 0xe4);

public:
    NativeRocketClassQuery(float ignition_delay, bool anti_air,
                          bool air_ground_same) noexcept
        : shared_flag_f874fd_(static_cast<std::uint8_t>(air_ground_same)) {
        fields_.ignition_delay_e0 = ignition_delay;
        fields_.anti_air_e4 = static_cast<std::uint8_t>(anti_air);
    }

    void* data() noexcept { return &fields_; }

    void* global_address(std::uint32_t address) override {
        if (address != 0x00f874fdu) std::abort();
        return &shared_flag_f874fd_;
    }

private:
    Fields fields_; // Unread bytes deliberately have no fabricated field values.
    std::uint8_t shared_flag_f874fd_;
};

} // namespace bsp
