#include "bsp/native_record_vector_tidy.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
void __fastcall tidy_native_record_vector_0041f950(void* actual_receiver) {
    static_assert(sizeof(void*) == 4);
    auto* const bytes = static_cast<char*>(actual_receiver);
    void* buffer;
    std::memcpy(&buffer, bytes + 0x60, sizeof(buffer));
    if (buffer != nullptr)
        singleton_lifetime_free(buffer);

    const std::uint32_t zero = 0;
    std::memcpy(bytes + 0x60, &zero, sizeof(zero));
    std::memcpy(bytes + 0x64, &zero, sizeof(zero));
    std::memcpy(bytes + 0x68, &zero, sizeof(zero));
}
} // namespace bsp
