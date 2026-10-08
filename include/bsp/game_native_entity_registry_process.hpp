#pragma once
#include "bsp/native_entity_registry_lifetime.hpp"

namespace bsp::game {

// Private canonical facade. Borrow the permanent core owner and the SAME
// GameNativeStringProcess manager cell. No shadow manager/cells/ID arrays.
// Construction binds references only; explicit initialization below is still
// required. Normal app startup and actual entity/World producers are separate.
class GameNativeEntityRegistryProcess final {
public:
    GameNativeEntityRegistryProcess(const GameNativeEntityRegistryProcess&) = delete;
    GameNativeEntityRegistryProcess& operator=(const GameNativeEntityRegistryProcess&) = delete;
    NativeEntityRegistryCrtStatuses initialize_crt_once();
    NativeEntityRegistryLockOwner* pending_lock_00924810();
    NativeEntityRegistryLockOwner* scene_lock_00928240();
    void enqueue_pending_init_00926be0(void* actual_entity_root);
    ObjectHandleTables handle_tables() const noexcept;
    NativeEntityRegistryProcess& owner() noexcept;
private:
    friend GameNativeEntityRegistryProcess& game_native_entity_registry_process();
    GameNativeEntityRegistryProcess();
    NativeEntityRegistryProcess& owner_;
    void* volatile& manager_;
};
static_assert(std::is_trivially_destructible_v<GameNativeEntityRegistryProcess>);
GameNativeEntityRegistryProcess& game_native_entity_registry_process();
} // namespace bsp::game
