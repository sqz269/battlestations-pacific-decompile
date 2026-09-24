#include "bsp/native_gui_layer_storage.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include <cstdlib>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native GUI layer storage requires MSVC Win32.
#endif

namespace bsp::game {
GameNativeGuiLayerPoolProcess::GameNativeGuiLayerPoolProcess()
    : pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          storage_00f8bf50_) {}

GameNativeGuiLayerPoolProcess& game_native_gui_layer_pool_process() {
    static GameNativeGuiLayerPoolProcess process;
    return process;
}

int GameNativeGuiLayerPoolProcess::initialize_once_00cd73d0() {
    std::lock_guard lock(startup_mutex_);
    if (state_ == State::returned) return registration_status_;
    if (state_ == State::threw)
        throw std::logic_error("GUI layer pool startup previously threw");
    state_ = State::threw;
    pool_.initialize_00ac4d70();
    registration_status_ = std::atexit(&GameNativeGuiLayerPoolProcess::destroy_00ce0ad0);
    state_ = State::returned;
    return registration_status_;
}

NativeGuiLayerPool& GameNativeGuiLayerPoolProcess::pool_00f8bf50() {
    std::lock_guard lock(startup_mutex_);
    if (state_ != State::returned)
        throw std::logic_error("GUI layer pool requires completed explicit startup");
    return pool_;
}

void GameNativeGuiLayerPoolProcess::destroy_00ce0ad0() noexcept {
    game_native_gui_layer_pool_process().pool_.destroy_00ac4670();
}
} // namespace bsp::game

namespace bsp {
void* allocate_native_gui_layer_slot_00ac51a0() {
    return game::game_native_gui_layer_pool_process().pool_00f8bf50().allocate_00ac4e50();
}
void return_native_gui_layer_slot_00ac4c40(void* raw_slot) {
    game::game_native_gui_layer_pool_process().pool_00f8bf50().return_00ac47f0(raw_slot);
}
} // namespace bsp
