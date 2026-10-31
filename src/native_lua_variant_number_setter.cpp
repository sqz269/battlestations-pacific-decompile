#include "bsp/native_lua_variant_number_setter.hpp"

#include <climits>
#include <cstdint>
#include <cstring>
#include <intrin.h>

namespace bsp {
namespace {

static_assert(CHAR_BIT == 8);
static_assert(sizeof(void*) == 4 && sizeof(std::uintptr_t) == 4);
static_assert(sizeof(std::uint32_t) == 4);

std::uint32_t current_word(const void* address) noexcept {
    std::uint32_t bits;
    _ReadWriteBarrier();
    std::memcpy(&bits, address, sizeof bits);
    _ReadWriteBarrier();
    return bits;
}

void put_word(void* address, std::uint32_t bits) noexcept {
    _ReadWriteBarrier();
    std::memcpy(address, &bits, sizeof bits);
    _ReadWriteBarrier();
}

} // namespace

NativeLuaVariantRetirementResult
set_native_lua_variant_number_008869d0_cpp_cleanup(
    void* actual_variant, const void* actual_float32_home,
    NativeStringRawPoolContext& strings,
    NativeLuaVariantRetirementOperation& operation,
    const NativeLuaVariantCppCleanupBacking& cleanup) {
    void* const receiver = actual_variant;
    const void* const home = actual_float32_home;
    const auto result = clear_native_lua_variant_00886920_cpp_cleanup(
        receiver, strings, operation, cleanup);
    if (result != NativeLuaVariantRetirementResult::completed) return result;

    const std::uint32_t captured_bits = current_word(home);
    put_word(static_cast<unsigned char*>(receiver) + 4u, 0u);
    put_word(static_cast<unsigned char*>(receiver) + 8u, captured_bits);
    return NativeLuaVariantRetirementResult::completed;
}

} // namespace bsp
