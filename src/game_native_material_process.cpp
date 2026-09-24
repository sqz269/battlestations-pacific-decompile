#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_material_factory.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native material process requires MSVC Win32.
#endif

namespace bsp::game {
static_assert(sizeof(void*) == 4 && sizeof(NativeMaterialPoolStorage) == 0x38);

GameNativeMaterialProcess::GameNativeMaterialProcess()
    : materials_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          material_storage_),
      parameters_(game_native_physical_pool_process().allocator_list_domain_00e188b4(),
          parameter_storage_) {}

GameNativeMaterialProcess& game_native_material_process() {
    static GameNativeMaterialProcess process;
    return process;
}

int GameNativeMaterialProcess::initialize_material_once_00cd78d0() {
    std::lock_guard lock(startup_mutex_);
    if (material_state_ == State::returned) return material_registration_status_;
    if (material_state_ == State::threw)
        throw std::logic_error("material pool startup previously threw");
    material_state_ = State::threw;
    bind_static_native_material_pool_00f8d3ac(materials_);
    material_registration_status_ = initialize_static_native_material_pool_00cd78d0();
    material_state_ = State::returned;
    return material_registration_status_;
}

int GameNativeMaterialProcess::initialize_parameters_once_00cd78f0() {
    std::lock_guard lock(startup_mutex_);
    if (parameter_state_ == State::returned) return parameter_registration_status_;
    if (parameter_state_ == State::threw)
        throw std::logic_error("material parameter pool startup previously threw");
    parameter_state_ = State::threw;
    bind_static_native_material_parameter_pool_00f8d3e4(parameters_);
    parameter_registration_status_ = initialize_static_native_material_parameter_pool_00cd78f0();
    parameter_state_ = State::returned;
    return parameter_registration_status_;
}

NativeMaterialPool& GameNativeMaterialProcess::material_pool_00f8d3ac() {
    std::lock_guard lock(startup_mutex_);
    if (material_state_ != State::returned)
        throw std::logic_error("material pool requires completed explicit startup");
    return materials_;
}

NativeMaterialParameterPool& GameNativeMaterialProcess::parameter_pool_00f8d3e4() {
    std::lock_guard lock(startup_mutex_);
    if (parameter_state_ != State::returned)
        throw std::logic_error("material parameter pool requires completed explicit startup");
    return parameters_;
}
} // namespace bsp::game
