#pragma once
#include "bsp/d3d9_startup.hpp"
#include "bsp/random_threads.hpp"
#include <array>

namespace bsp {
// Semantic equivalents of globals 0108d6dc/dd/e0. The original counter is
// non-atomic. Configure locking before workers start; live mode changes unverified.
struct RendererSynchronization {
    bool enabled{};
    bool observed_enabled{};
    std::uint32_t nesting{};
};

// New interface, not the original renderer's memory layout. Device, shared sync
// state and optional tracked lock must outlive this object. No COM ownership.
class D3D9StateCache {
public:
    D3D9StateCache(IDirect3DDevice9& device, RendererSynchronization& synchronization,
        TrackedCriticalSection* lock) : device_(device), synchronization_(synchronization), lock_(lock) {}
    void set_render_state_00b24460(D3DRENDERSTATETYPE state, DWORD value);
    void set_sampler_state_00b24610(UINT sampler, D3DSAMPLERSTATETYPE state, DWORD value);
    void initialize_defaults_00b26170();
    // Needed after device reset; caller owns reset sequencing.
    void invalidate();
    std::uint32_t render_calls() const { return render_calls_; }
    std::uint32_t sampler_calls() const { return sampler_calls_; }
private:
    struct Entry { bool valid{}; DWORD value{}; };
    struct Guard;
    bool enter_00b33ad0();
    void leave_00b33b00(bool entered);
    IDirect3DDevice9& device_;
    RendererSynchronization& synchronization_;
    TrackedCriticalSection* lock_;
    // Native render-valid area +40h..113h; sampler states 0..13 in twenty banks.
    std::array<Entry, 212> render_{};
    std::array<std::array<Entry, 14>, 20> samplers_{};
    std::uint32_t render_calls_{};
    std::uint32_t sampler_calls_{};
};
}
