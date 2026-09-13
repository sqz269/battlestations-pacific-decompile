#pragma once
#include <cstdint>

namespace bsp {
struct NativeRendererDeviceRecreationContext;

// Borrow the same actual renderer, current D5F0A8 profile, synchronization,
// gamma and resource domains as recreation. The pending byte is the actual
// application cell consumed by Reset; it is neither initialized nor mirrored.
struct NativeRendererPresentationModeContext {
    NativeRendererDeviceRecreationContext& recreation;
    volatile std::uint8_t& actual_pending_0108d4b8;
};

// Complete B29E60..B2A063. Native ECX renderer, six DWORD stack slots, RET18h;
// only AL=1 is semantic. This source interface adds a borrowed context.
// Flag slots retain their raw low bytes, including noncanonical values.
// Zero dimensions retain the current presentation dimensions on the update
// path. Valid actual renderer storage extends through +1D93. Current concrete
// D5F0A8 slots +2C/B1FE20 and +F0/B21960 and all reached recreation domains
// must be valid. No semantic renderer/presentation owner is introduced.
std::uint8_t change_native_renderer_presentation_mode_00b29e60(
    void* actual_renderer, std::uint32_t width_slot, std::uint32_t height_slot,
    std::uint32_t fullscreen_slot, std::uint32_t multisample_slot,
    std::uint32_t interval_selector_slot, std::uint32_t force_slot,
    NativeRendererPresentationModeContext&);

// Normal exits reload the current lifecycle lock. Native EH cleans only the
// optional guard; exceptional paths retain lifecycle depth/ownership and all
// completed writes. A skipped guard remains uninitialized, so enabling its
// cleanup later is outside the valid caller domain. Complete source behavior
// does not establish native caller/FH3/SEH identity or game/runtime integration.
} // namespace bsp
