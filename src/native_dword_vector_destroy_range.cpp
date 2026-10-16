#include "bsp/native_dword_vector_destroy_range.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(std::uintptr_t) == 4);

void __fastcall destroy_native_dword_vector_header_range_004d49b0(
    void* first, const void* last,
    std::uint32_t /*unused_metadata0*/, std::uint32_t /*unused_metadata1*/) {
    const auto captured_last = reinterpret_cast<std::uintptr_t>(last);
    auto current = reinterpret_cast<std::uintptr_t>(first);
    while (current != captured_last) {
        const auto backing =
            *reinterpret_cast<const volatile std::uint32_t*>(current + 4u);
        if (backing != 0u)
            singleton_lifetime_free(reinterpret_cast<void*>(backing));
        *reinterpret_cast<volatile std::uint32_t*>(current + 4u) = 0u;
        *reinterpret_cast<volatile std::uint32_t*>(current + 8u) = 0u;
        *reinterpret_cast<volatile std::uint32_t*>(current + 12u) = 0u;
        current += 0x10u;
    }
}

void __stdcall destroy_native_dword_vector_header_range_dispatch_004d9e40(
    void* first, const void* last) {
    destroy_native_dword_vector_header_range_004d49b0(first, last, 0u, 0u);
}

} // namespace bsp
