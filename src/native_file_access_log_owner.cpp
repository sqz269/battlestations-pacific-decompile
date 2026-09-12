#include "bsp/native_file_access_log_owner.hpp"

#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native file-access-log ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
void set_table(void* owner, std::uint32_t table) noexcept {
    std::memcpy(owner, &table, sizeof table);
}
template<class T> T read(const void* p, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(p) + offset, sizeof value);
    return value;
}
} // namespace

void* construct_native_file_access_log_base_007374a0(void* owner,
    NativeFileAccessLogLifetimeBindings& bindings) {
    set_table(owner, 0x00cfea6c);
    try {
        CapturedSoundLifetimeSection section(bindings.lifetime);
        bindings.publication_0109cee8 = owner;
        auto manager = bindings.lifetime.get_manager_00415350();
        manager.register_object(bindings.publication_0109cee8);
    } catch (...) {
        // C86188 releases the guard, then state0/C86180 runs412430.
        set_table(owner, 0x00ce3818);
        throw;
    }
    return owner;
}

void destroy_native_file_access_log_base_00737540(void* owner,
    NativeFileAccessLogLifetimeBindings& bindings) {
    set_table(owner, 0x00cfea6c);
    try {
        CapturedSoundLifetimeSection section(bindings.lifetime);
        auto manager = bindings.lifetime.get_manager_00415350();
        manager.unregister_object(bindings.publication_0109cee8);
        bindings.publication_0109cee8 = nullptr;
    } catch (...) {
        // C861A8 releases the guard, then state0/C861A0 runs412430.
        set_table(owner, 0x00ce3818);
        throw;
    }
    set_table(owner, 0x00ce3818);
}

void* construct_native_file_access_log_00737c40(void* owner, void* subject,
    NativeFileAccessLogLifetimeBindings& bindings, NativeStringStorage& strings) {
    try {
        construct_native_file_access_log_base_007374a0(owner, bindings);
    } catch (...) {
        // C86240 destroys the current stacked header while state0 is armed.
        destroy_native_string_header_0041dd20(subject, strings);
        throw;
    }
    char* const data = read<char*>(subject, 4);
    set_table(owner, 0x00cfeae0);
    // Native state becomes -1 before pool getter/return. The established
    // noexcept release bridge covers returning pool-getter operations.
    if (data) strings.release(data, read<std::uint32_t>(subject, 0) + 1u);
    return owner;
}

void* scalar_delete_native_file_access_log_base_007376a0(void* owner,
    std::uint32_t flags, NativeFileAccessLogLifetimeBindings& bindings) {
    destroy_native_file_access_log_base_00737540(owner, bindings);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

void* scalar_delete_native_file_access_log_00737cc0(void* owner,
    std::uint32_t flags, NativeFileAccessLogLifetimeBindings& bindings) {
    destroy_native_file_access_log_base_00737540(owner, bindings);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

} // namespace bsp
