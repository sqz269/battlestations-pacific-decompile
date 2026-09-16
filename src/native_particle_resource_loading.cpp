#include "bsp/native_particle_resource_loading.hpp"

#include "bsp/native_ref_counted.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

namespace bsp {
namespace {
void* at(const void* storage, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(const_cast<void*>(storage)) + offset;
}
template<class T> volatile T& field(const void* storage, std::size_t offset = 0) noexcept {
    return *reinterpret_cast<volatile T*>(at(storage, offset));
}
struct ResourceUnwind {
    void* owner;
    NativeStringRawPoolContext& strings;
    bool armed = true;
    ~ResourceUnwind() noexcept {
        if (armed) {
            // DF2A30 state1 ->0 ->-1: current name, then refcounted base.
            // A second exception terminates before any later unwind action.
            destroy_native_string_header_0041dd20(at(owner, 8), strings);
            destroy_native_ref_counted_base_00bd30f0(owner);
        }
    }
};
} // namespace

void* construct_native_particle_resource_00af45d0(
    void* owner, const void* name, NativeStringRawPoolContext& strings) {
    field<std::uint32_t>(owner) = 0x00ceb130u;
    field<std::uint32_t>(owner, 4) = 1;
    field<std::uint32_t>(owner) = 0x00d5d958u;
    void* const destination = at(owner, 8);
    field<std::uint32_t>(destination) = 0;
    field<void*>(destination, 4) = nullptr;
    const bool same = destination == name;
    field<std::uint32_t>(owner, 0x30) = 0;
    ResourceUnwind cleanup{owner, strings};
    field<std::uint32_t>(owner, 0x54) = 0;
    if (!same) {
        resize_native_string_header_0041dd40(destination, strings,
            field<std::uint32_t>(name), true);
        if (field<std::uint32_t>(name) != 0) {
            const auto count = field<std::uint32_t>(destination);
            void* const source_data = field<void*>(name, 4);
            void* const output_data = field<void*>(destination, 4);
            if (count != 0) std::memmove(output_data, source_data, count);
        }
    }
    // Immutable .rdata CE3D08=100, CFA424=3000, CEB4B0=60. Preserve bits;
    // native MOVSS performs no arithmetic or x87 conversion here.
    field<std::uint32_t>(owner, 0x60) = 0x42c80000u;
    field<std::uint32_t>(owner, 0x68) = 0x453b8000u;
    field<std::uint32_t>(owner, 0x6c) = 0x453b8000u;
    field<std::uint32_t>(owner, 0x74) = 0x42700000u;
    field<std::uint8_t>(owner, 0x65) = 0;
    field<std::uint8_t>(owner, 0x64) = 0;
    field<std::uint8_t>(owner, 0x66) = 0;
    field<std::uint8_t>(owner, 0x70) = 0;
    field<std::uint8_t>(owner, 0x78) = 0;
    field<std::uint8_t>(owner, 0x79) = 0;
    field<std::uint8_t>(owner, 0x7a) = 0;
    field<std::uint32_t>(owner, 0x58) = 1;
    field<std::uint32_t>(owner, 0x5c) = 30;
    field<std::uint32_t>(owner, 0x8c) = 0;
    field<std::uint32_t>(owner, 0x7c) = 0;
    field<std::uint32_t>(owner, 0x80) = 0;
    field<std::uint32_t>(owner, 0x84) = 0;
    field<std::uint32_t>(owner, 0x88) = 0;
    cleanup.armed = false;
    return owner;
}

void destroy_native_particle_text_buffer_00af5620(
    void* text, NativeStringRawPoolContext& strings) {
    void* const allocation = field<void*>(text, 0x14);
    if (allocation != nullptr) singleton_lifetime_free(allocation);
    destroy_native_string_header_0041dd20(at(text, 0x0c), strings);
}
} // namespace bsp
