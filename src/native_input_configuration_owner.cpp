#include "bsp/native_input_configuration_owner.hpp"
#include "bsp/native_lua_objects.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstdint>
#include <initializer_list>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native input configuration ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
template<class T> T read(void* p, std::uint32_t offset) noexcept {
    return *static_cast<const volatile T*>(at(p, offset));
}
template<class T> void write(void* p, std::uint32_t offset, T value) noexcept {
    *static_cast<volatile T*>(at(p, offset)) = value;
}
void clear_triplet(void* header) noexcept {
    write<void*>(header, 4, nullptr);
    write<void*>(header, 8, nullptr);
    write<void*>(header, 0xc, nullptr);
}
void release_flat_buffer(void* header) noexcept {
    void* const begin = read<void*>(header, 4);
    if (begin) singleton_lifetime_free(begin);
    clear_triplet(header);
}
// Source adapter for the consumed4D49B0 destruction library contract. Native
// ECX/EDX capture begin/end; two unused stack words are consumed by RET8.
// Retain that endpoint and each10h row's opaque+0; no header/capacity guard or
// original STL function/name is supplied by this source-only helper.
void destroy_checked_dword_rows(void* row, void* const end) noexcept {
    while (row != end) {
        release_flat_buffer(row);
        row = at(row, 0x10);
    }
}
} // namespace

void* construct_native_input_configuration_00698680(void* configuration) noexcept {
    construct_native_lua_state_00b66bd0(configuration);
    for (std::uint32_t offset : {0x4d4u, 0x4d8u, 0x4dcu, 0x4e4u, 0x4e8u,
            0x4ecu, 0x4f4u, 0x4f8u, 0x4fcu, 0x504u, 0x508u, 0x50cu,
            0x514u, 0x518u, 0x51cu}) {
        write<std::uint32_t>(configuration, offset, 0);
    }
    for (std::uint32_t offset : {0x4c8u, 0x4cbu, 0x4cau, 0x4c9u, 0x520u}) {
        write<std::uint8_t>(configuration, offset, 0);
    }
    return configuration;
}

void destroy_native_input_configuration_004dceb0(void* configuration) {
    release_flat_buffer(at(configuration, 0x510));
    release_flat_buffer(at(configuration, 0x500));
    release_flat_buffer(at(configuration, 0x4f0));
    release_flat_buffer(at(configuration, 0x4e0));

    void* const outer = at(configuration, 0x4d0);
    void* const begin = read<void*>(outer, 4);
    if (begin) {
        void* const end = read<void*>(outer, 8);
        destroy_checked_dword_rows(begin, end);
        singleton_lifetime_free(read<void*>(outer, 4));
    }
    clear_triplet(outer);
    close_native_lua_state_00b669a0(*static_cast<NativeLuaStateStorage*>(configuration));
}
} // namespace bsp
