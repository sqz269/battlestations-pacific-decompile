#include "bsp/native_pool_storage_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
void __fastcall cleanup_native_pool_storage_0041f820(
    void* actual_record, NativeStringRawPoolContext& context) {
    static_assert(sizeof(void*) == 4);
    auto* const bytes = static_cast<char*>(actual_record);
    void* buffer;
    std::memcpy(&buffer, bytes + 0x10, sizeof(buffer));
    if (buffer != nullptr)
        singleton_lifetime_free(buffer);

    const std::uint32_t zero = 0;
    std::memcpy(bytes + 0x10, &zero, sizeof(zero));
    std::memcpy(bytes + 0x14, &zero, sizeof(zero));
    std::memcpy(bytes + 0x18, &zero, sizeof(zero));

    // The established 8-byte header body has the identical access schedule:
    // capture data, guard it, capture length+1, get pool, then return block.
    // It does not clear the header and does not cache the pool publication.
    destroy_native_string_header_0041dd20(bytes + 4, context);
}
} // namespace bsp
