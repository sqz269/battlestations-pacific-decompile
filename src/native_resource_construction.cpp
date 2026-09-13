#include "bsp/native_resource_construction.hpp"

#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
void* at(void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
const void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
std::uint32_t read_word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(base, offset));
}
static_assert(sizeof(void*) == 4);
} // namespace

void* construct_native_resource_base_00b88260(void* object,
    const void* name, NativeStringRawPoolContext& pool) {
    word(object) = 0x00ceb130;
    word(object, 4) = 1;
    void* const destination = at(object, 8);
    const bool same_header = destination == name; // B88296, before header stores.
    word(object) = 0x00d63228;
    // State0 is armed before the header is zeroed. Its sole action is BD30F0.
    try {
        word(destination) = 0;
        word(destination, 4) = 0;
        if (!same_header) {
            resize_native_string_header_0041dd40(destination, pool,
                read_word(name), true);
            if (read_word(name) != 0) {
                const auto count = read_word(destination);
                const void* const source = reinterpret_cast<const void*>(read_word(name, 4));
                void* const target = reinterpret_cast<void*>(read_word(destination, 4));
                // Existing raw-pool BF7680 policy: admit overlap and omit zero
                // bytes after the original header reads; no CRT algorithm port.
                if (count != 0) std::memmove(target, source, count);
            }
        }
        word(object, 0x10) = 0;
        word(object, 0x14) = 0;
        word(object, 0x18) = 0;
        word(object, 0x1c) = 0;
        word(object, 0x20) = 0;
        word(object, 0x24) = 0;
        word(object, 0x28) = 0; // Six native MOVSS positive-zero stores.
        word(object, 0x2c) = 0;
        word(object, 0x30) = 0;
        word(object, 0x34) = 0;
        word(object, 0x38) = 0;
        word(object, 0x3c) = 0;
        word(object, 0x40) = 0;
    } catch (...) {
        destroy_native_ref_counted_base_00bd30f0(object);
        throw;
    }
    return object;
}

void* construct_native_game_resource_0071b810(void* object,
    const void* name, NativeStringRawPoolContext& pool) {
    construct_native_resource_base_00b88260(object, name, pool);
    word(object) = 0x00cfd8cc;
    word(object, 0x48) = 0;
    word(object, 0x4c) = 0;
    word(object, 0x50) = 0;
    word(object, 0x58) = 0;
    word(object, 0x5c) = 0;
    word(object, 0x60) = 0;
    word(object, 0x68) = 0;
    word(object, 0x6c) = 0;
    word(object, 0x70) = 0;
    return object;
}

void* create_native_game_resource_0071b870(const void* name,
    NativeStringRawPoolContext& pool) {
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x74, 0x74});
    try {
        if (allocation)
            return construct_native_game_resource_0071b810(allocation, name, pool);
    } catch (...) {
        singleton_lifetime_free(allocation);
        throw;
    }
    return nullptr;
}

void* create_native_default_resource_00b88340(const void* name,
    NativeStringRawPoolContext& pool) {
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, 0x44, 0x44});
    try {
        if (allocation)
            return construct_native_resource_base_00b88260(allocation, name, pool);
    } catch (...) {
        singleton_lifetime_free(allocation);
        throw;
    }
    return nullptr;
}
} // namespace bsp
