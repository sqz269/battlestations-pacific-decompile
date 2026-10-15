#include "bsp/native_vector_storage_tidy.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>

namespace bsp {

static_assert(sizeof(std::uintptr_t) == 4);

void __fastcall tidy_native_vector_storage_00468840(void* actual_storage16) {
    const auto storage = reinterpret_cast<std::uintptr_t>(actual_storage16);
    const auto captured = *reinterpret_cast<const volatile std::uint32_t*>(storage + 4u);
    if (captured != 0)
        singleton_lifetime_free(reinterpret_cast<void*>(captured));
    *reinterpret_cast<volatile std::uint32_t*>(storage + 4u) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(storage + 8u) = 0;
    *reinterpret_cast<volatile std::uint32_t*>(storage + 12u) = 0;
}

} // namespace bsp
