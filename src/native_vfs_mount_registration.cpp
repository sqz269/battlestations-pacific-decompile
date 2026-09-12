#include "bsp/native_vfs_mount_registration.hpp"

#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_vfs_factory_selection.hpp"
#include "bsp/native_vfs_mount_insert.hpp"
#include "bsp/native_vfs_mount_records.hpp"

#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
void* at(const void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(owner, offset));
}
void put_word(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(owner, offset)) = value;
}
void release_header(void* header, NativePathCanonicalizerServices& services) {
    void* const captured_data = reinterpret_cast<void*>(word(header, 4));
    if (!captured_data) return;
    const auto bytes = word(header) + 1u;
    auto* const pool = services.string_pool_00419cc0();
    services.return_string_00bd1510(pool, captured_data, bytes, 1);
}
}

void register_native_vfs_mount_00be1740(void* manager, void* provider,
    const void* prefix, std::uint32_t priority, std::uint32_t flags,
    NativePathCanonicalizerContext& context) {
    alignas(4) std::byte canonical[8];
    alignas(4) std::byte payload[0x10];
    alignas(4) std::byte record[0x14];
    alignas(4) std::byte copied_record[0x14];
    alignas(4) std::byte inserted[0x0c];
    canonicalize_native_path_00bee390(canonical, prefix, context);
    int state = 0; // E00FA0: canonical, record, copied record; no payload state.
    try {
        trim_native_string_right_00584110(canonical, "/", context.strings);
        // BE1782..BE17C4: these two local headers are distinct. Preserve the
        // current post-allocation reads and BF7680's verified overlap support.
        put_word(payload, 0, 0);
        put_word(payload, 4, 0);
        resize_native_string_header_0041dd40(payload, context.strings, word(canonical), true);
        if (word(canonical) != 0) {
            const auto length = word(payload);
            const auto* const from = reinterpret_cast<const void*>(word(canonical, 4));
            auto* const to = reinterpret_cast<void*>(word(payload, 4));
            if (length != 0) std::memmove(to, from, length);
        }
        put_word(payload, 8, reinterpret_cast<std::uint32_t>(provider));
        *static_cast<volatile std::uint8_t*>(at(payload, 0x0c)) =
            static_cast<std::uint8_t>(flags);
        // BDEEC0 owns destruction of the by-value payload, including its unwind.
        construct_native_vfs_priority_record_00bdeec0(record, priority, payload, context.strings);
        state = 1;
        copy_native_vfs_mount_record_00bdce40(copied_record, record, context.strings);
        state = 2;
        insert_native_vfs_mount_record_00be1330(at(manager, 0x3c), inserted,
            copied_record, context.strings);
        state = 1;
        release_header(at(copied_record, 4), context.services);
        state = 0;
        release_header(at(record, 4), context.services);
        state = -1;
        release_header(canonical, context.services);
    } catch (...) {
        // CC6890 -> BDB590, CC6888 -> BDB570, CC6880 -> 41DD20.
        // Those bodies release only the same actual embedded string headers.
        if (state >= 2) {
            state = 1;
            release_header(at(copied_record, 4), context.services);
        }
        if (state >= 1) {
            state = 0;
            release_header(at(record, 4), context.services);
        }
        if (state >= 0) release_header(canonical, context.services);
        throw;
    }
}

void* mount_native_vfs_system_path_00be1890(void* manager,
    const void* system, const void* virtual_name, std::uint32_t priority,
    std::uint32_t flags, std::uint32_t device_id,
    NativeVfsMountRegistrationContext& context) {
    const auto table = word(manager);
    const auto selected = word(reinterpret_cast<void*>(table), 0x18);
    if (selected != 0x00bdb040)
        throw std::invalid_argument("Unimplemented current native VFS factory selector");
    void* const provider = select_native_vfs_factory_00bdb040(manager, system,
        virtual_name, context.factories, context.invalid_parameters);
    if (!provider) {
        void* const current_manager = context.manager_0109ceec;
        const auto callback = word(current_manager, 0x90);
        context.failure.mount_failure_00be18b8(callback, current_manager);
        return nullptr;
    }
    put_word(provider, 0x10, device_id);
    // BE18CA..BE18EB reads these headers and substitutes 0109CEF0 for null.
    // 4254B0 is a verified single RET, so no pointed-to character is read.
    const auto virtual_data = word(virtual_name, 4);
    const auto system_data = word(system, 4);
    const auto virtual_argument = virtual_data ? virtual_data : 0x0109cef0u;
    const auto system_argument = system_data ? system_data : 0x0109cef0u;
    (void)virtual_argument;
    (void)system_argument;
    register_native_vfs_mount_00be1740(manager, provider, virtual_name, priority,
        flags, context.canonicalizer);
    return provider;
}
} // namespace bsp
