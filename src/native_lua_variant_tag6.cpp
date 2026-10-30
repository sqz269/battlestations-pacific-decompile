#include "bsp/native_lua_variant_tag6.hpp"

#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace bsp {

NativeLuaVariantRetirementResult initialize_native_lua_variant_tag6_0074fb30(
    void* actual_variant, NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup) {
    static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
    static_assert(sizeof(std::uint32_t) == 4);

    void* const captured_receiver = actual_variant;
    const NativeLuaVariantRetirementResult result =
        clear_native_lua_variant_00886920_cpp_cleanup(
            captured_receiver, strings, operation, cleanup);
    if (result != NativeLuaVariantRetirementResult::completed) return result;

    const std::uint32_t address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(captured_receiver)) + std::uint32_t{4};
    const std::uint32_t tag = 6;
    _ReadWriteBarrier();
    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(address)),
        &tag, sizeof(tag));
    _ReadWriteBarrier();
    return NativeLuaVariantRetirementResult::completed;
}

} // namespace bsp
