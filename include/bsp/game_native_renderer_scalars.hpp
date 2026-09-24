#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

#include <cstddef>
#include <cstdint>

namespace bsp::game {

using GameNativeRendererAtomicImport = long (__stdcall*)(volatile long*);
using GameNativeRendererSectionImport = void (__stdcall*)(void*);

// Canonical source owner for the loader-zero renderer scalar domain researched
// at 0108D4BC,0108D4C0,0108D6DC,0108D6E4,0108D6E8,0108DAF8 and0108DAFC.
// Also retains the shadow publication00F8BBF0 and real Windows import cells.
// This object does not create or publish the actual renderer at00F8D394.
class GameNativeRendererScalarProcess final {
public:
    GameNativeRendererScalarProcess(const GameNativeRendererScalarProcess&) = delete;
    GameNativeRendererScalarProcess& operator=(const GameNativeRendererScalarProcess&) = delete;

    NativeRendererSynchronizationGlobals& synchronization_0108d6dc() noexcept;
    // Cumulative native factory accounting, not live-owner counts. One copy
    // per process, shared by every application runtime/target/depth factory;
    // native deletion and context construction do not reset these cells.
    volatile std::uint32_t& texture_allocation_bytes_0108d4bc() noexcept;
    volatile std::uint32_t& surface_allocation_bytes_0108d4c0() noexcept;
    // Address-stable CURRENT pointer cell. Resolve actual kernel32 once;
    // borrowing it performs no increment and creates no alternate counter.
    GameNativeRendererAtomicImport volatile& increment_iat_00ce221c() noexcept;
    // One loader-zero publication for the actual2Ch shadow owner. Borrowing
    // never constructs, clears or resets it across application instances.
    void* volatile& shadow_target_publication_00f8bbf0() noexcept;
    GameNativeRendererSectionImport volatile& enter_iat_00ce2218() noexcept;
    GameNativeRendererSectionImport volatile& leave_iat_00ce2210() noexcept;
    // Raw DWORD bits used by the renderer worker's MOVSS/FSUB schedule. This is
    // deliberately not a float-valued interface.
    std::uint32_t& renderer_worker_time_bits_0108d6e4() noexcept;
    std::uint32_t& logical_texture_serial_0108d6e8() noexcept;
    std::uint32_t& texture_tracking_counter_0108daf8() noexcept;
    std::uint32_t& surface_tracking_counter_0108dafc() noexcept;

private:
    friend GameNativeRendererScalarProcess& game_native_renderer_scalar_process();
    GameNativeRendererScalarProcess();
    ~GameNativeRendererScalarProcess() = default;

    std::uint32_t texture_allocation_bytes_0108d4bc_{};
    std::uint32_t surface_allocation_bytes_0108d4c0_{};
    GameNativeRendererAtomicImport volatile increment_iat_00ce221c_;
    NativeRendererSynchronizationGlobals synchronization_0108d6dc_{};
    std::uint32_t renderer_worker_time_bits_0108d6e4_{};
    std::uint32_t logical_texture_serial_0108d6e8_{};
    std::uint32_t texture_tracking_counter_0108daf8_{};
    std::uint32_t surface_tracking_counter_0108dafc_{};
    void* volatile shadow_target_publication_00f8bbf0_{};
    GameNativeRendererSectionImport volatile enter_iat_00ce2218_;
    GameNativeRendererSectionImport volatile leave_iat_00ce2210_;
};

// The function-local process object retains loader-zero scalar preimages and
// resolves the actual import once. Its accessors return stable borrowed cells;
// there is no reset, native CRT callback or renderer/device lifecycle here.
GameNativeRendererScalarProcess& game_native_renderer_scalar_process();

static_assert(sizeof(NativeRendererSynchronizationGlobals) == 0x08);
static_assert(offsetof(NativeRendererSynchronizationGlobals, mode_00) == 0x00);
static_assert(offsetof(NativeRendererSynchronizationGlobals, observed_mode_01) == 0x01);
static_assert(offsetof(NativeRendererSynchronizationGlobals, preserved_02) == 0x02);
static_assert(offsetof(NativeRendererSynchronizationGlobals, nesting_04) == 0x04);

} // namespace bsp::game
