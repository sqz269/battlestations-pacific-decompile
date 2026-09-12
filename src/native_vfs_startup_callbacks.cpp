#include "bsp/native_vfs_startup_callbacks.hpp"

#include <cstdint>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS startup callback storage requires MSVC Win32.
#endif

namespace bsp {

void ignore_native_vfs_mount_failure_00530620() noexcept {}
void ignore_native_vfs_callback_00735b30() noexcept {}

void install_native_vfs_startup_callbacks_0073d63c(
    void* volatile& publication) noexcept {
    auto* const first = static_cast<unsigned char*>(publication);
    *reinterpret_cast<volatile std::uint32_t*>(first + 0x90) = 0x00530620;
    auto* const second = static_cast<unsigned char*>(publication);
    *reinterpret_cast<volatile std::uint32_t*>(second + 0x8c) = 0x00735b30;
}

void NativeVfsStartupCallbacks::mount_failure_00be18b8(
    std::uintptr_t target, void*) {
    if (target != 0x00530620) {
        throw std::invalid_argument("Unimplemented native VFS startup mount callback");
    }
    ignore_native_vfs_mount_failure_00530620();
}

} // namespace bsp
