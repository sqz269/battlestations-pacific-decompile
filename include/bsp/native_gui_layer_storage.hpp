#pragma once

#include "bsp/native_material_pools.hpp"
#include <mutex>

namespace bsp {

// The one 38h process pool at F8BF50. It shares the existing fixed-pool
// storage layout and allocator-list domain, not any material pool's slabs.
using NativeGuiLayerPoolStorage = NativeMaterialPoolStorage;

class NativeGuiLayerPool final {
public:
    static constexpr std::uint32_t native_global = 0x00f8bf50;
    static constexpr std::uint32_t native_vtable = 0x00d5caf0;
    static constexpr std::uint32_t native_virtual0 = 0x00ac4750;
    static constexpr std::size_t object_bytes = 0x124;
    static constexpr std::size_t slot_bytes = 0x128;
    static constexpr std::size_t slot_slab_index_offset = 0x124;
    static constexpr std::size_t slab_bytes = 0x2544;
    static constexpr std::size_t free_indices_offset = 0x2500;
    static constexpr std::size_t free_count_offset = 0x2540;
    static constexpr std::uint32_t slots_per_slab = 32;

    NativeGuiLayerPool(AllocatorListDomain&, NativeGuiLayerPoolStorage&);
    ~NativeGuiLayerPool();
    NativeGuiLayerPool(const NativeGuiLayerPool&) = delete;
    NativeGuiLayerPool& operator=(const NativeGuiLayerPool&) = delete;

    void initialize_00ac4d70();
    void* allocate_00ac4e50();
    void return_00ac47f0(void* slot) noexcept;
    void trim_00ac4750();
    void destroy_00ac4670();
    NativeGuiLayerPoolStorage& storage() noexcept { return storage_; }
private:
    AllocatorListDomain& allocator_list_;
    NativeGuiLayerPoolStorage& storage_;
    static void invoke_trim(void*);
};

namespace game {
// Source counterpart of the process-static F8BF50 storage. Construction binds
// the existing E188B4 allocator-list domain; explicit CD73D0 startup performs
// AC4D70 and registers CE0AD0. No allocation is legal before that startup.
class GameNativeGuiLayerPoolProcess final {
public:
    GameNativeGuiLayerPoolProcess(const GameNativeGuiLayerPoolProcess&) = delete;
    GameNativeGuiLayerPoolProcess& operator=(const GameNativeGuiLayerPoolProcess&) = delete;
    int initialize_once_00cd73d0();
    NativeGuiLayerPool& pool_00f8bf50();
private:
    friend GameNativeGuiLayerPoolProcess& game_native_gui_layer_pool_process();
    GameNativeGuiLayerPoolProcess();
    static void destroy_00ce0ad0() noexcept;
    enum class State { unattempted, returned, threw };
    NativeGuiLayerPoolStorage storage_00f8bf50_{};
    NativeGuiLayerPool pool_;
    std::mutex startup_mutex_;
    State state_{State::unattempted};
    int registration_status_{};
};
GameNativeGuiLayerPoolProcess& game_native_gui_layer_pool_process();
} // namespace game

// AC51A0 ignores the caller's ECX size and selects F8BF50. AC4C40 takes the
// same returned raw slot, without constructing or destroying a GUI page.
void* allocate_native_gui_layer_slot_00ac51a0();
void return_native_gui_layer_slot_00ac4c40(void* raw_slot);

} // namespace bsp
