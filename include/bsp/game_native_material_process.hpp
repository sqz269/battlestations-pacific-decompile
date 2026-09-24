#pragma once

#include "bsp/native_material_pools.hpp"
#include <mutex>

namespace bsp::game {

// One process owns the distinct F8D3AC material and F8D3E4 parameter pools,
// plus the loader-zero effect serial F8D3A8. Both pools borrow the existing
// E188B4 allocator list; the material-pass pool 0108FBF8 remains separate.
class GameNativeMaterialProcess final {
public:
    GameNativeMaterialProcess(const GameNativeMaterialProcess&) = delete;
    GameNativeMaterialProcess& operator=(const GameNativeMaterialProcess&) = delete;

    // Explicit native CRT order: CD78D0 then CD78F0. One attempt per pool;
    // repeat access returns the original atexit status, never resets storage.
    // A thrown attempt cannot retry. Nonzero atexit leaves native initialization
    // intact without inventing a cleanup callback or rolling it back.
    int initialize_material_once_00cd78d0();
    int initialize_parameters_once_00cd78f0();
    NativeMaterialPool& material_pool_00f8d3ac();
    NativeMaterialParameterPool& parameter_pool_00f8d3e4();

    // The SAME current DWORD for every actual B18D60 construction. Native
    // load/store increment is serialized by callers, not an interlocked count.
    // Access does not consume a serial or depend on pool initialization.
    volatile std::uint32_t& effect_serial_00f8d3a8() noexcept { return effect_serial_; }

private:
    friend GameNativeMaterialProcess& game_native_material_process();
    GameNativeMaterialProcess();
    ~GameNativeMaterialProcess() = default; // CRT callbacks own native destruction.
    enum class State { unattempted, returned, threw };
    volatile std::uint32_t effect_serial_{};
    NativeMaterialPoolStorage material_storage_{};
    NativeMaterialParameterPoolStorage parameter_storage_{};
    NativeMaterialPool materials_;
    NativeMaterialParameterPool parameters_;
    std::mutex startup_mutex_;
    State material_state_{State::unattempted}, parameter_state_{State::unattempted};
    int material_registration_status_{}, parameter_registration_status_{};
};

// Construct the shared allocator owner and this object BEFORE registering
// CE0BF0/CE0C00. Both objects survive application drain and reverse native
// atexit callbacks (parameter first, material second). Pool callbacks free
// slabs, not live payloads: all material/parameter owners, queued uses and
// borrowed operation frames must be settled before those callbacks run.
GameNativeMaterialProcess& game_native_material_process();

} // namespace bsp::game
