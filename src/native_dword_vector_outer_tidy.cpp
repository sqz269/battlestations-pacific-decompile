#include "bsp/native_dword_vector_outer_tidy.hpp"

#include "bsp/native_dword_vector_destroy_range.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(std::uintptr_t) == 4);

void __fastcall tidy_native_dword_vector_outer_storage_004dc1c0(
    void* actual_header, std::uint32_t /*unused_edx*/) {
    const auto header = reinterpret_cast<std::uintptr_t>(actual_header);
    const auto initial_begin =
        *reinterpret_cast<const volatile std::uint32_t*>(header + 4u);
    if (initial_begin != 0u) {
        const auto captured_end =
            *reinterpret_cast<const volatile std::uint32_t*>(header + 8u);
        const auto header_bits = static_cast<std::uint32_t>(header);
        destroy_native_dword_vector_header_range_004d49b0(
            reinterpret_cast<void*>(initial_begin),
            reinterpret_cast<const void*>(captured_end), header_bits, header_bits);
        const auto current_begin =
            *reinterpret_cast<const volatile std::uint32_t*>(header + 4u);
        singleton_lifetime_free(reinterpret_cast<void*>(current_begin));
    }
    *reinterpret_cast<volatile std::uint32_t*>(header + 4u) = 0u;
    *reinterpret_cast<volatile std::uint32_t*>(header + 8u) = 0u;
    *reinterpret_cast<volatile std::uint32_t*>(header + 12u) = 0u;
}

void __fastcall tidy_native_dword_vector_outer_storage_004dca30(
    void* actual_header, std::uint32_t /*unused_edx*/) {
    tidy_native_dword_vector_outer_storage_004dc1c0(actual_header, 0u);
}

} // namespace bsp
