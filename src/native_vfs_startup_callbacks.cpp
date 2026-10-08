#include "bsp/native_vfs_startup_callbacks.hpp"
#include "bsp/native_physical_failure_entries.hpp"

#include <cstdint>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS startup callback storage requires MSVC Win32.
#endif

namespace bsp {

void ignore_native_vfs_mount_failure_00530620() noexcept {}
void ignore_native_vfs_callback_00735b30() noexcept {}

std::uintptr_t qualified_native_vfs_raw_failure_target() {
    static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4,
        "Native VFS failure publication requires four-byte pointers");
    const auto target = reinterpret_cast<std::uintptr_t>(
        &raw_ignore_native_vfs_mount_failure_00530620);
    if (*reinterpret_cast<const volatile std::uint8_t*>(target) != 0xc3) {
        throw std::logic_error("Native VFS raw failure callback is not the qualified C3 entry");
    }
    return target;
}

bool invoke_native_vfs_startup_failure_target(std::uintptr_t target) {
    if (target == 0x00530620) {
        ignore_native_vfs_mount_failure_00530620();
        return true;
    }
    if (target != reinterpret_cast<std::uintptr_t>(
            &raw_ignore_native_vfs_mount_failure_00530620)) {
        return false;
    }
    (void)qualified_native_vfs_raw_failure_target();
    raw_ignore_native_vfs_mount_failure_00530620();
    return true;
}

void install_native_vfs_startup_callbacks_0073d63c(
    void* volatile& publication) noexcept {
    auto* const first = static_cast<unsigned char*>(publication);
    *reinterpret_cast<volatile std::uint32_t*>(first + 0x90) = 0x00530620;
    auto* const second = static_cast<unsigned char*>(publication);
    *reinterpret_cast<volatile std::uint32_t*>(second + 0x8c) = 0x00735b30;
}

void NativeVfsStartupCallbacks::mount_failure_00be18b8(
    std::uintptr_t target, void*) {
    if (!invoke_native_vfs_startup_failure_target(target)) {
        throw std::invalid_argument("Unimplemented native VFS startup mount callback");
    }
}

} // namespace bsp
