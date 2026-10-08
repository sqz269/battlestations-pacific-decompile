#include "bsp/game_native_entity_registry_process.hpp"
#include "bsp/game_native_string_process.hpp"

namespace bsp::game {
GameNativeEntityRegistryProcess::GameNativeEntityRegistryProcess()
    : owner_(native_entity_registry_process()),
      manager_(game_native_string_process().manager_01090aa0()) {}

GameNativeEntityRegistryProcess& game_native_entity_registry_process() {
    // Trivial destruction keeps references/storage valid for later CRT calls.
    static GameNativeEntityRegistryProcess process;
    return process;
}

NativeEntityRegistryCrtStatuses GameNativeEntityRegistryProcess::initialize_crt_once() {
    return owner_.initialize_crt_once();
}
NativeEntityRegistryLockOwner* GameNativeEntityRegistryProcess::pending_lock_00924810() {
    return get_native_pending_init_lock_00924810(manager_, owner_.pending_lock_00f899e4());
}
NativeEntityRegistryLockOwner* GameNativeEntityRegistryProcess::scene_lock_00928240() {
    return get_native_scene_registry_lock_00928240(manager_, owner_.scene_lock_00f899fc());
}
void GameNativeEntityRegistryProcess::enqueue_pending_init_00926be0(void* actual_entity_root) {
    enqueue_native_pending_init_00926be0(owner_, actual_entity_root, manager_);
}
ObjectHandleTables GameNativeEntityRegistryProcess::handle_tables() const noexcept {
    return owner_.handle_tables();
}
NativeEntityRegistryProcess& GameNativeEntityRegistryProcess::owner() noexcept {
    return owner_;
}
} // namespace bsp::game
