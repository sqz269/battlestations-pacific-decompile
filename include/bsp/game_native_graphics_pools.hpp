#pragma once

#include "bsp/native_graphics_pool_lifetime.hpp"
#include "bsp/native_material_pools.hpp"
#include "bsp/d3d9_surface_pool.hpp"

#include <array>
#include <mutex>

namespace bsp::game {

enum class GameNativeGraphicsPool : std::uint32_t {
    cube_texture = 0x0108db70, volume_texture = 0x0108dba8,
    material_pass = 0x0108fbf8, vertex_declaration = 0x0108fd38,
    layout_record = 0x0108fd70, physical_index = 0x0108fda8,
    physical_vertex = 0x0108fde0, logical_vertex = 0x0108fe18,
    logical_index = 0x0108fe50, hardware_layout = 0x0108fe9c
};

// Canonical process storage for the ten CRT entries at CE3518..CE353C.
// All borrow the existing E188B4 allocator list. FD70's legacy "hardware
// vertex layout" pool has 44h slots; FE9C's actual hardware-layout pool has
// 48h slots. They are distinct, as are the two physical-buffer pools.
class GameNativeGraphicsPoolProcess final {
public:
    GameNativeGraphicsPoolProcess(const GameNativeGraphicsPoolProcess&) = delete;
    GameNativeGraphicsPoolProcess& operator=(const GameNativeGraphicsPoolProcess&) = delete;

    // One attempt per global, retaining the original atexit status. Failed
    // registration does not roll back initialization. A thrown attempt cannot
    // be retried. Call in original table order before any renderer consumers.
    int initialize_once(GameNativeGraphicsPool);

    void* cube_texture_pool_0108db70();
    D3D9SurfacePool& volume_texture_pool_0108dba8();
    NativeMaterialPassPool& material_pass_pool_0108fbf8();
    NativeGraphicsPoolStorage& vertex_declaration_pool_0108fd38();
    NativeGraphicsPoolStorage& layout_record_pool_0108fd70();
    NativeGraphicsPoolStorage& physical_index_pool_0108fda8();
    NativeGraphicsPoolStorage& physical_vertex_pool_0108fde0();
    NativeGraphicsPoolStorage& logical_vertex_pool_0108fe18();
    NativeGraphicsPoolStorage& logical_index_pool_0108fe50();
    void* hardware_layout_pool_0108fe9c();

private:
    friend GameNativeGraphicsPoolProcess& game_native_graphics_pool_process();
    GameNativeGraphicsPoolProcess();
    ~GameNativeGraphicsPoolProcess() = default; // real CRT callbacks own native cleanup
    enum class StartupState { unattempted, returned, threw };
    static std::size_t index_of(GameNativeGraphicsPool);
    void require_returned(std::size_t);
    NativeGraphicsPoolStorage& graphics_storage(std::size_t);

    AllocatorListDomain& list_;
    alignas(4) std::array<std::byte, 0x38> cube_storage_0108db70_{};
    D3D9SurfacePoolStorage volume_storage_0108dba8_{};
    D3D9SurfacePool volume_;
    NativeMaterialPassPoolStorage pass_storage_0108fbf8_{};
    NativeMaterialPassPool pass_;
    std::array<NativeGraphicsPoolStorage, 6> graphics_storage_{};
    std::array<NativeGraphicsPoolLifetime, 6> graphics_;
    alignas(4) std::array<std::byte, 0x38> hardware_storage_0108fe9c_{};
    std::mutex startup_mutex_;
    std::array<StartupState, 10> states_{};
    std::array<int, 10> registration_status_{};
};

// C++ static construction and its bookkeeping destructor registration finish
// before native callbacks are registered. The existing allocator process is
// constructed first and outlives all ten native pool shutdown callbacks.
GameNativeGraphicsPoolProcess& game_native_graphics_pool_process();

} // namespace bsp::game
