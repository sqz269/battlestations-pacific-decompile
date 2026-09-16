#include "bsp/game_native_graphics_pools.hpp"

#include "bsp/game_native_physical_pool.hpp"
#include "bsp/native_cube_texture_pool_static.hpp"
#include "bsp/native_hardware_layout_pool_static.hpp"
#include "bsp/native_volume_texture_pool_static.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native graphics pools require MSVC Win32.
#endif

namespace bsp::game {

GameNativeGraphicsPoolProcess::GameNativeGraphicsPoolProcess()
    : list_(game_native_physical_pool_process().allocator_list_domain_00e188b4()),
      volume_(list_, volume_storage_0108dba8_), pass_(list_, pass_storage_0108fbf8_),
      graphics_{{
          {list_, graphics_storage_[0], NativeGraphicsPoolKind::vertex_declaration},
          {list_, graphics_storage_[1], NativeGraphicsPoolKind::hardware_layout},
          {list_, graphics_storage_[2], NativeGraphicsPoolKind::physical_buffer},
          {list_, graphics_storage_[3], NativeGraphicsPoolKind::physical_buffer},
          {list_, graphics_storage_[4], NativeGraphicsPoolKind::logical_vertex},
          {list_, graphics_storage_[5], NativeGraphicsPoolKind::logical_index}}} {}

GameNativeGraphicsPoolProcess& game_native_graphics_pool_process() {
    static GameNativeGraphicsPoolProcess process;
    return process;
}

std::size_t GameNativeGraphicsPoolProcess::index_of(GameNativeGraphicsPool kind) {
    switch (kind) {
    case GameNativeGraphicsPool::cube_texture: return 0;
    case GameNativeGraphicsPool::volume_texture: return 1;
    case GameNativeGraphicsPool::material_pass: return 2;
    case GameNativeGraphicsPool::vertex_declaration: return 3;
    case GameNativeGraphicsPool::layout_record: return 4;
    case GameNativeGraphicsPool::physical_index: return 5;
    case GameNativeGraphicsPool::physical_vertex: return 6;
    case GameNativeGraphicsPool::logical_vertex: return 7;
    case GameNativeGraphicsPool::logical_index: return 8;
    case GameNativeGraphicsPool::hardware_layout: return 9;
    }
    throw std::invalid_argument("unknown native graphics pool global");
}

int GameNativeGraphicsPoolProcess::initialize_once(GameNativeGraphicsPool kind) {
    const auto index = index_of(kind);
    std::lock_guard lock(startup_mutex_);
    if (states_[index] == StartupState::returned) return registration_status_[index];
    if (states_[index] == StartupState::threw)
        throw std::logic_error("native graphics pool startup previously threw");
    states_[index] = StartupState::threw;
    if (index >= 3 && index <= 8)
        bind_static_native_graphics_pool(static_cast<NativeGraphicsPoolGlobal>(index - 3),
            graphics_[index - 3]);
    int status;
    switch (kind) {
    case GameNativeGraphicsPool::cube_texture:
        bind_static_native_cube_texture_pool_0108db70(cube_storage_0108db70_.data(), list_);
        status = initialize_static_native_cube_texture_pool_00cd7b80();
        break;
    case GameNativeGraphicsPool::volume_texture:
        bind_static_native_volume_texture_pool_0108dba8(volume_);
        status = initialize_static_native_volume_texture_pool_00cd7ba0();
        break;
    case GameNativeGraphicsPool::material_pass:
        bind_static_native_material_pass_pool_0108fbf8(pass_);
        status = initialize_static_native_material_pass_pool_00cd7bc0();
        break;
    case GameNativeGraphicsPool::vertex_declaration:
        status = initialize_static_native_vertex_declaration_pool_00cd7be0();
        break;
    case GameNativeGraphicsPool::layout_record:
        status = initialize_static_native_hardware_layout_pool_00cd7c00();
        break;
    case GameNativeGraphicsPool::physical_index:
        status = initialize_static_native_physical_index_pool_00cd7c20();
        break;
    case GameNativeGraphicsPool::physical_vertex:
        status = initialize_static_native_physical_vertex_pool_00cd7c40();
        break;
    case GameNativeGraphicsPool::logical_vertex:
        status = initialize_static_native_logical_vertex_pool_00cd7c60();
        break;
    case GameNativeGraphicsPool::logical_index:
        status = initialize_static_native_logical_index_pool_00cd7c80();
        break;
    case GameNativeGraphicsPool::hardware_layout:
        bind_static_native_hardware_layout_pool_0108fe9c(hardware_storage_0108fe9c_.data(), list_);
        status = initialize_static_native_hardware_layout_pool_00cd7ca0();
        break;
    default: throw std::invalid_argument("unknown native graphics pool global");
    }
    registration_status_[index] = status;
    states_[index] = StartupState::returned;
    return status;
}

void GameNativeGraphicsPoolProcess::require_returned(std::size_t index) {
    std::lock_guard lock(startup_mutex_);
    if (states_[index] != StartupState::returned)
        throw std::logic_error("native graphics pool requires completed explicit startup");
}

NativeGraphicsPoolStorage& GameNativeGraphicsPoolProcess::graphics_storage(std::size_t index) {
    require_returned(index + 3);
    return graphics_storage_[index];
}

void* GameNativeGraphicsPoolProcess::cube_texture_pool_0108db70() {
    require_returned(0); return cube_storage_0108db70_.data();
}
D3D9SurfacePool& GameNativeGraphicsPoolProcess::volume_texture_pool_0108dba8() {
    require_returned(1); return volume_;
}
NativeMaterialPassPool& GameNativeGraphicsPoolProcess::material_pass_pool_0108fbf8() {
    require_returned(2); return pass_;
}
NativeGraphicsPoolStorage& GameNativeGraphicsPoolProcess::vertex_declaration_pool_0108fd38() {
    return graphics_storage(0);
}
NativeGraphicsPoolStorage& GameNativeGraphicsPoolProcess::layout_record_pool_0108fd70() {
    return graphics_storage(1);
}
NativeGraphicsPoolStorage& GameNativeGraphicsPoolProcess::physical_index_pool_0108fda8() {
    return graphics_storage(2);
}
NativeGraphicsPoolStorage& GameNativeGraphicsPoolProcess::physical_vertex_pool_0108fde0() {
    return graphics_storage(3);
}
NativeGraphicsPoolStorage& GameNativeGraphicsPoolProcess::logical_vertex_pool_0108fe18() {
    return graphics_storage(4);
}
NativeGraphicsPoolStorage& GameNativeGraphicsPoolProcess::logical_index_pool_0108fe50() {
    return graphics_storage(5);
}
void* GameNativeGraphicsPoolProcess::hardware_layout_pool_0108fe9c() {
    require_returned(9); return hardware_storage_0108fe9c_.data();
}

} // namespace bsp::game
