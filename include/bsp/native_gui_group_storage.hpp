#pragma once
#include "bsp/native_material_pools.hpp"
#include <mutex>

namespace bsp {
// Actual distinct38h F8BFA0 storage in the SAME E188B4 allocator-list domain.
// No implicit native initialization/destruction or payload ownership.
using NativeGuiGroupPoolStorage=NativeMaterialPoolStorage;
class NativeGuiGroupPool final {
public:
    static constexpr std::uint32_t native_global=0x00f8bfa0;
    static constexpr std::uint32_t native_vtable=0x00d5cbfc;
    static constexpr std::uint32_t native_virtual0=0x00ac74f0;
    static constexpr std::size_t object_bytes=0xec;
    static constexpr std::size_t slot_bytes=0xf0;
    static constexpr std::size_t slot_slab_index_offset=0xec;
    static constexpr std::size_t slab_bytes=0x1e44;
    static constexpr std::size_t free_indices_offset=0x1e00;
    static constexpr std::size_t free_count_offset=0x1e40;
    static constexpr std::uint32_t slots_per_slab=32;
    NativeGuiGroupPool(AllocatorListDomain&,NativeGuiGroupPoolStorage&);
    ~NativeGuiGroupPool();
    NativeGuiGroupPool(const NativeGuiGroupPool&)=delete;
    NativeGuiGroupPool& operator=(const NativeGuiGroupPool&)=delete;
    void initialize_00ac7410();
    void* allocate_00ac7590();
    void return_00ac7260(void* actual_slot) noexcept;
    void trim_00ac74f0(); // native virtual0: no internal lock
    void destroy_00ac71a0(); // frees slabs, never destroys payloads
    NativeGuiGroupPoolStorage& storage() noexcept { return storage_; }
private:
    AllocatorListDomain& allocator_list_;
    NativeGuiGroupPoolStorage& storage_;
    static void invoke_trim(void*);
};
// Complete metadata-only slab initializer; preserves EC payloads and tail2.
void* initialize_native_gui_group_slab_00ac6ff0(void*,std::uint32_t index) noexcept;
// Complete pointer-table cleanup; actual header is pool+28. No clearing.
void free_native_gui_group_pool_table_00ac70d0(void* actual_header) noexcept;

namespace game {
// Process storage binding only. Explicit CD7440 first performs AC7410, then
// CRT atexit(CE0AE0). Nonzero registration result retains initialized storage.
// Construction/startup is NOT implicitly installed into the application.
class GameNativeGuiGroupPoolProcess final {
public:
    GameNativeGuiGroupPoolProcess(const GameNativeGuiGroupPoolProcess&)=delete;
    GameNativeGuiGroupPoolProcess& operator=(const GameNativeGuiGroupPoolProcess&)=delete;
    int initialize_once_00cd7440();
    NativeGuiGroupPool& pool_00f8bfa0();
private:
    friend GameNativeGuiGroupPoolProcess& game_native_gui_group_pool_process();
    GameNativeGuiGroupPoolProcess();
    static void destroy_00ce0ae0() noexcept;
    enum class State { unattempted,returned,threw };
    NativeGuiGroupPoolStorage storage_00f8bfa0_{};
    NativeGuiGroupPool pool_;
    std::mutex startup_mutex_;
    State state_{State::unattempted};
    int registration_status_{};
};
GameNativeGuiGroupPoolProcess& game_native_gui_group_pool_process();
} // namespace game
// Complete selectors, with the same explicitly initialized process storage.
// AC76D0 ignores incoming size; AC73D0 returns a failed slot without teardown.
void* allocate_native_gui_group_slot_00ac76d0();
void return_native_gui_group_slot_00ac73d0(void* actual_slot);
} // namespace bsp
