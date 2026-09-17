#include "bsp/game_native_shader_process.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include <stdexcept>

namespace bsp::game {
GameNativeShaderProcess::GameNativeShaderProcess()
    :pool_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),pool_storage_) {}
GameNativeShaderProcess& game_native_shader_process() {
    static auto* const process=new GameNativeShaderProcess;
    return *process;
}
int GameNativeShaderProcess::initialize_state_pool_once_00cd7cc0() {
    std::lock_guard lock(mutex_);
    if(state_==State::returned)return registration_status_;
    if(state_==State::threw)throw std::logic_error("shader state-list pool startup previously threw");
    state_=State::threw;
    bind_static_native_shader_state_list_pool_0108fee4(pool_);
    registration_status_=initialize_static_native_shader_state_list_pool_00cd7cc0();
    state_=State::returned;return registration_status_;
}
NativeShaderStateListPool& GameNativeShaderProcess::state_list_pool_0108fee4() {
    std::lock_guard lock(mutex_);
    if(state_!=State::returned)throw std::logic_error("shader state-list pool requires completed startup");
    return pool_;
}
void GameNativeShaderProcess::configure_startup_modes(const char* mode,GameNativeReadOnlyData& data) {
    const NativeStartupShaderModeLiterals literals{
        static_cast<const char*>(data.data_at(0x00ce7f7c,11)),
        static_cast<const char*>(data.data_at(0x00ce7f70,11)),
        static_cast<const char*>(data.data_at(0x00ce7f98,10)),
        static_cast<const char*>(data.data_at(0x00ce7f88,16)),
        static_cast<const char*>(data.data_at(0x00ce7f68,6))};
    apply_native_startup_shader_modes_0073d4c2(mode,modes_,literals,
        game_native_string_process().raw_context(),mode_operation_);
}
} // namespace bsp::game
