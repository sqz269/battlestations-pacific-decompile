#include "bsp/native_shader_global_readers.hpp"

namespace bsp {
namespace {

// Move the existing production process's live scalar here; its member binds
// a reference to this one authority. The Native image contains DWORD 0x4D.
// This is writable, constant-initialized storage, not a constant getter.
volatile std::uint32_t register_limit_storage_00e13078{0x4dU};
void* volatile system_registry_storage_0108fe94{};

} // namespace

volatile std::uint32_t& native_shader_register_limit_cell_00e13078() noexcept {
    return register_limit_storage_00e13078;
}

std::uint32_t __cdecl native_shader_register_limit_00b343f0() noexcept {
    return register_limit_storage_00e13078;
}

void* volatile& native_shader_system_registry_cell_0108fe94() noexcept {
    return system_registry_storage_0108fe94;
}

void* __cdecl native_shader_system_registry_00b34490() noexcept {
    return system_registry_storage_0108fe94;
}

} // namespace bsp
