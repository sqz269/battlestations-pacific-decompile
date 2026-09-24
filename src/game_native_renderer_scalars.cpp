#include "bsp/game_native_renderer_scalars.hpp"
#include <Windows.h>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Game native renderer scalar process requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
GameNativeRendererAtomicImport resolve_increment() {
    const auto address=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"InterlockedIncrement");
    if(!address)throw std::runtime_error("missing actual InterlockedIncrement export");
    GameNativeRendererAtomicImport result;
    static_assert(sizeof(result)==sizeof(address));
    std::memcpy(&result,&address,sizeof(result));
    return result;
}
GameNativeRendererSectionImport resolve_section(const char* name) {
    const auto address=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),name);
    if(!address)throw std::runtime_error("missing actual critical-section import");
    GameNativeRendererSectionImport result;
    static_assert(sizeof(result)==sizeof(address));
    std::memcpy(&result,&address,sizeof(result));
    return result;
}
} // namespace

GameNativeRendererScalarProcess::GameNativeRendererScalarProcess()
    :increment_iat_00ce221c_(resolve_increment()),
      enter_iat_00ce2218_(resolve_section("EnterCriticalSection")),
      leave_iat_00ce2210_(resolve_section("LeaveCriticalSection")) {}

GameNativeRendererScalarProcess& game_native_renderer_scalar_process() {
    static GameNativeRendererScalarProcess process;
    return process;
}

NativeRendererSynchronizationGlobals&
GameNativeRendererScalarProcess::synchronization_0108d6dc() noexcept {
    return synchronization_0108d6dc_;
}

volatile std::uint32_t&
GameNativeRendererScalarProcess::texture_allocation_bytes_0108d4bc() noexcept {
    return texture_allocation_bytes_0108d4bc_;
}

volatile std::uint32_t&
GameNativeRendererScalarProcess::surface_allocation_bytes_0108d4c0() noexcept {
    return surface_allocation_bytes_0108d4c0_;
}

GameNativeRendererAtomicImport volatile&
GameNativeRendererScalarProcess::increment_iat_00ce221c() noexcept {
    return increment_iat_00ce221c_;
}

void* volatile&
GameNativeRendererScalarProcess::shadow_target_publication_00f8bbf0() noexcept {
    return shadow_target_publication_00f8bbf0_;
}
GameNativeRendererSectionImport volatile&
GameNativeRendererScalarProcess::enter_iat_00ce2218() noexcept {
    return enter_iat_00ce2218_;
}
GameNativeRendererSectionImport volatile&
GameNativeRendererScalarProcess::leave_iat_00ce2210() noexcept {
    return leave_iat_00ce2210_;
}

std::uint32_t&
GameNativeRendererScalarProcess::renderer_worker_time_bits_0108d6e4() noexcept {
    return renderer_worker_time_bits_0108d6e4_;
}

std::uint32_t& GameNativeRendererScalarProcess::logical_texture_serial_0108d6e8() noexcept {
    return logical_texture_serial_0108d6e8_;
}

std::uint32_t&
GameNativeRendererScalarProcess::texture_tracking_counter_0108daf8() noexcept {
    return texture_tracking_counter_0108daf8_;
}

std::uint32_t&
GameNativeRendererScalarProcess::surface_tracking_counter_0108dafc() noexcept {
    return surface_tracking_counter_0108dafc_;
}

} // namespace bsp::game
