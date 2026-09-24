#include "bsp/native_gui_group_storage.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI Group storage requires MSVC Win32.
#endif
namespace bsp::game {
GameNativeGuiGroupPoolProcess::GameNativeGuiGroupPoolProcess()
    : pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
        storage_00f8bfa0_) {}
GameNativeGuiGroupPoolProcess& game_native_gui_group_pool_process() {
    static GameNativeGuiGroupPoolProcess process;
    return process;
}
int GameNativeGuiGroupPoolProcess::initialize_once_00cd7440() {
    std::lock_guard lock(startup_mutex_);
    if (state_==State::returned) return registration_status_;
    if (state_==State::threw) throw std::logic_error("GUI Group pool startup previously threw");
    state_=State::threw;
    pool_.initialize_00ac7410();
    registration_status_=std::atexit(&GameNativeGuiGroupPoolProcess::destroy_00ce0ae0);
    state_=State::returned;
    return registration_status_;
}
NativeGuiGroupPool& GameNativeGuiGroupPoolProcess::pool_00f8bfa0() {
    std::lock_guard lock(startup_mutex_);
    if (state_!=State::returned) throw std::logic_error("GUI Group pool requires completed explicit startup");
    return pool_;
}
void GameNativeGuiGroupPoolProcess::destroy_00ce0ae0() noexcept {
    game_native_gui_group_pool_process().pool_.destroy_00ac71a0();
}
} // namespace bsp::game
namespace bsp {
void* allocate_native_gui_group_slot_00ac76d0() {
    return game::game_native_gui_group_pool_process().pool_00f8bfa0().allocate_00ac7590();
}
void return_native_gui_group_slot_00ac73d0(void* slot) {
    game::game_native_gui_group_pool_process().pool_00f8bfa0().return_00ac7260(slot);
}
} // namespace bsp
